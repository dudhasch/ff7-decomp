#!/usr/bin/env python3
"""Compare a built function against its target by opcode and by symbol.

    .venv/bin/python3 tools/insn_histogram.py src/field/field5.c FieldDebugRenderPage

`checkfn.py` and `variant_eval.py` answer "how many rows differ"; on a function
that is hundreds of rows out that number says almost nothing about *what* is
wrong, because one displaced instruction renames every register after it. This
answers a different question -- which instructions are there in the wrong
quantity -- and it stays meaningful at any distance, since it is alignment-free.

Two tables. The opcode histogram folds objdump aliases back to the mnemonics
splat writes (`move` is `addu ..,$zero`, `li` is `ori ..,$zero`), or every
comparison drowns in spelling. The `%hi` table counts address materialisations
per *address*, which is where an addressing mistake shows up as a clean
integer.

Per address, not per name, and that is the whole of what makes the table
readable. MIPS uses REL relocations, so the addend is not in the relocation --
it is split across the `lui` and `%lo` immediate pair and has to be
reassembled. And the two sides spell a symbol differently: our object
relocates against the project name for a global while a frozen `.s` still says
`D_800722C4`. Counted by name, those two facts invent complementary rows on
every function that touches a renamed global -- twelve of them on every
handler in `src/field/field4.c`, all phantom, and quite enough to read as a
diagnosis and send a session chasing an addressing bug that is not there.

Read the tables as a trade, not as a list of faults. `FieldDebugRenderPage` at
+1 instruction reports `nop +14, lui +8` against `addu -12, sll -7, sra -3`:
the target re-expands an index and rebuilds addresses, and gets its load-delay
slots filled for it. That is one sentence about the whole residue, and it is
not visible in 964 diff rows.

The function is unparked for the build and the source restored afterwards, the
same way `variant_eval.py` does it, so a `MASPSX_OVERRIDE` body is fine.
"""
import collections
import glob
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
SYMCFG = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
AUTONAME = re.compile(r"^(?:D|jtbl|func)_([0-9A-Fa-f]{8})$")
HIREF = re.compile(r"%hi\(\s*([A-Za-z_]\w*)\s*"
                   r"(?:\+\s*(0x[0-9A-Fa-f]+|\d+))?\s*\)")


def symbol_addresses():
    """name -> address, from every symbol config plus the self-naming symbols.

    `D_800722C4` states its own address; a renamed global keeps its address in
    `config/symbols.*.txt` or `config/sym_*.txt` as `NAME = 0xADDR;`."""
    addr = {}
    for p in (sorted(glob.glob(os.path.join(REPO, "config", "symbols.*.txt")))
              + sorted(glob.glob(os.path.join(REPO, "config", "sym_*.txt")))):
        for line in io.open(p, encoding="utf-8", errors="replace"):
            m = SYMCFG.match(line)
            if m:
                addr.setdefault(m.group(1), int(m.group(2), 16))
    return addr


def resolve(name, addend, addr):
    """(name, addend) -> a key both sides agree on.

    An address when one is known, otherwise the name itself -- `.rodata` and a
    compiler-local label have no entry anywhere and are still worth counting,
    just not worth folding."""
    base = addr.get(name)
    if base is None:
        m = AUTONAME.match(name)
        base = int(m.group(1), 16) if m else None
    if base is None:
        return name
    return "0x%08X" % (base + addend)


def target_ops(src, func, addr, names):
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
    hi = collections.Counter()
    for name, off in HIREF.findall(text):
        key = resolve(name, int(off, 0) if off else 0, addr)
        names.setdefault(key, name)
        hi[key] += 1
    return ops, hi


def ours_ops(obj, func, addr, names):
    text = subprocess.run(["mipsel-linux-gnu-objdump", "-drz", obj],
                          capture_output=True, text=True).stdout
    ops, on, word, relocs = [], False, {}, {}
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
        mm = re.match(r"\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(\S+)", line)
        if mm:
            ops.append(mm.group(3))
            word[int(mm.group(1), 16)] = int(mm.group(2), 16)
            continue
        rm = re.match(r"([0-9a-f]+):\s+(R_MIPS_\S+)\s+(\S+)", line.strip())
        if rm:
            relocs[int(rm.group(1), 16)] = (rm.group(2),
                                            rm.group(3).split("+")[0])

    los = sorted((o, s) for o, (k, s) in relocs.items() if k == "R_MIPS_LO16")
    hi = collections.Counter()
    for off, (kind, sym) in sorted(relocs.items()):
        if kind != "R_MIPS_HI16":
            continue
        # REL: the addend is the immediate pair, not part of the relocation.
        lo = 0
        for o2, s2 in los:
            if o2 > off and s2 == sym:
                lo = word.get(o2, 0) & 0xFFFF
                if lo >= 0x8000:
                    lo -= 0x10000
                break
        key = resolve(sym, ((word.get(off, 0) & 0xFFFF) << 16) + lo, addr)
        names.setdefault(key, sym)
        hi[key] += 1
    return ops, hi


def table(title, ours, tgt, limit, names=None):
    rows = [(k, ours[k], tgt[k]) for k in set(ours) | set(tgt)
            if ours[k] != tgt[k]]
    rows.sort(key=lambda r: -abs(r[1] - r[2]))
    print("-- %s" % title)
    if not rows:
        print("   (identical)")
    for k, a, b in rows[:limit]:
        label = k if names is None else "%s %s" % (names.get(k, "?"), k)
        print("   %-40s %5d %5d %+6d" % (label[:40], a, b, a - b))


def main(argv):
    if len(argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    src, func = argv[0], argv[1]
    limit = int(argv[2]) if len(argv) > 2 else 12
    addr, names = symbol_addresses(), {}

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
        ours, ohi = ours_ops(os.path.join(REPO, obj), func, addr, names)
    finally:
        shutil.copyfile(backup, os.path.join(REPO, src))
        os.unlink(backup)

    tgt, thi = target_ops(src, func, addr, names)
    co = collections.Counter(ALIAS.get(x, x) for x in ours)
    ct = collections.Counter(ALIAS.get(x, x) for x in tgt)
    print("length ours %d, target %d  (%+d instructions)"
          % (len(ours), len(tgt), len(ours) - len(tgt)))
    table("opcodes (objdump aliases folded to the .s spelling)", co, ct, limit)
    table("%hi materialisations per address", ohi, thi, limit, names)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
