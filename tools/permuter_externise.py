"""Extra symbol alignment for a permuter scratch, on top of `permuter_macros.py align`.

`align` handles renamed globals, string literals and interior addresses of plain
arrays. Two more classes cost 5 points per reference on this project and it does
not cover either:

  * **file-scope statics.** They are *defined* in the candidate object, so the
    assembler relocates against the section (`.data+0xa10`), while target.s --
    which holds only the function -- relocates against the name
    (`D_801D24B8+0x4`). Dropping the definitions makes them undefined externals
    in the candidate too. The scratch is never linked, so losing the data is free.

  * **interiors of a named struct.** splat calls the field at 0x8009D7BE
    `D_8009D7BE`; the C calls it `Savemap.config`, which relocates as
    `Savemap+0x10da`. Same address, different text.

The first rewrite is free -- verified byte-identical codegen on func_801D080C.
The second is NOT, and is off unless you pass --struct: replacing
`Savemap.config` with a standalone `extern u16 D_8009D7BE` stops gcc CSE-ing the
struct's base address into a register, so `lui/addiu` once plus `lhu 0(reg)`
becomes a `lui`+`lo` pair at every access. It made this function's score go from
990 to 9620. Keep struct members as struct members and treat those rows as part
of the noise floor; the same is true of compiler-generated jump tables, which
have no name to align to.

Both rewrites touch only the scratch's base.c, never src/. Translate the names
back when a winning candidate goes into src/.

Usage: permuter_externise.py <scratch>/base.c [--struct]
"""

import re
import sys

# Savemap (0x8009C6E4) fields this overlay touches, by the name splat gives them.
STRUCT_FIELDS = {
    "Savemap.battle_speed": ("D_8009D7BC", "extern u8 D_8009D7BC;"),
    "Savemap.battle_msg_speed": ("D_8009D7BD", "extern u8 D_8009D7BD;"),
    "Savemap.config": ("D_8009D7BE", "extern u16 D_8009D7BE;"),
    "Savemap.button_config": ("D_8009D7C0", "extern u8 D_8009D7C0[16];"),
    "Savemap.field_msg_speed": ("D_8009D7D0", "extern u8 D_8009D7D0;"),
}

path = sys.argv[1]
lines = open(path, encoding="utf-8").readlines()
out = []
converted = []
# The declarations must land after the typedefs, or u8/u16 are not in scope yet
# and gcc silently falls back to int -- 32-bit accesses, and a score in the
# thousands. The first file-scope `static` is safely past them.
insert_at = next(
    (n for n, l in enumerate(lines) if re.match(r"^static\s", l)), len(lines)
)

for line in lines:
    m = re.match(r"^static\s+(.*?)\s*(=.*)?;\s*$", line)
    decl = m.group(1) if m else None
    # a function declaration has "(" before any array subscript
    if decl and "(" not in decl.split("[")[0]:
        out.append("extern " + decl + ";\n")
        converted.append(decl.split("[")[0].split()[-1])
    else:
        out.append(line)

# Not attempted: folding D_801D24BC into a u16[3] alongside D_801D24B8 so the
# +4 rides in the relocation's addend instead of the symbol name. It removes 8
# rows of noise but is not codegen-neutral -- as one aggregate gcc caches the
# base address in a register across the reads in switch case 3, where the
# original reloads it, and the score goes 805 -> 990. Same trap as making
# `trigger` a third struct member. Those 8 rows stay part of the floor.

used = []
if "--struct" in sys.argv:
    for member, (name, decl) in sorted(STRUCT_FIELDS.items()):
        if any(member in l for l in out):
            out = [l.replace(member, name) for l in out]
            used.append(decl + "\n")
    out[insert_at:insert_at] = used

open(path, "w", encoding="utf-8", newline="").writelines(out)
print("statics -> extern (%d): %s" % (len(converted), " ".join(converted)))
print("struct interiors renamed: %d" % len(used))
