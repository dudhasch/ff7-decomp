#!/usr/bin/env python3
"""Census the field overlay: what is left to decompile, and what is left to name.

    .venv/bin/python3 tools/field_status.py                  # print
    .venv/bin/python3 tools/field_status.py --measure        # + run parked_queue
    .venv/bin/python3 tools/field_status.py --html out.html  # render the tracker

Every number is derived from the tree, so this can be re-run after any change
and the tracker regenerated. Three groups:

  decompilation  per unit: source lines, function bodies, MASPSX_OVERRIDE
                 (parked) and INCLUDE_ASM counts, and how many of the
                 INCLUDE_ASM ones are handwritten assembly. A handwritten
                 function can never become matching C, so it is not remaining
                 work -- counting it as such makes the overlay look further
                 from done than it is.

  parked queue   rows / insertions / length delta per parked body. This is the
                 only part that needs a compile, so it runs only under
                 --measure and is otherwise carried forward from the stored
                 snapshot. Length is the number to read first: rows only
                 compare between bodies of the same length.

  naming         address-named globals still referenced from src/field/, split
                 by whether a frozen .s pins the name. splat writes
                 nonmatchings/<fn>.s only for functions the .c still holds as
                 INCLUDE_ASM, so a parked or handwritten function's .s is
                 frozen at the names of the moment it stopped being one, and a
                 symbol it mentions cannot be renamed until that function
                 matches. That split is the whole scheduling story: naming is
                 downstream of decompiling, not parallel to it.

--snapshot appends the totals to the history in docs/decomp/field-status.json,
dated, so the tracker can show movement. Pass --date to keep it reproducible.
"""
import argparse
import collections
import glob
import io
import json
import os
import re
import subprocess
import sys

PARKED = re.compile(r'MASPSX_OVERRIDE\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', re.S)
INCASM = re.compile(r'INCLUDE_ASM\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', re.S)
BODY = re.compile(r"^\}$", re.M)
DSYM = re.compile(r"\bD_[0-9A-F]{8}\b")
FSYM = re.compile(r"\bfunc_[0-9A-F]{8}\b")
M2C = re.compile(r"\b(?:var|temp)_[a-z0-9_]+\b")
UNK = re.compile(r"\bunk[0-9A-Fa-f]{1,3}\b")
INSN = re.compile(r"^\s*/\* [0-9A-F]+ ", re.M)
QUEUE = re.compile(r"^\s+(\w+)\s+(\d+) rows\s+(\d+) ins\s+length (.+)$", re.M)

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def read(p):
    return io.open(p, encoding="utf-8", errors="replace").read()


def code(p):
    """A source file with its comments removed.

    Park notes name the symbols they discuss, and counting those as references
    overstates the naming debt -- D_800E4900 appeared as "renameable" on the
    strength of one mention inside a comment and no use at all."""
    s = re.sub(r"/\*.*?\*/", " ", read(p), flags=re.S)
    return re.sub(r"//[^\n]*", " ", s)


def asm_for(unit, name):
    p = os.path.join(HERE, "asm/us/field/nonmatchings/%s/%s.s" % (unit, name))
    return p if os.path.exists(p) else None


def census(src_glob="src/field/field*.c"):
    units, frozen_parked, frozen_hand = [], [], []
    hand_insns = 0
    for f in sorted(glob.glob(os.path.join(HERE, src_glob))):
        unit = os.path.basename(f)[:-2]
        s = read(f)
        park, inc = PARKED.findall(s), INCASM.findall(s)
        hand = []
        for n in inc:
            p = asm_for(unit, n)
            if p and "Handwritten function" in read(p):
                hand.append(n)
                hand_insns += len(INSN.findall(read(p)))
        for n in park:
            p = asm_for(unit, n)
            if p:
                frozen_parked.append(p)
        for n in inc:
            p = asm_for(unit, n)
            if p:
                frozen_hand.append(p)
        bodies = len(BODY.findall(s))
        units.append({
            "unit": os.path.basename(f), "lines": s.count("\n"),
            "bodies": bodies, "matching": bodies - len(park),
            "parked": len(park), "include_asm": len(inc),
            "handwritten": len(hand),
        })

    P = set(DSYM.findall("".join(read(p) for p in frozen_parked)))
    H = set(DSYM.findall("".join(read(p) for p in frozen_hand)))
    used, refs = set(), collections.Counter()
    m2c, unnamed_fn = collections.Counter(), set()
    for f in (sorted(glob.glob(os.path.join(HERE, "src/field/*.c")))
              + sorted(glob.glob(os.path.join(HERE, "src/field/*.h")))):
        s = code(f)
        for x in DSYM.findall(s):
            used.add(x)
            refs[x] += 1
        unnamed_fn |= set(FSYM.findall(s))
        for x in M2C.findall(s):
            m2c[x] += 1
    free = sorted(used - P - H)
    # field's own .rodata holds string literals, which are not globals to name
    strings = [x for x in free if 0x800A0000 <= int(x[2:], 16) < 0x800A1400]
    # unkNN members also live in structs declared inside a .c -- FieldBgScroll
    # was twelve of them and invisible to a headers-only scan.
    unk = sum(len(UNK.findall(code(f)))
              for f in ([os.path.join(HERE, "include/game.h")]
                        + sorted(glob.glob(os.path.join(HERE, "src/field/*.h")))
                        + sorted(glob.glob(os.path.join(HERE, "src/field/*.c")))))

    tot = {k: sum(u[k] for u in units) for k in
           ("lines", "bodies", "matching", "parked", "include_asm",
            "handwritten")}
    tot["functions"] = tot["matching"] + tot["parked"] + tot["include_asm"]
    return {
        "units": units, "totals": tot, "handwritten_insns": hand_insns,
        "naming": {
            "globals_total": len(used),
            "globals_refs": sum(refs.values()),
            "renameable": len(free),
            "renameable_strings": len(strings),
            "renameable_real": len(free) - len(strings),
            "renameable_names": [x for x in free if x not in strings],
            "locked": len(used & (P | H)),
            "locked_by_parked_only": len((used & P) - H),
            "locked_by_handwritten": len(used & H),
            "unnamed_functions": len(unnamed_fn),
            "m2c_locals": len(m2c), "m2c_local_refs": sum(m2c.values()),
            "unk_members": unk,
        },
    }


def measure_queue():
    py = os.path.join(HERE, ".venv", "bin", "python3")
    if not os.path.exists(py):
        py = sys.executable
    out = subprocess.run(
        [py, os.path.join(HERE, "tools", "parked_queue.py"),
         "src/field/field*.c", "--jobs", "8"],
        capture_output=True, text=True, cwd=HERE).stdout
    q = []
    for name, rows, ins, length in QUEUE.findall(out):
        d = 0 if length.strip() == "exact" else int(length.split()[0])
        q.append({"name": name, "rows": int(rows), "ins": int(ins),
                  "length_delta": d})
    return q


def render(data, template, out):
    html = read(template).replace("/*{{DATA}}*/ null",
                                  json.dumps(data, indent=1, sort_keys=False))
    io.open(out, "w", encoding="utf-8", newline="\n").write(html)


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("--measure", action="store_true",
                    help="run parked_queue.py for fresh row/length numbers")
    ap.add_argument("--snapshot", action="store_true",
                    help="append the totals to the dated history")
    ap.add_argument("--date", default="", help="date for --snapshot")
    ap.add_argument("--json", default="docs/decomp/field-status.json")
    ap.add_argument("--html", help="render the tracker here")
    ap.add_argument("--template", default="tools/field_status.template.html")
    a = ap.parse_args(argv)

    jpath = os.path.join(HERE, a.json)
    prev = json.loads(read(jpath)) if os.path.exists(jpath) else {}
    data = census()
    data["queue"] = measure_queue() if a.measure else prev.get("queue", [])
    data["queue_measured"] = bool(a.measure) or prev.get("queue_measured", False)
    data["history"] = prev.get("history", [])
    if a.snapshot:
        if not a.date:
            sys.exit("--snapshot needs --date, so the file stays reproducible")
        entry = dict(data["totals"], date=a.date)
        if not data["history"] or data["history"][-1] != entry:
            data["history"] = [h for h in data["history"]
                               if h.get("date") != a.date] + [entry]

    if not os.path.isdir(os.path.dirname(jpath)):
        os.makedirs(os.path.dirname(jpath))
    io.open(jpath, "w", encoding="utf-8", newline="\n").write(
        json.dumps(data, indent=1, sort_keys=False) + "\n")

    t = data["totals"]
    print("field overlay: %d functions -- %d matching, %d parked, %d handwritten"
          % (t["functions"], t["matching"], t["parked"], t["handwritten"]))
    print("  handwritten assembly: %d instructions, never becomes C"
          % data["handwritten_insns"])
    n = data["naming"]
    print("naming: %d address-named globals referenced (%d references)"
          % (n["globals_total"], n["globals_refs"]))
    print("  renameable now %d (%d real, %d .rodata strings)"
          % (n["renameable"], n["renameable_real"], n["renameable_strings"]))
    print("  locked %d -- %d unlock as the parked bodies match, %d by "
          "handwritten .s" % (n["locked"], n["locked_by_parked_only"],
                              n["locked_by_handwritten"]))
    print("  also: %d func_ names, %d m2c locals (%d refs), %d unkNN members"
          % (n["unnamed_functions"], n["m2c_locals"], n["m2c_local_refs"],
             n["unk_members"]))
    print("-> %s" % a.json)
    if a.html:
        render(data, os.path.join(HERE, a.template),
               os.path.join(HERE, a.html))
        print("-> %s" % a.html)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
