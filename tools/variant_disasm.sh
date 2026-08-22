#!/bin/sh
# Score one variant_eval spec and disassemble the object it built.
#
#     sh tools/variant_disasm.sh .variants/my-idea.json /tmp/out.dis
#
# variant_eval.py --keep leaves its private object in a temp directory and
# prints the path; this wires the two together so the disassembly of exactly
# the body that was just scored is available to read.
#
# The check it exists for: a local whose only assignment sits on an
# unreachable path keeps its *use* and loses its *def*, so global_alloc hands
# the undefined value a hard register and the function still compiles, links
# and scores -- sometimes better than the correct program. Grep the function
# for writes to whatever register the diff names:
#
#     sh tools/variant_disasm.sh .variants/base.json /tmp/f.dis
#     awk '/<AddBackgroundToRender>:/,0' /tmp/f.dis | grep -nE '\bs3\b'
#
# A read with no write above it is a bug, not a codegen residue. See
# "Eight ways a clean-looking diff lies" in CLAUDE.md.
set -e

spec="$1"
out="${2:-/tmp/variant.dis}"
if [ -z "$spec" ]; then
    echo "usage: sh tools/variant_disasm.sh <spec.json> [out.dis]" >&2
    exit 2
fi

repo=$(cd "$(dirname "$0")/.." && pwd)
log=$(mktemp)
trap 'rm -f "$log"' EXIT

"$repo/.venv/bin/python3" "$repo/tools/variant_eval.py" "$spec" --keep >"$log" 2>&1 || true
cat "$log"

dir=$(grep '^kept ' "$log" | sed 's/^kept //')
if [ -z "$dir" ]; then
    echo "variant_disasm: variant_eval did not keep an object (see above)" >&2
    exit 1
fi

obj=$(ls "$dir"/*.o 2>/dev/null | head -1)
if [ -z "$obj" ]; then
    echo "variant_disasm: no object in $dir" >&2
    exit 1
fi

mipsel-linux-gnu-objdump -dr "$obj" >"$out"
echo "disassembly -> $out"
