//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 3 of 5, split out of field.c. .rodata 0x800A00E0-0x800A0104, base 0
 * mod 8. Exists only to give jtbl_800A00E0 -- which FieldModelLoadBcx.s
 * precedes with `.align 3`
 * -- a base it is congruent to. */

extern u8* D_800E0204;

/* Copy one global (BCX) model into place. `pkts` is the streaming buffer the
 * file lands in; the record it holds is linked against 0x80000000, so every
 * pointer inside it is rebased by (pkts - 0x80000000) on the way through.
 * When the model is already resident (globalLoaded set) the copy is made from
 * the earlier entry that carries the same globalModelId instead of from disk.
 */
u8* FieldModelLoadBcx(
    FieldModelFileDesc* desc, FieldModelData* data, u8* pkts, s32 index) {
    FieldModelLoaderData* models;
    u32* fileInfo;
    FieldModelEntry* src;
    FieldModelEntry* dst;
    s32* s;
    s32* d;
    s32* sp;
    s32* dp;
    s32* sa;
    s32* da;
    s32 fixup;
    s32 id;
    u32 count;
    u32 i;
    u32 count2;
    u32 i2;
    u32 j;

    models = desc->models;
    fileInfo = (u32*)((s32*)0x1F800000)[0];
    if (models[index].npcFlag == 0) {
        return pkts;
    }
    id = models[index].globalModelId;
    if (id >= 1 && id <= 9) {
        if (models[index].globalLoaded == 0) {
            switch (id) {
            case 1:
                DS_read(fileInfo[0], fileInfo[1], (u_long*)pkts, NULL);
                break;
            case 2:
                DS_read(fileInfo[2], fileInfo[3], (u_long*)pkts, NULL);
                break;
            case 3:
                DS_read(fileInfo[4], fileInfo[5], (u_long*)pkts, NULL);
                break;
            case 4:
                DS_read(fileInfo[6], fileInfo[7], (u_long*)pkts, NULL);
                break;
            case 5:
                DS_read(fileInfo[8], fileInfo[9], (u_long*)pkts, NULL);
                break;
            case 6:
                DS_read(fileInfo[10], fileInfo[11], (u_long*)pkts, NULL);
                break;
            case 7:
                DS_read(fileInfo[12], fileInfo[13], (u_long*)pkts, NULL);
                break;
            case 8:
                DS_read(fileInfo[14], fileInfo[15], (u_long*)pkts, NULL);
                break;
            case 9:
                DS_read(fileInfo[16], fileInfo[17], (u_long*)pkts, NULL);
                break;
            }
            while (SystemCdromReadChain() != 0) {
            }
            for (i = 0; i < desc->count; i++) {
                if (models[i].globalModelId == id) {
                    models[i].globalLoaded = 1;
                }
            }
            dst = &data->modelEntries[models[index].modelEntryIndex];
            src = (FieldModelEntry*)(pkts + ((u32*)pkts)[1]);
            fixup = (s32)pkts - 0x80000000;
            count = src->boneCount;
            src->modelData = (u8*)((s32)src->modelData + fixup);
            d = (s32*)dst->modelData;
            s = (s32*)src->modelData;
            for (i = 0; i < count; i++) {
                d[i] = s[i];
            }
            count = src->partCount;
            dp = (s32*)(dst->modelData + dst->partsOffset);
            sp = (s32*)(src->modelData + src->partsOffset);
            for (i = 0; i < count; i++) {
                dp[i * 8 + 0] = sp[i * 8 + 0];
                dp[i * 8 + 1] = sp[i * 8 + 1];
                dp[i * 8 + 2] = sp[i * 8 + 2];
                dp[i * 8 + 3] = sp[i * 8 + 3];
                dp[i * 8 + 4] = sp[i * 8 + 4];
                dp[i * 8 + 5] = sp[i * 8 + 5];
                dp[i * 8 + 6] = sp[i * 8 + 6];
                dp[i * 8 + 7] = sp[i * 8 + 7];
                dp[i * 8 + 6] = sp[i * 8 + 6] + fixup;
            }
            count = src->animationCount;
            da = (s32*)(dst->modelData + dst->animationOffset);
            sa = (s32*)(src->modelData + src->animationOffset);
            for (i = 0; i < count; i++) {
                da[i * 4 + 0] = sa[i * 4 + 0];
                da[i * 4 + 1] = sa[i * 4 + 1];
                da[i * 4 + 2] = sa[i * 4 + 2];
                da[i * 4 + 3] = sa[i * 4 + 3];
                da[i * 4 + 3] = sa[i * 4 + 3] + fixup;
            }
            D_800E0204 = (u8*)src;
            return (u8*)src;
        }
        for (j = 0; j < index; j++) {
            if (models[j].globalModelId == id) {
                dst = &data->modelEntries[models[index].modelEntryIndex];
                src = &data->modelEntries[j];

                d = (s32*)dst->modelData;
                count2 = src->boneCount;
                s = (s32*)src->modelData;
                for (i2 = 0; i2 < count2; i2++) {
                    d[i2] = s[i2];
                }
                count2 = src->partCount;
                dp = (s32*)(dst->modelData + dst->partsOffset);
                sp = (s32*)(src->modelData + src->partsOffset);
                for (i2 = 0; i2 < count2; i2++) {
                    dp[i2 * 8 + 0] = sp[i2 * 8 + 0];
                    dp[i2 * 8 + 1] = sp[i2 * 8 + 1];
                    dp[i2 * 8 + 2] = sp[i2 * 8 + 2];
                    dp[i2 * 8 + 3] = sp[i2 * 8 + 3];
                    dp[i2 * 8 + 4] = sp[i2 * 8 + 4];
                    dp[i2 * 8 + 5] = sp[i2 * 8 + 5];
                    dp[i2 * 8 + 6] = sp[i2 * 8 + 6];
                    dp[i2 * 8 + 7] = sp[i2 * 8 + 7];
                }
                count2 = src->animationCount;
                da = (s32*)(dst->modelData + dst->animationOffset);
                sa = (s32*)(src->modelData + src->animationOffset);
                for (i2 = 0; i2 < count2; i2++) {
                    da[i2 * 4 + 0] = sa[i2 * 4 + 0];
                    da[i2 * 4 + 1] = sa[i2 * 4 + 1];
                    da[i2 * 4 + 2] = sa[i2 * 4 + 2];
                    da[i2 * 4 + 3] = sa[i2 * 4 + 3];
                }
                break;
            }
        }
        D_800E0204 = pkts;
    }
    return pkts;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelPrepareRender);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelAddToRender);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelAnimCalcMtrxs);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelScaleModel);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelScalePartVrtxs);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelScaleAnimTranslat);

/////////////////////////////////////////////////
// Begin of field_kawai_char_model.c
/////////////////////////////////////////////////

void KawaiClearData(void) {
    u8* p = D_800DFDFC;
    s32 count = 16;
    s32 i;

    p[0] = count;
    for (i = 0; i < count; i++) {
        p[i * 2 + 2] = 0;
        p[i * 2 + 3] = 0;
    }
}
