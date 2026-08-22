#!/usr/bin/env python3
"""Print local_alloc's ranking terms for every pseudo of one function.

    .venv/bin/python3 tools/qty_pri.py <spec.json|-:src> <func>

`block_alloc` ranks quantities by
`QTY_CMP_PRI = floor_log2(n_refs) * n_refs * size / (death - birth)` and hands
each the lowest-numbered free register, so a caller-saved rotation is a
statement about that ratio. cc1's `-dl` dump prints `n_refs`, `live_length`
and, in the `;; Register N in H.` lines, the hard register each pseudo got.
"""
import json
import math
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "tools"))
import variant_eval as ve  # noqa: E402

spec_path, func = sys.argv[1], sys.argv[2]
if spec_path.startswith("-:"):
    spec = {"tag": "base", "source": spec_path[2:], "edits": []}
else:
    spec = json.load(open(spec_path, encoding="utf-8"))
tag, source = spec["tag"], spec["source"]
text = ve.unpark(ve.apply_edits(ve.read_base(source), spec.get("edits", [])), func)
out = "/tmp/qt/%s" % tag
subprocess.run(["rm", "-rf", out], check=False)
os.makedirs(out)
open(os.path.join(out, "v.c"), "w", newline="\n").write(text)
build = ve.resolve_build(source)
subprocess.run(["bash", "-c",
    "mipsel-linux-gnu-cpp -Iinclude -Iinclude/psxsdk -I%s -DUSE_INCLUDE_ASM "
    "-DFF7_STR -lang-c -undef -fno-builtin %s/v.c | bin/str | "
    "iconv --from-code=UTF-8 --to-code=Shift-JIS > %s/v.i"
    % (os.path.join(REPO, os.path.dirname(source)), out, out)], cwd=REPO, check=True)
subprocess.run([os.path.join(REPO, "bin", build["cc1"]), "-quiet", "-mcpu=3000",
                "-mgas", "-O2", "-G0", "-dumpbase", "v.c", "-dl",
                "%s/v.i" % out, "-o", "/dev/null"], cwd=out, capture_output=True, text=True)

txt = open(os.path.join(out, "v.c.lreg"), encoding="utf-8", errors="ignore").read()
i = txt.index(";; Function %s\n" % func)
j = txt.find(";; Function ", i + 10)
sec = txt[i:j if j > 0 else len(txt)]

REGS = ("zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 s0 s1 s2 s3 s4 s5 s6 s7 "
        "t8 t9 k0 k1 gp sp fp ra").split()
hard = {int(a): int(b) for a, b in re.findall(r";; Register (\d+) in (\d+)\.", sec)}
# name pseudos from the RTL: the insn that sets them
setsrc = {}
for blk in re.split(r"\n(?=\((?:insn|jump_insn|call_insn|note|code_label|barrier))", sec):
    m = re.match(r"\((?:insn|jump_insn) (\d+) ", blk)
    if not m:
        continue
    flat = " ".join(blk.split())
    d = re.search(r"\(set \((reg[^)]*\d+)\) (.*?)\) \d+ \{", flat)
    if d:
        r = re.search(r"(\d+)$", d.group(1).strip())
        if r:
            setsrc.setdefault(int(r.group(1)), d.group(2)[:58])

rows = []
for m in re.finditer(r"Register (\d+) used (\d+) times across (\d+) insns"
                     r"(?:[^\n]*?)(?:; set (\d+) times)?[^\n]*", sec):
    n, refs, life = int(m.group(1)), int(m.group(2)), int(m.group(3))
    size = 2 if re.search(r"\(reg[^)]*:HI %d\)" % n, sec) else 4
    pri = math.floor(math.log2(refs)) * refs * size / max(life, 1) if refs else 0
    h = hard.get(n)
    rows.append((pri, n, refs, life, size, REGS[h] if h is not None and h < len(REGS) else "-",
                 setsrc.get(n, "")))
for r in sorted(rows, key=lambda r: -r[0]):
    print("pri %7.3f  reg %-5d refs %-4d life %-5d size %d  -> %-4s %s" % r)
