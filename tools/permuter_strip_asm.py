#!/usr/bin/env python3
"""Make a decomp-permuter scratch score the target function, not the overlay.

    .venv/bin/python3 tools/permuter_strip_asm.py nonmatchings/OpcodeFuncSwcol

Run this once after every `import.py`, before `permuter.py`.

`import.py` preprocesses the `.c` with cpp, so by the time it builds the
scratch every `INCLUDE_ASM(...)` has already expanded into

    void __maspsx_include_asm_hack_<fn>() { __asm__(".include \"...<fn>.s\""); }

and each of those is stored in `base.c` as a `#pragma _permuter b64literal`.
The permuter decodes them into every candidate it compiles, so each candidate
object contains the *entire overlay's* assembly -- about 44,000 instructions
for src/field/field.c -- while `target.o` holds only the one function being
permuted. The diff is then dominated by code the permuter cannot influence:

    [FieldDebugPageAddPos]   base score = 4446740
    [OpcodeFuncMpPlus]       base score = 4422490
    [UpdateFieldExitArrows]  base score = 4361645

Scores in the millions that sit within a percent of each other across
completely unrelated functions are the signature. The search still "improves"
by a few thousand and never converges, because the real signal is a rounding
error next to the noise. Stripping the blobs restores a usable score -- the
same UpdateFieldExitArrows scratch drops from 4361645 to 45.

`-DSKIP_ASM=1` on the scratch's `compile.sh` does *not* fix this: the macro
was consumed by cpp at import time and is long gone. It only ever helped by
accident, when an import happened to run while `make report` had left
`build.ninja` in its SKIP_ASM configuration.
"""
import base64
import io
import os
import re
import sys

B64_RE = re.compile(r"#pragma _permuter b64literal (\S+)\s*$")
MARKER = "__maspsx_include_asm_hack_"


def strip(path):
    kept, dropped = [], 0
    for line in io.open(path, encoding="utf-8"):
        m = B64_RE.match(line)
        if m:
            try:
                blob = base64.b64decode(m.group(1)).decode("utf-8", "replace")
            except Exception:
                blob = ""
            if MARKER in blob:
                dropped += 1
                continue
        kept.append(line)
    return "".join(kept), dropped


def main(argv):
    if len(argv) != 2:
        sys.stderr.write(__doc__)
        return 2
    scratch = argv[1]
    base = scratch if scratch.endswith(".c") else os.path.join(scratch, "base.c")
    if not os.path.exists(base):
        sys.stderr.write("permuter_strip_asm: no such file: %s\n" % base)
        return 2

    text, dropped = strip(base)
    if not dropped:
        print("%s: already clean" % base)
        return 0
    backup = base + ".orig"
    if not os.path.exists(backup):
        io.open(backup, "w", encoding="utf-8", newline="").write(
            io.open(base, encoding="utf-8").read())
    io.open(base, "w", encoding="utf-8", newline="").write(text)
    print("%s: dropped %d INCLUDE_ASM blob%s (original kept as %s)"
          % (base, dropped, "" if dropped == 1 else "s",
             os.path.basename(backup)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
