//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 2 of 5, split out of field.c. .rodata 0x800A0054-0x800A00E0, base 4 mod
 * 8 -> --phase 4. Every jump table here sits at an address 4 mod 8; `.align 3`
 * would push it 4 bytes forward. Do not merge this unit into a 0 mod 8
 * neighbour. */

/* Seed the background-scroll state machine from the requested scroll mode.
 * Only runs while idle (D_8009AC13 == 0). Modes: 0 stops and recentres; 1 arms
 * scrolling in place; 2/3 begin a single-target scroll; 4 teleports the current
 * position to the alt source; 5-9 begin a dual-target (eased) scroll. The
 * target positions/step/fraction are what FieldBGScrollUpdate consumes each
 * frame.
 *
 * Modes 7-9 are empty arms, not part of the 5/6 dual-target block: the target's
 * table sends .rodata+0x1c/0x20/0x24 straight to the epilogue while 0x14/0x18
 * reach the work block. They still have to be written out as `case 7: case 8:
 * case 9: break;` rather than dropped, or the range test narrows from
 * `sltiu 0xa` to `sltiu 0x7` and the table loses three entries. This was
 * previously mis-read as the jump-table alignment phase; a `.rodata+0xNN` row
 * from checkfn means the table's *contents* differ, and only a `want:/got:` on
 * the table's own address means alignment. */
extern u8 D_8009AC11;  // scroll mode (jump-table selector)
extern u8 D_8009AC13;  // scroll state (0 = idle)
extern s16 D_8009A100; // scroll enable
extern u16 D_8009AC14; // scroll source X
extern u16 D_8009ABFE; // alt scroll source X
extern u16 D_8009AC00; // alt scroll source Y
extern s16 D_80071E38; // current scroll X
extern s16 D_80071E3C; // current scroll Y
extern s16 D_8009C558; // scroll step
extern s16 D_80075CF8; // scroll sub-position / fraction
extern s16 D_80075E14; // target scroll X
extern s16 D_80075E1C; // target scroll Y
extern s16 D_80075E18; // alt target scroll X
extern s16 D_80075E20; // alt target scroll Y

void FieldBGScrollInit(void) {
    if (D_8009AC13 != 0) {
        return;
    }
    switch (D_8009AC11) {
    case 0:
        D_8009A100 = 0;
        D_80071E38 = 0;
        D_80071E3C = 0;
        D_8009AC13 = 2;
        break;
    case 1:
        D_8009A100 = 1;
        D_8009AC13 = 1;
        break;
    case 2:
    case 3:
        D_8009A100 = 1;
        D_80075CF8 = 0;
        D_8009AC13 = 1;
        D_8009C558 = D_8009AC14;
        D_80075E14 = D_80071E38;
        D_80075E1C = D_80071E3C;
        break;
    case 4:
        D_8009A100 = 1;
        D_8009AC13 = 2;
        D_80071E38 = D_8009ABFE;
        D_80071E3C = D_8009AC00;
        break;
    case 5:
    case 6:
        D_8009A100 = 1;
        D_80075CF8 = 0;
        D_8009AC13 = 1;
        D_8009C558 = D_8009AC14;
        D_80075E14 = D_80071E38;
        D_80075E1C = D_80071E3C;
        D_80075E18 = D_8009ABFE;
        D_80075E20 = D_8009AC00;
        break;
    case 7:
    case 8:
    case 9:
        break;
    }
}

/* Project a point onto a trigger line: for a type-1 or type-2 trigger compute
 * the closest on-line point to the entity and write it back into arg1. m2c
 * seed; the residual is regalloc across the divide-by-length-squared and the
 * two near-duplicate branches. Codegen pinned via MASPSX_OVERRIDE pending a
 * permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldCalcPointOnLine);
#else
void FieldCalcPointOnLine(void* arg0, void* arg1) {
    s16 temp_a2_3;
    s16 temp_t0;
    s16 temp_t1;
    s16 temp_v0;
    s16 temp_v1;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_4;
    s32 temp_t0_2;
    s32 temp_t2;
    s32 temp_t2_2;

    if (arg0->unk14 == 1) {
        temp_t0 = arg0->unkC;
        temp_a2 = arg0->unk10 - (temp_t0 + 0x140);
        temp_v0 = arg0->unkE;
        temp_a0 = arg0->unk12 - (temp_v0 + 0xF0);
        temp_t2 = -(((temp_t0 - (arg1->unk0 - 0xA0)) * temp_a2) +
                    ((temp_v0 - (arg1->unk2 - 0x78)) * temp_a0));
        temp_a2_2 = (s32)((temp_a2 * temp_a2) + (temp_a0 * temp_a0)) >> 8;
        arg1->unk0 = (s16)(((s32)((s32)(temp_t2 * temp_a2) / temp_a2_2) >> 8) +
                           0xA0 + temp_t0);
        arg1->unk2 = (s16)(((s32)((s32)(temp_t2 * temp_a0) / temp_a2_2) >> 8) +
                           0x78 + (u16)arg0->unkE);
    }
    if (arg0->unk14 == 2) {
        temp_t1 = arg0->unkC;
        temp_t0_2 = arg0->unk10 - (temp_t1 + 0x140);
        temp_a2_3 = arg0->unk12;
        temp_v1 = arg0->unkE;
        temp_a1 = temp_v1 - (temp_a2_3 - 0xF0);
        temp_t2_2 = -(((temp_t1 - (arg1->unk0 - 0xA0)) * temp_t0_2) +
                      ((temp_a2_3 - (arg1->unk2 + 0x78)) * temp_a1));
        temp_a2_4 = (s32)((temp_t0_2 * temp_t0_2) +
                          ((temp_v1 - temp_a2_3) * temp_a1)) >>
                    8;
        arg1->unk0 =
            (s16)(((s32)((s32)(temp_t2_2 * temp_t0_2) / temp_a2_4) >> 8) +
                  0xA0 + temp_t1);
        arg1->unk2 =
            (s16)(((s32)((s32)(temp_t2_2 * temp_a1) / temp_a2_4) >> 8) - 0x78 +
                  (u16)arg0->unk12);
    }
}
#endif

/* Scroll limits at the head of the field's trigger block. g_FieldTriggers is
 * typed s32 because it is assigned as a raw word on load. */
typedef struct {
    /* 0x00 */ u8 unk00[0xC];
    /* 0x0C */ s16 minX;
    /* 0x0E */ s16 minY;
    /* 0x10 */ s16 maxX;
    /* 0x12 */ s16 maxY;
} FieldScrollLimits;

#define FIELD_SCROLL_LIMITS ((FieldScrollLimits*)g_FieldTriggers)

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
extern s16 D_80075CF8;
extern s16 D_80075E14;
extern s16 D_80075E18;
extern s16 D_80075E1C;
extern s16 D_80075E20;
extern u8 D_8009AC11;
extern u8 D_8009AC13;
extern s16 D_8009C558;

/* Per-frame background scroll: on the field's scroll state machine, drive the
 * background X/Y toward the entity's clamped screen position (linear or
 * ease-in-out depending on the mode). The seed is semantically close; the
 * residual is the oversized stack frame (dead locals the original declared)
 * plus regalloc. Codegen pinned via MASPSX_OVERRIDE pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldBGScrollUpdate);
#else
void FieldBGScrollUpdate(void) {
    s32 sp10;
    s16 var_a1;
    s16 var_v0_2;
    s32 var_v0;

#define unksp12 (((s16*)&sp10)[1])
    if (D_8009AC13 == 1) {
        switch (D_8009AC11) {
        case 1:
            FieldBGGetEntityScreenPos(&sp10);
            FieldBGClampPos((s16*)&sp10);
            D_80071E38 = -(s16)(u16)sp10;
            D_80071E3C = -(s16)unksp12;
            return;
        case 2:
            FieldBGGetEntityScreenPos(&sp10);
            FieldBGClampPos((s16*)&sp10);
            D_80071E38 = FieldCalcLinearStep((s32)D_80075E14, (s32) - (s16)sp10,
                                             (s32)D_8009C558, (s32)D_80075CF8);
            var_a1 = -unksp12;
        block_5:
            var_v0 = FieldCalcLinearStep(
                (s32)D_80075E1C, (s32)var_a1, (s32)D_8009C558, (s32)D_80075CF8);
        block_6:
            D_80071E3C = (s16)var_v0;
            if (D_8009C558 != D_80075CF8) {
                var_v0_2 = D_80075CF8 + 1;
            block_13:
                D_80075CF8 = var_v0_2;
            } else {
            block_11:
                D_8009AC13 = 2;
                return;
            }
            break;
        case 3:
            FieldBGGetEntityScreenPos(&sp10);
            FieldBGClampPos((s16*)&sp10);
            D_80071E38 = FieldCalcEaseInOut((s32)D_80075E14, (s32) - (s16)sp10,
                                            (s32)D_8009C558, (s32)D_80075CF8);
            var_v0 = FieldCalcEaseInOut((s32)D_80075E1C, (s32)-unksp12,
                                        (s32)D_8009C558, (s32)D_80075CF8);
            goto block_6;
        case 5:
            var_a1 = D_80075E20;
            D_80071E38 = FieldCalcLinearStep((s32)D_80075E14, (s32)D_80075E18,
                                             (s32)D_8009C558, (s32)D_80075CF8);
            goto block_5;
        case 6:
            D_80071E38 = FieldCalcEaseInOut((s32)D_80075E14, (s32)D_80075E18,
                                            (s32)D_8009C558, (s32)D_80075CF8);
            D_80071E3C = FieldCalcEaseInOut((s32)D_80075E1C, (s32)D_80075E20,
                                            (s32)D_8009C558, (s32)D_80075CF8);
            if (D_8009C558 == D_80075CF8) {
                goto block_11;
            }
            var_v0_2 = D_80075CF8 + 1;
            goto block_13;
        }
    } else {
    default:
    }
#undef unksp12
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field2", FieldBGUpdateDrawenv);

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
        g_PlayerModelId = D_8009ABF4.pcModelId;
        g_FieldEntity[g_PlayerModelId].PosI = D_8009ABF4.pcWalkMeshId;
        if (D_8009ABF4.pcPosX == 0x7FFF) {
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
            g_FieldEntity[g_PlayerModelId].PosX = D_8009ABF4.pcPosX << 12;
            g_FieldEntity[g_PlayerModelId].PosY = D_8009ABF4.pcPosY << 12;
            FieldEntityVectorSub(
                edgeA,
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 4],
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12]);
            FieldEntityVectorSub(
                edgeB,
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 8],
                &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12 + 4]);
            point[0] = D_8009ABF4.pcPosX;
            point[1] = D_8009ABF4.pcPosY;
            g_FieldEntity[g_PlayerModelId].PosZ =
                FieldEntityCalculateZ(
                    edgeA, edgeB, point,
                    &D_800E4274[g_FieldEntity[g_PlayerModelId].PosI * 12])
                << 12;
        }
        g_FieldEntity[g_PlayerModelId].SolidRange =
            (D_8009ABF4.currentFieldScale * 0x11) >> 8;
        moveSpeed = D_8009ABF4.currentFieldScale * 2;
        g_FieldEntity[g_PlayerModelId].animSpeed = 0x10;
        g_FieldEntity[g_PlayerModelId].MoveSpeed = moveSpeed;
    }
    for (i = 0; i < D_8009ABF4.modelCount; i++) {
        g_FieldEntity[i].MoveDirAdd = 0;
    }
}

void FieldEntityAddRotate(s32 arg0, s16 entityIdx) {
    if (g_FieldAnimLock == 0) {
        if (D_8009ABF4.activeKeys2 & PADR1) {
            g_FieldEntity[entityIdx].MoveDirAdd = 0xE0;
        } else if (D_8009ABF4.activeKeys2 & PADL1) {
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

INCLUDE_ASM("asm/us/field/nonmatchings/field2", FieldEntityMovementUpdate);

void FieldEntityGatewayMapLoad(FieldGateway* gateway) {
    D_8009ABF4.eventCmd = EVTCMD_FIELD_MAP_CHANGE;
    D_8009ABF4.eventCmdParam = gateway->destFieldId;
    D_8009ABF4.pcPosX = gateway->destPosX;
    D_8009ABF4.pcPosY = gateway->destPosY;
    D_8009ABF4.pcWalkMeshId = gateway->destWalkMeshId;
    *(u16*)&D_8009ABF4.pcDirection = gateway->destDirection;
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
 * The three rows left: the target masks the FieldEntityDirByVec result with
 * `andi v0,v0,0xff` before subtracting it from Dir, filling the load-delay slot
 * of the `lbu` that reads Dir, where combine folds our inner mask away and
 * maspsx emits a nop. Every type for `dirTo` (u8, s16, u16, u32, s32, with and
 * without an explicit `(u8)` at the use) compiles identically, so the mask is
 * not coming from the variable's declaration. The other two rows are the two
 * `li v1,<const>` of the final test: the target issues each before the
 * neighbouring shift pair and gcc after, which is dbr choosing a different
 * insn for the same slot -- nesting the two conditions instead of `&&` does
 * not change it. Codegen pinned via MASPSX_OVERRIDE. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityCheckTalk);
#else
void FieldEntityCheckTalk(void) {
    VECTOR from;
    VECTOR to;
    s16 quality[16];
    s32 sqrDist;
    s16 bestId;
    s16 best;
    u8 dirTo;
    s32 i;

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
        quality[i] = (u8)(g_FieldEntity[g_PlayerModelId].Dir - dirTo);
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
        g_FieldEntity[bestId].requestTalkScript = 1;
    }
}
#endif

s16 FieldEntityGetDirVectorX(u8 arg0) { return D_800DF120[arg0][0]; }

s16 FieldEntityGetDirVectorY(u8 arg0) { return D_800DF120[arg0][1]; }

extern u8 D_800DEF88[];

/* Direction (0-255) from one point to another. The third parameter is
 * in/out: it is written with the squared distance, then *overwritten with the
 * distance itself* -- callers compare it against a plain range, not a squared
 * one. The slope of each axis is taken in 12-bit fixed point, divided down by
 * 32, and the arctan table D_800DEF88 is indexed by whichever axis is the
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
 * And the last row: the negative table index is `D_800DEF88[-dy * 2]`, not
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
                angle = D_800DEF88[dy * 2];
            } else {
                angle = -D_800DEF88[-dy * 2];
            }
        } else {
            if (dy > 0) {
                angle = -0x80 - D_800DEF88[dy * 2];
            } else {
                angle = D_800DEF88[-dy * 2] - 0x80;
            }
        }
    } else {
        if (dy > 0) {
            if (dx > 0) {
                angle = 0x40 - D_800DEF88[dx * 2];
            } else {
                angle = D_800DEF88[-dx * 2] + 0x40;
            }
        } else {
            if (dx > 0) {
                angle = D_800DEF88[dx * 2] - 0x40;
            } else {
                angle = -0x40 - D_800DEF88[-dx * 2];
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

extern /*?*/ s32 D_8009ACA6;
extern u16 D_80113F28;
extern s32 D_80114458;
extern s16 D_801144CC;

/* Detect when a moving entity crosses a walkmesh triangle edge: walk the
 * triangle's edges, compute the cross products against the entity's position,
 * and return which edge (if any) the entity is crossing plus the resulting Z.
 * Uses the 0x1F8000xx scratchpad for the per-edge vectors. m2c seed; residual
 * is the cross-product regalloc and the scratchpad access ordering. Pinned
 * pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityWalkmechCross);
#else
s32 FieldEntityWalkmechCross(u16* arg0, void* arg1, void* arg2, void* arg3) {
    s16 var_v0_3;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_a2_3;
    s32 temp_a2_4;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 var_s3;
    s32 var_v0;
    s32 var_v0_2;
    u16 temp_v1;
    u16 var_a0;
    void* temp_a0;

    var_s3 = 0;
    var_v0 = arg1->unk0;
    if (var_v0 < 0) {
        var_v0 += 0xFFF;
    }
    *(s32*)0x1F800030 = var_v0 >> 0xC;
    var_v0_2 = arg1->unk4;
    if (var_v0_2 < 0) {
        var_v0_2 += 0xFFF;
    }
    *(s32*)0x1F800034 = var_v0_2 >> 0xC;
    *(s32*)0x1F800038 = 0;
    D_80113F28 = 0xFFFF;
loop_5:
    temp_a2 = *arg0 * 0x18;
    FieldEntityVectorSub(
        (s32*)0x1F800000, temp_a2 + 8 + D_800E4274, temp_a2 + D_800E4274);
    temp_a2_2 = *arg0 * 0x18;
    FieldEntityVectorSub((s32*)0x1F800000, temp_a2_2 + 0x10 + D_800E4274,
                         temp_a2_2 + 8 + D_800E4274);
    temp_a2_3 = *arg0 * 0x18;
    FieldEntityVectorSub((s32*)0x1F800000, temp_a2_3 + D_800E4274,
                         temp_a2_3 + 0x10 + D_800E4274);
    temp_v1 = *arg0;
    temp_a1 = *(s32*)0x1F800030;
    temp_a0 = (temp_v1 * 0x18) + D_800E4274;
    temp_a2_4 = *(s32*)0x1F800034;
    temp_t1 = ((temp_a1 - temp_a0->unk8) * *(s32*)0x1F800014) -
              ((temp_a2_4 - temp_a0->unkA) * *(s32*)0x1F800010);
    temp_a0_2 = ((temp_a1 - temp_a0->unk10) * *(s32*)0x1F800024) -
                ((temp_a2_4 - temp_a0->unk12) * *(s32*)0x1F800020);
    if ((((temp_a1 - temp_a0->unk0) * *(s32*)0x1F800004) -
         ((temp_a2_4 - temp_a0->unk2) * *(s32*)0x1F800000)) >= 0) {
        if (temp_t1 >= 0) {
            if (temp_a0_2 < 0) {
                if (temp_t1 < 0) {
                    goto block_15;
                }
                if (temp_a0_2 < 0) {
                    var_a0 = ((temp_v1 * 6) + D_80114458)->unk4;
                    temp_v0 = (s32)(var_a0 << 0x10) >> 0x13;
                    if (((s16)var_a0 >= 0) &&
                        !(((s32) * (&D_8009ACA6 + temp_v0) >>
                           ((s16)var_a0 - (temp_v0 * 8))) &
                          1)) {
                        goto block_23;
                    }
                    arg3->unk0 = (s32) * (s32*)0x1F800020;
                    arg3->unk4 = (s32) * (s32*)0x1F800024;
                    arg3->unk8 = (s32) * (s32*)0x1F800028;
                    var_s3 = -8;
                    if (((*(s32*)0x1F800020 * arg2->unk0) +
                         (*(s32*)0x1F800024 * arg2->unk4)) >= 0) {
                        var_s3 = 8;
                    }
                    var_v0_3 = 2;
                    goto block_27;
                }
                goto loop_5;
            }
        } else {
        block_15:
            var_a0 = ((temp_v1 * 6) + D_80114458)->unk2;
            temp_v0_2 = (s32)(var_a0 << 0x10) >> 0x13;
            if (((s16)var_a0 < 0) || (((s32) * (&D_8009ACA6 + temp_v0_2) >>
                                       ((s16)var_a0 - (temp_v0_2 * 8))) &
                                      1)) {
                arg3->unk0 = (s32) * (s32*)0x1F800010;
                arg3->unk4 = (s32) * (s32*)0x1F800014;
                arg3->unk8 = (s32) * (s32*)0x1F800018;
                var_s3 = -8;
                if (((*(s32*)0x1F800010 * arg2->unk0) +
                     (*(s32*)0x1F800014 * arg2->unk4)) >= 0) {
                    var_s3 = 8;
                }
                var_v0_3 = 1;
            block_27:
                D_801144CC = var_v0_3;
                D_80113F28 = *arg0;
            } else {
                goto block_23;
            }
        }
    } else {
        var_a0 = *((temp_v1 * 6) + D_80114458);
        temp_v0_3 = (s32)(var_a0 << 0x10) >> 0x13;
        if (((s16)var_a0 < 0) || (((s32) * (&D_8009ACA6 + temp_v0_3) >>
                                   ((s16)var_a0 - (temp_v0_3 * 8))) &
                                  1)) {
            arg3->unk0 = (s32) * (s32*)0x1F800000;
            arg3->unk4 = (s32) * (s32*)0x1F800004;
            arg3->unk8 = (s32) * (s32*)0x1F800008;
            var_s3 = -8;
            if (((*(s32*)0x1F800000 * arg2->unk0) +
                 (*(s32*)0x1F800004 * arg2->unk4)) >= 0) {
                var_s3 = 8;
            }
            D_801144CC = 0;
            D_80113F28 = *arg0;
        } else {
        block_23:
            *arg0 = var_a0;
            goto loop_5;
        }
    }
    arg1->unk8 = FieldEntityCalculateZ(
        (s32*)0x1F800000, (s32*)0x1F800010, (s32*)0x1F800030,
        (*arg0 * 0x18) + D_800E4274);
    return var_s3;
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

INCLUDE_ASM("asm/us/field/nonmatchings/field2", FieldEntityMove);

extern s16 D_8009AC1C;

/* Would `pos` put entity `entityId` inside another solid entity? Two entities
 * collide when their horizontal distance falls under the mean of their two
 * solid radii and they are within ~127 units of each other vertically, so
 * characters on a different floor of the same map never block one another.
 * Only the player's own collisions arm the other entity's push script.
 *
 * The entity count is `D_8009ABF4.modelCount` -- FieldState + 0x28 -- and
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
    for (i = 0; i < D_8009ABF4.modelCount; i++) {
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
 * 91 rows -> 58. Three of the corrections are program, not codegen:
 *   - the return value is only raised for a line whose `slipDisabled` byte is
 *     1, not for every line in range. The target reads +0x16 for that test and
 *     +0x0E (touch) for the touch-on test right after it; the old body read
 *     +0x0E for both and set `hit` unconditionally.
 *   - the four-way sign ladder is *not* negated. `across` is set when one of
 *     the four terms holds, and the target's first branch (`bltz crossFrom` to
 *     the second term, else `bltz crossTo` straight to the store) says so
 *     plainly. Worth 5 rows on its own.
 *   - the facing test needs an `s32` local: `(u8)(...) >= 0x80` inline gives
 *     `sltiu`, through a local `slti`, which is what the target has.
 * And the shape of the walk: `line` is a walking `FieldLine*` used for the
 * SqrDistToLine call and for `pos.x1`, while every other field is reached as
 * `lines[i].<field>`. That is not a scaffold -- it is what the target's two
 * base registers say. Writing all the `pos` fields through `line` measures 88,
 * all of them through `lines[i]` measures 79, and one pointer for everything
 * (either spelling) 91-96.
 *
 * The 58 rows left are one fact with a long tail: the target's second base is
 * `lines + 0x0E` and it keeps the counter `i` alive beside it, where gcc bases
 * the giv at +0 and then eliminates `i` entirely, rewriting the exit test as a
 * pointer compare against `lines + 0x300`. Every field offset therefore reads
 * 0x0E high and the saved-register list is one short (the target uses s0..s8,
 * this uses s0..s7). Nothing tried moves it: `&lines[i]` for the call, the
 * flags through a separate `u8*`, indexing everything, walking everything.
 * Codegen pinned via MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityLineCheck);
#else
u8 FieldEntityLineCheck(FieldEntity* entity, FieldLine* lines, VECTOR* dest) {
    s32* from;
    s32* to;
    s32* nearest;
    FieldLine* line;
    s32 sqrDist;
    s32 crossFrom;
    s32 crossTo;
    u8 hit;
    s32 delta;
    s32 i;

    from = (s32*)0x1F800000;
    to = (s32*)0x1F800010;
    nearest = (s32*)0x1F800020;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    to[0] = dest->vx;
    to[1] = dest->vy;
    to[2] = entity->PosZ >> 12;
    hit = 0;
    for (i = 0, line = lines; i < 32; i++, line++) {
        if (lines[i].isActive != 1) {
            continue;
        }
        lines[i].isOnLine = 0;
        sqrDist = FieldEntitySqrDistToLine(line, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < entity->SolidRange * entity->SolidRange) {
            if (lines[i].slipDisabled == 1) {
                hit = 1;
            }
            if (lines[i].touch == 0) {
                lines[i].requestTouchOnScript = 1;
            }
            lines[i].touch = 1;
            crossFrom =
                (lines[i].pos.x2 - line->pos.x1) * (from[1] - lines[i].pos.y1) -
                (from[0] - line->pos.x1) * (lines[i].pos.y2 - lines[i].pos.y1);
            crossTo =
                (lines[i].pos.x2 - line->pos.x1) * (to[1] - lines[i].pos.y1) -
                (to[0] - line->pos.x1) * (lines[i].pos.y2 - lines[i].pos.y1);
            if ((crossFrom >= 0 && crossTo < 0) ||
                (crossTo >= 0 && crossFrom < 0) ||
                (crossFrom > 0 && crossTo <= 0) ||
                (crossTo > 0 && crossFrom <= 0)) {
                lines[i].across = 1;
            }
            if (nearest[0] != from[0] || nearest[1] != from[1]) {
                lines[i].proximityAngle = FieldEntityDirByVec(
                    (VECTOR*)from, (VECTOR*)nearest, &sqrDist);
                delta = (u8)(lines[i].proximityAngle - entity->MoveDir + 0x40);
                if (delta >= 0x80) {
                    continue;
                }
            } else {
                continue;
            }
            lines[i].requestPushScript = 1;
            lines[i].isOnLine = 1;
        } else {
            if (lines[i].touch == 1) {
                lines[i].requestTouchOffScript = 1;
            }
            lines[i].touch = 0;
        }
    }
    return hit;
}
#endif

/* Walk the map's 32 trigger lines against one entity and raise the script
 * requests each one is due. Entering a line's radius arms its touch-on script
 * and leaving it arms touch-off, with `touch` holding the edge state between
 * frames. The talk request additionally needs the entity facing within +/-32
 * of the line's proximity angle and the OK button newly pressed this frame --
 * pad2 current has the bit and pad2 previous does not. An entity under script
 * control (scriptedMoveMode) triggers nothing.
 *
 * 19 rows, and all but one of them are a single tie-break: the original keeps
 * the walking line pointer in s2 and the hoisted constant 1 in s3, gcc the
 * other way round, and every use of either follows. global-alloc orders
 * allocnos by roughly log2(refs) * refs / live_length, and the 1 has seven
 * references against the pointer's three, so the constant is allocated first
 * and takes s2. Nothing tried moves it: declaration order, statement order,
 * indexing `line[i]` instead of walking, and assigning `pad2` before or after
 * the `from[]` stores (22 rows) were all measured. The one row that did move
 * is the facing test -- `(u8)(...) >= 0x40` written inline gives `sltiu`,
 * because combine folds the zero-extension into the compare and proves the
 * sign bit clear; through the `s32 diff` local below the extension stays its
 * own `andi` and the compare is `slti`, which is what the target has.
 *
 * The remaining tell is that `&g_FieldPad2State` is materialised early here
 * and late in the target, after the `li 1`. Reading the two words inline as
 * `(&g_FieldPad2State)[0]` and `[1]`, so the address becomes a movable rather
 * than a source statement, is worse (49 rows) -- the local is right. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityLineInteract);
#else
void FieldEntityLineInteract(FieldEntity* entity, FieldLine* line) {
    s32* from;
    s32* nearest;
    s32 i;
    s32 sqrDist;
    s32 diff;
    u32* pad2;

    from = (s32*)0x1F800000;
    nearest = (s32*)0x1F800010;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    pad2 = &g_FieldPad2State;
    for (i = 0; i < 32; i++, line++) {
        if (line->isActive != 1) {
            continue;
        }
        if (entity->scriptedMoveMode != 0) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine(line, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < entity->SolidRange * entity->SolidRange) {
            if (line->touch == 0) {
                line->requestTouchOnScript = 1;
            }
            line->touch = 1;
        } else {
            if (line->touch == 1) {
                line->requestTouchOffScript = 1;
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
        line->requestTalkScript = 1;
    }
}
#endif

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

/* One entry of the map's background-trigger block. Even `type`s arm the
 * trigger, odd ones disarm it. */
typedef struct {
    /* 0x00 */ u8 unk00[0xC];
    /* 0x0C */ u8 entityId;
    /* 0x0D */ u8 unk0D;
    /* 0x0E */ u8 type;
    /* 0x0F */ u8 unk0F;
} FieldBgTrigger;

/* Arms (even type) or disarms (odd type) one background trigger, and reports
 * whether that actually changed the bit -- the caller only redraws when it did.
 *
 * Blocked by the same jump-table alignment problem as FieldEntityBgTriggerInit:
 * the original table sits at .rodata+0xa4 and gcc's `.align 3` puts ours at
 * +0xa8. Unlike Init this C is not yet instruction-exact either -- gcc
 * cross-jumps the two arms' shared store tail here and does not in the
 * original -- so it needs another pass once the file is split. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field2", FieldEntityBgTriggerActivate);
#else
s32 FieldEntityBgTriggerActivate(FieldBgTrigger* trigger, u8 type) {
    s32 changed;
    s32 bit;
    s32 old;
    s32 mask;
    u8 merged;

    changed = 0;
    switch (type) {
    case 0:
    case 2:
    case 4:
        old = g_FieldEntityBgTrigger[trigger->entityId];
        bit = 1 << trigger->unk0D;
        if ((old & bit) == 0) {
            changed = 1;
        }
        g_FieldEntityBgTrigger[trigger->entityId] = bit | old;
        break;
    case 1:
    case 3:
    case 5:
        mask = ~(1 << trigger->unk0D);
        old = g_FieldEntityBgTrigger[trigger->entityId];
        merged = old | mask;
        if (merged == 0xFF) {
            changed = 1;
        }
        g_FieldEntityBgTrigger[trigger->entityId] = mask & old;
        break;
    }
    return changed;
}
#endif

const u32 D_800A00BC[] = {0x00360000, 0x012A007A};
/* Walk the 12 background triggers against one entity and arm/disarm each it
 * crosses or comes near. In-proximity arms directly when the entity stands on
 * the line, else needs the entity facing it within +/-64; crossing types 4/5
 * arm/disarm on the back-side sign test. Each state change plays the trigger's
 * sound effect. Verified C kept as the #else; codegen pinned via
 * MASPSX_OVERRIDE (the dual walking-pointer regalloc wall). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldEntityTriggerCheck);
#else
void FieldEntityTriggerCheck(
    FieldEntity* entity, FieldBgTrigger* triggers, VECTOR* dest) {
    s16 seIds[4];
    s32* from;
    s32* nearest;
    FieldBgTrigger* trigger;
    s32 sqrDist;
    s32 cross;
    u8 dir;
    s32 i;

    memcpy(seIds, (void*)D_800A00BC, 8);
    from = (s32*)0x1F800000;
    nearest = (s32*)0x1F800020;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    for (i = 0; i < 12; i++) {
        trigger = &triggers[i];
        if (trigger->entityId == 0xFF) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine((FieldLine*)trigger, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < entity->SolidRange * entity->SolidRange) {
            if (from[0] == nearest[0] && from[1] == nearest[1]) {
                if (FieldEntityBgTriggerActivate(trigger, trigger->type) == 1) {
                    func_8001117C(seIds[trigger->unk0F]);
                }
                continue;
            }
            dir =
                FieldEntityDirByVec((VECTOR*)from, (VECTOR*)nearest, &sqrDist);
            if ((u8)(dir - entity->MoveDir + 0x40) >= 0x80) {
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
                    (from[0] - trigger->pos.x1) *
                        (trigger->pos.y2 - trigger->pos.y1);
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
#endif

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
/* Top-level field model loader: build the FieldModelData from the loaded model
 * header, stream the field's model set off the CD, load the global and local
 * models, then push each model's eye/mouth textures to VRAM and reset the KAWAI
 * state. Verified C kept as the #else; codegen pinned via MASPSX_OVERRIDE. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldModelLoadAndInit);
#else
void FieldModelLoadAndInit(void) {
    FieldModelData* data;
    s32 i;

    D_800DFCA0 = (u_long*)0x80128000;
    data = g_FieldModelData;
    FieldModelStructInit((FieldModelFileDesc*)D_8007E770, data);
    DS_read(g_FieldLzsInfo[g_CurrentFieldIndex * 6],
            g_FieldLzsInfo[g_CurrentFieldIndex * 6 + 1], (u32*)0x80128000,
            NULL);
    while (SystemCdromReadChain() != 0) {
    }
    D_80075E10 = (u32)FieldModelLoadGlobalModels(
        (FieldModelFileDesc*)D_8007E770, g_FieldModelData, (u8*)D_80075E10, 1);
    ((s32*)0x1F800000)[0] = (s32)D_800DF08C;
    ((s32*)0x1F800000)[1] = (s32)D_800DF0D4;
    D_80075E10 = (u32)LoadLocalFieldModelAndInitAll(
        g_FieldModelData, D_8007E770, (u8*)D_80075E10);
    for (i = 0; i < g_FieldModelData->modelCount; i++) {
        KawaiLoadEyesMouthTexToVram(
            &g_FieldModelData->modelEntries[i], (u8*)0x1F800000);
    }
    KawaiClearData();
}
#endif

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
 * `BlinkOn` guard below it; in insn order the 2 therefore always comes first
 * and no rewriting of these two statements reverses it. Whatever the original
 * did, it did not put a plain constant assignment at the top of this loop --
 * look for a third statement there that materialises the 1. */
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
        D_80071E40 = D_80083270;
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
    if (D_8009ABF4.eventCmd == EVTCMD_UNK14) {
        func_80035658();
        g_FieldMovieStreamActive = 0;
        g_FieldMoviePlayed = 0;
        D_8009ABF4.movieCommandState = MOVCMD_DONE;
        return;
    }
    status = SystemCdromReadChain();
    switch (status) {
    case 0:
        if (D_8009ABF4.eventCmd == EVTCMD_LOAD_MOVIE &&
            D_8009ABF4.movieCommandState == MOVCMD_IDLE) {
            if (D_80075E10 <= 0x801AFFFF) {
                func_80034FC8(D_80075E10, D_8009ABF4.eventCmdParam);
            } else {
                func_80034FC8(0x801B0000, D_8009ABF4.eventCmdParam);
            }
            D_8009ABF4.movieCommandState = MOVCMD_ACTIVE;
            g_FieldMoviePlayed = 1;
        }
        if ((s16)g_FieldMovieStreamActive == 1) {
            g_FieldMovieStreamDone = 1;
            g_FieldMovieStreamActive = 0;
            g_FieldMoviePlayed = 0;
            D_8009ABF4.movieCommandState = MOVCMD_DONE;
        }
        break;
    case 0xA:
        if (D_8009ABF4.eventCmd == EVTCMD_LOAD_MOVIE) {
            D_8009ABF4.movieCommandState = MOVCMD_DONE;
        }
        if (D_8009ABF4.eventCmd == EVTCMD_PLAY_MOVIE) {
            D_8009ABF4.movieCommandState = MOVCMD_ACTIVE;
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
extern s16 D_800E42EE[0x40][12];

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
        if (D_800E42EE[i][0] == 1) {
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
    D_80071C20++;
    return g_RandomTable[D_80071C20];
}

extern s8 D_800716D0;
extern u16 D_8007173C;
extern /*?*/ s32 D_80074F14;
extern s16 D_8007E774;
extern s8 D_8007EBC8;
extern s16 D_8009ABF6;
extern u8 D_8009AC30;
extern u8 D_8009C6D8;

/* Check for a random or scripted battle this frame: roll the encounter, pick
 * the battle from the field's encounter table, and kick off the transition if
 * one triggers. m2c seed; residual is the encounter-table regalloc and the
 * divide scheduling. Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldBattleCheck);
#else
void FieldBattleCheck(void) {
    s16 temp_v0;
    s16 var_v0;
    s32 temp_s0;
    s32 temp_v1;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s0_3;
    s32 var_s1;
    s32 var_v0_2;
    s32 var_v0_3;
    u16 temp_a0;
    u16 temp_a1;
    u16 temp_v1_2;
    u16 temp_v1_3;
    u32 temp_a0_2;
    u32 temp_a2;
    u32 temp_a2_2;
    u32 temp_a2_3;

    if (D_8009AC30 == 0) {
        var_s1 = g_FieldEncounters;
    } else {
        var_s1 = g_FieldEncounters + 0x18;
    }
    D_8009C6D8 += 0x20;
    if (D_8009C6D8 == 0) {
        func_800262D8();
        Savemap.memory_bank_4[6].unk0 = (u8)(Savemap.memory_bank_4[6].unk0 + 1);
        if ((Savemap.memory_bank_4[6].unk0 == 0) &&
            (Savemap.memory_bank_4[6].unk1 != 0xFF)) {
            Savemap.memory_bank_4[6].unk1 =
                (u8)(Savemap.memory_bank_4[6].unk1 + 1);
        }
        temp_a0 = var_s1->unk0;
        if ((temp_a0 & 1) && (g_FieldMovieStreamActive == 0) &&
            (D_8009AC2F == 0)) {
            D_8007173C += (s32) * (&D_80074F14 + (g_PlayerModelId * 0x84)) /
                          (s32)(temp_a0 >> 8);
            if ((u32)(FieldGetRandomU8FromList() & 0xFF) <
                (u32)(D_80062F1B & 0x7F)) {
                D_800716D0 = 4;
            } else {
                D_800716D0 = 0;
            }
            if ((u32)(FieldGetRandomU8FromList() & 0xFF) <
                (u32)((u32)(D_8007173C * D_80062F19) >> 0xC)) {
                StopFieldMapPreload();
                D_8009ABF5 = 2;
                D_8007EBC8 = 1;
                temp_a0_2 = (u32)(FieldGetNextRandomU8() & 0xFF) >> 2;
                if (!(D_80062F1B & 0x80)) {
                    var_s0 = (s32)(var_s1->unkE << 0x10) >> 0x1A;
                } else {
                    var_s0 = (s32)(var_s1->unkE << 0x10) >> 0x1B;
                }
                if ((u32)(temp_a0_2 & 0xFF) < (u32)(var_s0 & 0xFF)) {
                    D_800716D0 = 0;
                    var_v0 = var_s1->unkE & 0x3FF;
                    goto block_31;
                }
                if (!(D_80062F1B & 0x80)) {
                    var_v0_2 = (s32)(var_s1->unk10 << 0x10) >> 0x1A;
                } else {
                    var_v0_2 = (s32)(var_s1->unk10 << 0x10) >> 0x1B;
                }
                temp_s0 = var_s0 + var_v0_2;
                temp_a2 = temp_a0_2 & 0xFF;
                if (temp_a2 < (u32)(temp_s0 & 0xFF)) {
                    D_800716D0 = 0;
                    var_v0 = var_s1->unk10 & 0x3FF;
                    goto block_31;
                }
                temp_a1 = var_s1->unk12;
                temp_v1 = temp_s0 + ((s32)(temp_a1 << 0x10) >> 0x1A);
                if (temp_a2 < (u32)(temp_v1 & 0xFF)) {
                    D_8009ABF6 = temp_a1 & 0x3FF;
                    return;
                }
                if (!(D_80062F1B & 0x80)) {
                    var_v0_3 = (s32)(var_s1->unk14 << 0x10) >> 0x1A;
                } else {
                    var_v0_3 = (s32)(var_s1->unk14 << 0x10) >> 0x1B;
                }
                if ((u32)(temp_a0_2 & 0xFF) <
                    (u32)((temp_v1 + var_v0_3) & 0xFF)) {
                    var_v0 = var_s1->unk14 & 0x3FF;
                block_31:
                    D_8009ABF6 = var_v0;
                    return;
                }
                var_s0_2 = 0;
                var_a0 = 0;
                temp_a2_2 = (u32)(FieldGetNextRandomU8() & 0xFF) >> 2;
                var_a1 = var_s1;
                D_8009ABF6 = var_s1->unkC & 0x3FF;
            loop_34:
                temp_v1_2 = var_a1->unk2;
                var_s0_2 += (s32)(temp_v1_2 << 0x10) >> 0x1A;
                if (temp_a2_2 >= (u32)(var_s0_2 & 0xFF)) {
                    var_a0 += 1;
                    var_a1 += 2;
                    if (var_a0 < 5) {
                        goto loop_34;
                    }
                } else {
                    D_8009ABF6 = temp_v1_2 & 0x3FF;
                }
                if (D_8009ABF6 != D_8007E774) {
                    D_8007E774 = D_8009ABF6;
                    return;
                }
                var_s0_3 = 0;
                var_a0_2 = 0;
                temp_a2_3 = (u32)(FieldGetNextRandomU8() & 0xFF) >> 2;
                var_a1_2 = var_s1;
                D_8009ABF6 = var_s1->unkC & 0x3FF;
            loop_40:
                temp_v1_3 = var_a1_2->unk2;
                var_s0_3 += (s32)(temp_v1_3 << 0x10) >> 0x1A;
                var_a0_2 += 1;
                if (temp_a2_3 >= (u32)(var_s0_3 & 0xFF)) {
                    var_a1_2 += 2;
                    if (var_a0_2 >= 5) {

                    } else {
                        goto loop_40;
                    }
                } else {
                    temp_v0 = temp_v1_3 & 0x3FF;
                    D_8009ABF6 = temp_v0;
                    D_8007E774 = temp_v0;
                }
            }
        }
    }
}
#endif

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

extern u16 D_8011446C;

/* Project the field-exit-arrow marker positions (the per-trigger 3D points)
 * through the camera and add a sprite packet for each visible one to the OT,
 * in both the trigger-arrow and the field-exit sets. m2c seed; the residual is
 * the OT-link store scheduling and the GTE RotTransPers arg setup. Codegen
 * pinned via MASPSX_OVERRIDE pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldArrowsAddToRender);
#else
void FieldArrowsAddToRender(void* arg0, MATRIX* arg1, s32 arg2) {
    u16 sp10;
    u16 sp12;
    u16 sp14;
    s32 sp18;
    s32 sp1C;
    s16 var_s4;
    s16 var_s4_2;
    s16 var_v0;
    s16 var_v0_3;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a2;
    s32 temp_s0;
    s32 temp_s0_2;
    s32 temp_s3;
    s32 var_v0_2;
    s32 var_v1;
    void* temp_a0;
    void* temp_a1_2;
    void* temp_s0_3;
    void* temp_s1;
    void* temp_v1;

    if (((*g_FieldExitArrowState == 1) && (g_FieldAnimLock == 0)) ||
        (*g_FieldExitArrowState == 2)) {
        var_s4 = 0;
        PushMatrix();
        SetRotMatrix(arg1);
        SetTransMatrix(arg1);
        var_v1 = 0 << 0x10;
        do {
            temp_s0 = var_v1 >> 0x10;
            var_v0 = var_s4 + 1;
            if (((g_FieldTriggers + temp_s0)->unk218 == 1) &&
                ((temp_a0 = (temp_s0 * 0x18) + arg2,
                  temp_a1 = (s32)(temp_a0->unk0 + temp_a0->unk6) / 2,
                  sp10 = (u16)temp_a1,
                  temp_a2 = (s32)(temp_a0->unk2 + temp_a0->unk8) / 2,
                  sp12 = (u16)temp_a2,
                  sp14 = (u16)((s32)(temp_a0->unk4 + temp_a0->unkA) / 2),
                  ((temp_a1 << 0x10) != 0)) ||
                 (var_v0 = var_s4 + 1, ((temp_a2 << 0x10) != 0)))) {
                RotTransPers((SVECTOR*)&sp10, (s32*)&sp10, &sp18, &sp1C);
                temp_a0_2 = temp_s0 * 0x10;
                temp_a1_2 = temp_a0_2 + arg0;
                temp_a1_2->unk400D = 0xD0;
                temp_a1_2->unk400C = (s8)(((D_8011446C * 4) & 0x30) + 0x30);
                temp_a1_2->unk4008 = (s16)(sp10 - 7);
                temp_a1_2->unk400A = (s16)(sp12 - 8);
                temp_a1_2->unk4000 = (s32)((temp_a1_2->unk4000 & 0xFF000000) |
                                           (arg0->unk0 & 0xFFFFFF));
                arg0->unk0 = (s32)((arg0->unk0 & 0xFF000000) |
                                   ((arg0 + (temp_a0_2 + 0x4000)) & 0xFFFFFF));
                var_v0 = var_s4 + 1;
            }
            var_s4 = var_v0;
            var_v1 = var_s4 << 0x10;
        } while (var_v0 < 0xC);
        var_s4_2 = 0;
        var_v0_2 = 0 << 0x10;
        do {
            temp_s0_2 = var_v0_2 >> 0x10;
            temp_s3 = temp_s0_2 * 0x10;
            temp_v1 = g_FieldTriggers + temp_s3;
            var_v0_3 = var_s4_2 + 1;
            if (temp_v1->unk230 != 0) {
                sp10 = temp_v1->unk224;
                sp12 = temp_v1->unk228;
                sp14 = temp_v1->unk22C;
                RotTransPers((SVECTOR*)&sp10, (s32*)&sp10, &sp18, &sp1C);
                temp_s1 = temp_s3 + arg0;
                temp_s1->unk40CD = 0xD0;
                temp_s1->unk40CC = (s8)(((D_8011446C * 4) & 0x30) + 0x30);
                temp_s0_3 = arg0 + ((temp_s0_2 + 0xC) * 0x10);
                temp_s0_3->unk4008 = (s16)(sp10 - 7);
                temp_s0_3->unk400A = (s16)(sp12 - 8);
                if ((g_FieldTriggers + temp_s3)->unk230 == 2) {
                    temp_s0_3->unk400E = GetClut(0x100, 0x1E8);
                }
                temp_s1->unk40C0 = (s32)((temp_s1->unk40C0 & 0xFF000000) |
                                         (arg0->unk0 & 0xFFFFFF));
                arg0->unk0 =
                    (s32)((arg0->unk0 & 0xFF000000) |
                          ((s32)(arg0 + (temp_s3 + 0x40C0)) & 0xFFFFFF));
                var_v0_3 = var_s4_2 + 1;
            }
            var_s4_2 = var_v0_3;
            var_v0_2 = var_s4_2 << 0x10;
        } while (var_v0_3 < 0xC);
        PopMatrix();
        arg0->unk4180 =
            (s32)((arg0->unk4180 & 0xFF000000) | (arg0->unk0 & 0xFFFFFF));
        arg0->unk0 = (s32)((arg0->unk0 & 0xFF000000) |
                           ((s32)(arg0 + 0x4180) & 0xFFFFFF));
        D_8011446C += 1;
    }
}
#endif

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

/* Header of the shared field-model texture block at *D_800DFCA0. */
typedef struct {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u16 numPages;   // 0x200-byte texture pages
    /* 0x6 */ u16 numCluts;   // 0x20-byte CLUTs
    /* 0x8 */ u32 pageOffset; // offset of the pages within the block
    /* 0xC */ u32 clutOffset; // offset of the CLUTs within the block
} FieldTexBlockHeader;

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

extern FieldTexBlockHeader* D_800DFCA0;
extern u8* D_800E0200;
extern u8* D_800E0204;

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
 * before the scratch pointer costs 5. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field2", LoadLocalFieldModelAndInitAll);
#else
u8* LoadLocalFieldModelAndInitAll(
    FieldModelFileDesc* desc, FieldModelData* data, u8* readFromCd, u32* buf) {
    MATRIX mtx;
    RECT rect;
    FieldModelLoaderData* models;
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
    fileInfo = *(u32**)scratch;
    if (*readFromCd != 0) {
        DS_read(fileInfo[0], fileInfo[1], buf, NULL);
        while (SystemCdromReadChain() != 0) {
        }
    } else {
        s = buf;
        d = (u32*)D_800E0204;
        words = (buf[0] >> 2) + ((buf[0] & 3) != 0);
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
        dm[0] = sm[0];
        dm[1] = sm[1];
        dm[2] = sm[2];
        dm[3] = sm[3];
        dm[4] = sm[4];
        dm[5] = sm[5];
        dm[6] = sm[6];
        dm[7] = sm[7];
        dm[8] = sm[8];
        dm[9] = sm[9];
        dm[10] = sm[10];
        dm[11] = sm[11];
        sm += 12;
        dm += 12;
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

extern u8* FieldModelCreatePktsForPart(u8* part, u8* pkts, s32 arg2, s32 arg3);
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
        pkts = FieldModelCreatePktsForPart(&parts[i * 32], pkts, 0, arg2);
    }
    FieldModelScaleModel(model, model->scale, 0);
    return pkts;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field2", FieldModelCreatePktsForPart);

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
 * 22 rows, down from 44 changed / 7 inserted. Four of the corrections were
 * program, not codegen, and each is readable straight off the target:
 *   - the guard is `count == 0`, not `count <= 0` (one `beqz`, no `blez`);
 *   - the record is indexed, `rec[i].f`, not walked with `rec += 0x14` — the
 *     walking form makes `rec` a biv and costs the reduced base (see the
 *     FieldModelLoadBcx bullet in CLAUDE.md);
 *   - the struct is five words, not four. A four-word TdbRecord makes `rec[i]`
 *     stride 0x10 while the target's scaled index is 0x14, and every field
 *     offset in the diff still looks right because the *first* record's are;
 *   - op 0's destination is absolute (`(u8*)rec[i].dst`), not `tdb + off`, and
 *     op 3's RECT lives inside the record: the target sets a0 = &rec->dst in
 *     the `beq v1,v0,<case3>` delay slot, which no `tdb + off` spelling
 *     reaches.
 * The residue is one register tie-break in the case-1/case-2 arms. Target:
 *   lw a0,%lo(D_800DFCA0) / lhu v0,4(a0) / lw v0,8(a0) / nop / addu v0,a0,v0
 *   / lw a0,4(s0)
 * ours holds `block` in $a1 and issues the `lw a0,4(s0)` (srcOff) *before* the
 * addu, filling the load-delay slot the target leaves as a nop. Rejected, all
 * measured: `block` declared after `rec` (22, identical); `block` hoisted to a
 * single assignment before the loop (47/4); `block` assigned inside the
 * guarded arm (28/3); the memcpy destination named in a `u8* dst` local so the
 * dest expression is complete before the source is loaded (56/1). The nop is
 * the tell — the target has nothing to put there, so srcOff is not yet live,
 * which no reordering of these four statements produces.
 * Codegen pinned via MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldModelBsxTdbModify);
#else
void FieldModelBsxTdbModify(u8* tdb) {
    FieldTexBlockHeader* block;
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
            block = (FieldTexBlockHeader*)D_800DFCA0;
            if (rec[i].dst < block->numPages) {
                memcpy((u8*)block + block->pageOffset + (rec[i].dst << 9),
                       tdb + rec[i].srcOff, 0x200);
            }
            break;
        case 2:
            block = (FieldTexBlockHeader*)D_800DFCA0;
            if (rec[i].dst < block->numCluts) {
                memcpy((u8*)block + block->clutOffset + (rec[i].dst << 5),
                       tdb + rec[i].srcOff, 0x20);
            }
            break;
        case 3:
            LoadImage((RECT*)&rec[i].dst, (u_long*)(tdb + rec[i].srcOff));
            break;
        }
    }
}
#endif

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
 * The residue is one allocation tie-break and its cascade. The target puts
 * `next` in $a1 -- the register `data` arrives in -- and copies `data` to $t0
 * at entry (`move t0,a1`, then `addu a1,a1,v0` computing next from the still
 * live incoming value); ours keeps `data` in $a1 and gives `next` $t0, which
 * renames roughly a dozen rows and costs the entry copy. Since the function is
 * a leaf, both pseudos are equally coalescable onto the argument register and
 * the winner is global-alloc priority (log2(refs)*refs/live_length), where the
 * two are close. Rejected, all measured: `next = (u8*)data;` split from the
 * `+=` (27/4); the same split hoisted to the top of the function (26/2); `next`
 * declared first among the locals (23/2, identical); computing `next` before
 * the four header stores (23/2, identical); a separate counter for the second
 * loop (40/1 -- it also loses the `move a2,a0` giv init, which is the other
 * inserted row: gcc cannot prove the counter is 0 at the second preheader
 * because a CODE_LABEL sits between the reset and the loop).
 * Codegen pinned via MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field2", FieldModelStructInit);
#else
void* FieldModelStructInit(FieldModelFileDesc* desc, FieldModelData* data) {
    FieldModelLoaderData* models;
    FieldModelEntry* entry;
    u8* next;
    u32 i;
    s16 partsOff;
    u8 unusedLocals[0x10];

    i = 0;
    data->modelCount = 0;
    models = desc->models;
    if (desc->count != 0) {
        do {
            if (models[i].npcFlag != 0) {
                models[i].modelEntryIndex = data->modelCount;
                data->modelCount = data->modelCount + 1;
            } else {
                models[i].modelEntryIndex = 0xFF;
            }
            i += 1;
        } while (i < desc->count);
        i = 0;
    }
    data->unk2 = 0;
    data->unk1 = 0;
    data->modelEntries = (FieldModelEntry*)((u8*)data + 0xC);
    data->unk8 = 0;
    next = (u8*)data + ((data->modelCount * 0x24) + 0xC);
    if (desc->count != 0) {
        do {
            if (models[i].npcFlag != 0) {
                if (((u32)(*(u8*)&models[i].globalModelId - 1) < 9) &&
                    (models[i].animationCount < 3)) {
                    models[i].animationCount = 3;
                }
                entry = &data->modelEntries[models[i].modelEntryIndex];
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
                entry->modelData = next;
                entry->partMatrices = NULL;
                entry->animationOffset = partsOff + (models[i].partCount << 5);
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
