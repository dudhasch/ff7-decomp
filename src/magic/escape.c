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
/* 22 of 361 instructions, and both remaining clusters are register-allocation
 * tie-breaks rather than anything about the C. Two levers found this pass took
 * the residue from 104 rows to 22, and both generalise:
 *
 *   1. gcc reduces a value to an induction variable with its own spill slot
 *      only when it is a *user variable computed early and used late*. Written
 *      inline -- in every spelling, listed below -- `(col - 21) * 2` is
 *      computed and consumed inside one statement and stays an
 *      `addiu`/`sll`/`addu` triple in the loop body. Hoisted to the top of the
 *      loop it becomes a giv with a slot, which is what the target has:
 *      `li t0,-42` at the row head and `+= 2` at the inner bottom. Same for
 *      `(row - 16) * 2` hoisted to the top of the row loop. This is the
 *      opposite of the usual advice -- here naming the temporary is what the
 *      compiler wanted.
 *
 *   2. Which of the four `&EscapeGrid[...]` corner addresses is *evaluated*
 *      first decides whether gcc strength-reduces the corner address at all.
 *      With `&EscapeGrid[row][col]` first -- in every phrasing tried, including
 *      a named `EscapeCell *`, `EscapeGrid[row] + col` and `[row + 0][col + 0]`
 *      -- gcc builds a giv `&EscapeGrid + col * 12`, spills it and reloads it
 *      once per tile, duplicating the `col * 12` giv already in s3 and costing
 *      a sixth stack slot. With `&EscapeGrid[row][col + 1]` evaluated first the
 *      giv never forms and all four corners come from s3, s5 and a
 *      rematerialised base, as the target does. Evaluating it into a temporary
 *      keeps the *store* order 0,1,2,3, which the target also has.
 *
 * What is left:
 *
 *   - 12 rows: the two row-level givs swap slots. The target has `row * 2 - 32`
 *     at 40(sp) and `row * 8 + 8` at 48(sp); we have them the other way round,
 *     so the two `li`s in the preheader, the `lbu` reload at the row head, the
 *     `lw` in the Vel.vy term and the two increments at the row bottom all name
 *     the other slot. gcc numbers reduced-giv pseudos in reverse of the order
 * it scans them and assigns slots in pseudo order, so the target's `row * 2 -
 * 32` is scanned *after* `row * 8 + 8` -- hoisted out of the inner loop by gcc
 * rather than by hand. Every way of arranging that either drops `row * 8 + 8`
 * back into a register (4 slots, 8 instructions short) or reorders three slots
 * instead of two.
 *
 *   - 10 rows: the shared grid base is `EscapeGrid + 12` where the target has
 *     `EscapeGrid`, so the corners are derived with -12/0/+480 rather than
 *     0/+12/+492. gcc materialises whichever base the *first* evaluated corner
 *     needs, and by (2) that corner has to be `[row][col + 1]`. The two
 *     constraints conflict, so one of them has a mechanism not yet understood.
 *
 * Measured and rejected this pass, on top of the ninety-six phrasings the
 * earlier note listed (rows/shape against the retail overlay; this body is
 * 22/18):
 *   - hoisting positions for the two products: both at the inner top 228/162,
 *     vy at the inner top with vx inline 238/178, vy immediately before its use
 *     228/162, either one hand-carried as an accumulator 32/22 to 44/26 -- an
 *     accumulator is a biv, gets a low pseudo and lands at 24(sp).
 *   - corner evaluation orders 2,0,1,3 and 3,0,1,2 rebuild the giv (206/144,
 *     212/148); 1,2,0,3 avoids it but stores in the wrong order (30/26); a
 *     walked `c++` pointer, `c - 1`/`c + 40`/`c + 41` off one temp, and
 *     `(EscapeCell *)EscapeGrid + row * 41 + col` all collapse s5 into a single
 *     flat giv and lose 5 to 12 instructions.
 *   - `(row + 1) * 8` for the v coordinate 224/174; `row * 8` and/or
 *     `row * 8 + 8` hoisted to the row top in every order 52/46 to 176/152.
 *   - declaration order and position of the two locals, and reusing the
 *     distance loop's x/y: no effect on the slot order (22/18), or they flatten
 *     the corner addressing (90/84).
 *
 * The permuter is no help here: its score is dominated by an `inline int
 * inline_fn(a, b)` rewrite whose out-of-line copy shifts the whole address
 * range. Re-measure anything it produces against the overlay. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B009C);
#else
void func_801B009C(void) {
    s32 row;
    s32 col;
    s32 n;
    s32 x;
    s32 y;
    s32 vx;
    s32 vy;

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
        vy = (row - 16) * 2;
        for (col = 0; col < 40; col++) {
            vx = (col - 21) * 2;
            EscapeTiles[n].Rot.vx = EscapeTiles[n].Rot.vy =
                EscapeTiles[n].Rot.vz = 0;
            EscapeTiles[n].RotVel.vx = (rand() & 0x3FF) - 0x200;
            EscapeTiles[n].RotVel.vy = (rand() & 0x3FF) - 0x200;
            EscapeTiles[n].RotVel.vz = (rand() & 0x3FF) - 0x200;

            EscapeTiles[n].Pos.vx = EscapeTiles[n].Pos.vy =
                EscapeTiles[n].Pos.vz = 0;
            EscapeTiles[n].Vel.vx = vx + (rand() & 3);
            EscapeTiles[n].Vel.vy = vy + (rand() & 3);
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

            {
                EscapeCell* c1 = &EscapeGrid[row][col + 1];
                EscapeTiles[n].Corner[0] = &EscapeGrid[row][col];
                EscapeTiles[n].Corner[1] = c1;
                EscapeTiles[n].Corner[2] = &EscapeGrid[row + 1][col];
                EscapeTiles[n].Corner[3] = &EscapeGrid[row + 1][col + 1];
            }

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
