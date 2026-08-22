//! G=8
#include "main_private.h"

void func_80015B44(s32 arg0);
u8* func_80014C80(s32 arg0);
s32 func_80015B50(void);
s32 func_80015B88(void);
extern u8 D_80083084[];
typedef struct {
    u8 count;
    u8 value;
    u8 unk2[3];
} Unk80069556;
extern Unk80069556 D_80069556[];
extern u8 D_800694C4[];
extern u8 D_800694D4[];
extern u8 D_80063048[];
extern void* D_800707C0;
extern u8 D_80082274[];
extern volatile u8 D_8009AC2C[];
extern volatile u16 D_8009AC42;
extern u16 D_8009AC44;
void func_80014804(void);
/* main's .bss, addressed `%gp_rel(<sym>)($gp)` by the target. These are
 * *tentative definitions*, not `extern` declarations, and the difference is
 * the whole addressing form: this unit is compiled `-G8`, so cc1 emits a
 * tentative definition of a small object as a `.comm`, and maspsx (given
 * `--use-comm-section -G8` by tools/ninja/gen.py) then reaches it through
 * `$gp` in one instruction. Declared `extern`, cc1 emits only
 * `.extern <sym>,<size>`, nothing is small, and every access is the two- or
 * three-instruction `%hi`/`%lo` pair the target does not have. The real
 * definitions live in `asm/us/main/data/536C4.bss.s`; `--use-comm-section`
 * keeps these COMMON so the link binds to those rather than seeing two.
 * Widths are read off the target's opcodes, not guessed. */
s8 D_80062FFC;
u8 D_80063020;
Unk80062F7C* D_80062F7C;
s32 D_80062F10;
s32 D_80062FBC;
s32 D_80062F14;
s32 D_80062F58;
s32 D_80062F60;
s32 D_80062F88;
s32 D_80062F9C;
u8 D_80062FEC;
s32 D_80062FF0;
u16 D_80063018;
Unk800A8D04* g_CurrentAction;

s32 D_80062D4C = 0x00000000;
s32 D_80062D50 = 0x000000FF;
s32 D_80062D54 = 0x00000000;
s32 D_80062D58 = 0x00000009;
s32 D_80062D5C = 0x00000006;
s16 D_80062D60 = 0x0000;
s16 D_80062D62 = 0x0000;
s16 D_80062D64 = 0x0000;
s16 D_80062D66 = 0x0000;
s16 D_80062D68 = 0x0000;
s16 D_80062D6A = 0x0000;
s16 D_80062D6C = 0x0000;
s16 D_80062D6E = 0x0000;
u8 g_bPadsInitialized = 0x00;
s8 D_80062D71 = 0x00;
s16 D_80062D72 = 0x0000;
s16 D_80062D74 = 0x0000;
s16 D_80062D76 = 0x0000;
u16 D_80062D78 = 0x0000;
s16 D_80062D7A = 0x0000;
u16 D_80062D7C = 0x0000;
u16 D_80062D7E = 0x0000;
u16 D_80062D80 = 0x0000;
u16 D_80062D82 = 0x0000;
s32 D_80062D84 = 0x00000000;
s16 D_80062D88 = 0x0000;
s16 D_80062D8A = 0x0000;
s32 D_80062D8C = 0x00000000;
s32 D_80062D90 = 0x00000000;
s32 D_80062D94 = 0x00000000;
u8 D_80062D98 = 0x00;
u8 D_80062D99 = 0x00;
static s16 D_80062D9A = 0x0000;
s32 D_80062D9C = 0x00000000;
s32 D_80062DA0 = 0x00140000;
s32 D_80062DA4 = 0x007800A0;
s32 D_80062DA8 = 0x00010002;
s32 D_80062DAC = 0x00060006;
s32 D_80062DB0 = 0x00010001;
s16 D_80062DB4 = 0x0007;
s16 D_80062DB6 = 0x0000;
s16 D_80062DB8 = 0x0000;
s16 D_80062DBA = 0x0000;
s16 D_80062DBC = 0x0000;
s16 D_80062DBE = 0x0000;
s32 D_80062DC0 = 0x00000000;
s32 D_80062DC4 = 0x00000000;
s32 D_80062DC8 = 0x00000000;
s32 D_80062DCC = 0x00000000;
s32 D_80062DD0 = 0x00000000;
s32 D_80062DD4 = 0x00000000;
s16 D_80062DD8 = 0x0000;
s8 D_80062DDA = 0x00;
u8 D_80062DDB = 0x00;
u8 D_80062DDC = 0x02;
static s8 _D_80062DDD = 0x00;
static s8 _D_80062DDE = 0x00;
static s8 _D_80062DDF = 0x00;
s32 D_80062DE0 = 0x00000000;
u8 D_80062DE4 = 0x00;
u8 D_80062DE5 = 0x00;
s16 D_80062DE6 = 0x00B4;
s16 D_80062DE8 = 0x0068;
s16 D_80062DEA = 0x0000;
s32 D_80062DEC = 0x801D0000;
s32 D_80062DF0 = 0x00000084;
s32 D_80062DF4 = 0xFFFFFFFF;
s32 D_80062DF8 = 0x00000001;
s8 D_80062DFC = 0x40;
s8 _D_80062DFD = 0x00;
static s8 _D_80062DFE = 0x00;
static s8 _D_80062DFF = 0x00;
s32 D_80062E00 = 0x00000000;
s32 D_80062E04 = 0x00000000;
s16 D_80062E08 = 0x0000;
s16 D_80062E0A = 0x0000;
s32 D_80062E0C = 0;
s32 D_80062E10 = 0; // most likely a struct with D_80062E14 (see func_80014C44)
s32 D_80062E14 = 0;
s32 D_80062E18 = 0;
s32 D_80062E1C = 0;
s32 D_80062E20 = 0;
s32 D_80062E24 = 0x00000000;
s32 D_80062E28 = 0x00000000;
s32 D_80062E2C = 0x00000000;
s32 D_80062E30 = 0x00000000;
s32 D_80062E34 = 0x00000000;
s32 D_80062E38 = 0x00000000;
s32 D_80062E3C = 0x00000000;
s32 D_80062E40 = 0x00000000;
s32 D_80062E44 = 0x00000000;
s32 D_80062E48 = 0x00000000;
s32 D_80062E4C = 0x00000000;
s8 D_80062E50 = 0x00;
static s8 _D_80062E51 = 0x00;
static s8 _D_80062E52 = 0x00;
static s8 _D_80062E53 = 0x00;
s8 D_80062E54 = 0x00;
static s8 _D_80062E55 = 0x00;
static s8 _D_80062E56 = 0x00;
static s8 _D_80062E57 = 0x00;
s8 D_80062E58 = 0x00;
static s8 _D_80062E59 = 0x00;
static s8 _D_80062E5A = 0x00;
static s8 _D_80062E5B = 0x00;
s8 D_80062E5C = 0x00;
static s8 _D_80062E5D = 0x00;
static s8 _D_80062E5E = 0x00;
static s8 _D_80062E5F = 0x00;
s32 D_80062E60 = 0;
s8 D_80062E64 = 0x00;
static s8 _D_80062E65 = 0x00;
static s8 _D_80062E66 = 0x00;
static s8 _D_80062E67 = 0x00;
s8 D_80062E68 = 0x00;
static s8 _D_80062E69 = 0x00;
static s8 _D_80062E6A = 0x00;
static s8 _D_80062E6B = 0x00;
s32 D_80062E6C = 0x00000000;
s16 D_80062E70 = 0x0000;
s16 D_80062E72 = 0x0000;
s16 D_80062E74 = 0x0000;
s16 D_80062E76 = 0x0000;
s8 D_80062E78 = 0x00;
static s8 _D_80062E79 = 0x00;
static s8 _D_80062E7A = 0x00;
static s8 _D_80062E7B = 0x00;
s32 D_80062E7C = 0x00000000;
s32 D_80062E80 = 0x00000000;
s32 D_80062E84 = 0x00000000;
s16 D_80062E88 = 0x0000;
s16 D_80062E8A = 0x0000;
s8 D_80062E8C = 0x00;
static s8 _D_80062E8D = 0x00;
static s8 _D_80062E8E = 0x00;
static s8 _D_80062E8F = 0x00;
s8 D_80062E90 = 0x00;
static s8 _D_80062E91 = 0x00;
static s8 _D_80062E92 = 0x00;
static s8 _D_80062E93 = 0x00;
s32 D_80062E94 = 0x00000000;
s32 D_80062E98 = 0x00000000;
s32 D_80062E9C = 0x00000000;
s32 D_80062EA0 = 0x00000000;
s32 D_80062EA4 = 0x00000000;
s16 D_80062EA8 = 0x0000;
s16 D_80062EAA = 0x0000;
s16 D_80062EAC = 0x0000;
s16 D_80062EAE = 0x0000;
s16 D_80062EB0 = 0x0000;
s16 D_80062EB2 = 0x0000;
s8 D_80062EB4 = 0x00;
static s8 _D_80062EB5 = 0x00;
static s8 _D_80062EB6 = 0x00;
static s8 _D_80062EB7 = 0x00;
s32 D_80062EB8 = 0;

void func_8001155C(void);
void func_80014A00(s32* dst, s32* src, s32 len);
u16* func_80014D9C(s32, s32, s32);
s32 func_800150E4(u16*, u16*);
u16* func_800151F4(s32);
void func_80015CA0(GzHeader* src, s32* dst);
s32 func_8001AC9C(u8, s32);
void func_8001B834(s32);
void func_8001BD50(u8, u8, u8);
u8 func_8001F6B4();

void __main(void) {}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", __SN_ENTRY_POINT);

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/18B8", func_8001117C);
#else
/* PARKED at 14 rows / -2 instructions, and the whole residue is one address.
 * The target materialises `&D_8009A000` once into $s1 and stores through
 * `0($s1)` on both sides of the call -- a second callee-saved register and the
 * save/restore pair are exactly the two instructions we are short. This build
 * rematerialises `lui $at,%hi` / `sh %lo($at)` at each store, which is what
 * gcc 2.6.3 does when a symbol has only two references: CONST_COSTS makes a
 * two-instruction address cheaper than a register that has to be saved.
 *
 * Measured and rejected, all exactly 14 rows / -2:
 *   the plain `D_8009A000[0] = ...` at both stores
 *   `s16* cmd = D_8009A000;` (cse folds the local back to the symbol)
 *   `volatile s16* cmd = ...;`  and  `*(volatile s16*)D_8009A000 = ...`
 * The volatile route is the one CLAUDE.md gives for reaching the register form
 * off a single reference, and it is inert here because these are stores, not
 * loads. D_8009A004 and D_8009A008 sit at +4 and +8 but the target reaches
 * them through their own $at expansions, so the "spell the neighbour as an
 * offset" lever is not what the original did either. Something gives that
 * address a third reference; it is not visible in this function. */
void func_8001117C(u16 arg0) {
    D_8009A000[0] = 0xF1;
    SystemAkaoExecute();
    D_8009A000[0] = 0x20;
    D_8009A004[0] = 0x40;
    D_8009A008[0] = arg0;
    SystemAkaoExecute();
}
#endif

void func_800111E4(void) {
    D_8009A000[0] = 0xF4;
    SystemAkaoExecute();
    if (!(D_8009D5E9 & 0x30)) {
        func_8001117C(0x2B);
    }
    D_800707BC = D_8009ABF6;
    g_BattleMode = D_8009AC32;
    g_BattleMode = D_800716D0 | g_BattleMode;
    func_800146A4();
    D_800716D0 = 0;
}

void func_80011274(void) {
    SystemLoadFileBySector(D_80048D1C, D_80048D20, (u_long*)0x800E0000, NULL);

    while (1) {
        if (SystemCdromReadChain() == 0) {
            break;
        }
    }

    SystemLoadFileBySector(D_80048D14, D_80048D18, (u_long*)0x800A0000, NULL);

    while (1) {
        if (SystemCdromReadChain() == 0) {
            break;
        }
    }

    func_80029818((u32*)0x800A0000, (u32*)0x800E0000);
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800112E8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001146C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001155C);

void func_8001171C(void) {
    StopCallback();
    ResetCallback();
    ResetGraph(0);
    func_80036298();
    D_80095DD4 = 0;
    VSyncCallback(&func_8001155C);
    SetGraphDebug(0);
    SetDispMask(0);
    InitGeom();
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80011784);

void func_800A16CC();  // field loop
void func_800CF60C();  // field load
extern s32 D_80048D24; // field.X sector
extern u32 D_80048D28; // field.X size

void func_80011860(void) {
    if (D_800965EC != 5 && D_800965EC != 13) {
        if (D_800965EC != 2) {
            SystemLoadFileBySector(
                D_80048D24, D_80048D28, (u_long*)0x80180000, NULL);
            while (1) {
                if (SystemCdromReadChain() == 0) {
                    break;
                }
            }
            func_80015CA0((GzHeader*)0x80180000, (s32*)0x800A0000);
        } else {
            while (1) {
                if (SystemCdromReadChain() == 0) {
                    break;
                }
            }
            func_80015CA0((GzHeader*)0x801C0000, (s32*)0x800A0000);
        }
    }
    func_800CF60C();
    func_800A16CC();
}

void func_80011920(void) {
    g_isFieldLoading = 0;
    D_80071A5C = 0;
}

void func_80011938(void) {
    SystemLoadFileBySector(D_80048CFC, D_80048D00, (u_long*)0x800F0000, NULL);
    do {
    } while (SystemCdromReadChain());
    SystemLoadFileBySector(D_80048D04, D_80048D08, (u_long*)0x801B0000, NULL);
    do {
    } while (SystemCdromReadChain());
    SystemLoadFileBySector(D_80048D0C, D_80048D10, (u_long*)0x801BC800, NULL);
    do {
    } while (SystemCdromReadChain());
    func_8002988C(0x800F0000, 0x801BC800);
    func_80029998(0x801B0000);
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800119E4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80011AEC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80011BB4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", main);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80012840);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800128B8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800129D0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80012A8C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80012DB0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800131B8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800134F4);

void func_80013564(void) {
    D_8009AC2C[0] = 0;
    D_8009AC42 = D_8009AC42 + D_8009AC44;
    if ((s16)D_8009AC42 >= 0x100) {
        D_8009AC42 = 0xFF;
    }
}

#ifndef NON_MATCHINGS
s16 func_800135C0(s16 from, s16 to, s16 step, s16 steps);
MASPSX_OVERRIDE("asm/us/main/nonmatchings/18B8", func_800135C0);
#else
/* PARKED at 2 rows, length exact (25). The body is
 * instruction-for-instruction right and the only difference is which register
 * the division's quotient lands in: the target has `mflo v0` / `addu a0,a0,v0`
 * and this gets `mflo a3` / `addu a0,a0,a3`. Both `$v0` (which held the
 * product, dead after the `div`) and `$a3` (which held `steps`, likewise) are
 * free, so this is `block_alloc` handing the quantity the lowest-numbered
 * register it thinks is available -- a QTY_CMP_PRI tie, which CLAUDE.md
 * records as a park rather than a search.
 *
 * Measured, all exactly 2 rows: a named `s32 q` for the quotient, a named
 * `s32 t` for the product with `t / steps` at the return, one variable
 * carrying both (`t = ...; t = t / steps;`), `q /= steps` as a compound
 * assignment, `(s32)steps` at the division, `s16 d = to - from;` as its own
 * statement, and `(...) / steps + from` with the addition's operands swapped.
 * Worse: `s32 steps` (4 rows -- the target sign-extends it), `step * (s16)(to
 * - from)` with the multiply's operands swapped (9). */
s16 func_800135C0(s16 from, s16 to, s16 step, s16 steps) {
    return from + (s16)(to - from) * step / steps;
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80013624);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80013800);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800138EC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80013C9C);

void func_800140A4(void) {
    D_8019DAA0++;
    if (!(D_8019DAA0 & 1)) {
        DrawOTag(D_8019D5E8);
        func_80013C9C();
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800140F4);

// obtain file sector from a YamadaFile
s32 func_800144D8(s32 file_no) { return D_80048D84[file_no].loc; }

void func_800144F0(s32 file_no) { func_80033DAC(file_no, 0); }

void func_80014510(s32 file_no) { func_800144F0(D_80048D84[file_no].loc); }

// used to load WORLD/WORLD.BIN or FIELD/FIELD.BIN
void func_80014540(void) {
    SystemLoadFileBySector(D_80071744, D_80095DD8, D_800722C8, NULL);
}

void func_80014578(s32 file_no, void* dst, void (*cb)(void)) {
    SystemLoadFileBySector(
        D_80048D84[file_no].loc, D_80048D84[file_no].len, dst, cb);
}

void func_800145BC(void (*cb)(void)) {
    while (SystemCdromReadChain()) {
        if (cb) {
            cb();
        }
    }
}

void func_80014608(void) {}

// initialize LBA system
void func_80014610(void) {
    u8 buf[2048];
    SystemLoadFileBySector(LBA_INIT_YAMADA, sizeof(buf), (u_long*)&buf, NULL);
    func_800145BC(0);
    func_80014A00(
        (s32*)D_80048D84, (s32*)&buf, sizeof(Yamada) * YAMADA_FILE_NUM);
}

void func_80014658(s32 file_no, void (*cb)(void)) {
    func_80014578(file_no, (void*)0x801B0000, 0);
    func_800145BC(0);
    func_80015CA0((GzHeader*)0x801B0000, (s32*)0x800A0000);
    cb();
}

void func_800146A4(void) {
    s32 var_s0 = -1;
    while (var_s0) {
        /* The target loads this with `lhu` and sign-extends it by hand
         * (`sll 16` / `sra 16`), which is what a volatile 16-bit read gives --
         * the plain type folds the extension into an `lh`. See CLAUDE.md. */
        switch (*(volatile s16*)&D_8009C560) {
        case 4:
            func_800145BC(0);
            func_80014658(BATTLE_BROM, D_800A00CC);
            break;
        case 2:
            func_800140F4();
            func_80014658(BATTLE_BATTLE, D_800A1158);
            break;
        default:
            var_s0 = 0;
            break;
        }
    }
}

void func_80014750(void) {
    s32 temp_a0;
    s32 temp_s0;

    func_80015B44(0x801B0000);
    while (1) {
        temp_s0 = func_80015B50() & 0xFFFF;
        if (temp_s0 == 0xFFFF) {
            break;
        }
        temp_a0 = func_80015B88() & 0xFFFF;
        if (temp_s0 == 9) {
            func_80015BC0(func_80014C80(temp_a0));
        } else if (D_80048DD4[temp_s0]) {
            func_80015BC0(D_80048DD4[temp_s0]);
        }
    }
}

void func_80014804(void) {
    s32 temp_s0;

    func_80015B44(0x801B0000);
    while (1) {
        temp_s0 = func_80015B50() & 0xFFFF;
        if (temp_s0 == 0xFFFF) {
            break;
        }
        /* A switch, not an if/else-if chain: the target emits both compares
         * ahead of both arms, which is expand_end_case's shape. */
        switch (temp_s0) {
        case 0:
            func_80015BC0((u8*)0x801C0000);
            func_800149E0();
            func_80014980((u8*)0x801C0000);
            break;
        case 1:
            func_80015BC0(D_80063048);
            break;
        }
    }
}

void func_800148A0(void) {
    D_80062F88 = 0;
    g_BattleMode = 0;
}

s32 func_800148B4(void) {
    func_800148A0();
    g_CurrentAction = (Unk800A8D04*)0x1F800000;
    D_800707C0 = D_80063048;
    func_80014610();
    func_80014C70();
    func_80014578(1, (void*)0x801B0000, func_80014804);
    func_800145BC(0);
    func_80014578(2, (void*)0x801B0000, func_80014750);
    func_800145BC(0);
    return 1;
}

void func_80014934(void) {
    func_800148A0();
    func_80014578(INIT_KERNEL, (void*)0x801B0000, 0);
    func_800145BC(0);
    func_80015C3C(0x801B0000, D_8009C738, KERNEL_INIT);
}

/* A TIM: flags at +4, CLUT length at +8, then the CLUT's RECT at +0xC and its
 * data at +0x14, followed by the pixel block laid out the same way. */
void func_80014980(u8* tim) {
    if (*(s32*)(tim + 4) & 8) {
        LoadImage((RECT*)(tim + 0xC), (u_long*)(tim + 0x14));
        tim += (*(u32*)(tim + 8) >> 2) * 4;
    }
    LoadImage((RECT*)(tim + 0xC), (u_long*)(tim + 0x14));
}

void func_800149E0(void) { DrawSync(0); }

void func_80014A00(s32* dst, s32* src, s32 len) {
    int i;
    for (i = 0; i < len >> 2; i++) {
        *dst++ = *src++;
    }
}

s32 func_80014A38(u32 arg0) {
    s32 i;
    for (i = 0;; i++) {
        arg0 >>= 1;
        if (!arg0) {
            return i;
        }
    }
}

s32 func_80014A58(u32 arg0) {
    s32 i;
    i = 0;
    while (arg0) {
        if (arg0 & 1) {
            i++;
        }
        arg0 >>= 1;
    }
    return i;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80014A84);

/* Clamped add/subtract. The second `sltu $v0,$a2,$a0` is emitted twice --
 * once in the first branch's delay slot and once after the overflow clamp --
 * because `value` differs on the two paths, which is what says the two tests
 * are separate `if`s rather than one `||`. */
u32 func_80014B08(u32 value, u32 delta, u32 limit, s32 add) {
    u32 orig = value;

    if (add) {
        value += delta;
        if (value < orig) {
            value = limit;
        }
        if (limit < value) {
            value = limit;
        }
    } else {
        value -= delta;
        if (orig < value) {
            value = 0;
        }
    }
    return value;
}

void func_80014B54(void) { D_80062E18 = (D_80062E18 + 1) & 7; }

s32 func_80014B70(void) {
    u8* state;
    u8 index;

    state = (u8*)&D_80062E10 + D_80062E18;
    index = *state;
    *state = index + 1;
    return D_80083084[index];
}

s32 func_80014BA8(s32 arg0) {
    return (u8)(((func_80014B70() & 0xFF) * arg0) >> 8);
}

s32 func_80014BE4(void) {
    s32 lo;
    s32 hi;

    lo = func_80014B70();
    if ((D_80062D4C++ & 7) != 0) {
        func_80014B54();
    }
    /* `hi` has to be its own local: fold puts the more complex operand of the
     * `or` first, so with the shift written inline it becomes op0 and the two
     * registers come out the other way round. */
    hi = (func_80014B70() & 0xFF) << 8;
    return (lo & 0xFF) | hi;
}

/* The eight bytes at D_80062E10 are one object -- func_80014B70 walks them the
 * same way. Indexing through the cast keeps the `%hi`/`%lo` addressing the
 * target has, where a walked `u8*` local turns both bases into givs and costs
 * an instruction; see CLAUDE.md on -G8 and %gp_rel. */
void func_80014C44(s32 arg0) {
    s32 i;
    for (i = 0; i < 8; i++) {
        ((u8*)&D_80062E10)[i] = arg0;
        arg0 >>= 1;
    }
    D_80062E18 = 0;
}

void func_80014C70() {
    D_80062E1C = 0;
    D_80062E20 = 0;
}

u8* func_80014C80(s32 arg0) {
    s32 text_index;
    s32 text_offset;

    text_index = D_80062E1C++;
    text_offset = D_80062E20;
    D_80069490[text_index] = text_offset;
    D_80062E20 = text_offset + arg0;
    return D_80063690 + text_offset;
}

s32 func_80014CBC(s32 arg0, s32 arg1) {
    s32 var_a2;
    u8 var_v1;

    var_v1 = 0xFF;
    var_a2 = -1;
    switch (arg0) {
    case 0:
    case 1:
    case 2:
        var_v1 = D_800708D4[(D_80010100[arg0] + arg1) * 0x1C];
        break;
    case 4:
        if (arg1 < 0x80) {
            var_v1 = D_800722DC[arg1 * 0x1C];
        }
    }
    if (var_v1 != 0xFF) {
        var_a2 = var_v1;
    }
    return var_a2;
}

u8* func_80014D58(u8* arg0, u8* arg1, s32 arg2) {
    u8 var_a3 = *arg1;
    while (var_a3 != 0xFF) {
        *arg0 = var_a3;
        arg1++;
        arg2--;
        arg0++;
        if (arg2 == -1) {
            break;
        }
        var_a3 = *arg1;
    }
    return arg0;
}

u16* func_80014D9C(s32 arg0, s32 arg1, s32 arg2) {
    u8* temp_v1 = D_80063690 + D_80069490[arg0 + arg2];
    return (u16*)&temp_v1[*(u16*)&temp_v1[arg1 * 2]];
}

void func_80014DD0(s32 arg0, s32 arg1, u8* arg2) {
    func_80014D58(arg2, (u8*)func_80014D9C(arg0, arg1, 0), -1);
}

u8* func_80014E0C(s32 charId, u8* dst) {
    s32 i;

    /* One variable, not two: the target returns `move $v0,$a0` from a single
     * exit, with the call's result written back over `dst`. */
    for (i = 0; i < 9; i++) {
        if (D_8009C738[i].char_id == charId) {
            dst = func_80014D58(dst, D_8009C738[i].name, 0xC);
            break;
        }
    }
    return dst;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80014E74);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800150E4);

u16* func_800151F4(s32 arg0) { return func_80014D9C(0x10, arg0, 0); }

s32 func_8001521C(s32 arg0) {
    u16* temp_v0 = func_800151F4(arg0);
    return func_800150E4(temp_v0, temp_v0);
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80015248);

void func_800155A4(s32 arg0) { D_80062F14 = arg0; }

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800155B0);

void func_80015654(s32 arg0) {
    D_80062E24 = 0;
    D_80062E28 = 0;
    D_80062E2C = arg0;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80015668);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800159B0);

s32 func_80015AFC(u32 arg0, s32 arg1) {
    /* The frame is 0x30 with no saved register and nothing stored to it --
     * a local this function no longer uses. Its identity is not recoverable. */
    u8 unusedLocals[0x30];
    s32 ret;
    u8* row;

    ret = 0x7F;
    if (arg0 < 9) {
        row = &D_80082274[arg0 * 56];
        /* `arg1 + (s32)row`, not `row[arg1]`: fold canonicalises a pointer
         * PLUS so the pointer is op0, and the target adds the index first. */
        ret = *(u8*)(arg1 + (s32)row) - 0x80;
    }
    return ret;
}

void func_80015B44(s32 arg0) { D_80062E30 = arg0; }

s32 func_80015B50(void) {
    u8* entry;
    s32 entry_len;
    s32 entry_type;

    entry = (u8*)D_80062E30;
    entry_len = entry[0] | (entry[1] << 8);
    entry_type = 0xFFFF;
    if (entry_len != 0) {
        entry_type = entry[4] | (entry[5] << 8);
    }
    return entry_type;
}

s32 func_80015B88(void) {
    u8* entry;
    s32 entry_len;
    s32 entry_id;

    entry = (u8*)D_80062E30;
    entry_len = entry[0] | (entry[1] << 8);
    entry_id = 0;
    if (entry_len != 0) {
        entry_id = entry[2] | (entry[3] << 8);
    }
    return entry_id;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80015BC0);

// load kernel module by its ID
// https://wiki.ffrtt.ru/index.php/FF7/Kernel/Low_level_libraries#BIN-GZIP_Type_Archives
s32 func_80015C3C(u8* src, void* dst, s32 id) {
    /* `len` is u16, not s32: the target ORs into one register and copies the
     * result into `len`'s own (`or v1,v1,v0` / `move a3,v1`), which is an
     * HImode pseudo refusing to coalesce with the SImode OR. */
    u16 len;
    s32 cur_id;
    s32 ret;

    ret = -1;
    while (len = (src[0] | (src[1] << 8))) {
        cur_id = src[4] | (src[5] << 8);
        if (cur_id == id) {
            ret = func_80017108(src + 6, dst);
            break;
        }
        src += len + 6;
    }
    return ret;
}

void func_80015CA0(GzHeader* src, s32* dst) {
    s32 i;
    s32* var_s1;
    u32 len;
    s32 unk4;

    unk4 = src->unk4;
    len = src->len;
    func_80017108(src + 1, dst);
    var_s1 = &dst[len >> 2];
    unk4 = (u32)unk4 >> 2;
    for (i = 0; i < unk4; i++) {
        var_s1[i] = 0;
    }
}

/* A bump allocator over the 0x1000-word arena at D_80062E44; when the
 * request does not fit it resets the cursor and retries. The recursion has to
 * go through a local -- `return func_80015D14(size);` is a bare CALL_EXPR in
 * the return, which gcc 2.6.3 turns into a jump back to the top and costs the
 * whole frame. */
void* func_80015D14(u32 size) {
    u32 cur;
    u32 next;
    void* p;

    cur = D_80062D54;
    next = cur + ((size + 3) >> 2);
    if (next <= 0x1000) {
        D_80062D54 = next;
        p = (void*)(D_80062E44 + cur * 4);
    } else {
        D_80062D54 = 0;
        p = func_80015D14(size);
    }
    return p;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80015D64);

s32 func_80016320(s32* arg0) {
    while (arg0) {
        arg0 = (s32*)arg0[-1];
    }
    return 0;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80016340);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800166C0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80016808);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800169B8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80016F90);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001708C);

// gzip inflate
INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80017108);

void func_80017238(u32 arg0, u32* arg1, u8* arg2) {
    *arg2 = arg0;
    *arg1 = arg0 >> 8;
    func_8001AC9C(*arg2, *arg1);
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001726C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80017678);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001786C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80017E68);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80017F38);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018028);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018220);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800182FC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018390);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001840C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800184C0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800185A8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018630);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018834);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018934);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018A04);

#ifndef NON_MATCHINGS
void func_80018AB0(u8 arg0);
MASPSX_OVERRIDE("asm/us/main/nonmatchings/18B8", func_80018AB0);
#else
/* PARKED at 2 rows, length exact (25). The whole residue is the order of two
 * entry-block insns: the target is `move t0,zero` / `andi a0,a0,0xff` /
 * `li a3,0x1c0` and this is `move t0,zero` / `li a3,0x1c0` / `andi a0,a0,0xff`
 * -- i.e. `off`'s initialiser has to be emitted after the `u8` parameter's
 * promotion mask, and nothing tried moves it. Everything else, including the
 * `0x10f(a2)` displacement and the `addiu a3,a3,8` in the mid-loop branch's
 * delay slot, is instruction-for-instruction.
 *
 * Measured, all against a base of 8 rows unless noted:
 *   - the `p` pointer local (`p[0x10F]` rather than
 *     `*(u8*)(D_80062E60 + off + 0x10F)`) is what gets the displacement and
 *     the base register right: 25 rows without it, 8 with. Written as one
 *     expression gcc folds 0x1C0 + 0x10F and the `li a3,0x1c0` disappears.
 *   - `off += 8` moved out of the `for` increment into the loop body is the
 *     other half: 8 -> 2, because reorg can then steal it into the delay slot
 *     of the `lvl >= 6` branch. All three placements inside the body (after
 *     `p =`, after `cur =`, after `lvl =`) measure exactly 2.
 *   - inert at 2: declaring `off` before `i`, splitting the `for` init into
 *     two statements, `s32 arg0` with `arg0 & 0xFF` hoisted to a loop-top
 *     local.
 *   - worse: `i * 8 + 0x1C0` as a giv with no `off` at all (14), the same
 *     with the folded 0x2CF displacement (14), `off += 8` before `i++` in
 *     the `for` increment list (12), `off = 0x1C0` hoisted above the `for`
 *     with the increment in the body (9). */
void func_80018AB0(u8 arg0) {
    s32 i;
    s32 off;
    u8 cur;
    s32 lvl;
    u8* p;

    for (i = 0, off = 0x1C0; i < 0x10; i++) {
        p = (u8*)(D_80062E60 + off);
        off += 8;
        cur = p[0x10F];
        lvl = (cur >> 5) + arg0;
        if (lvl >= 6) {
            lvl = 5;
        }
        p[0x10F] = (cur & 0x1F) | ((lvl & 7) << 5);
    }
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018B14);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018BB8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018C94);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018D4C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018E18);

void func_80018E90(void) {
    s32 i;
    s32 off;

    for (i = 0, off = 0x1C0; i < 0x10; i++, off += 8) {
        *(u8*)(D_80062E60 + off + 0x10F) |= D_80062E78;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018ECC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80018FC0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019064);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800190E8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800191A0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019254);

void func_80019338(s32 delta) {
    s32 i;
    s32 v;

    for (i = 0; i < 0x38; i++) {
        D_80069556[i].count += 1;
        v = D_80069556[i].value;
        D_80069556[i].value = delta + v;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001937C);

void func_800193F4(s32 delta) {
    s32 i;
    s32 v;

    for (i = 0; i < 0x10; i++) {
        D_800694C4[i] += 1;
        /* Read into a local first: written `D_800694D4[i] += delta` or
         * `delta + D_800694D4[i]`, fold ranks the array reference as the more
         * complex operand and makes it op0 of the `addu`. */
        v = D_800694D4[i];
        D_800694D4[i] = delta + v;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019440);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800194BC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019544);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019608);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001964C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019690);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800197B8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019978);

void func_80019E84(u8, u8);
void func_80019D1C(u8 arg0, u8 arg1, s32 arg2) {
    u8 temp_s1;
    u8 temp_a0;

    temp_s1 = arg1;
    temp_a0 = func_8001AC9C(temp_s1, arg2);
    if (arg0 == 1) {
        func_80019E84(temp_a0, temp_s1);
    }
}

void func_80019D74(u8 arg0, u8 arg1) {
    if (arg1 == 0xB) {
        func_80019E4C(arg0);
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019DA0);

void func_80019E4C(void) {
    if (D_80063020 == 0) {
        *(u8*)(D_80062E60 + 0x23) |= 4;
    } else {
        D_80062FFC = 0x12;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019E84);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80019F90);

void func_8001A280(u8, s32);
void func_8001A1C8(u8, s32);
void func_8001A174(u8 arg0, s32 arg1, s32 arg2) {
    switch (arg0) {
    case 2:
        func_8001A280(arg1, arg2);
        break;
    case 3:
        func_8001A1C8(arg1, arg2);
        break;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A1C8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A280);

// file cut between func_8001A384 to func_8001C0EC

void func_8001A384(u8 arg0, s32 arg1) {
    func_8001AC9C(arg0, arg1);
    if (D_80063020) {
        D_80062FFC = 11;
    }
}

void func_8001A3B8(s32 arg0, s32 arg1, s32 arg2) {
    u8 param;
    s32 i;
    s32 enabled;
    s32 bits;

    if (D_80063020 == 0) {
        bits = arg2 & 0xFFFFFF;
        for (i = 0; i < 0x18; i++) {
            enabled = bits & 1;
            bits >>= 1;
            if (enabled) {
                param = i + 0x48;
                func_8001BD50(i, param, param);
            }
        }

        func_8001B834(13);
        return;
    }
    D_80062FFC = 8;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A440);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A4A8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A518);

void func_8001A780(u8, s32);
void func_8001A874(u8, s32);
void func_8001A5B4(u8 arg0, u8 arg1, s32 arg2) {
    u8 temp_s1;
    u8 temp_a0;

    temp_s1 = arg1;
    temp_a0 = func_8001AC9C(temp_s1, arg2);
    switch (arg0) {
    case 0:
        func_8001A684(temp_s1, arg2);
        break;
    case 2:
        func_8001A780(temp_s1, arg2);
        break;
    case 4:
        func_8001A874(temp_s1, arg2);
        break;
    case 3:
        func_80019D74(temp_a0, temp_s1);
        break;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A684);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A780);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A874);

void func_8001AB1C(u8, s32);
void func_8001A9CC(u8, s32);
void func_8001A980(u8 arg0, s32 arg1, s32 arg2) {
    switch (arg0) {
    case 4:
        func_8001AB1C(arg1, arg2);
        break;
    case 2:
        func_8001A9CC(arg1, arg2);
        break;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001A9CC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001AB1C);

#ifndef NON_MATCHINGS
s32 func_8001AC9C(u8, s32);
MASPSX_OVERRIDE("asm/us/main/nonmatchings/18B8", func_8001AC9C);
#else
/* PARKED at 35 rows / +10 instructions. Eleven of those rows are the same
 * %gp_rel blocker as func_8001A384 above -- four .bss symbols, one extra
 * `lui` each -- so the honest residue is smaller than the number and cannot be
 * measured until the .bss import lands. Do not spend a codegen budget here
 * before that; the note on func_8001A384 has the rejected spellings. */
s32 func_8001AC9C(u8 arg0, s32 arg1) {
    s32 i;
    s32 found;
    u16 temp_a2;
    Unk80062F7C* new_var;

    found = 1;
    for (i = 3; i >= 0; i--) {
        temp_a2 = D_800730CC[arg0].unk4[i];
        if (temp_a2 == 0xFFFF || arg1 < temp_a2 * 100) {
            continue;
        }
        found = i + 2;
        break;
    }
    D_80062FBC = 1;
    for (i = 0; i < 4; i++) {
        temp_a2 = D_800730CC[arg0].unk4[i];
        if (temp_a2 != 0xFFFF) {
            D_80062FBC++;
        }
    }
    if (D_80063020) {
        temp_a2 = D_800730CC[arg0].unk4[found - 1];
        if (temp_a2 == 0xFFFF || found == D_80062FBC) {
            D_80062F10 = 0;
        } else {
            D_80062F10 = temp_a2 * 100 - arg1;
        }
        new_var = D_80062F7C;
        new_var->unk0 = found;
        new_var->unk1 = *(u8*)&D_80062FBC;
        new_var->unk4 = D_80062F10;
    }
    return found;
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001AE08);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001AEE4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B4A0);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B570);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B5E4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B704);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B834);

// ------------------------ ??

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B8A8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001B944);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001BA54);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001BB30);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001BC18);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001BCE8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001BD50);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001BDB0);

// file cut between func_8001A384 to func_8001C0EC
const u8 D_8001029C[] = {
    0x00, 0x01, 0x02, 0x03, 0x00, 0x02, 0x01, 0x03, 0x02, 0x00, 0x01, 0x03,
    0x01, 0x00, 0x02, 0x03, 0x01, 0x02, 0x00, 0x03, 0x02, 0x01, 0x00, 0x03};
INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C0EC);

void func_8001C3C4(void) {}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C3CC);

void SetupGamepad(void) {
    if (g_bPadsInitialized == 0) {
        g_bPadsInitialized = 1;
        StartPAD();
        InitPAD(&D_800696AC.padABuffer, 4, &D_800696AC.padBBuffer, 4);
    }
    D_80062FA0 = 0;
}

void func_8001C484(s32 arg0) {
    D_80062E9C = arg0;
    D_80062E94 = 0x14;
}

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/18B8", func_8001C498);
#else
/* PARKED at 7 rows / +1 instruction. Reads controller port A's 32-byte buffer:
 * byte 0 is the status (0xFF = nothing plugged in), byte 1 the pad type (0x41
 * = digital), bytes 2-3 the button bits, active-low, hence the `nor`.
 *
 * The residue is two coupled placements and nothing else. The target keeps the
 * result in $v0 with **two** separate `move v0,zero` sites -- one stolen into
 * the first `beq`'s delay slot, one in its own block at the second failure --
 * and issues `nor v0,v1,v0` in the delay slot of the `j` to a shared epilogue
 * whose own delay slot holds the `andi v0,v0,0xffff`. Every body written here
 * either collapses the two zero sites into one (and then loses 2 instructions)
 * or puts the `andi` in the `j`'s delay slot and leaves the `nor` standing on
 * its own (+1 instruction).
 *
 * Measured, all on the same load order (unk2 then unk3[0], which every
 * spelling produces because fold puts the shift first):
 *   three `return`s, `~((unk2 << 8) | unk3[0])`              7 rows, +1
 *   three `return`s, `~(unk3[0] | (unk2 << 8))`              7 rows, +1
 *   s32 return with an explicit (u16) cast on the value      7 rows, +1
 *   u16 `buttons` local, nested ifs, single return           7 rows, -2
 *   `hi` named local so the nor keeps source operand order   7 rows, +1
 *   ... same, with nested ifs                                7 rows, -2
 *   u16 `buttons`, two `goto out` with buttons = 0          10 rows
 *   s32 `buttons`, two `goto out` with buttons = 0           9 rows
 *   u16 `buttons`, `goto out`, named `hi`                   10 rows
 * The named-local lever for the `nor`'s operand order is exactly inert, which
 * says fold is not what decides it. The two shapes sit at +1 and -2, so this
 * is the paired-lever shape: whatever produces the target's second zero site
 * has to be crossed with whatever moves the `andi` to the epilogue, and
 * neither alone reaches it. perm_ins_block on the second failure arm is the
 * next thing to try. */
u16 func_8001C498(void) {
    if (D_800696AC.padABuffer == 0xFF) {
        return 0;
    }
    if (D_800696AC.unk1 != 0x41) {
        return 0;
    }
    return ~((D_800696AC.unk2 << 8) | D_800696AC.unk3[0]);
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C4E8);

void func_8001C58C(void) {
    if (!func_8001F6B4()) {
        D_80062D71 = 0;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C5BC);

u16 func_8001C788(void) {
    u16 result;

    result = 0;
    if (D_80062E94 != 0) {
        D_80062E94--;
    } else if (func_80023050() == 0 || func_80023050() == 1) {
        if (SystemCdromReadChain() == 0) {
            result = func_8001C5BC();
        }
    }
    return result;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C808);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C8D4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001C980);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001CB48);

void func_8001CDA4(void) {
    SetPolyFT4(D_80062F24.ft4);
    SetShadeTex(D_80062F24.ft4, 1);
    D_80062F24.ft4->x0 = 0;
    D_80062F24.ft4->y0 = 5;
    D_80062F24.ft4->x1 = 0x40;
    D_80062F24.ft4->y1 = 5;
    D_80062F24.ft4->x2 = 0;
    D_80062F24.ft4->y2 = 0x45;
    D_80062F24.ft4->x3 = 0x40;
    D_80062F24.ft4->y3 = 0x45;
    D_80062F24.ft4->u0 = 0;
    D_80062F24.ft4->v0 = 0;
    D_80062F24.ft4->u1 = 128;
    D_80062F24.ft4->v1 = 0;
    D_80062F24.ft4->u2 = 0;
    D_80062F24.ft4->v2 = 128;
    D_80062F24.ft4->u3 = 128;
    D_80062F24.ft4->v3 = 128;
    D_80062F24.ft4->clut = GetClut(0, 0x1FE);
    D_80062F24.ft4->tpage = GetTPage(1, 0, 0x340, 0);
    AddPrim(D_80062FC4, D_80062F24.ft4);
    D_80062F24.ft4++;
}

void func_8001CF3C(s16 x, s16 y, s16 w, s16 h, u16 tx, u16 ty, u16 tw, u16 th,
                   s16 clut, s32 tex) {
    SetPolyFT4(D_80062F24.ft4);
    SetShadeTex(D_80062F24.ft4, 1);
    if (tex << 0x10) {
        SetSemiTrans(D_80062F24.ft4, 1);
    }
    D_80062F24.ft4->x0 = x;
    D_80062F24.ft4->y0 = y;
    D_80062F24.ft4->x1 = x + w;
    D_80062F24.ft4->y1 = y;
    D_80062F24.ft4->x2 = x;
    D_80062F24.ft4->y2 = y + h;
    D_80062F24.ft4->x3 = x + w;
    D_80062F24.ft4->y3 = y + h;
    D_80062F24.ft4->u0 = tx;
    D_80062F24.ft4->v0 = ty;
    D_80062F24.ft4->u1 = tx + tw;
    D_80062F24.ft4->v1 = ty;
    D_80062F24.ft4->u2 = tx;
    D_80062F24.ft4->v2 = ty + th;
    D_80062F24.ft4->u3 = tx + tw;
    D_80062F24.ft4->v3 = ty + th;
    D_80062F24.ft4->clut = GetClut(0x100, (s16)clut + 0x1E0);
    D_80062F24.ft4->tpage = GetTPage(1, 0, 0x3C0, 0x100);
    AddPrim(D_80062FC4, D_80062F24.ft4);
    D_80062F24.ft4++;
}

void func_8001D180(s16 x, s16 y, s16 w, s16 h, u16 tx, u16 ty, u16 tw, u16 th,
                   s16 clut, s32 tex) {
    SetPolyFT4(D_80062F24.ft4);
    SetShadeTex(D_80062F24.ft4, 1);
    if (tex << 0x10) {
        SetSemiTrans(D_80062F24.ft4, 1);
    }
    D_80062F24.ft4->x0 = x;
    D_80062F24.ft4->y0 = y;
    D_80062F24.ft4->x1 = x + w;
    D_80062F24.ft4->y1 = y;
    D_80062F24.ft4->x2 = x;
    D_80062F24.ft4->y2 = y + h;
    D_80062F24.ft4->x3 = x + w;
    D_80062F24.ft4->y3 = y + h;
    D_80062F24.ft4->u0 = tx;
    D_80062F24.ft4->v0 = ty;
    D_80062F24.ft4->u1 = tx + tw;
    D_80062F24.ft4->v1 = ty;
    D_80062F24.ft4->u2 = tx;
    D_80062F24.ft4->v2 = ty + th;
    D_80062F24.ft4->u3 = tx + tw;
    D_80062F24.ft4->v3 = ty + th;
    D_80062F24.ft4->clut = GetClut(0x180, (s16)clut);
    D_80062F24.ft4->tpage = GetTPage(1, 0, 0x340, 0x100);
    AddPrim(D_80062FC4, D_80062F24.ft4);
    D_80062F24.ft4++;
}

void func_8001D3C0(s16 x, s16 y) {
    SetTile1(D_80062F24.tile1);
    D_80062F24.tile1->x0 = x;
    D_80062F24.tile1->y0 = y;
    D_80062F24.tile1->r0 = 0xFF;
    D_80062F24.tile1->g0 = 0xFF;
    D_80062F24.tile1->b0 = 0;
    AddPrim(D_80062FC4, D_80062F24.tile1);
    D_80062F24.tile1++;
}

void func_8001D47C(s16 x0, s16 x1, s16 y, s32 color) {
    SetLineF2(D_80062F24.linef2);
    D_80062F24.linef2->r0 = color >> 16;
    D_80062F24.linef2->g0 = color >> 8;
    D_80062F24.linef2->b0 = color;
    D_80062F24.linef2->x0 = x0;
    D_80062F24.linef2->y0 = y;
    D_80062F24.linef2->x1 = x1;
    D_80062F24.linef2->y1 = y;
    AddPrim(D_80062FC4, D_80062F24.linef2);
    D_80062F24.linef2++;
}

void func_8001D56C(s16 x0, s16 y0, s16 x1, s16 y1, s16 is_yellow) {
    if (is_yellow) {
        SetLineF2(D_80062F24.linef2);
        D_80062F24.linef2->r0 = 0xFF;
        D_80062F24.linef2->g0 = 0xFF;
        D_80062F24.linef2->b0 = 0;
    } else {
        SetLineF2(D_80062F24.linef2);
        D_80062F24.linef2->r0 = 0x80;
        D_80062F24.linef2->g0 = 0x80;
        D_80062F24.linef2->b0 = 0x80;
    }
    D_80062F24.linef2->x0 = x0;
    D_80062F24.linef2->y0 = y0;
    D_80062F24.linef2->x1 = x1;
    D_80062F24.linef2->y1 = y1;
    AddPrim(D_80062FC4, D_80062F24.linef2);
    D_80062F24.linef2++;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001D6A8);

void func_8001DE0C(Unk8001DE0C* arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {
    arg0->unk0 = arg1;
    arg0->unk2 = arg2;
    arg0->unk4 = arg3;
    arg0->unk6 = arg4;
}

// translate window dialog
void func_8001DE24(Unk8001DE0C* arg0, s32 arg1, s32 arg2) {
    arg0->unk0 = arg0->unk0 + arg1;
    arg0->unk2 = arg0->unk2 + arg2;
}

// set window dialog rect
void func_8001DE40(Unk8001DE0C* arg0, Unk8001DE0C* arg1) {
    arg0->unk0 = arg1->unk0;
    arg0->unk2 = arg1->unk2;
    arg0->unk4 = arg1->unk4;
    arg0->unk6 = arg1->unk6;
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001DE70);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001DEB0);

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/18B8", func_8001DEF0);
#else
/* PARKED at 3 rows / -2 instructions, and no C reaches it: both sides use the
 * assembler's $at macro for the indexed store, they just expand it differently.
 * maspsx turns on `nop_at_expansion` and `addiu_at` for aspsx < 2.30, which is
 * exactly the target's extra `nop` after the `lbu` and its
 * `addiu at,at,%lo(...)` before the `addu` -- the two instructions we are
 * short. This unit is built at 2.34 (//! G=8 -> the default). Switching it is
 * not the wrong answer -- 77 of the 151 remaining functions here carry the
 * < 2.30 expansion and none of the 72 already-matching ones do, and the object
 * is byte-identical either way -- but `//! G=8 ASPSX=2.21` currently turns the
 * build red in the menu overlays through a splat/sym_export feedback loop that
 * has nothing to do with this unit. See CLAUDE.md; unpark this the day that is
 * fixed. The body below is correct. */
// sets the menu color with a quadruplet of RGB values
void func_8001DEF0(u8* menu_colors) {
    s32 i;
    for (i = 0; i < 12; i++) {
        g_FieldWindowColors[i] = *menu_colors++;
    }
}
#endif

void func_8001DF24(RECT* rect, u8 arg1, u8 arg2, u8 arg3) {
    setTile(D_80062F24.tile);
    SetShadeTex(D_80062F24.tile, 1);
    D_80062F24.tile->x0 = rect->x;
    D_80062F24.tile->y0 = rect->y;
    D_80062F24.tile->w = rect->w;
    D_80062F24.tile->h = rect->h;
    D_80062F24.tile->r0 = arg1;
    D_80062F24.tile->g0 = arg2;
    D_80062F24.tile->b0 = arg3;
    AddPrim(D_80062FC4, D_80062F24.tile++);
}

// prints menu window
INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001E040);

// print menu cursor
void func_8001EB2C(s16 x, s16 y) {
    RECT rect;

    setSprt(D_80062F24.sprt);
    SetSemiTrans(D_80062F24.sprt, 1);
    SetShadeTex(D_80062F24.sprt, 1);
    D_80062F24.sprt->x0 = x;
    D_80062F24.sprt->y0 = y;
    D_80062F24.sprt->u0 = 224;
    D_80062F24.sprt->v0 = 8;
    D_80062F24.sprt->w = 24;
    D_80062F24.sprt->h = 16;
    D_80062F24.sprt->clut = GetClut(0x100, 0x1E1);
    AddPrim(D_80062FC4, D_80062F24.sprt++);
    rect.x = 0;
    rect.y = 0;
    rect.w = 0xFF;
    rect.h = 0xFF;
    func_80026A34(0, 1, (u16)GetTPage(0, 2, 0x3C0, 0x100), &rect);
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001EC70);

void func_8001EF84(s32 x, s32 y, s32 n, s32 len) {
    RECT rect;
    s32 i;
    s32 uv;

    for (i = 0; i < 8; i++) {
        uv = n / D_80049224[i];
        setSprt(D_80062F24.sprt);
        SetShadeTex(D_80062F24.sprt, 1);
        D_80062F24.sprt->x0 = x;
        D_80062F24.sprt->y0 = y;
        D_80062F24.sprt->u0 = (uv % 5) * 16 - 80;
        D_80062F24.sprt->v0 = uv >= 5 ? 104 : 80;
        D_80062F24.sprt->w = 16;
        D_80062F24.sprt->h = 21;
        D_80062F24.sprt->clut = GetClut(0x100, 0x1EC);
        if (len >= 8 - i) {
            x += 16;
            AddPrim(D_80062FC4, D_80062F24.sprt++);
        }
        n %= D_80049224[i];
    }
    rect.x = 0;
    rect.y = 0;
    rect.w = 255;
    rect.h = 255;
    func_80026A34(0, 1, (u16)GetTPage(0, 1, 0x3C0, 0x100), &rect);
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", SystemMenuDrawDialog);

void func_8001F6AC(void) {}

u8 func_8001F6B4(void) { return D_80062DDB; }

void func_8001F6C0(s32 arg0, s8 arg1) {
    D_80062DDB = 1;
    D_80062DDC = arg1;
    D_80062DE0 = 0x28;
    D_80062EB8 = arg0;
    D_80062DE5 = 1;
}

void func_8001F6E4(s16 arg0, s16 arg1, s16 arg2) {
    D_80062DE4 = arg0;
    if (arg0) {
        D_80062DE6 = arg1;
        D_80062DE8 = arg2;
    } else {
        D_80062DDB = 0;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001F710);

void func_8001FA28(u16 arg0) {
    D_8009A000[0] = 0x30;
    D_8009A004[0] = arg0;
    D_8009A008[0] = arg0;
    SystemAkaoExecute();
}

void func_8001FA68(u16 arg0) {
    D_8009A000[0] = 0x28;
    D_8009A004[0] = 0x40;
    D_8009A008[0] = arg0;
    SystemAkaoExecute();
}

void func_8001FAAC(u16 arg0) {
    D_8009A000[0] = 0x29;
    D_8009A004[0] = 0x40;
    D_8009A008[0] = arg0;
    SystemAkaoExecute();
}

void func_8001FAF0(void) {}

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FAF8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FBAC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FCDC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FE6C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FF50);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FF8C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8001FFD4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_8002001C);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80020058);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800206E4);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80020B68);

void func_80021044(DRAWENV* draw_env, DISPENV* disp_env) {
    VSync(0);
    SetDefDrawEnv(draw_env, 0, 0, 0x180, 0x1D8);
    draw_env[0].dfe = 1;
    draw_env[0].isbg = 1;
    PutDrawEnv(draw_env);
    VSync(0);
    SetDefDrawEnv(draw_env, 0, 8, 0x180, 0xE0);
    SetDefDrawEnv(&draw_env[1], 0, 0xF0, 0x180, 0xE0);
    SetDefDispEnv(&disp_env[0], 0, 0xE8, 0x16C, 0xF0);
    SetDefDispEnv(&disp_env[1], 0, 0, 0x16C, 0xF0);
    draw_env[1].isbg = 1;
    draw_env[0].isbg = 1;
    draw_env[1].dfe = 1;
    draw_env[0].dfe = 1;
    draw_env[1].dtd = 1;
    draw_env[0].dtd = 1;
    draw_env[0].r0 = 0;
    draw_env[0].g0 = 0;
    draw_env[0].b0 = 0;
    draw_env[1].r0 = 0;
    draw_env[1].g0 = 0;
    draw_env[1].b0 = 0;
    draw_env[0].tpage = draw_env[1].tpage =
        GetGraphType() != 1 && GetGraphType() != 2 ? 0x3F : 0xAF;
    VSync(0);
    PutDispEnv(disp_env);
    PutDrawEnv(draw_env);
    SetDispMask(1);
}

void func_800211B8(s32 arg0) { D_80062DEC = arg0; }

void func_800211C4(s32 id) {
    SystemLoadFileBySector(
        D_80048F60[id * 2], D_80048F60[id * 2 + 1], (u_long*)D_80062DEC, NULL);
    SystemCdromReadChain();
}

void func_8002120C(s32 arg0) {
    s32 prev;

    prev = D_80062DD4;
    D_80062DD4 = arg0;
    D_80062DD0 = prev;
    if (arg0 != 0 && (prev < 3 || prev > 4 || arg0 < 3 || arg0 > 4)) {
        func_800211C4(arg0);
    }
}

void func_80021258(s32 arg0) { func_80015248(13, arg0, 8); }

void func_80021280(s32 arg0) { func_80015248(4, arg0, 8); }

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_800212A8);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80021BAC);

INCLUDE_ASM("asm/us/main/nonmatchings/18B8", func_80021C4C);
