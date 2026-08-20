#!/usr/bin/env python3
"""Decide whether decompiled functions assemble to the target bytes.

    .venv/bin/python3 tools/checkfn.py [--rows] src/field/field.c OpcodeFuncSwcol ...

Prints one verdict line per function and exits non-zero unless every one of
them matched. Run it after every edit; only run `make build` once it is happy.

`--rows` prints every differing instruction rather than just the first, with
the symbol-alias rows already filtered out -- which is the whole reason to
reach for it on a near-miss, since reading them out of `diff.py` by hand means
separating them from the aliases by eye.

Why this exists rather than reading `diff.py` output by eye:

* `diff.py` shows a window that runs past the end of the requested function,
  so neighbouring functions' diffs look like they belong to this one. This
  scopes the comparison to exactly the instructions the target `.s` contains.
* A difference that is only a symbol *name* (`D_800722C4` in the target `.s`
  versus `g_CurrentEntity` from `config/symbols.*.txt`) resolves to the same
  address and assembles to the same bytes. Those are reported separately from
  real differences instead of being counted against the function.
* `make report` rewrites `build.ninja` to build into `report/build/`. After it
  runs, `ninja build/us/...` finds no such target, prints "no work to do", and
  leaves a stale object behind -- so `diff.py` compares the *previous* build and
  happily reports a match. This refuses to run in that state.
* A function still pinned with `MASPSX_OVERRIDE` is not compiled from the C
  beside it at all: the macro assembles the reference `.s` into the object, so
  the comparison is the target against itself and the verdict is MATCH no
  matter what the `#else` body says. This refuses to run in that state too.
"""
import json
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PYTHON = os.path.join(REPO, ".venv", "bin", "python3")
DIFF = os.path.join(REPO, "tools", "asm-differ", "diff.py")

# "  /* 16AE4 800B6AE4 0000A790 */  lbu  $a3, 0x0($a1)" -- one target instruction.
INSN_RE = re.compile(r"^\s+/\* [0-9A-Fa-f]+ [0-9A-Fa-f]{8} [0-9A-Fa-f]{8} \*/")
SYM_DEF_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
ADDR_NAME_RE = re.compile(r"^(?:D_|jtbl_|func_|D)([0-9A-Fa-f]{8})$")
# `.rodata+0x18`, and also a bare `.rodata` -- objdump omits the `+0x0` for a
# reference to the very start of a section. Without the optional group, an
# item at offset 0 never resolves, so it can neither be recognised as an alias
# nor contribute its section's base to the calibration below. A jump table is
# usually the first thing in .rodata, which is exactly that case.
SECTION_REF_RE = re.compile(r"^\.([a-z]+)(?:\+0x([0-9A-Fa-f]+))?$")


def die(msg):
    sys.stderr.write("checkfn: %s\n" % msg)
    sys.exit(2)


def load_symbols():
    """name -> address, from every hand-maintained and generated symbol file."""
    syms = {}
    cfg = os.path.join(REPO, "config")
    for name in sorted(os.listdir(cfg)):
        if not name.endswith(".txt"):
            continue
        with open(os.path.join(cfg, name), errors="replace") as fh:
            for line in fh:
                m = SYM_DEF_RE.match(line)
                if m:
                    syms.setdefault(m.group(1), int(m.group(2), 16))
    return syms


def asm_path(source, func):
    """src/field/field.c + Foo -> asm/us/field/nonmatchings/field/Foo.s"""
    rel = os.path.relpath(os.path.abspath(source), REPO).replace(os.sep, "/")
    m = re.match(r"^src/([^/]+)/([^/]+)\.c$", rel)
    if not m:
        die("cannot derive the asm directory from %r" % rel)
    overlay, unit = m.group(1), m.group(2)
    return os.path.join(
        REPO, "asm", "us", overlay, "nonmatchings", unit, func + ".s")


def target_insn_count(source, func):
    path = asm_path(source, func)
    if not os.path.exists(path):
        die("no target asm for %s (looked for %s)" % (func, path))
    with open(path, errors="replace") as fh:
        return sum(1 for line in fh if INSN_RE.match(line))


def object_for(source):
    rel = os.path.relpath(os.path.abspath(source), REPO).replace(os.sep, "/")
    return os.path.join(REPO, "build", "us", rel + ".o")


# "src/field/field.c:452: `D_8009AC26' undeclared (first use this function)".
# gcc 2.6.3 says this and *carries on*, folding the unknown value to 0. The
# compile line ends in the assembler, so the pipeline still exits 0 and ninja
# reports success: the object silently contains code you did not write.
DIAG_RE = re.compile(r"^\S.*:\d+: (?!warning:)")


def diagnostics(output):
    return [line for line in output.splitlines() if DIAG_RE.match(line)]


def rebuild(source):
    """Bring the object up to date, refusing to proceed on a stale one."""
    obj = object_for(source)
    rel = os.path.relpath(obj, REPO).replace(os.sep, "/")
    with open(os.path.join(REPO, "build.ninja"), errors="replace") as fh:
        graph = fh.read()
    if ("\nbuild %s:" % rel) not in graph:
        die("build.ninja has no rule for %s.\n"
            "         `make report` rewrites build.ninja to build into "
            "report/build/, and\n"
            "         plain ninja then silently does nothing. Run `make build` "
            "to restore it." % rel)
    proc = subprocess.run(["ninja", rel], cwd=REPO, check=False,
                          capture_output=True, text=True)
    if not os.path.exists(obj):
        die("%s was not built" % rel)
    if os.path.getmtime(obj) < os.path.getmtime(source):
        die("%s is older than %s -- the object is stale" % (rel, source))

    # ninja only prints the compile output when it actually compiles, so keep
    # the verdict beside the object: a second run that says "no work to do"
    # must not look clean just because the errors have scrolled away.
    stamp = obj + ".checkfn-diag"
    if "no work to do" in proc.stdout:
        bad = open(stamp).read().splitlines() if os.path.exists(stamp) else []
    else:
        bad = diagnostics(proc.stdout + proc.stderr)
        with open(stamp, "w") as fh:
            fh.write("\n".join(bad))
    if bad:
        die("%s compiled with errors -- the object does not match the source.\n"
            "         gcc reports these and keeps going, and the assembler at "
            "the end of the\n"
            "         pipe still exits 0, so ninja calls it a success.\n\n%s"
            % (source, "\n".join("         " + line for line in bad)))
    return obj


def text_of(side):
    if side is None:
        return None
    out = []
    for chunk in side.get("text", []):
        if chunk.get("format") == "source_line_num":
            continue
        out.append(chunk["text"])
    joined = "".join(out)
    # Each side is rendered as "[marker] [source line] 30914:   insn". Keep only
    # what follows the object offset: the marker and line number are presentation,
    # and the offset itself is layout, which an insert/delete would already show.
    return re.sub(r"^.*?[0-9a-f]+:\s*", "", joined, count=1).strip()


def resolve(token, syms, section_bases):
    """Map a symbol reference to an address, so aliases compare equal."""
    token = token.strip()
    m = re.match(r"^%(?:hi|lo)\((.*)\)$", token)
    if m:
        token = m.group(1)
    m = SECTION_REF_RE.match(token)
    if m:
        base = section_bases.get(m.group(1))
        off = int(m.group(2), 16) if m.group(2) else 0
        return None if base is None else base + off
    # `D_80049208+0x1` and `D_80049209` are the same address written two ways;
    # gcc emits the former when you index one array, the .s names the latter.
    # The offset can be negative -- C that reaches an object through its higher
    # neighbour (`(u8*)D_800E4D94 - 4`) relocates against that neighbour with a
    # negative addend, where the .s names the object itself. Same address, same
    # linked bytes, so the two must compare equal.
    m = re.match(r"^(.+?)([-+])0x([0-9A-Fa-f]+)$", token)
    if m:
        base = resolve(m.group(1), syms, section_bases)
        if base is None:
            return None
        off = int(m.group(3), 16)
        return base - off if m.group(2) == "-" else base + off
    if token in syms:
        return syms[token]
    m = ADDR_NAME_RE.match(token)
    if m:
        value = int(m.group(1), 16)
        if value >= 0x80000000:
            return value
    return None


SYM_TOKEN_RE = re.compile(r"%(?:hi|lo)\([^)]*\)|\.[a-z]+\+0x[0-9a-f]+|[A-Za-z_][A-Za-z0-9_]*")


def normalise(text, syms, section_bases):
    """Replace resolvable symbol references with their addresses."""
    def sub(m):
        token = m.group(0)
        # A `lui rX,%hi(sym)` is only ever emitted paired with an `addiu`/load
        # carrying `%lo(sym)`, and the %lo form spells out the full offset
        # (`.rodata+0xe30`) where %hi does not (`.rodata`). Collapse every %hi so
        # the pair is judged by its %lo half, which is the discriminating one.
        if token.startswith("%hi("):
            return "@HI"
        addr = resolve(token, syms, section_bases)
        return "@%08X" % addr if addr is not None else token
    return SYM_TOKEN_RE.sub(sub, text)


def calibrate_sections(rows, syms):
    """Learn e.g. .rodata's base from rows pairing D_800A0674 with .rodata+0x674."""
    votes = {}
    for row in rows:
        base, cur = row.get("base"), row.get("current")
        if not base or not cur:
            continue
        bsym, csym = base.get("symbol"), cur.get("symbol")
        if not bsym or not csym:
            continue
        # A %hi operand never carries an offset -- objdump prints `%hi(.rodata)`
        # whether the item is at +0 or +0x18 -- so it cannot say where the
        # section starts, only that the reference is somewhere inside it.
        # Letting it vote would put the base at addr(symbol) for any item, and
        # a wrong base makes an unrelated address compare equal, i.e. reports a
        # genuine difference as an alias. Only %lo rows carry the offset.
        if csym.startswith("%hi("):
            continue
        m = SECTION_REF_RE.match(re.sub(r"^%(?:hi|lo)\((.*)\)$", r"\1", csym))
        if not m:
            continue
        addr = resolve(bsym, syms, {})
        if addr is None:
            continue
        off = int(m.group(2), 16) if m.group(2) else 0
        votes.setdefault(m.group(1), {}).setdefault(addr - off, 0)
        votes[m.group(1)][addr - off] += 1
    return {sect: max(cands, key=cands.get) for sect, cands in votes.items()}


# `MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldModelLoadAndInit);`
# -- clang-format wraps the call across two lines whenever the folder string and
# the name do not fit, so this has to be matched with DOTALL rather than per
# line. A line-based grep misses roughly one pinned function in eight.
OVERRIDE_RE = re.compile(
    r'MASPSX_OVERRIDE\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', re.S)


def pinned_functions(source):
    with open(os.path.join(REPO, source), errors="replace") as fh:
        return set(OVERRIDE_RE.findall(fh.read()))


def check(source, func, syms, show_rows=False, pinned=()):
    if func in pinned:
        die("%s is still pinned with MASPSX_OVERRIDE in %s.\n"
            "         The macro assembles the reference .s into the "
            "object, so the C beside\n"
            "         it is never compiled and the verdict would be the "
            "target against itself --\n"
            "         MATCH, whatever the #else body says. Unpark it "
            "first."
            % (func, source))
    want = target_insn_count(source, func)
    # diff.py truncates at --max-lines, which defaults to 1024. A function
    # longer than that would silently be judged on its first 1024 instructions
    # only: the tail never enters the comparison, so it can differ freely and
    # still be reported as a match. Ask for the whole thing, with headroom for
    # the insertions and the label rows diff.py interleaves.
    #
    # The flag has to come *after* the function name: given it before, argparse
    # binds the function to `end` rather than `start` and diff.py returns an
    # empty diff -- score 0, no rows, indistinguishable from a match to anything
    # that does not count instructions.
    proc = subprocess.run(
        [PYTHON, DIFF, "-o", "--format=json", func,
         "--max-lines", str(want * 2 + 128)],
        cwd=REPO, capture_output=True, text=True)
    if proc.returncode != 0:
        die("diff.py failed for %s:\n%s" % (func, proc.stderr.strip()))
    rows = json.loads(proc.stdout)["rows"]

    # diff.py keeps going past the end of the function; stop once we have seen
    # every instruction the target .s declares.
    scoped, seen = [], 0
    for row in rows:
        if seen >= want and row.get("base"):
            break
        scoped.append(row)
        if row.get("base"):
            seen += 1
    if seen < want:
        die("diff.py returned only %d of %s's %d instructions -- the verdict "
            "would cover\n         part of the function. Raise --max-lines."
            % (seen, func, want))

    bases = calibrate_sections(scoped, syms)
    real, alias, ins, dele = 0, 0, 0, 0
    first = None
    bad_rows = []
    for row in scoped:
        b, c = text_of(row.get("base")), text_of(row.get("current"))
        if b is None:
            ins += 1
            bad_rows.append(("+", "", c))
        elif c is None:
            dele += 1
            bad_rows.append(("-", b, ""))
        elif b == c:
            continue
        elif normalise(b, syms, bases) == normalise(c, syms, bases):
            alias += 1
        else:
            real += 1
            bad_rows.append(("!", b, c))
            if first is None:
                first = (b, c)

    bad = real + ins + dele
    note = "" if not alias else "  (%d symbol alias%s)" % (
        alias, "" if alias == 1 else "es")
    if bad == 0:
        print("MATCH    %s%s" % (func, note))
        return True
    detail = []
    if real:
        detail.append("%d changed" % real)
    if ins:
        detail.append("%d inserted" % ins)
    if dele:
        detail.append("%d deleted" % dele)
    print("MISMATCH %s  %s%s" % (func, ", ".join(detail), note))
    if show_rows:
        for kind, b, c in bad_rows:
            print("        %s want: %-38s got: %s" % (kind, b, c))
    elif first:
        print("           want: %s" % first[0])
        print("           got:  %s" % first[1])
    return False


def main(argv):
    if len(argv) < 3:
        sys.stderr.write(__doc__)
        return 2
    argv = list(argv)
    show_rows = "--rows" in argv
    if show_rows:
        argv.remove("--rows")
    if len(argv) < 3:
        sys.stderr.write(__doc__)
        return 2
    source, funcs = argv[1], argv[2:]
    if not os.path.exists(source):
        die("no such source file: %s" % source)
    rebuild(source)
    syms = load_symbols()
    pinned = pinned_functions(source)
    return 0 if all(
        [check(source, f, syms, show_rows, pinned) for f in funcs]) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
