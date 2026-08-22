//! PSYQ=3.3 CC1=2.6.3
#include <game.h>

/* A node on the overlay's priority-ordered task list. func_800A310C builds the
 * two sentinels (D_800AF3C8 head, D_800AF3D8 tail) as two of these laid out
 * back to back in .bss, which is what fixes every offset below: `next` at 0x4
 * is the symbol splat calls D_800AF3CC, `prio` at 0xF is D_800AF3D7 = 0xFF.
 * func_800A3178 inserts by descending `prio`, func_800A3210 dispatches `fn`. */
typedef struct EndingTask {
    /* 0x00 */ struct EndingTask* prev;
    /* 0x04 */ struct EndingTask* next;
    /* 0x08 */ void (*fn)(struct EndingTask*);
    /* 0x0C */ u16 id;
    /* 0x0E */ u8 state;
    /* 0x0F */ u8 prio;
} EndingTask;

/* A node in the transform hierarchy. func_800A2E80 pins the whole layout: the
 * MATRIX at 0x8 (RotMatrix's second argument, and t[0] lands at 0x1C), the
 * scale VECTOR at 0x28 (ScaleMatrix's second) and the two SVECTORs at 0x38 and
 * 0x40 (RotMatrix's first and RotTrans's first). 0x0 is read as the parent
 * whose matrix the local one is concatenated onto. */
typedef struct EndingModel {
    /* 0x00 */ struct EndingModel* parent;
    /* 0x04 */ struct EndingModel* child;
    /* 0x08 */ MATRIX mtx;
    /* 0x28 */ VECTOR scale;
    /* 0x38 */ SVECTOR rot;
    /* 0x40 */ SVECTOR trans;
} EndingModel;

extern s16* D_800A6528;
extern u8 D_800A652C[];
extern s32 D_800A6390;
extern s32 D_800AF40C;
extern s32 D_800AF410;
extern EndingTask* D_800AF3CC;

/* Not declared anywhere shared: libgte's ScaleMatrix has no prototype in
 * include/psxsdk, and the three psxsdk.c helpers are still INCLUDE_ASM. */
MATRIX* ScaleMatrix(MATRIX* m, VECTOR* v);
s32 func_80034410(void);
u_long* func_80034D18(u_long* base, s32 index);
s32 func_80034D2C(u_long* src, u_long* dst);
s32 func_80034D5C(void);
s32 func_800484A8(void);
s32 func_80048540(s32 arg0);
void func_800A2888(u_long* tim, u16* tpage, u16* clut);

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

s32 func_800A1FC8(void) {
    u16 tpage;
    u16 clut;

    func_800A2888(
        func_80034D18((u_long*)0x800D0000, *D_800A6528++), &tpage, &clut);
    return 1;
}

s32 func_800A2014(void) {
    u16 tpage;
    u16 clut;
    s32 index = *D_800A6528++;

    if (D_800A6390 != 0) {
        func_80034D2C(
            func_80034D18((u_long*)0x800D0000, index), (u_long*)0x80120000);
    }
    if (func_80034D5C() != 0) {
        return 0;
    }
    func_800A2888((u_long*)0x80120000, &tpage, &clut);
    return 1;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A208C);

s32 func_800A20D4(void) { return func_80034410() == 0; }

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A20F8);

s32 func_800A2190(void) {
    SetDispMask(*D_800A6528++);
    return 1;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A21CC);

s32 func_800A2248(void) {
    func_800A32D8(func_800A3314(4));
    return 1;
}

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

void func_800A24A8(void) {
    s32 status;

    while ((status = func_800484A8()) == -1) {
        VSync(0);
    }
    if (status != 0) {
        while (func_80048540(1) != 0) {
            ;
        }
    }
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2504);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A273C);

void func_800A2888(u_long* tim, u16* tpage, u16* clut) {
    TIM_IMAGE image;

    OpenTIM(tim);
    ReadTIM(&image);
    if (image.paddr != NULL) {
        LoadImage(image.prect, image.paddr);
        *tpage = GetTPage(image.mode, 0, image.prect->x, image.prect->y);
    }
    if (image.caddr != NULL) {
        LoadImage(image.crect, image.caddr);
        *clut = GetClut(image.crect->x, image.crect->y);
    }
}

void func_800A2934(EndingModel* arg0, EndingModel* arg1) {
    arg0->child = arg1;
    arg1->parent = arg0;
    arg1->scale.vx = arg1->scale.vy = arg1->scale.vz = arg1->scale.pad = 0x1000;
    arg1->rot.vx = arg1->rot.vy = arg1->rot.vz = arg1->rot.pad =
        arg1->trans.vx = arg1->trans.vy = arg1->trans.vz = arg1->trans.pad = 0;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2974);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2A2C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2C68);

s32 func_800A2E80(EndingModel* model) {
    s32 flag;

    RotMatrix(&model->rot, &model->mtx);
    model->mtx.t[0] = model->trans.vx;
    model->mtx.t[1] = model->trans.vy;
    model->mtx.t[2] = model->trans.vz;
    ScaleMatrix(&model->mtx, &model->scale);
    MulMatrix2(&model->parent->mtx, &model->mtx);
    SetRotMatrix(&model->parent->mtx);
    SetTransMatrix(&model->parent->mtx);
    RotTrans(&model->trans, (VECTOR*)model->mtx.t, &flag);
    return flag;
}

s32 func_800A2F1C(EndingModel* model) {
    s32 flag;

    RotMatrixYXZ(&model->rot, &model->mtx);
    model->mtx.t[0] = model->trans.vx;
    model->mtx.t[1] = model->trans.vy;
    model->mtx.t[2] = model->trans.vz;
    ScaleMatrix(&model->mtx, &model->scale);
    MulMatrix2(&model->parent->mtx, &model->mtx);
    SetRotMatrix(&model->parent->mtx);
    SetTransMatrix(&model->parent->mtx);
    RotTrans(&model->trans, (VECTOR*)model->mtx.t, &flag);
    return flag;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A2FB8);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A310C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A3178);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A3210);

void func_800A32D8(EndingTask* arg0) {
    EndingTask* prev = arg0->prev;
    EndingTask* next = arg0->next;

    prev->next = next;
    next->prev = prev;
}

void func_800A32F0(u8* arg0) { arg0[0xE] = 8; }

void func_800A32FC(u8* arg0) { arg0[0xE] = 4; }

void func_800A3308(u8* arg0) { arg0[0xE] = 0x10; }

EndingTask* func_800A3314(s16 arg0) {
    EndingTask* p = D_800AF3CC;

    while (p->next != NULL) {
        if (p->id == arg0) {
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
