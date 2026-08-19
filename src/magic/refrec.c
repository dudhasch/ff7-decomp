//! PSYQ=3.3 CC1=2.6.3

// MAGIC/REFREC.BIN -- "Reflect": bounces the next four reflectable spells
// back at their caster, from Barrier Materia at level 3. Japanese リフレク.
// See docs/spells/magic/reflect.md.

#include "common.h"
#include "../battle/battle.h"

// Battle effect instance, as this overlay lays it out. Same 0x20-byte slot the
// other MAGIC overlays share, with its own field assignment: the reflect ring
// carries its scale and per-frame scale step in the instance rather than
// deriving them from the frame counter.
typedef struct ReflectData {
    s16 StartFrame;
    s16 AnimationFrame;
    SVECTOR Pos;
    SVECTOR Rot;
    s16 Scale;
    s16 ScaleStep;
    s16 TargetId;
    char pad1A[0x6];
} ReflectData;

// Battle effect instances.
extern ReflectData D_80162978[];

// Embedded model, same format as barrier.c's bari_a1/bari_a2: a vertex-data
// size in bytes, the vertex table (8 bytes per vertex: x, y, z, pad), then the
// primitive groups. A primitive is a pair of vertex-index words followed by a
// GPU command word (0x30 = POLY_G3, 0x38 = POLY_G4) and its vertex colours.
// 0xC8 bytes = 25 vertices, spread over several groups.
static s32 refrec_a1[] = {
    0x000000C8, 0x00000000, 0x0000FD8C, 0x0000FF93, 0x0000FE24, 0xFEE20000,
    0x0000FE70, 0x0000006D, 0x0000FE24, 0xFF5B003F, 0x0000FE29, 0xFF5BFFC1,
    0x0000FE29, 0x00000030, 0x0000FDC8, 0x0000FFD0, 0x0000FDC8, 0xFF810000,
    0x0000FDC8, 0xFF61006C, 0x0000FE83, 0xFEBC003C, 0x0000FF02, 0xFF61FF94,
    0x0000FE83, 0xFEBCFFC4, 0x0000FF02, 0x00000087, 0x0000FE59, 0xFE030000,
    0x00000008, 0x0000FF79, 0x0000FE59, 0x01FD0000, 0x00000008, 0x0144FFC4,
    0x0000FF02, 0x009FFF94, 0x0000FE83, 0x0144003C, 0x0000FF02, 0x009F006C,
    0x0000FE83, 0x007F0000, 0x0000FDC8, 0x00A5FFC1, 0x0000FE29, 0x00A5003F,
    0x0000FE29, 0x011E0000, 0x0000FE70, 0x00200000, 0x00000000, 0x00000048,
    0x003800A8, 0x000000B0, 0x3000FFFF, 0x0000FFFF, 0x0000FF80, 0x00A80030,
    0x000000B8, 0x3000FFFF, 0x0000FFFF, 0x0000FF80, 0x00300018, 0x000000B8,
    0x3000FF80, 0x0000FFFF, 0x0000FF80, 0x00A80000, 0x00000030, 0x30FFFFFF,
    0x0000FFFF, 0x0000FFFF, 0x00B800C0, 0x000000A8, 0x3000FF80, 0x0000FF80,
    0x0000FFFF, 0x00A800C0, 0x000000B0, 0x3000FF80, 0x0000FFFF, 0x0000FF80,
    0x00380000, 0x000000A8, 0x30FFFFFF, 0x0000FFFF, 0x0000FFFF, 0x00B00008,
    0x00000038, 0x3000FF80, 0x0000FF80, 0x0000FFFF, 0x00B80018, 0x000000A0,
    0x3000FF80, 0x0000FF80, 0x00008000, 0x00B800A0, 0x00000098, 0x30008000,
    0x0000FF80, 0x00008000, 0x00C000B8, 0x00000098, 0x3000FF80, 0x0000FF80,
    0x00008000, 0x000800B0, 0x00000090, 0x3000FF80, 0x0000FF80, 0x00008000,
    0x00B00088, 0x00000090, 0x30008000, 0x0000FF80, 0x00008000, 0x00B000C0,
    0x00000088, 0x3000FF80, 0x0000FF80, 0x00008000, 0x00180068, 0x000000A0,
    0x30008000, 0x0000FF80, 0x00008000, 0x00C00080, 0x00000088, 0x30008000,
    0x0000FF80, 0x00008000, 0x00C00098, 0x00000080, 0x30008000, 0x0000FF80,
    0x00008000, 0x00080090, 0x00000078, 0x30008000, 0x0000FF80, 0x00008000,
    0x00080078, 0x00000058, 0x30008000, 0x0000FF80, 0x00008000, 0x00100070,
    0x00000050, 0x30008000, 0x0000FF80, 0x00008000, 0x00100060, 0x00000070,
    0x30008000, 0x0000FF80, 0x00008000, 0x00180048, 0x00000068, 0x30008000,
    0x0000FF80, 0x00008000, 0x00280060, 0x00000010, 0x30008000, 0x0000FF80,
    0x0000FF80, 0x00280058, 0x00000060, 0x30008000, 0x0000FF80, 0x00008000,
    0x00080058, 0x00000028, 0x30008000, 0x0000FF80, 0x0000FF80, 0x00100050,
    0x00000020, 0x30008000, 0x0000FF80, 0x0000FF80, 0x00200050, 0x00000048,
    0x30008000, 0x0000FF80, 0x00008000, 0x00200048, 0x00000018, 0x30008000,
    0x0000FF80, 0x0000FF80, 0x00280038, 0x00000008, 0x3000FFFF, 0x0000FF80,
    0x0000FF80, 0x00380040, 0x00000000, 0x3000FFFF, 0x0000FFFF, 0x00FFFFFF,
    0x00400028, 0x00000010, 0x3000FF80, 0x0000FFFF, 0x0000FF80, 0x00200040,
    0x00000010, 0x3000FFFF, 0x0000FF80, 0x0000FF80, 0x00400030, 0x00000000,
    0x3000FFFF, 0x0000FFFF, 0x00FFFFFF, 0x00300020, 0x00000018, 0x3000FF80,
    0x0000FFFF, 0x0000FF80, 0x00400020, 0x00000030, 0x3000FF80, 0x0000FFFF,
    0x0000FFFF, 0x00380028, 0x00000040, 0x3000FF80, 0x0000FFFF, 0x0000FFFF,
    0x00B000A8, 0x00000038, 0x3000FFFF, 0x0000FF80, 0x0000FFFF, 0x00B80030,
    0x000000A8, 0x3000FFFF, 0x0000FF80, 0x0000FFFF, 0x00B80018, 0x00000030,
    0x3000FF80, 0x0000FF80, 0x0000FFFF, 0x00300000, 0x000000A8, 0x30FFFFFF,
    0x0000FFFF, 0x0000FFFF, 0x00A800C0, 0x000000B8, 0x3000FF80, 0x0000FFFF,
    0x0000FF80, 0x00B000C0, 0x000000A8, 0x3000FF80, 0x0000FF80, 0x0000FFFF,
    0x00A80000, 0x00000038, 0x30FFFFFF, 0x0000FFFF, 0x0000FFFF, 0x00380008,
    0x000000B0, 0x3000FF80, 0x0000FFFF, 0x0000FF80, 0x00A00018, 0x000000B8,
    0x3000FF80, 0x00008000, 0x0000FF80, 0x009800A0, 0x000000B8, 0x30008000,
    0x00008000, 0x0000FF80, 0x009800B8, 0x000000C0, 0x3000FF80, 0x00008000,
    0x0000FF80, 0x009000B0, 0x00000008, 0x3000FF80, 0x00008000, 0x0000FF80,
    0x00900088, 0x000000B0, 0x30008000, 0x00008000, 0x0000FF80, 0x008800C0,
    0x000000B0, 0x3000FF80, 0x00008000, 0x0000FF80, 0x00A00068, 0x00000018,
    0x30008000, 0x00008000, 0x0000FF80, 0x00880080, 0x000000C0, 0x30008000,
    0x00008000, 0x0000FF80, 0x00800098, 0x000000C0, 0x30008000, 0x00008000,
    0x0000FF80, 0x00780090, 0x00000008, 0x30008000, 0x00008000, 0x0000FF80,
    0x00580078, 0x00000008, 0x30008000, 0x00008000, 0x0000FF80, 0x00500070,
    0x00000010, 0x30008000, 0x00008000, 0x0000FF80, 0x00700060, 0x00000010,
    0x30008000, 0x00008000, 0x0000FF80, 0x00680048, 0x00000018, 0x30008000,
    0x00008000, 0x0000FF80, 0x00100060, 0x00000028, 0x30008000, 0x0000FF80,
    0x0000FF80, 0x00600058, 0x00000028, 0x30008000, 0x00008000, 0x0000FF80,
    0x00280058, 0x00000008, 0x30008000, 0x0000FF80, 0x0000FF80, 0x00200050,
    0x00000010, 0x30008000, 0x0000FF80, 0x0000FF80, 0x00480050, 0x00000020,
    0x30008000, 0x00008000, 0x0000FF80, 0x00180048, 0x00000020, 0x30008000,
    0x0000FF80, 0x0000FF80, 0x00080038, 0x00000028, 0x3000FFFF, 0x0000FF80,
    0x0000FF80, 0x00000040, 0x00000038, 0x3000FFFF, 0x00FFFFFF, 0x0000FFFF,
    0x00100028, 0x00000040, 0x3000FF80, 0x0000FF80, 0x0000FFFF, 0x00100040,
    0x00000020, 0x3000FFFF, 0x0000FF80, 0x0000FF80, 0x00000030, 0x00000040,
    0x3000FFFF, 0x00FFFFFF, 0x0000FFFF, 0x00180020, 0x00000030, 0x3000FF80,
    0x0000FF80, 0x0000FFFF, 0x00300020, 0x00000040, 0x3000FF80, 0x0000FFFF,
    0x0000FFFF, 0x00400028, 0x00000038, 0x3000FF80, 0x0000FFFF, 0x0000FFFF,
    0x00000000,
};

static Unk801B0C98 ReflectRenderDesc = {refrec_a1, 0x88, 0, 0, 0x20};
static s32 ReflectBaseScale;
static s32 ReflectBlend;
static char ReflectPrimBuffer[0x20000];
static void* ReflectBufferPtr;

// refrec.c forward declarations
static void ReflectMainSetup(int arg0, int arg1);

void MAGIC_Reflect(int arg0, int arg1) { ReflectMainSetup(arg0, arg1); }

static void ReflectRenderShield(void) {
    VECTOR scale;
    MATRIX matrix;
    ReflectData* effect = &D_80162978[D_8015169C];
    int var_s2;

    scale.vx = scale.vy = scale.vz = (effect->Scale * ReflectBaseScale) >> 12;

    if (effect->AnimationFrame < 8) {
        var_s2 = 0;
    } else {
        var_s2 = (effect->AnimationFrame - 8) << 9;
    }

    var_s2 += ((0x1000 - var_s2) * ReflectBlend) >> 12;

    RotMatrixYXZ(&effect->Rot, &matrix);
    matrix.t[0] = effect->Pos.vx;
    matrix.t[1] = effect->Pos.vy;
    matrix.t[2] = effect->Pos.vz;
    ScaleMatrix(&matrix, &scale);
    CompMatrix(&D_800FA63C.m, &matrix, &matrix);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    SetFarColor(0, 0, 0);

    ReflectRenderDesc.unkA = var_s2;
    ReflectBufferPtr =
        func_800D29D4(&ReflectRenderDesc, g_cDb->unk70, 12, ReflectBufferPtr);

    if (D_80062D98 == 0) {
        effect->AnimationFrame++;
        if (effect->AnimationFrame >= 16) {
            effect->StartFrame = -1;
        } else {
            effect->Scale += effect->ScaleStep;
        }
    }
}

static void ReflectSequenceManage(void) {
    ReflectData* effect = &D_80162978[D_8015169C]; // model instance
    ReflectData* next;

    if (D_80062D98 != 0) {
        return;
    }

    if (effect->AnimationFrame == 0) {
        next = &D_80162978[func_800BBEAC(ReflectRenderShield)];
        next->Pos = effect->Pos;
        next->Rot = effect->Rot;
        next->Scale = 0x200;
        next->ScaleStep = 0x160;
    }

    if (effect->AnimationFrame == 8) {
        next = &D_80162978[func_800BBEAC(ReflectRenderShield)];
        next->Pos = effect->Pos;
        next->Rot = effect->Rot;
        next->Scale = 0x200;
        next->ScaleStep = 0x180;
    }

    if (effect->AnimationFrame == 16) {
        next = &D_80162978[func_800BBEAC(ReflectRenderShield)];
        next->Pos = effect->Pos;
        next->Rot = effect->Rot;
        next->Scale = 0x200;
        next->ScaleStep = 0x1A0;
    }

    if (effect->AnimationFrame == 24) {
        func_800D5774(effect->TargetId);
        effect->StartFrame = -1;
    }

    effect->AnimationFrame++;
}

static void ReflectAttachToTarget(int arg0) {
    ReflectData* effect = &D_80162978[func_800BBEAC(ReflectSequenceManage)];
    BattleModel* model = &D_801518E4[arg0];

    func_800D3994(arg0, model->D_8015190F, &effect->Pos);
    effect->Pos.vx -= (rsin(model->unk160.vy) * model->unk12) >> 12;
    effect->Pos.vz -= (rcos(model->unk160.vy) * model->unk12) >> 12;
    effect->Rot = model->unk160;
    effect->TargetId = arg0;
}

static void ReflectDoubleBufferFlip(void) {
    ReflectData* effect = &D_80162978[D_8015169C];

    ReflectBufferPtr = &ReflectPrimBuffer[effect->AnimationFrame * 65536];
    effect->AnimationFrame ^= 1;

    if (D_80162080 < 2) {
        effect->StartFrame = -1;
    }
}

static void ReflectMainSetup(int arg0, int arg1) {
    ReflectBaseScale = 0x2000;
    ReflectBlend = 0;
    func_800BBEAC(ReflectDoubleBufferFlip);
    func_800D5444(arg0, arg1, 0, ReflectAttachToTarget);
    func_800D55F4(0x20, 0x40, 0x48);
}
