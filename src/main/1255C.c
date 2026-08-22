//! PSYQ=3.3 CC1=2.7.2 G=8
#include "main_private.h"

// Tentative definitions, not `extern` declarations, and the difference is the
// whole addressing form. This unit is compiled `-G8`: cc1 puts an object it
// defines that is no larger than the threshold into `.sdata`/`.sbss`, emits a
// tentative definition as a small `.comm`, and the assembler then reaches any
// of those through `$gp` -- one instruction, `%gp_rel(<sym>)($gp)`. Declared
// `extern` instead, cc1 emits only `.extern <sym>,<size>`, nothing lands in
// small data, and every access is the two- or three-instruction `%hi`/`%lo`
// form that the target does not have. The real definitions live in
// `src/main/18B8.c` and `asm/us/main/data/536C4.bss.s`; `--use-comm-section`
// (see tools/ninja/gen.py) keeps these as COMMON so the link binds to those
// rather than seeing two definitions.
s32 D_80062DF8;
s8 D_80062DFC;
s32 D_80062ECC;
s32 D_80062F0C;
s32 D_80062F20;
s32 D_80062F90;
s32 D_80062F94;
s32 D_80062FC0;
u_long* D_80062FC4; // also declared extern in game.h, for the units at -G0
Gpu D_80063008;
u_long* D_8006300C;

u8 D_80062EBC = 0;
static s8 _D_80062EBD = 0;
static s8 _D_80062EBE = 0;
static s8 _D_80062EBF = 0;
s8 D_80062EC0 = 0;
static s8 _D_80062EC1 = 0;
static s8 _D_80062EC2 = 0;
static s8 _D_80062EC3 = 0;

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80021D5C);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80021E70);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80021F58);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80022B5C);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80022DE4);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80022FE0);

s32 func_80023050(void) { return D_80062DF8; }

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/1255C", func_8002305C);
#else
/* PARKED -- 12 changed, 3 inserted, 27 against 26 (+1 instruction).
 * The prologue (six insns) and all three arm bodies are byte-identical; the
 * whole residue is the shape of the switch's compare tree.
 *   target: li 2 / beq -> case2, slti 3 / bnez -> default, li 4 / beq,
 *           li 5 / beq, j default   -- a tree rooted at 2 with an EMPTY left
 *           subtree and right = {4,5}, so the `< 3` test falls straight to
 *           the default and case 5's `ori v0,0x10` rides its beq's delay slot
 *   ours:   li 4 / beq, slti 5 / beqz, li 2 / beq, li 5 / beq -- rooted at 4,
 *           left = {2}, right = {5}, which costs the extra j/nop pair
 * That is `balance_case_nodes`, and nothing tried moves it. Measured, all
 * against the same body (rows / length against the target's 26):
 *   switch {2,4,5}, three separate arms      15 / +1   (this body)
 *   switch, `case 2: case 4:` sharing an arm 15 / +1
 *   switch with an empty `case 3: break;`    17 / +3
 *   `if (==2) else if (>2) { if (==4) ... }` 12 / -2
 *   flat `if/else if/else if` chain          12 / -4
 *   `if (arg0 < 3) {...return;}` + switch{4,5} 14 / +1
 * The enum hypothesis is measured and dead: `balance_case_nodes` consults an
 * ASCII cost table only when `estimate_case_costs` accepts the case values
 * (all within -1..127, which 2/4/5 are) and the index is not an
 * ENUMERAL_TYPE, so an enum-typed index should take the plain median split
 * instead. All four spellings -- `switch ((Subsystem)arg0)`, the parameter
 * declared `Subsystem`, enumerator names in the labels, and the shared-arm
 * form of each -- measure *exactly* 15 / +1, i.e. the cast does not reach
 * `orig_index`'s type at all.
 * Next: read cc1's tree dump for the case list rather than guessing at the
 * split, or hand it to the permuter with perm_ins_block weighted. */
void func_8002305C(s32 arg0, s32 arg1) {
    s32 old = D_80062F94;
    D_80062DF8 = arg0;
    D_80062F94 = arg1 - 1;
    D_80062F0C = old;
    switch (arg0) {
    case 2:
        D_80062F20 = 0x10;
        break;
    case 4:
        D_80062F20 = 0x10;
        break;
    case 5:
        D_8009A0D3 = old;
        D_80062ECC = 0;
        D_80062F20 = 0x10;
        break;
    }
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_800230C4);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_8002368C);

// Extract the hours field (0-99) of the HH:MM play-time clock from a seconds
// counter, capped at 99:59:59 (0x57E3F seconds). Returned as a plain decimal
// (tens*10 + units) so the 2-digit number drawer renders it. func_8002382C
// formats the matching minutes field.
s32 func_80023788(s32 arg0) {
    s32 var_a0;

    var_a0 = arg0;
    if (var_a0 > 0x57E3F) { // clamp to 99:59:59, in seconds
        var_a0 = 0x57E3F;
    }
    // tens-of-hours (sec / 36000) * 10 + units-of-hours ((sec % 36000) / 3600)
    return ((var_a0 / D_80049474) * 0xA) + ((var_a0 % D_80049474) / D_80049478);
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_8002382C);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80023940);

void func_80023AC4(void) { D_80062FC0 = 2; }

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80023AD4);

// Push the current display and draw environments to the GPU: the per-frame
// double-buffer flip (activate the finished buffer for scanout, point drawing
// at the other one).
static void func_80024A04(void) {
    PutDispEnv(&D_8007075C);
    PutDrawEnv(&D_80070700);
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80024A3C);

void func_80024D88(s32 arg0) {
    func_800211C4(0xD);
    do {
    } while (SystemCdromReadChain());
    VSync(30);
    func_801D131C(arg0);
}

void func_80024DD4(s32 arg0) {
    func_800211C4(0xE);
    do {
    } while (SystemCdromReadChain());
    func_801D1A6C(arg0);
}

void func_80024E18(s32 arg0) {
    func_800211C4(0xF);
    do {
    } while (SystemCdromReadChain());
    func_801D4118(arg0);
}

// This should be the title screen handler
void func_80024E5C(void) {
    func_800211C4(0x10); // load title screen?
    do {                 // wait until it's loaded?
    } while (SystemCdromReadChain());
    func_801D4CC0(); // jump into title screen loop?
}

void func_80024E94(void) {
    func_800211C4(0xA);
    do {
    } while (SystemCdromReadChain());
    func_801D1774();
}

void func_80024ECC(void) {
    func_800211C4(1);
    do {
    } while (SystemCdromReadChain());
    func_801D2D74();
}

void func_80024F04(void) {
    func_800211C4(1);
    do {
    } while (SystemCdromReadChain());
    func_801D2E84();
}

void func_80024F3C(s32 arg0) {
    func_800211C4(1);
    do {
    } while (SystemCdromReadChain());
    func_801D2F00(arg0);
}

void func_80024F80(s32 arg0) {
    func_800211C4(1);
    do {
    } while (SystemCdromReadChain());
    func_801D3138(arg0);
}

void func_80024FC4(s32 arg0) {
    func_800211C4(1);
    do {
    } while (SystemCdromReadChain());
    func_801D3018(arg0);
}

void func_80025008(void) {
    func_800211C4(1);
    do {
    } while (SystemCdromReadChain());
    func_801D3228();
}

// MENU event 0x18: snapshot each present party member's level into
// D_8009D44C[]. The endgame battle AI (Jenova-SYNTHESIS) counts how many of
// these are 99 to scale Safer-Sephiroth's HP.
void SnapshotPartyLevels(void) {
    s32 i;
    u16* present;
    for (i = 0, present = &D_8009D78A; i < 8; i++) {
        if ((*present >> D_80049500[i]) & 1) {
            D_8009D44C[i] = D_8009C738[D_80049500[i]].level;
        }
    }
}

void func_800250B4(void) {
    func_800211C4(0xC);
    do {
    } while (SystemCdromReadChain());
    func_801D027C();
}

void func_800250EC(s32 arg0) {
    func_800211C4(0xC);
    do {
    } while (SystemCdromReadChain());
    func_801D05C4(arg0);
}

void func_80025130(s32 arg0) {
    func_800211C4(0xC);
    do {
    } while (SystemCdromReadChain());
    func_801D0704(arg0);
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025174);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025288);

// Find the inventory slot holding item `arg0` and return the whole packed
// entry ((count << 9) | id), or 0xFFFF if the item is not carried.
s32 func_80025310(s32 arg0) {
    s32 i;
    u16 ret;

    arg0 &= 0x1FF;
    ret = 0xFFFF;
    for (i = 0; i < 0x140; i++) {
        if (D_8009CBE0[i] != 0xFFFF && arg0 == (D_8009CBE0[i] & 0x1FF)) {
            return D_8009CBE0[i];
        }
    }
    return ret;
}

void func_80025360() { func_8001FA28(0x19F); }

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025380);

s32 func_8002542C(s32 arg0) {
    s32 i;
    for (i = 0; i < MAX_MATERIA_COUNT; i++) {
        if (Savemap.materia[i] == -1) {
            Savemap.materia[i] = arg0;
            if (func_8002603C(arg0 & 0xFF) == 10) {
                Savemap.memory_bank_1[75] |= 1;
            }
            if ((arg0 & 0xFF) == 44) {
                Savemap.memory_bank_1[75] |= 2;
            }
            return -1;
        }
    }
    return arg0;
}

void func_800254D8() { D_80062EBC = 0; }

void func_800254E4(s8 arg0) {
    D_80069800[D_80062EBC] = arg0;
    D_80062EBC = D_80062EBC + 1;
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025514);

void func_80025648(void) {}

void func_80025650(void) {}

// get party leader (Cloud) level
s32 func_80025658() { return D_8009C738[0].level; }

// Party slot -> equipped character -> that character's equipped armor's
// materia-slot configuration (slot count / linked-pair layout / growth rate;
// see ArmorRecord in main_private.h). Returns sentinel (void*)0xFF for an
// empty party slot.
void* GetPartySlotArmorMateriaSlots(s32 arg0) {
    u8 temp_v1;
    void* var_v0;

    temp_v1 = D_8009CBDC[arg0];
    var_v0 = (void*)0xFF;
    if (temp_v1 != 0xFF) {
        u32 idx = g_BattleCharIdToCharId[temp_v1];
        var_v0 = g_ArmorTable[D_8009C755[idx * 0x84]].materiaSlot;
    }
    return var_v0;
}

// Weapon counterpart of GetPartySlotArmorMateriaSlots: party slot -> equipped
// character -> that character's equipped weapon's materia-slot configuration
// (see WeaponRecord in main_private.h). Returns sentinel (void*)0xFF for an
// empty party slot.
void* GetPartySlotWeaponMateriaSlots(s32 arg0) {
    u8 temp_v1;
    void* var_v0;

    temp_v1 = D_8009CBDC[arg0];
    var_v0 = (void*)0xFF;
    if (temp_v1 != 0xFF) {
        u32 idx = g_BattleCharIdToCharId[temp_v1];
        // SMELL: raw 0x84 char-record stride math; wants a CharacterRecord
        // struct (equippedWeapon at +0xC) ->
        // g_CharacterRecords[idx].equippedWeapon
        var_v0 = g_WeaponTable[D_8009C754[idx * 0x84]].materiaSlot;
    }
    return var_v0;
}

s32* func_80025758(s32 arg0) { return (s32*)&g_ArmorTable[arg0]; }

s32* func_80025774(s32 arg0) { return (s32*)&g_AccessoryTable[arg0]; }

Unk8009D84C* func_80025788(s32 arg0) {
    if (Savemap.partyID[arg0] != 0xFF) {
        return &D_8009D84C[arg0];
    }
    // BUG undefined return
}

void func_800257C4(void) {}

u8* GetCharacterName(s32 battleCharId) {
    return Savemap.party[g_BattleCharIdToCharId[battleCharId]].name;
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025800);

// Add `amount` HP to the battle record of party slot `partyId`, clamped to the
// member's max HP, then mirror the new value into the save-game party record
// (Savemap.party[charId].hp_cur). Empty party slots (0xFF) are a no-op.
void SystemMenuAddHpByPartyId(s32 partyId, s32 amount) {
    u8 battleCharId;
    s32 new_var;
    s32 emptySlot;
    s32 charId;

    battleCharId = D_8009CBDC[partyId];
    emptySlot = 0xFF;
    if (battleCharId != emptySlot) {
        new_var = g_BattleCharIdToCharId[battleCharId];
        charId = new_var;
        D_8009D84C[partyId].hp += amount;
        if (D_8009D84C[partyId].hp > D_8009D84C[partyId].unk12) {
            D_8009D84C[partyId].hp = D_8009D84C[partyId].unk12;
        }
        Savemap.party[charId].hp_cur = D_8009D84C[partyId].hp;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025988);

// Add `amount` MP to the battle record of party slot `partyId`, clamped to the
// member's max MP, then mirror the new value into the save-game party record
// (Savemap.party[charId].mp_cur). Empty party slots (0xFF) are a no-op.
void SystemMenuAddMpByPartyId(s32 partyId, s32 amount) {
    u8 battleCharId;
    s32 new_var;
    s32 emptySlot;
    s32 charId;

    battleCharId = D_8009CBDC[partyId];
    emptySlot = 0xFF;
    if (battleCharId != emptySlot) {
        new_var = g_BattleCharIdToCharId[battleCharId];
        charId = new_var;
        D_8009D84C[partyId].mp += amount;
        if (D_8009D84C[partyId].mp > D_8009D84C[partyId].unk16) {
            D_8009D84C[partyId].mp = D_8009D84C[partyId].unk16;
        }
        Savemap.party[charId].mp_cur = D_8009D84C[partyId].mp;
    }
}

// Deduct `arg0` gil from the party's purse, clamping at zero. Returns the
// amount actually taken. D_8009D260 is Savemap.gil (0x8009C6E4 + 0xB7C) --
// reaching it through the struct is what puts its address in a register.
u32 SystemMenuRemovePartyGold(u32 arg0) {
    u32 gold = Savemap.gil;

    if (gold < arg0) {
        Savemap.gil = 0;
        return gold;
    }
    Savemap.gil = gold - arg0;
    return arg0;
}

// Add `arg0` gil to the party's purse, saturating at 0xFFFFFFFF on overflow.
void SystemMenuAddPartyGold(s32 arg0) {
    u32 gold = Savemap.gil;
    u32 sum = gold + arg0;

    if (sum < gold) {
        Savemap.gil = -1;
        return;
    }
    Savemap.gil = sum;
}

s32 SystemMenuGetPartyGold(void) { return Savemap.gil; }

void func_80025B8C(u_long* image) {
    RECT rect;
    rect.x = 0x340;
    rect.y = 0x184;
    rect.w = 0x30;
    rect.h = 0x78;
    StoreImage(&rect, image);
}

void func_80025BD0(u_long* image) {
    RECT rect;
    rect.x = 0x340;
    rect.y = 0x184;
    rect.w = 0x30;
    rect.h = 0x78;
    LoadImage(&rect, image);
}

void func_80025C14(u_long* image) {
    RECT rect;
    rect.x = 0x180;
    rect.y = 0;
    rect.w = 0x100;
    rect.h = 9;
    StoreImage(&rect, image);
}

void func_80025C54(u_long* image) {
    RECT rect;
    rect.x = 0x180;
    rect.y = 0;
    rect.w = 0x100;
    rect.h = 9;
    LoadImage(&rect, image);
}

void func_80025C94(u_long* image) {
    RECT rect;
    rect.x = 0x100;
    rect.y = 0x1ED;
    rect.w = 0x100;
    rect.h = 3;
    LoadImage(&rect, image);
}

void func_80025CD4(u_long* image) {
    RECT rect;
    rect.x = 0x100;
    rect.y = 0x1ED;
    rect.w = 0x100;
    rect.h = 3;
    StoreImage(&rect, image);
}

void func_80025D14(u_long* addr, s32 px, s32 py, s32 cx, s32 cy) {
    TIM_IMAGE tim;
    OpenTIM(addr);
    while (ReadTIM(&tim)) {
        if (tim.caddr) {
            tim.crect->x = cx;
            tim.crect->y = cy;
            LoadImage(tim.crect, tim.caddr);
            DrawSync(0);
        }
        if (tim.paddr) {
            tim.prect->x = px;
            tim.prect->y = py;
            LoadImage(tim.prect, tim.paddr);
            DrawSync(0);
        }
    }
}

// this function seems to be responsible of loading the characters' portrait
void func_80025DF8(void) {
    u8 dummy[8];
    u8 buf[0x1000];
    u_long* dst;
    s32 i;
    s32* sector_off;
    s32* length;
    s32 cx, cy;

    i = 0;
    dst = (u_long*)buf;
    sector_off = &D_80048FE8->sector_off;
    length = &D_80048FE8->length;
    for (; i < 9; i++) {
        func_80033F40(sector_off[i * 2], length[i * 2], dst, 0);
        cx = 0x340 + (i / 5) * 0x18;
        cy = 0x100 + (i % 5) * 0x30;
        func_80025D14(dst, cx, cy, 0x180, i);
        DrawSync(0);
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80025ED4);

void func_80026034(void) {}

s32 func_8002603C(u8 arg0) {
    return D_80049520[D_80049528[D_800730DC[arg0][1] & 0xF]];
}

void func_80026090(void) {
    do {
    } while (SystemCdromReadChain());
    func_800211C4(7);
    do {
    } while (SystemCdromReadChain());
    func_801D11A8();
    D_80062F90 = 0;
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_800260DC);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80026258);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_800262D8);

// Likely plays a sound effect: writes a sound command (0x30) and the masked
// 16-bit sound id (arg0, duplicated into both parameter words) into the
// sound-request globals, then dispatches via SystemAkaoExecute.
// NOTE: SystemAkaoExecute's own body computes a value in $v0 before
// returning, so its game.h prototype has been corrected to `int`. Its other
// callers across the codebase still discard the result via a bare statement;
// propagating this same int-return pattern to those sibling wrappers may be a
// good change.
static int func_80026408(u16 arg0) {
    D_8009A000[0] = 0x30;
    D_8009A004[0] = arg0;
    D_8009A008[0] = arg0;
    return SystemAkaoExecute();
}

void func_80026448(Unk80026448* arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                   s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10,
                   s32 arg11, s32 arg12, u16 arg13) {
    arg0->unkA = arg1;
    arg0->unkB = arg2;
    arg0->unkC = arg3;
    arg0->unkD = arg4;
    arg0->unk0 = arg5;
    arg0->unk2 = arg6;
    arg0->unk4 = arg7;
    arg0->unk6 = arg8;
    arg0->unkE = arg9;
    arg0->unkF = arg10;
    arg0->unk10 = arg11;
    arg0->unk11 = arg12;
    arg0->unk8 = arg13;
}

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_800264A8);

void func_800269C0(void* arg0) { D_80062F24.poly = arg0; }

void func_800269D0(void) { D_80063008 = D_80062F24; }

void func_800269E8(void) { D_80062F24 = D_80063008; }

void func_80026A00(u_long* arg0) { D_80062FC4 = arg0; }

void func_80026A0C(void) { D_8006300C = D_80062FC4; }

void func_80026A20(void) { D_80062FC4 = D_8006300C; }

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80026A34);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80026A94);

void func_80026B5C(void) {}

void func_80026B64(s8 arg0) { D_80062DFC = arg0; }

// strlen but for FF7 strings
// FF7 string is 0x00: ' ', 0x10: '0', 0x21: 'A', 0xFF: terminator
INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80026B70);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80026C5C);

// print FF7 string
INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80026F44);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_8002708C);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80027354);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80027408);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80027990);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80027B84);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80028030);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80028484);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_800285AC);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80028930);

void func_80028CA0(s16, s16, s16, s16, s16, s16, s16, s16);
INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80028CA0);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80028E00);

INCLUDE_ASM("asm/us/main/nonmatchings/1255C", func_80029114);
