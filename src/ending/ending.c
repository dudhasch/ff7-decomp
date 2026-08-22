//! PSYQ=3.3 CC1=2.6.3
#include <game.h>

/* A doubly-linked node in the ending overlay's display list. Fields are named
 * from the offsets the assembly touches; nothing here is a guess about
 * semantics beyond `prev`/`next`, which func_800A2934 and func_800A32D8 pin
 * down between them (0x0 back-link, 0x4 forward-link). */
typedef struct EndingObj {
    /* 0x00 */ struct EndingObj* prev;
    /* 0x04 */ struct EndingObj* next;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ u16 unk0C;
    /* 0x0E */ u8 unk0E;
    /* 0x0F */ u8 unk0F;
    /* 0x10 */ u8 unk10[0x18];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s16 unk38;
    /* 0x3A */ s16 unk3A;
    /* 0x3C */ s16 unk3C;
    /* 0x3E */ s16 unk3E;
    /* 0x40 */ s16 unk40;
    /* 0x42 */ s16 unk42;
    /* 0x44 */ s16 unk44;
    /* 0x46 */ s16 unk46;
} EndingObj;

extern s16* D_800A6528;
extern u8 D_800A652C[];
extern s32 D_800AF40C;
extern s32 D_800AF410;
extern EndingObj* D_800AF3CC;

s32 func_80034410(void);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0030);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A04C4);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A09DC);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0AB8);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0BA8);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0CAC);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0E68);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0F90);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A11B4);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A12F0);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A139C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A14BC);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A16E4);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A17C0);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A19A4);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A1E20);

void func_800A1ED4(s16* arg0) { D_800A6528 = arg0; }

s32 func_800A1EE4(void) { return 0; }

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A1EEC);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A1F48);

s32 func_800A1FA4(void) { return func_80034410() == 0; }

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A1FC8);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2014);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A208C);

s32 func_800A20D4(void) { return func_80034410() == 0; }

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A20F8);

s32 func_800A2190(void) {
    SetDispMask(*D_800A6528++);
    return 1;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A21CC);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2248);

s32 func_800A2274(void) {
    s32 back = *D_800A6528 + 1;

    D_800A6528 -= back;
    return 1;
}

s32 func_800A22A4(void) {
    D_800AF40C = *D_800A6528++;
    return 1;
}

s32 func_800A22D4(void) {
    D_800AF410 = 0;
    return 1;
}

s32 func_800A22E4(void) {
    s32 off = *D_800A6528++ * 0x88;

    *(s16*)(D_800A652C + off) = 0;
    return 1;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2328);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2380);

s32 func_800A23F8(void) { return func_80034410() == 8; }

s32 func_800A2420(void) {
    if (g_MovieStream->currentFrame >= *D_800A6528++) {
        return 1;
    }
    return 0;
}

void func_800A2458(void) {
    StopCallback();
    ResetCallback();
    ResetGraph(0);
    PadInit(0);
    InitGeom();
    func_80036298();
    func_80033B70();
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A24A8);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2504);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A273C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2888);

void func_800A2934(EndingObj* arg0, EndingObj* arg1) {
    arg0->next = arg1;
    arg1->prev = arg0;
    arg1->unk28 = arg1->unk2C = arg1->unk30 = arg1->unk34 = 0x1000;
    arg1->unk38 = arg1->unk3A = arg1->unk3C = arg1->unk3E = arg1->unk40 =
        arg1->unk42 = arg1->unk44 = arg1->unk46 = 0;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2974);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2A2C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2C68);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2E80);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2F1C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2FB8);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A310C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A3178);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A3210);

void func_800A32D8(EndingObj* arg0) {
    EndingObj* prev = arg0->prev;
    EndingObj* next = arg0->next;

    prev->next = next;
    next->prev = prev;
}

void func_800A32F0(u8* arg0) { arg0[0xE] = 8; }

void func_800A32FC(u8* arg0) { arg0[0xE] = 4; }

void func_800A3308(u8* arg0) { arg0[0xE] = 0x10; }

EndingObj* func_800A3314(s16 arg0) {
    EndingObj* p = D_800AF3CC;

    while (p->next != NULL) {
        if (p->unk0C == arg0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A3368);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A343C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A34C4);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A358C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A379C);
