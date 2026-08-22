//! PSYQ=3.3 CC1=2.6.3
#include "battle_private.h"

static void func_800D67BC(s32 arg0);
static void func_800D67E8(s32 arg0);
void func_800D6814(s32 arg0);

extern Yamada D_800EEBB8[]; // MAGIC/*.BIN overlay
extern s16 D_800EF5B0[];
extern s16 D_800EF63C[];
extern s16 D_800EF6A8[];
extern s16 D_800EF6D8[];
extern s16 D_800EF6FC[];
extern s16 D_800EF838[];
extern s16 D_800EF8D8[];

s32 func_800BBF7C(void (*f)(void));
s32 func_800BC04C(void (*f)());
void func_800C2928();
void func_800C328C();
void func_800C3578();
void func_800C3950();
void func_800C3CA8();
void func_800C40F4();
void func_800C44B4();
void func_800C4814();
void func_800CFB14();
void func_800D1530();
s32 func_800D376C(BattleModelSub* arg0, s32 arg1, s16 nItems, u8* arg3);
void func_800D3AF0();
static void func_800D4D4C(s32 arg0, s32 arg1);

// MAGIC/ entrypoints
void func_801B037C(s16, u8);
void func_801B0000(s16, u8);
void func_801B0000_2(s16, u8);
void func_801B000C(s16, u8);
void func_801B0040(s16, u8);
void func_801B0054(s16, u8);
void func_801B0084(s16, u8);

static s32 func_800C7B60(s16 arg0, s16 nItems, u8* arg2) {
    BattleModelSub* var_a0;
    s32 var_a1;
    s32 temp_s0;
    s32 temp_v0;

    var_a0 = D_801518E4[arg0].D_80151A58;
    var_a1 = D_801518E4[arg0].D_80151958;
    D_801518E4[arg0].D_80151958 = func_800D376C(var_a0, var_a1, nItems, arg2);
    return D_801518E4[arg0].D_80151958 == 0;
}

static void func_800C7BE8(s16 arg0, s16 arg1, u8* arg2) {
    D_800FA6D8[arg0].unk0 =
        func_800D376C(D_800FA6D8[arg0].unk8, D_800FA6D8[arg0].unk0, arg1, arg2);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800C7C4C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800CD400);

s16 func_800CD558(s16 arg0, u8* arg1) {
    u32 val;
    val = arg1[D_801518E4[arg0].D_80151920++];
    return (arg1[D_801518E4[arg0].D_80151920++] << 8) + val;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800CD5E4);

static void func_800CD798(u8 arg0) {
    D_801518E4[arg0].D_80151A4C.vx = D_80163C80[arg0].vx;
    D_801518E4[arg0].D_80151A4C.vy = D_80163C80[arg0].vy;
    D_801518E4[arg0].D_80151A4C.vz = D_80163C80[arg0].vz;
}

static void func_800CD82C(void) {
    D_801590DC = 1;
    D_801621F0[D_801590D4].D_801621F0 = -1;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800CD860);

static void func_800CDD44(s16 arg0) {
    D_801518E4[arg0].D_8015191F = 1;
    D_801518E4[arg0].D_80151920 = 0;
    D_801518E4[arg0].D_80151921 = 0;
}

void func_800CDDA4(void) {
    if (D_80166F68) {
        D_800F9DA4 = D_800F9DA4 | 1;
    } else {
        D_800F9DA4 = D_800F9DA4 & ~1;
    }
}

static void func_800CDDE8(void) {
    func_800BBA84(D_801621F0[D_801590D4].D_801621F4, D_801590CC, 0);
    func_800D7178(D_801590CC, D_801621F0[D_801590D4].D_801621F2);
    D_801621F0[D_801590D4].D_801621F0 = -1;
}

static void func_800CDE78(void) {
    if (g_modelScreenPos[D_801590CC].prevX <
        g_modelScreenPos[D_801621F0[D_801590D4].unk8].prevX) {
        D_801621F0[D_801590D4].unk14 |= 0x100;
    }
    func_800D4D4C(D_801621F0[D_801590D4].unk8, D_801621F0[D_801590D4].unk14);
    D_801621F0[D_801590D4].D_801621F0 = -1;
}

void func_800CDF6C(s32 arg0, s16 arg1) {
    func_800BBA84(0x14, arg1, 0);
    func_800D58D0(arg0, 1, 1);
    func_800D4D4C(D_800FA6D4, 0x2C);
}

static void func_800CDFC4(void) {
    if (!D_801621F0[D_801590D4].D_801621F4) {
        D_80163C74 = (DR_MODE*)func_800C4FC8(0xFA, 0xFA, 0xFA);
        D_801621F0[D_801590D4].D_801621F0 = -1;
        return;
    }
    D_801621F0[D_801590D4].D_801621F4--;
}

static void func_800CE058(s16 arg0) {
    Unk80151200* dst = &D_80151200[arg0];
    dst->D_8015123C = 0x1000;
    dst->D_8015123A = 0x1000;
    D_80151200[arg0].D_80151238 = 0x1000;
    D_80151200[arg0].D_8015120C |= 0x20;
}

static void func_800CE0C8(s16 arg0, u8 arg1, u8 arg2) {
    s32 ret;

    func_800CE058(arg0);
    switch (arg1) {
    case 0:
        ret = func_800BBEAC(func_800C3578);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg2;
        break;
    case 8:
        ret = func_800BBEAC(func_800C4814);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg2;
        break;
    case 1:
    case 10:
        ret = func_800BBF7C(func_800C3950);
        D_801620AC[ret].D_801621B2 = arg2;
        D_801620AC[ret].D_801621B4 = arg0;
        D_801620AC[ret].D_801621B6 = 0xF8;
        return;
    case 2:
        ret = func_800BBEAC(func_800C3CA8);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg2;
        break;
    case 3:
        ret = func_800BBEAC(func_800C328C);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg2;
        break;
    case 17:
    case 18:
        ret = func_800BBEAC(func_800C40F4);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg2;
        break;
    case 7:
        ret = func_800BBEAC(func_800C44B4);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg2;
        break;
    }
}

static void func_800CE21C(s16 arg0, u8 arg1) {
    s32 ret;

    func_800CE058(arg0);
    switch (D_801636B8[arg0].D_801636BC) {
    case 0:
        ret = func_800BBEAC(func_800C3578);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg1;
        break;
    case 8:
        ret = func_800BBEAC(func_800C4814);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg1;
        break;
    case 1:
    case 10:
        ret = func_800BBF7C(func_800C3950);
        D_801620AC[ret].D_801621B2 = arg1;
        D_801620AC[ret].D_801621B4 = arg0;
        D_801620AC[ret].D_801621B6 = 0xF8;
        break;
    case 2:
        ret = func_800BBEAC(func_800C3CA8);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg1;
        break;
    case 3:
        ret = func_800BBEAC(func_800C328C);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg1;
        break;
    case 17:
    case 18:
        ret = func_800BBEAC(func_800C40F4);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg1;
        break;
    case 7:
        ret = func_800BBEAC(func_800C44B4);
        D_80162978[ret].D_80162980 = arg0;
        D_80162978[ret].D_8016297E = arg1;
        break;
    }
}

static void func_800CE384(void) {
    u8* ptr;
    u8 do_work;
    u32 param;
    u32 param_hi;

    if (D_80151200[D_801621F0[D_801590D4].D_801621F6].D_80151234 !=
        D_801621F0[D_801590D4].D_801621F2) {
        D_801621F0[D_801590D4].D_801621F0 = -1;
        return;
    }
    if (D_801621F0[D_801590D4].D_801621F4) {
        D_801621F0[D_801590D4].D_801621F4--;
        return;
    }
    do_work = 1;
    while (do_work) {
        ptr = D_801621F0[D_801590D4].unk10.ptr;
        switch (ptr[D_801621F0[D_801590D4].unk18++]) {
        case 0xFD:
            param = ptr[D_801621F0[D_801590D4].unk18++];
            param_hi = ptr[D_801621F0[D_801590D4].unk18++];
            param_hi <<= 8;
            param |= param_hi;
            D_801621F0[D_801590D4].D_801621F4 =
                ptr[D_801621F0[D_801590D4].unk18++];
            D_80151200[D_801621F0[D_801590D4].D_801621F6].D_8015122E = param;
            do_work = 0;
            break;
        case 0xFE:
            D_801621F0[D_801590D4].unk18 = 0;
            break;
        case 0xFF:
            D_801621F0[D_801590D4].D_801621F0 = -1;
            do_work = 0;
            break;
        default:
            do_work = 0;
            break;
        }
    }
}

static void func_800CE638(void) {
    switch (D_801621F0[D_801590D4].D_801621F2) {
    case 0:
        D_801621F0[D_801590D4].D_801621F4 = 3;
        D_801621F0[D_801590D4].D_801621F2++;
    case 1:
        if (D_801621F0[D_801590D4].D_801621F4 == 0) {
            D_801621F0[D_801590D4].D_801621F0 = -1;
            func_800A3534(
                D_801621F0[D_801590D4].unkA, D_801621F0[D_801590D4].unk8);
        }
        D_801621F0[D_801590D4].D_801621F4--;
        break;
    }
}

static void func_800CE75C(void) {
    if ((D_800FA69C >> D_80162978[D_8015169C].D_80162980) & 1) {
        func_800D67E8(D_80162978[D_8015169C].D_80162980);
    } else if ((D_80163608 >> D_80162978[D_8015169C].D_80162980) & 1) {
        func_800D67BC(D_80162978[D_8015169C].D_80162980);
    }
}

static void func_800CE7E0(void) {
    s32 dst;

    if (!D_80162978[D_8015169C].D_8016297C) {
        if (D_80162978[D_8015169C].D_8016297E != -1) {
            func_800CE75C();
            dst = func_800BC04C(func_800C2928);
            D_801621F0[dst].unk14 = D_80162978[D_8015169C].unkA;
            D_801621F0[dst].unkE = D_80162978[D_8015169C].D_80162982;
            D_801621F0[dst].unk10.ptr =
                (u8*)(u32)D_80162978[D_8015169C].D_80162980;
            dst = func_800BC04C(func_800CE638);
            D_801621F0[dst].unkA = D_80162978[D_8015169C].unk15;
            D_801621F0[dst].unk8 = D_80162978[D_8015169C].D_8016297E;
        }
        D_80162978[D_8015169C].D_80162978 = -1;
        return;
    } else {
        D_80162978[D_8015169C].D_8016297C--;
    }
}

void func_800CEB48(void);
void func_800CE970(void) {
    s32 dst;

    if (!D_80162978[D_8015169C].D_8016297C) {
        if (D_80162978[D_8015169C].unkA & 2) {
            D_80163C74 = (DR_MODE*)func_800C4FC8(0xFA, 0xFA, 0xFA);
        }
        if (D_80162978[D_8015169C].D_8016297E != -1 &&
            D_80162978[D_8015169C].unk14 != 1) {
            func_800CE75C();
            dst = func_800BC04C(func_800C2928);
            D_801621F0[dst].unk14 = D_80162978[D_8015169C].unkA;
            D_801621F0[dst].unkE = D_80162978[D_8015169C].D_80162982;
            D_801621F0[dst].unk10.ptr =
                (u8*)(u32)D_80162978[D_8015169C].D_80162980;
            dst = func_800BC04C(func_800CE638);
            D_801621F0[dst].unkA = D_80162978[D_8015169C].unk15;
            D_801621F0[dst].unk8 = D_80162978[D_8015169C].D_8016297E;
        }
        func_800CEB48();
        return;
    } else {
        D_80162978[D_8015169C].D_8016297C--;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800CEB48);

static void func_800CF2F0(void) {
    s16 index;

    if (D_801620AC[D_801590D0].D_801621B0 == 0) {
        D_801620AC[D_801590D0].D_801621AC = -1;
        return;
    }
    index = D_801620AC[D_801590D0].D_801621B4;
    D_801518E4[index].D_80151A4C.vy += D_801620AC[D_801590D0].D_801621B6;
    *(s32*)0x1F80000C = index;
    D_801620AC[D_801590D0].D_801621B0 = D_801620AC[D_801590D0].D_801621B0 - 1;
}

static void func_800CF3CC(void) {
    s16 index;

    index = D_801620AC[D_801590D0].D_801621B4;
    *(s32*)0x1F80000C = index;
    if (D_801620AC[D_801590D0].D_801621B0 == 0) {
        D_801620AC[D_801590D0].D_801621AC = -1;
        return;
    }
    D_801518E4[index].unk160.vy += D_801620AC[D_801590D0].unkA;
    D_801620AC[D_801590D0].D_801621B0--;
}

static void func_800CF4A8(void) {
    s16 index;

    if (D_801620AC[D_801590D0].D_801621B0 == 0) {
        D_801620AC[D_801590D0].D_801621AC = -1;
        return;
    }
    index = D_801620AC[D_801590D0].D_801621B4;
    D_801518E4[index].D_80151A4C.vx += D_801620AC[D_801590D0].D_801621B6;
    D_801518E4[index].D_80151A4C.vz += D_801620AC[D_801590D0].unk8;
    *(s32*)0x1F80000C = index;
    D_801620AC[D_801590D0].D_801621B0 = D_801620AC[D_801590D0].D_801621B0 - 1;
}

void func_800CF5BC(void) {
#define MUL(a, b) (((a) * (b)) >> 12)
#define IDX1 *(s32*)0x1F80000C
#define IDX2 *(s32*)0x1F800008
    IDX1 = D_801620AC[D_801590D0].D_801621B4;
    IDX2 = D_801620AC[D_801590D0].D_801621B6;
    if (D_801620AC[D_801590D0].D_801621AE == 0) {
        D_801620AC[D_801590D0].D_801621AE = 1;
        if (IDX1 >= 4) {
            if (D_801031F0 == 0) {
                D_801620AC[D_801590D0].D_801621B2 =
                    (MUL(D_801518E4[IDX2].D_80151A4C.vy,
                         D_801518E4[IDX2].D_801518EA) -
                     MUL(D_801518E4[IDX1].D_80151A4C.vy,
                         D_801518E4[IDX1].D_801518EA)) /
                    D_801620AC[D_801590D0].D_801621B0;
            } else {
                D_801620AC[D_801590D0].D_801621B2 = 0;
            }
        } else {
            D_801620AC[D_801590D0].D_801621B2 = 0;
        }
    }
    if (!D_801620AC[D_801590D0].unk14) {
        D_80166F58 = 0;
        if (D_801620AC[D_801590D0].D_801621B0 == 0) {
            D_801620AC[D_801590D0].D_801621AC = -1;
            return;
        }
        D_801518E4[IDX1].D_80151A4C.vx += D_801620AC[D_801590D0].unk8;
        D_801518E4[IDX1].D_80151A4C.vz += D_801620AC[D_801590D0].unkA;
        D_801518E4[IDX1].D_80151A4C.vy += D_801620AC[D_801590D0].D_801621B2;
        D_801620AC[D_801590D0].D_801621B0--;
        return;
    }
    D_801620AC[D_801590D0].unk14--;
#undef IDX2
#undef IDX1
#undef MUL
}

void func_800CF8C0(s16 arg0, s16 arg1, u8 arg2) {
    u8 dst;

    dst = func_800BBF7C(func_800CFB14);
    D_801620AC[dst].D_801621B4 = arg0;
    D_801620AC[dst].D_801621B0 = arg1;
    D_801620AC[dst].D_801621B6 = D_800F99E8;
    D_801620AC[dst].unk14 = arg2;
    if (D_800F99E8 == arg0) {
        D_801620AC[dst].unk8 = 0;
        D_801620AC[dst].unkA = 0;
        D_801620AC[dst].D_801621B2 = 0;
        return;
    }
    D_801620AC[dst].unk8 =
        (D_80163C80[arg0].vx - D_801518E4[arg0].D_80151A4C.vx) / arg1;
    D_801620AC[dst].unkA =
        (D_80163C80[arg0].vz - D_801518E4[arg0].D_80151A4C.vz) / arg1;
    D_801620AC[dst].D_801621B2 =
        (D_80163C80[arg0].vy - D_801518E4[arg0].D_80151A4C.vy) / arg1;
}

void func_800CFB14(void) {
    s16 dst;

    dst = D_801620AC[D_801590D0].D_801621B4;
    *(s32*)0x1F80000C = dst;
    *(s32*)0x1F800008 = D_801620AC[D_801590D0].D_801621B6;
    if (D_801620AC[D_801590D0].unk14 == 0) {
        if (D_801620AC[D_801590D0].D_801621B0 == 0) {
            D_801620AC[D_801590D0].D_801621AC = -1;
            return;
        }
        D_801518E4[dst].D_80151A4C.vx += D_801620AC[D_801590D0].unk8;
        D_801518E4[dst].D_80151A4C.vz += D_801620AC[D_801590D0].unkA;
        D_801518E4[dst].D_80151A4C.vy += D_801620AC[D_801590D0].D_801621B2;
        D_801620AC[D_801590D0].D_801621B0--;
        return;
    }
    D_801620AC[D_801590D0].unk14--;
}

void func_800CFCB0(void) {
    s32 temp_a3;
    s16 temp_a2;
    s32 temp_a1;

    if (D_801620AC[D_801590D0].D_801621B0 == 0) {
        D_801620AC[D_801590D0].D_801621AC = -1;
        return;
    }
    temp_a2 = D_801620AC[D_801590D0].D_801621B4;
    temp_a3 = D_801620AC[D_801590D0].D_801621B6;
    temp_a1 = D_801620AC[D_801590D0].unkC;
    *((s32*)0x1F80000C) = temp_a2;
    *((s32*)0x1F800008) = temp_a3;
    *((s32*)0x1F800010) = temp_a1;
    D_801518E4[temp_a2].D_80151A4C.vx += D_801620AC[D_801590D0].unk8;
    D_801518E4[temp_a2].D_80151A4C.vz += D_801620AC[D_801590D0].unkA;
    D_801518E4[temp_a2].D_80151A4C.vy +=
        D_800EEB28[temp_a1][D_801620AC[D_801590D0].unk14++];
    D_801620AC[D_801590D0].D_801621B0--;
}

void func_800CFE60(void) {
#define MUL(a, b) (((a) * (b)) >> 12)
#define IDX1 *(s32*)0x1F80000C
#define IDX2 *(s32*)0x1F800008
    IDX1 = D_801620AC[D_801590D0].D_801621B4;
    IDX2 = D_801620AC[D_801590D0].D_801621B6;
    switch (D_801620AC[D_801590D0].D_801621AE) {
    case 0:
        D_801620AC[D_801590D0].D_801621AE = 1;
        D_801620AC[D_801590D0].D_801621B0 = D_801620AC[D_801590D0].unk15;
        break;
    case 1:
        if (D_801620AC[D_801590D0].D_801621B0 == 0) {
            D_801620AC[D_801590D0].D_801621AE = 2;
            D_801620AC[D_801590D0].D_801621B0 = D_801620AC[D_801590D0].unk16;
            D_801620AC[D_801590D0].D_801621B2 =
                MUL(D_801518E4[IDX1].D_80151A4C.vy -
                        D_801620AC[D_801590D0].unkC,
                    D_801518E4[IDX2].D_801518EA) /
                D_801620AC[D_801590D0].unk16;
            return;
        }
        D_801518E4[IDX1].D_80151A4C.vy += D_801620AC[D_801590D0].unk10;
        D_801620AC[D_801590D0].D_801621B0--;
        break;
    case 2:
        if (D_801620AC[D_801590D0].D_801621B0 == 0) {
            D_801620AC[D_801590D0].D_801621AC = -1;
            return;
        }
        D_801518E4[IDX1].D_80151A4C.vx += D_801620AC[D_801590D0].unk8;
        D_801518E4[IDX1].D_80151A4C.vz += D_801620AC[D_801590D0].unkA;
        D_801518E4[IDX1].D_80151A4C.vy -= D_801620AC[D_801590D0].D_801621B2;
        D_801620AC[D_801590D0].D_801621B0--;
        break;
    }
#undef IDX2
#undef IDX1
#undef MUL
}

void func_800D01C0(void) {
#define MUL(a, b) (((a) * (b)) >> 12)
#define IDX1 *(s32*)0x1F80000C
#define IDX2 *(s32*)0x1F800008
    IDX2 = D_801620AC[D_801590D0].D_801621B6;
    IDX1 = D_801620AC[D_801590D0].D_801621B4;
    switch (D_801620AC[D_801590D0].D_801621AE) {
    case 0:
        D_801620AC[D_801590D0].D_801621B0 = D_801620AC[D_801590D0].unk16;
        D_801620AC[D_801590D0].D_801621B2 =
            (D_801620AC[D_801590D0].unk10 - D_801518E4[IDX1].D_80151A4C.vy) /
            D_801620AC[D_801590D0].unk16;
        D_801620AC[D_801590D0].D_801621AE = 1;
        break;
    case 1:
        if (D_801620AC[D_801590D0].D_801621B0 == 0) {
            D_801620AC[D_801590D0].D_801621B0 = D_801620AC[D_801590D0].unk15;
            D_801620AC[D_801590D0].D_801621B2 =
                MUL(D_801620AC[D_801590D0].unk10 - D_801620AC[D_801590D0].unkC,
                    D_801518E4[D_801620AC[D_801590D0].D_801621B6].D_801518EA) /
                D_801620AC[D_801590D0].unk15;
            D_801620AC[D_801590D0].D_801621AE = 2;
            return;
        }
        D_801518E4[IDX1].D_80151A4C.vx += D_801620AC[D_801590D0].unk8;
        D_801518E4[IDX1].D_80151A4C.vz += D_801620AC[D_801590D0].unkA;
        D_801518E4[IDX1].D_80151A4C.vy += D_801620AC[D_801590D0].D_801621B2;
        D_801620AC[D_801590D0].D_801621B0--;
        break;
    case 2:
        if (D_801620AC[D_801590D0].D_801621B0 == 0) {
            D_801620AC[D_801590D0].D_801621AC = -1;
            return;
        }
        D_801518E4[IDX1].D_80151A4C.vy -= D_801620AC[D_801590D0].D_801621B2;
        D_801620AC[D_801590D0].D_801621B0--;
        break;
    }
#undef IDX2
#undef IDX1
#undef MUL
}

void func_800D0578(void) {
    if (D_801621F0[D_801590D4].D_801621F4 == 0) {
        func_800D4D4C(
            D_801621F0[D_801590D4].unk8, D_801621F0[D_801590D4].D_801621F6);
        D_801621F0[D_801590D4].D_801621F0 = -1;
        return;
    }
    D_801621F0[D_801590D4].D_801621F4--;
}

void func_800D061C(void) {
    if (D_801621F0[D_801590D4].D_801621F4 == 0) {
        func_800BBA84(D_801621F0[D_801590D4].D_801621F6, D_801590CC, 0);
        D_801621F0[D_801590D4].D_801621F0 = -1;
        return;
    }
    D_801621F0[D_801590D4].D_801621F4--;
}

void func_800D06B8(void) {
    if (D_801621F0[D_801590D4].D_801621F4 == 0) {
        if (D_801518DC == 0) {
            func_800D0C80(D_801621F0[D_801590D4].D_801621F6);
            D_801621F0[D_801590D4].D_801621F0 = -1;
        }
    } else {
        D_801621F0[D_801590D4].D_801621F4--;
    }
}

void func_800D0760(void) {
    if (D_80162978[D_8015169C].D_8016297E == 0) {
        if (D_80162978[D_8015169C].D_8016297C == 0) {
            D_80162978[D_8015169C].D_80162978 = -1;
            return;
        }
        func_800DCF60(D_801518E4[D_801590CC].D_80151907,
                      D_80151200[D_801590CC].D_8015123E);
        D_80162978[D_8015169C].D_8016297C--;
        return;
    }
    D_80162978[D_8015169C].D_8016297E--;
}

void func_800D088C(s32 loc, s32 len) {
    SystemLoadFileBySector(loc, len, (u_long*)0x801B0000, NULL);
    func_800B7FB4();
}

extern u32 D_800F9984[][8];
extern u32 D_801679BC[][8];

// Rebase the eight script pointers at +0xBC and the eight at +0x18C of
// player arg0's model block: on disc they are offsets from the start of the
// table, in memory they have to be offsets from the block itself.
//
// `(i << 2)` rather than `i * 4` is load-bearing in all three functions
// below -- fold ranks a MULT_EXPR above the pointer and emits
// `addu <idx>,<base>`, where the shift ties and leaves source order, giving
// the target's `addu <base>,<idx>`.
void func_800D08B8(u8 arg0, u32* arg1) {
    s32 i;

    if (arg1 == NULL) {
        return;
    }
    for (i = 0; i < 8; i++) {
        *(u32*)(D_800F8384[arg0] + (i << 2) + 0xBC) =
            (u32)arg1 + arg1[i + 1] - (u32)D_800F8384[arg0];
    }
    for (i = 0; i < 8; i++) {
        *(u32*)(D_800F8384[arg0] + (i << 2) + 0x18C) =
            (u32)arg1 + arg1[i + 9] - (u32)D_800F8384[arg0];
    }
}

// Save those sixteen pointers away; func_800D09D0 puts them back.
void func_800D0958(u8 arg0) {
    s32 i;

    for (i = 0; i < 8; i++) {
        D_800F9984[arg0][i] = *(u32*)(D_800F8384[arg0] + (i << 2) + 0xBC);
        D_801679BC[arg0][i] = *(u32*)(D_800F8384[arg0] + (i << 2) + 0x18C);
    }
}

void func_800D09D0(u8 arg0) {
    s32 i;

    for (i = 0; i < 8; i++) {
        *(u32*)(D_800F8384[arg0] + (i << 2) + 0xBC) = D_800F9984[arg0][i];
        *(u32*)(D_800F8384[arg0] + (i << 2) + 0x18C) = D_801679BC[arg0][i];
    }
}

void func_800D0A44(void) {}

static void func_800D0AD4(void);
void func_800D0A4C(void) {
    s32 ret;
    s32 i;

    for (i = 0; i < 3; i++) {
        D_801518E4[i].D_80151909 |= 1;
    }
    func_801B0040(D_80151774, D_801590CC);
    ret = func_800BC04C(func_800D0AD4);
    *(s32*)0x1F800000 = ret;
    D_801621F0[ret].D_801621F4 = 2;
}

static void func_800D0AD4(void) {
    if (!D_801621F0[D_801590D4].D_801621F4) {
        D_801621F0[D_801590D4].D_801621F0 = -1;
        func_800BB978();
        return;
    }
    D_801621F0[D_801590D4].D_801621F4--;
}

void func_800D0B4C(u8 arg0) {
    D_800F8CF0 = 0;
    func_800D1530();
    switch (D_801518E4[arg0].D_80151907) {
    case 4:
        D_800EF9D8[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
        break;
    case 7:
        func_801B037C(D_80151774, D_801590CC);
        break;
    case 8:
        D_800EFFE0[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
        break;
    }
}

void func_800D0C80(u8 arg0) {
    D_800F8CF0 = 0;
    func_800D1530();
    switch (D_801518E4[arg0].D_80151907) {
    case 2:
        if (D_801031F0 == 0) {
            if (D_801518E4[arg0].D_80151906 == 25) {
                D_801518E4[0].D_8015190A = 1;
                D_801518E4[1].D_8015190A = 1;
                D_801518E4[2].D_8015190A = 1;
            }
            D_800EFAF0[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
            return;
        }
        switch (D_801518E4[arg0].D_80151906) {
        case 41:
            func_801B0000(D_80151774, D_801590CC);
            break;
        case 44:
            func_801B0000_2(D_80151774, D_801590CC);
            break;
        case 35:
            func_801B000C(D_80151774, D_801590CC);
            break;
        case 32:
            func_801B0054(D_80151774, D_801590CC);
            break;
        case 29:
            func_801B0084(D_80151774, D_801590CC);
            break;
        default:
            D_800EFAF0[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
            break;
        }
        break;
    case 13:
        D_800EFBC8[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
        break;
    case 20:
        if (D_801518E4[arg0].D_80151906 == 2) {
            if (D_801590CC == D_800FA9E8) {
                D_80163A98 = 0;
            } else {
                D_80163A98 = 1;
            }
        }
        *(s32*)0x1F800000 =
            D_800EFEA0[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
        switch (D_801518E4[arg0].D_80151906) {
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x38:
        case 0x39:
        case 0x3A:
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4F:
            *(s32*)0x1F800000 = 0;
            break;
        }
        func_800D08B8(arg0, *(s32*)0x1F800000);
        break;
    case 32:
        D_800EFC28[D_801518E4[arg0].D_80151906](D_80151774, D_801590CC);
        break;
    case 3:
        func_800C64AC();
        break;
    }
}

void func_800D1110(u8 arg0) {
    s32 lba;
    s32 var_a1;
    s32 id;

    switch (D_801518E4[arg0].D_80151907) {
    case 2:
        if (D_801031F0 == 0) {
            id = D_800EF63C[D_801518E4[arg0].D_80151906];
            func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        } else {
            switch (D_801518E4[arg0].D_80151906) {
            case 29:
                func_800D088C(D_800EEBB8[77].loc, D_800EEBB8[77].len);
                break;
            case 41:
                func_800D088C(D_800EEBB8[32].loc, D_800EEBB8[32].len);
                break;
            case 44:
                func_800D088C(D_800EEBB8[26].loc, D_800EEBB8[26].len);
                break;
            case 32:
                func_800D088C(D_800EEBB8[255].loc, D_800EEBB8[255].len);
                break;
            case 35:
                func_800D088C(D_800EEBB8[6].loc, D_800EEBB8[6].len);
                break;
            default:
                id = D_800EF63C[D_801518E4[arg0].D_80151906];
                lba = D_800EEBB8[id].loc;
                var_a1 = D_800EEBB8[id].len;
                func_800D088C(lba, var_a1);
                break;
            }
        }
        break;
    case 7:
        func_800D088C(D_800EEBB8[221].loc, D_800EEBB8[221].len);
        break;
    case 8:
        id = D_800EF8D8[D_801518E4[arg0].D_80151906];
        func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        break;
    case 13:
        id = D_800EF6A8[D_801518E4[arg0].D_80151906];
        func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        break;
    case 20:
        id = D_800EF838[D_801518E4[arg0].D_80151906];
        func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        break;
    case 4:
        id = D_800EF5B0[D_801518E4[arg0].D_80151906];
        func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        break;
    case 32:
        id = D_800EF6FC[D_801518E4[arg0].D_80151906];
        func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        break;
    case 3:
        id = D_800EF6D8[D_801518E4[arg0].D_80151906];
        func_800D088C(D_800EEBB8[id].loc, D_800EEBB8[id].len);
        break;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D1530);

void BATTLE_EnqueueLoadImage(RECT* rect, u_long* ptr) {
    D_800F01DC->method = QUEUE_LOAD_IMAGE;
    D_800F01DC->rect = rect;
    D_800F01DC->ptr = ptr;
    D_800F01DC++;
}

void BATTLE_EnqueueStoreImage(RECT* rect, u_long* ptr) {
    D_800F01DC->method = QUEUE_STORE_IMAGE;
    D_800F01DC->rect = rect;
    D_800F01DC->ptr = ptr;
    D_800F01DC++;
}

void BATTLE_EnqueueMoveImage(RECT* rect, s32 x, s32 y) {
    D_800F01DC->method = QUEUE_MOVE_IMAGE;
    D_800F01DC->rect = rect;
    D_800F01DC->x = x;
    D_800F01DC->y = y;
    D_800F01DC++;
}

void BATTLE_EnqueueClearImage(RECT* rect) {
    D_800F01DC->method = QUEUE_CLEAR_IMAGE;
    D_800F01DC->rect = rect;
    D_800F01DC++;
}

void BATTLE_FlushImageQueue(void) {
    Unk800F01DC* item;

    for (item = D_800F4BAC; item < D_800F01DC; item++) {
        switch (item->method) {
        case QUEUE_LOAD_IMAGE:
            LoadImage(item->rect, item->ptr);
            break;
        case QUEUE_STORE_IMAGE:
            StoreImage(item->rect, item->ptr);
            break;
        case QUEUE_MOVE_IMAGE:
            MoveImage(item->rect, item->x, item->y);
            break;
        case QUEUE_CLEAR_IMAGE:
            ClearImage(item->rect, 0, 0, 0);
            break;
        }
    }
    D_800F01DC = D_800F4BAC;
}

void BATTLE_ResetImageQueue(void) { D_800F01DC = D_800F4BAC; }

void func_800D2710(u_long* addr, s16 x, s16 y) {
    TIM_IMAGE tim;

    OpenTIM(addr);
    ReadTIM(&tim);
    if (tim.crect && tim.caddr) {
        D_800F4B2C[D_800F01E0] = *tim.crect;
        D_800F4B2C[D_800F01E0].x += x & ~15;
        D_800F4B2C[D_800F01E0].y =
            y + D_800F4B2C[D_800F01E0].y; // requires GCC 2.6.3
        BATTLE_EnqueueLoadImage(&D_800F4B2C[D_800F01E0], tim.caddr);
        D_800F01E0 = (D_800F01E0 + 1) & 7;
    }
}

void func_800D2828(u_long* addr, s32 xy) {
    TIM_IMAGE tim;
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_a2;

    OpenTIM(addr);
    ReadTIM(&tim);
    if (tim.prect && tim.paddr) {
        D_800F4B6C[D_800F01E4] = *tim.prect;
        temp_a1 = (tim.prect->y & 0x300) >> 4 | (tim.prect->x & 0x3FF) >> 6;
        temp_a2 = temp_a1 + xy;
        temp_a3 = (temp_a1 & 0x0F) * 0x40;
        D_800F4B6C[D_800F01E4].x =
            ((temp_a2 & 0x0F) * 0x40 + (D_800F4B6C[D_800F01E4].x - temp_a3)) &
            0x3FF;
        temp_a3 = (temp_a1 & 0x30) * 0x10;
        D_800F4B6C[D_800F01E4].y =
            ((temp_a2 & 0x30) * 0x10 + (D_800F4B6C[D_800F01E4].y - temp_a3)) &
            0x1FF;
        BATTLE_EnqueueLoadImage(&D_800F4B6C[D_800F01E4], tim.paddr);
        D_800F01E4 = (D_800F01E4 + 1) & 7;
    }
}

void func_800D2980(u_long* addr, s16 imgXY, s16 clutX, s16 clutY) {
    func_800D2710(addr, clutX, clutY);
    func_800D2828(addr, imgXY);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D29D4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D32B4);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D3354);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D3418);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D3474);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D34C8);

/* Advance a counter that lives 4 bytes past a self-relative header: the word
 * at arg0 is the byte offset from &arg0[1] to the record.
 *
 * The body below is byte-exact for all nine of the target's instructions.
 * It is parked on a tenth "instruction" that is not code: `func_800D3520.s`
 * carries a `nop` *after* its own `.size` directive, i.e. four bytes of
 * object padding, and it is the only `.s` in this unit that does. Functions
 * in this overlay are not 8-byte aligned in general (func_800D7B1C sits at
 * ...B1C), so this is an original translation-unit boundary rather than
 * function alignment -- battle2.c is several of the original `.c` files glued
 * together, and the pad belongs to the end of one of those objects. Nothing
 * written inside a merged unit emits it: landing the body compiles 36 bytes
 * where the target has 40, every later symbol in battle.elf shifts, and what
 * you see is `batini.c: undefined reference to D_800F7ED0` -- an overlay this
 * file does not touch. Verified by a red `make build`.
 *
 * Three levers in the body are real and each was measured; keep them if the
 * padding is ever solved (a `.align 3` on the *next* function's INCLUDE_ASM
 * would do it, but the macro hard-codes `.align 2`):
 *   * the three locals keep `addiu a0,a0,4` on the pointer -- written inline
 *     as `(s16*)((u8*)(arg0 + 1) + *arg0)`, fold associates the +4 onto the
 *     loaded value (`addiu v0,v0,4`) instead, 7 rows. `(s32)(arg0 + 1)` is
 *     the same 7.
 *   * `s32 v` rather than `s16 v` keeps the sign-extending `lh`: with an s16
 *     local combine narrows to `lhu`, since the sign is dead into a narrowing
 *     store. 5 rows.
 *   * the `volatile` on the store is what leaves the `jr ra` delay slot
 *     empty. Without it the RTL is byte-identical through `.sched2` and reorg
 *     pulls the `sh` into the slot at `.dbr` (confirmed with `--rtl=a`),
 *     3 rows and one instruction short.
 * Also measured: `void` return, 4 rows -- the sum lands in `a1` not `v0`;
 * `void` plus reusing `v` for the sum, 4; `return q[1] = arg1 + v;`, 3 rows
 * and +1 instruction from a `sll`/`sra` pair re-widening the s16 value; a
 * `volatile s16*` local instead of the cast, byte-identical to the below.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/battle2", func_800D3520);
#else
s32 func_800D3520(s32* arg0, s32 arg1) {
    u8* p = (u8*)(arg0 + 1);
    s32 off = *arg0;
    s16* q = (s16*)(p + off);
    s32 v = q[1];
    s32 t = arg1 + v;

    *(volatile s16*)&q[1] = t;
    return t;
}
#endif

// Read one run-length token out of the bitstream at *pos: a flag bit, then
// either 16 bits (17 consumed) or 7 bits (8 consumed), sign-extended.
//
// `mask` has to be a named local. Written inline, combine rewrites
// `(w & (1 << n)) != 0` into `(w >> n) & 1` -- a `srav`/`andi` pair where the
// target has `li 1`/`sllv`/`and` -- and inverts the branch with it.
s32 func_800D3548(u8* buf, s32* pos) {
    s32 bitpos = *pos;
    u8* p = buf + bitpos / 8;
    s32 sh = bitpos & 7;
    s32 w = (p[0] << 8) | p[1];
    s32 mask = 1 << (0xF - sh);

    if ((w & mask) == 0) {
        *pos = bitpos + 8;
        return ((w << (sh + 1)) << 16) >> 25;
    }
    w = (w << 8) | p[2];
    *pos = bitpos + 0x11;
    return ((w << (sh + 1)) << 8) >> 16;
}

s32 func_800D35D8(u8* arg0, s32* arg1, s32 arg2) {
    s32 bits;
    s32 i;

    bits = 0;
    for (i = 0; i < arg2; i++) {
        bits <<= 1;
        if ((arg0[*arg1 / 8] >> (7 - (*arg1 & 7))) & 1) {
            bits++;
        }
        *arg1 = *arg1 + 1;
    }
    bits <<= 32 - arg2;
    bits >>= 32 - arg2;
    return bits;
}

// Read one variable-length signed coefficient out of the bitstream: a
// 3-bit magnitude class, then that many value bits, recentred and scaled.
// Falls out of the switch with no return on the impossible n > 7.
//
// `case 1 ... case 6` has to be spelled out rather than written `default`:
// three case ranges make `balance_case_nodes` build the tree whose root test
// is the target's `slti v0,s0,7`, where a default gives a linear compare
// chain and is 16 instructions short. And case 0's constant is `0xFFFF`, not
// `0xFFFF0000` -- combine reassociates `(0xFFFF << arg2) << 16` into the
// target's `lui 0xffff` / `sllv`, while the pre-shifted constant makes the
// `(s16)` cast provably zero and gcc folds the whole arm into `return 0`.
s32 func_800D3658(u8* arg0, s32* arg1, s32 arg2) {
    s32 n;
    s32 v;

    if (func_800D35D8(arg0, arg1, 1) == 0) {
        return 0;
    }
    n = func_800D35D8(arg0, arg1, 3) & 7;
    switch (n) {
    case 0:
        return (s16)(0xFFFF << arg2);
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        v = func_800D35D8(arg0, arg1, n);
        if (v >= 0) {
            v += 1 << (n - 1);
        } else {
            v -= 1 << (n - 1);
        }
        return (s16)(v << arg2);
    case 7:
        return (s16)(func_800D35D8(arg0, arg1, 0xC - arg2) << arg2);
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D376C);

void func_800D3994(s32 arg0, s32 arg1, void* arg2);
// func_800D3A6C below with the source matrix reached by index rather than by
// pointer: model arg0's sub-part arg1. The two multiplies are written arg1
// first because fold swaps a sum of two multiplies -- written arg0 first the
// chains come out in the other order and it is 20 rows.
void func_800D3994(s32 arg0, s32 arg1, void* arg2) {
    MATRIX sp10;
    s32 off = arg1 * sizeof(BattleModelSub) + arg0 * sizeof(BattleModel);

    ((SVECTOR*)arg2)->vx = (s16)(*(u16*)((u8*)D_801518E4 + 0x188 + off) -
                                 *(u16*)&D_800FA63C.m.t[0]);
    ((SVECTOR*)arg2)->vy = (s16)(*(u16*)((u8*)D_801518E4 + 0x18C + off) -
                                 *(u16*)&D_800FA63C.m.t[1]);
    ((SVECTOR*)arg2)->vz = (s16)(*(u16*)((u8*)D_801518E4 + 0x190 + off) -
                                 *(u16*)&D_800FA63C.m.t[2]);
    TransposeMatrix(&D_800FA63C.m, &sp10);
    ApplyMatrixSV(&sp10, (SVECTOR*)arg2, (SVECTOR*)arg2);
}

// Take the low 16 bits of each of arg0's translation components relative to the
// camera D_800FA63C, then rotate that offset by the camera's transposed
// orientation into arg1.
void func_800D3A6C(MATRIX* arg0, SVECTOR* arg1) {
    MATRIX sp10;

    arg1->vx = (s16)(*(u16*)&arg0->t[0] - *(u16*)&D_800FA63C.m.t[0]);
    arg1->vy = (s16)(*(u16*)&arg0->t[1] - *(u16*)&D_800FA63C.m.t[1]);
    arg1->vz = (s16)(*(u16*)&arg0->t[2] - *(u16*)&D_800FA63C.m.t[2]);
    TransposeMatrix(&D_800FA63C.m, &sp10);
    ApplyMatrixSV(&sp10, arg1, arg1);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D3AF0);

const s32 D_800A0D98[] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                          0x00001000, 0x00000000, 0x00000000, 0x00000000};
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D3BF0);

void func_800D3D88(void) {
    Unk801621F0* temp_s0_2;
    Unk801621F0* temp_s1;
    s32 temp_s0;
    u16 temp_s2;

    temp_s1 = &D_801621F0[D_801590D4];
    temp_s0 = temp_s1->D_801621F0;
    temp_s2 = ((u8*)&D_801518E4[temp_s0].D_8015191A)[temp_s1->D_801621F2 & 1];
    temp_s0++; // !FAKE
    temp_s0--; // !FAKE
    if (temp_s2 != 0xFF) {
        temp_s0_2 = &D_801621F0[func_800BC04C(func_800D3BF0)];
        func_800D3994(temp_s0, temp_s2, &temp_s0_2->D_801621F4);
        temp_s0_2->D_801621F6 = 0;
        temp_s0_2->unkE = temp_s1->unkE;
        temp_s0_2->unk10.unk.unk0 = temp_s1->unk10.unk.unk0;
    }
    temp_s1->D_801621F2++;
    if (temp_s1->D_801621F2 == 4) {
        temp_s1->D_801621F0 = -1;
    }
}

void func_800D3E8C(s32 arg0) {
    Unk801621F0* temp_v0;

    temp_v0 = &D_801621F0[func_800BC04C(func_800D3D88)];
    temp_v0->D_801621F0 = arg0;
    temp_v0->unkE = *(s16*)& temp_v0->unk10 = D_801518E4[arg0].D_801518EA;
}

// Claim a slot running func_800D3AF0 and seed it with a position plus two
// scalars. The four lwl/lwr + swl/swr pairs are the block move for an 8-byte
// aggregate of alignment 2, i.e. one SVECTOR assignment, not four stores.
void func_800D3F0C(SVECTOR* arg0, s32 arg1, s32 arg2) {
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D3AF0)];

    *(SVECTOR*)&p->D_801621F4 = *arg0;
    p->unkE = arg1;
    p->unk10.unk.unk0 = arg2;
}

void func_800D3F8C(void) {
    Unk801621F0* temp_s0;
    Unk801621F0* temp_s1;

    temp_s1 = &D_801621F0[D_801590D4];
    if (D_80062D98 == 0) {
        temp_s1->unkC--;
        if (temp_s1->unkC == -1) {
            temp_s0 = &D_801621F0[func_800BC04C(func_800D3AF0)];
            RotMatrixYXZ(&D_801518E4[temp_s1->unk10.unk.unk2].unk160,
                         (MATRIX*)0x1F800008);
            ApplyMatrixSV((MATRIX*)0x1F800008, (SVECTOR*)&temp_s1->D_801621F4,
                          (SVECTOR*)0x1F800000);
            temp_s0->D_801621F4 =
                D_801518E4[temp_s1->unk10.unk.unk2].D_80151A4C.vx +
                ((SVECTOR*)0x1F800000)->vx;
            temp_s0->D_801621F6 =
                D_801518E4[temp_s1->unk10.unk.unk2].D_80151A4C.vy +
                ((SVECTOR*)0x1F800000)->vy;
            temp_s0->unk8 = D_801518E4[temp_s1->unk10.unk.unk2].D_80151A4C.vz +
                            ((SVECTOR*)0x1F800000)->vz;
            temp_s0->unkE = temp_s1->unkE;
            temp_s0->unk10.unk.unk0 = temp_s1->unk10.unk.unk0;
            temp_s1->D_801621F0 = -1;
        }
    }
}

// Claim a slot running func_800D3F8C. arg4 arrives on the caller's stack and
// is loaded with `lhu`, which types it u16.
void func_800D415C(s32 arg0, SVECTOR* arg1, s32 arg2, s32 arg3, u16 arg4) {
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D3F8C)];

    p->unk10.unk.unk2 = arg0;
    *(SVECTOR*)&p->D_801621F4 = *arg1;
    p->unkC = arg2;
    p->unkE = arg3;
    p->unk10.unk.unk0 = arg4;
}

static void func_800D41FC(MATRIX* arg0, MATRIX* arg1, MATRIX* arg2) {
    arg2->t[0] = arg1->t[0] - arg0->t[0];
    arg2->t[1] = arg1->t[1] - arg0->t[1];
    arg2->t[2] = arg1->t[2] - arg0->t[2];
    TransposeMatrix(arg0, arg2);
    ApplyMatrixLV(arg2, arg2->t, arg2->t);
    MulMatrix(arg2, arg1);
}

// Project arg0 through the battle camera into arg2's translation column, then
// push it arg1/4096 of the way along its own direction.
MATRIX* func_800D4284(SVECTOR* arg0, s32 arg1, MATRIX* arg2) {
    VECTOR sp10;
    long sp20;

    SetRotMatrix(&D_800FA63C.m);
    SetTransMatrix(&D_800FA63C.m);
    RotTrans(arg0, (VECTOR*)&arg2->t[0], &sp20);
    if (arg1 != 0) {
        VectorNormal((VECTOR*)&arg2->t[0], &sp10);
        arg2->t[0] = ((arg1 * sp10.vx) >> 12) + arg2->t[0];
        arg2->t[1] = ((arg1 * sp10.vy) >> 12) + arg2->t[1];
        arg2->t[2] = ((arg1 * sp10.vz) >> 12) + arg2->t[2];
    }
    return arg2;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D4368);

static void func_800D4484(u_long* ot, u16 tpage) {
    DR_MODE* dr_mode;

    dr_mode = D_80163C74;
    SetDrawMode(dr_mode, 0, 1, tpage, NULL);
    AddPrim(ot, (void*)dr_mode);
    D_80163C74 = dr_mode + 1;
}

const s32 D_800A0DB8[] = {0x00000000, 0xFFFFF000, 0x00000000, 0x00000000};
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D44E8);

// Re-orthonormalise a 3x3 rotation in place: normalise the middle column,
// build the first from its perpendicular in the XY plane, and take the third
// as their cross product.
void func_800D461C(MATRIX* m) {
    VECTOR sp10;
    VECTOR sp20;
    VECTOR sp30;

    sp20.vx = m->m[0][1];
    sp20.vy = m->m[1][1];
    sp20.vz = m->m[2][1];
    VectorNormal(&sp20, &sp20);
    m->m[0][1] = sp20.vx;
    m->m[1][1] = sp20.vy;
    m->m[2][1] = sp20.vz;
    sp10.vx = sp20.vy;
    sp10.vy = -sp20.vx;
    sp10.vz = 0;
    VectorNormal(&sp10, &sp10);
    m->m[0][0] = sp10.vx;
    m->m[1][0] = sp10.vy;
    m->m[2][0] = sp10.vz;
    OuterProduct12(&sp10, &sp20, &sp30);
    VectorNormal(&sp30, &sp30);
    m->m[0][2] = sp30.vx;
    m->m[1][2] = sp30.vy;
    m->m[2][2] = sp30.vz;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D4710);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D491C);

void func_800D4A64();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D4A64);

static void func_800D4D6C(s32 arg0, s32 arg1, s32 arg2);
void func_800D4C08(void* arg0, s32 arg1, s32 arg2, s32 arg3);
extern u8 D_800F0F98[];

// Claim a slot running func_800D4A64. `off` is a pre-scaled byte offset, so
// the D_800F0F98 lookup keeps the assembler's $at expansion the target has --
// a scaled subscript would fold the symbol into a base register instead.
void func_800D4C08(void* arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D4A64)];
    s32 off = (arg1 & 0xFF) << 2;

    p->D_801621F0 = arg1 & 0xFF00;
    *(s32*)&p->unkC = *(s32*)(D_800F0F98 + off);
    *(SVECTOR*)&p->D_801621F4 = *(SVECTOR*)arg0;
    p->unk10.unk.unk2 = arg2;
    *(s16*)&p->unk18 = arg3;
}

void func_800D4CBC(s32 arg0, s32 arg1, s32 arg2) {
    s32 sp10;

    func_800D3994(arg0, D_801518E4[arg0].D_8015190F, &sp10);
    func_800D4C08(&sp10, arg1, arg2, -D_801518E4[arg0].unk12);
}

static void func_800D4D4C(s32 arg0, s32 arg1) {
    func_800D4CBC(arg0, arg1, 0x1000);
}

static void func_800D4D6C(s32 arg0, s32 arg1, s32 arg2) {
    func_800D4C08(arg0, arg1, 0x1000, arg2);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D4D90);

extern s32 D_800F10D8;
extern s32 D_800F4CEC[16];
extern s16 D_800F4D2C[16][10];

static s16* func_800D4FA8(s32 arg0) {
    s32 idx = D_800F10D8;
    s32 next = (idx + 1) & 0xF;

    D_800F4CEC[idx] = arg0;
    D_800F10D8 = next;
    return D_800F4D2C[idx];
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D4FF0);

void func_800D508C();
extern Unk80162978* D_800F10E0;

// The screen-fade tick: step the accumulator, release the slot when it runs
// past zero, clamp it at 0xFFFF, and push its high byte out as a flat colour.
// `cur` is named so the sum keeps source order -- inline, fold ranks the
// deref below `step` and emits `addu <acc>,<step>,<cur>`.
void func_800D508C(void) {
    Unk80162978* e = &D_80162978[D_8015169C];
    s32 step;
    s32 acc;
    s32 cur;

    if (D_80062D98 == 0) {
        step = *(s32*)&e->unk8;
        if (step != 0) {
            cur = *(s32*)&e->D_8016297C;
            acc = cur + step;
            *(s32*)&e->D_8016297C = acc;
            if (acc <= 0) {
                e->D_80162978 = -1;
                D_800F10E0 = NULL;
                return;
            }
            if (0xFFFF < acc) {
                *(s32*)&e->D_8016297C = 0xFFFF;
                *(s32*)&e->unk8 = 0;
            }
        }
    }
    D_80163C74 = (DR_MODE*)func_800C4FC8(
        *(u8*)((u8*)e + 5), *(u8*)((u8*)e + 5), *(u8*)((u8*)e + 5));
}

// Reset the fixed-point ramp: zero the accumulator (0x04) and seed the
// countdown (0x0C) so it lasts arg0 ticks.
void func_800D5138(s32 arg0) {
    if (D_800F10E0 == NULL) {
        D_800F10E0 = &D_80162978[func_800BBEAC(func_800D508C)];
    }
    *(s32*)&D_800F10E0->D_8016297C = 0;
    *(s32*)&D_800F10E0->unk8 = 0x10000 / arg0;
}

void func_800D51D4(s32 arg0);
// Re-aim the ramp at a new duration: the step is what is left to travel
// divided by the ticks remaining.
void func_800D51D4(s32 arg0) {
    if (D_800F10E0 != NULL) {
        *(s32*)&D_800F10E0->unk8 = -*(s32*)&D_800F10E0->D_8016297C / arg0;
    }
}

extern s32 D_800F10E4;
extern s16 D_800F5B74;

// Step the ramp once: accumulate (0x04 += 0x08), publish the high word, and
// free the slot when the countdown (0x0C) reaches 0.
void func_800D5230(void) {
    Unk80162978* slot = &D_80162978[D_8015169C];
    s32 v0;
    s32 v1;

    if (D_80062D98 == 0) {
        v0 = *(s32*)&slot->D_8016297C + *(s32*)&slot->D_80162980;
        *(s32*)&slot->D_8016297C = v0;
        D_800F5B74 = v0 >> 0x10;
        v1 = *(s32*)&slot->unk8 - 1;
        *(s32*)&slot->unk8 = v1;
        if (v1 == 0) {
            D_800F10E4 = 0;
            slot->D_80162978 = -1;
        }
    }
}

// Start a fixed-point ramp from the current D_800F5B74 to arg0 over arg1
// ticks, unless one is already running.
void func_800D52A0(s32 arg0, s32 arg1) {
    Unk80162978* e;
    s32 from;

    if (D_800F10E4 == 0) {
        e = &D_80162978[func_800BBEAC(func_800D5230)];
        from = D_800F5B74 << 16;
        D_800F10E4 = (s32)e;
        *(s32*)&e->unk8 = arg1;
        *(s32*)&e->D_8016297C = from;
        *(s32*)&e->D_80162980 = ((arg0 << 16) - from) / arg1;
    }
}

void func_800D5350();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D5350);

void func_800D5444(int arg0, int arg1, int arg2, void (*arg3)(int)) {
    Unk80162978* temp_v0 = &D_80162978[func_800BBEAC(func_800D5350)];
    temp_v0->D_80162978 = 0;
    temp_v0->D_8016297C = arg0;
    temp_v0->D_8016297E = arg1;
    temp_v0->D_80162980 = arg2;
    *(s32*)&temp_v0->unk8 = (s32)arg3;
}

s32 func_800D54BC(s32 arg0) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 10; i++) {
        if ((arg0 >> i) & 1) {
            count++;
        }
    }
    return count;
}

// Bounding box of the selected models' world positions, averaged into out.
// arg0 is a bitmask of model slots.
//
// `p` has to be assigned from the subscript *inside* the loop body, not
// walked in the `for` clause. Walked, `p` is a biv, the two accesses become
// givs and `combine_givs` merges them onto the later offset -- the base
// register comes out at `D_801518E4+0x16c` with displacements -4 and 0.
// Assigned from `&D_801518E4[i].m`, `p` is itself the giv, its add_val is
// `D_801518E4+0x140`, and both accesses stay plain displacements off it,
// which is what the target has. `BattleModel* b` walked with `b++`, an
// `s16*` walked and indexed `p[0x14]`, and a shared `(SVECTOR*)(p + 0x28)`
// all give the merged form and measure 3 rows.
s16* func_800D54EC(s32 arg0, s16* out) {
    s32 i;
    s32 minX;
    s32 minY;
    s32 maxX;
    s32 maxY;
    s32 v;
    u8* p;

    minX = 0x7FFF;
    minY = 0x7FFF;
    maxX = -0x8000;
    maxY = -0x8000;
    for (i = 0; i < 10; i++) {
        p = (u8*)&D_801518E4[i].m;
        if ((arg0 >> i) & 1) {
            v = *(s16*)(p + 0x28);
            if (v < minX) {
                minX = v;
            }
            if (maxX < v) {
                maxX = v;
            }
            v = *(s16*)(p + 0x2C);
            if (v < minY) {
                minY = v;
            }
            if (maxY < v) {
                maxY = v;
            }
        }
    }
    out[0] = (minX + maxX) / 2;
    out[2] = (minY + maxY) / 2;
    out[1] = 0;
    return out;
}

s32 func_800D55A4(s32 arg0) {
    return (D_801518E4[arg0].unk12 * 0x10) * D_801518E4[arg0].D_801518EA >> 0xC;
}

// Generic AKAO sound-command dispatcher: the first vararg's low 16 bits are
// the command id, which selects how many trailing u32 params get copied into
// the D_8009A004 queue before calling SystemAkaoExecute.
void func_800D55F4(s32 arg0, ...) {
    void** args = &arg0;
    u32* dst = (u32*)arg0;
    u32* src;
    s32 cmd = *(u16*)args;
    s32 count;
    s32 nExtra;

    D_8009A000[0] = cmd;
    switch (cmd & 0xFFFF) {
    case 0x21:
        nExtra = 3;
        break;
    case 0x22:
        nExtra = 4;
        break;
    case 0x23:
        nExtra = 5;
        break;
    default:
        nExtra = 2;
        break;
    }
    count = 1;
    if (count <= nExtra) {
        dst = D_8009A004;
        src = (u32*)args + 1;
        for (; count <= nExtra; count++) {
            *dst++ = *src++;
        }
    }
    // The four values below are already live in $a0..$a3 at this point; the
    // original passes nothing, and SystemAkaoExecute reads them as registers.
    SystemAkaoExecute();
}

// Project a point through the current view matrix and convert its clamped
// on-screen X (0..319) into a 0..127 stereo pan value.
s32 func_800D56A8(SVECTOR* sv) {
    s16 sxy[2];
    s32 p;
    s32 flag;

    SetRotMatrix(&D_800FA63C.m);
    SetTransMatrix(&D_800FA63C.m);
    RotTransPers(sv, (long*)sxy, (long*)&p, (long*)&flag);
    if (sxy[0] < 0) {
        sxy[0] = 0;
    } else if (sxy[0] >= 0x140) {
        sxy[0] = 0x13F;
    }
    return (sxy[0] * 128) / 320;
}

s32 func_800D574C(s32 arg0) {
    SVECTOR sv;

    func_800D54EC(arg0, &sv);
    return func_800D56A8(&sv);
}

// Queue a popup carrying bit index arg0, using push type 6 if that bit is
// set in the D_800F836C flag word, else type 4.
void func_800D5774(u32 arg0) {
    s32 cond;
    s16* ptr;

    cond = (D_800F836C >> arg0) & 1;
    if (cond) {
        ptr = func_800D4FA8(6);
    } else {
        ptr = func_800D4FA8(4);
    }
    *ptr = arg0;
}

void func_800D57C0();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D57C0);

void func_800D58D0(s16 arg0, s16 arg1, s16 arg2) {
    Unk80162978* temp_v0 = &D_80162978[func_800BBEAC(func_800D57C0)];
    temp_v0->D_80162978 = 0;
    temp_v0->D_80162980 = arg0;
    temp_v0->D_8016297E = arg2;
    temp_v0->D_8016297C = arg1;
}

void func_800D5938();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D5938);

static void func_800D5A68(s16 arg0, s16 arg1) {
    Unk80162978* temp_v0;

    temp_v0 = &D_80162978[func_800BBEAC(func_800D5938)];
    temp_v0->D_80162978 = 0;
    temp_v0->D_8016297E = arg1;
    temp_v0->D_8016297C = arg0;
}

// Divide each byte lane of a packed color independently by a divisor,
// yielding a per-channel step (e.g. a color-fade increment).
static s32 func_800D5AC0(s32 arg0, s32 arg1) {
    return (((arg0 & 0xFF0000) / arg1) & 0xFF0000) |
           (((arg0 & 0xFF00) / arg1) & 0xFF00) |
           (((arg0 & 0xFF) / arg1) & 0xFF);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D5B6C);

void func_800D5D28();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D5D28);

// Claim a slot running func_800D5D28. arg2's top byte is the slot kind and
// its low 24 bits are re-tagged 0x3A before being stored.
void func_800D61AC(s32 arg0, s32 arg1, u32 arg2, s32 arg3, s32 arg4) {
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D5D28)];

    p->D_801621F0 = arg2 >> 24;
    *(s32*)&p->D_801621F4 = arg0;
    *(s16*)&p->unk14 = -arg1;
    *(s32*)&p->unk8 = 0;
    *(s32*)&p->unkC = (arg2 & 0xFFFFFF) | 0x3A000000;
    *(s32*)&p->unk10 = func_800D5AC0(arg2, arg3);
    ((s16*)&p->unk14)[1] = arg4;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6260);

extern Unk801B0C98 D_800F14D0;

// Draw a model 4 times through func_800D29D4 (same request-struct pattern as
// barrier.c's D_801B0C98/D_801B0CB0), toggling the 0x1/0x2 flag bits between
// passes.
void func_800D6394(s32* arg0, s16 arg1) {
    D_800F14D0.unk0 = arg0;
    D_800F14D0.unkA = arg1;
    SetFarColor(0, 0, 0);
    PushMatrix();
    D_80163C74 = func_800D29D4(&D_800F14D0, g_cDb->unk70, 0xC, D_80163C74);
    PopMatrix();
    PushMatrix();
    D_800F14D0.unk4 |= 1;
    D_80163C74 = func_800D29D4(&D_800F14D0, g_cDb->unk70, 0xC, D_80163C74);
    PopMatrix();
    PushMatrix();
    D_800F14D0.unk4 |= 2;
    D_80163C74 = func_800D29D4(&D_800F14D0, g_cDb->unk70, 0xC, D_80163C74);
    PopMatrix();
    D_800F14D0.unk4 &= ~1;
    D_80163C74 = func_800D29D4(&D_800F14D0, g_cDb->unk70, 0xC, D_80163C74);
    D_800F14D0.unk4 &= ~2;
}

void func_800D650C();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D650C);

extern u8 D_800F10EC[];
extern u8 D_800F11E8[];
extern u8 D_800F1304[];

// `tbl` is a local aggregate initialiser, so its constant is the .rodata blob
// the target copies in with three lw/sw pairs before the call -- it is the
// object that used to be carried here as `u8* const D_800A0DC8[]` while this
// function was still INCLUDE_ASM, and that object had to go with it or the
// unit would emit the same three words twice.
void func_800D6734(s32 arg0, s32 arg1) {
    u8* tbl[] = {D_800F10EC, D_800F11E8, D_800F1304};
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D650C)];

    p->unk8 = arg0;
    *(u8**)&p->D_801621F4 = tbl[arg1];
}

void func_800D6734(s32, s32);
extern s32 D_800F14D4;

static void func_800D67BC(s32 arg0) {
    D_800F14D4 = 0x88;
    func_800D6734(arg0, 0);
}

static void func_800D67E8(s32 arg0) {
    D_800F14D4 = 0xA8;
    func_800D6734(arg0, 1);
}

void func_800D6814(s32 arg0) {
    D_800F14D4 = 0x88;
    func_800D6734(arg0, 2);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6840);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6998);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6ACC);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6C20);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6D8C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D6F78);

// Every fourth tick, hand the current slot's position to a fresh slot running
// the callback it carries at +0x1C; retire the slot after 13 ticks.
//
// `q` has to be a named pointer: written inline as
// `&D_801621F0[...].D_801621F4` the +4 folds into the base register and the
// block move stores at 0/4/7/0xB, where the target keeps the element address
// and carries the +4 as a displacement.
void func_800D70C0(void) {
    Unk801621F0* p = &D_801621F0[D_801590D4];
    Unk801621F0* q;

    if (D_80062D98 != 0) {
        return;
    }
    if ((p->D_801621F2 & 3) == 0) {
        q = &D_801621F0[func_800BC04C(*(void (**)()) & p->unk1C)];
        *(SVECTOR*)&q->D_801621F4 = *(SVECTOR*)&p->D_801621F4;
    }
    p->D_801621F2++;
    if (p->D_801621F2 == 0xD) {
        p->D_801621F0 = -1;
    }
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D7178);

void func_800D72B4(void) {
    Unk801621F0* elem = &D_801621F0[D_801590D4];

    if (D_80062D98 == 0) {
        // Advance this slot's per-tick state machine (field 0x2).
        if (elem->D_801621F2 == 0) {
            func_800D5138(1);
        }
        if (elem->D_801621F2 == 2) {
            func_800D51D4(1);
            elem->D_801621F0 = -1;
        }
        elem->D_801621F2++;
    }
}

static void func_800D7340(void) { func_800BBEAC(func_800D72B4); }

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D7368);

void func_800D751C();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D751C);

// Claim a free effect slot running func_800D751C and seed it with a position.
void func_800D76B8(SVECTOR* arg0) {
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D751C)];

    *(SVECTOR*)&p->unk8 = *arg0;
    p->D_801621F0 = 1;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D7724);

void func_800D7888();
INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D7888);

// Claim a slot running func_800D7888. Six parameters, so arg4 and arg5 come
// off the caller's stack; the store order 0x4, 0x6, 0x1C, 0x1A, 0x8, 0xA is
// the source order, since gcc keeps struct stores where they were written.
void func_800D7A88(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    Unk801621F0* p = &D_801621F0[func_800BC04C(func_800D7888)];

    p->D_801621F4 = arg0;
    p->D_801621F6 = arg1;
    *(s32*)&p->unk1C = arg2;
    p->unk1A = arg3;
    p->unk8 = arg4;
    p->unkA = arg5;
}

// Queue a func_800D7888 effect on one of model arg1's sub-parts. The part is
// picked by the byte at +0x31 and the angle comes from the halfword at +0x1A.
//
// The byte-offset addressing is what the target has: a scaled subscript
// (`D_801518E4[arg1].member`) folds the symbol into one base register, where
// an `off` local computed separately leaves `(plus (symbol) (reg))` in the
// mem and the assembler expands it through $at at each access, which is the
// three-instruction form here. `base` and `d` are named so the two sums keep
// source order -- inline, fold ranks the multiply above them and computes
// `(d + base) + off` instead of `(base + off) + d`.
void func_800D7B1C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 off = arg1 * sizeof(BattleModel);
    s32 n = *(u8*)((u8*)D_801518E4 + 0x31 + off);
    s32 a = *(s16*)((u8*)D_801518E4 + 0x1A + off);
    s32 base = (s32)((u8*)D_801518E4 + 0x174) + off;
    s32 d = n * sizeof(BattleModelSub);

    func_800D7A88(arg0, arg1, base + d, a, arg2, arg3);
}

// The same as func_800D7B1C one part along -- +0x32 and +0x1C.
void func_800D7BA4(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 off = arg1 * sizeof(BattleModel);
    s32 n = *(u8*)((u8*)D_801518E4 + 0x32 + off);
    s32 a = *(s16*)((u8*)D_801518E4 + 0x1C + off);
    s32 base = (s32)((u8*)D_801518E4 + 0x174) + off;
    s32 d = n * sizeof(BattleModelSub);

    func_800D7A88(arg0, arg1, base + d, a, arg2, arg3);
}

// func_800D7B1C and func_800D7BA4 in one call pair, sharing the model offset
// and the sub-part array base across both.
void func_800D7C2C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 off = arg1 * sizeof(BattleModel);
    s32 n = *(u8*)((u8*)D_801518E4 + 0x31 + off);
    s32 a = *(s16*)((u8*)D_801518E4 + 0x1A + off);
    s32 base = (s32)((u8*)D_801518E4 + 0x174) + off;
    s32 d = n * sizeof(BattleModelSub);

    func_800D7A88(arg0, arg1, base + d, a, arg2, arg3);
    n = *(u8*)((u8*)D_801518E4 + 0x32 + off);
    a = *(s16*)((u8*)D_801518E4 + 0x1C + off);
    d = n * sizeof(BattleModelSub);
    func_800D7A88(arg0, arg1, base + d, a, arg2, arg3);
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D7D3C);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D8304);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D83A4);

typedef struct {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u16 unk8;
    /* 0x0A */ u16 unkA;
    /* 0x0C */ u16 unkC;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
} UnkStruct800D8468; // size:0x20

void func_800D8468(UnkStruct800D8468* dst, UnkStruct800D8468* src) {
    dst->unk0 = src->unk0;
    dst->unk6 = src->unk6;
    dst->unkC = src->unkC;
    dst->unk2 = src->unk2;
    dst->unk8 = src->unk8;
    dst->unkE = src->unkE;
    dst->unk4 = src->unk4;
    dst->unkA = src->unkA;
    dst->unk10 = src->unk10;
    dst->unk14 = src->unk14;
    dst->unk18 = src->unk18;
    dst->unk1C = src->unk1C;
}

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D84F8);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D85B0);

INCLUDE_ASM("asm/us/battle/nonmatchings/battle2", func_800D87EC);
