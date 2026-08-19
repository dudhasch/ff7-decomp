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
 * Instructions all match; the only diff is the jump table landing at
 * .rodata+0x54 (target) vs +0x58 (ours) — the field overlay's .rodata base is
 * 4 mod 8 and this function needs the --phase 4 jump-table demotion, but the
 * overlay carries both phases at once (the file-split residue). Codegen pinned
 * via MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldBGScrollInit);
#else
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
    case 7:
    case 8:
    case 9:
        D_8009A100 = 1;
        D_80075CF8 = 0;
        D_8009AC13 = 1;
        D_8009C558 = D_8009AC14;
        D_80075E14 = D_80071E38;
        D_80075E1C = D_80071E3C;
        D_80075E18 = D_8009ABFE;
        D_80075E20 = D_8009AC00;
        break;
    }
}
#endif

/* Project a point onto a trigger line: for a type-1 or type-2 trigger compute
 * the closest on-line point to the entity and write it back into arg1. m2c
 * seed; the residual is regalloc across the divide-by-length-squared and the
 * two near-duplicate branches. Codegen pinned via MASPSX_OVERRIDE pending a
 * permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldCalcPointOnLine);
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
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldBGScrollUpdate);
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBGUpdateDrawenv);

/////////////////////////////////////////////////
// Begin of field_entity.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityInitPos);

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
 * Not matching: the original reserves an unused 8-byte frame, and its clamp
 * arm keeps animLastFrame live across the branch to recompute `* 16` in the
 * delay slot where gcc reuses the already-shifted value. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityAnimationUpdate);
#else
void FieldEntityAnimationUpdate(s32 entityId) {
    FieldModelEntry* model;
    u8* anims;
    u8 entryIndex;

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
            g_FieldEntity[entityId].animLastFrame * 16;
    }
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityMovementUpdate);

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
 * script of the best candidate. Verified C kept as the #else; codegen pinned
 * via MASPSX_OVERRIDE (the walking quality-array pointer regalloc wall). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityCheckTalk);
#else
void FieldEntityCheckTalk(void) {
    VECTOR from;
    VECTOR to;
    s16 quality[16];
    s32 sqrDist;
    s16 best;
    u16 bestId;
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
        if ((u8)(g_FieldEntity[g_PlayerModelId].Dir - dirTo) >= 0x81) {
            quality[i] =
                0x100 - (u8)(g_FieldEntity[g_PlayerModelId].Dir - dirTo);
        } else {
            quality[i] = (u8)(g_FieldEntity[g_PlayerModelId].Dir - dirTo);
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

/* Direction (0-255) from one point to another, plus the squared distance.
 * Computes the fixed-point slope of the dominant axis, looks up the angle in
 * the arctan table D_800DEF88, and corrects for the quadrant. The two hardware
 * divisions and the quadrant branch ladder are the wall; codegen pinned via
 * MASPSX_OVERRIDE, #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityDirByVec);
#else
u8 FieldEntityDirByVec(VECTOR* from, VECTOR* to, s32* sqrDist) {
    s32 dx;
    s32 dy;
    s32 dist;
    s32 slope;
    s32 slopeX;
    s32 slopeY;
    u8 angle;

    dx = to->vx - from->vx;
    dy = to->vy - from->vy;
    *sqrDist = dx * dx + dy * dy;
    dist = SquareRoot0(*sqrDist);
    slopeX = (dx << 12) / dist >> 5;
    slopeY = (dy << 12) / dist >> 5;
    if (slopeX * slopeX < slopeY * slopeY) {
        if (slopeY > 0) {
            angle = D_800DEF88[slopeX * 2] + 0x40;
        } else {
            angle = -0x40 - D_800DEF88[-slopeX * 2];
        }
    } else {
        if (slopeX > 0) {
            angle = D_800DEF88[slopeY * 2] - 0x40;
        } else {
            angle = -0x80 - D_800DEF88[-slopeY * 2];
        }
    }
    return angle & 0xFF;
}
#endif

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
extern s32 D_800E4274;
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
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityWalkmechCross);
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityMove);

extern s16 D_8009AC1C;

/* Would `pos` put entity `entityId` inside another solid entity? Two entities
 * collide when their horizontal distance falls under the mean of their two
 * solid radii and they are within ~127 units of each other vertically, so
 * characters on a different floor of the same map never block one another.
 * Only the player's own collisions arm the other entity's push script.
 *
 * Not matching: register assignment only. Every instruction is in the right
 * place, but the original holds &D_8009AC1C in a1 and copies it into t4 for the
 * loop, where gcc keeps one register for both and renames the rest downstream.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityCollisionCheck);
#else
s32 FieldEntityCollisionCheck(s16 entityId, VECTOR* pos) {
    s16 i;
    s32 hit;
    s32 sqrRadius;
    s32 range;
    s16* entityCount;
    s32 dz;
    s32 radius;
    s32 dx;
    s32 dy;

    hit = 0;
    range = g_FieldEntity[entityId].SolidRange;
    entityCount = &D_8009AC1C;
    for (i = 0; i < *entityCount; i++) {
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
#endif

/* Squared distance from `point` to the segment `line`, with the foot of the
 * perpendicular written to `nearest`. Returns -1 when that foot lands outside
 * the segment on either the x or the y axis, which is how callers tell "past
 * the end of the line" apart from "near it". The line parameter runs in 8-bit
 * fixed point, so the projection stays in integer arithmetic throughout.
 *
 * Not matching: register assignment only. Every instruction and its order are
 * right; the original accumulates the dot product in v1 and holds y1 in a0,
 * where gcc picks the two the other way round and the swap propagates. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntitySqrDistToLine);
#else
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
            return t;
        }
    }
    return -1;
}
#endif

/* Walk the map's 32 trigger lines against one entity and raise the script
 * requests each is due. Entering a line's radius arms touch-on (and, if the
 * entity crossed the line this frame and faces it within +/-64, push and
 * isOnLine), leaving arms touch-off. Returns 1 if any line is in range. The
 * crossing test is the four-way sign ladder; the walking flag pointer is the
 * regalloc wall. Codegen pinned via MASPSX_OVERRIDE; the #else is verified C.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityLineCheck);
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
    for (i = 0; i < 32; i++) {
        line = &lines[i];
        if (line->isActive != 1) {
            continue;
        }
        line->isOnLine = 0;
        sqrDist = FieldEntitySqrDistToLine(line, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < entity->SolidRange * entity->SolidRange) {
            hit = 1;
            if (line->touch == 0) {
                line->requestTouchOnScript = 1;
            }
            line->touch = 1;
            crossFrom =
                (line->pos.x2 - line->pos.x1) * (from[1] - line->pos.y1) -
                (from[0] - line->pos.x1) * (line->pos.y2 - line->pos.y1);
            crossTo = (line->pos.x2 - line->pos.x1) * (to[1] - line->pos.y1) -
                      (to[0] - line->pos.x1) * (line->pos.y2 - line->pos.y1);
            if (!((crossFrom >= 0 && crossTo < 0) ||
                  (crossTo >= 0 && crossFrom < 0) ||
                  (crossFrom > 0 && crossTo <= 0) ||
                  (crossTo > 0 && crossFrom <= 0))) {
                line->across = 1;
            }
            if (nearest[0] != from[0] || nearest[1] != from[1]) {
                line->proximityAngle = FieldEntityDirByVec(
                    (VECTOR*)from, (VECTOR*)nearest, &sqrDist);
                if ((u8)(line->proximityAngle - entity->MoveDir + 0x40) >=
                    0x80) {
                    continue;
                }
            } else {
                continue;
            }
            line->requestPushScript = 1;
            line->isOnLine = 1;
        } else {
            if (line->touch == 1) {
                line->requestTouchOffScript = 1;
            }
            line->touch = 0;
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
 * Not matching: register assignment only. The original keeps the walking line
 * pointer in s2 and the constant 1 in s3; gcc allocates them the other way
 * round. Neither declaration order, statement order, nor indexing with
 * `line[i]` instead of a walking pointer shifts the tie. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityLineInteract);
#else
void FieldEntityLineInteract(FieldEntity* entity, FieldLine* line) {
    s32* from;
    s32* nearest;
    s32 i;
    s32 sqrDist;
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
        if ((u8)(line->proximityAngle - entity->MoveDir + 0x20) >= 0x40) {
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
    "asm/us/field/nonmatchings/field", FieldEntityBgTriggerActivate);
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
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEntityTriggerCheck);
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
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldModelLoadAndInit);
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
        g_FieldModelData, D_8007E770, (u8*)D_80075E10, 1);
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", HandleKawaiDataInModel);

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

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldRainUpdate);
#else

extern u8 g_RainControl;
extern s16 g_PlayerModelId;

extern FieldEntity g_FieldEntities[];
extern u8 g_RandomTable[];
extern struct FieldRain g_FieldRain[];

void FieldRainUpdate(void) {
    s32 i;
    s32 limit;
    s32 player;
    s32 max = 255;
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
                    (g_FieldEntities[player].PosX >> 12) +
                    g_RandomTable[g_FieldRain[i].rndSeed & 0xFF] * 12 - 0x600;

                seed3 = g_FieldRain[i].rndSeed * 3;
                g_FieldRain[i].p2.vy = (g_FieldEntities[player].PosY >> 12) +
                                       g_RandomTable[seed3] * 12 - 0x600;

                g_FieldRain[i].p1.vx = g_FieldRain[i].p2.vx;
                g_FieldRain[i].p1.vy = g_FieldRain[i].p2.vy;

                g_FieldRain[i].z = (g_FieldEntities[player].PosZ >> 12) - 0x300;
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
#endif

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
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldBattleCheck);
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
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldArrowsAddToRender);
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", LoadLocalFieldModelAndInitAll);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelCreatePktsForPart);

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
    /* 0x5 */ u8 pad[3];
    /* 0x8 */ BsxTexEntry entries[1];
} BsxTexHeader;

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

/* Header of the shared field-model texture block at *D_800DFCA0. */
typedef struct {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u16 numPages;   // 0x200-byte texture pages
    /* 0x6 */ u16 numCluts;   // 0x20-byte CLUTs
    /* 0x8 */ u32 pageOffset; // offset of the pages within the block
    /* 0xC */ u32 clutOffset; // offset of the CLUTs within the block
} FieldTexBlockHeader;

/* One record of a TDB ("texture delta") chunk inside a BSX model file. */
typedef struct {
    /* 0x00 */ u32 opcode; // 0=memcpy, 1=page patch, 2=CLUT patch, 3=LoadImage
    /* 0x04 */ u32 srcOff; // source RECT (0,3) / pixels (1,2), rel. to tdb
    /* 0x08 */ u32 size;   // memcpy byte count (op 0)
    /* 0x0C */ u32 dstOff; // dest rel. tdb (0) / page idx (1) / CLUT idx (2) /
                           // RECT (3)
} TdbRecord;               // size 0x14

/* Apply a TDB ("texture delta") chunk from a BSX model file. Each record
 * relocates a raw blob (op 0), splices one 0x200-byte page (op 1) or one
 * 0x20-byte CLUT (op 2) into the shared model texture block at *D_800DFCA0, or
 * uploads an embedded image straight to VRAM (op 3). The fixed-size copies are
 * gcc's inlined memcpy expansion (dual lwl/lw form) — a scheduler/expansion
 * coupling. Codegen pinned via MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldModelBsxTdbModify);
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
    if (count <= 0) {
        return;
    }
    rec = (TdbRecord*)(tdb + 8);
    for (i = 0; i < count; i++, rec = (TdbRecord*)((u8*)rec + 0x14)) {
        switch (rec->opcode) {
        case 0:
            memcpy(tdb + rec->dstOff, tdb + rec->srcOff, rec->size);
            break;
        case 1:
            block = (FieldTexBlockHeader*)D_800DFCA0;
            if (rec->dstOff < block->numPages) {
                memcpy((u8*)block + block->pageOffset + (rec->dstOff << 9),
                       tdb + rec->srcOff, 0x200);
            }
            break;
        case 2:
            block = (FieldTexBlockHeader*)D_800DFCA0;
            if (rec->dstOff < block->numCluts) {
                memcpy((u8*)block + block->clutOffset + (rec->dstOff << 5),
                       tdb + rec->srcOff, 0x20);
            }
            break;
        case 3:
            LoadImage((RECT*)(tdb + rec->dstOff), (u_long*)(tdb + rec->srcOff));
            break;
        }
    }
}
#endif

extern s32 D_800E0204;

typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u16 count;
    /* 0x04 */ FieldModelLoaderData models[0]; // variable length
} FieldModelFileDesc;

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldModelStructInit);
#else
/* 65 rows, ALL pure regalloc/stack-frame: every field offset is correct
 * (verified in diff). Target uses a minimal -0x10 frame with NO callee-saved
 * regs (args kept in $t0/$t1); the named-struct body spills 6 saved regs for a
 * -0x38 frame. Two-pass model-file-descriptor -> FieldModelData init. Needs
 * permuter to find the lean local set. models[0] (not [1]) avoids 4-byte
 * alignment padding of the +4 entry array. Added FieldModelData.unk8 for the
 * sw zero,0x8($t0) init. */
void* FieldModelStructInit(FieldModelFileDesc* arg0, FieldModelData* arg1) {
    s16 temp_a0_2;
    u32 var_a3;
    FieldModelLoaderData* temp_a0;
    FieldModelEntry* temp_v1;
    u8* var_a1;
    FieldModelLoaderData* var_a2;
    FieldModelLoaderData* var_v1;

    var_a3 = 0;
    arg1->modelCount = 0;
    temp_a0 = &arg0->models[0];
    if (arg0->count != 0) {
        var_v1 = temp_a0;
        do {
            if (var_v1->npcFlag != 0) {
                var_v1->modelEntryIndex = arg1->modelCount;
                arg1->modelCount = arg1->modelCount + 1;
            } else {
                var_v1->modelEntryIndex = 0xFF;
            }
            var_a3 += 1;
            var_v1 += 1;
        } while (var_a3 < arg0->count);
        var_a3 = 0;
    }
    arg1->unk2 = 0;
    arg1->unk1 = 0;
    arg1->modelEntries = (FieldModelEntry*)((u8*)arg1 + 0xC);
    arg1->unk8 = 0;
    var_a1 = (u8*)arg1 + ((arg1->modelCount * 0x24) + 0xC);
    if (arg0->count != 0) {
        var_a2 = temp_a0;
        do {
            if (var_a2->npcFlag != 0) {
                if (((u32)(var_a2->globalModelId - 1) < 9) &&
                    (var_a2->partCount < 3)) {
                    var_a2->partCount = 3;
                }
                temp_v1 = &arg1->modelEntries[var_a2->modelEntryIndex];
                temp_v1->flags = 1;
                temp_v1->kawaiType = -1;
                temp_v1->boneCount = var_a2->boneCount;
                temp_v1->partCount = var_a2->partCount;
                temp_v1->rotationZ = 0;
                temp_v1->rotationY = 0;
                temp_v1->rotationX = 0;
                temp_v1->translationZ = 0;
                temp_v1->translationY = 0;
                temp_v1->translationX = 0;
                temp_v1->animationCount = var_a2->partCount;
                temp_v1->globalModelId = var_a2->globalModelId;
                temp_v1->scale = 0x1000;
                temp_v1->textureFaceId = var_a2->faceId;
                temp_a0_2 = var_a2->boneCount * 4;
                temp_v1->partsOffset = temp_a0_2;
                temp_v1->modelData = var_a1;
                temp_v1->partMatrices = 0;
                temp_v1->animationOffset = temp_a0_2 + (var_a2->partCount << 5);
                var_a1 += (var_a2->boneCount * 4) + (var_a2->partCount << 5) +
                          (var_a2->animationCount * 0x10);
            }
            var_a3 += 1;
            var_a2 += 1;
        } while (var_a3 < arg0->count);
    }
    D_800E0204 = 0;
    return var_a1;
}
#endif

extern u_long* D_800DFCA0;
u8* FieldModelLoadBcx(FieldModelData* data, s32 arg1, u8* pkts, s32 index);

/* Loads every global (BCX) model in the header, then optionally kicks off the
 * next streamed read. Scratchpad word 0 is clobbered by each load and restored
 * before the next one; word 1 holds the sector/size pair for that read. */
u8* FieldModelLoadGlobalModels(
    FieldModelData* data, s32 arg1, u8* pkts, s32 readFile) {
    u32* fileInfo;
    s32 saved;
    u32 i;

    saved = ((s32*)0x1F800000)[0];
    fileInfo = (u32*)((s32*)0x1F800000)[1];
    for (i = 0; i < data->unk2; i++) {
        ((s32*)0x1F800000)[0] = saved;
        pkts = FieldModelLoadBcx(data, arg1, pkts, i);
    }
    if (readFile) {
        DS_read(fileInfo[0], fileInfo[1], D_800DFCA0, NULL);
        while (SystemCdromReadChain() != 0) {
        }
    }
    return pkts;
}
