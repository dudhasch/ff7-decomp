#!/usr/bin/env python3
"""Report whether a function can be decompiled without disturbing .rodata.

    .venv/bin/python3 tools/rodata_owner.py src/field/field.c OpcodeFuncMenu2
    .venv/bin/python3 tools/rodata_owner.py src/field/field.c --all

Run this *before* writing the C. Two ways a function that diffs perfectly can
still fail the link check, both of them invisible in `diff.py`:

BORROWS -- the function references a .rodata label that a *different* `.s` file
    defines. `menu2` prints the string `menu` owns. Writing the literal makes gcc
    emit a second copy of it, which shifts every later .rodata offset and breaks
    the overlay. Decompile the owner in the same change, or leave this function
    alone.

LENDS -- the function owns a .rodata label that other, still-INCLUDE_ASM `.s`
    files reference. `IfCheck` owns the `"ope err="` that both If2Check* use.
    Decompiling it alone removes the definition and the link fails with an
    undefined reference. Decompile the whole group together.

A function reported SAFE owns everything it references and lends nothing.
"""
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GLABEL_RE = re.compile(r"^glabel\s+([A-Za-z_][A-Za-z0-9_]*)")
REF_RE = re.compile(r"%hi\(([A-Za-z_][A-Za-z0-9_]*)\)")
SECTION_RE = re.compile(r"^\.section\s+(\S+)")


def scan(asm_dir):
    """-> (owner: label -> file, refs: file -> {labels})"""
    owner, refs = {}, {}
    for name in sorted(os.listdir(asm_dir)):
        if not name.endswith(".s"):
            continue
        stem = name[:-2]
        section = None
        refs[stem] = set()
        with open(os.path.join(asm_dir, name), errors="replace") as fh:
            for line in fh:
                m = SECTION_RE.match(line)
                if m:
                    section = m.group(1)
                    continue
                if line.startswith(".text"):
                    section = ".text"
                    continue
                m = GLABEL_RE.match(line)
                if m and section == ".rodata":
                    owner[m.group(1)] = stem
                    continue
                for ref in REF_RE.findall(line):
                    refs[stem].add(ref)
    return owner, refs


def asm_dir_for(source):
    rel = os.path.relpath(os.path.abspath(source), REPO).replace(os.sep, "/")
    m = re.match(r"^src/([^/]+)/([^/]+)\.c$", rel)
    if not m:
        sys.stderr.write("rodata_owner: cannot derive asm dir from %r\n" % rel)
        sys.exit(2)
    return os.path.join(REPO, "asm", "us", m.group(1), "nonmatchings", m.group(2))


def still_asm(source):
    with open(source, errors="replace") as fh:
        body = fh.read()
    return set(re.findall(
        r'INCLUDE_ASM\("[^"]*", ([A-Za-z0-9_]+)\);', body))


def report(func, owner, refs, pending):
    if func not in refs:
        print("UNKNOWN  %s (no .s in this unit)" % func)
        return False
    borrows = sorted(
        (label, owner[label]) for label in refs[func]
        if label in owner and owner[label] != func)
    lends = sorted(
        (label, sorted(f for f, r in refs.items()
                       if label in r and f != func and f in pending))
        for label, holder in owner.items() if holder == func)
    lends = [(label, users) for label, users in lends if users]

    # Borrowing from an owner that is already C is fine: gcc folds the two
    # identical literals into one. It only bites while the owner is still asm.
    blocked = [(l, h) for l, h in borrows if h in pending]
    shared = [(l, h) for l, h in borrows if h not in pending]

    if not blocked and not lends:
        for label, holder in shared:
            print("SHARES   %s -> %s, also used by %s; pass the identical literal"
                  % (func, label, holder))
        if not shared:
            print("SAFE     %s" % func)
        return True
    for label, holder in blocked:
        print("BORROWS  %s -> %s, owned by %s (still INCLUDE_ASM)"
              % (func, label, holder))
    for label, users in lends:
        print("LENDS    %s -> %s, still needed by %s"
              % (func, label, ", ".join(users)))
    return False


def main(argv):
    if len(argv) < 3:
        sys.stderr.write(__doc__)
        return 2
    source = argv[1]
    owner, refs = scan(asm_dir_for(source))
    pending = still_asm(source)
    funcs = sorted(pending) if argv[2] == "--all" else argv[2:]
    ok = True
    for func in funcs:
        ok &= report(func, owner, refs, pending)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
