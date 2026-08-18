//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 4 of 5, split out of field.c. .rodata 0x800A0104-0x800A0F10, base 4 mod
 * 8 -> --phase 4. The large middle run: 20 of the overlay's jump tables, all 4
 * mod 8. */

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiExecute);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetCustomLightToModelPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetVertexColorFromLighting);

/* Applies one unaligned little-endian u16 per colour channel to every part of
 * the model, and puts the seventh byte in the scratchpad for the handwritten
 * KawaiSetColorToPartPkts to pick up.
 *
 * The twelve bytes of locals are load-bearing, in two separate ways.
 *
 * `unused` is not a variable the original had a name for -- it is 8 bytes of
 * frame that nothing ever stores to, and its only job here is to make the
 * prologue `addiu sp,sp,-0x40` rather than -0x38. gcc counts an aggregate
 * local into the frame whether or not it is read, and a scalar one does not
 * survive at all, so an array is the only way to reproduce it. What the
 * original declared there is unrecoverable from the object code.
 *
 * The six byte reads have to be their own statements, high byte before low
 * byte in each pair, because that is the order the loads issue in. Folding
 * them back into the three ORs leaves the operands right and the schedule
 * wrong; hoisting only data[5] gets every register correct and still emits
 * its load first instead of fifth. */
s32 KawaiSetColorToModelPkts(FieldModelEntry* model, u8* data) {
    u8 unused[8];
    u8* parts;
    u32 count;
    u32 i;
    s32 r;
    s32 g;
    s32 b;
    u32 redLo;
    u32 greenLo;
    u32 blueLo;
    u32 redHi;
    u32 greenHi;
    u32 blueHi;

    count = model->partCount;
    parts = model->modelData + model->partsOffset;
    redHi = data[1];
    redLo = data[0];
    greenHi = data[3];
    greenLo = data[2];
    blueHi = data[5];
    blueLo = data[4];
    r = redLo | (redHi << 8);
    g = greenLo | (greenHi << 8);
    b = blueLo | (blueHi << 8);
    *(u32*)0x1F800200 = data[6];
    for (i = 0; i < count; i++) {
        KawaiSetColorToPartPkts(&parts[i * 32], r, g, b);
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToPartPkts);

/* Load this model's animated eye/mouth textures into VRAM. The face selector
 * (arg1) is four bytes: two mouth frames, one eye frame, and a "has animation"
 * flag; values 0x21+ in the flag mean the model has no animated face and the
 * function is a no-op. Each present variant is looked up in a per-textureFaceId
 * index table (mouth: stride 7, eye: stride 3, 0x7E = none) and the matching
 * 0x200-byte page of the model's texture block is uploaded to its VRAM tile.
 *
 * Semantically right, codegen pinned via MASPSX_OVERRIDE: the verified C is the
 * #else. The target keeps the table base in a callee-saved register and
 * strength-reduces faceId*7 / faceId*3; gcc picks different registers. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", KawaiLoadEyesMouthTexToVram);
#else
extern u8 D_800DFCA4[]; /* mouth texture index table, stride 7 per face */
extern u8 D_800DFD94[]; /* eye texture index table, stride 3 per face */

s32 KawaiLoadEyesMouthTexToVram(FieldModelEntry* model, u8* faceSel) {
    RECT rect;
    u8* texBlock;
    u8* texData;
    s32 n;
    s32 q;
    u8 faceId;

    n = faceSel[3];
    if (n >= 0x21) {
        return 1;
    }
    texBlock = (u8*)D_800DFCA0;
    faceId = model->textureFaceId;

    /* First mouth frame. */
    q = n >> 2;
    rect.x = ((n - q * 4) << 4) + 0x300;
    rect.y = (q << 5) + 0x100;
    rect.w = 8;
    rect.h = 0x20;
    texData = *(u8**)(texBlock + 8);
    LoadImage(
        &rect, (u_long*)(texData + (D_800DFCA4[faceId * 7 + faceSel[0]] << 9)));

    /* Second mouth frame, one tile to the right. */
    rect.x = ((n - q * 4) << 4) + 0x308;
    rect.y = (q << 5) + 0x100;
    rect.w = 8;
    rect.h = 0x20;
    texData = *(u8**)(texBlock + 8);
    LoadImage(
        &rect, (u_long*)(texData + (D_800DFCA4[faceId * 7 + faceSel[1]] << 9)));

    /* Eye frame. */
    q = n >> 3;
    rect.x = ((n - q * 8) << 3) + 0x300;
    rect.y = (q << 5) + 0x1A0;
    rect.w = 8;
    rect.h = 0x20;
    texData = *(u8**)(texBlock + 8);
    LoadImage(
        &rect, (u_long*)(texData + (D_800DFD94[faceId * 3 + faceSel[2]] << 9)));
    return 1;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiLightingApplyToModel);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiLightingApplyToPolyColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetModelTransparency);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToPktsBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToPartPktsBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiFadeModelColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetCustomLighting);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiColorFadeBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetLightingToModelPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetLightingToPartPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetSplashToPktsBelowLvl);

extern s32 D_800E0200;

/* Initialise the two-primitive splash/water GPU packets for one field model.
 * Walks a 0x5C-byte packet per of 31 levels, setting the sprite code, the
 * semi-transparency/shade flags, the CLUT/tpage, and the per-level UV from the
 * model's part data. The $s0 base is computed into the `jal GetGraphType`
 * delay slot and the packet pointer strength-reduces through the loop — a
 * scheduler/strength-reduction coupling gcc cannot reproduce from C. Codegen
 * pinned via MASPSX_OVERRIDE. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", KawaiInitSplashPkts);
#else
void KawaiInitSplashPkts(void* arg0, s32 arg1) {
    s16 clut;
    s32 i;
    u8* pkt;
    u16* parts;
    u8* base;

    base = (u8*)D_800E0200 + arg1 * 0xAC8;
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        clut = 0x22B;
    } else {
        clut = 0x9B;
    }
    i = 1;
    pkt = base + 0x5C;
    parts = (u16*)(*(u32*)((u8*)arg0 + 0x1C) + 4);
    do {
        pkt[0x7] = 0x2C;
        pkt[0x2F] = 0x2C;
        i++;
        pkt[0x3] = 9;
        pkt[0x2B] = 9;
        pkt[0x2E] = 0x80;
        pkt[0x6] = 0x80;
        pkt[0x2D] = 0x80;
        pkt[0x5] = 0x80;
        pkt[0x2C] = 0x80;
        pkt[0x4] = 0x80;
        *(s16*)(pkt + 0x36) = 0x6C2C;
        *(s16*)(pkt + 0xE) = 0x6C2C;
        *(s16*)(pkt + 0x3E) = clut;
        *(s16*)(pkt + 0x16) = clut;
        *(s16*)(pkt + 0x50) = 0;
        *(s16*)(pkt + 0x52) = 0;
        *(s16*)(pkt + 0x54) = 0;
        pkt[0x7] |= 2;
        pkt[0x2F] |= 2;
        *(s16*)(pkt + 0x5A) = 0;
        *(s16*)(pkt + 0x58) = -*(s16*)parts;
        parts += 2;
        pkt += 0x5C;
    } while (i < 0x1F);
}
#endif

s32 KawaiSetPartAttribute(FieldModelEntry* model, u8* data) {
    u8* parts;
    s32 count;
    s32 i;
    s32 partIdx;

    count = data[0];
    if (count > 0) {
        parts = model->modelData + model->partsOffset;
        for (i = 0; i < count; i++) {
            partIdx = data[i * 2 + 1];
            if (partIdx < model->partCount) {
                parts[partIdx * 32] = data[i * 2 + 2];
            }
        }
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiApplyBoneTransform);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiRenderClippedPart);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiDirectionalColorGradient);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiGradientColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiAnimatedPointLight);

/////////////////////////////////////////////////
// Begin of field_event.c
/////////////////////////////////////////////////

extern u8 D_800716D4;
void FieldWindowResetAll(void);
void FieldInitDefaultValues(void);
void FieldEventRunInit(void);

/* Installs the field's state, model and script pointers, checks the script
 * header's version bytes, then brings the event system up. */
void FieldEventInit(
    FieldState* state, FieldEntity* models, FieldScriptHeader* scripts) {
    s32 flags;

    /* The high half of FieldState's 0x68 word. The low half is the
     * controller-1 key bits (see OpcodeFuncKeyEx, which matches against
     * activeKeys as a u32), so this cannot be a named field without splitting
     * that member. Widening to s32 is what makes the load lh rather than lhu:
     * held in an s16 the value is only ever masked, and gcc narrows it. */
    flags = *(s16*)((u8*)state + 0x6A);
    g_FieldState = state;
    g_FieldModels = models;
    g_FieldScripts = scripts;
    D_80095DCC = 0;
    D_8007EBE0 = 1;
    D_8009FE8C = 0;
    if (flags & 0x100) {
        D_80095DCC = 1;
        g_FieldScriptRunState = 4;
    }
    if (scripts->eventDataVersion < 2) {
        SystemError('K', 10);
    }
    if (scripts->eventDataVersion > 2 || scripts->eventVersion > 5) {
        SystemError('K', 12);
    }
    if (scripts->eventVersion < 5) {
        SystemError('K', 11);
    }
    FieldWindowResetAll();
    FieldInitDefaultValues();
    FieldEventRunInit();
    if (D_800716D4 == 0) {
        FieldEventClearAkaoStruct();
        *D_8009A000 = 0xF2;
        SystemAkaoExecute();
    }
}

void InitFieldDebugPages(void);
void FieldEventUpdate(s32 arg0) {
    if (D_8007EBE0) {
        FieldWindowResetTextAll();
        ResetFieldRenderState();
        FieldDebugInitBuffers();
        InitFieldDebugPages();
        D_80095DCC = 0;
        D_8009FE8C = 0;
        D_8007EBE0 = 0;
        if (g_FieldScripts->eventVersion < 5) {
            SystemError('K', 11);
        }
        if (g_FieldScripts->eventDataVersion < 2) {
            SystemError('K', 10);
        }
        if (g_FieldScripts->eventDataVersion > 2 ||
            g_FieldScripts->eventVersion > 5) {
            SystemError('K', 12);
        }
    }
    if (g_FieldScriptRunState != 4) {
        if (g_FieldScriptRunState != 5 || D_80070788 != 0) {
            FieldEventOpcodeCycle();
        }
    }
    if (g_WindowCount) {
        SystemMenuDrawDialog(
            g_WindowData, 4, arg0, g_FieldState->renderBuffer ^ 1);
    }
    UpdateFieldExitArrows(arg0);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldInitDefaultValues);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventRunInit);

/* Enable the loaded field models that correspond to party members, then
 * disable (make non-solid, non-talkable, invisible) every model whose loader
 * slot was not claimed. Codegen pinned via MASPSX_OVERRIDE: the #else body is
 * the verified-correct C; its bytes come from the reference .s (the
 * s16-walking-counter strength-reduction wall). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEnablePartyModels);
#else
void FieldEnablePartyModels(void) {
    s16 i;
    s16 j;
    s16 modelCount;
    u8 charId;
    u8 modelId;

    /* Mark the loader slot of each present party member's model as an NPC. */
    for (i = 0; i < 3; i++) {
        charId = Savemap.memory_bank_2[9 + i];
        if (charId == 0xFF) {
            continue;
        }
        modelId = g_CharIdToEntity[charId];
        if (modelId == 0xFF) {
            continue;
        }
        if (g_EntityToModel[modelId] == 0xFF) {
            continue;
        }
        if (g_EntityToModel[modelId] <
            ((FieldModelFileDesc*)D_8007E770)->count) {
            g_FieldModelLoaderData[g_EntityToModel[modelId]].npcFlag = 1;
        }
    }

    /* Disable every model whose loader slot was not claimed above. */
    modelCount = ((FieldModelFileDesc*)D_8007E770)->count;
    if (modelCount != 0) {
        for (i = 0; i < modelCount; i++) {
            if (g_FieldModelLoaderData[i].npcFlag == 0) {
                if (i < g_FieldScripts->numModels) {
                    for (j = 0; j < g_FieldScripts->numModels; j++) {
                        if (g_EntityToModel[j] == i) {
                            g_EntityToModel[j] = 0xFF;
                            g_FieldModels[i].visible = 0;
                            g_FieldModels[i].SolidOff = 1;
                            g_FieldModels[i].TalkOff = 1;
                        }
                    }
                }
            }
        }
    }
}
#endif

// Inline as empty string when more is decompiled. Checksum fails now.
const char D_800A013C[8] = {0};

void FieldEventOpcodeCycle(void) {
    s32 i, j, count;
    u16 hours, seconds;
    s32 talkDone = 0;

    // Update display values for play time and countdown.
    hours = Savemap.time / 3600;
    if (hours > 255) {
        hours = 255;
    }
    Savemap.memory_bank_1[16] = hours;
    hours = Savemap.time % 3600;
    Savemap.memory_bank_1[17] = hours / 60;
    seconds = hours % 60;
    if (Savemap.memory_bank_1[18] != seconds) {
        Savemap.memory_bank_1[18] = seconds;
        Savemap.memory_bank_1[19] = 0;
    } else {
        Savemap.memory_bank_1[19]++;
    }

    hours = Savemap.countdown_timer_seconds / 3600;
    if (hours > 255) {
        hours = 255;
    }
    Savemap.memory_bank_1[20] = hours;
    hours = Savemap.countdown_timer_seconds % 3600;
    Savemap.memory_bank_1[21] = hours / 60;
    seconds = hours % 60;
    if (Savemap.memory_bank_1[22] != seconds) {
        Savemap.memory_bank_1[22] = seconds;
        Savemap.memory_bank_1[23] = 30;
    } else if (Savemap.memory_bank_1[23]) {
        Savemap.memory_bank_1[23]--;
    }

    count = g_FieldScripts->numModels;
    for (i = 0; i < count; i++) {
        if (g_FieldModels[i].requestTalkScript) {
            if (!g_FieldState->characterLock && !talkDone) {
                FieldEventRequestRun(g_FieldModels[i].entityId, 1, 1);
                talkDone = 1;
            }
            g_FieldModels[i].requestTalkScript = 0;
        }
        if (g_FieldModels[i].requestPushScript) {
            FieldEventRequestRun(g_FieldModels[i].entityId, 1, 2);
            g_FieldModels[i].requestPushScript = 0;
        }
    }
    for (i = 0; i < g_FieldLineCount; i++) {
        if (g_FieldLines[i].requestTalkScript) {
            if (!g_FieldState->characterLock) {
                FieldEventRequestRun(g_FieldLines[i].entityId, 1, 1);
            }
            g_FieldLines[i].requestTalkScript = 0;
        }
        if (g_FieldLines[i].requestPushScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 2);
            g_FieldLines[i].requestPushScript = 0;
        }
        if (g_FieldLines[i].across) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 3);
            g_FieldLines[i].across = 0;
        }
        if (g_FieldLines[i].requestTouchOnScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 5);
            g_FieldLines[i].requestTouchOnScript = 0;
        }
        if (g_FieldLines[i].requestTouchOffScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 6);
            g_FieldLines[i].requestTouchOffScript = 0;
        }
        if (g_FieldLines[i].touch) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 4);
        }
    }

    // Loop through all entities in field map and execute up to 8 opcodes of
    // each entity's active script.
    count = g_FieldScripts->numEntities;
    do {
        if (g_CurrentEntity >= g_FieldScripts->numEntities) {
            g_CurrentEntity = 0;
        }
        if (g_FieldScriptDebugFlags & 3) {
            DebugUpdateActor(4, g_CurrentEntity);
        }

        // Skip entities involved in a split or join animation
        // (g_EntitySplitJoinState[entity] != 0) except the entity they're
        // splitting from or joining to (g_EntityForSplitJoin).
        if (g_EntitySplitJoinState[g_CurrentEntity] == 0 ||
            g_EntityForSplitJoin == g_CurrentEntity) {
            for (j = 8; j != 0; j--) {
                if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                    (!(g_FieldScriptDebugFlags & 4) ||
                     g_FieldScriptDebugEntities[g_CurrentEntity])) {
                    for (i = 1; i < 9; i++) {
                        SetStrToDebugRow(3, i, D_800A013C);
                    }
                }
                g_FieldCurrentOpcode =
                    ((u8*)g_FieldScripts)[g_FieldScriptPC[g_CurrentEntity]];

                // Script can yield early if opcode returns 1.
                if (g_FieldOpcodes[g_FieldCurrentOpcode]()) {
                    if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                        (!(g_FieldScriptDebugFlags & 4) ||
                         g_FieldScriptDebugEntities[g_CurrentEntity])) {
                        g_CurrentEntity++;
                        goto done;
                    }
                    break;
                }
                if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                    (!(g_FieldScriptDebugFlags & 4) ||
                     g_FieldScriptDebugEntities[g_CurrentEntity])) {
                    if (++D_8009A064 >= 8) {
                        D_8009A064 = 0;
                        g_CurrentEntity++;
                    }
                    goto done;
                }
            }
        }
        g_CurrentEntity++;
        count--;
        if (g_FieldScriptRunState == 5 && g_FieldScriptDebugFlags & 1 &&
            (!(g_FieldScriptDebugFlags & 4) ||
             g_FieldScriptDebugEntities[g_CurrentEntity])) {
            break;
        }
    } while (count != 0);

done:
    if (g_FieldScriptRunState == 5) {
        D_80070788 = 0;
    }
    FieldUpdateAnimationState();
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldUpdateAnimationState);

u8 FieldEventRequestRun(s16 entityId, s16 priority, s16 scriptId) {
    u16 offset;
    s32 scriptOffset;
    s32 entityDataSize;
    s32 extrasHeaderSize;

    if (g_DebugLevel & 3) {
        switch (scriptId) {
        case 1: // Pressed OK.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Talk=");
            break;
        case 2: // Pushed / within entity's collision radius.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Push=");
            break;
        case 3: // Across line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Acrs=");
            break;
        case 4: // Touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Toch=");
            break;
        case 5: // Started touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "TochON =");
            break;
        case 6: // Ended touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "TochOFF=");
            break;
        }
        // Prints entity name.
        FieldDebugStringConcat(
            g_DebugMessageBuffer,
            (char*)g_FieldScripts + sizeof(FieldScriptHeader) + entityId * 8);
        FieldDebugAddParseValueToPage2(g_DebugMessageBuffer, 0, 0);
    }

    // Only request script if active script has lower priority.
    if (g_FieldScriptPriority[entityId] > priority) {

        // Entity is busy waiting for another script to return.
        if (g_FieldScriptSyncState[entityId][priority] != SYNC_NONE) {
            return g_FieldScriptSyncState[entityId][priority];
        }

        scriptOffset = scriptId * 2;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);
        entityDataSize = entityId * 64;
        entityDataSize += g_FieldScripts->numEntities * 8;

        offset = *((u8*)(scriptOffset + entityDataSize + extrasHeaderSize +
                         (s32)g_FieldScripts) +
                   sizeof(FieldScriptHeader));
        offset |=
            *((u8*)(scriptOffset + (entityDataSize + (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        // Empty event scripts consist of just a RET (0x00) opcode.
        if (((u8*)g_FieldScripts)[offset] != 0) {

            // Save position of current active script of lower priority and
            // replace with new script.
            SavedScriptIds[entityId][priority] = scriptId;
            g_SavedFieldScriptPC[entityId][g_FieldScriptPriority[entityId]] =
                g_FieldScriptPC[entityId];
            g_FieldScriptPC[entityId] = offset;
            g_FieldScriptPriority[entityId] = priority;

            // Clear running animation if entity has a model.
            if (g_EntityToModel[entityId] != 0xFF) {
                if (g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode ==
                    SMODE_WALK) {
                    g_FieldModels[g_EntityToModel[entityId]].activeAnimId = 0;
                    g_FieldModels[g_EntityToModel[entityId]].animCurrentFrame =
                        0;
                    g_FieldModels[g_EntityToModel[entityId]].animLastFrame = 0;
                }
                g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode =
                    SMODE_NONE;
            }

            // Reset wait counter.
            g_FieldWaitCounter[entityId] = 0;

            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("=recieved", 0, 0);
            }
        } else {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("=ret", 0, 0);
            }
        }
        return 1;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("=ignored", 0, 0);
    }
    return 0;
}

void ResetFieldRenderState(void) {
    s16 tpage;

    D_80114490 = 0;
    D_80114464 = 0x7FFF;
    D_80114468 = 0x7FFF;
    setPolyFT4(&D_800E48F4[0]);
    setPolyFT4(&D_800E48F4[1]);
    setSemiTrans(&D_800E48F4[0], 0);
    setSemiTrans(&D_800E48F4[1], 0);
    setShadeTex(&D_800E48F4[0], 1);
    setShadeTex(&D_800E48F4[1], 1);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x2F;
    } else {
        tpage = 0x1F;
    }
    D_800E48F4[1].tpage = tpage;
    D_800E48F4[0].tpage = tpage;
    D_800E48F4[1].clut = 0x7850;
    D_800E48F4[0].clut = 0x7850;
    D_800E48F4[0].r0 = 0;
    D_800E48F4[1].r0 = 0;
    D_800E48F4[0].g0 = 0;
    D_800E48F4[1].g0 = 0;
    D_800E48F4[0].b0 = 0;
    D_800E48F4[1].b0 = 0;
}

/* Unprototyped on purpose: the original passes nothing, but arg0 has to stay
 * live across the call for the cached &g_FieldExitArrowState to land in $a1. */
void DrawFieldExitArrow();

/* Select toggles the exit arrows on and off (bit 0); bit 1 is a debug override
 * that shows them regardless of the toggle and of the movement lock. */
void UpdateFieldExitArrows(s32 arg0) {
    if (g_FieldState->newActiveKeys2 & (1 << 8)) {
        g_FieldExitArrowState[0] ^= 1;
    }
    if (((g_FieldExitArrowState[0] == 1) &&
         (g_FieldState->characterLock == 0)) ||
        (g_FieldExitArrowState[0] & 2)) {
        DrawFieldExitArrow(arg0);
    }
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", DrawFieldExitArrow);

/////////////////////////////////////////////////
// Begin of field_event_debug.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", DebugUpdateActor);

/* Traces one field-script opcode to debug page 3 and/or the on-screen window:
 * the mnemonic first, then one "arg<n>=<byte>" line per operand read straight
 * back out of the script stream. Bit 4 of g_FieldScriptDebugFlags restricts
 * tracing to the entities flagged in g_FieldScriptDebugEntities. */
void DebugPrintOpcode(char* name, u32 numArgs) {
    u32 total;
    u32 i;

    if ((g_FieldScriptDebugFlags & 4) &&
        !g_FieldScriptDebugEntities[g_CurrentEntity]) {
        return;
    }
    FieldDebugStringCopy(g_DebugText, &D_800E0630);
    FieldDebugStringConcat(g_DebugText, name);
    if (g_DebugLevel & 1) {
        SetStrToDebugRow(3, 0, g_DebugText);
    }
    if (g_DebugLevel & 2) {
        DebugPrintToFieldWindow(g_DebugText);
    }
    total = numArgs + 1;
    while (numArgs != 0) {
        i = total - numArgs;
        FieldDebugStringCopy(g_DebugText, "arg");
        FieldDebugStringU8hex(i, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, "=");
        FieldDebugStringU16hex(GET_PARAM_U8(i), g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        if (g_DebugLevel & 1) {
            SetStrToDebugRow(3, i, g_DebugText);
        }
        if (g_DebugLevel & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
        numArgs--;
    }
}

void FieldDebugAddParseValueToPage2(const char* str, s32 val, s32 kind) {
    if (!(g_FieldScriptDebugFlags & 4) ||
        g_FieldScriptDebugEntities[g_CurrentEntity]) {
        FieldDebugStringCopy(g_DebugText, str);
        switch (kind) {
        case 1:
            FieldDebugStringU8hex(
                val, g_DebugMessageBuffer); // to single hex digit
            break;
        case 2:
            FieldDebugStringU16hex(
                val, g_DebugMessageBuffer); // to double hex digit
            break;
        case 4:
            FieldDebugStringU32hex(
                val, g_DebugMessageBuffer); // to four hex digits
            break;
        default:
            FieldDebugStringCopy(g_DebugMessageBuffer, D_800A0270);
            break;
        }
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        if (g_DebugLevel & 1) {
            AddStrNextDebugRow(2, g_DebugText);
        }
        if (g_DebugLevel & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
    }
}

/////////////////////////////////////////////////
// Begin of field_event_memory_bank.c
/////////////////////////////////////////////////

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEventReadMemoryU8);
#else
u8 FieldEventReadMemoryU8(s16 mb_half, s16 offset) {
    s32 indx;
    u8 value;
    u8 bankId;

    switch (mb_half) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 0:
        value = GET_PARAM_U8(offset);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G cons=", value, 2);
        }
        return value;
    case 1:
    case 2:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 3:
    case 4:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 11:
    case 12:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 13:
    case 14:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 15:
    case 7:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 5:
    case 6:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 2);
        }
        return value;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
    return 0;
}
#endif

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEventWriteMemoryU8);
#else
void FieldEventWriteMemoryU8(s16 arg0, s16 arg1, u8 value) {
    u8 bankId;
    s32 indx;

    switch (arg0) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 1:
    case 2:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 3:
    case 4:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 11:
    case 12:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 13:
    case 14:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 15:
    case 7:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 5:
    case 6:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 2);
        }
        return;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
}
#endif

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEventReadMemoryS16);
#else
s16 FieldEventReadMemoryS16(s16 bank_id, s16 offset) {
    u8 bankId;
    s32 indx;
    s16 value;

    switch (bank_id) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 0:
        GET_PARAM_S16(value, offset);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G cons=", value, 4);
        }
        return value;
    case 1:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 2:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 3:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 4:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 11:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 12:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 13:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 14:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 15:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 7:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 5:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 4);
        }
        return value;
    case 6:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        value |= g_FieldMapVars[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 4);
        }
        return value;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
    return 0;
}
#endif

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEventWriteMemoryS16);
#else
void FieldEventWriteMemoryS16(s16 arg0, s16 arg1, s16 value) {
    u8 bankId;
    s32 indx;

    switch (arg0) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 1:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 2:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 3:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 4:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 11:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 12:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 13:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 14:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 15:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 7:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 5:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 2);
        }
        return;
    case 6:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = (u8)value;
        g_FieldMapVars[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 4);
        }
        return;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
}
#endif

//////////////////////////////////////////////////
// Start of field_opcode_system.c
/////////////////////////////////////////////////

// This is called when there the script tries to execute an invalid opcode
// called for opcodes:
// 0C 0D 1A 1B 1C 1D 1E 1F 44 46 4C 4E BE
s32 OpcodeFuncBad(void) {
    if (g_DebugLevel & 3) {
        FieldDebugStringU16hex(g_FieldCurrentOpcode, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugMessageBuffer, "???");
        DebugPrintOpcode(g_DebugMessageBuffer, 8);
        FieldDebugPageSetColor(3, 0x7F, 0, 0);
    } else {
        FieldEventDebugError("Bad Event code!");
    }
    return 1;
}

/**
 @brief Opcode 0x5F - **WAIT1* - Wait 1 frame

 Memory layout:

 | 0x5F |
 @details
 Waits one frame and returns 1
 @note
 This does not emit a debug message.
 */
s32 OpcodeFuncWait1(void) {
    PC_INC(1);
    return 1;
}

/**
 * @brief Opcode 0x24 - **WAIT** - Wait
 *
 * Memory layout:
 *
 * | 0x24 | A |
 *
 * - const UShort A: Amount (number of frames) to wait.
 * @details
 * g_FieldWaitCounter[g_CurrentEntity] == 0 by default. The opcode then
 * sets it to how many frames to wait before returning 1, which halts
 * execution of the script until next frame.
 *
 * If parameter == 0, the opcode behaves the same way as NOP.
 *
 * The opcode is then called once per frame, decrementing the counter until it
 * reaches 1, at which point it's set to 0 and 0 is returned, which
 * tells the script parser to continue executing next opcode.
 */

s32 OpcodeFuncWait(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wait", 2);
    }

    if (g_FieldWaitCounter[g_CurrentEntity] == 0) {
        GET_PARAM_S16(g_FieldWaitCounter[g_CurrentEntity], 1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2(
                "wait_st=", g_FieldWaitCounter[g_CurrentEntity], 4);
        }
        if (g_FieldWaitCounter[g_CurrentEntity] == 0) {
            PC_INC(3);
            return 1;
        }
        return 1;
    }

    if (g_FieldWaitCounter[g_CurrentEntity] == 1) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("wait_end=", 1, 4);
        }
        g_FieldWaitCounter[g_CurrentEntity] = 0;
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "wait=", g_FieldWaitCounter[g_CurrentEntity], 4);
    }

    g_FieldWaitCounter[g_CurrentEntity]--;
    return 1;
}

//////////////////////////////////////////////////
// Start of field_opcode_vars.c
/////////////////////////////////////////////////

s32 OpcodeFuncSet(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("set", 3);
    }
    FieldEventWriteMemoryU8(1, 2, FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncSet2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("set2", 4);
    }
    FieldEventWriteMemoryS16(1, 2, FieldEventReadMemoryS16(2, 3));
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncLbyte(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lbyte", 3);
    }
    FieldEventWriteMemoryU8(1, 2, FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncHbyte(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("hbyte", 4);
    }
    FieldEventWriteMemoryU8(1, 2, (u8)(FieldEventReadMemoryS16(2, 3) >> 8));
    PC_INC(5);
    return 0;
}

s32 OpcodeFunc2byte(void) {
    s16 lhs;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("2byte", 5);
    }
    lhs = FieldEventReadMemoryU8(2, 4);
    FieldEventWriteMemoryS16(1, 3, lhs | (FieldEventReadMemoryU8(4, 5) << 8));
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncSetx(void) {
    s16 offset;
    u8 bank;
    u8 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("setx", 6);
    }
    bank = GET_PARAM_U8(1) >> 4;
    offset = GET_PARAM_U8(3) + FieldEventReadMemoryS16(2, 3);
    value = FieldEventReadMemoryU8(4, 5);
    switch (bank) {
    case 15:
        offset += 256;
    case 13:
        offset += 256;
    case 11:
        offset += 256;
    case 3:
        offset += 256;
    case 1:
        if (offset >= 1280) {
            offset = 1279;
        }
        Savemap.memory_bank_1[offset] = value;
        break;
    case 5:
        if (offset >= 256) {
            offset = 255;
        }
        g_FieldMapVars[offset] = value;
        break;
    }
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncGetx(void) {
    s16 offset;
    u8 bank;
    u8 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("getx", 6);
    }
    bank = GET_PARAM_U8(1) >> 4;
    offset = GET_PARAM_U8(3) + FieldEventReadMemoryS16(2, 3);
    switch (bank) {
    case 15:
        offset += 256;
    case 13:
        offset += 256;
    case 11:
        offset += 256;
    case 3:
        offset += 256;
    case 1:
        if (offset >= 1280) {
            offset = 1279;
        }
        value = Savemap.memory_bank_1[offset];
        break;
    case 5:
        if (offset >= 256) {
            offset = 255;
        }
        value = g_FieldMapVars[offset];
        break;
    }

    FieldEventWriteMemoryU8(4, 5, value);
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncSrchx(void) {
    s16 end;
    s16 start;
    s16 where;
    u8 bank;
    u8 value;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("srchx", 8);
    }
    bank = GET_PARAM_U8(1) >> 4;
    start = GET_PARAM_U8(4) + FieldEventReadMemoryS16(2, 5);
    end = GET_PARAM_U8(4) + FieldEventReadMemoryS16(3, 7);
    value = FieldEventReadMemoryU8(4, 9);
    switch (bank) {
    case 15:
        start += 256;
        end += 256;
    case 13:
        start += 256;
        end += 256;
    case 11:
        start += 256;
        end += 256;
    case 3:
        start += 256;
        end += 256;
    case 1:
        if (start >= 1280) {
            start = 1279;
        }
        if (end >= 1280) {
            end = 1279;
        }
        for (i = start; i <= end; i++) {
            if (Savemap.memory_bank_1[i] == value) {
                FieldEventWriteMemoryS16(6, 10, i);
                PC_INC(11);
                return 0;
            }
        }
        break;
    case 5:
        if (start >= 256) {
            start = 255;
        }
        if (end >= 256) {
            end = 255;
        }
        for (i = start; i <= end; i++) {
            if (g_FieldMapVars[i] == value) {
                FieldEventWriteMemoryS16(6, 10, i);
                PC_INC(11);
                return 0;
            }
        }
        break;
    }
    FieldEventWriteMemoryS16(6, 10, -1);
    PC_INC(11);
    return 0;
}

s32 OpcodeFuncBiton(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("biton", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) | (1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBitof(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bitof", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) & ~(1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBitxr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bitxr", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) ^ (1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_line.c
/////////////////////////////////////////////////

s32 OpcodeFuncLine(void) {
    s16 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("line", 8);
    }

    if (g_FieldLineCount >= 32) {
        FieldEventDebugError("many lineobj!");
        PC_INC(13);
        return 0;
    }

    g_EntityToLine[g_CurrentEntity] = g_FieldLineCount;
    GET_PARAM_S16(value, 1);
    g_FieldLines[g_FieldLineCount].pos.x1 = value;
    GET_PARAM_S16(value, 3);
    g_FieldLines[g_FieldLineCount].pos.y1 = value;
    GET_PARAM_S16(value, 5);
    g_FieldLines[g_FieldLineCount].pos.z1 = value;
    GET_PARAM_S16(value, 7);
    g_FieldLines[g_FieldLineCount].pos.x2 = value;
    GET_PARAM_S16(value, 9);
    g_FieldLines[g_FieldLineCount].pos.y2 = value;
    GET_PARAM_S16(value, 11);
    g_FieldLines[g_FieldLineCount].pos.z2 = value;
    g_FieldLines[g_FieldLineCount].isActive = 1;
    g_FieldLines[g_FieldLineCount].entityId = g_CurrentEntity;
    g_FieldLineCount++;
    PC_INC(13);
    return 0;
}

s32 OpcodeFuncSline(void) {
    u8 lineId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sline", 8);
    }
    lineId = g_EntityToLine[g_CurrentEntity];
    g_FieldLines[lineId].pos.x1 = FieldEventReadMemoryS16(1, 4);
    g_FieldLines[lineId].pos.y1 = FieldEventReadMemoryS16(2, 6);
    g_FieldLines[lineId].pos.z1 = FieldEventReadMemoryS16(3, 8);
    g_FieldLines[lineId].pos.x2 = FieldEventReadMemoryS16(4, 10);
    g_FieldLines[lineId].pos.y2 = FieldEventReadMemoryS16(5, 12);
    g_FieldLines[lineId].pos.z2 = FieldEventReadMemoryS16(6, 14);
    PC_INC(16);
    return 0;
}

s32 OpcodeFuncLinon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("linon", 1);
    }
    g_FieldLines[g_EntityToLine[g_CurrentEntity]].isActive = GET_PARAM_U8(1);
    if (GET_PARAM_U8(1) == 0) {
        g_FieldLines[g_EntityToLine[g_CurrentEntity]].touch = 0;
    }
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode SLIP: Enables or disables slipping along a line
 *
 * Slipping allows the player to slide along a wall when running
 * against it instead of stopping. The wall must previously have a
 * line defined alongside it with opcode LINE.
 */

s32 OpcodeFuncSlip(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("slip", 1);
    }
    g_FieldLines[g_EntityToLine[g_CurrentEntity]].slipDisabled =
        GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_if.c
/////////////////////////////////////////////////

/*
 * Field-script opcode IF: If, 1 byte, unsigned
 *
 * Compares two u8 using a given logical operator.
 * Jumps given number of bytes ahead if the comparison is false.
 */

s32 OpcodeFuncIf(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if", 5);
    }
    if (IfCheck()) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if=true", 0, 0);
        }
        // If comparison is true, continue executing next opcode.
        PC_INC(6);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if=false", 0, 0);
        }
        // If comparison is false, jump number of bytes give in last parameter
        // from last parameter.
        PC_INC(GET_PARAM_U8(5) + 5);
    }
    return 0;
}

/*
 * Field-script opcode LIF: Long If, 1 byte, unsigned
 *
 * Compares two u8 using a given logical operator.
 * Identical to IF except that the jump parameter is s16, allowing for longer
 * jumps.
 */

s32 OpcodeFuncLif(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif", 6);
    }
    if (IfCheck()) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif=true", 0, 0);
        }
        PC_INC(7);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif=false", 0, 0);
        }
        GET_PARAM_S16(param, 5);
        PC_INC(param + 5);
    }
    return 0;
}

u32 IfCheck(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(4);
    switch (ope) {
    case IF_EQ:
        result = FieldEventReadMemoryU8(1, 2) == FieldEventReadMemoryU8(2, 3);
        break;
    case IF_NOT_EQ:
        result = FieldEventReadMemoryU8(1, 2) != FieldEventReadMemoryU8(2, 3);
        break;
    case IF_GT:
        result = FieldEventReadMemoryU8(1, 2) > FieldEventReadMemoryU8(2, 3);
        break;
    case IF_LT:
        result = FieldEventReadMemoryU8(1, 2) < FieldEventReadMemoryU8(2, 3);
        break;
    case IF_GTE:
        result = FieldEventReadMemoryU8(1, 2) >= FieldEventReadMemoryU8(2, 3);
        break;
    case IF_LTE:
        result = FieldEventReadMemoryU8(1, 2) <= FieldEventReadMemoryU8(2, 3);
        break;
    case IF_AND:
        result = FieldEventReadMemoryU8(1, 2) & FieldEventReadMemoryU8(2, 3);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryU8(1, 2) ^ FieldEventReadMemoryU8(2, 3);
        break;
    case IF_OR:
        result = FieldEventReadMemoryU8(1, 2) | FieldEventReadMemoryU8(2, 3);
        break;
    case IF_BIT:
        result =
            FieldEventReadMemoryU8(1, 2) & (1 << FieldEventReadMemoryU8(2, 3));
        break;
    case IF_NOT_BIT:
        result =
            FieldEventReadMemoryU8(1, 2) & (1 << FieldEventReadMemoryU8(2, 3));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}

/*
 * Field-script opcode IF2: If, 2 bytes, signed
 *
 * Compares two s16 using a given logical operator.
 */
s32 OpcodeFuncIf2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if2", 7);
    }
    if (If2CheckSigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=true", 0, 0);
        }
        PC_INC(8);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(7) + 7);
    }
    return 0;
}

/*
 * Field-script opcode LIF2: Long if, 2 bytes, signed
 *
 * Compares two s16 using a given logical operator.
 */
s32 OpcodeFuncLif2(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif2", 8);
    }
    if (If2CheckSigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=true", 0, 0);
        }
        PC_INC(9);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=false", 0, 0);
        }
        GET_PARAM_S16(param, 7);
        PC_INC(param + 7);
    }
    return 0;
}

u32 If2CheckSigned(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(6);
    switch (ope) {
    case IF_EQ:
        result = FieldEventReadMemoryS16(1, 2) == FieldEventReadMemoryS16(2, 4);
        break;
    case IF_NOT_EQ:
        result = FieldEventReadMemoryS16(1, 2) != FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GT:
        result = FieldEventReadMemoryS16(1, 2) > FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LT:
        result = FieldEventReadMemoryS16(1, 2) < FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GTE:
        result = FieldEventReadMemoryS16(1, 2) >= FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LTE:
        result = FieldEventReadMemoryS16(1, 2) <= FieldEventReadMemoryS16(2, 4);
        break;
    case IF_AND:
        result = FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 4);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 4);
        break;
    case IF_OR:
        result = FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 4);
        break;
    case IF_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        break;
    case IF_NOT_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}

/*
 * Field-script opcode IF2U: If, 2 bytes, unsigned
 *
 * Compares two u16 using a given logical operator.
 */
s32 OpcodeFuncIf2u(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if2", 7);
    }
    if (If2CheckUnsigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=true", 0, 0);
        }
        PC_INC(8);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(7) + 7);
    }
    return 0;
}

/*
 * Field-script opcode LIF2U: Long if, 2 bytes, unsigned
 *
 * Compares two u16 using a given logical operator.
 */
s32 OpcodeFuncLif2u(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif2", 8);
    }
    if (If2CheckUnsigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=true", 0, 0);
        }
        PC_INC(9);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=false", 0, 0);
        }
        GET_PARAM_S16(param, 7);
        PC_INC(param + 7);
    }
    return 0;
}

u32 If2CheckUnsigned(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(6);
    switch (ope) {
    case IF_EQ:
        result = (u16)FieldEventReadMemoryS16(1, 2) ==
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_NOT_EQ:
        result = (u16)FieldEventReadMemoryS16(1, 2) !=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GT:
        result = (u16)FieldEventReadMemoryS16(1, 2) >
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LT:
        result = (u16)FieldEventReadMemoryS16(1, 2) <
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GTE:
        result = (u16)FieldEventReadMemoryS16(1, 2) >=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LTE:
        result = (u16)FieldEventReadMemoryS16(1, 2) <=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_AND:
        result = FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 4);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 4);
        break;
    case IF_OR:
        result = FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 4);
        break;
    case IF_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        break;
    case IF_NOT_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}

//////////////////////////////////////////////////
// Start of field_opcode_controller.c
/////////////////////////////////////////////////

/*
 * Field-script opcode KEY!: Key check
 *
 * Jumps ahead given number of bytes if given key(s) are not active.
 * All key opcodes only check the lower half word which contains the keys
 * for controller 1.
 */

s32 OpcodeFuncKeyEx(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("key!", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->activeKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->activeKeys);
    }
}

/*
 * Field-script opcode KEYON: Key On
 *
 * Checks keys that player pressed this frame.
 */

s32 OpcodeFuncKeyon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("keyon", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->newActiveKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->newActiveKeys);
    }
}

/*
 * Field-script opcode KEYOF: Key Off
 *
 * Checks keys that player released this frame.
 */

s32 OpcodeFuncKeyof(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("keyof", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->newInactiveKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->newInactiveKeys);
    }
}

s32 KeyCheck(u16 keys) {
    u16 param;

    GET_PARAM_S16(param, 1);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("key now=", keys, 4);
        FieldDebugAddParseValueToPage2("key chk=", param, 4);
    }
    if (keys & param) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("key=true", 0, 0);
        }
        PC_INC(4);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("key=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(3) + 3);
    }
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_request.c
/////////////////////////////////////////////////

s32 OpcodeFuncReq(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("req", 2);
    }
    return FieldEventRequest(1, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncReqsw(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("reqsw", 2);
    }
    return FieldEventRequest(2, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncReqew(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("reqew", 2);
    }
    return FieldEventRequest(3, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPreq(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("preq", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(1, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPrqsw(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prqsw", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(2, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPrqew(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prqew", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(3, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

// Depends on decomp of DebugUpdateActor due to shared string.
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEventRequest);
#else
s32 FieldEventRequest(s16 type, u8 target, u8 priority, u8 scriptId) {
    s32 scriptOffset;
    s32 entityDataSize;
    s32 extrasHeaderSize;

    if (target == 0xFF) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rqew=no one", 0, 0);
        }
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugStringCopy(g_DebugMessageBuffer, "rq=");
        FieldDebugStringConcat(
            g_DebugMessageBuffer, (char*)((s32)g_FieldScripts) +
                                      sizeof(FieldScriptHeader) + (target * 8));
        FieldDebugStringConcat(g_DebugMessageBuffer, "/");
        FieldDebugAddParseValueToPage2(g_DebugMessageBuffer, scriptId, 2);
    }

    if (type > 0) {
        if (type >= 3) {
            if (type == 3 && g_FieldScriptSyncWaitEntity[target][priority] ==
                                 g_CurrentEntity) {
                switch (g_FieldScriptSyncState[target][priority]) {
                case SYNC_WAITING:
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("rqew=wait", 0, 0);
                    }
                    return 1;
                case SYNC_DONE:
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("rqew=end", 0, 0);
                    }
                    g_FieldScriptSyncState[target][priority] = SYNC_NONE;
                    g_FieldScriptSyncWaitEntity[target][priority] = 0xFF;
                    PC_INC(3);
                    return 0;
                }
            }
        }
    }

    if (g_FieldScriptPriority[target] == priority) {
        switch (type) {
        case 1:
            PC_INC(3);
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("rq=skip", 0, 0);
            }
            return 0;
        case 2:
        case 3:
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("rqw=busy", 0, 0);
            }
        }
        return 1;
    } else if (g_FieldScriptPriority[target] < priority) {
        if (g_SavedFieldScriptPC[target][priority] != 0) {
            switch (type) {
            case 1:
                PC_INC(3);
                if (g_DebugLevel & 3) {
                    FieldDebugAddParseValueToPage2("rq=skip", 0, 0);
                }
                return 0;
            case 2:
            case 3:
                if (g_DebugLevel & 3) {
                    FieldDebugAddParseValueToPage2("rqw=busy", 0, 0);
                }
            }
            return 1;
        }
        scriptOffset = scriptId * 2;
        entityDataSize = target * 64;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

        g_SavedFieldScriptPC[target][priority] =
            *((u8*)(scriptOffset +
                    (entityDataSize + (g_FieldScripts->numEntities << 3)) +
                    extrasHeaderSize + (s32)g_FieldScripts) +
              sizeof(FieldScriptHeader));
        g_SavedFieldScriptPC[target][priority] |=
            *((u8*)(scriptOffset +
                    ((entityDataSize + (g_FieldScripts->numEntities << 3)) +
                     (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rq=send", 0, 0);
        }

        if (type <= 0) {
            return 1;
        }

        if (type >= 3) {
            if (type != 3) {
                return 1;
            }
        } else {
            PC_INC(3);
            return 0;
        }

        g_FieldScriptSyncWaitEntity[target][priority] = g_CurrentEntity;
        g_FieldScriptSyncState[target][priority] = SYNC_WAITING;
        return 1;
    } else if (g_FieldScriptSyncState[target][priority] == SYNC_NONE) {
        s32 scriptOffset;
        s32 entityDataSize;
        s32 extrasHeaderSize;

        SavedScriptIds[target][priority] = scriptId;
        g_SavedFieldScriptPC[target][g_FieldScriptPriority[target]] =
            g_FieldScriptPC[target];

        scriptOffset = scriptId * 2;
        entityDataSize = target * 64;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

        g_FieldScriptPC[target] =
            *((u8*)(scriptOffset +
                    (entityDataSize + (g_FieldScripts->numEntities << 3)) +
                    extrasHeaderSize + (s32)g_FieldScripts) +
              sizeof(FieldScriptHeader));
        g_FieldScriptPC[target] |=
            *((u8*)(scriptOffset +
                    ((entityDataSize + (g_FieldScripts->numEntities << 3)) +
                     (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        g_FieldScriptPriority[target] = priority;

        if (g_EntityToModel[target] != 0xFF) {
            g_FieldModels[g_EntityToModel[target]].scriptedMoveMode =
                SMODE_NONE;
        }
        g_FieldWaitCounter[target] = 0;

        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rq=send", 0, 0);
        }

        if (type <= 0) {
            return 1;
        }

        if (type >= 3) {
            if (type != 3) {
                return 1;
            }
        } else {
            PC_INC(3);
            return 0;
        }

        g_FieldScriptSyncWaitEntity[target][priority] = g_CurrentEntity;
        g_FieldScriptSyncState[target][priority] = SYNC_WAITING;
        return 1;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("rqw=busy*", 0, 0);
    }
    return 1;
}
#endif

s32 OpcodeFuncRet(void) {
    u16* fieldScriptPC;
    u16(*savedPC)[8];
    u16* savedRow;
    u16 scriptPc;
    u32 entity;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ret", 0);
    }
    if (g_FieldScriptPriority[g_CurrentEntity] >= 7) {
        return 1;
    }

    if (g_FieldScriptSyncState[g_CurrentEntity]
                              [g_FieldScriptPriority[g_CurrentEntity]] ==
        SYNC_WAITING) {
        g_FieldScriptSyncState[g_CurrentEntity]
                              [g_FieldScriptPriority[g_CurrentEntity]] =
                                  SYNC_DONE;
    }

    g_FieldScriptPriority[g_CurrentEntity]++;

    entity = g_CurrentEntity;
    savedPC = g_SavedFieldScriptPC;
    fieldScriptPC = g_FieldScriptPC;

    savedRow = savedPC[entity];
    scriptPc =
        *(u16*)((g_FieldScriptPriority[entity] * sizeof(u16)) + (s32)savedRow);
    fieldScriptPC[entity] = scriptPc;

    while (scriptPc == 0 && g_FieldScriptPriority[entity] < 7) {
        u16* activePcSlot;
        u16* loopSavedRow;
        u16 nextPc;

        g_FieldScriptPriority[g_CurrentEntity]++;
        entity = g_CurrentEntity;

        activePcSlot =
            (u16*)((entity * sizeof(*fieldScriptPC)) + (s32)fieldScriptPC);

        loopSavedRow = (u16*)((entity * sizeof(*savedPC)) + (s32)savedPC);

        nextPc = *(u16*)((g_FieldScriptPriority[entity] * sizeof(u16)) +
                         (s32)loopSavedRow);

        *activePcSlot = nextPc;
        scriptPc = nextPc;
    }

    g_SavedFieldScriptPC[g_CurrentEntity]
                        [g_FieldScriptPriority[g_CurrentEntity]] = 0;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "ret=", g_FieldScriptPriority[g_CurrentEntity], 2);
    }
    return 0;
}

s32 OpcodeFuncRetto(void) {
    s16 scriptId;
    u8 priority;
    s32 extrasHeaderSize;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("retto", 1);
    }

    priority = GET_PRIORITY(GET_PARAM_U8(1));
    scriptId = GET_SCRIPTID(GET_PARAM_U8(1));

    while (g_FieldScriptPriority[g_CurrentEntity] < (priority - 1) &&
           g_FieldScriptPriority[g_CurrentEntity] < 7) {
        if (g_FieldScriptSyncState[g_CurrentEntity]
                                  [g_FieldScriptPriority[g_CurrentEntity]] ==
            SYNC_WAITING) {
            g_FieldScriptSyncState[g_CurrentEntity]
                                  [g_FieldScriptPriority[g_CurrentEntity]] =
                                      SYNC_DONE;
        }
        g_FieldScriptPriority[g_CurrentEntity]++;
        g_SavedFieldScriptPC[g_CurrentEntity]
                            [g_FieldScriptPriority[g_CurrentEntity]] = 0;
    }
    SavedScriptIds[g_CurrentEntity][priority] = scriptId;
    scriptId *= 2;
    extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

    g_FieldScriptPC[g_CurrentEntity] =
        *((u8*)(scriptId +
                ((g_FieldScripts->numEntities * 8) + (g_CurrentEntity * 64)) +
                extrasHeaderSize + (s32)g_FieldScripts) +
          sizeof(FieldScriptHeader));
    g_FieldScriptPC[g_CurrentEntity] |=
        *((u8*)(scriptId +
                (((g_FieldScripts->numEntities * 8) + (g_CurrentEntity * 64)) +
                 (s32)g_FieldScripts) +
                extrasHeaderSize) +
          sizeof(FieldScriptHeader) + 1)
        << 8;

    g_FieldScriptPriority[g_CurrentEntity] = priority;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "ret=", g_FieldScriptPriority[g_CurrentEntity], 2);
    }
    return 0;
}

s32 OpcodeFuncBack(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("back", 1);
    }
    PC_DEC(GET_PARAM_U8(1));
    return 1;
}

s32 OpcodeFuncLback(void) {
    u16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lback", 2);
    }
    GET_PARAM_S16(param, 1);
    PC_DEC(param);
    return 1;
}

s32 OpcodeFuncSkip(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("skip", 1);
    }
    PC_INC(GET_PARAM_U8(1) + 1);
    return 0;
}

s32 OpcodeFuncLskip(void) {
    u16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lskip", 2);
    }
    GET_PARAM_S16(param, 1);
    PC_INC(param + 1);
    return 0;
}

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncMjump);
#else
s32 OpcodeFuncMjump(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mjump", 8);
    }

    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_FIELD_MAP_CHANGE;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        GET_PARAM_S16(g_FieldState->eventCmdParam, 1);
        GET_PARAM_S16(g_FieldState->pcPosX, 3);
        GET_PARAM_S16(g_FieldState->pcPosY, 5);
        GET_PARAM_S16(g_FieldState->pcWalkMeshId, 7);
        g_FieldState->pcDirection = GET_PARAM_U8(9);
        return 1;
    case EVTCMD_FIELD_MAP_CHANGE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(10);
            g_FieldState->eventCmd = EVTCMD_NONE;
            return 0;
        }
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("evt cmd=", g_FieldState->eventCmd, 2);
    }
    return 1;
}
#endif

s32 OpcodeFuncPmjmp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmjmp", 8);
    }
    GET_PARAM_S16(g_FieldPreloadMapId, 1);
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncPmjmp2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmjmp", 8);
    }
    if (g_isFieldLoading != 2) {
        return 1;
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncMgame(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mgame", 8);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_LOAD_MINIGAME;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        GET_PARAM_S16(g_FieldState->eventCmdParam, 1);
        GET_PARAM_S16(g_FieldState->pcPosX, 3);
        GET_PARAM_S16(g_FieldState->pcPosY, 5);
        GET_PARAM_S16(g_FieldState->pcWalkMeshId, 7);
        *(s16*)&g_FieldState->pcDirection = GET_PARAM_U8(9);
        *(u8*)((u8*)g_FieldState + 0xF2) = GET_PARAM_U8(10);
        return 1;
    case EVTCMD_LOAD_MINIGAME:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(11);
            g_FieldState->eventCmd = EVTCMD_NONE;
            return 0;
        }
        return 1;
    }
    return 1;
}

s32 OpcodeFuncBatle(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("batle", 3);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        FieldWindowResetTextAll();
        g_FieldState->eventCmd = EVTCMD_ENTERING_BATTLE;
        g_FieldState->eventCmdParam = FieldEventReadMemoryS16(2, 2);
        D_8007EBE0 = 1;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        return 1;
    case EVTCMD_ENTERING_BATTLE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(4);
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            return 0;
        }
        break;
    }
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_akao_sound.c
/////////////////////////////////////////////////

void FieldEventClearAkaoStruct(void) {
    s32 i;
    s16* p;

    D_8009A000[0] = 0;
    for (i = 5, p = &D_8009A000[10]; i >= 0; i--) {
        *(s32*)(p + 2) = 0;
        p -= 2;
    }
}

s32 OpcodeFuncAkao(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("akao", 3);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(4);
    *D_8009A004 = FieldEventReadMemoryU8(1, 5);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(2, 6);
    D_8009A00C = (s16)FieldEventReadMemoryS16(3, 8);
    D_8009A010 = (s16)FieldEventReadMemoryS16(4, 10);
    D_8009A014 = (s16)FieldEventReadMemoryS16(6, 12);
    SystemAkaoExecute();
    PC_INC(14);
    return 0;
}

s32 OpcodeFuncAkao2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("akao2", 3);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(4);
    *D_8009A004 = (s16)FieldEventReadMemoryS16(1, 5);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(2, 7);
    D_8009A00C = (s16)FieldEventReadMemoryS16(3, 9);
    D_8009A010 = (s16)FieldEventReadMemoryS16(4, 11);
    D_8009A014 = (s16)FieldEventReadMemoryS16(6, 13);
    SystemAkaoExecute();
    PC_INC(15);
    return 0;
}

s32 OpcodeFuncSe(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("se", 3);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x20;
    D_8009A004[0] = FieldEventReadMemoryU8(2, 4);
    D_8009A008[0] = (s16)FieldEventReadMemoryS16(1, 2);
    SystemAkaoExecute();
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncMusic(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("music", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x10;
    return SetAndApplyAkao();
}

s32 OpcodeFuncMusvt(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("musvt", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x14;
    return SetAndApplyAkao();
}

s32 OpcodeFuncMusvm(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("musvm", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x15;
    return SetAndApplyAkao();
}

s32 OpcodeFuncCmusc(void) {
    u32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cmusc", 5);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(3);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(3, 4);
    D_8009A00C = (s16)FieldEventReadMemoryS16(4, 6);
    result = SetAndApplyAkao();
    PC_INC(6);
    return result;
}

s32 SetAndApplyAkao(void) {
    // Indexes into AKAO block of field file which contains the list of music
    // tracks available for current field.
    u8 akaoId;

    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("music=", akaoId, 2);
        }
        *D_8009A004 = (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
        g_FieldState->nextFieldMusic = *D_8009A004;
        SystemAkaoExecute();
    }
    PC_INC(2);
    return 0;
}

u32 GetAkaoBlockOffset(s16 akaoId) {
    s32 akaoData;
    u32 akaoOffset;

    akaoData =
        akaoId * 4 + g_FieldScripts->numEntities * 8 + (s32)g_FieldScripts;
    akaoOffset = ((u8*)akaoData)[sizeof(FieldScriptHeader)];
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 1] << 8;
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 2] << 16;
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 3] << 24;
    return akaoOffset;
}

s32 OpcodeFuncBmusc(void) {
    u8 akaoId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bmusc", 1);
    }
    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("bmusic=", akaoId, 2);
        }
        g_FieldState->nextBattleMusic =
            (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
    } else {
        g_FieldState->nextBattleMusic = 0;
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncFmusc(void) {
    u8 akaoId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fmusc", 1);
    }
    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("bmusic=", akaoId, 2);
        }
        g_FieldState->nextFieldMusic =
            (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
    } else {
        g_FieldState->nextFieldMusic = 0;
    }
    PC_INC(2);
    return 0;
}

// In Akao because it uses the AKAO block area
/* TUTOR (0x21): open the main menu and play the tutorial with the given id.
 * First call arms the PARTY_MENU event command, flags the menu overlay and
 * resolves the tutorial's block into D_800E48E0 for the main loop to stream;
 * once the menu reports MOVCMD_DONE, clear the command and advance past the
 * operand. Verified C is the #else; codegen pinned via MASPSX_OVERRIDE. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncTutor);
#else
extern u8* D_800E48E0;

s32 OpcodeFuncTutor(void) {
    s16 tutorialId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tutor", 1);
    }
    if (g_FieldState->eventCmd == EVTCMD_NONE) {
        g_FieldState->eventCmd = EVTCMD_PARTY_MENU;
        D_8007EBE0 = 1;
        g_FieldState->eventCmdParam = 1;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        tutorialId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("data=", tutorialId, 2);
        }
        D_800E48E0 = (u8*)g_FieldScripts + GetAkaoBlockOffset(tutorialId);
        return 1;
    }
    if (g_FieldState->eventCmd == EVTCMD_PARTY_MENU) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2(
                "evt result=", g_FieldState->movieCommandState, 2);
        }
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(2);
            return 0;
        }
    }
    return 1;
}
#endif

/////////////////////////////////////////////////
// Start of field_opcode_movie_overlay.c
/////////////////////////////////////////////////

/*
 * Field-script opcode MULCK (0xF5): set the music lock from the opcode operand.
 *
 * While g_FieldMusicLock is nonzero the MUSIC/FMUSC opcodes skip handing the
 * song to the sound engine, so field music stops responding until a later
 * MULCK 0 (or a reset) clears it again.
 *
 * The operand is read straight out of the running script:
 *   g_FieldScripts          - the current map's script bytecode
 *   g_FieldScriptPC[entity]  - that entity's program counter (byte offset into
 * it) g_CurrentEntity          - the entity whose script is currently executing
 * so g_FieldScripts[pc + 1] is the 1-byte operand. The program counter is then
 * stepped past the 2-byte instruction (opcode + operand).
 */
s32 OpcodeFuncMulck(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mulck", 1);
    }
    g_FieldMusicLock = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncBgmovie(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgmovie", 1);
    }
    g_FieldState->backgroundMovieEnabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncScrlo(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlo", 1);
    }
    g_FieldState->scrloSet = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode DSKCG: request a disc change.
 *
 * Runs as a small state machine on the field main-loop step (opcode):
 * on first execution it stores the requested disc number and switches the
 * field loop into the disc-change step (13), then keeps returning 1
 * (opcode not finished) until the loop reports the swap is done
 * (movieCommandState == 2). Only then does the script advance past the opcode.
 */
s32 OpcodeFuncDskcg(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dskcg", 1);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_CD_CHANGE;
        D_8009D588 = GET_PARAM_U8(1);
        return 1;
    case EVTCMD_CD_CHANGE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            g_FieldState->eventCmd = EVTCMD_NONE;
            PC_INC(2);
            return 0;
        }
        return 1;
    default:
        return 1;
    }
}

/*
 * Field-script opcode UC: lock or unlock player control.
 *
 * A nonzero operand freezes the player character; on unlock the
 * per-model flag of the player's model is cleared as well.
 */
s32 OpcodeFuncUc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("uc", 1);
    }
    g_CharacterLock = g_FieldState->characterLock = GET_PARAM_U8(1);
    if (g_CharacterLock == 0) {
        D_800756E8[g_FieldState->pcModelId] = 0;
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncBtlon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btlon", 1);
    }
    g_FieldState->battlesDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMpdsp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpdsp", 1);
    }
    g_FieldState->mpdspSet = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMvcam(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mvcam", 1);
    }
    g_FieldState->movieCamDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncGmovr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gmovr", 0);
    }
    g_FieldState->eventCmd = EVTCMD_GAME_OVER;
    g_FieldState->movieCommandState = MOVCMD_IDLE;
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_char_control.c
/////////////////////////////////////////////////

/*
 * Field-script opcode CC: hand player control to another entity.
 *
 * The operand is a script entity id; if that entity has a field model
 * assigned (g_EntityToModel entry != 0xFF) it becomes the new player model.
 */
s32 OpcodeFuncCc(void) {
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cc", 1);
    }
    charId = GET_PARAM_U8(1);
    if (g_EntityToModel[charId] != 0xFF) {
        g_FieldState->pcModelId = g_EntityToModel[charId];
    }
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode CHAR: attach a field model to the current entity.
 *
 * Allocates the next model slot (g_FieldModelCount) for the executing entity,
 * records the mapping in g_EntityToModel and initializes the model with the
 * model id from the opcode operand and the owning entity id.
 */
s32 OpcodeFuncChar(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("char", 1);
    }
    g_EntityToModel[g_CurrentEntity] = g_FieldModelCount++;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].charId = GET_PARAM_U8(1);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].entityId = g_CurrentEntity;
    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_animate.c
/////////////////////////////////////////////////

/*
 * Field-script opcode DFANM: set a model's default (looping) animation.
 *
 * Stores the animation id and playback speed (per-model base speed divided
 * by the speed operand) for the model attached to the executing entity.
 * A model holding the last frame of a script animation (state 3) is
 * released so the new default animation starts playing.
 */
s32 OpcodeFuncDfanm(void) {
    u8 modelIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dfanm", 2);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        D_8008325C[g_EntityToModel[g_CurrentEntity]] = GET_PARAM_U8(1);
        D_80082248[g_EntityToModel[g_CurrentEntity]] =
            D_8009D828[g_EntityToModel[g_CurrentEntity]] / GET_PARAM_U8(2);
        modelIdx = g_EntityToModel[g_CurrentEntity];
        if (D_800756E8[modelIdx] == 3) {
            D_800756E8[modelIdx] = 0;
        }
    }
    PC_INC(3);
    return 1;
}

/*
 * Field-script opcode CCANM: set one of the player animation ids
 * (0: idle, 1: walk, 2: run) used while the player controls a model.
 */
s32 OpcodeFuncCcanm(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ccanm", 3);
    }
    switch (GET_PARAM_U8(3)) {
    case 0:
        g_FieldState->idleAnimId = GET_PARAM_U8(1);
        break;
    case 1:
        g_FieldState->walkAnimId = GET_PARAM_U8(1);
        break;
    case 2:
        g_FieldState->runAnimId = GET_PARAM_U8(1);
        break;
    }
    PC_INC(4);
    return 0;
}

/*
 * Starts the animation requested by the current ANIME-style opcode on the
 * model attached to the executing entity: animation id from the first
 * operand, playback speed from the per-model base speed divided by the
 * second operand, frame counter rewound and the last frame looked up in
 * the animation header of the model's file.
 */
void StartModelAnimation(void) {
    u8 modelIdx;
    u8* anims;
    FieldModelEntry* model;

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].activeAnimId =
        GET_PARAM_U8(1);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed =
        D_8009D828[g_EntityToModel[g_CurrentEntity]] / GET_PARAM_U8(2);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
    modelIdx = g_EntityToModel[g_CurrentEntity];
    model =
        &g_FieldModelData
             ->modelEntries[g_FieldModelLoaderData[modelIdx].modelEntryIndex];
    anims = model->modelData + model->animationOffset;
    g_FieldModels[modelIdx].animLastFrame =
        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
}

/*
 * Field-script opcode ANIME1/ANIME2: play an animation on the entity's
 * model. g_FieldCurrentOpcode distinguishes which opcode invoked the handler:
 * the asynchronous variant (0xAE, ANIME2) marks the model as playing (state 5)
 * and lets the script continue, while ANIME1 blocks (state 2) until the
 * animation system reports completion (state 4), then resets the model to
 * its default animation.
 */
s32 OpcodeFuncAnime(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("anime", 2);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(3);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        StartModelAnimation();
        if (g_FieldCurrentOpcode == 0xAE) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 5;
            PC_INC(3);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
        PC_INC(3);
        return 0;
    }
    return 1;
}

/*
 * Field-script opcode ANIM!1/ANIM!2: like ANIME1/ANIME2 but the model
 * keeps holding the last frame once the animation completes (state 3)
 * instead of returning to its default animation. 0xAE becomes 0xAF and
 * state 5 becomes 6 to tell the two opcode pairs apart.
 */
s32 OpcodeFuncAnimEx(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("anim!", 2);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(3);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        StartModelAnimation();
        if (g_FieldCurrentOpcode == 0xAF) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 6;
            PC_INC(3);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
        PC_INC(3);
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCanim);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCanmEx);

s32 OpcodeFuncAnimw(void) {
    u8 modelIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("animw", 0);
    }
    modelIdx = g_EntityToModel[g_CurrentEntity];
    if (modelIdx == 0xFF) {
        PC_INC(1);
        return 0;
    }
    switch (D_800756E8[modelIdx]) {
    case 2:
    case 5:
    case 6:
        return 1;
    case 4:
        D_800756E8[modelIdx] = 0;
        break;
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncAnimb(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("animb", 0);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animLastFrame =
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame >>
            4;
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
    }
    PC_INC(1);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_move.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMove);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncFmove);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCmove);

s32 OpcodeFuncFcfix(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fcfix", 1);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock =
            GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncJump);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncLader);

void OpcodeFuncPmova(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmova", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldMoveToEntityUpdate(actorId);
}

void OpcodeFuncMova(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mova", 1);
    }
    FieldMoveToEntityUpdate(GET_PARAM_U8(1));
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldMoveToEntityUpdate);

void OpcodeFuncDira(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dira", 1);
    }
    FieldEventSetDirByActorId(GET_PARAM_U8(1));
}

void OpcodeFuncPdira(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pdira", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldEventSetDirByActorId(actorId);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSetDirByActorId);

void OpcodeFuncTura(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tura", 3);
    }
    FieldEntityTurnToEntity(GET_PARAM_U8(1));
}

void OpcodeFuncPtura(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ptura", 3);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldEntityTurnToEntity(actorId);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityTurnToEntity);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncOfstd);

/* Block until this entity's offset animation finishes. OfsType 3 means the last
 * step ran, so clear it and fall through; 0 means there was never one. */
s32 OpcodeFuncOfstw(void) {
    FieldEntity* model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ofstw", 0);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(1);
        return 0;
    }
    model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (model->OfsType != 0 && model->OfsType != 3) {
        return 1;
    }
    model->OfsType = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStep = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetSteps = 0;
    PC_INC(1);
    return 0;
}

/* Block until this entity's turn finishes. Returning 1 without advancing the
 * PC re-runs the opcode next frame; TurnType 3 means the turn just completed,
 * so clear it and fall through.
 *
 * Instruction-for-instruction identical; what is left is where gcc cross-jumps
 * the PC_INC tail. The original merges the three paths *after* the reload of
 * g_CurrentEntity, so the model == 0xFF path reuses the copy loaded at the top
 * of the function; gcc merges two instructions earlier and reloads. Writing the
 * tail out twice stops it cross-jumping at all (9 extra instructions), and
 * inverting the test to an early return flips the branch to `bne`. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncTurnw);
#else
s32 OpcodeFuncTurnw(void) {
    FieldEntity* model;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("turnw", 0);
        }
        model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
        if (model->TurnType != 0) {
            if (model->TurnType != 3) {
                return 1;
            }
            model->TurnType = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
        }
    }
    PC_INC(1);
    return 0;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncTurn);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncTurnr);

/* Snap this entity to a facing, cancelling any turn in progress. Returns 1 when
 * the entity actually has a model, unlike most opcodes. */
s32 OpcodeFuncDir(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("dir", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir =
            FieldEventReadMemoryU8(2, 2);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
        PC_INC(3);
        return 1;
    }
    PC_INC(3);
    return 0;
}

/* SLIDR: set this entity's collision radius. The script value is in map units,
 * so it is scaled by the field's own scale and divided back down by 512. */
s32 OpcodeFuncSlidr(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("slidR", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidRange =
            (FieldEventReadMemoryU8(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(3);
    return 0;
}

/* SLDR2: SLIDR with a 16-bit radius. */
s32 OpcodeFuncSldr2(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("sldR2", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidRange =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

/* TALKR: set this entity's talk radius, scaled the same way as SLIDR. */
s32 OpcodeFuncTalkr(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("talkR", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkRange =
            (FieldEventReadMemoryU8(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(3);
    return 0;
}

/* TLKR2: TALKR with a 16-bit radius. */
s32 OpcodeFuncTlkr2(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("tlkR2", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkRange =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_state.c
/////////////////////////////////////////////////

/* MSPED: set this entity's movement speed, scaled like the radius opcodes. */
s32 OpcodeFuncMsped(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("msped", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveSpeed =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncAsped(void) {
    u8 modelIdx;
    s16 speed;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("asped", 3);
        }
        speed = FieldEventReadMemoryS16(2, 2);
        modelIdx = g_EntityToModel[g_CurrentEntity];
        g_FieldModels[modelIdx].animSpeed = speed;
        D_8009D828[modelIdx] = speed;
    }
    PC_INC(4);
    return 0;
}

/* GTDIR: write another entity's facing direction back into a memory bank. */
s32 OpcodeFuncGtdir(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("gtdir", 3);
        }
        FieldEventWriteMemoryU8(
            2, 3, g_FieldModels[g_EntityToModel[entityId]].Dir);
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncPgtdr(void) {
    u8 slot;
    u8 partyId;
    u8 actorId;

    slot = GET_PARAM_U8(2);
    if (slot < 3) {
        partyId = D_8009D391[slot];
        if (partyId != 0xFF) {
            actorId = D_8009AD30[partyId];
            if (actorId != 0xFF) {
                if (g_EntityToModel[actorId] != 0xFF) {
                    if (g_DebugLevel & 3) {
                        DebugPrintOpcode("pgtdr", 3);
                    }
                    FieldEventWriteMemoryU8(
                        2, 3, g_FieldModels[g_EntityToModel[actorId]].Dir);
                }
            }
        }
    }
    PC_INC(4);
    return 0;
}

/* GETAI: write another entity's walkmesh triangle id back into a memory bank.
 */
s32 OpcodeFuncGetai(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("getai", 3);
        }
        FieldEventWriteMemoryS16(
            2, 3, g_FieldModels[g_EntityToModel[entityId]].PosI);
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncGetaxy(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("getaxy", 4);
        }
        FieldEventWriteMemoryS16(
            1, 3, g_FieldModels[g_EntityToModel[entityId]].PosX >> 12);
        FieldEventWriteMemoryS16(
            2, 4, g_FieldModels[g_EntityToModel[entityId]].PosY >> 12);
    }
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncAxyzi(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(3);
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("axyzi", 7);
    }
    if (g_EntityToModel[entityId] != 0xFF) {
        FieldEventWriteMemoryS16(
            1, 4, g_FieldModels[g_EntityToModel[entityId]].PosX >> 12);
        FieldEventWriteMemoryS16(
            2, 5, g_FieldModels[g_EntityToModel[entityId]].PosY >> 12);
        FieldEventWriteMemoryS16(
            3, 6, g_FieldModels[g_EntityToModel[entityId]].PosZ >> 12);
        FieldEventWriteMemoryS16(
            4, 7, g_FieldModels[g_EntityToModel[entityId]].PosI);
    }
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncPxyzi(void) {
    u8 slot;
    u8 partyId;
    u8 actorId;

    slot = GET_PARAM_U8(3);
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pxyzi", 7);
    }
    if (slot < 3) {
        partyId = D_8009D391[slot];
        if (partyId < 9) {
            actorId = D_8009AD30[partyId];
            if (g_EntityToModel[actorId] != 0xFF) {
                FieldEventWriteMemoryS16(
                    1, 4, g_FieldModels[g_EntityToModel[actorId]].PosX >> 12);
                FieldEventWriteMemoryS16(
                    2, 5, g_FieldModels[g_EntityToModel[actorId]].PosY >> 12);
                FieldEventWriteMemoryS16(
                    3, 6, g_FieldModels[g_EntityToModel[actorId]].PosZ >> 12);
                FieldEventWriteMemoryS16(
                    4, 7, g_FieldModels[g_EntityToModel[actorId]].PosI);
            }
        }
    }
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncVisi(void) {
    u8 model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("visi", 1);
    }
    model = g_EntityToModel[g_CurrentEntity];
    if (model != 0xFF) {
        g_FieldModels[model].visible = GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncTlkon(void) {
    u8 model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tlkon", 1);
    }
    model = g_EntityToModel[g_CurrentEntity];
    if (model != 0xFF) {
        g_FieldModels[model].TalkOff = GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncXyzi(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyzi", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ =
            FieldEventReadMemoryS16(3, 7) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosI =
            FieldEventReadMemoryS16(4, 9);
    }
    PC_INC(11);
    return 1;
}

s32 OpcodeFuncXyz(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyz", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ =
            FieldEventReadMemoryS16(3, 7) << 12;
    }
    PC_INC(9);
    return 1;
}

s32 OpcodeFuncXyi(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyi", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosI =
            FieldEventReadMemoryS16(3, 7);
    }
    PC_INC(9);
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_message.c
/////////////////////////////////////////////////

s32 OpcodeFuncMes(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mes", 2);
    }
    if (FieldDialogMessageUpdateStates(GET_PARAM_U8(1), GET_PARAM_U8(2)) != 0) {
        PC_INC(3);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncMpnam(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpnam", 1);
    }
    CopyDialogToMapName(GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode ASK: run a menu prompt and store the chosen row.
 *
 * Blocks (returning 1 and holding the player) until FieldDialogAskUpdateStates
 * reports the prompt is finished; the answer is written back to the script
 * memory bank either way.
 */
s32 OpcodeFuncAsk(void) {
    s16 answer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ask", 6);
    }
    answer = FieldEventReadMemoryU8(2, 6);
    if (FieldDialogAskUpdateStates(
            GET_PARAM_U8(2), GET_PARAM_U8(3), GET_PARAM_U8(4), GET_PARAM_U8(5),
            &answer) != 0) {
        FieldEventWriteMemoryU8(2, 6, answer);
        g_FieldState->characterLock = D_80081DC4;
        PC_INC(7);
        return 0;
    } else {
        FieldEventWriteMemoryU8(2, 6, answer);
        g_FieldState->characterLock = 1;
        return 1;
    }
}

/////////////////////////////////////////////////
// Start of field_opcode_window.c
/////////////////////////////////////////////////

s32 OpcodeFuncWclsEx(void) {
    s16 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wcls!", 0);
    }
    window = GET_PARAM_U8(1);
    if (D_8008326C[window] == 0xFF) {
        PC_INC(2);
        return 0;
    }
    FieldWindowSetStateToClose(window);
    FieldDialogMessageUpdateStates(window, 0);
    return 1;
}

s32 OpcodeFuncWsizw(void) {
    s16 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wsizw", 8);
    }
    window = GET_PARAM_U8(1);
    if (D_8008326C[window] == 0xFF) {
        return OpcodeFuncWsize();
    }
    if (D_8008326C[window] == g_CurrentEntity) {
        FieldWindowSetStateToClose(window);
        FieldDialogMessageUpdateStates(window, 0);
    }
    return 1;
}

s32 OpcodeFuncWsize(void) {
    s16 x;
    s16 y;
    s16 w;
    s16 h;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wsize", 8);
    }
    GET_PARAM_S16(x, 2);
    GET_PARAM_S16(y, 4);
    GET_PARAM_S16(w, 6);
    GET_PARAM_S16(h, 8);
    FieldDialogSetSize(GET_PARAM_U8(1), x, y, w, h);
    PC_INC(10);
    /* Not cosmetic: the statement boundary stops gcc sinking `move v0,zero`
     * into the load delay slot of the PC_INC read, which is what forces the
     * original's $v0 for the incremented value and its trailing `nop`.
     * Most likely a macro in the original. Found by decomp-permuter. */
    do {
        return 0;
    } while (0);
}

s32 OpcodeFuncWrow(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wrow", 2);
    }
    FieldDialogSetWindowHeight(GET_PARAM_U8(1), (GET_PARAM_U8(2) << 4) | 9);
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncWmove(void) {
    s16 dx;
    s16 dy;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wmove", 8);
    }
    GET_PARAM_S16(dx, 2);
    GET_PARAM_S16(dy, 4);
    FieldDialogMove(GET_PARAM_U8(1), dx, dy);
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncWrest(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wrest", 1);
    }
    FieldWindowReset(GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncWclse(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wclse", 1);
    }
    if (FieldWindowSetStateToClose(GET_PARAM_U8(1)) != 0) {
        PC_INC(2);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncWmode(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wmode", 3);
    }
    FieldDialogSetWindowStyleCbc(
        GET_PARAM_U8(1), GET_PARAM_U8(2), GET_PARAM_U8(3));
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_math.c
/////////////////////////////////////////////////

/**
 * @brief Opcode 0x8F - **AND** - Bitwise AND (8-bit)
 *
 * Memory layout:
 *
 * | 0x8F | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise AND and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise AND.
 * @details
 * Performs a bitwise AND operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the "Oper" is the
 * operand to AND with. If the Source Bank is an 8 bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncAnd(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("and", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) & FieldEventReadMemoryU8(2, 3) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x90 - **AND2** - Bitwise AND (16-bit)
 *
 * Memory layout:
 *
 * | 0x90 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise AND, and
 * that which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise AND, or address of the
 * second operand, if S is non-zero.
 * @details
 * Performs a bitwise AND operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then the "Oper" is the
 * operand to AND with. If the Source Bank is a 16-bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncAnd2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("and2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x91 - **OR** - Bitwise OR (8-bit)
 *
 * Memory layout:
 *
 * | 0x91 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise OR and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise OR.
 * @details
 * Performs a bitwise OR operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the "Oper" is the
 * operand to OR with. If the Source Bank
 * is an 8 bit bank, then the "Oper" is the address in that bank where
 * the operand is.
 */
s32 OpcodeFuncOr(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("or", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) | FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x92 - **OR2** - Bitwise OR (16-bit)
 *
 * Memory layout:
 *
 * | 0x92 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise OR, and that
 * which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise OR, or address of the
 * second operand, if S is non-zero
 * @details
 * Performs a bitwise OR operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then "Oper" is the operand
 * to OR with. If the Source Bank is a 16-bit bank, then the "Oper" is the
 * address in that bank where the operand is.
 */
s32 OpcodeFuncOr2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("or2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x93 - **XOR** - Bitwise XOR (8-bit)
 *
 * Memory layout:
 *
 * | 0x93 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise XOR and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise XOR.
 * @details
 * Performs a bitwise XOR operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the Operis the operand
 * to XOR with. If the Source Bank is an 8 bit bank, then the "Oper" is the
 * address in that bank where the operand is.
 */
s32 OpcodeFuncXor(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("xor", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) ^ FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x94 - **XOR2** - Bitwise XOR (16-bit)
 *
 * Memory layout:
 *
 * | 0x94 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise XOR, and
 * that which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise XOR, or address of the
 * second operand, if S is non-zero.
 * @details
 * Performs a bitwise XOR operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then the "Oper" is the
 * operand to XOR with. If the Source Bank is a 16-bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncXor2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("xor2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x85 - **PLUS** - Addition (8-bit)
 *
 * Memory layout:
 *
 * | 0x85 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const UByte Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into  "Dest" . The
 * result of the addition wraps around into the range of 0-255. If the Source
 * Bank is 0 then the "Oper" is added to the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncPlus(void) {
    u16* temp_a0;
    u8 temp_s0;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) + FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x76 - **PLUS!** - Saturated Addition (8-bit)
 *
 * Memory layout:
 *
 * | 0x76 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const UByte Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into "Dest". The result
 * of the addition is capped at 255. If the Source Bank is 0 then the "Oper" is
 * added to the destination value. If the Source Bank is an 8 bit bank, then the
 * "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncPlusEx(void) {
    u8 a;
    u8 b;
    s16 sum;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus!", 3);
    }

    a = FieldEventReadMemoryU8(1, 2);
    b = FieldEventReadMemoryU8(2, 3);
    sum = a + b;
    if (sum > 255) {
        sum = 255;
    }

    FieldEventWriteMemoryU8(1, 2, sum);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x86 - **PLUS2** - Addition (16-bit)
 *
 * Memory layout:
 *
 * | 0x86 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const SWord Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into  "Dest" . The
 * result of the addition wraps around into the 16-bit range. If the Source Bank
 * is 0 then the "Oper" is added to the destination value. If the
 * Source Bank is an 16 bit bank, then the "Oper" is the address in that bank
 * where the operand is.
 */
s32 OpcodeFuncPlus2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) + FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x77 - **PLS2!** - Saturated Addition (16-bit)
 *
 * Memory layout:
 *
 * | 0x77 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const SWord Oper: The operand, added to the destination
 * @details
 * Adds two numbers together and stores the result back into "Dest" The result
 * of the addition is capped at 32767. The result is not capped at the negative
 * end, however (-32768), so adding two large negative numbers together will
 * still produce wrap-around. If the Source Bank is 0 then the "Oper" is added
 * to the destination value. If the Source Bank is an 16 bit bank, then the
 * "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncPls2Ex(void) {
    s16 a;
    s16 b;
    s32 sum;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pls2!", 3);
    }
    a = FieldEventReadMemoryS16(1, 2);
    b = FieldEventReadMemoryS16(2, 3);
    sum = a + b;
    if (sum > 0x7FFF) {
        sum = 0x7FFF;
    }
    FieldEventWriteMemoryS16(1, 2, sum);
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x87 - **MINUS** - Subtraction (8-bit)
 *
 * Memory layout:
 *
 * | 0x87 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const UByte Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts two numbers and stores the result back into  "Dest" . The result of
 * the subtraction wraps around into the range of 0-255. If the Source Bank is 0
 * then the "Oper" is subtracted from the destination value. If the Source Bank
 * is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMinus(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("minus", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) - FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x78 - **MINS!** - Saturated Subtraction (8-bit)
 *
 * Memory layout:
 *
 * | 0x78 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const UByte Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts "Oper" from "Dest" and stores the result back into "Dest". The
 * result of the subtraction is capped at 0. If the Source Bank is 0 then the
 * "Oper" is subtracted from the destination value. If the Source Bank is an 8
 * bit bank, then the "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncMinsEx(void) {
    u8 a;
    u8 b;
    s16 differ;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mins!", 3);
    }
    a = FieldEventReadMemoryU8(1, 2);
    b = FieldEventReadMemoryU8(2, 3);
    differ = a - b;
    if (differ < 0) {
        differ = 0;
    }
    FieldEventWriteMemoryU8(1, 2, differ & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x88 - **MINS2** - Subtraction (16-bit)
 *
 * Memory layout:
 *
 * | 0x98 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const SWord Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts two numbers and stores the result back into "Dest". The result of
 * the subtraction wraps around into the 16-bit range. If the Source Bank is 0
 * then the "Oper" is subtracted from the destination value. If the Source Bank
 * is an 16 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMins2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mins2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) - FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x79 - **MNS2!** - Saturated Subtraction (16-bit)
 *
 * Memory layout:
 *
 * | 0x79 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const SWord Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts "Oper" from "Dest" and stores the result back into "Dest". The
 * result of the subtraction is capped at -32768. The result is not capped at
 * the positive end (32767), so subtracting a large negative number from a large
 * positive number will still produce wrap-around. If the Source Bank is 0 then
 * the "Oper" is subtracted from the destination value. If the
 * Source
 * Bank is an 16 bit bank, then the "Oper" is the address in that bank
 * where the operand is.
 */
s32 OpcodeFuncMns2Ex(void) {
    s16 a;
    s16 b;
    s32 differ;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mns2!", 3);
    }
    a = FieldEventReadMemoryS16(1, 2);
    b = FieldEventReadMemoryS16(2, 3);
    differ = a - b;
    if (differ <= 0x7FFF) {
        differ = 0x8000;
    }
    FieldEventWriteMemoryS16(1, 2, differ);
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x89 - **MUL** - Multiplication (8-bit)
 *
 * Memory layout:
 *
 * | 0x89 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * multiplied.
 * - const UByte Oper: The operand, which is multiplied with the destination.
 * @details
 * Multiplies two numbers together and stores the result back into "Dest". The
 * result of the Multiplication is capped at 255. If the Source Bank is 0 then
 * the the value "Oper" is multiplied with the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMul(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mul", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) * FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8A - **MUL2** - Multiplication (16-bit)
 *
 * Memory layout:
 *
 * | 0x8A | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * multiplied.
 * - const SWord Oper: The operand, which is multiplied with the destination.
 * @details
 * Multiplies two numbers together and stores the result back into "Dest". The
 * result of the Multiplication is capped at 32767. If the Source Bank is 0 then
 * the the value "Oper" is multiplied with the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMul2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mul2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) * FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x8B - **DIV** - Division (8-bit)
 *
 * Memory layout:
 *
 * | 0x8B | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * quotient.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the result back into "Dest". The result of
 * the division is rounded towards zero to the nearest integer. If the Source
 * Bank is 0 then the "Den" is the denominator. If the Source Bank is an 8 bit
 * bank, then the "Den" is the address in that bank where the denominator is.
 */
s32 OpcodeFuncDiv(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("div", 3);
    }
    FieldEventWriteMemoryU8(1, 2,
                            (u8)((u32)(FieldEventReadMemoryU8(1, 2) & 0xFF) /
                                 (u32)(FieldEventReadMemoryU8(2, 3) & 0xFF)));
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8C - **DIV2** - Division (16-bit)
 *
 * Memory layout:
 *
 * | 0x8C | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * quotient.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the result back into "Dest". The
 * result of the division is rounded towards zero to the nearest integer. If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 8 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncDiv2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("div2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) / FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x8D - **REMAI** - Modulus (8-bit)
 *
 * Memory layout:
 *
 * | 0x8D | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * remainder.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the remainder back into "Dest". If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 8 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncRemai(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("remai", 3);
    }
    FieldEventWriteMemoryU8(1, 2,
                            (u8)((u32)(FieldEventReadMemoryU8(1, 2) & 0xFF) %
                                 (u32)(FieldEventReadMemoryU8(2, 3) & 0xFF)));
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8E - **REMA2** - Modulus (16-bit)
 *
 * Memory layout:
 *
 * | 0x8E | D/S | Dest | Den
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * remainder.
 * - const SWord Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the remainder back into "Dest". If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 16 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncRema2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rema2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) % FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x95 - **INC** - Increment (8-bit)
 *
 * Memory layout:
 *
 * | 0x95 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Increments the 8-bit value found at bank B, address A. If the value is 0xFF,
 * it will roll over to 0x00. If you specify a 16-bit bank, only the lower byte
 * will be incremented, and if the lower byte is 0xFF, the higher byte will be
 * unaffected whilst the lower byte will return to 0x00.
 */
s32 OpcodeFuncInc(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc", 2);
    }
    FieldEventWriteMemoryU8(2, 2, (FieldEventReadMemoryU8(2, 2) + 1) & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7A - **INC!** - Saturated Increment (8-bit)
 *
 * Memory layout:
 *
 * | 0x7A | 0/D | Dest |
 *
 * - const Bit[4] D: Destination bank
 * - const UByte Dest: The destination address in the bank where the variable is
 * incremented.
 * @details
 * Increments the value in "Dest" by 1. The result is capped at 255.
 */
s32 OpcodeFuncIncEx(void) {
    s16 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc!", 2);
    }
    result = (FieldEventReadMemoryU8(2, 2) & 0xFF) + 1;
    if (result >= 0x100) {
        result = 0xFF;
    }
    FieldEventWriteMemoryU8(2, 2, result & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode  0x96 - **INC2** - Increment (16-bit)
 *
 * Memory layout:
 *
 * | 0x96 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Increments the 16-bit value found at bank B, address A. If the value is
 * 0xFFFF, it will roll over to 0x0000.
 */
s32 OpcodeFuncInc2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc2", 3);
    }
    FieldEventWriteMemoryS16(2, 2, (s16)(FieldEventReadMemoryS16(2, 2) + 1));
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode  0x7B - **INC2!** - Saturated Increment (16-bit)
 *
 * Memory layout:
 *
 * | 0x7B | 0/D | Dest |
 *
 * - const UByte B: Destination bank.
 * - const UByte Dest: The destination address in the bank where the variable is
 * incremented.
 * @details
 * Increments the value in "Dest" by 1. The result is capped at
 * 32767.
 */
s32 OpcodeFuncInc2Ex(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc2!", 3);
    }
    result = FieldEventReadMemoryS16(2, 2) + 1;
    if (result > 0x7FFF) {
        result = 0x7FFF;
    }
    FieldEventWriteMemoryS16(2, 2, result);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x97 - **DEC** - Decrement (8-bit)
 *
 * Memory layout:
 *
 * | 0x97 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Decrements the 8-bit value found at bank B, address A. If the value is
 * 0x00, it will roll over to 0xFF. If you specify a 16-bit bank, only the
 * lower byte will be decremented, and if the lower byte is 0x00, the higher
 * byte will be unaffected whilst the lower byte will return to 0xFF.
 */
s32 OpcodeFuncDec(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec", 2);
    }
    FieldEventWriteMemoryU8(2, 2, (FieldEventReadMemoryU8(2, 2) - 1) & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7C - **DEC!** - Saturated Decrement (8-bit)
 *
 * Memory layout:
 *
 * | 0x7C | 0/D | Dest |
 *
 * - const UByte B: Destination bank.
 * - const UByte Dest: The destination address in the bank where the variable is
 * deccremented.
 * @details
 * Decreases the value in "Dest" by 1. The result is capped at 0.
 */
s32 OpcodeFuncDecEx(void) {
    s16 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec!", 2);
    }
    result = (FieldEventReadMemoryU8(2, 2) & 0xFF) - 1;
    if (result < 0) {
        result = 0;
    }
    FieldEventWriteMemoryU8(2, 2, result & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x98 - **DEC2** - Decrement (16-bit)
 *
 * Memory layout:
 *
 * | 0x98 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Decrements the 16-bit value found at bank B, address A. If the value is
 * 0x0000, it will roll over to 0xFFFF.
 */
s32 OpcodeFuncDec2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec2", 3);
    }
    FieldEventWriteMemoryS16(2, 2, (s16)(FieldEventReadMemoryS16(2, 2) - 1));
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7D - **DEC2!** - Saturated Decrement (16-bit)
 *
 * Memory layout:
 *
 * | 0x7D | 0/D | Dest |
 *
 * - const Bit[4] D: Destination bank
 * - const UByte Dest: The destination address in the bank where the variable is
 * Decremented.
 * @details
 * Decreases the value in "Dest" by 1. The result is capped at -32768.
 */
s32 OpcodeFuncDec2Ex(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec2!", 3);
    }
    result = FieldEventReadMemoryS16(2, 2) - 1;
    if (result <= 0x7FFF) {
        result = 0x8000;
    }
    FieldEventWriteMemoryS16(2, 2, result);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x99 - **RANDM** - Random
 *
 * Memory layout:
 *
 * | 0x99 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Destination address.
 * @details
 * Places a random 8-bit value into the destination bank and address specified.
 * If you specify a 16-bit bank, only the lower byte is randomised.
 */
s32 OpcodeFuncRandm(void) {
    u16* temp_v1;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("randm", 2);
    }
    g_RandomTableIndex += g_RandomTableStep;
    FieldEventWriteMemoryU8(2, 2, g_RandomTable[g_RandomTableIndex]);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7F - **RDMSD** - Seed Random Generator
 *
 * Memory layout:
 *
 * | 0x7F | B | S |
 *
 * - const UByte B: Bank in which the seed value is stored, or zero if S is
 * specified as a literal value.
 * - const UByte A: Destination address.
 * @details
 * Seeds the random number generator used by RANDOM. The lower four bits of the
 * arguments are used as the seed value by altering the offset used to take a
 * value from the table of pseudo-random numbers.
 */
s32 OpcodeFuncRdmsd(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rdmsd", 2);
    }
    g_RandomTableStep = (FieldEventReadMemoryU8(2, 2) << 4) + 1;
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_background.c
/////////////////////////////////////////////////

s32 OpcodeFuncBgon(void) {
    u8 layer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgon", 3);
    }
    layer = FieldEventReadMemoryU8(1, 2);
    g_FieldState->backgroundLayerVisibility[layer] |=
        1 << FieldEventReadMemoryU8(2, 3);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBgoff(void) {
    u8 layer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgoff", 3);
    }
    layer = FieldEventReadMemoryU8(1, 2);
    g_FieldState->backgroundLayerVisibility[layer] &=
        ~(1 << FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBgclr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgclr", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] = 0;
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBgrol(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgrol", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] <<= 1;
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBgrol2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgrol", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] >>= 1;
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_movie.c
////////////////////////////////////////////////

/* Preload the movie named by the parameter, blocking until the load finishes.
 * Same post-then-poll shape as OpcodeFuncMovie, one event command earlier. */
s32 OpcodeFuncPmvie(void) {
    s16 movieId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmvie", 1);
    }
    if (D_800716CC != 0) {
        PC_INC(2);
        return 0;
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_LOAD_MOVIE:
        switch (g_FieldState->movieCommandState) {
        case MOVCMD_ACTIVE:
            break;
        case MOVCMD_DONE:
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(2);
            return 0;
        }
        return 1;
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_LOAD_MOVIE;
        movieId = GET_PARAM_U8(1);
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        g_FieldState->eventCmdParam = movieId;
        break;
    }
    return 1;
}

/* Play the field map's movie, blocking until it finishes. Returning 1 without
 * advancing the PC re-runs the opcode next frame, so the request is posted once
 * as an event command and then polled. */
s32 OpcodeFuncMovie(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("movie", 0);
    }
    g_FieldMovieOpcodeActive = 1;
    if (D_800716CC != 0) {
        D_801144D4 = 0;
        PC_INC(1);
        return 0;
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_PLAY_MOVIE:
        switch (g_FieldState->movieCommandState) {
        case MOVCMD_ACTIVE:
            break;
        case MOVCMD_DONE:
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(1);
            return 0;
        }
        return 1;
    case EVTCMD_UNK14:
        PC_INC(1);
        return 0;
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_PLAY_MOVIE;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        break;
    }
    return 1;
}

s32 OpcodeFuncMvief(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mvief", 2);
    }
    if (D_800716CC != 0) {
        FieldEventWriteMemoryS16(2, 2, D_801144D4);
        D_801144D4++;
        PC_INC(3);
        return 0;
    } else {
        FieldEventWriteMemoryS16(2, 2, g_FieldState->currentMovieFrame);
        PC_INC(3);
        return 0;
    }
}

s32 OpcodeFuncMpjpo(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpjpo", 0);
    }
    g_FieldState->mapJumpDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_scroll.c
////////////////////////////////////////////////

s32 OpcodeFuncScr2d(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2d", 5);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_INSTANT;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 2);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 4);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncScrlc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlc", 0);
    }
    g_FieldState->cameraScrollMode = GET_PARAM_U8(4);
    g_FieldState->cameraScrollTargetId = g_FieldState->pcModelId;
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(5);
    return 0;
}

/* Scroll the camera to an entity over a number of frames. Unlike SCR2D the
 * target is an entity id, so a missing model makes the opcode a no-op. */
s32 OpcodeFuncScrla(void) {
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrla", 0);
    }
    entityId = GET_PARAM_U8(4);
    if (g_EntityToModel[entityId] != 0xFF) {
        g_FieldState->cameraScrollMode = GET_PARAM_U8(5);
        g_FieldState->cameraScrollTargetId = g_EntityToModel[entityId];
        g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
        g_FieldState->cameraScrollState = 0;
    }
    PC_INC(6);
    return 0;
}

/* SCRLP is SCRLA addressed by party slot rather than by entity: the slot picks
 * a character, the character picks the field entity that represents them.
 *
 * The copy back into partyId is load-bearing, not redundant. Indexing
 * g_EntityToModel with actorId directly widens it in place as
 * `andi a1,v0,0xff`, where the original holds the resolved actor in v0 and
 * copies it out with a plain `move`. Going through the (by now dead) slot
 * variable is what produces that copy. Found by decomp-permuter. */
s32 OpcodeFuncScrlp(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlp", 0);
    }
    partyId = D_8009D391[GET_PARAM_U8(4)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    partyId = actorId;
    if (g_EntityToModel[partyId] != 0xFF) {
        g_FieldState->cameraScrollMode = GET_PARAM_U8(5);
        g_FieldState->cameraScrollTargetId = g_EntityToModel[partyId];
        g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
        g_FieldState->cameraScrollState = 0;
    }
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncScrcc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrcc", 0);
    }
    g_FieldState->cameraScrollMode = SCRL_OFF;
    g_FieldState->cameraScrollTargetId = g_FieldState->pcModelId;
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncScr2dc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2dc", 8);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_SMOOTH;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 3);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 5);
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(4, 7);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(9);
    return 0;
}

s32 OpcodeFuncScr2dl(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2dl", 8);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_LINEAR;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 3);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 5);
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(4, 7);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(9);
    return 0;
}

s32 OpcodeFuncScrlw(void) {
    s32 mode;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlw", 0);
    }
    if (g_FieldState->cameraScrollState == SCRLST_DONE) {
        mode = g_FieldState->cameraScrollMode;
        if (mode != SCRL_OFF) {
            if (mode < SCRL_TO_COORDS_INSTANT) {
                g_FieldState->cameraScrollMode = SCRL_TO_ENTITY_INSTANT;
            } else if (mode < 7) {
                if (mode >= SCRL_TO_COORDS_LINEAR) {
                    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_INSTANT;
                }
            }
        }
        g_FieldState->cameraScrollState = SCRLST_INIT;
        PC_INC(1);
        return 0;
    }
    return 1;
}

/////////////////////////////////////////////////
// Begin of field_opcode_palette.c
////////////////////////////////////////////////

s32 OpcodeFuncStpal(void) {
    RECT rect;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpal", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = FieldEventReadMemoryU8(1, 2) + 0x1E0;
    rect.x = 0;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    StoreImage(&rect, (u_long*)&D_80095DE0[FieldEventReadMemoryU8(2, 3) * 32]);
    PC_INC(5);
    return 0;
}

/* STPAL with a start entry: the run of colours saved out of VRAM begins `x`
 * entries into the palette rather than at entry 0.
 *
 * Four instructions out, and the residue is which addend &D_80095DE0 joins.
 * The original groups the address as pal*32 + (base + x*2); gcc groups it
 * (base + pal*32) + x*2, materialising the symbol after the palette id is
 * loaded rather than before.
 *
 * Rewriting the expression does not move it. Twenty-odd phrasings -- the two
 * offsets as one index or as separate addends, either operand order, a u16 or
 * u_long view of the base, the pointer sum cast through u32, the palette id
 * hoisted into a local -- all compile to the same four rows, which says gcc
 * canonicalises the address tree before it lays anything out. Assigning
 * (base + x*2) to a local *does* produce the target's grouping, but the extra
 * pseudo shifts the script pointer out of $a1 and costs twenty rows elsewhere.
 * decomp-permuter got 265 -> 185 and no further in 13k iterations, and its one
 * find was retyping the extern to short, which is not the same address. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncStpls);
#else
s32 OpcodeFuncStpls(void) {
    RECT rect;
    s16 x;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(1) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    StoreImage(&rect, (u_long*)&D_80095DE0[GET_PARAM_U8(2) * 32 + x * 2]);
    PC_INC(5);
    return 0;
}
#endif

s32 OpcodeFuncLdpal(void) {
    RECT rect;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpal", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = FieldEventReadMemoryU8(2, 3) + 0x1E0;
    rect.x = 0;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    LoadImage(&rect, (u_long*)&D_80095DE0[FieldEventReadMemoryU8(1, 2) * 32]);
    PC_INC(5);
    return 0;
}

/* LDPAL with a start entry; same address-grouping residue as OpcodeFuncStpls
 * above, and the same phrasings have been tried against it. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncLdpls);
#else
s32 OpcodeFuncLdpls(void) {
    RECT rect;
    s16 x;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(2) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    LoadImage(&rect, (u_long*)&D_80095DE0[GET_PARAM_U8(1) * 32 + x * 2]);
    PC_INC(5);
    return 0;
}
#endif

void FieldEventRectClear(s16* arg0) {
    arg0[0] = 0;
    arg0[1] = 0;
    arg0[2] = 0;
    arg0[3] = 0;
}

/* Copy the first `count` entries of one 16-colour palette over another. The
 * palette store is a flat byte array of 32-byte pages, so both ends have to be
 * re-cast to u16 to walk entries rather than bytes. Declaring the two pointers
 * inside the loop is what makes gcc hoist each as one invariant; written above
 * the loop they land ahead of the zero-trip guard, and written inline gcc
 * reassociates the base out and the body needs a third `addu`.
 *
 * One instruction from matching: the original materialises &D_80095DE0 between
 * the `andi` that widens the palette id and the `sll` that scales it, gcc after
 * both. That is the same residue as OpcodeFuncStpls -- gcc will not put the
 * symbol's address early -- and it survives every way of writing the address:
 * casting the sum or the base, indexing in bytes or in u16 entries, `<< 5`
 * against `* 32`, and even redeclaring D_80095DE0 as the `u16[][16]` it
 * actually is and letting the scale come from the element type. All seven
 * phrasings produce byte-identical output, which says gcc canonicalises the
 * address tree well before it decides an order.
 *
 * The two are a .rodata unit -- OpcodeFuncCppal owns the "cppal" string that
 * OpcodeFuncCppal2 prints -- so neither can land without the other. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncCppal);
#else
s32 OpcodeFuncCppal(void) {
    s16 count;
    u8 src;
    u8 dst;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cppal", 4);
    }
    count = GET_PARAM_U8(4) + 1;
    src = FieldEventReadMemoryU8(1, 2);
    dst = FieldEventReadMemoryU8(2, 3);
    for (i = 0; i < count; i++) {
        u16* dstPal = (u16*)(D_80095DE0 + dst * 32);
        u16* srcPal = (u16*)(D_80095DE0 + src * 32);

        dstPal[i] = srcPal[i];
    }
    PC_INC(5);
    return 0;
}
#endif

/* As CPPAL, but source and destination each get their own start entry, so the
 * copy can shift a run of colours within or between palettes.
 *
 * Instruction-for-instruction identical; the preheader swaps $v0 and $v1
 * between the scaled index and the base address. Reversing the operand order in
 * the C does not move it. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncCppal2);
#else
s32 OpcodeFuncCppal2(void) {
    s16 count;
    s16 srcPal;
    s16 dstPal;
    s16 src;
    s16 dst;
    s16 end;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cppal", 7);
    }
    count = FieldEventReadMemoryU8(4, 7) + 1;
    srcPal = GET_PARAM_U8(3);
    dstPal = GET_PARAM_U8(4);
    src = FieldEventReadMemoryU8(1, 5);
    dst = FieldEventReadMemoryU8(2, 6);
    end = src + count;
    while (src < end) {
        u16* to = (u16*)(D_80095DE0 + dstPal * 32);
        u16* from = (u16*)(D_80095DE0 + srcPal * 32);

        to[dst] = from[src];
        src++;
        dst++;
    }
    PC_INC(8);
    return 0;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncRtpal);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncRtpal2);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncAdpal);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncAdpal2);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMppal2);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMppal);

static void SetPcModel(void) {
    if (Savemap.memory_bank_2[9] != 0xFF &&
        g_CharIdToEntity[Savemap.memory_bank_2[9]] != 0xFF &&
        g_EntityToModel[g_CharIdToEntity[Savemap.memory_bank_2[9]]] != 0xFF) {
        g_FieldState->pcModelId =
            g_EntityToModel[g_CharIdToEntity[Savemap.memory_bank_2[9]]];
    }
}

s32 OpcodeFuncPc(void) {
    u8 charId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pc", 1);
    }

    charId = GET_PARAM_U8(1);
    g_CharIdToEntity[charId] = g_CurrentEntity;

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            if (i != 0) {
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 0;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff = 1;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkOff = 1;
            } else {
                g_FieldState->pcModelId = g_EntityToModel[g_CurrentEntity];
            }

            PC_INC(2);
            return 0;
        }
    }

    g_CharIdToEntity[charId] = g_CurrentEntity;

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkOff = 1;

    PC_INC(2);
    return 0;
}

s32 OpcodeFuncPrtyp(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtyp", 1);
    }

    charId = GET_PARAM_U8(1);
    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            PC_INC(2);
            SetPcModel();
            PartyFromBank2ToSave(0);
            return 0;
        }
    }

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == 0xFF) {
            ADD_PARTY_MEMBER(i, charId);

            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2(
                    "p+ ef=", g_CharIdToEntity[charId], 2);
            }
            PC_INC(2);
            SetPcModel();
            PartyFromBank2ToSave(1);
            return 0;
        }
    }

    ADD_PARTY_MEMBER(2, charId);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("p+ lf=", g_CharIdToEntity[charId], 2);
    }
    PC_INC(2);
    SetPcModel();
    PartyFromBank2ToSave(1);
    return 0;
}

s32 OpcodeFuncPrtym(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtym", 1);
    }

    charId = GET_PARAM_U8(1);

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            Savemap.memory_bank_2[9 + i] = 0xFF;
            PartyFromBank2ToSave(1);
            SetPcModel();
            PC_INC(2);
            return 0;
        }
    }

    PartyFromBank2ToSave(1);
    SetPcModel();
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncPrtye(void) {
    u8 newParty[3];
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtye", 3);
    }

    for (i = 0; i < 3; i++) {
        newParty[i] = (&GET_PARAM_U8(1))[i];
    }

    PartyReplace(newParty);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncSptye(void) {
    u8 newParty[3];
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sptye", 5);
    }

    for (i = 0; i < 3; i++) {
        newParty[i] = FieldEventReadMemoryU8(1 + i, 3 + i);
    }

    PartyReplace(newParty);
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncGptye(void) {
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gptye", 5);
    }

    for (i = 0; i < 3; i++) {
        FieldEventWriteMemoryU8(1 + i, 3 + i, Savemap.memory_bank_2[9 + i]);
    }
    PC_INC(6);
    return 0;
}

// Partial replace of bank 2 party with newParty.
// Any free slots in newParty are taken up by members of bank 2 party.
// The result is then transferred to main party in save.
void PartyReplace(u8* newParty) {
    s32 i, j;

    // Remove requested members from old party.
    for (i = 0; i < 3; i++) {
        if (newParty[i] != 0xFF) {
            for (j = 0; j < 3; j++) {
                if (newParty[i] == Savemap.memory_bank_2[9 + j]) {
                    Savemap.memory_bank_2[9 + j] = 0xFF;
                }
            }
        }
    }

    // Add remaining members of old party to empty slots in new party.
    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] != 0xFF) {
            for (j = 0; j < 3; j++) {
                if (newParty[j] == 0xFF) {
                    newParty[j] = Savemap.memory_bank_2[9 + i];
                    j = 3;
                }
            }
        }
    }

    // Overwrite old party with new party.
    for (i = 0; i < 3; i++) {
        // Convert forced empty slots to regular empty slots.
        if (newParty[i] == 0xFE) {
            newParty[i] = 0xFF;
        }

        ADD_PARTY_MEMBER(i, newParty[i]);
    }

    PartyFromBank2ToSave(1);
    SetPcModel();
}

// Compares two sets of parties and returns which members don't exist in both.
static void PartyCompare(
    u8* party1, u8* party2, u8* party2Only, u8* party1Only) {
    s32 i, j, k;

    for (i = 0; i < 3; i++) {
        party2Only[i] = 0xFF;
        party1Only[i] = 0xFF;
    }

    k = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party2[i] == party1[j]) {
                goto foundInParty1;
            }
        }
        party2Only[k++] = party2[i];
    foundInParty1:;
    }

    k = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party1[i] == party2[j]) {
                goto foundInParty2;
            }
        }
        party1Only[k++] = party1[i];
    foundInParty2:;
    }
}

// Transfers party from bank 2 to save while preserving order in save of
// characters existing in both parties.
void PartyFromBank2ToSave(s32 unused) {
    u8 notInSave[3];
    u8 notInBank2[3];

    PartyCompare(
        Savemap.partyID, &Savemap.memory_bank_2[9], notInSave, notInBank2);
    PartyRemove(Savemap.partyID, notInBank2);
    PartyAdd(Savemap.partyID, notInSave);
    g_PartyUpdatedByFieldScript = 1;
}

// Transfers party from save to bank 2 while preserving order in bank 2 of
// characters existing in both parties.
void PartyFromSaveToBank2(void) {
    u8 notInBank2[3];
    u8 notInSave[3];

    PartyCompare(
        &Savemap.memory_bank_2[9], Savemap.partyID, notInBank2, notInSave);
    PartyRemove(&Savemap.memory_bank_2[9], notInSave);
    PartyAdd(&Savemap.memory_bank_2[9], notInBank2);
}

void PartyRemove(u8* party, u8* toRemove) {
    s32 i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (toRemove[i] == party[j]) {
                party[j] = 0xFF;
            }
        }
    }
}

// Adds characters from toAdd to the first free slots in party.
// Does not use force freed slots.
void PartyAdd(u8* party, u8* toAdd) {
    s32 i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party[j] == 0xFF) {
                party[j] = toAdd[i];
                break;
            }
        }
    }
}

s32 OpcodeFuncPrtyq(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtyq", 2);
    }

    charId = GET_PARAM_U8(1);

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("prty=TRUE", 0, 0);
            }
            PC_INC(3);
            return 0;
        }
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("prty=FALSE", 0, 0);
    }
    PC_INC(GET_PARAM_U8(2) + 2);
    return 0;
}

s32 OpcodeFuncMembq(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("membq", 2);
    }

    if ((1 << GET_PARAM_U8(1)) & Savemap.phs_visibility_mask) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("memb=TRUE", 0, 0);
        }
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("memb=FALSE", 0, 0);
    }
    PC_INC(GET_PARAM_U8(2) + 2);
    return 0;
}

s32 OpcodeFuncMmbPlusMinus(void) {
    s16 i;
    s16 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmb+-", 3);
    }

    charId = GET_PARAM_U8(2);

    if (GET_PARAM_U8(1)) {
        Savemap.phs_visibility_mask |= 1 << charId;
    } else {
        Savemap.phs_visibility_mask &= ~(1 << charId);
        for (i = 0; i < 3; i++) {
            if (Savemap.memory_bank_2[9 + i] == charId) {
                Savemap.memory_bank_2[9 + i] = 0xFF;
            }
        }
    }

    PC_INC(3);
    return 0;
}

s32 OpcodeFuncMmblk(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmblk", 3);
    }

    Savemap.phs_locking_mask |= 1 << GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMmbuk(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmbuk", 3);
    }

    Savemap.phs_locking_mask &= ~(1 << GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncSolid(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("solid", 1);
    }

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff =
            GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

/* Set the camera's view offset. A non-zero mode eases from the current offset
 * to the target over N steps; mode 0 applies it immediately and clears the
 * animation state. */
/* Every instruction matches except the tail merge: gcc cross-jumps the whole
 * shared PC_INC(7) tail, where the original keeps the
 * &g_FieldScriptPC[g_CurrentEntity] computation duplicated in both arms. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncVwoft);
#else
s32 OpcodeFuncVwoft(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("vwoft", 6);
    }
    if (GET_PARAM_U8(6)) {
        g_FieldState->viewOffsetStart = g_FieldState->viewOffset;
        g_FieldState->viewOffsetTarget = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetNumSteps = FieldEventReadMemoryS16(2, 4);
        g_FieldState->viewOffsetMode = GET_PARAM_U8(6);
        g_FieldState->viewOffsetCurrentStep = 0;
    } else {
        g_FieldState->viewOffsetNumSteps = 0;
        g_FieldState->viewOffset = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetCurrentStep = 0;
        g_FieldState->viewOffsetMode = 0;
        g_FieldState->viewOffsetStart = 0;
        g_FieldState->viewOffsetTarget = 0;
    }
    PC_INC(7);
    return 0;
}
#endif

/////////////////////////////////////////////////
// Begin of field_opcode_party_manage.c
/////////////////////////////////////////////////

s32 FieldEventJoinSet(u8, u8); // extern

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncJoin);
#else
/* 25 rows: gcc hoists the 0xFF constant into a saved reg ($s1) for the two
 * memory_bank_2[10]/[11] compares and reuses it across both FieldEventJoinSet
 * calls; target reloads `li v0,0xff` per compare and keeps the stack frame at
 * -0x18 (no $s1 save). GET_PARAM_U8(1) shared by both calls is the hoist
 * trigger. g_FieldModels idiom (not g_FieldEntity) was the key fix that cut
 * 66->25 rows. Polarity flip and block-scope arg temp both plateau at 25. */
s32 OpcodeFuncJoin(void) {
    s32 joinOk;
    s32 splitOk;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("join", 1);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    joinOk = 1;
    if (Savemap.memory_bank_2[10] != 0xFF) {
        joinOk = FieldEventJoinSet(
            g_CharIdToEntity[Savemap.memory_bank_2[10]], GET_PARAM_U8(1));
    }
    if (Savemap.memory_bank_2[11] != 0xFF) {
        splitOk = FieldEventJoinSet(
            g_CharIdToEntity[Savemap.memory_bank_2[11]], GET_PARAM_U8(1));
    } else {
        splitOk = 1;
    }
    if (joinOk && splitOk) {
        for (i = 0; i < 3; i++) {
            charId = Savemap.memory_bank_2[9 + i];
            if (charId != 0xFF) {
                g_EntitySplitJoinState[g_CharIdToEntity[charId]] = 0;
                if (i == 0) {
                    modelId = g_CharIdToEntity[charId];
                    if (modelId != 0xFF) {
                        g_FieldModels[g_EntityToModel[modelId]].SolidOff = 0;
                    }
                }
            }
        }
        g_FieldState->characterLock = g_CharacterLock;
        g_EntityForSplitJoin = 0xFF;
        PC_INC(2);
        return 0;
    }
    g_FieldState->characterLock = 1;
    if (Savemap.memory_bank_2[9] != 0xFF) {
        modelId = g_CharIdToEntity[Savemap.memory_bank_2[9]];
        if (modelId != 0xFF) {
            g_EntitySplitJoinState[modelId] = 1;
            g_FieldModels[g_EntityToModel[modelId]].scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[modelId]].ActionState = 0;
            g_FieldModels[g_EntityToModel[modelId]].SolidOff = 1;
        }
    }
    return 1;
}
#endif

s32 FieldEventSplitSet(u8, s16, s16, s32, s32); // extern
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", OpcodeFuncSplit);
#else
/* 25 rows: same $s1 0xFF-constant hoist as OpcodeFuncJoin (twin function).
 * if==0xFF polarity matches target; the != form regressed to 39. g_FieldModels
 * idiom applied. Solve Join and the recipe transfers here. */
s32 OpcodeFuncSplit(void) {
    s32 splitOkA;
    s32 splitOkB;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("split", 8);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    if (Savemap.memory_bank_2[10] == 0xFF) {
        splitOkA = 1;
    } else {
        splitOkA = FieldEventSplitSet(
            g_CharIdToEntity[Savemap.memory_bank_2[10]],
            FieldEventReadMemoryS16(1, 4), FieldEventReadMemoryS16(2, 6),
            FieldEventReadMemoryU8(3, 8) & 0xFF, GET_PARAM_U8(14));
    }
    if (Savemap.memory_bank_2[11] == 0xFF) {
        splitOkB = 1;
    } else {
        splitOkB = FieldEventSplitSet(
            g_CharIdToEntity[Savemap.memory_bank_2[11]],
            FieldEventReadMemoryS16(4, 9), FieldEventReadMemoryS16(5, 11),
            FieldEventReadMemoryU8(6, 13) & 0xFF, GET_PARAM_U8(14));
    }
    if (splitOkA && splitOkB) {
        for (i = 0; i < 3; i++) {
            charId = Savemap.memory_bank_2[9 + i];
            if (charId != 0xFF) {
                g_EntitySplitJoinState[g_CharIdToEntity[charId]] = 0;
                if (i == 0) {
                    modelId = g_CharIdToEntity[charId];
                    if (modelId != 0xFF) {
                        g_FieldModels[g_EntityToModel[modelId]].SolidOff = 0;
                    }
                }
            }
        }
        g_FieldState->characterLock = g_CharacterLock;
        g_EntityForSplitJoin = 0xFF;
        PC_INC(15);
        return 0;
    }
    g_FieldState->characterLock = 1;
    if (Savemap.memory_bank_2[9] != 0xFF) {
        modelId = g_CharIdToEntity[Savemap.memory_bank_2[9]];
        if (modelId != 0xFF) {
            g_EntitySplitJoinState[modelId] = 1;
            g_FieldModels[g_EntityToModel[modelId]].scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[modelId]].ActionState = 0;
            g_FieldModels[g_EntityToModel[modelId]].SolidOff = 1;
        }
    }
    return 1;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventJoinSet);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSplitSet);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSplitJoinSetMove);

/* Poll one party member's walk during a SPLIT or JOIN. ActionState 2 means the
 * move just finished, so release the scripted-move lock and restore the
 * model's default speed. */
s32 FieldEventSplitJoinEndMove(s16 entityId) {
    if (g_FieldModels[g_EntityToModel[entityId]].ActionState != 2) {
        return 0;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("end move", 0, 0);
    }
    g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode = 0;
    g_FieldModels[g_EntityToModel[entityId]].ActionState = 0;
    D_800756E8[g_EntityToModel[entityId]] = 0;
    g_FieldModels[g_EntityToModel[entityId]].MoveSpeed =
        D_800E42A8[g_EntityToModel[entityId]];
    return 1;
}

/* Begin a party member's turn to a facing during a SPLIT or JOIN. Sets the
 * turn target and step budget, then if the raw delta would exceed half a
 * circle wraps the target the short way round. Codegen pinned via
 * MASPSX_OVERRIDE: the #else body is the verified-correct C; its bytes come
 * from the reference .s (the g_FieldModels *0x84 base register allocation). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldEventSplitJoinSetTurn);
#else
void FieldEventSplitJoinSetTurn(s16 entityId, s16 startDir, s16 endDir) {
    FieldEntity* model;
    s16 delta;

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("set turn=", endDir & 0xFF, 2);
    }
    if (g_EntityToModel[entityId] != 0xFF) {
        model = &g_FieldModels[g_EntityToModel[entityId]];
        model->TurnStart = startDir & 0xFF;
        g_FieldModels[g_EntityToModel[entityId]].TurnType = 2;
        g_FieldModels[g_EntityToModel[entityId]].TurnStep = 0;
        g_FieldModels[g_EntityToModel[entityId]].TurnSteps = 0x10;
        g_FieldModels[g_EntityToModel[entityId]].TurnEnd = endDir & 0xFF;
        model = &g_FieldModels[g_EntityToModel[entityId]];
        delta = model->TurnEnd - model->TurnStart;
        if (delta < 0) {
            delta = ~delta + 1;
        }
        if (delta >= 0x81) {
            if ((s16)model->TurnStart < (s16)model->TurnEnd) {
                model->TurnEnd -= 0x100;
            } else {
                model->TurnEnd += 0x100;
            }
        }
    }
}
#endif

/* Poll one party member's turn during a SPLIT or JOIN. Returns 1 once the
 * entity has finished turning -- or has no model to turn -- and 0 while it is
 * still in progress. */
s32 FieldEventSplitJoinEndTurn(s16 entityId) {
    if (g_EntityToModel[entityId] == 0xFF) {
        return 1;
    }
    if (g_FieldModels[g_EntityToModel[entityId]].TurnType != 3) {
        return 0;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("end turn", 0, 0);
    }
    g_FieldModels[g_EntityToModel[entityId]].TurnType = 0;
    g_FieldModels[g_EntityToModel[entityId]].TurnStep = 0;
    g_FieldModels[g_EntityToModel[entityId]].TurnSteps = 0;
    return 1;
}

/////////////////////////////////////////////////
// Begin of field_opcode_fade.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncFade);

/* The two volatile casts are delay-slot barriers, not a claim about the
 * hardware. gcc reorg happily sinks a plain store sitting just ahead of a call
 * into that call's delay slot; the original does not, leaving the first
 * FieldEventReadMemoryU8's slot empty outright and filling the
 * FieldEventReadMemoryS16's with the `ori a1,7` from the argument setup
 * instead. A volatile store is the one thing reorg refuses to move, so casting
 * exactly the two stores that precede a call pins them. The other four
 * assignments are plain: they are not adjacent to a call and schedule the same
 * either way. (A do/while barrier costs six extra instructions here by
 * breaking the g_FieldState CSE.) */
s32 OpcodeFuncNfade(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("nfade", 8);
    }
    *(volatile u16*)&g_FieldState->fadeType = GET_PARAM_U8(3);
    g_FieldState->nFadeRedTarget = FieldEventReadMemoryU8(1, 4);
    g_FieldState->nFadeGreenTarget = FieldEventReadMemoryU8(2, 5);
    g_FieldState->nFadeBlueTarget = FieldEventReadMemoryU8(3, 6);
    *(volatile s16*)&g_FieldState->fadeAdjust = 0;
    g_FieldState->fadeSpeed = FieldEventReadMemoryS16(4, 7);
    PC_INC(9);
    return 0;
}

/* PARKED: 9 rows, all in one switch arm. The jump table and all four loads
 * match; what is left is cross-jumping. The original keeps the case 1/5/7/9
 * arm as its own block (lhu / nop / beqz / li 1 / j); gcc merges it into the
 * case 2/6/8/10 arm because both end in the same `return 1'. Not an alignment
 * problem -- this function's table is correctly placed since the split.
 *
 * Rejected, all 22 rows: switching on a plain (s16) cast of the member, on an
 * s32 temp, and on an s16 temp. All three let gcc fold the lhu + sll + sra
 * into a single lh. Only the volatile u16 read below reproduces the load form,
 * and it is what took the diff from 22 rows to 9.
 *
 * Next step is the permuter, not another hand-shaped attempt. */
/* FADEW: block the script until the fade started by FADE/NFADE has finished.
 * What counts as finished depends on the fade's direction, so the switch is on
 * fadeType and the eleven arms collapse to three tests: a fade to black is done
 * when fadeAdjust has run down to 0, a fade from black when it has run up to
 * 0xFF, and the NFADE forms when it has reached fadeSpeed. Types 0 and 4 are
 * not fades and fall straight through.
 *
 * Every read of the three fields goes through a volatile u16. The original
 * loads each one zero-extended and then sign-extends it in registers
 * (lhu / sll / sra); reading the s16 members directly lets gcc fold the two
 * into a single lh, which is a byte shorter everywhere it appears. volatile is
 * the one thing that keeps the load in the form the member's own type implies.
 * The `!= 0` arm takes no cast because the original does not sign-extend
 * there -- a zero test does not need it. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncFadew);
#else
s32 OpcodeFuncFadew(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fadew", 0);
    }
    switch ((s16) * (volatile u16*)&g_FieldState->fadeType) {
    case 1:
    case 5:
    case 7:
    case 9:
        if (*(volatile u16*)&g_FieldState->fadeAdjust != 0) {
            return 1;
        }
        break;
    case 2:
    case 6:
    case 8:
    case 10:
        if ((s16) * (volatile u16*)&g_FieldState->fadeAdjust < 0xFF) {
            return 1;
        }
        break;
    case 0:
    case 4:
        break;
    default:
        if ((s16) * (volatile u16*)&g_FieldState->fadeAdjust !=
            (s16) * (volatile u16*)&g_FieldState->fadeSpeed) {
            return 1;
        }
        break;
    }
    PC_INC(1);
    return 0;
}
#endif

/////////////////////////////////////////////////
// Begin of field_opcode_intersect.c
/////////////////////////////////////////////////

/* IDLCK: set or clear the "player may not cross this walkmesh edge" bit for
 * one triangle. blockedAccesses is a bitfield, eight triangles per byte. */
s32 OpcodeFuncIdlck(void) {
    s16 triId;
    s32 byteIdx;
    s32 bitIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("idlck", 3);
    }
    GET_PARAM_S16(triId, 1);
    byteIdx = triId / 8;
    bitIdx = triId - byteIdx * 8;
    if (GET_PARAM_U8(3)) {
        g_FieldState->blockedAccesses[byteIdx] |= 1 << bitIdx;
    } else {
        g_FieldState->blockedAccesses[byteIdx] &= ~(1 << bitIdx);
    }
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_window_color.c
/////////////////////////////////////////////////

s32 OpcodeFuncGwcol(void) {
    s16 pane;
    s32 corner;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gwcol", 6);
    }
    pane = FieldEventReadMemoryU8(1, 3);
    corner = pane * 3;
    FieldEventWriteMemoryU8(2, 4, D_80049208[corner]);
    FieldEventWriteMemoryU8(3, 5, D_80049208[corner + 1]);
    FieldEventWriteMemoryU8(4, 6, D_80049208[corner + 2]);
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncSwcol(void) {
    s16 pane;
    s32 corner;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("swcol", 6);
    }
    pane = FieldEventReadMemoryU8(1, 3);
    corner = pane * 3;
    D_80049208[corner] = FieldEventReadMemoryU8(2, 4);
    D_80049208[corner + 1] = FieldEventReadMemoryU8(3, 5);
    D_80049208[corner + 2] = FieldEventReadMemoryU8(4, 6);
    PC_INC(7);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_field_effect.c
/////////////////////////////////////////////////

s32 OpcodeFuncLstmp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lstmp", 2);
    }
    FieldEventWriteMemoryS16(2, 2, g_FieldState->prevFieldId);
    PC_INC(3);
    return 0;
}

/* SHAKE: arm the randomized camera shake on either axis. Bit 0 of parameter 3
 * enables the X shake, bit 1 the Y shake; a clear bit disables that axis. */
s32 OpcodeFuncShake(void) {
    s32 axes;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("shake", 7);
    }
    axes = GET_PARAM_U8(3);
    if (axes & 1) {
        g_FieldState->shakeX.enabled = 1;
        g_FieldState->shakeX.amplitude = FieldEventReadMemoryU8(1, 4);
        g_FieldState->shakeX.numStepsPerSegment = FieldEventReadMemoryU8(2, 5);
    } else {
        g_FieldState->shakeX.enabled = 0;
    }
    if (axes & 2) {
        g_FieldState->shakeY.enabled = 1;
        g_FieldState->shakeY.amplitude = FieldEventReadMemoryU8(3, 6);
        g_FieldState->shakeY.numStepsPerSegment = FieldEventReadMemoryU8(4, 7);
    } else {
        g_FieldState->shakeY.enabled = 0;
    }
    PC_INC(8);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_items.c
/////////////////////////////////////////////////

s32 OpcodeFuncStitm(void) {
    s32 itemHi;
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stitm", 4);
    }
    itemHi = FieldEventReadMemoryU8(2, 4);
    itemId = (itemHi & 0xFF) << 9;
    itemId |= FieldEventReadMemoryS16(1, 2);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S item=", itemId, 4);
    }
    func_80025380(itemId);
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncDlitm(void) {
    s32 itemHi;
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dlitm", 4);
    }
    itemHi = FieldEventReadMemoryU8(2, 4);
    itemId = (itemHi & 0xFF) << 9;
    itemId |= FieldEventReadMemoryS16(1, 2);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G item=", itemId, 4);
    }
    func_80025288(itemId);
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncCkitm(void) {
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ckitm", 4);
    }
    itemId = func_80025310(FieldEventReadMemoryS16(1, 2));
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("C item=", itemId, 4);
    }
    if (itemId == 0xFFFF) {
        itemId = 0;
    }
    FieldEventWriteMemoryU8(2, 4, itemId >> 9);
    PC_INC(5);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_special.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncSpcal);
