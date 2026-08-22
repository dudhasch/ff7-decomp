//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 2 of 5, split out of field.c. .rodata 0x800A0054-0x800A00E0, base 4 mod
 * 8 -> --phase 4. Every jump table here sits at an address 4 mod 8; `.align 3`
 * would push it 4 bytes forward. Do not merge this unit into a 0 mod 8
 * neighbour. */

/* Seed the background-scroll state machine from the requested scroll mode.
 * Only runs while idle (g_FieldStateData.cameraScrollState == 0). Modes: 0
 * stops and recentres; 1 arms scrolling in place; 2/3 begin a single-target
 * scroll; 4 teleports the current position to the alt source; 5-9 begin a
 * dual-target (eased) scroll. The target positions/step/fraction are what
 * FieldBGScrollUpdate consumes each frame.
 *
 * Modes 7-9 are empty arms, not part of the 5/6 dual-target block: the target's
 * table sends .rodata+0x1c/0x20/0x24 straight to the epilogue while 0x14/0x18
 * reach the work block. They still have to be written out as `case 7: case 8:
 * case 9: break;` rather than dropped, or the range test narrows from
 * `sltiu 0xa` to `sltiu 0x7` and the table loses three entries. This was
 * previously mis-read as the jump-table alignment phase; a `.rodata+0xNN` row
 * from checkfn means the table's *contents* differ, and only a `want:/got:` on
 * the table's own address means alignment. */
extern s16 D_8009A100;           // scroll enable
extern s16 D_80071E38;           // current scroll X
extern s16 D_80071E3C;           // current scroll Y
extern s16 g_CameraScrollSteps;  // total steps of the active scroll
extern s16 g_CameraScrollStep;   // current step, 0..g_CameraScrollSteps
extern s16 g_CameraScrollStartX; // where the scroll began
extern s16 g_CameraScrollStartY; // where the scroll began
extern s16 g_CameraScrollEndX;   // copied from cameraScrollTargetX
extern s16 g_CameraScrollEndY;   // copied from cameraScrollTargetY

void FieldBGScrollInit(void) {
    if (g_FieldStateData.cameraScrollState != 0) {
        return;
    }
    switch (g_FieldStateData.cameraScrollMode) {
    case 0:
        D_8009A100 = 0;
        D_80071E38 = 0;
        D_80071E3C = 0;
        g_FieldStateData.cameraScrollState = 2;
        break;
    case 1:
        D_8009A100 = 1;
        g_FieldStateData.cameraScrollState = 1;
        break;
    case 2:
    case 3:
        D_8009A100 = 1;
        g_CameraScrollStep = 0;
        g_FieldStateData.cameraScrollState = 1;
        g_CameraScrollSteps = g_FieldStateData.cameraScrollNumSteps;
        g_CameraScrollStartX = D_80071E38;
        g_CameraScrollStartY = D_80071E3C;
        break;
    case 4:
        D_8009A100 = 1;
        g_FieldStateData.cameraScrollState = 2;
        D_80071E38 = g_FieldStateData.cameraScrollTargetX;
        D_80071E3C = g_FieldStateData.cameraScrollTargetY;
        break;
    case 5:
    case 6:
        D_8009A100 = 1;
        g_CameraScrollStep = 0;
        g_FieldStateData.cameraScrollState = 1;
        g_CameraScrollSteps = g_FieldStateData.cameraScrollNumSteps;
        g_CameraScrollStartX = D_80071E38;
        g_CameraScrollStartY = D_80071E3C;
        g_CameraScrollEndX = g_FieldStateData.cameraScrollTargetX;
        g_CameraScrollEndY = g_FieldStateData.cameraScrollTargetY;
        break;
    case 7:
    case 8:
    case 9:
        break;
    }
}

/* Scroll limits at the head of the field's trigger block: the rectangle the
 * background scroll position is kept inside, and how the camera is allowed to
 * move within it. Types 1 and 2 rail the camera along the diagonal between the
 * two corners -- see FieldCalcPointOnLine. g_FieldTriggers is typed s32
 * because it is assigned as a raw word on load. */
typedef struct {
    /* 0x00 */ u8 unk00[0xC];
    /* 0x0C */ s16 minX;
    /* 0x0E */ s16 minY;
    /* 0x10 */ s16 maxX;
    /* 0x12 */ s16 maxY;
    /* 0x14 */ u8 scrollType;
} FieldScrollLimits;

#define FIELD_SCROLL_LIMITS ((FieldScrollLimits*)g_FieldTriggers)

/* Rail the background scroll position onto the diagonal of the map's scroll
 * rectangle. Scroll type 1 runs the line from the top-left corner towards
 * (maxX, maxY) offset by a screen; type 2 runs it from the bottom-left, and is
 * not the mirror image it looks like -- its denominator uses (minY - maxY) * dy
 * where type 1 uses dy * dy. Both project the current position onto the line,
 * in 8.8 fixed point, and write the foot of the perpendicular back through the
 * pointer.
 *
 * The type byte is re-read for the second test because the stores through
 * `pos` in the first arm may alias it.
 *
 * 74 rows out, from an m2c seed that did not compile at all (`void*`
 * parameters dereferenced as `->unkNN`), so this is the first number the
 * function has ever had. What the rewrite established:
 *   - arg0 is the FieldScrollLimits at the head of the trigger block, arg1 the
 *     same `s16*` scroll position FieldBGClampPos takes; 0x14 is a type byte
 *     the rest of the record did not reach.
 *   - minX, minY and maxY have to be `s32`. As `s16` locals, combine narrows
 *     the closing `+ minX` back into a fresh `lhu` of the member rather than
 *     reusing the sign-extended load the target keeps live: 88 rows against 79.
 *   - `px = pos[0] - 0xA0` has to be its own local. Written inline, fold
 *     associates the constant onto minX and the subtraction comes out as
 *     `(minX + 0xA0) - pos[0]` where the target has `pos[0] - 0xA0` first.
 *   - the target reserves a 0x10 frame with no saved registers and no calls,
 *     so there is a local here nobody can see. Reserving it is worth 5 rows.
 *
 *   - the closing constant has to be added in its own statement. Written
 *     `pos[0] = ((num * dx / den) >> 8) + 0xA0 + minX;` fold's `associate`
 *     step lifts the literal onto the *other* variable and emits
 *     `addiu a0,minX,0xa0` / `addu`, where the target adds 0xA0 to the
 *     quotient and minX after it. Splitting it -- `fx = (...) + 0xA0;` then
 *     `pos[0] = fx + minX;` -- puts the literal back where the target has it,
 *     and is worth 74/7 -> 73/3. This is now the body.
 *
 * 73 rows out. Everything left is one lever plus its fallout: gcc 2.6.3's
 * fold associates *every* `A - (B + C)` in this function, and the target has
 * none of them associated. `maxX - (minX + 0x140)` (twice) comes out as
 * `(maxX - 0x140) - minX` and `minY - (maxY - 0xF0)` as `(minY + 0xF0) - maxY`
 * -- same instruction count each time, different register lifetimes, and the
 * whole allocation cascades off them (the target copies a0 into t3 and keeps
 * the negated numerator in t2; we use t2 and a2). fold's `split_tree` strips
 * only conversions that keep the machine mode, so nothing spelled in SImode
 * blocks it. Measured, all worse or neutral against 73/3:
 *   - a named local per fold site, four sites: 75/3 (with the fx/fy split),
 *     78/7 (without). Two sites only: 75/3 for the dx pair, 73/3 for the dy
 *     pair -- i.e. exactly no effect.
 *   - one shared `off` local across all sites: 78/7, 76/7, 76/7.
 *   - flattening the sums instead (`maxX - minX - 0x140`, `minY - maxY +
 *     0xF0`): 85/5 alone, 80/1 with the fx/fy split. Note the 1: that
 *     spelling gets the *length* right and the registers wrong, so it is the
 *     better permuter seed of the two if the search is scored on insertions.
 *   - earlier rounds: px/py as `s16` (88), dx/dy inline in the dot product
 *     (91), no locals at all (85).
 * A `(s16)`/`(u16)` cast is the only construct that would defeat `split_tree`
 * and it costs a truncation pair per site, so it is not the answer either.
 * This remains a good permuter target: no calls, pure arithmetic, and the
 * instruction count is now within three. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldCalcPointOnLine);
#else
void FieldCalcPointOnLine(FieldScrollLimits* limits, s16* pos) {
    s32 minX;
    s32 minY;
    s32 maxY;
    s32 px;
    s32 py;
    s32 dx;
    s32 dy;
    s32 num;
    s32 den;
    s32 fx;
    s32 fy;
    u8 unusedLocals[0x10];

    if (limits->scrollType == 1) {
        px = pos[0] - 0xA0;
        minX = limits->minX;
        dx = limits->maxX - (minX + 0x140);
        py = pos[1] - 0x78;
        minY = limits->minY;
        dy = limits->maxY - (minY + 0xF0);
        num = -(((minX - px) * dx) + ((minY - py) * dy));
        den = ((dx * dx) + (dy * dy)) >> 8;
        fx = ((num * dx / den) >> 8) + 0xA0;
        pos[0] = fx + minX;
        fy = ((num * dy / den) >> 8) + 0x78;
        pos[1] = fy + (u16)limits->minY;
    }
    if (limits->scrollType == 2) {
        px = pos[0] - 0xA0;
        minX = limits->minX;
        dx = limits->maxX - (minX + 0x140);
        py = pos[1] + 0x78;
        maxY = limits->maxY;
        minY = limits->minY;
        dy = minY - (maxY - 0xF0);
        num = -(((minX - px) * dx) + ((maxY - py) * dy));
        den = ((dx * dx) + ((minY - maxY) * dy)) >> 8;
        fx = ((num * dx / den) >> 8) + 0xA0;
        pos[0] = fx + minX;
        fy = ((num * dy / den) >> 8) - 0x78;
        pos[1] = fy + (u16)limits->maxY;
    }
}
#endif

/* Keeps a background scroll position half a screen (0xA0 x 0x78) inside the
 * map's scroll limits. */
void FieldBGClampPos(s16* pos) {
    if (FIELD_SCROLL_LIMITS->maxX - 0xA0 < pos[0]) {
        pos[0] = FIELD_SCROLL_LIMITS->maxX - 0xA0;
    }
    if (pos[0] < FIELD_SCROLL_LIMITS->minX + 0xA0) {
        pos[0] = FIELD_SCROLL_LIMITS->minX + 0xA0;
    }
    if (FIELD_SCROLL_LIMITS->maxY - 0x78 < pos[1]) {
        pos[1] = FIELD_SCROLL_LIMITS->maxY - 0x78;
    }
    if (pos[1] < FIELD_SCROLL_LIMITS->minY + 0x78) {
        pos[1] = FIELD_SCROLL_LIMITS->minY + 0x78;
    }
}

/* Project the tracked entity's world position onto the screen, lifting it by
 * the camera's height bias. */
s32 FieldBGGetEntityScreenPos(long* screenPos) {
    SVECTOR pos;
    volatile u8* tracked;

    tracked = &g_FieldBGTrackedEntity;
    pos.vx = g_FieldEntity[*tracked].PosX >> 12;
    pos.vy = g_FieldEntity[*tracked].PosY >> 12;
    pos.vz = (g_FieldEntity[*tracked].PosZ >> 12) + g_FieldBGCameraHeightBias;
    return FieldCalcWorldToScreenPos(&pos, screenPos);
}

extern s16 D_80071E38;
extern s16 D_80071E3C;
extern s16 g_CameraScrollStep;
extern s16 g_CameraScrollStartX;
extern s16 g_CameraScrollEndX;
extern s16 g_CameraScrollStartY;
extern s16 g_CameraScrollEndY;
extern s16 g_CameraScrollSteps;

/* Per-frame background scroll: on the field's scroll state machine, drive the
 * background X/Y toward the entity's clamped screen position (linear or
 * ease-in-out depending on the mode).
 *
 * The arms that finish a scroll do not each write
 * `g_FieldStateData.cameraScrollState = 2;` -- the three `!=` arms `goto` the
 * copy inside arm 6. Written out per arm, all four tails are the same nine
 * instructions (`lh g_CameraScrollStep` / `sh D_80071E3C` / `lh
 * g_CameraScrollSteps` / branch), jump_optimize inverts them to a common
 * polarity and cross-jumps them into one, and the body comes out ten
 * instructions short. The target carries two physical copies: one inline in arm
 * 2 that arms 3 and 5 jump backwards into, and arm 6's.
 *
 * Three earlier corrections, all read off the target rather than guessed:
 * the jump table has ten entries and not six (`sltiu v0,v1,0xa` on the
 * selector with no `addiu -1` in front of it); arms 2, 3 and 5 test `!=`
 * with the increment first while arm 6 tests `==` with the state store
 * first; and there is no dead local -- `long screenPos` alone gives the
 * target's 0x40 frame. */
void FieldBGScrollUpdate(void) {
    long screenPos;

#define SCREEN_X (((s16*)&screenPos)[0])
#define SCREEN_Y (((s16*)&screenPos)[1])
    if (g_FieldStateData.cameraScrollState == 1) {
        switch (g_FieldStateData.cameraScrollMode) {
        case 1:
            FieldBGGetEntityScreenPos(&screenPos);
            FieldBGClampPos((s16*)&screenPos);
            D_80071E38 = -SCREEN_X;
            D_80071E3C = -SCREEN_Y;
            break;
        case 2:
            FieldBGGetEntityScreenPos(&screenPos);
            FieldBGClampPos((s16*)&screenPos);
            D_80071E38 = FieldCalcLinearStep(
                g_CameraScrollStartX, -SCREEN_X, g_CameraScrollSteps,
                g_CameraScrollStep);
            D_80071E3C = FieldCalcLinearStep(
                g_CameraScrollStartY, -SCREEN_Y, g_CameraScrollSteps,
                g_CameraScrollStep);
            if (g_CameraScrollSteps != g_CameraScrollStep) {
                g_CameraScrollStep = g_CameraScrollStep + 1;
            } else {
                goto scrollDone;
            }
            break;
        case 3:
            FieldBGGetEntityScreenPos(&screenPos);
            FieldBGClampPos((s16*)&screenPos);
            D_80071E38 = FieldCalcEaseInOut(
                g_CameraScrollStartX, -SCREEN_X, g_CameraScrollSteps,
                g_CameraScrollStep);
            D_80071E3C = FieldCalcEaseInOut(
                g_CameraScrollStartY, -SCREEN_Y, g_CameraScrollSteps,
                g_CameraScrollStep);
            if (g_CameraScrollSteps != g_CameraScrollStep) {
                g_CameraScrollStep = g_CameraScrollStep + 1;
            } else {
                goto scrollDone;
            }
            break;
        case 5:
            D_80071E38 = FieldCalcLinearStep(
                g_CameraScrollStartX, g_CameraScrollEndX, g_CameraScrollSteps,
                g_CameraScrollStep);
            D_80071E3C = FieldCalcLinearStep(
                g_CameraScrollStartY, g_CameraScrollEndY, g_CameraScrollSteps,
                g_CameraScrollStep);
            if (g_CameraScrollSteps != g_CameraScrollStep) {
                g_CameraScrollStep = g_CameraScrollStep + 1;
            } else {
                goto scrollDone;
            }
            break;
        case 6:
            D_80071E38 = FieldCalcEaseInOut(
                g_CameraScrollStartX, g_CameraScrollEndX, g_CameraScrollSteps,
                g_CameraScrollStep);
            D_80071E3C = FieldCalcEaseInOut(
                g_CameraScrollStartY, g_CameraScrollEndY, g_CameraScrollSteps,
                g_CameraScrollStep);
            if (g_CameraScrollSteps == g_CameraScrollStep) {
            scrollDone:
                g_FieldStateData.cameraScrollState = 2;
            } else {
                g_CameraScrollStep = g_CameraScrollStep + 1;
            }
            break;
        case 0:
        case 4:
        case 7:
        case 8:
        case 9:
            break;
        }
    }
#undef SCREEN_X
#undef SCREEN_Y
}

extern s32 FieldCalcEaseInOut(s32 from, s32 to, s32 total, s32 step);
extern s32 FieldCalcLinearStep(s32 start, s32 target, s32 duration, s32 step);
extern void FieldCalcPointOnLine(FieldScrollLimits* limits, s16* pos);
extern s16 D_80071A48;
extern s16 D_80071A4A;
extern s16 D_80071A4C;
extern s16 D_80071A4E;
extern s16 D_80071A50;
extern s16 D_80071A52;
extern /*?*/ s32 D_80074EB0;
extern /*?*/ s32 D_80074EB4;
extern /*?*/ s32 D_80074EB8;
extern /*?*/ s32 D_80074EE4;
extern /*?*/ s32 D_80074EEA;
extern /*?*/ s32 D_80074EF0;
extern s16 D_8009AC04;
extern u8 D_8009AC06;
extern u8 D_8009AC07;
extern u8 D_8009AC08;
extern s16 D_8009AC0C;
extern s16 D_8009AC0E;
extern u8 D_8009AC2E;
extern u8 D_8009AC81;
extern u8 D_8009AC8F;
extern u16 D_8009AC9A;
extern u16 D_8009AC9C;
extern u16 D_8009AC9E;
extern u16 D_8009ACA0;
extern u16 D_800E48E4;
extern u16 D_800E48E6;
extern s32 D_800E48EC;
extern DR_ENV D_80100860;
extern s16 D_80113F34;
extern s16 D_80113F36;
extern s16 D_80113F90;
extern s16 D_80113F92;
extern s16 D_80113FEC;
extern s16 D_80113FEE;
extern s16 D_80114048;
extern s16 D_8011404A;
extern s16 D_801140A4;
extern s16 D_801140A6;
extern s16 D_80114100;
extern s16 D_80114102;
extern s16 D_8011415C;
extern s16 D_8011415E;
extern s16 D_801141B8;
extern s16 D_801141BA;
extern s16 D_80114214;
extern s16 D_80114216;
extern s16 D_80114270;
extern s16 D_80114272;
extern u16 g_FieldScreenCenterX;
extern u16 g_FieldScreenCenterY;

/* The trigger block carries the background-scroll parameters at +0x18: four
 * layer extents in tiles, then two banks of four scroll offsets that wrap
 * against them (`offset %% (extent * 0x10)`). Spelled as a struct rather than
 * `*(u16*)(g_FieldTriggers + 0x20)` for the reason field.c's header records:
 * a struct load does not alias a store to a plain extern, so gcc may hoist
 * it. */
typedef struct {
    /* 0x00 */ u8 unk00[0x18];
    /* 0x18 */ u16 unk18;
    /* 0x1A */ u16 unk1A;
    /* 0x1C */ u16 unk1C;
    /* 0x1E */ u16 unk1E;
    /* 0x20 */ u16 unk20;
    /* 0x22 */ u16 unk22;
    /* 0x24 */ u16 unk24;
    /* 0x26 */ u16 unk26;
    /* 0x28 */ u16 unk28;
    /* 0x2A */ u16 unk2A;
    /* 0x2C */ u16 unk2C;
    /* 0x2E */ u16 unk2E;
} FieldBgScroll;

/* What D_80071E40 really points at: the view matrix the rest of the overlay
 * passes around as a MATRIX*, followed by the projection the GTE needs.
 * unk24 is the screen distance SetGeomScreen takes; unk20/unk22 shift the
 * screen centre. */
typedef struct {
    /* 0x00 */ MATRIX view;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ s16 unk24;
} FieldCamera;

/* 677 changed / 134 inserted at 1293 instructions against 1266, from an m2c
 * seed that did not compile. Parked with the types settled.
 *
 * Four things the seed could not know:
 *
 * 1. **`g_FieldTriggers` is an `s32` holding an address**, so m2c's
 *    `g_FieldTriggers->unk20` is not C at all. The block at +0x18 is the
 *    background-scroll parameters -- four layer extents in tiles at
 *    0x18..0x1E, then two banks of four scroll offsets at 0x20..0x2E that
 *    wrap against them, `offset %% (extent * 0x10)`. Reached through
 *    `FieldBgScroll` rather than `*(u16*)(g_FieldTriggers + 0x20)` for the
 *    reason field.c's own header records: a struct load does not alias a
 *    store to a plain extern, so gcc may hoist it.
 * 2. **`D_80071E40` is not a `MATRIX*`**, although the rest of the overlay
 *    passes it around as one. Offsets 0x20/0x22/0x24 sit past the matrix:
 *    `unk24` is the screen distance `SetGeomScreen` takes and `unk20`/`unk22`
 *    shift the screen centre. `FieldCamera` here is a local cast so the
 *    matching callers in field.c keep their `MATRIX*`.
 * 3. **The entity reads are byte offsets against `s32`/`u16` externs**, so
 *    every `*(&D_80074EB0 + modelId * 0x84)` in the seed was scaled a second
 *    time -- four times over for the `s32`s. The target references those
 *    interior labels directly (`%hi(D_80074EB0)`, and 181 of its 1266
 *    instructions are `$at` expansions), so the byte-offset form is right:
 *    `*(s32*)((u8*)&D_80074EB0 + off)`. The widths are read off the target,
 *    `lw` at 0xC/0x10/0x14 and `lhu` at 0x40/0x46/0x4C.
 * 4. `unksp1A` is the second half of the screen position
 *    `FieldCalcWorldToScreenPos` writes, so the two stack objects are an
 *    `SVECTOR` at 0x10 and a `DVECTOR` at 0x18. `DVECTOR` was missing from
 *    `include/psxsdk/libgte.h` and is added there.
 *
 * Then two of the levers this file already records, worth five instructions
 * each: `u8 unusedLocals[0x18]` for the frame (the target is -0x70 against
 * our -0x58, with the same saved-register set), and one entity-offset
 * variable computed once rather than m2c's two.
 *
 * What is left is a control-flow difference, not scheduling, and
 * `tools/insn_histogram.py` now says so in numbers rather than by inference.
 * At 811 rows and +27 instructions the whole residue is addressing:
 *
 *     addiu  90/134  -44      lui   253/223  +30
 *     lh      21/35  -14      lhu   149/137  +12
 *
 * That pair is one fact: `lui rN,%hi(sym)` plus a `%lo` embedded in the load
 * is the form gcc emits for a single reference, and `lui`+`addiu` into a
 * register that several accesses then share is the form it emits when the
 * address survives. The target holds addresses; this body rebuilds them. Per
 * symbol, `%hi` counts: `g_FieldScreenCenterX` 27 against 9,
 * `g_FieldScreenCenterY` 27 against 9, and every one of the ten `D_801139..`
 * / `D_80114...` objects 4 or 7 against 2 or 4.
 *
 * **The obvious lever hits the length and is a trap.** Reading
 * `g_FieldScreenCenterY` through a named `s16*` local -- CLAUDE.md's
 * `DebugUpdateActor` idiom -- takes the function to **exactly 1266
 * instructions**, the target's length, at 818 rows. It is still wrong: the
 * pointer collapses that symbol to **1** materialisation where the target has
 * 9, and the length only comes out right because a +18 error on X cancels a
 * -8 error on Y. Doing both symbols is -24 instructions, X alone -4. This is
 * the `FieldBackgroundInitPackets` shape exactly -- two wrong things that add
 * to a right number and then lock each other in place -- so it was measured
 * and deliberately not landed.
 *
 * What that leaves is a single question, and it is the one the note already
 * had: the target materialises *both* screen-centre symbols nine times, and
 * neither reading them directly (27) nor holding a pointer (1) produces nine.
 * Nine is a count of live ranges, which is a count of basic blocks, so the
 * addressing follows the control flow rather than the other way round. Two
 * concrete traces of that flow to reconstruct first:
 *   - the target has a `slti v0,v1,2` / `bnez` range test and an
 *     `addiu a0,sp,0x10` this body never emits (`slti` 0 against 1 in the
 *     histogram), and CLAUDE.md's rule about a folded range test in front of a
 *     `switch` deleting the switch's own bounds check is what to check.
 *   - `D_80071E38` and `D_80071E3C` are each read **once signed and seven
 *     times unsigned** in the target, against eight unsigned here. One use of
 *     each wants an `lh`, and finding which is a free extra data point about
 *     where the blocks divide.
 *
 * Two theories checked and dead, so nobody re-checks them: the ten
 * `D_801139..`/`D_80114...` objects are 0x5C apart with no gaps, which reads
 * like one array of ten -- but the target materialises each of their symbols
 * separately, so they are ten objects and indexing them would be wrong. And
 * the `lh`/`lhu` gap is not a declaration error: the two symbols above are
 * read both ways *within the target itself*.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldBGUpdateDrawenv);
#else
void FieldBGUpdateDrawenv(s32 arg0) {
    s32 temp_v1;

    u8 unusedLocals[0x18];
    SVECTOR pos;
    DVECTOR screen;
    u16 sp20;
    u16 sp28;
    u16 sp30;
    DR_ENV* var_a0;
    DR_ENV* var_a0_2;
    DR_ENV* var_a0_3;
    s16 temp_a2_2;
    s16 temp_a3;
    s16 var_v0_2;
    s16* var_a1;
    s16* var_a1_2;
    s16* var_a1_3;
    s32 temp_hi;
    s32 temp_hi_2;
    s32 temp_hi_3;
    s32 temp_hi_4;
    s32 temp_hi_5;
    s32 temp_hi_6;
    s32 temp_s3;
    s32 temp_s5;
    u8 var_v0;

    ((FieldBgScroll*)g_FieldTriggers)->unk20 =
        (u16)(((FieldBgScroll*)g_FieldTriggers)->unk20 + D_8009AC9A);
    ((FieldBgScroll*)g_FieldTriggers)->unk22 =
        (u16)(((FieldBgScroll*)g_FieldTriggers)->unk22 + D_8009AC9C);
    ((FieldBgScroll*)g_FieldTriggers)->unk20 =
        (u16)((s16)((FieldBgScroll*)g_FieldTriggers)->unk20 %
              (s32)(((FieldBgScroll*)g_FieldTriggers)->unk18 * 0x10));
    ((FieldBgScroll*)g_FieldTriggers)->unk22 =
        (u16)((s16)((FieldBgScroll*)g_FieldTriggers)->unk22 %
              (s32)(((FieldBgScroll*)g_FieldTriggers)->unk1A * 0x10));
    ((FieldBgScroll*)g_FieldTriggers)->unk24 =
        (u16)(((FieldBgScroll*)g_FieldTriggers)->unk24 + D_8009AC9E);
    ((FieldBgScroll*)g_FieldTriggers)->unk26 =
        (u16)(((FieldBgScroll*)g_FieldTriggers)->unk26 + D_8009ACA0);
    ((FieldBgScroll*)g_FieldTriggers)->unk24 =
        (u16)((s16)((FieldBgScroll*)g_FieldTriggers)->unk24 %
              (s32)(((FieldBgScroll*)g_FieldTriggers)->unk1C * 0x10));
    ((FieldBgScroll*)g_FieldTriggers)->unk26 =
        (u16)((s16)((FieldBgScroll*)g_FieldTriggers)->unk26 %
              (s32)(((FieldBgScroll*)g_FieldTriggers)->unk1E * 0x10));
    SetGeomScreen((s32)((FieldCamera*)D_80071E40)->unk24);
    if ((g_FieldMovieStreamActive == 0) || (D_8009AC2E != 0)) {
        if (D_8009A100 == 0) {
            switch (D_8009AC08) { // irregular
            case 1:
                g_FieldBGCameraHeightBias =
                    FieldCalcLinearStep(
                        D_8009AC0C, D_8009AC0E, D_8009AC06, D_8009AC07) &
                    0xFF;
                if (D_8009AC07 != D_8009AC06) {
                    var_v0 = D_8009AC07 + 1;
                block_12:
                    D_8009AC07 = var_v0;
                } else {
                block_11:
                    D_8009AC08 = 3;
                }
                break;
            case 2:
                g_FieldBGCameraHeightBias =
                    FieldCalcEaseInOut(
                        D_8009AC0C, D_8009AC0E, D_8009AC06, D_8009AC07) &
                    0xFF;
                var_v0 = D_8009AC07 + 1;
                if (D_8009AC07 == D_8009AC06) {
                    goto block_11;
                }
                goto block_12;
            }
            temp_v1 = g_PlayerModelId * 0x84;
            pos.vx = ((s32) * (s32*)((u8*)&D_80074EB0 + temp_v1) >> 0xC) +
                     *(u16*)((u8*)&D_80074EE4 + temp_v1);
            pos.vy = ((s32) * (s32*)((u8*)&D_80074EB4 + temp_v1) >> 0xC) +
                     *(u16*)((u8*)&D_80074EEA + temp_v1);
            pos.vz =
                ((s32) * (s32*)((u8*)&D_80074EB8 + temp_v1) >> 0xC) +
                *(u16*)((u8*)&D_80074EF0 + temp_v1) + ((s16)D_8009AC04 >> 2);
            FieldCalcWorldToScreenPos(&pos, &screen);
            g_FieldExitArrowX = screen.vx + g_FieldScreenCenterX;
            pos.vx = (s16)((s32) * (s32*)((u8*)&D_80074EB0 + temp_v1) >> 0xC);
            g_FieldExitArrowY = screen.vy + g_FieldScreenCenterY;
            pos.vy = (s16)((s32) * (s32*)((u8*)&D_80074EB4 + temp_v1) >> 0xC);
            pos.vz = ((s32) * (s32*)((u8*)&D_80074EB8 + temp_v1) >> 0xC) +
                     g_FieldBGCameraHeightBias;
            D_800E48EC = FieldCalcWorldToScreenPos(&pos, &screen);
            D_800E48E4 = screen.vx;
            D_800E48E6 = screen.vy;
            FieldBGClampPos((s16*)&screen);
            FieldCalcPointOnLine(g_FieldTriggers, &screen);
            temp_s5 = (s16)(((s32)((s16)screen.vx *
                                   ((FieldBgScroll*)g_FieldTriggers)->unk28) >>
                             8) +
                            ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk20
                                   << 0x10) >>
                             0x14)) %
                      (s16)((FieldBgScroll*)g_FieldTriggers)->unk18;
            temp_hi = (s16)(((s32)(screen.vy *
                                   ((FieldBgScroll*)g_FieldTriggers)->unk2A) >>
                             8) +
                            ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk22
                                   << 0x10) >>
                             0x14)) %
                      (s16)((FieldBgScroll*)g_FieldTriggers)->unk1A;
            temp_hi_2 =
                (s16)(((s32)((s16)screen.vx *
                             ((FieldBgScroll*)g_FieldTriggers)->unk2C) >>
                       8) +
                      ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk24
                             << 0x10) >>
                       0x14)) %
                (s16)((FieldBgScroll*)g_FieldTriggers)->unk1C;
            temp_hi_3 =
                (s16)(((s32)(screen.vy *
                             ((FieldBgScroll*)g_FieldTriggers)->unk2E) >>
                       8) +
                      ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk26
                             << 0x10) >>
                       0x14)) %
                (s16)((FieldBgScroll*)g_FieldTriggers)->unk1E;
            g_FieldExitArrowY = (u16)g_FieldExitArrowY - screen.vy;
            g_FieldExitArrowX = (u16)g_FieldExitArrowX - screen.vx;
            sp20 = (u16)temp_hi;
            sp28 = (u16)temp_hi_2;
            sp30 = (u16)temp_hi_3;
            if (arg0 == g_FieldRenderData) {
                D_80113F34 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - screen.vx);
                D_80113F36 =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - screen.vy);
                SetDrawEnv(arg0 + 0x41D4, (DRAWENV*)(&D_80113F34 - 8));
                D_8011415C =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - screen.vx);
                D_8011415E =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - screen.vy);
                SetDrawEnv(arg0 + 0x4294, (DRAWENV*)(&D_8011415C - 8));
                D_80114214 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - screen.vx);
                D_80114216 =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - screen.vy);
                SetDrawEnv(arg0 + 0x42D4, (DRAWENV*)(&D_80114214 - 8));
                D_80113FEC = (s8)D_8009AC81 + (g_FieldScreenCenterX - temp_s5);
                D_80113FEE = (s8)D_8009AC8F + (g_FieldScreenCenterY - temp_hi);
                SetDrawEnv(arg0 + 0x4214, (DRAWENV*)(&D_80113FEC - 8));
                var_a0 = arg0 + 0x4254;
                D_801140A4 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - temp_hi_2);
                D_801140A6 =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - temp_hi_3);
                var_a1 = &D_801140A4 - 8;
            } else {
                D_80113F90 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - screen.vx);
                D_80113F92 =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - screen.vy) + 0xE8;
                SetDrawEnv(&D_80100860, (DRAWENV*)(&D_80113F90 - 8));
                D_801141B8 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - screen.vx);
                D_801141BA =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - screen.vy) + 0xE8;
                SetDrawEnv(&D_80100860 + 0xC0, (DRAWENV*)(&D_801141B8 - 8));
                D_80114270 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - screen.vx);
                D_80114272 =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - screen.vy) + 0xE8;
                SetDrawEnv(&D_80100860 + 0x100, (DRAWENV*)(&D_80114270 - 8));
                D_80114048 = (s8)D_8009AC81 + (g_FieldScreenCenterX - temp_s5);
                D_8011404A =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - temp_hi) + 0xE8;
                SetDrawEnv(&D_80100860 + 0x40, (DRAWENV*)(&D_80114048 - 8));
                var_a0 = &D_80100860 + 0x80;
                var_a1 = &D_80114100 - 8;
                D_80114100 =
                    (s8)D_8009AC81 + (g_FieldScreenCenterX - temp_hi_2);
                D_80114102 =
                    (s8)D_8009AC8F + (g_FieldScreenCenterY - temp_hi_3) + 0xE8;
            }
            SetDrawEnv(var_a0, (DRAWENV*)var_a1);
            D_80071E38 = -(s16)screen.vx;
            D_80071A48 =
                ((screen.vx + 0x140) - g_FieldScreenCenterX) - (s8)D_8009AC81;
            D_80071A4C =
                ((temp_s5 + 0x140) - g_FieldScreenCenterX) - (s8)D_8009AC81;
            D_80071E3C = -screen.vy;
            D_80071A4E =
                ((sp20 + 0xE8) - g_FieldScreenCenterY) - (s8)D_8009AC8F;
            D_80071A4A =
                ((screen.vy + 0xE8) - g_FieldScreenCenterY) - (s8)D_8009AC8F;
            D_80071A50 =
                ((sp28 + 0x140) - g_FieldScreenCenterX) - (s8)D_8009AC81;
            D_80071A52 =
                ((sp30 + 0xE8) - g_FieldScreenCenterY) - (s8)D_8009AC8F;
            return;
        }
        temp_a3 = -D_80071E38;
        temp_a2_2 = -D_80071E3C;
        temp_s3 =
            (s16)(((s32)(temp_a3 * ((FieldBgScroll*)g_FieldTriggers)->unk28) >>
                   8) +
                  ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk20 << 0x10) >>
                   0x14)) %
            (s16)((FieldBgScroll*)g_FieldTriggers)->unk18;
        temp_hi_4 =
            (s16)(((s32)(temp_a2_2 *
                         ((FieldBgScroll*)g_FieldTriggers)->unk2A) >>
                   8) +
                  ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk22 << 0x10) >>
                   0x14)) %
            (s16)((FieldBgScroll*)g_FieldTriggers)->unk1A;
        temp_hi_5 =
            (s16)(((s32)(temp_a3 * ((FieldBgScroll*)g_FieldTriggers)->unk2C) >>
                   8) +
                  ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk24 << 0x10) >>
                   0x14)) %
            (s16)((FieldBgScroll*)g_FieldTriggers)->unk1C;
        temp_hi_6 =
            (s16)(((s32)(temp_a2_2 *
                         ((FieldBgScroll*)g_FieldTriggers)->unk2E) >>
                   8) +
                  ((s32)(((FieldBgScroll*)g_FieldTriggers)->unk26 << 0x10) >>
                   0x14)) %
            (s16)((FieldBgScroll*)g_FieldTriggers)->unk1E;
        sp20 = (u16)temp_hi_4;
        sp28 = (u16)temp_hi_5;
        sp30 = (u16)temp_hi_6;
        if (arg0 == g_FieldRenderData) {
            D_80113F34 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) +
                 D_80071E38);
            D_80113F36 = (s8)D_8009AC8F +
                         (g_FieldScreenCenterY +
                          ((FieldCamera*)D_80071E40)->unk22 + D_80071E3C);
            SetDrawEnv(arg0 + 0x41D4, (DRAWENV*)(&D_80113F34 - 8));
            D_8011415C =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) +
                 (u16)D_80071E38);
            D_8011415E = (s8)D_8009AC8F +
                         (g_FieldScreenCenterY +
                          ((FieldCamera*)D_80071E40)->unk22 + (u16)D_80071E3C);
            SetDrawEnv(arg0 + 0x4294, (DRAWENV*)(&D_8011415C - 8));
            D_80114214 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) +
                 (u16)D_80071E38);
            D_80114216 = (s8)D_8009AC8F +
                         (g_FieldScreenCenterY +
                          ((FieldCamera*)D_80071E40)->unk22 + (u16)D_80071E3C);
            SetDrawEnv(arg0 + 0x42D4, (DRAWENV*)(&D_80114214 - 8));
            D_80113FEC =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) -
                 temp_s3);
            D_80113FEE =
                (s8)D_8009AC8F +
                ((g_FieldScreenCenterY + ((FieldCamera*)D_80071E40)->unk22) -
                 temp_hi_4);
            SetDrawEnv(arg0 + 0x4214, (DRAWENV*)(&D_80113FEC - 8));
            var_a0_2 = arg0 + 0x4254;
            D_801140A4 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) -
                 temp_hi_5);
            D_801140A6 =
                (s8)D_8009AC8F +
                ((g_FieldScreenCenterY + ((FieldCamera*)D_80071E40)->unk22) -
                 temp_hi_6);
            var_a1_2 = &D_801140A4 - 8;
        } else {
            D_80113F90 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) +
                 D_80071E38);
            D_80113F92 = (s8)D_8009AC8F +
                         (g_FieldScreenCenterY +
                          ((FieldCamera*)D_80071E40)->unk22 + D_80071E3C) +
                         0xE8;
            SetDrawEnv(&D_80100860, (DRAWENV*)(&D_80113F90 - 8));
            D_801141B8 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) +
                 (u16)D_80071E38);
            D_801141BA = (s8)D_8009AC8F +
                         (g_FieldScreenCenterY +
                          ((FieldCamera*)D_80071E40)->unk22 + (u16)D_80071E3C) +
                         0xE8;
            SetDrawEnv(&D_80100860 + 0xC0, (DRAWENV*)(&D_801141B8 - 8));
            D_80114270 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) +
                 (u16)D_80071E38);
            D_80114272 = (s8)D_8009AC8F +
                         (g_FieldScreenCenterY +
                          ((FieldCamera*)D_80071E40)->unk22 + (u16)D_80071E3C) +
                         0xE8;
            SetDrawEnv(&D_80100860 + 0x100, (DRAWENV*)(&D_80114270 - 8));
            D_80114048 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) -
                 temp_s3);
            D_8011404A =
                (s8)D_8009AC8F +
                ((g_FieldScreenCenterY + ((FieldCamera*)D_80071E40)->unk22) -
                 temp_hi_4) +
                0xE8;
            SetDrawEnv(&D_80100860 + 0x40, (DRAWENV*)(&D_80114048 - 8));
            var_a0_2 = &D_80100860 + 0x80;
            var_a1_2 = &D_80114100 - 8;
            D_80114100 =
                (s8)D_8009AC81 +
                ((g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20) -
                 temp_hi_5);
            D_80114102 =
                (s8)D_8009AC8F +
                ((g_FieldScreenCenterY + ((FieldCamera*)D_80071E40)->unk22) -
                 temp_hi_6) +
                0xE8;
        }
        SetDrawEnv(var_a0_2, (DRAWENV*)var_a1_2);
        D_80071A48 =
            ((0x140 - (u16)D_80071E38) - g_FieldScreenCenterX) - (s8)D_8009AC81;
        D_80071A4A =
            ((0xE8 - (u16)D_80071E3C) - g_FieldScreenCenterY) - (s8)D_8009AC8F;
        D_80071A4C =
            ((temp_s3 + 0x140) - g_FieldScreenCenterX) - (s8)D_8009AC81;
        D_80071A4E = ((sp20 + 0xE8) - g_FieldScreenCenterY) - (s8)D_8009AC8F;
        D_80071A50 = ((sp28 + 0x140) - g_FieldScreenCenterX) - (s8)D_8009AC81;
        D_80071A52 = ((sp30 + 0xE8) - g_FieldScreenCenterY) - (s8)D_8009AC8F;
        return;
    }
    if (g_FieldCameraMatrixSel == 1) {
        var_a0_3 = arg0 + 0x41D4;
        if (arg0 == g_FieldRenderData) {
            D_80113F34 = g_FieldScreenCenterX + (u16)D_80071E38;
            D_80113F36 = g_FieldScreenCenterY + (u16)D_80071E3C;
            var_a1_3 = &D_80113F34 - 8;
        } else {
            var_a0_3 = &D_80100860;
            var_a1_3 = &D_80113F90 - 8;
            var_v0_2 = g_FieldScreenCenterY + (u16)D_80071E3C + 0xE8;
            D_80113F90 = g_FieldScreenCenterX + (u16)D_80071E38;
            goto block_29;
        }
    } else {
        var_a0_3 = arg0 + 0x41D4;
        if (arg0 == g_FieldRenderData) {
            D_80113F34 =
                g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20;
            D_80113F36 =
                g_FieldScreenCenterY + ((FieldCamera*)D_80071E40)->unk22;
            var_a1_3 = &D_80113F34 - 8;
        } else {
            var_a0_3 = &D_80100860;
            D_80113F90 =
                g_FieldScreenCenterX - ((FieldCamera*)D_80071E40)->unk20;
            var_a1_3 = &D_80113F90 - 8;
            var_v0_2 =
                g_FieldScreenCenterY + ((FieldCamera*)D_80071E40)->unk22 + 0xE8;
        block_29:
            D_80113F92 = var_v0_2;
        }
    }
    SetDrawEnv(var_a0_3, (DRAWENV*)var_a1_3);
}
#endif

/////////////////////////////////////////////////
// Begin of field_entity.c
/////////////////////////////////////////////////

/* Points at the field walk mesh: three vertices per triangle, each three s16
 * plus a pad word, so 12 shorts per triangle and 4 per vertex. */
extern s16* D_800E4274;

/* Place the player's model when a field map starts. The walk mesh triangle the
 * player stands on comes from FieldState; when no exit position was stored
 * (pcPosX is the 0x7FFF sentinel) the model is dropped on that triangle's
 * centroid, otherwise it keeps the stored X/Y and its height is solved from
 * the triangle's plane. Interaction radius and walk speed both scale with the
 * map, and every entity's queued turn is cleared.
 *
 * D_800E4274 is the walk mesh: three vertices per triangle, each vertex three
 * s16 plus a pad word, so 12 shorts per triangle and 4 per vertex. */
void FieldEntityInitPos(void) {
    s32 edgeA[3];
    s32 edgeB[3];
    s32 point[3];
    s16 moveSpeed;
    s16 i;

    if (g_FieldAnimLock == 0) {
        g_PlayerModelId = g_FieldStateData.pcModelId;
        g_FieldEntity[g_PlayerModelId].PosI = g_FieldStateData.pcWalkMeshId;
        if (g_FieldStateData.pcPosX == 0x7FFF) {
            g_FieldEntity[g_PlayerModelId].PosX =
                ((D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 0] +
                  D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 4] +
                  D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 8]) /
                 3)
                << 12;
            g_FieldEntity[g_PlayerModelId].PosY =
                ((D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 1] +
                  D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 5] +
                  D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 9]) /
                 3)
                << 12;
            g_FieldEntity[g_PlayerModelId].PosZ =
                ((D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 2] +
                  D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 6] +
                  D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 10]) /
                 3)
                << 12;
        } else {
            g_FieldEntity[g_PlayerModelId].PosX = g_FieldStateData.pcPosX << 12;
            g_FieldEntity[g_PlayerModelId].PosY = g_FieldStateData.pcPosY << 12;
            FieldEntityVectorSub(
                edgeA,
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 4],
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12]);
            FieldEntityVectorSub(
                edgeB,
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 8],
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 4]);
            point[0] = g_FieldStateData.pcPosX;
            point[1] = g_FieldStateData.pcPosY;
            g_FieldEntity[g_PlayerModelId].PosZ =
                FieldEntityCalculateZ(
                    edgeA, edgeB, point,
                    &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12])
                << 12;
        }
        g_FieldEntity[g_PlayerModelId].SolidRange =
            (g_FieldStateData.currentFieldScale * 0x11) >> 8;
        moveSpeed = g_FieldStateData.currentFieldScale * 2;
        g_FieldEntity[g_PlayerModelId].animSpeed = 0x10;
        g_FieldEntity[g_PlayerModelId].MoveSpeed = moveSpeed;
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        g_FieldEntity[i].MoveDirAdd = 0;
    }
}

void FieldEntityAddRotate(s32 arg0, s16 entityIdx) {
    if (g_FieldAnimLock == 0) {
        if (g_FieldStateData.activeKeys2 & PADR1) {
            g_FieldEntity[entityIdx].MoveDirAdd = 0xE0;
        } else if (g_FieldStateData.activeKeys2 & PADL1) {
            g_FieldEntity[entityIdx].MoveDirAdd = 0x20;
        } else {
            g_FieldEntity[entityIdx].MoveDirAdd = 0;
        }
    }
}

/* Advance one entity's animation clock. animCurrentFrame counts in 1/16ths of
 * a frame, so the comparisons scale animLastFrame by 16. The player's own
 * model loops back to the start; every other entity holds on its last frame.
 * g_FieldAnimFreeze freezes all field animation at once.
 *
 * Two things this needed. The clamp arm compares with `* 16` and stores with
 * `<< 4`: cse unifies only *identical* rtx, and MULT_EXPR and LSHIFT_EXPR
 * survive expand as different trees even though both become an `ashift`, so
 * spelling them apart gives the two shifts the original has. Written the same
 * way, gcc computes one shift, keeps it live across the branch and fills the
 * delay slot with a nop; written apart, the raw halfword stays live (`move
 * a1,v0`) and reorg pulls the second `sll` into the delay slot. And the
 * function reserves a frame it never touches -- one dead scalar is enough,
 * since MIPS rounds the frame to 8. */
void FieldEntityAnimationUpdate(s32 entityId) {
    FieldModelEntry* model;
    u8* anims;
    u8 entryIndex;
    s32 unusedLocal;

    entryIndex = g_FieldModelLoaderData[entityId].modelEntryIndex;
    if (entryIndex == 0xFF) {
        return;
    }
    model = &g_FieldModelData->modelEntries[entryIndex];
    anims = model->modelData + model->animationOffset;
    if (g_FieldAnimFreeze != 0) {
        return;
    }
    g_FieldEntity[entityId].animCurrentFrame +=
        g_FieldEntity[entityId].animSpeed;
    if (entityId == g_PlayerModelId && g_FieldAnimLock == 0) {
        g_FieldEntity[entityId].animLastFrame =
            *(u16*)&anims[g_FieldEntity[entityId].activeAnimId * 16] - 1;
        if (g_FieldEntity[entityId].animLastFrame * 16 <
            g_FieldEntity[entityId].animCurrentFrame) {
            g_FieldEntity[entityId].animCurrentFrame = 0;
        }
    } else if (g_FieldEntity[entityId].animLastFrame * 16 <
               g_FieldEntity[entityId].animCurrentFrame) {
        g_FieldEntity[entityId].animCurrentFrame =
            g_FieldEntity[entityId].animLastFrame << 4;
    }
}

extern void FieldBattleCheck(void);
extern s32 FieldEntityMove(s16 entityId);
extern /*?*/ s32 D_80074EBC;
extern /*?*/ s32 D_80074EC0;
extern /*?*/ s32 D_80074EC4;
extern /*?*/ s32 D_80074ED0;
extern /*?*/ s32 D_80074ED4;
extern /*?*/ s32 D_80074ED6;
extern /*?*/ s32 D_80074ED9;
extern /*?*/ s32 D_80074EDA;
extern /*?*/ s32 D_80074EDB;
extern /*?*/ s32 D_80074EDC;
extern /*?*/ s32 D_80074EDD;
extern /*?*/ s32 D_80074EDE;
extern /*?*/ s32 D_80074EDF;
extern /*?*/ s32 D_80074EE0;
extern /*?*/ s32 D_80074EE2;
extern /*?*/ s32 D_80074EE6;
extern /*?*/ s32 D_80074EE8;
extern /*?*/ s32 D_80074EEC;
extern /*?*/ s32 D_80074EEE;
extern /*?*/ s32 D_80074EF2;
extern /*?*/ s32 D_80074EF4;
extern /*?*/ s32 D_80074EF6;
extern /*?*/ s32 D_80074EF8;
extern /*?*/ s32 D_80074EFA;
extern /*?*/ s32 D_80074F00;
extern /*?*/ s32 D_80074F01;
extern /*?*/ s32 D_80074F02;
extern /*?*/ s32 D_80074F04;
extern /*?*/ s32 D_80074F06;
extern /*?*/ s32 D_80074F08;
extern /*?*/ s32 D_80074F0C;
extern /*?*/ s32 D_80074F0E;
extern /*?*/ s32 D_80074F14;
extern /*?*/ s32 D_80074F16;
extern /*?*/ s32 D_80074F18;
extern /*?*/ s32 D_80074F1C;
extern /*?*/ s32 D_80074F20;
extern /*?*/ s32 D_80074F24;
extern FieldLine D_8007E7AC;

/* Advance every field entity one frame: turn, offset, walk, jump and the
 * player's own movement, in eight passes over g_FieldEntity.
 *
 * 572 rows / 64 insertions at 1775 instructions against 1855, from an m2c
 * seed that resolved no entity member at all. The comparable number is the
 * seed's 879 rows at exactly the same 1775 instructions.
 *
 * What the rewrite established, all of it from the repository rather than
 * from the diff:
 *
 *   - Every `*(&D_80074Exx + temp)` in the seed is `g_FieldEntity[i].<member>`.
 *     splat names the interior labels after their addresses and checkfn
 *     discounts the difference as a symbol alias -- 443 of them here, which is
 *     what says the addressing is the target's.
 *   - m2c's `temp_s1` is `&g_FieldStateData.characterLock`: the target's
 *     `addiu $s1, $v1, 0xA` with $v1 = &D_8009AC1C proves it, and every
 *     displacement off it is a FieldState member sharing the one symbol_ref.
 *   - FieldEntityMove takes one argument; its .s prologue reads only $a0.
 *   - `MoveB` is `s32`, not `s16` plus two pad bytes: the store at 0x2C is
 *     `sw`. It had no user outside this function and the pad had none at all,
 *     so include/game.h now says so.
 *   - **The prototypes m2c invented for FieldCalcLinearStep and
 *     FieldCalcEaseInOut narrow every argument.** It wrote `(s16, s16, u8, u8)`
 *     from what the callers pass; the definitions in src/field/field.c take
 *     four `s32`. force_to_mode pushed the narrowing into the loads, so `lbu`
 *     was 28 over and half the MoveStart/MoveEnd words read `lh`. Correcting
 *     the two declarations was worth 36 rows and took the per-member width
 *     audit from 29 differing (offset, opcode) pairs to 11. Deleting the casts
 *     m2c also writes at the call sites is exactly inert.
 *   - The three anim members are read with a `(u16)` cast at the use site, not
 *     retyped -- they have 19 to 45 users each, and this is the spelling the
 *     matching FieldUpdateAnimationState and its five siblings in field4.c
 *     use. animLastFrame is the exception that proves it is a use-site fact:
 *     `lhu` in the frame-decrement group and plain `lh` in the increment one.
 *   - The animation table is read as u16 with the `- 1` in an s32 local
 *     assigned immediately before the store, which is the same six sites'
 *     spelling and the force_to_mode trap CLAUDE.md names them for.
 *
 * Three structural findings:
 *
 *   - The eight loops are SEQUENTIAL, not nested. m2c renders them nested
 *     because all eight `blez` guards branch to the epilogue: on a guard's
 *     false edge no store has happened, so cse -- which follows both edges out
 *     of a conditional branch and records the outcome -- folds every later
 *     guard on that path and threads it straight out. Nested: 876 rows at 1771
 *     instructions; flat: 707 at 1777.
 *   - They share ONE counter, `$s2` in the target: the counter-merging idiom
 *     recorded for FieldInitDefaultValues.
 *   - `x != 1 / (s32)x >= 2 / x != 2` is expand_end_case's compare chain for a
 *     two-case switch, and the two arms share their tail through `goto` rather
 *     than duplicating it. .L800A67F4 and .L800A680C each have two
 *     predecessors, which is exactly why the TurnStep store falls back to the
 *     hoisted base register (`addu $v0,$s0,$s1` / `sb $v1,0x3A($v0)`) where
 *     every other access in the loop uses the $at macro. Duplicated: 1765
 *     instructions; shared: 1794.
 *
 * Passes 7 and 8 are near-clones and were 30 instructions short each. Both
 * defects are the same two levers, and both are about what gcc is allowed to
 * merge or hoist:
 *
 *   - **The carried mask has to be assigned at the END of each ActionArg
 *     arm.** The two arms differ only in which of arg0's bits drives the step
 *     and which is carried to the join (0x1000/0x4000 in pass 7,
 *     0x8000/0x2000 in pass 8), so if that assignment is anywhere but the
 *     arm's tail the two arms compile to identical tails and cross-jumping
 *     merges one of them away -- taking a whole frame-decrement group with it:
 *     5 $at triples, 3 lhu, 2 sh, 1 subu and 1 bgez per pass. m2c writes the
 *     assignment at the top of the arm and again on every interior path,
 *     because reorg replicates it into each delay slot on the way to the join,
 *     and all of those spellings measure the same -80 instructions. One
 *     assignment after the arm's inner `if` is -42 with pass 7 alone and -3
 *     with both. Asymmetric placements (top in one arm, end in the other) are
 *     a row worse either way round.
 *   - **Each pass hoists two movables into its own preheader**: `ori s6,0x2`
 *     for the three `ActionState = 2` stores, then
 *     `lui/addiu s5,%hi/%lo(g_FieldEntity)`, which serves exactly three stores
 *     -- the two arms' MoveStep and block_134's -- while every other access in
 *     the loop goes through the $at macro. That is the "same packet reached
 *     two ways in one loop" idiom, and the order is not free: move_movables
 *     emits hoists in insn order, so `actionEnd = 2;` has to be written before
 *     `ents = g_FieldEntity;` at the top of the loop body. Together they are
 *     worth 68 rows (538 to 470); the pointer alone is 506 and the constant
 *     alone 534.
 *
 * The pointer has to be assigned per pass, inside the loop body. As one
 * function-scope `FieldEntity* ents = g_FieldEntity;` -- which is what closed
 * the last two rows of passes 2 and 3 when those were the ones being worked --
 * the base is materialised at function entry instead, where the target has
 * nothing, and the whole function is a row worse and three instructions short.
 * The target materialises it four times: $s1 in pass 2's preheader, $a0 once
 * inside pass 4, and $s5 in each of passes 7 and 8.
 *
 * Measured and rejected:
 *   - `actionEnd = 2;` on its own, before the arms were split: 613 rows at
 *     1788. The same lever is worth 32 rows once they are.
 *   - Routing the shared-block stores through a `FieldEntity*` in passes 7
 *     and 8 while they were still 30 short: 1788.
 *   - Snapshotting the step into a local before the call: the target loads
 *     0x3A twice, once for the call and once for the compare.
 *
 * Pass 4 was 16 instructions short, and all of it was one statement. The
 * four arms that pick the walk/run speed each compute
 * `g_PlayerModelId * 0x84` in the target and share only the
 * `lui at / addiu at / addu at,at,v1 / sh v0,0(at)` tail -- which is
 * cross-jumping merging four identical stores, not a store written once after
 * the chain. m2c renders the merged tail as a store outside the chain and the
 * per-arm index as a dead local, and written that way gcc computes the index
 * once and the four `lh` of g_PlayerModelId collapse to one. Writing
 * `g_FieldEntity[g_PlayerModelId].MoveSpeed = ... * N;` inside each arm is
 * worth 14 instructions and 25 rows. Moving `var_v0_11 = 0xE0` inside the
 * `arg0 & 0x2000` test that follows -- where the target has it, in the branch
 * delay slot -- is three rows more. Spelling the `temp_s3` tests as explicit
 * `goto`s to fix their branch polarity is exactly inert.
 *
 * Pass 6 wanted one type change: `temp_a1_3`, which holds MoveSteps for the
 * jump-arc divisor, must be `s32`. As `s16` the member is read `lhu` and
 * sign-extended by hand where the target reads `lh` -- ten rows. `temp_a0_7`
 * is inert either way and `temp_v1_4` is a row worse as `s32`.
 *
 * Where the remaining instructions are, per pass (want/got):
 *     1: 44/44    2: 110/119  3: 209/215  4: 224/221
 *     5: 87/87    6: 293/286  7: 433/440  8: 441/450
 * Passes 1 and 5 are exact.
 *
 * The leads, in order of size:
 *   - Passes 4 and 6 are three and seven instructions short, and both are the
 *     same shape: gcc folding a computation the target repeats.
 *
 *     Pass 4 is two `andi` short, and the diagnosis is exact: we hoist
 *     `arg0 & 0x2000` into $s3 in the loop preheader, three lines after the
 *     `andi s3,s4,0x8000` both builds hoist, and the target computes it three
 *     times -- once in each of the direction arms. The three tests are
 *     identical invariant expressions, so `combine_movables` links them and
 *     `move_movables` lifts one copy; the target's does not, which puts its
 *     savings at or below the threshold that lets a *conditional* movable
 *     move at all. Measured and inert: `var_v0_11` as `u8` and as `s32` (the
 *     `li -32` / `li -96` we emit where the target has `ori 0xE0` / `ori 0xA0`
 *     is the QImode narrowing of the constant against the `u8 MoveDir` store,
 *     and the local's type does not reach it).
 *
 *     Pass 6 is two `lh` short, and they are the MoveStep reloads: the target
 *     stores `MoveStep = step + 1` and immediately reloads 0x32 into $a3 for
 *     the call's fourth argument, then reloads it again for the second call,
 *     where cse hands us the stored register and narrows it with sll/sra.
 *     Measured and inert there: the `s16` local assigned inside the `else`
 *     arm rather than before the `if`, which is what the target's
 *     `move a3,v1` in the branch delay slot looks like it should need.
 *   - Passes 2 (-9) and 3 (-6) are over, and both are the index again: the
 *     target computes `sra v0,v0,16 / sll / addu / sll s0,v1,2` once at the
 *     loop top and addresses the shared switch tails as `addu at,at,s0`,
 *     where we recompute it in each tail. Unlike passes 7 and 8 neither pass
 *     re-extends the counter anywhere, so here the single `s32 idx` local
 *     genuinely is the right structure -- with it, **pass 3 lands on exactly
 *     209 instructions** and pass 2 on 115 of 110. It costs 92 rows, and that
 *     is the whole difficulty: `idx` in pass 3 alone is 498 rows against 406,
 *     in pass 2 alone 493, in both 502, and the declaration's position among
 *     the locals is inert (measured last as well as first), so the cost is
 *     the extra pseudo competing across the function rather than a spill-slot
 *     shift. Something else has to absorb that before the index local can be
 *     kept.
 *
 *     Also measured and exactly inert on these two passes: a `u8` local for
 *     the switch selector and an `s32` one -- CLAUDE.md's prescription for
 *     the missing `slti` range check, which is one of pass 2's rows -- and
 *     the passes-7/8 hoist recipe, `typeDone = 3; ents = g_FieldEntity;` at
 *     the top of pass 2's loop body with the turnDone store through the
 *     constant and the turnStep store through the pointer. The target's pass
 *     2 does hoist both (`ori s3,zero,0x3` then `lui/addiu s1`) and its
 *     turnStep store is `addu v0,s0,s1 / sb v1,0x3A(v0)`, so the shape is
 *     right and gcc simply does not hoist here: it costs 19 rows and moves
 *     the pass's instruction count not at all. Pass 3's preheader hoists
 *     nothing and both its shared blocks use the $at macro, so it does not
 *     want the recipe in the first place.
 *   - Passes 7 and 8 recompute `i * 0x84` twice more than the target does,
 *     near the end of the loop body, and their whole residue is that: pass 7
 *     is `sll` +4, `sra` +2, `addu` +1 and pass 8 one more of each.
 *
 *     **A single `s32` index local is not the answer, and the target says so
 *     outright.** `idx = i;` at the top of the loop body with every
 *     `g_FieldEntity[i]` rewritten to `g_FieldEntity[idx]` takes passes 7 and
 *     8 from +7/+9 to -2/-4 and the whole function to exactly 1855
 *     instructions -- and costs 13 rows. Rewriting *every* use of `i` in the
 *     two loops takes them to +3/+1, the lowest per-pass error measured, and
 *     costs 23. Both are wrong: at want@225 the target re-extends the counter
 *     inside the second arm (`sll v1,s2,16` / `sra v1,v1,16`), and with one
 *     `s32` index that pair cannot exist -- `sra` goes from 14 (two too many)
 *     to 8 (four too few) and never through 12. The target holds the scaled
 *     index in two registers at once, $s0 for most of the body and $a0 for
 *     the frame-advance group, and re-derives it rather than keeping one.
 *
 *     The scope of the local is not decomposable either: `idx` on the two
 *     array subscripts and `idx` on the array subscripts plus the
 *     g_PlayerModelId compares measure identically (457 rows), and so do the
 *     write-only `temp_s0_8 = idx * 0x84` variant and the plain one; adding
 *     the call arguments on top of the subscripts is 508. That is the
 *     "a scaled-index local can be right at some of its use sites and wrong
 *     at the rest" case, and CLAUDE.md's advice for it is decomp-permuter's
 *     `perm_temp_for_expr`, once the function is close enough for the search
 *     to mean anything.
 *   - The write-only index locals are not the recomputation. Dropping the
 *     two in passes 7 and 8 (`temp_s0_9`, `temp_s0_11`) is exactly inert, so
 *     gcc does delete them; dropping all eleven costs 20 rows and an
 *     instruction, and that cost is entirely in passes 4 and 6, which have
 *     not been read yet. Leave them until those are.
 *   - A second index local for the frame-advance group alone -- `adv = i;`
 *     after the block_134/block_175 label, with that group's four accesses
 *     indexed by it, which is where the target reaches through $a0 -- is also
 *     exactly inert: cse propagates the copy straight back to `i`. The `idx`
 *     variants above are levers only because they cover the whole loop body,
 *     not because an s32 copy is one.
 *   - After SquareRoot0 the target copies the result with `move v1,v0` and
 *     shifts out of $v1; ours coalesces the two and shifts in place, leaving
 *     the branch delay slot empty. One row per pass.

 * **Do not convert the entity reads to the byte-offset form here.** It was
 * tried and reverted, and the reason it looked right is worth more than the
 * attempt: `insn_histogram.py` reported `%hi(g_FieldEntity)` 243 times against
 * the target's 4, which reads as the interior-label fault the debug functions
 * and `FieldEntityMove` really do have. It was a tool bug. MIPS uses REL
 * relocations, so objdump prints only the symbol and the addend lives split
 * across the `lui`/`lo` immediate pair; without reassembling it, every
 * `g_FieldEntity[i].member` looked like a reference to the base symbol rather
 * than to `g_FieldEntity + 0x32`, which is the same byte as `D_80074ED6`.
 *
 * With the addend recovered, this body's addressing was already right: three
 * small count differences against the target, at 500 rows and +21
 * instructions. The conversion -- `*(s16*)((u8*)&D_80074ED6 + off)` with an
 * `s16 off` assigned at the top of each of the eight loops -- reaches -9
 * instructions, and that is the only thing it improves: rows go 500 to 781 and
 * the `%hi` differences go from three to six. Two of the three measures got
 * worse, so it is out.
 *
 * What survives is the list of count differences, which is where the real
 * faults are and which the corrected tool now states cleanly:
 *   - `D_80074ED6` (+0x32, `MoveStep`) 20 against 22, and `D_80074EDE`
 *     (+0x3A, `TurnStep`) 5 against 4.
 *   - `g_FieldEntity` itself 3 against 4.
 * Those are single-figure differences in how often a member is re-read, not an
 * addressing form, and they are what to chase next.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityMovementUpdate);
#else
void FieldEntityMovementUpdate(s32 arg0) {
    s16 i;
    s16 actionEnd;
    FieldEntity* ents;
    s32 next;
    s32 lastFrame;
    VECTOR sp10;
    VECTOR sp20;
    VECTOR sp30;
    FieldModelEntry* temp_v0_13;
    FieldModelEntry* temp_v0_5;
    FieldModelEntry* temp_v1;
    s16 temp_a0_7;
    s32 temp_a1_3;
    s16 temp_v0_10;
    s32 temp_v0_11;
    s16 temp_v0_12;
    s16 temp_v0_15;
    s16 temp_v0_16;
    s16 temp_v0_17;
    s16 temp_v0_18;
    s16 temp_v0_19;
    s16 temp_v0_8;
    s32 temp_v0_9;
    s16 temp_v1_10;
    s16 temp_v1_4;
    s16 temp_v1_6;
    s16 temp_v1_7;
    s16 temp_v1_9;
    s16 var_v0_10;
    s16 var_v1_5;
    s16 var_v1_6;
    s32 temp_a0_6;
    s32 temp_a1_4;
    s32 temp_a1_5;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_s0_10;
    s32 temp_s0_11;
    s32 temp_s0_3;
    s32 temp_s0_4;
    s32 temp_s0_6;
    s32 temp_s0_8;
    s32 temp_s0_9;
    s32 temp_s3;
    s32 temp_v0_14;
    s32 temp_v0_4;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v1_5;
    s32 temp_v1_8;
    s32 var_a0_3;
    s32 var_a0_4;
    s32 var_a0_5;
    s32 var_a0_6;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_v0_15;
    s32 var_v0_16;
    s32 var_v0_19;
    s32 var_v0_20;
    s32 var_v0_24;
    s32 var_v0_25;
    s32 var_v0_26;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s8 var_v0_11;
    u16 temp_a0_4;
    u16 temp_a0_5;
    u8 temp_a0_8;
    u8 temp_a0_9;
    u8 temp_a1_2;
    u8 temp_v0_2;
    u8* temp_s3_2;
    u8* temp_s3_3;

    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        temp_v0_2 = g_FieldModelLoaderData[i].modelEntryIndex;
        if (temp_v0_2 != 0xFF) {
            temp_v1 = &g_FieldModelData->modelEntries[temp_v0_2];
            if (g_FieldEntity[i].visible == 1) {
                temp_v1->flags = 1;
            } else {
                temp_v1->flags = 0;
            }
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        switch (g_FieldEntity[i].TurnType) {
        case 1:
            g_FieldEntity[i].Dir = FieldCalcLinearStep(
                g_FieldEntity[i].TurnStart, g_FieldEntity[i].TurnEnd,
                g_FieldEntity[i].TurnSteps, g_FieldEntity[i].TurnStep);
            next = g_FieldEntity[i].TurnStep + 1;
            if (g_FieldEntity[i].TurnStep == g_FieldEntity[i].TurnSteps) {
                goto turnDone;
            }
            goto turnStep;
        case 2:
            g_FieldEntity[i].Dir = FieldCalcEaseInOut(
                g_FieldEntity[i].TurnStart, g_FieldEntity[i].TurnEnd,
                g_FieldEntity[i].TurnSteps, g_FieldEntity[i].TurnStep);
            next = g_FieldEntity[i].TurnStep + 1;
            if (g_FieldEntity[i].TurnStep != g_FieldEntity[i].TurnSteps) {
                goto turnStep;
            }
        turnDone:
            g_FieldEntity[i].TurnType = 3;
            break;
        turnStep:
            g_FieldEntity[i].TurnStep = next;
            break;
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        switch (g_FieldEntity[i].OfsType) {
        case 1:
            g_FieldEntity[i].OffsetX = FieldCalcLinearStep(
                g_FieldEntity[i].OffsetStartX, g_FieldEntity[i].OffsetEndX,
                g_FieldEntity[i].OffsetSteps, g_FieldEntity[i].OffsetStep);
            g_FieldEntity[i].OffsetY = FieldCalcLinearStep(
                g_FieldEntity[i].OffsetStartY, g_FieldEntity[i].OffsetEndY,
                g_FieldEntity[i].OffsetSteps, g_FieldEntity[i].OffsetStep);
            next = g_FieldEntity[i].OffsetStep + 1;
            g_FieldEntity[i].OffsetZ = FieldCalcLinearStep(
                g_FieldEntity[i].OffsetStartZ, g_FieldEntity[i].OffsetEndZ,
                g_FieldEntity[i].OffsetSteps, g_FieldEntity[i].OffsetStep);
            if (g_FieldEntity[i].OffsetStep == g_FieldEntity[i].OffsetSteps) {
                goto ofsDone;
            }
            goto ofsStep;
        case 2:
            g_FieldEntity[i].OffsetX = FieldCalcEaseInOut(
                g_FieldEntity[i].OffsetStartX, g_FieldEntity[i].OffsetEndX,
                g_FieldEntity[i].OffsetSteps, g_FieldEntity[i].OffsetStep);
            g_FieldEntity[i].OffsetY = FieldCalcEaseInOut(
                g_FieldEntity[i].OffsetStartY, g_FieldEntity[i].OffsetEndY,
                g_FieldEntity[i].OffsetSteps, g_FieldEntity[i].OffsetStep);
            next = g_FieldEntity[i].OffsetStep + 1;
            g_FieldEntity[i].OffsetZ = FieldCalcEaseInOut(
                g_FieldEntity[i].OffsetStartZ, g_FieldEntity[i].OffsetEndZ,
                g_FieldEntity[i].OffsetSteps, g_FieldEntity[i].OffsetStep);
            if (g_FieldEntity[i].OffsetStep != g_FieldEntity[i].OffsetSteps) {
                goto ofsStep;
            }
        ofsDone:
            g_FieldEntity[i].OfsType = 3;
            goto ofsEnd;
        ofsStep:
            g_FieldEntity[i].OffsetStep = next;
        ofsEnd:
            if (i == g_PlayerModelId) {
                FieldEntityLineClear(&D_8007E7AC);
            }
            break;
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        temp_s3 = arg0 & 0x8000;
        if (g_FieldEntity[i].scriptedMoveMode == 0) {
            if (i == g_PlayerModelId) {
                i = i;
                if (g_FieldStateData.characterLock != 1) {
                    FieldEntityAddRotate(arg0, i);
                    g_FieldEntity[g_PlayerModelId].activeAnimId =
                        g_FieldStateData.idleAnimId;
                    if (arg0 & 0x40) {
                        if (g_FieldStateData.backgroundMovieEnabled == 0) {
                            g_FieldEntity[g_PlayerModelId].MoveSpeed =
                                g_FieldStateData.currentFieldScale * 8;
                        } else {
                            g_FieldEntity[g_PlayerModelId].MoveSpeed =
                                g_FieldStateData.currentFieldScale * 0xC;
                        }
                    } else if (g_FieldStateData.backgroundMovieEnabled == 0) {
                        g_FieldEntity[g_PlayerModelId].MoveSpeed =
                            g_FieldStateData.currentFieldScale * 2;
                    } else {
                        g_FieldEntity[g_PlayerModelId].MoveSpeed =
                            g_FieldStateData.currentFieldScale * 3;
                    }
                    if (arg0 & 0xF000) {
                        if (arg0 & 0x1000) {
                            var_v1_3 = i * 0x84;
                            g_FieldEntity[i].MoveDir = 0;
                            if (temp_s3 != 0) {
                                g_FieldEntity[i].MoveDir = 0x20;
                            }
                            if (arg0 & 0x2000) {
                                var_v0_11 = 0xE0;
                                goto block_61;
                            }
                        } else if (arg0 & 0x4000) {
                            var_v1_3 = i * 0x84;
                            g_FieldEntity[i].MoveDir = 0x80;
                            if (temp_s3 != 0) {
                                g_FieldEntity[i].MoveDir = 0x60;
                            }
                            if (arg0 & 0x2000) {
                                var_v0_11 = 0xA0;
                                goto block_61;
                            }
                        } else {
                            if (arg0 & 0x2000) {
                                g_FieldEntity[i].MoveDir = 0xC0;
                            }
                            if (temp_s3 != 0) {
                                var_v1_3 = i * 0x84;
                                var_v0_11 = 0x40;
                            block_61:
                                g_FieldEntity[i].MoveDir = var_v0_11;
                            }
                        }
                        temp_s0_3 = i * 0x84;
                        temp_a1_2 = g_FieldEntity[i].MoveDirAdd;
                        g_FieldEntity[i].MoveDir =
                            g_FieldEntity[i].MoveDir +
                            (*(u8*)(g_FieldTriggers + 0x9) + temp_a1_2);
                        temp_a0_6 = FieldEntityMove(i);
                        if (g_FieldEntity[i].DirLock == 0) {
                            g_FieldEntity[i].Dir = g_FieldEntity[i].MoveDir;
                        }
                        if (g_FieldStateData.eventCmd != 1) {
                            if (temp_a0_6 == 1) {
                                FieldBattleCheck();
                                goto block_67;
                            }
                        } else {
                            goto block_67;
                        }
                    } else {
                    block_67:
                    }
                    goto block_68;
                }
            } else {
            block_68:
            }
            FieldEntityAnimationUpdate(i);
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        temp_s0_4 = i * 0x84;
        if (g_FieldEntity[i].scriptedMoveMode == 1) {
            if (g_FieldAnimFreeze != 1) {
                g_FieldEntity[i].MoveDirAdd = 0;
                if (FieldEntityAutoMove(
                        &g_FieldEntity[i], g_FieldEntity[i].ActionArg) == 0) {
                    g_FieldEntity[i].ActionState = 2;
                } else {
                    g_FieldEntity[i].ActionState = 1;
                    FieldEntityMove(i);
                    if (g_FieldEntity[i].DirLock == 0) {
                        g_FieldEntity[i].Dir = g_FieldEntity[i].MoveDir;
                    }
                }
                FieldEntityAnimationUpdate(i);
                if (i == g_PlayerModelId) {
                    FieldEntityLineClear(&D_8007E7AC);
                }
            }
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        temp_s0_6 = i * 0x84;
        if (g_FieldEntity[i].scriptedMoveMode == 3) {
            if (g_FieldEntity[i].ActionState == 0) {
                g_FieldEntity[i].MoveDirAdd = 0;
                temp_a2 = (u16)g_FieldEntity[i].MoveEndI * 0x18;
                g_FieldEntity[i].MoveStartX = g_FieldEntity[i].PosX;
                g_FieldEntity[i].MoveStartY = g_FieldEntity[i].PosY;
                g_FieldEntity[i].MoveStartZ = g_FieldEntity[i].PosZ;
                FieldEntityVectorSub(
                    &sp10, temp_a2 + 8 + D_800E4274, temp_a2 + D_800E4274);
                temp_a2_2 = (u16)g_FieldEntity[i].MoveEndI * 0x18;
                FieldEntityVectorSub(&sp20, temp_a2_2 + 0x10 + D_800E4274,
                                     temp_a2_2 + 8 + D_800E4274);
                var_v0_15 = g_FieldEntity[i].MoveEndX;
                if (var_v0_15 < 0) {
                    var_v0_15 += 0xFFF;
                }
                sp30.vx = var_v0_15 >> 0xC;
                var_v0_16 = g_FieldEntity[i].MoveEndY;
                if (var_v0_16 < 0) {
                    var_v0_16 += 0xFFF;
                }
                sp30.vy = var_v0_16 >> 0xC;
                temp_v0_4 =
                    FieldEntityCalculateZ(
                        &sp10, &sp20, &sp30,
                        ((u16)g_FieldEntity[i].MoveEndI * 0x18) + D_800E4274)
                    << 0xC;
                temp_a1_3 = g_FieldEntity[i].MoveSteps;
                g_FieldEntity[i].MoveEndZ = temp_v0_4;
                g_FieldEntity[i].MoveStep = 0;
                g_FieldEntity[i].ActionState = 1;
                g_FieldEntity[i].MoveB =
                    ((s32)(temp_v0_4 - g_FieldEntity[i].MoveStartZ) /
                     temp_a1_3) -
                    ((s32) - (temp_a1_3 * 0x3E80) / 2);
            } else {
                temp_v1_4 = g_FieldEntity[i].MoveStep;
                if (g_FieldEntity[i].MoveSteps == temp_v1_4) {
                    g_FieldEntity[i].ActionState = 2;
                    g_FieldEntity[i].PosI = (u16)g_FieldEntity[i].MoveEndI;
                } else {
                    g_FieldEntity[i].MoveStep = temp_v1_4 + 1;
                    g_FieldEntity[i].PosX = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartX, g_FieldEntity[i].MoveEndX,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                    temp_a0_7 = g_FieldEntity[i].MoveStep;
                    g_FieldEntity[i].PosY = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartY, g_FieldEntity[i].MoveEndY,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                    g_FieldEntity[i].PosZ =
                        (temp_a0_7 * g_FieldEntity[i].MoveB) +
                        ((s32)(temp_a0_7 * -(temp_a0_7 * 0x3E80)) / 2) +
                        g_FieldEntity[i].MoveStartZ;
                }
            }
            FieldEntityAnimationUpdate(i);
            if (i == g_PlayerModelId) {
                FieldEntityLineClear(&D_8007E7AC);
            }
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        actionEnd = 2;
        ents = g_FieldEntity;
        temp_s0_8 = i * 0x84;
        if (g_FieldEntity[i].scriptedMoveMode == 4) {
            temp_a0_8 = g_FieldModelLoaderData[i].modelEntryIndex;
            if (temp_a0_8 != 0xFF) {
                temp_v0_5 = &g_FieldModelData->modelEntries[temp_a0_8];
                temp_s3_2 = &temp_v0_5->modelData[temp_v0_5->animationOffset];
                if (g_FieldEntity[i].ActionState == 0) {
                    g_FieldEntity[i].MoveStartX = g_FieldEntity[i].PosX;
                    g_FieldEntity[i].MoveDirAdd = 0;
                    g_FieldEntity[i].MoveStartY = g_FieldEntity[i].PosY;
                    g_FieldEntity[i].MoveStartZ = g_FieldEntity[i].PosZ;
                    var_a1 =
                        g_FieldEntity[i].MoveEndX - g_FieldEntity[i].MoveStartX;
                    if (var_a1 < 0) {
                        var_a1 += 0xFFF;
                    }
                    temp_a1_4 = var_a1 >> 0xC;
                    sp10.vx = temp_a1_4;
                    temp_v1_5 =
                        g_FieldEntity[i].MoveEndY - g_FieldEntity[i].MoveStartY;
                    var_a0_3 = temp_v1_5 >> 0xC;
                    if (temp_v1_5 < 0) {
                        var_a0_3 = (s32)(temp_v1_5 + 0xFFF) >> 0xC;
                    }
                    sp10.vy = var_a0_3;
                    var_v0_19 =
                        g_FieldEntity[i].MoveEndZ - g_FieldEntity[i].MoveStartZ;
                    if (var_v0_19 < 0) {
                        var_v0_19 += 0xFFF;
                    }
                    temp_v0_6 = var_v0_19 >> 0xC;
                    sp10.vz = temp_v0_6;
                    temp_v0_7 = SquareRoot0(
                        (temp_a1_4 * temp_a1_4) + (var_a0_3 * var_a0_3) +
                        (temp_v0_6 * temp_v0_6));
                    var_v1_4 = temp_v0_7;
                    if (temp_v0_7 < 0) {
                        var_v1_4 = temp_v0_7 + 3;
                    }
                    g_FieldEntity[i].MoveSteps = (s16)(var_v1_4 >> 2);
                    g_FieldEntity[i].MoveStep = 0;
                    g_FieldEntity[i].ActionState = 1;
                    lastFrame =
                        *(u16*)&temp_s3_2[g_FieldEntity[i].activeAnimId * 16] -
                        1;
                    g_FieldEntity[i].animLastFrame = lastFrame;
                    if (i == g_PlayerModelId) {
                        FieldEntityLineClear(&D_8007E7AC);
                    }
                } else {
                    if (i == g_PlayerModelId) {
                        if (g_FieldAnimLock == 0) {
                            if (g_FieldEntity[i].ActionArg == 0) {
                                if (arg0 & 0x1000) {
                                    temp_v0_8 = g_FieldEntity[i].MoveStep;
                                    if (temp_v0_8 == 0) {
                                        g_FieldEntity[i].ActionState =
                                            actionEnd;
                                    } else {
                                        ents[i].MoveStep = temp_v0_8 - 1;
                                        temp_v0_9 =
                                            (u16)g_FieldEntity[i]
                                                .animCurrentFrame -
                                            (u16)g_FieldEntity[i].animSpeed;
                                        g_FieldEntity[i].animCurrentFrame =
                                            temp_v0_9;
                                        if (temp_v0_9 < 0) {
                                            g_FieldEntity[i].animCurrentFrame =
                                                (u16)g_FieldEntity[i]
                                                    .animLastFrame *
                                                0x10;
                                        }
                                    }
                                }
                                var_v0_20 = arg0 & 0x4000;
                            } else {
                                if (arg0 & 0x4000) {
                                    temp_v0_10 = g_FieldEntity[i].MoveStep;
                                    if (temp_v0_10 == 0) {
                                        g_FieldEntity[i].ActionState =
                                            actionEnd;
                                    } else {
                                        ents[i].MoveStep = temp_v0_10 - 1;
                                        temp_v0_11 =
                                            (u16)g_FieldEntity[i]
                                                .animCurrentFrame -
                                            (u16)g_FieldEntity[i].animSpeed;
                                        g_FieldEntity[i].animCurrentFrame =
                                            temp_v0_11;
                                        if (temp_v0_11 < 0) {
                                            g_FieldEntity[i].animCurrentFrame =
                                                (u16)g_FieldEntity[i]
                                                    .animLastFrame *
                                                0x10;
                                        }
                                    }
                                }
                                var_v0_20 = arg0 & 0x1000;
                            }
                            if (var_v0_20 != 0) {
                                var_a0_4 = i * 0x84;
                                temp_v1_6 = g_FieldEntity[i].MoveStep;
                                if (temp_v1_6 != g_FieldEntity[i].MoveSteps) {
                                    var_v1_5 = temp_v1_6 + 1;
                                    goto block_134;
                                }
                                goto block_132;
                            }
                            goto block_136;
                        }
                        goto block_131;
                    }
                block_131:
                    var_a0_4 = i * 0x84;
                    temp_v1_7 = g_FieldEntity[i].MoveStep;
                    if (temp_v1_7 == g_FieldEntity[i].MoveSteps) {
                    block_132:
                        g_FieldEntity[i].ActionState = actionEnd;
                        g_FieldEntity[i].PosI = (u16)g_FieldEntity[i].MoveEndI;
                    } else {
                        var_v1_5 = temp_v1_7 + 1;
                    block_134:
                        ents[i].MoveStep = var_v1_5;
                        temp_v0_12 = (u16)g_FieldEntity[i].animCurrentFrame +
                                     (u16)g_FieldEntity[i].animSpeed;
                        g_FieldEntity[i].animCurrentFrame = temp_v0_12;
                        if ((g_FieldEntity[i].animLastFrame * 0x10) <
                            temp_v0_12) {
                            g_FieldEntity[i].animCurrentFrame = 0;
                        block_136:
                        }
                    }
                    temp_s0_9 = (i) * 0x84;
                    g_FieldEntity[i].PosX = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartX, g_FieldEntity[i].MoveEndX,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                    g_FieldEntity[i].PosY = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartY, g_FieldEntity[i].MoveEndY,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                    g_FieldEntity[i].PosZ = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartZ, g_FieldEntity[i].MoveEndZ,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                }
            }
        }
    }
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        actionEnd = 2;
        ents = g_FieldEntity;
        temp_s0_10 = i * 0x84;
        if (g_FieldEntity[i].scriptedMoveMode == 5) {
            temp_a0_9 = g_FieldModelLoaderData[i].modelEntryIndex;
            if (temp_a0_9 != 0xFF) {
                temp_v0_13 = &g_FieldModelData->modelEntries[temp_a0_9];
                temp_s3_3 = &temp_v0_13->modelData[temp_v0_13->animationOffset];
                if (g_FieldEntity[i].ActionState == 0) {
                    g_FieldEntity[i].MoveStartX = g_FieldEntity[i].PosX;
                    g_FieldEntity[i].MoveDirAdd = 0;
                    g_FieldEntity[i].MoveStartY = g_FieldEntity[i].PosY;
                    g_FieldEntity[i].MoveStartZ = g_FieldEntity[i].PosZ;
                    var_a1_2 =
                        g_FieldEntity[i].MoveEndX - g_FieldEntity[i].MoveStartX;
                    if (var_a1_2 < 0) {
                        var_a1_2 += 0xFFF;
                    }
                    temp_a1_5 = var_a1_2 >> 0xC;
                    sp10.vx = temp_a1_5;
                    temp_v1_8 =
                        g_FieldEntity[i].MoveEndY - g_FieldEntity[i].MoveStartY;
                    var_a0_5 = temp_v1_8 >> 0xC;
                    if (temp_v1_8 < 0) {
                        var_a0_5 = (s32)(temp_v1_8 + 0xFFF) >> 0xC;
                    }
                    sp10.vy = var_a0_5;
                    var_v0_24 =
                        g_FieldEntity[i].MoveEndZ - g_FieldEntity[i].MoveStartZ;
                    if (var_v0_24 < 0) {
                        var_v0_24 += 0xFFF;
                    }
                    temp_v0_14 = var_v0_24 >> 0xC;
                    sp10.vz = temp_v0_14;
                    var_v0_25 = SquareRoot0(
                        (temp_a1_5 * temp_a1_5) + (var_a0_5 * var_a0_5) +
                        (temp_v0_14 * temp_v0_14));
                    if (var_v0_25 < 0) {
                        var_v0_25 += 3;
                    }
                    g_FieldEntity[i].MoveSteps = (s16)(var_v0_25 >> 2);
                    g_FieldEntity[i].MoveStep = 0;
                    g_FieldEntity[i].ActionState = 1;
                    lastFrame =
                        *(u16*)&temp_s3_3[g_FieldEntity[i].activeAnimId * 16] -
                        1;
                    g_FieldEntity[i].animLastFrame = lastFrame;
                    if (i == g_PlayerModelId) {
                        FieldEntityLineClear(&D_8007E7AC);
                    }
                } else {
                    if (i == g_PlayerModelId) {
                        if (g_FieldAnimLock == 0) {
                            if (g_FieldEntity[i].ActionArg == 0) {
                                if (arg0 & 0x8000) {
                                    temp_v0_15 = g_FieldEntity[i].MoveStep;
                                    if (temp_v0_15 == 0) {
                                        g_FieldEntity[i].ActionState =
                                            actionEnd;
                                    } else {
                                        ents[i].MoveStep = temp_v0_15 - 1;
                                        temp_v0_16 =
                                            (u16)g_FieldEntity[i]
                                                .animCurrentFrame -
                                            (u16)g_FieldEntity[i].animSpeed;
                                        g_FieldEntity[i].animCurrentFrame =
                                            temp_v0_16;
                                        if (temp_v0_16 < 0) {
                                            g_FieldEntity[i].animCurrentFrame =
                                                (u16)g_FieldEntity[i]
                                                    .animLastFrame *
                                                0x10;
                                        }
                                    }
                                }
                                var_v0_26 = arg0 & 0x2000;
                            } else {
                                if (arg0 & 0x2000) {
                                    temp_v0_17 = g_FieldEntity[i].MoveStep;
                                    if (temp_v0_17 == 0) {
                                        g_FieldEntity[i].ActionState =
                                            actionEnd;
                                    } else {
                                        ents[i].MoveStep = temp_v0_17 - 1;
                                        temp_v0_18 =
                                            (u16)g_FieldEntity[i]
                                                .animCurrentFrame -
                                            (u16)g_FieldEntity[i].animSpeed;
                                        g_FieldEntity[i].animCurrentFrame =
                                            temp_v0_18;
                                        if (temp_v0_18 < 0) {
                                            g_FieldEntity[i].animCurrentFrame =
                                                (u16)g_FieldEntity[i]
                                                    .animLastFrame *
                                                0x10;
                                        }
                                    }
                                }
                                var_v0_26 = arg0 & 0x8000;
                            }
                            if (var_v0_26 != 0) {
                                var_a0_6 = i * 0x84;
                                temp_v1_9 = g_FieldEntity[i].MoveStep;
                                if (temp_v1_9 != g_FieldEntity[i].MoveSteps) {
                                    var_v1_6 = temp_v1_9 + 1;
                                    goto block_175;
                                }
                                goto block_173;
                            }
                            goto block_177;
                        }
                        goto block_172;
                    }
                block_172:
                    var_a0_6 = i * 0x84;
                    temp_v1_10 = g_FieldEntity[i].MoveStep;
                    if (temp_v1_10 == g_FieldEntity[i].MoveSteps) {
                    block_173:
                        g_FieldEntity[i].ActionState = actionEnd;
                        g_FieldEntity[i].PosI = (u16)g_FieldEntity[i].MoveEndI;
                    } else {
                        var_v1_6 = temp_v1_10 + 1;
                    block_175:
                        ents[i].MoveStep = var_v1_6;
                        temp_v0_19 = (u16)g_FieldEntity[i].animCurrentFrame +
                                     (u16)g_FieldEntity[i].animSpeed;
                        g_FieldEntity[i].animCurrentFrame = temp_v0_19;
                        if ((g_FieldEntity[i].animLastFrame * 0x10) <
                            temp_v0_19) {
                            g_FieldEntity[i].animCurrentFrame = 0;
                        block_177:
                        }
                    }
                    temp_s0_11 = (i) * 0x84;
                    g_FieldEntity[i].PosX = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartX, g_FieldEntity[i].MoveEndX,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                    g_FieldEntity[i].PosY = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartY, g_FieldEntity[i].MoveEndY,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                    g_FieldEntity[i].PosZ = FieldCalcLinearStep(
                        g_FieldEntity[i].MoveStartZ, g_FieldEntity[i].MoveEndZ,
                        g_FieldEntity[i].MoveSteps, g_FieldEntity[i].MoveStep);
                }
            }
        }
    }
}
#endif

void FieldEntityGatewayMapLoad(FieldGateway* gateway) {
    g_FieldStateData.eventCmd = EVTCMD_FIELD_MAP_CHANGE;
    g_FieldStateData.eventCmdParam = gateway->destFieldId;
    g_FieldStateData.pcPosX = gateway->destPosX;
    g_FieldStateData.pcPosY = gateway->destPosY;
    g_FieldStateData.pcWalkMeshId = gateway->destWalkMeshId;
    *(u16*)&g_FieldStateData.pcDirection = gateway->destDirection;
}

/* Per-frame talk scan: on the rising edge of the OK button, score every entity
 * by how directly the player faces it (and how near), then request the talk
 * script of the best candidate.
 *
 * 30 rows -> 3, on two lines and a declaration:
 *   - the score is written into `quality[i]` and then read back out of it, not
 *     computed into a temporary and stored once. `quality[i] = (u8)(Dir -
 *     dirTo); if (quality[i] >= 0x81) quality[i] = 0x100 - quality[i];` gives
 *     the target's two stores, with the unconditional one sunk into the branch
 *     delay slot and overwritten on the other path; cse hands the stored
 *     register straight back, and because it is now an `s16` element rather
 *     than a promoted `u8` the compare is `slti`, not `sltiu`. Writing it as an
 *     if/else over one temporary merges the two stores, needs a second `andi`,
 *     and -- for reasons that only show up in the diff -- also reverses the
 *     order of the two givs. Worth 12 rows and it was the whole "walking
 *     quality-array pointer regalloc wall" this note used to describe.
 *   - `bestId` is `s16`. As `u16` the final `bestId != g_PlayerModelId` test
 *     zero-extends (`andi a0,a3,0xffff`) where the target sign-extends: 7 rows.
 *   - `bestId` is declared *before* `best`. The two are the same type and both
 *     live across the second loop, and the one declared first gets the higher
 *     register; the target has best in a3 and bestId in a2: 8 rows. (Yes, this
 *     contradicts the usual rule that declaration order is inert -- it is inert
 *     for values that do not compete for the same register.)
 *
 * The mask row is fixed: `(Dir - dirTo) & 0xFF`, not `(u8)(Dir - dirTo)`. The
 * cast is folded into the halfword store -- `sh` truncates anyway, so combine
 * drops the `andi` and maspsx fills the slot with a nop -- while the explicit
 * `& 0xFF` is an arithmetic operation on the promoted `int` that survives to
 * the store. The two forms are the same value and one instruction apart; this
 * note previously recorded every *type* for `dirTo` as compiling identically,
 * which is true and was the wrong place to look, because the mask comes from
 * the expression, not the declaration.
 *
 * The last two rows were both about where a constant lands, and neither was
 * a codegen question.
 *
 *   - `requestTalkScript = 1` needs its value in its own statement.
 *     `expand_assignment` computes the destination address before the value,
 *     so `g_FieldEntity[bestId].requestTalkScript = 1;` emits the index
 *     arithmetic ahead of the constant, while `talk = 1;` first emits the
 *     constant and then the store, which is the target's order. The local has
 *     to be `s16`; an `s32` puts a widening node back into the halfword store.
 *
 *   - **The function is not `void`.** The remaining row was `ori v1,zero,0x40`
 *     for the final `best != 0x40` test, one instruction later than the
 *     target has it -- the target puts it in the delay slot of the `beq` that
 *     branches to the epilogue. This note previously called that a pure sched2
 *     permutation, on the reasoning that `sll -> sra -> beq` is a longer
 *     dependence chain than `ori -> beq` so the shift always goes first, and
 *     listed six spellings measured byte-identical and three measured worse.
 *     All of that is true and none of it was the lever: declaring a return
 *     type the body never uses matches outright. It is the same rule
 *     CLAUDE.md records for FieldMainLoop, arrived at from the other side --
 *     there a `void` function was needed to *fill* a slot, here a non-`void`
 *     one is. `s32`, `u8` and `s16` all match, so the .s cannot tell them
 *     apart; `s32` is written here because K&R implicit `int` is the likely
 *     original spelling, and the one caller (src/field/field.c) discards the
 *     result and has no prototype in scope, so it is unaffected either way.
 *
 *     decomp-permuter found this in 721 iterations from a base score of 60,
 *     via `perm_randomize_internal_type`, after a hand search had exhausted
 *     the statement-level dimensions. It is not something reading the target
 *     suggests: the return type is invisible in a function that never sets
 *     $v0.
 */
s32 FieldEntityCheckTalk(void) {
    VECTOR from;
    VECTOR to;
    s16 quality[16];
    s32 sqrDist;
    s16 bestId;
    s16 best;
    u8 dirTo;
    s32 i;
    s16 talk;

    if (!(g_FieldPad2State & 0x20) || (g_FieldPad2PrevState & 0x20)) {
        return;
    }
    from.vx = g_FieldEntity[g_PlayerModelId].PosX >> 12;
    from.vy = g_FieldEntity[g_PlayerModelId].PosY >> 12;
    from.vz = g_FieldEntity[g_PlayerModelId].PosZ >> 12;
    for (i = 0; i < D_8009AC1C; i++) {
        quality[i] = 0x100;
        if (i == g_PlayerModelId) {
            continue;
        }
        if (g_FieldEntity[i].TalkOff != 0) {
            continue;
        }
        to.vx = g_FieldEntity[i].PosX >> 12;
        to.vy = g_FieldEntity[i].PosY >> 12;
        to.vz = g_FieldEntity[i].PosZ >> 12;
        if (from.vx == to.vx && from.vy == to.vy) {
            continue;
        }
        if ((u32)(from.vz - to.vz + 0xFF) >= 0x1FF) {
            continue;
        }
        dirTo = FieldEntityDirByVec(&from, &to, &sqrDist);
        quality[i] = (g_FieldEntity[g_PlayerModelId].Dir - dirTo) & 0xFF;
        if (quality[i] >= 0x81) {
            quality[i] = 0x100 - quality[i];
        }
        if (sqrDist >= g_FieldEntity[i].TalkRange +
                           g_FieldEntity[g_PlayerModelId].SolidRange) {
            quality[i] = 0x100;
        }
    }
    best = 0x40;
    bestId = g_PlayerModelId;
    for (i = 0; i < D_8009AC1C; i++) {
        if (quality[i] < best) {
            best = quality[i];
            bestId = i;
        }
    }
    if (bestId != g_PlayerModelId && best != 0x40) {
        talk = 1;
        g_FieldEntity[bestId].requestTalkScript = talk;
    }
}

s16 FieldEntityGetDirVectorX(u8 arg0) { return g_FieldDirVectors[arg0][0]; }

s16 FieldEntityGetDirVectorY(u8 arg0) { return g_FieldDirVectors[arg0][1]; }

extern u8 g_FieldAtanTable[];

/* Direction (0-255) from one point to another. The third parameter is
 * in/out: it is written with the squared distance, then *overwritten with the
 * distance itself* -- callers compare it against a plain range, not a squared
 * one. The slope of each axis is taken in 12-bit fixed point, divided down by
 * 32, and the arctan table g_FieldAtanTable is indexed by whichever axis is the
 * minor one; the eight-arm ladder is the quadrant correction and every arm
 * shares one final `+ 0x40` and one `& 0xFF`.
 *
 * 91 rows to zero, and none of the five corrections was a codegen tweak:
 *   - `/ 32`, not `>> 5`. A signed division by a power of two carries the
 *     `bgez`/`addiu 0x1f` rounding pair, which a shift does not, and the two
 *     pairs are eight instructions.
 *   - `*sqrDist = dist;` after the SquareRoot0 call. That second store is what
 *     forces the pointer into a callee-saved register (`move s0,a2`), so it
 *     costs a whole extra saved register and the frame with it.
 *   - the dominance test is `slopeX * slopeX > slopeY * slopeY`, evaluated in
 *     that order. Written `<` with the operands swapped it is the same test
 *     and the two `mult`s come out in the other order.
 *   - eight arms, not four: each half tests *both* signs. The four-arm form
 *     looks equivalent because the table is symmetric, but it is a different
 *     program and no amount of scheduling reaches it.
 *   - `slopeX`/`slopeY` are the same variables as `dx`/`dy`. As separate
 *     locals the slopes land in caller-saved registers and the whole ladder
 *     renames; reusing dx/dy lets them coalesce into the registers dx and dy
 *     already hold, which is what the target does.
 * And the last row: the negative table index is `g_FieldAtanTable[-dy * 2]`,
 * not
 * `[-(dy * 2)]`. Negating first makes gcc compute the index into its own
 * register before materialising the table base, so the shift can be stolen
 * into the preceding `blez`'s delay slot and the base is subtracted from;
 * folding the negation outward computes the base first and leaves the slot
 * empty. Same value, three rows. */
u8 FieldEntityDirByVec(VECTOR* from, VECTOR* to, s32* sqrDist) {
    s32 dx;
    s32 dy;
    s32 dist;
    s32 angle;

    dx = to->vx - from->vx;
    dy = to->vy - from->vy;
    *sqrDist = dx * dx + dy * dy;
    dist = SquareRoot0(*sqrDist);
    *sqrDist = dist;
    dx = (dx << 12) / dist / 32;
    dy = (dy << 12) / dist / 32;
    if (dx * dx > dy * dy) {
        if (dx > 0) {
            if (dy > 0) {
                angle = g_FieldAtanTable[dy * 2];
            } else {
                angle = -g_FieldAtanTable[-dy * 2];
            }
        } else {
            if (dy > 0) {
                angle = -0x80 - g_FieldAtanTable[dy * 2];
            } else {
                angle = g_FieldAtanTable[-dy * 2] - 0x80;
            }
        }
    } else {
        if (dy > 0) {
            if (dx > 0) {
                angle = 0x40 - g_FieldAtanTable[dx * 2];
            } else {
                angle = g_FieldAtanTable[-dx * 2] + 0x40;
            }
        } else {
            if (dx > 0) {
                angle = g_FieldAtanTable[dx * 2] - 0x40;
            } else {
                angle = -0x40 - g_FieldAtanTable[-dx * 2];
            }
        }
    }
    return (angle + 0x40) & 0xFF;
}

u8 FieldEntityDirByVec(VECTOR* from, VECTOR* to, s32* sqrDist);

/* One step of "walk towards MoveEnd". Returns 1 while still moving, 0 once the
 * entity is close enough -- either because it came within `range` of the goal
 * or because the remaining distance is below one frame of MoveSpeed, in which
 * case the position is snapped onto the goal exactly. */
s32 FieldEntityAutoMove(FieldEntity* entity, s16 range) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    s32 reach;

    from.vx = entity->PosX >> 12;
    from.vy = entity->PosY >> 12;
    to.vx = entity->MoveEndX >> 12;
    to.vy = entity->MoveEndY >> 12;
    reach = entity->SolidRange + range;
    sqrDist = (to.vx - from.vx) * (to.vx - from.vx) +
              (to.vy - from.vy) * (to.vy - from.vy);
    reach = reach * reach + 0x1000;
    if (range != 0 && reach >= sqrDist) {
        return 0;
    }
    if (sqrDist < (entity->MoveSpeed * entity->MoveSpeed) >> 16 ||
        sqrDist < 4) {
        entity->PosX = entity->MoveEndX;
        entity->PosY = entity->MoveEndY;
        return 0;
    }
    entity->MoveDir =
        FieldEntityDirByVec(&from, &to, &sqrDist) - entity->MoveDirAdd;
    return 1;
}

extern u8 D_8009ACA6[]; // per-triangle edge access bits, one bit per link
extern u16 D_80113F28;
extern u16* D_80114458; // per-triangle edge links, three u16 per triangle
extern s16 D_801144CC;

/* Detect when a moving entity crosses a walkmesh triangle edge: walk the
 * triangle's edges, compute the cross products against the entity's position,
 * and return which edge (if any) the entity is crossing plus the resulting Z.
 *
 * The scratchpad at 0x1F800000 holds the three edge vectors (0x00, 0x10, 0x20,
 * three s32 each) and the entity's position in mesh units at 0x30/0x34, with
 * 0x38 zeroed. D_80114458 is a pointer to three u16 per triangle -- the
 * neighbouring triangle across each edge, or a negative value for none -- and
 * D_8009ACA6 is the per-link access bitmap, one bit per link id, indexed
 * `link >> 3` and shifted by `link - (link >> 3) * 8`. Crossing an edge whose
 * bit is clear just moves to the neighbour and starts over; crossing one whose
 * bit is set (or which has no neighbour) is a real collision and returns +-8
 * according to the sign of the edge dotted with the movement delta.
 *
 * 141 rows, 4 inserted, from an m2c seed that did not compile. Two things got
 * it there and both are general:
 *
 *   - A named `s32* scratch = (s32*)0x1F800000;` assigned in front of the
 *     loop. Written as raw `*(s32*)0x1F8000NN` casts at every access, gcc
 *     rematerialises `lui <t>,0x1f80` before each one -- fifteen extra
 *     instructions -- because a two-instruction constant is cheaper than a
 *     callee-saved register by CONST_COSTS. The target holds the base in $s0
 *     across the whole function, which is what the local produces. The two
 *     stores *before* the loop stay as raw casts: the target reaches them
 *     through the `$at` macro, and it does so precisely because they precede
 *     the first use that creates the pseudo. 211 rows to 160.
 *   - The three arms as a flat `if (c0 < 0) ... else if (c1 < 0) ... else if
 *     (c2 < 0) ... else goto loop;` chain behind a fast-path
 *     `if (c0 >= 0 && c1 >= 0 && c2 >= 0) goto done;`. m2c reconstructs this
 *     as a nest whose inner arms re-test c1 and c2 -- which is what the target
 *     does, since jump threading redirects each fast-path branch straight at
 *     its arm and leaves the chain's own tests behind -- but transcribed
 *     literally the nest lets cse fold the re-tests, and the edge-2 arm then
 *     falls through where the target has edge 0. 160 rows to 141.
 *
 * Measured and rejected:
 *   - the mesh address as a byte offset, `(s16*)(id * 0x18 + 8 +
 *     (s32)D_800E4274)` rather than `D_800E4274 + id * 0xC + 4`: 148 (and 167
 *     against the pre-chain body). The target does add the constant before the
 *     base -- `addiu a1,a2,0x10` then `addu a1,a1,v0` -- so the CLAUDE.md
 *     `n + (s32)p` rule points the right way and still measures worse; the
 *     cost is elsewhere in the same block.
 *   - m2c's arm polarity, accept-in-the-then-arm for edges 0 and 1 and
 *     retry-in-the-then-arm for edge 2: 162.
 *   - every declaration order of the locals, including `result` first and
 *     last: all exactly 141.
 *   - the fast path as `!(c0 >= 0 && c1 >= 0 && c2 >= 0)` and as
 *     `c0 < 0 || c1 < 0 || c2 < 0`: both exactly 141, so fold normalises the
 *     three spellings and the remaining branch-polarity row is not reachable
 *     from the condition.
 *
 * 145 rows to **99 changed / +4 instructions**. Three levers, all found by
 * reading the rows rather than by permuting:
 *
 *   - Write `D_801144CC` and `D_80113F28` out in all three arms rather than
 *     sharing a `store:` tail behind an `edge` variable. Worth 10 rows. The
 *     hypothesis behind it is only half confirmed and the other half is the
 *     open question here: the target's relocation multiset names `D_80113F28`
 *     **six** times against our four, which is three store sites against two,
 *     so gcc is merging the identical `D_80113F28 = *triId;` suffix that all
 *     three arms end with. Writing the arms out does *not* stop it -- the
 *     count is still four afterwards -- so the 10 rows come from the block
 *     layout rather than from the merge, and whatever source shape gives the
 *     original three surviving sites has not been found. Do not re-derive
 *     this from the rows; re-run `permuter_scratch.sh` and read the count.
 *   - Assign `scratch` *before* `result = 0`, not after the `D_80113F28`
 *     store. Whichever pseudo is created first wins the first `bgez`'s delay
 *     slot, and the target puts `lui s0,0x1f80` there. 3 rows.
 *   - The position-to-mesh conversion on its **own** local. The seed reused
 *     `px` for `pos->vx`, `pos->vy` and then the mesh-space x the cross
 *     products need, which welds three unrelated live ranges into one pseudo:
 *     it takes a `$t` register rather than `$v0`, and the scheduler hoists the
 *     `pos->vy` load into the block above to cover it. A separate `mesh` local
 *     is worth **29 rows** and costs nothing. This is the counter-merging
 *     idiom read backwards -- merge counters that describe the same walk,
 *     split values that merely happen to be spelled alike.
 *
 * Measured and rejected on top of that, none better than 99:
 *   - the sibling scratch slots as raw `(s32*)0x1F800010` constants (101), as
 *     `(s32*)(sbase + 0x10)` off a `u8*` twin (112), the triangle base in a
 *     per-call local (139) and as an integer sum (114). The target reaches the
 *     second slot with `addiu a0,s0,%lo(D_1F800010)` where we emit
 *     `ori a0,s0,0x10`: cse relates the two constants either way and picks the
 *     operator itself, and nothing in the C chooses it.
 *   - arm 2 as a fall-through behind `if (cross2 >= 0) goto loop;`, which is
 *     the target's branch polarity at that test -- exactly inert.
 *   - `scratch` declared `VECTOR*` so the three edge copies are
 *     MEM_IN_STRUCT_P on both sides, the standing hypothesis for the length
 *     gap -- exactly inert, 145 to the row.
 *
 * Do not read a barrier sweep here as progress: `do { } while (0);` ahead of
 * the fast-path test scored 132 against the then-current 145 and ahead of the
 * cross products 127, both by *adding* 8 or 9 load-delay nops (+4 insertions
 * becomes +12/+13). Rows are only comparable at equal length, so all six
 * placements were rejected.
 *
 * What is left is the three `FieldEntityVectorSub` call sites, which differ
 * only in whether the +8 lands on the scaled index or on the computed pointer,
 * and the register rotation behind them. A permuter target. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityWalkmechCross);
#else
s32 FieldEntityWalkmechCross(
    u16* triId, VECTOR* pos, VECTOR* delta, VECTOR* outEdge) {
    s16* v;
    s32 cross0;
    s32 cross1;
    s32 cross2;
    s32 px;
    s32 mesh;
    s32 py;
    s32* scratch;
    s32 result;
    s32 shift;
    s16 link;

    scratch = (s32*)0x1F800000;
    result = 0;
    mesh = pos->vx;
    if (mesh < 0) {
        mesh += 0xFFF;
    }
    *(s32*)0x1F800030 = mesh >> 12;
    mesh = pos->vy;
    if (mesh < 0) {
        mesh += 0xFFF;
    }
    *(s32*)0x1F800034 = mesh >> 12;
    *(s32*)0x1F800038 = 0;
    D_80113F28 = 0xFFFF;
loop:
    FieldEntityVectorSub(
        scratch, D_800E4274 + *triId * 0xC + 4, D_800E4274 + *triId * 0xC);
    FieldEntityVectorSub(scratch + 4, D_800E4274 + *triId * 0xC + 8,
                         D_800E4274 + *triId * 0xC + 4);
    FieldEntityVectorSub(
        scratch + 8, D_800E4274 + *triId * 0xC, D_800E4274 + *triId * 0xC + 8);
    v = D_800E4274 + *triId * 0xC;
    px = scratch[12];
    py = scratch[13];
    cross0 = (px - v[0]) * scratch[1] - (py - v[1]) * scratch[0];
    cross1 = (px - v[4]) * scratch[5] - (py - v[5]) * scratch[4];
    cross2 = (px - v[8]) * scratch[9] - (py - v[9]) * scratch[8];
    if (cross0 >= 0 && cross1 >= 0 && cross2 >= 0) {
        goto done;
    }
    if (cross0 < 0) {
        link = D_80114458[*triId * 3];
        shift = link >> 3;
        if (link >= 0 && ((D_8009ACA6[shift] >> (link - shift * 8)) & 1) == 0) {
            goto retry;
        }
        outEdge->vx = scratch[0];
        outEdge->vy = scratch[1];
        outEdge->vz = scratch[2];
        result = -8;
        if (scratch[0] * delta->vx + scratch[1] * delta->vy >= 0) {
            result = 8;
        }
        D_801144CC = 0;
        D_80113F28 = *triId;
    } else if (cross1 < 0) {
        link = D_80114458[*triId * 3 + 1];
        shift = link >> 3;
        if (link >= 0 && ((D_8009ACA6[shift] >> (link - shift * 8)) & 1) == 0) {
            goto retry;
        }
        outEdge->vx = scratch[4];
        outEdge->vy = scratch[5];
        outEdge->vz = scratch[6];
        result = -8;
        if (scratch[4] * delta->vx + scratch[5] * delta->vy >= 0) {
            result = 8;
        }
        D_801144CC = 1;
        D_80113F28 = *triId;
    } else if (cross2 < 0) {
        link = D_80114458[*triId * 3 + 2];
        shift = link >> 3;
        if (link >= 0 && ((D_8009ACA6[shift] >> (link - shift * 8)) & 1) == 0) {
        retry:
            *triId = link;
            goto loop;
        }
        outEdge->vx = scratch[8];
        outEdge->vy = scratch[9];
        outEdge->vz = scratch[10];
        result = -8;
        if (scratch[8] * delta->vx + scratch[9] * delta->vy >= 0) {
            result = 8;
        }
        D_801144CC = 2;
        D_80113F28 = *triId;
    } else {
        goto loop;
    }
done:
    pos->vz = FieldEntityCalculateZ(
        scratch, scratch + 4, scratch + 12, D_800E4274 + *triId * 0xC);
    return result;
}
#endif

void FieldEntityVectorSub(s32* arg0, s16* arg1, s16* arg2) {
    arg0[0] = arg1[0] - arg2[0];
    arg0[1] = arg1[1] - arg2[1];
    arg0[2] = arg1[2] - arg2[2];
}

/* Height of `point` on the triangle plane spanned by edgeA/edgeB through
 * `vertex`. edgeA doubles as scratch: once the normal is known it is reloaded
 * with the vertex, so the caller must treat it as clobbered. */
s32 FieldEntityCalculateZ(s32* edgeA, s32* edgeB, s32* point, s16* vertex) {
    s32 normal[3];

    normal[0] = -edgeA[1] * edgeB[2] + edgeB[1] * edgeA[2];
    normal[1] = -edgeA[2] * edgeB[0] + edgeA[0] * edgeB[2];
    normal[2] = -edgeA[0] * edgeB[1] + edgeB[0] * edgeA[1];
    edgeA[0] = vertex[0];
    edgeA[1] = vertex[1];
    edgeA[2] = vertex[2];
    return (normal[0] * edgeA[0] + normal[1] * edgeA[1] + normal[2] * edgeA[2] -
            normal[0] * point[0] - normal[1] * point[1]) /
           normal[2];
}

/* One entry of the map's background-trigger block. Even `type`s arm the
 * trigger, odd ones disarm it. */
typedef struct {
    /* 0x00 */ LinePos pos;
    /* 0x0C */ u8 entityId;
    /* 0x0D */ u8 unk0D;
    /* 0x0E */ u8 type;
    /* 0x0F */ u8 unk0F;
} FieldBgTrigger;

u8 FieldEntityLineCheck(FieldEntity*, FieldLine*, VECTOR*); // extern
extern void FieldEntityTriggerCheck(
    FieldEntity* entity, FieldBgTrigger* trigger, VECTOR* dest);
extern s32 FieldEntityWalkmechCross(
    u16* triId, VECTOR* pos, VECTOR* delta, VECTOR* outEdge);
extern void OuterProduct0(VECTOR* v0, VECTOR* v1, VECTOR* out);
extern s32 VectorNormal(VECTOR* v0, VECTOR* out);
extern /*?*/ s32 D_80074F10;
extern s16 D_8009AC22;
extern s16 D_8009AC24;
extern u8 D_8009AC2A;
extern u8 g_FieldLineCheckResult;

/* Step one field entity along its current direction, sliding along walkmesh
 * edges when the straight path is blocked.
 *
 * The walkmesh triangle the entity stands on is three SVECTORs, 0x18 apart in
 * the mesh data; the two edge vectors and their cross product give the
 * triangle's plane, VectorNormal turns that into a unit normal, and the two
 * horizontal components are divided out to a 1.12 slope that the vertical step
 * follows. The entity is then probed three times per attempt -- at its
 * direction, at +0x20 and at -0x20 -- and the direction is nudged by 8 until
 * one of them clears, up to sixteen times.
 *
 * 325 rows / 56 insertions at 786 instructions against 759: the first numbers
 * this function has ever had. The m2c seed did not compile at all -- it read
 * the scratchpad through `(void*)0x1F800040->unkNN`, opened with four
 * `saved_reg_sN` reads, and invented prototypes for five callees. What the
 * rewrite established:
 *
 *   - The scratchpad from 0x1F800040 is six VECTORs, not the flat words m2c
 *     renders: edge0, edge1, the slope, the destination, the probe position
 *     and the step delta, at 0x40, 0x50, 0x60, 0x70, 0x80 and 0x90. m2c writes
 *     them three different ways -- `*(s32*)0x1F8000NN`, `*(void*)0x1F8000NN`
 *     and `(void*)0x1F800040->unkNN` -- and all three are the same objects.
 *   - **One base pointer, not six.** `VECTOR* spad = (VECTOR*)0x1F800040;`
 *     with `spad[0..5]` is worth 18 instructions and 25 rows over six named
 *     pointers: as six locals they spill and every use is an `lw`, where the
 *     target derives each from one register with an `addiu`. That is the same
 *     lever the parked FieldEntityWalkmechCross note above describes for the
 *     scratchpad at 0x1F800000, arrived at independently here.
 *   - The entity index wants an `s32` local. `g_FieldEntity[entityId]` with the
 *     `s16` parameter re-extends and re-scales at each of 37 subscripts;
 *     `s32 id = entityId;` is worth 19 instructions.
 *   - m2c's prototypes were wrong for all five callees. The real ones:
 *     `FieldEntityWalkmechCross(u16*, VECTOR*, VECTOR*, VECTOR*)` (m2c wrote
 *     the last two `void*`), `OuterProduct0(VECTOR*, VECTOR*, VECTOR*)` and
 *     `VectorNormal(VECTOR*, VECTOR*)` -- m2c gave each an extra argument it
 *     read out of caller register setup -- and FieldEntityTriggerCheck's
 *     middle parameter is a pointer, not the `s32` m2c inferred.
 *   - The four `var_X = saved_reg_X;` reads at the top are m2c modelling
 *     locals that really are read uninitialised: on the path where the probe
 *     block is skipped entirely, sp20, sp28, var_s5, var_s6 and var_fp reach
 *     the final test unwritten. Deleting the reads is faithful.
 *
 * 27 instructions over, and the residue is one shape: `addu` +12 and `sll` +11
 * are still index arithmetic the target does not repeat, against `addiu` -7.
 * m2c's seed carried three byte-offset locals (temp_s0, temp_s3, temp_a1) for
 * exactly the three points where the target recomputes `entityId * 0x84`, so
 * the next thing to try is the byte-offset access form CLAUDE.md prescribes
 * for the $at wall -- `*(s32*)((u8*)g_FieldEntity + off + 0x0C)` -- with those
 * three locals restored, rather than the subscripts used here. The gap tool
 * puts the largest missing runs at want[211] (an index computation and a
 * store) and want[128] (`mflo lui lw sll mult`, a multiply the slope
 * calculation does not have here).*
 * 381 rows / +27 instructions -> **357 / +6**, and the lever was found by
 * counting rather than reading: `tools/insn_histogram.py` reports
 * `%hi(g_FieldEntity)` **36** times here against the target's **zero**. The
 * target never names the array. It names ten *interior* labels instead, and
 * the counts line up one-for-one with this body's member accesses:
 *
 *     D_80074EDA +0x36 MoveDir          15    D_80074F04 +0x60 animSpeed 1
 *     D_80074F10 +0x6C SolidRange        6    D_80074F01 +0x5D scriptedMoveMode
 * 1 D_80074F16 +0x72 PosI              4    D_80074F02 +0x5E activeAnimId 1
 *     D_80074F14 +0x70 MoveSpeed         2    D_80074EB0 +0x0C PosX 2
 *     D_80074EB4 +0x10 PosY              2    D_80074EB8 +0x14 PosZ 2
 *
 * Ten labels, 36 references, exactly the 36 `g_FieldEntity[id].member`
 * accesses below. So every entity read in the original is the byte-offset
 * form CLAUDE.md records for the debug functions -- `*(u16*)((u8*)&D_80074F16
 * + off)` -- and the subscript spelling is what was costing 27 instructions
 * and most of the register naming.
 *
 * The offset has to be a **named local computed once**: written inline as
 * `id * 0x84` at all 36 sites the body comes out at **-10 instructions**
 * (472 rows), because cse folds the multiply and the addresses collapse;
 * `off = id * 0x84;` as the first statement is +6.
 *
 * `FieldEntityMovementUpdate` has the same fault and it is larger there --
 * 243 materialisations of `g_FieldEntity` against 4, with `D_80074ED6`
 * (+0x32, `MoveStep`) 22 times and `D_80074F06` (+0x62, `animCurrentFrame`)
 * 18 -- so the same transformation is the next thing to do to it.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityMove);
#else
s32 FieldEntityMove(s16 entityId) {
    VECTOR* spad = (VECTOR*)0x1F800040;
    s32 id = entityId;
    s32 off;
    u16 sp10;
    s16 sp18;
    s32 sp20;
    s32 sp28;
    s32 sp30;
    s32 sp38;
    s32 sp40;

    FieldEntity* temp_s0_2;
    FieldModelLoaderData* var_v0_10;
    s16 temp_a3;
    s16* var_a2_2;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_lo;
    s32 temp_lo_2;
    s32 temp_lo_3;
    s32 temp_lo_4;
    s32 temp_s0;
    s32 temp_s3;
    s32 temp_t3;
    s32 temp_t3_2;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2;
    s32 var_fp;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_5;
    s32 var_v0_6;
    s32 var_v0_7;
    s32 var_v0_8;
    s32 var_v1;
    s32 var_v1_2;
    s8 var_v0_9;
    u32 var_s7;
    u8 var_a0_3;
    SVECTOR* tri;

    off = id * 0x84;
    sp10 = *(u16*)((u8*)&D_80074F16 + off);
    tri = (SVECTOR*)((u8*)D_800E4274 + sp10 * 0x18);
    spad[0].vx = (s32)(tri[1].vx - tri[0].vx);
    spad[0].vy = tri[1].vy - tri[0].vy;
    spad[0].vz = tri[1].vz - tri[0].vz;
    spad[1].vx = tri[2].vx - tri[1].vx;
    temp_a3 = tri[1].vy;
    spad[1].vy = tri[2].vy - temp_a3;
    sp18 = entityId;
    spad[1].vz = tri[2].vz - tri[1].vz;
    OuterProduct0(&spad[0], &spad[1], &spad[2]);
    var_v0_2 = spad[2].vx;
    if (var_v0_2 < 0) {
        var_v0_2 += 0xFF;
    }
    var_v1 = spad[2].vy;
    spad[2].vx = var_v0_2 >> 8;
    if (var_v1 < 0) {
        var_v1 += 0xFF;
    }
    var_a2 = spad[2].vz;
    spad[2].vy = (s32)(var_v1 >> 8);
    if (var_a2 < 0) {
        var_a2 += 0xFF;
    }
    spad[2].vz = (s32)(var_a2 >> 8);
    VectorNormal(&spad[2], &spad[2]);
    temp_v0_2 = spad[2].vx;
    var_v1_2 = temp_v0_2 * temp_v0_2;
    if (var_v1_2 < 0) {
        var_v1_2 += 0xFFF;
    }
    temp_v0_3 = spad[2].vz;
    var_a0 = temp_v0_3 * temp_v0_3;
    if (var_a0 < 0) {
        var_a0 += 0xFFF;
    }
    temp_a0 = spad[2].vz;
    temp_v0_4 = spad[2].vy;
    var_v0_3 = temp_v0_4 * temp_v0_4;
    spad[2].vx = (s32)(temp_a0 << 0xC) /
                 SquareRoot12((var_v1_2 >> 0xC) + (var_a0 >> 0xC));
    if (var_v0_3 < 0) {
        var_v0_3 += 0xFFF;
    }
    var_a0_2 = temp_a0 * temp_a0;
    if (var_a0_2 < 0) {
        var_a0_2 += 0xFFF;
    }
    temp_lo = (s32)(spad[2].vz << 0xC) /
              SquareRoot12((var_v0_3 >> 0xC) + (var_a0_2 >> 0xC));
    spad[2].vy = temp_lo;
    if (spad[2].vx >= 0x1001) {
        spad[2].vx = 0x1000;
    }
    if (spad[2].vx < -0x1000) {
        spad[2].vx = -0x1000;
    }
    if (temp_lo >= 0x1001) {
        spad[2].vy = 0x1000;
    }
    if (spad[2].vy < -0x1000) {
        spad[2].vy = -0x1000;
    }
    if (spad[2].vz >= 0x1001) {
        spad[2].vz = 0x1000;
    }
    if (spad[2].vz < -0x1000) {
        spad[2].vz = -0x1000;
    }
    temp_t3 = spad[2].vx;
    sp38 = temp_t3;
    if (temp_t3 < 0) {
        sp38 = -temp_t3;
    }
    temp_t3_2 = spad[2].vy;
    sp40 = temp_t3_2;
    if (temp_t3_2 < 0) {
        sp40 = -temp_t3_2;
    }
    var_s7 = 0;
    temp_s0 = sp18 * 0x84;
loop_31:
    var_s7 += 1;
    if (sp18 == g_PlayerModelId) {
        var_v0_4 = var_s7 < 0x11U;
        if (g_FieldLineCheckResult == 1) {
            if (var_s7 >= 3U) {
                g_FieldLineCheckResult = 0;
            } else {
                goto block_37;
            }
        } else {
            goto block_36;
        }
    } else {
        var_v0_4 = var_s7 < 0x11U;
    block_36:
        if (var_v0_4 != 0) {
        block_37:
            var_v0_5 =
                FieldEntityGetDirVectorX(*(u8*)((u8*)&D_80074EDA + off)) * sp38;
            if (var_v0_5 < 0) {
                var_v0_5 += 0xFFF;
            }
            spad[3].vx = (s32)(var_v0_5 >> 0xC);
            var_v0_6 =
                -(FieldEntityGetDirVectorY(*(u8*)((u8*)&D_80074EDA + off)) *
                  sp40);
            if (var_v0_6 < 0) {
                var_v0_6 += 0xFFF;
            }
            spad[3].vy = (s32)(var_v0_6 >> 0xC);
            var_v0_7 = *(u16*)((u8*)&D_80074F14 + off) * spad[3].vx;
            if (var_v0_7 < 0) {
                var_v0_7 += 0xFF;
            }
            spad[3].vx = (s32)(var_v0_7 >> 8);
            var_v0_8 = *(u16*)((u8*)&D_80074F14 + off) * spad[3].vy;
            if (var_v0_8 < 0) {
                var_v0_8 += 0xFF;
            }
            spad[3].vy = (s32)(var_v0_8 >> 8);
            spad[3].vx = (s32)(*(s32*)((u8*)&D_80074EB0 + off) + spad[3].vx);
            spad[3].vy = (s32)(*(s32*)((u8*)&D_80074EB4 + off) + spad[3].vy);
            spad[3].vz = (s32) * (s32*)((u8*)&D_80074EB8 + off);
            spad[5].vx =
                (s32)(FieldEntityGetDirVectorX(
                          (*(u8*)((u8*)&D_80074EDA + off) + 0x20) & 0xFF) *
                      *(u16*)((u8*)&D_80074F10 + off));
            temp_lo_2 = -FieldEntityGetDirVectorY(
                            (*(u8*)((u8*)&D_80074EDA + off) + 0x20) & 0xFF) *
                        *(u16*)((u8*)&D_80074F10 + off);
            spad[4].vz = (s32)spad[3].vz;
            spad[5].vy = temp_lo_2;
            spad[4].vx = (s32)(spad[3].vx + spad[5].vx);
            spad[4].vy = (s32)(spad[3].vy + spad[5].vy);
            sp20 =
                FieldEntityWalkmechCross(&sp10, &spad[4], &spad[5], &spad[1]);
            temp_v0_5 = FieldEntityCollisionCheck(sp18, &spad[4]);
            sp10 = *(u16*)((u8*)&D_80074F16 + off);
            sp28 = temp_v0_5 != 0;
            spad[5].vx =
                (s32)(FieldEntityGetDirVectorX(
                          (*(u8*)((u8*)&D_80074EDA + off) - 0x20) & 0xFF) *
                      *(u16*)((u8*)&D_80074F10 + off));
            temp_lo_3 = -FieldEntityGetDirVectorY(
                            (*(u8*)((u8*)&D_80074EDA + off) - 0x20) & 0xFF) *
                        *(u16*)((u8*)&D_80074F10 + off);
            spad[4].vz = (s32)spad[3].vz;
            spad[5].vy = temp_lo_3;
            spad[4].vx = (s32)(spad[3].vx + spad[5].vx);
            spad[4].vy = (s32)(spad[3].vy + spad[5].vy);
            var_fp =
                FieldEntityWalkmechCross(&sp10, &spad[4], &spad[5], &spad[2]);
            temp_v0_6 = FieldEntityCollisionCheck(sp18, &spad[4]);
            sp10 = *(u16*)((u8*)&D_80074F16 + off);
            var_s6 = temp_v0_6 != 0;
            spad[5].vx =
                (s32)(FieldEntityGetDirVectorX(*(u8*)((u8*)&D_80074EDA + off)) *
                      *(u16*)((u8*)&D_80074F10 + off));
            var_s4 = 0;
            temp_lo_4 =
                -FieldEntityGetDirVectorY(*(u8*)((u8*)&D_80074EDA + off)) *
                *(u16*)((u8*)&D_80074F10 + off);
            spad[4].vz = (s32)spad[3].vz;
            spad[5].vy = temp_lo_4;
            spad[4].vx = (s32)(spad[3].vx + spad[5].vx);
            spad[4].vy = (s32)(spad[3].vy + spad[5].vy);
            var_s5 =
                FieldEntityWalkmechCross(&sp10, &spad[4], &spad[5], &spad[0]);
            if (FieldEntityCollisionCheck(sp18, &spad[4]) != 0) {
                var_s4 = (var_s5 == 0) * 8;
            }
            if ((var_s5 != 0) || (sp20 != 0) || (var_fp != 0) ||
                (var_s4 != 0) || (sp28 != 0) || (var_s6 != 0)) {
                if ((sp18 == g_PlayerModelId) && (g_FieldAnimLock == 0)) {
                    if ((var_s4 == 0) && (sp28 == 0) && (var_s6 == 0)) {
                        goto block_68;
                    }
                } else {
                    if ((var_s5 != 0) && (sp20 == 0) && (var_fp == 0)) {
                        var_v0_9 = *(u8*)((u8*)&D_80074EDA + off) - var_s5;
                        goto block_67;
                    }
                    if ((var_s4 != 0) && (sp28 == 0) && (var_s6 == 0)) {
                        var_v0_9 = *(u8*)((u8*)&D_80074EDA + off) - var_s4;
                    block_67:
                        *(u8*)((u8*)&D_80074EDA + off) = var_v0_9;
                    }
                block_68:
                    if (sp20 != 0) {
                        if (var_fp == 0) {
                            goto block_72;
                        }
                    } else {
                        if (sp28 != 0) {
                        block_72:
                            *(u8*)((u8*)&D_80074EDA + off) =
                                *(u8*)((u8*)&D_80074EDA + off) + 0xF8;
                        } else if ((var_fp != 0) || (var_s6 != 0)) {
                            *(u8*)((u8*)&D_80074EDA + off) =
                                *(u8*)((u8*)&D_80074EDA + off) + 8;
                        }
                        goto loop_31;
                    }
                }
            }
        }
    }
    temp_s3 = sp18 * 0x84;
    sp30 = FieldEntityWalkmechCross(
        &*(u16*)((u8*)&D_80074F16 + off), &spad[3], &spad[5], &spad[0]);
    if ((sp18 == g_PlayerModelId) && (g_FieldAnimLock == 0)) {
        temp_s0_2 = &g_FieldEntity[id];
        g_FieldLineCheckResult =
            FieldEntityLineCheck(temp_s0_2, &D_8007E7AC, &spad[3]);
        if (g_FieldStateData.mapJumpDisabled == 0) {
            FieldEntityGatewayCheck(
                temp_s0_2, g_FieldTriggers + 0x38, &spad[3]);
        }
        FieldEntityTriggerCheck(temp_s0_2, g_FieldTriggers + 0x158, &spad[3]);
    }
    var_v0 = 0;
    if ((var_s5 == 0) && (sp20 == 0) && (var_fp == 0) && (var_s4 == 0) &&
        (sp28 == 0) && (var_s6 == 0) && (sp30 == 0)) {
        temp_a1 = sp18 * 0x84;
        *(s32*)((u8*)&D_80074EB0 + off) = spad[3].vx;
        *(s32*)((u8*)&D_80074EB4 + off) = spad[3].vy;
        *(s32*)((u8*)&D_80074EB8 + off) = spad[3].vz << 0xC;
        var_v0 = 1;
        if (*(u8*)((u8*)&D_80074F01 + off) == 0) {
            var_v0 = 1;
            if (sp18 == g_PlayerModelId) {
                *(s16*)((u8*)&D_80074F04 + off) = 0x10;
                if (g_FieldPadRaw & 0x40) {
                    var_a2_2 = &g_FieldStateData.runAnimId;
                    var_v0_10 = &g_FieldModelLoaderData[sp18];
                } else {
                    var_a2_2 = &g_FieldStateData.walkAnimId;
                    var_v0_10 = &g_FieldModelLoaderData[sp18];
                }
                var_a0_3 = 0;
                if (*var_a2_2 < (s32)g_FieldModelData
                                    ->modelEntries[var_v0_10->modelEntryIndex]
                                    .animationCount) {
                    var_a0_3 = (u8)*var_a2_2;
                }
                *(u8*)((u8*)&D_80074F02 + off) = var_a0_3;
                var_v0 = 1;
            }
        }
    }
    return var_v0;
}
#endif

extern s16 D_8009AC1C;

/* Would `pos` put entity `entityId` inside another solid entity? Two entities
 * collide when their horizontal distance falls under the mean of their two
 * solid radii and they are within ~127 units of each other vertically, so
 * characters on a different floor of the same map never block one another.
 * Only the player's own collisions arm the other entity's push script.
 *
 * The entity count is `g_FieldStateData.modelCount` -- FieldState + 0x28 -- and
 * reading it as that struct member rather than through the flat `D_8009AC1C`
 * symbol is what makes this match. As a struct reference the load may alias the
 * `g_FieldEntity[i]` stores in the body, so gcc leaves it in the loop and
 * hoists only its `%hi`/`%lo` address; that address is a movable with a
 * REG_EQUAL note, which `move_movables` lifts into a fresh pseudo and copies
 * into the original one -- the `move t4,a1` the flat spelling cannot produce.
 * A `FieldState*` local instead of the direct member is not the same thing and
 * scores eight rows. */
s32 FieldEntityCollisionCheck(s16 entityId, VECTOR* pos) {
    s16 i;
    s32 hit;
    s32 sqrRadius;
    s32 range;
    s32 dz;
    s32 radius;
    s32 dx;
    s32 dy;

    hit = 0;
    range = g_FieldEntity[entityId].SolidRange;
    for (i = 0; i < g_FieldStateData.modelCount; i++) {
        if (i == entityId) {
            continue;
        }
        if (g_FieldEntity[i].SolidOff != 0) {
            continue;
        }
        dz = (g_FieldEntity[i].PosZ >> 12) - pos->vz;
        if (dz < -126 || dz > 127) {
            continue;
        }
        sqrRadius = (range + g_FieldEntity[i].SolidRange) >> 1;
        radius = sqrRadius;
        dx = (g_FieldEntity[i].PosX - pos->vx) >> 12;
        dy = (g_FieldEntity[i].PosY - pos->vy) >> 12;
        sqrRadius = radius * radius;
        if (sqrRadius > dx * dx + dy * dy) {
            hit = 1;
            if (entityId == g_PlayerModelId) {
                g_FieldEntity[i].requestPushScript = 1;
            }
        }
    }
    return hit;
}

/* Squared distance from `point` to the segment `line`, with the foot of the
 * perpendicular written to `nearest`. Returns -1 when that foot lands outside
 * the segment on either the x or the y axis, which is how callers tell "past
 * the end of the line" apart from "near it". The line parameter runs in 8-bit
 * fixed point, so the projection stays in integer arithmetic throughout.
 *
 * The `goto out` is the whole function: the original returns through a single
 * exit, so the value lives in a pseudo ($a0) and is copied to $v0 in the
 * `jr ra` delay slot. Written as two `return` statements gcc coalesces each
 * one straight into $v0, the delay slot stays empty, and the three `li -1`
 * paths and the final `addu` all name the wrong register -- 6 rows. It has to
 * be the *same* variable that held the line parameter, too: a second local for
 * the distance measures 22, and pre-setting it to -1 above the tests 30. */
s32 FieldEntitySqrDistToLine(FieldLine* line, s32* point, s32* nearest) {
    s32 t;

    t = -(((line->pos.x1 - point[0]) * (line->pos.x2 - line->pos.x1) +
           (line->pos.y1 - point[1]) * (line->pos.y2 - line->pos.y1) +
           (line->pos.z1 - point[2]) * (line->pos.z2 - line->pos.z1))
          << 8) /
        ((line->pos.x2 - line->pos.x1) * (line->pos.x2 - line->pos.x1) +
         (line->pos.y2 - line->pos.y1) * (line->pos.y2 - line->pos.y1) +
         (line->pos.z2 - line->pos.z1) * (line->pos.z2 - line->pos.z1));
    nearest[0] = ((t * (line->pos.x2 - line->pos.x1)) >> 8) + line->pos.x1;
    nearest[1] = ((t * (line->pos.y2 - line->pos.y1)) >> 8) + line->pos.y1;
    nearest[2] = ((t * (line->pos.z2 - line->pos.z1)) >> 8) + line->pos.z1;
    if ((line->pos.x1 - nearest[0] >= 0 && line->pos.x2 - nearest[0] <= 0) ||
        (line->pos.x1 - nearest[0] <= 0 && line->pos.x2 - nearest[0] >= 0)) {
        if ((line->pos.y1 - nearest[1] >= 0 &&
             line->pos.y2 - nearest[1] <= 0) ||
            (line->pos.y1 - nearest[1] <= 0 &&
             line->pos.y2 - nearest[1] >= 0)) {
            t = (nearest[0] - point[0]) * (nearest[0] - point[0]) +
                (nearest[1] - point[1]) * (nearest[1] - point[1]) +
                (nearest[2] - point[2]) * (nearest[2] - point[2]);
            goto out;
        }
    }
    t = -1;
out:
    return t;
}

/* Walk the map's 32 trigger lines against one entity and raise the script
 * requests each is due. Entering a line's radius arms touch-on (and, if the
 * entity crossed the line this frame and faces it within +/-64, push and
 * isOnLine), leaving arms touch-off.
 *
 * Six corrections, four of them program rather than codegen:
 *   - the return value is only raised for a line whose `slipDisabled` byte is
 *     1, not for every line in range. The target reads +0x16 for that test and
 *     +0x0E (touch) for the touch-on test right after it.
 *   - the four-way sign ladder is *not* negated; `across` is set when one of
 *     the four terms holds, which the target's first branch says plainly.
 *   - `to[0]`/`to[1]` are `dest->vx >> 12` and `dest->vy >> 12`, not the raw
 *     members; mixing raw 20.12 coordinates into a buffer whose other four
 *     words are already shifted was a bug.
 *   - the `else { continue; }` on the from/nearest equality test is a
 *     fallthrough into the two stores, not a skip: an entity standing exactly
 *     on a line has to raise its push request.
 *   - `from[k] != nearest[k]`, not the reverse -- gcc 2.6.3 evaluates a
 *     comparison's operands in source order and the target loads `from` first.
 *   - `(s32)(entity->SolidRange * entity->SolidRange)`: two `u16` operands
 *     promote to *unsigned* int, so the compare came out `sltu` for `slt`.
 *
 * The last 40 rows were the shape of the walk, and the answer is the one the
 * earlier notes here ruled out -- because they measured it against a body that
 * was still the wrong program, and none of those numbers transferred:
 *
 *   - every field goes through the walking pointer, none through `lines[i]`.
 *     The subscript form gives the second base a giv at +0 where the target
 *     has `lines + 0x0E`, so every displacement in the loop reads 0x0E high;
 *     40 rows to 24, and the whole loop body then matches to the byte.
 *   - the walking pointer *is the parameter*. With a separate `FieldLine*
 *     line = lines`, gcc merges the two and emits the copy where the loop
 *     starts; using `lines` itself puts `move s2,a1` in the entry block where
 *     the target has it, and the prologue's save order follows. 24 to 9.
 *   - `angle = lines->proximityAngle;` into an `s32` local after the store,
 *     read at the use. Written `delta = (u8)(lines->proximityAngle - ...)`,
 *     cse substitutes the call's return register for the load -- gcc trusts a
 *     `u8`-returning callee to have extended it, so the substitution is free
 *     and the target's `lbu` disappears, which is also the one instruction
 *     this was short. A `u8` local is inert (cse coalesces it with the return
 *     value); `s16` and `s32` both match, and a `volatile` cast at the use
 *     reaches 1 row. 9 to 0. */
u8 FieldEntityLineCheck(FieldEntity* entity, FieldLine* lines, VECTOR* dest) {
    s32* from;
    s32* to;
    s32* nearest;
    s32 sqrDist;
    s32 crossFrom;
    s32 crossTo;
    u8 hit;
    s32 angle;
    s32 delta;
    s32 i;

    from = (s32*)0x1F800000;
    to = (s32*)0x1F800010;
    nearest = (s32*)0x1F800020;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    to[0] = dest->vx >> 12;
    to[1] = dest->vy >> 12;
    to[2] = entity->PosZ >> 12;
    hit = 0;
    for (i = 0; i < 32; i++, lines++) {
        if (lines->isActive != 1) {
            continue;
        }
        lines->isOnLine = 0;
        sqrDist = FieldEntitySqrDistToLine(lines, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < (s32)(entity->SolidRange * entity->SolidRange)) {
            if (lines->slipDisabled == 1) {
                hit = 1;
            }
            if (lines->touch == 0) {
                lines->requestTouchOnScript = 1;
            }
            lines->touch = 1;
            crossFrom =
                (lines->pos.x2 - lines->pos.x1) * (from[1] - lines->pos.y1) -
                (from[0] - lines->pos.x1) * (lines->pos.y2 - lines->pos.y1);
            crossTo =
                (lines->pos.x2 - lines->pos.x1) * (to[1] - lines->pos.y1) -
                (to[0] - lines->pos.x1) * (lines->pos.y2 - lines->pos.y1);
            if ((crossFrom >= 0 && crossTo < 0) ||
                (crossTo >= 0 && crossFrom < 0) ||
                (crossFrom > 0 && crossTo <= 0) ||
                (crossTo > 0 && crossFrom <= 0)) {
                lines->across = 1;
            }
            if (from[0] != nearest[0] || from[1] != nearest[1]) {
                lines->proximityAngle = FieldEntityDirByVec(
                    (VECTOR*)from, (VECTOR*)nearest, &sqrDist);
                angle = lines->proximityAngle;
                delta = (u8)(angle - entity->MoveDir + 0x40);
                if (delta >= 0x80) {
                    continue;
                }
            }
            lines->requestPushScript = 1;
            lines->isOnLine = 1;
        } else {
            if (lines->touch == 1) {
                lines->requestTouchOffScript = 1;
            }
            lines->touch = 0;
        }
    }
    return hit;
}

/* Walk the 32 field lines against one entity: enter/leave each line's trigger
 * volume, and fire its on/off scripts. The `active = 1;` at the top of the loop
 * body is load-bearing and is not a style choice.
 *
 * gcc 2.6.3 hoists two loop-invariants here -- the constant 1 and the address
 * of g_FieldPad2State -- and `move_movables` emits them in the order
 * `scan_loop` recorded them, which is insn order in the loop body. The target
 * has the 1 first. With `pad2 = &g_FieldPad2State;` written *above* the loop it
 * is not a movable at all but an ordinary statement, so it lands ahead of both
 * (19 rows); written as the loop's first statement it becomes a movable but
 * still precedes the 1, whose first use is the `isActive` compare below it
 * (17 rows). Naming the constant and assigning it above the pointer puts the
 * two movables in the target's order, and the whole s2/s3 rename cascade goes
 * with it. Placing the pointer after the two `continue` guards instead is
 * worse (20/2): one conditional jump ahead of it is enough to lose the hoist.
 *
 * Two earlier findings that still hold: the facing test is `(u8)(...) >= 0x40`
 * through the `s32 diff` local, not inline -- inline, combine folds the
 * zero-extension into the compare, proves the sign bit clear and gives `sltiu`
 * where the target has `slti` -- and the line pointer is walked, not indexed.
 * Reading the pad words as `(&g_FieldPad2State)[0]`/`[1]` instead of through
 * the local is 49 rows. */
void FieldEntityLineInteract(FieldEntity* entity, FieldLine* line) {
    s32* from;
    s32* nearest;
    s32 i;
    s32 sqrDist;
    s32 diff;
    u32* pad2;
    s32 active;

    from = (s32*)0x1F800000;
    nearest = (s32*)0x1F800010;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    for (i = 0; i < 32; i++, line++) {
        active = 1;
        pad2 = &g_FieldPad2State;
        if (line->isActive != active) {
            continue;
        }
        if (entity->scriptedMoveMode != 0) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine(line, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < entity->SolidRange * entity->SolidRange) {
            if (line->touch == 0) {
                line->requestTouchOnScript = active;
            }
            line->touch = active;
        } else {
            if (line->touch == 1) {
                line->requestTouchOffScript = active;
            }
            line->touch = 0;
        }
        if (line->isOnLine != 1) {
            continue;
        }
        diff = (u8)(line->proximityAngle - entity->MoveDir + 0x20);
        if (diff >= 0x40) {
            continue;
        }
        if (!(pad2[0] & 0x20)) {
            continue;
        }
        if (pad2[1] & 0x20) {
            continue;
        }
        line->requestTalkScript = active;
    }
}

void FieldEntityLineClear(FieldLine* lines) {
    s32 i;

    for (i = 0; i < LEN(g_FieldLines); i++) {
        lines->isOnLine = 0;
        lines++;
    }
}

/* Did this step take the entity across one of the map's twelve gateway lines?
 * The move is staged in the PS1 scratchpad as two points -- where the entity is
 * now and where it wants to go -- and each gateway near enough to matter gets a
 * pair of 2D cross products, one per point. Opposite signs mean the segment
 * crossed the line, which loads the destination map. A gateway whose
 * destFieldId is 0x7FFF is an unused slot. */
void FieldEntityGatewayCheck(
    FieldEntity* entity, FieldGateway* gateway, VECTOR* dest) {
    s32* from;
    s32* to;
    s32* nearest;
    s32 i;
    s32 sqrDist;
    s16 x1;
    s16 y1;
    s32 dx;
    s32 dy;
    s32 crossFrom;
    s32 crossTo;

    from = (s32*)0x1F800000;
    to = (s32*)0x1F800010;
    nearest = (s32*)0x1F800020;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    to[0] = dest->vx >> 12;
    to[1] = dest->vy >> 12;
    to[2] = entity->PosZ >> 12;
    for (i = 0; i < 12; i++, gateway++) {
        if (gateway->destFieldId == 0x7FFF) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine((FieldLine*)gateway, from, nearest);
        if (sqrDist == -1) {
            continue;
        }
        if (sqrDist >= entity->SolidRange * entity->SolidRange) {
            continue;
        }
        x1 = gateway->pos.x1;
        y1 = gateway->pos.y1;
        dx = gateway->pos.x2 - x1;
        dy = gateway->pos.y2 - y1;
        crossFrom = dx * (from[1] - y1) - (from[0] - x1) * dy;
        crossTo = dx * (to[1] - y1) - (to[0] - x1) * dy;
        if ((crossFrom >= 0 && crossTo < 0) ||
            (crossTo >= 0 && crossFrom < 0) ||
            (crossFrom > 0 && crossTo <= 0) ||
            (crossTo > 0 && crossFrom <= 0)) {
            FieldEntityGatewayMapLoad(gateway);
        }
    }
}

/* Declared out here rather than relying on the definition below: that one is
 * inside a NON_MATCHINGS arm, so the matching build never sees it and gcc
 * falls back to an implicit `int` return -- which drops the sign extension
 * FieldEntityTriggerCheck's callers of it depend on. */
s16 FieldEntityBgTriggerActivate(FieldBgTrigger* trigger, u8 type);

/* Arms (even type) or disarms (odd type) one background trigger, and reports
 * whether that actually changed the bit -- the caller only redraws when it did.
 *
 * The array element is read inline at each use rather than through an `old`
 * local: with the local, both arms allocate the same register for the index
 * and gcc's post-reload cross-jump merges their two identical store tails into
 * one, which is four instructions short of the original. */
s16 FieldEntityBgTriggerActivate(FieldBgTrigger* trigger, u8 type) {
    s32 changed;
    s32 bit;
    s32 mask;

    changed = 0;
    switch (type) {
    case 0:
    case 2:
    case 4:
        bit = 1 << trigger->unk0D;
        if ((g_FieldEntityBgTrigger[trigger->entityId] & bit) == 0) {
            changed = 1;
        }
        g_FieldEntityBgTrigger[trigger->entityId] =
            bit | g_FieldEntityBgTrigger[trigger->entityId];
        break;
    case 1:
    case 3:
    case 5:
        mask = ~(1 << trigger->unk0D);
        if ((u8)(g_FieldEntityBgTrigger[trigger->entityId] | mask) == 0xFF) {
            changed = 1;
        }
        g_FieldEntityBgTrigger[trigger->entityId] =
            mask & g_FieldEntityBgTrigger[trigger->entityId];
        break;
    }
    return changed;
}

/* Walk the 12 background triggers against one entity and arm/disarm each it
 * crosses or comes near. In-proximity arms directly when the entity stands on
 * the line, else needs the entity facing it within +/-64; crossing types 4/5
 * arm/disarm on the back-side sign test. Each state change plays the trigger's
 * sound effect.
 *
 * Six things this needed, in the order they were worth:
 *   - `trigger` is a walking pointer bumped in the `for` header, not
 *     `&triggers[i]`: the pointer form makes it a biv and gcc reduces the
 *     record's byte fields onto a second base at +0xF, which is the
 *     `addiu s0,s1,0xf` and the negative offsets all through the loop. The
 *     subscript form has one base and no giv -- 105 rows against 69.
 *   - `from` and `nearest` are named `s32*` locals. Without them the two
 *     scratchpad addresses are rebuilt through the assembler's $at macro at
 *     every use and the function is two callee-saved registers and 8 bytes
 *     of frame short: 69 rows against 14. The three opening stores stay
 *     spelled as absolute casts even so -- the target writes `0(s5)` for the
 *     first and `4($at)`/`8($at)` for the other two, which is cse relating
 *     only the first to the pointer's own pseudo.
 *   - the sound-effect table is a local `u16 seIds[4]` initialiser. The
 *     target copies it in with lwl/lwr and swl/swr, the unaligned block move
 *     an `s16`-aligned initialiser gets, and reads it back with `lhu`.
 *   - FieldEntityBgTriggerActivate returns `s16`: every caller here
 *     sign-extends v0 through `sll`/`sra` before comparing it against 1.
 *   - the direction test wants an `s32` local for the call result (so the
 *     return is zero-extended into it) and a second `s32` local for the
 *     masked difference (so the comparison is `slti`, not `sltiu`).
 *   - `&entity->PosZ` in an `s32*` local, read by the third store. That
 *     splits the PosZ load off its `sra`/`sw` chain so sched2 can spread the
 *     two `addiu`s into the load-delay slots the way the target does; it was
 *     decomp-permuter's `perm_temp_for_expr`, not a reading of the target.
 *
 * The last row was the two pointers' *declarations*, and it is a general
 * rule worth keeping: a local aggregate initialiser is a block copy emitted
 * at its declaration, so it is a scheduling barrier, and anything that has
 * to issue ahead of it must be declared above it. Written as statements
 * after the declaration block, `nearest`'s `lui`/`ori` is scheduled after
 * the `seIds` copy; written as an initialiser on a declaration placed ahead
 * of `seIds`, gcc emits it as the function's first body insn, which drags
 * `sw s3,0x2c(sp)` in front of `sw ra` in the register saves -- which is
 * what the target has. Only the position relative to `seIds` matters:
 * initialised at a declaration *below* `seIds` it is still 9 rows, and every
 * arrangement above it matches. Five statement orderings of {from, nearest,
 * posZ} had been measured at exactly 7 before this, which is what made the
 * residue look like a scheduling knot with no lever. */
void FieldEntityTriggerCheck(
    FieldEntity* entity, FieldBgTrigger* trigger, VECTOR* dest) {
    s32* from = (s32*)0x1F800000;
    s32* nearest = (s32*)0x1F800020;
    u16 seIds[4] = {0x0, 0x36, 0x7A, 0x12A};
    s32 sqrDist;
    s32 cross;
    s32* posZ;
    s32 dir;
    s32 rel;
    s32 i;

    *(s32*)0x1F800000 = entity->PosX >> 12;
    posZ = &entity->PosZ;
    *(s32*)0x1F800004 = entity->PosY >> 12;
    *(s32*)0x1F800008 = *posZ >> 12;
    for (i = 0; i < 12; i++, trigger++) {
        if (trigger->entityId == 0xFF) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine((FieldLine*)trigger, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < (s32)(entity->SolidRange * entity->SolidRange)) {
            if (from[0] == nearest[0] && from[1] == nearest[1]) {
                if (FieldEntityBgTriggerActivate(trigger, trigger->type) == 1) {
                    func_8001117C(seIds[trigger->unk0F]);
                }
                continue;
            }
            dir =
                FieldEntityDirByVec((VECTOR*)from, (VECTOR*)nearest, &sqrDist);
            rel = (u8)(dir - entity->MoveDir + 0x40);
            if (rel >= 0x80) {
                continue;
            }
            if (FieldEntityBgTriggerActivate(trigger, trigger->type) == 1) {
                func_8001117C(seIds[trigger->unk0F]);
            }
            continue;
        }
        if (trigger->type >= 4) {
            cross = (trigger->pos.x2 - trigger->pos.x1) *
                        (from[1] - trigger->pos.y1) -
                    (trigger->pos.y2 - trigger->pos.y1) *
                        (from[0] - trigger->pos.x1);
            if (cross > 0) {
                continue;
            }
        }
        if (trigger->type == 2 || trigger->type == 4) {
            if (FieldEntityBgTriggerActivate(trigger, 1) == 1) {
                func_8001117C(seIds[trigger->unk0F]);
            }
        }
        if (trigger->type == 3 || trigger->type == 5) {
            if (FieldEntityBgTriggerActivate(trigger, 0) == 1) {
                func_8001117C(seIds[trigger->unk0F]);
            }
        }
    }
}

/* FieldEntityBgTriggerInit below is left as INCLUDE_ASM: every instruction of
 * the C matches, but gcc precedes the switch's jump table with `.align 3` and
 * the original has it 4-byte aligned at .rodata+0xC4, so the table (and all
 * later .rodata) shifts by 4. Same maspsx limitation as IfCheck and friends. */

void FieldEntityBgTriggerInit(FieldBgTrigger* triggers) {
    s32 i;

    /* The pointer walk belongs in the header, after `i++`: the original
     * increments the counter first and the walking pointer second, and a
     * `triggers++` at the end of the body emits them the other way round. */
    for (i = 0; i < 12; i++, triggers++) {
        if (triggers->entityId != 0xFF) {
            switch (triggers->type) {
            case 0:
            case 2:
            case 4:
                FieldEntityBgTriggerActivate(triggers, 1);
                break;
            case 1:
            case 3:
            case 5:
                FieldEntityBgTriggerActivate(triggers, 0);
                break;
            }
        }
    }
}

/////////////////////////////////////////////////
// Begin of field_camera.c
/////////////////////////////////////////////////

const u32 D_800A00DC[] = {0x00000000};

/* The two scratchpad slots FieldModelLoadGlobalModels reads its part and
 * animation staging buffers from. Declared here rather than inside the #else
 * below: an extern that only exists in a NON_MATCHINGS arm is not compiled
 * into the matching build, so gcc substitutes 0 for the identifier, keeps
 * generating code, and the assembler at the end of the pipe still exits 0. */
extern u8 D_800DF08C[];
extern u8 D_800DF0D4[];
extern u8* D_800E0204;

/* The per-field model-file table: one 24-byte record per field, of which the
 * loader uses the first two words as DS_read's sector and size. It is its own
 * object, immediately behind g_FieldLzsInfo. */
extern u32 g_FieldFileSectors[];

/* The face-selection block the model loader leaves in the scratchpad for
 * KawaiLoadEyesMouthTexToVram to read back: which mouth and eye frame to
 * upload, and the model slot whose VRAM tile they go to.
 *
 * Three of the four bytes are written through the record and the last one is
 * not, and the asymmetry is load-bearing rather than sloppy: gcc 2.6.3's
 * true_dependence lets a struct load float past a non-struct store but not
 * past a struct one, so the modelEntries load that sets up the call settles
 * exactly between the slot store and the byte-0 store -- which is where the
 * target has it. All four spelled as casts lets the load float to the top of
 * the body and costs a load-delay nop; all four spelled as members pins it
 * below all of them and costs the same row the other way. */
typedef struct {
    /* 0x0 */ u8 mouth1;
    /* 0x1 */ u8 mouth2;
    /* 0x2 */ u8 eye;
    /* 0x3 */ u8 slot;
} KawaiFaceSel;

void* FieldModelStructInit(FieldModelFileDesc* desc, FieldModelData* data);
u8* FieldModelLoadGlobalModels(
    FieldModelFileDesc* desc, FieldModelData* data, u8* pkts, s32 readFile);
u8* LoadLocalFieldModelAndInitAll(
    FieldModelFileDesc* desc, FieldModelData* data, u8* readFromCd, u32* buf);
s32 KawaiLoadEyesMouthTexToVram(FieldModelEntry* model, u8* faceSel);
void KawaiClearData(void);

/* Top-level field model loader: build the FieldModelData from the loaded model
 * header, stream the field's model set off the CD into the overlay staging
 * buffer at 0x801B0000, load the global and then the local models, clear every
 * model's flags byte except the player's, and finally push each model's
 * eye/mouth texture to VRAM before resetting the KAWAI state.
 *
 * The three scratchpad slots are the loaders' out-of-band parameter block:
 * words 0 and 1 hand FieldModelLoadGlobalModels its part and animation staging
 * buffers, and bytes 0..3 hand KawaiLoadEyesMouthTexToVram the eye/mouth
 * selection for the model it is about to upload. */
void FieldModelLoadAndInit(void) {
    u8* buf;
    u32 i;

    D_800DFCA0 = (FieldTexBlockHeader*)0x80128000;
    buf =
        FieldModelStructInit((FieldModelFileDesc*)D_8007E770, g_FieldModelData);
    g_FieldModelBufferTop = (u32)buf;
    D_800E0204 = buf;
    DS_read(g_FieldFileSectors[g_CurrentFieldIndex * 6],
            g_FieldFileSectors[g_CurrentFieldIndex * 6 + 1],
            (u_long*)0x801B0000, NULL);
    while (SystemCdromReadChain() != 0) {
    }
    ((u8**)0x1F800000)[0] = D_800DF08C;
    ((u8**)0x1F800000)[1] = D_800DF0D4;
    g_FieldModelBufferTop = (u32)FieldModelLoadGlobalModels(
        (FieldModelFileDesc*)D_8007E770, g_FieldModelData,
        (u8*)g_FieldModelBufferTop, 1);
    g_FieldModelBufferTop = (u32)LoadLocalFieldModelAndInitAll(
        (FieldModelFileDesc*)D_8007E770, g_FieldModelData, (u8*)D_800A00DC,
        (u32*)0x801B0000);
    for (i = 1; i < g_FieldModelData->modelCount; i++) {
        g_FieldModelData->modelEntries[i].flags = 0;
    }
    for (i = 0; i < g_FieldModelData->modelCount; i++) {
        ((KawaiFaceSel*)0x1F800000)->mouth2 = 1;
        ((KawaiFaceSel*)0x1F800000)->eye = 0;
        ((KawaiFaceSel*)0x1F800000)->slot = i;
        *(u8*)0x1F800000 = 1; /* deliberately not through the record */
        KawaiLoadEyesMouthTexToVram(
            &g_FieldModelData->modelEntries[i], (u8*)0x1F800000);
    }
    KawaiClearData();
}

s32 FieldCalcWorldToScreenPos(SVECTOR* worldPos, long* screenPos);
void FieldModelAnimCalcMtrxs(
    FieldModelEntry* model, MATRIX* mtx, u8 animId, s32 frame);
void FieldModelPrepareRender(FieldModelEntry* model);
s32 KawaiExecute(FieldModelEntry* model, u8* kawaiData, u8 index, MATRIX* mtx);
s32 KawaiLoadEyesMouthTexToVram(FieldModelEntry* model, u8* faceSel);

extern s8 D_800DF114;
extern struct FieldRenderData* D_800DF118;
extern u8 D_801144D8; // blink RNG cursor

/* Per-frame KAWAI pass over every field entity, in four sweeps. First place
 * each model at its entity's position plus offset and, for the model types the
 * KAWAI script drives (4, 8, 9, 11, 12), build the animation matrices into a
 * scratch matrix and copy the current view matrix into the model's part
 * matrices -- everything else animates straight into the view matrix. Then
 * queue every visible model for rendering, run its KAWAI script, and finally
 * push the eye/mouth texture for the frame, blinking on a random countdown.
 *
 * 2 rows out, and both are the same row: the fourth loop's preheader loads the
 * two eye-state constants in the other order -- target `li s4,1` then
 * `li s6,2`, ours `li s6,2` then `li s4,1`. Everything else, including the
 * frame size, the nine saved registers and every hard-register assignment,
 * matches.
 *
 * The `blinkClosed = 2;` at the top of that loop is not decoration: gcc 2.6.3
 * hoists a loop-invariant constant only when its defining insn is on the
 * loop's always-executed path, so the literal 2 written inside the
 * `KawaiA == 0` arm stays in the loop (`li v0,2` in a branch delay slot) while
 * the 1, whose first use is the unconditional `BlinkOn == 1` compare, is
 * hoisted into a callee-saved register. That costs the whole function a saved
 * register and renames every s-register -- 55 rows. Assigning the 2 to a local
 * at the top of the body makes it hoistable and takes the diff to 2.
 *
 * move_movables emits the hoists in insn order, so the loop-top assignment is
 * always emitted first, and the only way to reverse it is for the 1's movable
 * to come first -- which needs it to be a local too. Written that way
 * (`blinkOpen = 1; blinkClosed = 2;`) the two `li`s come out in the target's
 * order, but the allocator then gives `blinkClosed` s5 and `faceSel` s6 where
 * the target has them the other way round: 13 rows. Measured both ways at s32
 * and u8; the type changes nothing except that a u8 `blinkOpen` folds back
 * into the compare and disappears. Declaration order is inert, as ever.
 *
 * Re-measured against the current body, four more spellings, all of them
 * dead ends and all of them cheap to re-try by accident:
 *   - `blinkOpen = 1;` as an extra loop-top local, in either declaration
 *     order, used in the guard, in the else-arm stores, or in both: 2 rows,
 *     byte-identical to the body below. cse folds the local straight back into
 *     its uses, so no second movable is ever created and the order does not
 *     move. It is not that the spelling is wrong -- it compiles to nothing.
 *   - `blinkClosed = 2;` moved below the first `continue`, or below both: 55.
 *     One conditional jump ahead of it is enough to lose the hoist.
 *   - no local at all, the literal 2 written twice inside the arm: 55.
 *   - a loop-top local holding 1 with the literal 2 left in the arm: 55.
 * So the 2 is hoistable only from the loop top, and the 1's first use is the
 * `BlinkOn` guard below it; in insn order the 2 therefore always comes first.
 *
 * The gate on the 2 is not `maybe_never` -- the 1 is hoisted from uses that
 * are themselves behind two conditional jumps -- it is move_movables'
 * savings/lifetime threshold. The loop-top assignment does not dodge a jump,
 * it stretches the movable's live range across the whole body so
 * `savings * lifetime` clears the bar. Written in the arm the range is two
 * insns and the constant stays put; that is why every arm-local spelling
 * measures 55.
 *
 * The one spelling that reverses the order is `s32 blinkOpen = 1;` at the loop
 * top, used only in the else-arm stores, with the guard keeping its literal:
 * an SImode pseudo stored to a `u8` needs a truncation, so cse cannot fold it
 * away the way it folds a `u8` or `s16` local (both of those measure 2 rows,
 * byte-identical to the body below). That gets `li s4,1` into exactly the
 * target's slot and leaves a *different* residue -- 13 rows that are nothing
 * but `faceSel` and `blinkClosed` trading s5 and s6, both ways round, in the
 * prologue saves, all eight `sb`s and the call argument.
 *
 * That swap is global_alloc priority, `log2(n_refs) * n_refs * freq /
 * live_length`: `blinkClosed` has 3 refs over one loop, `faceSel` 10 refs over
 * the whole function, and the short range wins, so `blinkClosed` is processed
 * first and takes the lower register. Nothing available moves it -- all four
 * declaration positions and `s16`/`s32`/`u8` were measured (13, 13, 13, 2),
 * and assigning `faceSel` just before the fourth loop to shorten its range
 * costs 48. Next pass: attack the priority, not the movable order.
 *
 * The priority has now been attacked, from both sides, and it does not move.
 * `n_refs` is not reachable from the source, because cse re-shares the two
 * constants however they are spelled: writing one of `blinkClosed`'s two
 * stores as the literal 2 (either one), giving the 1 an extra pair of refs by
 * letting `blinkOpen` carry the else-arm stores, and both changes together all
 * measure the same 13 rows and the same s5/s6 swap. So does every declaration
 * position for `blinkOpen`, including first in the function.
 *
 * The other side -- carrying the 1 in a pseudo that already exists, so no
 * allocno is added at all -- is the more interesting negative. `kawaiOp`, the
 * s16 the third loop uses and which is dead by the fourth, gives a *third*
 * residue: 3 rows, with `li s4,0x1` in exactly the target's slot and the
 * faceSel/blinkClosed registers correct, but the guard rematerialising its own
 * `li v0,0x1` instead of comparing against s4 -- an HImode pseudo cannot serve
 * a QImode compare, so cse hands it a fresh constant and the movable is used
 * only by the else-arm stores. `blink`, the s32 the loop assigns later, gives
 * 68: extending its live range to the loop top makes it a movable of its own.
 * So the three shapes available are 2 rows with the order wrong, 3 rows with
 * the compare rematerialised, and 13 rows with two registers swapped, and the
 * function wants a fourth that C does not appear to spell.
 *
 * decomp-permuter has now had a proper run at it and did not find one:
 * 47,000 iterations on 11 workers from a clean scratch (no diagnostics,
 * base.o within 1% of target.o, relocations identical) at base score 20,
 * with perm_ins_block raised to 20 alongside the settings' own
 * perm_temp_for_expr 150 and perm_reorder_stmts 15 -- not one improvement
 * over the base. Declaring the function non-`void`, which is what closed
 * FieldEntityCheckTalk's last row, is exactly inert here too. That is
 * consistent with the diagnosis above being right: allocno_compare's
 * ranking is not something a source-level randomiser can reach.
 *
 * Five more, aimed at the two halves the note names as open, and all of them
 * dead. On the movable-order half: carrying the 1 in `kawaiType` promoted to
 * function scope -- an s8 pseudo, so neither the HImode mismatch that spoils
 * `kawaiOp` nor the live range that spoils `blink` -- is **78 rows**, because
 * hoisting it out of the first two loops' block scope costs those loops. And
 * `s32 blinkOpen` used in the guard *as well as* the else-arm stores is 13,
 * i.e. identical to using it in the stores alone: the guard's compare against
 * an SImode pseudo changes nothing.
 *
 * On the priority half, where the note says to attack next: no. Stretching
 * `blinkClosed`'s live range with a second assignment -- at the top of the
 * function, or just above the third loop -- is exactly 13 either way, because
 * the earlier store is dead and flow deletes it, so `live_length` never moves.
 * Giving `faceSel` two loop-weighted references instead, by writing the first
 * two loops' `*(s32*)0x1F800000 = 3;` through it (`REG_N_REFS += loop_depth`,
 * so each counts double), does raise its rank -- and costs 5 instructions,
 * 27 rows, because those stores then lose their rematerialised `lui`.
 *
 * So the three shapes stand at 2 / 3 / 13 and neither term of
 * allocno_compare is reachable: `n_refs` cannot be raised without emitting
 * something, and `live_length` cannot be raised at all, since every spelling
 * that would stretch it is dead code.
 * Confirmed again from the .lreg dump, which puts numbers on the 13-row
 * shape: faceSel is 19 refs over 644 insns (priority 1180) and blinkClosed
 * 5 over 84 (1190), a 0.8% gap, and the allocno list has them in that order.
 * So one reference on faceSel, or one insn of live range on blinkClosed,
 * would flip it -- and neither is reachable. Measured and all exactly 13:
 * every width of blinkClosed (s32/u16/u32/s16), blinkOpen used in the guard
 * as well as the stores, blinkClosed derived as `blinkOpen + 1` or
 * `blinkOpen * 2` (the forced-movable order is the same), blinkOpen used in
 * the guard alone, and an extra dead local. blink's width is 13 at u32/long
 * and 14 at s16/u8/u16/s8, so it changes the loop but never that pair.
 * Assigning blinkOpen *before* the loop rather than at its top is worse
 * still -- 16 to 51 rows and always +1 instruction, because the loop then
 * builds its own second constant. A u8 blinkOpen at the loop top is 2 rows,
 * i.e. exactly the body below: cse folds it away and nothing changes.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", HandleKawaiDataInModel);
#else
void HandleKawaiDataInModel(struct FieldRenderData* buf) {
    SVECTOR pos;
    long screenPos;
    FieldModelLoaderData* models;
    FieldModelEntry* model;
    u8* faceSel;
    s32* dst;
    s32* src;
    s16 kawaiOp;
    s32 blink;
    u8 blinkClosed;
    s32 i;

    faceSel = (u8*)0x1F800000;
    D_800DF114 = D_80075DEC;
    D_800DF118 = buf;
    models = ((FieldModelFileDesc*)D_8007E770)->models;

    for (i = 0; i < D_8009AC1C; i++) {
        s8 kawaiType;

        if (models[i].modelEntryIndex == 0xFF) {
            continue;
        }
        pos.vx = (g_FieldEntity[i].PosX >> 12) + g_FieldEntity[i].OffsetX;
        pos.vy = (g_FieldEntity[i].PosY >> 12) + g_FieldEntity[i].OffsetY;
        pos.vz =
            ((g_FieldEntity[i].PosZ >> 12) + g_FieldEntity[i].OffsetZ) - 10;
        g_FieldModelData->modelEntries[models[i].modelEntryIndex].translationX =
            pos.vx;
        g_FieldModelData->modelEntries[models[i].modelEntryIndex].translationY =
            pos.vy;
        g_FieldModelData->modelEntries[models[i].modelEntryIndex].translationZ =
            pos.vz;
        if (FieldCalcWorldToScreenPos(&pos, &screenPos) < 0xF00) {
            g_FieldModelData->modelEntries[models[i].modelEntryIndex]
                .rotationZ = g_FieldEntity[i].Dir;
            kawaiType =
                g_FieldModelData->modelEntries[models[i].modelEntryIndex]
                    .kawaiType;
            if (kawaiType == 4 || kawaiType == 8 || kawaiType == 9 ||
                kawaiType == 11 || kawaiType == 12) {
                MATRIX mtx;

                mtx.m[0][0] = mtx.m[1][1] = mtx.m[2][2] = 0x1000;
                mtx.t[0] = mtx.t[1] = mtx.t[2] = 0;
                mtx.m[0][1] = mtx.m[0][2] = mtx.m[1][0] = mtx.m[1][2] =
                    mtx.m[2][0] = mtx.m[2][1] = 0;
                *(s32*)0x1F800000 = 3;
                FieldModelAnimCalcMtrxs(
                    &g_FieldModelData->modelEntries[models[i].modelEntryIndex],
                    &mtx, g_FieldEntity[i].activeAnimId,
                    g_FieldEntity[i].animCurrentFrame >> 4);
                dst = (s32*)g_FieldModelData
                          ->modelEntries[models[i].modelEntryIndex]
                          .partMatrices;
                src = (s32*)D_80071E40;
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
                dst[3] = src[3];
                dst[4] = src[4];
                dst[5] = src[5];
                dst[6] = src[6];
                dst[7] = src[7];
            } else {
                *(s32*)0x1F800000 = 3;
                FieldModelAnimCalcMtrxs(
                    &g_FieldModelData->modelEntries[models[i].modelEntryIndex],
                    D_80071E40, g_FieldEntity[i].activeAnimId,
                    g_FieldEntity[i].animCurrentFrame >> 4);
            }
        }
    }

    for (i = 0; i < D_8009AC1C; i++) {
        s8 kawaiType;

        if (models[i].modelEntryIndex == 0xFF) {
            continue;
        }
        pos.vx = g_FieldEntity[i].PosX >> 12;
        pos.vy = g_FieldEntity[i].PosY >> 12;
        pos.vz = (g_FieldEntity[i].PosZ >> 12) - 10;
        if (FieldCalcWorldToScreenPos(&pos, &screenPos) < 0xF00) {
            model = &g_FieldModelData->modelEntries[models[i].modelEntryIndex];
            kawaiType = model->kawaiType;
            if (kawaiType == 4 || kawaiType == 8 || kawaiType == 9 ||
                kawaiType == 11 || kawaiType == 12) {
                FieldModelPrepareRender(
                    &g_FieldModelData->modelEntries[models[i].modelEntryIndex]);
            } else {
                model->kawaiType = -1;
                FieldModelPrepareRender(
                    &g_FieldModelData->modelEntries[models[i].modelEntryIndex]);
                g_FieldModelData->modelEntries[models[i].modelEntryIndex]
                    .kawaiType = kawaiType;
            }
        }
    }

    for (i = 0; i < D_8009AC1C; i++) {
        if (models[i].modelEntryIndex == 0xFF) {
            continue;
        }
        kawaiOp = g_FieldEntity[i].KawaiOp1;
        if (kawaiOp != 1) {
            continue;
        }
        pos.vx = g_FieldEntity[i].PosX >> 12;
        pos.vy = g_FieldEntity[i].PosY >> 12;
        pos.vz = (g_FieldEntity[i].PosZ >> 12) - 10;
        if (FieldCalcWorldToScreenPos(&pos, &screenPos) < 0xF00) {
            if (KawaiExecute(
                    &g_FieldModelData->modelEntries[models[i].modelEntryIndex],
                    g_FieldEntity[i].KawaiDataOffset, models[i].modelEntryIndex,
                    D_80071E40) == kawaiOp) {
                g_FieldEntity[i].KawaiOp1 = 2;
            }
        }
    }

    for (i = 0; i < D_8009AC1C; i++) {
        blinkClosed = 2;
        if (models[i].modelEntryIndex == 0xFF) {
            continue;
        }
        if (g_FieldEntity[i].BlinkOn == 1) {
            continue;
        }
        if (g_FieldEntity[i].KawaiA == 0) {
            faceSel[0] = blinkClosed;
            faceSel[1] = blinkClosed;
            faceSel[2] = 0;
            faceSel[3] = i;
            blink = (g_RandomTable[D_801144D8++] & 0x1F) + 0x40;
        } else {
            faceSel[0] = 1;
            faceSel[1] = 1;
            faceSel[2] = 0;
            faceSel[3] = i;
            blink = g_FieldEntity[i].KawaiA - 1;
        }
        g_FieldEntity[i].KawaiA = blink;
        KawaiLoadEyesMouthTexToVram(
            &g_FieldModelData->modelEntries[i], faceSel);
    }
}
#endif

// Possable Debug routine. Ran at beginning of every main field loop. (FPS?)
void DebugRunEveryLoop(void) {}

void FieldCameraAssign(void) {
    if (g_FieldMovieStreamActive == 0 || g_FieldCameraMatrixSel == 1) {
        D_80071E40 = *D_80083578;
    } else {
        D_80071E40 = g_DebugMatrixP;
    }
}

/* Drive the CD stream that feeds the MDEC. While a field map is still loading
 * the stream is not touched at all; otherwise the chain reader's status decides
 * whether to arm the ring buffer, start playback, or tear it down. */
void FieldUpdateMovieStream(void) {
    u32 status;

    if (g_isFieldLoading == 1) {
        if (SystemCdromReadChain() == 0) {
            g_isFieldLoading = 2;
        }
        return;
    }
    if (g_FieldStateData.eventCmd == EVTCMD_UNK14) {
        func_80035658();
        g_FieldMovieStreamActive = 0;
        g_FieldMoviePlayed = 0;
        g_FieldStateData.movieCommandState = MOVCMD_DONE;
        return;
    }
    status = SystemCdromReadChain();
    switch (status) {
    case 0:
        if (g_FieldStateData.eventCmd == EVTCMD_LOAD_MOVIE &&
            g_FieldStateData.movieCommandState == MOVCMD_IDLE) {
            if (g_FieldModelBufferTop <= 0x801AFFFF) {
                func_80034FC8(
                    g_FieldModelBufferTop, g_FieldStateData.eventCmdParam);
            } else {
                func_80034FC8(0x801B0000, g_FieldStateData.eventCmdParam);
            }
            g_FieldStateData.movieCommandState = MOVCMD_ACTIVE;
            g_FieldMoviePlayed = 1;
        }
        if ((s16)g_FieldMovieStreamActive == 1) {
            g_FieldMovieStreamDone = 1;
            g_FieldMovieStreamActive = 0;
            g_FieldMoviePlayed = 0;
            g_FieldStateData.movieCommandState = MOVCMD_DONE;
        }
        break;
    case 0xA:
        if (g_FieldStateData.eventCmd == EVTCMD_LOAD_MOVIE) {
            g_FieldStateData.movieCommandState = MOVCMD_DONE;
        }
        if (g_FieldStateData.eventCmd == EVTCMD_PLAY_MOVIE) {
            g_FieldStateData.movieCommandState = MOVCMD_ACTIVE;
            func_800354CC();
            g_FieldMovieStreamActive = 1;
        }
        break;
    }
}

/////////////////////////////////////////////////
// Begin of field_rain.c
/////////////////////////////////////////////////

struct FieldRain {
    /* 0x00 */ SVECTOR p1;
    /* 0x08 */ SVECTOR p2;
    /* 0x10 */ s16 wait;
    /* 0x12 */ s16 rndSeed;
    /* 0x14 */ s16 z;
    /* 0x16 */ s16 render;
};

extern struct FieldRain g_FieldRain[64];
extern u8 g_RainForce;
extern s16 g_FieldRainDrops[0x40][12];

void FieldRainInit(struct FieldRenderData* renderData) {
    LINE_F2* line;
    s32 i;
    s32 adjustedIndex;

    for (i = 0; i < LEN(g_FieldRain); i++) {
        g_FieldRain[i].render = 0;
        g_FieldRain[i].rndSeed = i * 4;
        g_FieldRain[i].wait = i % 8;

        line = &renderData->Rain[i];

        SetLineF2(line);
        SetSemiTrans(line, 1);

        renderData->Rain[i].r0 = 0x10;
        renderData->Rain[i].g0 = 0x10;
        renderData->Rain[i].b0 = 0x10;
    }

    SetDrawMode(&renderData->RainDm, 0, 0, GetTPage(0, 1, 0, 0) & 0xffff, NULL);
}

void FieldRainAddToRender(
    u32* ot, LINE_F2* rain, MATRIX* matrix, DR_MODE* rainDm) {
    long p;
    long flag;
    s32 i;
    s32 j;

    PushMatrix();
    SetRotMatrix(matrix);
    SetTransMatrix(matrix);

    for (i = 0, j = 0; i < LEN(g_FieldRain); i++) {
        // 12 * sizeof(s16) = 24 bytes (0x18), the exact size of FieldRain
        if (g_FieldRainDrops[i][0] == 1) {
            RotTransPers(&g_FieldRain[i].p1, &rain->x0, &p, &flag);
            RotTransPers(&g_FieldRain[i].p2, &rain->x1, &p, &flag);
            AddPrim(ot, rain);
        }
        rain++;
    }

    PopMatrix();

    *(u32*)rainDm = (*(u32*)rainDm & 0xFF000000) | (*ot & 0xFFFFFF);

    *ot = (*ot & 0xFF000000) | ((u32)rainDm & 0xFFFFFF);
}

extern u8 g_RainControl;
extern s16 g_PlayerModelId;

extern u8 g_RandomTable[];
extern struct FieldRain g_FieldRain[];

/* Ramp the rain force towards 0 or 255 with the weather bit, then respawn any
 * drop whose wait has run out at a random offset around the player.
 *
 * The ceiling has to be a `u8` local (`u8 max = 255;`): as `s32` the constant
 * and the loaded `g_RainForce` swap $v0 and $v1 in the compare, which is four
 * rows. The parked body also carried a `g_FieldEntities[]` extern of its own,
 * which is not a symbol -- the array is `g_FieldEntity` -- so three rows were
 * checkfn refusing to alias `g_FieldEntities+0xc` onto `D_80074EB0`, and the
 * body would not have linked. Watch for that whenever a park's residue is a
 * handful of `%lo(sym+N)` rows against `D_` symbols: check the extern is the
 * real one before touching codegen. */
void FieldRainUpdate(void) {
    s32 i;
    s32 limit;
    s32 player;
    u8 max = 255;
    s32 vz;

    if ((g_RainControl & 0x80) == 0) {
        if (g_RainForce != 0) {
            g_RainForce--;
        }
    } else {
        if (g_RainForce != max) {
            g_RainForce++;
        }
    }

    limit = g_RainForce / 4;
    player = g_PlayerModelId;

    for (i = 0; i < 0x40; i++) {
        if (g_FieldRain[i].wait == 0) {
            if (i < limit) {

                u8 seed3;

                g_FieldRain[i].render = 1;
                g_FieldRain[i].rndSeed++;
                g_FieldRain[i].wait = 7;

                g_FieldRain[i].p2.vx =
                    (g_FieldEntity[player].PosX >> 12) +
                    g_RandomTable[g_FieldRain[i].rndSeed & 0xFF] * 12 - 0x600;

                seed3 = g_FieldRain[i].rndSeed * 3;
                g_FieldRain[i].p2.vy = (g_FieldEntity[player].PosY >> 12) +
                                       g_RandomTable[seed3] * 12 - 0x600;

                g_FieldRain[i].p1.vx = g_FieldRain[i].p2.vx;
                g_FieldRain[i].p1.vy = g_FieldRain[i].p2.vy;

                g_FieldRain[i].z = (g_FieldEntity[player].PosZ >> 12) - 0x300;
            } else {
                g_FieldRain[i].wait = 1;
                g_FieldRain[i].render = 0;
            }
        }

        g_FieldRain[i].p2.vz =
            g_FieldRain[i].z + (g_FieldRain[i].wait & 0x7) * 0x80;

        vz = (g_FieldRain[i].wait & 0x7) * 0x80;
        vz += 0x100;

        g_FieldRain[i].p1.vz = g_FieldRain[i].z + vz;

        g_FieldRain[i].wait--;
    }
}

/////////////////////////////////////////////////
// Begin of field_battle.c
/////////////////////////////////////////////////

u8 FieldGetRandomU8FromList(void) {
    g_FieldRandListIndex++;
    if (g_FieldRandListIndex == 0) {
        g_FieldRandListOffset += 13;
    }
    return g_RandomTable[g_FieldRandListIndex] - g_FieldRandListOffset;
}

u8 FieldGetNextRandomU8(void) {
    g_FieldRandomIndex++;
    return g_RandomTable[g_FieldRandomIndex];
}

extern s8 D_800716D0;
extern u16 D_8007173C;
extern s16 D_8007E774;
extern s8 D_8007EBC8;
extern s16 D_8009ABF6;
extern u8 D_8009AC30;
extern u8 D_8009C6D8;

/* Check for a random or scripted battle this frame: roll the encounter, pick
 * the battle from the field's encounter table, and kick off the transition if
 * one triggers.
 *
 * 83 rows / 4 inserted. The C is semantically right and the structure is the
 * target's; what is left is three groups, all measured:
 *
 *  - The roll's mask. The target emits `andi <t>,a0,0xff` on `roll` at every
 *    one of the four special-encounter compares; we emit it at compares 2 and
 *    3 (where cse shares one temp) and drop it at 1 and 4, because `roll`
 *    comes from `srl a0,v0,2` of a zero-extended byte and combine's
 *    `nonzero_bits` therefore proves the top 26 bits clear.  Four rows.
 *    Rejected, all measured identical: `s32 roll` (B), an explicit `u8 limit`
 *    local assigned from the sum before each compare (E), and writing the
 *    masks out as `(roll & 0xFF) < (u8)sum` (F).  Getting the mask back needs
 *    a spelling in which gcc cannot see the shift, and none of `u8`/`s32`/
 *    explicit-mask is one.
 *
 *  - The third callee-saved register.  The target holds `&D_8009ABF6` in `s2`
 *    from the `D_8009ABF6 != D_8007E774` test onward (`lui s2 / addiu s2 /
 *    lh v1,0(s2)`, then `sh v0,0(s2)` in the second loop); we emit the
 *    two-instruction `%hi`/`%lo` form at the load and a fresh `$at` expansion
 *    at the store, so the frame is 0x20 rather than 0x28 and `s2` is never
 *    saved.  cse follows the `beq` into the reroll loop in both builds and the
 *    label structure is identical, so the reason it relates the two references
 *    there and not here is not yet understood.  Six rows plus the four frame
 *    rows.
 *
 *  - `i` and `p` swap `a0`/`a1` in both loops, and the fallback load is read
 *    through `p` (`lhu v1,0xc(a0)`) rather than through `enc` (`lhu v0,
 *    0xc(s1)`).  Rejected: declaration order (`s32 i` before `u16* p`, J/K,
 *    inert here despite the same-type/same-live-range rule), `p` assigned as
 *    its own statement before the fallback store (G) and inside the `for`
 *    init after it (I) - identical.
 *
 * What did move it, and is in the body above:
 *  - `((u32)D_8007173C * D_80062F19) >> 12` rather than a signed product:
 *    gives the target's `srl`/`sltu` instead of `sra`/`slt`.
 *  - Splitting the special-encounter running total into `sum` / `total` /
 *    `rate` so the target's `s0` (running sum) and `v1` (the special[2]
 *    total, which special[3] adds to) are two variables rather than one.
 *  - Dropping a redundant `i = 0;` written before `FieldGetNextRandomU8()`:
 *    it made `i` live across the call, so `i` took a callee-saved register
 *    and pushed `enc` from `s1` to `s2`, which renamed roughly twenty rows.
 *    This is the same lever as the `KawaiFadeModelColor` note in CLAUDE.md,
 *    run in the other direction - there a dead store before a call is what
 *    *buys* the callee-saved register.
 */
/* Check for a random or scripted battle this frame. The field's encounter
 * table is one of two 0x18-byte blocks (D_8009AC30 picks the second), holding
 * four "special" formations with their own 6- or 5-bit rates, a fallback, and
 * five ordinary formations. The step counter advances by the player's move
 * speed over the table's divisor; when a roll against it succeeds, the four
 * specials are tried in order against a cumulative rate, and failing those the
 * five ordinary slots are walked twice -- once, and again if the first walk
 * picked the same formation as last time.
 *
 * 83 changed / 4 inserted -> 62 / 1, on one line: the two five-slot walks are
 * indexed off the counter, `((u16*)enc)[i + 1]`, not walked with a second
 * pointer. The pointer form makes `p` a biv beside `i`, and the two then trade
 * $a0 and $a1 -- with the pointer's initialising `move` scheduled next to the
 * counter's instead of after the fallback read, which is where the target puts
 * it. Neither the order of the two initialisers, nor the order of the two
 * increments, nor all four combinations moves it; only removing the second biv
 * does. Same lever as the `d[i * 8 + k]` bullet in CLAUDE.md, and worth 21
 * rows plus 3 of the 4 insertions.
 *
 * What is left is one register: the target keeps `&D_8009ABF6` in $s2 from the
 * `D_8009ABF6 != D_8007E774` test through the second walk -- `addiu s2,...` and
 * then `lh v1,0(s2)` / `sh v0,0(s2)` -- where this build fuses the `%lo` into
 * each access and rebuilds the address through the `$at` macro for the store.
 * That is the one insertion, and the frame is 4 bytes smaller for it, so every
 * saved-register offset and every branch displacement in the function reads
 * wrong: 62 rows from one allocation. The address is referenced on both sides
 * of a `FieldGetNextRandomU8` call, which is exactly the shape that should
 * give it a callee-saved register, so what is missing is whatever makes cse
 * relate the two references rather than fusing each one.
 *
 * Measured and inert, all 62/1: `roll` as s32, u32, u16 or s16 (the `(u8)`
 * casts at the comparisons carry the masking either way). `u8 sum` is 61/1 --
 * one row, and it makes the accumulator's width a semantic question for no
 * structural gain, so it is not taken.
 *
 * 62 rows / 1 insertion / -4 -> 40 / 0 / -2 on a named pointer local: `s16*
 * cur = &D_8009ABF6;` assigned immediately above the `!= D_8007E774` test,
 * with that test, its store and the *second* walk's fallback store reaching
 * the symbol through `*cur`. That is what makes cse relate the two references
 * instead of fusing each `%lo` into its own access, which puts the address in
 * a callee-saved register across the FieldGetNextRandomU8 call as the target
 * has it. Scope matters: routing the second walk's `slot` store through `cur`
 * as well is 42, and the first walk's stores must stay on the bare symbol.
 *
 * The two instructions it was still short were the `andi <r>,<r>,0xff` masking
 * `roll` at the first and fourth comparisons, and they are not reachable from
 * the comparison: `(u8)roll`, `(roll & 0xFF)`, every width of `roll` and both
 * operand orders all measure the same, because combine proves the high bits
 * clear from `roll = FieldGetNextRandomU8() >> 2;` -- a lbu-normalised call
 * result shifted right by two -- and folds the mask into the `sltu`. Splitting
 * the statement breaks the proof: `roll = FieldGetNextRandomU8(); roll >>= 2;`
 * stores the result into the `u8` pseudo first, so the shift reads a QImode
 * value whose bound gcc no longer carries past the block. 40 / -2 -> 13 and
 * the exact length. The `u8` and the split are one lever -- widening `roll`
 * after the split loses the masks again (s32/u32 52, u16 40).
 *
 * The last 13 were `slot` and `total` trading $a1 and $v1 across the third
 * comparison, and both steps out of it are decomp-permuter's, re-measured with
 * variant_eval rather than trusted from the score:
 *   - `total = roll;` between the fourth cumulative sum and its comparison,
 *     with the comparison reading `total`. 13 -> 6. Reusing *that* variable is
 *     the whole of it: a fresh `s32` local measures 13, `rate` 9, `formation`
 *     13 and `i` 30.
 *   - `D_8009ABF6 = (total = slot) & 0x3FF;` in the third comparison's arm.
 *     6 -> 0, found in 3,150 candidates from the 6-row body.
 * Both are dead assignments to `total` -- it is not read again on either path
 * -- so neither emits an instruction; what they change is `total`'s reference
 * count, which is the term `block_alloc` ranks on. This is CLAUDE.md's
 * assign-to-an-existing-local idiom twice over, and what natural C the original
 * wrote to get there is not recovered. */
void FieldBattleCheck(void) {
    FieldEncounterTable* enc;
    s16* cur;
    s32 i;
    s32 sum;
    s32 rate;
    s32 total;
    s32 formation;
    u8 roll;
    u16 control;
    u16 slot;

    if (D_8009AC30 == 0) {
        enc = (FieldEncounterTable*)g_FieldEncounters;
    } else {
        enc = (FieldEncounterTable*)(g_FieldEncounters + 0x18);
    }
    D_8009C6D8 += 0x20;
    if (D_8009C6D8 == 0) {
        func_800262D8();
        Savemap.memory_bank_4[6]++;
        if (Savemap.memory_bank_4[6] == 0 && Savemap.memory_bank_4[7] != 0xFF) {
            Savemap.memory_bank_4[7]++;
        }
        control = enc->control;
        if ((control & 1) && g_FieldMovieStreamActive == 0 && D_8009AC2F == 0) {
            D_8007173C += (s32)g_FieldEntity[g_PlayerModelId].MoveSpeed /
                          (s32)(control >> 8);
            if (FieldGetRandomU8FromList() < (D_80062F1B & 0x7F)) {
                D_800716D0 = 4;
            } else {
                D_800716D0 = 0;
            }
            if (FieldGetRandomU8FromList() < ((u32)D_8007173C * D_80062F19) >>
                12) {
                StopFieldMapPreload();
                D_8009ABF5 = 2;
                D_8007EBC8 = 1;
                roll = FieldGetNextRandomU8();
                roll >>= 2;
                if (!(D_80062F1B & 0x80)) {
                    sum = (s32)(enc->special[0] << 16) >> 26;
                } else {
                    sum = (s32)(enc->special[0] << 16) >> 27;
                }
                if ((u8)roll < (u8)sum) {
                    D_800716D0 = 0;
                    formation = enc->special[0] & 0x3FF;
                    goto found;
                }
                if (!(D_80062F1B & 0x80)) {
                    rate = (s32)(enc->special[1] << 16) >> 26;
                } else {
                    rate = (s32)(enc->special[1] << 16) >> 27;
                }
                sum += rate;
                if ((u8)roll < (u8)sum) {
                    D_800716D0 = 0;
                    formation = enc->special[1] & 0x3FF;
                    goto found;
                }
                slot = enc->special[2];
                total = sum + ((s32)(slot << 16) >> 26);
                if ((u8)roll < (u8)total) {
                    D_8009ABF6 = (total = slot) & 0x3FF;
                    return;
                }
                if (!(D_80062F1B & 0x80)) {
                    rate = (s32)(enc->special[3] << 16) >> 26;
                } else {
                    rate = (s32)(enc->special[3] << 16) >> 27;
                }
                sum = total + rate;
                total = roll;
                if ((u8)total < (u8)sum) {
                    formation = enc->special[3] & 0x3FF;
                found:
                    D_8009ABF6 = formation;
                    return;
                }
                sum = 0;
                roll = FieldGetNextRandomU8();
                roll >>= 2;
                D_8009ABF6 = enc->fallback & 0x3FF;
                for (i = 0; i < 5; i++) {
                    slot = ((u16*)enc)[i + 1];
                    sum += (s32)(slot << 16) >> 26;
                    if ((u8)roll < (u8)sum) {
                        D_8009ABF6 = slot & 0x3FF;
                        break;
                    }
                }
                cur = &D_8009ABF6;
                if (*cur != D_8007E774) {
                    D_8007E774 = *cur;
                    return;
                }
                sum = 0;
                roll = FieldGetNextRandomU8();
                roll >>= 2;
                *cur = enc->fallback & 0x3FF;
                for (i = 0; i < 5; i++) {
                    slot = ((u16*)enc)[i + 1];
                    sum += (s32)(slot << 16) >> 26;
                    if ((u8)roll < (u8)sum) {
                        D_8009ABF6 = slot & 0x3FF;
                        D_8007E774 = slot & 0x3FF;
                        break;
                    }
                }
            }
        }
    }
}

/////////////////////////////////////////////////
// Begin of field_arrow.c
/////////////////////////////////////////////////

void FieldArrowsInit(SPRT_16* sprt, DR_MODE* dm) {
    s16 i;

    for (i = 0; i < 24; i++, sprt++) {
        SetSprt16(sprt);
        SetShadeTex(sprt, 1);
        SetSemiTrans(sprt, 0);
        sprt->r0 = 0x80;
        sprt->g0 = 0x80;
        sprt->b0 = 0x80;
        sprt->clut = GetClut(0x100, 0x1E9);
    }
    SetDrawMode(dm, 0, 1, GetTPage(0, 0, 0x3C0, 0x100), NULL);
}

extern u16 g_FieldArrowAnimTick;

/* Queue the field exit arrows: up to 12 gateway markers, then up to 12
 * "point" arrows the script can place, each projected through the current
 * camera matrix and linked into the render OT.
 *
 * Matching, and three things in here look wrong and are not. Do not tidy them.
 *
 * 1. `off = i * 0x10;` is used at five of the seven places that need the
 *    scaled index and NOT at the other two -- `pos.vy` and the `.v0` store
 *    still spell `i * 0x10` inline. That asymmetry is the match: the same
 *    packet reached two ways in one loop is a shape the originals use (see
 *    CLAUDE.md), and using `off` everywhere costs 43 rows, using it nowhere
 *    costs 2, and using it at these five costs nothing. Its declaration slot
 *    matters too -- between `pos` and `sz`.
 *
 * 2. `do { } while (0);` after the addPrim is worth 7 rows and its identity
 *    is not recoverable. What it buys is the basic-block boundary that
 *    expand_end_loop's exit CODE_LABEL puts *inside* the `if` body, before
 *    the join; `while (0) { }` is byte-identical, so whatever the original
 *    wrote there emitted loop notes. Everything natural that was tried is
 *    7 rows short: plain braces around the tail, an inverted guard with the
 *    label before `i++`, the barrier moved outside the `if`, the clut check
 *    written as a goto over its body, and moving the addPrim above the clut
 *    check or above the x0/y0 stores. The PSY-Q `addPrim` macro is a comma
 *    expression here and in the real SDK, so it is not a do-while wrapper in
 *    disguise. If someone works out what belongs here, this is the place.
 *
 * 3. The second walk's addPrim argument is `&((SPRT_16*)((s32)buf +
 * 0x40C0))[i]`
 *    -- an ARRAY_REF off a based pointer, not the integer sum
 *    `i * 0x10 + 0x40C0 + (s32)buf` the first walk's `&buf->Arrows[i]` would
 *    suggest. fold reassociates the integer form to `(buf + 0x40C0) + i * 0x10`
 *    and emits the two adds the other way round; an ARRAY_REF is not a
 *    PLUS_EXPR, so there is nothing to reassociate. Casts, parentheses and
 *    named locals were all measured against this and are inert or much worse.
 *
 * The `.u0` and `.v0` stores go through a cast to FieldRenderData at
 * `i * 0x10 + (s32)buf` while `x0`/`y0`/`clut` use the plain
 * `buf->Arrows[i + K]`. Both address the same byte; the first keeps the
 * scaled index as the base register with 0x400C / 0x40CC as the
 * displacement, the second computes `(i + 0xC) * 0x10`. The original uses
 * both forms in the same loop. */
void FieldArrowsAddToRender(
    struct FieldRenderData* buf, MATRIX* mtx, s32 markers) {
    SVECTOR pos;
    s32 off;
    s32 sz;
    s32 flag;
    s16 i;

    if ((g_FieldExitArrowState[0] == 1 && g_FieldAnimLock == 0) ||
        g_FieldExitArrowState[0] == 2) {
        i = 0;
        PushMatrix();
        SetRotMatrix(mtx);
        SetTransMatrix(mtx);
        do {
            if (*(u8*)(g_FieldTriggers + i + 0x218) == 1) {
                pos.vx = (*(s16*)(markers + i * 0x18) +
                          *(s16*)(markers + i * 0x18 + 6)) /
                         2;
                pos.vy = (*(s16*)(markers + i * 0x18 + 2) +
                          *(s16*)(markers + i * 0x18 + 8)) /
                         2;
                pos.vz = (*(s16*)(markers + i * 0x18 + 4) +
                          *(s16*)(markers + i * 0x18 + 0xA)) /
                         2;
                if (pos.vx != 0 || pos.vy != 0) {
                    RotTransPers(&pos, (s32*)&pos, &sz, &flag);
                    ((struct FieldRenderData*)(i * 0x10 + (s32)buf))
                        ->Arrows[0]
                        .u0 = (g_FieldArrowAnimTick * 4 & 0x30) + 0x30;
                    ((struct FieldRenderData*)(i * 0x10 + (s32)buf))
                        ->Arrows[0]
                        .v0 = 0xD0;
                    buf->Arrows[i].x0 = pos.vx - 7;
                    buf->Arrows[i].y0 = pos.vy - 8;
                    addPrim(buf->ot, &buf->Arrows[i]);
                }
            }
            i++;
        } while (i < 0xC);
        i = 0;
        do {
            off = i * 0x10;
            if (*(s32*)((u8*)g_FieldTriggers + off + 0x230) != 0) {
                pos.vx = *(u16*)((u8*)g_FieldTriggers + off + 0x224);
                pos.vy = *(u16*)((u8*)g_FieldTriggers + i * 0x10 + 0x228);
                pos.vz = *(u16*)((u8*)g_FieldTriggers + off + 0x22C);
                RotTransPers(&pos, (s32*)&pos, &sz, &flag);
                ((struct FieldRenderData*)(off + (s32)buf))->Arrows[0xC].u0 =
                    (g_FieldArrowAnimTick * 4 & 0x30) + 0x30;
                ((struct FieldRenderData*)(i * 0x10 + (s32)buf))
                    ->Arrows[0xC]
                    .v0 = 0xD0;
                buf->Arrows[i + 0xC].x0 = pos.vx - 7;
                buf->Arrows[i + 0xC].y0 = pos.vy - 8;
                if (*(s32*)((u8*)g_FieldTriggers + off + 0x230) == 2) {
                    buf->Arrows[i + 0xC].clut = GetClut(0x100, 0x1E8);
                }
                addPrim(buf->ot, &((SPRT_16*)((s32)buf + 0x40C0))[i]);
                do {
                } while (0);
            }
            i++;
        } while (i < 0xC);
        PopMatrix();
        addPrim(buf->ot, &buf->ArrowsDm);
        g_FieldArrowAnimTick++;
    }
}

/////////////////////////////////////////////////
// Begin of field_model.c
/////////////////////////////////////////////////

/* One texture page inside a BSX model file: where it lives in VRAM and where
 * its pixels sit relative to the start of the file. */
typedef struct {
    /* 0x0 */ u16 w;
    /* 0x2 */ u16 h;
    /* 0x4 */ u16 x;
    /* 0x6 */ u16 y;
    /* 0x8 */ u32 dataOffset;
} BsxTexEntry; // size:0xC

typedef struct {
    /* 0x0 */ u32 unk0;
    /* 0x4 */ u8 texCount;
    /* 0x5 */ u8 tdbOffsetHi;  // 24-bit offset of the TDB chunk, big-endian:
    /* 0x6 */ u16 tdbOffsetLo; // (hi << 16) | lo, zero when there is none
    /* 0x8 */ BsxTexEntry entries[1];
} BsxTexHeader;

/* One model's record inside a BSX model file. The bone, part and animation
 * blocks all live at dataOffset, back to back, and each says where in the
 * destination model it belongs; the four colour groups are the KAWAI lighting
 * the field hands to KawaiLightingApplyToModel -- three directional lights and
 * an ambient one. */
typedef struct {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u16 scale;
    /* 0x04 */ u32 dataOffset; // bone/part/anim data, relative to this record
    /* 0x08 */ u8 light0[3];
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u16 light0Dir[3];
    /* 0x12 */ s8 boneIndex;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 light1[3];
    /* 0x17 */ u8 boneCount;
    /* 0x18 */ u16 light1Dir[3];
    /* 0x1E */ s8 partIndex;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ u8 light2[3];
    /* 0x23 */ u8 partCount;
    /* 0x24 */ u16 light2Dir[3];
    /* 0x2A */ s8 animIndex;
    /* 0x2B */ u8 unk2B;
    /* 0x2C */ u8 ambient[3];
    /* 0x2F */ u8 animCount;
} BsxModelRecord; // size:0x30

/* The model block of a BSX file: one record per model, the texture header, and
 * the offset of the scratch copy of the records the field keeps live. */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ u32 modelCount;
    /* 0x08 */ u32 texOffset;
    /* 0x0C */ u32 recordsOffset;
    /* 0x10 */ BsxModelRecord models[1];
} BsxModelBlock;

extern u8* D_800E0200;

void FieldModelBsxTdbModify(u8* tdb);
void FieldModelLoadBsxTexToVram(BsxTexHeader* bsx);
u8* FieldModelCreatePktsAndScale(FieldModelEntry* model, u8* pkts, s32 arg2);
void KawaiLightingApplyToModel(FieldModelEntry* model, u8* light);
void KawaiSetColorToModelPkts(FieldModelEntry* model, u8* color);

/* Load the field map's own model file and bring every model in it up: either
 * stream it off the CD or copy the block already in memory down to D_800E0204,
 * apply its texture delta and push its textures to VRAM, then per model splice
 * the bone, part and animation records into the model entry -- relocating the
 * pointer each part and animation carries -- and finally build its packets,
 * load its face texture, pose it and apply its KAWAI lighting and colour.
 * Returns the scratch copy of the model records, which is also where the next
 * allocation starts.
 *
 * 103 rows out with 7 insertions, and every one of them is downstream of a
 * single allocator decision: the target spills `models` (0x38) and `records`
 * (0x40) to the stack and gives `pkts` the frame-pointer register, where this C
 * keeps `models` and `records` in s-registers and spills `pkts` instead. The
 * frame is the same 0xa8, the same nine registers are saved, every loop has the
 * same shape and the same induction variables -- what differs is which value
 * lost, and that renames most of the s-registers from the first loop onward.
 *
 * Measured on the way here, all against this same body: hoisting the model
 * entry, the three record counts and `words / 4` into locals is worth 58 rows
 * and 29 insertions (gcc reloads a count and re-derives the entry after every
 * store otherwise, since a store through s32* may alias them); giving the
 * 0x30-record copy its own pair of pointers rather than reusing the word
 * copy's is worth 20; taking the address of D_800DF114 into a local before the
 * third loop is worth 19, because loop.c will not hoist an address whose only
 * uses are inside a conditional arm -- the same rule as the constant hoist in
 * HandleKawaiDataInModel. Walking a record pointer through the lighting block
 * instead of indexing `records[i]` costs 30 rows, and computing `models`
 * before the scratch pointer costs 5.
 * **110 rows / -2 instructions -> 46 / -1**, and the note's standing
 * diagnosis turned out to be right *and* actionable. It said the residue was
 * "one allocator decision: the target spills `models` (0x38) and `records`
 * (0x40) to the stack and gives `pkts` the frame-pointer register". It does,
 * and there is a way to ask for it.
 *
 *   - **Take the address of `models` and never dereference it.**
 *     `FieldModelLoaderData** pmodels; ... pmodels = &models;` sets
 *     TREE_ADDRESSABLE, which sends the variable through `put_var_into_stack`
 *     and puts it exactly where the target reads it from, `0x38(sp)`. The
 *     store to `pmodels` is dead and costs nothing. **76 rows -> 46.** This is
 *     the same idiom `FieldBackgroundInitPackets` uses for a counter, applied
 *     to a pointer, and it is the only way to spell "keep this in memory".
 *   - **The 0x30-record copy is indexed off the counter, not walked.**
 *     `dm[i * 12 + k] = sm[i * 12 + k]` rather than twelve stores followed by
 *     `sm += 12; dm += 12;`. The walked form makes both pointers bivs and gcc
 *     reduces the element addresses onto a second base -- CLAUDE.md's
 *     `FieldModelLoadBcx` bullet, and worth **110 rows -> 76** here.
 *
 * The second loop goes the *other* way and that is measured, not assumed: the
 * target walks `t5` by 8 and `t6` by 0x30 through it, which reads like an
 * invitation to write `mw++`/`rw++`, and doing so costs rows every way round
 * -- models alone 84, records alone 78, both 111, against 46. Read the target
 * for which form a loop wants and then check it; the two loops in this
 * function want opposite ones.
 *
 * Also measured and rejected: swapping the operands of `words = (buf[0] >> 2)
 * + ((buf[0] & 3) != 0)` is exactly inert (fold canonicalises the sum), and
 * splitting it into two statements is 95. Computing `models` before `scratch`
 * is 115.
 *
 * The one instruction and 46 rows left are two clusters. The target evaluates
 * that sum's `srl` before its `andi` where this body does the mask first --
 * neither spelling above reaches it. And the second loop's index arithmetic
 * lands in different registers, which is downstream of the same choice.

 * 46 -> 37 on a dead conditional at the bottom of the parts loop:
 * `if (!models && !models) { }`. `models` is a pointer that is provably
 * non-null there and the test is pure, so jump_optimize and flow delete all
 * of it -- the length is unchanged at -1 -- and what survives is the basic
 * block it ends, which is a scheduling boundary inside the unrolled copy.
 * decomp-permuter's `perm_ins_block`, re-measured with variant_eval; its own
 * score moved 1055 -> 580 for it, which on this function means nothing on its
 * own (see the note below).
 *
 * Also measured on the way and all exactly 46: `words = ((buf[0] & 3) != 0) +
 * (buf[0] >> 2)` (the target shifts first and we mask first, and neither
 * operand order nor splitting the statement reaches it -- the split is 66),
 * the `s`/`d` copy pointers in either declaration or assignment order, and
 * both together. Walking `models` in the third loop rather than indexing it
 * is 48 and turns -1 into +1, so the spilled pointer the target reloads at
 * 0x38(sp) is not simply `models` advanced.

 * 37 -> 32 on a second permuter find, same class as the first:
 * `words = 3;` immediately above the statement, with the mask reading the
 * local. The store is dead -- `words` is overwritten by the very next
 * statement -- so it emits nothing and only raises the local's reference
 * count, which is what the target's register assignment there needs. The
 * declaration reorder the same candidate carried (`pmodels` after `block`) is
 * exactly inert.
 *
 * Run rating, for the next pass: base 580 -> best 355 over 98,144 candidates
 * on ten workers, worth five real rows. That is a *tenth* of the yield of the
 * FieldBattleCheck run that matched in 3,150. Both finds here are
 * perm_temp_for_expr on an existing local, so weight that pass and stop early
 * if nothing appears in the first few thousand.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field2", LoadLocalFieldModelAndInitAll);
#else
u8* LoadLocalFieldModelAndInitAll(
    FieldModelFileDesc* desc, FieldModelData* data, u8* readFromCd, u32* buf) {
    MATRIX mtx;
    RECT rect;
    FieldModelLoaderData* models;
    FieldModelLoaderData** pmodels;
    BsxModelBlock* block;
    BsxModelRecord* rec;
    BsxModelRecord* records;
    BsxTexHeader* bsx;
    u32* fileInfo;
    u32* s;
    u32* d;
    u32* sm;
    u32* dm;
    s32* src;
    s32* bones;
    s32* parts;
    s32* anims;
    u8* scratch;
    u8* pkts;
    u8* flip;
    FieldModelEntry* entry;
    s32 fixup;
    s32 words;
    s32 w;
    s32 n;
    u32 count;
    u32 i;
    u32 j;
    s32 modelIndex;

    scratch = (u8*)0x1F800000;
    models = desc->models;
    pmodels = &models;
    fileInfo = *(u32**)scratch;
    if (*readFromCd != 0) {
        DS_read(fileInfo[0], fileInfo[1], buf, NULL);
        while (SystemCdromReadChain() != 0) {
        }
    } else {
        s = buf;
        d = (u32*)D_800E0204;
        words = 3;
        words = (buf[0] >> 2) + ((buf[0] & words) != 0);
        n = words / 4;
        for (w = 0; w < n; w++) {
            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];
            d[3] = s[3];
            s += 4;
            d += 4;
        }
        for (w = n * 4; w < words; w++) {
            *d++ = *s++;
        }
        buf = (u32*)D_800E0204;
    }

    block = (BsxModelBlock*)((u8*)buf + buf[1]);
    bsx = (BsxTexHeader*)((u8*)block + block->texOffset);
    if (*(u32*)&bsx->texCount & ~0xFF) {
        FieldModelBsxTdbModify(
            (u8*)bsx + ((bsx->tdbOffsetHi << 16) | bsx->tdbOffsetLo));
    }
    FieldModelLoadBsxTexToVram(bsx);
    DrawSync(0);

    count = block->modelCount;
    fixup = (s32)buf - 0x80000000;
    for (i = 0; i < count; i++) {
        if (models[i].npcFlag != 0) {
            rec = &block->models[i];
            entry = &data->modelEntries[models[i].modelEntryIndex];
            entry->scale = rec->scale;
            bones = (s32*)entry->modelData;
            src = (s32*)((u8*)rec + rec->dataOffset);
            n = rec->boneCount;
            for (j = 0; j < n; j++) {
                bones[rec->boneIndex + j] = *src++;
            }
            parts = (s32*)(entry->modelData + entry->partsOffset);
            n = rec->partCount;
            for (j = 0; j < n; j++) {
                parts[(rec->partIndex + j) * 8 + 0] = src[0];
                parts[(rec->partIndex + j) * 8 + 1] = src[1];
                parts[(rec->partIndex + j) * 8 + 2] = src[2];
                parts[(rec->partIndex + j) * 8 + 3] = src[3];
                parts[(rec->partIndex + j) * 8 + 4] = src[4];
                parts[(rec->partIndex + j) * 8 + 5] = src[5];
                parts[(rec->partIndex + j) * 8 + 6] = src[6];
                parts[(rec->partIndex + j) * 8 + 7] = src[7];
                parts[(rec->partIndex + j) * 8 + 6] = src[6] + fixup;
                src += 8;
                if (!models && !models) {
                }
            }
            anims = (s32*)(entry->modelData + entry->animationOffset);
            n = rec->animCount;
            for (j = 0; j < n; j++) {
                anims[(rec->animIndex + j) * 4 + 0] = src[0];
                anims[(rec->animIndex + j) * 4 + 1] = src[1];
                anims[(rec->animIndex + j) * 4 + 2] = src[2];
                anims[(rec->animIndex + j) * 4 + 3] = src[3];
                anims[(rec->animIndex + j) * 4 + 3] = src[3] + fixup;
                src += 4;
            }
        }
    }

    records = (BsxModelRecord*)((u8*)block + block->recordsOffset);
    dm = (u32*)records;
    sm = (u32*)block->models;
    for (i = 0; i < count; i++) {
        dm[i * 12 + 0] = sm[i * 12 + 0];
        dm[i * 12 + 1] = sm[i * 12 + 1];
        dm[i * 12 + 2] = sm[i * 12 + 2];
        dm[i * 12 + 3] = sm[i * 12 + 3];
        dm[i * 12 + 4] = sm[i * 12 + 4];
        dm[i * 12 + 5] = sm[i * 12 + 5];
        dm[i * 12 + 6] = sm[i * 12 + 6];
        dm[i * 12 + 7] = sm[i * 12 + 7];
        dm[i * 12 + 8] = sm[i * 12 + 8];
        dm[i * 12 + 9] = sm[i * 12 + 9];
        dm[i * 12 + 10] = sm[i * 12 + 10];
        dm[i * 12 + 11] = sm[i * 12 + 11];
    }

    pkts = (u8*)block;
    flip = &D_800DF114;
    for (i = 0; i < count; i++) {
        if (models[i].npcFlag != 0) {
            modelIndex = models[i].modelEntryIndex;
            pkts = FieldModelCreatePktsAndScale(
                &data->modelEntries[modelIndex], pkts, modelIndex);
            if (data->modelEntries[modelIndex].textureFaceId < 0x21) {
                rect.x = 0x140;
                rect.y = modelIndex + 0x1E0;
                rect.w = 0x10;
                rect.h = 1;
                LoadImage(
                    &rect,
                    (u_long*)((u8*)D_800DFCA0 + D_800DFCA0->clutOffset +
                              (data->modelEntries[modelIndex].textureFaceId
                               << 5)));
                scratch[0] = 0;
                scratch[1] = 0;
                scratch[2] = 0;
                scratch[3] = modelIndex;
                KawaiLoadEyesMouthTexToVram(
                    &data->modelEntries[modelIndex], scratch);
            }
            mtx.m[0][0] = mtx.m[1][1] = mtx.m[2][2] = 0x1000;
            mtx.t[0] = mtx.t[1] = mtx.t[2] = 0;
            mtx.m[0][1] = mtx.m[0][2] = mtx.m[1][0] = mtx.m[1][2] =
                mtx.m[2][0] = mtx.m[2][1] = 0;
            *(s32*)0x1F800000 = 1;
            FieldModelAnimCalcMtrxs(
                &data->modelEntries[modelIndex], &mtx, 0, 0);
            scratch[0] = records[i].light0[0];
            scratch[1] = records[i].light0[1];
            scratch[2] = records[i].light0[2];
            scratch[3] = records[i].light1[0];
            scratch[4] = records[i].light1[1];
            scratch[5] = records[i].light1[2];
            scratch[0xC] = records[i].light0Dir[0];
            scratch[0xD] = records[i].light0Dir[0] >> 8;
            scratch[0xE] = records[i].light0Dir[1];
            scratch[0xF] = records[i].light0Dir[1] >> 8;
            scratch[0x10] = records[i].light0Dir[2];
            scratch[0x11] = records[i].light0Dir[2] >> 8;
            scratch[6] = records[i].light2[0];
            scratch[7] = records[i].light2[1];
            scratch[8] = records[i].light2[2];
            scratch[0x12] = records[i].light1Dir[0];
            scratch[0x13] = records[i].light1Dir[0] >> 8;
            scratch[0x14] = records[i].light1Dir[1];
            scratch[0x15] = records[i].light1Dir[1] >> 8;
            scratch[0x16] = records[i].light1Dir[2];
            scratch[0x17] = records[i].light1Dir[2] >> 8;
            scratch[9] = records[i].ambient[0];
            scratch[0xA] = records[i].ambient[1];
            scratch[0xB] = records[i].ambient[2];
            scratch[0x18] = records[i].light2Dir[0];
            scratch[0x19] = records[i].light2Dir[0] >> 8;
            scratch[0x1A] = records[i].light2Dir[1];
            scratch[0x1B] = records[i].light2Dir[1] >> 8;
            scratch[0x1C] = records[i].light2Dir[2];
            scratch[0x1D] = records[i].light2Dir[2] >> 8;
            scratch[0x1E] = 0;
            KawaiLightingApplyToModel(&data->modelEntries[modelIndex], scratch);
            scratch[0] = 0;
            scratch[1] = 0;
            scratch[2] = 0;
            scratch[3] = 0;
            scratch[4] = 0;
            scratch[5] = 0;
            scratch[6] = 1;
            KawaiSetColorToModelPkts(&data->modelEntries[modelIndex], scratch);
            scratch[0] = 0;
            scratch[1] = 0;
            scratch[2] = 0;
            scratch[3] = 0;
            scratch[4] = 0;
            scratch[5] = 0;
            scratch[6] = 1;
            *flip ^= 1;
            KawaiSetColorToModelPkts(&data->modelEntries[modelIndex], scratch);
            *flip ^= 1;
        }
    }

    D_800E0200 = (u8*)records;
    return (u8*)records;
}
#endif

/* One part of a BSX model: the eight primitive-kind counts, then the offsets
 * of the four tables FieldModelCreatePktsForPart walks. */
typedef struct {
    /* 0x00 */ u8 unk0[4];
    /* 0x04 */ u8 gt4Count;
    /* 0x05 */ u8 gt3Count;
    /* 0x06 */ u8 ft4Count;
    /* 0x07 */ u8 ft3Count;
    /* 0x08 */ u8 f3Count;
    /* 0x09 */ u8 f4Count;
    /* 0x0A */ u8 g3Count;
    /* 0x0B */ u8 g4Count;
    /* 0x0C */ u8 unkC[2];
    /* 0x0E */ u16 polyOffset;     // the interleaved polygon table
    /* 0x10 */ u16 texCoordOffset; // u16 pairs, one per vertex
    /* 0x12 */ u16 texInfoOffset;  // u32 per texture slot: clut, tpage, mode
    /* 0x14 */ u16 texIndexOffset; // one byte per textured primitive
    /* 0x16 */ u16 pktSize;        // bytes of packets one pass emits
    /* 0x18 */ u8* data;
    /* 0x1C */ u8* pkts;
} FieldModelPart; // size:0x20

extern u8* FieldModelCreatePktsForPart(
    FieldModelPart* part, u8* pkts, s32 reset, s32 texY);
extern void FieldModelScaleModel(FieldModelEntry* model, s16 scale, s32 arg2);

/* Reserves one 32-byte matrix slot per bone at the head of the packet buffer,
 * then emits the drawing packets for every part behind them. */
u8* FieldModelCreatePktsAndScale(FieldModelEntry* model, u8* pkts, s32 arg2) {
    u8* parts;
    u32 i;

    model->partMatrices = pkts;
    pkts += model->boneCount * 32;
    parts = model->modelData + model->partsOffset;
    for (i = 0; i < model->partCount; i++) {
        pkts = FieldModelCreatePktsForPart(
            (FieldModelPart*)&parts[i * 32], pkts, 0, arg2);
    }
    FieldModelScaleModel(model, model->scale, 0);
    return pkts;
}

/* Builds the drawing packets for one model part, twice -- the renderer
 * double-buffers them. Eight primitive kinds are emitted in a fixed order,
 * each from its own run of the part's interleaved polygon table; the four
 * textured kinds also consume one byte of the texture-index table and one
 * u32 of the texture-info table per primitive. `texY` shifts every UV by the
 * position of the part's texture inside the shared page.
 */
/* 218 changed / 29 inserted at exactly 727 instructions, from 412/60 and 28
 * instructions long as a raw m2c seed. Everything below is measured; the
 * length is exact, so what is left is allocation and scheduling.
 *
 * The shape, which the seed does not show: eight primitive kinds emitted in
 * a fixed order (GT4, GT3, FT4, FT3, F3, F4, G3, G4 -- codes 0x3C, 0x34,
 * 0x2C, 0x24, 0x20, 0x28, 0x30, 0x38), each from its own run of one
 * interleaved polygon table walked by a single cursor, and the whole thing
 * run twice because the renderer double-buffers the packets. Every `unkN`
 * in the seed resolves against the PSY-Q layouts: `unk-4` is the tag's `len`
 * byte, `unk0` its `code`, `unk-3` the r0/g0/b0/code word, and the rest are
 * the u/v pairs and the clut and tpage halfwords. m2c's `var_s4 = var_s3 +
 * 0x14` walking with negative displacements is one biv plus a combined giv,
 * so the source has one cursor.
 *
 * Seven levers, in the order they were worth:
 *
 * 1. **No hand-written zero-trip guard.** `if (count != 0) { for (i = 0; i <
 *    count; i++) ... }` tests twice -- gcc does not fold the two -- and the
 *    bare `for` compiles to exactly the `if`/`do`/`while` m2c prints. Worth
 *    **16 instructions**, two per loop across all eight.
 *
 * 2. **The graph-type test is `!= 1 && !=  2` with the arms swapped.** This
 *    is the whole of the remaining length: every other spelling is +4, one
 *    instruction per textured loop. `== 1 || == 2` with the arms in the
 *    target's order is 239/33 at 731, and so is a `goto` chain that
 *    reproduces the target's CFG literally (`if (gt == 1) goto big; if (gt
 *    != 2) goto small; big: ... goto joined; small: ...`) -- which is worth
 *    recording, because that chain gives the target's branch polarity and
 *    block order and is still two instructions long per loop. What the
 *    inverted form buys is `t & 0xC0` computed twice rather than three
 *    times (the two copies land in the two branch delay slots and both arms
 *    read them) and `((t >> 4) & 0x100) >> 4` surviving as three
 *    instructions rather than folding to `srl 8`/`andi 0x10`. Neither is
 *    reachable any other way: all six spellings of that shift measure
 *    identically (`(u16)` and `(s32)` casts, `/ 16`, `& 0x1000 >> 8`, a
 *    named intermediate, and splitting it into two `>> 2`), and hoisting
 *    `t & 0xC0` into a local ahead of the `if` is **+8** because the local
 *    is then live across both calls and spills.
 *
 * 3. **The `!= 2` test needs its own local.** Written inline, `(((t & 0x3F)
 *    != 2) ? texY : 0)` -- and `texY & -((t & 0x3F) != 2)`, which fold turns
 *    back into the same COND_EXPR -- reaches `expand_expr` under the `+` and
 *    comes out as a branch around the addition. Assigned first, `shift = (t
 *    & 0x3F) != 2;` is a statement of its own, `do_store_flag` runs, and the
 *    target's `andi`/`xori`/`sltu`/`negu`/`and` appears. Five instructions
 *    per textured loop.
 *
 * 4. **Every base+offset is written offset-first.** `(u8*)(part->
 *    texIndexOffset + (s32)data)` rather than `data + part->texIndexOffset`:
 *    fold canonicalises a pointer PLUS so the pointer is op0, and casting it
 *    to `s32` makes it an ordinary integer PLUS that keeps source order.
 *    That is what decides which of the two `lhu`/`lw` goes first and the
 *    `addu`'s operand order, at four sites here.
 *
 * 5. **Declaration order decides the spill-slot order.** `expand_decl`
 *    creates a pseudo per local at the top of the function in declaration
 *    order, so the slots follow it: the target's are `pass` 0x28, `uOff0`
 *    0x30, `vOff0` 0x38, `uOff1` 0x40, `vOff1` 0x48, `texInfo` 0x50, and
 *    declaring them in that order lands every one. Note this is the *only*
 *    thing declaration order does here -- moving `f` to the end of the list,
 *    or the four pointers to the front, is exactly inert, which is what
 *    CLAUDE.md's rule predicts.
 *
 * 6. **The pointer bumps belong in the `for` increment.** `for (i = 0; i <
 *    count; i++, out += sizeof(POLY_GT4), poly += 6)` emits `i++` first and
 *    the two givs behind it, which is the target's order; as body statements
 *    they come out ahead of `i++`.
 *
 * 7. **`f` is `u8`.** `u32` and `s32` are both 232/31 at +2 -- the extra is
 *    the `lbu`'s zero-extension no longer folding into the three masks.
 *
 * Also measured and rejected: `r = texY - q * 4;` as a named remainder, to
 * get the target's `q * 4` computed ahead of `q * 32` (263/30 at +1);
 * computing `poly` before the `if (pass != 0)` rather than after, to stop
 * `part->data` being reloaded at the join (225/29).
 *
 * The arm-swap and flag-local dimensions are now closed *exhaustively*, not
 * sampled. Both were written as `PERM_GENERAL` macros at all four textured
 * loops and handed to decomp-permuter in enumerate mode: 256 candidates,
 * every one compiled, forty seconds, and the search terminates by itself
 * rather than walking forever. Re-measured with `checkfn.py`, every single
 * combination other than the one here is longer -- each of the four
 * single-site arm swaps is exactly +1 instruction (243/30, 237/30, 233/30,
 * 227/30 at 728) and the ternary spelling of the clut is inert at every
 * site. There is no asymmetry to find: all four loops want the same form.
 *
 * That run is also the clearest evidence for the rule CLAUDE.md now records
 * about this project's permuter scores. The scorer ranked the candidates
 * monotonically by how many sites were in the *longer* form -- 5570 for all
 * four swapped, 5685 for three, 5800 for two, against 6030 for the body
 * here, which is the shortest and the only one at 727. The score improves as
 * the code gets further away.
 *
 * What is left is three clusters. A rotation of `$s3`/`$s4`/`$s5`/`$s6` --
 * the target ranks `f` last of the four and we rank it first, with `poly`,
 * its `+0x14` giv and `out` all shifted one place -- which is
 * `allocno_compare` and neither `f`'s type nor any declaration order moves
 * it. The u2/u3 texture-coordinate lookups, where the target computes the
 * second address into a second register so the first load issues in its
 * shadow and we serialise them. And `q * 4` being computed after `q * 32`
 * rather than before, at both of the two divisions.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field2", FieldModelCreatePktsForPart);
#else
u8* FieldModelCreatePktsForPart(
    FieldModelPart* part, u8* pkts, s32 reset, s32 texY) {
    u32 pass;
    s32 uOff0;
    s32 vOff0;
    s32 uOff1;
    s32 vOff1;
    u32* texInfo;
    u8* uv;
    u8* data;
    u8* texIdx;
    u8* out;
    u32* poly;
    s32 q;
    u32 i;
    u32 count;
    u8 f;
    u32 t;
    u32 v;
    s32 n;
    s32 du;
    s32 dv;
    s32 shift;
    s32 tp;
    s32 tpBit;

    texInfo = (u32*)(part->texInfoOffset + (s32)part->data);
    uv = (u8*)(part->texCoordOffset + (s32)part->data);
    if (reset != 0) {
        part->data = (u8*)part + 0x20;
    }
    part->pkts = pkts;

    q = texY / 4;
    vOff0 = q * 32;
    uOff0 = (texY - q * 4) * 64;
    q = texY / 8;
    vOff1 = q * 32;
    uOff1 = (texY - q * 8) * 32;

    for (pass = 0; pass < 2; pass++) {
        out = pkts;
        data = part->data;
        texIdx = (u8*)(part->texIndexOffset + (s32)data);
        if (pass != 0) {
            out += part->pktSize;
        }
        poly = (u32*)(part->polyOffset + (s32)data);

        count = part->gt4Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_GT4), poly += 6) {
            POLY_GT4* p = (POLY_GT4*)out;

            *(u32*)&p->r0 = poly[1];
            *(u32*)&p->r1 = poly[2];
            *(u32*)&p->r2 = poly[3];
            *(u32*)&p->r3 = poly[4];
            v = poly[5];
            *(u16*)&p->u0 = *(u16*)((v & 0xFF) * 2 + (s32)uv);
            *(u16*)&p->u1 = *(u16*)(((v & 0xFF00) >> 7) + (s32)uv);
            *(u16*)&p->u2 = *(u16*)(((v >> 15) & 0x1FE) + (s32)uv);
            *(u16*)&p->u3 = *(u16*)((v >> 24) * 2 + (s32)uv);
            f = *texIdx++;
            t = texInfo[f & 0xF];
            shift = (t & 0x3F) != 2;
            p->clut =
                ((((t * 2) >> 23) + (texY & -shift)) << 6) | ((t >> 16) & 0x3F);
            if (GetGraphType() != 1 && GetGraphType() != 2) {
                tp = ((t & 0xC0) * 2) | (f & 0x60);
                tpBit = ((t >> 4) & 0x100) >> 4;
            } else {
                tp = ((t & 0xC0) * 8) | ((f * 4) & 0x180);
                tpBit = (t >> 7) & 0x20;
            }
            p->tpage = tp | tpBit | ((t & 0xF00) >> 8);
            n = t & 0x3F;
            if (n == 0) {
                du = uOff0;
                dv = vOff0;
            } else {
                dv = 0;
                if (n == 1) {
                    du = uOff1;
                    dv = vOff1;
                } else {
                    du = 0;
                }
            }
            setPolyGT4(p);
            p->u0 += du;
            p->v0 += dv;
            p->u1 += du;
            p->v1 += dv;
            p->u2 += du;
            p->v2 += dv;
            p->u3 += du;
            p->v3 += dv;
            if (f & 0x10) {
                setcode(p, 0x3E);
            }
        }

        count = part->gt3Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_GT3), poly += 5) {
            POLY_GT3* p = (POLY_GT3*)out;

            *(u32*)&p->r0 = poly[1];
            *(u32*)&p->r1 = poly[2];
            *(u32*)&p->r2 = poly[3];
            v = poly[4];
            *(u16*)&p->u0 = *(u16*)((v & 0xFF) * 2 + (s32)uv);
            *(u16*)&p->u1 = *(u16*)(((v & 0xFF00) >> 7) + (s32)uv);
            *(u16*)&p->u2 = *(u16*)(((v >> 15) & 0x1FE) + (s32)uv);
            f = *texIdx++;
            t = texInfo[f & 0xF];
            shift = (t & 0x3F) != 2;
            p->clut =
                ((((t * 2) >> 23) + (texY & -shift)) << 6) | ((t >> 16) & 0x3F);
            if (GetGraphType() != 1 && GetGraphType() != 2) {
                tp = ((t & 0xC0) * 2) | (f & 0x60);
                tpBit = ((t >> 4) & 0x100) >> 4;
            } else {
                tp = ((t & 0xC0) * 8) | ((f * 4) & 0x180);
                tpBit = (t >> 7) & 0x20;
            }
            p->tpage = tp | tpBit | ((t & 0xF00) >> 8);
            n = t & 0x3F;
            if (n == 0) {
                du = uOff0;
                dv = vOff0;
            } else {
                dv = 0;
                if (n == 1) {
                    du = uOff1;
                    dv = vOff1;
                } else {
                    du = 0;
                }
            }
            setPolyGT3(p);
            p->u0 += du;
            p->v0 += dv;
            p->u1 += du;
            p->v1 += dv;
            p->u2 += du;
            p->v2 += dv;
            if (f & 0x10) {
                setcode(p, 0x36);
            }
        }

        count = part->ft4Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_FT4), poly += 3) {
            POLY_FT4* p = (POLY_FT4*)out;

            *(u32*)&p->r0 = poly[1];
            v = poly[2];
            *(u16*)&p->u0 = *(u16*)((v & 0xFF) * 2 + (s32)uv);
            *(u16*)&p->u1 = *(u16*)(((v & 0xFF00) >> 7) + (s32)uv);
            *(u16*)&p->u2 = *(u16*)(((v >> 15) & 0x1FE) + (s32)uv);
            *(u16*)&p->u3 = *(u16*)((v >> 24) * 2 + (s32)uv);
            f = *texIdx++;
            t = texInfo[f & 0xF];
            shift = (t & 0x3F) != 2;
            p->clut =
                ((((t * 2) >> 23) + (texY & -shift)) << 6) | ((t >> 16) & 0x3F);
            if (GetGraphType() != 1 && GetGraphType() != 2) {
                tp = ((t & 0xC0) * 2) | (f & 0x60);
                tpBit = ((t >> 4) & 0x100) >> 4;
            } else {
                tp = ((t & 0xC0) * 8) | ((f * 4) & 0x180);
                tpBit = (t >> 7) & 0x20;
            }
            p->tpage = tp | tpBit | ((t & 0xF00) >> 8);
            n = t & 0x3F;
            if (n == 0) {
                du = uOff0;
                dv = vOff0;
            } else {
                dv = 0;
                if (n == 1) {
                    du = uOff1;
                    dv = vOff1;
                } else {
                    du = 0;
                }
            }
            setPolyFT4(p);
            p->u0 += du;
            p->v0 += dv;
            p->u1 += du;
            p->v1 += dv;
            p->u2 += du;
            p->v2 += dv;
            p->u3 += du;
            p->v3 += dv;
            if (f & 0x10) {
                setcode(p, 0x2E);
            }
        }

        count = part->ft3Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_FT3), poly += 3) {
            POLY_FT3* p = (POLY_FT3*)out;

            *(u32*)&p->r0 = poly[1];
            v = poly[2];
            *(u16*)&p->u0 = *(u16*)((v & 0xFF) * 2 + (s32)uv);
            *(u16*)&p->u1 = *(u16*)(((v & 0xFF00) >> 7) + (s32)uv);
            *(u16*)&p->u2 = *(u16*)(((v >> 15) & 0x1FE) + (s32)uv);
            f = *texIdx++;
            t = texInfo[f & 0xF];
            shift = (t & 0x3F) != 2;
            p->clut =
                ((((t * 2) >> 23) + (texY & -shift)) << 6) | ((t >> 16) & 0x3F);
            if (GetGraphType() != 1 && GetGraphType() != 2) {
                tp = ((t & 0xC0) * 2) | (f & 0x60);
                tpBit = ((t >> 4) & 0x100) >> 4;
            } else {
                tp = ((t & 0xC0) * 8) | ((f * 4) & 0x180);
                tpBit = (t >> 7) & 0x20;
            }
            p->tpage = tp | tpBit | ((t & 0xF00) >> 8);
            n = t & 0x3F;
            if (n == 0) {
                du = uOff0;
                dv = vOff0;
            } else {
                dv = 0;
                if (n == 1) {
                    du = uOff1;
                    dv = vOff1;
                } else {
                    du = 0;
                }
            }
            setPolyFT3(p);
            p->u0 += du;
            p->v0 += dv;
            p->u1 += du;
            p->v1 += dv;
            p->u2 += du;
            p->v2 += dv;
            if (f & 0x10) {
                setcode(p, 0x26);
            }
        }

        count = part->f3Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_F3), poly += 2) {
            POLY_F3* p = (POLY_F3*)out;

            *(u32*)&p->r0 = poly[1];
            setPolyF3(p);
        }

        count = part->f4Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_F4), poly += 2) {
            POLY_F4* p = (POLY_F4*)out;

            *(u32*)&p->r0 = poly[1];
            setPolyF4(p);
        }

        count = part->g3Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_G3), poly += 4) {
            POLY_G3* p = (POLY_G3*)out;

            *(u32*)&p->r0 = poly[1];
            *(u32*)&p->r1 = poly[2];
            *(u32*)&p->r2 = poly[3];
            setPolyG3(p);
        }

        count = part->g4Count;
        for (i = 0; i < count; i++, out += sizeof(POLY_G4), poly += 5) {
            POLY_G4* p = (POLY_G4*)out;

            *(u32*)&p->r0 = poly[1];
            *(u32*)&p->r1 = poly[2];
            *(u32*)&p->r2 = poly[3];
            *(u32*)&p->r3 = poly[4];
            setPolyG4(p);
        }
    }
    return pkts + part->pktSize * 2;
}
#endif

void FieldModelLoadBsxTexToVram(BsxTexHeader* bsx) {
    RECT rect;
    u32 i;
    u32 count;
    BsxTexEntry* entries;

    count = bsx->texCount;
    entries = bsx->entries;
    for (i = 0; i < count; i++) {
        rect.x = entries[i].x;
        rect.y = entries[i].y;
        rect.w = entries[i].w;
        rect.h = entries[i].h;
        LoadImage(&rect, (u_long*)((u8*)bsx + entries[i].dataOffset));
    }
}

/* One record of a TDB ("texture delta") chunk inside a BSX model file. */
typedef struct {
    /* 0x00 */ u32 opcode; // 0=memcpy, 1=page patch, 2=CLUT patch, 3=LoadImage
    /* 0x04 */ u32 srcOff; // source pixels, relative to the tdb chunk
    /* 0x08 */ u32 size;   // memcpy byte count (op 0)
    /* 0x0C */ u32 dst;    // absolute dest (0) / page idx (1) / CLUT idx (2) /
                           // with dst2, the RECT LoadImage takes (op 3)
    /* 0x10 */ u32 dst2;
} TdbRecord; // size 0x14

/* Apply a TDB ("texture delta") chunk from a BSX model file. Each record
 * relocates a raw blob (op 0), splices one 0x200-byte page (op 1) or one
 * 0x20-byte CLUT (op 2) into the shared model texture block at *D_800DFCA0, or
 * uploads an embedded image straight to VRAM (op 3).
 *
 * The texture block is reached by writing the cast out at every use rather
 * than through a `FieldTexBlockHeader* block` local. The local is the obvious
 * spelling and it costs 7 rows: it makes the pointer live across the whole
 * arm, so gcc loads *D_800DFCA0 into a callee-saved-style temp and can hoist
 * the record's srcOff load into the load-delay slot the target leaves empty.
 * Spelled inline, cse rematerialises the base per reference and the schedule
 * is the target's. decomp-permuter found this (325 -> 0 in 457 candidates).
 *
 * The other four corrections are program, not codegen, and each is readable
 * straight off the target: the guard is `count == 0`, not `count <= 0`; the
 * record is indexed rather than walked (a walked `rec` is a biv and gcc
 * reduces the field addresses onto it); TdbRecord is five words, so a
 * four-word struct strode 0x10 where the target strides 0x14 and every field
 * offset still looked right because the first record's are; and op 0's
 * destination is absolute while op 3's RECT lives inside the record -- the
 * target sets a0 = &rec->dst in the `beq v1,v0,<case3>` delay slot, which no
 * `tdb + off` spelling reaches. */
void FieldModelBsxTdbModify(u8* tdb) {
    TdbRecord* rec;
    s32 count;
    s32 i;

    if (tdb == NULL) {
        return;
    }
    count = *(s32*)tdb;
    if (count == 0) {
        return;
    }
    rec = (TdbRecord*)(tdb + 8);
    for (i = 0; i < count; i++) {
        switch (rec[i].opcode) {
        case 0:
            memcpy((u8*)rec[i].dst, tdb + rec[i].srcOff, rec[i].size);
            break;
        case 1:
            if (rec[i].dst < ((FieldTexBlockHeader*)D_800DFCA0)->numPages) {
                memcpy((u8*)D_800DFCA0 +
                           ((FieldTexBlockHeader*)D_800DFCA0)->pageOffset +
                           (rec[i].dst << 9),
                       tdb + rec[i].srcOff, 0x200);
            }
            break;
        case 2:
            if (rec[i].dst < ((FieldTexBlockHeader*)D_800DFCA0)->numCluts) {
                memcpy((u8*)D_800DFCA0 +
                           ((FieldTexBlockHeader*)D_800DFCA0)->clutOffset +
                           (rec[i].dst << 5),
                       tdb + rec[i].srcOff, 0x20);
            }
            break;
        case 3:
            LoadImage((RECT*)&rec[i].dst, (u_long*)(tdb + rec[i].srcOff));
            break;
        }
    }
}

/* Build the per-model FieldModelEntry table from the loaded model-file
 * descriptor. First pass numbers the NPC-flagged records; second pass fills
 * one entry each and hands out a running offset into the model data block.
 *
 * 22 rows, down from 64 changed / 8 inserted, and every one of the 50 rows
 * recovered came from reading the target rather than from allocation work:
 *   - the old note claimed "-0x38 frame, 6 callee-saved regs". That frame
 *     belonged to the *next* function -- diff.py renders past the end of the
 *     one you asked for (CLAUDE.md, "Neighbouring functions"). The real frame
 *     is -0x10 with no saved registers, and a leaf with no locals gets none,
 *     so the 0x10 is a local nothing references: reserved below, worth 8 rows.
 *   - `models[i].f`, not a walked `m++`. The walk makes the record pointer a
 *     biv, gcc reduces the field addresses onto it and rebases the register to
 *     whichever offset is referenced most -- +4 in the first loop, +3 in the
 *     second -- so every offset in the diff is wrong by a constant and a second
 *     base register appears for the offset-0 access. Worth 33 rows. Both loops
 *     as pointer walks measured 54/5, second loop only 46/4.
 *   - `*(u8*)&models[i].globalModelId` for the (id-1)<9 test and the copy: the
 *     target loads it with `lbu` and the field is `s8` in game.h, where
 *     field3.c's FieldModelLoadBcx needs the `lb`. Retyping the field to u8
 *     buys this row and costs three in FieldModelLoadBcx (measured, and `s8 id`
 *     there is worse still at 50/10) -- so the two translation units genuinely
 *     read the byte with different signedness and the cast is the honest
 *     spelling.
 * Two more, worth another 3 rows and most of the structure:
 *   - `i = 0' between the two loops belongs *after* the first `if', not
 *     inside it. Inside, the count==0 path skips it, the two paths join, and
 *     `strength_reduce' cannot fold the second loop's giv initialiser to
 *     `models + 0' -- so where the target has `move a2,a0' this build emits
 *     `sll v0,a3,3 / addu a2,v0,a0'. One definition dominating the preheader
 *     is all it takes.
 *   - the target holds `data' in *two* registers: the incoming $a1, for the
 *     first store and for computing `next' (`addu a1,a1,v0', in place), and a
 *     copy in $t0 for everything else. One variable cannot do that, since the
 *     parameter's pseudo gets one register -- so the body below takes a second
 *     pointer `d = data;' right after the first store and reads through it.
 *     That is what frees $a1 for `next' and produces the entry `move'; with a
 *     single pointer `next' lands in $t0 and a dozen rows rename. Deriving
 *     `next' from `d' instead of from `data' undoes it (26 rows), and so does
 *     assigning `d' before the first store (21) or only after the first loop
 *     (26). cse does not fold the copy away because every use of `d' is in a
 *     different basic block from the assignment.
 *
 * The residue is 20 rows, and it is one allocation tie-break: `d' and `i' hold
 * each other's register -- the target has `i' in $a3 and the copy in $t0, this
 * build the other way round -- and since $a3 comes earlier in REG_ALLOC_ORDER
 * that means global_alloc processes `i' first there and `d' first here. The
 * published priority is log2(refs)*refs/live_length and it says the opposite:
 * the target's $t0 carries eleven references against $a3's six, over the same
 * range. Whatever orders them is not the ref count. Rejected, all measured
 * against the body below: `next = (u8*)data;` split from the `+=` and hoisted
 * to the top (20/1, identical); all four declaration positions for `d' (20/1,
 * identical -- declaration order is inert here as everywhere else); reading
 * loop 2's `entry' through `data' rather than `d' (19/1, but it puts `next'
 * back in $t0 and is structurally further away); a separate counter for the
 * second loop (40/1).
 *
 * 21 rows -> **1 changed / 0 inserted**, and the note above was wrong about
 * why. Reading cc1's `-dl` and `-dg` dumps names the two quantities exactly:
 * reg 73 is `d' and gets $a3, reg 77 is `i' and gets $t0, and gcc prints its
 * own ordering -- `;; 9 regs to allocate: 210 213 73 77 76 71 72 96 74'. So
 * `d' is processed one place ahead of `i', and the published priority does
 * *not* say the opposite once the right numbers are used: `d' is 15 refs over
 * 97 insns and `i' is 14 over 99, which is floor_log2(15)*15/97 = 0.464
 * against 0.424. A 9% gap, and the earlier note had counted references off
 * the `.s' rather than out of the dump.
 *
 * Two changes close it:
 *   - Route **one** of loop 1's two `d->modelCount' accesses through `data'.
 *     That drops `d' to 14 refs -- and the reference is inside a loop, which
 *     flow weights by depth, so one is enough to put `i' first. Which of the
 *     two moves is irrelevant (all three placements measure identically), and
 *     that is the evidence it is the count and not the expression. Dropping
 *     two *non-loop* references instead -- any pair of the five middle stores
 *     -- does not work: `data' and `d' then both stay live and each costs a
 *     load, 5 to 7 rows with 2 insertions.
 *   - Write `entry->animationOffset' **before** `modelData' and
 *     `partMatrices'. Ours issued those two stores five slots early; the
 *     target issues them after the shift chain. Worth 4 rows on its own and
 *     it is what takes the length from +1 to exact. The reorder alone,
 *     without the reference split, is 17.
 *
 * The one row left is that split: the target reads *both* loop-1 accesses
 * through the copy ($t0) where this build reads one through $a1. So the
 * original had `d' at 15 refs and `i' still winning, which means its ordering
 * came from something other than this ratio -- worth knowing before anyone
 * spends a budget trying to spell the second access differently.
 * Codegen pinned via MASPSX_OVERRIDE; the #else is the verified C.
 * The one row is now measured, not guessed. cc1's .lreg dump gives `d` 13
 * refs over 97 insns and `i` 14 over 99, so allocno_compare ranks i first
 * and i takes $a3 -- which is the target. Spelling the loop-1 read as
 * `d->modelCount` adds two loop-weighted refs, d goes to 15, 3*15/97 beats
 * 3*14/99, and the two exchange registers: 17 rows of pure $a3/$t0 swap.
 * So the row is not the read's spelling, it is that the target reads through
 * `d` *and* keeps i ahead of it, which needs i at 16 refs or d at 13.
 * Everything tried to move either number emits an instruction: duplicating
 * `i += 1` into both arms of loop 1 or loop 2 is +2/+4 (cross-jumping does
 * not merge them, the shared loop test is a separate block), the
 * `if (i) { i += 1; goto adv; }` dead-conditional form is +4/+6, and every
 * middle-block `d` reference moved to `data` to buy the flip back costs the
 * one row it was meant to save (unk8 7, next 6, unk2+unk1 5, and in that
 * last one every register is right and only the two stores' base is $a1).
 * Also inert or worse, all at 17-18: dropping or resizing the frame pad
 * (u8[0x10] is required, [0x8] 19, s32[4] 17, none -3 instructions), moving
 * `i = 0` below the middle block in three places, and adding a `de = desc`
 * copy to mirror the target's $t1 in three placements. decomp-permuter has
 * 87,000 iterations on the 1-row body at base score 5 with no improvement.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldModelStructInit);
#else
void* FieldModelStructInit(FieldModelFileDesc* desc, FieldModelData* data) {
    FieldModelData* d;
    FieldModelLoaderData* models;
    FieldModelEntry* entry;
    u8* next;
    u32 i;
    s16 partsOff;
    u8 unusedLocals[0x10];

    i = 0;
    data->modelCount = 0;
    d = data;
    models = desc->models;
    if (desc->count != 0) {
        do {
            if (models[i].npcFlag != 0) {
                models[i].modelEntryIndex = data->modelCount;
                d->modelCount = d->modelCount + 1;
            } else {
                models[i].modelEntryIndex = 0xFF;
            }
            i += 1;
        } while (i < desc->count);
    }
    i = 0;
    d->unk2 = 0;
    d->unk1 = 0;
    d->modelEntries = (FieldModelEntry*)((u8*)d + 0xC);
    d->unk8 = 0;
    next = (u8*)data + ((d->modelCount * 0x24) + 0xC);
    if (desc->count != 0) {
        do {
            if (models[i].npcFlag != 0) {
                if (((u32)(*(u8*)&models[i].globalModelId - 1) < 9) &&
                    (models[i].animationCount < 3)) {
                    models[i].animationCount = 3;
                }
                entry = &d->modelEntries[models[i].modelEntryIndex];
                entry->flags = 1;
                entry->kawaiType = -1;
                entry->boneCount = models[i].boneCount;
                entry->partCount = models[i].partCount;
                entry->animationCount = models[i].animationCount;
                entry->rotationZ = 0;
                entry->rotationY = 0;
                entry->rotationX = 0;
                entry->translationZ = 0;
                entry->translationY = 0;
                entry->translationX = 0;
                entry->globalModelId = *(u8*)&models[i].globalModelId;
                entry->textureFaceId = models[i].faceId;
                entry->scale = 0x1000;
                partsOff = models[i].boneCount * 4;
                entry->partsOffset = partsOff;
                entry->animationOffset = partsOff + (models[i].partCount << 5);
                entry->modelData = next;
                entry->partMatrices = NULL;
                next += (models[i].boneCount * 4) + (models[i].partCount << 5) +
                        (models[i].animationCount * 0x10);
            }
            i += 1;
        } while (i < desc->count);
    }
    D_800E0204 = 0;
    return next;
}
#endif

u8* FieldModelLoadBcx(
    FieldModelFileDesc* desc, FieldModelData* data, u8* pkts, s32 index);

/* Loads every global (BCX) model in the header, then optionally kicks off the
 * next streamed read. Scratchpad word 0 is clobbered by each load and restored
 * before the next one; word 1 holds the sector/size pair for that read. */
u8* FieldModelLoadGlobalModels(
    FieldModelFileDesc* desc, FieldModelData* data, u8* pkts, s32 readFile) {
    u32* fileInfo;
    s32 saved;
    u32 i;

    saved = ((s32*)0x1F800000)[0];
    fileInfo = (u32*)((s32*)0x1F800000)[1];
    for (i = 0; i < desc->count; i++) {
        ((s32*)0x1F800000)[0] = saved;
        pkts = FieldModelLoadBcx(desc, data, pkts, i);
    }
    if (readFile) {
        DS_read(fileInfo[0], fileInfo[1], D_800DFCA0, NULL);
        while (SystemCdromReadChain() != 0) {
        }
    }
    return pkts;
}
