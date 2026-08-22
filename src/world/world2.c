#include "world.h"

/* One drawable part of a world model.  Lives in the buffer at
 * WorldModel::unk1C, WorldModel::unk18 bytes in, WorldModel::unk3 of them
 * back to back.  numPrims[] counts the eight primitive lists that follow the
 * part header; func_800C3948 and func_800C6104 read those eight bytes as two
 * words instead. */
typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 numPrims[8];
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u8* unk18;
    /* 0x1C */ u8* unk1C;
} WorldModelPart; /* size: 0x20 */

typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ u32 unkC;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u16 unk18;
    /* 0x1A */ u16 unk1A;
    /* 0x1C */ u8* unk1C;
    /* 0x20 */ u8* unk20;
} WorldModel; /* size: 0x24 */

/* One VRAM upload in the group func_800C0808 walks: a rect plus the byte
 * offset of the pixel data from the start of the group. */
typedef struct {
    /* 0x0 */ u16 w;
    /* 0x2 */ u16 h;
    /* 0x4 */ u16 x;
    /* 0x6 */ u16 y;
    /* 0x8 */ u32 offset;
} WorldTim; /* size: 0xC */

typedef struct {
    /* 0x0 */ u32 unk0;
    /* 0x4 */ u8 count;
    /* 0x5 */ u8 unk5;
    /* 0x6 */ u16 unk6;
    /* 0x8 */ WorldTim entries[1];
} WorldTimGroup;

u8* func_800BFCAC(WorldModelPart* part, u8* buf, s32 arg2, s32 arg3);
void func_800C1D58(WorldModel* model, s16 arg1, s32 arg2);

u8* func_800BFBF0(WorldModel* model, u8* buf, s32 arg2) {
    u32 i;
    WorldModelPart* part;

    model->unk20 = buf;
    buf += model->unk2 * 32;
    part = (WorldModelPart*)(model->unk18 + (s32)model->unk1C);
    for (i = 0; i < model->unk3; i++) {
        buf = func_800BFCAC(&part[i], buf, 0, arg2);
    }
    func_800C1D58(model, model->unk16, 0);
    return buf;
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800BFCAC);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C02F4);

void func_800C0808(WorldTimGroup* group) {
    RECT rect;
    u32 i;
    s32 count;
    WorldTim* tim;

    count = group->count;
    tim = group->entries;
    for (i = 0; i < count; i++) {
        rect.x = tim[i].x;
        rect.y = tim[i].y;
        rect.w = tim[i].w;
        rect.h = tim[i].h;
        LoadImage(&rect, (u_long*)((u8*)group + tim[i].offset));
    }
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C08A8);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C0B48);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C1490);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C1D58);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C1FD8);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C2130);

void func_800C2524(WorldModelPart* part, s16 arg1, s16 arg2, s16 arg3);

s32 func_800C2450(WorldModel* model, u8* p) {
    u32 i;
    s32 count;
    WorldModelPart* part;
    s16 c0;
    s16 c1;
    s16 c2;
    /* The target's frame is 8 bytes larger than this body needs, with the
     * extra between the outgoing-argument area and the register saves.  The
     * identity of the local is not recoverable. */
    u8 unusedLocals[8];

    count = model->unk3;
    part = (WorldModelPart*)(model->unk18 + (s32)model->unk1C);
    c0 = (p[1] << 8) | p[0];
    c1 = (p[3] << 8) | p[2];
    c2 = (p[5] << 8) | p[4];
    *(u32*)0x1F800200 = p[6];
    for (i = 0; i < count; i++) {
        func_800C2524(&part[i], c0, c1, c2);
    }
    return 1;
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C2524);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C31F0);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C3948);

void func_800C3DB0(WorldModelPart* part, s32 flag) {
    u32 i;
    u32 j;
    s32 count;
    u8* p;
    /* 8 bytes of locals this body does not declare; identity not
     * recoverable.  Frame is 0x40 against the 0x38 the code needs. */
    u8 unusedLocals[8];

    for (j = 0; j < 2; j++) {
        p = part->unk1C;
        i = 0;
        if (j != 0) {
            p += part->unk16;
        }

        /* The first list is spelled as an explicit guard plus a do/while so
         * that `i = 0` lands in the block above the `if` -- reorg then fills
         * that branch's delay slot with it, which is what the target has.  A
         * plain `for (i = 0; ...)` puts the init in the join block and the
         * slot goes to the count guard instead (2 rows), and moving the init
         * out of a `for` without the explicit guard costs the zero-trip fold
         * (51 rows, +1 instruction).  The other seven keep the `for`. */
        count = part->numPrims[0];
        if (count != 0) {
            do {
                setSemiTrans(p, flag);
                setShadeTex(p, flag);
                i++;
                p += 0x34;
            } while (i < count);
        }
        count = part->numPrims[1];
        for (i = 0; i < count; i++, p += 0x28) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[2];
        for (i = 0; i < count; i++, p += 0x28) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[3];
        for (i = 0; i < count; i++, p += 0x20) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[4];
        for (i = 0; i < count; i++, p += 0x14) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[5];
        for (i = 0; i < count; i++, p += 0x18) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[6];
        for (i = 0; i < count; i++, p += 0x1C) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[7];
        for (i = 0; i < count; i++, p += 0x24) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
    }
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C4148);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C4FB4);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C5CD4);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C6104);

s32 func_800C6598(WorldModel* model) {
    u32 i;
    s32 size;
    WorldModelPart* part;

    size = model->unk2 * 32;
    part = (WorldModelPart*)(model->unk18 + (s32)model->unk1C);
    for (i = 0; i < model->unk3; i++) {
        size += part->unk16 * 2;
        part++;
    }
    return size;
}
