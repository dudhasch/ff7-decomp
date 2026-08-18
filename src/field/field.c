//! PSYQ=3.3 CC1=2.6.3
#include <game.h>
#include <libetc.h>

#define GET_PARAM_U8(offset)                                                   \
    (*(u8*)((s32)g_FieldScripts + g_FieldScriptPC[g_CurrentEntity] + (offset)))
#define GET_PARAM_S16(value, offset)                                           \
    value = GET_PARAM_U8(offset);                                              \
    value |= (GET_PARAM_U8((offset) + 1) << 8)
#define PC_INC(x) (g_FieldScriptPC[g_CurrentEntity] += (x))
#define PC_DEC(x) (g_FieldScriptPC[g_CurrentEntity] -= (x))

#define GET_PRIORITY(x) (((x) >> 5) & 0x7)
#define GET_SCRIPTID(x) ((x) & 0x1F)

#define ADD_PARTY_MEMBER(slot, charId)                                         \
    Savemap.memory_bank_2[9 + slot] = charId;                                  \
    if (charId != 0xFF) {                                                      \
        u16 mask;                                                              \
        u16 bit;                                                               \
        bit = charId;                                                          \
        mask = Savemap.phs_visibility_mask;                                    \
        bit = 1 << bit;                                                        \
        mask |= bit;                                                           \
        Savemap.phs_visibility_mask = mask;                                    \
    }

typedef enum {
    IF_EQ,
    IF_NOT_EQ,
    IF_GT,
    IF_LT,
    IF_GTE,
    IF_LTE,
    IF_AND,
    IF_XOR,
    IF_OR,
    IF_BIT,
    IF_NOT_BIT
} IfOps;

typedef struct FieldRenderData {
    OT_TYPE ot[0x1000];   // 0x00000: Main scene ordering table
    SPRT_16 Arrows[0x18]; // 0x04000: Field arrow sprite packets
    DR_MODE ArrowsDm;     // 0x04180: Arrow sprite draw mode

    OT_TYPE OtFadeDrenv;  // 0x0418c: Fade draw environment OT entry
    OT_TYPE OtSceneDrenv; // 0x04190: Scene draw environment OT entry

    DR_ENV FadeDrenv;  // 0x04194: Screen fade draw environment
    DR_ENV SceneDrenv; // 0x041d4: Main scene draw environment

    DR_ENV BgDrenv3S; // 0x04214: Background layer 3 start env
    DR_ENV BgDrenv4S; // 0x04254: Background layer 4 start env
    DR_ENV BgDrenv3E; // 0x04294: Background layer 3 end env
    DR_ENV BgDrenv4E; // 0x042d4: Background layer 4 end env

    u8 unk4314[0x600]; // 0x04314: Unknown render data

    SPRT_16 Bg1[0x9c4]; // 0x04914: Background layer 1/2 sprites
    SPRT Bg2[0x200];    // 0x0e554: Background layer 3/4 sprites

    u16 BgAnim[0xbc4];   // 0x10d54: Background animation data
    DR_MODE BgDm[0x6a4]; // 0x124dc: Background draw mode packets

    OT_TYPE OtUi;       // 0x1748c: UI ordering table
    DR_MODE RainDm;     // 0x17490: Rain draw mode
    LINE_F2 Rain[0x40]; // 0x1749c: Rain line primitives
};
extern struct FieldRenderData g_FieldRenderData[2]; // double buffered

const u32 D_800A0000[] = {0, 0x01D801E0};
extern char g_FieldDebugDigits[16]; // '0' to 'F' for hex digits
extern char D_800A0270[4];
extern s32 (*g_FieldOpcodes[256])(void);
extern s8 D_800E0628;
extern s8 D_800E0630;
extern u16 g_FieldBGCameraHeightBias; // camera height bias applied to the
                                      // tracked entity
extern volatile u8
    g_FieldBGTrackedEntity; // entity the background scroll tracks
extern u32 g_FieldPadRaw;   // last raw pad state read this frame
extern u32
    g_FieldPad1State; // pad 1: state, previous, newly pressed, newly released
extern u8 g_FieldEntityBgTrigger[]; // per-entity background-trigger bits, one
                                    // byte each
extern u32 g_FieldPad1PrevState;
extern u32 g_FieldPad1Pressed;
extern u32 g_FieldPad1Released;
extern u32 g_FieldPad2State; // pad 2: same four
extern u32 g_FieldPad2PrevState;
extern u32 g_FieldPad2Pressed;
extern u32 g_FieldPad2Released;
extern s32 func_8001C808(void);
extern s16 D_800E0748[];
extern s16 D_800E074A[];
extern s16 D_800E074C[];
extern s16 D_800E074E[];
extern u8 D_800E0750[];
extern u8 D_800E0751[];
extern u8 D_800E0752[];
extern s16 D_800E0754[];
extern s16 D_800E0756[];
extern char D_800E0758[];
extern u8 D_800E08A8[];
extern u8 D_800E08C0[];
extern u8 D_800DFDFC[];
extern u8 D_80071C20;
extern u8 g_EntityForSplitJoin;
extern s16 D_800DF120[][2];
extern s16 g_FieldDebugRb;
/* Double-buffered 7-entry ordering table for the debug overlay. Entry 6 is the
 * tail the overlay's primitives hang off, entry 0 the head linked into the
 * caller's OT. */
extern u_long D_800E41C8[2][7];
extern s16 g_FieldDebugRChars;
extern s16 g_FieldDebugRLines;
extern s16 g_FieldDebugRRect;
extern s16 g_FieldDebugRDm;
extern u16 g_FieldDebugTransp;
extern char g_DebugText[];          // debug text
extern char g_DebugMessageBuffer[]; // debug value transformed into text

extern u8 g_FieldScriptDebugEntities[];
extern u8 g_actorIdCur;
extern u8 g_RandomTableStep;
extern u8 g_RandomTableIndex;
extern u8 g_RandomTable[256];
extern u8 g_DialogDigitCharacters[16];
extern s16 D_800E42EE[0x40][12];
extern u8 g_WindowReplaceBank[4][8];
extern u16 g_WindowReplaceBankAddr[4][8];
extern s16 g_WindowWaitTime[4];
extern u8* g_WindowStringPtr[4];
extern u8 g_WindowString[4][256];
extern s16 g_WindowNameCopyCount[4];
extern s16 g_WindowReplaceParam[4];
extern s16 g_WindowExtraRows[4];
extern s16 g_WindowTextBudget[4];
extern s16 g_WindowFastForwardLevel[4];
extern s16 g_WindowBufferPos[4];
extern u8 g_WindowBuffer[4][16];
extern s16 g_WindowTotalRowsHeight[4];

/* volatile: the movie stream sets this from an interrupt callback, and it is
 * what keeps the s16 conversion in FieldUpdateMovieStream a separate
 * sign-extension instead of folding into a signed load. */
extern volatile u16 g_FieldMovieStreamActive;
extern u8 g_FieldExitArrowState[];
extern u8 D_8009D5A7;
extern u8 D_800716CC;
extern u8
    g_FieldMovieOpcodeActive; // set while a movie opcode is driving playback
extern u32 D_80075E10;        // top of the buffer the movie stream decodes into
extern u16 D_800E42A8[]; // per-model default walk speed, indexed by model id
extern s16 g_FieldMovieStreamDone;
void func_80034FC8(u32 buffer, s16 movieId); // STR ring setup
void func_800354CC(void);                    // STR playback start
void func_80035658(void);                    // STR playback stop
extern s32 D_8009A010;
extern s32 D_8009A014;
extern s16 D_801144D4;
extern u8 D_80095DE0[];
extern s32 g_BattleCharIdToCharId[11];
extern u8 D_8009AD30[];
extern FieldState D_8009ABF4;
extern u8 g_FieldRandListIndex;
extern u8 g_FieldRandListOffset;
extern s32 D_8009A108;
extern s32 D_80099FCC[];
extern u8 g_FieldCameraMatrixSel;
extern u8 g_FieldAnimLock;
extern u8 g_FieldAnimFreeze;
extern u8 D_80081DC4;
extern s16 D_80114464;
extern s16 D_80114468;
extern u8 D_80114490;
// Two POLY_FT4 at 0x800E48F4, filling the gap between g_EntityForSplitJoin
// (0x800E48F0) and g_WindowString (0x800E4944) exactly. Confirmed by the
// 0x28 stride, len 9 / code 0x2C at +3 / +7, clut at +0x0E and tpage at +0x16.
extern POLY_FT4 D_800E48F4[2];
s32 GetGraphType(void);
extern MATRIX** D_80083578;
extern MATRIX* D_80083270;
extern s16 D_8009A162;
extern u8 D_8009A15C;

void SystemRefreshParty(void);
/* Handwritten assembly. The s16 colour params are load-bearing: they put the
 * truncation at the call site, which is what splits `sll`/`sra` across the
 * loop preheader and body in KawaiSetColorToModelPkts. */
void KawaiSetColorToPartPkts(u8* part, s16 r, s16 g, s16 b);
void func_80025648(u32 materia, u8 slot);
void FieldDialogSetWindowStyleCbc(s16 window, u8 style, s16 preventClose);
void FieldDialogSetWindowHeight(s16 window, s16 height);
void FieldDebugPageSetPosSize(s16 page, s16 x, s16 y, s16 w, s16 h);
void FieldDebugPageResetStrings(s16 page);
void FieldDialogMove(s16 window, s16 dx, s16 dy);
u16 func_80025310(u16 itemId);
s32 OpcodeFuncWsize(void);
s32 FieldWindowSetStateToClose(s16 window);
s32 FieldDialogMessageUpdateStates(u8 window, u8 message);
void func_80025288(u16 itemId);
void func_80025380(u16 itemId);
s32 func_8002542C(u32 materia);
u8 func_80025650(u32 materia, u8 slot);
void SystemMenuAddHpByPartyId(s32 partyId, s32 amount);
void SystemMenuAddMpByPartyId(s32 partyId, s32 amount);
void func_80025800(s32 partyId, s32 amount);
void func_80025988(s32 partyId, s32 amount);
void FieldEventSetDirByActorId(u8 actorId);
void FieldMoveToEntityUpdate(u8 actorId);
void FieldEntityTurnToEntity(u8 actorId);
void func_80020058(s16 partyId);
void func_8001786C(s16 partyId);
void func_80017678(void);

typedef struct {
    /* 0x00 */ LinePos pos;
    /* 0x0C */ s16 destPosX;
    /* 0x0E */ s16 destPosY;
    /* 0x10 */ u16 destWalkMeshId;
    /* 0x12 */ u16 destFieldId; // 0x7FFF marks an unused gateway slot
    /* 0x14 */ u8 destDirection;
    /* 0x15 */ u8 pad[3];
} FieldGateway; // size:0x18

void AddBackgroundToRender(struct FieldRenderData* buf);
s32 FieldEntitySqrDistToLine(FieldLine*, s32*, s32*);
void FieldEntityLineInteract(FieldEntity* arg0, FieldLine* arg1);
void HandleKawaiDataInModel(struct FieldRenderData* buf);
void FieldEventOpcodeCycle(void);
void FieldUpdateAnimationState(void);
u8 FieldEventRequestRun(s16 entityId, s16 priority, s16 scriptId);
void DebugUpdateActor(s32 arg0, u8 actorId);
void DebugPrintOpcode(char* arg0, u32 arg1);
u8 FieldEventReadMemoryU8(s16 arg0, s16 arg1);
void FieldEventWriteMemoryU8(s16 arg0, s16 arg1, u8 value);
s32 FieldDialogAskUpdateStates(
    u8 windowId, u8 firstRow, u8 lastRow, u8 cancelRow, s16* answer);
s16 FieldEventReadMemoryS16(s16 arg0, s16 arg1);
void FieldEventWriteMemoryS16(s16 arg0, s16 arg1, s16 value);
u32 IfCheck(void);
u32 If2CheckSigned(void);
u32 If2CheckUnsigned(void);
static s32 KeyCheck(u16 keys);
s32 FieldEventRequest(s16 type, u8 target, u8 priority, u8 scriptId);
static u32 GetAkaoBlockOffset(s16 akaoId);
static void PartyReplace(u8* newParty);
static void PartyFromBank2ToSave(s32 unused);
static void PartyRemove(u8* party, u8* toRemove);
static void PartyAdd(u8* party, u8* toAdd);
static void DebugPrintToFieldWindow(const char* str);
static void FieldEventDebugError(const char* errmsg);
void FieldWindowReset(s16 window);
void FieldWindowResetTextAll(void);
s32 AddStrNextDebugRow(s16 page, const char* str);
s32 SetStrToDebugRow(s16 page, s16 row, const char* str);
static void FieldDebugStringCopy(char* dst, const char* src);
static void FieldDebugStringConcat(char* arg0, char* arg1);
static void FieldDebugStringU8hex(s32 val, char* msg_out);
static void FieldDebugStringU16hex(s32 val, char* msg_out);
static void FieldDebugStringU32hex(s32 val, char* msg_out);
static void PlayWindowPointerClickSound(void);
s32 FieldDialogWindowInit(s16 window, s16 stringId);
void FieldDialogWindowGrowth(s16 window);
void FieldDialogCopyTextFromField(s16 window);
void DialogScrollText(s16 window);
void DialogScrollTextDuringOk(s16 window);
void FieldDialogWindowInitNext(s16 window);
s32 FieldDialogWindowDecrease(s16 window);
u16 FieldDialogGetVariableFromBank(s16 window);
void ConvertDigitToString(u16 value, u8* dst);
void ConvertNumToStrWithSpace(u16 value, u8* dst);
void ConvertHexToString(u16 value, u8* dst);

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

extern FieldFileInfo g_FieldFileTable[];
extern u16 g_FieldMoviePlayed;
extern u16 g_FieldPreloadMapId;
extern s32 g_WmPreSector;
extern u32 g_WmPreSize;

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", PreloadNextFieldMap);
#else

// External Declarations
extern u8 D_8009ABF5;
extern u8 g_FieldAnimLock;
extern s16 D_80071A5C;

// D_8009ABF5 = g_FieldState -> command

void PreloadNextFieldMap(FieldEntity* Player, FieldLine* gateway) {
    s16* ptr_a3;
    s32* scratchpad;
    s32 min_dist;
    s32 counter;
    s16* ptr_a1;
    s32 term_val;
    s32 diff_x, diff_y, dist;
    s16 map_id;
    FieldFileInfo* table;
    s32 sector;
    u32 size;

    ptr_a3 = gateway;
    min_dist = 0x7FFFFFFF;

    scratchpad = 0x1F800000;
    scratchpad[0] = Player->PosX >> 12;
    scratchpad[1] = Player->PosY >> 12;
    scratchpad[2] = Player->PosZ >> 12;

    if (g_FieldAnimLock == 0) {
        counter = 0;
        term_val = 0x7FFF;
        ptr_a1 = (gateway + 0x12);

        do {
            map_id = ptr_a1[0];
            if (map_id != term_val) {
                diff_x = ptr_a3[0] - scratchpad[0];
                diff_y = ptr_a1[-8] - scratchpad[1];
                dist = (diff_x * diff_x) + (diff_y * diff_y);

                if (dist < min_dist) {
                    min_dist = dist;
                    g_FieldPreloadMapId = map_id;
                }
            }

            counter++;
            ptr_a1 = (ptr_a1 + 0x18);
            ptr_a3 = (ptr_a3 + 0x18);
        } while (counter < 12);
    }

    if (D_8009ABF5 == 3 || (g_FieldMoviePlayed == 1) || D_8009ABF5 == 2) {
        StopFieldMapPreload();
        return;
    }

    if (D_80071A5C == g_FieldPreloadMapId) {
        return;
    }

    table = g_FieldFileTable;
    if (0x4DFFF < table[g_FieldPreloadMapId].datSize) {
        return;
    }

    StopFieldMapPreload();
    D_80071A5C = g_FieldPreloadMapId;

    if (D_80071A5C >= 0x41) {
        sector = table[D_80071A5C].datSector;
        size = table[D_80071A5C].datSize;
    } else {
        sector = g_WmPreSector;
        size = g_WmPreSize;
    }

    SystemLoadFileBySector(sector, size, 0x801B0000, NULL);
    g_isFieldLoading = 1;
}

#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldMain);

const u32 D_800A0024[] = {0x00000000, 0x000801E0};
const u32 D_800A002C[] = {0x00E80000, 0x000801E0};
const u32 D_800A0034[] = {0x01D00000, 0x000801E0};
const u32 D_800A003C[] = {0x00000000, 0x00080140};
const u32 D_800A0044[] = {0x00E80000, 0x00080140};
const u32 D_800A004C[] = {0x01D00000, 0x00080140};
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldMainLoop);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldLoadMimToVram);

/* Latch both pads: keep the raw state, the previous state, and the edges
 * (newly pressed / newly released) derived from the two. */
/* Two instructions out, both register choices; see the comment on the second
 * pad's release computation below. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldButtonsUpdate);
#else
void FieldButtonsUpdate(void) {
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
    /* The do/while is not cosmetic: without the statement boundary gcc hoists
     * this whole expression above the g_FieldPad2Pressed store. It is most
     * likely a macro in the original. What is left is one register choice --
     * the original puts the `nor` in $a0, freed by the store just above, where
     * gcc reuses $v0. */
    do {
        g_FieldPad2Released = (pad2 ^ old2) & ~pad2;
    } while (0);
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBackgroundInitPackets);

INCLUDE_ASM("asm/us/field/nonmatchings/field", AddBackgroundToRender);

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

static s32 FieldCalcWorldToScreenPos(SVECTOR* worldPos, long* screenPos) {
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBGScrollInit);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldCalcPointOnLine);

/* Scroll limits at the head of the field's trigger block. g_FieldTriggers is
 * typed s32 because it is assigned as a raw word on load. */
typedef struct {
    /* 0x00 */ u8 unk00[0xC];
    /* 0x0C */ s16 minX;
    /* 0x0E */ s16 minY;
    /* 0x10 */ s16 maxX;
    /* 0x12 */ s16 maxY;
} FieldScrollLimits;

#define FIELD_SCROLL_LIMITS ((FieldScrollLimits*)g_FieldTriggers)

/* Keeps a background scroll position half a screen (0xA0 x 0x78) inside the
 * map's scroll limits. */
void FieldBGClampPos(s16* pos) {
    if (FIELD_SCROLL_LIMITS->maxX - 0xA0 < pos[0]) {
        pos[0] = FIELD_SCROLL_LIMITS->maxX - 0xA0;
    }
    if (pos[0] < FIELD_SCROLL_LIMITS->minX + 0xA0) {
        pos[0] = FIELD_SCROLL_LIMITS->minX + 0xA0;
    }
    if (FIELD_SCROLL_LIMITS->maxY - 0x78 < pos[1]) {
        pos[1] = FIELD_SCROLL_LIMITS->maxY - 0x78;
    }
    if (pos[1] < FIELD_SCROLL_LIMITS->minY + 0x78) {
        pos[1] = FIELD_SCROLL_LIMITS->minY + 0x78;
    }
}

/* Project the tracked entity's world position onto the screen, lifting it by
 * the camera's height bias. */
s32 FieldBGGetEntityScreenPos(long* screenPos) {
    SVECTOR pos;
    volatile u8* tracked;

    tracked = &g_FieldBGTrackedEntity;
    pos.vx = g_FieldEntity[*tracked].PosX >> 12;
    pos.vy = g_FieldEntity[*tracked].PosY >> 12;
    pos.vz = (g_FieldEntity[*tracked].PosZ >> 12) + g_FieldBGCameraHeightBias;
    return FieldCalcWorldToScreenPos(&pos, screenPos);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBGScrollUpdate);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBGUpdateDrawenv);

/////////////////////////////////////////////////
// Begin of field_entity.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityInitPos);

void FieldEntityAddRotate(s32 arg0, s16 entityIdx) {
    if (g_FieldAnimLock == 0) {
        if (D_8009ABF4.activeKeys2 & PADR1) {
            g_FieldEntity[entityIdx].MoveDirAdd = 0xE0;
        } else if (D_8009ABF4.activeKeys2 & PADL1) {
            g_FieldEntity[entityIdx].MoveDirAdd = 0x20;
        } else {
            g_FieldEntity[entityIdx].MoveDirAdd = 0;
        }
    }
}

/* Advance one entity's animation clock. animCurrentFrame counts in 1/16ths of
 * a frame, so the comparisons scale animLastFrame by 16. The player's own
 * model loops back to the start; every other entity holds on its last frame.
 * g_FieldAnimFreeze freezes all field animation at once.
 *
 * Not matching: the original reserves an unused 8-byte frame, and its clamp
 * arm keeps animLastFrame live across the branch to recompute `* 16` in the
 * delay slot where gcc reuses the already-shifted value. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityAnimationUpdate);
#else
void FieldEntityAnimationUpdate(s32 entityId) {
    FieldModelEntry* model;
    u8* anims;
    u8 entryIndex;

    entryIndex = g_FieldModelLoaderData[entityId].modelEntryIndex;
    if (entryIndex == 0xFF) {
        return;
    }
    model = &g_FieldModelData->modelEntries[entryIndex];
    anims = model->modelData + model->animationOffset;
    if (g_FieldAnimFreeze != 0) {
        return;
    }
    g_FieldEntity[entityId].animCurrentFrame +=
        g_FieldEntity[entityId].animSpeed;
    if (entityId == g_PlayerModelId && g_FieldAnimLock == 0) {
        g_FieldEntity[entityId].animLastFrame =
            *(u16*)&anims[g_FieldEntity[entityId].activeAnimId * 16] - 1;
        if (g_FieldEntity[entityId].animLastFrame * 16 <
            g_FieldEntity[entityId].animCurrentFrame) {
            g_FieldEntity[entityId].animCurrentFrame = 0;
        }
    } else if (g_FieldEntity[entityId].animLastFrame * 16 <
               g_FieldEntity[entityId].animCurrentFrame) {
        g_FieldEntity[entityId].animCurrentFrame =
            g_FieldEntity[entityId].animLastFrame * 16;
    }
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityMovementUpdate);

void FieldEntityGatewayMapLoad(FieldGateway* gateway) {
    D_8009ABF4.eventCmd = EVTCMD_FIELD_MAP_CHANGE;
    D_8009ABF4.eventCmdParam = gateway->destFieldId;
    D_8009ABF4.pcPosX = gateway->destPosX;
    D_8009ABF4.pcPosY = gateway->destPosY;
    D_8009ABF4.pcWalkMeshId = gateway->destWalkMeshId;
    *(u16*)&D_8009ABF4.pcDirection = gateway->destDirection;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityCheckTalk);

s16 FieldEntityGetDirVectorX(u8 arg0) { return D_800DF120[arg0][0]; }

s16 FieldEntityGetDirVectorY(u8 arg0) { return D_800DF120[arg0][1]; }

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityDirByVec);

u8 FieldEntityDirByVec(VECTOR* from, VECTOR* to, s32* sqrDist);

/* One step of "walk towards MoveEnd". Returns 1 while still moving, 0 once the
 * entity is close enough -- either because it came within `range` of the goal
 * or because the remaining distance is below one frame of MoveSpeed, in which
 * case the position is snapped onto the goal exactly. */
s32 FieldEntityAutoMove(FieldEntity* entity, s16 range) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    s32 reach;

    from.vx = entity->PosX >> 12;
    from.vy = entity->PosY >> 12;
    to.vx = entity->MoveEndX >> 12;
    to.vy = entity->MoveEndY >> 12;
    reach = entity->SolidRange + range;
    sqrDist = (to.vx - from.vx) * (to.vx - from.vx) +
              (to.vy - from.vy) * (to.vy - from.vy);
    reach = reach * reach + 0x1000;
    if (range != 0 && reach >= sqrDist) {
        return 0;
    }
    if (sqrDist < (entity->MoveSpeed * entity->MoveSpeed) >> 16 ||
        sqrDist < 4) {
        entity->PosX = entity->MoveEndX;
        entity->PosY = entity->MoveEndY;
        return 0;
    }
    entity->MoveDir =
        FieldEntityDirByVec(&from, &to, &sqrDist) - entity->MoveDirAdd;
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityWalkmechCross);

static void FieldEntityVectorSub(s32* arg0, s16* arg1, s16* arg2) {
    arg0[0] = arg1[0] - arg2[0];
    arg0[1] = arg1[1] - arg2[1];
    arg0[2] = arg1[2] - arg2[2];
}

/* Height of `point` on the triangle plane spanned by edgeA/edgeB through
 * `vertex`. edgeA doubles as scratch: once the normal is known it is reloaded
 * with the vertex, so the caller must treat it as clobbered. */
s32 FieldEntityCalculateZ(s32* edgeA, s32* edgeB, s32* point, s16* vertex) {
    s32 normal[3];

    normal[0] = -edgeA[1] * edgeB[2] + edgeB[1] * edgeA[2];
    normal[1] = -edgeA[2] * edgeB[0] + edgeA[0] * edgeB[2];
    normal[2] = -edgeA[0] * edgeB[1] + edgeB[0] * edgeA[1];
    edgeA[0] = vertex[0];
    edgeA[1] = vertex[1];
    edgeA[2] = vertex[2];
    return (normal[0] * edgeA[0] + normal[1] * edgeA[1] + normal[2] * edgeA[2] -
            normal[0] * point[0] - normal[1] * point[1]) /
           normal[2];
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityMove);

extern s16 D_8009AC1C;

/* Would `pos` put entity `entityId` inside another solid entity? Two entities
 * collide when their horizontal distance falls under the mean of their two
 * solid radii and they are within ~127 units of each other vertically, so
 * characters on a different floor of the same map never block one another.
 * Only the player's own collisions arm the other entity's push script.
 *
 * Not matching: register assignment only. Every instruction is in the right
 * place, but the original holds &D_8009AC1C in a1 and copies it into t4 for the
 * loop, where gcc keeps one register for both and renames the rest downstream.
 */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityCollisionCheck);
#else
s32 FieldEntityCollisionCheck(s16 entityId, VECTOR* pos) {
    s16 i;
    s32 hit;
    s32 sqrRadius;
    s32 range;
    s16* entityCount;
    s32 dz;
    s32 radius;
    s32 dx;
    s32 dy;

    hit = 0;
    range = g_FieldEntity[entityId].SolidRange;
    entityCount = &D_8009AC1C;
    for (i = 0; i < *entityCount; i++) {
        if (i == entityId) {
            continue;
        }
        if (g_FieldEntity[i].SolidOff != 0) {
            continue;
        }
        dz = (g_FieldEntity[i].PosZ >> 12) - pos->vz;
        if (dz < -126 || dz > 127) {
            continue;
        }
        sqrRadius = (range + g_FieldEntity[i].SolidRange) >> 1;
        radius = sqrRadius;
        dx = (g_FieldEntity[i].PosX - pos->vx) >> 12;
        dy = (g_FieldEntity[i].PosY - pos->vy) >> 12;
        sqrRadius = radius * radius;
        if (sqrRadius > dx * dx + dy * dy) {
            hit = 1;
            if (entityId == g_PlayerModelId) {
                g_FieldEntity[i].requestPushScript = 1;
            }
        }
    }
    return hit;
}
#endif

/* Squared distance from `point` to the segment `line`, with the foot of the
 * perpendicular written to `nearest`. Returns -1 when that foot lands outside
 * the segment on either the x or the y axis, which is how callers tell "past
 * the end of the line" apart from "near it". The line parameter runs in 8-bit
 * fixed point, so the projection stays in integer arithmetic throughout.
 *
 * Not matching: register assignment only. Every instruction and its order are
 * right; the original accumulates the dot product in v1 and holds y1 in a0,
 * where gcc picks the two the other way round and the swap propagates. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntitySqrDistToLine);
#else
s32 FieldEntitySqrDistToLine(FieldLine* line, s32* point, s32* nearest) {
    s32 t;

    t = -(((line->pos.x1 - point[0]) * (line->pos.x2 - line->pos.x1) +
           (line->pos.y1 - point[1]) * (line->pos.y2 - line->pos.y1) +
           (line->pos.z1 - point[2]) * (line->pos.z2 - line->pos.z1))
          << 8) /
        ((line->pos.x2 - line->pos.x1) * (line->pos.x2 - line->pos.x1) +
         (line->pos.y2 - line->pos.y1) * (line->pos.y2 - line->pos.y1) +
         (line->pos.z2 - line->pos.z1) * (line->pos.z2 - line->pos.z1));
    nearest[0] = ((t * (line->pos.x2 - line->pos.x1)) >> 8) + line->pos.x1;
    nearest[1] = ((t * (line->pos.y2 - line->pos.y1)) >> 8) + line->pos.y1;
    nearest[2] = ((t * (line->pos.z2 - line->pos.z1)) >> 8) + line->pos.z1;
    if ((line->pos.x1 - nearest[0] >= 0 && line->pos.x2 - nearest[0] <= 0) ||
        (line->pos.x1 - nearest[0] <= 0 && line->pos.x2 - nearest[0] >= 0)) {
        if ((line->pos.y1 - nearest[1] >= 0 &&
             line->pos.y2 - nearest[1] <= 0) ||
            (line->pos.y1 - nearest[1] <= 0 &&
             line->pos.y2 - nearest[1] >= 0)) {
            t = (nearest[0] - point[0]) * (nearest[0] - point[0]) +
                (nearest[1] - point[1]) * (nearest[1] - point[1]) +
                (nearest[2] - point[2]) * (nearest[2] - point[2]);
            return t;
        }
    }
    return -1;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityLineCheck);

/* Walk the map's 32 trigger lines against one entity and raise the script
 * requests each one is due. Entering a line's radius arms its touch-on script
 * and leaving it arms touch-off, with `touch` holding the edge state between
 * frames. The talk request additionally needs the entity facing within +/-32
 * of the line's proximity angle and the OK button newly pressed this frame --
 * pad2 current has the bit and pad2 previous does not. An entity under script
 * control (scriptedMoveMode) triggers nothing.
 *
 * Not matching: register assignment only. The original keeps the walking line
 * pointer in s2 and the constant 1 in s3; gcc allocates them the other way
 * round. Neither declaration order, statement order, nor indexing with
 * `line[i]` instead of a walking pointer shifts the tie. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityLineInteract);
#else
void FieldEntityLineInteract(FieldEntity* entity, FieldLine* line) {
    s32* from;
    s32* nearest;
    s32 i;
    s32 sqrDist;
    u32* pad2;

    from = (s32*)0x1F800000;
    nearest = (s32*)0x1F800010;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    pad2 = &g_FieldPad2State;
    for (i = 0; i < 32; i++, line++) {
        if (line->isActive != 1) {
            continue;
        }
        if (entity->scriptedMoveMode != 0) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine(line, from, nearest);
        if (sqrDist != -1 &&
            sqrDist < entity->SolidRange * entity->SolidRange) {
            if (line->touch == 0) {
                line->requestTouchOnScript = 1;
            }
            line->touch = 1;
        } else {
            if (line->touch == 1) {
                line->requestTouchOffScript = 1;
            }
            line->touch = 0;
        }
        if (line->isOnLine != 1) {
            continue;
        }
        if ((u8)(line->proximityAngle - entity->MoveDir + 0x20) >= 0x40) {
            continue;
        }
        if (!(pad2[0] & 0x20)) {
            continue;
        }
        if (pad2[1] & 0x20) {
            continue;
        }
        line->requestTalkScript = 1;
    }
}
#endif

static void FieldEntityLineClear(FieldLine* lines) {
    s32 i;

    for (i = 0; i < LEN(g_FieldLines); i++) {
        lines->isOnLine = 0;
        lines++;
    }
}

/* Did this step take the entity across one of the map's twelve gateway lines?
 * The move is staged in the PS1 scratchpad as two points -- where the entity is
 * now and where it wants to go -- and each gateway near enough to matter gets a
 * pair of 2D cross products, one per point. Opposite signs mean the segment
 * crossed the line, which loads the destination map. A gateway whose
 * destFieldId is 0x7FFF is an unused slot. */
void FieldEntityGatewayCheck(
    FieldEntity* entity, FieldGateway* gateway, VECTOR* dest) {
    s32* from;
    s32* to;
    s32* nearest;
    s32 i;
    s32 sqrDist;
    s16 x1;
    s16 y1;
    s32 dx;
    s32 dy;
    s32 crossFrom;
    s32 crossTo;

    from = (s32*)0x1F800000;
    to = (s32*)0x1F800010;
    nearest = (s32*)0x1F800020;
    from[0] = entity->PosX >> 12;
    from[1] = entity->PosY >> 12;
    from[2] = entity->PosZ >> 12;
    to[0] = dest->vx >> 12;
    to[1] = dest->vy >> 12;
    to[2] = entity->PosZ >> 12;
    for (i = 0; i < 12; i++, gateway++) {
        if (gateway->destFieldId == 0x7FFF) {
            continue;
        }
        sqrDist = FieldEntitySqrDistToLine((FieldLine*)gateway, from, nearest);
        if (sqrDist == -1) {
            continue;
        }
        if (sqrDist >= entity->SolidRange * entity->SolidRange) {
            continue;
        }
        x1 = gateway->pos.x1;
        y1 = gateway->pos.y1;
        dx = gateway->pos.x2 - x1;
        dy = gateway->pos.y2 - y1;
        crossFrom = dx * (from[1] - y1) - (from[0] - x1) * dy;
        crossTo = dx * (to[1] - y1) - (to[0] - x1) * dy;
        if ((crossFrom >= 0 && crossTo < 0) ||
            (crossTo >= 0 && crossFrom < 0) ||
            (crossFrom > 0 && crossTo <= 0) ||
            (crossTo > 0 && crossFrom <= 0)) {
            FieldEntityGatewayMapLoad(gateway);
        }
    }
}

/* One entry of the map's background-trigger block. Even `type`s arm the
 * trigger, odd ones disarm it. */
typedef struct {
    /* 0x00 */ u8 unk00[0xC];
    /* 0x0C */ u8 entityId;
    /* 0x0D */ u8 unk0D;
    /* 0x0E */ u8 type;
    /* 0x0F */ u8 unk0F;
} FieldBgTrigger;

/* Arms (even type) or disarms (odd type) one background trigger, and reports
 * whether that actually changed the bit -- the caller only redraws when it did.
 *
 * Blocked by the same jump-table alignment problem as FieldEntityBgTriggerInit:
 * the original table sits at .rodata+0xa4 and gcc's `.align 3` puts ours at
 * +0xa8. Unlike Init this C is not yet instruction-exact either -- gcc
 * cross-jumps the two arms' shared store tail here and does not in the
 * original -- so it needs another pass once the file is split. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityBgTriggerActivate);
#else
s32 FieldEntityBgTriggerActivate(FieldBgTrigger* trigger, u8 type) {
    s32 changed;
    s32 bit;
    s32 old;
    s32 mask;
    u8 merged;

    changed = 0;
    switch (type) {
    case 0:
    case 2:
    case 4:
        old = g_FieldEntityBgTrigger[trigger->entityId];
        bit = 1 << trigger->unk0D;
        if ((old & bit) == 0) {
            changed = 1;
        }
        g_FieldEntityBgTrigger[trigger->entityId] = bit | old;
        break;
    case 1:
    case 3:
    case 5:
        mask = ~(1 << trigger->unk0D);
        old = g_FieldEntityBgTrigger[trigger->entityId];
        merged = old | mask;
        if (merged == 0xFF) {
            changed = 1;
        }
        g_FieldEntityBgTrigger[trigger->entityId] = mask & old;
        break;
    }
    return changed;
}
#endif

const u32 D_800A00BC[] = {0x00360000, 0x012A007A};
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityTriggerCheck);

/* FieldEntityBgTriggerInit below is left as INCLUDE_ASM: every instruction of
 * the C matches, but gcc precedes the switch's jump table with `.align 3` and
 * the original has it 4-byte aligned at .rodata+0xC4, so the table (and all
 * later .rodata) shifts by 4. Same maspsx limitation as IfCheck and friends. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityBgTriggerInit);
#else

void FieldEntityBgTriggerInit(FieldBgTrigger* triggers) {
    s32 i;

    for (i = 0; i < 12; i++) {
        if (triggers->entityId != 0xFF) {
            switch (triggers->type) {
            case 0:
            case 2:
            case 4:
                FieldEntityBgTriggerActivate(triggers, 1);
                break;
            case 1:
            case 3:
            case 5:
                FieldEntityBgTriggerActivate(triggers, 0);
                break;
            }
        }
        triggers++;
    }
}
#endif

/////////////////////////////////////////////////
// Begin of field_camera.c
/////////////////////////////////////////////////

const u32 D_800A00DC[] = {0x00000000};
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelLoadAndInit);

INCLUDE_ASM("asm/us/field/nonmatchings/field", HandleKawaiDataInModel);

// Possable Debug routine. Ran at beginning of every main field loop. (FPS?)
void DebugRunEveryLoop(void) {}

void FieldCameraAssign(void) {
    if (g_FieldMovieStreamActive == 0 || g_FieldCameraMatrixSel == 1) {
        D_80071E40 = *D_80083578;
    } else {
        D_80071E40 = D_80083270;
    }
}

/* Drive the CD stream that feeds the MDEC. While a field map is still loading
 * the stream is not touched at all; otherwise the chain reader's status decides
 * whether to arm the ring buffer, start playback, or tear it down. */
void FieldUpdateMovieStream(void) {
    u32 status;

    if (g_isFieldLoading == 1) {
        if (SystemCdromReadChain() == 0) {
            g_isFieldLoading = 2;
        }
        return;
    }
    if (D_8009ABF4.eventCmd == EVTCMD_UNK14) {
        func_80035658();
        g_FieldMovieStreamActive = 0;
        g_FieldMoviePlayed = 0;
        D_8009ABF4.movieCommandState = MOVCMD_DONE;
        return;
    }
    status = SystemCdromReadChain();
    switch (status) {
    case 0:
        if (D_8009ABF4.eventCmd == EVTCMD_LOAD_MOVIE &&
            D_8009ABF4.movieCommandState == MOVCMD_IDLE) {
            if (D_80075E10 <= 0x801AFFFF) {
                func_80034FC8(D_80075E10, D_8009ABF4.eventCmdParam);
            } else {
                func_80034FC8(0x801B0000, D_8009ABF4.eventCmdParam);
            }
            D_8009ABF4.movieCommandState = MOVCMD_ACTIVE;
            g_FieldMoviePlayed = 1;
        }
        if ((s16)g_FieldMovieStreamActive == 1) {
            g_FieldMovieStreamDone = 1;
            g_FieldMovieStreamActive = 0;
            g_FieldMoviePlayed = 0;
            D_8009ABF4.movieCommandState = MOVCMD_DONE;
        }
        break;
    case 0xA:
        if (D_8009ABF4.eventCmd == EVTCMD_LOAD_MOVIE) {
            D_8009ABF4.movieCommandState = MOVCMD_DONE;
        }
        if (D_8009ABF4.eventCmd == EVTCMD_PLAY_MOVIE) {
            D_8009ABF4.movieCommandState = MOVCMD_ACTIVE;
            func_800354CC();
            g_FieldMovieStreamActive = 1;
        }
        break;
    }
}

/////////////////////////////////////////////////
// Begin of field_rain.c
/////////////////////////////////////////////////

struct FieldRain {
    /* 0x00 */ SVECTOR p1;
    /* 0x08 */ SVECTOR p2;
    /* 0x10 */ s16 wait;
    /* 0x12 */ s16 rndSeed;
    /* 0x14 */ s16 z;
    /* 0x16 */ s16 render;
};

extern struct FieldRain g_FieldRain[64];
extern u8 g_RainForce;
extern s16 D_800E42EE[0x40][12];

void FieldRainInit(struct FieldRenderData* renderData) {
    LINE_F2* line;
    s32 i;
    s32 adjustedIndex;

    for (i = 0; i < LEN(g_FieldRain); i++) {
        g_FieldRain[i].render = 0;
        g_FieldRain[i].rndSeed = i * 4;
        g_FieldRain[i].wait = i % 8;

        line = &renderData->Rain[i];

        SetLineF2(line);
        SetSemiTrans(line, 1);

        renderData->Rain[i].r0 = 0x10;
        renderData->Rain[i].g0 = 0x10;
        renderData->Rain[i].b0 = 0x10;
    }

    SetDrawMode(&renderData->RainDm, 0, 0, GetTPage(0, 1, 0, 0) & 0xffff, NULL);
}

void FieldRainAddToRender(
    u32* ot, LINE_F2* rain, MATRIX* matrix, DR_MODE* rainDm) {
    long p;
    long flag;
    s32 i;
    s32 j;

    PushMatrix();
    SetRotMatrix(matrix);
    SetTransMatrix(matrix);

    for (i = 0, j = 0; i < LEN(g_FieldRain); i++) {
        // 12 * sizeof(s16) = 24 bytes (0x18), the exact size of FieldRain
        if (D_800E42EE[i][0] == 1) {
            RotTransPers(&g_FieldRain[i].p1, &rain->x0, &p, &flag);
            RotTransPers(&g_FieldRain[i].p2, &rain->x1, &p, &flag);
            AddPrim(ot, rain);
        }
        rain++;
    }

    PopMatrix();

    *(u32*)rainDm = (*(u32*)rainDm & 0xFF000000) | (*ot & 0xFFFFFF);

    *ot = (*ot & 0xFF000000) | ((u32)rainDm & 0xFFFFFF);
}

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldRainUpdate);
#else

extern u8 g_RainControl;
extern s16 g_PlayerModelId;

extern FieldEntity g_FieldEntities[];
extern u8 g_RandomTable[];
extern struct FieldRain g_FieldRain[];

void FieldRainUpdate(void) {
    s32 i;
    s32 limit;
    s32 player;
    s32 max = 255;
    s32 vz;

    if ((g_RainControl & 0x80) == 0) {
        if (g_RainForce != 0) {
            g_RainForce--;
        }
    } else {
        if (g_RainForce != max) {
            g_RainForce++;
        }
    }

    limit = g_RainForce / 4;
    player = g_PlayerModelId;

    for (i = 0; i < 0x40; i++) {
        if (g_FieldRain[i].wait == 0) {
            if (i < limit) {

                u8 seed3;

                g_FieldRain[i].render = 1;
                g_FieldRain[i].rndSeed++;
                g_FieldRain[i].wait = 7;

                g_FieldRain[i].p2.vx =
                    (g_FieldEntities[player].PosX >> 12) +
                    g_RandomTable[g_FieldRain[i].rndSeed & 0xFF] * 12 - 0x600;

                seed3 = g_FieldRain[i].rndSeed * 3;
                g_FieldRain[i].p2.vy = (g_FieldEntities[player].PosY >> 12) +
                                       g_RandomTable[seed3] * 12 - 0x600;

                g_FieldRain[i].p1.vx = g_FieldRain[i].p2.vx;
                g_FieldRain[i].p1.vy = g_FieldRain[i].p2.vy;

                g_FieldRain[i].z = (g_FieldEntities[player].PosZ >> 12) - 0x300;
            } else {
                g_FieldRain[i].wait = 1;
                g_FieldRain[i].render = 0;
            }
        }

        g_FieldRain[i].p2.vz =
            g_FieldRain[i].z + (g_FieldRain[i].wait & 0x7) * 0x80;

        vz = (g_FieldRain[i].wait & 0x7) * 0x80;
        vz += 0x100;

        g_FieldRain[i].p1.vz = g_FieldRain[i].z + vz;

        g_FieldRain[i].wait--;
    }
}
#endif

/////////////////////////////////////////////////
// Begin of field_battle.c
/////////////////////////////////////////////////

u8 FieldGetRandomU8FromList(void) {
    g_FieldRandListIndex++;
    if (g_FieldRandListIndex == 0) {
        g_FieldRandListOffset += 13;
    }
    return g_RandomTable[g_FieldRandListIndex] - g_FieldRandListOffset;
}

u8 FieldGetNextRandomU8(void) {
    D_80071C20++;
    return g_RandomTable[D_80071C20];
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldBattleCheck);

/////////////////////////////////////////////////
// Begin of field_arrow.c
/////////////////////////////////////////////////

void FieldArrowsInit(SPRT_16* sprt, DR_MODE* dm) {
    s16 i;

    for (i = 0; i < 24; i++, sprt++) {
        SetSprt16(sprt);
        SetShadeTex(sprt, 1);
        SetSemiTrans(sprt, 0);
        sprt->r0 = 0x80;
        sprt->g0 = 0x80;
        sprt->b0 = 0x80;
        sprt->clut = GetClut(0x100, 0x1E9);
    }
    SetDrawMode(dm, 0, 1, GetTPage(0, 0, 0x3C0, 0x100), NULL);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldArrowsAddToRender);

/////////////////////////////////////////////////
// Begin of field_model.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", LoadLocalFieldModelAndInitAll);

extern u8* FieldModelCreatePktsForPart(u8* part, u8* pkts, s32 arg2, s32 arg3);
extern void FieldModelScaleModel(FieldModelEntry* model, s16 scale, s32 arg2);

/* Reserves one 32-byte matrix slot per bone at the head of the packet buffer,
 * then emits the drawing packets for every part behind them. */
u8* FieldModelCreatePktsAndScale(FieldModelEntry* model, u8* pkts, s32 arg2) {
    u8* parts;
    u32 i;

    model->partMatrices = pkts;
    pkts += model->boneCount * 32;
    parts = model->modelData + model->partsOffset;
    for (i = 0; i < model->partCount; i++) {
        pkts = FieldModelCreatePktsForPart(&parts[i * 32], pkts, 0, arg2);
    }
    FieldModelScaleModel(model, model->scale, 0);
    return pkts;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelCreatePktsForPart);

/* One texture page inside a BSX model file: where it lives in VRAM and where
 * its pixels sit relative to the start of the file. */
typedef struct {
    /* 0x0 */ u16 w;
    /* 0x2 */ u16 h;
    /* 0x4 */ u16 x;
    /* 0x6 */ u16 y;
    /* 0x8 */ u32 dataOffset;
} BsxTexEntry; // size:0xC

typedef struct {
    /* 0x0 */ u32 unk0;
    /* 0x4 */ u8 texCount;
    /* 0x5 */ u8 pad[3];
    /* 0x8 */ BsxTexEntry entries[1];
} BsxTexHeader;

void FieldModelLoadBsxTexToVram(BsxTexHeader* bsx) {
    RECT rect;
    u32 i;
    u32 count;
    BsxTexEntry* entries;

    count = bsx->texCount;
    entries = bsx->entries;
    for (i = 0; i < count; i++) {
        rect.x = entries[i].x;
        rect.y = entries[i].y;
        rect.w = entries[i].w;
        rect.h = entries[i].h;
        LoadImage(&rect, (u_long*)((u8*)bsx + entries[i].dataOffset));
    }
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelBsxTdbModify);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelStructInit);

extern u_long* D_800DFCA0;
u8* FieldModelLoadBcx(FieldModelData* data, s32 arg1, u8* pkts, s32 index);

/* Loads every global (BCX) model in the header, then optionally kicks off the
 * next streamed read. Scratchpad word 0 is clobbered by each load and restored
 * before the next one; word 1 holds the sector/size pair for that read. */
u8* FieldModelLoadGlobalModels(
    FieldModelData* data, s32 arg1, u8* pkts, s32 readFile) {
    u32* fileInfo;
    s32 saved;
    u32 i;

    saved = ((s32*)0x1F800000)[0];
    fileInfo = (u32*)((s32*)0x1F800000)[1];
    for (i = 0; i < data->unk2; i++) {
        ((s32*)0x1F800000)[0] = saved;
        pkts = FieldModelLoadBcx(data, arg1, pkts, i);
    }
    if (readFile) {
        DS_read(fileInfo[0], fileInfo[1], D_800DFCA0, NULL);
        while (SystemCdromReadChain() != 0) {
        }
    }
    return pkts;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelLoadBcx);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelPrepareRender);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelAddToRender);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelAnimCalcMtrxs);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelScaleModel);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelScalePartVrtxs);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldModelScaleAnimTranslat);

/////////////////////////////////////////////////
// Begin of field_kawai_char_model.c
/////////////////////////////////////////////////

void KawaiClearData(void) {
    u8* p = D_800DFDFC;
    s32 count = 16;
    s32 i;

    p[0] = count;
    for (i = 0; i < count; i++) {
        p[i * 2 + 2] = 0;
        p[i * 2 + 3] = 0;
    }
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiExecute);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetCustomLightToModelPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetVertexColorFromLighting);

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToModelPkts);
#else
/* Every instruction is in the right place; what is left is register naming
 * (the target numbers the parts base after the three colour temps) and an
 * 8-byte larger frame, i.e. the original had 8 bytes of addressable locals
 * this does not. */
s32 KawaiSetColorToModelPkts(FieldModelEntry* model, u8* data) {
    u8* parts;
    u32 count;
    u32 i;
    s32 r;
    s32 g;
    s32 b;

    count = model->partCount;
    parts = model->modelData + model->partsOffset;
    r = data[0] | (data[1] << 8);
    g = data[2] | (data[3] << 8);
    b = data[4] | (data[5] << 8);
    *(u32*)0x1F800200 = data[6];
    for (i = 0; i < count; i++) {
        KawaiSetColorToPartPkts(&parts[i * 32], r, g, b);
    }
    return 1;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToPartPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiLoadEyesMouthTexToVram);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiLightingApplyToModel);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiLightingApplyToPolyColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetModelTransparency);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToPktsBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetColorToPartPktsBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiFadeModelColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetCustomLighting);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiColorFadeBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetLightingToModelPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetLightingToPartPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiSetSplashToPktsBelowLvl);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiInitSplashPkts);

s32 KawaiSetPartAttribute(FieldModelEntry* model, u8* data) {
    u8* parts;
    s32 count;
    s32 i;
    s32 partIdx;

    count = data[0];
    if (count > 0) {
        parts = model->modelData + model->partsOffset;
        for (i = 0; i < count; i++) {
            partIdx = data[i * 2 + 1];
            if (partIdx < model->partCount) {
                parts[partIdx * 32] = data[i * 2 + 2];
            }
        }
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiApplyBoneTransform);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiRenderClippedPart);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiDirectionalColorGradient);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiGradientColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field", KawaiAnimatedPointLight);

/////////////////////////////////////////////////
// Begin of field_event.c
/////////////////////////////////////////////////

extern u8 D_800716D4;
void FieldWindowResetAll(void);
void FieldInitDefaultValues(void);
void FieldEventRunInit(void);

/* Installs the field's state, model and script pointers, checks the script
 * header's version bytes, then brings the event system up. */
void FieldEventInit(
    FieldState* state, FieldEntity* models, FieldScriptHeader* scripts) {
    s32 flags;

    /* The high half of FieldState's 0x68 word. The low half is the
     * controller-1 key bits (see OpcodeFuncKeyEx, which matches against
     * activeKeys as a u32), so this cannot be a named field without splitting
     * that member. Widening to s32 is what makes the load lh rather than lhu:
     * held in an s16 the value is only ever masked, and gcc narrows it. */
    flags = *(s16*)((u8*)state + 0x6A);
    g_FieldState = state;
    g_FieldModels = models;
    g_FieldScripts = scripts;
    D_80095DCC = 0;
    D_8007EBE0 = 1;
    D_8009FE8C = 0;
    if (flags & 0x100) {
        D_80095DCC = 1;
        g_FieldScriptRunState = 4;
    }
    if (scripts->eventDataVersion < 2) {
        SystemError('K', 10);
    }
    if (scripts->eventDataVersion > 2 || scripts->eventVersion > 5) {
        SystemError('K', 12);
    }
    if (scripts->eventVersion < 5) {
        SystemError('K', 11);
    }
    FieldWindowResetAll();
    FieldInitDefaultValues();
    FieldEventRunInit();
    if (D_800716D4 == 0) {
        FieldEventClearAkaoStruct();
        *D_8009A000 = 0xF2;
        SystemAkaoExecute();
    }
}

static void InitFieldDebugPages(void);
void FieldEventUpdate(s32 arg0) {
    if (D_8007EBE0) {
        FieldWindowResetTextAll();
        ResetFieldRenderState();
        FieldDebugInitBuffers();
        InitFieldDebugPages();
        D_80095DCC = 0;
        D_8009FE8C = 0;
        D_8007EBE0 = 0;
        if (g_FieldScripts->eventVersion < 5) {
            SystemError('K', 11);
        }
        if (g_FieldScripts->eventDataVersion < 2) {
            SystemError('K', 10);
        }
        if (g_FieldScripts->eventDataVersion > 2 ||
            g_FieldScripts->eventVersion > 5) {
            SystemError('K', 12);
        }
    }
    if (g_FieldScriptRunState != 4) {
        if (g_FieldScriptRunState != 5 || D_80070788 != 0) {
            FieldEventOpcodeCycle();
        }
    }
    if (g_WindowCount) {
        SystemMenuDrawDialog(
            g_WindowData, 4, arg0, g_FieldState->renderBuffer ^ 1);
    }
    UpdateFieldExitArrows(arg0);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldInitDefaultValues);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventRunInit);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEnablePartyModels);

// Inline as empty string when more is decompiled. Checksum fails now.
const char D_800A013C[8] = {0};

void FieldEventOpcodeCycle(void) {
    s32 i, j, count;
    u16 hours, seconds;
    s32 talkDone = 0;

    // Update display values for play time and countdown.
    hours = Savemap.time / 3600;
    if (hours > 255) {
        hours = 255;
    }
    Savemap.memory_bank_1[16] = hours;
    hours = Savemap.time % 3600;
    Savemap.memory_bank_1[17] = hours / 60;
    seconds = hours % 60;
    if (Savemap.memory_bank_1[18] != seconds) {
        Savemap.memory_bank_1[18] = seconds;
        Savemap.memory_bank_1[19] = 0;
    } else {
        Savemap.memory_bank_1[19]++;
    }

    hours = Savemap.countdown_timer_seconds / 3600;
    if (hours > 255) {
        hours = 255;
    }
    Savemap.memory_bank_1[20] = hours;
    hours = Savemap.countdown_timer_seconds % 3600;
    Savemap.memory_bank_1[21] = hours / 60;
    seconds = hours % 60;
    if (Savemap.memory_bank_1[22] != seconds) {
        Savemap.memory_bank_1[22] = seconds;
        Savemap.memory_bank_1[23] = 30;
    } else if (Savemap.memory_bank_1[23]) {
        Savemap.memory_bank_1[23]--;
    }

    count = g_FieldScripts->numModels;
    for (i = 0; i < count; i++) {
        if (g_FieldModels[i].requestTalkScript) {
            if (!g_FieldState->characterLock && !talkDone) {
                FieldEventRequestRun(g_FieldModels[i].entityId, 1, 1);
                talkDone = 1;
            }
            g_FieldModels[i].requestTalkScript = 0;
        }
        if (g_FieldModels[i].requestPushScript) {
            FieldEventRequestRun(g_FieldModels[i].entityId, 1, 2);
            g_FieldModels[i].requestPushScript = 0;
        }
    }
    for (i = 0; i < g_FieldLineCount; i++) {
        if (g_FieldLines[i].requestTalkScript) {
            if (!g_FieldState->characterLock) {
                FieldEventRequestRun(g_FieldLines[i].entityId, 1, 1);
            }
            g_FieldLines[i].requestTalkScript = 0;
        }
        if (g_FieldLines[i].requestPushScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 2);
            g_FieldLines[i].requestPushScript = 0;
        }
        if (g_FieldLines[i].across) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 3);
            g_FieldLines[i].across = 0;
        }
        if (g_FieldLines[i].requestTouchOnScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 5);
            g_FieldLines[i].requestTouchOnScript = 0;
        }
        if (g_FieldLines[i].requestTouchOffScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 6);
            g_FieldLines[i].requestTouchOffScript = 0;
        }
        if (g_FieldLines[i].touch) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 4);
        }
    }

    // Loop through all entities in field map and execute up to 8 opcodes of
    // each entity's active script.
    count = g_FieldScripts->numEntities;
    do {
        if (g_CurrentEntity >= g_FieldScripts->numEntities) {
            g_CurrentEntity = 0;
        }
        if (g_FieldScriptDebugFlags & 3) {
            DebugUpdateActor(4, g_CurrentEntity);
        }

        // Skip entities involved in a split or join animation
        // (g_EntitySplitJoinState[entity] != 0) except the entity they're
        // splitting from or joining to (g_EntityForSplitJoin).
        if (g_EntitySplitJoinState[g_CurrentEntity] == 0 ||
            g_EntityForSplitJoin == g_CurrentEntity) {
            for (j = 8; j != 0; j--) {
                if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                    (!(g_FieldScriptDebugFlags & 4) ||
                     g_FieldScriptDebugEntities[g_CurrentEntity])) {
                    for (i = 1; i < 9; i++) {
                        SetStrToDebugRow(3, i, D_800A013C);
                    }
                }
                g_FieldCurrentOpcode =
                    ((u8*)g_FieldScripts)[g_FieldScriptPC[g_CurrentEntity]];

                // Script can yield early if opcode returns 1.
                if (g_FieldOpcodes[g_FieldCurrentOpcode]()) {
                    if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                        (!(g_FieldScriptDebugFlags & 4) ||
                         g_FieldScriptDebugEntities[g_CurrentEntity])) {
                        g_CurrentEntity++;
                        goto done;
                    }
                    break;
                }
                if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                    (!(g_FieldScriptDebugFlags & 4) ||
                     g_FieldScriptDebugEntities[g_CurrentEntity])) {
                    if (++D_8009A064 >= 8) {
                        D_8009A064 = 0;
                        g_CurrentEntity++;
                    }
                    goto done;
                }
            }
        }
        g_CurrentEntity++;
        count--;
        if (g_FieldScriptRunState == 5 && g_FieldScriptDebugFlags & 1 &&
            (!(g_FieldScriptDebugFlags & 4) ||
             g_FieldScriptDebugEntities[g_CurrentEntity])) {
            break;
        }
    } while (count != 0);

done:
    if (g_FieldScriptRunState == 5) {
        D_80070788 = 0;
    }
    FieldUpdateAnimationState();
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldUpdateAnimationState);

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventRequestRun);
#else
u8 FieldEventRequestRun(s16 entityId, s16 priority, s16 scriptId) {
    u16 offset;
    s32 scriptOffset;
    s32 entityDataSize;
    s32 extrasHeaderSize;

    if (g_DebugLevel & 3) {
        switch (scriptId) {
        case 1: // Pressed OK.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Talk=");
            break;
        case 2: // Pushed / within entity's collision radius.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Push=");
            break;
        case 3: // Across line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Acrs=");
            break;
        case 4: // Touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Toch=");
            break;
        case 5: // Started touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "TochON =");
            break;
        case 6: // Ended touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "TochOFF=");
            break;
        }
        // Prints entity name.
        FieldDebugStringConcat(
            g_DebugMessageBuffer,
            (char*)g_FieldScripts + sizeof(FieldScriptHeader) + entityId * 8);
        FieldDebugAddParseValueToPage2(g_DebugMessageBuffer, 0, 0);
    }

    // Only request script if active script has lower priority.
    if (g_FieldScriptPriority[entityId] > priority) {

        // Entity is busy waiting for another script to return.
        if (g_FieldScriptSyncState[entityId][priority] != SYNC_NONE) {
            return g_FieldScriptSyncState[entityId][priority];
        }

        scriptOffset = scriptId * 2;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);
        entityDataSize = entityId * 64;
        entityDataSize += g_FieldScripts->numEntities * 8;

        offset = *((u8*)(scriptOffset + entityDataSize + extrasHeaderSize +
                         (s32)g_FieldScripts) +
                   sizeof(FieldScriptHeader));
        offset |=
            *((u8*)(scriptOffset + (entityDataSize + (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        // Empty event scripts consist of just a RET (0x00) opcode.
        if (((u8*)g_FieldScripts)[offset] != 0) {

            // Save position of current active script of lower priority and
            // replace with new script.
            SavedScriptIds[entityId][priority] = scriptId;
            g_SavedFieldScriptPC[entityId][g_FieldScriptPriority[entityId]] =
                g_FieldScriptPC[entityId];
            g_FieldScriptPC[entityId] = offset;
            g_FieldScriptPriority[entityId] = priority;

            // Clear running animation if entity has a model.
            if (g_EntityToModel[entityId] != 0xFF) {
                if (g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode ==
                    SMODE_WALK) {
                    g_FieldModels[g_EntityToModel[entityId]].activeAnimId = 0;
                    g_FieldModels[g_EntityToModel[entityId]].animCurrentFrame =
                        0;
                    g_FieldModels[g_EntityToModel[entityId]].animLastFrame = 0;
                }
                g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode =
                    SMODE_NONE;
            }

            // Reset wait counter.
            g_FieldWaitCounter[entityId] = 0;

            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("=recieved", 0, 0);
            }
        } else {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("=ret", 0, 0);
            }
        }
        return 1;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("=ignored", 0, 0);
    }
    return 0;
}
#endif

void ResetFieldRenderState(void) {
    s16 tpage;

    D_80114490 = 0;
    D_80114464 = 0x7FFF;
    D_80114468 = 0x7FFF;
    setPolyFT4(&D_800E48F4[0]);
    setPolyFT4(&D_800E48F4[1]);
    setSemiTrans(&D_800E48F4[0], 0);
    setSemiTrans(&D_800E48F4[1], 0);
    setShadeTex(&D_800E48F4[0], 1);
    setShadeTex(&D_800E48F4[1], 1);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x2F;
    } else {
        tpage = 0x1F;
    }
    D_800E48F4[1].tpage = tpage;
    D_800E48F4[0].tpage = tpage;
    D_800E48F4[1].clut = 0x7850;
    D_800E48F4[0].clut = 0x7850;
    D_800E48F4[0].r0 = 0;
    D_800E48F4[1].r0 = 0;
    D_800E48F4[0].g0 = 0;
    D_800E48F4[1].g0 = 0;
    D_800E48F4[0].b0 = 0;
    D_800E48F4[1].b0 = 0;
}

/* Unprototyped on purpose: the original passes nothing, but arg0 has to stay
 * live across the call for the cached &g_FieldExitArrowState to land in $a1. */
void DrawFieldExitArrow();

/* Select toggles the exit arrows on and off (bit 0); bit 1 is a debug override
 * that shows them regardless of the toggle and of the movement lock. */
void UpdateFieldExitArrows(s32 arg0) {
    if (g_FieldState->newActiveKeys2 & (1 << 8)) {
        g_FieldExitArrowState[0] ^= 1;
    }
    if (((g_FieldExitArrowState[0] == 1) &&
         (g_FieldState->characterLock == 0)) ||
        (g_FieldExitArrowState[0] & 2)) {
        DrawFieldExitArrow(arg0);
    }
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", DrawFieldExitArrow);

/////////////////////////////////////////////////
// Begin of field_event_debug.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", DebugUpdateActor);

/* Traces one field-script opcode to debug page 3 and/or the on-screen window:
 * the mnemonic first, then one "arg<n>=<byte>" line per operand read straight
 * back out of the script stream. Bit 4 of g_FieldScriptDebugFlags restricts
 * tracing to the entities flagged in g_FieldScriptDebugEntities. */
void DebugPrintOpcode(char* name, u32 numArgs) {
    u32 total;
    u32 i;

    if ((g_FieldScriptDebugFlags & 4) &&
        !g_FieldScriptDebugEntities[g_CurrentEntity]) {
        return;
    }
    FieldDebugStringCopy(g_DebugText, &D_800E0630);
    FieldDebugStringConcat(g_DebugText, name);
    if (g_DebugLevel & 1) {
        SetStrToDebugRow(3, 0, g_DebugText);
    }
    if (g_DebugLevel & 2) {
        DebugPrintToFieldWindow(g_DebugText);
    }
    total = numArgs + 1;
    while (numArgs != 0) {
        i = total - numArgs;
        FieldDebugStringCopy(g_DebugText, "arg");
        FieldDebugStringU8hex(i, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, "=");
        FieldDebugStringU16hex(GET_PARAM_U8(i), g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        if (g_DebugLevel & 1) {
            SetStrToDebugRow(3, i, g_DebugText);
        }
        if (g_DebugLevel & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
        numArgs--;
    }
}

static void FieldDebugAddParseValueToPage2(const char* str, s32 val, s32 kind) {
    if (!(g_FieldScriptDebugFlags & 4) ||
        g_FieldScriptDebugEntities[g_CurrentEntity]) {
        FieldDebugStringCopy(g_DebugText, str);
        switch (kind) {
        case 1:
            FieldDebugStringU8hex(
                val, g_DebugMessageBuffer); // to single hex digit
            break;
        case 2:
            FieldDebugStringU16hex(
                val, g_DebugMessageBuffer); // to double hex digit
            break;
        case 4:
            FieldDebugStringU32hex(
                val, g_DebugMessageBuffer); // to four hex digits
            break;
        default:
            FieldDebugStringCopy(g_DebugMessageBuffer, D_800A0270);
            break;
        }
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        if (g_DebugLevel & 1) {
            AddStrNextDebugRow(2, g_DebugText);
        }
        if (g_DebugLevel & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
    }
}

/////////////////////////////////////////////////
// Begin of field_event_memory_bank.c
/////////////////////////////////////////////////

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventReadMemoryU8);
#else
static u8 FieldEventReadMemoryU8(s16 mb_half, s16 offset) {
    s32 indx;
    u8 value;
    u8 bankId;

    switch (mb_half) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 0:
        value = GET_PARAM_U8(offset);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G cons=", value, 2);
        }
        return value;
    case 1:
    case 2:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 3:
    case 4:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 11:
    case 12:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 13:
    case 14:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 15:
    case 7:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 5:
    case 6:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 2);
        }
        return value;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
    return 0;
}
#endif

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventWriteMemoryU8);
#else
static void FieldEventWriteMemoryU8(s16 arg0, s16 arg1, u8 value) {
    u8 bankId;
    s32 indx;

    switch (arg0) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 1:
    case 2:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 3:
    case 4:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 11:
    case 12:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 13:
    case 14:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 15:
    case 7:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 5:
    case 6:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 2);
        }
        return;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
}
#endif

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventReadMemoryS16);
#else
static s16 FieldEventReadMemoryS16(s16 bank_id, s16 offset) {
    u8 bankId;
    s32 indx;
    s16 value;

    switch (bank_id) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 0:
        GET_PARAM_S16(value, offset);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G cons=", value, 4);
        }
        return value;
    case 1:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 2:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 3:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 4:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 11:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 12:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 13:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 14:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 15:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 7:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 5:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 4);
        }
        return value;
    case 6:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        value |= g_FieldMapVars[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 4);
        }
        return value;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
    return 0;
}
#endif

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventWriteMemoryS16);
#else
static void FieldEventWriteMemoryS16(s16 arg0, s16 arg1, s16 value) {
    u8 bankId;
    s32 indx;

    switch (arg0) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 1:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 2:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 3:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 4:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 11:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 12:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 13:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 14:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 15:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 7:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 5:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 2);
        }
        return;
    case 6:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = (u8)value;
        g_FieldMapVars[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 4);
        }
        return;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
}
#endif

//////////////////////////////////////////////////
// Start of field_opcode_system.c
/////////////////////////////////////////////////

// This is called when there the script tries to execute an invalid opcode
// called for opcodes:
// 0C 0D 1A 1B 1C 1D 1E 1F 44 46 4C 4E BE
s32 OpcodeFuncBad(void) {
    if (g_DebugLevel & 3) {
        FieldDebugStringU16hex(g_FieldCurrentOpcode, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugMessageBuffer, "???");
        DebugPrintOpcode(g_DebugMessageBuffer, 8);
        FieldDebugPageSetColor(3, 0x7F, 0, 0);
    } else {
        FieldEventDebugError("Bad Event code!");
    }
    return 1;
}

/**
 @brief Opcode 0x5F - **WAIT1* - Wait 1 frame

 Memory layout:

 | 0x5F |
 @details
 Waits one frame and returns 1
 @note
 This does not emit a debug message.
 */
s32 OpcodeFuncWait1(void) {
    PC_INC(1);
    return 1;
}

/**
 * @brief Opcode 0x24 - **WAIT** - Wait
 *
 * Memory layout:
 *
 * | 0x24 | A |
 *
 * - const UShort A: Amount (number of frames) to wait.
 * @details
 * g_FieldWaitCounter[g_CurrentEntity] == 0 by default. The opcode then
 * sets it to how many frames to wait before returning 1, which halts
 * execution of the script until next frame.
 *
 * If parameter == 0, the opcode behaves the same way as NOP.
 *
 * The opcode is then called once per frame, decrementing the counter until it
 * reaches 1, at which point it's set to 0 and 0 is returned, which
 * tells the script parser to continue executing next opcode.
 */

s32 OpcodeFuncWait(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wait", 2);
    }

    if (g_FieldWaitCounter[g_CurrentEntity] == 0) {
        GET_PARAM_S16(g_FieldWaitCounter[g_CurrentEntity], 1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2(
                "wait_st=", g_FieldWaitCounter[g_CurrentEntity], 4);
        }
        if (g_FieldWaitCounter[g_CurrentEntity] == 0) {
            PC_INC(3);
            return 1;
        }
        return 1;
    }

    if (g_FieldWaitCounter[g_CurrentEntity] == 1) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("wait_end=", 1, 4);
        }
        g_FieldWaitCounter[g_CurrentEntity] = 0;
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "wait=", g_FieldWaitCounter[g_CurrentEntity], 4);
    }

    g_FieldWaitCounter[g_CurrentEntity]--;
    return 1;
}

//////////////////////////////////////////////////
// Start of field_opcode_vars.c
/////////////////////////////////////////////////

s32 OpcodeFuncSet(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("set", 3);
    }
    FieldEventWriteMemoryU8(1, 2, FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncSet2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("set2", 4);
    }
    FieldEventWriteMemoryS16(1, 2, FieldEventReadMemoryS16(2, 3));
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncLbyte(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lbyte", 3);
    }
    FieldEventWriteMemoryU8(1, 2, FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncHbyte(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("hbyte", 4);
    }
    FieldEventWriteMemoryU8(1, 2, (u8)(FieldEventReadMemoryS16(2, 3) >> 8));
    PC_INC(5);
    return 0;
}

s32 OpcodeFunc2byte(void) {
    s16 lhs;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("2byte", 5);
    }
    lhs = FieldEventReadMemoryU8(2, 4);
    FieldEventWriteMemoryS16(1, 3, lhs | (FieldEventReadMemoryU8(4, 5) << 8));
    PC_INC(6);
    return 0;
}

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncSetx);
#else
s32 OpcodeFuncSetx(void) {
    s16 offset;
    u8 bank;
    u8 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("setx", 6);
    }
    bank = GET_PARAM_U8(1) >> 4;
    offset = GET_PARAM_U8(3) + FieldEventReadMemoryS16(2, 3);
    value = FieldEventReadMemoryU8(4, 5);
    switch (bank) {
    case 15:
        offset += 256;
    case 13:
        offset += 256;
    case 11:
        offset += 256;
    case 3:
        offset += 256;
    case 1:
        if (offset >= 1280) {
            offset = 1279;
        }
        Savemap.memory_bank_1[offset] = value;
        break;
    case 5:
        if (offset >= 256) {
            offset = 255;
        }
        g_FieldMapVars[offset] = value;
        break;
    }
    PC_INC(7);
    return 0;
}
#endif

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncGetx);
#else
s32 OpcodeFuncGetx(void) {
    s16 offset;
    u8 bank;
    u8 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("getx", 6);
    }
    bank = GET_PARAM_U8(1) >> 4;
    offset = GET_PARAM_U8(3) + FieldEventReadMemoryS16(2, 3);
    switch (bank) {
    case 15:
        offset += 256;
    case 13:
        offset += 256;
    case 11:
        offset += 256;
    case 3:
        offset += 256;
    case 1:
        if (offset >= 1280) {
            offset = 1279;
        }
        value = Savemap.memory_bank_1[offset];
        break;
    case 5:
        if (offset >= 256) {
            offset = 255;
        }
        value = g_FieldMapVars[offset];
        break;
    }

    FieldEventWriteMemoryU8(4, 5, value);
    PC_INC(7);
    return 0;
}
#endif

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncSrchx);
#else
s32 OpcodeFuncSrchx(void) {
    s16 end;
    s16 start;
    s16 where;
    u8 bank;
    u8 value;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("srchx", 8);
    }
    bank = GET_PARAM_U8(1) >> 4;
    start = GET_PARAM_U8(4) + FieldEventReadMemoryS16(2, 5);
    end = GET_PARAM_U8(4) + FieldEventReadMemoryS16(3, 7);
    value = FieldEventReadMemoryU8(4, 9);
    switch (bank) {
    case 15:
        start += 256;
        end += 256;
    case 13:
        start += 256;
        end += 256;
    case 11:
        start += 256;
        end += 256;
    case 3:
        start += 256;
        end += 256;
    case 1:
        if (start >= 1280) {
            start = 1279;
        }
        if (end >= 1280) {
            end = 1279;
        }
        for (i = start; i <= end; i++) {
            if (Savemap.memory_bank_1[i] == value) {
                FieldEventWriteMemoryS16(6, 10, i);
                PC_INC(11);
                return 0;
            }
        }
        break;
    case 5:
        if (start >= 256) {
            start = 255;
        }
        if (end >= 256) {
            end = 255;
        }
        for (i = start; i <= end; i++) {
            if (g_FieldMapVars[i] == value) {
                FieldEventWriteMemoryS16(6, 10, i);
                PC_INC(11);
                return 0;
            }
        }
        break;
    }
    FieldEventWriteMemoryS16(6, 10, -1);
    PC_INC(11);
    return 0;
}
#endif

s32 OpcodeFuncBiton(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("biton", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) | (1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBitof(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bitof", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) & ~(1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBitxr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bitxr", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) ^ (1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_line.c
/////////////////////////////////////////////////

s32 OpcodeFuncLine(void) {
    s16 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("line", 8);
    }

    if (g_FieldLineCount >= 32) {
        FieldEventDebugError("many lineobj!");
        PC_INC(13);
        return 0;
    }

    g_EntityToLine[g_CurrentEntity] = g_FieldLineCount;
    GET_PARAM_S16(value, 1);
    g_FieldLines[g_FieldLineCount].pos.x1 = value;
    GET_PARAM_S16(value, 3);
    g_FieldLines[g_FieldLineCount].pos.y1 = value;
    GET_PARAM_S16(value, 5);
    g_FieldLines[g_FieldLineCount].pos.z1 = value;
    GET_PARAM_S16(value, 7);
    g_FieldLines[g_FieldLineCount].pos.x2 = value;
    GET_PARAM_S16(value, 9);
    g_FieldLines[g_FieldLineCount].pos.y2 = value;
    GET_PARAM_S16(value, 11);
    g_FieldLines[g_FieldLineCount].pos.z2 = value;
    g_FieldLines[g_FieldLineCount].isActive = 1;
    g_FieldLines[g_FieldLineCount].entityId = g_CurrentEntity;
    g_FieldLineCount++;
    PC_INC(13);
    return 0;
}

s32 OpcodeFuncSline(void) {
    u8 lineId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sline", 8);
    }
    lineId = g_EntityToLine[g_CurrentEntity];
    g_FieldLines[lineId].pos.x1 = FieldEventReadMemoryS16(1, 4);
    g_FieldLines[lineId].pos.y1 = FieldEventReadMemoryS16(2, 6);
    g_FieldLines[lineId].pos.z1 = FieldEventReadMemoryS16(3, 8);
    g_FieldLines[lineId].pos.x2 = FieldEventReadMemoryS16(4, 10);
    g_FieldLines[lineId].pos.y2 = FieldEventReadMemoryS16(5, 12);
    g_FieldLines[lineId].pos.z2 = FieldEventReadMemoryS16(6, 14);
    PC_INC(16);
    return 0;
}

s32 OpcodeFuncLinon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("linon", 1);
    }
    g_FieldLines[g_EntityToLine[g_CurrentEntity]].isActive = GET_PARAM_U8(1);
    if (GET_PARAM_U8(1) == 0) {
        g_FieldLines[g_EntityToLine[g_CurrentEntity]].touch = 0;
    }
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode SLIP: Enables or disables slipping along a line
 *
 * Slipping allows the player to slide along a wall when running
 * against it instead of stopping. The wall must previously have a
 * line defined alongside it with opcode LINE.
 */

s32 OpcodeFuncSlip(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("slip", 1);
    }
    g_FieldLines[g_EntityToLine[g_CurrentEntity]].slipDisabled =
        GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_if.c
/////////////////////////////////////////////////

/*
 * Field-script opcode IF: If, 1 byte, unsigned
 *
 * Compares two u8 using a given logical operator.
 * Jumps given number of bytes ahead if the comparison is false.
 */

s32 OpcodeFuncIf(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if", 5);
    }
    if (IfCheck()) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if=true", 0, 0);
        }
        // If comparison is true, continue executing next opcode.
        PC_INC(6);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if=false", 0, 0);
        }
        // If comparison is false, jump number of bytes give in last parameter
        // from last parameter.
        PC_INC(GET_PARAM_U8(5) + 5);
    }
    return 0;
}

/*
 * Field-script opcode LIF: Long If, 1 byte, unsigned
 *
 * Compares two u8 using a given logical operator.
 * Identical to IF except that the jump parameter is s16, allowing for longer
 * jumps.
 */

s32 OpcodeFuncLif(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif", 6);
    }
    if (IfCheck()) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif=true", 0, 0);
        }
        PC_INC(7);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif=false", 0, 0);
        }
        GET_PARAM_S16(param, 5);
        PC_INC(param + 5);
    }
    return 0;
}

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", IfCheck);
#else
u32 IfCheck(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(4);
    switch (ope) {
    case IF_EQ:
        result = FieldEventReadMemoryU8(1, 2) == FieldEventReadMemoryU8(2, 3);
        break;
    case IF_NOT_EQ:
        result = FieldEventReadMemoryU8(1, 2) != FieldEventReadMemoryU8(2, 3);
        break;
    case IF_GT:
        result = FieldEventReadMemoryU8(1, 2) > FieldEventReadMemoryU8(2, 3);
        break;
    case IF_LT:
        result = FieldEventReadMemoryU8(1, 2) < FieldEventReadMemoryU8(2, 3);
        break;
    case IF_GTE:
        result = FieldEventReadMemoryU8(1, 2) >= FieldEventReadMemoryU8(2, 3);
        break;
    case IF_LTE:
        result = FieldEventReadMemoryU8(1, 2) <= FieldEventReadMemoryU8(2, 3);
        break;
    case IF_AND:
        result = FieldEventReadMemoryU8(1, 2) & FieldEventReadMemoryU8(2, 3);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryU8(1, 2) ^ FieldEventReadMemoryU8(2, 3);
        break;
    case IF_OR:
        result = FieldEventReadMemoryU8(1, 2) | FieldEventReadMemoryU8(2, 3);
        break;
    case IF_BIT:
        result =
            FieldEventReadMemoryU8(1, 2) & (1 << FieldEventReadMemoryU8(2, 3));
        break;
    case IF_NOT_BIT:
        result =
            FieldEventReadMemoryU8(1, 2) & (1 << FieldEventReadMemoryU8(2, 3));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}
#endif

/*
 * Field-script opcode IF2: If, 2 bytes, signed
 *
 * Compares two s16 using a given logical operator.
 */
s32 OpcodeFuncIf2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if2", 7);
    }
    if (If2CheckSigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=true", 0, 0);
        }
        PC_INC(8);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(7) + 7);
    }
    return 0;
}

/*
 * Field-script opcode LIF2: Long if, 2 bytes, signed
 *
 * Compares two s16 using a given logical operator.
 */
s32 OpcodeFuncLif2(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif2", 8);
    }
    if (If2CheckSigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=true", 0, 0);
        }
        PC_INC(9);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=false", 0, 0);
        }
        GET_PARAM_S16(param, 7);
        PC_INC(param + 7);
    }
    return 0;
}

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", If2CheckSigned);
#else
u32 If2CheckSigned(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(6);
    switch (ope) {
    case IF_EQ:
        result = FieldEventReadMemoryS16(1, 2) == FieldEventReadMemoryS16(2, 4);
        break;
    case IF_NOT_EQ:
        result = FieldEventReadMemoryS16(1, 2) != FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GT:
        result = FieldEventReadMemoryS16(1, 2) > FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LT:
        result = FieldEventReadMemoryS16(1, 2) < FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GTE:
        result = FieldEventReadMemoryS16(1, 2) >= FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LTE:
        result = FieldEventReadMemoryS16(1, 2) <= FieldEventReadMemoryS16(2, 4);
        break;
    case IF_AND:
        result = FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 4);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 4);
        break;
    case IF_OR:
        result = FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 4);
        break;
    case IF_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        break;
    case IF_NOT_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}
#endif

/*
 * Field-script opcode IF2U: If, 2 bytes, unsigned
 *
 * Compares two u16 using a given logical operator.
 */
s32 OpcodeFuncIf2u(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if2", 7);
    }
    if (If2CheckUnsigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=true", 0, 0);
        }
        PC_INC(8);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(7) + 7);
    }
    return 0;
}

/*
 * Field-script opcode LIF2U: Long if, 2 bytes, unsigned
 *
 * Compares two u16 using a given logical operator.
 */
s32 OpcodeFuncLif2u(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif2", 8);
    }
    if (If2CheckUnsigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=true", 0, 0);
        }
        PC_INC(9);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=false", 0, 0);
        }
        GET_PARAM_S16(param, 7);
        PC_INC(param + 7);
    }
    return 0;
}

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", If2CheckUnsigned);
#else
u32 If2CheckUnsigned(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(6);
    switch (ope) {
    case IF_EQ:
        result = (u16)FieldEventReadMemoryS16(1, 2) ==
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_NOT_EQ:
        result = (u16)FieldEventReadMemoryS16(1, 2) !=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GT:
        result = (u16)FieldEventReadMemoryS16(1, 2) >
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LT:
        result = (u16)FieldEventReadMemoryS16(1, 2) <
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GTE:
        result = (u16)FieldEventReadMemoryS16(1, 2) >=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LTE:
        result = (u16)FieldEventReadMemoryS16(1, 2) <=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_AND:
        result = FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 4);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 4);
        break;
    case IF_OR:
        result = FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 4);
        break;
    case IF_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        break;
    case IF_NOT_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}
#endif

//////////////////////////////////////////////////
// Start of field_opcode_controller.c
/////////////////////////////////////////////////

/*
 * Field-script opcode KEY!: Key check
 *
 * Jumps ahead given number of bytes if given key(s) are not active.
 * All key opcodes only check the lower half word which contains the keys
 * for controller 1.
 */

s32 OpcodeFuncKeyEx(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("key!", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->activeKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->activeKeys);
    }
}

/*
 * Field-script opcode KEYON: Key On
 *
 * Checks keys that player pressed this frame.
 */

s32 OpcodeFuncKeyon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("keyon", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->newActiveKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->newActiveKeys);
    }
}

/*
 * Field-script opcode KEYOF: Key Off
 *
 * Checks keys that player released this frame.
 */

s32 OpcodeFuncKeyof(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("keyof", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->newInactiveKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->newInactiveKeys);
    }
}

static s32 KeyCheck(u16 keys) {
    u16 param;

    GET_PARAM_S16(param, 1);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("key now=", keys, 4);
        FieldDebugAddParseValueToPage2("key chk=", param, 4);
    }
    if (keys & param) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("key=true", 0, 0);
        }
        PC_INC(4);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("key=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(3) + 3);
    }
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_request.c
/////////////////////////////////////////////////

s32 OpcodeFuncReq(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("req", 2);
    }
    return FieldEventRequest(1, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncReqsw(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("reqsw", 2);
    }
    return FieldEventRequest(2, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncReqew(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("reqew", 2);
    }
    return FieldEventRequest(3, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPreq(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("preq", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(1, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPrqsw(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prqsw", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(2, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPrqew(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prqew", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(3, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

// Depends on decomp of DebugUpdateActor due to shared string.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventRequest);
#else
s32 FieldEventRequest(s16 type, u8 target, u8 priority, u8 scriptId) {
    s32 scriptOffset;
    s32 entityDataSize;
    s32 extrasHeaderSize;

    if (target == 0xFF) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rqew=no one", 0, 0);
        }
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugStringCopy(g_DebugMessageBuffer, "rq=");
        FieldDebugStringConcat(
            g_DebugMessageBuffer, (char*)((s32)g_FieldScripts) +
                                      sizeof(FieldScriptHeader) + (target * 8));
        FieldDebugStringConcat(g_DebugMessageBuffer, "/");
        FieldDebugAddParseValueToPage2(g_DebugMessageBuffer, scriptId, 2);
    }

    if (type > 0) {
        if (type >= 3) {
            if (type == 3 && g_FieldScriptSyncWaitEntity[target][priority] ==
                                 g_CurrentEntity) {
                switch (g_FieldScriptSyncState[target][priority]) {
                case SYNC_WAITING:
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("rqew=wait", 0, 0);
                    }
                    return 1;
                case SYNC_DONE:
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("rqew=end", 0, 0);
                    }
                    g_FieldScriptSyncState[target][priority] = SYNC_NONE;
                    g_FieldScriptSyncWaitEntity[target][priority] = 0xFF;
                    PC_INC(3);
                    return 0;
                }
            }
        }
    }

    if (g_FieldScriptPriority[target] == priority) {
        switch (type) {
        case 1:
            PC_INC(3);
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("rq=skip", 0, 0);
            }
            return 0;
        case 2:
        case 3:
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("rqw=busy", 0, 0);
            }
        }
        return 1;
    } else if (g_FieldScriptPriority[target] < priority) {
        if (g_SavedFieldScriptPC[target][priority] != 0) {
            switch (type) {
            case 1:
                PC_INC(3);
                if (g_DebugLevel & 3) {
                    FieldDebugAddParseValueToPage2("rq=skip", 0, 0);
                }
                return 0;
            case 2:
            case 3:
                if (g_DebugLevel & 3) {
                    FieldDebugAddParseValueToPage2("rqw=busy", 0, 0);
                }
            }
            return 1;
        }
        scriptOffset = scriptId * 2;
        entityDataSize = target * 64;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

        g_SavedFieldScriptPC[target][priority] =
            *((u8*)(scriptOffset +
                    (entityDataSize + (g_FieldScripts->numEntities << 3)) +
                    extrasHeaderSize + (s32)g_FieldScripts) +
              sizeof(FieldScriptHeader));
        g_SavedFieldScriptPC[target][priority] |=
            *((u8*)(scriptOffset +
                    ((entityDataSize + (g_FieldScripts->numEntities << 3)) +
                     (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rq=send", 0, 0);
        }

        if (type <= 0) {
            return 1;
        }

        if (type >= 3) {
            if (type != 3) {
                return 1;
            }
        } else {
            PC_INC(3);
            return 0;
        }

        g_FieldScriptSyncWaitEntity[target][priority] = g_CurrentEntity;
        g_FieldScriptSyncState[target][priority] = SYNC_WAITING;
        return 1;
    } else if (g_FieldScriptSyncState[target][priority] == SYNC_NONE) {
        s32 scriptOffset;
        s32 entityDataSize;
        s32 extrasHeaderSize;

        SavedScriptIds[target][priority] = scriptId;
        g_SavedFieldScriptPC[target][g_FieldScriptPriority[target]] =
            g_FieldScriptPC[target];

        scriptOffset = scriptId * 2;
        entityDataSize = target * 64;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

        g_FieldScriptPC[target] =
            *((u8*)(scriptOffset +
                    (entityDataSize + (g_FieldScripts->numEntities << 3)) +
                    extrasHeaderSize + (s32)g_FieldScripts) +
              sizeof(FieldScriptHeader));
        g_FieldScriptPC[target] |=
            *((u8*)(scriptOffset +
                    ((entityDataSize + (g_FieldScripts->numEntities << 3)) +
                     (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        g_FieldScriptPriority[target] = priority;

        if (g_EntityToModel[target] != 0xFF) {
            g_FieldModels[g_EntityToModel[target]].scriptedMoveMode =
                SMODE_NONE;
        }
        g_FieldWaitCounter[target] = 0;

        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rq=send", 0, 0);
        }

        if (type <= 0) {
            return 1;
        }

        if (type >= 3) {
            if (type != 3) {
                return 1;
            }
        } else {
            PC_INC(3);
            return 0;
        }

        g_FieldScriptSyncWaitEntity[target][priority] = g_CurrentEntity;
        g_FieldScriptSyncState[target][priority] = SYNC_WAITING;
        return 1;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("rqw=busy*", 0, 0);
    }
    return 1;
}
#endif

s32 OpcodeFuncRet(void) {
    u16* fieldScriptPC;
    u16(*savedPC)[8];
    u16* savedRow;
    u16 scriptPc;
    u32 entity;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ret", 0);
    }
    if (g_FieldScriptPriority[g_CurrentEntity] >= 7) {
        return 1;
    }

    if (g_FieldScriptSyncState[g_CurrentEntity]
                              [g_FieldScriptPriority[g_CurrentEntity]] ==
        SYNC_WAITING) {
        g_FieldScriptSyncState[g_CurrentEntity]
                              [g_FieldScriptPriority[g_CurrentEntity]] =
                                  SYNC_DONE;
    }

    g_FieldScriptPriority[g_CurrentEntity]++;

    entity = g_CurrentEntity;
    savedPC = g_SavedFieldScriptPC;
    fieldScriptPC = g_FieldScriptPC;

    savedRow = savedPC[entity];
    scriptPc =
        *(u16*)((g_FieldScriptPriority[entity] * sizeof(u16)) + (s32)savedRow);
    fieldScriptPC[entity] = scriptPc;

    while (scriptPc == 0 && g_FieldScriptPriority[entity] < 7) {
        u16* activePcSlot;
        u16* loopSavedRow;
        u16 nextPc;

        g_FieldScriptPriority[g_CurrentEntity]++;
        entity = g_CurrentEntity;

        activePcSlot =
            (u16*)((entity * sizeof(*fieldScriptPC)) + (s32)fieldScriptPC);

        loopSavedRow = (u16*)((entity * sizeof(*savedPC)) + (s32)savedPC);

        nextPc = *(u16*)((g_FieldScriptPriority[entity] * sizeof(u16)) +
                         (s32)loopSavedRow);

        *activePcSlot = nextPc;
        scriptPc = nextPc;
    }

    g_SavedFieldScriptPC[g_CurrentEntity]
                        [g_FieldScriptPriority[g_CurrentEntity]] = 0;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "ret=", g_FieldScriptPriority[g_CurrentEntity], 2);
    }
    return 0;
}

s32 OpcodeFuncRetto(void) {
    s16 scriptId;
    u8 priority;
    s32 extrasHeaderSize;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("retto", 1);
    }

    priority = GET_PRIORITY(GET_PARAM_U8(1));
    scriptId = GET_SCRIPTID(GET_PARAM_U8(1));

    while (g_FieldScriptPriority[g_CurrentEntity] < (priority - 1) &&
           g_FieldScriptPriority[g_CurrentEntity] < 7) {
        if (g_FieldScriptSyncState[g_CurrentEntity]
                                  [g_FieldScriptPriority[g_CurrentEntity]] ==
            SYNC_WAITING) {
            g_FieldScriptSyncState[g_CurrentEntity]
                                  [g_FieldScriptPriority[g_CurrentEntity]] =
                                      SYNC_DONE;
        }
        g_FieldScriptPriority[g_CurrentEntity]++;
        g_SavedFieldScriptPC[g_CurrentEntity]
                            [g_FieldScriptPriority[g_CurrentEntity]] = 0;
    }
    SavedScriptIds[g_CurrentEntity][priority] = scriptId;
    scriptId *= 2;
    extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

    g_FieldScriptPC[g_CurrentEntity] =
        *((u8*)(scriptId +
                ((g_FieldScripts->numEntities * 8) + (g_CurrentEntity * 64)) +
                extrasHeaderSize + (s32)g_FieldScripts) +
          sizeof(FieldScriptHeader));
    g_FieldScriptPC[g_CurrentEntity] |=
        *((u8*)(scriptId +
                (((g_FieldScripts->numEntities * 8) + (g_CurrentEntity * 64)) +
                 (s32)g_FieldScripts) +
                extrasHeaderSize) +
          sizeof(FieldScriptHeader) + 1)
        << 8;

    g_FieldScriptPriority[g_CurrentEntity] = priority;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "ret=", g_FieldScriptPriority[g_CurrentEntity], 2);
    }
    return 0;
}

s32 OpcodeFuncBack(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("back", 1);
    }
    PC_DEC(GET_PARAM_U8(1));
    return 1;
}

s32 OpcodeFuncLback(void) {
    u16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lback", 2);
    }
    GET_PARAM_S16(param, 1);
    PC_DEC(param);
    return 1;
}

s32 OpcodeFuncSkip(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("skip", 1);
    }
    PC_INC(GET_PARAM_U8(1) + 1);
    return 0;
}

s32 OpcodeFuncLskip(void) {
    u16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lskip", 2);
    }
    GET_PARAM_S16(param, 1);
    PC_INC(param + 1);
    return 0;
}

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMjump);
#else
s32 OpcodeFuncMjump(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mjump", 8);
    }

    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_FIELD_MAP_CHANGE;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        GET_PARAM_S16(g_FieldState->eventCmdParam, 1);
        GET_PARAM_S16(g_FieldState->pcPosX, 3);
        GET_PARAM_S16(g_FieldState->pcPosY, 5);
        GET_PARAM_S16(g_FieldState->pcWalkMeshId, 7);
        g_FieldState->pcDirection = GET_PARAM_U8(9);
        return 1;
    case EVTCMD_FIELD_MAP_CHANGE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(10);
            g_FieldState->eventCmd = EVTCMD_NONE;
            return 0;
        }
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("evt cmd=", g_FieldState->eventCmd, 2);
    }
    return 1;
}
#endif

s32 OpcodeFuncPmjmp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmjmp", 8);
    }
    GET_PARAM_S16(g_FieldPreloadMapId, 1);
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncPmjmp2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmjmp", 8);
    }
    if (g_isFieldLoading != 2) {
        return 1;
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncMgame(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mgame", 8);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_LOAD_MINIGAME;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        GET_PARAM_S16(g_FieldState->eventCmdParam, 1);
        GET_PARAM_S16(g_FieldState->pcPosX, 3);
        GET_PARAM_S16(g_FieldState->pcPosY, 5);
        GET_PARAM_S16(g_FieldState->pcWalkMeshId, 7);
        *(s16*)&g_FieldState->pcDirection = GET_PARAM_U8(9);
        *(u8*)((u8*)g_FieldState + 0xF2) = GET_PARAM_U8(10);
        return 1;
    case EVTCMD_LOAD_MINIGAME:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(11);
            g_FieldState->eventCmd = EVTCMD_NONE;
            return 0;
        }
        return 1;
    }
    return 1;
}

s32 OpcodeFuncBatle(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("batle", 3);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        FieldWindowResetTextAll();
        g_FieldState->eventCmd = EVTCMD_ENTERING_BATTLE;
        g_FieldState->eventCmdParam = FieldEventReadMemoryS16(2, 2);
        D_8007EBE0 = 1;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        return 1;
    case EVTCMD_ENTERING_BATTLE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(4);
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            return 0;
        }
        break;
    }
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_akao_sound.c
/////////////////////////////////////////////////

void FieldEventClearAkaoStruct(void) {
    s32 i;
    s16* p;

    D_8009A000[0] = 0;
    for (i = 5, p = &D_8009A000[10]; i >= 0; i--) {
        *(s32*)(p + 2) = 0;
        p -= 2;
    }
}

s32 OpcodeFuncAkao(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("akao", 3);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(4);
    *D_8009A004 = FieldEventReadMemoryU8(1, 5);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(2, 6);
    D_8009A00C = (s16)FieldEventReadMemoryS16(3, 8);
    D_8009A010 = (s16)FieldEventReadMemoryS16(4, 10);
    D_8009A014 = (s16)FieldEventReadMemoryS16(6, 12);
    SystemAkaoExecute();
    PC_INC(14);
    return 0;
}

s32 OpcodeFuncAkao2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("akao2", 3);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(4);
    *D_8009A004 = (s16)FieldEventReadMemoryS16(1, 5);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(2, 7);
    D_8009A00C = (s16)FieldEventReadMemoryS16(3, 9);
    D_8009A010 = (s16)FieldEventReadMemoryS16(4, 11);
    D_8009A014 = (s16)FieldEventReadMemoryS16(6, 13);
    SystemAkaoExecute();
    PC_INC(15);
    return 0;
}

s32 OpcodeFuncSe(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("se", 3);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x20;
    D_8009A004[0] = FieldEventReadMemoryU8(2, 4);
    D_8009A008[0] = (s16)FieldEventReadMemoryS16(1, 2);
    SystemAkaoExecute();
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncMusic(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("music", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x10;
    return SetAndApplyAkao();
}

s32 OpcodeFuncMusvt(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("musvt", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x14;
    return SetAndApplyAkao();
}

s32 OpcodeFuncMusvm(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("musvm", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x15;
    return SetAndApplyAkao();
}

s32 OpcodeFuncCmusc(void) {
    u32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cmusc", 5);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(3);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(3, 4);
    D_8009A00C = (s16)FieldEventReadMemoryS16(4, 6);
    result = SetAndApplyAkao();
    PC_INC(6);
    return result;
}

s32 SetAndApplyAkao(void) {
    // Indexes into AKAO block of field file which contains the list of music
    // tracks available for current field.
    u8 akaoId;

    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("music=", akaoId, 2);
        }
        *D_8009A004 = (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
        g_FieldState->nextFieldMusic = *D_8009A004;
        SystemAkaoExecute();
    }
    PC_INC(2);
    return 0;
}

static u32 GetAkaoBlockOffset(s16 akaoId) {
    s32 akaoData;
    u32 akaoOffset;

    akaoData =
        akaoId * 4 + g_FieldScripts->numEntities * 8 + (s32)g_FieldScripts;
    akaoOffset = ((u8*)akaoData)[sizeof(FieldScriptHeader)];
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 1] << 8;
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 2] << 16;
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 3] << 24;
    return akaoOffset;
}

s32 OpcodeFuncBmusc(void) {
    u8 akaoId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bmusc", 1);
    }
    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("bmusic=", akaoId, 2);
        }
        g_FieldState->nextBattleMusic =
            (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
    } else {
        g_FieldState->nextBattleMusic = 0;
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncFmusc(void) {
    u8 akaoId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fmusc", 1);
    }
    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("bmusic=", akaoId, 2);
        }
        g_FieldState->nextFieldMusic =
            (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
    } else {
        g_FieldState->nextFieldMusic = 0;
    }
    PC_INC(2);
    return 0;
}

// In Akao because it uses the AKAO block area
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncTutor);

/////////////////////////////////////////////////
// Start of field_opcode_movie_overlay.c
/////////////////////////////////////////////////

/*
 * Field-script opcode MULCK (0xF5): set the music lock from the opcode operand.
 *
 * While g_FieldMusicLock is nonzero the MUSIC/FMUSC opcodes skip handing the
 * song to the sound engine, so field music stops responding until a later
 * MULCK 0 (or a reset) clears it again.
 *
 * The operand is read straight out of the running script:
 *   g_FieldScripts          - the current map's script bytecode
 *   g_FieldScriptPC[entity]  - that entity's program counter (byte offset into
 * it) g_CurrentEntity          - the entity whose script is currently executing
 * so g_FieldScripts[pc + 1] is the 1-byte operand. The program counter is then
 * stepped past the 2-byte instruction (opcode + operand).
 */
s32 OpcodeFuncMulck(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mulck", 1);
    }
    g_FieldMusicLock = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncBgmovie(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgmovie", 1);
    }
    g_FieldState->backgroundMovieEnabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncScrlo(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlo", 1);
    }
    g_FieldState->scrloSet = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode DSKCG: request a disc change.
 *
 * Runs as a small state machine on the field main-loop step (opcode):
 * on first execution it stores the requested disc number and switches the
 * field loop into the disc-change step (13), then keeps returning 1
 * (opcode not finished) until the loop reports the swap is done
 * (movieCommandState == 2). Only then does the script advance past the opcode.
 */
s32 OpcodeFuncDskcg(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dskcg", 1);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_CD_CHANGE;
        D_8009D588 = GET_PARAM_U8(1);
        return 1;
    case EVTCMD_CD_CHANGE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            g_FieldState->eventCmd = EVTCMD_NONE;
            PC_INC(2);
            return 0;
        }
        return 1;
    default:
        return 1;
    }
}

/*
 * Field-script opcode UC: lock or unlock player control.
 *
 * A nonzero operand freezes the player character; on unlock the
 * per-model flag of the player's model is cleared as well.
 */
s32 OpcodeFuncUc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("uc", 1);
    }
    g_CharacterLock = g_FieldState->characterLock = GET_PARAM_U8(1);
    if (g_CharacterLock == 0) {
        D_800756E8[g_FieldState->pcModelId] = 0;
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncBtlon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btlon", 1);
    }
    g_FieldState->battlesDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMpdsp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpdsp", 1);
    }
    g_FieldState->mpdspSet = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMvcam(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mvcam", 1);
    }
    g_FieldState->movieCamDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncGmovr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gmovr", 0);
    }
    g_FieldState->eventCmd = EVTCMD_GAME_OVER;
    g_FieldState->movieCommandState = MOVCMD_IDLE;
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_char_control.c
/////////////////////////////////////////////////

/*
 * Field-script opcode CC: hand player control to another entity.
 *
 * The operand is a script entity id; if that entity has a field model
 * assigned (g_EntityToModel entry != 0xFF) it becomes the new player model.
 */
s32 OpcodeFuncCc(void) {
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cc", 1);
    }
    charId = GET_PARAM_U8(1);
    if (g_EntityToModel[charId] != 0xFF) {
        g_FieldState->pcModelId = g_EntityToModel[charId];
    }
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode CHAR: attach a field model to the current entity.
 *
 * Allocates the next model slot (g_FieldModelCount) for the executing entity,
 * records the mapping in g_EntityToModel and initializes the model with the
 * model id from the opcode operand and the owning entity id.
 */
s32 OpcodeFuncChar(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("char", 1);
    }
    g_EntityToModel[g_CurrentEntity] = g_FieldModelCount++;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].charId = GET_PARAM_U8(1);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].entityId = g_CurrentEntity;
    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_animate.c
/////////////////////////////////////////////////

/*
 * Field-script opcode DFANM: set a model's default (looping) animation.
 *
 * Stores the animation id and playback speed (per-model base speed divided
 * by the speed operand) for the model attached to the executing entity.
 * A model holding the last frame of a script animation (state 3) is
 * released so the new default animation starts playing.
 */
s32 OpcodeFuncDfanm(void) {
    u8 modelIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dfanm", 2);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        D_8008325C[g_EntityToModel[g_CurrentEntity]] = GET_PARAM_U8(1);
        D_80082248[g_EntityToModel[g_CurrentEntity]] =
            D_8009D828[g_EntityToModel[g_CurrentEntity]] / GET_PARAM_U8(2);
        modelIdx = g_EntityToModel[g_CurrentEntity];
        if (D_800756E8[modelIdx] == 3) {
            D_800756E8[modelIdx] = 0;
        }
    }
    PC_INC(3);
    return 1;
}

/*
 * Field-script opcode CCANM: set one of the player animation ids
 * (0: idle, 1: walk, 2: run) used while the player controls a model.
 */
s32 OpcodeFuncCcanm(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ccanm", 3);
    }
    switch (GET_PARAM_U8(3)) {
    case 0:
        g_FieldState->idleAnimId = GET_PARAM_U8(1);
        break;
    case 1:
        g_FieldState->walkAnimId = GET_PARAM_U8(1);
        break;
    case 2:
        g_FieldState->runAnimId = GET_PARAM_U8(1);
        break;
    }
    PC_INC(4);
    return 0;
}

/*
 * Starts the animation requested by the current ANIME-style opcode on the
 * model attached to the executing entity: animation id from the first
 * operand, playback speed from the per-model base speed divided by the
 * second operand, frame counter rewound and the last frame looked up in
 * the animation header of the model's file.
 */
void StartModelAnimation(void) {
    u8 modelIdx;
    u8* anims;
    FieldModelEntry* model;

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].activeAnimId =
        GET_PARAM_U8(1);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed =
        D_8009D828[g_EntityToModel[g_CurrentEntity]] / GET_PARAM_U8(2);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
    modelIdx = g_EntityToModel[g_CurrentEntity];
    model =
        &g_FieldModelData
             ->modelEntries[g_FieldModelLoaderData[modelIdx].modelEntryIndex];
    anims = model->modelData + model->animationOffset;
    g_FieldModels[modelIdx].animLastFrame =
        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
}

/*
 * Field-script opcode ANIME1/ANIME2: play an animation on the entity's
 * model. g_FieldCurrentOpcode distinguishes which opcode invoked the handler:
 * the asynchronous variant (0xAE, ANIME2) marks the model as playing (state 5)
 * and lets the script continue, while ANIME1 blocks (state 2) until the
 * animation system reports completion (state 4), then resets the model to
 * its default animation.
 */
s32 OpcodeFuncAnime(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("anime", 2);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(3);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        StartModelAnimation();
        if (g_FieldCurrentOpcode == 0xAE) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 5;
            PC_INC(3);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
        PC_INC(3);
        return 0;
    }
    return 1;
}

/*
 * Field-script opcode ANIM!1/ANIM!2: like ANIME1/ANIME2 but the model
 * keeps holding the last frame once the animation completes (state 3)
 * instead of returning to its default animation. 0xAE becomes 0xAF and
 * state 5 becomes 6 to tell the two opcode pairs apart.
 */
s32 OpcodeFuncAnimEx(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("anim!", 2);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(3);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        StartModelAnimation();
        if (g_FieldCurrentOpcode == 0xAF) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 6;
            PC_INC(3);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
        PC_INC(3);
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCanim);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCanmEx);

s32 OpcodeFuncAnimw(void) {
    u8 modelIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("animw", 0);
    }
    modelIdx = g_EntityToModel[g_CurrentEntity];
    if (modelIdx == 0xFF) {
        PC_INC(1);
        return 0;
    }
    switch (D_800756E8[modelIdx]) {
    case 2:
    case 5:
    case 6:
        return 1;
    case 4:
        D_800756E8[modelIdx] = 0;
        break;
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncAnimb(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("animb", 0);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animLastFrame =
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame >>
            4;
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
    }
    PC_INC(1);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_move.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMove);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncFmove);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCmove);

s32 OpcodeFuncFcfix(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fcfix", 1);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock =
            GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncJump);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncLader);

void OpcodeFuncPmova(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmova", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldMoveToEntityUpdate(actorId);
}

void OpcodeFuncMova(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mova", 1);
    }
    FieldMoveToEntityUpdate(GET_PARAM_U8(1));
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldMoveToEntityUpdate);

void OpcodeFuncDira(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dira", 1);
    }
    FieldEventSetDirByActorId(GET_PARAM_U8(1));
}

void OpcodeFuncPdira(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pdira", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldEventSetDirByActorId(actorId);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSetDirByActorId);

void OpcodeFuncTura(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tura", 3);
    }
    FieldEntityTurnToEntity(GET_PARAM_U8(1));
}

void OpcodeFuncPtura(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ptura", 3);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldEntityTurnToEntity(actorId);
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEntityTurnToEntity);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncOfstd);

/* Block until this entity's offset animation finishes. OfsType 3 means the last
 * step ran, so clear it and fall through; 0 means there was never one. */
s32 OpcodeFuncOfstw(void) {
    FieldEntity* model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ofstw", 0);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(1);
        return 0;
    }
    model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (model->OfsType != 0 && model->OfsType != 3) {
        return 1;
    }
    model->OfsType = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStep = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetSteps = 0;
    PC_INC(1);
    return 0;
}

/* Block until this entity's turn finishes. Returning 1 without advancing the
 * PC re-runs the opcode next frame; TurnType 3 means the turn just completed,
 * so clear it and fall through.
 *
 * Instruction-for-instruction identical; what is left is where gcc cross-jumps
 * the PC_INC tail. The original merges the three paths *after* the reload of
 * g_CurrentEntity, so the model == 0xFF path reuses the copy loaded at the top
 * of the function; gcc merges two instructions earlier and reloads. Writing the
 * tail out twice stops it cross-jumping at all (9 extra instructions), and
 * inverting the test to an early return flips the branch to `bne`. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncTurnw);
#else
s32 OpcodeFuncTurnw(void) {
    FieldEntity* model;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("turnw", 0);
        }
        model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
        if (model->TurnType != 0) {
            if (model->TurnType != 3) {
                return 1;
            }
            model->TurnType = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
        }
    }
    PC_INC(1);
    return 0;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncTurn);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncTurnr);

/* Snap this entity to a facing, cancelling any turn in progress. Returns 1 when
 * the entity actually has a model, unlike most opcodes. */
s32 OpcodeFuncDir(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("dir", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir =
            FieldEventReadMemoryU8(2, 2);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
        PC_INC(3);
        return 1;
    }
    PC_INC(3);
    return 0;
}

/* SLIDR: set this entity's collision radius. The script value is in map units,
 * so it is scaled by the field's own scale and divided back down by 512. */
s32 OpcodeFuncSlidr(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("slidR", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidRange =
            (FieldEventReadMemoryU8(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(3);
    return 0;
}

/* SLDR2: SLIDR with a 16-bit radius. */
s32 OpcodeFuncSldr2(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("sldR2", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidRange =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

/* TALKR: set this entity's talk radius, scaled the same way as SLIDR. */
s32 OpcodeFuncTalkr(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("talkR", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkRange =
            (FieldEventReadMemoryU8(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(3);
    return 0;
}

/* TLKR2: TALKR with a 16-bit radius. */
s32 OpcodeFuncTlkr2(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("tlkR2", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkRange =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_state.c
/////////////////////////////////////////////////

/* MSPED: set this entity's movement speed, scaled like the radius opcodes. */
s32 OpcodeFuncMsped(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("msped", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveSpeed =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncAsped(void) {
    u8 modelIdx;
    s16 speed;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("asped", 3);
        }
        speed = FieldEventReadMemoryS16(2, 2);
        modelIdx = g_EntityToModel[g_CurrentEntity];
        g_FieldModels[modelIdx].animSpeed = speed;
        D_8009D828[modelIdx] = speed;
    }
    PC_INC(4);
    return 0;
}

/* GTDIR: write another entity's facing direction back into a memory bank. */
s32 OpcodeFuncGtdir(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("gtdir", 3);
        }
        FieldEventWriteMemoryU8(
            2, 3, g_FieldModels[g_EntityToModel[entityId]].Dir);
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncPgtdr(void) {
    u8 slot;
    u8 partyId;
    u8 actorId;

    slot = GET_PARAM_U8(2);
    if (slot < 3) {
        partyId = D_8009D391[slot];
        if (partyId != 0xFF) {
            actorId = D_8009AD30[partyId];
            if (actorId != 0xFF) {
                if (g_EntityToModel[actorId] != 0xFF) {
                    if (g_DebugLevel & 3) {
                        DebugPrintOpcode("pgtdr", 3);
                    }
                    FieldEventWriteMemoryU8(
                        2, 3, g_FieldModels[g_EntityToModel[actorId]].Dir);
                }
            }
        }
    }
    PC_INC(4);
    return 0;
}

/* GETAI: write another entity's walkmesh triangle id back into a memory bank.
 */
s32 OpcodeFuncGetai(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("getai", 3);
        }
        FieldEventWriteMemoryS16(
            2, 3, g_FieldModels[g_EntityToModel[entityId]].PosI);
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncGetaxy(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("getaxy", 4);
        }
        FieldEventWriteMemoryS16(
            1, 3, g_FieldModels[g_EntityToModel[entityId]].PosX >> 12);
        FieldEventWriteMemoryS16(
            2, 4, g_FieldModels[g_EntityToModel[entityId]].PosY >> 12);
    }
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncAxyzi(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(3);
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("axyzi", 7);
    }
    if (g_EntityToModel[entityId] != 0xFF) {
        FieldEventWriteMemoryS16(
            1, 4, g_FieldModels[g_EntityToModel[entityId]].PosX >> 12);
        FieldEventWriteMemoryS16(
            2, 5, g_FieldModels[g_EntityToModel[entityId]].PosY >> 12);
        FieldEventWriteMemoryS16(
            3, 6, g_FieldModels[g_EntityToModel[entityId]].PosZ >> 12);
        FieldEventWriteMemoryS16(
            4, 7, g_FieldModels[g_EntityToModel[entityId]].PosI);
    }
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncPxyzi(void) {
    u8 slot;
    u8 partyId;
    u8 actorId;

    slot = GET_PARAM_U8(3);
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pxyzi", 7);
    }
    if (slot < 3) {
        partyId = D_8009D391[slot];
        if (partyId < 9) {
            actorId = D_8009AD30[partyId];
            if (g_EntityToModel[actorId] != 0xFF) {
                FieldEventWriteMemoryS16(
                    1, 4, g_FieldModels[g_EntityToModel[actorId]].PosX >> 12);
                FieldEventWriteMemoryS16(
                    2, 5, g_FieldModels[g_EntityToModel[actorId]].PosY >> 12);
                FieldEventWriteMemoryS16(
                    3, 6, g_FieldModels[g_EntityToModel[actorId]].PosZ >> 12);
                FieldEventWriteMemoryS16(
                    4, 7, g_FieldModels[g_EntityToModel[actorId]].PosI);
            }
        }
    }
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncVisi(void) {
    u8 model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("visi", 1);
    }
    model = g_EntityToModel[g_CurrentEntity];
    if (model != 0xFF) {
        g_FieldModels[model].visible = GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncTlkon(void) {
    u8 model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tlkon", 1);
    }
    model = g_EntityToModel[g_CurrentEntity];
    if (model != 0xFF) {
        g_FieldModels[model].TalkOff = GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncXyzi(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyzi", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ =
            FieldEventReadMemoryS16(3, 7) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosI =
            FieldEventReadMemoryS16(4, 9);
    }
    PC_INC(11);
    return 1;
}

s32 OpcodeFuncXyz(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyz", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ =
            FieldEventReadMemoryS16(3, 7) << 12;
    }
    PC_INC(9);
    return 1;
}

s32 OpcodeFuncXyi(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyi", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosI =
            FieldEventReadMemoryS16(3, 7);
    }
    PC_INC(9);
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_message.c
/////////////////////////////////////////////////

s32 OpcodeFuncMes(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mes", 2);
    }
    if (FieldDialogMessageUpdateStates(GET_PARAM_U8(1), GET_PARAM_U8(2)) != 0) {
        PC_INC(3);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncMpnam(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpnam", 1);
    }
    CopyDialogToMapName(GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode ASK: run a menu prompt and store the chosen row.
 *
 * Blocks (returning 1 and holding the player) until FieldDialogAskUpdateStates
 * reports the prompt is finished; the answer is written back to the script
 * memory bank either way.
 */
s32 OpcodeFuncAsk(void) {
    s16 answer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ask", 6);
    }
    answer = FieldEventReadMemoryU8(2, 6);
    if (FieldDialogAskUpdateStates(
            GET_PARAM_U8(2), GET_PARAM_U8(3), GET_PARAM_U8(4), GET_PARAM_U8(5),
            &answer) != 0) {
        FieldEventWriteMemoryU8(2, 6, answer);
        g_FieldState->characterLock = D_80081DC4;
        PC_INC(7);
        return 0;
    } else {
        FieldEventWriteMemoryU8(2, 6, answer);
        g_FieldState->characterLock = 1;
        return 1;
    }
}

/////////////////////////////////////////////////
// Start of field_opcode_window.c
/////////////////////////////////////////////////

s32 OpcodeFuncWclsEx(void) {
    s16 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wcls!", 0);
    }
    window = GET_PARAM_U8(1);
    if (D_8008326C[window] == 0xFF) {
        PC_INC(2);
        return 0;
    }
    FieldWindowSetStateToClose(window);
    FieldDialogMessageUpdateStates(window, 0);
    return 1;
}

s32 OpcodeFuncWsizw(void) {
    s16 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wsizw", 8);
    }
    window = GET_PARAM_U8(1);
    if (D_8008326C[window] == 0xFF) {
        return OpcodeFuncWsize();
    }
    if (D_8008326C[window] == g_CurrentEntity) {
        FieldWindowSetStateToClose(window);
        FieldDialogMessageUpdateStates(window, 0);
    }
    return 1;
}

s32 OpcodeFuncWsize(void) {
    s16 x;
    s16 y;
    s16 w;
    s16 h;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wsize", 8);
    }
    GET_PARAM_S16(x, 2);
    GET_PARAM_S16(y, 4);
    GET_PARAM_S16(w, 6);
    GET_PARAM_S16(h, 8);
    FieldDialogSetSize(GET_PARAM_U8(1), x, y, w, h);
    PC_INC(10);
    /* Not cosmetic: the statement boundary stops gcc sinking `move v0,zero`
     * into the load delay slot of the PC_INC read, which is what forces the
     * original's $v0 for the incremented value and its trailing `nop`.
     * Most likely a macro in the original. Found by decomp-permuter. */
    do {
        return 0;
    } while (0);
}

s32 OpcodeFuncWrow(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wrow", 2);
    }
    FieldDialogSetWindowHeight(GET_PARAM_U8(1), (GET_PARAM_U8(2) << 4) | 9);
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncWmove(void) {
    s16 dx;
    s16 dy;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wmove", 8);
    }
    GET_PARAM_S16(dx, 2);
    GET_PARAM_S16(dy, 4);
    FieldDialogMove(GET_PARAM_U8(1), dx, dy);
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncWrest(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wrest", 1);
    }
    FieldWindowReset(GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncWclse(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wclse", 1);
    }
    if (FieldWindowSetStateToClose(GET_PARAM_U8(1)) != 0) {
        PC_INC(2);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncWmode(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wmode", 3);
    }
    FieldDialogSetWindowStyleCbc(
        GET_PARAM_U8(1), GET_PARAM_U8(2), GET_PARAM_U8(3));
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_math.c
/////////////////////////////////////////////////

/**
 * @brief Opcode 0x8F - **AND** - Bitwise AND (8-bit)
 *
 * Memory layout:
 *
 * | 0x8F | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise AND and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise AND.
 * @details
 * Performs a bitwise AND operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the "Oper" is the
 * operand to AND with. If the Source Bank is an 8 bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncAnd(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("and", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) & FieldEventReadMemoryU8(2, 3) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x90 - **AND2** - Bitwise AND (16-bit)
 *
 * Memory layout:
 *
 * | 0x90 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise AND, and
 * that which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise AND, or address of the
 * second operand, if S is non-zero.
 * @details
 * Performs a bitwise AND operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then the "Oper" is the
 * operand to AND with. If the Source Bank is a 16-bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncAnd2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("and2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x91 - **OR** - Bitwise OR (8-bit)
 *
 * Memory layout:
 *
 * | 0x91 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise OR and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise OR.
 * @details
 * Performs a bitwise OR operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the "Oper" is the
 * operand to OR with. If the Source Bank
 * is an 8 bit bank, then the "Oper" is the address in that bank where
 * the operand is.
 */
s32 OpcodeFuncOr(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("or", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) | FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x92 - **OR2** - Bitwise OR (16-bit)
 *
 * Memory layout:
 *
 * | 0x92 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise OR, and that
 * which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise OR, or address of the
 * second operand, if S is non-zero
 * @details
 * Performs a bitwise OR operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then "Oper" is the operand
 * to OR with. If the Source Bank is a 16-bit bank, then the "Oper" is the
 * address in that bank where the operand is.
 */
s32 OpcodeFuncOr2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("or2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x93 - **XOR** - Bitwise XOR (8-bit)
 *
 * Memory layout:
 *
 * | 0x93 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise XOR and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise XOR.
 * @details
 * Performs a bitwise XOR operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the Operis the operand
 * to XOR with. If the Source Bank is an 8 bit bank, then the "Oper" is the
 * address in that bank where the operand is.
 */
s32 OpcodeFuncXor(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("xor", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) ^ FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x94 - **XOR2** - Bitwise XOR (16-bit)
 *
 * Memory layout:
 *
 * | 0x94 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise XOR, and
 * that which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise XOR, or address of the
 * second operand, if S is non-zero.
 * @details
 * Performs a bitwise XOR operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then the "Oper" is the
 * operand to XOR with. If the Source Bank is a 16-bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncXor2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("xor2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x85 - **PLUS** - Addition (8-bit)
 *
 * Memory layout:
 *
 * | 0x85 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const UByte Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into  "Dest" . The
 * result of the addition wraps around into the range of 0-255. If the Source
 * Bank is 0 then the "Oper" is added to the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncPlus(void) {
    u16* temp_a0;
    u8 temp_s0;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) + FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x76 - **PLUS!** - Saturated Addition (8-bit)
 *
 * Memory layout:
 *
 * | 0x76 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const UByte Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into "Dest". The result
 * of the addition is capped at 255. If the Source Bank is 0 then the "Oper" is
 * added to the destination value. If the Source Bank is an 8 bit bank, then the
 * "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncPlusEx(void) {
    u8 a;
    u8 b;
    s16 sum;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus!", 3);
    }

    a = FieldEventReadMemoryU8(1, 2);
    b = FieldEventReadMemoryU8(2, 3);
    sum = a + b;
    if (sum > 255) {
        sum = 255;
    }

    FieldEventWriteMemoryU8(1, 2, sum);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x86 - **PLUS2** - Addition (16-bit)
 *
 * Memory layout:
 *
 * | 0x86 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const SWord Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into  "Dest" . The
 * result of the addition wraps around into the 16-bit range. If the Source Bank
 * is 0 then the "Oper" is added to the destination value. If the
 * Source Bank is an 16 bit bank, then the "Oper" is the address in that bank
 * where the operand is.
 */
s32 OpcodeFuncPlus2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) + FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x77 - **PLS2!** - Saturated Addition (16-bit)
 *
 * Memory layout:
 *
 * | 0x77 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const SWord Oper: The operand, added to the destination
 * @details
 * Adds two numbers together and stores the result back into "Dest" The result
 * of the addition is capped at 32767. The result is not capped at the negative
 * end, however (-32768), so adding two large negative numbers together will
 * still produce wrap-around. If the Source Bank is 0 then the "Oper" is added
 * to the destination value. If the Source Bank is an 16 bit bank, then the
 * "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncPls2Ex(void) {
    s16 a;
    s16 b;
    s32 sum;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pls2!", 3);
    }
    a = FieldEventReadMemoryS16(1, 2);
    b = FieldEventReadMemoryS16(2, 3);
    sum = a + b;
    if (sum > 0x7FFF) {
        sum = 0x7FFF;
    }
    FieldEventWriteMemoryS16(1, 2, sum);
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x87 - **MINUS** - Subtraction (8-bit)
 *
 * Memory layout:
 *
 * | 0x87 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const UByte Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts two numbers and stores the result back into  "Dest" . The result of
 * the subtraction wraps around into the range of 0-255. If the Source Bank is 0
 * then the "Oper" is subtracted from the destination value. If the Source Bank
 * is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMinus(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("minus", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) - FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x78 - **MINS!** - Saturated Subtraction (8-bit)
 *
 * Memory layout:
 *
 * | 0x78 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const UByte Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts "Oper" from "Dest" and stores the result back into "Dest". The
 * result of the subtraction is capped at 0. If the Source Bank is 0 then the
 * "Oper" is subtracted from the destination value. If the Source Bank is an 8
 * bit bank, then the "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncMinsEx(void) {
    u8 a;
    u8 b;
    s16 differ;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mins!", 3);
    }
    a = FieldEventReadMemoryU8(1, 2);
    b = FieldEventReadMemoryU8(2, 3);
    differ = a - b;
    if (differ < 0) {
        differ = 0;
    }
    FieldEventWriteMemoryU8(1, 2, differ & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x88 - **MINS2** - Subtraction (16-bit)
 *
 * Memory layout:
 *
 * | 0x98 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const SWord Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts two numbers and stores the result back into "Dest". The result of
 * the subtraction wraps around into the 16-bit range. If the Source Bank is 0
 * then the "Oper" is subtracted from the destination value. If the Source Bank
 * is an 16 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMins2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mins2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) - FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x79 - **MNS2!** - Saturated Subtraction (16-bit)
 *
 * Memory layout:
 *
 * | 0x79 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const SWord Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts "Oper" from "Dest" and stores the result back into "Dest". The
 * result of the subtraction is capped at -32768. The result is not capped at
 * the positive end (32767), so subtracting a large negative number from a large
 * positive number will still produce wrap-around. If the Source Bank is 0 then
 * the "Oper" is subtracted from the destination value. If the
 * Source
 * Bank is an 16 bit bank, then the "Oper" is the address in that bank
 * where the operand is.
 */
s32 OpcodeFuncMns2Ex(void) {
    s16 a;
    s16 b;
    s32 differ;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mns2!", 3);
    }
    a = FieldEventReadMemoryS16(1, 2);
    b = FieldEventReadMemoryS16(2, 3);
    differ = a - b;
    if (differ <= 0x7FFF) {
        differ = 0x8000;
    }
    FieldEventWriteMemoryS16(1, 2, differ);
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x89 - **MUL** - Multiplication (8-bit)
 *
 * Memory layout:
 *
 * | 0x89 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * multiplied.
 * - const UByte Oper: The operand, which is multiplied with the destination.
 * @details
 * Multiplies two numbers together and stores the result back into "Dest". The
 * result of the Multiplication is capped at 255. If the Source Bank is 0 then
 * the the value "Oper" is multiplied with the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMul(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mul", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) * FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8A - **MUL2** - Multiplication (16-bit)
 *
 * Memory layout:
 *
 * | 0x8A | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * multiplied.
 * - const SWord Oper: The operand, which is multiplied with the destination.
 * @details
 * Multiplies two numbers together and stores the result back into "Dest". The
 * result of the Multiplication is capped at 32767. If the Source Bank is 0 then
 * the the value "Oper" is multiplied with the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMul2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mul2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) * FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x8B - **DIV** - Division (8-bit)
 *
 * Memory layout:
 *
 * | 0x8B | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * quotient.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the result back into "Dest". The result of
 * the division is rounded towards zero to the nearest integer. If the Source
 * Bank is 0 then the "Den" is the denominator. If the Source Bank is an 8 bit
 * bank, then the "Den" is the address in that bank where the denominator is.
 */
s32 OpcodeFuncDiv(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("div", 3);
    }
    FieldEventWriteMemoryU8(1, 2,
                            (u8)((u32)(FieldEventReadMemoryU8(1, 2) & 0xFF) /
                                 (u32)(FieldEventReadMemoryU8(2, 3) & 0xFF)));
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8C - **DIV2** - Division (16-bit)
 *
 * Memory layout:
 *
 * | 0x8C | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * quotient.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the result back into "Dest". The
 * result of the division is rounded towards zero to the nearest integer. If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 8 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncDiv2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("div2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) / FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x8D - **REMAI** - Modulus (8-bit)
 *
 * Memory layout:
 *
 * | 0x8D | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * remainder.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the remainder back into "Dest". If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 8 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncRemai(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("remai", 3);
    }
    FieldEventWriteMemoryU8(1, 2,
                            (u8)((u32)(FieldEventReadMemoryU8(1, 2) & 0xFF) %
                                 (u32)(FieldEventReadMemoryU8(2, 3) & 0xFF)));
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8E - **REMA2** - Modulus (16-bit)
 *
 * Memory layout:
 *
 * | 0x8E | D/S | Dest | Den
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * remainder.
 * - const SWord Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the remainder back into "Dest". If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 16 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncRema2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rema2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) % FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x95 - **INC** - Increment (8-bit)
 *
 * Memory layout:
 *
 * | 0x95 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Increments the 8-bit value found at bank B, address A. If the value is 0xFF,
 * it will roll over to 0x00. If you specify a 16-bit bank, only the lower byte
 * will be incremented, and if the lower byte is 0xFF, the higher byte will be
 * unaffected whilst the lower byte will return to 0x00.
 */
s32 OpcodeFuncInc(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc", 2);
    }
    FieldEventWriteMemoryU8(2, 2, (FieldEventReadMemoryU8(2, 2) + 1) & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7A - **INC!** - Saturated Increment (8-bit)
 *
 * Memory layout:
 *
 * | 0x7A | 0/D | Dest |
 *
 * - const Bit[4] D: Destination bank
 * - const UByte Dest: The destination address in the bank where the variable is
 * incremented.
 * @details
 * Increments the value in "Dest" by 1. The result is capped at 255.
 */
s32 OpcodeFuncIncEx(void) {
    s16 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc!", 2);
    }
    result = (FieldEventReadMemoryU8(2, 2) & 0xFF) + 1;
    if (result >= 0x100) {
        result = 0xFF;
    }
    FieldEventWriteMemoryU8(2, 2, result & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode  0x96 - **INC2** - Increment (16-bit)
 *
 * Memory layout:
 *
 * | 0x96 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Increments the 16-bit value found at bank B, address A. If the value is
 * 0xFFFF, it will roll over to 0x0000.
 */
s32 OpcodeFuncInc2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc2", 3);
    }
    FieldEventWriteMemoryS16(2, 2, (s16)(FieldEventReadMemoryS16(2, 2) + 1));
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode  0x7B - **INC2!** - Saturated Increment (16-bit)
 *
 * Memory layout:
 *
 * | 0x7B | 0/D | Dest |
 *
 * - const UByte B: Destination bank.
 * - const UByte Dest: The destination address in the bank where the variable is
 * incremented.
 * @details
 * Increments the value in "Dest" by 1. The result is capped at
 * 32767.
 */
s32 OpcodeFuncInc2Ex(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc2!", 3);
    }
    result = FieldEventReadMemoryS16(2, 2) + 1;
    if (result > 0x7FFF) {
        result = 0x7FFF;
    }
    FieldEventWriteMemoryS16(2, 2, result);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x97 - **DEC** - Decrement (8-bit)
 *
 * Memory layout:
 *
 * | 0x97 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Decrements the 8-bit value found at bank B, address A. If the value is
 * 0x00, it will roll over to 0xFF. If you specify a 16-bit bank, only the
 * lower byte will be decremented, and if the lower byte is 0x00, the higher
 * byte will be unaffected whilst the lower byte will return to 0xFF.
 */
s32 OpcodeFuncDec(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec", 2);
    }
    FieldEventWriteMemoryU8(2, 2, (FieldEventReadMemoryU8(2, 2) - 1) & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7C - **DEC!** - Saturated Decrement (8-bit)
 *
 * Memory layout:
 *
 * | 0x7C | 0/D | Dest |
 *
 * - const UByte B: Destination bank.
 * - const UByte Dest: The destination address in the bank where the variable is
 * deccremented.
 * @details
 * Decreases the value in "Dest" by 1. The result is capped at 0.
 */
s32 OpcodeFuncDecEx(void) {
    s16 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec!", 2);
    }
    result = (FieldEventReadMemoryU8(2, 2) & 0xFF) - 1;
    if (result < 0) {
        result = 0;
    }
    FieldEventWriteMemoryU8(2, 2, result & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x98 - **DEC2** - Decrement (16-bit)
 *
 * Memory layout:
 *
 * | 0x98 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Decrements the 16-bit value found at bank B, address A. If the value is
 * 0x0000, it will roll over to 0xFFFF.
 */
s32 OpcodeFuncDec2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec2", 3);
    }
    FieldEventWriteMemoryS16(2, 2, (s16)(FieldEventReadMemoryS16(2, 2) - 1));
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7D - **DEC2!** - Saturated Decrement (16-bit)
 *
 * Memory layout:
 *
 * | 0x7D | 0/D | Dest |
 *
 * - const Bit[4] D: Destination bank
 * - const UByte Dest: The destination address in the bank where the variable is
 * Decremented.
 * @details
 * Decreases the value in "Dest" by 1. The result is capped at -32768.
 */
s32 OpcodeFuncDec2Ex(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec2!", 3);
    }
    result = FieldEventReadMemoryS16(2, 2) - 1;
    if (result <= 0x7FFF) {
        result = 0x8000;
    }
    FieldEventWriteMemoryS16(2, 2, result);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x99 - **RANDM** - Random
 *
 * Memory layout:
 *
 * | 0x99 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Destination address.
 * @details
 * Places a random 8-bit value into the destination bank and address specified.
 * If you specify a 16-bit bank, only the lower byte is randomised.
 */
s32 OpcodeFuncRandm(void) {
    u16* temp_v1;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("randm", 2);
    }
    g_RandomTableIndex += g_RandomTableStep;
    FieldEventWriteMemoryU8(2, 2, g_RandomTable[g_RandomTableIndex]);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7F - **RDMSD** - Seed Random Generator
 *
 * Memory layout:
 *
 * | 0x7F | B | S |
 *
 * - const UByte B: Bank in which the seed value is stored, or zero if S is
 * specified as a literal value.
 * - const UByte A: Destination address.
 * @details
 * Seeds the random number generator used by RANDOM. The lower four bits of the
 * arguments are used as the seed value by altering the offset used to take a
 * value from the table of pseudo-random numbers.
 */
s32 OpcodeFuncRdmsd(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rdmsd", 2);
    }
    g_RandomTableStep = (FieldEventReadMemoryU8(2, 2) << 4) + 1;
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_background.c
/////////////////////////////////////////////////

s32 OpcodeFuncBgon(void) {
    u8 layer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgon", 3);
    }
    layer = FieldEventReadMemoryU8(1, 2);
    g_FieldState->backgroundLayerVisibility[layer] |=
        1 << FieldEventReadMemoryU8(2, 3);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBgoff(void) {
    u8 layer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgoff", 3);
    }
    layer = FieldEventReadMemoryU8(1, 2);
    g_FieldState->backgroundLayerVisibility[layer] &=
        ~(1 << FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBgclr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgclr", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] = 0;
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBgrol(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgrol", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] <<= 1;
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBgrol2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgrol", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] >>= 1;
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_movie.c
////////////////////////////////////////////////

/* Preload the movie named by the parameter, blocking until the load finishes.
 * Same post-then-poll shape as OpcodeFuncMovie, one event command earlier. */
s32 OpcodeFuncPmvie(void) {
    s16 movieId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmvie", 1);
    }
    if (D_800716CC != 0) {
        PC_INC(2);
        return 0;
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_LOAD_MOVIE:
        switch (g_FieldState->movieCommandState) {
        case MOVCMD_ACTIVE:
            break;
        case MOVCMD_DONE:
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(2);
            return 0;
        }
        return 1;
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_LOAD_MOVIE;
        movieId = GET_PARAM_U8(1);
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        g_FieldState->eventCmdParam = movieId;
        break;
    }
    return 1;
}

/* Play the field map's movie, blocking until it finishes. Returning 1 without
 * advancing the PC re-runs the opcode next frame, so the request is posted once
 * as an event command and then polled. */
s32 OpcodeFuncMovie(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("movie", 0);
    }
    g_FieldMovieOpcodeActive = 1;
    if (D_800716CC != 0) {
        D_801144D4 = 0;
        PC_INC(1);
        return 0;
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_PLAY_MOVIE:
        switch (g_FieldState->movieCommandState) {
        case MOVCMD_ACTIVE:
            break;
        case MOVCMD_DONE:
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(1);
            return 0;
        }
        return 1;
    case EVTCMD_UNK14:
        PC_INC(1);
        return 0;
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_PLAY_MOVIE;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        break;
    }
    return 1;
}

s32 OpcodeFuncMvief(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mvief", 2);
    }
    if (D_800716CC != 0) {
        FieldEventWriteMemoryS16(2, 2, D_801144D4);
        D_801144D4++;
        PC_INC(3);
        return 0;
    } else {
        FieldEventWriteMemoryS16(2, 2, g_FieldState->currentMovieFrame);
        PC_INC(3);
        return 0;
    }
}

s32 OpcodeFuncMpjpo(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpjpo", 0);
    }
    g_FieldState->mapJumpDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_scroll.c
////////////////////////////////////////////////

s32 OpcodeFuncScr2d(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2d", 5);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_INSTANT;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 2);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 4);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncScrlc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlc", 0);
    }
    g_FieldState->cameraScrollMode = GET_PARAM_U8(4);
    g_FieldState->cameraScrollTargetId = g_FieldState->pcModelId;
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(5);
    return 0;
}

/* Scroll the camera to an entity over a number of frames. Unlike SCR2D the
 * target is an entity id, so a missing model makes the opcode a no-op. */
s32 OpcodeFuncScrla(void) {
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrla", 0);
    }
    entityId = GET_PARAM_U8(4);
    if (g_EntityToModel[entityId] != 0xFF) {
        g_FieldState->cameraScrollMode = GET_PARAM_U8(5);
        g_FieldState->cameraScrollTargetId = g_EntityToModel[entityId];
        g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
        g_FieldState->cameraScrollState = 0;
    }
    PC_INC(6);
    return 0;
}

/* SCRLP is SCRLA addressed by party slot rather than by entity: the slot picks
 * a character, the character picks the field entity that represents them.
 *
 * The copy back into partyId is load-bearing, not redundant. Indexing
 * g_EntityToModel with actorId directly widens it in place as
 * `andi a1,v0,0xff`, where the original holds the resolved actor in v0 and
 * copies it out with a plain `move`. Going through the (by now dead) slot
 * variable is what produces that copy. Found by decomp-permuter. */
s32 OpcodeFuncScrlp(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlp", 0);
    }
    partyId = D_8009D391[GET_PARAM_U8(4)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    partyId = actorId;
    if (g_EntityToModel[partyId] != 0xFF) {
        g_FieldState->cameraScrollMode = GET_PARAM_U8(5);
        g_FieldState->cameraScrollTargetId = g_EntityToModel[partyId];
        g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
        g_FieldState->cameraScrollState = 0;
    }
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncScrcc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrcc", 0);
    }
    g_FieldState->cameraScrollMode = SCRL_OFF;
    g_FieldState->cameraScrollTargetId = g_FieldState->pcModelId;
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncScr2dc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2dc", 8);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_SMOOTH;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 3);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 5);
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(4, 7);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(9);
    return 0;
}

s32 OpcodeFuncScr2dl(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2dl", 8);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_LINEAR;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 3);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 5);
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(4, 7);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(9);
    return 0;
}

s32 OpcodeFuncScrlw(void) {
    s32 mode;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlw", 0);
    }
    if (g_FieldState->cameraScrollState == SCRLST_DONE) {
        mode = g_FieldState->cameraScrollMode;
        if (mode != SCRL_OFF) {
            if (mode < SCRL_TO_COORDS_INSTANT) {
                g_FieldState->cameraScrollMode = SCRL_TO_ENTITY_INSTANT;
            } else if (mode < 7) {
                if (mode >= SCRL_TO_COORDS_LINEAR) {
                    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_INSTANT;
                }
            }
        }
        g_FieldState->cameraScrollState = SCRLST_INIT;
        PC_INC(1);
        return 0;
    }
    return 1;
}

/////////////////////////////////////////////////
// Begin of field_opcode_palette.c
////////////////////////////////////////////////

s32 OpcodeFuncStpal(void) {
    RECT rect;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpal", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = FieldEventReadMemoryU8(1, 2) + 0x1E0;
    rect.x = 0;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    StoreImage(&rect, (u_long*)&D_80095DE0[FieldEventReadMemoryU8(2, 3) * 32]);
    PC_INC(5);
    return 0;
}

/* Four instructions out: gcc folds the palette address to
 * base + (pal*32 + x*2); the original groups it (base + x*2) + pal*32. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncStpls);
#else
s32 OpcodeFuncStpls(void) {
    RECT rect;
    s16 x;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(1) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    StoreImage(&rect, (u_long*)&D_80095DE0[GET_PARAM_U8(2) * 32 + x * 2]);
    PC_INC(5);
    return 0;
}
#endif

s32 OpcodeFuncLdpal(void) {
    RECT rect;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpal", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = FieldEventReadMemoryU8(2, 3) + 0x1E0;
    rect.x = 0;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    LoadImage(&rect, (u_long*)&D_80095DE0[FieldEventReadMemoryU8(1, 2) * 32]);
    PC_INC(5);
    return 0;
}

/* Same address-grouping residue as OpcodeFuncStpls above. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncLdpls);
#else
s32 OpcodeFuncLdpls(void) {
    RECT rect;
    s16 x;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(2) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    LoadImage(&rect, (u_long*)&D_80095DE0[GET_PARAM_U8(1) * 32 + x * 2]);
    PC_INC(5);
    return 0;
}
#endif

static void FieldEventRectClear(s16* arg0) {
    arg0[0] = 0;
    arg0[1] = 0;
    arg0[2] = 0;
    arg0[3] = 0;
}

/* Copy the first `count` entries of one 16-colour palette over another. The
 * palette store is a flat byte array of 32-byte pages, so both ends have to be
 * re-cast to u16 to walk entries rather than bytes. Declaring the two pointers
 * inside the loop is what makes gcc hoist each as one invariant; written above
 * the loop they land ahead of the zero-trip guard, and written inline gcc
 * reassociates the base out and the body needs a third `addu`.
 *
 * One instruction from matching: the original materialises &D_80095DE0 between
 * the `andi` that widens the palette id and the `sll` that scales it, gcc after
 * both. The two are a .rodata unit -- OpcodeFuncCppal owns the "cppal" string
 * that OpcodeFuncCppal2 prints -- so neither can land without the other. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCppal);
#else
s32 OpcodeFuncCppal(void) {
    s16 count;
    u8 src;
    u8 dst;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cppal", 4);
    }
    count = GET_PARAM_U8(4) + 1;
    src = FieldEventReadMemoryU8(1, 2);
    dst = FieldEventReadMemoryU8(2, 3);
    for (i = 0; i < count; i++) {
        u16* dstPal = (u16*)(D_80095DE0 + dst * 32);
        u16* srcPal = (u16*)(D_80095DE0 + src * 32);

        dstPal[i] = srcPal[i];
    }
    PC_INC(5);
    return 0;
}
#endif

/* As CPPAL, but source and destination each get their own start entry, so the
 * copy can shift a run of colours within or between palettes.
 *
 * Instruction-for-instruction identical; the preheader swaps $v0 and $v1
 * between the scaled index and the base address. Reversing the operand order in
 * the C does not move it. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncCppal2);
#else
s32 OpcodeFuncCppal2(void) {
    s16 count;
    s16 srcPal;
    s16 dstPal;
    s16 src;
    s16 dst;
    s16 end;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cppal", 7);
    }
    count = FieldEventReadMemoryU8(4, 7) + 1;
    srcPal = GET_PARAM_U8(3);
    dstPal = GET_PARAM_U8(4);
    src = FieldEventReadMemoryU8(1, 5);
    dst = FieldEventReadMemoryU8(2, 6);
    end = src + count;
    while (src < end) {
        u16* to = (u16*)(D_80095DE0 + dstPal * 32);
        u16* from = (u16*)(D_80095DE0 + srcPal * 32);

        to[dst] = from[src];
        src++;
        dst++;
    }
    PC_INC(8);
    return 0;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncRtpal);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncRtpal2);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncAdpal);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncAdpal2);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMppal2);

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMppal);

static void SetPcModel(void) {
    if (Savemap.memory_bank_2[9] != 0xFF &&
        g_CharIdToEntity[Savemap.memory_bank_2[9]] != 0xFF &&
        g_EntityToModel[g_CharIdToEntity[Savemap.memory_bank_2[9]]] != 0xFF) {
        g_FieldState->pcModelId =
            g_EntityToModel[g_CharIdToEntity[Savemap.memory_bank_2[9]]];
    }
}

s32 OpcodeFuncPc(void) {
    u8 charId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pc", 1);
    }

    charId = GET_PARAM_U8(1);
    g_CharIdToEntity[charId] = g_CurrentEntity;

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            if (i != 0) {
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 0;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff = 1;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkOff = 1;
            } else {
                g_FieldState->pcModelId = g_EntityToModel[g_CurrentEntity];
            }

            PC_INC(2);
            return 0;
        }
    }

    g_CharIdToEntity[charId] = g_CurrentEntity;

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkOff = 1;

    PC_INC(2);
    return 0;
}

s32 OpcodeFuncPrtyp(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtyp", 1);
    }

    charId = GET_PARAM_U8(1);
    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            PC_INC(2);
            SetPcModel();
            PartyFromBank2ToSave(0);
            return 0;
        }
    }

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == 0xFF) {
            ADD_PARTY_MEMBER(i, charId);

            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2(
                    "p+ ef=", g_CharIdToEntity[charId], 2);
            }
            PC_INC(2);
            SetPcModel();
            PartyFromBank2ToSave(1);
            return 0;
        }
    }

    ADD_PARTY_MEMBER(2, charId);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("p+ lf=", g_CharIdToEntity[charId], 2);
    }
    PC_INC(2);
    SetPcModel();
    PartyFromBank2ToSave(1);
    return 0;
}

s32 OpcodeFuncPrtym(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtym", 1);
    }

    charId = GET_PARAM_U8(1);

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            Savemap.memory_bank_2[9 + i] = 0xFF;
            PartyFromBank2ToSave(1);
            SetPcModel();
            PC_INC(2);
            return 0;
        }
    }

    PartyFromBank2ToSave(1);
    SetPcModel();
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncPrtye(void) {
    u8 newParty[3];
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtye", 3);
    }

    for (i = 0; i < 3; i++) {
        newParty[i] = (&GET_PARAM_U8(1))[i];
    }

    PartyReplace(newParty);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncSptye(void) {
    u8 newParty[3];
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sptye", 5);
    }

    for (i = 0; i < 3; i++) {
        newParty[i] = FieldEventReadMemoryU8(1 + i, 3 + i);
    }

    PartyReplace(newParty);
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncGptye(void) {
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gptye", 5);
    }

    for (i = 0; i < 3; i++) {
        FieldEventWriteMemoryU8(1 + i, 3 + i, Savemap.memory_bank_2[9 + i]);
    }
    PC_INC(6);
    return 0;
}

// Partial replace of bank 2 party with newParty.
// Any free slots in newParty are taken up by members of bank 2 party.
// The result is then transferred to main party in save.
static void PartyReplace(u8* newParty) {
    s32 i, j;

    // Remove requested members from old party.
    for (i = 0; i < 3; i++) {
        if (newParty[i] != 0xFF) {
            for (j = 0; j < 3; j++) {
                if (newParty[i] == Savemap.memory_bank_2[9 + j]) {
                    Savemap.memory_bank_2[9 + j] = 0xFF;
                }
            }
        }
    }

    // Add remaining members of old party to empty slots in new party.
    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] != 0xFF) {
            for (j = 0; j < 3; j++) {
                if (newParty[j] == 0xFF) {
                    newParty[j] = Savemap.memory_bank_2[9 + i];
                    j = 3;
                }
            }
        }
    }

    // Overwrite old party with new party.
    for (i = 0; i < 3; i++) {
        // Convert forced empty slots to regular empty slots.
        if (newParty[i] == 0xFE) {
            newParty[i] = 0xFF;
        }

        ADD_PARTY_MEMBER(i, newParty[i]);
    }

    PartyFromBank2ToSave(1);
    SetPcModel();
}

// Compares two sets of parties and returns which members don't exist in both.
static void PartyCompare(
    u8* party1, u8* party2, u8* party2Only, u8* party1Only) {
    s32 i, j, k;

    for (i = 0; i < 3; i++) {
        party2Only[i] = 0xFF;
        party1Only[i] = 0xFF;
    }

    k = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party2[i] == party1[j]) {
                goto foundInParty1;
            }
        }
        party2Only[k++] = party2[i];
    foundInParty1:;
    }

    k = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party1[i] == party2[j]) {
                goto foundInParty2;
            }
        }
        party1Only[k++] = party1[i];
    foundInParty2:;
    }
}

// Transfers party from bank 2 to save while preserving order in save of
// characters existing in both parties.
static void PartyFromBank2ToSave(s32 unused) {
    u8 notInSave[3];
    u8 notInBank2[3];

    PartyCompare(
        Savemap.partyID, &Savemap.memory_bank_2[9], notInSave, notInBank2);
    PartyRemove(Savemap.partyID, notInBank2);
    PartyAdd(Savemap.partyID, notInSave);
    g_PartyUpdatedByFieldScript = 1;
}

// Transfers party from save to bank 2 while preserving order in bank 2 of
// characters existing in both parties.
static void PartyFromSaveToBank2(void) {
    u8 notInBank2[3];
    u8 notInSave[3];

    PartyCompare(
        &Savemap.memory_bank_2[9], Savemap.partyID, notInBank2, notInSave);
    PartyRemove(&Savemap.memory_bank_2[9], notInSave);
    PartyAdd(&Savemap.memory_bank_2[9], notInBank2);
}

static void PartyRemove(u8* party, u8* toRemove) {
    s32 i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (toRemove[i] == party[j]) {
                party[j] = 0xFF;
            }
        }
    }
}

// Adds characters from toAdd to the first free slots in party.
// Does not use force freed slots.
static void PartyAdd(u8* party, u8* toAdd) {
    s32 i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party[j] == 0xFF) {
                party[j] = toAdd[i];
                break;
            }
        }
    }
}

s32 OpcodeFuncPrtyq(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtyq", 2);
    }

    charId = GET_PARAM_U8(1);

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("prty=TRUE", 0, 0);
            }
            PC_INC(3);
            return 0;
        }
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("prty=FALSE", 0, 0);
    }
    PC_INC(GET_PARAM_U8(2) + 2);
    return 0;
}

s32 OpcodeFuncMembq(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("membq", 2);
    }

    if ((1 << GET_PARAM_U8(1)) & Savemap.phs_visibility_mask) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("memb=TRUE", 0, 0);
        }
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("memb=FALSE", 0, 0);
    }
    PC_INC(GET_PARAM_U8(2) + 2);
    return 0;
}

s32 OpcodeFuncMmbPlusMinus(void) {
    s16 i;
    s16 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmb+-", 3);
    }

    charId = GET_PARAM_U8(2);

    if (GET_PARAM_U8(1)) {
        Savemap.phs_visibility_mask |= 1 << charId;
    } else {
        Savemap.phs_visibility_mask &= ~(1 << charId);
        for (i = 0; i < 3; i++) {
            if (Savemap.memory_bank_2[9 + i] == charId) {
                Savemap.memory_bank_2[9 + i] = 0xFF;
            }
        }
    }

    PC_INC(3);
    return 0;
}

s32 OpcodeFuncMmblk(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmblk", 3);
    }

    Savemap.phs_locking_mask |= 1 << GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMmbuk(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmbuk", 3);
    }

    Savemap.phs_locking_mask &= ~(1 << GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncSolid(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("solid", 1);
    }

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff =
            GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

/* Set the camera's view offset. A non-zero mode eases from the current offset
 * to the target over N steps; mode 0 applies it immediately and clears the
 * animation state. */
/* Every instruction matches except the tail merge: gcc cross-jumps the whole
 * shared PC_INC(7) tail, where the original keeps the
 * &g_FieldScriptPC[g_CurrentEntity] computation duplicated in both arms. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncVwoft);
#else
s32 OpcodeFuncVwoft(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("vwoft", 6);
    }
    if (GET_PARAM_U8(6)) {
        g_FieldState->viewOffsetStart = g_FieldState->viewOffset;
        g_FieldState->viewOffsetTarget = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetNumSteps = FieldEventReadMemoryS16(2, 4);
        g_FieldState->viewOffsetMode = GET_PARAM_U8(6);
        g_FieldState->viewOffsetCurrentStep = 0;
    } else {
        g_FieldState->viewOffsetNumSteps = 0;
        g_FieldState->viewOffset = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetCurrentStep = 0;
        g_FieldState->viewOffsetMode = 0;
        g_FieldState->viewOffsetStart = 0;
        g_FieldState->viewOffsetTarget = 0;
    }
    PC_INC(7);
    return 0;
}
#endif

/////////////////////////////////////////////////
// Begin of field_opcode_party_manage.c
/////////////////////////////////////////////////

s32 FieldEventJoinSet(u8, u8); // extern

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncJoin);
#else
/* 25 rows: gcc hoists the 0xFF constant into a saved reg ($s1) for the two
 * memory_bank_2[10]/[11] compares and reuses it across both FieldEventJoinSet
 * calls; target reloads `li v0,0xff` per compare and keeps the stack frame at
 * -0x18 (no $s1 save). GET_PARAM_U8(1) shared by both calls is the hoist
 * trigger. g_FieldModels idiom (not g_FieldEntity) was the key fix that cut
 * 66->25 rows. Polarity flip and block-scope arg temp both plateau at 25. */
s32 OpcodeFuncJoin(void) {
    s32 joinOk;
    s32 splitOk;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("join", 1);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    joinOk = 1;
    if (Savemap.memory_bank_2[10] != 0xFF) {
        joinOk = FieldEventJoinSet(
            g_CharIdToEntity[Savemap.memory_bank_2[10]], GET_PARAM_U8(1));
    }
    if (Savemap.memory_bank_2[11] != 0xFF) {
        splitOk = FieldEventJoinSet(
            g_CharIdToEntity[Savemap.memory_bank_2[11]], GET_PARAM_U8(1));
    } else {
        splitOk = 1;
    }
    if (joinOk && splitOk) {
        for (i = 0; i < 3; i++) {
            charId = Savemap.memory_bank_2[9 + i];
            if (charId != 0xFF) {
                g_EntitySplitJoinState[g_CharIdToEntity[charId]] = 0;
                if (i == 0) {
                    modelId = g_CharIdToEntity[charId];
                    if (modelId != 0xFF) {
                        g_FieldModels[g_EntityToModel[modelId]].SolidOff = 0;
                    }
                }
            }
        }
        g_FieldState->characterLock = g_CharacterLock;
        g_EntityForSplitJoin = 0xFF;
        PC_INC(2);
        return 0;
    }
    g_FieldState->characterLock = 1;
    if (Savemap.memory_bank_2[9] != 0xFF) {
        modelId = g_CharIdToEntity[Savemap.memory_bank_2[9]];
        if (modelId != 0xFF) {
            g_EntitySplitJoinState[modelId] = 1;
            g_FieldModels[g_EntityToModel[modelId]].scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[modelId]].ActionState = 0;
            g_FieldModels[g_EntityToModel[modelId]].SolidOff = 1;
        }
    }
    return 1;
}
#endif

s32 FieldEventSplitSet(u8, s16, s16, s32, s32); // extern
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncSplit);
#else
/* 25 rows: same $s1 0xFF-constant hoist as OpcodeFuncJoin (twin function).
 * if==0xFF polarity matches target; the != form regressed to 39. g_FieldModels
 * idiom applied. Solve Join and the recipe transfers here. */
s32 OpcodeFuncSplit(void) {
    s32 splitOkA;
    s32 splitOkB;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("split", 8);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    if (Savemap.memory_bank_2[10] == 0xFF) {
        splitOkA = 1;
    } else {
        splitOkA = FieldEventSplitSet(
            g_CharIdToEntity[Savemap.memory_bank_2[10]],
            FieldEventReadMemoryS16(1, 4), FieldEventReadMemoryS16(2, 6),
            FieldEventReadMemoryU8(3, 8) & 0xFF, GET_PARAM_U8(14));
    }
    if (Savemap.memory_bank_2[11] == 0xFF) {
        splitOkB = 1;
    } else {
        splitOkB = FieldEventSplitSet(
            g_CharIdToEntity[Savemap.memory_bank_2[11]],
            FieldEventReadMemoryS16(4, 9), FieldEventReadMemoryS16(5, 11),
            FieldEventReadMemoryU8(6, 13) & 0xFF, GET_PARAM_U8(14));
    }
    if (splitOkA && splitOkB) {
        for (i = 0; i < 3; i++) {
            charId = Savemap.memory_bank_2[9 + i];
            if (charId != 0xFF) {
                g_EntitySplitJoinState[g_CharIdToEntity[charId]] = 0;
                if (i == 0) {
                    modelId = g_CharIdToEntity[charId];
                    if (modelId != 0xFF) {
                        g_FieldModels[g_EntityToModel[modelId]].SolidOff = 0;
                    }
                }
            }
        }
        g_FieldState->characterLock = g_CharacterLock;
        g_EntityForSplitJoin = 0xFF;
        PC_INC(15);
        return 0;
    }
    g_FieldState->characterLock = 1;
    if (Savemap.memory_bank_2[9] != 0xFF) {
        modelId = g_CharIdToEntity[Savemap.memory_bank_2[9]];
        if (modelId != 0xFF) {
            g_EntitySplitJoinState[modelId] = 1;
            g_FieldModels[g_EntityToModel[modelId]].scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[modelId]].ActionState = 0;
            g_FieldModels[g_EntityToModel[modelId]].SolidOff = 1;
        }
    }
    return 1;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventJoinSet);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSplitSet);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSplitJoinSetMove);

/* Poll one party member's walk during a SPLIT or JOIN. ActionState 2 means the
 * move just finished, so release the scripted-move lock and restore the
 * model's default speed. */
s32 FieldEventSplitJoinEndMove(s16 entityId) {
    if (g_FieldModels[g_EntityToModel[entityId]].ActionState != 2) {
        return 0;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("end move", 0, 0);
    }
    g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode = 0;
    g_FieldModels[g_EntityToModel[entityId]].ActionState = 0;
    D_800756E8[g_EntityToModel[entityId]] = 0;
    g_FieldModels[g_EntityToModel[entityId]].MoveSpeed =
        D_800E42A8[g_EntityToModel[entityId]];
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldEventSplitJoinSetTurn);

/* Poll one party member's turn during a SPLIT or JOIN. Returns 1 once the
 * entity has finished turning -- or has no model to turn -- and 0 while it is
 * still in progress. */
s32 FieldEventSplitJoinEndTurn(s16 entityId) {
    if (g_EntityToModel[entityId] == 0xFF) {
        return 1;
    }
    if (g_FieldModels[g_EntityToModel[entityId]].TurnType != 3) {
        return 0;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("end turn", 0, 0);
    }
    g_FieldModels[g_EntityToModel[entityId]].TurnType = 0;
    g_FieldModels[g_EntityToModel[entityId]].TurnStep = 0;
    g_FieldModels[g_EntityToModel[entityId]].TurnSteps = 0;
    return 1;
}

/////////////////////////////////////////////////
// Begin of field_opcode_fade.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncFade);

/* Every instruction matches except one: the original leaves the delay slot of
 * the first FieldEventReadMemoryU8 call empty and stores fadeType ahead of it,
 * where gcc sinks that store into the slot and comes out one instruction short.
 * It fills the slot the same way for the later three calls. Neither a temp for
 * the parameter nor the do/while barrier that fixes OpcodeFuncWsize helps --
 * the barrier costs six more instructions by breaking the g_FieldState CSE. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncNfade);
#else
s32 OpcodeFuncNfade(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("nfade", 8);
    }
    g_FieldState->fadeType = GET_PARAM_U8(3);
    g_FieldState->nFadeRedTarget = FieldEventReadMemoryU8(1, 4);
    g_FieldState->nFadeGreenTarget = FieldEventReadMemoryU8(2, 5);
    g_FieldState->nFadeBlueTarget = FieldEventReadMemoryU8(3, 6);
    g_FieldState->fadeAdjust = 0;
    g_FieldState->fadeSpeed = FieldEventReadMemoryS16(4, 7);
    PC_INC(9);
    return 0;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncFadew);

/////////////////////////////////////////////////
// Begin of field_opcode_intersect.c
/////////////////////////////////////////////////

/* IDLCK: set or clear the "player may not cross this walkmesh edge" bit for
 * one triangle. blockedAccesses is a bitfield, eight triangles per byte. */
s32 OpcodeFuncIdlck(void) {
    s16 triId;
    s32 byteIdx;
    s32 bitIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("idlck", 3);
    }
    GET_PARAM_S16(triId, 1);
    byteIdx = triId / 8;
    bitIdx = triId - byteIdx * 8;
    if (GET_PARAM_U8(3)) {
        g_FieldState->blockedAccesses[byteIdx] |= 1 << bitIdx;
    } else {
        g_FieldState->blockedAccesses[byteIdx] &= ~(1 << bitIdx);
    }
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_window_color.c
/////////////////////////////////////////////////

s32 OpcodeFuncGwcol(void) {
    s16 pane;
    s32 corner;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gwcol", 6);
    }
    pane = FieldEventReadMemoryU8(1, 3);
    corner = pane * 3;
    FieldEventWriteMemoryU8(2, 4, D_80049208[corner]);
    FieldEventWriteMemoryU8(3, 5, D_80049208[corner + 1]);
    FieldEventWriteMemoryU8(4, 6, D_80049208[corner + 2]);
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncSwcol(void) {
    s16 pane;
    s32 corner;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("swcol", 6);
    }
    pane = FieldEventReadMemoryU8(1, 3);
    corner = pane * 3;
    D_80049208[corner] = FieldEventReadMemoryU8(2, 4);
    D_80049208[corner + 1] = FieldEventReadMemoryU8(3, 5);
    D_80049208[corner + 2] = FieldEventReadMemoryU8(4, 6);
    PC_INC(7);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_field_effect.c
/////////////////////////////////////////////////

s32 OpcodeFuncLstmp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lstmp", 2);
    }
    FieldEventWriteMemoryS16(2, 2, g_FieldState->prevFieldId);
    PC_INC(3);
    return 0;
}

/* SHAKE: arm the randomized camera shake on either axis. Bit 0 of parameter 3
 * enables the X shake, bit 1 the Y shake; a clear bit disables that axis. */
s32 OpcodeFuncShake(void) {
    s32 axes;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("shake", 7);
    }
    axes = GET_PARAM_U8(3);
    if (axes & 1) {
        g_FieldState->shakeX.enabled = 1;
        g_FieldState->shakeX.amplitude = FieldEventReadMemoryU8(1, 4);
        g_FieldState->shakeX.numStepsPerSegment = FieldEventReadMemoryU8(2, 5);
    } else {
        g_FieldState->shakeX.enabled = 0;
    }
    if (axes & 2) {
        g_FieldState->shakeY.enabled = 1;
        g_FieldState->shakeY.amplitude = FieldEventReadMemoryU8(3, 6);
        g_FieldState->shakeY.numStepsPerSegment = FieldEventReadMemoryU8(4, 7);
    } else {
        g_FieldState->shakeY.enabled = 0;
    }
    PC_INC(8);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_items.c
/////////////////////////////////////////////////

s32 OpcodeFuncStitm(void) {
    s32 itemHi;
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stitm", 4);
    }
    itemHi = FieldEventReadMemoryU8(2, 4);
    itemId = (itemHi & 0xFF) << 9;
    itemId |= FieldEventReadMemoryS16(1, 2);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S item=", itemId, 4);
    }
    func_80025380(itemId);
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncDlitm(void) {
    s32 itemHi;
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dlitm", 4);
    }
    itemHi = FieldEventReadMemoryU8(2, 4);
    itemId = (itemHi & 0xFF) << 9;
    itemId |= FieldEventReadMemoryS16(1, 2);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G item=", itemId, 4);
    }
    func_80025288(itemId);
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncCkitm(void) {
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ckitm", 4);
    }
    itemId = func_80025310(FieldEventReadMemoryS16(1, 2));
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("C item=", itemId, 4);
    }
    if (itemId == 0xFFFF) {
        itemId = 0;
    }
    FieldEventWriteMemoryU8(2, 4, itemId >> 9);
    PC_INC(5);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_special.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncSpcal);

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

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMenu);
#else
s32 OpcodeFuncMenu(void) {
    if (g_DebugLevel & 3) {
        func_800BEAD4("menu", 3);
    }
    if (g_DebugLevel & 3) {
        func_800BECA4("evt cmd=", g_FieldState->eventCmd, 2);
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
                "evt result=", g_FieldState->movieCommandState, 1);
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
#endif

#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", OpcodeFuncMenu2);
#else
s32 OpcodeFuncMenu2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("menu", 1);
    }
    g_FieldState->menuDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}
#endif

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

static void DebugPrintToFieldWindow(const char* str) {
    // used to print debug messages -- dummied out on release
}

static void FieldEventDebugError(const char* errmsg) {
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

static void PlayWindowPointerClickSound(void) {
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldDebugInitBuffers);

static void InitFieldDebugPages(void) {
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

/* gcc hoists the array's %hi/%lo into a register because the same address is
 * read and written; the original rematerialises it through $at each time. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldDebugPageAddPos);
#else
void FieldDebugPageAddPos(s16 page, s16 x, s16 y) {
    D_8009D824 = 1;
    D_800E0748[page * 189] += x;
    D_800E074A[page * 189] += y;
}
#endif

/* Same $at rematerialisation residue as FieldDebugPageAddPos above. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldDebugPageAddSize);
#else
void FieldDebugPageAddSize(s16 page, s16 w, s16 h) {
    D_8009D824 = 1;
    D_800E074C[page * 189] += w;
    D_800E074E[page * 189] += h;
}
#endif

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

static void FieldDebugRenderClear(void) {
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

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldDebugRenderPage);

INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldDebugRenderString);

INCLUDE_ASM("asm/us/field/nonmatchings/field", AddStrNextDebugRow);

/* Append a coloured line to a debug page, wrapping back to the top row once the
 * page's pixel height can no longer hold another 10-pixel row.
 *
 * Semantically right, not yet matching. One root cause behind the register
 * renames: the original keeps only `page * 378` in a callee-saved register and
 * lets the assembler rebuild `&D_800E0754 + that` through $at on each of the
 * five accesses, where gcc CSEs the whole address into a second callee-saved
 * register. Dropping the `colors` local removed three spurious instructions and
 * fixed the frame size; the address CSE is what is left. */
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", AddColorStrNextDebugRow);
#else
s32 AddColorStrNextDebugRow(s16 page, const char* str, u8 color) {
    char* rows = D_800E0758 + page * 378;

    FieldDebugStringCopy(&rows[D_800E0754[page * 189] * 14], str);
    D_800E08A8[page * 378 + D_800E0754[page * 189]] = color;
    D_800E0754[page * 189]++;
    if ((D_800E074E[page * 189] - 8) / 10 < D_800E0754[page * 189]) {
        D_800E0754[page * 189] = 0;
    }
    D_8009D824 = 1;
    return 1;
}
#endif

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

static void FieldDebugStringCopy(char* dst, const char* src) {
    if (*src) {
        do {
            *dst++ = *src++;
        } while (*src != '\0');
    }
    *dst = '\0';
}

static void FieldDebugStringConcat(char* dest, char* src) {
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

static void FieldDebugStringU8hex(s32 val, char* msg_out) {
    msg_out[1] = '\0';
    msg_out[0] = g_FieldDebugDigits[val & 0xF];
}

static void FieldDebugStringU16hex(s32 val, char* msg_out) {
    msg_out[2] = '\0';
    msg_out[0] = g_FieldDebugDigits[(val & 0xF0) >> 4];
    msg_out[1] = g_FieldDebugDigits[val & 0xF];
}

static void FieldDebugStringU32hex(s32 val, char* msg_out) {
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
