#!/usr/bin/env python3
"""Read the access width of every global a target function touches.

m2c writes the byte offset it read out of the `addu` and then renders the
access as pointer arithmetic on whatever the project has already declared the
symbol as -- so `*(SYM + off)` against an `s16[]` addresses twice as far as
the target does, and against an `s32[]` four times. The C is valid, the
function compiles, and the whole diff reads as register noise because every
address in it is wrong. See CLAUDE.md.

The fix is the byte-offset form, `*(s16*)((u8*)&SYM + off)`, and to write it
you need the width per symbol. That is not in the header -- an entity record
has `lw` at one offset and `lhu` at the next -- but it is in the target,
one instruction after each `%hi`. This prints it:

    tools/asm_widths.py asm/us/field/nonmatchings/field2/FieldEntityMove.s

    D_80074EB0     lw     s32
    D_80074EE4     lhu    u16
    D_80074EDA     sb     u8

Ambiguity worth knowing about: a store gives you the width but not the
signedness, so `sh` is reported as `s16` and `sb` as `u8` to match the `lh`
and `lbu` a load of the same field would give. Where a symbol is both loaded
and stored, the load wins.
"""

import argparse
import re
import sys

MEM = ("lw", "lh", "lhu", "lb", "lbu", "sw", "sh", "sb")
LOADS = ("lw", "lh", "lhu", "lb", "lbu")
CTYPE = {"lw": "s32", "sw": "s32", "lh": "s16", "sh": "s16",
         "lhu": "u16", "lb": "s8", "sb": "u8", "lbu": "u8"}

INSN = re.compile(r"\*/\s+(\w+)\s+(.*?)\s*$")
HI = re.compile(r"%hi\((\w+)\)")


def widths(path, window):
    ins = []
    with open(path, errors="replace") as fh:
        for line in fh:
            m = INSN.search(line)
            if m:
                ins.append((m.group(1), m.group(2)))
    out = {}
    for i, (op, arg) in enumerate(ins):
        m = HI.search(arg)
        if not m:
            continue
        sym = m.group(1)
        for op2, _ in ins[i + 1:i + 1 + window]:
            if op2 in MEM:
                # a load pins the signedness; a store only the width
                if sym not in out or (op2 in LOADS and out[sym] not in LOADS):
                    out[sym] = op2
                break
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("asm", help="the target .s")
    ap.add_argument("--window", type=int, default=4,
                    help="how far after a %%hi to look for the access (default 4)")
    ap.add_argument("--prefix", default="",
                    help="only report symbols with this prefix")
    args = ap.parse_args()

    w = widths(args.asm, args.window)
    if not w:
        print("no %hi-addressed symbols in %s" % args.asm, file=sys.stderr)
        return 1
    for sym in sorted(w):
        if sym.startswith(args.prefix):
            print("%-16s %-6s %s" % (sym, w[sym], CTYPE.get(w[sym], "?")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
