#ifndef FIELD_PRIVATE_H
#define FIELD_PRIVATE_H

/* Shared by src/field/field.c and field2.c..field5.c. Those five files were
 * one translation unit until the jump-table alignment split; the split is
 * forced by .rodata alignment, not by module structure, so this header is
 * simply field.c's old preamble. See the comment atop each unit and the
 * `field` segment in config/us.yaml. */

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

/* The two bytes FieldBackgroundInitPackets copies out of each layer 2/3/4
 * tile. A sprite is linked into the OT only when the named entity's
 * background-trigger byte still has this bit set. */
typedef struct {
    /* 0x00 */ u8 entity; // low 6 bits index g_FieldEntityBgTrigger
    /* 0x01 */ u8 mask;   // bit tested against that entity's byte
} FieldBgAnimPair;

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

    FieldBgAnimPair BgAnim[0xbc4]; // 0x10d54: Per-sprite animation gate
    DR_MODE BgDm[0x6a4];           // 0x124dc: Background draw mode packets

    OT_TYPE OtUi;       // 0x1748c: UI ordering table
    DR_MODE RainDm;     // 0x17490: Rain draw mode
    LINE_F2 Rain[0x40]; // 0x1749c: Rain line primitives
};
extern struct FieldRenderData g_FieldRenderData[2]; // double buffered

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
extern s16 g_FieldDebugPageY[];
extern s16 D_800E074C[];
extern s16 g_FieldDebugPageH[];
extern u8 g_FieldDebugPageR[];
extern u8 g_FieldDebugPageG[];
extern u8 g_FieldDebugPageB[];
extern s16 g_FieldDebugPageHeadRow[];
extern s16 g_FieldDebugPageRow[];
extern char g_FieldDebugRowText[];
extern u8 D_800E08A8[];
extern u8 g_FieldDebugPageHidden[];
extern u8 D_800E1028[];
extern u8 D_800E3B28[];
extern u8 D_800E3FA8[];
extern u8 D_800E4128[];
extern u16 D_800E4200[];
extern u8 g_FieldMimPalData[];
extern u8 g_FieldMimPalSize[];
extern u8 g_FieldMimPalX[];
extern u8 g_FieldMimPalY[];
extern u8 g_FieldMimPalW[];
extern u8 g_FieldMimPalH[];
extern u8 g_FieldMimTex0Data[];
extern u8 g_FieldMimTex0Size[];
extern u8 g_FieldMimTex0X[];
extern u8 g_FieldMimTex0Y[];
extern u8 g_FieldMimTex0Rect[];
extern u8 g_FieldMimTex0Tpage[];
extern u8 g_FieldMimTex1Data[];
extern u8 g_FieldMimTex1Size[];
extern u8 g_FieldMimTex1X[];
extern u8 g_FieldMimTex1Y[];
extern u8 g_FieldMimTex1Rect[];
extern u8 g_FieldMimTex1Tpage[];
extern u8 D_800DFDFC[];
extern u8 g_FieldRandomIndex;
extern u8 g_EntityForSplitJoin;
extern s16 g_FieldDirVectors[][2];
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
extern u32
    g_FieldModelBufferTop; // top of the buffer the movie stream decodes into
extern u16 g_FieldModelSavedMoveSpeed[]; // per-model default walk speed,
                                         // indexed by model id
extern s16 g_FieldMovieStreamDone;
void func_80034FC8(u32 buffer, s16 movieId); // STR ring setup
void func_800354CC(void);                    // STR playback start
void func_80035658(void);                    // STR playback stop
extern s32 D_8009A010;
extern s32 D_8009A014;
extern s16 D_801144D4;
extern u8 D_80095DE0[];
/* "evt cmd=" and "evt result=", written into field4.c's .rodata by
 * OpcodeFuncMjump and OpcodeFuncTutor and shared with OpcodeFuncMenu over in
 * field5.c. See the comment there. */
extern char D_800A0848[];
extern char D_800A08D0[];
extern s32 g_BattleCharIdToCharId[11];
extern u8 g_CharIdToEntity[];
extern FieldState g_FieldStateData;
extern u8 g_FieldRandListIndex;
extern u8 g_FieldRandListOffset;
extern s32 D_8009A108;
extern s32 D_80099FCC[];
extern u8 g_FieldCameraMatrixSel;
extern u8 g_FieldAnimLock;
extern u8 g_FieldAnimFreeze;
extern u8 D_80081DC4;
extern s16 g_FieldExitArrowX;
extern s16 g_FieldExitArrowY;
extern u8 g_FieldExitArrowPktIdx;
// Two POLY_FT4 at 0x800E48F4, filling the gap between g_EntityForSplitJoin
// (0x800E48F0) and g_WindowString (0x800E4944) exactly. Confirmed by the
// 0x28 stride, len 9 / code 0x2C at +3 / +7, clut at +0x0E and tpage at +0x16.
extern POLY_FT4 g_FieldExitArrowPkts[2];
s32 GetGraphType(void);
extern MATRIX** D_80083578;
extern MATRIX* g_DebugMatrixP;
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
s32 FieldEventSetDirByActorId(s16 actorId);
s32 FieldMoveToEntityUpdate(s32 targetEntityId);
s32 FieldEntityTurnToEntity(s16 actorId);
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
s32 FieldEventRequest(s16 type, u8 target, u8 priority, u8 scriptId);
void FieldWindowReset(s16 window);
void FieldWindowResetTextAll(void);
s32 AddStrNextDebugRow(s16 page, const char* str);
s32 SetStrToDebugRow(s16 page, s16 row, const char* str);
void DebugUpdateActor(s16 arg0, s16 actorId);
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

/* Were `static` before the split: each is now called from a unit other
 * than the one defining it, several of them from INCLUDE_ASM bodies. */
s32 KeyCheck(u16 keys);
u32 GetAkaoBlockOffset(s16 akaoId);
void DebugPrintToFieldWindow(const char* str);
void FieldDebugStringConcat(char* arg0, char* arg1);
/* Defined in field4.c; the debug tracer in field5.c calls it across the split.
 */
void FieldDebugAddParseValueToPage2(const char* str, s32 val, s32 kind);
void FieldDebugStringCopy(char* dst, const char* src);
void FieldDebugStringU16hex(s32 val, char* msg_out);
void FieldDebugStringU32hex(s32 val, char* msg_out);
void FieldDebugStringU8hex(s32 val, char* msg_out);
void FieldEventDebugError(const char* errmsg);
void InitFieldDebugPages(void);
void PartyAdd(u8* party, u8* toAdd);
void PartyFromBank2ToSave(s32 unused);
void PartyRemove(u8* party, u8* toRemove);
void PartyReplace(u8* newParty);
void PlayWindowPointerClickSound(void);

/* Declared in a unit body before the split. */
extern s16 D_80071A5C;
extern s16 D_8009AC1C;
extern s16 D_800E42EE[0x40][12];
extern s16 g_CurrentFieldIndex;
extern s16 g_PlayerModelId;
extern s32 D_8007E770;
/* One field map's encounter table: a control word, five ordinary formations
 * and four "special" ones. Each formation word packs a cumulative six-bit
 * rate in its top bits and the formation id in the low ten. Two of these sit
 * back to back and BTLTB switches between them, which is why the second is
 * reached as +0x18. */
typedef struct {
    /* 0x00 */ u16 control; /* bit 0 enables encounters, high byte is the step
                               divisor */
    /* 0x02 */ u16 encounters[5];
    /* 0x0C */ u16 fallback;
    /* 0x0E */ u16 special[4];
    /* 0x16 */ u16 pad;
} FieldEncounterTable; /* size:0x18 */

extern s32 g_FieldEncounters;
extern s32 g_FieldTriggers;
extern s32 g_WmPreSector;
extern s32* g_FieldEncountersP;
extern s32* g_FieldModelsP;
extern s32* g_FieldTriggersP;
extern volatile u16 g_FieldMoviePlayed;
extern u16 g_FieldPreloadMapId;
extern u32 g_FieldLzsInfo[];
extern u32 g_WmPreSize;
extern u8 g_FieldMusicLock;
extern u8 D_8009ABF5;
extern u8 g_FieldAnimLock;
extern u8 g_RainControl;
extern u8 g_RainForce;
extern u8 g_RandomTable[];

/* Header of the field's model-loader file (FieldModelStructInit parses it,
 * FieldEnablePartyModels reads its count). Shared: defined in field2.c,
 * used from field4.c, and the units must all see it before first use --
 * pycparser cannot prune a permuter scratch past a cast to an unknown type,
 * which is what silently leaves the whole unit in base.c. */
typedef struct {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u16 count;
    /* 0x04 */ FieldModelLoaderData models[0]; // variable length
} FieldModelFileDesc;

/* The field background's tile tables. One header sits at the head of the
 * loaded background block; each layer's tiles are a flat array behind one of
 * its four byte offsets, and the run list at +0x10 says how many consecutive
 * tiles share a texture page. A run is three s16 -- tag, sprite index, count
 * -- except the 0x7FFF terminator, which is a bare tag. */
typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ u8 u;
    /* 0x05 */ u8 v;
    /* 0x06 */ u16 clut;
} FieldBgTile1; // size:0x8

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ u8 u;
    /* 0x05 */ u8 v;
    /* 0x06 */ u16 clut;
    /* 0x08 */ u16 tpage;
    /* 0x0A */ u16 rg; // red in the low byte, green in the high one
    /* 0x0C */ u8 flags;
    /* 0x0D */ u8 param;
} FieldBgTile2; // size:0xE

typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ u8 u;
    /* 0x05 */ u8 v;
    /* 0x06 */ u16 clut;
    /* 0x08 */ u8 flags;
    /* 0x09 */ u8 param;
} FieldBgTile3; // size:0xA

typedef struct {
    /* 0x00 */ u32 layer1Offset;
    /* 0x04 */ u32 tpageOffset;
    /* 0x08 */ u32 layer2Offset;
    /* 0x0C */ u32 layer34Offset;
    /* 0x10 */ s16 runs[0]; // variable length
} FieldBgData;

/* Header of the shared field-model texture block at *D_800DFCA0. */
typedef struct {
    /* 0x0 */ u32 magic;
    /* 0x4 */ u16 numPages;   // 0x200-byte texture pages
    /* 0x6 */ u16 numCluts;   // 0x20-byte CLUTs
    /* 0x8 */ u32 pageOffset; // offset of the pages within the block
    /* 0xC */ u32 clutOffset; // offset of the CLUTs within the block
} FieldTexBlockHeader;

/* The block itself is loaded to 0x80128000; the pointer is set by
 * FieldModelLoadAndInit and read from both field2.c and field4.c, so the type
 * has to live here -- an extern whose type is declared in one unit becomes a
 * tentative definition in the other and only fails at link time. */
extern FieldTexBlockHeader* D_800DFCA0;

#endif /* FIELD_PRIVATE_H */
