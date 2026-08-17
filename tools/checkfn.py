#!/usr/bin/env python3
"""Decide whether decompiled functions assemble to the target bytes.

    .venv/bin/python3 tools/checkfn.py src/field/field.c OpcodeFuncSwcol ...

Prints one verdict line per function and exits non-zero unless every one of
them matched. Run it after every edit; only run `make build` once it is happy.

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
SECTION_REF_RE = re.compile(r"^\.([a-z]+)\+0x([0-9A-Fa-f]+)$")


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
    subprocess.run(["ninja", rel], cwd=REPO, check=False,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not os.path.exists(obj):
        die("%s was not built" % rel)
    if os.path.getmtime(obj) < os.path.getmtime(source):
        die("%s is older than %s -- the object is stale" % (rel, source))
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
        return None if base is None else base + int(m.group(2), 16)
    # `D_80049208+0x1` and `D_80049209` are the same address written two ways;
    # gcc emits the former when you index one array, the .s names the latter.
    m = re.match(r"^(.+?)\+0x([0-9A-Fa-f]+)$", token)
    if m:
        base = resolve(m.group(1), syms, section_bases)
        return None if base is None else base + int(m.group(2), 16)
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
        m = SECTION_REF_RE.match(re.sub(r"^%(?:hi|lo)\((.*)\)$", r"\1", csym))
        if not m:
            continue
        addr = resolve(bsym, syms, {})
        if addr is None:
            continue
        votes.setdefault(m.group(1), {}).setdefault(
            addr - int(m.group(2), 16), 0)
        votes[m.group(1)][addr - int(m.group(2), 16)] += 1
    return {sect: max(cands, key=cands.get) for sect, cands in votes.items()}


def check(source, func, syms):
    want = target_insn_count(source, func)
    proc = subprocess.run(
        [PYTHON, DIFF, "-o", "--format=json", func],
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

    bases = calibrate_sections(scoped, syms)
    real, alias, ins, dele = 0, 0, 0, 0
    first = None
    for row in scoped:
        b, c = text_of(row.get("base")), text_of(row.get("current"))
        if b is None:
            ins += 1
        elif c is None:
            dele += 1
        elif b == c:
            continue
        elif normalise(b, syms, bases) == normalise(c, syms, bases):
            alias += 1
        else:
            real += 1
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
    if first:
        print("           want: %s" % first[0])
        print("           got:  %s" % first[1])
    return False


def main(argv):
    if len(argv) < 3:
        sys.stderr.write(__doc__)
        return 2
    source, funcs = argv[1], argv[2:]
    if not os.path.exists(source):
        die("no such source file: %s" % source)
    rebuild(source)
    syms = load_symbols()
    return 0 if all([check(source, f, syms) for f in funcs]) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
