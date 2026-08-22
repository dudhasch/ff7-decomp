#!/usr/bin/env python3
"""Print loop.c's move_movables decisions for one variant of one function.

    .venv/bin/python3 tools/loop_movables.py <spec.json|-:src:func> <func>

Each line of the -dL dump is one decision; the SET_SRC of the insn is looked up
in the .cse RTL so a movable can be named ("symbol_ref D_800E08A8",
"const_int 16777215") instead of read as a bare insn number. A movable is
hoisted iff `threshold * savings * lifetime >= insn_count` or it rides a
movable it forces, so `savings * life` is printed as the quantity to move.
"""
import json
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "tools"))
import variant_eval as ve  # noqa: E402

spec_path, func = sys.argv[1], sys.argv[2]
if spec_path.startswith("-:"):
    _, source = spec_path.split(":", 1)
    spec = {"tag": "base", "source": source, "edits": []}
else:
    spec = json.load(open(spec_path, encoding="utf-8"))
tag, source = spec["tag"], spec["source"]

text = ve.unpark(ve.apply_edits(ve.read_base(source), spec.get("edits", [])), func)
out = "/tmp/mv/%s" % tag
subprocess.run(["rm", "-rf", out], check=False)
os.makedirs(out)
with open(os.path.join(out, "v.c"), "w", encoding="utf-8", newline="\n") as fh:
    fh.write(text)

build = ve.resolve_build(source)
subprocess.run(["bash", "-c",
    "mipsel-linux-gnu-cpp -Iinclude -Iinclude/psxsdk -I%s -DUSE_INCLUDE_ASM "
    "-DFF7_STR -lang-c -undef -fno-builtin %s/v.c | bin/str | "
    "iconv --from-code=UTF-8 --to-code=Shift-JIS > %s/v.i"
    % (os.path.join(REPO, os.path.dirname(source)), out, out)], cwd=REPO, check=True)
subprocess.run([os.path.join(REPO, "bin", build["cc1"]), "-quiet", "-mcpu=3000",
                "-mgas", "-O2", "-G0", "-dumpbase", "v.c", "-dL", "-ds",
                "%s/v.i" % out, "-o", "/dev/null"],
               cwd=out, capture_output=True, text=True)


def section(path):
    txt = open(path, encoding="utf-8", errors="ignore").read()
    i = txt.index(";; Function %s\n" % func)
    j = txt.find(";; Function ", i + 10)
    return txt[i:j if j > 0 else len(txt)]


# name every insn by its SET_SRC, from the RTL loop.c was handed
srcs = {}
cse = section(os.path.join(out, "v.c.cse"))
for blk in re.split(r"\n(?=\((?:insn|jump_insn|call_insn|note|code_label|barrier))", cse):
    m = re.match(r"\((?:insn|jump_insn) (\d+) ", blk)
    if not m:
        continue
    flat = " ".join(blk.split())
    s = re.search(r"\(set \(reg[^)]*\) (.*?)\) \d+ \{", flat)
    srcs[int(m.group(1))] = (s.group(1) if s else flat)[:60]

dump = section(os.path.join(out, "v.c.loop"))
insn_count = None
for line in dump.splitlines():
    m = re.match(r"Loop from \d+ to \d+: (\d+) real insns", line)
    if m:
        insn_count = int(m.group(1))
        print("%-22s insn_count %s" % (tag, insn_count))
        continue
    m = re.match(r"Insn (\d+): regno (\d+) \(life (\d+)\),(.*)", line)
    if not m:
        continue
    insn, life, rest = int(m.group(1)), int(m.group(3)), m.group(4).strip()
    sv = re.search(r"savings (\d+)", rest)
    prod = int(sv.group(1)) * life if sv else 0
    verdict = "MOVED" if "moved" in rest else ("stay " if "not desirable" in rest else "ride ")
    name = srcs.get(insn, "?")[:52]
    print("  %-5s insn %-5d life %-4d %-24s prod %-5s %s"
          % (verdict, insn, life, rest.replace("moved to", "->")[:24], prod or "-", name))
