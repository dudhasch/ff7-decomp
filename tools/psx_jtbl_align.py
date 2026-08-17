#!/usr/bin/env python3
"""Align compiler-generated jump tables the way PSY-Q did: on absolute addresses.

cc1 writes `.rdata` / `.align 3` ahead of every jump table it generates. GNU
`as` honours that **relative to the start of the object's `.rodata` section**,
so a table lands on an 8-byte boundary counted from offset 0. The original
toolchain aligned on the address the table would actually have.

The two agree only when the unit's `.rodata` section itself starts on an 8-byte
boundary. It usually does -- `field` starts at 0x800A0008, `battle` at
0x800A0120 -- and those units match today. But four units start 4 bytes off:

    battle1     0x800A05DC        18B8    0x8001029C
    battle3     0x800A0DD4        savemenu 0x801D017C

There, `.align 3` rounds the *offset* to 8 and so puts the table at an absolute
address that is 4 mod 8 -- 4 bytes past where the retail image has it, taking
every later `.rodata` item with it. That is the mismatch CLAUDE.md describes as
"a string immediately followed by a jump table": the string is a red herring,
it is the section base that decides.

With `--phase 4` the `.align 3` is demoted to `.align 2`, so the table keeps
its natural 4-byte-aligned offset -- which, measured from a base that is itself
4 mod 8, is an 8-byte-aligned address. That is what the original emitted.

Only jump tables are touched: an `.align 3` is rewritten just when the next
directive is a label whose first entry is `.word $L<n>`. `.align 3` in front of
a `double` -- where 8 bytes is a real requirement -- is left alone.

Reads stdin, writes stdout; sits between maspsx and the assembler.
"""
import argparse
import re
import sys

ALIGN3 = re.compile(r"^(\s*\.align\s+)3(\s*)$")
LABEL = re.compile(r"^\s*(\$?\w+):\s*$")
# A jump table's entries are gcc's numbered *code* labels, `$L12`. Constant
# pool labels are `$LC12` and an array of those is ordinary data, not a table.
JTBL_ENTRY = re.compile(r"^\s*\.word\s+\$L\d+\s*$")


def is_jump_table(lines, i):
    """True if the .align 3 at lines[i] introduces a jump table."""
    j = i + 1
    seen_label = False
    while j < len(lines):
        line = lines[j]
        if not line.strip() or line.lstrip().startswith("#"):
            j += 1
            continue
        if not seen_label and LABEL.match(line):
            seen_label = True
            j += 1
            continue
        return seen_label and bool(JTBL_ENTRY.match(line))
    return False


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--phase",
        type=int,
        default=0,
        choices=(0, 4),
        help="the unit's .rodata base address mod 8 (default 0: pass through)",
    )
    args = ap.parse_args()

    lines = sys.stdin.read().split("\n")
    if args.phase == 4:
        for i, line in enumerate(lines):
            m = ALIGN3.match(line)
            if m and is_jump_table(lines, i):
                lines[i] = f"{m.group(1)}2{m.group(2)}"
    sys.stdout.write("\n".join(lines))


if __name__ == "__main__":
    main()
