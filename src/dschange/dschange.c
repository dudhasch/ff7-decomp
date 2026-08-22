#include "game.h"

/* DSCHANGE.X -- the "please insert disc N" screen.  Two functions, both the
 * same shape: set up two DISPENV/DRAWENV pairs, build a TILE and a DR_MODE
 * into one of two OTs, and spin on the pad while the CD is swapped.
 * func_800A0000 is 790 instructions and drives the swap itself (CdControlB,
 * DS_read, SystemCdromReadChain, LoadImage, GetODE); func_800A0C58 is 586 and
 * is the quieter variant -- no disc I/O, two SystemAkaoExecute calls.
 *
 * The .bss layout is SOLVED and is written out in the externs below; it fell
 * straight out of the PSY-Q sizes once the two pairs were read the right way
 * round.  The earlier reconnaissance note in this file had DISPENV and DRAWENV
 * swapped -- the stride between 0x15E4 and 0x1640 is 0x5C (DRAWENV, which is
 * 0x1C + sizeof(DR_ENV) = 0x1C + 0x40, NOT the 0x58 the header comment in
 * include/psxsdk/libgpu.h claims) and between 0x169C and 0x16B0 is 0x14
 * (DISPENV).  Every interior label splat emitted is then a named field:
 * D_800A15F8 = D_800A15E4[0].tpage, D_800A15FA/FB/FC = dtd/dfe/isbg,
 * D_800A15FD..FF = r0/g0/b0, and the same 0x5C higher for [1];
 * D_800A16AC/AD = D_800A169C[0].isinter/isrgb24, 0x14 higher for [1].
 *
 * func_800A0000 shares its whole render half with func_800A0C58, so the types
 * and the flip/pad block below carry straight over to it. */

INCLUDE_ASM("asm/us/dschange/nonmatchings/dschange", func_800A0000);

extern s32 func_8001C808(void);
extern s32 func_800484A8(void);
extern s32 func_80048540(s32 arg0);
extern s32 GetODE(void);

/* The overlay's own .bss, defined by asm/us/dschange/data/15E0.bss.s.  The
 * interior labels splat gave the fields (D_800A15F8 = draw[0].tpage,
 * D_800A16AC = disp[0].isinter, ...) all fall out of the PSY-Q layouts:
 * sizeof(DRAWENV) is 0x5C and sizeof(DISPENV) is 0x14, and 0x15E4 + 0x5C is
 * 0x1640 while 0x169C + 0x14 is 0x16B0. */
extern s32 D_800A15E0; /* which of the two envs is on screen */
extern DRAWENV D_800A15E4[2];
extern DRAWENV D_800A1640;
extern DISPENV D_800A169C[2];
extern DISPENV D_800A16B0;
extern DISPENV* D_800A16C4;
extern DRAWENV* D_800A16C8;
extern u32 D_800A16CC; /* ~pad0 */
extern u32 D_800A16D0; /* ~pad1 */
extern u32 D_800A16D4; /* pad0 */
extern u32 D_800A16D8; /* pad1 */
extern u32 D_800A16DC; /* pad0 newly pressed */
extern u32 D_800A16E0; /* pad1 newly pressed */
extern u_long D_800A16E4[2];
extern TILE D_800A16EC[2];
extern DR_MODE D_800A170C[2];

/* 223 rows (197 changed / 26 inserted) at the EXACT target length of 586, from
 * a raw m2c seed at 204/-3.  What got it there:
 *
 *   * the .bss typing above -- read the two strides, do not trust m2c's byte
 *     offsets or the `size = 0x58` comment on DRAWENV;
 *   * `pad1 = pad >> 0x10;` as a named local right after func_8001C808(),
 *     which is what the target's `srl a2,v0,0x10` in the first slot after the
 *     call says (all four flip blocks);
 *   * the second DRAWENV/DISPENV of each pair passed to SetDef* through its
 *     own splat label (D_800A1640, D_800A16B0) rather than as `[1]`, which
 *     stops cse deriving it as base+0x5C: 583 -> 585 instructions;
 *   * `idx = 0;` as the function's first statement with `D_800A15E0 = idx;`
 *     later -- the target's otherwise inexplicable `move s2,zero` in the
 *     prologue, and the last instruction of the length: 585 -> 586, exact.
 *
 * The whole 223-row residue is ONE fact and its cascade.  This build uses
 * EIGHT callee-saved registers where the target uses seven, because gcc keeps
 * `&D_800A15E4` and `&D_800A169C` in pseudos from the four SetDef* calls all
 * the way down to the first flip block, where cse substitutes them into
 * `&D_800A169C[D_800A15E0]`.  The target materialises each SetDef* argument
 * straight into `$a0` (so the call clobbers it and nothing survives), then
 * builds a fresh base right before the first flip -- and derives the DISPENV
 * one as `addiu s1,s1,-0x10` off `&D_800A169C[0].isinter`, which it is holding
 * because that byte store is referenced twice.  With two fewer long-lived
 * values the target reuses the registers that held -1 and 0x1E0; we cannot,
 * so every s-register from s1 up is renamed and the frame is 8 bytes short.
 *
 * Measured and rejected, all against this body:
 *   the envs as four separate scalar objects, `(&D_800A169C)[idx]` for the
 *     flips                                            223 (EXACTLY inert)
 *   `SetDefDrawEnv(D_800A15E4, ...)` (array decay)      224
 *   the second env's byte fields through D_800A1640 too 224 (inert on top)
 *   the SetDef* calls moved below the disp byte stores  243
 *   `D_800A16C4 = PutDispEnv(...)` (use the return)     244, -14 instructions
 *   the 0x1E0 argument as a named local                 236
 *
 * The inert result on the first line is the important one: the array-versus-
 * scalar spelling of the objects is not the lever, because fold gives both
 * the same tree.
 *
 * cc1's `-dr` dump answers where the pseudo comes from and rules out the
 * obvious escapes.  `expand_call` precomputes EVERY argument of a call that
 * has a stack argument, so `SetDefDrawEnv(&D_800A15E4, 0, 0, 0x280, 0x1E0)`
 * emits `(set (reg 88) (symbol_ref "D_800A15E4"))` and `(set (reg 89)
 * (const_int 480))` before the call in *both* builds -- that is also where
 * the target's hoisted `li s1,0x1e0` comes from.  Whether the pseudo survives
 * is decided afterwards: with one use it dies at the `move a0,reg88`, combine
 * folds the two into `lui a0`/`addiu a0`, and nothing is left; with a second
 * use it lives.  The second use is cse substituting reg 88 for the bare
 * `symbol_ref` in the first flip's `&D_800A169C[D_800A15E0]`, which it can do
 * because everything from `.L800A0CC8` (ResetGraph) down to the first flip is
 * ONE basic block -- there is no label between them in the target either, so
 * the target's source somehow denies cse that substitution inside a single
 * extended basic block.  Four `do { } while (0);` boundaries were measured to
 * test the block theory: after ClearImage exactly inert (223), the other three
 * 233/233/251.  So it is not a block-structure fact.
 *
 * That leaves the possibility this note cannot test: that the original's flip
 * does not name the array at all -- e.g. it walks a pointer that gcc cannot
 * relate to the SetDef* argument.  The give-away to look for is whether the
 * target ever recomputes `&D_800A169C` from scratch (it does not; it derives
 * it as `addiu s1,s1,-0x10` off `&D_800A169C[0].isinter`, which is the byte
 * store's own address kept in a register because that store is referenced
 * twice).  Reproducing THAT derivation -- getting gcc to keep the isinter
 * store's address rather than the array base -- is the next thing to try, and
 * it is the same lever CLAUDE.md records for `FieldMain`'s `eventCmd`. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/dschange/nonmatchings/dschange", func_800A0C58);
#else
void func_800A0C58(void) {
    RECT rect;
    s32 ret;
    s32 ode;
    s32 idx;
    u32 pad;
    u32 pad1;
    u32 not0;
    u32 not1;
    u_long* src;
    u_long* ot;
    TILE* tile;
    DR_MODE* mode;
    s16 bright;
    s16 frames;

    idx = 0;
    SetDispMask(0);
    do {
        ret = func_800484A8();
        if (ret == -1) {
            VSync(0);
        }
    } while (ret == -1);
    if (ret != 0) {
        do {
        } while (func_80048540(1) != 0);
    }
    ResetGraph(1);
    SetDefDrawEnv(&D_800A15E4[0], 0, 0, 0x280, 0x1E0);
    SetDefDispEnv(&D_800A169C[0], 0, 0, 0x280, 0x1E0);
    SetDefDrawEnv(&D_800A1640, 0, 0, 0x280, 0x1E0);
    SetDefDispEnv(&D_800A16B0, 0, 0, 0x280, 0x1E0);
    D_800A169C[1].isrgb24 = 0;
    D_800A169C[0].isrgb24 = 0;
    D_800A169C[1].isinter = 1;
    D_800A169C[0].isinter = 1;
    D_800A15E4[1].isbg = 0;
    D_800A15E4[0].isbg = 0;
    D_800A15E4[1].dfe = 0;
    D_800A15E4[0].dfe = 0;
    D_800A15E4[1].dtd = 0;
    D_800A15E4[0].dtd = 0;
    D_800A15E4[1].tpage = 0;
    D_800A15E4[0].tpage = 0;
    D_800A15E4[0].r0 = 0;
    D_800A15E4[0].g0 = 0;
    D_800A15E4[0].b0 = 0;
    D_800A15E4[1].r0 = 0;
    D_800A15E4[1].g0 = 0;
    D_800A15E4[1].b0 = 0;
    D_800A15E0 = idx;
    func_80033F40(0x1F480, 0x800, (u_long*)0x800B0000, 0);
    D_8009A000[0] = 0x10;
    D_8009A004[0] = 0x800B0000;
    SystemAkaoExecute();
    func_80033FC4(0x1F400, 0x40000, (u_long*)0x800B0000, 0);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x280;
    rect.h = 0x1E0;
    ClearImage(&rect, 0, 0, 0);

    DrawSync(0);
    VSync(0);
    D_800A15E0 = D_800A15E0 == 0;
    PutDispEnv(&D_800A169C[D_800A15E0]);
    PutDrawEnv(&D_800A15E4[D_800A15E0]);
    D_800A16C4 = &D_800A169C[D_800A15E0];
    D_800A16C8 = &D_800A15E4[D_800A15E0];
    pad = func_8001C808();
    pad1 = pad >> 0x10;
    not0 = D_800A16CC;
    not1 = D_800A16D0;
    D_800A16CC = ~pad;
    D_800A16D4 = pad;
    D_800A16D8 = pad1;
    D_800A16D0 = ~pad1;
    D_800A16DC = not0 & pad;
    D_800A16E0 = not1 & pad1;

    do {
    } while (SystemCdromReadChain() != 0);
    SetDispMask(1);
    bright = 0xFF;
    do {
        DrawSync(0);
        VSync(0);
        D_800A15E0 = D_800A15E0 == 0;
        PutDispEnv(&D_800A169C[D_800A15E0]);
        PutDrawEnv(&D_800A15E4[D_800A15E0]);
        D_800A16C4 = &D_800A169C[D_800A15E0];
        D_800A16C8 = &D_800A15E4[D_800A15E0];
        idx = D_800A15E0;
        pad = func_8001C808();
        pad1 = pad >> 0x10;
        not0 = D_800A16CC;
        not1 = D_800A16D0;
        D_800A16CC = ~pad;
        D_800A16D4 = pad;
        D_800A16D8 = pad1;
        D_800A16D0 = ~pad1;
        D_800A16DC = not0 & pad;
        D_800A16E0 = not1 & pad1;

        ode = GetODE() ^ 1;
        src = (u_long*)(ode * 0x480 + 0x800B0000);
        rect.w = 0x240;
        rect.x = 0x20;
        rect.y = 0x20;
        rect.h = 1;
        for (rect.y = ode + 0x20; rect.y < 0x1C0; rect.y += 2) {
            LoadImage(&rect, src);
            src += 0x240;
        }
        ot = &D_800A16E4[idx];
        ClearOTagR(ot, 1);
        tile = &D_800A16EC[idx];
        SetTile(tile);
        SetSemiTrans(tile, 1);
        tile->w = 0x240;
        tile->r0 = bright;
        tile->g0 = bright;
        tile->b0 = bright;
        tile->x0 = 0x20;
        tile->y0 = 0x20;
        tile->h = 0x1A0;
        AddPrim(ot, tile);
        mode = &D_800A170C[idx];
        SetDrawMode(mode, 0, 0, GetTPage(2, 2, 0, 0), 0);
        AddPrim(ot, mode);
        DrawOTag(ot);
        if (D_800A16DC & 0x9F0) {
            break;
        }
        bright -= 4;
    } while (bright >= 0);

    while (!(D_800A16DC & 0x9F0)) {
        DrawSync(0);
        VSync(0);
        D_800A15E0 = D_800A15E0 == 0;
        PutDispEnv(&D_800A169C[D_800A15E0]);
        PutDrawEnv(&D_800A15E4[D_800A15E0]);
        D_800A16C4 = &D_800A169C[D_800A15E0];
        D_800A16C8 = &D_800A15E4[D_800A15E0];
        pad = func_8001C808();
        pad1 = pad >> 0x10;
        not0 = D_800A16CC;
        not1 = D_800A16D0;
        D_800A16CC = ~pad;
        D_800A16D4 = pad;
        D_800A16D8 = pad1;
        D_800A16D0 = ~pad1;
        D_800A16DC = not0 & pad;
        D_800A16E0 = not1 & pad1;
    }

    D_8009A000[0] = 0xC1;
    D_8009A004[0] = 0x3C;
    D_8009A008[0] = 0;
    frames = 0x40;
    SystemAkaoExecute();
    do {
        DrawSync(0);
        VSync(0);
        D_800A15E0 = D_800A15E0 == 0;
        PutDispEnv(&D_800A169C[D_800A15E0]);
        PutDrawEnv(&D_800A15E4[D_800A15E0]);
        D_800A16C4 = &D_800A169C[D_800A15E0];
        D_800A16C8 = &D_800A15E4[D_800A15E0];
        idx = D_800A15E0;
        pad = func_8001C808();
        pad1 = pad >> 0x10;
        not0 = D_800A16CC;
        not1 = D_800A16D0;
        D_800A16CC = ~pad;
        D_800A16D0 = ~pad1;
        D_800A16D4 = pad;
        D_800A16D8 = pad1;
        ot = &D_800A16E4[idx];
        D_800A16DC = not0 & pad;
        D_800A16E0 = not1 & pad1;
        ClearOTagR(ot, 1);
        tile = &D_800A16EC[idx];
        SetTile(tile);
        SetSemiTrans(tile, 1);
        tile->r0 = 8;
        tile->g0 = 8;
        tile->b0 = 8;
        tile->w = 0x280;
        tile->x0 = 0;
        tile->y0 = 0;
        tile->h = 0x1E0;
        AddPrim(ot, tile);
        mode = &D_800A170C[idx];
        SetDrawMode(mode, 0, 0, GetTPage(2, 2, 0, 0), 0);
        AddPrim(ot, mode);
        DrawOTag(ot);
        frames--;
    } while (frames != 0);
}
#endif
