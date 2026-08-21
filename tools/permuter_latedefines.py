#!/usr/bin/env python3
"""Materialise a permuter scratch's latedefine pragmas so base.c will compile.

    tools/permuter_latedefines.py nonmatchings/<fn>/base.c /tmp/check.c

`import.py` leaves every macro named in `config/permuter_settings.toml`'s
`[preserve_macros]` unexpanded, as a `#pragma _permuter latedefine` block plus
an opaque declaration (`void PC_INC();`) so pycparser can still type the file.
Only the permuter's own per-candidate preprocessing turns those pragmas back
into real `#define`s.

That matters for anything that compiles `base.c` directly -- which is what
`permuter_scratch.sh`'s scoreability checks do. cpp ignores an unknown pragma
silently, so a plain compile emits a *call* to `PC_INC` rather than its
expansion: the object comes out short by a `%hi`/`%lo` pair per site and
carries a `PC_INC` relocation the target obviously does not have. On
`OpcodeFuncFadew` that read as 372 bytes against 396 and six mismatched
relocation symbols, which is indistinguishable from a genuinely broken import.
"""
import io
import re
import sys


def convert(text):
    out, defines = [], []
    for line in text.split("\n"):
        m = re.match(r"\s*#pragma _permuter define (.*)$", line)
        if m:
            defines.append("#define " + m.group(1))
            out.append("")
        elif re.match(r"\s*#pragma _permuter latedefine (start|end)\s*$", line):
            out.append("")
        else:
            out.append(line)
    body = "\n".join(out)
    # Drop the opaque declaration import.py emitted for each preserved macro:
    # once the #define is real, `void PC_INC();` expands into nonsense.
    for d in defines:
        name = re.match(r"#define (\w+)", d).group(1)
        body = re.sub(r"^[^\n]*\b%s\s*\(\s*\)\s*;[ \t]*$" % re.escape(name),
                      "", body, flags=re.M)
    return "\n".join(defines) + "\n" + body


def main(argv):
    if len(argv) != 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    src, dst = argv
    with io.open(src, encoding="utf-8", newline="") as fh:
        text = fh.read()
    with io.open(dst, "w", encoding="utf-8", newline="") as fh:
        fh.write(convert(text))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
