//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 5 of 5, split out of field.c. .rodata 0x800A0F10-0x800A1368, base 0
 * mod 8. Two compiler-generated jump tables in this unit genuinely need the 4
 * bytes of padding
 * `.align 3` inserts, so this unit must stay phase 0. */

/////////////////////////////////////////////////
// Begin of field_opcode_layer.c
/////////////////////////////////////////////////

s32 OpcodeFuncBgscr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgscr", 8);
    }
    switch (GET_PARAM_U8(2)) {
    case 2:
        g_FieldState->layer2_bgScrollXSpeed = FieldEventReadMemoryS16(1, 3);
        g_FieldState->layer2_bgScrollYSpeed = FieldEventReadMemoryS16(2, 5);
        break;
    case 3:
        g_FieldState->layer3_bgScrollXSpeed = FieldEventReadMemoryS16(1, 3);
        g_FieldState->layer3_bgScrollYSpeed = FieldEventReadMemoryS16(2, 5);
        break;
    }
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncBgdph(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgdph", 8);
    }
    switch (GET_PARAM_U8(2)) {
    case 2:
        g_FieldState->layer2_depth = FieldEventReadMemoryS16(1, 3);
        break;
    case 3:
        g_FieldState->layer3_depth = FieldEventReadMemoryS16(1, 3);
        break;
    }
    PC_INC(5);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_materia.c
/////////////////////////////////////////////////

s32 OpcodeFuncSmtra(void) {
    u32 materia;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("smtra", 6);
    }
    materia = FieldEventReadMemoryU8(1, 3);
    materia |= FieldEventReadMemoryU8(2, 4) << 8;
    materia |= FieldEventReadMemoryU8(3, 5) << 16;
    materia |= FieldEventReadMemoryU8(4, 6) << 24;
    if (func_8002542C(materia) == -1) {
        D_8009D5A7 = 0;
    } else {
        D_8009D5A7 = 1;
    }
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncDmtra(void) {
    u32 materia;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dmtra", 7);
    }
    materia = FieldEventReadMemoryU8(1, 3);
    materia |= FieldEventReadMemoryU8(2, 4) << 8;
    materia |= FieldEventReadMemoryU8(3, 5) << 16;
    materia |= FieldEventReadMemoryU8(4, 6) << 24;
    func_80025648(materia, GET_PARAM_U8(7));
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncCmtra(void) {
    u32 materia;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cmtra", 8);
    }
    materia = FieldEventReadMemoryU8(1, 4);
    materia |= FieldEventReadMemoryU8(2, 5) << 8;
    materia |= FieldEventReadMemoryU8(3, 6) << 16;
    materia |= FieldEventReadMemoryU8(4, 7) << 24;
    FieldEventWriteMemoryU8(6, 9, func_80025650(materia, GET_PARAM_U8(8)));
    PC_INC(10);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_menu.c
/////////////////////////////////////////////////

/* MENU: hand the field over to a menu, a movie or another whole-screen event,
 * then poll until it reports back. Returning 1 re-runs the opcode next frame.
 *
 * `D_800A0848` is the string "evt cmd=", and it has to be spelled as an extern
 * rather than written as a literal: it lives in field4.c's .rodata, where
 * OpcodeFuncMjump owns it, while this function is in field5.c. The retail
 * object files shared it the same way -- what the split cannot reproduce is a
 * literal emitted into another unit's .rodata, so the reference is written as
 * what it links to. */
s32 OpcodeFuncMenu(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("menu", 3);
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(D_800A0848, g_FieldState->eventCmd, 2);
    }

    if (g_FieldState->eventCmd == EVTCMD_NONE) {
        g_FieldState->eventCmd = GET_PARAM_U8(2);
        g_FieldState->eventCmdParam = FieldEventReadMemoryU8(2, 3);
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        D_8007EBE0 = 1;
        if (g_FieldState->eventCmd == EVTCMD_PARTY_MENU &&
            g_FieldState->eventCmdParam == 0) {
            PC_INC(4);
        }
        return 1;
    }

    if (g_FieldState->eventCmd == GET_PARAM_U8(2)) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2(
                D_800A08D0, g_FieldState->movieCommandState, 1);
        }
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(4);
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PartyFromSaveToBank2();
            return 0;
        }
    } else if (GET_PARAM_U8(2) == EVTCMD_UNK14 &&
               g_FieldState->eventCmd == EVTCMD_PLAY_MOVIE) {
        g_FieldState->eventCmd = GET_PARAM_U8(2);
        g_FieldState->movieCommandState = MOVCMD_IDLE;
    }
    return 1;
}

s32 OpcodeFuncMenu2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("menu", 1);
    }
    g_FieldState->menuDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncGetpc(void) {
    s32 slot;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("getpc", 3);
    }
    slot = GET_PARAM_U8(2);
    if (slot < 3) {
        FieldEventWriteMemoryU8(2, 3, D_8009CBDC[slot]);
    }
    PC_INC(4);
    return 0;
}

/* MPARA: bind one of a window's replaceable text parameters to a memory bank
 * slot, so the window redraws with the current value of that variable. */
s32 OpcodeFuncMpara(void) {
    s32 window;
    s32 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpara", 4);
    }
    window = GET_PARAM_U8(2);
    param = FieldEventReadMemoryU8(1, 3);
    g_WindowReplaceBank[window][param] = GET_PARAM_U8(1) & 0xF;
    g_WindowReplaceBankAddr[window][param] = GET_PARAM_U8(4);
    PC_INC(5);
    return 0;
}

/* MPRA2: as MPARA, but the bound address is 16-bit. Writing it through
 * GET_PARAM_S16 stores the low byte and then the combined halfword, which is
 * why the same slot is written twice. */
s32 OpcodeFuncMpra2(void) {
    s32 window;
    s32 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpra2", 5);
    }
    window = GET_PARAM_U8(2);
    param = FieldEventReadMemoryU8(1, 3);
    g_WindowReplaceBank[window][param] = GET_PARAM_U8(1) & 0xF;
    GET_PARAM_S16(g_WindowReplaceBankAddr[window][param], 4);
    PC_INC(6);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_angle.c
/////////////////////////////////////////////////

/**
 * @brief Opcode 0xD4 - **SIN** - sine
 *
 * Memory layout:
 *
 * | 0xD4 | B1 / B2 | B3 / B4 | D | M | A | S |
 *
 * - const Bit[4] B1: Destination bank.
 * - const Bit[4] B2: Bank to retrieve M, or zero if M is specified as a literal
 * value.
 * - const Bit[4] B3: Bank to retrieve A, or zero if A is specified as a literal
 * value.
 * - const Bit[4] B4: Bank to retrieve S, or zero if S is specified as a literal
 * value.
 * - const UByte D: Destination address.
 * - const UByte M: Multiplicand, or address to retrieve value if B2 is
 * non-zero.
 * - const UByte A: Addition, or address to retrieve value if B3 is non-zero.
 * - const UByte S: Variable for sin angle, or source address to retrieve value
 * if B4 is non-zero.
 * @details
 * Creates a variable from the another variable, with SIN, a multiplicand and an
 * addition factor
 */
s32 OpcodeFuncSin(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sin", 8);
    }

    result = rsin(FieldEventReadMemoryS16(1, 3));
    result *= FieldEventReadMemoryS16(2, 5);
    result += FieldEventReadMemoryS16(3, 7);

    FieldEventWriteMemoryS16(4, 9, (s16)(result >> 12));

    PC_INC(10);
    return 0;
}

/**
 * @brief Opcode 0xD4 - **COS** - cosine
 *
 * Memory layout:
 *
 * | 0xD5 | B1 / B2 | B3 / B4 | D | M | A | S |
 *
 * - const Bit[4] B1: Destination bank.
 * - const Bit[4] B2: Bank to retrieve M, or zero if M is specified as a literal
 * value.
 * - const Bit[4] B3: Bank to retrieve A, or zero if A is specified as a literal
 * value.
 * - const Bit[4] B4: Bank to retrieve S, or zero if S is specified as a literal
 * value.
 * - const UByte D: Destination address.
 * - const UByte M: Multiplicand, or address to retrieve value if B2 is
 * non-zero.
 * - const UByte A: Addition, or address to retrieve value if B3 is non-zero.
 * - const UByte S: Variable for sin angle, or source address to retrieve value
 * if B4 is non-zero.
 * @details
 * Creates a variable from the another variable, with COS, a multiplicand and an
 * addition factor
 */
s32 OpcodeFuncCos(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cos", 8);
    }

    result = rcos(FieldEventReadMemoryS16(1, 3));
    result *= FieldEventReadMemoryS16(2, 5);
    result += FieldEventReadMemoryS16(3, 7);

    FieldEventWriteMemoryS16(4, 9, (s16)(result >> 12));

    PC_INC(10);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_party_stats.c
/////////////////////////////////////////////////

void SystemRefreshParty(void) {
    s16 i;

    for (i = 0; i < 3; i++) {
        if (D_8009CBDC[i] != 0xFF) {
            func_80020058(i);
            func_8001786C(i);
        }
    }
    func_80017678();
}

void SystemResoreParty(void) {
    s32 i;
    s32 charId;

    SystemRefreshParty();
    for (i = 0; i < 3; i++) {
        SystemMenuAddHpByPartyId(i, 10000);
        SystemMenuAddMpByPartyId(i, 10000);
        if (D_8009CBDC[i] != 0xFF) {
            charId = g_BattleCharIdToCharId[D_8009CBDC[i]];
            if (charId < 9) {
                Savemap.party[charId].status_flags = 0;
            }
        }
    }
}

/* MHMMX: debug helper that walks the three fixed party line-ups (characters
 * 0-2, 3-5, 6-8) through SystemResoreParty, then puts the real party back. */
s32 OpcodeFuncMhmmx(void) {
    u8 saved[3];
    u8* slot;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mhmmx", 0);
    }
    for (i = 0; i < 3; i++) {
        saved[i] = D_8009CBDC[i];
    }
    for (i = 2, slot = &D_8009CBDC[2]; i >= 0; i--) {
        *slot-- = i;
    }
    SystemResoreParty();
    for (i = 2; i >= 0; i--) {
        D_8009CBDC[i] = i + 3;
    }
    SystemResoreParty();
    for (i = 2; i >= 0; i--) {
        D_8009CBDC[i] = i + 6;
    }
    SystemResoreParty();
    for (i = 0; i < 3; i++) {
        D_8009CBDC[i] = saved[i];
    }
    SystemResoreParty();
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncHmpmx(void) {
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("hmpmx", 0);
    }
    SystemRefreshParty();
    for (i = 0; i < 3; i++) {
        SystemMenuAddHpByPartyId(i, 10000);
        SystemMenuAddMpByPartyId(i, 10000);
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncMpPlus(void) {
    s32 partyId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mp+", 4);
    }
    SystemRefreshParty();
    partyId = GET_PARAM_U8(2);
    if (D_8009D391[partyId] != 0xFF) {
        partyId = D_8009D391[partyId];
        for (i = 0; i < 3; i++) {
            if (D_8009CBDC[i] == partyId) {
                SystemMenuAddMpByPartyId(i, FieldEventReadMemoryS16(2, 3));
            }
        }
    }
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncMpMinus(void) {
    s32 partyId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mp-", 4);
    }
    SystemRefreshParty();
    partyId = GET_PARAM_U8(2);
    if (D_8009D391[partyId] != 0xFF) {
        partyId = D_8009D391[partyId];
        for (i = 0; i < 3; i++) {
            if (D_8009CBDC[i] == partyId) {
                func_80025988(i, FieldEventReadMemoryS16(2, 3));
            }
        }
    }
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncHpPlus(void) {
    s32 partyId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("hp+", 4);
    }
    SystemRefreshParty();
    partyId = GET_PARAM_U8(2);
    if (D_8009D391[partyId] != 0xFF) {
        partyId = D_8009D391[partyId];
        for (i = 0; i < 3; i++) {
            if (D_8009CBDC[i] == partyId) {
                SystemMenuAddHpByPartyId(i, FieldEventReadMemoryS16(2, 3));
            }
        }
    }
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncHpMinus(void) {
    s32 partyId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("hp-", 4);
    }
    SystemRefreshParty();
    partyId = GET_PARAM_U8(2);
    if (D_8009D391[partyId] != 0xFF) {
        partyId = D_8009D391[partyId];
        for (i = 0; i < 3; i++) {
            if (D_8009CBDC[i] == partyId) {
                func_80025800(i, FieldEventReadMemoryS16(2, 3));
            }
        }
    }
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x39 - **GOLDU** - Gold Up
 *
 * Memory layout:
 *
 * | 0x39 | 0 | A |
 *
 * Increase by a constant amount:
 * - const UByte 0: Zero.
 * - const ULong A: Amount to increase.
 *
 * Increase by an amount found in memory:
 * - const Bit[4] B: Source bank.
 * - const UByte A: Source address.
 * - const UByte[3] 0: Three zero bytes.
 * @details
 * Increases the amount of gil by a constant amount, or by an amount found in
 * the source bank B and address A. The total gil is capped above by 0xFFFFFFFF;
 * attempts to increment further will fail.
 */
s32 OpcodeFuncGoldPlus(void) {
    u32 gold;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gold+", 5);
    }
    gold = (u16)FieldEventReadMemoryS16(1, 2);
    gold |= (u16)FieldEventReadMemoryS16(2, 4) << 16;
    SystemMenuAddPartyGold(gold);
    PC_INC(6);
    return 0;
}

/**
 * @brief Opcode 0x3A - **GOLDD** - Gold Down
 *
 * Memory layout:
 *
 * | 0x3A | 0 | A |
 *
 * Decrease by a constant amount:
 * - const UByte 0: Zero.
 * - const ULong A: Amount to decrease
 *
 * Decrease by an amount found in memory:
 * - const Bit[4] B: Source bank.
 * - const UByte A: Source address.
 * - const UByte[3] 0: Three zero bytes.
 * @details
 * Decreases the amount of gil by a constant amount, or by an amount found in
 * the source bank B and address A. The total gil is capped below by 0; attempts
 * to decrement further will fail.
 */
s32 OpcodeFuncGoldMinus(void) {
    u32 gold;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gold-", 5);
    }
    gold = (u16)FieldEventReadMemoryS16(1, 2);
    gold |= (u16)FieldEventReadMemoryS16(2, 4) << 16;
    SystemMenuRemovePartyGold(gold);
    PC_INC(6);
    return 0;
}

/**
 * @brief Opcode 0x3B - **CHGLD** - Change Gold
 *
 * Memory layout:
 *
 * | 0x3B | B1 / B2 | A1 | A2 |
 *
 * - const Bit[4] B1: Destination bank 1.
 * - const Bit[4] B2: Destination bank 2.
 * - const UByte A1: Destination address 1.
 * - const UByte A2: Destination address 2.
 * @details
 * Copies the amount of gil the party has into the destination addresses.
 * As the gil amount is a four-byte value, the arguments require two destination
 * addresses to place two two-byte values into. Address 1 takes the lower two
 * bytes of the gil amount, while address 2 takes the higher two bytes.
 */
s32 OpcodeFuncChgld(void) {
    u32 partyGold;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("chgld", 3);
    }
    partyGold = SystemMenuGetPartyGold();
    FieldEventWriteMemoryS16(1, 2, (u16)partyGold);
    FieldEventWriteMemoryS16(2, 3, (u16)(partyGold >> 16));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncChmph(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("chmph", 3);
    }
    FieldEventWriteMemoryS16(1, 2, D_8009A162);
    FieldEventWriteMemoryU8(2, 3, D_8009A15C);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncChmst(void) {
    u8 state;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("chmst", 2);
    }
    state = D_8009A108 != 0;
    if (D_80099FCC[0] != 0) {
        state |= 2;
    }
    FieldEventWriteMemoryU8(2, 2, state);
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_window_timer.c
/////////////////////////////////////////////////

s32 OpcodeFuncSttim(void) {
    s32 time;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sttim", 5);
    }

    time = FieldEventReadMemoryU8(1, 3) * 60 * 60;
    time += FieldEventReadMemoryU8(2, 4) * 60;
    time += FieldEventReadMemoryU8(4, 5);
    Savemap.countdown_timer_seconds = time;

    PC_INC(6);
    return 0;
}

s32 OpcodeFuncWspcl(void) {
    u8 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wspcl", 5);
    }

    window = GET_PARAM_U8(1);
    g_WindowData[window].numDisplayType = GET_PARAM_U8(2);
    g_WindowData[window].numDisplayX = GET_PARAM_U8(3);
    g_WindowData[window].numDisplayY = GET_PARAM_U8(4);

    PC_INC(5);
    return 0;
}

s32 OpcodeFuncWnumb(void) {
    u8 window;
    s32 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wnumb", 7);
    }

    window = GET_PARAM_U8(2);
    value = FieldEventReadMemoryS16(1, 3);
    value |= FieldEventReadMemoryS16(2, 5) << 16;
    g_WindowData[window].numDisplayValue = value;
    g_WindowData[window].numDisplayLength = GET_PARAM_U8(7);

    PC_INC(8);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_battle.c
/////////////////////////////////////////////////

s32 OpcodeFuncBtlmd(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btlmd", 2);
    }

    g_FieldState->battleMode2 = GET_PARAM_U8(1);
    g_FieldState->battleMode1 = GET_PARAM_U8(2);

    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBtmd2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btmd2", 2);
    }

    g_FieldState->battleMode2 = GET_PARAM_U8(1);
    g_FieldState->battleMode2 |= GET_PARAM_U8(2) << 8;
    g_FieldState->battleMode1 = GET_PARAM_U8(3);
    g_FieldState->battleMode1 |= GET_PARAM_U8(4) << 8;

    PC_INC(5);
    return 0;
}

s32 OpcodeFuncBtrlt(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btrlt", 2);
    }

    FieldEventWriteMemoryS16(2, 2, g_BattleMode);

    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBtltb(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btltb", 1);
    }

    g_FieldState->encounterTableId = GET_PARAM_U8(1);

    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_kawai_char.c
/////////////////////////////////////////////////

s32 OpcodeFuncBlink(void) {
    u8 modelId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("blink", 8);
    }

    modelId = g_EntityToModel[g_CurrentEntity];
    if (modelId != 0xFF) {
        g_FieldModels[modelId].BlinkOn = GET_PARAM_U8(1);
    }

    PC_INC(2);
    return 0;
}

s32 OpcodeFuncKawai(void) {
    u16 size;
    u8 modelId;
    u8 kawaiType;
    u8 type;
    u8* params;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("kawai", 8);
    }

    size = GET_PARAM_U8(1);
    modelId = g_EntityToModel[g_CurrentEntity];
    if (modelId != 0xFF) {
        kawaiType = GET_PARAM_U8(2);
        g_FieldModelData
            ->modelEntries[g_FieldModelLoaderData[modelId].modelEntryIndex]
            .kawaiType = kawaiType;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiOp1 = 1;
        type = kawaiType;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiOp0 = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiDataOffset =
            &GET_PARAM_U8(3);
    }

    if (type == 0) {
        params =
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiDataOffset;
        if (params[0] == 1 && params[1] == params[0] && params[2] == 0) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].BlinkOn = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiA = 0;
        } else {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].BlinkOn = 1;
        }
    }

    PC_INC(size);
    return 0;
}

s32 OpcodeFuncKawiw(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("kawiw", 0);
    }

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiOp1 != 1) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].KawaiOp1 = 0;
            PC_INC(1);
            return 0;
        }
        return 1;
    }

    PC_INC(1);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_dialog.c
/////////////////////////////////////////////////

static void DebugDummyFunc(void) {}

void DebugPrintToFieldWindow(const char* str) {
    // used to print debug messages -- dummied out on release
}

void FieldEventDebugError(const char* errmsg) {
    FieldDebugPageInit(0, 100, 100, 150, 12);
    FieldDebugPageSetColor(0, 0x7F, 0, 0);
    AddStrNextDebugRow(0, errmsg);
    D_80095DCC = 1;
    g_FieldScriptRunState = 4;
}

void FieldWindowResetAll(void) {
    s32 i;

    g_WindowCount = 0;
    for (i = 0; i < 4; i++) {
        FieldWindowReset(i);
    }
    if (g_FieldScripts->stringOffset != 0) {
        g_FieldText = (u8*)g_FieldScripts + g_FieldScripts->stringOffset;
    } else {
        g_FieldText = NULL;
    }
}

void FieldWindowReset(s16 window) {
    s32 i;

    if (window == 1) {
        g_WindowData[window].y = 8;
    } else {
        g_WindowData[window].y = 149;
    }

    g_WindowData[window].x = 8;
    g_WindowData[window].width = 304;
    g_WindowData[window].height = 73;
    g_WindowData[window].currentWidth = 1;
    g_WindowData[window].currentHeight = 1;
    g_WindowData[window].state = WSTATE_INIT;
    g_WindowData[window].style = WSTYLE_NORMAL;
    g_WindowData[window].numDisplayType = WNDT_OFF;
    g_WindowData[window].unk1C = 0;
    g_WindowData[window].numDisplayLength = 6;
    g_WindowData[window].numDisplayX = 0;
    g_WindowData[window].numDisplayY = 0;
    g_WindowData[window].preventClose = 0;
    g_WindowToEntity[window] = 0xFF;

    for (i = 0; i < 8; i++) {
        g_WindowReplaceBank[window][i] = 0;
        g_WindowReplaceBankAddr[window][i] = 0;
    }

    g_WindowWaitTime[window] = 0;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("mes reset=", window, 1);
    }
}

s32 FieldWindowSetStateToClose(s16 window) {
    switch (g_WindowData[window].state) {
    case WSTATE_SHOW:
        return 0;
    case WSTATE_TXT:
    case WSTATE_WAIT_ROW:
    case WSTATE_TXT_DONE:
    case WSTATE_SCROLL_ROW:
    case WSTATE_PAUSE_TXT_SCROLL_UNTIL_OK:
    case WSTATE_PAUSE_TXT_UNTIL_OK:
        g_WindowData[window].state = WSTATE_CLOSING;
    }
    return 1;
}

void FieldDialogSetWindowStyleCbc(s16 window, u8 style, s16 preventClose) {
    g_WindowData[window].style = style;
    g_WindowData[window].preventClose = preventClose;
}

void FieldWindowResetTextAll(void) {
    s32 i;

    for (i = 0; i < 4; i++) {
        g_WindowData[i].state = WSTATE_INIT;
        g_WindowData[i].stringLength = 0;
        g_WindowToEntity[i] = 0xFF;
        g_WindowWaitTime[i] = 0;
    }
    g_WindowCount = 0;
}

void FieldDialogSetSize(s16 window, s16 x, s16 y, s16 width, s16 height) {
    if (x < 8) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("win limit x=", x, 2);
        }
        x = 8;
    }
    if (x + width > 312) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("win limit x=", x + width, 3);
        }
        x = 312 - width;
    }
    if (y < 8) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("win limit y=", y, 2);
        }
        y = 8;
    }
    if (y + height > 224) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("win limit y=", y + height, 3);
        }
        y = 224 - height;
    }

    g_WindowData[window].x = x;
    g_WindowData[window].y = y;
    g_WindowData[window].width = width;
    g_WindowData[window].height = height;
}

void FieldDialogMove(s16 window, s16 dx, s16 dy) {
    g_WindowData[window].x += dx;
    g_WindowData[window].y += dy;
}

void FieldDialogSetWindowHeight(s16 window, s16 height) {
    g_WindowData[window].height = height;
}

s32 FieldDialogMessageUpdateStates(u8 window, u8 message) {
    switch (g_WindowData[window].state) {
    case WSTATE_INIT:
        if (FieldDialogWindowInit(window, message)) {
            return 1;
        }
        break;
    case WSTATE_SHOW:
        FieldDialogWindowGrowth(window);
        break;
    case WSTATE_TXT:
        FieldDialogCopyTextFromField(window);
        break;
    case WSTATE_SCROLL_ROW:
        DialogScrollText(window);
        break;
    case WSTATE_SCROLL_TXT_WHILE_OK:
        DialogScrollTextDuringOk(window);
        break;
    case WSTATE_PAUSE_TXT_UNTIL_OK:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            g_WindowData[window].state = WSTATE_TXT;
        }
        break;
    case WSTATE_PAUSE_TXT:
        if (g_WindowWaitTime[window] == 0) {
            g_WindowData[window].state = WSTATE_TXT;
        } else {
            g_WindowWaitTime[window]--;
        }
        break;
    case WSTATE_WAIT_ROW:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            if (g_WindowData[window].currentRow ==
                (g_WindowData[window].height - 9) / 16 - 1 +
                    g_WindowExtraRows[window]) {
                g_WindowData[window].state = WSTATE_SCROLL_ROW;
                g_WindowData[window].textScrolling -= 2;
                g_WindowExtraRows[window]++;
            }
        }
        break;
    case WSTATE_TXT_DONE:
        if (!(g_WindowData[window].preventClose & 1) &&
            (g_FieldState->newActiveKeys2 & PADRright)) {
            g_WindowData[window].state = WSTATE_CLOSING;
            FieldDialogWindowDecrease(window);
        }
        break;
    case WSTATE_WAIT_NEXT_WINDOW:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            FieldDialogWindowInitNext(window);
        }
        break;
    case WSTATE_PAUSE_TXT_SCROLL_UNTIL_OK:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            g_WindowData[window].state = WSTATE_SCROLL_TXT_WHILE_OK;
            g_WindowTotalRowsHeight[window] =
                g_WindowData[window].currentRow * 16 + 17;
            g_WindowData[window].textScrolling -= 2;
        }
        break;
    case WSTATE_INIT_NEXT:
        FieldDialogWindowInitNext(window);
        break;
    case WSTATE_UNK5:
    case WSTATE_CLOSING:
        if (FieldDialogWindowDecrease(window)) {
            return 1;
        }
        break;
    }

    return 0;
}

s32 FieldDialogAskUpdateStates(
    u8 window, u8 message, u8 first, u8 last, s16* selectedLine) {
    switch (g_WindowData[window].state) {

    // Clears window, sets width/height to 1/4 of what was previously set
    // with FieldDialogSetSize, and sets state = WSTATE_SHOW.
    case WSTATE_INIT:
        if (FieldDialogWindowInit(window, message)) {
            return 1;
        }
        break;

    // Increases window to full size and sets state WSTATE_TXT.
    case WSTATE_SHOW:
        FieldDialogWindowGrowth(window);
        break;

    // Renders text gradually. Can transition to other states to pause or scroll
    // text or open new window. Sets state = WSTATE_TXT_DONE when all text is
    // displayed.
    case WSTATE_TXT:
        FieldDialogCopyTextFromField(window);
        break;
    case WSTATE_SCROLL_ROW:
        DialogScrollText(window);
        break;
    case WSTATE_SCROLL_TXT_WHILE_OK:
        DialogScrollTextDuringOk(window);
        break;
    case WSTATE_PAUSE_TXT_UNTIL_OK:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            g_WindowData[window].state = WSTATE_TXT;
        }
        break;
    case WSTATE_PAUSE_TXT:
        if (g_WindowWaitTime[window] == 0) {
            g_WindowData[window].state = WSTATE_TXT;
        } else {
            g_WindowWaitTime[window]--;
        }
        break;
    case WSTATE_WAIT_ROW:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            if (g_WindowData[window].currentRow ==
                (g_WindowData[window].height - 9) / 16 - 1 +
                    g_WindowExtraRows[window]) {
                g_WindowData[window].state = WSTATE_SCROLL_ROW;
                g_WindowData[window].textScrolling -= 2;
                g_WindowExtraRows[window]++;
            }
        }
        break;

    // Displays all text and pointer for user to choose an option.
    case WSTATE_TXT_DONE:
        if (!(g_WindowData[window].preventClose & 1)) {
            g_WindowData[window].pointerEnabled = 1;

            if (g_FieldState->newActiveKeys & PADLup) {
                if (first < *selectedLine) {
                    PlayWindowPointerClickSound();
                }
                (*selectedLine)--;
            }
            if (g_FieldState->newActiveKeys & PADLdown) {
                if (*selectedLine < last) {
                    PlayWindowPointerClickSound();
                }
                (*selectedLine)++;
            }
            if (*selectedLine < first) {
                *selectedLine = first;
            }
            if (last < *selectedLine) {
                *selectedLine = last;
            }

            g_WindowData[window].pointerX = 5;
            g_WindowData[window].pointerY = *selectedLine * 16 + 6;

            // User has pressed OK to choose an option.
            if (g_FieldState->newActiveKeys2 & PADRright) {
                PlayWindowPointerClickSound();
                g_WindowData[window].state = WSTATE_CLOSING;
                FieldDialogWindowDecrease(window);
            }
        }
        break;
    case WSTATE_WAIT_NEXT_WINDOW:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            FieldDialogWindowInitNext(window);
        }
        break;
    case WSTATE_PAUSE_TXT_SCROLL_UNTIL_OK:
        if (g_FieldState->newActiveKeys2 & PADRright) {
            g_WindowData[window].state = WSTATE_SCROLL_TXT_WHILE_OK;
            g_WindowTotalRowsHeight[window] =
                g_WindowData[window].currentRow * 16 + 17;
            g_WindowData[window].textScrolling -= 2;
        }
        break;
    case WSTATE_INIT_NEXT:
        FieldDialogWindowInitNext(window);
        break;
    case WSTATE_UNK5:
    case WSTATE_CLOSING:
        if (FieldDialogWindowDecrease(window)) {
            g_WindowData[window].pointerEnabled = 0;
            return 1;
        }
        // Make pointer blink while window is closing.
        g_WindowData[window].pointerEnabled ^= 1;
        break;
    }

    return 0;
}

void PlayWindowPointerClickSound(void) {
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x30;
    D_8009A004[0] = 1;
    D_8009A008[0] = 0x40;
    SystemAkaoExecute();
}

s32 FieldDialogWindowInit(s16 window, s16 stringId) {
    if (g_FieldText == NULL) {
        FieldEventDebugError("No mes data!");
        return 1;
    }

    if (g_WindowToEntity[window] != 0xFF) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return 0;
    }

    g_WindowToEntity[window] = g_CurrentEntity;
    g_WindowData[window].currentWidth = g_WindowData[window].width / 4;
    g_WindowData[window].currentHeight = g_WindowData[window].height / 4;
    if (g_WindowData[window].currentHeight < 8) {
        g_WindowData[window].currentHeight = 8;
    }
    if (g_WindowData[window].currentWidth < 8) {
        g_WindowData[window].currentWidth = 8;
    }

    g_WindowData[window].text = g_WindowString[window];
    g_WindowData[window].textScrolling = 0;
    g_WindowData[window].stringLength = 0;
    g_WindowData[window].stringByteLength = 0;
    g_WindowData[window].currentRow = 0;
    g_WindowData[window].pointerEnabled = 0;
    g_WindowString[window][0] = 0xFF;

    g_WindowStringPtr[window] = g_FieldText;
    g_WindowStringPtr[window] += g_FieldText[stringId * 2 + 2];
    g_WindowStringPtr[window] += g_FieldText[stringId * 2 + 3] << 8;

    g_WindowCount++;
    g_WindowFastForwardLevel[window] = 1;
    g_WindowTextBudget[window] = 0;
    g_WindowExtraRows[window] = 0;
    g_WindowNameCopyCount[window] = 0;
    g_WindowReplaceParam[window] = 0;
    g_WindowBufferPos[window] = -1;
    g_WindowData[window].state = WSTATE_SHOW;
    return 0;
}

void FieldDialogWindowGrowth(s16 window) {
    if (g_WindowToEntity[window] != g_CurrentEntity) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return;
    }

    g_WindowData[window].currentWidth += g_WindowData[window].width / 4;
    if (g_WindowData[window].currentWidth < 8) {
        g_WindowData[window].currentWidth = 8;
    }
    if (g_WindowData[window].width < g_WindowData[window].currentWidth) {
        g_WindowData[window].currentWidth = g_WindowData[window].width;
    }

    g_WindowData[window].currentHeight += g_WindowData[window].height / 4;
    if (g_WindowData[window].currentHeight < 8) {
        g_WindowData[window].currentHeight = 8;
    }
    if (g_WindowData[window].height < g_WindowData[window].currentHeight) {
        g_WindowData[window].currentHeight = g_WindowData[window].height;
    }

    if (g_WindowData[window].currentWidth == g_WindowData[window].width &&
        g_WindowData[window].currentHeight == g_WindowData[window].height) {
        g_WindowData[window].state = WSTATE_TXT;
    }
}

void FieldDialogCopyTextFromField(s16 window) {
    u8 opcode;
    u16 len;
    s16 i;
    s16 baseCredit;
    s16 characterCost;
    u8* name;
    u16 value;

    if (g_WindowToEntity[window] != g_CurrentEntity) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return;
    }

    /*
     * To render text gradually, the game implements a text-writing credit
     * system. The > comparison in the while-loop means it can emit one
     * fewer character on the first update.
     * g_WindowFastForwardLevel can add 0-8 characters per update and increase
     * scrolling speed. It's ramped up or down based on the state of OK.
     * field_msg_speed  baseCredit  characterCost   chars/update
     * 0                6           1               6
     * 1-32             5           1               5
     * 33-64            4           1               4
     * 65-96            3           1               3
     * 97-159           2           1               2
     * 160-191          2           2               1
     * 192-223          2           3               2/3
     * 224-255          2           4               1/2
     */

    if (g_WindowData[window].preventClose & 2) {
        baseCredit = 256;
        characterCost = 1;
    } else {
        SaveWork* save;

        // Holding OK down increases text and scrolling speed.
        if (g_FieldState->activeKeys2 & PADRright) {
            g_WindowFastForwardLevel[window]++;
            if (g_WindowFastForwardLevel[window] > 128) {
                g_WindowFastForwardLevel[window] = 128;
            }
        } else {
            g_WindowFastForwardLevel[window]--;
            if (g_WindowFastForwardLevel[window] < 2) {
                g_WindowFastForwardLevel[window] = 1;
            }
        }

        save = &Savemap;
        if (save->field_msg_speed < 128) {
            baseCredit = ((128 - save->field_msg_speed) >> 5) + 2;
            characterCost = 1;
        } else {
            baseCredit = 2;
            characterCost = ((save->field_msg_speed - 128) >> 5) + 1;
        }
    }

    g_WindowTextBudget[window] +=
        characterCost * (g_WindowFastForwardLevel[window] >> 4) + baseCredit;

    while (g_WindowTextBudget[window] > characterCost) {
        switch (*g_WindowStringPtr[window]) {
        // End of string.
        case 0xFF:
            g_WindowData[window].state = WSTATE_TXT_DONE;
            g_WindowTextBudget[window] = 0;
            goto end;

        // Next row.
        // If we haven't reached max rows, add new row character and continue.
        // If we have, set state to WSTATE_WAIT_ROW.
        case 0xE7:
            if (g_WindowData[window].currentRow ==
                (g_WindowData[window].height - 9) / 16 - 1 +
                    g_WindowExtraRows[window]) {
                g_WindowData[window].state = WSTATE_WAIT_ROW;
                g_WindowFastForwardLevel[window] = 1;
                g_WindowTextBudget[window] = 0;
                goto end;
            }
            g_WindowString[window][g_WindowData[window].stringByteLength] =
                *g_WindowStringPtr[window];
            g_WindowStringPtr[window]++;
            g_WindowData[window].stringByteLength++;
            g_WindowData[window].currentRow++;
            continue;

        // Wait for next window. Stop string copying and return.
        case 0xE8:
        case 0xE9:
            g_WindowStringPtr[window]++;
            g_WindowData[window].state = WSTATE_WAIT_NEXT_WINDOW;
            g_WindowFastForwardLevel[window] = 1;
            g_WindowTextBudget[window] = 0;
            goto end;

        // Write 4 spaces.
        case 0xE1:
            g_WindowStringPtr[window]++;
            for (i = 0; i < 4; i++) {
                g_WindowString[window][g_WindowData[window].stringByteLength] =
                    0;
                g_WindowData[window].stringByteLength++;
                g_WindowData[window].stringLength++;
            }
            continue;

        // Write 10 spaces.
        case 0xE0:
            g_WindowStringPtr[window]++;
            for (i = 0; i < 10; i++) {
                g_WindowString[window][g_WindowData[window].stringByteLength] =
                    0;
                g_WindowData[window].stringByteLength++;
                g_WindowData[window].stringLength++;
            }
            continue;

        // Write ', '
        case 0xE2:
            g_WindowStringPtr[window]++;
            g_WindowString[window][g_WindowData[window].stringByteLength] = 0xC;
            g_WindowData[window].stringByteLength++;
            g_WindowString[window][g_WindowData[window].stringByteLength] = 0;
            g_WindowData[window].stringByteLength++;
            g_WindowData[window].stringLength += 2;
            continue;

        // Write '."'
        case 0xE3:
            g_WindowStringPtr[window]++;
            g_WindowString[window][g_WindowData[window].stringByteLength] = 0xE;
            g_WindowData[window].stringByteLength++;
            g_WindowString[window][g_WindowData[window].stringByteLength] = 2;
            g_WindowData[window].stringByteLength++;
            g_WindowData[window].stringLength += 2;
            continue;

        // Write '…"'
        case 0xE4:
            g_WindowStringPtr[window]++;
            g_WindowString[window][g_WindowData[window].stringByteLength] =
                0xA9;
            g_WindowData[window].stringByteLength++;
            g_WindowString[window][g_WindowData[window].stringByteLength] = 2;
            g_WindowData[window].stringByteLength++;
            g_WindowData[window].stringLength += 2;
            continue;

        // Write player-chosen character name from savemap.
        case 0xEA: // Cloud
        case 0xEB: // Barret
        case 0xEC: // Tifa
        case 0xED: // Aerith
        case 0xEE: // Red XIII
        case 0xEF: // Yuffie
        case 0xF0: // Cait Sith
        case 0xF1: // Vincent
        case 0xF2: // Cid
            value = *g_WindowStringPtr[window] - 0xEA;
            name = GetCharacterName(value);
            if (name[g_WindowNameCopyCount[window]] == 0xFF ||
                g_WindowNameCopyCount[window] >= 9) {
                g_WindowStringPtr[window]++;
                g_WindowNameCopyCount[window] = 0;
            } else {
                g_WindowString[window][g_WindowData[window].stringByteLength] =
                    name[g_WindowNameCopyCount[window]];
                g_WindowData[window].stringByteLength++;
                g_WindowNameCopyCount[window]++;
                g_WindowData[window].stringLength++;
                g_WindowTextBudget[window] -= characterCost;
            }
            continue;

        // Write name of party member.
        case 0xF3:
        case 0xF4:
        case 0xF5:
            value = Savemap.memory_bank_1[22 + *g_WindowStringPtr[window]];
            // Empty party slot. Write 9 ellipsis characters in place of name.
            if (value == 0xFF) {
                if (g_WindowNameCopyCount[window] >= 9) {
                    g_WindowStringPtr[window]++;
                    g_WindowNameCopyCount[window] = 0;
                } else {
                    g_WindowString[window]
                                  [g_WindowData[window].stringByteLength] =
                                      0xA9;
                    g_WindowData[window].stringByteLength++;
                    g_WindowNameCopyCount[window]++;
                    g_WindowData[window].stringLength++;
                    g_WindowTextBudget[window] -= characterCost;
                }
            } else {
                // Party member exists. Get player-chosen name from savemap.
                name = GetCharacterName(value);
                if (name[g_WindowNameCopyCount[window]] == 0xFF ||
                    g_WindowNameCopyCount[window] >= 9) {
                    g_WindowStringPtr[window]++;
                    g_WindowNameCopyCount[window] = 0;
                } else {
                    g_WindowString[window]
                                  [g_WindowData[window].stringByteLength] =
                                      name[g_WindowNameCopyCount[window]];
                    g_WindowData[window].stringByteLength++;
                    g_WindowNameCopyCount[window]++;
                    g_WindowData[window].stringLength++;
                    g_WindowTextBudget[window] -= characterCost;
                }
            }
            continue;

        // Opcode prefix.
        case 0xFE:
            g_WindowString[window][g_WindowData[window].stringByteLength] =
                *g_WindowStringPtr[window];
            g_WindowStringPtr[window]++;
            g_WindowData[window].stringByteLength++;
            switch (*g_WindowStringPtr[window]) {
            // Pause writing text until the player presses OK.
            case 0xDC:
                g_WindowData[window].stringByteLength--;
                g_WindowStringPtr[window]++;
                g_WindowData[window].state = WSTATE_PAUSE_TXT_UNTIL_OK;
                g_WindowFastForwardLevel[window] = 1;
                g_WindowTextBudget[window] = 0;
                goto end;

            // Pause writing and wait for OK before scrolling the text.
            case 0xE0:
                g_WindowData[window].stringByteLength--;
                g_WindowStringPtr[window]++;
                g_WindowData[window].state = WSTATE_PAUSE_TXT_SCROLL_UNTIL_OK;
                g_WindowFastForwardLevel[window] = 1;
                g_WindowTextBudget[window] = 0;
                goto end;

            // Copy an integer from a memory bank.
            case 0xDE:
            case 0xDF:
            case 0xE1:
                g_WindowData[window].stringByteLength--;
                g_WindowStringPtr[window]--;
                if (g_WindowBufferPos[window] == -1) {
                    // First iteration. Fetch and convert the value.
                    value = FieldDialogGetVariableFromBank(window);
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("mpara=", value, 4);
                    }
                    opcode = g_WindowStringPtr[window][1];
                    switch (opcode) {
                    // Integer to decimal string.
                    case 0xDE:
                        ConvertDigitToString(value, g_WindowBuffer[window]);
                        break;
                    // Integer to decimal string with space fill.
                    case 0xE1:
                        ConvertNumToStrWithSpace(value, g_WindowBuffer[window]);
                        break;
                    // Integer to hexadecimal string.
                    case 0xDF:
                        ConvertHexToString(value, g_WindowBuffer[window]);
                        break;
                    }
                    g_WindowBufferPos[window]++;
                } else if (
                    g_WindowBuffer[window][g_WindowBufferPos[window]] == 0xFF ||
                    g_WindowBufferPos[window] >= 16) {
                    // Last converted character has been copied.
                    g_WindowStringPtr[window] += 2;
                    g_WindowBufferPos[window] = -1;
                    g_WindowReplaceParam[window]++;
                } else {
                    // Copy the next character of the converted integer.
                    g_WindowString[window][g_WindowData[window]
                                               .stringByteLength] =
                        g_WindowBuffer[window][g_WindowBufferPos[window]];
                    g_WindowData[window].stringByteLength++;
                    g_WindowBufferPos[window]++;
                    g_WindowData[window].stringLength++;
                    g_WindowTextBudget[window] -= characterCost;
                }
                continue;

            // Copy a string from a memory bank.
            case 0xE2:
                g_WindowData[window].stringByteLength--;
                g_WindowStringPtr[window]--;
                if (g_WindowBufferPos[window] == -1) {
                    value = g_WindowStringPtr[window][2];
                    value |= g_WindowStringPtr[window][3] << 8;
                    len = g_WindowStringPtr[window][4];
                    len |= g_WindowStringPtr[window][5] << 8;
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("gstr=", value, 4);
                        if (g_DebugLevel & 3) {
                            FieldDebugAddParseValueToPage2("glen=", len, 4);
                        }
                    }
                    for (i = 0; i < len; i++) {
                        g_WindowBuffer[window][i] =
                            Savemap.memory_bank_1[value + i];
                    }
                    g_WindowBuffer[window][i] = 0xFF;
                    g_WindowBufferPos[window]++;
                } else if (
                    g_WindowBuffer[window][g_WindowBufferPos[window]] == 0xFF) {
                    g_WindowStringPtr[window] += 6;
                    g_WindowBufferPos[window] = -1;
                } else {
                    g_WindowString[window][g_WindowData[window]
                                               .stringByteLength] =
                        g_WindowBuffer[window][g_WindowBufferPos[window]];
                    g_WindowData[window].stringByteLength++;
                    g_WindowBufferPos[window]++;
                    g_WindowData[window].stringLength++;
                    g_WindowTextBudget[window] -= characterCost;
                }
                continue;

            // Font colors.
            case 0xD2: // Gray
            case 0xD3: // Blue
            case 0xD4: // Red
            case 0xD5: // Purple
            case 0xD6: // Green
            case 0xD7: // Cyan
            case 0xD8: // Yellow
            case 0xD9: // White
            // Special global colors.
            case 0xDA: // Flash colors
            case 0xDB: // Rainbow colors, changes color for each character
            // Toggle left padding of characters.
            case 0xE9:
                g_WindowString[window][g_WindowData[window].stringByteLength] =
                    *g_WindowStringPtr[window];
                g_WindowStringPtr[window]++;
                g_WindowData[window].stringByteLength++;
                continue;

            // Wait until g_WindowWaitTime reaches 0 before resuming.
            case 0xDD:
                g_WindowData[window].state = WSTATE_PAUSE_TXT;
                g_WindowStringPtr[window]++;
                g_WindowData[window].stringByteLength++;
                g_WindowWaitTime[window] = *g_WindowStringPtr[window];
                g_WindowStringPtr[window]++;
                g_WindowData[window].stringByteLength++;
                g_WindowWaitTime[window] |= *g_WindowStringPtr[window] << 8;
                g_WindowStringPtr[window]++;
                g_WindowData[window].stringByteLength++;
                goto end;

            default:
                g_WindowString[window][g_WindowData[window].stringByteLength] =
                    *g_WindowStringPtr[window];
                g_WindowStringPtr[window]++;
                g_WindowData[window].stringByteLength++;
                g_WindowData[window].stringLength++;
                g_WindowTextBudget[window] -= characterCost;
                continue;
            }

        // Two byte characters used in Japanese extended font(?)
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
            g_WindowString[window][g_WindowData[window].stringByteLength] =
                *g_WindowStringPtr[window];
            g_WindowStringPtr[window]++;
            g_WindowData[window].stringByteLength++;

        // Fall through to copy the second byte of the character.
        // Also used to copy all other characters directly.
        default:
            g_WindowString[window][g_WindowData[window].stringByteLength] =
                *g_WindowStringPtr[window];
            g_WindowStringPtr[window]++;
            g_WindowData[window].stringByteLength++;
            g_WindowData[window].stringLength++;
            g_WindowTextBudget[window] -= characterCost;
            continue;
        }
    }

end:
    g_WindowString[window][g_WindowData[window].stringByteLength] = 0xFF;
}

void DialogScrollText(s16 window) {
    if (g_WindowToEntity[window] != g_CurrentEntity) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return;
    }

    if (g_WindowData[window].textScrolling & 0xF) {
        g_WindowData[window].textScrolling -= 2;
    } else {
        g_WindowData[window].state = WSTATE_TXT;
    }
}

void DialogScrollTextDuringOk(s16 window) {
    if (g_WindowToEntity[window] != g_CurrentEntity) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return;
    }

    if (g_WindowData[window].textScrolling + g_WindowTotalRowsHeight[window] >
        0) {
        g_WindowData[window].textScrolling -=
            g_WindowFastForwardLevel[window] >> 2;
        if (g_FieldState->activeKeys2 & PADRright) {
            g_WindowFastForwardLevel[window]++;
            if (g_WindowFastForwardLevel[window] > 128) {
                g_WindowFastForwardLevel[window] = 128;
            }
        } else {
            g_WindowFastForwardLevel[window]--;
            if (g_WindowFastForwardLevel[window] < 2) {
                g_WindowFastForwardLevel[window] = 1;
            }
        }
    } else {
        g_WindowData[window].state = WSTATE_INIT_NEXT;
    }
}

void FieldDialogWindowInitNext(s16 window) {
    if (g_WindowToEntity[window] != g_CurrentEntity) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return;
    }

    g_WindowData[window].state = WSTATE_TXT;
    g_WindowData[window].stringByteLength = 0;
    g_WindowData[window].stringLength = 0;
    g_WindowData[window].textScrolling = 0;
    g_WindowData[window].currentRow = 0;
    g_WindowString[window][0] = 0xFF;
    g_WindowExtraRows[window] = 0;
    g_WindowFastForwardLevel[window] = 1;
}

s32 FieldDialogWindowDecrease(s16 window) {
    if (g_WindowToEntity[window] != g_CurrentEntity) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("mes busy=", window, 1);
        }
        return;
    }

    if (g_WindowData[window].currentWidth >= 8) {
        g_WindowData[window].currentWidth -= g_WindowData[window].width / 4;
    } else {
        g_WindowData[window].currentWidth = 8;
    }

    if (g_WindowData[window].currentHeight >= 8) {
        g_WindowData[window].currentHeight -= g_WindowData[window].height / 4;
    } else {
        g_WindowData[window].currentHeight = 8;
    }

    if (g_WindowData[window].currentWidth < 9 &&
        g_WindowData[window].currentHeight < 9) {
        g_WindowData[window].stringLength = 0;
        g_WindowData[window].state = WSTATE_INIT;
        g_WindowToEntity[window] = 0xFF;
        g_WindowCount--;
        return 1;
    }
    return 0;
}

u16 FieldDialogGetVariableFromBank(s16 window) {
    u16 value;
    u16 offset;

    switch (g_WindowReplaceBank[window][g_WindowReplaceParam[window]]) {
    case 0:
        value = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]];
        break;
    case 1:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]];
        value = Savemap.memory_bank_1[offset];
        break;
    case 2:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]];
        value = Savemap.memory_bank_1[offset];
        value |= Savemap.memory_bank_1[offset + 1] << 8;
        break;
    case 3:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x100;
        value = Savemap.memory_bank_1[offset];
        break;
    case 4:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x100;
        value = Savemap.memory_bank_1[offset];
        value |= Savemap.memory_bank_1[offset + 1] << 8;
        break;
    case 11:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x200;
        value = Savemap.memory_bank_1[offset];
        break;
    case 12:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x200;
        value = Savemap.memory_bank_1[offset];
        value |= Savemap.memory_bank_1[offset + 1] << 8;
        break;
    case 13:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x300;
        value = Savemap.memory_bank_1[offset];
        break;
    case 15:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x400;
        value = Savemap.memory_bank_1[offset];
        break;
    case 14:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x300;
        value = Savemap.memory_bank_1[offset];
        value |= Savemap.memory_bank_1[offset + 1] << 8;
        break;
    case 7:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]] +
                 0x400;
        value = Savemap.memory_bank_1[offset];
        value |= Savemap.memory_bank_1[offset + 1] << 8;
        break;
    case 5:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]];
        value = g_FieldMapVars[offset];
        break;
    case 6:
        offset = g_WindowReplaceBankAddr[window][g_WindowReplaceParam[window]];
        value = g_FieldMapVars[offset];
        value |= g_FieldMapVars[offset + 1] << 8;
        break;
    default:
        value = 0;
        break;
    }

    return value;
}

void ConvertDigitToString(u16 value, u8* dst) {
    u32 foundDigit;
    s16 i;
    s16 divisor;
    s16 digit;

    foundDigit = 0;
    divisor = 10000;
    i = 0;
    while (divisor > 1) {
        digit = value / divisor;
        if (foundDigit || digit) {
            foundDigit = 1;
            dst[i] = g_DialogDigitCharacters[digit];
            i++;
        }
        value -= digit * divisor;
        divisor /= 10;
    }
    dst[i] = g_DialogDigitCharacters[value];
    dst[i + 1] = 0xFF;
}

void ConvertNumToStrWithSpace(u16 value, u8* dst) {
    s32 foundDigit;
    s16 i;
    s16 divisor;
    s16 digit;

    foundDigit = 0;
    divisor = 10000;
    i = 0;
    while (divisor > 1) {
        digit = value / divisor;
        if (foundDigit || digit) {
            foundDigit = 1;
            dst[i] = g_DialogDigitCharacters[digit];
            i++;
        } else {
            dst[i] = 0;
            i++;
        }
        value -= digit * divisor;
        divisor /= 10;
    }
    dst[i] = g_DialogDigitCharacters[value];
    dst[i + 1] = 0xFF;
}

void ConvertHexToString(u16 value, u8* dst) {
    u32 foundDigit;
    s16 i;
    s16 divisor;
    s16 digit;

    foundDigit = 0;
    divisor = 0x1000;
    i = 0;
    while (divisor > 1) {
        digit = value / divisor;
        if (foundDigit || digit) {
            foundDigit = 1;
            dst[i] = g_DialogDigitCharacters[digit];
            i++;
        }
        value -= digit * divisor;
        divisor /= 16;
    }
    dst[i] = g_DialogDigitCharacters[value];
    dst[i + 1] = 0xFF;
}

s32 CopyDialogToMapName(s16 stringId) {
    s16 i;
    s16 j;
    u8* str;
    u8* charName;
    u8 value;

    if (g_FieldText == NULL) {
        FieldEventDebugError("No mes data!");
        return 0;
    }

    str = g_FieldText;
    j = 0;
    i = 0;
    // Field text section starts with an array of 16-bit offsets to each string.
    str += g_FieldText[stringId * 2 + 2];
    str += g_FieldText[stringId * 2 + 3] << 8;

    do {
        switch (*str) {
        // End of string.
        case 0xFF:
            goto end;

        // Write ', '
        case 0xE2:
            str++;
            Savemap.memory_bank_4[104 + i] = 0xC;
            i++;
            Savemap.memory_bank_4[104 + i] = 0;
            i++;
            break;

        // Write '."'
        case 0xE3:
            str++;
            Savemap.memory_bank_4[104 + i] = 0xE;
            i++;
            Savemap.memory_bank_4[104 + i] = 2;
            i++;
            break;

        // Write '…"'
        case 0xE4:
            str++;
            Savemap.memory_bank_4[104 + i] = 0xA9;
            i++;
            Savemap.memory_bank_4[104 + i] = 2;
            i++;
            break;

        // Write player-chosen character name from savemap.
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
            charName = GetCharacterName((s16)(*str - 0xEA)) + j;
            if (*charName == 0xFF || j >= 9) {
                str++;
                j = 0;
            } else {
                j++;
                Savemap.memory_bank_4[104 + i] = *charName;
                i++;
            }
            break;

        // Two byte characters used in Japanese extended font(?)
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
            value = *str;
            str++;
            Savemap.memory_bank_4[104 + i] = value;
            i++;

        // Fall through to copy the second byte of the character.
        // Also used to copy all other characters directly.
        default:
            value = *str;
            str++;
            Savemap.memory_bank_4[104 + i] = value;
            i++;
            break;
        }
    } while (i < 23);

end:
    Savemap.memory_bank_4[104 + i] = 0xFF;
    return 1;
}

void SystemMessageSetCharName(s16 battleCharId, s16 stringId) {
    u8* newName;
    s16 len;
    u8* charName;

    if (g_FieldText == NULL) {
        FieldEventDebugError("No mes data!");
        return;
    }

    newName = g_FieldText;
    newName += g_FieldText[stringId * 2 + 2];
    newName += g_FieldText[stringId * 2 + 3] << 8;
    len = 0;
    charName = GetCharacterName(battleCharId);

    while (*newName != 0xFF) {
        *charName++ = *newName++;
        len++;
    }

    if (len < 9) {
        *charName = 0xFF;
    }
}

/////////////////////////////////////////////////
// Begin of field_debug.c
/////////////////////////////////////////////////

/* Initialise the debug renderer's GPU buffers: hide all six pages, then build
 * both framebuffers' primitive arrays (sprites, lines, tiles) with their packet
 * codes and blend bits, the CLUT table, and the two draw-mode blocks.
 *
 * Four things this needed, all of them structural:
 *
 *   - one pointer pair per loop, not one pair reused four times. A single pair
 *     is live across GetGraphType/SetDrawMode, so gcc gives it a callee-saved
 *     register and every buffer base is set up in one; the original's first
 *     three bases die in their own preheader and sit in $v0.
 *   - each element addressed as `&base[i * stride]`, not by bumping the
 *     pointer. Bumped, `base` is a biv and gcc strength-reduces the three
 *     references to byte 7 onto a second base register biased by +7, so the
 *     stores come out `-4(a0)`/`0(a0)` instead of `3(v1)`/`7(v1)`.
 *   - the sprite loop sets the shade-texture bit, not semi-transparency:
 *     `ori 0x1`, not `ori 0x2`. The line loop really is setSemiTrans.
 *   - `tpage` is a u16 -- the `andi $s3,$a3,0xffff` at the join of the
 *     GetGraphType tests is the widening, and an s32 has none.
 *
 * `unusedLocals` is not a placeholder for something better understood: the
 * original's frame is 0x50 against 0x30 for the same code, and the 0x20 sits
 * between the outgoing-argument area and the register saves, which is where
 * gcc puts locals. Some aggregate local was declared, given a slot by
 * `expand_decl`, and had all of its uses optimised away; nothing in the
 * emitted code names it. The frame size is part of the match, so the slot has
 * to be reserved. */
void FieldDebugInitBuffers(void) {
    s32 i;
    s32 off;
    u8* sprt0;
    u8* sprt1;
    u8* line0;
    u8* line1;
    u8* tile0;
    u8* tile1;
    u8* dm0;
    u8* dm1;
    u16 tpage;
    s32 hide;
    u8 unusedLocals[0x20];

    hide = 1;
    for (off = 0x762; off >= 0; off -= 0x17A) {
        D_800E08C0[off] = hide;
    }
    D_8009D824 = 1;
    g_FieldDebugRb = 0;
    g_FieldDebugCurPage = 0;
    g_FieldDebugTransp = 0;

    sprt0 = (u8*)&D_800E1028[0];
    sprt1 = sprt0 + 0x1580;
    for (i = 0; i < 0x158; i++) {
        setlen(&sprt0[i * 0x10], 3);
        setcode(&sprt0[i * 0x10], 0x74);
        setlen(&sprt1[i * 0x10], 3);
        setcode(&sprt1[i * 0x10], 0x74);
        setShadeTex(&sprt0[i * 0x10], 1);
        setShadeTex(&sprt1[i * 0x10], 1);
    }

    for (i = 0; i < 8; i++) {
        D_800E4200[i] = ((0x1E7 - i) << 6) | 0x10;
    }

    line0 = (u8*)&D_800E3FA8[0];
    line1 = line0 + 0xC0;
    for (i = 0; i < 0xC; i++) {
        setlen(&line0[i * 0x10], 3);
        setcode(&line0[i * 0x10], 0x60);
        setlen(&line1[i * 0x10], 3);
        setcode(&line1[i * 0x10], 0x60);
        setSemiTrans(&line0[i * 0x10], 1);
        setSemiTrans(&line1[i * 0x10], 1);
    }

    tile0 = (u8*)&D_800E3B28[0];
    tile1 = tile0 + 0x240;
    for (i = 0; i < 0x18; i++) {
        setlen(&tile0[i * 0x18], 5);
        setcode(&tile0[i * 0x18], 0x48);
        *(u32*)((u8*)&tile0[i * 0x18] + 0x14) = 0x55555555;
        setlen(&tile1[i * 0x18], 5);
        setcode(&tile1[i * 0x18], 0x48);
        *(u32*)((u8*)&tile1[i * 0x18] + 0x14) = 0x55555555;
    }

    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x2F;
    } else {
        tpage = 0x1F;
    }
    dm0 = (u8*)&D_800E4128[0];
    dm1 = dm0 + 0x48;
    for (i = 0; i < 6; i++) {
        SetDrawMode((DR_MODE*)&dm0[i * 0xC], 0, 0, tpage, NULL);
        SetDrawMode((DR_MODE*)&dm1[i * 0xC], 0, 0, tpage, NULL);
    }
}

void InitFieldDebugPages(void) {
    FieldDebugPageInit(5, 0x6C, 0, 0x6C, 0x52);
    FieldDebugStringCopy(g_DebugText, "Authr:");
    FieldDebugStringConcat(g_DebugText, g_FieldScripts->author);
    AddStrNextDebugRow(5, g_DebugText);
    FieldDebugStringCopy(g_DebugText, "Event:");
    FieldDebugStringConcat(g_DebugText, g_FieldScripts->name);
    AddStrNextDebugRow(5, g_DebugText);
    AddStrNextDebugRow(5, "  Go");
    AddStrNextDebugRow(5, "  Stop");
    AddStrNextDebugRow(5, "  Step");
    SetStrToDebugRow(5, 5, "  Actor OFF");
    SetStrToDebugRow(5, 6, "  Info  OFF");
    FieldDebugPageHide(5);
    FieldDebugPageInit(4, 0x6C, 0x52, 0x6C, 0x52);
    AddStrNextDebugRow(4, &D_800E0628);
    FieldDebugPageHide(4);
    FieldDebugPageInit(3, 0x6C, 0xA4, 0x6C, 0x5C);
    AddStrNextDebugRow(3, &D_800E0630);
    FieldDebugPageHide(3);
    FieldDebugPageInit(1, 0, 0, 0x6C, 0xCA);
    AddStrNextDebugRow(1, &D_800E0628);
    FieldDebugPageHide(1);
    g_FieldScriptRunState = 3;
    D_8007EBCC = 4;
    D_8007EBDC = 8;
    g_FieldScriptDebugFlags = 0;
    g_DebugLevel = 0;
    D_80070788 = 0;
    g_FieldDebugCurPage = 5;
    FieldDebugPageSetHeadRow(5, 4);
}

/* Move the first hidden debug page to (x, y, w, h) and clear its text, falling
 * back to page 0 when every page is currently being rendered.
 *
 * The element address has to go through `page` rather than being indexed
 * inline: as a bare `D_800E08C0[i * 378]` gcc hoists the symbol's %hi/%lo out
 * of the loop, where the original rematerialises it each iteration. */
s16 FieldDebugPagesResetPosSize(s16 x, s16 y, s16 w, s16 h) {
    s16 i;

    for (i = 0; i < 6; i++) {
        u8* page;

        page = &D_800E08C0[i * 378];
        if (*page) {
            FieldDebugPageSetPosSize(i, x, y, w, h);
            FieldDebugPageResetStrings(i);
            return i;
        }
    }
    FieldDebugPageSetPosSize(0, x, y, w, h);
    FieldDebugPageResetStrings(0);
    return 0;
}

/* `offClear` is deliberately a second copy of `off`: with one variable feeding
 * both the test and the store gcc coalesces the multiply into $v0 and needs an
 * extra `move` to get it into the address register. */
void FieldDebugPageInit(s16 page, s16 x, s16 y, s16 w, s16 h) {
    s32 off;
    s32 offClear;

    FieldDebugPageSetPosSize(page, x, y, w, h);
    off = page * 378;
    offClear = off;
    if (D_800E08C0[off] != 2) {
        FieldDebugPageResetStrings(page);
    } else {
        D_800E08C0[offClear] = 0;
        D_8009D824 = 1;
    }
}

void FieldDebugPageSetPosSize(s16 page, s16 x, s16 y, s16 w, s16 h) {
    D_800E0748[page * 189] = x;
    D_800E074A[page * 189] = y;
    D_800E074C[page * 189] = w;
    D_800E074E[page * 189] = h;
    D_8009D824 = 1;
}

/* The byte offset has to be a named local and the elements reached through a
 * `u8*` cast: written as `D_800E0748[page * 189] += x` the index needs a
 * scaling `sll`, and gcc folds the symbol's %hi/%lo into that same `addu`, so
 * one base register serves both the load and the store. With `off` already
 * holding the byte offset the address stays `(symbol)(reg)` in all four mems
 * and the assembler rematerialises it through $at each time, which is what the
 * original does. Splitting the `+=` into a load and a store is needed too --
 * a compound assignment computes the address once by construction. */
void FieldDebugPageAddPos(s16 page, s16 x, s16 y) {
    s32 off;
    s16 px;
    s16 py;

    D_8009D824 = 1;
    off = page * 378;
    px = *(s16*)((u8*)D_800E0748 + off);
    py = *(s16*)((u8*)D_800E074A + off);
    *(s16*)((u8*)D_800E0748 + off) = px + x;
    *(s16*)((u8*)D_800E074A + off) = py + y;
}

/* Same shape as FieldDebugPageAddPos above; see the note there. */
void FieldDebugPageAddSize(s16 page, s16 w, s16 h) {
    s32 off;
    s16 pw;
    s16 ph;

    D_8009D824 = 1;
    off = page * 378;
    pw = *(s16*)((u8*)D_800E074C + off);
    ph = *(s16*)((u8*)D_800E074E + off);
    *(s16*)((u8*)D_800E074C + off) = pw + w;
    *(s16*)((u8*)D_800E074E + off) = ph + h;
}
bool FieldDebugPageIsRender(s16 arg0) { return D_800E08C0[arg0 * 378] == 0; }

/* Blank all 24 rows of a debug page and restore its default colour. The row
 * text is a 14-byte record per row, the per-row colour a single byte, so the
 * two arrays walk the page at different strides. */
void FieldDebugPageResetStrings(s16 page) {
    s32 i;
    s32 off;
    u8* colors;

    i = 0;
    colors = &D_800E08A8[page * 378];
    off = page * 378;
    while (i < 24) {
        D_800E0758[off] = 0;
        *colors++ = 0;
        i++;
        off += 14;
    }
    D_800E0750[page * 378] = 7;
    D_800E0751[page * 378] = 0xF;
    D_800E0752[page * 378] = 0x1F;
    D_800E0756[page * 189] = 0;
    D_800E0754[page * 189] = 0;
    D_800E08C0[page * 378] = 0;
    D_8009D824 = 1;
}

void FieldDebugRenderClear(void) {
    g_FieldDebugRChars = 0;
    g_FieldDebugRLines = 0;
    g_FieldDebugRRect = 0;
    g_FieldDebugRDm = 0;
    g_FieldDebugRb ^= 1;
}

void FieldDebugRenderPage(s16 page);

/* Rebuilds the debug overlay into this frame's ordering table when anything
 * marked it dirty, then links that table into the caller's OT.
 *
 * The page counter and the byte offset have to be two independent induction
 * variables, and the visibility byte reached by bare subscript. Anything that
 * derives the offset from the counter (`D_800E08C0[page * 378]`, with or
 * without a pointer local) lets gcc strength-reduce the two into one walking
 * pointer with the symbol folded into its start value; giving it a plain
 * register index instead leaves the `symbol(reg)` addressing the original has,
 * which maspsx rematerialises through $at every iteration. */
void FieldDebugRender(u_long* ot) {
    s32 page;
    s32 off;

    if (D_8009D824) {
        FieldDebugRenderClear();
        page = 0;
        off = 0;
        ClearOTag(D_800E41C8[g_FieldDebugRb], 7);
        do {
            if (D_800E08C0[off] == 0) {
                FieldDebugRenderPage(page);
            }
            off += 378;
            page++;
        } while (page < 6);
        D_8009D824 = 0;
    }
    addPrims(ot, D_800E41C8[g_FieldDebugRb], &D_800E41C8[g_FieldDebugRb][6]);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field5", FieldDebugRenderPage);

extern /*?*/ s32 D_800E1036;

/* Render one debug-page string's characters as GPU sprites into the render
 * buffer. Decodes each FF7 text char to a glyph index (the switch is the
 * jump table), computes the UV coords, links the packet into the OT. m2c
 * seed, semantically close; the residual is regalloc across the jump-table
 * dispatch and the OT-link store ordering. Codegen pinned via MASPSX_OVERRIDE
 * pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field5", FieldDebugRenderString);
#else
void FieldDebugRenderString(s16 arg0, s16 arg1, u8* arg2, s16 arg3, s32 arg4) {
    s16 var_t3;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_t0;
    s32 temp_t5;
    s32* temp_a0_2;
    u32 var_a0;
    u8 temp_v0;
    u8 temp_v1;
    u8* temp_a2;
    u8* var_t2;

    var_t2 = arg2;
    var_t3 = arg3;
    if (*var_t2 != 0) {
        temp_t5 = arg0 * 0x17A;
    loop_2:
        if ((((*(D_800E0748 + temp_t5) + *(D_800E074C + temp_t5)) - 8) >=
             var_t3) &&
            (g_FieldDebugRChars < 0x158)) {
            temp_v0 = *var_t2;
            switch (temp_v0) {
            case 0x20:
                var_a0 = 0x3F;
                break;
            case 0x3A:
                var_a0 = 0xD5;
                break;
            case 0x2E:
                var_a0 = 0xB2;
                break;
            case 0x2B:
                var_a0 = 0xB3;
                break;
            case 0x2F:
                var_a0 = 0xD4;
                break;
            case 0x2D:
                var_a0 = 0xD0;
                break;
            case 0x2A:
                var_a0 = 0xCF;
                break;
            case 0x21:
                var_a0 = 0xAE;
                break;
            case 0x3F:
                var_a0 = 0xAF;
                break;
            case 0x3D:
                var_a0 = 0xDA;
                break;
            case 0x23:
                var_a0 = 0xD6;
                break;
            case 0x3E:
                var_a0 = 0xD9;
                break;
            default:
                temp_v1 = *var_t2;
                if (temp_v1 < 0x3AU) {
                    var_a0 = *var_t2 + 3;
                } else if (temp_v1 >= 0x61U) {
                    var_a0 = *var_t2 + 0x53;
                } else {
                    var_a0 = *var_t2 + 0x73;
                }
                break;
            }
            var_t2 += 1;
            D_800E1028[(g_FieldDebugRb * 0x1580) + (g_FieldDebugRChars * 0x10)]
                .unkC = (s8)(((var_a0 & 0xF) * 8) - 0x80);
            D_800E1028[(g_FieldDebugRb * 0x1580) + (g_FieldDebugRChars * 0x10)]
                .unkD = (s8)(((var_a0 >> 1) & 0x78) - 0x80);
            temp_a1 = g_FieldDebugRb * 0x1580;
            temp_t0 = g_FieldDebugRChars * 0x10;
            temp_a0 = temp_a1 + temp_t0;
            temp_a2 = &D_800E1028[temp_a0];
            temp_a2->unk8 = var_t3;
            temp_a2->unkA = (s16)arg4;
            *(&D_800E1036 + temp_a0) =
                D_800E4200[*(temp_t5 + D_800E08A8 + arg1)];
            temp_a0_2 = (g_FieldDebugRb * 0x1C) + (arg0 * 4) + D_800E41C8;
            temp_a2->unk0 =
                (s32)((temp_a2->unk0 & 0xFF000000) | (*temp_a0_2 & 0xFFFFFF));
            g_FieldDebugRChars += 1;
            *temp_a0_2 = (*temp_a0_2 & 0xFF000000) |
                         ((s32)(temp_a1 + (temp_t0 + D_800E1028)) & 0xFFFFFF);
            var_t3 += 8;
            if (*var_t2 != 0) {
                goto loop_2;
            }
        }
    }
}
#endif

/* The debug pages are a 378-byte record: a 0x10-byte header whose fields have
 * their own splat symbols (D_800E0748 = x, 074A = y, 074C = w, 074E = h,
 * 0754 = headRow) followed by 362 bytes of row text at D_800E0758. Only the
 * header offsets the two Add*NextDebugRow functions reach off `rows` are given
 * names here; everything else keeps its D_ symbol. */
typedef struct {
    s16 unk00[6];
    s16 headRow;
    s16 unk0E;
} FieldDebugPageHdr;

/* Append a line to a debug page (no colour), wrapping back to the top row once
 * the page's pixel height can no longer hold another 10-pixel row.
 *
 * Two spellings carry this function, and neither is optional:
 *
 *   - the header fields are reached as `*(s16*)((u8*)D_800E0754 + off)` with
 *     `off` already a *byte* offset. Indexed as `D_800E0754[page * 189]` the
 *     element needs a scaling `sll`, and gcc folds the symbol's %hi/%lo into
 *     that same `addu`, so one base register then serves every access. With
 *     `off` already scaled the address stays `(symbol)(reg)` in the mem and
 *     the assembler rebuilds it through $at each time, which is what the
 *     original does.
 *   - the store of the incremented head row goes through `hdr`, a pointer to
 *     the record header derived from the *text* symbol (`D_800E0758 - 0x10`).
 *     That is where `addiu s0,s0,-0x10` / `sh v0,0xc(s0)` comes from: `rows`
 *     keeps the bare symbol live in a callee-saved register across the call,
 *     and the header base is that register adjusted, not a fresh %hi/%lo.
 *     Writing the store as `*(s16*)(rows - 4)` folds the two constants in the
 *     tree and gives `sh v0,-4(s0)` instead -- 9 rows out. */
s32 AddStrNextDebugRow(s16 page, const char* str) {
    s32 off;
    char* rows;
    FieldDebugPageHdr* hdr;

    off = page * 378;
    rows = D_800E0758 + off;
    FieldDebugStringCopy(&rows[*(s16*)((u8*)D_800E0754 + off) * 14], str);
    hdr = (FieldDebugPageHdr*)(D_800E0758 - 0x10 + off);
    hdr->headRow = *(s16*)((u8*)D_800E0754 + off) + 1;
    if ((*(s16*)((u8*)D_800E074E + off) - 8) / 10 <
        *(s16*)((u8*)D_800E0754 + off)) {
        *(s16*)((u8*)D_800E0754 + off) = 0;
    }
    D_8009D824 = 1;
    return 1;
}

/* Append a coloured line to a debug page, wrapping back to the top row once the
 * page's pixel height can no longer hold another 10-pixel row. Same two
 * spellings as AddStrNextDebugRow above, plus the colour byte reached as
 * `D_800E0758 + 0x150 + off` so that its base is the live `rows` register too.
 *
 * The head-row increment has to be a named `s16` local computed *before* the
 * header pointer, not written inline into `hdr->headRow`. Inline, the store's
 * address and its value are one statement and the scheduler issues the
 * `lui a0,0x6666` that materialises the /10 magic constant one slot too early;
 * split, the address computation fills those slots instead. `s32 next` does
 * not work -- the widening node changes the store. Found by decomp-permuter
 * after six hand phrasings (moving `colors`, moving `hdr`, `*(colors + row)`,
 * an `s32 limit` for the quotient, reversing the comparison) all measured the
 * identical 1-row residue. */
s32 AddColorStrNextDebugRow(s16 page, const char* str, u8 color) {
    s32 off;
    char* rows;
    u8* colors;
    FieldDebugPageHdr* hdr;
    s16 next;

    off = page * 378;
    rows = D_800E0758 + off;
    FieldDebugStringCopy(&rows[*(s16*)((u8*)D_800E0754 + off) * 14], str);
    colors = (u8*)(D_800E0758 + 0x150 + off);
    colors[*(s16*)((u8*)D_800E0754 + off)] = color;
    next = *(s16*)((u8*)D_800E0754 + off) + 1;
    hdr = (FieldDebugPageHdr*)(D_800E0758 - 0x10 + off);
    hdr->headRow = next;
    if ((*(s16*)((u8*)D_800E074E + off) - 8) / 10 <
        *(s16*)((u8*)D_800E0754 + off)) {
        *(s16*)((u8*)D_800E0754 + off) = 0;
    }
    D_8009D824 = 1;
    return 1;
}

s32 SetStrToDebugRow(s16 page, s16 row, const char* str) {
    char* rows = D_800E0758 + page * 378;

    FieldDebugStringCopy(&rows[row * 14], str);
    D_8009D824 = 1;
    return 1;
}

s32 SetDebugStrRowColor(s16 page, s16 row, u8 color) {
    s32 offset = page * 378;
    s32 index = row;
    u8* colors = D_800E08A8 + offset;

    colors[index] = color;
    return 1;
}

void FieldDebugPageSetHeadRow(s16 page, s16 row) {
    D_800E0756[page * 189] = row;
    D_8009D824 = 1;
}

void FieldDebugPageSetColor(s16 page, u8 r, u8 g, u8 b) {
    if (D_800E08C0[page * 378] == 0) {
        D_800E0750[page * 378] = r;
        D_800E0751[page * 378] = g;
        D_800E0752[page * 378] = b;
        D_8009D824 = 1;
    }
}

void FieldDebugPageNotInit(s16 page) {
    D_800E08C0[page * 378] = 1;
    D_8009D824 = 1;
}

void FieldDebugPageHide(s16 page) {
    D_800E08C0[page * 378] = 2;
    D_8009D824 = 1;
}

static void FieldDebugTranspSwitch(void) {
    g_FieldDebugTransp = (g_FieldDebugTransp + 1) & 3;
}

void FieldDebugStringCopy(char* dst, const char* src) {
    if (*src) {
        do {
            *dst++ = *src++;
        } while (*src != '\0');
    }
    *dst = '\0';
}

void FieldDebugStringConcat(char* dest, char* src) {
    if (*dest != '\0') {
        while (*++dest != '\0') {
        }
    }
    if (*src != '\0') {
        do {
            *dest++ = *src++;
        } while (*src != '\0');
    }
    *dest = '\0';
}

static s32 FieldDebugStringSize(char* src) {
    s32 len = 0;

    while (*src != '\0') {
        src++;
        len++;
    }
    return len;
}

static void FieldDebugStringPartCopy(char* dst, char* src, s32 len) {
    s32 i;
    for (i = len - 1; i != -1; i--) {
        *dst = *src;
        src++;
        dst++;
    }
}

void FieldDebugStringU8hex(s32 val, char* msg_out) {
    msg_out[1] = '\0';
    msg_out[0] = g_FieldDebugDigits[val & 0xF];
}

void FieldDebugStringU16hex(s32 val, char* msg_out) {
    msg_out[2] = '\0';
    msg_out[0] = g_FieldDebugDigits[(val & 0xF0) >> 4];
    msg_out[1] = g_FieldDebugDigits[val & 0xF];
}

void FieldDebugStringU32hex(s32 val, char* msg_out) {
    msg_out[4] = '\0';
    msg_out[0] = g_FieldDebugDigits[(val & 0xF000) >> 0xC];
    msg_out[1] = g_FieldDebugDigits[(val & 0xF00) >> 8];
    msg_out[2] = g_FieldDebugDigits[(val & 0xF0) >> 4];
    msg_out[3] = g_FieldDebugDigits[val & 0xF];
}

/* Writes value as decimal into out, suppressing leading zeros. Five digits
 * plus the units place, so the range is 0..99999. */
void FieldDebugIntToString(s32 value, char* out) {
    char* end;
    s32 started;
    s32 divisor;
    s32 count;
    s32 digit;
    u8 lastDigit;

    started = 0;
    divisor = 10000;
    count = 0;
    do {
        digit = value / divisor;
        if (started || digit != 0) {
            started = 1;
            out[count] = g_FieldDebugDigits[digit];
            count++;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (divisor >= 2);
    end = out + count;
    digit = g_FieldDebugDigits[value];
    lastDigit = digit;
    end[1] = '\0';
    end[0] = lastDigit;
}
