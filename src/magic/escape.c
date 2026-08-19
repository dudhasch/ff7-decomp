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
/* 12 of 361 instructions. The residue is one register-allocation tie-break:
 * the two row-level induction variables hold each other's stack slot. The
 * target has `row * 2 - 32` at 40(sp) and `row * 8 + 8` at 48(sp); we have them
 * the other way round, so the two `li`s in the preheader, the `lbu` reload at
 * the row head, the `lw` in the Vel.vy term and the two increments at the row
 * bottom all name the other slot. Everything else -- frame size, slot count,
 * instruction count, every giv, every address -- agrees with the target.
 *
 * Four levers got here from a 104-row body, and the first two generalise:
 *
 *   1. gcc 2.6.3 reduces an expression over a loop counter to an induction
 *      variable with its own spill slot only when the value is a *named local
 *      computed early and used late*. Inline, `(rand() & 3) + (col - 21) * 2`
 *      computes and consumes the product inside one statement and stays an
 *      `addiu`/`sll`/`addu` triple. Hoisted to the top of the loop as `vx` it
 *      becomes a giv: `li t0,-42` in the preheader, `+= 2` at the bottom,
 *      `lw`/`addu` at the use -- what the target has, for both `(col - 21) * 2`
 *      and `(row - 16) * 2`.
 *
 *   2. Which sibling address expression is *evaluated* first decides whether
 *      gcc reduces any of them. With `&EscapeGrid[row][col]` first, it builds a
 *      giv `&EscapeGrid + col * 12`, spills it and reloads it once per tile,
 *      duplicating the `col * 12` giv already in s3 and costing a sixth slot.
 *      With `&EscapeGrid[row][col + 1]` first the giv never forms. Evaluating
 *      it into `c1` keeps the *store* order 0,1,2,3, which the target has too.
 *
 *   3. `c0`, holding `&EscapeGrid[row][col]`, assigned before the corner block.
 *
 *   4. `m = n`, with three of the eight v-coordinate stores indexed through `m`
 *      instead of `n`.
 *
 * 3 and 4 only work together -- either alone is 202/142, both is 12/8 -- and
 * they are the one part of this body nobody would have written. They came out
 * of the permuter, and what they do is split the address CSE for those three
 * stores, so they use the `base + n * 128` register form while the rest use
 * maspsx's `symbol + offset(index)` form. That mix is what the target has. It
 * is reproducible and measured, but it is a compiler artefact rather than
 * source: the original reached the same split some other way. Finding that way
 * is the next move, not another phrasing of the arithmetic. Making the alias
 * uniform breaks it (202/142); a `EscapeTile *` pointer in place of the integer
 * alias breaks it (210/142); the sixteen *other* aliased accesses the permuter
 * also produced are inert and were removed.
 *
 * Measured and rejected, as rows/shape against the retail overlay (this body is
 * 12/8), on top of the ninety-six phrasings recorded before this pass:
 *   - hoisting positions for the two products: both at the inner top 228/162,
 *     vy at the inner top with vx inline 238/178, vy immediately before its use
 *     228/162, either one hand-carried as an accumulator 32/22 to 44/26 -- an
 *     accumulator is a biv, gets a low pseudo and lands at 24(sp).
 *   - for the slot tie-break specifically: hand-hoisting `row * 8 + 8` and/or
 *     `row * 8` to the row top in every order (52/46 to 224/174), letting gcc
 *     hoist `(row - 16) * 2` out of the inner loop in six spellings (188/130,
 *     and it then takes a register rather than a slot), separate loop variables
 *     for the tile loop (264/204) or either earlier loop (30/26, 38/34).
 *   - corner evaluation orders 2,0,1,3 and 3,0,1,2 rebuild the giv (206/144,
 *     212/148); 1,2,0,3 avoids it but stores in the wrong order (30/26); a
 *     walked `c++` pointer, `c - 1`/`c + 40`/`c + 41` off one temp, explicit
 *     `(u8 *)EscapeGrid + row * 492 + col * 12` byte arithmetic and
 *     `EscapeGrid[row] + col` all collapse s5 and s3 into one flat giv and lose
 *     5 to 12 instructions.
 *   - declaration order and position of the locals, `register` on any subset,
 *     and reusing the distance loop's x/y: no effect, or they flatten the
 *     corner addressing (90/84).
 *
 * The permuter needs three corrections before its score means anything here,
 * all now in CLAUDE.md: `--stack-diffs` (off by default, so the stack residue
 * above is invisible and the search optimises only the corner one),
 * `D_80151909` rewritten in the scratch's target.s to the `D_801518E4 + 0x25`
 * form this C relocates against (otherwise a perfect candidate still scores 6
 * and `--stop-on-zero` never fires), and `-g -gcoff` dropped from the scratch's
 * compile.sh. Uncorrected, it reported 85 against a base of 250 for a candidate
 * measuring 28/24 -- worse than the body it started from. Corrected, it found
 * 3 and 4 in 2.8M iterations. Re-measure every output against the overlay.
 *
 * The slot tie-break has since been searched exhaustively rather than randomly,
 * with the `giv-hoist`, `addr-eval-order` and `cse-split` recipes this body
 * produced (docs/PERMUTER_MACROS.md): 8100 candidates over the hoist site of
 * each of `vy`, `vx`, `m` and `c0`, five m/n split patterns for each of the two
 * v-coordinate statements, and every declaration order. It ran to completion
 * and found nothing below the score of this body -- several distinct points tie
 * with it, none beat it. So the remaining twelve rows are not reachable through
 * any of those four axes, and re-running that space is wasted time. The next
 * hypothesis has to come from somewhere else. */
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
    s32 m;
    EscapeCell* c0;

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
            m = n;
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

            EscapeTiles[m].prim[0].v0 = EscapeTiles[n].prim[1].v0 =
                EscapeTiles[n].prim[0].v1 = EscapeTiles[m].prim[1].v1 = row * 8;
            EscapeTiles[n].prim[0].v2 = EscapeTiles[n].prim[1].v2 =
                EscapeTiles[m].prim[0].v3 = EscapeTiles[n].prim[1].v3 =
                    row * 8 + 8;

            c0 = &EscapeGrid[row][col];
            {
                EscapeCell* c1 = &EscapeGrid[row][col + 1];
                EscapeTiles[n].Corner[0] = c0;
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
