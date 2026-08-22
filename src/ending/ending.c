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
    /* 0x08 */ void (*fn)();
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

/* An animated on-screen actor, 0x88 bytes -- which is the stride every loop
 * over D_800A652C walks, and it is what closes the layout: func_800A3368 hands
 * actor+0x1C to func_800A379C as an EndingModel* and actor+0x78 as its target
 * VECTOR, and 0x1C + sizeof(EndingModel) + sizeof(s32[4]) + sizeof(VECTOR) is
 * exactly 0x88. The three `sh` stores at 0x5C/0x5E/0x60 that read as loose
 * halfwords are the embedded model's translation SVECTOR.
 *
 * func_800A343C fixes the colour block (0x10 current, 0x14 per-step delta,
 * 0x18 final) and the step counter at 0x2; func_800A34C4 fixes the animation
 * pointer at 0xC with its delay and frame counters. */
typedef struct EndingActor {
    /* 0x00 */ u16 flags;
    /* 0x02 */ s16 fadeSteps;
    /* 0x04 */ s16 speed;
    /* 0x06 */ u16 delay;
    /* 0x08 */ u16 frame;
    /* 0x0A */ u16 unk0A;
    /* 0x0C */ u_long* anim;
    /* 0x10 */ u8 r;
    /* 0x11 */ u8 g;
    /* 0x12 */ u8 b;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 stepR;
    /* 0x15 */ u8 stepG;
    /* 0x16 */ u8 stepB;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 endR;
    /* 0x19 */ u8 endG;
    /* 0x1A */ u8 endB;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ EndingModel model;
    /* 0x64 */ s32 unk64;
    /* 0x68 */ s32 posX;
    /* 0x6C */ s32 posY;
    /* 0x70 */ s32 posZ;
    /* 0x74 */ s32 unk74;
    /* 0x78 */ VECTOR target;
} EndingActor;

extern s16* D_800A6528;
extern u8 D_800A652C[];
extern u8 D_800A6532[];
extern u8 D_800A6534[];
extern u8 D_800A6538[];
extern u8 D_800A653C[];
extern u8 D_800A653D[];
extern u8 D_800A653E[];
extern u8 D_800A6588[];
extern u8 D_800A658A[];
extern u8 D_800A658C[];
extern s32 (*D_800A63DC[])(void);
extern s32 D_800A6390;
extern s32 D_800A6394;
/* Three (sector, size) pairs in .data -- the ending overlay's own file
 * table, indexed by the script halfword func_800A1EEC and func_800A1F48 read.
 */
extern s32 D_800A6398[];
extern s32 D_800A63B0;
extern u_long* D_800A6524;
extern DRAWENV D_800AF2E0[];
extern DISPENV D_800AF398[];
extern u32 D_800AF3C0;
extern u32 D_800AF3C4;
extern EndingTask D_800AF3C8;
extern EndingTask* D_800AF3CC;
extern EndingTask D_800A762C;
extern u8 D_800A763C[];
extern u8 D_800A765C[];
extern EndingTask D_800AF3D8;
extern u32 D_800AF3EC;
extern u32 D_800AF3F0;
extern u32 D_800AF3F4;
extern u32 D_800AF3F8;
extern void* D_800AF3E8;
extern void* D_800AF3FC;
extern s32 D_800AF408;
extern s32 D_800AF40C;
extern s32 D_800AF410;

extern DRAWENV* D_8007EBD0;
extern DISPENV* D_8007EBD8;
extern u8* D_8003623C;

/* Not declared anywhere shared: libgte's ScaleMatrix has no prototype in
 * include/psxsdk, and the three psxsdk.c helpers are still INCLUDE_ASM. */
MATRIX* ScaleMatrix(MATRIX* m, VECTOR* v);
s32 VectorNormal(VECTOR* v0, VECTOR* out);
u32 func_8001C808(void);
s32 func_80034410(void);
void func_80036244(u_long* anim, s32 frame);
u_long* func_80034D18(u_long* base, s32 index);
s32 func_80034D2C(u_long* src, u_long* dst);
s32 func_80034D5C(void);
s32 func_80034FC8(u_long* dst, s32 index);
s32 func_800484A8(void);
s32 func_80048540(s32 arg0);
void func_800A2504(s32 x, s32 y, s32 w, s32 r, u8 g, u8 b);
void* func_800A358C(void* arg0, s32 arg1, void* arg2, EndingActor* actor);
void func_800A3368(EndingActor* actor);
void func_800A343C(EndingActor* actor);
void func_800A34C4(EndingActor* actor);
s32 func_800A379C(EndingModel* model, VECTOR* target, VECTOR* dir, s32 speed);
s32 func_800A273C(s32 arg0);
void func_800A2888(u_long* tim, u16* tpage, u16* clut);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0030);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A04C4);

void func_800A09DC(void) {
    /* 0x100 bytes of frame no instruction names -- the target's
     * `sw ra,0x11c(sp)` against a 0x10 outgoing-argument area. */
    u8 unusedLocals[0x100];
    s32 i;
    s32 off;
    EndingActor* actor;

    for (i = 0, off = 0; i < 0x20; i++, off += 0x88) {
        if (*(u16*)(D_800A652C + off) & 1) {
            actor = (EndingActor*)(off + (s32)D_800A652C);
            *(s16*)(D_800A6588 + off) = 0x28;
            *(s16*)(D_800A658A + off) = 0x20;
            *(s16*)(D_800A658C + off) = 0;
            func_800A34C4(actor);
            func_800A343C(actor);
            D_800AF3FC = func_800A358C(D_800AF3E8, 0, D_800AF3FC, actor);
        }
    }
}

s32 func_800A0AB8(void) {
    s32 i;
    s32 off;

    /* The increments are the other way round from func_800A09DC's identical
     * walk: with no `if` in the body there is no branch delay slot for reorg
     * to steal `i++` into, so the written order is the emitted order and
     * `i++, off += 0x88` puts the counter at the loop top instead. */
    for (i = 0, off = 0; i < 0x20; off += 0x88, i++) {
        *(s16*)(D_800A652C + off) = 0;
        *(s16*)(D_800A6532 + off) = 0;
        *(s16*)(D_800A6534 + off) = 0;
        *(s32*)(D_800A6538 + off) = 0;
        *(s16*)(D_800A6588 + off) = 0;
        *(s16*)(D_800A658A + off) = 0;
        *(s16*)(D_800A658C + off) = 0;
        *(u8*)(D_800A653C + off) = 0;
        *(u8*)(D_800A653D + off) = 0;
        *(u8*)(D_800A653E + off) = 0;
    }
    func_800A3178(&D_800A762C, 4, 0x80, func_800A09DC);
    return 1;
}

s32 func_800A0BA8(void) {
    s32 slot = *D_800A6528++;
    s32 file = *D_800A6528++;
    s32 off = slot * 0x88;

    *(s16*)(D_800A652C + off) = 7;
    *(s16*)(D_800A6532 + off) = 0;
    *(s16*)(D_800A6534 + off) = 0;
    *(u_long**)(D_800A6538 + off) = func_80034D18((u_long*)0x800D0000, file);
    *(s16*)(D_800A6588 + off) = 0;
    *(s16*)(D_800A658A + off) = 0;
    *(s16*)(D_800A658C + off) = 0;
    *(u8*)(D_800A653C + off) = 0;
    *(u8*)(D_800A653D + off) = 0;
    *(u8*)(D_800A653E + off) = 0;
    return 1;
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0CAC);

void func_800A0E68(void) {
    s32 i;
    s32 off;
    EndingActor* actor;

    AddPrim(D_800AF3E8, (void*)(D_800AF408 * 16 + (s32)D_800A763C));
    AddPrim(D_800AF3E8, (void*)(D_800AF408 * 16 + (s32)D_800A765C));
    for (i = 0; i < 0x20; i++) {
        off = i * 0x88;
        if (*(u16*)(D_800A652C + off) & 1) {
            s32 v = *(u16*)(D_800A652C + off + 0x5E) - 1;

            actor = (EndingActor*)(off + (s32)D_800A652C);
            actor->model.trans.vy = v;
            if (*(s16*)(D_800A652C + off + 0x5E) == -0x10) {
                *(s16*)(D_800A652C + off) = 0;
            }
            func_800A34C4(actor);
            func_800A343C(actor);
            D_800AF3FC = func_800A358C(D_800AF3E8, 0, D_800AF3FC, actor);
        }
    }
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A0F90);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A11B4);

void func_800A12F0(void) {
    s32 i;
    s32 off;
    EndingActor* actor;

    /* `off` is a giv here and a second biv in func_800A09DC -- two loops that
     * differ in nothing else, and swapping the two spellings costs 17 rows in
     * either direction. As a `for` increment the base address of D_800A652C
     * becomes a loop invariant gcc hoists into a fifth callee-saved register;
     * as `i * 0x88` it does not. */
    for (i = 0; i < 0x20; i++) {
        off = i * 0x88;
        if (*(u16*)(D_800A652C + off) & 1) {
            actor = (EndingActor*)(off + (s32)D_800A652C);
            func_800A3368(actor);
            func_800A34C4(actor);
            func_800A343C(actor);
            D_800AF3FC = func_800A358C(D_800AF3E8, 0, D_800AF3FC, actor);
        }
    }
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A139C);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A14BC);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A16E4);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A17C0);

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A19A4);

void func_800A1E20(void) {
    s16* pc;

    do {
        pc = D_800A6528;
        D_800A6394 = 0;
        D_800A6528 = pc + 1;
        if (D_800A63DC[*pc]() == 0) {
            D_800A6528 = pc;
            D_800A6390 = 0;
        } else {
            D_800A6390 = 1;
        }
    } while (D_800A6394 != 0);
}

void func_800A1ED4(s16* arg0) { D_800A6528 = arg0; }

s32 func_800A1EE4(void) { return 0; }

s32 func_800A1EEC(void) {
    s32 file = *D_800A6528++;

    SystemLoadFileBySector(D_800A6398[file * 2], D_800A6398[file * 2 + 1],
                           (u_long*)0x800D0000, NULL);
    return 1;
}

s32 func_800A1F48(void) {
    s32 file = *D_800A6528++;

    DS_read(D_800A6398[file * 2], D_800A6398[file * 2 + 1], (u_long*)0x800D0000,
            NULL);
    return 1;
}

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

s32 func_800A208C(void) {
    D_800A6524 = (u_long*)0x801A0000;
    func_80034FC8((u_long*)0x801A0000, *D_800A6528++);
    return 1;
}

s32 func_800A20D4(void) { return func_80034410() == 0; }

s32 func_800A20F8(void) {
    /* 8 bytes of frame the emitted code never names -- the target's `sw
     * ra,0x20(sp)` against a 0x18 outgoing-argument area. Any size from 2 to 8
     * gives the same layout; the identity of the local is not recoverable. */
    u8 unusedLocals[8];
    s32 x = *D_800A6528++;
    s32 y = *D_800A6528++;
    s32 w = *D_800A6528++;
    u8 r = *D_800A6528++;
    u8 g = *D_800A6528++;

    func_800A2504(x, y, w, r, g, *D_800A6528++);
    func_800A273C(0);
    return 1;
}

s32 func_800A2190(void) {
    SetDispMask(*D_800A6528++);
    return 1;
}

s32 func_800A21CC(void) {
    if (D_800A6390 != 0) {
        D_800A63B0 = *D_800A6528++;
    } else {
        D_800A6528++;
    }
    return --D_800A63B0 == 0;
}

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

s32 func_800A2328(void) {
    D_8009A000[0] = 0x10;
    D_8009A004[0] = (u32)func_80034D18((u_long*)0x800D0000, *D_800A6528++);
    SystemAkaoExecute();
    return 1;
}

s32 func_800A2380(void) {
    D_8009A000[0] = *D_800A6528++;
    D_8009A004[0] = *D_800A6528++;
    D_8009A008[0] = *D_800A6528++;
    SystemAkaoExecute();
    return 1;
}

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

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/ending/nonmatchings/ending", func_800A273C);
#else
/* 9 rows at the exact 83 instructions, and every one of them is a register
 * name: `pad1` and `prev0` hold each other's register (target a3/a1, ours
 * a1/a3) and everything downstream of the two `and`s follows. `pad0` -> $a2
 * and `prev1` -> $a0 are already right, so this is one adjacent pair in
 * local_alloc's quantity order and nothing else.
 *
 * What the compiler discovery was worth here: the whole function was 15 rows
 * and -3 instructions until `return D_800AF408;` -- the target's `lw v0`
 * after the call is not a dead load, it is the return value, and it is also
 * what forces the `move a2,v0` that a void version coalesces away.
 *
 * Measured and rejected, all at the same 83 instructions:
 *   reading D_800AF3C0/C4 inline instead of into prev0/prev1   13
 *   the six stores in nine other orders (b,c,e,f,g,h,i,j,k)    9 (k), 13-16
 *   prev1 loaded before prev0                                   9
 *   declaration order prev0/prev1 ahead of pad0/pad1            9
 *   D_800AF3F8 written as `pad0 >> 16` (one fewer pad1 ref)     9
 *   a `pad` local kept alongside pad0                           9
 *   ~pad0/~pad1 hoisted into inv0/inv1 locals                   9
 *   pad1 assigned after the two prev loads                      9
 *   `pad0 & prev0` operand order                               12
 *   prev0 loaded late, just above the D_800AF3C0 store          18
 *   prev0 inline / prev1 inline                                10 / 21
 *   D_800AF408 read into a local before the stores             16
 *   chained `D_800AF3F4 = pad0 = func_8001C808()`              21
 *   pad0/pad1 as s32                                           13
 *
 * Nine of those measure exactly 9, which is CLAUDE.md's flat-dimension
 * signature: the residue is allocno_compare/QTY_CMP_PRI arithmetic, and
 * prev0 would have to go from 2 references to 3 (a floor_log2 step) to
 * overtake pad1's 4. There is no way to spell that without emitting an
 * instruction, so this is a park and not a permuter target. */
s32 func_800A273C(s32 arg0) {
    u32 pad0;
    u32 pad1;
    u32 prev0;
    u32 prev1;

    D_800AF408 ^= 1;
    DrawSync(0);
    VSync(arg0);
    PutDispEnv(&D_800AF398[D_800AF408]);
    PutDrawEnv(&D_800AF2E0[D_800AF408]);
    D_8007EBD8 = &D_800AF398[D_800AF408];
    D_8007EBD0 = &D_800AF2E0[D_800AF408];
    pad0 = func_8001C808();
    pad1 = pad0 >> 16;
    prev0 = D_800AF3C0;
    prev1 = D_800AF3C4;
    D_800AF3C0 = ~pad0;
    D_800AF3F4 = pad0;
    D_800AF3F8 = pad1;
    D_800AF3C4 = ~pad1;
    D_800AF3EC = prev0 & pad0;
    D_800AF3F0 = prev1 & pad1;
    return D_800AF408;
}
#endif

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

s32 func_800A2FB8(EndingModel* model, VECTOR* target, VECTOR* dir, s32 speed) {
    VECTOR delta;

    delta.vx = target->vx - model->trans.vx;
    delta.vy = target->vy - model->trans.vy;
    delta.vz = target->vz - model->trans.vz;
    VectorNormal(&delta, dir);
    if (delta.vx >= -2 && delta.vx < 2 && delta.vy >= -2 && delta.vy < 2 &&
        delta.vz >= -2 && delta.vz < 2) {
        dir->vx = delta.vx << 12;
        dir->vy = delta.vy << 12;
        dir->vz = delta.vz << 12;
        return 1;
    }
    if (speed != 0x1000) {
        dir->vx = speed * dir->vx / 0x1000;
        dir->vy = speed * dir->vy / 0x1000;
        dir->vz = speed * dir->vz / 0x1000;
    }
    return 0;
}

void func_800A310C(void) {
    D_800AF3C8.id = 0;
    D_800AF3C8.state = 1;
    D_800AF3C8.prio = 0xFF;
    D_800AF3C8.prev = NULL;
    D_800AF3C8.next = &D_800AF3D8;
    D_800AF3D8.id = 1;
    D_800AF3D8.state = 1;
    D_800AF3D8.prio = 0;
    D_800AF3D8.prev = &D_800AF3C8;
    D_800AF3D8.next = NULL;
}

void func_800A3178(EndingTask* task, u16 id, u8 prio, void (*fn)()) {
    EndingTask* n = &D_800AF3C8;

    do {
        if (n->prio < prio) {
            task->id = id;
            task->fn = fn;
            task->state = 2;
            goto insert;
        }
        n = n->next;
    } while (n->next != NULL);
    if (n->prio >= prio) {
        return;
    }
    task->id = id;
    task->fn = fn;
    task->state = 2;
insert:
    task->prio = prio;
    task->next = n;
    task->prev = n->prev;
    n->prev = task;
    n = task->prev;
    n->next = task;
}

void func_800A3210(void) {
    EndingTask* t;

    t = D_800AF3CC;
    while (t->next != NULL) {
        if (t->state == 4) {
            t->fn(t);
        }
        t = t->next;
    }
    t = D_800AF3CC;
    while (t->next != NULL) {
        if (t->state == 2) {
            t->state = 4;
        }
        t = t->next;
    }
}

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

void func_800A3368(EndingActor* actor) {
    VECTOR step;

    if (actor->flags & 0x10) {
        if (func_800A379C(&actor->model, &actor->target, &step, actor->speed) !=
            0) {
            actor->flags ^= 0x10;
        }
        actor->posX += step.vx;
        actor->posY += step.vy;
        actor->posZ += step.vz;
        actor->model.trans.vx = actor->posX / 0x1000;
        actor->model.trans.vy = actor->posY / 0x1000;
        actor->model.trans.vz = actor->posZ / 0x1000;
    }
}

void func_800A343C(EndingActor* actor) {
    if (actor->flags & 8) {
        actor->r += actor->stepR;
        actor->g += actor->stepG;
        actor->b += actor->stepB;
        if (--actor->fadeSteps == 0) {
            actor->flags ^= 8;
            actor->r = actor->endR;
            actor->g = actor->endG;
            actor->b = actor->endB;
        }
    }
}

void func_800A34C4(EndingActor* actor) {
    u16 count;

    if (actor->flags & 2) {
        count = *(u16*)actor->anim;
        if (actor->delay == 0) {
            actor->frame++;
            if (actor->frame >= count) {
                if (actor->flags & 4) {
                    actor->frame = 0;
                } else {
                    actor->frame--;
                }
            }
            func_80036244(actor->anim, actor->frame);
            actor->delay = D_8003623C[1];
        }
        actor->delay--;
    }
}

INCLUDE_ASM("asm/us/ending/nonmatchings/ending", func_800A358C);

s32 func_800A379C(EndingModel* model, VECTOR* target, VECTOR* dir, s32 speed) {
    VECTOR delta;
    s32 step;

    delta.vx = target->vx - model->trans.vx;
    delta.vy = target->vy - model->trans.vy;
    delta.vz = target->vz - model->trans.vz;
    if (delta.vx == 0) {
        step = 0x1000;
        if (delta.vy < 0) {
            step = -0x1000;
        }
        dir->vx = 0;
        dir->vy = step;
        dir->vz = 0;
    } else if (delta.vy == 0) {
        step = 0x1000;
        if (delta.vx < 0) {
            step = -0x1000;
        }
        dir->vx = step;
        dir->vy = 0;
        dir->vz = 0;
    } else {
        VectorNormal(&delta, dir);
    }
    if (delta.vx >= -2 && delta.vx < 2 && delta.vy >= -2 && delta.vy < 2 &&
        delta.vz >= -2 && delta.vz < 2) {
        dir->vx = delta.vx << 12;
        dir->vy = delta.vy << 12;
        dir->vz = delta.vz << 12;
        return 1;
    }
    if (speed != 0x1000) {
        dir->vx = speed * dir->vx / 0x1000;
        dir->vy = speed * dir->vy / 0x1000;
        dir->vz = speed * dir->vz / 0x1000;
    }
    return 0;
}
