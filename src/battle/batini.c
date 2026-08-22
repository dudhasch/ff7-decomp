#include "battle.h"

extern Unk801B2308 D_80163624;
extern u16 D_8016376C;
void func_800A3354(void); // battle callback for batini, move to battle.h
void func_801B2308(void);

static void func_801B23E0(s32 sceneID, void (*cb)(void));

extern u8 D_800F6934[0x40][8]; // same records as D_800F6936, from offset 0
extern u8 D_800F6B34[0xA][8];
extern s8 D_800F6B86[2][8];
extern u16 D_800F7DE8;
extern u32 D_800FAFD0;
extern u32 D_800F7ED0;
extern u8 D_8009D7BC;
extern u16 D_8009D7BE;
extern s32 D_80062F88;
extern s32 D_801620A8;

/* Carried across a battle at 0x80075D04: a bitmask of slots whose HP is stale,
 * the formation index the HP was saved for, and the HP itself, six enemies per
 * formation. BATINI reaches all three off one base -- the target derives
 * D_80075D04 and D_80075D0C as -4/+4 from the register holding the middle
 * member -- so they have to be one object here or cse cannot relate them. */
typedef struct {
    /* 0x00 */ s32 staleMask;
    /* 0x04 */ s32 formation;
    /* 0x08 */ s32 hp[8][6];
} BattleSavedHp;
extern BattleSavedHp D_80075D04;

void func_80014C44(s32 arg0);
void func_800A3278(void);
void func_800A283C(void);
void func_800AD480(void);
void func_800A71F4(void);
void func_800DCF94(s32 arg0);
void func_800A55BC(void);
void func_801B1CB0(void);
void func_800AE954(s32 slot);
void func_801B19AC(void);
void func_800A4540(void);
void func_801B1120(void);
void func_800A61D4(void);
void func_800A4480(void);
void func_800A5BC8(s32 arg0, s32 arg1);
void func_801B137C(s32 arg0);
void func_800A2894(void);
void func_801B08C0(void);
void func_801B1E0C(void);
void func_801B0668(void);

// entrypoint
void func_801B0050(s32 sceneID) {
    Unk800F83E0* c;
    u8* p;
    s32 i;
    s32 k;
    s32 formation;
    s32 bit;
    s32 hp;

    func_80014C44(VSync(-1));
    VSync(-1);
    for (i = 0; i < 3; i++) {
        func_80020058(i);
        func_8001786C(i);
    }
    func_80017678();
    p = (u8*)func_8001521C(0x7E);
    D_800FAFD0 = p[0];
    D_800F7ED0 = p[1];
    func_800A3278();
    func_800A283C();
    func_800AD480();
    /* The three descending clears walk their own thing and need their own
     * counter: sharing `i` with the ascending loops below stretches that
     * pseudo across all of them, and the -1 for D_801620A8 then loses $v0 to
     * a callee-saved register -- which reorg can legally sink into the
     * preceding call's delay slot where $v0 could not, because the call
     * clobbers $v0. 3 rows, at the same length. Merge counters that describe
     * one walk; these are a different one. */
    for (k = 0x3F; k >= 0; k--) {
        D_800F6934[k][0] = 0xFF;
    }
    for (k = 9; k >= 0; k--) {
        D_800F6B34[k][0] = 0xFF;
    }
    for (k = 1; k >= 0; k--) {
        D_800F6B86[k][0] = -1;
    }
    func_800A71F4();
    D_801620A8 = -1;
    func_800DCF94(-1);
    for (i = 0; i < 10; i++) {
        g_BattleState.combatant[i].unk8 = -1;
        g_BattleState.combatant[i].unk13 = 0x10;
    }
    func_800A55BC();
    func_801B08C0();
    func_801B1CB0();
    func_801B23E0(sceneID, 0);
    func_801B1E0C();
    func_801B085C(D_8009D7BC);
    D_800F5F44.D_800F7DAA = (D_8009D7BE & 0xC0) >> 6;
    for (i = 0; i < 10; i++) {
        func_800AE954(i);
        c = &g_BattleState.combatant[i];
        if (c->unk8 != -1) {
            g_BattleState.presentMask |= 1 << i;
        }
    }
    g_BattleState.sceneID = sceneID;
    D_800F83A8 = D_80163624.unk2;
    func_801B19AC();
    func_800A4540();
    func_801B1120();
    func_801B2308();
    func_800A61D4();
    func_800A4540();
    func_801B0668();
    func_800A4480();
    D_800F7DE8 |= 1;
    for (i = 0; i < 3; i++) {
        func_800A5BC8(i, 1);
    }
    if (g_BattleState.setupFlags & 8) {
        func_801B085C(0x80);
        D_800F5F44.D_800F7DAA = 0;
        for (i = 0; i < 3; i++) {
            func_801B137C(i);
        }
    } else {
        func_800A2894();
    }
    if (g_BattleState.setupFlags & 4) {
        if (!(D_80062F88 & 4)) {
            D_80062F88 |= 4;
            D_80075D04.staleMask = -1;
        }
        formation = g_BattleState.unk28;
        D_80075D04.formation = formation;
        bit = 1 << (formation * 6);
        for (i = 0; i < 6; i++) {
            if (D_80075D04.staleMask & bit) {
                D_80075D04.staleMask &= ~bit;
            } else if (g_BattleState.combatant[i + 4].unk8 != -1) {
                hp = D_80075D04.hp[formation][i];
                g_BattleState.combatant[i + 4].curHP = hp;
                if (hp == 0) {
                    g_BattleState.combatant[i + 4].status |= 1;
                    g_BattleState.combatant[i + 4].unk44 |= 1;
                    g_BattleState.combatant[i + 4].unk4 &= ~0x18;
                }
            }
            bit <<= 1;
        }
        func_800A4540();
    }
}
void func_801B0490(s32 sceneID) {
    Unk800F83E0* c;
    s32 i;
    s32 var_s1;

    var_s1 = 4;
    if (D_8016376C) {
        var_s1 = 0;
        func_800A7254(0, 0, 15, 0);
        func_800A7254(0, 0, 14, 0);
    }
    for (i = 0; i < 0x40; i++) {
        if (D_800F6936[i][0] >= var_s1) {
            D_800F6936[i][0] = -1;
        }
    }
    for (i = 0; i < 3; i++) {
        D_800F5E60[i].unk6 = 0;
    }
    if (D_8016376C) {
        func_801B0F08();
    }
    func_801B23E0(sceneID, func_800A3354);
    func_801B1E0C();
    g_BattleState.presentMask = 0;
    for (i = 0; i < 10; i++) {
        func_800AE954(i);
        c = &g_BattleState.combatant[i];
        if (c->unk8 != -1) {
            g_BattleState.presentMask |= 1 << i;
        }
    }
    g_BattleState.sceneID = sceneID;
    D_800F83A8 = D_80163624.unk2;
    func_801B19AC();
    func_800A4540();
    func_801B2308();
    func_800A4540();
    for (i = 4; i < 10; i++) {
        D_800F5BBC[i][0] = ((u8)func_80014BA8(0x40) + 0x80) << 8;
        func_800B108C(i);
    }
}

extern u16 D_8016375A;
extern u16 D_8009D864[][0x220]; // stride 0x440, one per Unk8009D84C record
u16 func_800B2F50(void);        // random, 16-bit

// Rolls the initial ATB timer of every present combatant and writes it into
// D_800F5BBC. The battle type (D_800F7DC8) then biases those timers: a
// preemptive-style opening zeroes the party's, an ambush pushes it towards the
// enemies, and a Battle Square opening (setup flag 8) overrides both.
void func_801B0668(void) {
    s32 timer[10];
    s32 presentMask;
    s32 max;
    s32 val;
    s32 t;
    s32 i;

    presentMask = D_8016375A;
    max = 0;
    for (i = 0; i < 10; i++) {
        D_800F5BBC[i][0] = 0;
        val = 0;
        if ((presentMask >> i) & 1) {
            val = func_800B2F50() >> 1;
            if (max < val) {
                max = val;
            }
        }
        timer[i] = val;
    }
    for (i = 0; i < 10; i++) {
        if ((presentMask >> i) & 1) {
            switch (D_800F5F44.D_800F7DC8) {
            case 0:
            case 5:
                t = timer[i] + 0xE000;
                timer[i] = t - max;
                break;
            case 2:
            case 4:
                if (i < 4) {
                    timer[i] = 0;
                } else {
                    t = timer[i] + 0xF000;
                    timer[i] = t - max;
                }
                break;
            default:
                if (i < 4) {
                    timer[i] = 0xFFFE;
                } else {
                    timer[i] = timer[i] >> 3;
                }
                break;
            }
            if (g_BattleState.setupFlags & 8) {
                if (i < 3) {
                    timer[i] = 0xFFFE;
                } else {
                    timer[i] = 0;
                }
            }
            D_800F5BBC[i][0] = timer[i];
        }
    }
    for (i = 0; i < 3; i++) {
        D_8009D864[i][0] = D_800F5BBC[i][0];
    }
}

void func_801B085C(s32 arg0) {
    D_800F5F44.D_800F7DA6 = 0x10000 / ((arg0 * 480 / 256 + 0x78) * 2);
}

/* Not started. Reconnaissance so the next pass does not re-derive it:
 *
 * 410 instructions, and the frame opens by zeroing EIGHT stack slots
 * (0x10, 0x18, 0x20, 0x30, 0x38, 0x40, 0x48, 0x50) that the loop body then
 * reloads one at a time -- eight spilled induction variables, all live across
 * the whole outer loop. That is the hardest shape in this codebase: which slot
 * each reduced giv gets is the order `strength_reduce` creates their pseudos,
 * which is the REVERSE of the order it discovers them, i.e. reverse insn order
 * in the body. Read cc1's `.loop` dump rather than inferring it from a diff --
 * see CLAUDE.md's "Two spilled induction variables holding each other's slot".
 *
 * The bases the eight offsets index, read off the preheader:
 *   g_CombatantTurnState            (+0x2A8 = party[0], +0x344 = setup[0])
 *   D_8009CBDC                      a byte per party slot
 *   D_801636B8                      a byte per party slot
 *   D_8009D84C                      stride 0x40 (`sll v1,t4,6`)
 *   g_BattleState.combatant         stride 0x68
 *   g_BattleState.unk10 (D_800F83BC)  zeroed at entry
 *
 * func_801B0490 in this file is the sibling that already matches and shares
 * much of the shape; read it before reading the target. */
INCLUDE_ASM("asm/us/battle/nonmatchings/batini", func_801B08C0);

// The per-party work area at 0x800F5BB8: the turn state, the three party
// records (D_800F5E60) and their setup config (D_800F5EFC) are one object, so
// BATINI reaches all three off a single base.
typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4[0xA];
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u8 unk10[4];
    /* 0x14 */ s32 unk14;
} Unk800F5EFC; // size:0x18

typedef struct {
    /* 0x000 */ Unk800AF470 turn[10];
    /* 0x2A8 */ Unk800F5E60 party[3];
    /* 0x344 */ Unk800F5EFC setup[3];
} BattleWork; // size:0x38C

extern BattleWork g_CombatantTurnState;
extern SavePartyMember D_8009C738[];
void func_801B1598(s32 slot, s32 accessory);
void func_801B11BC(s32 slot);
void func_800A4BA4(s32 slot);
s32 func_801B1734(s32 slot);

// Seeds the three live party slots from the save data: finds each slot's
// party member record, copies HP/MP and the derived battle stats across, then
// applies the equipped accessory, the command list and the row/limit setup.
void func_801B0F08(void) {
    Unk800F5E60* party;
    Unk8009D84C* rec;
    Unk800F83E0* c;
    Unk800AF470* t;
    Unk800F5EFC* setup;
    SavePartyMember* m;
    s32 id;
    s32 i;
    s32 j;

    for (i = 0; i < 3; i++) {
        t = &g_CombatantTurnState.turn[i];
        party = &g_CombatantTurnState.party[i];
        rec = &D_8009D84C[i];
        c = &g_BattleState.combatant[i];
        setup = &g_CombatantTurnState.setup[i];
        id = D_8009CBDC[i];
        if (id != 0xFF) {
            for (j = 0; j < 9; j++) {
                m = &D_8009C738[j];
                if (m->char_id == id) {
                    c->unk9 = m->level;
                    c->curHP = m->hp_cur;
                    c->unk28 = m->mp_cur;
                    t->unk3C = c->curHP;
                    t->unk3E = c->unk28;
                    func_801B18F8(rec, party, c);
                    t->unk34 = rec->unk48;
                    setup->unkE = rec->un418 | rec->unk3C;
                    setup->unk14 = rec->unk44;
                    setup->unk3 = rec->un410;
                    setup->unk0 = rec->un408;
                    t->unk29 &= 0xFD;
                    if (rec->unk23 & 4) {
                        setup->unk0 &= 0xDF;
                    }
                    if (!(setup->unk0 & 0x20)) {
                        t->unk29 |= 2;
                    }
                    func_801B1598(i, m->accessory);
                    func_801B11BC(i);
                    func_800A4BA4(i);
                    if (func_801B1734(i) == 0) {
                        func_800B108C(i);
                    }
                    break;
                }
            }
        }
    }
}

extern void func_800A6000(s32, s32, s32);

void func_801B1120(void) {
    s32 i;

    for (i = 0; i < 3; i++) {
        if (((s8)D_80163624.unk94[i][0] != -1) &&
            !(g_BattleState.combatant[i].status & 1)) {
            func_800A6000(i, 0, 0);
        }
    }
}

extern u8 D_800707C5[][8];    // command table, 8-byte stride
extern u8 D_800708D0[][0x1C]; // attack table, 0x1C stride
extern u8 D_800F5EFC[][0x18]; // per-slot formation-setup config
extern u8 D_800F5BE1[][0x44]; // same records as D_800F5BBC

// Fixes up party member arg0's battle command list: each of the 16 command
// slots gets its target flags from the command table (falling back to the
// formation setup), with extra flags for the Enemy Skill / W- commands, and
// unk21 ends up as the number of command rows in use. The second pass clears
// the "usable" byte of every equipped materia whose attack is not flagged
// battle-usable.
void func_801B11BC(s32 arg0) {
    Unk8009D84C* e;
    s32 cmd;
    s32 flags;
    s32 id;
    s32 i;

    e = &D_8009D84C[arg0];
    e->unk21 = 1;
    for (i = 0; i < 16; i++) {
        flags = 0xFF;
        cmd = e->un4C[i][0];
        if (cmd != 0xFF) {
            flags = D_800707C5[cmd][0];
            if (flags == 0xFF) {
                flags = D_800F5EFC[arg0][0];
            }
            if (cmd < 0x1C) {
                if (cmd >= 0x18) {
                    e->un4C[i][4] = 0xFF;
                }
            }
            if (e->un4C[i][1] == 7) {
                if (D_800F5BE1[arg0][0] & 2) {
                    e->un4C[i][1] = 0;
                }
                if (e->un4C[i][4] != 0) {
                    if (e->un4C[i][0] != 0x19) {
                        flags |= 0xC;
                    }
                }
                cmd = e->un4C[i][0];
                if (cmd == 5 || cmd == 0x11) {
                    flags |= 0x10;
                    if (e->un4C[i][4] != 0) {
                        e->un4C[i][1] = 0;
                    }
                }
            }
            e->unk21 = i / 4 + 1;
        }
        e->un4C[i][2] = flags;
    }
    for (i = 0; i < 0x60; i++) {
        id = e->unk108[i].unk0;
        if (id != 0xFF) {
            if (i >= 0x48) {
                id += 0x48;
            } else if (i >= 0x38) {
                id += 0x38;
            }
            if (i < 0x38) {
                if (!(D_800708D0[id][0] & 8)) {
                    e->unk108[i].unk2 = 0;
                }
            }
        }
    }
}

void func_801B137C(s32 arg0) {
    s32 i;
    Unk8009D84C* unk;

    unk = &D_8009D84C[arg0];
    unk->unk21 = 1;
    for (i = 1; i < 4; i++) {
        unk->un4C[i][0] = 0xFF;
        unk->un4C[i][1] = 0;
        unk->un4C[i][2] = 0;
        unk->un4C[i][3] = 3;
        unk->un4C[i][4] = 0;
        unk->un4C[i][5] = 0;
    }
}

s32 func_80015AFC(s32, s32); // extern

typedef struct {
    /* 00 */ u8 materiaID[3];
    /* 03 */ u8 unk3[3];
    /* 06 */ u8 count;
    /* 07 */ u8 unk7;
    /* 08 */ u8 unk8[0xC];
    /* 14 */ struct {
        u8 unk0;
        u8 unk1[0x1B];
    } unk14[3];
} Unk801B13DC; // size:0x68

// Resolves up to three materia slots of character arg0 against the equipment
// mask arg1: a slot whose bit is set copies its paired value into unk3 and
// counts towards unk6.
void func_801B13DC(s32 arg0, s32 arg1, Unk801B13DC* arg2) {
    s32 count;
    s32 i;
    s32 j;

    count = 0;
    for (i = 0; i < 3; i++) {
        if (arg2->materiaID[i] != 0xFF) {
            for (j = 0; j < 12; j++) {
                if (func_80015AFC(arg0, j) == arg2->materiaID[i]) {
                    break;
                }
            }
            if (j == 12) {
                func_800155A4(0x26);
            } else if ((arg1 >> j) & 1) {
                count++;
                arg2->unk3[i] = arg2->unk14[i].unk0;
            }
        }
    }
    arg2->unk7 = 0;
    arg2->count = count;
}

s32 func_801B14E8(u32 arg0) {
    u8 temp_v1;
    s32 ret;

    temp_v1 = arg0;
    ret = 0;
    if (temp_v1 != 0xFF && (D_800730CC[temp_v1].unk11 & 0xF) == 7) {
        ret = (arg0 >> 8) | 0x80000000;
    }
    return ret;
}

s32 func_801B1530(u32* arg0) {
    s32 ret;
    s32 i;

    ret = 0;
    for (i = 0; i < 8; i++) {
        ret |= func_801B14E8(arg0[0x10 + i]);
        ret |= func_801B14E8(arg0[0x18 + i]);
    }
    return ret;
}

extern u8 D_80071C29[][0x10]; // accessory table, 0x10 stride

// Applies party member `slot`'s equipped accessory: the status the previously
// equipped one granted is cleared first, then the new accessory's permanent
// status is ORed into the combatant, its turn state and the party record.
void func_801B1598(s32 slot, s32 accessory) {
    Unk800AF470* t;
    Unk800F5E60* party;
    Unk800F83E0* c;
    u8 effect;

    t = &g_CombatantTurnState.turn[slot];
    party = &g_CombatantTurnState.party[slot];
    c = &g_BattleState.combatant[slot];
    c->status &= ~party->unk20;
    t->unk34 &= ~party->unk20;
    party->unk20 = 0;
    t->unkD = 0xFF;
    if (accessory != 0xFF) {
        effect = D_80071C29[accessory][0];
        t->unkD = effect;
        switch (effect) {
        case 0:
            c->status |= STATUS_HASTE;
            t->unk34 |= STATUS_HASTE;
            party->unk20 |= STATUS_HASTE;
            break;
        case 1:
            c->status |= STATUS_BERSERK;
            t->unk34 |= STATUS_BERSERK;
            party->unk20 |= STATUS_BERSERK;
            break;
        case 2:
            c->status |= STATUS_D_SENTENCE;
            t->unk34 |= STATUS_D_SENTENCE;
            party->unk20 |= STATUS_D_SENTENCE;
            t->unk12 = 0xFF;
            break;
        case 3:
            c->status |= STATUS_REFLECT;
            t->unk34 |= STATUS_REFLECT;
            party->unk20 |= STATUS_REFLECT;
            break;
        case 6:
            c->status |= STATUS_BARRIER | STATUS_M_BARRIER;
            party->unk20 |= STATUS_BARRIER | STATUS_M_BARRIER;
            break;
        }
    }
}

const s32 D_801B001C[] = {0x0000, 0x1000, 0x0008, 0x0800};
const s32 D_801B002C[] = {0x0000, 0x000A, 0x0027, 0x000A};
extern u8 D_800F9DA0; // pending battle-start status flags, one bit per entry
                      // of D_801B001C / D_801B002C (bit 4 = full-heal)
void func_800A7254(s32, s32, s32, s32);

// Applies the pending battle-start effects in D_800F9DA0 to party member
// `slot`: bit 4 restores half its max HP, bits 0-3 inflict the matching status
// from D_801B001C unless the member's turn state already carries it. Returns
// nonzero if any status was inflicted.
s32 func_801B1734(s32 slot) {
    s32 mask;
    s32 ret;
    s32 i;

    mask = g_CombatantTurnState.turn[slot].unk34;
    g_BattleState.combatant[slot].status &= ~STATUS_D_SENTENCE;
    ret = 0;
    if (g_CombatantTurnState.turn[slot].unk29 & 8) {
        mask |= STATUS_FROG;
    }
    if (D_800F9DA0 & 0x10) {
        g_BattleState.combatant[slot].curHP +=
            g_BattleState.combatant[slot].maxHP >> 1;
        if (g_BattleState.combatant[slot].curHP >
            g_BattleState.combatant[slot].maxHP) {
            g_BattleState.combatant[slot].curHP =
                g_BattleState.combatant[slot].maxHP;
        }
        func_800A7254(2, slot, 0x17, 0);
    }
    for (i = 0; i < 4; i++) {
        if ((D_800F9DA0 >> i) & 1) {
            g_BattleState.combatant[slot].status |= D_801B001C[i] & ~mask;
            func_800A7254(2, slot, 0x17, D_801B002C[i]);
            ret = 1;
        }
    }
    return ret;
}

void func_801B18F8(Unk8009D84C* arg0, Unk800F5E60* arg1, Unk800F83E0* arg2) {
    arg2->unk14 = arg0->unk6;
    arg2->unk15 = arg0->unk7;
    arg2->maxHP = arg0->unk12;
    arg2->unk2A = arg0->unk16;
    arg2->unkD = arg0->unk8;
    arg2->unkE = arg0->unkC;
    arg2->unk20 = arg0->unkA;
    arg2->unk22 = arg0->unkE;
    if (arg2->unkD == 0) {
        arg2->unkD = 1;
    }
    arg1->maxHP = arg2->maxHP;
    arg1->maxMP = arg2->unk2A;
    if (arg0->unk23 & 8) {
        arg1->capHP = 999;
        arg1->capMP = 9999;
    } else {
        arg1->capHP = 9999;
        arg1->capMP = 999;
    }
}

const u8 D_801B003C[] = {0xFF, 0x32, 0x33, 0x34, 0x35, 0xFF, 0x48, 0x07};
extern u16 D_8016376E[3];
extern s16 D_801636BE[][8]; // stride 0x10
void func_800B1060(s32);

// Lays out the two sides for the opening of the battle. D_801B003C picks the
// intro animation for the battle type, then the type decides which rows the
// party and the enemies occupy (row[0]/row[1]/row[2]) and which combatants
// start "turned around" (bit 0x80 of unk4) -- back attacks, side attacks and
// pincers each split the party differently. Finally the front/back row bit is
// re-derived for the three party slots.
void func_801B19AC(void) {
    u16 row[3];
    s32 enemyMask;
    s32 partyMask;
    s32 sideMask;
    u16 mask;
    s32 back;
    s32 intro;
    s32 i;

    enemyMask = g_BattleState.unk12;
    partyMask = g_BattleState.unk10;
    sideMask = 5;
    if (D_8016360C.setup.type == SETUP_SIDE_ATTACK_3) {
        sideMask = ~5;
    }
    intro = D_801B003C[D_800F5F44.D_800F7DC8];
    if (intro != 0xFF && g_BattleState.sceneID != 0x3D6) {
        func_800B1060(intro);
    }
    mask = 0;
    row[0] = 0;
    row[1] = 0;
    row[2] = 0;
    switch (D_800F5F44.D_800F7DC8) {
    case 0:
        mask = enemyMask;
        /* fallthrough */
    case 1:
        row[0] = partyMask;
        row[1] = enemyMask;
        break;
    case 2:
        mask = partyMask;
        row[0] = enemyMask;
        row[1] = mask;
        break;
    case 4:
        row[1] = partyMask;
        for (i = 0; i < 6; i++) {
            if ((enemyMask >> (i + 4)) & 1) {
                row[g_BattleState.combatant[i + 4].unk4 & 2] |= 1 << (i + 4);
            }
        }
        mask = row[2] | (partyMask & 2);
        if (g_BattleState.sceneID == 0x3D6) {
            mask &= ~partyMask;
        }
        break;
    default:
        row[0] = partyMask & sideMask;
        row[1] = enemyMask;
        row[2] = partyMask & ~sideMask;
        mask = row[2];
        for (i = 0; i < 6; i++) {
            if (((enemyMask >> (i + 4)) & 1) && D_80163624.unk34[i].unk6 >= 0) {
                mask |= 1 << (i + 4);
            }
        }
        break;
    }
    for (i = 0; i < 10; i++) {
        g_BattleState.combatant[i].unk4 &= ~0x82;
        if ((row[2] >> i) & 1) {
            g_BattleState.combatant[i].unk4 |= 2;
        }
        if ((mask >> i) & 1) {
            g_BattleState.combatant[i].unk4 |= 0x80;
        }
    }
    for (i = 0; i < 3; i++) {
        back = g_BattleState.combatant[i].unk4 >> 6;
        back &= 1;
        switch (D_800F5F44.D_800F7DC8) {
        case 0:
        case 1:
            break;
        case 2:
            back = !back;
            g_BattleState.combatant[i].unk4 ^= 0x40;
            break;
        default:
            back = 0;
            g_BattleState.combatant[i].unk4 &= ~0x40;
            break;
        }
        D_801636BE[i][0] = back;
    }
    D_8016376E[0] = row[0];
    D_8016376E[1] = row[1];
    D_8016376E[2] = row[2];
}

extern u16 D_8009CBE0[];     // item inventory (320 slots; (count << 9) | id)
extern u16 D_800722D6[][14]; // item table, stride 0x1C
extern u8 D_800722D8[][28];
extern u16 D_800738CA[][22]; // weapon table, stride 0x2C
extern u8 D_800738A0[][44];
extern u16 D_80071E64[][18]; // armor table, stride 0x24
extern u16 D_80071C32[][8];  // accessory table, stride 0x10
extern u8 D_80166F74;
extern BattleItemEntry D_801671B8[];

// Builds the in-battle item list from the inventory: every one of the 320
// inventory slots becomes one BattleItemEntry, with the target and restriction
// flags pulled from the item / weapon / armor / accessory table the id falls
// in. D_80166F74 ends up as half the number of slots up to the last used one
// (at least 3) -- the row count the item widget scrolls over.
void func_801B1CB0(void) {
    BattleItemEntry* entry;
    s32 i;
    s32 last;
    s32 rows;
    s32 id;
    s32 count;
    s32 targetFlags;
    s32 flags;

    last = 0;
    for (i = 0; i < 0x140; i++) {
        entry = &D_801671B8[i];
        id = D_8009CBE0[i];
        count = 0;
        targetFlags = 0;
        flags = 0xB;
        if (id != 0xFFFF) {
            count = (u32)id >> 9;
            id &= 0x1FF;
            if (id < 0x80) {
                flags = D_800722D6[id][0] & 0xB;
                targetFlags = D_800722D8[id][0];
            } else if (id < 0x100) {
                flags = D_800738CA[id - 0x80][0] & 0xB;
                targetFlags = D_800738A0[id - 0x80][0];
            } else if (id < 0x120) {
                flags = D_80071E64[id - 0x100][0] & 0xB;
                targetFlags = 3;
            } else if (id < 0x140) {
                flags = D_80071C32[id - 0x120][0] & 0xB;
                targetFlags = 3;
            }
            last = i + 1;
        }
        entry->id = id;
        entry->count = count;
        entry->targetFlags = targetFlags;
        entry->unk4 = flags;
    }
    rows = (last + 1) / 2;
    if (rows < 3) {
        rows = 3;
    }
    D_80166F74 = rows;
}

extern u8 D_800F7DD4[3]; // per-loaded-model count of formation entries using it
extern u32 D_800F89F0[6][0x20];
extern u8 D_80166F78[6][0x10][6];

/* Builds the six enemy combatants out of the loaded formation: matches each
 * formation entry against the three loaded enemy models, copies that model's
 * stats into the combatant record, and resolves its sixteen attack slots
 * against the scene's attack-id table.
 *
 * 50 rows at the EXACT target length of 319, from 165. The four levers that
 * got it there are all *program* facts, not codegen knobs, and each is written
 * up in CLAUDE.md:
 *
 *   1. `Unk800F83E0.unk52` was declared `s16`, so `c->unk52 = 0xFFFF` folded
 *      to a QI/HImode -1 and shared a movable with `c->unk8 = -1` and the
 *      `(s16)enemyID != -1` test. Three uses made it worth hoisting, it took a
 *      callee-saved register the target spends on `&D_800F5F44.attackIDs`,
 *      and the displaced pseudo -- the `i * 0x60` snapshot the attack loops
 *      index off -- spilled, growing the frame to 0x38 against 0x30.
 *      Retyping the member `u16` (its only user is this function) is worth
 *      **59 rows and the whole frame**: the constant becomes `ori 0xffff`,
 *      cannot share, nothing is hoisted, and the spill goes with it.
 *   2. `c->maxHP = e->maxHP; c->curHP = e->maxHP;` reloads, because the first
 *      store may alias the load. `c->curHP = c->maxHP = e->maxHP;` loads once
 *      and stores 0x30 then 0x2c, which is the target's order. Same for
 *      `unk2A`/`unk28`. -4 instructions.
 *   3. A walking pointer's initialiser belongs in the `for` init clause.
 *      +2 instructions and 53 -> 50 rows across the three clear loops.
 *   4. One walking pointer per clear loop, not one reused. 53 -> 50.
 *
 * What is left is five clusters, every one of them a sched2 or allocation
 * permutation of code that is already the right length and the right program:
 *
 *   R2 (4 rows)  the target copies the model index out of the match loop's
 *                counter -- `move a0,a2` -- and stores the copy to
 *                `.enemyID` and `c->unk8` while the counter still serves the
 *                `(s16)` cast and the two `D_800F7DD4` indexes. A named
 *                `s32 mi = j;` is EXACTLY inert (cse propagates it away) and
 *                `u8 mi` is 65 rows.
 *   R3 (8 rows)  `li 1` for `c->unk11`/`c->unk4C` fills the `lw 0xac` load
 *                delay slot here and the `lw 0xa8` one in the target, which
 *                also reorders `sb 0x11` above `sw 0x58`. Store order in the
 *                source already matches the target's.
 *   R4 (4 rows)  `&g_BattleState.unk12` is materialised 14 slots early.
 *   R5 (13 rows) the biggest: the target issues `lhu 0x94(t2)` *before* the
 *                four `atk` stores, keeps the value in one register across the
 *                inner loop with an `andi v1,v1,0xffff`, and fills the `beq`
 *                delay slot with `atk[3] = 3`. This build stores first, loads
 *                after, and pays a `move t0,a0` instead of the `andi` -- same
 *                instruction count, opposite structure. The `andi` is the
 *                zero-extension surviving as its own insn; here combine folds
 *                it into the `lhu` and the cross-block copy appears instead.
 *   R6 (10 rows) the second fill loop issues its giv increment and `slti`
 *                before the four stores; the target issues them after.
 *
 * Measured and rejected against THIS body (re-measure before trusting -- the
 * note's predecessor's numbers were all taken against the 165-row body):
 *   attackID re-read inline at both compares          152 rows
 *   attackID as s32 / u32                              57 rows, -1 insn
 *   attackID as s16 / u8 / s8                     120-125 rows, -4/-5 insns
 *   `(s32)attackID` in the inner compare                50 (inert)
 *   `0xFFFF == attackID` operand order                  50 (inert)
 *   `continue` rewritten as an `if` block                50 (inert)
 *   a second u16 local for the inner compare             50 (inert)
 *   `*(atk + k)` instead of `atk[k]`                     50 (inert) -- p[k] on
 *      a POINTER is already an INDIRECT_REF, so it never had MEM_IN_STRUCT_P
 *      to clear; CLAUDE.md's ARRAY_REF rule is about real arrays
 *   `atk = ...` moved after the load                    106 rows, -1 insn
 *   the four stores as `D_80166F78[i][j][k]`            212 rows
 *   a separate pointer for the second fill loop         105 rows
 *   a `u16* ids` local for the attack-id table          worse at every slot
 *   `do { } while (0);` at eight positions        50 (one), 59-131 (seven)
 *   width_sweep over all seven scalar locals      flat at 50, nothing better
 *
 * The residue is `perm_ins_block` / `perm_temp_for_expr` shaped, not allocno
 * arithmetic, so it is a legitimate permuter target.
 *
 * Program facts already established -- read off the target, not up for
 * re-derivation:
 *   * the `-1` formation entry skips to `t->unk29 = 0`, which is the loop
 *     body's LAST statement, so the guard is an `if`, not a `continue`;
 *   * `D_8016360C.enemyModelIDs[j]` is read SIGNED (`lh`) in the match loop
 *     while `formation[i].enemyID` is read UNSIGNED (`lhu`) -- hence the u16
 *     local plus the `*(s16*)&` cast;
 *   * `Unk800F83E0.unk24` is `u16` (`lhu` at 0x24 in the duplicate-count
 *     loop), and 0x44..0x67 is not an array of words -- see battle.h;
 *   * `formation[i].flags` and `.row` are reached by their own scaled index,
 *     not through the pointer that serves `.enemyID`; a shared `fm` pointer
 *     collapses three %hi materialisations into one and is 3 instructions.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/battle/nonmatchings/batini", func_801B1E0C);
#else
void func_801B1E0C(void) {
    Unk800F83E0* c;
    Unk800AF470* t;
    SceneEnemy* e;
    u8* atk;
    u8* p;
    u8* q;
    u8* q2;
    s32 i;
    s32 j;
    s32 k;
    s32 m;
    s32 n;
    u16 enemyID;
    u16 attackID;

    g_BattleState.unk12 = 0;
    for (i = 2, p = &D_800F7DD4[2]; i >= 0; i--, p--) {
        *p = 0;
    }
    for (i = 0; i < 6; i++) {
        for (j = 0x1F; j >= 0; j--) {
            D_800F89F0[i][j] = 0;
        }
    }
    for (i = 0; i < 6; i++) {
        t = &g_CombatantTurnState.turn[i + 4];
        c = &g_BattleState.combatant[i + 4];
        enemyID = D_8016360C.formation[i].enemyID;
        c->unk8 = -1;
        c->unk24 = 0xFFFF;
        c->unk4 = 0;
        c->status = 0;
        c->unk4F = 0xFF;
        D_800F6B34[i + 4][0] = 0xFF;
        /* 0x10..0x1F and 0x20..0x27 are cleared as two byte runs; battle.c
         * needs the named fields there, so the runs are spelled as a byte
         * view. An ARRAY_REF on a cast pointer still sets MEM_IN_STRUCT_P,
         * so this aliases exactly as a real `u8 [0x10]` member would. */
        for (j = 0xF, q = (u8*)t + 0xF; j >= 0; j--, q--) {
            q[0x10] = 0;
        }
        for (j = 7, q2 = (u8*)t + 7; j >= 0; j--, q2--) {
            q2[0x20] = 0;
        }
        if ((s16)enemyID != -1) {
            c->unk24 = enemyID;
            for (j = 0; j < 3; j++) {
                if (*(s16*)&D_8016360C.enemyModelIDs[j] == (s16)enemyID) {
                    break;
                }
            }
            D_800F7DD4[j]++;
            e = &D_800F5F44.enemy[(s16)j];
            D_8016360C.formation[i].enemyID = j;
            c->unk8 = j;
            c->curHP = c->maxHP = e->maxHP;
            c->unk28 = c->unk2A = e->unk9C;
            c->unkD = e->strength;
            c->unkE = e->magic;
            c->unk20 = e->defense * 2;
            c->unk22 = e->magicDef * 2;
            c->unkF = e->evade;
            c->unk14 = e->speed;
            c->unk15 = e->luck;
            c->unk9 = e->level;
            c->unk12 = e->unkA2;
            c->unk58 = e->unkAC;
            c->unk11 = 1;
            c->unk5C = e->unkA8;
            c->unk4C = 1;
            c->unk56 = 2;
            c->unk10 = 0;
            c->status = 0;
            c->unk44 = 0;
            c->unk50 = 0;
            c->unk52 = 0xFFFF;
            c->unk4 = D_8016360C.formation[i].flags & 0x1F;
            c->unk4E = D_8016360C.formation[i].row;
            t->unk38 = (s32)e;
            t->unkD = 0xFF;
            t->unkC = 0xFF;
            t->unkF = 0xFF;
            t->unk34 = ~e->unkB0;
            g_BattleState.unk12 |= 1 << (i + 4);
            c->unkC = 0;
            for (j = 0; j < i; j++) {
                if (g_BattleState.combatant[j + 4].unk24 == c->unk24) {
                    c->unkC++;
                }
            }
            n = 3;
            for (j = 0; j < n; j++) {
                atk = D_80166F78[i][j];
                atk[0] = 0xFF;
                atk[1] = 0;
                atk[2] = 0;
                atk[3] = 3;
                attackID = e->attackIndex[j];
                if (attackID == 0xFFFF) {
                    continue;
                }
                for (m = 0; m < 0x20; m++) {
                    if (D_800F5F44.attackIDs[m] == attackID) {
                        k = D_800F5F44.attacks[m].targetFlags;
                        if (k != 0) {
                            k ^= 2;
                        }
                        atk[0] = m;
                        atk[2] = k;
                        atk[3] = 0;
                        break;
                    }
                }
            }
            for (j = n; j < 0x10; j++) {
                atk = D_80166F78[i][j];
                atk[0] = 0xFF;
                atk[1] = 0;
                atk[2] = 0;
                atk[3] = 3;
            }
        }
        t->unk29 = 0;
    }
    for (i = 0; i < 6; i++) {
        if (D_800F7DD4[D_8016360C.formation[i].enemyID] >= 2) {
            g_CombatantTurnState.turn[i + 4].unkF =
                g_BattleState.combatant[i + 4].unkC;
        }
    }
}
#endif

void func_801B2308(void) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if (D_80163624.unk34[i].unk0 != -1) {
            func_800A6000(i + 4, 0, 0);
        }
    };
    for (i = 0; i < 6; i++) {
        D_80163624.unk34[i].unkC = g_BattleState.combatant[4 + i].unk4;
        D_80163624.unk94[4 + i][1] = g_BattleState.combatant[4 + i].unk10;
        g_BattleState.combatant[4 + i].unk44 =
            g_BattleState.combatant[4 + i].status;
    }
}

static const s8 D_801B0044[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x03, 0x03, 0x03, 0x05, 0x6E, 0x64, 0x62};
static void func_801B23E0(s32 sceneID, void (*cb)(void)) {
    u8 dummy[0x100];
    SceneContainer scene;
    s32 chunkID;
    s32 temp_s1;
    s32 formationIndex;
    s32 i;
    u_long* var_s2;
    s32* var_s5;
    s32* var_s3_2;

    var_s5 = (s32*)0x801C0000;
    chunkID = sceneID / 4;
    temp_s1 = func_801B2738(chunkID); // sector modified based on the Chunk ID
    SystemLoadFileBySector(           // load file from disk
        func_800144D8(BATTLE_SCENE) +
            temp_s1 * 4, // Disk sector where to load the file from
        0x800 * 4,       // Size in bytes to copy
        (u_long*)var_s5, // Destination
        NULL);
    formationIndex = chunkID - D_80083184[temp_s1];
    func_800145BC(cb); // wait until all data is read, keep executing the vsync
                       // callback until then
    i = var_s5[formationIndex];
    var_s3_2 = &var_s5[i];
    var_s2 = (u_long*)&scene;
    func_80017108( // gzip decompress
        var_s3_2,  // src
        var_s2);   // dst
    formationIndex = sceneID - chunkID * 4;
    func_80014A00(D_8016360C.enemyModelIDs, scene.enemyModelIDs,
                  sizeof(scene.enemyModelIDs));
    func_80014A00((s32*)&D_8016360C.setup, &scene.setup[formationIndex],
                  sizeof(BattleSetup));
    func_80014A00((s32*)&D_8016360C.camera, &scene.camera[formationIndex],
                  sizeof(CameraPlacement) * 4);
    func_80014A00((s32*)&D_8016360C.formation, &scene.formation[formationIndex],
                  sizeof(FormationEntry) * 6);
    func_80014A00((s32*)&D_800F5F44.enemy, &scene.enemy, sizeof(scene.enemy));
    func_80014A00(
        (s32*)&D_800F5F44.attacks, &scene.attacks, sizeof(scene.attacks));
    func_80014A00(
        (s32*)&D_800F5F44.attackIDs, scene.attackIDs, sizeof(scene.attackIDs));
    func_80014A00((s32*)&D_800F5F44.attackNames, &scene.attackNames,
                  sizeof(scene.attackNames));
    func_80014A00((s32*)&D_800F5F44._5, &scene.unkC80, sizeof(Unk800F5F44_5));
    func_80014A00(
        (s32*)&D_800F5F44.script, &scene.script, sizeof(scene.script));
    if (D_8016376A & 4 && D_8016360C.setup.flags & SETUP_NO_PREEMPTIVE_STRIKE) {
        if (D_8016360C.setup.type == SETUP_DEFAULT) {
            D_8016360C.setup.type = SETUP_PREEMPTIVE;
        }
    }
    D_800F5F44.D_800F7DC8 = (u8)D_801B0044[D_8016360C.setup.type];
    if (D_8016376A & EVENT_BATTLE_SQUARE) {
        D_8016360C.setup.stageID = 37;
        D_8016360C.setup.flags |= SETUP_CANNOT_ESCAPE;
        D_8016360C.setup.cameraID = (func_80014B70() & 3) + 0x60;
        D_8016360C.setup.escapeCounter = 1;
        // enemy strength and magic is 25% higher at battle square
        for (i = 0; i < 3; i++) {
            D_800F5F44.enemy[i].maxHP *= 2;
            D_800F5F44.enemy[i].strength =
                func_801B2770(D_800F5F44.enemy[i].strength);
            D_800F5F44.enemy[i].magic =
                func_801B2770(D_800F5F44.enemy[i].magic);
        }
    } else if (D_8016376A & 8) {
        D_8016360C.setup.flags &= ~SETUP_CANNOT_ESCAPE;
    }
    if (!(D_8016360C.setup.flags & SETUP_CANNOT_ESCAPE)) {
        D_8016376A |= 8;
    }
    D_800F5F44.D_800F7DB2 = D_8016360C.setup.escapeCounter;
    if (D_800F5F44.D_800F7DC8 == 1 || D_800F5F44.D_800F7DC8 == 3) {
        D_800F5F44.D_800F7DB2 = 1;
    }
    D_800F5F44.D_800F7DB6 = D_800F5F44.D_800F7DB2;
}

s32 func_801B2738(s32 arg0) {
    u32 i;

    for (i = 1; i < LEN(D_80083184); i++) {
        if (arg0 < D_80083184[i]) {
            break;
        }
    }
    return i - 1;
}

// increase param by 25%
s32 func_801B2770(s32 arg0) {
    arg0 = (arg0 * 125) / 100;
    if (arg0 > 255) {
        arg0 = 255;
    }
    return arg0;
}
