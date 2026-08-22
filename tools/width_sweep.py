#!/usr/bin/env python3
"""Score every alternative width for every scalar local of a parked function.

    .venv/bin/python3 tools/width_sweep.py src/field/field.c FieldMain --jobs 8

CLAUDE.md's first move on a parked body: a wrong length together with an
`lhu`/`lh` or `andi`/`sll`+`sra` imbalance in `insn_histogram.py` is a
*declaration* fact rather than codegen, and the fix is one word. The widths
are a small finite cross-product and `variant_eval.py` scores the whole set in
one run, so there is no reason to sample it by hand -- `FieldMain` went from
81 rows and +1 instruction to 54 and the exact length on two locals whose
types a long park note had never swept.

Each variant retypes exactly one local. The edit replaces the function's whole
declaration block, which makes it unique in the file without needing an anchor
per line -- a bare `    s32 i;` occurs in most functions in these units.

Reads the row count *and* the length, and sorts by length first: a row count
only compares between bodies of the same length, so a variant that improves
rows while growing the body is a step backwards and is reported as such.
"""
import argparse
import io
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FUNC = re.compile(r"^([A-Za-z_][\w \*]*?)(\w+)\s*\([^;{]*?\)\s*\{", re.M)
DECL = re.compile(r"^(\s+)(u8|s8|u16|s16|u32|s32|long|int)( +)(\w+);\s*$")
WIDTHS = ["s8", "u8", "s16", "u16", "s32", "u32"]
RESULT = re.compile(r"^TOTAL (\d+) .*?; target has (\d+) insns\)$", re.M)
LENGTH = re.compile(r"^\s+length (\d+) against (\d+)", re.M)


def body_span(text, func):
    st = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), text, flags=re.S)
    starts = [(m.start(), m.group(2)) for m in FUNC.finditer(st)]
    for k, (pos, name) in enumerate(starts):
        if name == func:
            return pos, starts[k + 1][0] if k + 1 < len(starts) else len(text)
    sys.exit("no function %s in that file" % func)


ANYDECL = re.compile(r"^\s+[A-Za-z_][\w \*]*?[\* ]\**\w+(\[[^\]]*\])?;\s*$")


def decl_block(body):
    """The function's declaration block: every leading one-line declaration.

    Pointer, array and struct locals are kept in the block -- the edit
    replaces the whole thing, so it has to be reproduced verbatim -- but only
    the plain scalars are swept. Stopping the block at the first non-scalar
    line found two of DebugUpdateActor's locals and missed the rest."""
    lines = body.splitlines(True)
    first = last = None
    for i, line in enumerate(lines):
        if i == 0 or not line.strip():
            continue
        if ANYDECL.match(line) and "(" not in line and "=" not in line:
            if first is None:
                first = i
            last = i
        elif first is not None:
            break
    if first is None or not any(DECL.match(l) for l in lines[first:last + 1]):
        sys.exit("no scalar declarations found")
    return "".join(lines[first:last + 1]), lines[first:last + 1]


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("source")
    ap.add_argument("func")
    ap.add_argument("--jobs", type=int, default=8)
    ap.add_argument("--only", help="restrict to this local")
    a = ap.parse_args(argv)

    path = os.path.join(HERE, a.source)
    text = io.open(path, encoding="utf-8", newline="").read()
    lo, hi = body_span(text, a.func)
    block, lines = decl_block(text[lo:hi])
    if text.count(block) != 1:
        sys.exit("declaration block is not unique in the file")

    vdir = os.path.join(HERE, ".variants")
    specs, label = [], {}
    for i, line in enumerate(lines):
        m = DECL.match(line)
        if not m:
            continue          # a pointer/array local: carried, not swept
        name, cur = m.group(4), m.group(2)
        if a.only and name != a.only:
            continue
        for w in WIDTHS:
            if w == cur:
                continue
            new_lines = list(lines)
            new_lines[i] = "%s%s%s%s;\n" % (m.group(1), w,
                                            " " * max(1, len(m.group(3)) +
                                                      len(cur) - len(w)), name)
            tag = "ws_%s_%s_%s" % (a.func[:18], name, w)
            p = os.path.join(vdir, tag + ".json")
            io.open(p, "w", encoding="utf-8", newline="\n").write(json.dumps(
                {"source": a.source.replace("\\", "/"), "func": a.func,
                 "edits": [[block, "".join(new_lines)]]}, indent=1))
            specs.append(p)
            label[tag] = "%s %s -> %s" % (name, cur, w)

    if not specs:
        sys.exit("nothing to sweep")
    print("%d variants over %d scalar locals (%d declarations in the block)"
          % (len(specs), len([l for l in lines if DECL.match(l)]), len(lines)))

    py = os.path.join(HERE, ".venv", "bin", "python3")
    out = subprocess.run(
        [py if os.path.exists(py) else sys.executable,
         os.path.join(HERE, "tools", "variant_eval.py")] + specs +
        ["--jobs", str(a.jobs)], capture_output=True, text=True, cwd=HERE).stdout

    rows = []
    for chunk in out.split("VARIANT ")[1:]:
        tag = chunk.split()[0]
        r = RESULT.search(chunk)
        if not r:
            continue
        L = LENGTH.search(chunk)
        delta = int(L.group(1)) - int(L.group(2)) if L else 0
        rows.append((abs(delta), int(r.group(1)), delta, label.get(tag, tag)))
    rows.sort()
    print("\n  %-28s %7s %8s" % ("change", "rows", "length"))
    for _, n, d, lab in rows:
        print("  %-28s %7d %8s" % (lab, n, "exact" if d == 0 else "%+d" % d))
    for p in specs:
        os.remove(p)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
