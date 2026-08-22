//! PSYQ=3.3 CC1=2.6.3 G=8

#include "common.h"
#include "game.h"
#include "psxsdk/libspu.h"

// This is the first of the two original translation units splat merged into
// one `akao` segment, and the two were built with *different* `-G`. Every
// in-window global these nine functions touch is addressed
// `%gp_rel(<sym>)($gp)` in the target, and every global outside the +-32K
// window around `_gp` (0x80062D44) is `%hi`/`%lo` in the same functions --
// which is what an addressing mode looks like rather than a property of the
// symbols. `src/main/akao2.c` from 0x80029A50 on is `-G0` and reads several
// of the same globals through `%hi`/`%lo`, so one `//!` line cannot serve
// both and the file has to be split. See the `%gp_rel` section of CLAUDE.md.
//
// Tentative definitions, not `extern` declarations, and the difference is the
// whole addressing form: at `-G8` cc1 emits an object this unit defines that
// is no larger than the threshold as a small `.comm`, and maspsx then reaches
// it through `$gp`. Declared `extern` it emits `.extern <sym>,<size>`,
// nothing lands in small data, and every access is the two- or
// three-instruction `%hi`/`%lo` form the target does not have. The real
// definitions are in `src/main/18B8.c` (the two `.sdata` ones) and
// `asm/us/main/data/536C4.bss.s`; `--use-comm-section` (tools/ninja/gen.py)
// keeps these COMMON so the link binds to those rather than seeing two
// definitions.
s32 D_80062E00;
s16 D_80062E08;
s32 D_80062F00;
s32 D_80062F68;
u16 D_80062F70;
s32 D_80062F74;
s16 D_80062F78;
s32 D_80062F84;
s32 D_80062F8C;
u16 D_80062FB8;
s32 D_80062FD8;
s32 D_80062FF8;
s32 D_80063010;
s32 g_AkaoCdVol;
u16 g_AkaoCdVolSlideSteps;
s32 g_AkaoPitchMulMusic;
s16 g_AkaoPitchMulMusicSlideSteps;
s32 g_AkaoTempoMulMusic;
s16 g_AkaoTempoMulMusicSlideSteps;
s32 g_AkaoVolMulMusic;
s16 g_AkaoVolMulMusicSlideSteps;

// One 0x40-byte record per instrument. Only the first 16 bytes are read by
// the driver: two words of SPU envelope, then eight LFO/parameter bytes.
typedef struct {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ u8 unk8;
    /* 0x9 */ u8 unk9;
    /* 0xA */ u8 unkA;
    /* 0xB */ u8 unkB;
    /* 0xC */ u8 unkC;
    /* 0xD */ u8 unkD;
    /* 0xE */ u8 unkE;
    /* 0xF */ u8 unkF;
    /* 0x10 */ u8 unk10[0x30];
} AkaoInstrument; // size:0x40

extern AkaoInstrument D_80075F28[];
extern s32 D_80076C68[];

// SPU transfer-complete callback: clears the "transfer in progress" flag.
void func_800293D0(void) {
    SpuSetTransferCallback(0);
    D_80062E08 = 0;
}

// Arm func_800293D0 as the transfer-complete callback and mark a transfer in
// progress.
void func_800293F4(void) {
    D_80062E08 = 1;
    SpuSetTransferCallback(func_800293D0);
}

static void func_80029424(s32 arg0, s32 arg1) {
    func_800293F4();
    func_80038F04(arg0, arg1);
}

static void func_80029464(s32 arg0, s32 arg1) {
    func_800293F4();
    SpuRead(arg0, arg1);
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800294A4);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800294BC);

/* Kick off a DMA of the header at arg0 into SPU RAM, then stage the payload
 * into the instrument table while the transfer runs and wait for it.
 *
 * PARKED, 3 rows, 29 instructions (exact length). The whole residue is the
 * loop counter: the target holds it in $a1 and this holds it in $a0, so the
 * `li`, the `addiu -1` and the `bnez` all name the wrong register and
 * nothing else in the function differs. $a1 is where func_80029424's second
 * argument was, and it is free by the loop; $a0 is free too and is lower, so
 * gcc takes it.
 *
 * 21 spellings measured, every one of them **exactly 3 rows**: both
 * declaration orders, `s32`/`u32`/`u16` counters, `do`/`while` against
 * `for`, `--i` in the test against `i--` at the top of the body, the copy
 * split into four statements, a separate cursor for `data`, both orders of
 * the `i = 0x800` / `dst = ...` setup, named locals for either or both of
 * func_80029424's arguments (to give $a0 an occupant), `&src[3]`/`src[0]`
 * subscripts, and the call's second argument shared with the counter as one
 * variable (CLAUDE.md's "a value that a call clobbers and a result that
 * replaces it are one variable") -- 4, 7 and 8 rows for the three sharing
 * shapes, so sharing is a regression, not the answer.
 *
 * The pass is settled, not guessed: a `do { } while (0);` barrier before the
 * loop, inside the body and after the loop measures **exactly 3 rows in all
 * three positions**, which per CLAUDE.md means the residue is register
 * allocation and not sched2 -- so every reordering and re-spelling above was
 * inert by construction. What is left is `QTY_CMP_PRI`'s choice of hard
 * register for a block-local quantity, which is not reachable from C without
 * emitting an instruction; this is a park, not a permuter target.
 *
 * func_80029818 below is the same function with a different destination and
 * count and carries the identical residue -- fix one and the other follows.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/akao", func_800297A4);
#else
void func_800297A4(s32* src, s32* data) {
    s32* dst;
    s32 i;

    SpuSetTransferStartAddr(*src++);
    func_80029424((s32)(src + 3), *src);
    i = 0x800;
    dst = (s32*)D_80075F28;
    do {
        i--;
        *dst++ = *data++;
    } while (i != 0);
    func_800294A4();
}
#endif

/* PARKED, 3 rows, 30 instructions (exact length). Identical residue to
 * func_800297A4 above -- see that note for the 21 measured spellings and the
 * barrier probe that says it is register allocation. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/akao", func_80029818);
#else
void func_80029818(s32* src, s32* data) {
    s32* dst;
    s32 i;

    SpuSetTransferStartAddr(*src++);
    func_80029424((s32)(src + 3), *src);
    i = 0x4B0;
    dst = (s32*)D_80076C68;
    do {
        i--;
        *dst++ = *data++;
    } while (i != 0);
    func_800294A4();
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002988C);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80029998);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800299C8);
