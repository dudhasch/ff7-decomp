#!/usr/bin/env python3
"""Demote a permuter scratch target's `.rodata` labels to local symbols.

    tools/permuter_rodata_local.py nonmatchings/<fn>

The permuter scores by diffing objdump text, and a relocation shows up there as
the symbol's *name*. gcc emits a function's string literals and jump tables as
**local** labels, so every reference in a candidate object relocates against
`.rodata` with an addend; splat gives the same bytes real names, so the target
object relocates against `D_800A0DE8` and `jtbl_800A0DF4`. Same address, same
bytes, permanent penalty -- and `--stop-on-zero` can then never fire even on a
byte-identical candidate.

`permuter_macros.py align --strings` rewrites *declarations in base.c* to match
the target's names, which is the right fix when the C names the object. It
cannot help here: a string literal and a jump table have no declaration to
rewrite, and there is no way to spell "give this literal external linkage" that
does not also change where gcc puts it (see CLAUDE.md on the named-string
escape, which produces a matching function and a broken overlay). So the target
side is what has to move.

This rewrites `glabel <sym>` to `glabel <sym>, local` inside `.section .rodata`
only -- the prelude's glabel macro already takes a visibility argument -- and
reassembles target.o. `.text` labels are left alone: those are the function's
own name and the labels the scorer needs.
"""
import io
import os
import re
import subprocess
import sys

AS_FLAGS = ["-Iinclude", "-march=r3000", "-mtune=r3000",
            "-no-pad-sections", "-O1", "-G0"]


def demote(text):
    out, in_rodata, n = [], False, 0
    for line in text.split("\n"):
        s = line.strip()
        if s.startswith(".section"):
            in_rodata = ".rodata" in s
        m = re.match(r"glabel\s+(\w+)\s*$", s)
        if in_rodata and m:
            out.append(line.replace(s, "glabel %s, local" % m.group(1)))
            n += 1
        else:
            out.append(line)
    return "\n".join(out), n


def main(argv):
    if len(argv) != 1:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    d = argv[0].rstrip("/\\")
    s_path = os.path.join(d, "target.s")
    with io.open(s_path, encoding="utf-8", newline="") as fh:
        text = fh.read()
    new, n = demote(text)
    if not n:
        print("no .rodata glabels in %s" % s_path)
        return 0
    with io.open(s_path, "w", encoding="utf-8", newline="") as fh:
        fh.write(new)
    # The shim on PATH forwards to mipsel-linux-gnu-as with the build's flags;
    # call the real assembler directly so this works without it.
    cmd = ["mipsel-linux-gnu-as"] + AS_FLAGS + [s_path, "-o",
                                                os.path.join(d, "target.o")]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        print(r.stderr, file=sys.stderr)
        return 1
    print("demoted %d .rodata label(s) and reassembled target.o" % n)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
