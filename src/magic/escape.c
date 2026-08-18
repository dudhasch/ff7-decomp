//! PSYQ=3.3 CC1=2.6.3

// MAGIC/ESCAPE.BIN -- "Escape": ends the battle with no reward, cast from
// Exit Materia at level 1. See docs/spells/magic/escape.md.
//
// The effect grabs the framebuffer, cuts it into a 21x40 grid of textured
// quads and blows the grid apart. EscapeCaptureScreen copies the two halves of
// the drawing area into the texture pages the quads sample from;
// func_801B009C lays the grid out and gives every tile its own random drift;
// EscapeUpdateGrid runs the two phases of the animation.

#include "common.h"
#include "../battle/battle.h"

// A screen point. Grid corners are copied straight into a POLY_FT4's x/y pair,
// which is only 2-byte aligned, so this cannot be a VECTOR.
typedef struct EscapePoint {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} EscapePoint;

// One corner of the grid: where that corner currently sits on screen, plus the
// distance from the screen centre to it, sampled once on the 8-pixel lattice
// and reused every frame as the phase of the ripple.
typedef struct EscapeCell {
    /* 0x0 */ EscapePoint Pos;
    /* 0x4 */ s32 Dist;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
} EscapeCell;

// One tile of the shattered screen: the two POLY_FT4s it is drawn with (one
// per frame buffer), the four grid corners it spans, and its own position,
// velocity, rotation and spin.
typedef struct EscapeTile {
    /* 0x00 */ POLY_FT4 prim[2];
    /* 0x50 */ EscapeCell* Corner[4];
    /* 0x60 */ SVECTOR Pos;
    /* 0x68 */ SVECTOR Vel;
    /* 0x70 */ SVECTOR Rot;
    /* 0x78 */ SVECTOR RotVel;
} EscapeTile;

// Battle effect instance, as this overlay lays it out.
typedef struct EscapeData {
    /* 0x0 */ s16 Id;
    /* 0x2 */ s16 AnimationFrame;
    char pad4[0x1C];
} EscapeData;

// Battle effect instances.
extern EscapeData D_80162978[];
extern u8 D_800F8380;

s32 EscapeBuf = 0;
s32 EscapeWobble = 0;
VECTOR EscapeScale = {0x4000, 0x4000, 0x4000, 0};
s32 EscapeUnk18 = 0;
EscapeCell EscapeGrid[22][41];
EscapeTile EscapeTiles[840];
POLY_F4 EscapeBackPrims[2];

// func_801B009C is assembled from asm/ in the matching build; it is declared so
// that calling it and taking its address works. Without a declaration gcc
// substitutes 0 for the identifier and emits `move a0,zero` with no diagnostic
// the build surfaces.
void func_801B009C(void);
static void EscapeMainSetup(void);

void MAGIC_Escape(void) { EscapeMainSetup(); }

// Copies the drawing area into the two texture pages the grid samples from.
static void EscapeCaptureScreen(void) {
    RECT left;
    RECT right;

    left.x = g_cDb->DispX;
    right.x = left.x + 0xA0;
    left.y = right.y = g_cDb->DispY;
    left.w = right.w = 0xA0;
    left.h = right.h = 0xA8;
    MoveImage(&left, 0x300, 0);
    MoveImage(&right, 0x280, 0x100);
    DrawSync(0);
}

// Lays out the 21x40 grid: first the radial distance table the ripple is
// driven from, then one tile per cell -- two prims chained into a single list,
// the texture window it samples (the captured screen is split across two
// texture pages, at u 0x300 and 0x280), the four corners it spans, and a
// random velocity and spin.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B009C);
#else
/* 52 of 361 instructions. One structural difference, and it is a cost decision
 * inside gcc's loop optimiser rather than anything about the C. Four values are
 * derived from `col` in the tile loop and gcc keeps four; the target's fourth
 * is `col * 2 - 42`, ours is a giv holding `&EscapeGrid[0][col]` that
 * duplicates the `col * 12` giv already in s3 and that we spill and reload for
 * Corner[0]. The target has no such giv -- it rematerialises the bare
 * `&EscapeGrid` constant and adds its col*12 and row*492 givs to it, exactly as
 * it already does for Corner[1..3] -- and spends the freed slot on `col * 2 -
 * 42` and `row * 2 - 32` rather than the `addiu`/`sll` pair we emit inline. Its
 * frame is 0x60 with five spilled values:
 *
 *     16(sp) n            24(sp) row * 8      48(sp) row * 8 + 8
 *     32(sp) col * 2 - 42 40(sp) row * 2 - 32
 *
 * ours is 0x58 with four. Everything else -- both earlier loops, the prim
 * setup, all four Corner stores, every register -- is identical, and the
 * function is exactly the right length.
 *
 * Why gcc will not make `col * 2 - 42` an induction variable, when it happily
 * makes one of `(col - 20) * 8` in the same loop (`li s7,-160` / `addiu
 * s7,s7,8`): giv benefit scales with uses, and the `* 8` value feeds a
 * four-deep assignment chain while the `* 2` value is used once. Whatever the
 * original wrote, it was not a single-use expression over `col`.
 *
 * 96 phrasings have been measured against the retail overlay, as rows (exact)
 * and shape (after normalising stack offsets and branch targets, so a
 * frame-size change does not cascade). Base is 104/46. None beat it:
 *
 *   - every spelling of the two products -- `col * 2 - 42`, `(col - 21) << 1`,
 *     `2 * (col - 21)`, `col + col - 42`, and the same with the constant split
 *     out or folded into the rand term. All compile to what is below (104/46)
 *     or to 164/152.
 *   - hoisting them into locals, the form that makes `col * 8 - 0xA0` a perfect
 *     giv in the distance loop above: inner-loop top 218/164, reusing x and y
 *     274/226, `row * 2 - 32` in the outer loop where it is genuinely invariant
 *     222/208.
 *   - carrying them by hand as accumulators (`x = -42` per row, `x += 2` per
 *     tile), which is what makes them bivs rather than givs and is the closest
 *     thing to asking for a spill slot in C: 272/210. gcc gives the biv a
 *     register and spills a sixth value instead, so the frame grows to 0x60
 *     with six slots rather than the target's five.
 *   - ten phrasings of the Corner block, to stop the redundant giv forming: a
 *     walked `EscapeCell *c` (264/206), one row pointer (332/290) or two
 *     (316/258) hoisted into the outer loop, flat `&EscapeGrid[0][row * 41 +
 *     col]` (324/272), corners derived from Corner[0] (206/202), reordering the
 *     four stores (212/156). Only `EscapeGrid[row] + col` for Corner[0] and
 *     `&EscapeGrid[row][col + 41]` for Corner[2] are neutral (104/46, 106/48);
 *     the block as written is what the target compiles.
 *   - moving the Corner block elsewhere in the loop body: 226 shape wherever it
 *     goes. Its position is load-bearing.
 *   - separate loop counters for the distance table (162/110), carrying
 *     `row * 8` in a local so the `+ 8` is an add rather than a second
 *     induction variable (270/224).
 *   - declaration order (all five permutations) and `register` on any subset of
 *     the locals: no effect at all. gcc 2.6.3's allocator ignores both here.
 *
 * What did help, and is in the code below: writing `rand()` as the *first*
 * operand of the two Vel sums. `(rand() & 3) + (col - 21) * 2` is two rows
 * better than the other order and the same again for vy -- 108/50 to 104/46 --
 * because the induction value is then computed after the call instead of having
 * to survive it.
 *
 * Permuter: base 2400, best 840 over 27k iterations at -j20, and that 840 is an
 * artefact. Its find is the usual `inline int inline_fn(a, b) { return a * b;
 * }` rewrite; `inline` without `static` emits an out-of-line copy of the
 * helper, which lands ahead of the function and shifts the whole address range.
 * Written as `static inline` so no copy is emitted, the same rewrite measures
 * 140/134 -- worse than doing nothing. The permuter's score does not track the
 * real distance for this function; re-measure anything it produces here. */
void func_801B009C(void) {
    s32 row;
    s32 col;
    s32 n;
    s32 x;
    s32 y;

    for (row = 0; row < 10; row++) {
        D_801518E4[row].D_80151909 = (D_801518E4[row].D_80151909 & 0x7F) | 2;
    }

    for (row = 0; row < 22; row++) {
        for (col = 0; col < 41; col++) {
            x = col * 8 - 0xA0;
            y = row * 8 - 0x78;
            EscapeGrid[row][col].Dist = SquareRoot0(x * x + y * y);
        }
    }

    n = 0;

    for (row = 0; row < 21; row++) {
        for (col = 0; col < 40; col++) {
            EscapeTiles[n].Rot.vx = EscapeTiles[n].Rot.vy =
                EscapeTiles[n].Rot.vz = 0;
            EscapeTiles[n].RotVel.vx = (rand() & 0x3FF) - 0x200;
            EscapeTiles[n].RotVel.vy = (rand() & 0x3FF) - 0x200;
            EscapeTiles[n].RotVel.vz = (rand() & 0x3FF) - 0x200;

            EscapeTiles[n].Pos.vx = EscapeTiles[n].Pos.vy =
                EscapeTiles[n].Pos.vz = 0;
            EscapeTiles[n].Vel.vx = (rand() & 3) + (col - 21) * 2;
            EscapeTiles[n].Vel.vy = (rand() & 3) + (row - 16) * 2;
            EscapeTiles[n].Vel.vz = rand() | 0xFF80;

            SetPolyFT4(&EscapeTiles[n].prim[0]);
            setRGB0(&EscapeTiles[n].prim[0], 0x80, 0x80, 0x80);
            SetPolyFT4(&EscapeTiles[n].prim[1]);
            setRGB0(&EscapeTiles[n].prim[1], 0x80, 0x80, 0x80);

            if (col < 20) {
                EscapeTiles[n].prim[0].u0 = EscapeTiles[n].prim[1].u0 =
                    EscapeTiles[n].prim[0].u2 = EscapeTiles[n].prim[1].u2 =
                        col * 8;
                EscapeTiles[n].prim[0].u1 = EscapeTiles[n].prim[1].u1 =
                    EscapeTiles[n].prim[0].u3 = EscapeTiles[n].prim[1].u3 =
                        col * 8 + 8;
                EscapeTiles[n].prim[0].tpage = GetTPage(2, 0, 0x300, 0);
                EscapeTiles[n].prim[1].tpage = GetTPage(2, 0, 0x300, 0);
            } else {
                EscapeTiles[n].prim[0].u0 = EscapeTiles[n].prim[1].u0 =
                    EscapeTiles[n].prim[0].u2 = EscapeTiles[n].prim[1].u2 =
                        (col - 20) * 8;
                EscapeTiles[n].prim[0].u1 = EscapeTiles[n].prim[1].u1 =
                    EscapeTiles[n].prim[0].u3 = EscapeTiles[n].prim[1].u3 =
                        (col - 20) * 8 + 8;
                EscapeTiles[n].prim[0].tpage = GetTPage(2, 0, 0x280, 0x100);
                EscapeTiles[n].prim[1].tpage = GetTPage(2, 0, 0x280, 0x100);
            }

            EscapeTiles[n].prim[0].v0 = EscapeTiles[n].prim[1].v0 =
                EscapeTiles[n].prim[0].v1 = EscapeTiles[n].prim[1].v1 = row * 8;
            EscapeTiles[n].prim[0].v2 = EscapeTiles[n].prim[1].v2 =
                EscapeTiles[n].prim[0].v3 = EscapeTiles[n].prim[1].v3 =
                    row * 8 + 8;

            EscapeTiles[n].Corner[0] = &EscapeGrid[row][col];
            EscapeTiles[n].Corner[1] = &EscapeGrid[row][col + 1];
            EscapeTiles[n].Corner[2] = &EscapeGrid[row + 1][col];
            EscapeTiles[n].Corner[3] = &EscapeGrid[row + 1][col + 1];

            if (n != 0) {
                CatPrim(&EscapeTiles[n - 1].prim[0], &EscapeTiles[n].prim[0]);
                CatPrim(&EscapeTiles[n - 1].prim[1], &EscapeTiles[n].prim[1]);
            }

            n++;
        }
    }

    SetPolyF4(&EscapeBackPrims[0]);
    setRGB0(&EscapeBackPrims[0], 0, 0, 0);
    setXYWH(&EscapeBackPrims[0], 0, 0, 0x140, 0xA6);
    SetPolyF4(&EscapeBackPrims[1]);
    setRGB0(&EscapeBackPrims[1], 0, 0, 0);
    setXYWH(&EscapeBackPrims[1], 0, 0, 0x140, 0xA6);
}
#endif

// One frame of the animation. Frames 0..29 ripple the grid in place -- every
// corner is pushed away from the screen centre by a sine wave whose phase is
// that corner's distance from the centre -- and the tiles just track their four
// corners. Frames 30..59 cut the tiles loose: each becomes its own quad, spun
// and thrown by the velocity it was given in func_801B009C. Frames 60..64 draw
// nothing, then the effect ends.
void func_801B063C(void) {
    // Scratchpad: the tile's four corners, then their centre in the fifth
    // slot's vx/vy, then the matrix it is drawn with.
    SVECTOR* corner = (SVECTOR*)0x1F800000;
    MATRIX* m = (MATRIX*)0x1F800028;
    EscapeData* effect = &D_80162978[D_8015169C];
    POLY_FT4* prim;
    s32 frame = effect->AnimationFrame;
    s32 row;
    s32 col;
    s32 x;
    s32 y;
    s32 wave;
    s32 h;
    long p;
    long flag;

    if (frame < 30) {
        EscapeWobble += 0x18;

        for (row = 0; row < 22; row++) {
            for (col = 0; col < 41; col++) {
                x = col * 8 - 0xA0;
                y = row * 8 - 0x78;
                wave = rsin(EscapeGrid[row][col].Dist * 100 - frame * 500);
                wave = ((wave * EscapeWobble) >> 12) + 0x1000;
                wave += EscapeWobble;
                EscapeGrid[row][col].Pos.x = ((x * wave) >> 12) + 0xA0;
                EscapeGrid[row][col].Pos.y = ((y * wave) >> 12) + 0x78;
            }
        }

        for (row = 0; row < 840; row++) {
            *(EscapePoint*)&EscapeTiles[row].prim[EscapeBuf].x0 =
                EscapeTiles[row].Corner[0]->Pos;
            *(EscapePoint*)&EscapeTiles[row].prim[EscapeBuf].x1 =
                EscapeTiles[row].Corner[1]->Pos;
            *(EscapePoint*)&EscapeTiles[row].prim[EscapeBuf].x2 =
                EscapeTiles[row].Corner[2]->Pos;
            *(EscapePoint*)&EscapeTiles[row].prim[EscapeBuf].x3 =
                EscapeTiles[row].Corner[3]->Pos;
        }

    } else {
        frame -= 30;

        if (frame < 30) {
            h = ReadGeomScreen() * 4;

            for (row = 0; row < 840; row++) {
                corner[0].vx = EscapeTiles[row].Corner[0]->Pos.x - 0xA0;
                corner[0].vy = EscapeTiles[row].Corner[0]->Pos.y - 0x78;
                corner[0].vz = 0;
                corner[1].vx = EscapeTiles[row].Corner[1]->Pos.x - 0xA0;
                corner[1].vy = EscapeTiles[row].Corner[1]->Pos.y - 0x78;
                corner[1].vz = 0;
                corner[2].vx = EscapeTiles[row].Corner[2]->Pos.x - 0xA0;
                corner[2].vy = EscapeTiles[row].Corner[2]->Pos.y - 0x78;
                corner[2].vz = 0;
                corner[3].vx = EscapeTiles[row].Corner[3]->Pos.x - 0xA0;
                corner[3].vy = EscapeTiles[row].Corner[3]->Pos.y - 0x78;
                corner[3].vz = 0;

                corner[4].vx = (corner[0].vx + corner[1].vx + corner[2].vx +
                                corner[3].vx) >>
                               2;
                corner[4].vy = (corner[0].vy + corner[1].vy + corner[2].vy +
                                corner[3].vy) >>
                               2;

                corner[0].vx -= corner[4].vx;
                corner[0].vy -= corner[4].vy;
                corner[1].vx -= corner[4].vx;
                corner[1].vy -= corner[4].vy;
                corner[2].vx -= corner[4].vx;
                corner[2].vy -= corner[4].vy;
                corner[3].vx -= corner[4].vx;
                corner[3].vy -= corner[4].vy;

                EscapeTiles[row].Pos.vx += EscapeTiles[row].Vel.vx;
                EscapeTiles[row].Pos.vy += EscapeTiles[row].Vel.vy;
                EscapeTiles[row].Pos.vz += EscapeTiles[row].Vel.vz;
                EscapeTiles[row].Vel.vz -= EscapeTiles[row].Vel.vz >> 4;
                EscapeTiles[row].Vel.vy += 4;

                RotMatrixYXZ(&EscapeTiles[row].Rot, m);
                ScaleMatrix(m, &EscapeScale);

                m->t[0] = corner[4].vx * 4 + EscapeTiles[row].Pos.vx;
                m->t[1] = corner[4].vy * 4 + EscapeTiles[row].Pos.vy;
                m->t[2] = h + EscapeTiles[row].Pos.vz;

                SetRotMatrix(m);
                SetTransMatrix(m);
                prim = &EscapeTiles[row].prim[EscapeBuf];
                RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                              (long*)&prim->x0, (long*)&prim->x1,
                              (long*)&prim->x2, (long*)&prim->x3, &p, &flag);

                EscapeTiles[row].Rot.vx += EscapeTiles[row].RotVel.vx;
                EscapeTiles[row].Rot.vy += EscapeTiles[row].RotVel.vy;
                EscapeTiles[row].Rot.vz += EscapeTiles[row].RotVel.vz;
            }

        } else {
            frame -= 30;

            if (frame >= 5) {
                effect->Id = -1;
                return;
            }

            goto flip;
        }
    }

    AddPrims(&g_cDb->unk4080[1], &EscapeTiles[0].prim[EscapeBuf],
             &EscapeTiles[839].prim[EscapeBuf]);

flip:
    EscapeBuf ^= 1;
    effect->AnimationFrame++;
}

// Runs once the grid has finished flying apart: releases every battle model
// and marks the effect done, which is what actually ends the battle.
static void EscapeFinish(void) {
    EscapeData* effect = &D_80162978[D_8015169C];
    s32 i;

    D_800F8380 |= 7;

    for (i = 0; i < 10; i++) {
        D_801518E4[i].D_80151909 |= 2;
    }

    effect->Id = -1;
}

static void EscapeMainSetup(void) {
    func_801B009C();
    EscapeCaptureScreen();
    func_800BBEAC(func_801B063C);
    func_800BBEAC(EscapeFinish);
    func_800D55F4(0x20, 0x40, 0x5D);
}
