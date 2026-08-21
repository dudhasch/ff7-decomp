#!/usr/bin/env python3
"""Compare, per struct member, how wide the target reads it and how wide we do.

`tools/asm_widths.py` reads the target's access width for a symbol. This is
the other half: it counts every access to an array-of-struct global on *both*
sides, keyed by (member offset, opcode), so a typing or prototype error shows
up as a row rather than as a wall of register noise in the diff.

    tools/struct_access_audit.py field2 FieldEntityMovementUpdate \
        --symbol g_FieldEntity --base 0x80074EA4 --size 0x84

    offset   op     want    got
      0x30 lbu       0      8  -8
      0x30 lh       14      5  +9

reads: the target loads +0x30 as a signed halfword fourteen times and we load
it as an unsigned byte eight times, so the member is being narrowed somewhere
-- by a cast at the use site, by the wrong declaration, or (the case this tool
was written for) by a prototype m2c invented for the callee.

Both sides are counted alignment-free, so the numbers stay meaningful while
the function is still hundreds of rows out and the diff cannot line up. The
target side reads `%hi(<interior label>)` out of the .s; splat names those
after their addresses, so the offset is the label minus the array base. Our
side reads the R_MIPS_LO16 addend out of the relocatable object, since the
$at macro leaves the address unresolved until link time.

Requires the function to be live in the .c (not MASPSX_OVERRIDE'd) and its
object already built.
"""

import argparse
import collections
import os
import re
import subprocess
import sys

MEM = ("lw", "lh", "lhu", "lb", "lbu", "sw", "sh", "sb")
MEM_RE = re.compile(r"\t(%s)\t" % "|".join(MEM))


def target_side(asm, base, size):
    out = collections.Counter()
    ins = []
    with open(asm, errors="replace") as fh:
        for line in fh:
            m = re.search(r"\*/\s+(\w+)\s+(.*?)\s*$", line)
            if m:
                ins.append((m.group(1), m.group(2)))
    for i, (_, arg) in enumerate(ins):
        h = re.search(r"%hi\((\w+)\)", arg)
        if not h or not h.group(1).startswith("D_"):
            continue
        off = int(h.group(1)[2:], 16) - base
        if not 0 <= off < size:
            continue
        for op2, _ in ins[i + 1:i + 5]:
            if op2 in MEM:
                out[(off, op2)] += 1
                break
    return out


def our_side(obj, fn, symbol):
    dis = subprocess.check_output(
        ["mipsel-linux-gnu-objdump", "-dr", "--disassemble=" + fn, obj],
        text=True, errors="replace").splitlines()
    out = collections.Counter()
    reloc = re.compile(r"R_MIPS_LO16\s+%s\b" % re.escape(symbol))
    for i, line in enumerate(dis):
        m = re.search(r"\taddiu\t(at,at,\S+)", line)
        if not m or i + 1 >= len(dis) or not reloc.search(dis[i + 1]):
            continue
        off = int(m.group(1).split(",")[2], 0)
        for k in range(i + 2, min(i + 8, len(dis))):
            mm = MEM_RE.search(dis[k])
            if mm:
                out[(off, mm.group(1))] += 1
                break
    return out


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("unit", help="the overlay unit, e.g. field2")
    ap.add_argument("func")
    ap.add_argument("--symbol", required=True, help="the array's own symbol")
    ap.add_argument("--base", required=True, help="its vram address")
    ap.add_argument("--size", required=True, help="one element's size")
    ap.add_argument("--overlay", default="field")
    ap.add_argument("--all", action="store_true",
                    help="print matching rows too")
    args = ap.parse_args()

    base, size = int(args.base, 0), int(args.size, 0)
    asm = "asm/us/%s/nonmatchings/%s/%s.s" % (args.overlay, args.unit, args.func)
    obj = "build/us/src/%s/%s.c.o" % (args.overlay, args.unit)
    for p in (asm, obj):
        if not os.path.exists(p):
            sys.exit("no %s" % p)

    want = target_side(asm, base, size)
    got = our_side(obj, args.func, args.symbol)
    if not got:
        sys.exit("no %s accesses in %s -- is the function still parked?" %
                 (args.symbol, obj))

    print("%-8s %-4s %6s %6s" % ("offset", "op", "want", "got"))
    bad = 0
    for k in sorted(set(want) | set(got)):
        if want[k] == got[k] and not args.all:
            continue
        bad += want[k] != got[k]
        print("  0x%02X %-4s %6d %6d  %+d"
              % (k[0], k[1], want[k], got[k], want[k] - got[k]))
    print("differing: %d ; accesses want=%d got=%d"
          % (bad, sum(want.values()), sum(got.values())))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
