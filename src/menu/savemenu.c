//! PSYQ=3.3 CC1=2.7.2
#include <psxsdk/libapi.h>
#include <psxsdk/libgpu.h>
#include <psxsdk/kernel.h>

#include "savemenu.h"

static void func_801D0408(u16 arg0) {
    D_8009A000[0] = 0x30;
    D_8009A004[0] = arg0;
    D_8009A008[0] = arg0;
    SystemAkaoExecute();
}

static s32 func_801D0448(s32 arg0) {
    RECT rect;

    setTile(D_80062F24.tile);
    SetSemiTrans(D_80062F24.tile, 1);
    D_80062F24.tile->x0 = 0;
    D_80062F24.tile->y0 = 0;
    D_80062F24.tile->w = 0x180;
    D_80062F24.tile->h = 0xE8;
    D_80062F24.tile->r0 = D_801D4EC4;
    D_80062F24.tile->g0 = D_801D4EC4;
    D_80062F24.tile->b0 = D_801D4EC4;
    AddPrim(D_80062FC4, D_80062F24.tile++);
    rect.x = 0;
    rect.y = 0;
    rect.w = 255;
    rect.h = 255;
    func_80026A34(0, 1, 0x5F, &rect);
    D_801D4EC4 += arg0;
    if (D_801D4EC4 < 0) {
        D_801D4EC4 = 0;
    }
    if (D_801D4EC4 >= 0x100) {
        D_801D4EC4 = 0xFF;
    }
    return D_801D4EC4;
}

void func_801D05C0(u8 arg0) {
    D_801E3860 = 0xF0;
    D_801E36B8 = arg0;
    D_801E3850 = 0;
    func_80026448(menus.D_801E379C, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 0, 1, 0);
    func_80025B8C(&D_801E8F44);
    func_80025C14(&D_801E4538);
    func_80025DF8();
    func_801D19C4();
}

static void func_801D0670(void) {
    func_80025BD0(D_801E8F44);
    func_80025C54(D_801E4538);
    func_801D1BA4();
}

int func_801D06B0(s32 arg0) {
    RECT sp38;
    RECT rect;
    s32 temp_s1;
    s32 temp_s2;
    s32 temp_v1;
    s32 var_s0;
    s32 var_s1;
    s32 var_s2;
    s32 var_s3;
    s32 var_v0_6;
    s8 temp_s0_2;

    if (D_801E36B8 == 0) {
        func_800230C4(D_80062F58);
    } else if (D_801E36B0 == 0) {
        if (func_801D0448(-15) == 0) {
            D_801E36B0 = 1;
        }
    } else if (D_801E36B0 == 2) {
        if (func_801D0448(15) == 0xFF) {
            D_801E36B0 = -1;
        }
    }
    if (!func_80023050() || (D_801E36B8 && D_801E36B0 == 1)) {
        if (!(u8)func_8001F6B4()) {
            if (D_801E3850 >= 0 && D_801E3850 < 2) {
                func_801D3668(arg0);
            }
            if (D_801E3860) {
                D_801E3860--;
            }
        }
    }
    func_80026B5C(0x80);
    switch (D_801E3850) {
    case 0:
        func_8001EB2C(D_801D4EC8.x - 18,
                      D_801D4EC8.y + 6 + (menus.D_801E379C[0].unkB * 12));
        func_80026F44(0xA, 0xB, D_801E2CFC[1], 7);
        func_80026F44(D_801D4EC8.x + 12, D_801D4EC8.y + 5, D_801E2CFC[3],
                      -(D_801E8F38[0][0] != 0) & 7);
        func_80026F44(D_801D4EC8.x + 12, D_801D4EC8.y + 17, D_801E2CFC[4],
                      -(D_801E8F3B != 0) & 7);
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x100;
        rect.h = 0x100;
        func_80026A34(0, 1, 0x7F, &rect);
        func_8001E040(&D_801D4EC8);
        break;
    case 7:
        func_8001EB2C(D_801D4ED0.x + 0x16,
                      0x15 + D_801D4ED0.y + menus.D_801E3808[1].unkB * 0xC);
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x100;
        rect.h = 0x100;
        func_80026A34(0, 1, 0x7F, &rect);
        func_80026F44(D_801D4ED0.x + 0xA, D_801D4ED0.y + 6, D_801E2CFC[33], 7);
        func_80026F44(D_801D4ED0.x + 48, D_801D4ED0.y + 19, D_801E2CFC[34], 7);
        func_80026F44(D_801D4ED0.x + 48, D_801D4ED0.y + 31, D_801E2CFC[35], 7);
        func_8001E040(&D_801D4ED0);
        /* fallthrough */
    case 1:
        if (!D_801E8F38[menus.D_801E379C[0].unkB][0]) {
            D_801E3850 = 0;
        } else {
            func_800269D0();
            if (D_801E36B8 == 0) {
                func_800269C0(D_80062F58 * 0x5000 + D_801D4EDC);
            } else {
                func_800269C0(D_801E36B4 * 0x5000 + D_801D4EDC);
            }
            if (D_801E3850 != 7 || (arg0 & 2)) {
                func_8001EB2C(8, (menus.D_801E379C[1].unkB << 6) | 0x38);
            }
            var_s3 = !menus.D_801E379C[1].unk8 ? 3 : 4;
            for (var_s0 = 0; var_s0 < var_s3; var_s0++) {
                if ((D_80062F3C >> (var_s0 + menus.D_801E379C[1].unk2)) & 1) {
                    func_8001DE70();
                    func_801D370C(
                        0, var_s0 * 64 + 29 + menus.D_801E379C[1].unkF * 8,
                        var_s0 + menus.D_801E379C[1].unk2);
                    func_8001DEB0();
                } else {
                    func_80026F44(
                        0x32, var_s0 * 64 + 55 + menus.D_801E379C[1].unkF * 8,
                        D_801E2CFC[8], 6);
                    func_8001DE40(&sp38, &D_801DEEF4);
                    func_8001DE24(
                        &sp38, 0,
                        var_s0 * 64 + 29 + menus.D_801E379C[1].unkF * 8);
                    func_8001E040(&sp38);
                }
            }
            func_80026B5C(0x80);
            rect.y = 29;
            rect.w = 364;
            rect.x = 0;
            rect.h = 195;
            if (D_801E36B8 == 0) {
                func_80026A94(&D_800706A4[D_80062F58], &rect);
            } else {
                func_80026A94(&D_801E36BC[D_801E36B4], &rect);
            }
            func_80026F44(10, 11, D_801E2CFC[2], 7);
            func_80026F44(206, 11, D_801E2CFC[9], 6);
            func_80026F44(
                func_80026B70(D_801E2CFC[9]) + 208, 11,
                ((13 + menus.D_801E379C[1].unkB + menus.D_801E379C[1].unk2) *
                 36) +
                    D_801E2CFC[0],
                7);
            func_8001DE0C(&sp38, 200, 5, 78, 24);
            func_8001E040(&sp38);
            func_800269E8();
        }
        break;
    case 2:
    case 3:
        if (D_801E3850 == 2) {
            var_s2 = 64;
            var_s1 = 32;
            var_s0 = 160;
        } else {
            var_s2 = 224;
            var_s1 = 128;
            var_s0 = 0;
        }
        func_80026F44(10, 11, D_801E2CFC[12], 7);
        if (D_801E36A8 == 0) {
            func_800285AC(
                122, 117, (D_801E36AC + 1) * 8, 8, var_s2, var_s1, var_s0);
            rect.x = 0;
            rect.y = 0;
            rect.w = 0xFF;
            rect.h = 0xFF;
            func_80026A34(0, 1, 0x3F, &rect);
        }
        func_8001DE0C(&sp38, 0x70, 0x6D, 0x8C, 0x18);
        func_8001E040(&sp38);
        break;
    case 4:
        temp_s1 = func_80026B70(D_801E2CFC[7]) + 0x10;
        func_80026F44(190 - temp_s1 / 2, 115, D_801E2CFC[7], 7);
        func_8001DE0C(&sp38, 182 - temp_s1 / 2, 109, temp_s1, 24);
        func_8001E040(&sp38);
        break;
    case 6:
        if (arg0 & 2) {
            func_8001EB2C(D_801D4EC8.x - 18,
                          D_801D4EC8.y + 6 + menus.D_801E379C[0].unkB * 0xC);
        }
        func_80026F44(D_801D4EC8.x + 12, D_801D4EC8.y + 5, D_801E2CFC[3],
                      -(D_801E8F38[0][0] != 0) & 7);
        func_80026F44(D_801D4EC8.x + 12, D_801D4EC8.y + 0x11, D_801E2CFC[4],
                      -(D_801E8F38[1][0] != 0) & 7);
        rect.x = 0;
        rect.y = 0;
        rect.w = 0x100;
        rect.h = 0x100;
        func_80026A34(0, 1, 0x7F, &rect);
        func_8001E040(&D_801D4EC8);
        func_80026F44(10, 11, D_801E3260[4], 7);
        temp_s2 = func_80026B70(D_801E3260[5]) + 0x10;
        func_80026F44(190 - temp_s2 / 2, D_801D4EC8.h + 99, D_801E3260[5], 7);
        func_80026F44(228 - temp_s2 / 2, D_801D4EC8.h + 112, D_801E2CFC[34], 7);
        func_80026F44(228 - temp_s2 / 2, D_801D4EC8.h + 124, D_801E2CFC[35], 7);
        func_8001EB2C(200 - temp_s2 / 2,
                      115 + (menus.D_801E3808[0].unkB * 12) + D_801D4EC8.h);
        func_8001DE0C(
            &sp38, 182 - temp_s2 / 2, D_801D4EC8.h + 93, temp_s2, 0x30);
        func_8001E040(&sp38);
        break;
    }
    if (D_801E36B8 != 0) {
        func_80026B5C(0x80);
        func_80026F44(294, 11, &D_801DEEDC, 7);
        func_8001E040(&D_801DEEFC);
    }
    func_8001DE0C(&sp38, 0, 5, 364, 24);
    func_8001E040(&sp38);
    if (!(D_801E36B8 == 0 && !func_80023050()) &&
        (D_801E36B8 == 0 || D_801E36B0 != 1)) {
        return;
    }
    if (func_8001F6B4() & 0xFF) {
        return;
    }
    switch (D_801E3850) {
    case 0:
        if (D_80062D7C & 0x20) {
            if (D_801E8F38[menus.D_801E379C[0].unkB][0]) {
                func_801D0408(1);
                if (D_801E8F38[menus.D_801E379C[0].unkB][2]) {
                    D_801E3850 = 6;
                    func_80026448(&menus.D_801E3808[0], 0, 1, 1, 2, 0, 0, 1, 2,
                                  0, 0, 0, 1, 0);
                } else {
                    D_801E3850 = 2;
                    D_801E36AC = 0;
                    D_801E36A0 = 0;
                    D_80062F3C = 0;
                    D_801E36A8 = 1;
                    D_801E36A4 = 0x3C;
                    func_80026448(&menus.D_801E379C[1], 0, 0, 1, 3, 0, 0, 1, 15,
                                  0, 0, 0, 0, 0);
                }
            } else {
                func_801D0408(3);
                func_8001F6C0(!D_801E3860 ? D_801E33B0[0] : D_801E3260[6], 7);
            }
        } else {
            func_800264A8(&menus.D_801E379C[0]);
            if (D_801E36B8 != 0) {
                if (D_80062D7C & 0x40) {
                    func_801D0408(4);
                    D_801E36B0 = 2;
                }
            } else if (D_80062D7C & 0x40) {
                func_801D0408(4);
                func_801D0670();
                func_8002305C(5, 0);
                func_8002120C(0);
            }
        }
        break;
    case 1:
        var_s0 = menus.D_801E379C[1].unkF;
        func_801D2DA8(&menus.D_801E379C[1]);
        if ((menus.D_801E379C[1].unkF == 0) && (var_s0 == 0)) {
            if (D_80062D7C & 0x20) {
                D_801E3850 = 7;
                func_80026448(&menus.D_801E3808[1], 0, 0, 1, 2, 0, 0, 1, 2, 0,
                              0, 0, 1, 0);
                func_801D0408(1);
            } else if (D_80062D7C & 0x40) {
                func_801D0408(4);
                D_801E3850 = 0;
            }
        }
        break;
    case 2:
        if (D_801E36A4 == 0) {
            if (D_801E36A8) {
                D_801E36A4 = 0;
                D_801E36A8 = 0;
                D_80062F3C = func_801D1C2C(menus.D_801E379C[0].unkB);
            } else {
                var_s0 = 0;
                if ((D_80062F3C >> D_801E36AC) & 1) {
                    var_s0 =
                        func_801D3698(menus.D_801E379C[0].unkB, D_801E36AC);
                }
                D_801E36AC++;
                if (var_s0) {
                    D_801E3850 = 0;
                    func_8001F6C0(D_801E33B0[8], 2);
                }
                if (D_801E36AC == 0xF) {
                    D_801E36AC = 0xE;
                    D_801E3850 = 3;
                    D_801E36A4 = 0xA;
                    func_801D0408(2);
                }
            }
        } else {
            D_801E36A4--;
        }
        break;
    case 3:
        if (D_801E36A4 == 0) {
            D_801E3850 = 1;
        }
        D_801E36A4--;
        break;
    case 4:
        if (D_801E36A4 != 0) {
            D_801E36A4--;
            return;
        }
        D_801E3850 = 1;
        var_v0_6 = menus.D_801E379C[1].unkB + menus.D_801E379C[1].unk2;
        if (menus.D_801E379C[0].unkB != 0) {
            var_v0_6 |= 0x10;
        }
        if (!func_801D2A34(var_v0_6)) {
            func_801D0408(0xD0);
            func_8001F6C0(D_801E2CFC[28], 7);
            D_80062F3C |=
                1 << (menus.D_801E379C[1].unkB + menus.D_801E379C[1].unk2);
        } else {
            func_801D0408(3);
            func_8001F6C0(D_801E33B0[3], 7);
        }
        break;
    case 6:
        func_800264A8(&menus.D_801E3808[0]);
        if (D_80062D7C & 0x20) {
            if (menus.D_801E3808[0].unkB) {
                D_801E3850 = 0;
                func_801D0408(4);
            } else {
                if (menus.D_801E379C[0].unkB) {
                    temp_v1 = format("bu10:");
                } else {
                    temp_v1 = format("bu00:");
                }
                D_801E3850 = 0;
                if (temp_v1 == 1) {
                    D_801E8F38[menus.D_801E379C[0].unkB][2] = 0;
                    func_8001F6C0(D_801E2CFC[41], 7);
                    func_801D0408(0xD0);
                } else {
                    func_8001F6C0(D_801E3260[3], 7);
                    func_801D0408(3);
                }
            }
        } else if (D_80062D7C & 0x40) {
            D_801E3850 = 0;
            func_801D0408(4);
        }
        break;
    case 7:
        if (D_80062D7C & 0x20) {
            temp_s0_2 = menus.D_801E3808[1].unkB;
            switch (menus.D_801E3808[1].unkB) {
            case 0:
                func_801D0408(1);
                D_801E3850 = 4;
                D_801E36A4 = 0xA;
                break;
            case 1:
                func_801D0408(4);
                D_801E3850 = temp_s0_2;
                break;
            }
        } else if (D_80062D7C & 0x40) {
            D_801E3850 = 1;
            func_801D0408(4);
        } else {
            func_800264A8(&menus.D_801E3808[1]);
        }
        break;
    }
}

static const char* D_801E2C78[] = {
    "BASCUS-94163FF7-S01", "BASCUS-94163FF7-S02", "BASCUS-94163FF7-S03",
    "BASCUS-94163FF7-S04", "BASCUS-94163FF7-S05", "BASCUS-94163FF7-S06",
    "BASCUS-94163FF7-S07", "BASCUS-94163FF7-S08", "BASCUS-94163FF7-S09",
    "BASCUS-94163FF7-S10", "BASCUS-94163FF7-S11", "BASCUS-94163FF7-S12",
    "BASCUS-94163FF7-S13", "BASCUS-94163FF7-S14", "BASCUS-94163FF7-S15",
};
static s32 D_801E2CB4 = 0;

s32 func_801D1774(void) {
    s32 ret;
    s32 i;

    func_80021044(D_801E36BC, D_801E3774);
    i = 0;
    D_801E36B0 = 0;
    func_801D05C0(1);
    D_801E36B4 = 0;
    while (1) {
        func_8001CB48();
        func_800269C0(D_80077F64[D_801E36B4]);
        D_801E3854 = (u_long*)D_801E3858[D_801E36B4];
        ClearOTag(D_801E3854, 1);
        func_80026A00(D_801E3854);
        func_8001F710();
        ret = func_801D06B0(i);
        if (D_801E36B0 == -1) {
            break;
        }
        DrawSync(0);
        VSync(0);
        PutDispEnv(&D_801E3774[D_801E36B4]);
        PutDrawEnv(&D_801E36BC[D_801E36B4]);
        DrawOTag(D_801E3854);
        D_801E36B4 ^= 1;
        i++;
    }
    func_801D0670();
    VSync(0);
    PutDispEnv(&D_801E3774[0]);
    PutDrawEnv(&D_801E36BC[0]);
    VSync(0);
    PutDispEnv(&D_801E3774[1]);
    PutDrawEnv(&D_801E36BC[1]);
    return ret;
}

u16 func_801D1950(u16 len, u8* data) {
    u16 i, j;
    s32 sum = 0xFFFF;
    for (i = 0; i < len; i++) {
        sum ^= *(data + i) << 8;
        for (j = 0; j < 8; j++) {
            if (sum & 0x8000) {
                sum = (sum * 2) ^ 0x1021;
            } else {
                sum *= 2;
            }
        }
    }
    return ~sum;
}

void func_801D19C4(void) {
    s32 i;

    if (D_80062DCC == 0) {
        EnterCriticalSection();
        D_8009A024[0] = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
        D_8009A024[1] = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
        D_8009A024[2] = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
        D_8009A024[3] = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
        D_8009A024[4] = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
        D_8009A024[5] = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
        D_8009A024[6] = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
        D_8009A024[7] = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
        InitCARD(1);
        StartCARD();
        ChangeClearPAD(0);
        _bu_init();
        _card_auto(0);
        for (i = 0; i < 8; i++) {
            EnableEvent(D_8009A024[i]);
        }
        ExitCriticalSection();
        D_80062DCC = 1;
    }
    for (i = 0; i < 2; i++) {
        D_801E8F38[i][0] = 0;
        D_801E8F38[i][1] = 0;
        D_801E8F38[i][2] = 0;
    }
}

void func_801D1BA4(void) {}

static void func_801D1BAC(s32 arg0, s32 arg1) { TestEvent(D_8009A024[arg1]); }

// strcmp?
static s32 func_801D1BE0(u8* arg0, u8* arg1) {
    while (1) {
        if (*arg0++ != *arg1++) {
            return 0;
        }
        if (!*arg0 && !*arg1) {
            return 1;
        }
    }
}

// Bitmask of which of the 15 FF7 save slots exist on the card in slot arg0.
// firstfile is retried up to 100 times because the card takes a moment to come
// up after an insert; a card that never answers reports no saves at all.
//
// 5 rows, 58 instructions against 60 -- and the two missing instructions are
// deleted by a pass that runs *after* register allocation, which is as far as
// C reaches. cc1's own dumps say so exactly (variant_eval.py --rtl=a): at
// `.greg` this body already has the target's tail, insn for insn,
//
//     (insn 162  (set (reg/i:SI 2 v0) (zero_extend:SI (reg/v:HI 19 s3))))
//     (jump_insn 164 (set (pc) (label_ref 192)))     <- the `j` past `end:`
//     (code_label 166 "end")
//     (insn 170  (set (reg/i:SI 2 v0) (zero_extend:SI (reg/v:HI 19 s3))))
//     (code_label 192)                               <- the epilogue
//
// and at `.jump2` insns 162 and 164 are gone: the post-reload jump_optimize
// merges the two identical `andi; return` tails, so the success path falls
// straight into the `end:` copy. The target keeps both, with reorg then
// duplicating the `andi` into the `j`'s delay slot:
//
//     1ce8  j     1cf4           <- success path, jumping over the copy below
//     1cec  andi  v0,s3,0xffff   <- ...duplicated into the slot by reorg
//     1cf0  andi  v0,s3,0xffff   <- the `end:` copy, reached by `j` from 1c90
//     1cf4  lw    ra,0x50(sp)
//
// The two insns are byte-identical after reload in the target too, so nothing
// about *this* function's spelling can be what stops the merge there.
//
// The one lever that has moved it, 9 rows -> 5: a redundant `var_s3 = 0;` at
// `end:`, on top of the one the failure path already does. gcc deletes the
// earlier store, but the extra reference is enough to change reorg's mind
// about the scan loop's back edge, which then takes the loop-top `i = 0` into
// its delay slot the way the target does. Moving the zero down to `end:`
// *instead* of leaving it in the loop is 6 rows, because cse then folds the
// return to `move v0,zero` where the original keeps `move s3,zero` plus the
// shared `andi`.
//
// Measured and exactly inert, all at 9 rows on the body without that store and
// 5 with it: the scan loop as `while` / `do`-`while` / `for (; entry; entry =
// nextfile(entry))`; `i = 0` as a `for` init or as its own statement; an empty
// `do { } while (0);` before either return or at `end:`; `goto end;` for the
// success exit; `return (u16)var_s3` / `return var_s3 & 0xFFFF` at either
// exit; a second `u16 result` local for one or both exits, with and without a
// cse barrier between its store and its return; `var_s3` as `s32` or `u32`.
// Twenty-two spellings, three distinct row counts -- jump_optimize normalises
// the whole tail dimension before anything interesting happens to it.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/menu/nonmatchings/savemenu", func_801D1C2C);
#else
const char D_801D017C[] = "bu10:*";
const char D_801D0184[] = "bu00:*";

u16 func_801D1C2C(s32 arg0) {
    struct DIRENTRY sp10;
    const char* memcard;
    s32 i;
    struct DIRENTRY* entry;
    u16 var_s3;

    var_s3 = 0;
    i = 0;
    while (1) {
        memcard = D_801D0184;
        if (arg0) {
            memcard = D_801D017C;
        }
        entry = firstfile(memcard, &sp10);
        if (entry) {
            break;
        }
        i++;
        if (i >= 100) {
            var_s3 = 0;
            goto end;
        }
    }
    while (entry) {
        for (i = 0; i < 15; i++) {
            if (func_801D1BE0(entry->name, D_801E2C78[i]) != 0) {
                var_s3 |= 1 << i;
            }
        }
        entry = nextfile(entry);
    }
    return var_s3;
end:
    // Redundant -- gcc deletes the store the failure path already made. It is
    // here for its *reference*: see the note above, it is worth 4 rows.
    var_s3 = 0;
    return var_s3;
}
#endif

const char D_801D018C[] = "bu10:%s";
const char D_801D0194[] = "bu00:%s";

SaveHeader* func_801D1D1C(s32 arg0) { return &D_801E3864[arg0]; }

// Load one save slot's header off the memory card. read() is retried up to 30
// times, each attempt asking only for the bytes the previous one did not
// deliver. 1 = could not open, 2 = gave up reading, 0 = header copied in.
s32 func_801D1D40(s32 arg0) {
    // Never touched, but the original reserved 0x28 bytes below `path`: the
    // target's sprintf buffer is at sp+0x38 in a 0x70 frame, and without this
    // the frame is 0x48 with the buffer at sp+0x10. A DIRENTRY is exactly the
    // 0x28 the gap wants, and this file uses one in func_801D1C2C.
    struct DIRENTRY dir;
    char path[0x20];
    s32 fd;
    s32 retries;
    s32 n;

    D_801E8F40 = 0x280;
    if (arg0 & 0x10) {
        sprintf(path, D_801D018C, D_801E2C78[arg0 & 0xF]);
    } else {
        sprintf(path, D_801D0194, D_801E2C78[arg0 & 0xF]);
    }
    fd = open(path, 1);
    if (fd == -1) {
        return 1;
    }
    retries = 30;
    do {
        n = read(fd, D_801E6F38, D_801E8F40);
        if (n == D_801E8F40) {
            goto ok;
        }
        retries--;
        if (n != -1) {
            D_801E8F40 -= n;
        }
    } while (retries != 0);
    close(fd);
    return 2;
ok:
    close(fd);
    memcpy(&D_801E3864[arg0 & 0xF], D_801E7138, sizeof(SaveHeader));
    return 0;
}

// Load a whole save file over the live Savemap, then push its menu colours
// back into the global palette. Same open/read/retry shape as func_801D1D40,
// but 0x2000 bytes and the destination is the save work area itself.
//
// The two `do { close(fd); } while (0);` are a reference multiplier, not a
// control-flow statement: `flow.c` counts REG_N_REFS += loop_depth, so a
// reference inside an empty loop counts twice while emitting nothing. It is
// needed because `global_alloc` ranks by floor_log2(n_refs) * n_refs /
// live_length and the two callee-saved values here land either side of the
// step. Read off cc1's -dl dump (variant_eval.py --rtl): `retries` is 7 refs
// over 14 insns, so 2*7/14 = 1.00, and `fd` is 6 over 19, so 2*6/19 = 0.63 --
// `retries` wins $s0 and the target has `fd` there. Eight references put `fd`
// at 3*8/19 = 1.26 and the whole function falls into place, and eight is what
// the two barriers buy. Seven (one barrier, either one) is still 2*7/19 =
// 0.74 and measures 8 rows, which is the same wrong assignment.
//
// What the original wrote there is not recoverable, in the same sense as a
// `u8 unusedLocals[N]` frame reservation. Everything natural was measured:
// `n` as the colour loop's counter is required (a separate counter, or
// reusing `retries` or `fd`, is 8/13/19 rows), and writing the success path
// inside the retry loop -- which reaches 7 references honestly -- is +2
// instructions and 40 rows. Declaration order is flat across all eight
// permutations, which is `allocno_compare`'s tie-break saying the priorities
// were never tied; a width sweep over all four locals is flat at 5 or worse.
s32 func_801D1F40(s32 arg0) {
    struct DIRENTRY dir; // see func_801D1D40: reserved, never read
    char path[0x20];
    s32 retries;
    s32 fd;
    s32 n;

    n = arg0;
    D_801E8F40 = 0x2000;
    if (arg0 & 0x10) {
        sprintf(path, D_801D018C, D_801E2C78[n & 0xF]);
    } else {
        sprintf(path, D_801D0194, D_801E2C78[arg0 & 0xF]);
    }
    fd = open(path, 1);
    if (fd == -1) {
        return 1;
    }
    retries = 30;
    do {
        n = read(fd, D_801E6F38, D_801E8F40);
        if (n == D_801E8F40) {
            goto ok;
        }
        // BUG: counts up, not down -- func_801D1D40's twin does `retries--`.
        // The loop only ever leaves through the read succeeding.
        retries++;
        if (n != -1) {
            D_801E8F40 -= n;
        }
    } while (retries != 0);
    do {
        close(fd);
    } while (0);
    return 2;
ok:
    do {
        close(fd);
    } while (0);
    memcpy(&Savemap, D_801E7138, sizeof(SaveWork));
    for (n = 0; n < 12; n++) {
        g_FieldWindowColors[n] = ((u8*)Savemap.header.menu_color)[n];
    }
    return 0;
}

static s32 func_801D2150(s8 arg0) {
    if (arg0 > -0x68 && arg0 < -0x60 || arg0 > -32 && arg0 < -3) {
        return 1;
    }
    return 0;
}

s32 func_801D2184(s8 arg0) {
    if (arg0 > -0x80 && arg0 < -0x60 || arg0 > -33 && arg0 < -3) {
        return 1;
    }
    return 0;
}

static void func_801D21B8(u8* arg0, u8* arg1) {
    s32 i;
    for (i = 0; i < 0x40; i++) {
        *arg0++ = *arg1++;
    }
}

static void func_801D21E0(s32 arg0) { D_801E2CB4 = arg0; }

void func_801D21F0(u8* arg0, u8* arg1) {
    s32 i;
    for (i = 0; i < D_801E2CB4; i++) {
        *arg0++ = *arg1;
        if (*arg1 == 0xFF) {
            break;
        }
        arg1++;
    }
}

// Snapshot the live party into the save header, so the file-select screen can
// show it without loading the file. The leader is the first of the three party
// slots that is filled; its HP/MP come from the live battle records, which are
// four parallel arrays with a 0x440-byte stride.
void func_801D224C(void) {
    s16 charId;
    s32 i;
    s32 statsOfs;

    for (i = 0; i < 3; i++) {
        (&Savemap.header.leader_portrait)[i] = D_8009CBDC[i];
    }
    func_801D21E0(0x10);
    i = 0;
    do {
        charId = D_8009CBDC[i];
        if (charId != 0xFF) {
            // One offset shared by all four reads: computing it per access
            // gives gcc four induction variables and it caches the four base
            // addresses in registers instead of re-materialising them.
            statsOfs = i * 0x440;
            func_801D21F0(
                Savemap.header.leader_name, Savemap.party[charId].name);
            Savemap.header.leader_level = Savemap.party[charId].level;
            Savemap.header.leader_hp = *(u16*)((u8*)D_8009D85C + statsOfs);
            Savemap.header.leader_hp_max = *(u16*)((u8*)D_8009D85E + statsOfs);
            Savemap.header.leader_mp = *(u16*)((u8*)D_8009D860 + statsOfs);
            Savemap.header.leader_mp_max = *(u16*)((u8*)D_8009D862 + statsOfs);
            break;
        }
        i++;
    } while (i < 3);
    for (i = 0; i < 12; i++) {
        ((u8*)Savemap.header.menu_color)[i] = g_FieldWindowColors[i];
    }
    Savemap.header.gil = D_8009D260;
    Savemap.header.time = D_8009D264;
    func_801D21E0(0x18);
    func_801D21F0(Savemap.header.place_name, &Savemap.memory_bank_4[0x68]);
}

// Digit glyph pairs, indexed by (digit * 2 + 0x20): the low byte of each pair
// is at D_801DEF08 and the high byte at D_801DEF09, which is how the original
// reaches them -- two symbols one byte apart rather than one 2-byte table.
extern u8 D_801DEF08[];
extern u8 D_801DEF09[];
// The 15 save-slot records the file-select screen draws from, stride 0x3F6.
extern u8 D_801DF120[];
// The 0x200-byte memory-card file header being assembled: "SC", version, block
// count, the 0x40-byte title, then the play-clock digits at +0x1A and +0x20.
extern u8 D_801E6D38;
extern u8 D_801E6D39;
extern u8 D_801E6D3A;
extern u8 D_801E6D3B;
extern u8 D_801E6D52;
extern u8 D_801E6D53;
extern u8 D_801E6D54;
extern u8 D_801E6D55;
extern u8 D_801E6D58;
extern u8 D_801E6D59;
extern u8 D_801E6D5A;
extern u8 D_801E6D5B;
extern u8 D_801E6D98[];

s32 func_80023788(s32);
s32 func_8002382C(s32);

// Write the current game to one memory-card file. Builds the card's own
// 0x200-byte header block -- magic, block count, the title the caller passes,
// and the play clock rendered as four glyph indices -- copies the slot's
// record in behind it, checksums the live Savemap into D_801E7138, then
// deletes any existing file and creates a fresh one. Every step is retried
// 30 times because the card answers slowly after an insert.
//
// 1 = could not create the file, 2 = could not reopen it for writing,
// 3 = gave up part-way through the write, 0 = saved.
// 20 changed / 4 inserted at the exact 395 instructions, from a cold m2c seed
// at 90 rows. `insn_histogram.py` is down to one fault: `lui +1 / ori -1`,
// which is one extra materialisation of `&Savemap` (the per-address table
// names it), so the program is right and what is left is where three
// addresses live.
//
// The four residue clusters, and what each one is:
//
//  * `addiu s0,s0,0x5f` one slot earlier than the target's, against the
//    `li v0,0x1b` beside it. Pure sched2 ordering of two independent insns.
//  * the play-clock base: the target reuses $s0 (freed by the header walk)
//    for `&Savemap.header.time` and reads it twice, early; this build keeps
//    it in $s1 because the second read is *not* hoisted above the digit
//    stores. The reason is aliasing, and it is the whole knot: the target
//    stores the first digit through a base register (`sb v1,0(s0)`), which
//    only happens if `&D_801E6D52` has a second symbol reference for cse to
//    relate -- and writing that store through a pointer (`*p = ...`) is
//    exactly what stops the clock load floating above it, because a store
//    through a pointer may alias anything while a store to a plain `extern`
//    may not. So the two halves are mutually exclusive in every spelling
//    measured: `*p = ...` gives the base register and costs the hoist
//    (24 rows), `D_801E6D52 = ...` gives the hoist and rebuilds the address
//    (68). The original has both, which means it reaches `&D_801E6D52 + 0x66`
//    some way this note has not found.
//  * the reciprocal constant (0xCCCCCCCD, gcc's divide-by-10) in $s2 rather
//    than $s1 -- a consequence of the clock base taking $s1.
//  * `&Savemap` materialised once here and never in the target, which derives
//    it as `addiu a2,s0,-0x4` off the `base` local below.
//
// Measured and rejected, in rough order of how much each was worth:
//   split the header pointer into a base (`r`, block-local, dies at the
//     func_801D21B8 call) and a walker (`q`)                    103 -> 93
//   `u16 digit` for the two BCD digits                           93 -> 30
//     (`u8` is 93, `s8` 66, `s16`/`s32`/`u32` 88..92; the width sweep found
//      it and nothing in the target predicts it, since the mask the target
//      emits is `andi 0xff`)
//   `base` = &Savemap.header.leader_level, assigned *before* the 0x200-byte
//     memcpy, so sched2 issues `move a1,s0` ahead of the D_80062D99 store
//     and the checksum call's delay slot stays the `nop` the target has
//                                                                30 -> 24
//   the clear loop on its own counter rather than on `retries`   90 -> 69
//   deriving the checksum store and the second memcpy from `base` as well
//     (`*(u32*)(base - 4)`, `base - 4`), which is what the target's
//     `sw v0,-4(s0)` / `addiu a2,s0,-0x4` / `addiu t0,s0,0x10ec` look like:
//     39, because `base` is then assigned early enough that its
//     materialisation is hoisted above the memcpy
//   `t % 10` against `t - (t / 10) * 10`: identical
//   declaration order: flat over eight permutations, so `allocno_compare`'s
//     tie-break says these priorities were never tied
//   a `s32*` local for the clock, with and without the assignment folded
//     into the first read: exactly inert
//   empty `do { } while (0);` barriers at six points around the checksum
//     call: inert or worse (101, 111), so that residue is allocation and not
//     scheduling
//   `p = &D_801E6D52` assigned at four different points, and the memcpy
//     destination spelled off D_801E6D53/D_801E6D58 instead: all flat
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/menu/nonmatchings/savemenu", func_801D2408);
#else
s32 func_801D2408(s8* path, u8* title) {
    // Never touched; the original reserved 0x200 bytes below the outgoing
    // argument area, which is exactly the size of the header block this
    // function assembles in .bss. Its identity is not recoverable.
    u8 unusedLocals[0x200];
    u8* p;
    u8* q;
    s32 fd;
    s32 retries;
    s32 n;
    u8 t;
    u8 idx;
    u16 digit;
    u8* r;
    u8* base;
    s32 i;

    func_801D224C();
    D_801E8F40 = 0x2000;
    r = &D_801E6D38;
    *r = 0x53;         // 'S'
    D_801E6D39 = 0x43; // 'C'
    D_801E6D3A = 0x11;
    D_801E6D3B = 1; // one block
    func_801D21B8(r + 4, title);
    q = r + 0x5F;
    for (i = 0x1B; i >= 0; i--) {
        *q-- = 0;
    }

    t = func_80023788(Savemap.header.time);
    digit = t / 10;
    idx = digit * 2 + 0x20;
    p = &D_801E6D52;
    *p = D_801DEF08[idx];
    D_801E6D53 = D_801DEF09[idx];
    digit = t % 10;
    idx = digit * 2 + 0x20;
    D_801E6D54 = D_801DEF08[idx];
    D_801E6D55 = D_801DEF09[idx];
    p += 0x66;

    t = func_8002382C(Savemap.header.time);
    digit = t / 10;
    idx = digit * 2 + 0x20;
    D_801E6D58 = D_801DEF08[idx];
    D_801E6D59 = D_801DEF09[idx];
    digit = t % 10;
    idx = digit * 2 + 0x20;
    D_801E6D5A = D_801DEF08[idx];
    D_801E6D5B = D_801DEF09[idx];

    memcpy(D_801E6D98, &D_801DF120[D_801E3D50 * 0x3F6], 0x20);
    memcpy(p, &D_801DF120[D_801E3D50 * 0x3F6 + 0x2C], 0x80);
    base = &Savemap.header.leader_level;
    memcpy(D_801E6F38, &D_801E6D38, 0x200);

    D_80062D99 = 1;
    Savemap.header.checksum = func_801D1950(0x10F0, base);
    memcpy(D_801E7138, &Savemap, sizeof(SaveWork));
    D_80062D99 = 0;

    retries = 0;
    do {
        fd = open(path, 1);
        retries++;
        if (fd != -1) {
            delete (path);
            close(fd);
            break;
        }
    } while (retries < 10);

    retries = 0x1E;
    do {
        fd = open(path, (D_801E6D3B << 16) | 0x200);
        if (fd != -1) {
            goto created;
        }
        retries--;
    } while (retries != 0);
    return 1;
created:
    close(fd);
    retries = 0x1E;
    do {
        fd = open(path, 2);
        if (fd != -1) {
            goto opened;
        }
        retries--;
    } while (retries != 0);
    return 2;
opened:
    retries = 0x1E;
    do {
        n = write(fd, D_801E6F38, D_801E8F40);
        if (n == D_801E8F40) {
            goto ok;
        }
        retries--;
        if (n != -1) {
            D_801E8F40 -= n;
        }
    } while (retries != 0);
    close(fd);
    return 3;
ok:
    close(fd);
    return 0;
}
#endif

const char* D_801E2CB8[] = {
    "ＦＦ７／ＳＡＶＥ０１／００：００", "ＦＦ７／ＳＡＶＥ０２／００：００",
    "ＦＦ７／ＳＡＶＥ０３／００：００", "ＦＦ７／ＳＡＶＥ０４／００：００",
    "ＦＦ７／ＳＡＶＥ０５／００：００", "ＦＦ７／ＳＡＶＥ０６／００：００",
    "ＦＦ７／ＳＡＶＥ０７／００：００", "ＦＦ７／ＳＡＶＥ０８／００：００",
    "ＦＦ７／ＳＡＶＥ０９／００：００", "ＦＦ７／ＳＡＶＥ１０／１１：１１",
    "ＦＦ７／ＳＡＶＥ１１／１１：１１", "ＦＦ７／ＳＡＶＥ１２／１１：１１",
    "ＦＦ７／ＳＡＶＥ１３／１１：１１", "ＦＦ７／ＳＡＶＥ１４／１１：１１",
    "ＦＦ７／ＳＡＶＥ１５／１１：１１",
};

static s16 func_801D2A34(s32 save_id) {
    char path[0x40];
    s32 ret;
    s32 slot;

    if (save_id & 0x10) {
        sprintf(path, D_801D018C, D_801E2C78[save_id & 15]);
    } else {
        sprintf(path, D_801D0194, D_801E2C78[save_id & 15]);
    }
    slot = save_id & 15;
    D_801E3D50 = slot;
    ret = func_801D2408(path, D_801E2CB8[slot]);
    if (!(s16)ret) {
        memcpy(&D_801E3864[slot], &Savemap.header, sizeof(SaveHeader));
    }
    return ret;
}

s32 D_801E2CF4 = 0xFF;                                       // used by title.c
StartMenuMode g_MenuStartMode = START_MENU_MODE_SELECT_SLOT; // used by title.c

unsigned char D_801E2CFC[][0x24] = {
    _S("Load"),
    _S("Select a slot."),
    _S("Select a file."),
    _S("SLOT 1"),
    _S("SLOT 2"),
    _S("Are you sure?"),
    _S("Loading. Do not remove Memory card."),
    _S("Saving. Do not remove Memory card."),
    _S("EMPTY"),
    _S("FILE"),
    _S("Continue?"),
    _S("/15"),
    _S("Checking Memory card."),
    _S("01"),
    _S("02"),
    _S("03"),
    _S("04"),
    _S("05"),
    _S("06"),
    _S("07"),
    _S("08"),
    _S("09"),
    _S("10"),
    _S("11"),
    _S("12"),
    _S("13"),
    _S("14"),
    _S("15"),
    _S("Saved."),
    _S("Could not save."),
    _S("Could not load."),
    _S("File is ruined."),
    _S("NEW GAME"),
    _S("Are you sure you want to save?"),
    _S("Yes"),
    _S("No"),
    _S(""),
    _S("Completed."),
};
static u32 _padding[] = {0, 0, 0};
unsigned char D_801E3260[][0x30] = {
    _S(""),
    _S(""),
    _S("Formatted."),
    _S("Could not format."),
    _S("Not formatted."),
    _S("Want to format it now?"),
    _S("No Memory card."),
};
unsigned char D_801E33B0[][0x30] = {
    _S("No Memory card."),
    _S("This Memory card is damaged and cannot be used."),
    _S("Please insert another Memory card."),
    _S("No enough memory left on Memory card."),
    _S("Use another Memory card,"),
    _S("or erase 1 block of saved data."),
    _S(""),
    _S(""),
    _S("Couldn't read it."),
    _S("Still want to begin the game?"),
    _S("‘’"),
    _S("‘’"),
    _S("‘’"),
    _S(""),
};
