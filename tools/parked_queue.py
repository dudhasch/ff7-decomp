#!/usr/bin/env python3
"""Measure every parked function in a set of sources, cheapest residue first.

    .venv/bin/python3 tools/parked_queue.py src/field/field*.c --jobs 4

`worklist.py` answers "what is left to decompile"; this answers the other
question, "which of the near-misses is closest right now". Both are meant to be
regenerated per batch rather than remembered -- a row count written into a park
note ages the moment anything upstream of the function changes, and this repo
has had a queue ordered by numbers that were three sessions stale, which sent a
whole budget at a function documented as "-2 instructions" and actually 103
rows out.

It works by handing `variant_eval.py` one no-op spec per `MASPSX_OVERRIDE`
body: an edit that inserts a blank line, which cannot change codegen, so the
score it reports is the parked body's own. That also means every guard
`variant_eval` has applies -- the target function is unparked on its own (so no
neighbouring parked body shifts the offsets), compile diagnostics abort the
run, and symbol aliases are discounted.

The output is one row per function: the diff row count, and the *length*
delta against the target. Read the length first. Rows are only comparable
between two bodies of the same length -- across lengths the count measures how
well the diff happened to align, which is a property of where the insertion
landed rather than of how wrong the body is. A body of the wrong size cannot
match however few rows differ, so a nonzero length is both the thing to fix
first and a sign that structural work is still available; `exact` means what
is left is register allocation or scheduling.

The distinction is not academic. `FieldEntityWalkmechCross` carried a park
note reading "99 changed / +4 instructions" -- the 4 was the *insertion* count
and the body was 11 instructions short, so every lever that would fix it added
instructions and read as a regression by rows. The note had measured the one
that mattered and rejected it on exactly that evidence. Ranked by length
instead, the same function went to 34 rows at the exact size. Conversely
`FieldDebugRenderPage` is 964 rows and one instruction from the right length,
while `FieldModelCreatePktsForPart` is 247 rows at exactly the right size.

Pass --verbose to see the full changed/inserted/deleted breakdown.
"""
import glob
import json
import os
import re
import subprocess
import sys
import tempfile

PARKED = re.compile(r'MASPSX_OVERRIDE\(\s*"[^"]*"\s*,\s*(\w+)\s*\)\s*;', re.S)


def specs_for(source, outdir):
    """One no-op spec per parked body in `source`. Returns [(name, path)]."""
    with open(source, encoding="utf-8", errors="replace") as fh:
        text = fh.read()
    made = []
    for m in PARKED.finditer(text):
        name = m.group(1)
        body = text.index("#else", m.end())
        start = text.index("\n", body) + 1
        # Grow the anchor a line at a time until it is unique in the file: the
        # first line of a parked body is often just `{` or a common signature.
        end = text.index("\n", start)
        for _ in range(6):
            anchor = text[start:end + 1]
            if text.count(anchor) == 1:
                break
            end = text.index("\n", end + 1)
        else:
            print("skip %s: no unique anchor" % name, file=sys.stderr)
            continue
        path = os.path.join(outdir, name + ".json")
        with open(path, "w", encoding="utf-8", newline="\n") as fh:
            json.dump({"tag": name,
                       # forward slashes: variant_eval looks the source up in
                       # build.ninja, which never spells a path with a
                       # backslash even on a Windows host.
                       "source": source.replace("\\", "/"),
                       "func": name,
                       "edits": [[anchor, anchor + "\n"]]}, fh, indent=1)
        made.append((name, path))
    return made


def main(argv):
    jobs = "4"
    verbose = False
    sources = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--jobs":
            i += 1
            jobs = argv[i]
        elif a.startswith("--jobs="):
            jobs = a.split("=", 1)[1]
        elif a == "--verbose":
            verbose = True
        else:
            sources.extend(sorted(glob.glob(a)) or [a])
        i += 1
    if not sources:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    tmp = tempfile.mkdtemp(prefix="parked_queue_", dir=os.path.join(here, ".variants")
                           if os.path.isdir(os.path.join(here, ".variants")) else None)
    made = []
    for s in sources:
        made.extend(specs_for(s, tmp))
    if not made:
        print("no MASPSX_OVERRIDE bodies found", file=sys.stderr)
        return 1
    print("%d parked bodies in %d sources" % (len(made), len(sources)),
          file=sys.stderr)

    py = os.path.join(here, ".venv", "bin", "python3")
    if not os.path.exists(py):
        py = sys.executable
    # Pin a base per source once, or every variant_eval run re-pins and two
    # concurrent runs disagree about what they are measuring against.
    for s in sources:
        subprocess.call([py, os.path.join(here, "tools", "variant_eval.py"),
                         "--pin", s.replace("\\", "/")],
                        stdout=subprocess.DEVNULL)
    cmd = [py, os.path.join(here, "tools", "variant_eval.py")]
    cmd += [p for _, p in made] + ["--jobs=" + jobs]
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    if verbose:
        print(out)
        return 0
    print("=== parked queue, closest first")
    for name, rows, ins, delta in parse(out):
        if rows is None:
            print("  %-34s FAILED" % name)
            continue
        print("  %-34s %5d rows  %3d ins  %s"
              % (name, rows, ins,
                 "length exact" if delta == 0 else
                 "length %+d instructions" % delta))
    return 0


ROW = re.compile(r"^VARIANT (\S+)", re.M)
TOT = re.compile(r"^TOTAL (\d+)\s+\((\d+) changed, (\d+) inserted", re.M)
LEN = re.compile(r"^ +length \d+ against \d+\s+\(([-+]\d+) instructions\)", re.M)


def parse(out):
    """[(name, rows, inserted, length_delta)], length_delta 0 when exact.

    Read straight out of variant_eval's own text rather than its summary, so
    the length line travels with the row count -- keeping them together is the
    whole point of this table.
    """
    blocks = re.split(r"(?=^VARIANT )", out, flags=re.M)
    got = []
    for b in blocks:
        m = ROW.search(b)
        if not m or b.startswith("=== summary"):
            continue
        t, l = TOT.search(b), LEN.search(b)
        if not t:
            got.append((m.group(1), None, None, None))
        else:
            got.append((m.group(1), int(t.group(1)), int(t.group(3)),
                        int(l.group(1)) if l else 0))
    got.sort(key=lambda r: (r[1] is None, r[1] if r[1] is not None else 0))
    return got


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
