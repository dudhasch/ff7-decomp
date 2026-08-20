//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 1 of 5, split out of field.c. .rodata 0x800A0000-0x800A0054, base 0
 * mod 8. Holds jtbl_800A0008, whose `.align 3` in FieldMain.s is a no-op at
 * offset 0x8. */

const u32 D_800A0000[] = {0, 0x01D801E0};

/////////////////////////////////////////////////
// Begin of field_main.c
/////////////////////////////////////////////////

typedef struct {
    u32 datSector; // +0x00
    u32 datSize;   // +0x04
    u32 mimSector; // +0x08
    u32 mimSize;   // +0x0C
    u32 bsxSector; // +0x10
    u32 bsxSize;   // +0x14
} FieldFileInfo;

extern FieldFileInfo g_FieldFileInfo[];
extern void SystemLzsDecompress(void* dst, void* src);
extern s32* g_FieldModelsP;
extern s32 g_FieldTriggers;
extern s32 g_FieldEncounters;
extern s32 D_8007E770;
extern s16 g_CurrentFieldIndex;
extern s32* g_FieldTriggersP;
extern s32* g_FieldEncountersP;
extern u32 g_FieldLzsInfo[];

void FieldLoadMimDatFiles(void) {
    s32 temp;

    if (g_isFieldLoading == 0) {
        DS_read(g_FieldLzsInfo[g_CurrentFieldIndex * 6],
                g_FieldLzsInfo[g_CurrentFieldIndex * 6 + 1], (u32*)0x80128000,
                NULL);
        while (SystemCdromReadChain() != 0) {
        }
    } else {
        while (SystemCdromReadChain() != 0) {
        }
        SystemLzsDecompress((void*)0x801B0000, (void*)0x80128000);
    }
    DS_read(((u32*)g_FieldFileInfo)[g_CurrentFieldIndex * 6],
            ((u32*)g_FieldFileInfo)[g_CurrentFieldIndex * 6 + 1],
            (u32*)0x80114FE4, NULL);
    while (SystemCdromReadChain() != 0) {
    }
    g_FieldTriggers = *g_FieldTriggersP;
    g_FieldEncounters = *g_FieldEncountersP;
    temp = *g_FieldModelsP;
    D_8007E770 = temp;
    g_FieldModelLoaderData = temp + 4;
}

void StopFieldMapPreload(void) {
    if (g_isFieldLoading == 1) {
        SystemCdromAbortLoading();
    }
    D_80071A5C = 0; // needs to be called g_preloadedFieldMapId;
    g_isFieldLoading = 0;
}

/* 0x800DA5C4 == &g_FieldFileInfo[0].mimSize: the preload pulls the MIM, so
 * the size is at [id * 6] and its sector one word before it. */
extern u32 g_FieldFileTable[];
extern volatile u16 g_FieldMoviePlayed;
extern u16 g_FieldPreloadMapId;
extern s32 g_WmPreSector;
extern u32 g_WmPreSize;

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", PreloadNextFieldMap);
#else

/* Pick the nearest gateway on this map and start streaming its MIM off the CD,
 * so a map transition does not have to wait for the read. The gateway table is
 * twelve 0x18-byte records; destFieldId == 0x7FFF marks an unused slot. The
 * distance is squared, in map units, from the player position stashed in the
 * scratchpad at 0x1F800000.
 *
 * 13 rows, down from 28 on one line: which of the two walking pointers is
 * incremented first. Written `i++, ptrPos += 12, gateway += 12` the parameter's
 * own register a1 stays with the *advanced* pointer and ptrPos takes a3, which
 * is what the target does; written the other way round the two swap and every
 * use of either follows -- 15 rows of pure renaming. The rule that fits every
 * spelling measured (both in the `for`, either one moved to the end of the
 * body, both moved there) is that the pointer incremented *last* keeps the
 * incoming argument register, while the emitted order of the two `addiu`s
 * follows the source. The target wants gateway to keep a1 *and* to be bumped
 * first, which no arrangement of two increments reaches; four of the thirteen
 * rows are that. Making ptrPos a giv (`ptrPos[i * 12]`, with gateway walking)
 * puts the increments in the right order but costs 26/5, and the mirror image
 * 31/3.
 *
 * The other nine rows are outside the loop. The target computes the file-table
 * address twice, in opposite operand orders (`addu v1,s0,v0` for the sector at
 * -4 and `addu v0,v0,s0` for the size at 0) where cse merges ours into one; and
 * it materialises 0x801B0000 once, in the delay slot of the branch that picks
 * the arm, where ours emits it in both arms.
 *
 * Measured and rejected: making the second parameter `u16*` and advancing it in
 * place rather than deriving a third pointer (no change); swapping the two
 * pointer declarations (no change); assigning ptrPos inside the guard rather
 * than at the top (17/3); a `do/while` loop with the increments written out at
 * the bottom instead of a `for` with a comma increment (no change,
 * byte-identical output); indexing every access off one base with no second
 * pointer at all (36/1), which collapses the two givs the target keeps apart.
 *
 * Four things that WERE worth rows, all of them type errors rather than
 * codegen, and all of them now folded in (53 rows -> 28):
 *   - the m2c seed did its pointer arithmetic in FieldGateway units, so
 *     `gateway + 0x12` compiled to `+0x1b0`. The walk needs BYTE offsets: one
 *     s16* at the record base for pos.x1/pos.y1 and one u16* at +0x12 for
 *     destFieldId, with pos.y1 reached as `((s16*)gateway)[-8]` off the
 * *second* base -- which is what the target's `lh v0,-0x10(a1)` says gcc did.
 *   - g_FieldFileTable (0x800DA5C4) is not the head of the table: it is
 *     &g_FieldFileInfo[0].mimSize. The preload pulls the MIM, so the size is
 *     `table[id * 6]` and its sector is the word before it, `table[id * 6 -
 * 1]`. Typed as FieldFileInfo* and indexed `.datSize`, every offset is 4 out.
 *   - g_FieldMoviePlayed has to be `volatile u16`. Non-volatile, combine folds
 *     the (s16) conversion into the load and emits `lh`; the target loads `lhu`
 *     and sign-extends in two separate insns, which only survives if the MEM is
 *     volatile. The other users of the symbol only ever store constants to it,
 *     so the qualifier costs nothing elsewhere.
 *   - the (s16) cast on g_FieldPreloadMapId has to appear on the table index as
 *     well as on the comparison, or gcc reloads the global unsigned for the
 *     index and the two uses stop sharing a register. */
// External Declarations
extern u8 D_8009ABF5;
extern u8 g_FieldAnimLock;
extern s16 D_80071A5C;

// D_8009ABF5 = g_FieldState -> command

void PreloadNextFieldMap(FieldEntity* Player, u16* gateway) {
    s16* ptrPos;
    s32* scratchpad;
    s32 minDist;
    s32 i;
    s32 diffX, diffY, dist;
    u32* table;
    s32 sector;
    u32 size;

    ptrPos = (s16*)gateway;
    minDist = 0x7FFFFFFF;

    scratchpad = (s32*)0x1F800000;
    scratchpad[0] = Player->PosX >> 12;
    scratchpad[1] = Player->PosY >> 12;
    scratchpad[2] = Player->PosZ >> 12;

    if (g_FieldAnimLock == 0) {
        gateway += 9;

        for (i = 0; i < 12; i++, ptrPos += 12, gateway += 12) {
            if (gateway[0] != 0x7FFF) {
                diffX = ptrPos[0] - scratchpad[0];
                diffY = ((s16*)gateway)[-8] - scratchpad[1];
                dist = diffX * diffX + diffY * diffY;
                if (dist < minDist) {
                    minDist = dist;
                    g_FieldPreloadMapId = gateway[0];
                }
            }
        }
    }

    if (D_8009ABF5 == 3 || (s16)g_FieldMoviePlayed == 1 || D_8009ABF5 == 2) {
        StopFieldMapPreload();
        return;
    }

    if (D_80071A5C == (s16)g_FieldPreloadMapId) {
        return;
    }

    table = g_FieldFileTable;
    if (0x4DFFF < table[(s16)g_FieldPreloadMapId * 6]) {
        return;
    }

    StopFieldMapPreload();
    D_80071A5C = g_FieldPreloadMapId;

    if (D_80071A5C >= 0x41) {
        sector = table[D_80071A5C * 6 - 1];
        size = table[D_80071A5C * 6];
    } else {
        sector = g_WmPreSector;
        size = g_WmPreSize;
    }

    SystemLoadFileBySector(sector, size, 0x801B0000, NULL);
    g_isFieldLoading = 1;
}

#endif

extern DISPENV g_FieldDispEnv[2];
extern DRAWENV g_FieldDrawEnv[2];
extern DRAWENV g_FieldDrawEnvBg[2];
extern DRAWENV D_80113FE4[2];
extern DRAWENV D_8011409C[2];
extern DRAWENV D_80114154[2];
extern DRAWENV D_8011420C[2];
extern s8 D_800716D0;
extern s16 D_8007173C;
extern u8 D_80071A58;
extern s16 D_8007E768;
extern s32* D_8007EB64;
extern u8 D_8007EBC8;
extern u8 D_8009C6D8;
extern s8 D_8009A057;
extern s16 D_8009A100;
extern volatile u16 D_8009AC18;
extern s16 D_8009AC1A[1];
extern volatile s16 D_8009AC1E;
extern u16 D_8009AC40[1];
extern u32 D_8009AC3C[1];
extern void func_800128B8(void);
extern void func_800129D0(void);
extern void DebugRunEveryLoop(void);
extern void FieldEventInit(FieldState* state, FieldEntity* entities, s32 arg2);
extern void FieldEntityBgTriggerInit(void* triggers);
extern void FieldEnablePartyModels(void);
extern void FieldEntityLineClear(FieldLine* line);
extern void FieldArrowsInit(SPRT_16* sprites, DR_MODE* dm);
extern void FieldLoadMimToVram(s32 arg0, u8* mim);
/* The walk mesh: a u16 triangle count, then that many 24-byte triangles. */
typedef struct {
    /* 0x00 */ u16 triCount;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ s16 tris[1];
} FieldWalkmesh;

extern s32 FieldMainLoop(void);

/* The field's trigger block opens with a header; only the camera height
 * bias at +0xA is read here. Spelling it as a struct member -- rather than
 * *(u16*)(g_FieldTriggers + 0xA) -- is what lets gcc hoist the load above
 * the scalar stores in front of it: a struct load does not alias a store
 * to a plain extern. */
typedef struct {
    /* 0x00 */ u8 unk00[0xA];
    /* 0x0A */ u16 camHeightBias;
} FieldTriggerHeader;

extern FieldLine D_8007E7AC;
extern volatile s16 g_FieldNextModule;
extern FieldWalkmesh** D_8009A044;
extern FieldBgData** D_8009D848;
extern s16 D_80071E38;
extern s16 D_80071E3C;

/* Parked at 77 changed / 6 inserted of 793 instructions (78 further rows are
 * symbol aliases -- the C reaches g_FieldRenderData's members where the .s
 * names D_800E8F7C and friends). Everything down to func_800128B8 is
 * byte-identical, and so are the jump table, all seven of its arms and the
 * whole tail dispatch. What was learned, and what is left:
 *
 *   - the local `RECT clip` aggregate initialiser is what puts the 8-byte
 *     blob at .rodata+0 and the jump table at +8, which is the layout the
 *     overlay needs. Writing the four fields instead moves the table to 0.
 *     (While the function is parked the .s carries only the table, so the
 *     blob is the file-scope D_800A0000 at the top of this unit.)
 *   - eventCmd is read through a `volatile u8*`. Volatile is not decoration:
 *     it is what makes the target re-load the byte at every test and emit a
 *     separate `andi 0xff` for the zero-extension. Non-volatile, cse folds
 *     all six tests into one lbu and the function is ten instructions short.
 *   - that same pointer is the base register the whole tail addresses off
 *     (0(s1) eventCmd, 1(s1) eventCmdParam, 0x4b fadeType, 0x4d fadeAdjust,
 *     0x63 prevFieldId, 0xf1 the switch selector -- every FieldState offset
 *     minus one). Spelling those as D_8009ABF4.<member> instead costs a
 *     second anchor register (gcc picks &prevFieldId) and 37 rows.
 *   - D_8009AC1E and D_8009AC18 must be volatile: the target loads both with
 *     lhu, and a plain u16 with an (s16) cast folds to lh, a plain u16 stored
 *     into a u8 field narrows to lbu.
 *   - g_FieldBGCameraHeightBias is read as a struct member, not as
 *     *(u16*)(g_FieldTriggers + 0xA): only the struct form lets gcc hoist the
 *     load above the three scalar stores in front of it (7 rows).
 *   - the switch selector goes into a u8 local, not a u32 one -- the u32 form
 *     loses the andi.
 *
 * The residue is five clusters, all scheduling or address-form:
 *   1. D_8009AC40[0] = 0, D_8009AC1A[0] = 2 and the D_8009AC3C read: the
 *      target materialises the whole address into a general register
 *      (lui/addiu/op 0(reg)); every spelling tried here gives the two-
 *      instruction $at macro. Measured and rejected: scalar extern, one-
 *      element array, volatile on each of the three, and a volatile store
 *      through a cast. The one place the register form does appear --
 *      D_8009A000 -- has its address used twice, so cse hoists it; these
 *      three are used once. 6 rows.
 *   2. the fade block stores fadeType before fadeSpeed in the target and
 *      after it here, with the rest of the block identical. 4 rows.
 *   3. `li a0,-1` is hoisted above the rain if/else in the target and lands
 *      after the loop's two inits here. A named local for the fill value was
 *      measured and changes nothing. 3 rows.
 *   4. the tail reads D_80071A5C early and eventCmdParam late; here it is the
 *      other way round. Reading g_CurrentFieldIndex into a local first, and
 *      s32 vs s16 for the D_80071A5C temporary, were both measured and are
 *      worse. 6 rows.
 *   5. one delay slot at the cross-jump into the shared fade-reset tail.
 * The remaining rows are branch-target addresses, which follow from the
 * length difference and cost nothing once the clusters above are closed. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldMain);
#else

void FieldMain(void) {
    RECT clip = {0, 0, 480, 472};
    volatile u8* ev;
    s8* fill;
    s32 i;
    u16 fieldId;
    s16 preloadId;
    u8 exitKind;

    ClearOTagR(&g_FieldRenderData[0].OtFadeDrenv, 1);
    ClearOTagR(&g_FieldRenderData[1].OtFadeDrenv, 1);
    SetDrawEnv(&g_FieldRenderData[0].FadeDrenv, &g_FieldDrawEnv[0]);
    SetDrawEnv(&g_FieldRenderData[1].FadeDrenv, &g_FieldDrawEnv[1]);
    addPrim(&g_FieldRenderData[0].OtFadeDrenv, &g_FieldRenderData[0].FadeDrenv);
    addPrim(&g_FieldRenderData[1].OtFadeDrenv, &g_FieldRenderData[1].FadeDrenv);
    SetDefDrawEnv(&g_FieldDrawEnvBg[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&g_FieldDrawEnvBg[1], 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(&D_80114154[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_80114154[1], 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(&D_8011420C[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_8011420C[1], 0, 0xF0, 0x140, 0xE0);
    g_FieldDrawEnvBg[0].dtd = 1;
    g_FieldDrawEnvBg[1].dtd = 1;
    D_80114154[0].dtd = 1;
    D_80114154[1].dtd = 1;
    D_8011420C[0].dtd = 1;
    D_8011420C[1].dtd = 1;
    g_FieldDrawEnvBg[0].isbg = 0;
    g_FieldDrawEnvBg[1].isbg = 0;
    D_80114154[0].isbg = 0;
    D_80114154[1].isbg = 0;
    D_8011420C[0].isbg = 0;
    D_8011420C[1].isbg = 0;
    ClearOTagR(&g_FieldRenderData[0].OtSceneDrenv, 1);
    ClearOTagR(&g_FieldRenderData[1].OtSceneDrenv, 1);
    SetDrawEnv(&g_FieldRenderData[0].SceneDrenv, &g_FieldDrawEnvBg[0]);
    SetDrawEnv(&g_FieldRenderData[1].SceneDrenv, &g_FieldDrawEnvBg[1]);
    addPrim(
        &g_FieldRenderData[0].OtSceneDrenv, &g_FieldRenderData[0].SceneDrenv);
    addPrim(
        &g_FieldRenderData[1].OtSceneDrenv, &g_FieldRenderData[1].SceneDrenv);
    SetDefDrawEnv(&D_80113FE4[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_80113FE4[1], 0, 0xF0, 0x140, 0xE0);
    D_80113FE4[0].isbg = 0;
    D_80113FE4[1].isbg = 0;
    D_80113FE4[0].dtd = 1;
    D_80113FE4[1].dtd = 1;
    SetDefDrawEnv(&D_8011409C[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_8011409C[1], 0, 0xF0, 0x140, 0xE0);
    D_8011409C[0].isbg = 0;
    D_8011409C[1].isbg = 0;
    D_8011409C[0].dtd = 1;
    D_8011409C[1].dtd = 1;
    func_800128B8();
    ev = &D_8009ABF4.eventCmd;
    D_8009AC40[0] = 0;
    if (D_800965EC != 1 && D_800965EC != 2 && D_800965EC != 3 &&
        D_800965EC != 5 && D_800965EC != 0xD) {
        ClearImage(&clip, 0, 0, 0);
    }

    for (;;) {
        DebugRunEveryLoop();
        D_80071A5C = 0;
        g_FieldPreloadMapId = 0;
        if ((D_800965EC == 1 || D_800965EC == 3) && D_8009ABF4.fadeType == 0) {
            func_800129D0();
            D_8009ABF4.fadeType = 3;
            D_80071A58 = 3;
            D_8009ABF4.fadeAdjust = 0;
            D_8007E768 = 0;
            D_80095DD4 = 1;
        }
        if (D_800965EC != 5 && D_800965EC != 0xD) {
            D_8007EB64 = (s32*)0x80114FE4;
            D_8009A044 = (FieldWalkmesh**)0x80114FE8;
            D_8009D848 = (FieldBgData**)0x80114FEC;
            D_80083578 = (MATRIX**)0x80114FF0;
            g_FieldTriggersP = (s32*)0x80114FF4;
            g_FieldEncountersP = (s32*)0x80114FF8;
            g_FieldModelsP = (s32*)0x80114FFC;
            FieldLoadMimDatFiles();
        }
        if (D_800965EC == 2) {
            D_8007EBE0 = 1;
            if (D_8007EBC8 == 1) {
                D_8007EBC8 = 0;
                D_8009C6D8 = 0;
                D_8007173C = 0;
                *ev = 0;
            }
        }
        while (D_80095DD4 != 0) {
        }
        while (DrawSync(1) != 0) {
        }
        if (D_800965EC != 0xD) {
            D_8009ABF4.fadeType = 1;
            D_8009ABF4.fadeSpeed = 0x10;
            D_8009ABF4.fadeAdjust = 0x100;
            D_8009ABF4.fadeRed = 0;
            D_8009ABF4.fadeGreen = 0;
            D_8009ABF4.fadeBlue = 0;
        }
        if (D_800965EC == 0 || D_800965EC == 1 || D_800965EC == 3 ||
            D_800965EC == 6 || D_800965EC == 8 || D_800965EC == 7 ||
            D_800965EC == 9 || D_800965EC == 0xB || D_800965EC == 0xA) {
            D_8009ABF4.layer2_bgScrollXSpeed = 0;
            D_8009ABF4.layer2_bgScrollYSpeed = 0;
            D_8009ABF4.layer3_bgScrollXSpeed = 0;
            D_8009ABF4.layer3_bgScrollYSpeed = 0;
            D_8009ABF4.layer3_depth = 1;
            D_8009ABF4.layer2_depth = 0xFFF;
            D_8009A100 = 0;
            D_80071E38 = 0;
            D_80071E3C = 0;
            g_FieldBGCameraHeightBias =
                ((FieldTriggerHeader*)g_FieldTriggers)->camHeightBias;
            FieldEventInit(&D_8009ABF4, g_FieldEntity, *D_8007EB64);
            g_FieldEntity[D_8009AC1E].Dir = D_8009AC18;
            if ((g_RainControl & 0x80) == 0) {
                g_RainForce = 0;
            } else {
                g_RainForce = 0xFF;
            }
            i = 0xF;
            fill = &D_8009A057;
            do {
                *fill-- = -1;
            } while (--i >= 0);
            FieldEntityBgTriggerInit((void*)(g_FieldTriggers + 0x158));
        } else {
            D_8009AC1A[0] = 2;
        }
        FieldEnablePartyModels();
        FieldEntityLineClear(&D_8007E7AC);
        D_800716D0 = 0;
        FieldArrowsInit(
            g_FieldRenderData[0].Arrows, &g_FieldRenderData[0].ArrowsDm);
        FieldArrowsInit(
            g_FieldRenderData[1].Arrows, &g_FieldRenderData[1].ArrowsDm);
        if (D_800965EC != 5 && D_800965EC != 0xD) {
            FieldLoadMimToVram(0, (u8*)0x80128000);
        }
        if (D_800965EC == 2) {
            D_8009A000[0] = 0xF5;
            SystemAkaoExecute();
            D_8009A000[0] = 0x18;
            D_8009A008[0] = 4;
            D_8009A004[0] = D_8009AC3C[0];
            SystemAkaoExecute();
        }
        FieldMainLoop();
        while (DrawSync(1) != 0) {
        }
        VSync(1);
        g_FieldDispEnv[0].isrgb24 = 0;
        g_FieldDispEnv[1].isrgb24 = 0;
        PutDispEnv(&g_FieldDispEnv[(s16)D_80075DEC]);
        PutDrawEnv(&g_FieldDrawEnv[(s16)D_80075DEC]);
        D_800965EC = 1;
        if (*ev == 0xA || *ev == 0x1A || *ev == 5) {
            break;
        }
        if (*ev == 1) {
            preloadId = D_80071A5C;
            *(u16*)(ev + 0x63) = (u16)g_CurrentFieldIndex;
            fieldId = *(u16*)(ev + 1);
            g_CurrentFieldIndex = fieldId;
            if ((s16)fieldId != preloadId) {
                StopFieldMapPreload();
            }
            if ((u32)((u16)g_CurrentFieldIndex - 1) < 0x40) {
                g_FieldNextModule = 3;
                func_800129D0();
                *(u16*)(ev + 0x4B) = 3;
                D_80071A58 = 3;
                *(u16*)(ev + 0x4D) = 0;
                D_8007E768 = 0;
                D_80095DD4 = 1;
                break;
            }
        }
        if (*ev == 0xC) {
            *(u16*)(ev + 0x63) = (u16)g_CurrentFieldIndex;
            fieldId = *(u16*)(ev + 1);
            exitKind = ev[0xF1];
            g_CurrentFieldIndex = fieldId;
            switch (exitKind) {
            case 0:
                g_FieldNextModule = 6;
                break;
            case 1:
                g_FieldNextModule = 7;
                break;
            case 2:
                g_FieldNextModule = 8;
                break;
            case 3:
                g_FieldNextModule = 9;
                break;
            case 4:
                g_FieldNextModule = 0xA;
                break;
            case 5:
                g_FieldNextModule = 0xB;
                break;
            case 6:
                g_FieldNextModule = 0xE;
                break;
            }
            break;
        }
        if (*ev == 2 || *ev == 0xD) {
            break;
        }
        if (g_FieldNextModule == 5) {
            func_800129D0();
            *(u16*)(ev + 0x4B) = 0xD;
            D_80071A58 = 0xD;
            *(u16*)(ev + 0x4D) = 0;
            D_8007E768 = 0;
            D_80095DD4 = 1;
            break;
        }
        if (g_FieldNextModule == 0xD) {
            break;
        }
        if (g_FieldNextModule == 0x10) {
            break;
        }
    }
    VSync(0);
}

#endif

/* The field module's per-frame loop: flip the double buffer, clear both OTs,
 * run the event script and the entity/background updates, then hand the frame
 * to the GPU. Returns only when the event script asks for a different module.
 *
 * This function was one instruction out for a long time, and the instruction
 * was not codegen -- maspsx dropped the load-delay nop the original assembler
 * put at the loop-top join label:
 *
 *     lhu  v0,%lo(D_80075DEC)(v0)    <- dead re-read, from `D_80075DEC++`
 *   .Ljoin:
 *     nop
 *     lui  v0,%hi(D_80075DEC)        <- the next `lhu v0,D_80075DEC` macro
 *
 * A load with a symbolic operand and no base register is expanded by the
 * assembler through its own destination register, not $at, so its `lui` lands
 * in the delay slot of a preceding load into that same register. maspsx only
 * asked whether the next instruction *reads* the loaded register, and nothing
 * in the text of `lhu $2,D_80075DEC` does. Fixed in tools/maspsx
 * (_next_load_clobbers_reg, with tests in tests/test_symbol_load_clobber.py);
 * that change adds exactly one instruction to the whole build, this one.
 *
 * Everything else in here was derived and is worth keeping:
 *   - `s32` return, not `void`. v0 is then live at the epilogue, so gcc's
 *     delay-slot pass cannot steal the following `li v0,0xc` into the delay
 *     slot of the `eventCmd == 1` branch (the only branch here that jumps
 *     straight to the epilogue). Declared void, that slot gets filled and the
 *     function is one instruction short.
 *   - `D_80075DEC++`, not `D_80075DEC = D_80075DEC + 1`. The variable is
 *     volatile, and only the increment form re-reads it afterwards - three
 *     instructions the plain assignment does not emit.
 *   - the six RECTs are locals with aggregate initialisers, which is what
 *     emits their blobs into .rodata at 0x800A0024..0x800A0054 and copies them
 *     in with lwl/lwr. Do not also declare them at file scope: while this
 *     function was parked the .s referenced six `const u32 D_800A0024[]`
 *     definitions, and leaving those in beside the compiled C emits every
 *     blob twice and grows the overlay by 48 bytes.
 *   - D_8009A060 must NOT be volatile: volatile pins its load ahead of the
 *     `li v0,1`, and the target has the constant first (in the branch delay
 *     slot of the movie-stream test).
 *   - the C uses D_8009ABF4.eventCmd throughout, never the D_8009ABF5 alias;
 *     the alias costs a %hi/%lo pair per use where gcc wants 1(s2), and it is
 *     what lets s4 become the `addiu s4,s2,1` base the target uses for
 *     pcPosX/pcPosY/pcWalkMeshId.
 *   - `/ 4096`, not `>> 12`: the target has the bgez/addiu 0xfff rounding. */

extern FieldWalkmesh** D_8009A044;
extern s16* D_800E4274;
extern s16* D_80114458;
extern s32 D_8009A060;
extern volatile s32 D_800965E4;
extern u8 D_80071C0C;
extern s16 D_80071E38;
extern s16 D_80071E3C;
extern MATRIX* D_80071E40;
extern u8 D_8009AC2C;
extern u32 D_8007E7A0[2];
extern FieldLine D_8007E7AC;
extern s32 D_80114478;
extern s32 D_8011447C;
extern volatile s16 g_FieldNextModule;
extern s32 g_FieldScreenCenterX;
extern s32 g_FieldScreenCenterY;
extern DISPENV g_FieldDispEnv[2];
extern DRAWENV g_FieldDrawEnv[2];
extern DRAWENV g_FieldDrawEnvBg[2];
extern DISPENV* g_FieldCurDispEnv;
extern DRAWENV* g_FieldCurDrawEnv;

s32 FieldMainLoop(void) {
    RECT clip24Top = {0, 0, 480, 8};
    RECT clip24Mid = {0, 232, 480, 8};
    RECT clip24Bot = {0, 464, 480, 8};
    RECT clip16Top = {0, 0, 320, 8};
    RECT clip16Mid = {0, 232, 320, 8};
    RECT clip16Bot = {0, 464, 320, 8};
    struct FieldRenderData* buf;
    s16* tris;
    s16 first;

    g_FieldScreenCenterX = 160;
    g_FieldScreenCenterY = 120;
    if (D_800965EC != 5 && D_800965EC != 0xD) {
        FieldModelLoadAndInit();
    }
    tris = (*D_8009A044)->tris;
    D_800E4274 = tris;
    D_80114458 = (s16*)((*D_8009A044)->triCount * 24 + (s32)tris);
    if (D_800965EC != 5 && D_800965EC != 2 && D_800965EC != 0xD) {
        FieldEntityInitPos();
    }
    FieldBackgroundInitPackets(
        g_FieldRenderData[0].Bg1, g_FieldRenderData[0].Bg2,
        (u8*)g_FieldRenderData[0].BgAnim, g_FieldRenderData[0].BgDm);
    first = 1;
    FieldBackgroundInitPackets(
        g_FieldRenderData[1].Bg1, g_FieldRenderData[1].Bg2,
        (u8*)g_FieldRenderData[1].BgAnim, g_FieldRenderData[1].BgDm);
    FieldRainInit(&g_FieldRenderData[0]);
    FieldRainInit(&g_FieldRenderData[1]);
    g_FieldMovieStreamActive = 0;
    g_FieldMovieStreamDone = 0;
    g_FieldMoviePlayed = 0;
    D_80071C0C = 0;
    g_isFieldLoading = 0;

    for (;;) {
        if (first == 0) {
            D_80075DEC++;
        }
        D_80075DEC = D_80075DEC & 1;
        D_8009ABF4.renderBuffer = D_80075DEC;
        buf = &g_FieldRenderData[(s16)D_80075DEC];
        ClearOTagR(buf->ot, 0x1000);
        ClearOTagR(&buf->OtUi, 1);
        FieldCameraAssign();
        g_FieldPadRaw = FieldButtonsUpdate(&D_80071E38, &D_80071E3C);
        D_8009ABF4.currentMovieFrame = D_80075D00->unk8;
        FieldEventUpdate((s32)&buf->OtUi);
        g_PlayerModelId = D_8009ABF4.pcModelId;
        FieldBGScrollInit();
        FieldBGScrollUpdate();
        FieldBGShakeUpdate(&D_8009ABF4.shakeX);
        FieldBGShakeUpdate(&D_8009ABF4.shakeY);
        FieldBGUpdateDrawenv(buf);
        PreloadNextFieldMap(&g_FieldEntity[g_PlayerModelId],
                            (FieldLine*)(g_FieldTriggers + 0x38));
        if ((D_8009ABF4.activeKeys & 0x90F) == 0x90F) {
            D_8009ABF4.eventCmd = 0xA;
            func_80035658();
            StopFieldMapPreload();
            return;
        }
        if (D_8009ABF4.eventCmd == 1) {
            return;
        }
        if (D_8009ABF4.eventCmd == 0xC) {
            StopFieldMapPreload();
            return;
        }
        if (D_8009ABF4.eventCmd == 0xD) {
            StopFieldMapPreload();
            g_FieldNextModule = 0xC;
            return;
        }
        if (D_8009ABF4.eventCmd == 0x19) {
            g_FieldNextModule = 0x10;
            StopFieldMapPreload();
            return;
        }
        if (D_8009ABF4.eventCmd == 0xF || D_8009ABF4.eventCmd == 0x10 ||
            D_8009ABF4.eventCmd == 0x11 || D_8009ABF4.eventCmd == 0x15 ||
            D_8009ABF4.eventCmd == 0x16 || D_8009ABF4.eventCmd == 0x17 ||
            D_8009ABF4.eventCmd == 0x18) {
            g_FieldNextModule = 0xD;
            StopFieldMapPreload();
            return;
        }
        if (D_8009ABF4.eventCmd == 6 || D_8009ABF4.eventCmd == 7 ||
            D_8009ABF4.eventCmd == 9 || D_8009ABF4.eventCmd == 0xE ||
            D_8009ABF4.eventCmd == 8 || D_8009ABF4.eventCmd == 0x12 ||
            D_8009ABF4.eventCmd == 0x13) {
            g_FieldNextModule = 5;
            StopFieldMapPreload();
            return;
        }
        if ((g_FieldPadRaw & 0x10) && D_8009ABF4.menuDisabled == 0 &&
            g_FieldMoviePlayed == 0 && g_FieldMovieStreamActive == 0) {
            g_FieldNextModule = 5;
            D_8009ABF4.eventCmd = 9;
            D_8009ABF4.eventCmdParam = 0;
            StopFieldMapPreload();
            return;
        }
        if (D_8009ABF4.eventCmd == 5 || D_8009ABF4.eventCmd == 0x1A) {
            StopFieldMapPreload();
            return;
        }
        if (D_8009ABF4.eventCmd == 2) {
            D_8009ABF4.pcPosX = g_FieldEntity[g_PlayerModelId].PosX / 4096;
            D_8009ABF4.pcPosY = g_FieldEntity[g_PlayerModelId].PosY / 4096;
            g_FieldNextModule = 2;
            D_8009ABF4.pcWalkMeshId = g_FieldEntity[g_PlayerModelId].PosI;
            StopFieldMapPreload();
            return;
        }
        FieldEntityMovementUpdate(g_FieldPadRaw);
        FieldEntityLineInteract(&g_FieldEntity[g_PlayerModelId], &D_8007E7AC);
        FieldEntityCheckTalk();
        if (g_FieldMovieStreamActive == 0 || D_8009A060 == 1) {
            AddBackgroundToRender(buf);
        }
        HandleKawaiDataInModel(buf);
        FieldRainUpdate();
        FieldRainAddToRender(buf->ot, buf->Rain, D_80071E40, &buf->RainDm);
        FieldArrowsAddToRender(buf, D_80071E40, g_FieldTriggers + 0x38);
        func_800138EC();
        D_80114478 = VSync(1);
        while (DrawSync(1) != 0) {
        }
        D_8011447C = VSync(1);
        if (g_FieldMovieStreamActive != 0 && D_800965E4 != 1) {
            VSync(3);
        } else {
            VSync(2);
        }
        if (first != 0) {
            first--;
            if (first == 0) {
                SetDispMask(1);
            }
        }
        ResetGraph(1);
        if (g_FieldMovieStreamActive == 0) {
            if (g_FieldMovieStreamDone == 0) {
                g_FieldDispEnv[(s16)D_80075DEC].isrgb24 = 0;
            } else {
                g_FieldMovieStreamDone = 0;
            }
        }
        PutDispEnv(&g_FieldDispEnv[(s16)D_80075DEC]);
        PutDrawEnv(&g_FieldDrawEnv[(s16)D_80075DEC]);
        if (g_FieldMovieStreamActive == 0) {
            ClearImage(&g_FieldDrawEnv[(s16)D_80075DEC].clip, 0, 0, 0);
        } else if (g_FieldDispEnv[(s16)D_80075DEC].isrgb24 == 0) {
            ClearImage(&clip16Top, 0, 0, 0);
            ClearImage(&clip16Mid, 0, 0, 0);
            ClearImage(&clip16Bot, 0, 0, 0);
        } else {
            ClearImage(&clip24Top, 0, 0, 0);
            ClearImage(&clip24Mid, 0, 0, 0);
            ClearImage(&clip24Bot, 0, 0, 0);
        }
        g_FieldCurDispEnv = &g_FieldDispEnv[(s16)D_80075DEC];
        g_FieldCurDrawEnv = &g_FieldDrawEnvBg[(s16)D_80075DEC];
        FieldUpdateMovieStream();
        if (D_8009AC2C == 0) {
            DrawOTag(&buf->OtSceneDrenv);
            DrawOTag(&buf->ot[0xFFF]);
            DrawOTag(&buf->OtFadeDrenv);
            if (D_8009AC40[0] != 0) {
                DrawOTag(&D_8007E7A0[(s16)D_80075DEC]);
            }
        }
        DrawOTag(&buf->OtUi);
    }
}

/* Parse a MIM (field background map image) header and upload its palette and
 * two texture pages to VRAM. `mim` points at the loaded file; three
 * variable-length records follow one another, each opening with a 32-bit byte
 * length, and each seeds a slice of the state block at D_800E4D90. The palette
 * goes up with LoadImage, the two pages with LoadTPage, with a DrawSync
 * between every step.
 *
 * The body was rewritten from the target; the previous one did not compile
 * (LoadTPage takes seven arguments, not five) and so had never been measured.
 * Five things it gets right:
 *
 *   - `mim` itself advances. The target reaches the second record's fields as
 *     0(a1) and 2(a1) with `addiu a1,a1,4` between the pairs, which is a
 *     source-level walk; constant displacements off one base would come out as
 *     a single `addiu a1,a1,0xC`.
 *   - the second LoadTPage is guarded by `if (*(u32*)&D_800E4DD8[0] != 0)` --
 *     the `lw v1,D_800E4DD8` / `beqz v1` right after the first LoadTPage.
 *   - every value read back for the LoadTPage argument lists is spelled as an
 *     offset from D_800E4D90 rather than through its own symbol. Naming the
 *     symbol twice -- once for the store, once for the read -- makes gcc
 *     materialise its %hi/%lo into a register, and with nine such symbols the
 *     function grew nine callee-saved registers and a 0x50 frame. Reached as
 *     `(u8*)D_800E4D90 + 0x1C` the address is named once and the assembler
 *     rebuilds it at the use, which is what the target does. Worth 38 rows,
 *     and the same mechanism as the byte-offset idiom in CLAUDE.md.
 *   - `unusedLocals` reserves the 0x28 of stack the original allocates after
 *     `rect` and never touches; see FieldDebugInitBuffers for the same thing.
 *   - each block's length is stored to the state word first and then read back
 *     out of it for the `(len >> 2) * 4 - 0xC` skip. Held in a local instead,
 *     the pseudo is still live at the shift, so the chain needs two more
 *     registers and sched2 cannot spread it into the load-delay slots of the
 *     three `sh`s in front of it -- 15 rows of nops, and no declaration order,
 *     variable count or spelling of the arithmetic moves them. Read back
 *     through the global, cse hands the just-stored register straight back,
 *     the shift is done in place, and the slots fill. Re-reading the *source*
 *     word instead of the destination is not the same thing and costs 18.
 *   - the LoadImage source is `(u8*)D_800E4D94 - 4`, not `D_800E4D90` by name,
 *     even though the two are the same address and the store just above uses
 *     the name. Naming it twice gives cse a second reference, it promotes the
 *     address to a callee-saved register and the frame grows -- 7 rows. The
 *     `.s` names D_800E4D90 there, so the relocation reads `D_800E4D94-0x4`
 *     against the target's `D_800E4D90`; the linked bytes are identical and
 *     `tools/checkfn.py` resolves the negative addend. */
void FieldLoadMimToVram(s32 arg0, u8* mim) {
    RECT rect;
    u8 unusedLocals[0x28];
    u32 next;
    u16 unk0A;

    *(u32*)&D_800E4D94[0] = *(u32*)mim;
    next = (*(u32*)&D_800E4D94[0] >> 2) * 4 - 0xC;
    *(u16*)&D_800E4D98[0] = *(u16*)(mim + 4);
    *(u16*)&D_800E4D9A[0] = *(u16*)(mim + 6);
    *(u16*)&D_800E4D9C[0] = *(u16*)(mim + 8);
    unk0A = *(u16*)(mim + 0xA);
    mim += 0xC;
    *(u8**)&D_800E4D90[0] = mim;
    *(u16*)&D_800E4D9E[0] = unk0A;
    mim += next;

    /* First texture page block. */
    *(u32*)&D_800E4DA8[0] = *(u32*)mim;
    next = (*(u32*)&D_800E4DA8[0] >> 2) * 4 - 0xC;
    mim += 4;
    *(u16*)&D_800E4DAC[0] = *(u16*)mim;
    *(u16*)&D_800E4DAE[0] = *(u16*)(mim + 2);
    mim += 4;
    *(u16*)&D_800E4DB0[0] = *(u16*)mim * 2;
    *(u16*)((u8*)D_800E4DB0 + 2) = *(u16*)(mim + 2);
    mim += 4;
    *(u8**)&D_800E4DA4[0] = mim;
    mim += next;

    /* Second texture page block. */
    *(u32*)&D_800E4DD8[0] = *(u32*)mim;
    mim += 4;
    *(u16*)&D_800E4DDC[0] = *(u16*)mim;
    *(u16*)&D_800E4DDE[0] = *(u16*)(mim + 2);
    mim += 4;
    *(u16*)&D_800E4DE0[0] = *(u16*)mim * 2;
    *(u16*)((u8*)D_800E4DE0 + 2) = *(u16*)(mim + 2);
    mim += 4;
    *(u8**)&D_800E4DD4[0] = mim;

    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = 0x100;
    rect.h = 0x10;
    DrawSync(0);
    LoadImage(&rect, *(u_long**)((u8*)D_800E4D94 - 4));
    DrawSync(0);
    *(u16*)&D_800E4DB4[0] = LoadTPage(
        *(u_long**)((u8*)D_800E4D90 + 0x14), 1, 0,
        *(s16*)((u8*)D_800E4D90 + 0x1C), *(s16*)((u8*)D_800E4D90 + 0x1E),
        *(u16*)((u8*)D_800E4D90 + 0x20), *(u16*)((u8*)D_800E4D90 + 0x22));
    if (*(u32*)((u8*)D_800E4D90 + 0x48) != 0) {
        DrawSync(0);
        *(u16*)&D_800E4DE4[0] = LoadTPage(
            *(u_long**)((u8*)D_800E4D90 + 0x44), 1, 0,
            *(s16*)((u8*)D_800E4D90 + 0x4C), *(s16*)((u8*)D_800E4D90 + 0x4E),
            *(u16*)((u8*)D_800E4D90 + 0x50), *(u16*)((u8*)D_800E4D90 + 0x52));
    }
    DrawSync(0);
}

/* Latch both pads: keep the raw state, the previous state, and the edges
 * (newly pressed / newly released) derived from the two, and hand the caller
 * back pad 2's raw state.
 *
 * The return value is not decoration -- it is what makes the function match.
 * Declared void, gcc puts `~pad2` in v0 (the dying source of the `nor`) and
 * the last two instructions come out as `nor v0,zero,v0` / `and v1,v1,v0`.
 * The target writes `nor a0,zero,v0`, reusing a0 freed by the
 * g_FieldPad2Pressed store one instruction earlier, which it can only do
 * because v0 must still hold pad2 at the epilogue. The identical pad-1 block,
 * whose pad1 really is dead, does use v0 -- so the two halves being allocated
 * differently is the tell. FieldMainLoop's call site agrees: it stores the
 * result into g_FieldPadRaw. */
u32 FieldButtonsUpdate(void) {
    u32 pad1;
    u32 pad2;
    u32 old1;
    u32 old2;

    pad1 = func_8001C808();
    old1 = g_FieldPad1State;
    g_FieldPadRaw = pad1;
    g_FieldPad1State = pad1;
    g_FieldPad1PrevState = old1;
    g_FieldPad1Pressed = (pad1 ^ old1) & pad1;
    g_FieldPad1Released = (pad1 ^ old1) & ~pad1;

    pad2 = func_8001C8D4();
    old2 = g_FieldPad2State;
    g_FieldPadRaw = pad2;
    g_FieldPad2State = pad2;
    g_FieldPad2PrevState = old2;
    g_FieldPad2Pressed = (pad2 ^ old2) & pad2;
    g_FieldPad2Released = (pad2 ^ old2) & ~pad2;
    return pad2;
}

extern FieldBgData** D_8009D848;
extern FieldBgTile3* D_8007EBD4;
extern s16 D_8011448C;
extern s16 D_801144C8;
extern s16 D_801144D0;

/* Build the sprite packets for the field background's four layers. Layers 1
 * and 2 are 16x16 sprites, layers 3 and 4 are 32x32; each layer walks the run
 * list from where the previous one stopped, emitting one packet per tile and a
 * DR_MODE whenever a run asks for a different texture page. `pairs` collects
 * the two per-sprite parameter bytes the animation code later edits in place.
 *
 * The four run walks are goto loops on purpose. Written as `while` or as
 * `for (;;) { if (...) break; }` gcc's duplicate_loop_exit_test copies the
 * 0x7FFF test to the bottom of each loop, which costs two rows per layer and
 * shifts the whole register allocation; a backward goto is not a loop gcc
 * recognises, so the test stays at the top and is reached by a plain `j`.
 *
 * 25 rows out, and they are one register-allocation decision repeated: the
 * target keeps `modes` in s8 and spills both sprite counters (0x20 and 0x28);
 * this build keeps spriteCount in s8 and spills `modes`, so every `modes++`
 * costs an lw/addiu/sw instead of one addiu. The target's frame is 0x10 larger
 * than ours with 36 bytes of its locals never touched, which says it carried
 * one more long-lived value than this C creates -- find that value and the
 * allocation should fall out. Two structural findings are already folded in
 * above: the goto loops, and the explicit if/else around SetSemiTrans.
 * Measured and rejected: `while` loops with the same body (43 rows, frame
 * 0x10 too large), assigning `run` after tile1/tpages (no change), and
 * `for (;;) { if (...) break; }` (38 rows).
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldBackgroundInitPackets);
#else
void FieldBackgroundInitPackets(
    SPRT_16* sprt16, SPRT* sprt, u8* pairs, DR_MODE* modes) {
    FieldBgData* data;
    FieldBgTile1* tile1;
    FieldBgTile2* tile2;
    u16* tpages;
    s16* run;
    s16 count;
    u16 spriteCount;
    u16 sprite34Count;

    spriteCount = 0;
    sprite34Count = 0;
    D_8011448C = 0;
    D_801144D0 = 0;
    data = *D_8009D848;
    run = data->runs;
    tile1 = (FieldBgTile1*)((u8*)data + data->layer1Offset);
    tpages = (u16*)((u8*)data + data->tpageOffset);

layer1:
    if (run[0] == 0x7FFF) {
        run++;
        goto layer2;
    }
    if (run[0] == 0x7FFE) {
        SetDrawMode(modes, 0, 1, *tpages++, NULL);
        D_8011448C++;
        modes++;
    } else {
        count = run[2];
        if (count != 0) {
            do {
                SetSprt16(sprt16);
                SetShadeTex(sprt16, 1);
                SetSemiTrans(sprt16, 0);
                sprt16->r0 = 0x80;
                sprt16->g0 = 0x80;
                sprt16->b0 = 0x80;
                sprt16->x0 = tile1->x;
                sprt16->y0 = tile1->y;
                sprt16->u0 = tile1->u;
                sprt16->v0 = tile1->v;
                sprt16->clut = tile1->clut;
                spriteCount++;
                tile1++;
                sprt16++;
                pairs += 2;
            } while (--count != 0);
        }
    }
    run += 3;
    goto layer1;

layer2:
    D_8011448C = spriteCount - D_8011448C;
    data = *D_8009D848;
    tile2 = (FieldBgTile2*)((u8*)data + data->layer2Offset);

layer2run:
    if (run[0] == 0x7FFF) {
        run++;
        goto layer3;
    }
    count = run[2];
    if (count != 0) {
        do {
            SetDrawMode(modes, 0, 1, tile2->tpage, NULL);
            D_801144D0++;
            SetSprt16(sprt16);
            SetShadeTex(sprt16, 1);
            if (tile2->flags & 0x80) {
                SetSemiTrans(sprt16, 1);
            } else {
                SetSemiTrans(sprt16, 0);
            }
            modes++;
            sprt16->r0 = tile2->rg;
            sprt16->b0 = 0x80;
            sprt16->g0 = tile2->rg >> 8;
            sprt16->x0 = tile2->x;
            sprt16->y0 = tile2->y;
            sprt16->u0 = tile2->u;
            sprt16->v0 = tile2->v;
            sprt16->clut = tile2->clut;
            pairs[0] = tile2->flags;
            pairs[1] = tile2->param;
            spriteCount++;
            tile2++;
            sprt16++;
            pairs += 2;
        } while (--count != 0);
    }
    run += 3;
    goto layer2run;

layer3:
    D_801144C8 = spriteCount;
    data = *D_8009D848;
    D_8007EBD4 = (FieldBgTile3*)((u8*)data + data->layer34Offset);

layer3run:
    if (run[0] == 0x7FFF) {
        run++;
        goto layer4;
    }
    if (run[0] == 0x7FFE) {
        SetDrawMode(modes, 0, 1, *tpages++, NULL);
        modes++;
    } else {
        count = run[2];
        run[1] = sprite34Count;
        if (count != 0) {
            do {
                SetSprt(sprt);
                SetShadeTex(sprt, 1);
                if (D_8007EBD4->flags & 0x80) {
                    SetSemiTrans(sprt, 1);
                } else {
                    SetSemiTrans(sprt, 0);
                }
                sprt->r0 = 0x80;
                sprt->g0 = 0x80;
                sprt->b0 = 0x80;
                sprt->x0 = D_8007EBD4->x;
                sprt->y0 = D_8007EBD4->y;
                sprt->u0 = D_8007EBD4->u;
                sprt->v0 = D_8007EBD4->v;
                sprt->clut = D_8007EBD4->clut;
                sprt->w = 0x20;
                sprt->h = 0x20;
                pairs[0] = D_8007EBD4->flags;
                pairs[1] = D_8007EBD4->param;
                sprite34Count++;
                D_8007EBD4++;
                sprt++;
                pairs += 2;
            } while (--count != 0);
        }
    }
    run += 3;
    goto layer3run;

layer4:
    if (run[0] == 0x7FFF) {
        return;
    }
    if (run[0] == 0x7FFE) {
        SetDrawMode(modes, 0, 1, *tpages++, NULL);
        modes++;
    } else {
        count = run[2];
        run[1] = sprite34Count;
        if (count != 0) {
            do {
                SetSprt(sprt);
                SetShadeTex(sprt, 1);
                if (D_8007EBD4->flags & 0x80) {
                    SetSemiTrans(sprt, 1);
                } else {
                    SetSemiTrans(sprt, 0);
                }
                sprt->r0 = 0x80;
                sprt->g0 = 0x80;
                sprt->b0 = 0x80;
                sprt->x0 = D_8007EBD4->x;
                sprt->y0 = D_8007EBD4->y;
                sprt->u0 = D_8007EBD4->u;
                sprt->v0 = D_8007EBD4->v;
                sprt->clut = D_8007EBD4->clut;
                sprt->w = 0x20;
                sprt->h = 0x20;
                pairs[0] = D_8007EBD4->flags;
                pairs[1] = D_8007EBD4->param;
                sprite34Count++;
                D_8007EBD4++;
                sprt++;
                pairs += 2;
            } while (--count != 0);
        }
    }
    run += 3;
    goto layer4;
}
#endif

/* Background layer wrap distances, in the field's trigger block. Layers 3 and
 * 4 scroll on their own and a tile that leaves the camera window is moved a
 * whole layer width or height rather than being redrawn. */
typedef struct {
    /* 0x00 */ u8 unk00[0x18];
    /* 0x18 */ u16 wrapX3;
    /* 0x1A */ u16 wrapY3;
    /* 0x1C */ u16 wrapX4;
    /* 0x1E */ u16 wrapY4;
} FieldBgWrap;

/* Right and bottom edge of the camera window, one entry per scrolling group:
 * [0] layers 1 and 2, [1] layer 3, [2] layer 4. Reading these through the
 * array is what keeps them out of a register across the tile loops -- as a
 * plain scalar gcc decides they cannot alias the struct stores and hoists the
 * load, where the original reloads the bound every iteration. */
typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
} FieldBgCamera;

extern FieldBgCamera D_80071A48[3];
/* The ordering-table slots layers 3 and 4 link their sprites into. Read
 * through the struct for the same reason as FieldBgCamera above: as scalars
 * gcc hoists the load out of the walk and keeps the slot in a register. */
typedef struct {
    /* 0x00 */ u16 layer4;
    /* 0x02 */ u16 layer3;
} FieldBgOtSlot;

extern FieldBgOtSlot D_8009ACA2;

/* Link this frame's visible background tiles into the field's ordering table.
 * The four layers share one run list -- the same one FieldBackgroundInitPackets
 * walked to build the packets -- with a 0x7FFF word between them, so each layer
 * starts where the previous one stopped. A 0x7FFE run is a texture-page change
 * and contributes only its DR_MODE.
 *
 * Layers 1 and 2 are static: a run is skipped whole when its row is off the
 * camera, and each tile is culled again on x. Layer 2 additionally carries the
 * OT slot inside the sprite's own r0/g0 bytes and links a DR_MODE behind every
 * tile. Layers 3 and 4 scroll independently, so a tile that has left the
 * window is wrapped in place, and both are bracketed by their own draw
 * environments.
 *
 * Four findings are folded in above and are worth more than this function --
 * see CLAUDE.md, where each has its own bullet:
 *   - the camera bounds, the two OT slots and the animation pairs are read
 *     through an array or a struct, not as scalars, so their loads stay inside
 *     the tile loops instead of being hoisted (MEM_IN_STRUCT_P aliasing);
 *   - the four walks are `for (;;)` loops left by `goto`, which keeps the
 *     invariant hoisting a backward goto would lose without paying for
 *     duplicate_loop_exit_test's rotation;
 *   - the globals are read directly rather than through locals, so their loads
 *     join the other movables in each phase's preheader;
 *   - the wrap test is an `||` of two `<=`, not `!(a && b)`; gcc 2.6.3 does not
 *     apply De Morgan and the two spellings schedule differently.
 *
 * 1 instruction out and 211 rows of register naming. What is left is delay
 * slots: the target wastes a slot after each of the four wrap tests' camera
 * loads (it evaluates the sprite coordinate first, which no spelling of the
 * comparison reproduced -- five were measured), and fills the entity-check
 * branch in layer 3 with the join block's `sll` where this build emits a nop
 * and the `sll` after the join. Being those four instructions short shifts
 * every later branch immediate, and the temporaries all sit one register off:
 * the target puts `sprite` in t3 and the run walk in t7, this build t0 and t5.
 * The &run[2]-versus-&run[1] induction variable this note used to blame has
 * been measured and is not the problem: both builds compute the same two
 * bases, `addiu <r>,v0,0x10` and `addiu <r>,v0,0x14` at 0x2328, and differ
 * only in which register each lands in. Referencing run[2] before run[1] in
 * all four walks was measured -- 236 rows against 233 -- and does not move the
 * allocation either. Read as a whole the residue is a *permutation* of the
 * caller-saved set and nothing else: the target holds the run fields and the
 * sprite cursor in {t7, t1, t5, t2, t6, t8, t0, t9} where this build uses
 * {t5, t2, t6, t3, t7, t1, t8, t9} -- the same registers, the same count, and
 * every one of the 233 rows a rename. No missing or extra hoist, no frame
 * difference and no scheduling difference is left to find by reading; this one
 * belongs to decomp-permuter now. Measured and rejected: goto loops (434 rows,
 * no hoisting at all), `break` out of the loops (396), s32 wrap temporaries
 * (frame 0x30, 320 rows), a separate s32 temporary loaded before the wrap test
 * (278), locals for D_8011448C/D_801144C8/g_FieldTriggers (305), and both
 * `!(a && b)` and `a > lo && a < hi` for the wrap test (251).
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", AddBackgroundToRender);
#else
void AddBackgroundToRender(struct FieldRenderData* buf) {
    FieldBgData* data;
    s16* run;
    s16 count;
    s16 sprite;
    s16 x;
    s16 y;
    s32 otSlot;
    u8 entity;

    data = *D_8009D848;
    run = data->runs;

    for (;;) {
        if (run[0] == 0x7FFF) {
            run++;
            goto layer2;
        }
        if (run[0] == 0x7FFE) {
            addPrim(&buf->ot[0xFFF], &buf->BgDm[run[1]]);
        } else if (
            D_80071A48[0].y - 0x100 < run[0] && run[0] < D_80071A48[0].y) {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (D_80071A48[0].x - 0x150 < buf->Bg1[sprite].x0 &&
                        buf->Bg1[sprite].x0 < D_80071A48[0].x) {
                        addPrim(&buf->ot[0xFFF], &buf->Bg1[sprite]);
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer2:
    for (;;) {
        if (run[0] == 0x7FFF) {
            run++;
            goto layer3;
        }
        if (D_80071A48[0].y - 0x100 < run[0] && run[0] < D_80071A48[0].y) {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (D_80071A48[0].x - 0x150 < buf->Bg1[sprite].x0 &&
                        buf->Bg1[sprite].x0 < D_80071A48[0].x) {
                        entity = buf->BgAnim[sprite].entity & 0x3F;
                        if (entity == 0 || (buf->BgAnim[sprite].mask &
                                            g_FieldEntityBgTrigger[entity])) {
                            otSlot = buf->Bg1[sprite].r0 +
                                     (buf->Bg1[sprite].g0 << 8);
                            addPrim(&buf->ot[otSlot], &buf->Bg1[sprite]);
                            addPrim(&buf->ot[otSlot],
                                    &buf->BgDm[sprite - D_8011448C]);
                        }
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer3:
    addPrim(&buf->ot[D_8009ACA2.layer3], &buf->BgDrenv3E);
    for (;;) {
        if (run[0] == 0x7FFF) {
            addPrim(&buf->ot[D_8009ACA2.layer3], &buf->BgDrenv3S);
            run++;
            goto layer4;
        }
        if (run[0] == 0x7FFE) {
            addPrim(
                &buf->ot[D_8009ACA2.layer3], &buf->BgDm[run[1] + D_801144D0]);
        } else {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (buf->Bg2[sprite].x0 <= D_80071A48[1].x - 0x160 ||
                        D_80071A48[1].x <= buf->Bg2[sprite].x0) {
                        x = buf->Bg2[sprite].x0;
                        if (x < D_80071A48[1].x - 0xA0) {
                            buf->Bg2[sprite].x0 =
                                x + ((FieldBgWrap*)g_FieldTriggers)->wrapX3;
                        } else {
                            buf->Bg2[sprite].x0 =
                                x - ((FieldBgWrap*)g_FieldTriggers)->wrapX3;
                        }
                    }
                    if (buf->Bg2[sprite].y0 <= D_80071A48[1].y - 0x100 ||
                        D_80071A48[1].y <= buf->Bg2[sprite].y0) {
                        y = buf->Bg2[sprite].y0;
                        if (y < D_80071A48[1].y - 0x70) {
                            buf->Bg2[sprite].y0 =
                                y + ((FieldBgWrap*)g_FieldTriggers)->wrapY3;
                        } else {
                            buf->Bg2[sprite].y0 =
                                y - ((FieldBgWrap*)g_FieldTriggers)->wrapY3;
                        }
                    }
                    entity = buf->BgAnim[sprite + D_801144C8].entity & 0x3F;
                    if (entity == 0 || (buf->BgAnim[sprite + D_801144C8].mask &
                                        g_FieldEntityBgTrigger[entity])) {
                        addPrim(&buf->ot[D_8009ACA2.layer3], &buf->Bg2[sprite]);
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer4:
    addPrim(&buf->ot[D_8009ACA2.layer4], &buf->BgDrenv4E);
    for (;;) {
        if (run[0] == 0x7FFF) {
            addPrim(&buf->ot[D_8009ACA2.layer4], &buf->BgDrenv4S);
            return;
        }
        if (run[0] == 0x7FFE) {
            addPrim(
                &buf->ot[D_8009ACA2.layer4], &buf->BgDm[run[1] + D_801144D0]);
        } else {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (buf->Bg2[sprite].x0 <= D_80071A48[2].x - 0x160 ||
                        D_80071A48[2].x <= buf->Bg2[sprite].x0) {
                        x = buf->Bg2[sprite].x0;
                        if (x < D_80071A48[2].x - 0xA0) {
                            buf->Bg2[sprite].x0 =
                                x + ((FieldBgWrap*)g_FieldTriggers)->wrapX4;
                        } else {
                            buf->Bg2[sprite].x0 =
                                x - ((FieldBgWrap*)g_FieldTriggers)->wrapX4;
                        }
                    }
                    if (buf->Bg2[sprite].y0 <= D_80071A48[2].y - 0x100 ||
                        D_80071A48[2].y <= buf->Bg2[sprite].y0) {
                        y = buf->Bg2[sprite].y0;
                        if (y < D_80071A48[2].y - 0x70) {
                            buf->Bg2[sprite].y0 =
                                y + ((FieldBgWrap*)g_FieldTriggers)->wrapY4;
                        } else {
                            buf->Bg2[sprite].y0 =
                                y - ((FieldBgWrap*)g_FieldTriggers)->wrapY4;
                        }
                    }
                    if (D_80071A48[2].x - 0x160 < buf->Bg2[sprite].x0 &&
                        buf->Bg2[sprite].x0 < D_80071A48[2].x) {
                        entity = buf->BgAnim[sprite + D_801144C8].entity & 0x3F;
                        if (entity == 0 ||
                            (buf->BgAnim[sprite + D_801144C8].mask &
                             g_FieldEntityBgTrigger[entity])) {
                            addPrim(
                                &buf->ot[D_8009ACA2.layer4], &buf->Bg2[sprite]);
                        }
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }
}
#endif

s32 FieldCalcLinearStep(s32 start, s32 target, s32 duration, s32 step) {
    s32 delta = target - start;

    if ((u32)(delta + 0x7FFFF) <= 0xFFFFE) {
        start += (delta * step) / duration;
    } else {
        start += (delta / duration) * step;
    }

    return start;
}

s32 FieldCalcEaseInOut(s32 from, s32 to, s32 total, s32 step) {
    s32 angle;
    s32 diff;

    angle = ((step << 12) / total) / 32 - 0x80;
    diff = to - from;
    return from +
           ((FieldEntityGetDirVectorY(angle & 0xFF) + 0x1000) * diff) / 0x2000;
}

s32 FieldCalcWorldToScreenPos(SVECTOR* worldPos, long* screenPos) {
    long flag;
    long depth;
    s32 ret;

    PushMatrix();
    SetRotMatrix(D_80071E40);
    SetTransMatrix(D_80071E40);
    SetGeomOffset(0, 0);
    ret = RotTransPers(worldPos, screenPos, &flag, &depth);
    PopMatrix();
    return ret;
}

/* Advance one axis of the SHAKE camera effect. The shake runs as a chain of
 * segments: each one eases the background offset from where the previous
 * segment left off to a fresh random target, negated relative to the last so
 * the image swings either side of centre. Clearing `enabled` does not stop it
 * dead -- it eases one final segment back to zero first.
 *
 * Every instruction matches except the four that increment currentStep: gcc
 * coalesces `step + 1` into a3 (the argument register) where the original
 * keeps it in v0, which lets the original schedule the store into the call's
 * delay slot. */
void FieldBGShakeUpdate(FieldShakeData* shake) {
    s16 step;
    s16 target;

    if (shake->enabled == 1) {
        if (shake->segmentActive == 0) {
            shake->currentStep = 0;
            shake->start = 0;
            shake->target =
                (s16)(g_RandomTable[shake->rngId] * shake->amplitude) / 256;
            shake->segmentActive = 1;
            shake->rngId++;
            return;
        }
        step = shake->currentStep;
        if (shake->numStepsPerSegment < step) {
            target = shake->target;
            shake->currentStep = 0;
            shake->start = target;
            if (target < 0) {
                shake->target =
                    (s16)(g_RandomTable[shake->rngId] * shake->amplitude) / 256;
            } else {
                shake->target =
                    -(s16)(g_RandomTable[shake->rngId] * shake->amplitude) /
                    256;
            }
            shake->rngId++;
            return;
        }
    } else if (shake->segmentActive == 1) {
        step = shake->currentStep;
        if (shake->numStepsPerSegment < step) {
            shake->currentStep = 0;
            shake->start = shake->target;
            shake->target = 0;
            shake->segmentActive = 0;
            shake->rngId++;
            return;
        }
    } else {
        step = shake->currentStep;
        if (shake->numStepsPerSegment == step) {
            shake->currentOffset = 0;
            return;
        }
    }
    /* The two arms are deliberately identical. gcc cross-jumps them back into
     * one block, but only after it has allocated `step + 1` to its own
     * register instead of coalescing it into a3 -- which is what lets the
     * store of currentStep be scheduled into the call's delay slot, as the
     * original does. Written once, the tail is four instructions off. */
    if (step != 0) {
        step = step + 1;
        shake->currentStep = step;
        shake->currentOffset = FieldCalcEaseInOut(
            shake->start, shake->target, shake->numStepsPerSegment, step);
    } else {
        step = step + 1;
        shake->currentStep = step;
        shake->currentOffset = FieldCalcEaseInOut(
            shake->start, shake->target, shake->numStepsPerSegment, step);
    }
}
