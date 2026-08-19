#!/usr/bin/env python3
"""Dump the differing instruction rows of diff.py for one function."""
import json
import subprocess
import sys

func = sys.argv[1]
maxl = sys.argv[2] if len(sys.argv) > 2 else "600"
proc = subprocess.run(
    [sys.executable, "tools/asm-differ/diff.py", "-o", "--format=json",
     func, "--max-lines", maxl],
    capture_output=True, text=True)
d = json.loads(proc.stdout)
rows = d["rows"]


def txt(side):
    if side is None:
        return "<none>"
    parts = [x.get("text", "") for x in side.get("text", [])]
    return "".join(parts).strip()


n = 0
for r in rows:
    bt = txt(r.get("base"))
    ct = txt(r.get("current"))
    bi = bt.split(":")[-1].strip() if ":" in bt else bt
    ci = ct.split(":")[-1].strip() if ":" in ct else ct
    if bi != ci:
        n += 1
        print(f"{bi:50} | {ci}")
print("TOTAL DIFF ROWS:", n)
