//! PSYQ=3.3 CC1=2.6.3
#include "battle_private.h"
#include <libetc.h>
#include <libgpu.h>

static void func_800B37A0(void);
static void func_800B37EC(void);
static void func_800B38E0(void);
static void func_800B3D38(void);
static void func_800B3D88(void);
static void func_800B3DBC(void);
static s32 func_800B3FAC(s32 arg0);
static void func_800B798C(void);
static void func_800B7FDC(void);
static void func_800B8360(s32);
static void func_800B85E0();
static void func_800B88CC(s32 arg0);
static void func_800B8E48(s32 arg0);
static void func_800BA24C(void);
static void func_800BA4C8(void);
void func_800BA598(s16);
static void func_800BB030(s16);
static void func_800BB75C(Unk800BB75C* arg0, MATRIX* m, s16* arg2, s16* arg3);
static void func_800BB804(void);
static void func_800BB864(void);
static void func_800BC2F0(void);
static void func_800C0410(void);
static void func_800C0900(void);
static void func_800C20E8(s16 arg0, s16* arg1);
static void func_800C4D10(void);
DR_MODE* func_800C4DC8(s16 x, s16 y, s16 w, s16 h, s32*);
static void func_800C614C(u_long* arg0, s32 arg1);
static void func_800C627C(void);
void func_800C62F4(s32);
static void func_800BC81C(s16 arg0, s16 arg1);
static void func_800B3A04(void);
static void func_800B950C(void);

void func_800B30E4(void) {
    s32 i;

    g_cDb = &g_db;
    D_801031E4 = 0;
    g_dbIndex = 0;
    D_80162084 = 0x200;
    func_800B383C();
    func_800B430C();
    VSync(0);
    SetDispMask(0);
    D_800F9F34 = 0;
    *(s8*)&D_800FA63C.u.sub.unk34 = 0;
    D_800FA6A0 = 0;
    func_800B37A0();
    func_800B3E2C();
    func_800BB684();
    func_800BC04C(func_800C4D10);
    func_800B7FDC();
    func_800B7FDC();
    do {
    } while (D_80095DD4);
    func_800B37EC();
    SetDispMask(1);
    while (1) {
        switch (D_80163C7C) {
        case 0:
            D_801635FC = 0x3D;
            func_800B38E0();
            func_800B7FDC();
            D_80163C7C = 1;
            break;
        case 1:
            func_800B7FDC();
            if (D_800F7DF4 == (u8)D_80166F64 && D_801518DC == 0) {
                func_800B3D38();
                func_800B5138();
                D_80163C7C = 6;
            }
            break;
        case 6:
            func_800B7FDC();
            func_800B3D88();
            for (i = 4; i < D_800F7E04[0] + 4; i++) {
                D_801518E4[i].D_80151922 |= 4;
            }
            D_80163C7C = 2;
            break;
        case 2:
            func_800B7FDC();
            if ((u8)D_80166F64 == 3 && D_801518DC == 0) {
                func_800B3DBC();
                D_80163C7C = 3;
                D_801518E4[0].D_80151922 |= 4;
                D_801518E4[1].D_80151922 |= 4;
                D_801518E4[2].D_80151922 |= 4;
            }
            break;
        case 3:
            func_800B7FDC();
            if (D_801635FC == 0) {
                D_80163C7C = 4;
                func_800C61C0();
            }
            break;
        default:
            return;
        }
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B33A4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B36B4);

// one-shot setup call centered on the 320x240 screen
static void func_800B37A0(void) {
    func_800D91DC(0x140, 0xF0, D_80162084, D_800FA6A0, D_800FA63C.u.sub.unk34,
                  D_800F9F34);
}

static void func_800B37EC(void) {
    D_80162094 = 4;
    func_800D8A78(4);
    func_800E15D8();
    func_800D9E0C(-1, -1, 0);
    D_80095DD4 = 2;
}

// Load stage files
INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B383C);

// load stage entry i (D_800F7DF8[0]) into VRAM staging via DS_read
static void func_800B38E0(void) {
    s32 i = D_800F7DF8[0];

    DS_read(
        *&D_800E8050[i].loc, *&D_800E8050[i].len, 0x801B0000, &func_800B3A04);
    func_800B7FB4();
}

static void func_800B3934(void) {
    func_800B5D38(2);
    func_800B5CD4(2);
    D_80166F64 = 3;
}

// third link of the stage-load chain (func_800B38E0 -> func_800B3A04 ->
// here -> func_800B3934): unpack the part just read into the staging buffer,
// record where the next part lands (D_800F8390[n+1] = D_800F8390[n] + size),
// advance the D_80166F64 phase counter func_800B30E4 waits on, and queue the
// next part's read only while entries remain (D_800F7DF4 is the entry count)
static void func_800B3968(void) {
    s32 size;
    s32 i;

    func_800B5D38(1);
    size = func_800B5CD4(1);
    D_80166F64 = 2;
    D_800F8390[2] = size + D_800F8390[1];
    if (D_800F7DF4 >= 3U) {
        i = D_800F7DF8[2];
        DS_read(*&D_800E8050[i].loc, *&D_800E8050[i].len, (u_long*)0x801B0000,
                func_800B3934);
        func_800B7FB4();
    }
}

// second link of the chain: unpack part 0 out of the staging buffer, then
// queue part 1's read
static void func_800B3A04(void) {
    s32 size;
    s32 i;

    D_800F8390[0] = D_80130200;
    func_800B5D38(0);
    size = func_800B5CD4(0);
    D_80166F64 = 1;
    D_800F8390[1] = size + D_800F8390[0];
    if (D_800F7DF4 >= 2U) {
        i = D_800F7DF8[1];
        DS_read(*&D_800E8050[i].loc, *&D_800E8050[i].len, (u_long*)0x801B0000,
                func_800B3968);
        func_800B7FB4();
    }
}

static void func_800B3AB8(void);
void func_800B5C1C(s16);
void func_800B5E64(s16);
void func_800B3B84(void);
static void func_800B3AB8(void) {
    s16* s0;
    u8** dst;
    s16 v1;
    s16 cmp;

    s0 = &D_800FA9C6;
    v1 = *s0;
    dst = &D_800F8384[v1];
    *dst = D_80103200 + v1 * 0xF000;
    func_800B5E64(*s0);
    func_800B5C1C(*s0);
    cmp = D_800FA9C8;
    if (cmp != 0xC8) {
        DS_read(*&D_800E8068[cmp].loc, *&D_800E8068[cmp].len,
                (u_long*)0x801B0000, func_800B3B84);
        func_800B7FB4();
        return;
    }
    D_80166F64 = 3;
}

extern s16 D_800FA9CA;
extern s16 D_800FA9CC;
void func_800B3C50(void);

// fourth link of the chain, same shape as func_800B3AB8 one slot along
void func_800B3B84(void) {
    s16* s0;
    u8** dst;
    s16 v1;
    s16 cmp;

    s0 = &D_800FA9CA;
    v1 = *s0;
    dst = &D_800F8384[v1];
    *dst = D_80103200 + v1 * 0xF000;
    func_800B5E64(*s0);
    func_800B5C1C(*s0);
    cmp = D_800FA9CC;
    if (cmp != 0xC8) {
        DS_read(*&D_800E8068[cmp].loc, *&D_800E8068[cmp].len,
                (u_long*)0x801B0000, func_800B3C50);
        func_800B7FB4();
        return;
    }
    D_80166F64 = 3;
}

extern s16 D_800FA9CE;

// last link of the chain: unpack and stop
void func_800B3C50(void) {
    s16* s0;
    u8** dst;
    s16 v1;

    s0 = &D_800FA9CE;
    v1 = *s0;
    dst = &D_800F8384[v1];
    *dst = D_80103200 + v1 * 0xF000;
    func_800B5E64(*s0);
    func_800B5C1C(*s0);
    D_80166F64 = 3;
}

static void func_800B3CD0(void) {
    Yamada* y;
    u_long* dst;

    dst = (u_long*)0x801B0000;
    func_800D2980(dst, 0, 0, 0);
    y = &D_800E8068[D_800FA9C4];
    DS_read(y->loc, *&D_800E8068[D_800FA9C4].len, dst, func_800B3AB8);
    func_800B7FB4();
}

static void func_800B3D38(void) {
    func_800C5E94();
    D_800F839C = D_800EA50C;
    DS_read(LBA_ENEMY6_SEFFECT, 0xA800, (u_long*)0x801B0000, func_800B3CD0);
    func_800B7FB4();
}

static void func_800B3D88(void) {
    func_800B588C();
    func_800B6B98(4, 10);
    func_800B36B4();
}

static void func_800B3DBC(void) {
    s32 i;

    func_800B4794();
    func_800B6B98(0, 3);
    func_800B6B98(3, 3);
    if (D_8016360C.setup.stageID == 57) {
        for (i = 0; i < 10; i++) {
            D_801518E4[i].D_80151909 |= 0x10;
        }
    }
}

void func_800BC1E0(u8);
static void func_800C5BEC(void);
static void func_800B3E2C(void) {
    s32 i;
    u8 var_a0;

    D_80163C7C = 0;
    D_800F9D94 = 0;
    D_80162974 = 0;
    D_800F7DE4 = 1;
    D_800F837C = 0;
    D_801031E0 = 1;
    D_801590E0 = 0;
    D_801620A0 = 0;
    D_80163B38 = 0;
    D_801590CC = 0;
    D_800FA6D4 = 0;
    D_801517C4 = 0;
    D_801620A4 = 0;
    D_800FAFDC = 0;
    D_800F7ED4 = 0;
    D_800F9D9C = 0;
    D_800F9D98 = 0;
    D_801590D8 = 0;
    D_80166F58 = 0;
    D_801516A0 = 0;
    D_800F8380 = 0;
    for (i = 0; i < LEN(D_801518E4); i++) {
        D_801518E4[i].D_8015190A = 1;
    }
    for (i = 2; i >= 0; i--) {
        D_800F9F28[i] = 0;
    }
    var_a0 = D_801590CC;
    D_801518E4[var_a0].D_80151906 = 0;
    D_800F8374 = 0xE;
    D_80163798[D_801590E0].unk8 = -2;
    func_800BC1E0(var_a0);
    func_800C5BEC();
}

// search the formation's 6 enemy slots for one whose enemyID matches arg0;
// if found, bump a counter and return 0, else return -1
static s32 func_800B3FAC(s32 arg0) {
    s32 i;
    u8* p = &D_800F7DF4;

    for (i = 0; i < (s32)sizeof(D_8016360C.formation);
         i += sizeof(FormationEntry)) {
        if (((FormationEntry*)((u8*)D_8016360C.formation + i))->enemyID ==
            arg0) {
            *p += 1;
            return 0;
        }
    }
    return -1;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B3FFC);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B430C);

extern u8 D_800FA6D0;
extern s16 D_800E8F94[];
extern u16 D_800E8E88[];

// nudge each of the three party slots' stored angle by +/-0x204 depending on
// its stance. The walk has to be a backward goto: as a `for` it is a loop to
// loop_optimize, which hoists the 0x204 constants and strength-reduces the
// D_800E8E88 offset into a walking pointer -- the target rebuilds the address
// at all three sites. `base` likewise has to be named, or the symbol is
// materialised inside the pointer expression rather than ahead of it.
void func_800B45F0(void) {
    s32 i;
    s32 off;
    s32 k;
    s16* p;
    u8* base;

    i = 0;
    k = 0;
    base = (u8*)D_800E8F94;
    off = D_800FA6D0 * 18;
    p = (s16*)(base + D_800FA6D0 * 6);
loop:
    if (*(u16*)((u8*)D_801636B8 + k + 6) & 1) {
        if (*p == 0) {
            *(s16*)((u8*)D_800E8E88 + off) =
                *(u16*)((u8*)D_800E8E88 + off) + 0x204;
        } else {
            *(s16*)((u8*)D_800E8E88 + off) =
                *(u16*)((u8*)D_800E8E88 + off) - 0x204;
        }
    }
    off += 6;
    p++;
    i++;
    k += 0x10;
    if (i < 3) {
        goto loop;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B46B4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B4794);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B4E30);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B5138);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B54B8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B588C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B5AAC);

/* 9 rows at the exact length (46), of which SIX are not real: splat prints
 * the bare `lui`s of 0x801B0000, 0x801AFFC0 and 0x801AFFC4 as
 * %hi/%lo(func_801B0000) / D_801AFFC0 / D_801AFFC4 by pairing them with the
 * following displacement, and neither D_801AFFC0 nor D_801AFFC4 is defined
 * anywhere in config/ or asm/. Those operands assemble to identical bytes;
 * see the splat-heuristic bullet in CLAUDE.md.
 *
 * The three real rows are $v0/$v1 trading places between the `lbu` of
 * r[0x411] and the reload of p[0]: the target puts the byte in $v1 and keeps
 * func_80025788's return in $v0 for one more instruction, this body reuses
 * $v0 in place (`lbu v0,0x411(v0)`), which is block_alloc tying a destination
 * to a source that dies there.
 *
 * Measured and rejected, all exactly 9 rows: `(r[0x411] & 0xF) + p[0]` (both
 * operand orders), a named `nib` local as s32 and as u8, a named `k` for the
 * raw byte, a named `idx` for the whole sum, and reading the table through
 * *(u32*)0x801B0000 instead of through `p`. `void func_800B5C1C(s16)` is what
 * the header said and is wrong -- the target's `move s0,a0` cannot come from
 * a narrow parameter; s32 is worth 4 rows and the three callers are unmoved
 * by the change (measured). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle1", func_800B5C1C);
#else
extern u8 D_80163F34[];
Unk8009D84C* func_80025788(s32 arg0);

void func_800B5C1C(s32 arg0) {
    u_long* p;
    u8* r;

    p = (u_long*)0x801B0000;
    func_8001C3CC(D_80103200 + arg0 * 0xF000, (u_long*)0x801B0000,
                  *(u32*)(0x801AFFC0 + p[0] * 4));
    r = (u8*)func_80025788(arg0);
    func_8001C3CC(
        D_80163F34 + arg0 * 4108,
        (u_long*)(*(u32*)(0x801AFFC4 + (p[0] + (r[0x411] & 0xF)) * 4) + (s32)p),
        0x1000);
}
#endif

void func_8001C3CC(u8*, u_long*, s32);

// The overlay staged at 0x801B0000 begins with a table of self-relative
// offsets; entry [0] selects which of them holds the payload size. NOTE the
// base is a plain constant, not a symbol: the target keeps only the lui in
// $s0 and adds it back with `addu`, which no symbol reference can produce
// (a symbol would need an addiu %lo). splat pairs the lui/lw heuristically
// and prints it as %hi/%lo(func_801B0000); the linked bytes are identical,
// so checkfn reports those operands as rows it cannot alias away.
s32 func_800B5CD4(s32 arg0) {
    u_long* p;

    p = (u_long*)0x801B0000;
    func_8001C3CC(D_800F8390[arg0], (u_long*)0x801B0000, p[p[0]]);
    return p[p[0]];
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B5D38);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B5E64);

void func_800B60E0(s16);
void func_800B5FC4(s16 arg0) { func_800B60E0(arg0); }

/* 20 rows, 2 instructions short of 62. Both walks are backward gotos for the
 * same reason as func_800BB430 and func_800B45F0 (as `do`/`while` loops gcc
 * hoists the re-read bound and strength-reduces the second walk's index:
 * 29 rows), and the second walk has to reach D_800FA6D8 by byte offset
 * rather than through the declared struct (27 -> 20).
 *
 * Two things left, both in the second walk. The target rebuilds
 * %hi/%lo(D_800FA6D8+0x3C) for the bound at the bottom of every iteration;
 * this body reaches it as `-2(a1)` off the element base, because cse relates
 * two constants that share a symbol_ref. And the target reads arg0 straight
 * out of $a0 for both `arg0 * 64` sites, sharing only the `sll 16`, where
 * this body copies it to $a3 first and re-shifts.
 *
 * Frame: the target's is a bare -0x10 with nothing saved, so the 0x10 is pure
 * declared locals. `unusedLocals` measured 25 at 0, 22 at 8, 20 at 12 and 20
 * at 16. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle1", func_800B5FE8);
#else
void func_800B5FE8(s16 arg0) {
    s32 i;
    s32 off;
    u8* p;
    u8 unusedLocals[12];

    off = arg0 * 0xB9C;
    if (*(s16*)((u8*)D_801518E4 + 0x10 + off) > 0) {
        i = 0;
        p = (u8*)D_801518E4 + 0x3F + off;
    loop1:
        *p |= 8;
        i++;
        p++;
        if (i < *(s16*)((u8*)D_801518E4 + 0x10 + off)) {
            goto loop1;
        }
    }
    if (*(s16*)((u8*)D_800FA6D8 + 0x3C + arg0 * 64) > 0) {
        i = 0;
    loop2:
        *(u8*)((u8*)D_800FA6D8 + 0x3E + arg0 * 64 + i) |= 8;
        i++;
        if (i < *(s16*)((u8*)D_800FA6D8 + 0x3C + arg0 * 64)) {
            goto loop2;
        }
    }
}
#endif

void func_800B60E0(s16);
INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B60E0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B64CC);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B677C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B6B98);

// drains D_80163798 (12-byte entries, -1-terminated, index D_801590E0), one
// entry per call, dispatched by a type byte (0-5, jtbl_800A05FC) via m2c
// structural read (not yet decompiled):
//   0 callback-driven step (func_800BC04C(&func_800C494C)), immediate
//   1 gated on D_800F7DE4: walks a linked status list (D_800FA9D0/1/2),
//     looks like "hide next status icon" (sets D_800FA6D4/D_80161EEC/
//     D_800F99E8 icon slots, or 0xF when the list is exhausted)
//   2 func_800C5C18(4 entry fields), immediate -- shape matches a sound cue
//   3 gated on D_800F7DE4: same linked-list shape as case 1, opposite flag
//     direction -- looks like "show next status icon"
//   4 gated on D_800F7DE4: HP-counter tick-animation init -- writes to PS1
//     scratchpad (0x1F800004/8), computes abs(diff)/entryField, stores
//     start/target/increment into a D_80162978 slot (allocated via
//     func_800BBEAC)
//   5 immediate: sets a per-actor "step complete" flag, conditionally
//     copies animation-state fields
// D_800F7DE4 (the gate for cases 1/3/4) is set once per frame by
// func_800B7FDC below, once all actor slots are ready -- so this function
// is a generic "process the next queued visual/counter effect, one per
// frame" drainer, not itself the source of any particular command's
// damage/effect. See func_800A4AF4's comment in battle.c: opcode 0x14 just
// spins this to drain whatever's already queued
INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B6D6C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B7764);

extern u8 D_801517F0[0x4E];

static void func_800B798C(void) {
    s32 i;

    for (i = 0; i < LEN(D_801517F0); i += 1) {
        D_801517F0[i] = 0xFF;
        D_80163CC0[i].D_80163CC0 = 0;
        D_80163CC0[i].D_80163CC2 = 0;
        D_80163CC0[i].D_80163CC4 = 0;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B79F0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B7DB4);

static void func_800B7F6C(void) {
    volatile s32 padding;

    while (D_80062D99) {
        func_800B7FB4();
    }
    D_80062D98 = 0;
}

void func_800B7FB4(void) { D_801518DC = SystemCdromReadChain(); }

// per-frame tick: pumps the GPU ordering-table draw lists, runs render/vsync,
// drains the action-queue ring buffer (func_800A3ED0 -- see the queue-push
// writeup), and sets D_800F7DE4 = 1 exactly once per frame once every actor
// slot is ready and D_80162080 (a per-frame counter) reaches 0. func_800B6D6C
// gates several of its event-queue steps on this flag, effectively waiting
// for "the next frame is ready" before consuming a queued effect
static void func_800B7FDC(void) {
    s32 i;

    func_800B7FB4();
    ClearOTagR((u_long*)g_cDb->unk40A4, LEN(g_cDb->unk40A4));
    ClearOTag((u_long*)g_cDb->unk4070, LEN(g_cDb->unk4070));
    ClearOTag((u_long*)g_cDb->unk4078, LEN(g_cDb->unk4078));
    ClearOTagR((u_long*)g_cDb->unk70, LEN(g_cDb->unk70));
    ClearOTagR((u_long*)g_cDb->unk4080, LEN(g_cDb->unk4080));
    ClearOTag((u_long*)g_cDb->unk40E4, LEN(g_cDb->unk40E4));
    ClearOTag((u_long*)g_cDb->unk40EC, LEN(g_cDb->unk40EC));
    D_80163C74 = g_dbIndex == 0 ? (DR_MODE*)0x80168000 : (DR_MODE*)0x80184000;
    func_800B8360(1);
    func_800C5CC0();
    func_800B8438();
    for (i = 0; i < 10; i++) {
        if (D_801518E4[i].D_8015190A == 0) {
            D_800F7DE4 = 0;
            break;
        }
        if (D_80162080 == 0) {
            D_800F7DE4 = 1;
        } else {
            D_800F7DE4 = 0;
        }
    }
    func_800A3ED0();
    func_800B8360(2);
    func_800DCFD4((u_long*)g_cDb->unk40E4);
    if (D_800F9D94 == 0) {
        ResetGraph(1);
        D_800F9D94 = 1;
    }
    if (D_8016376A & 2) {
        func_800E16B8(g_cDb->unk40E4, 0x10, 0x10, D_8009D268[0]);
    }
    D_800FA9B8 = VSync(1);
    BATTLE_FlushImageQueue();
    func_800B7FB4();
    D_80158D08 = func_800D8A88();
    SetGeomScreen(D_80162084);
    D_801516F4++;
    func_800B7F6C();
    func_800B950C();
    D_801516A0 = D_800F198C;
}

void func_800B8234(s32 arg0) {
    if (arg0) {
        func_800D0C80(D_801590CC);
        D_801517BC = 0;
    }
}

static void func_800B8268(void) {
    s32 i;
    u8* var_a1;
    s32 var_t1;

    i = 0;
    var_t1 = 1;
    var_a1 = D_80163784;
    while (i < 10) {
        *var_a1 = D_801636B8[i].D_801636B9;
        if (!(D_80151200[i].D_8015120C & 8) &&
            D_801518E4[i].D_801518E6 != *var_a1 &&
            D_801518E4[i].D_8015190A == var_t1) {
            D_801518E4[i].D_80151922 |= 1;
            D_801518E4[i].D_801518E6 = *var_a1;
        }
        var_a1++;
        i += 1;
    }
    D_80163787 = 0;
}

// build a draw-mode prim (texture page selected by arg0) and add it to the OT
static void func_800B8360(s32 arg0) {
    SetDrawMode(D_80163C74, 1, 1, (arg0 & 3) << 5, 0);
    AddPrim(g_cDb->unk4078, D_80163C74++);
}

static void func_800B83C4() {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (D_80151200[i].D_8015120C & 1) {
            func_800BA4C8();
            func_800BA40C();
            return;
        }
    }
    func_800BA40C();
    func_800BA4C8();
}

void func_800B8438(void) {
    func_800B9568();
    if (D_801635FC) {
        D_801635FC--;
    }
    switch (D_80163C7C) {
    case 2:
        func_800B905C();
        func_800BC440();
        func_800BA4C8();
        break;
    case 0:
    case 1:
    case 6:
        break;
    case 3:
    case 4:
    case 5:
    default:
        func_800B8EE4();
        func_800B905C();
        func_800B8234(D_801517BC);
        func_800BC440();
        func_800B7FB4();
        func_800B83C4();
        func_800B8B48();
        break;
    }
    func_800B7FB4();
    func_800B91CC();
    D_80151694 = D_80163758[1];
    func_800B85E0();
    func_800BC81C(D_800F8370, D_801518E4[D_801590CC].D_80151906);
    func_800BC8B0(D_800F8370);
    func_800B8268();
    SetFarColor(0, 0, 0);
    func_800BC538();
    func_800BC348();
    func_800BB75C(&D_800FA63C, &D_800FA958, &D_80158D00, &D_801031E8);
    func_800C627C();
}

static void func_800B85E0() {
    s32 i;

    if (D_800F7ED4 != 100 && D_800FA6B8) {
        func_800BB804();
        D_80163C7C = 5;
        func_800D8B2C();
        D_800F7ED4 = 100;
        D_80163798[D_801590E0].unk8 = -3;
        func_800BB684();
        for (i = 0; i < 3; i++) {
            D_801518E4[i].D_80151922 |= 0x20;
            D_80151200[i].D_80151200 = D_801636B8[i].D_801636C0;
        }
    }
    if (D_800F9D98 != 100 && (g_BattleMode & 1)) {
        D_80163C7C = 5;
        func_800D8B2C();
        D_800F9D98 = 100;
        D_80163798[D_801590E0].unk8 = -1;
        func_800BB684();
    }
    if (!D_801590D8 && D_80163B80) {
        func_800BB864();
        D_801590D8 = 1;
    }
    if (D_800F9D9C != 100) {
        i = 0;
        if (g_BattleMode & 8) {
            for (; i < 3; i++) {
                D_801518E4[i].D_80151922 |= 1;
                D_801518E4[i].D_801518E6 = D_801636B8[i].D_801636B9;
                D_801518E4[i].D_80151922 |= 0x20;
                D_80151200[i].D_80151200 = D_801636B8[i].D_801636C0;
            }
            D_800F9D9C = 100;
            D_80163C7C = 5;
            func_800D8B2C();
            D_80163798[D_801590E0].unk8 = -1;
            func_800BB684();
        }
    }
}

extern u8 D_801517F0[0x4E];

s16 func_800B888C(s32 arg0) {
    s32 i;

    for (i = 0; i < LEN(D_801517F0); i++) {
        if (arg0 == D_801517F0[i]) {
            return i;
        }
    }
}

// initialize D_80162978 slot v (registered via func_800BBEAC) from arg0 and
// dispatch
static void func_800B88CC(s32 arg0) {
    s32 v = func_800BBEAC(&func_800CE970);

    D_80162978[v].D_8016297C = 0;
    D_80162978[v].D_80162980 = arg0;
    func_800B8A34(func_800B888C(arg0), v);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B8944);

/* 18 rows, 4 instructions long -- and the four are `nop`s in load-delay
 * slots. The target hoists each *next* source load above the current store
 * (`lhu a0,D_800F99EE(v1)` issues before `sh v0,D_80162978+0xC(a1)`), which
 * fills the slot; this body cannot move them and pays a nop at each of the
 * four sites.
 *
 * The obvious diagnosis is aliasing -- true_dependence lets a MEM_IN_STRUCT_P
 * load float past a store that is not one -- and it is wrong, or at least not
 * reachable this way. Measured: the source reads as subscripts on a cast
 * pointer, `((u16*)(D_800F99EC + s))[1]`, exactly 18 (a subscript on a
 * *pointer* is an INDIRECT_REF, not an ARRAY_REF, so no flag is set); the
 * source reads as COMPONENT_REFs through a struct-pointer cast, which does
 * set the flag, also exactly 18; the destination through a named
 * `Unk80162978*` local, 48 rows and 20 instructions short, because the
 * pointer collapses all eight $at expansions. So both sides of the flag are
 * closed and the lever is elsewhere.
 *
 * The prototype in battle_private.h said `(s16, s32)`; the target has no
 * entry conversion on either parameter, so it is `(s32, s32)`. That is
 * already fixed in the header. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle1", func_800B8A34);
#else
extern u8 D_800F99EC[];

void func_800B8A34(s32 arg0, s32 arg1) {
    s32 s;
    s32 d;

    s = arg0 * 12;
    d = arg1 * 32;
    *(s16*)((u8*)D_80162978 + 0xA + d) = *(u16*)(D_800F99EC + s);
    *(s16*)((u8*)D_80162978 + 0xC + d) = D_801590CC;
    *(s16*)((u8*)D_80162978 + 0xE + d) = *(u16*)(D_800F99EC + 2 + s);
    *(s32*)((u8*)D_80162978 + 0x10 + d) = *(s16*)(D_800F99EC + 4 + s);
    *(s32*)((u8*)D_80162978 + 0x14 + d) = *(s16*)(D_800F99EC + 6 + s);
    *(u8*)((u8*)D_80162978 + 0x18 + d) = D_800F8CF0;
    *(s16*)((u8*)D_80162978 + 6 + d) = *(u16*)(D_800F99EC + 8 + s);
    *(u8*)((u8*)D_80162978 + 0x19 + d) = *(u8*)(D_800F99EC + 10 + s);
}
#endif

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B8B48);

static void func_800B8E48(s32 arg0) {
    s32 temp_a0;

    temp_a0 = arg0 & 0xFF;
    D_801518E4[temp_a0].D_8015190A = 1;
    D_801518E4[temp_a0].D_80151909 &= 0x7F;
    D_80151200[temp_a0].D_8015120C &= 0xFFDF;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B8EE4);

extern s16 D_800F7E08[];

// `k` has to be its own local: written inline, fold distributes
// (arg0 - 4) * 12 into arg0 * 12 - 0x30 and folds the -0x30 onto the symbol,
// which is a different (and equally correct) address computation.
void func_800B8FCC(s32 arg0) {
    s32 idx;
    s32 k;
    Unk800F57D0* p;

    if (D_80151200[arg0].D_80151232 == 6) {
        idx = 6;
    } else {
        k = arg0 - 4;
        idx = *(s16*)((u8*)D_800F7E08 + k * 12);
    }
    p = (Unk800F57D0*)D_800F8384[idx];
    func_800C7C4C(arg0, p->unk8 + 0x68, p + 1, p);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B905C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B91CC);

// promote each model's staged (x, y) into its committed (prevX, prevY)
static void func_800B950C(void) {
    s32 i;

    for (i = 0; i < LEN(g_modelScreenPos); i++) {
        g_modelScreenPos[i].prevX = g_modelScreenPos[i].x;
        g_modelScreenPos[i].prevY = g_modelScreenPos[i].y;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800B9568);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BA11C);

// advances two wrapping counters (mod 4, mod 32) and accumulates an offset
// each time one wraps to 0; func_800BA2BC (still undecompiled) reads/writes
// the same D_80163B44/D_800F8182 pair with an analogous mod-32 decrement, so
// this is one tick of a periodic effect shared with that function
static void func_800BA24C(void) {
    D_800F8182[0] = 0;
    if (D_80163B44[0] == 0) {
        D_800F8182[0] = -0x28;
    }
    if (D_80163B44[1] == 0) {
        D_800F8182[0] -= 0x50;
    }
    D_80163B44[0] = (D_80163B44[0] - 1) & 3;
    D_80163B44[1] = (D_80163B44[1] - 1) & 0x1F;
}

/* 13 rows at the exact length (41). Every instruction and every opcode is
 * right; the whole residue is register naming. The target keeps `off` in $v0
 * and lets `arg0 * 2` and the loaded value share $v1; this body keeps `off` in
 * $a1 and the value in $v0.
 *
 * Measured and rejected, all against the same 41 instructions:
 *   - `off = arg0 * 52;` hoisted above the branch: 22 rows, -3 instructions
 *     (the target computes the multiply once per arm, not once).
 *   - no locals at all, the store written out in both arms: 27 rows, -1. That
 *     shape is structurally right -- 2 offset computations, 3 $at expansions,
 *     one cross-jumped store -- but a fourth reference to D_800F8182 makes cse
 *     promote its address to a register and all three $at expansions go.
 *   - the offset expression inline at all three sites: 13 rows, +4.
 *   - `v` as s32 / u16 / s16, and `v` declared before `off`: exactly 13, all
 *     four. A flat sweep over the declaration dimension.
 * cc1 -dl says there are three global allocnos: $a0's parameter (7 refs / 21
 * insns), `off` (6 / 8) and the loaded value (4 / 6). `off` already has the
 * highest allocno_compare priority (2.0 against 1.33) and still does not get
 * $v0, so the ranking is not the lever and neither term is reachable from C
 * without emitting an instruction. This is a park, not a permuter target. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle1", func_800BA2BC);
#else
void func_800BA2BC(s32 arg0) {
    s32 off;
    s16 v;

    if (D_80163B44[arg0] < 0x10) {
        off = arg0 * 52;
        v = *(u16*)((u8*)D_800F8182 + off) - 0x19;
    } else {
        off = arg0 * 52;
        v = *(u16*)((u8*)D_800F8182 + off) + 0x19;
    }
    *(s16*)((u8*)D_800F8182 + off) = v;
    D_80163B44[arg0] = (D_80163B44[arg0] - 1) & 0x1F;
}
#endif

extern s32 D_801590E8[];
extern u8 D_801590E4[];
extern u16 D_800F5B74;

// build a one-shot descriptor in the scratchpad and hand it to the display
// list builder. `neg` must be u8: as an s32 local, `v < 0` is folded to
// `srl v,31`, and only a QImode destination leaves do_store_flag's `slti`.
void func_800BA360(s32 arg0, u_long** arg1, s32 arg2, s32 arg3) {
    Unk801B0C98* pkt;
    s32 v;
    u8 neg;
    s32 flags;
    u8 unusedLocals[8];

    pkt = (Unk801B0C98*)0x1F800320;
    v = D_801590E8[arg0];
    pkt->unkC = 0x20;
    pkt->unkE = 0;
    pkt->unk8 = 0;
    pkt->unk0 = (s32*)((v & 0x7FFFFFFF) + (s32)D_801590E4);
    flags = arg3 | 0x180;
    neg = v < 0;
    pkt->unk4 = (neg << 3) | flags;
    pkt->unkA = D_800F5B74;
    D_80163C74 = func_800D29D4(pkt, arg1, arg2, D_80163C74);
}

static void func_800BA40C(void) {
    s32 i;
    u8 param;

    for (i = 0; i < 3; i++) {
        if (!(D_801518E4[i].D_80151909 & 2)) {
            param = i;
            func_800C1908(param);
            func_800BA598(i);
            if (D_801518E4[i].D_8015190B & 0x80) {
                func_800BB2A8(param);
                func_800BB030(i);
            }
        }
    }
}

static void func_800BA4C8(void) {
    s32 i;

    for (i = 4; i < D_800F7E04[0] + 4; i++) {
        if (!(D_801518E4[i].D_80151909 & 0x80)) {
            continue;
        }
        if (D_801518E4[i].D_80151909 & 2) {
            continue;
        }
        func_800C1908(i);
        func_800BA598(i);
        if (D_801518E4[i].D_8015190B & 0x80) {
            func_800BB030(i);
        }
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BA598);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BACEC);

static void func_800BAF34(BattleModelSub* modelSub) {
    s32 flag;

    *(MATRIX**)0x1F800020 = modelSub->pm;
    *(MATRIX*)0x1F800024 = **(MATRIX**)0x1F800020;
    MulMatrix2((MATRIX*)0x1F800024, &modelSub->m);
    SetRotMatrix((MATRIX*)0x1F800024);
    SetTransMatrix((MATRIX*)0x1F800024);
    RotTrans(&modelSub->sv2, (VECTOR*)modelSub->m.t, &flag);
    SetRotMatrix(&modelSub->m);
    SetTransMatrix(&modelSub->m);
}

static void func_800BAFF8(MATRIX* m, VECTOR* v) {
    ScaleMatrix(m, v);
    SetRotMatrix(m);
    SetTransMatrix(m);
}

static void func_800BB030(s16 arg0) {
    s32 i;
    Unk801B0C98* unk;

    unk = (Unk801B0C98*)0x1F800020;
    SetFarColor(D_801518E4[arg0].D_8015190C, D_801518E4[arg0].D_8015190D,
                D_801518E4[arg0].D_8015190E);
    SetRotMatrix(&D_801518E4[arg0].m);
    SetTransMatrix(&D_801518E4[arg0].m);
    for (i = 0; i < D_800FA6D8[arg0].unk3C; i++) {
        RotMatrixYXZ(
            &D_800FA6D8[arg0].unk8[i].sv1, &D_800FA6D8[arg0].unk8[i].m);
    }

    for (i = 0; i < D_800FA6D8[arg0].unk3C; i++) {
        func_800BAF34(&D_800FA6D8[arg0].unk8[i]);
        if (!D_800FA6D8[arg0].unk4[i])
            continue;
        unk->unk0 = D_800FA6D8[arg0].unk4[i];
        unk->unk4 = D_800FA6D8[arg0].unk3E[i] | 0x180;
        unk->unk8 = 0;
        unk->unkA = D_801518E4[arg0].unk14[0];
        unk->unkC = 0x20;
        unk->unkE = D_801518E4[arg0].unk14[1];
        if (D_801518E4[arg0].D_80151909 & 4) {
            continue;
        }
        D_80163C74 = func_800D29D4(unk, g_cDb->unk70, 12, D_80163C74);
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BB2A8);

/* 18 rows, one instruction short of 50. The loop shape is settled: written as
 * a `do`/`while` it is a loop to loop_optimize, which hoists the +0x78 base
 * into a fourth callee-saved register, strength-reduces the index into a
 * walking pointer and lifts the `(s16)arg1` conversion out of the body -- the
 * target does none of those, which is what a backward goto buys (32 rows ->
 * 27). The `q` local recomputed inside the loop is what puts the `+ off` on
 * the symbol side of the sum rather than on the index side (27 -> 18), and
 * `unusedLocals[8]` is the 8 bytes of frame between the outgoing-argument
 * area and the register saves (26 -> 18).
 *
 * Measured and rejected: the flat `((u8*)D_801518E4 + 0x78 + off) + i * 4`
 * and the subscript `((s32*)((u8*)D_801518E4 + 0x78 + off))[i]` written
 * inline, both 19; the goto loop with no `q` local, 27; no pad, 26.
 *
 * The residue is where `move s2,a1` lands -- the target copies arg1 into its
 * callee-saved register in the entry block, this body does it after the
 * zero-trip guard -- plus the one instruction that costs. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle1", func_800BB430);
#else
void func_800D3520(s32, s16);

void func_800BB430(s16 arg0, s32 arg1) {
    s32 i;
    s32 off;
    s32 p;
    s32* q;
    u8 unusedLocals[8];

    off = arg0 * 0xB9C;
    if (*(s16*)((u8*)D_801518E4 + 0x10 + off) > 0) {
        i = 0;
    loop:
        q = (s32*)((u8*)D_801518E4 + 0x78 + off);
        p = q[i];
        i++;
        if (p != 0) {
            func_800D3520(p, arg1);
        }
        if (i < *(s16*)((u8*)D_801518E4 + 0x10 + off)) {
            goto loop;
        }
    }
}
#endif

// See func_800B5CD4 for why the 0x801B0000 base is a literal and not the
// func_801B0000 symbol splat prints.
void func_800BB4F8(void) {
    u_long* p;

    p = (u_long*)0x801B0000;
    func_800D2980((u_long*)(p[p[0]] + (s32)p), 0, 0, 0);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BB538);

void func_800BB67C(s32 arg0, Unk800BB67C* arg1) { arg1->unk30 = arg0; }

extern u8 D_8015184C[];
extern u8 D_8015185A[];
extern u8 D_80151868[];
extern u8 D_80151876[];
extern u8 D_801518AC[];
extern u8 D_801518BA[];
extern u8 D_801518C8[];
extern u8 D_801518D6[];

// reset the eight per-slot cursors to "none" and adopt the camera the current
// setup entry names. The chain is not untidiness: a chained assignment stores
// right to left, which is the descending address order the target has.
void func_800BB684(void) {
    s16 v;

    v = D_80163798[D_801590E0].unk8;
    if (v == -4) {
        return;
    }
    D_800F8370 = v;
    D_801590DC = 0;
    *(s16*)D_8015184C = *(s16*)D_8015185A = *(s16*)D_80151868 =
        *(s16*)D_80151876 = *(s16*)D_801518AC = *(s16*)D_801518BA =
            *(s16*)D_801518C8 = *(s16*)D_801518D6 = 0xFF;
    func_800BC2F0();
    if (D_800F837C == 3) {
        return;
    }
    if ((D_801516F4 & 3) == 3) {
        return;
    }
    D_800F837C = D_801516F4 & 3;
}

static void func_800BB75C(Unk800BB75C* arg0, MATRIX* m, s16* arg2, s16* arg3) {
    int flag;

    func_800D85B0(m, arg2, arg3, &D_800E7D10);
    RotMatrixYXZ(&arg0->sv, &arg0->m);
    TransMatrix(&arg0->m, &arg0->u.v);
    MulMatrix2(m, &arg0->m);
    SetRotMatrix(m);
    SetTransMatrix(m);
    RotTrans(&arg0->u.sub.sv2, (VECTOR*)&arg0->m.t, &flag);
    func_800BAFF8(&arg0->m, &D_800E7D20);
}

static void func_800BB89C(void);
static void func_800BB804(void) {
    if (!(D_8016376A & 0x20)) {
        SystemLoadFileBySector(
            LBA_ENEMY6_FAN2, 0x1000, (u_long*)0x801D0000, func_800BB89C);
        func_800B7FB4();
        return;
    }
    D_80163B80 = 0;
    D_800FA6B8 = 0;
}

static void func_800BB864(void) {
    SystemLoadFileBySector(
        LBA_ENEMY6_OVER2, 0x800, (u_long*)0x801D0000, func_800BB89C);
    func_800B7FB4();
}

static void func_800BB89C(void) {
    D_80163B80 = 0;
    D_800FA6B8 = 0;
    D_8009A000[0] = !(!(D_8016376A & 0x10) && !D_80083338) ? 0x10 : 0x14;
    D_8009A004[0] = 0x801D0000;
    SystemAkaoExecute();
}

void func_800BB90C(void) {
    D_8009A000[0] = 0xA0;
    D_8009A004[0] = 0x7F;
    SystemAkaoExecute();
}

static void func_800BB944(void) {
    func_800BB90C();
    D_8009A000[0] = 0xF1;
    SystemAkaoExecute();
}

// queue sound command 0xC1
void func_800BB978(void) {
    D_8009A000[0] = 0xC1;
    D_8009A004[0] = 0x12C;
    D_8009A008[0] = 0;
    SystemAkaoExecute();
}

// queue sound command 0x30, dispatched directly via func_8002DF88 (akao.c)
// rather than the D_8009A000 global queue used by the sibling functions below
void func_800BB9B8(s32 arg0) {
    s16* ptr;

    ptr = &D_800F4AD0;
    *ptr = 0x30;
    D_800F4AD4 = arg0 & 0xFFFF;
    D_800F4AD8 = arg0 & 0xFFFF;
    func_8002DF88(ptr);
}

// queue sound command 0x2B
void func_800BB9FC(s32 arg0) {
    s32 param;

    D_8009A000[0] = 0x2B;
    param = arg0 & 0xFFFF;
    D_8009A004[0] = 0x40;
    D_8009A008[0] = param;
    SystemAkaoExecute();
}

// queue sound command 0x20
void func_800BBA40(s32 arg0) {
    s32 param;

    D_8009A000[0] = 0x20;
    param = arg0 & 0xFFFF;
    D_8009A004[0] = 0x40;
    D_8009A008[0] = param;
    SystemAkaoExecute();
}

// queue sound command 0x20 with a stereo pan derived from the model's
// committed screen X (arg1 == -1 means "use the caller's pan verbatim")
void func_800BBA84(u16 arg0, s16 arg1, s32 arg2) {
    s32 param;

    if (arg1 == -1) {
        param = (u8)arg2;
    } else {
        param = (g_modelScreenPos[arg1].prevX / 5 * 2) & 0x7E;
    }
    D_8009A004[0] = param;
    D_8009A000[0] = 0x20;
    D_8009A008[0] = arg0 & 0xFFFF;
    SystemAkaoExecute();
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BBB20);

static void func_800BBDF8(void) {
    if (g_dbIndex == 0) {
        D_800F4AF4 = D_80163C74;
        if ((u32)D_80163C74 > (u32)0x80184000) {
            PadStop();
            ResetGraph(1);
            StopCallback();
            SystemError('b', 0);
        }
    } else {
        D_800F4AF8 = D_80163C74;
        if ((u32)D_80163C74 > (u32)0x801A0000) {
            PadStop();
            ResetGraph(1);
            StopCallback();
            SystemError('b', 1);
        }
    }
}

extern void (*D_80161EF0[])(void);

// Four near-clone registration helpers: claim the first free callback slot at
// or after the reserved prefix, stamp the current phase into the parallel
// record array, bump the population count and return the slot index. A full
// table is fatal -- the error code is the only thing that distinguishes them.
s32 func_800BBEAC(void (*func)(void)) {
    s16 i;

    for (i = 0; i < 100; i++) {
        if (D_80161EF0[i] == NULL && i >= D_8015169C) {
            D_80161EF0[i] = func;
            D_80162978[i].D_80162978 = D_8015169C;
            D_80162080++;
            return i;
        }
    }
    PadStop();
    ResetGraph(1);
    StopCallback();
    SystemError(0x61, 1);
}

extern void (*D_80163B48[])(void);
extern u16 D_80163B7C;

s32 func_800BBF7C(void (*func)(void)) {
    s16 i;

    for (i = 0; i < 10; i++) {
        if (D_80163B48[i] == NULL && i >= D_801590D0) {
            D_80163B48[i] = func;
            D_801620AC[i].D_801621AC = D_801590D0;
            D_80163B7C++;
            return i;
        }
    }
    PadStop();
    ResetGraph(1);
    StopCallback();
    SystemError(0x61, 2);
}

extern void (*D_80163B84[])(void);
extern u16 D_80163C78;

s32 func_800BC04C(void (*func)(void)) {
    s16 i;

    for (i = 0; i < 60; i++) {
        if (D_80163B84[i] == NULL && i >= D_801590D4) {
            D_80163B84[i] = func;
            D_801621F0[i].D_801621F0 = D_801590D4;
            D_80163C78++;
            return i;
        }
    }
    PadStop();
    ResetGraph(1);
    StopCallback();
    SystemError(0x61, 4);
}

// same shape without the reserved prefix
s32 func_800BC11C(void (*func)(void)) {
    s16 i;

    for (i = 0; i < 0x10; i++) {
        if (D_800FA978[i] == 0) {
            D_800FA978[i] = (s32)func;
            D_800F7ED8[i].D_800F7ED8 = D_800F8360;
            D_800FA9BC++;
            return i;
        }
    }
    PadStop();
    ResetGraph(1);
    StopCallback();
    SystemError(0x61, 3);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BC1E0);

static void func_800BC2F0(void) {
    s32 i;

    D_800FA9BC = 0;
    for (i = 0; i < 0x10; i++) {
        D_800FA978[i] = 0;
        D_800F7ED8[i].D_800F7ED8 = 0;
        D_800F7ED8[i].D_800F7EDA = 0;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BC348);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BC440);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BC538);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BC630);

void func_800BCA58(s16);
void func_800C1104();
static void func_800BC72C(void) {
    func_800C1104();
    func_800BCA58(3);
}

extern u8 D_80163B3C;
extern s16 D_80158D02;
extern s16 D_80158D04;
extern s16 D_801031EA;
extern s16 D_801031EC;

// publish camera placement D_80163B3C of the current formation into the same
// six globals func_800BCA58 writes
void func_800BC754(void) {
    s32 off;

    off = D_80163B3C * 12;
    D_80158D00 = *(u16*)((u8*)&D_8016360C + 0x1C + off);
    D_80158D02 = *(u16*)((u8*)&D_8016360C + 0x1E + off);
    D_80158D04 = *(u16*)((u8*)&D_8016360C + 0x20 + off);
    D_801031E8 = *(u16*)((u8*)&D_8016360C + 0x22 + off);
    D_801031EA = *(u16*)((u8*)&D_8016360C + 0x24 + off);
    D_801031EC = *(u16*)((u8*)&D_8016360C + 0x26 + off);
}

void func_800BC630(void);
void func_800BCB1C(u8, s16, s16);
void func_800BEA38(u8, s16, s16);
// run both per-slot handlers for each of the three party slots, then the
// shared tail step; skipped entirely while D_801590DC is set
static void func_800BC81C(s16 arg0, s16 arg1) {
    s32 i;

    if (D_801590DC == 0) {
        for (i = 0; i < 3; i++) {
            func_800BEA38(i, arg1, arg0);
            func_800BCB1C(i, arg1, arg0);
        }
        func_800BC630();
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BC8B0);

extern u16 D_80151844[];
extern u16 D_80151846[];
extern u16 D_80151848[];
extern u16 D_801518A4[];
extern u16 D_801518A6[];
extern u16 D_801518A8[];
extern s16 D_80158D02;
extern s16 D_80158D04;
extern s16 D_801031EA;
extern s16 D_801031EC;

// two parallel 14-byte-stride records; publish slot arg0 of each
void func_800BCA58(s16 arg0) {
    s32 off;

    off = arg0 * 14;
    D_80158D00 = *(u16*)((u8*)D_80151844 + off);
    D_80158D02 = *(u16*)((u8*)D_80151846 + off);
    D_80158D04 = *(u16*)((u8*)D_80151848 + off);
    D_801031E8 = *(u16*)((u8*)D_801518A4 + off);
    D_801031EA = *(u16*)((u8*)D_801518A6 + off);
    D_801031EC = *(u16*)((u8*)D_801518A8 + off);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BCB1C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BE49C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BE69C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BE86C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BEA38);

extern u8 D_8015184C[];
extern u8 D_801518AC[];

// Read the next u16 from arg0's byte stream via this category's read cursor.
s16 func_800BFA98(u8* arg0, s32 arg1) {
    s32 off = (arg1 & 0xFF) * 14;
    u16 pos = *(u16*)(D_8015184C + off);
    u32 lo;
    u8 hi;

    *(u16*)(D_8015184C + off) = pos + 1;
    lo = arg0[pos];
    *(u16*)(D_8015184C + off) = pos + 2;
    hi = arg0[(u16)(pos + 1)];
    return (hi << 8) + lo;
}

s16 func_800BFB10(u8* arg0, s32 arg1) {
    s32 off = (arg1 & 0xFF) * 14;
    u16 pos = *(u16*)(D_801518AC + off);
    u32 lo;
    u8 hi;

    *(u16*)(D_801518AC + off) = pos + 1;
    lo = arg0[pos];
    *(u16*)(D_801518AC + off) = pos + 2;
    hi = arg0[(u16)(pos + 1)];
    return (hi << 8) + lo;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BFB88);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BFDA0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800BFF88);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C0088);

void func_800D54EC(s16, s16*);
void func_800C0DD8(s16, s32, s32);
s32 func_800C0314(s32, s32);

// Sample sp[3], then accumulate it into the scratchpad totals at 0x1F800000.
void func_800C018C(s16 arg0, s16 arg1, s32 arg2, s32 arg3) {
    s16 sp[3];

    if (arg0 == 0xF) {
        func_800D54EC(D_80151774, sp);
    } else {
        func_800D3994(arg0, arg1, sp);
        func_800C0DD8(arg0, arg2 & 0xFF, arg3 & 0xFF);
    }
    *(s32*)0x1F800000 += sp[0];
    *(s32*)0x1F800004 += sp[1];
    *(s32*)0x1F800008 += sp[2];
}

void func_800C0254(s16 arg0, s16 arg1) {
    s16 sp[3];

    if (arg0 == 0xF) {
        func_800D54EC(D_80151774, sp);
    } else {
        func_800D3994(arg0, arg1, sp);
        *(s32*)0x1F800004 = func_800C0314(*(s32*)0x1F800004, (u8)arg0);
    }
    *(s32*)0x1F800000 += sp[0];
    *(s32*)0x1F800004 += sp[1];
    *(s32*)0x1F800008 += sp[2];
}

s32 func_800C0314(s32 arg0, s32 arg1) {
    if (D_801518E4[(u8)arg1].D_801518E6 == 1 &&
        D_80151200[(u8)arg1].D_80151230 > 0) {
        return arg0;
    }
    return arg0 + D_80151200[(u8)arg1].D_80151230;
}

// magnitude of (arg0 - arg1) via GTE sqrt
static s16 func_800C03B8(s16 arg0, s16 arg1) {
    s32 delta;

    delta = arg0 - arg1;
    return SquareRoot0(delta * delta);
}

s32 func_800C03FC(s32 arg0, s32 arg1) { return arg0 < 0 ? -arg1 : arg1; }

static void func_800C0410(void) {
    switch (D_800F7ED8[D_800F8360].D_800F7EDA) {
    case 0:
        func_800C0480(D_800F8360);
        func_800C0630(D_800F8360);
        return;
    case 1:
        func_800C0630(D_800F8360);
        return;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C0480);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C0630);

static void func_800C0900(void) {
    switch (D_800F7ED8[D_800F8360].D_800F7EDA) {
    case 0:
        func_800C0970(D_800F8360);
        func_800C0B20(D_800F8360);
        return;
    case 1:
        func_800C0B20(D_800F8360);
        return;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C0970);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C0B20);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C0DD8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C1104);

// cosine-eased interpolation between arg0 and arg1
static s32 func_800C1304(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 val;
    s32 delta;

    delta = arg1 - arg0;
    val = (rcos((s16)(((arg3 << 0xB) / arg2) + 0x800)) + 0x1000) * delta;
    return arg0 + val / 0x2000;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C1394);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C14C0);

static s32 func_800C169C(u8 arg0) {
    D_801518E4[arg0].D_80151909 |= 8;
    if (D_80151200[arg0].D_80151200 & 0x2000) {
        return 10;
    }
    if (D_80151200[arg0].D_80151200 & 0x4000) {
        return 5;
    }
    if (D_80151200[arg0].D_80151200 & 0x0008) {
        return 1;
    }
    if (D_80151200[arg0].D_80151200 & 0x800000) {
        return 3;
    }
    if (D_80151200[arg0].D_80151200 & 0x01000000) {
        return 6;
    }
    if (D_80151200[arg0].D_80151200 & 0x04000000) {
        return 8;
    }
    if (D_80151200[arg0].D_80151200 & 0x8000) {
        return 9;
    }
    if (D_80151200[arg0].D_80151200 & 0x400000) {
        return 7;
    }
    D_801518E4[arg0].D_80151909 &= ~8;
    return 0;
}

static void func_800C17A0(s32 arg0, s32 arg1) {
    switch (D_800EA19C[arg1][0]) {
    case 0:
        D_801518E4[arg0].unk14[0] = 0;
        break;
    case 1:
        D_801518E4[arg0].unk14[0] = 0x800;
        break;
    case 2:
        D_801518E4[arg0].unk14[0] = 0xC00;
        break;
    }
    D_801518E4[arg0].D_8015190C = D_800EA19C[arg1][1];
    D_801518E4[arg0].D_8015190D = D_800EA19C[arg1][2];
    D_801518E4[arg0].D_8015190E = D_800EA19C[arg1][3];
    D_801518E4[arg0].D_80151908 = 0;
}

static void func_800C5468(u8 arg0);
void func_800C5170(u8);

static void func_800C1908(u8 arg0) {
    s32 temp_a1;
    s16 var_a0;
    u8 temp_s0;

    temp_s0 = arg0;
    if (D_801518E4[temp_s0].D_80151922 & 0x20) {
        if (temp_s0 < 4) {
            D_800F9F28[temp_s0] = D_801636B8[temp_s0].D_801636C0;
        }
        func_800C5170(temp_s0);
        func_800C5468(temp_s0);
        func_800C17A0(temp_s0, func_800C169C(temp_s0));
        D_801518E4[temp_s0].D_80151922 &= 0xDF;
    }
    temp_a1 = arg0;
    if (D_80151200[temp_a1].D_80151235 == 0) {
        if (D_80151200[temp_a1].D_80151200 & 0x4000) {
            D_80151200[temp_a1].D_80151233 = 3;
            return;
        }
        D_80151200[temp_a1].D_80151233 = 0;
        if (D_80151200[temp_a1].D_80151200 & 0x100) {
            D_80151200[temp_a1].D_80151233 = 1;
        }
        if (D_80151200[temp_a1].D_80151200 & 0x200) {
            D_80151200[temp_a1].D_80151233 = 2;
        }
        if (D_80151200[temp_a1].D_80151200 & 0x400) {
            D_80151200[temp_a1].D_80151233 = 3;
        }
        if (D_80151200[temp_a1].D_80151200 & 0x02000000) {
            D_80151200[temp_a1].D_80151233 = 3;
        }
        if (D_80151200[temp_a1].D_80151200 & 0x40) {
            if (D_801518E4[temp_a1].D_801518E6 == D_80163784[temp_a1]) {
                D_801518E4[temp_a1].unk160.vy += 0x100;
            }
        }
        var_a0 = arg0;
        if (D_80151200[var_a0].D_80151200 & 0x400000 &&
            D_801518E4[var_a0].D_801518E6 == D_80163784[var_a0]) {
            if (D_801518E4[var_a0].D_801518FC == 0) {
                D_801518E4[var_a0].unk160.vy = 0x800;
            } else {
                D_801518E4[var_a0].unk160.vy = 0;
            }
        }
        var_a0 = arg0;
        if (D_801518E4[var_a0].D_80151909 & 8) {
            if (D_801518E4[var_a0].D_80151908 < 0x10) {
                D_801518E4[var_a0].unk14[0] += 0x80;
            } else {
                D_801518E4[var_a0].unk14[0] -= 0x80;
            }
            D_801518E4[arg0].D_80151908--;
            D_801518E4[arg0].D_80151908 &= 0x1F;
        }
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C1D8C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C2000);

static void func_800C20E8(s16 arg0, s16* arg1) {
    s32 i;

    for (i = 0; i < 4; i++) {
        arg1[3 - i] = arg0 % 10;
        arg0 /= 0xA;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C2150);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C223C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C2704);

s32 func_800C2704(u_long*, s16, s16, s32, s32, s32, s32, s32);

// draw the limit gauge over the model's staged screen position. The two
// coordinates have to be loaded into s32 locals with the offsets applied at
// the call: applied inline, the `- 0xE` is narrowed against the s16 parameter
// and 0xFFF2 stops being a legal addiu immediate.
void func_800C2864(s32 arg0) {
    s32 off;
    s32 px;
    s32 py;

    if (D_80163C7C == 4 && D_800FAFDC == 0 && D_801620A4 == 0) {
        off = (u8)arg0 * 8;
        px = *(u16*)((u8*)g_modelScreenPos + 4 + off);
        py = *(u16*)((u8*)g_modelScreenPos + 6 + off);
        D_80163C74 = (DR_MODE*)func_800C2704(
            (u_long*)((u8*)g_cDb + 0x4084), px + 3, py - 0xE, 0, 0xD0, 0x30,
            0x10, 0);
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C2928);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C2C1C);

// render arg0 as four base-10 digit glyph ids (0x98 = '0') into arg1[0..3],
// most significant first, then strip the leading zeroes and report how many
// digits are left. `j` must be its own local: it is what makes the 3 in
// `3 - i` the first loop-invariant move_movables records, which is the order
// the target hoists the two constants in.
s32 func_800C2F20(s32 arg0, s16* arg1) {
    s32 i;
    s16 n;
    s32 t;
    s32 q;
    s32 j;

    n = arg0;
    for (i = 0; i < 4; i++) {
        j = 3 - i;
        t = n;
        q = t / 10;
        n = q;
        arg1[j] = (t - q * 10) * 8 + 0x98;
    }
    for (i = 0; i < 3; i++) {
        if (arg1[i] != 0x98) {
            return (u8)(4 - i);
        }
        arg1[i] = 0;
    }
    return 1;
}

// queue sound command 0x2A, same pan derivation as func_800BBA84
void func_800C2FD4(s32 arg0, s16 arg1, s32 arg2) {
    if ((u8)arg2 != 0) {
        D_8009A000[0] = 0x2A;
        D_8009A004[0] = (g_modelScreenPos[(u8)arg0].prevX / 5 * 2) & 0x7E;
        D_8009A008[0] = arg1;
        SystemAkaoExecute();
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3068);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C328C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C33F0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3578);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C36B4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3950);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3AA0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3CA8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3DE4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C3F44);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C40F4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C428C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C44B4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C45EC);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C4814);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C494C);

static void func_800C4B60(s16 arg0) {
    if (D_801621F0[arg0].D_801621F4 == 0) {
        D_801621F0[arg0].D_801621F0 = -1;
        return;
    }
    D_80163C74 = func_800C4DC8(0, D_801621F0[arg0].unkA, 320, 47, &D_800EA25C);
    D_80163C74 =
        func_800C4DC8(0, D_801621F0[arg0].unkA + 47, 320, 32, &D_800EA258);
    D_80163C74 = func_800C4DC8(0, D_801621F0[arg0].unk8, 320, 32, &D_800EA260);
    D_80163C74 =
        func_800C4DC8(0, D_801621F0[arg0].unk8 + 32, 320, 47, &D_800EA25C);
    D_801621F0[arg0].unk8 += 4;
    D_801621F0[arg0].unkA -= 4;
    D_801621F0[arg0].D_801621F4--;
}

static void func_800C4D10(void) {
    int arg0;

    arg0 = D_801590D4;
    switch (D_801621F0[arg0].D_801621F2) {
    case 0:
        D_801621F0[arg0].D_801621F4 = 21;
        D_801621F0[arg0].unk8 = 87;
        D_801621F0[arg0].unkA = 8;
        D_801621F0[arg0].D_801621F2++;
    case 1:
        func_800C4B60(arg0);
        break;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C4DC8);

static u_long* func_800C5040(u8 r, u8 g, u8 b, s32 tpage, u_long* ot);

u_long* func_800C4FC8(u8 r, u8 g, u8 b) {
    return func_800C5040(r, g, b, 1, (u_long*)&g_cDb->unk4080[1]);
}

u_long* func_800C5004(u8 r, u8 g, u8 b) {
    return func_800C5040(r, g, b, 2, (u_long*)&g_cDb->unk40EC);
}

static u_long* func_800C5040(u8 r, u8 g, u8 b, s32 tpage, u_long* ot) {
    DR_MODE* drMode;
    POLY_F4* poly;

    drMode = D_80163C74;
    SetDrawMode(drMode, 1, 0, (tpage & 3) << 5, NULL);
    poly = (POLY_F4*)(drMode + 24);
    SetPolyF4(poly);
    SetSemiTrans(poly, 1);
    poly->r0 = r;
    poly->g0 = g;
    poly->b0 = b;
    poly->x0 = 0;
    poly->y0 = 8;
    poly->x1 = 320;
    poly->y1 = 8;
    poly->x2 = 0;
    poly->y2 = 166;
    poly->x3 = 320;
    poly->y3 = 166;
    addPrim(ot, poly);
    addPrim(ot, drMode);
    return (u_long*)(poly + 1);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C5170);

void func_800C55B8(void);
static void func_800C5468(u8 arg0) {
    s32 var_v0_2;
    s32 var_v0;
    u16 temp_a1;

    var_v0 = arg0;
    if (D_80151200[var_v0].D_80151200 & 0x1000) {
        temp_a1 = D_80151200[var_v0].D_8015120C;
        if (!(temp_a1 & 0x80)) {
            D_80151200[var_v0].D_8015120C |= 0x80;
            var_v0_2 = func_800BC04C(func_800C55B8);
            D_801621F0[var_v0_2].D_801621F6 = arg0;
            D_801621F0[var_v0_2].D_801621F4 = 0x10;
            D_801621F0[var_v0_2].D_801621F2 = -0x80;
        }
    } else {
        temp_a1 = D_80151200[var_v0].D_8015120C;
        if (temp_a1 & 0x80) {
            D_80151200[var_v0].D_8015120C = temp_a1 & (~0x80);
            var_v0_2 = func_800BC04C(func_800C55B8);
            var_v0_2 = var_v0_2;
            D_801621F0[var_v0_2].D_801621F6 = arg0;
            D_801621F0[var_v0_2].D_801621F4 = 0x10;
            D_801621F0[var_v0_2].D_801621F2 = 0x80;
        }
    }
}

void func_800C55B8(void) {
    if (D_801621F0[D_801590D4].D_801621F4 == 0) {
        D_801621F0[D_801590D4].D_801621F0 = -1;
        return;
    }
    D_801518E4[D_801621F0[D_801590D4].D_801621F6].D_801518EA +=
        D_801621F0[D_801590D4].D_801621F2;
    D_801621F0[D_801590D4].D_801621F4--;
}

/* 15 rows, one instruction over 71. The extra insn is a `move a1,v1` copying
 * the D_80162978 byte offset: the target computes `D_8015169C * 32` straight
 * into the register that serves all six accesses, this body computes it into
 * a block-local and copies. The `li v0,-1` that the target puts in the guard
 * branch's delay slot lands after the model-index load here, which is reorg
 * having nothing better to steal once the copy exists.
 *
 * Measured and rejected: storing -1 before rather than after reading the
 * model index (15 either way -- statement order is inert here), and writing
 * `D_8015169C * 32` inline at all six sites instead of once (43 rows, 15
 * instructions short: cse folds it to two expansions where the target has
 * six). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle1", func_800C5694);
#else
void func_800C5694(void) {
    s32 a;
    s32 m;

    a = D_8015169C * 32;
    if (*(s16*)((u8*)D_80162978 + 4 + a) == 0) {
        *(s16*)((u8*)D_80162978 + a) = -1;
        m = *(s16*)((u8*)D_80162978 + 6 + a);
        *(u8*)((u8*)D_801518E4 + 0x26 + m * 0xB9C) = 1;
    } else {
        m = *(s16*)((u8*)D_80162978 + 6 + a) * 0xB9C;
        *(s16*)((u8*)D_801518E4 + 6 + m) =
            *(u16*)((u8*)D_801518E4 + 6 + m) + *(u16*)((u8*)D_80162978 + 2 + a);
        *(s16*)((u8*)D_80162978 + 4 + a) = *(u16*)((u8*)D_80162978 + 4 + a) - 1;
    }
}
#endif

extern BattleModelSub D_8015E1E8[];
s32 func_800B2F50(void);

// scatter the 256 particle records: random direction, a downward speed, and
// a shared model matrix. `base` has to be its own local -- without it gcc
// folds the +0x3800 into the symbol and derives the record cursor from the
// byte array instead of both from one materialised base, which is what the
// target does.
void func_800C57B0(void) {
    BattleModelSub* p;
    u8* q;
    u8* base;
    s32 i;

    base = (u8*)D_8015E1E8;
    q = base + 0x3800;
    p = (BattleModelSub*)base;
    for (i = 0; i < 0x100; i++) {
        p->sv2.vx = (func_800B2F50() & 0x3FFF) - 0x2000;
        p->sv2.vy = -0x2710 - func_800B2F50() / 2;
        p->sv2.vz = (func_800B2F50() & 0x3FFF) - 0x2000;
        p->sv1.vx = 0;
        p->sv1.vy = 0;
        p->sv1.vz = 0;
        p->pm = (MATRIX*)&D_800FA63C;
        p++;
        q[0] = 0;
        q[1] = 0;
        q += 2;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C5864);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C59B8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C5ADC);

// reset each slot's first field to -1 (empty)
static void func_800C5BEC(void) {
    s32 fill;
    s32 i;

    fill = -1;
    for (i = 0x17A; i >= 0; i -= 6) {
        *(s16*)&D_800F9DA8[i] = fill;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C5C18);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C5CC0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C5E94);

s32 func_800C60F4(void) { return Savemap.battle_msg_speed / 4 + 4; }

static void func_800C610C(void) {
    while (D_801518DC) {
        func_800B7FB4();
    }
}

static void func_800C614C(u_long* pTim, s32 palIndex) {
    TIM_IMAGE tim;
    u32* dst;
    s32 i;

    palIndex &= 0xFF;
    dst = D_800F8CF4[palIndex];
    OpenTIM(pTim);
    ReadTIM(&tim);
    for (i = 0; i < 0x18; i++) {
        *dst++ = *tim.caddr++;
    }
}

extern RECT D_800F4B1C;

// stage the 16x3 strip at VRAM y=480 for each combatant whose setup type is
// SETUP_SIDE_ATTACK_2. `.x` has to be assigned first: the base register the
// four stores share is materialised at the first field referenced, and the
// call needs it at offset 0.
void func_800C61C0(void) {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (D_801636B8[i].D_801636B8 == 6) {
            D_800F4B1C.x = 0x10;
            D_800F4B1C.y = 0x1E0;
            D_800F4B1C.w = 0x10;
            D_800F4B1C.h = 3;
            BATTLE_EnqueueLoadImage(&D_800F4B1C, D_800F8CF4[i]);
        }
    }
}

// load an image into VRAM
static void func_800C627C(void) {
    s32 i;

    for (i = 0; i < 0xA; i++) {
        func_800C62F4(i & 0xFF);
    }
    D_800F4B24.x = 0;
    D_800F4B24.y = 0x1E0;
    D_800F4B24.w = 0x10;
    D_800F4B24.h = 0x1E;
    BATTLE_EnqueueLoadImage(&D_800F4B24, D_80158D0C);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C62F4);

void func_800C679C(void);

void func_800C64AC(void) { func_800BBEAC(func_800C679C); }

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C64D4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C6628);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C679C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C6CB8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C70AC);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C7220);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C7340);

static void func_800C74A4(void) {
    if (!(D_801518E4[3].D_80151909 & 2)) {
        func_800C7C4C(3, D_800F57D0->unk8, D_800F57D0 + 1, D_800F57D0);
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C74E4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C76C8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle1", func_800C7924);
