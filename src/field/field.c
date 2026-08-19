//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 1 of 5, split out of field.c. .rodata 0x800A0000-0x800A0054, base 0
 * mod 8. Holds jtbl_800A0008, whose `.align 3` in FieldMain.s is a no-op at
 * offset 0x8. */

const u32 D_800A0000[] = {0, 0x01D801E0};

/////////////////////////////////////////////////
// Begin of field_main.c
/////////////////////////////////////////////////

typedef struct {
    u32 datSector; // +0x00
    u32 datSize;   // +0x04
    u32 mimSector; // +0x08
    u32 mimSize;   // +0x0C
    u32 bsxSector; // +0x10
    u32 bsxSize;   // +0x14
} FieldFileInfo;

extern FieldFileInfo g_FieldFileInfo[];
extern void SystemLzsDecompress(void* dst, void* src);
extern s32* g_FieldModelsP;
extern s32 g_FieldTriggers;
extern s32 g_FieldEncounters;
extern s32 D_8007E770;
extern s16 g_CurrentFieldIndex;
extern s32* g_FieldTriggersP;
extern s32* g_FieldEncountersP;
extern u32 g_FieldLzsInfo[];

void FieldLoadMimDatFiles(void) {
    s32 temp;

    if (g_isFieldLoading == 0) {
        DS_read(g_FieldLzsInfo[g_CurrentFieldIndex * 6],
                g_FieldLzsInfo[g_CurrentFieldIndex * 6 + 1], (u32*)0x80128000,
                NULL);
        while (SystemCdromReadChain() != 0) {
        }
    } else {
        while (SystemCdromReadChain() != 0) {
        }
        SystemLzsDecompress((void*)0x801B0000, (void*)0x80128000);
    }
    DS_read(((u32*)g_FieldFileInfo)[g_CurrentFieldIndex * 6],
            ((u32*)g_FieldFileInfo)[g_CurrentFieldIndex * 6 + 1],
            (u32*)0x80114FE4, NULL);
    while (SystemCdromReadChain() != 0) {
    }
    g_FieldTriggers = *g_FieldTriggersP;
    g_FieldEncounters = *g_FieldEncountersP;
    temp = *g_FieldModelsP;
    D_8007E770 = temp;
    g_FieldModelLoaderData = temp + 4;
}

void StopFieldMapPreload(void) {
    if (g_isFieldLoading == 1) {
        SystemCdromAbortLoading();
    }
    D_80071A5C = 0; // needs to be called g_preloadedFieldMapId;
    g_isFieldLoading = 0;
}

extern FieldFileInfo g_FieldFileTable[];
extern u16 g_FieldMoviePlayed;
extern u16 g_FieldPreloadMapId;
extern s32 g_WmPreSector;
extern u32 g_WmPreSize;

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", PreloadNextFieldMap);
#else

// External Declarations
extern u8 D_8009ABF5;
extern u8 g_FieldAnimLock;
extern s16 D_80071A5C;

// D_8009ABF5 = g_FieldState -> command

void PreloadNextFieldMap(FieldEntity* Player, FieldLine* gateway) {
    s16* ptr_a3;
    s32* scratchpad;
    s32 min_dist;
    s32 counter;
    s16* ptr_a1;
    s32 term_val;
    s32 diff_x, diff_y, dist;
    s16 map_id;
    FieldFileInfo* table;
    s32 sector;
    u32 size;

    ptr_a3 = gateway;
    min_dist = 0x7FFFFFFF;

    scratchpad = 0x1F800000;
    scratchpad[0] = Player->PosX >> 12;
    scratchpad[1] = Player->PosY >> 12;
    scratchpad[2] = Player->PosZ >> 12;

    if (g_FieldAnimLock == 0) {
        counter = 0;
        term_val = 0x7FFF;
        ptr_a1 = (gateway + 0x12);

        do {
            map_id = ptr_a1[0];
            if (map_id != term_val) {
                diff_x = ptr_a3[0] - scratchpad[0];
                diff_y = ptr_a1[-8] - scratchpad[1];
                dist = (diff_x * diff_x) + (diff_y * diff_y);

                if (dist < min_dist) {
                    min_dist = dist;
                    g_FieldPreloadMapId = map_id;
                }
            }

            counter++;
            ptr_a1 = (ptr_a1 + 0x18);
            ptr_a3 = (ptr_a3 + 0x18);
        } while (counter < 12);
    }

    if (D_8009ABF5 == 3 || (g_FieldMoviePlayed == 1) || D_8009ABF5 == 2) {
        StopFieldMapPreload();
        return;
    }

    if (D_80071A5C == g_FieldPreloadMapId) {
        return;
    }

    table = g_FieldFileTable;
    if (0x4DFFF < table[g_FieldPreloadMapId].datSize) {
        return;
    }

    StopFieldMapPreload();
    D_80071A5C = g_FieldPreloadMapId;

    if (D_80071A5C >= 0x41) {
        sector = table[D_80071A5C].datSector;
        size = table[D_80071A5C].datSize;
    } else {
        sector = g_WmPreSector;
        size = g_WmPreSize;
    }

    SystemLoadFileBySector(sector, size, 0x801B0000, NULL);
    g_isFieldLoading = 1;
}

#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldMain);

const u32 D_800A0024[] = {0x00000000, 0x000801E0};
const u32 D_800A002C[] = {0x00E80000, 0x000801E0};
const u32 D_800A0034[] = {0x01D00000, 0x000801E0};
const u32 D_800A003C[] = {0x00000000, 0x00080140};
const u32 D_800A0044[] = {0x00E80000, 0x00080140};
const u32 D_800A004C[] = {0x01D00000, 0x00080140};
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldMainLoop);

/* Parse a MIM (field background map image) header and upload its palettes and
 * tile pages to VRAM. arg1 points at the loaded MIM; the header's size and
 * dimensions seed a per-layer state block at D_800E4D90, then each palette
 * (LoadImage) and texture page (LoadTPage) is uploaded with a DrawSync between
 * steps. The $at-rematerialisation wall: the original rebuilds the state-block
 * base through $at on every store where gcc CSEs it. Codegen pinned via
 * MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldLoadMimToVram);
#else
void FieldLoadMimToVram(s32 arg0, u8* mim) {
    RECT rect;
    u8* layerData;
    u32 size;
    u32 layerOff;

    size = *(u32*)mim;
    *(u32*)&D_800E4D94[0] = size;
    *(u16*)&D_800E4D98[0] = *(u16*)(mim + 4);
    *(u16*)&D_800E4D9A[0] = *(u16*)(mim + 6);
    *(u16*)&D_800E4D9C[0] = *(u16*)(mim + 8);
    layerOff = (size >> 2) * 4 - 0xC;
    *(u8**)&D_800E4D90[0] = mim + 0xC;
    *(u16*)&D_800E4D9E[0] = *(u16*)(mim + 0xA);
    layerData = mim + 0xC + layerOff;

    /* First texture page block. */
    *(u32*)&D_800E4DA8[0] = *(u32*)layerData;
    *(u16*)&D_800E4DAC[0] = *(u16*)(layerData + 4);
    *(u16*)&D_800E4DAE[0] = *(u16*)(layerData + 6);
    *(u16*)&D_800E4DB0[0] = *(u16*)(layerData + 8) * 2;
    *(u16*)&D_800E4DB2[0] = *(u16*)(layerData + 0xA);
    *(u8**)&D_800E4DA4[0] = layerData + 0xC;

    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = 0x100;
    rect.h = 0x10;
    DrawSync(0);
    LoadImage(&rect, *(u_long**)&D_800E4D90[0]);
    DrawSync(0);
    *(u16*)&D_800E4DB4[0] =
        LoadTPage(*(u_long**)&D_800E4DA4[0], 1, 0, *(u16*)&D_800E4DB0[0],
                  *(u16*)&D_800E4DB2[0]);

    /* Second texture page block. */
    *(u32*)&D_800E4DD8[0] = *(u32*)(layerData + 0xC);
    *(u16*)&D_800E4DDC[0] = *(u16*)(layerData + 0x10);
    *(u16*)&D_800E4DDE[0] = *(u16*)(layerData + 0x12);
    *(u16*)&D_800E4DE0[0] = *(u16*)(layerData + 0x14) * 2;
    *(u16*)&D_800E4DE2[0] = *(u16*)(layerData + 0x16);
    *(u8**)&D_800E4DD4[0] = layerData + 0x18;

    DrawSync(0);
    *(u16*)&D_800E4DE4[0] =
        LoadTPage(*(u_long**)&D_800E4DD4[0], 1, 0, *(u16*)&D_800E4DE0[0],
                  *(u16*)&D_800E4DE2[0]);
    DrawSync(0);
}
#endif

/* Latch both pads: keep the raw state, the previous state, and the edges
 * (newly pressed / newly released) derived from the two. */
/* Two instructions out, both register choices; see the comment on the second
 * pad's release computation below. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldButtonsUpdate);
#else
void FieldButtonsUpdate(void) {
    u32 pad1;
    u32 pad2;
    u32 old1;
    u32 old2;

    pad1 = func_8001C808();
    old1 = g_FieldPad1State;
    g_FieldPadRaw = pad1;
    g_FieldPad1State = pad1;
    g_FieldPad1PrevState = old1;
    g_FieldPad1Pressed = (pad1 ^ old1) & pad1;
    g_FieldPad1Released = (pad1 ^ old1) & ~pad1;

    pad2 = func_8001C8D4();
    old2 = g_FieldPad2State;
    g_FieldPadRaw = pad2;
    g_FieldPad2State = pad2;
    g_FieldPad2PrevState = old2;
    g_FieldPad2Pressed = (pad2 ^ old2) & pad2;
    /* The do/while is not cosmetic: without the statement boundary gcc hoists
     * this whole expression above the g_FieldPad2Pressed store. It is most
     * likely a macro in the original. What is left is one register choice --
     * the original puts the `nor` in $a0, freed by the store just above, where
     * gcc reuses $v0. */
    do {
        g_FieldPad2Released = (pad2 ^ old2) & ~pad2;
    } while (0);
}
#endif

extern FieldBgData** D_8009D848;
extern FieldBgTile3* D_8007EBD4;
extern u16 D_8011448C;
extern u16 D_801144C8;
extern u16 D_801144D0;

/* Build the sprite packets for the field background's four layers. Layers 1
 * and 2 are 16x16 sprites, layers 3 and 4 are 32x32; each layer walks the run
 * list from where the previous one stopped, emitting one packet per tile and a
 * DR_MODE whenever a run asks for a different texture page. `pairs` collects
 * the two per-sprite parameter bytes the animation code later edits in place.
 *
 * The four run walks are goto loops on purpose. Written as `while` or as
 * `for (;;) { if (...) break; }` gcc's duplicate_loop_exit_test copies the
 * 0x7FFF test to the bottom of each loop, which costs two rows per layer and
 * shifts the whole register allocation; a backward goto is not a loop gcc
 * recognises, so the test stays at the top and is reached by a plain `j`.
 *
 * 25 rows out, and they are one register-allocation decision repeated: the
 * target keeps `modes` in s8 and spills both sprite counters (0x20 and 0x28);
 * this build keeps spriteCount in s8 and spills `modes`, so every `modes++`
 * costs an lw/addiu/sw instead of one addiu. The target's frame is 0x10 larger
 * than ours with 36 bytes of its locals never touched, which says it carried
 * one more long-lived value than this C creates -- find that value and the
 * allocation should fall out. Two structural findings are already folded in
 * above: the goto loops, and the explicit if/else around SetSemiTrans.
 * Measured and rejected: `while` loops with the same body (43 rows, frame
 * 0x10 too large), assigning `run` after tile1/tpages (no change), and
 * `for (;;) { if (...) break; }` (38 rows).
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldBackgroundInitPackets);
#else
void FieldBackgroundInitPackets(
    SPRT_16* sprt16, SPRT* sprt, u8* pairs, DR_MODE* modes) {
    FieldBgData* data;
    FieldBgTile1* tile1;
    FieldBgTile2* tile2;
    u16* tpages;
    s16* run;
    s16 count;
    u16 spriteCount;
    u16 sprite34Count;

    spriteCount = 0;
    sprite34Count = 0;
    D_8011448C = 0;
    D_801144D0 = 0;
    data = *D_8009D848;
    run = data->runs;
    tile1 = (FieldBgTile1*)((u8*)data + data->layer1Offset);
    tpages = (u16*)((u8*)data + data->tpageOffset);

layer1:
    if (run[0] == 0x7FFF) {
        run++;
        goto layer2;
    }
    if (run[0] == 0x7FFE) {
        SetDrawMode(modes, 0, 1, *tpages++, NULL);
        D_8011448C++;
        modes++;
    } else {
        count = run[2];
        if (count != 0) {
            do {
                SetSprt16(sprt16);
                SetShadeTex(sprt16, 1);
                SetSemiTrans(sprt16, 0);
                sprt16->r0 = 0x80;
                sprt16->g0 = 0x80;
                sprt16->b0 = 0x80;
                sprt16->x0 = tile1->x;
                sprt16->y0 = tile1->y;
                sprt16->u0 = tile1->u;
                sprt16->v0 = tile1->v;
                sprt16->clut = tile1->clut;
                spriteCount++;
                tile1++;
                sprt16++;
                pairs += 2;
            } while (--count != 0);
        }
    }
    run += 3;
    goto layer1;

layer2:
    D_8011448C = spriteCount - D_8011448C;
    data = *D_8009D848;
    tile2 = (FieldBgTile2*)((u8*)data + data->layer2Offset);

layer2run:
    if (run[0] == 0x7FFF) {
        run++;
        goto layer3;
    }
    count = run[2];
    if (count != 0) {
        do {
            SetDrawMode(modes, 0, 1, tile2->tpage, NULL);
            D_801144D0++;
            SetSprt16(sprt16);
            SetShadeTex(sprt16, 1);
            if (tile2->flags & 0x80) {
                SetSemiTrans(sprt16, 1);
            } else {
                SetSemiTrans(sprt16, 0);
            }
            modes++;
            sprt16->r0 = tile2->rg;
            sprt16->b0 = 0x80;
            sprt16->g0 = tile2->rg >> 8;
            sprt16->x0 = tile2->x;
            sprt16->y0 = tile2->y;
            sprt16->u0 = tile2->u;
            sprt16->v0 = tile2->v;
            sprt16->clut = tile2->clut;
            pairs[0] = tile2->flags;
            pairs[1] = tile2->param;
            spriteCount++;
            tile2++;
            sprt16++;
            pairs += 2;
        } while (--count != 0);
    }
    run += 3;
    goto layer2run;

layer3:
    D_801144C8 = spriteCount;
    data = *D_8009D848;
    D_8007EBD4 = (FieldBgTile3*)((u8*)data + data->layer34Offset);

layer3run:
    if (run[0] == 0x7FFF) {
        run++;
        goto layer4;
    }
    if (run[0] == 0x7FFE) {
        SetDrawMode(modes, 0, 1, *tpages++, NULL);
        modes++;
    } else {
        count = run[2];
        run[1] = sprite34Count;
        if (count != 0) {
            do {
                SetSprt(sprt);
                SetShadeTex(sprt, 1);
                if (D_8007EBD4->flags & 0x80) {
                    SetSemiTrans(sprt, 1);
                } else {
                    SetSemiTrans(sprt, 0);
                }
                sprt->r0 = 0x80;
                sprt->g0 = 0x80;
                sprt->b0 = 0x80;
                sprt->x0 = D_8007EBD4->x;
                sprt->y0 = D_8007EBD4->y;
                sprt->u0 = D_8007EBD4->u;
                sprt->v0 = D_8007EBD4->v;
                sprt->clut = D_8007EBD4->clut;
                sprt->w = 0x20;
                sprt->h = 0x20;
                pairs[0] = D_8007EBD4->flags;
                pairs[1] = D_8007EBD4->param;
                sprite34Count++;
                D_8007EBD4++;
                sprt++;
                pairs += 2;
            } while (--count != 0);
        }
    }
    run += 3;
    goto layer3run;

layer4:
    if (run[0] == 0x7FFF) {
        return;
    }
    if (run[0] == 0x7FFE) {
        SetDrawMode(modes, 0, 1, *tpages++, NULL);
        modes++;
    } else {
        count = run[2];
        run[1] = sprite34Count;
        if (count != 0) {
            do {
                SetSprt(sprt);
                SetShadeTex(sprt, 1);
                if (D_8007EBD4->flags & 0x80) {
                    SetSemiTrans(sprt, 1);
                } else {
                    SetSemiTrans(sprt, 0);
                }
                sprt->r0 = 0x80;
                sprt->g0 = 0x80;
                sprt->b0 = 0x80;
                sprt->x0 = D_8007EBD4->x;
                sprt->y0 = D_8007EBD4->y;
                sprt->u0 = D_8007EBD4->u;
                sprt->v0 = D_8007EBD4->v;
                sprt->clut = D_8007EBD4->clut;
                sprt->w = 0x20;
                sprt->h = 0x20;
                pairs[0] = D_8007EBD4->flags;
                pairs[1] = D_8007EBD4->param;
                sprite34Count++;
                D_8007EBD4++;
                sprt++;
                pairs += 2;
            } while (--count != 0);
        }
    }
    run += 3;
    goto layer4;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", AddBackgroundToRender);

s32 FieldCalcLinearStep(s32 start, s32 target, s32 duration, s32 step) {
    s32 delta = target - start;

    if ((u32)(delta + 0x7FFFF) <= 0xFFFFE) {
        start += (delta * step) / duration;
    } else {
        start += (delta / duration) * step;
    }

    return start;
}

s32 FieldCalcEaseInOut(s32 from, s32 to, s32 total, s32 step) {
    s32 angle;
    s32 diff;

    angle = ((step << 12) / total) / 32 - 0x80;
    diff = to - from;
    return from +
           ((FieldEntityGetDirVectorY(angle & 0xFF) + 0x1000) * diff) / 0x2000;
}

s32 FieldCalcWorldToScreenPos(SVECTOR* worldPos, long* screenPos) {
    long flag;
    long depth;
    s32 ret;

    PushMatrix();
    SetRotMatrix(D_80071E40);
    SetTransMatrix(D_80071E40);
    SetGeomOffset(0, 0);
    ret = RotTransPers(worldPos, screenPos, &flag, &depth);
    PopMatrix();
    return ret;
}

/* Advance one axis of the SHAKE camera effect. The shake runs as a chain of
 * segments: each one eases the background offset from where the previous
 * segment left off to a fresh random target, negated relative to the last so
 * the image swings either side of centre. Clearing `enabled` does not stop it
 * dead -- it eases one final segment back to zero first.
 *
 * Every instruction matches except the four that increment currentStep: gcc
 * coalesces `step + 1` into a3 (the argument register) where the original
 * keeps it in v0, which lets the original schedule the store into the call's
 * delay slot. */
void FieldBGShakeUpdate(FieldShakeData* shake) {
    s16 step;
    s16 target;

    if (shake->enabled == 1) {
        if (shake->segmentActive == 0) {
            shake->currentStep = 0;
            shake->start = 0;
            shake->target =
                (s16)(g_RandomTable[shake->rngId] * shake->amplitude) / 256;
            shake->segmentActive = 1;
            shake->rngId++;
            return;
        }
        step = shake->currentStep;
        if (shake->numStepsPerSegment < step) {
            target = shake->target;
            shake->currentStep = 0;
            shake->start = target;
            if (target < 0) {
                shake->target =
                    (s16)(g_RandomTable[shake->rngId] * shake->amplitude) / 256;
            } else {
                shake->target =
                    -(s16)(g_RandomTable[shake->rngId] * shake->amplitude) /
                    256;
            }
            shake->rngId++;
            return;
        }
    } else if (shake->segmentActive == 1) {
        step = shake->currentStep;
        if (shake->numStepsPerSegment < step) {
            shake->currentStep = 0;
            shake->start = shake->target;
            shake->target = 0;
            shake->segmentActive = 0;
            shake->rngId++;
            return;
        }
    } else {
        step = shake->currentStep;
        if (shake->numStepsPerSegment == step) {
            shake->currentOffset = 0;
            return;
        }
    }
    /* The two arms are deliberately identical. gcc cross-jumps them back into
     * one block, but only after it has allocated `step + 1` to its own
     * register instead of coalescing it into a3 -- which is what lets the
     * store of currentStep be scheduled into the call's delay slot, as the
     * original does. Written once, the tail is four instructions off. */
    if (step != 0) {
        step = step + 1;
        shake->currentStep = step;
        shake->currentOffset = FieldCalcEaseInOut(
            shake->start, shake->target, shake->numStepsPerSegment, step);
    } else {
        step = step + 1;
        shake->currentStep = step;
        shake->currentOffset = FieldCalcEaseInOut(
            shake->start, shake->target, shake->numStepsPerSegment, step);
    }
}
