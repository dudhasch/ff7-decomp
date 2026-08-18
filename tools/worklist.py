#!/usr/bin/env python3
"""Generate the ordered work list for one source file.

    .venv/bin/python3 tools/worklist.py src/field/field.c
    .venv/bin/python3 tools/worklist.py src/field/field.c -o docs/worklist-field.md

Answers, in one pass, the four questions that otherwise get re-derived by hand
at the start of every batch -- and re-derived again after every compaction:

  1. What is actually left?    remaining INCLUDE_ASM in the .c, not the full
                               `mako.sh rank` listing (which names every .s in
                               the overlay, decompiled ones included).
  2. What can never match?     .s files marked `/* Handwritten function */`.
  3. What is blocked?          jump tables in a unit whose .rodata phase cannot
                               satisfy them (CLAUDE.md "Jump table alignment"),
                               and BORROWS/LENDS .rodata groups.
  4. What is cheapest first?   a static cost proxy: instruction count, calls,
                               indirect calls, division.

The cost proxy is deliberately not `mako.sh rank` -- that needs the container
and the Go toolchain. Pass `--rank <file>` with saved `mako.sh rank` output to
merge the trained score in as an extra column; the two disagree often enough
to be worth reading together.

Nothing here needs the build container: it reads asm/ and src/ only.
"""
import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from rodata_owner import asm_dir_for, scan, still_asm  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# One `/* ... */` comment spimdisasm writes for functions it could not have
# come from C at all -- GTE-heavy code, hand-scheduled delay slots.
HANDWRITTEN_RE = re.compile(r"Handwritten function")
# `/* OFF ADDR WORD */  mnemonic ...` -- the instruction lines of the .text
# dump. Anything else in the file is directive, label or rodata.
INSN_RE = re.compile(r"^\s*/\* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ \*/\s+(\S+)")
JAL_RE = re.compile(r"^\s*jal\s+([A-Za-z_][A-Za-z0-9_]*)")
JTBL_RE = re.compile(r"jtbl_[0-9A-Fa-f]+")
# The repo's park convention: a near-miss body kept beside the INCLUDE_ASM.
PARKED_RE = re.compile(
    r"#ifndef NON_MATCHINGS\s*\n\s*INCLUDE_ASM\("
    r'"[^"]*", ([A-Za-z0-9_]+)\);')
RANK_RE = re.compile(r"^([0-9.]+):\s+(\S+)\.s")


def analyse(path):
    """Static shape of one .s: instruction count, call mix, hazards."""
    insns = 0
    calls = set()
    indirect = False
    divs = 0
    jtbl = False
    handwritten = False
    section = None
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if HANDWRITTEN_RE.search(line):
                handwritten = True
            if line.startswith(".section"):
                section = line.split()[1] if len(line.split()) > 1 else None
            if JTBL_RE.search(line):
                jtbl = True
            m = INSN_RE.match(line)
            if not m:
                continue
            if section is not None and ".text" not in section:
                continue
            insns += 1
            op = m.group(1)
            if op in ("div", "divu"):
                divs += 1
            elif op == "jalr":
                indirect = True
            elif op == "jal":
                jm = JAL_RE.search(line[line.index("*/") + 2:])
                if jm:
                    calls.add(jm.group(1))
                else:
                    parts = line.split()
                    if parts:
                        calls.add(parts[-1])
    return dict(insns=insns, calls=calls, indirect=indirect, divs=divs,
                jtbl=jtbl, handwritten=handwritten)


def cost(info, verdict, parked):
    """Cheap-first ordering key. Lower is a better next pick.

    Weights are a judgement call, not a fit: they encode "small, leaf, no
    jump table, no division" as the shape that lands in one sitting, which is
    what the session history actually shows landing in one sitting.
    """
    score = info["insns"]
    score += 12 * len(info["calls"])
    score += 60 if info["indirect"] else 0
    score += 40 * info["divs"]
    score += 80 if info["jtbl"] else 0
    if verdict != "SAFE":
        score += 150
    if parked:
        score -= 25  # a written near-miss is a head start, not a fresh start
    return score


def verdict_for(func, owner, refs, pending):
    """SAFE / SHARES / BORROWS / LENDS, mirroring tools/rodata_owner.py."""
    if func not in refs:
        return "UNKNOWN", ""
    borrows = [(label, owner[label]) for label in sorted(refs[func])
               if label in owner and owner[label] != func]
    blocked = [(l, h) for l, h in borrows if h in pending]
    shared = [(l, h) for l, h in borrows if h not in pending]
    lends = []
    for label, holder in sorted(owner.items()):
        if holder != func:
            continue
        users = sorted(f for f, r in refs.items()
                       if label in r and f != func and f in pending)
        if users:
            lends.append((label, users))
    if blocked:
        return "BORROWS", "%s owned by %s" % (blocked[0][0], blocked[0][1])
    if lends:
        return "LENDS", "%s needed by %s" % (
            lends[0][0], ", ".join(lends[0][1][:2]))
    if shared:
        return "SHARES", "%s with %s" % (shared[0][0], shared[0][1])
    return "SAFE", ""


def read_rank(path):
    scores = {}
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = RANK_RE.match(line.strip())
            if m:
                scores[m.group(2)] = float(m.group(1))
    return scores


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("source", help="the .c file, e.g. src/field/field.c")
    ap.add_argument("-o", "--output", help="write markdown here (default stdout)")
    ap.add_argument("--rank", help="saved `mako.sh rank <file>` output to merge in")
    ap.add_argument("--limit", type=int, default=40,
                    help="how many actionable rows to table (default 40)")
    ap.add_argument("--asm-root", default=None,
                    help="repo holding asm/, if not this one (worktrees)")
    args = ap.parse_args(argv)

    source = args.source
    if not os.path.exists(source):
        sys.stderr.write("worklist: no such file: %s\n" % source)
        return 2

    asm_dir = asm_dir_for(source)
    if args.asm_root:
        asm_dir = asm_dir.replace(REPO, os.path.abspath(args.asm_root), 1)
    if not os.path.isdir(asm_dir):
        sys.stderr.write(
            "worklist: %s does not exist.\n"
            "          A fresh worktree has no asm/ -- pass --asm-root <main checkout>.\n"
            % asm_dir)
        return 2

    owner, refs = scan(asm_dir)
    pending = still_asm(source)
    with open(source, encoding="utf-8", errors="replace") as fh:
        parked = set(PARKED_RE.findall(fh.read()))
    ranks = read_rank(args.rank) if args.rank else {}

    rows = []
    for func in sorted(pending):
        s = os.path.join(asm_dir, func + ".s")
        if not os.path.exists(s):
            continue
        info = analyse(s)
        v, why = verdict_for(func, owner, refs, pending)
        rows.append(dict(func=func, verdict=v, why=why, parked=func in parked,
                         rank=ranks.get(func), **info))

    hand = [r for r in rows if r["handwritten"]]
    live = [r for r in rows if not r["handwritten"]]
    live.sort(key=lambda r: (cost(r, r["verdict"], r["parked"]), r["func"]))
    blocked = [r for r in live if r["verdict"] in ("BORROWS", "LENDS")]
    ready = [r for r in live if r["verdict"] not in ("BORROWS", "LENDS")]

    out = []
    w = out.append
    unit = os.path.basename(source)
    w("# Work list — `%s`" % source)
    w("")
    w("Generated by `tools/worklist.py`. Regenerate at the start of every batch;")
    w("do not hand-edit. See CLAUDE.md §1 for how to use it.")
    w("")
    w("| | count |")
    w("| --- | --- |")
    w("| remaining `INCLUDE_ASM` | %d |" % len(pending))
    w("| handwritten — can never match | %d |" % len(hand))
    w("| blocked on a `.rodata` group | %d |" % len(blocked))
    w("| **actionable** | **%d** |" % len(ready))
    w("| …of which already parked near-miss | %d |" % sum(1 for r in ready if r["parked"]))
    w("")
    w("Columns: `i` instructions, `calls` direct calls, `*` indirect call,")
    w("`div` hardware divisions, `jt` jump table, `rank` = `mako.sh rank` score")
    w("if merged in with `--rank`. `P` marks a parked near-miss body already in")
    w("the `.c` — those are the cheapest wins left, finish them first.")
    w("")
    w("## Take these next")
    w("")
    w("| # | function | i | calls | * | div | jt | rodata | rank | P |")
    w("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for n, r in enumerate(ready[:args.limit], 1):
        w("| %d | `%s` | %d | %d | %s | %s | %s | %s | %s | %s |" % (
            n, r["func"], r["insns"], len(r["calls"]),
            "yes" if r["indirect"] else "",
            r["divs"] or "", "yes" if r["jtbl"] else "",
            r["verdict"] if r["verdict"] != "SAFE" else "",
            ("%.3f" % r["rank"]) if r["rank"] is not None else "",
            "P" if r["parked"] else ""))
    if len(ready) > args.limit:
        w("")
        w("_%d more actionable functions below the cut._" % (len(ready) - args.limit))
    w("")
    if any(r["jtbl"] for r in ready[:args.limit]):
        w("A `yes` in `jt` is a warning, not a verdict: the function may still be")
        w("stuck on jump-table `.rodata` alignment until this unit is split on its")
        w("original translation-unit boundaries. Read CLAUDE.md \"Jump table")
        w("alignment\" before spending a budget on one.")
        w("")

    if blocked:
        w("## Blocked — decompile the whole `.rodata` group or skip")
        w("")
        w("| function | i | verdict | why |")
        w("| --- | --- | --- | --- |")
        for r in blocked:
            w("| `%s` | %d | %s | %s |" % (r["func"], r["insns"], r["verdict"], r["why"]))
        w("")

    if hand:
        w("## Handwritten — never attempt")
        w("")
        w("These `.s` files carry `/* Handwritten function */`. No C compiles to")
        w("them. They are not part of the remaining work.")
        w("")
        w(", ".join("`%s`" % r["func"] for r in hand))
        w("")

    text = "\n".join(out) + "\n"
    if args.output:
        # encoding and newline are both explicit: on Windows the default is
        # cp1252 + CRLF, which mangles the em-dashes and makes the file churn
        # in git against everyone else's LF.
        with open(args.output, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        sys.stderr.write("worklist: %s — %d actionable, %d blocked, %d handwritten\n"
                         % (args.output, len(ready), len(blocked), len(hand)))
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
