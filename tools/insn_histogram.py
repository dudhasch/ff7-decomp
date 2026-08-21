#!/usr/bin/env python3
"""Compare a built function against its target by opcode and by symbol.

    .venv/bin/python3 tools/insn_histogram.py src/field/field5.c FieldDebugRenderPage

`checkfn.py` and `variant_eval.py` answer "how many rows differ"; on a function
that is hundreds of rows out that number says almost nothing about *what* is
wrong, because one displaced instruction renames every register after it. This
answers a different question -- which instructions are there in the wrong
quantity -- and it stays meaningful at any distance, since it is alignment-free.

Two tables. The opcode histogram folds objdump's aliases back to the mnemonics
splat writes (`move` is `addu ..,$zero`, `li` is `ori ..,$zero`), or every
comparison drowns in spelling. The `%hi` table counts address materialisations
per symbol, which is where an addressing mistake shows up as a clean integer.

Read them as a trade, not as a list of faults. `FieldDebugRenderPage` at +1
instruction reports `nop +14, lui +8` against `addu -12, sll -7, sra -3`: the
target re-expands an index and rebuilds addresses, and gets its load-delay
slots filled for it. That is one sentence about the whole residue, and it is
not visible in 964 diff rows.

The function is unparked for the build and the source restored afterwards, the
same way `variant_eval.py` does it, so a `MASPSX_OVERRIDE` body is fine.
"""
import collections
import io
import os
import re
import shutil
import subprocess
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ALIAS = {"move": "addu", "li": "ori", "b": "beq", "negu": "subu", "not": "nor",
         "nop": "nop"}


def target_ops(src, func):
    unit = os.path.basename(src)[:-2]
    path = os.path.join(REPO, "asm", "us", "field", "nonmatchings", unit,
                        func + ".s")
    if not os.path.exists(path):
        # not a field overlay: find it anywhere under asm/
        hits = [os.path.join(r, f)
                for r, _, fs in os.walk(os.path.join(REPO, "asm"))
                for f in fs if f == func + ".s"]
        if not hits:
            sys.exit("no target asm for %s" % func)
        path = hits[0]
    text = io.open(path, encoding="utf-8", errors="replace").read()
    ops = [m.group(1) for m in
           (re.match(r"\s*/\*[^*]*\*/\s+(\S+)", l) for l in text.splitlines())
           if m and not m.group(1).startswith(".")]
    return ops, collections.Counter(re.findall(r"%hi\(([A-Za-z_]\w*)", text))


def ours_ops(obj, func):
    text = subprocess.run(["mipsel-linux-gnu-objdump", "-drz", obj],
                          capture_output=True, text=True).stdout
    ops, hi, on = [], collections.Counter(), False
    for line in text.splitlines():
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", line)
        if m:
            # -g emits LMnnnn line labels inside the function; they are not
            # the end of it.
            if m.group(1) == func:
                on = True
            elif on and not m.group(1).startswith("LM"):
                break
            continue
        if not on:
            continue
        mm = re.match(r"\s*[0-9a-f]+:\s+[0-9a-f]{8}\s+(\S+)", line)
        if mm:
            ops.append(mm.group(1))
        hm = re.search(r"R_MIPS_HI16\s+(\S+)", line)
        if hm:
            hi[hm.group(1).split("+")[0]] += 1
    return ops, hi


def table(title, ours, tgt, limit):
    rows = [(k, ours[k], tgt[k]) for k in set(ours) | set(tgt)
            if ours[k] != tgt[k]]
    rows.sort(key=lambda r: -abs(r[1] - r[2]))
    print("-- %s" % title)
    if not rows:
        print("   (identical)")
    for k, a, b in rows[:limit]:
        print("   %-26s %5d %5d %+6d" % (k, a, b, a - b))


def main(argv):
    if len(argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    src, func = argv[0], argv[1]
    limit = int(argv[2]) if len(argv) > 2 else 12

    # ninja matches its targets by the path build.ninja spells, which is
    # relative to the repo root; an absolute path is simply not a target.
    obj = "build/us/" + src + ".o"
    backup = tempfile.mktemp(suffix=".c")
    shutil.copyfile(os.path.join(REPO, src), backup)
    try:
        subprocess.run([sys.executable, os.path.join(REPO, "tools", "unpark.py"),
                        src, func], stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL, cwd=REPO)
        r = subprocess.run(["ninja", obj], capture_output=True, text=True,
                           cwd=REPO)
        if r.returncode:
            sys.stderr.write((r.stdout + r.stderr)[-3000:])
            return 1
        ours, ohi = ours_ops(os.path.join(REPO, obj), func)
    finally:
        shutil.copyfile(backup, os.path.join(REPO, src))
        os.unlink(backup)

    tgt, thi = target_ops(src, func)
    co = collections.Counter(ALIAS.get(x, x) for x in ours)
    ct = collections.Counter(ALIAS.get(x, x) for x in tgt)
    print("length ours %d, target %d  (%+d instructions)"
          % (len(ours), len(tgt), len(ours) - len(tgt)))
    table("opcodes (objdump aliases folded to the .s spelling)", co, ct, limit)
    table("%hi materialisations per symbol", ohi, thi, limit)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
