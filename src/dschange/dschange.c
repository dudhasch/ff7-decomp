#include "common.h"

/* DSCHANGE.X -- the "please insert disc N" screen. Not started; this is
 * reconnaissance, so the next pass does not re-derive it.
 *
 * Both functions are the same shape: set up two DISPENV/DRAWENV pairs, build a
 * handful of TILE/SPRT primitives into two OTs, and spin on the pad while the
 * CD is swapped. func_800A0000 is 790 instructions and drives the swap itself
 * (CdControlB, DS_read, SystemCdromReadChain, LoadImage, GetODE);
 * func_800A0C58 is 586 and is the quieter variant -- no disc I/O, two
 * SystemAkaoExecute calls, otherwise the same render loop.
 *
 * The work is almost entirely TYPING, not codegen. Everything from 0x800A15E0
 * up is .bss the overlay owns, and splat has it as a real bss subsegment
 * (asm/us/dschange/data/15E0.bss.s) because the zero region is inside the
 * file -- so unlike brom these are ordinary objects, not undefined_syms
 * absolutes, and they can be given real C types. The interior labels the two
 * functions name are the members:
 *
 *   0x800A15E0  s32          a mode/flag word
 *   0x800A15E4  RECT[?]      0x14 bytes, cleared as five words in func_800A0000
 *   0x800A15F8  u16          + 0x15FA..0x15FF six bytes, one object
 *   0x800A1640  DISPENV[2]?  0x1640/0x1654/0x1656..0x165B are one record,
 *                            0x1640 + 0x14 = 0x1654 so read the accesses
 *                            against the DISPENV/DRAWENV layout first
 *   0x800A169C  DRAWENV      0x169C + 0x14 = 0x16B0 (tpage), +0x16/17/18 at
 *                            0x16AC/0x16AD -- that spacing types it outright
 *   0x800A16C0..0x800A170C   the primitive buffers and OTs
 *
 * `tools/asm_widths.py asm/us/dschange/nonmatchings/dschange/func_800A0C58.s`
 * prints the access width per symbol, which is the other half of the typing
 * and is what to run first. Do NOT read m2c's byte offsets as element indices
 * (CLAUDE.md's second m2c trap) -- with 36 interior labels that mistake would
 * be worth hundreds of rows of what looks like register noise.
 *
 * Start with func_800A0C58: it is 200 instructions shorter and has no disc
 * I/O, and the two share their whole render half, so whatever types come out
 * of it carry straight over. */

INCLUDE_ASM("asm/us/dschange/nonmatchings/dschange", func_800A0000);

INCLUDE_ASM("asm/us/dschange/nonmatchings/dschange", func_800A0C58);
