//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 2 of 5, split out of field.c. .rodata 0x800A0054-0x800A00E0, base 4 mod
 * 8 -> --phase 4. Every jump table here sits at an address 4 mod 8; `.align 3`
 * would push it 4 bytes forward. Do not merge this unit into a 0 mod 8
 * neighbour. */

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBGScrollInit);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldCalcPointOnLine);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBGScrollUpdate);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityCheckTalk);

s16 FieldEntityGetDirVectorX(u8 arg0) { return D_800DF120[arg0][0]; }

s16 FieldEntityGetDirVectorY(u8 arg0) { return D_800DF120[arg0][1]; }

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityDirByVec);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityWalkmechCross);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityLineCheck);

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
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityTriggerCheck);

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
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelLoadAndInit);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBattleCheck);

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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldArrowsAddToRender);

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
