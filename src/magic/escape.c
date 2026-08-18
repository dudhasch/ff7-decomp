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

// All three are assembled from asm/ in the matching build; declared so calling
// them and taking their address works. Without a declaration gcc substitutes 0
// for the identifier and emits `move a0,zero` with no diagnostic the build
// surfaces.
void func_801B0020(void);
void func_801B009C(void);
void func_801B063C(void);
static void EscapeMainSetup(void);

void MAGIC_Escape(void) { EscapeMainSetup(); }

// Copies the drawing area into the two texture pages the grid samples from.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B0020);
#else
/* 16 rows, all scheduling: the target emits the two x stores before the w/h
 * pairs and fills the load-delay slot after `lhu DispX` with `move a2,zero`
 * (a MoveImage argument), where we fill it with the `li 0xA0` for the widths
 * and sink the x stores below them. Register allocation follows from that --
 * the target keeps g_cDb in v1 and overwrites it with DispY, we keep it in a2.
 * Every instruction is otherwise identical and the function is the right
 * length. Tried, in order: separate field reads (4 loads instead of 2); s32
 * locals for x and y (turns lhu into lh); adjacent reads without locals (still
 * 2 loads of DispX); both chain directions of `a.w = b.w = 0xA0` (the
 * left-to-right one is correct and is kept below); the embedded-assignment
 * form `right.x = (left.x = ...) + 0xA0`; and one `RECT half[2]` instead of
 * two RECTs. None moves the x stores up. */
static void EscapeCaptureScreen(void) {
    RECT left;
    RECT right;

    left.x = g_cDb->DispX;
    right.x = left.x + 0xA0;
    left.w = right.w = 0xA0;
    left.h = right.h = 0xA8;
    left.y = right.y = g_cDb->DispY;
    MoveImage(&left, 0x300, 0);
    MoveImage(&right, 0x280, 0x100);
    DrawSync(0);
}
#endif

// Lays out the 21x40 grid: first the radial distance table the ripple is
// driven from, then one tile per cell -- two prims chained into a single list,
// the texture window it samples (the captured screen is split across two
// texture pages, at u 0x300 and 0x280), the four corners it spans, and a
// random velocity and spin.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B009C);
#else
/* 52 of 361 instructions. One structural difference, and it is a cost decision
 * inside gcc's loop optimiser rather than anything about the C. The target
 * carries five values in the tile loop's stack frame (0x60):
 *
 *     16(sp) n            24(sp) row * 8      48(sp) row * 8 + 8
 *     32(sp) col * 2 - 42 40(sp) row * 2 - 32
 *
 * We carry four (frame 0x58) -- n, row * 8, row * 8 + 8, and, in place of the
 * two arithmetic ones, a giv holding `&EscapeGrid[0][col]` that we spill and
 * reload for Corner[0]. The target has no such giv: it rematerialises the bare
 * `&EscapeGrid` constant and adds its col*12 and row*492 givs to it, and
 * recomputes nothing, spending the freed slots on `col * 2 - 42` and
 * `row * 2 - 32` instead of the `addiu`/`sll` pair we emit inline. Everything
 * else -- both earlier loops, the prim setup, all four Corner stores, every
 * register -- is identical, and the function is exactly the right length.
 *
 * Tried and rejected, each measured as an instruction-sequence diff against the
 * retail overlay (rows = exact, shape = after normalising stack offsets and
 * branch targets, so a frame-size change does not cascade; base is 104/46):
 *
 *   - every spelling of the two products: `col * 2 - 42`, `(col - 21) << 1`,
 *     `2 * (col - 21)`, `col + col - 42`. All compile identically to what is
 *     below (104/46) or, with the constant split out, to 164/152.
 *   - hoisting them into locals, which is the form that makes `col * 8 - 0xA0`
 *     a perfect giv in the distance loop above: at the top of the inner loop
 *     218/164, reusing x and y 274/226, and `row * 2 - 32` in the outer loop
 *     (where it is genuinely invariant) 222/208. A user variable wins a
 *     register outright; the target wants a spill slot, which no C phrasing
 *     asks for.
 *   - seven phrasings of the Corner block, to stop the address giv forming:
 *     a `EscapeCell *c` walked by +1/+41/+42 (264/206), a row pointer
 *     (288/246), flat `&EscapeGrid[0][row * 41 + col]` (324/272), `+` instead
 *     of `&[]` (266/208), and reordering the four stores (212/156). All worse:
 *     the block as written is what the target compiles.
 *   - separate loop counters for the distance table (162/110).
 *
 * What did help, and is in the code below: writing `rand()` as the *first*
 * operand of the two Vel sums. `(rand() & 3) + (col - 21) * 2` is two rows
 * better than `(col - 21) * 2 + (rand() & 3)` and the same again for vy --
 * 108/50 to 104/46 -- because the induction value is then computed after the
 * call instead of having to survive it.
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
    func_801B0020();
    func_800BBEAC(func_801B063C);
    func_800BBEAC(EscapeFinish);
    func_800D55F4(0x20, 0x40, 0x5D);
}
