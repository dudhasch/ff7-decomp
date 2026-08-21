#!/bin/bash
#
# Build a decomp-permuter scratch for one function and prove it is scoreable.
#
#   tools/permuter_scratch.sh <func> <src.c> [asm-dir]
#   tools/permuter_scratch.sh FieldMain src/field/field.c
#
# Run it inside the build container (bash, not sh -- it uses process
# substitution). It performs the whole recipe from CLAUDE.md's step 4 --
# unpark, build the object, import, strip the INCLUDE_ASM blobs, align the
# symbol names, retarget, set the weights, drop -g/-gcoff -- and then runs the
# three checks that decide whether a run is worth starting at all. Every one of
# them has silently wasted hours of search at least once:
#
#   1. diagnostics.  gcc 2.6.3 substitutes 0 for an undeclared identifier and
#      keeps generating code, and permuter_macros.py align has been seen to
#      delete a declaration while rewriting names -- so the search runs against
#      a program that computes something else and nothing says so.
#   2. size.  The `base.o` beside the scratch is written by import.py and never
#      rebuilt, so it still holds the whole overlay after the strip; only a
#      fresh compile of base.c says whether the prune happened.
#   3. relocations.  The scorer compares relocation *symbols*, where checkfn.py
#      discounts a difference that is only a name for the same address. Every
#      such alias left in the scratch is a free lever the search will pull, and
#      the winner then measures worse in the build than the body it came from.
#
# A non-empty relocation diff is not always fatal -- a mismatch no candidate
# can change (a local aggregate's .rodata blob, a jump table) is a constant
# penalty rather than a bias. One a candidate *can* change is a bias, and the
# fix is to rewrite those operands in the scratch's target.s and reassemble,
# never to touch asm/.
#
set -u
FN=${1:?usage: permuter_scratch.sh <func> <src.c> [asm-dir]}
SRC=${2:?usage: permuter_scratch.sh <func> <src.c> [asm-dir]}
UNIT=$(basename "$SRC" .c)
OVL=$(basename "$(dirname "$SRC")")
ASM=${3:-asm/us/$OVL/nonmatchings/$UNIT}
D=nonmatchings/$FN
PY=.venv/bin/python3
P=${DECOMP_PERMUTER:-../decomp-permuter}
export PATH="$PWD/tools/permuter-bin:$PATH"

[ -f "$ASM/$FN.s" ] || { echo "no target asm: $ASM/$FN.s"; exit 1; }

# import.py appends -2, -3, ... rather than overwriting, and a leftover suffixed
# directory from an earlier round is then what the next run picks up.
rm -rf "$D" "$D"-[0-9] "$D"-[0-9][0-9] "$D"-M

BK=$(mktemp)
cp "$SRC" "$BK"
$PY tools/unpark.py "$SRC" "$FN" >/dev/null 2>&1
ninja "build/us/$SRC.o" >/dev/null 2>&1
$PY "$P/import.py" "$SRC" "$ASM/$FN.s" 2>&1 | tail -1
cp "$BK" "$SRC"
rm -f "$BK"

[ -d "$D" ] || { echo "import landed elsewhere:"; ls -d "$D"* ; exit 1; }

$PY tools/permuter_strip_asm.py "$D"               >/dev/null 2>&1
$PY tools/permuter_macros.py align "$D" --strings  >/dev/null 2>&1
$PY tools/permuter_macros.py retarget "$D"         >/dev/null 2>&1
$PY tools/permuter_rodata_local.py "$D"          >/dev/null 2>&1
$PY tools/permuter_macros.py weights "$D"          >/dev/null 2>&1
sed -i -e 's/ -g / /g' -e 's/ -gcoff / /g' "$D/compile.sh"

# Extra weights, merged rather than appended: `weights` writes some of these
# already and toml rejects duplicate keys, which kills the run at startup with
# a TomlDecodeError and no candidates at all.
if [ -n "${PERM_WEIGHTS:-}" ]; then
    python3 - "$D/settings.toml" "$PERM_WEIGHTS" <<'PY'
import io, re, sys
path, spec = sys.argv[1], sys.argv[2]
extra = dict(kv.split('=', 1) for kv in spec.split(',') if kv)
out = []
for line in io.open(path, encoding='utf-8', newline='').read().split('\n'):
    m = re.match(r'^([A-Za-z_0-9]+)\s*=', line)
    if m and m.group(1) in extra:
        out.append('%s = %s' % (m.group(1), extra.pop(m.group(1))))
    else:
        out.append(line)
out += ['%s = %s' % kv for kv in sorted(extra.items())]
io.open(path, 'w', encoding='utf-8', newline='').write('\n'.join(out) + '\n')
PY
fi

# base.c is not compilable as written when the function uses a macro from
# [preserve_macros]: those survive only as `#pragma _permuter latedefine`
# blocks, which cpp ignores, so a plain compile emits a call to PC_INC rather
# than its expansion and every check below reads as a broken import. See
# tools/permuter_latedefines.py.
python3 tools/permuter_latedefines.py "$D/base.c" /tmp/permuter_scratch_check.c

echo "=== $FN: compile diagnostics (must be empty)"
"./$D/compile.sh" /tmp/permuter_scratch_check.c x /tmp/permuter_scratch.o 2>&1 | grep -v 'warning:'

echo "=== $FN: .text size (base must be within a few percent of target)"
mipsel-linux-gnu-size /tmp/permuter_scratch.o "$D/target.o"

echo "=== $FN: relocation symbols (empty diff means the score describes the code)"
diff <(mipsel-linux-gnu-objdump -drz /tmp/permuter_scratch.o \
        | grep -oE 'R_MIPS_[A-Z0-9_]+[[:space:]]+[^[:space:]]+' \
        | awk '{print $2}' | sort | uniq -c) \
     <(mipsel-linux-gnu-objdump -drz "$D/target.o" \
        | grep -oE 'R_MIPS_[A-Z0-9_]+[[:space:]]+[^[:space:]]+' \
        | awk '{print $2}' | sort | uniq -c) \
  && echo "clean"
