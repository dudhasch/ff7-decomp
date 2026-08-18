//! PSYQ=3.3 CC1=2.6.3

#include "common.h"
#include "../battle/battle.h"

// Battle effect instance, as this overlay lays it out. Same 0x20-byte slot as
// the one BARRIER.BIN uses, but the fields sit at different offsets.
typedef struct MBarrierData {
    s16 StartFrame;
    s16 AnimationFrame;
    SVECTOR Pos;
    SVECTOR Rot;
    s16 TargetId;
    char pad16[0xA];
} MBarrierData;

// Battle effect instances.
extern MBarrierData D_80162978[];

// Embedded model, same format as barrier.c's bari_a1/bari_a2: a vertex-data
// size in bytes, the vertex table (8 bytes per vertex: x, y, z, pad), then the
// primitive groups. A primitive is a pair of vertex-index words followed by a
// GPU command word (0x30 = POLY_G3, 0x38 = POLY_G4) and its vertex colours.
// 0x108 bytes = 33 vertices; unlike the barrier models this one has several
// groups, which is why it is transcribed rather than annotated per row.
static s32 mbari_a1[] = {
    0x00000108, 0x000000FA, 0x0000FFFB, 0xFF5000B0, 0x0000FFFB, 0xFF060000,
    0x0000FFFB, 0x00000000, 0x0000FF00, 0xFFEF0007, 0x0000FF14, 0xFF0E0008,
    0x0000FFF6, 0xFF4F00A5, 0x0000FFF6, 0xFFF90011, 0x0000FF14, 0xFF5B00B1,
    0x0000FFF6, 0xFFF800F2, 0x0000FFF6, 0x000800F2, 0x0000FFF6, 0x00A500B1,
    0x0000FFF6, 0x00070011, 0x0000FF14, 0x00B100A5, 0x0000FFF6, 0x00F20008,
    0x0000FFF6, 0x00110007, 0x0000FF14, 0x00FA0000, 0x0000FFFB, 0x00B000B0,
    0x0000FFFB, 0x00B0FF50, 0x0000FFFB, 0x0011FFF9, 0x0000FF14, 0x00F2FFF8,
    0x0000FFF6, 0x00B1FF5B, 0x0000FFF6, 0x0007FFEF, 0x0000FF14, 0x00A5FF4F,
    0x0000FFF6, 0x0008FF0E, 0x0000FFF6, 0xFFF8FF0E, 0x0000FFF6, 0xFF5BFF4F,
    0x0000FFF6, 0xFFF9FFEF, 0x0000FF14, 0xFF4FFF5B, 0x0000FFF6, 0xFF0EFFF8,
    0x0000FFF6, 0xFFEFFFF9, 0x0000FF14, 0xFF50FF50, 0x0000FFFB, 0x0000FF06,
    0x0000FFFB, 0x00200000, 0x00000000, 0x00000010, 0x00F000E0, 0x000000E8,
    0x30800080, 0x0080FFFF, 0x000000FF, 0x00D800C8, 0x000000D0, 0x30FD6868,
    0x0080FFFF, 0x00FF0080, 0x00B000B8, 0x000000C0, 0x30FF0080, 0x0080FFFF,
    0x00FD6868, 0x009800A0, 0x000000A8, 0x300000FF, 0x0080FFFF, 0x00800080,
    0x00780068, 0x00000070, 0x30800080, 0x0080FFFF, 0x000000FF, 0x00600050,
    0x00000058, 0x30FD6868, 0x0080FFFF, 0x00FF0080, 0x00380040, 0x00000048,
    0x30FF0080, 0x0080FFFF, 0x00FF9494, 0x00200028, 0x00000030, 0x300000FF,
    0x0080FFFF, 0x00800080, 0x00E800E0, 0x000000F0, 0x30800080, 0x000000FF,
    0x0080FFFF, 0x00D000C8, 0x000000D8, 0x30FD6868, 0x00FF0080, 0x0080FFFF,
    0x00C000B8, 0x000000B0, 0x30FF0080, 0x00FD6868, 0x0080FFFF, 0x00A800A0,
    0x00000098, 0x300000FF, 0x00800080, 0x0080FFFF, 0x00700068, 0x00000078,
    0x30800080, 0x000000FF, 0x0080FFFF, 0x00580050, 0x00000060, 0x30FD6868,
    0x00FF0080, 0x0080FFFF, 0x00480040, 0x00000038, 0x30FF0080, 0x00FF9494,
    0x0080FFFF, 0x00300028, 0x00000020, 0x300000FF, 0x00800080, 0x0080FFFF,
    0x00000030, 0x00E80010, 0x00F00018, 0x38C0C0C0, 0x00808080, 0x00C0C0C0,
    0x00808080, 0x00E000F8, 0x00E80010, 0x38FFFFFF, 0x00808080, 0x00FFFFFF,
    0x00808080, 0x00E000F0, 0x00F80018, 0x38808080, 0x00808080, 0x00C0C0C0,
    0x00FFFFFF, 0x00D800D0, 0x001800F8, 0x38808080, 0x00808080, 0x00808080,
    0x00C0C0C0, 0x00C80100, 0x00D000F8, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00D80018, 0x00C80100, 0x38FFFFFF, 0x00808080, 0x00FFFFFF,
    0x00808080, 0x00C00100, 0x00B00018, 0x38555555, 0x00555555, 0x006A6A6A,
    0x00808080, 0x00B80090, 0x00C00100, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00180090, 0x00B000B8, 0x38555555, 0x00949494, 0x00808080,
    0x00808080, 0x00900018, 0x00A80098, 0x38949494, 0x00555555, 0x00808080,
    0x00808080, 0x00A00080, 0x00A80090, 0x38353535, 0x00808080, 0x00353535,
    0x00808080, 0x00980018, 0x00A00080, 0x38949494, 0x00808080, 0x00555555,
    0x00808080, 0x00700080, 0x00780018, 0x38555555, 0x00808080, 0x00949494,
    0x00808080, 0x00680088, 0x00700080, 0x38353535, 0x00808080, 0x00353535,
    0x00808080, 0x00680078, 0x00880018, 0x38808080, 0x00808080, 0x00949494,
    0x00555555, 0x00600058, 0x00180088, 0x38808080, 0x00808080, 0x00555555,
    0x00949494, 0x00500000, 0x00580088, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00600018, 0x00500000, 0x386A6A6A, 0x00808080, 0x00555555,
    0x00555555, 0x00480000, 0x00380018, 0x38FFFFFF, 0x00808080, 0x00FFFFFF,
    0x00808080, 0x00400008, 0x00480000, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00180008, 0x00380040, 0x38808080, 0x00C0C0C0, 0x00808080,
    0x00808080, 0x00080018, 0x00300020, 0x38C0C0C0, 0x00FFFFFF, 0x00808080,
    0x00808080, 0x00280010, 0x00300008, 0x38FFFFFF, 0x00808080, 0x00C0C0C0,
    0x00808080, 0x00200018, 0x00280010, 0x38EEEEEE, 0x00808080, 0x00C0C0C0,
    0x00808080, 0x00180010, 0x00F000E8, 0x38C0C0C0, 0x00C0C0C0, 0x00808080,
    0x00808080, 0x001000F8, 0x00E800E0, 0x38FFFFFF, 0x00FFFFFF, 0x00808080,
    0x00808080, 0x001800F0, 0x00F800E0, 0x38808080, 0x00C0C0C0, 0x00808080,
    0x00FFFFFF, 0x00F800D0, 0x001800D8, 0x38808080, 0x00808080, 0x00808080,
    0x00C0C0C0, 0x00F80100, 0x00D000C8, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x01000018, 0x00C800D8, 0x38FFFFFF, 0x00FFFFFF, 0x00808080,
    0x00808080, 0x00180100, 0x00B000C0, 0x38555555, 0x006A6A6A, 0x00555555,
    0x00808080, 0x01000090, 0x00C000B8, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00B80090, 0x00B00018, 0x38555555, 0x00808080, 0x00949494,
    0x00808080, 0x00980018, 0x00A80090, 0x38949494, 0x00808080, 0x00555555,
    0x00808080, 0x00900080, 0x00A800A0, 0x38353535, 0x00353535, 0x00808080,
    0x00808080, 0x00800018, 0x00A00098, 0x38949494, 0x00555555, 0x00808080,
    0x00808080, 0x00180080, 0x00780070, 0x38555555, 0x00949494, 0x00808080,
    0x00808080, 0x00800088, 0x00700068, 0x38353535, 0x00353535, 0x00808080,
    0x00808080, 0x00180078, 0x00880068, 0x38808080, 0x00949494, 0x00808080,
    0x00555555, 0x00880058, 0x00180060, 0x38808080, 0x00555555, 0x00808080,
    0x00949494, 0x00880000, 0x00580050, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00000018, 0x00500060, 0x386A6A6A, 0x00555555, 0x00808080,
    0x00555555, 0x00180000, 0x00380048, 0x38FFFFFF, 0x00FFFFFF, 0x00808080,
    0x00808080, 0x00000008, 0x00480040, 0x38808080, 0x00808080, 0x00808080,
    0x00808080, 0x00400008, 0x00380018, 0x38808080, 0x00808080, 0x00C0C0C0,
    0x00808080, 0x00200018, 0x00300008, 0x38C0C0C0, 0x00808080, 0x00FFFFFF,
    0x00808080, 0x00080010, 0x00300028, 0x38FFFFFF, 0x00C0C0C0, 0x00808080,
    0x00808080, 0x00100018, 0x00280020, 0x38EEEEEE, 0x00C0C0C0, 0x00808080,
    0x00808080,
};

static Unk801B0C98 MBarrierRenderDesc = {mbari_a1, 0x88, 0, 0, 0x20};
static s32 MBarrierBaseScale;
static s32 MBarrierBlend;
static char MBarrierPrimBuffer[0x20000];
static void* MBarrierBufferPtr;

// mabaria.c forward declarations
static void MBarrierMainSetup(int arg0, int arg1);

void MAGIC_MBarrier(int arg0, int arg1) { MBarrierMainSetup(arg0, arg1); }

static void MBarrierRenderShield(void) {
    MATRIX matrix;
    VECTOR scale;
    MBarrierData* barrier = &D_80162978[D_8015169C];
    int var_s1;

    scale.vx = scale.vy = scale.vz =
        ((((barrier->AnimationFrame * 15359) / 16) + 0x400) *
         MBarrierBaseScale) >>
        12;

    if (barrier->AnimationFrame < 8) {
        var_s1 = 0;
    } else {
        var_s1 = (barrier->AnimationFrame - 8) << 9;
    }

    var_s1 += ((0x1000 - var_s1) * MBarrierBlend) >> 12;

    RotMatrixYXZ(&barrier->Rot, &matrix);
    matrix.t[0] = barrier->Pos.vx;
    matrix.t[1] = barrier->Pos.vy;
    matrix.t[2] = barrier->Pos.vz;
    ScaleMatrix(&matrix, &scale);
    CompMatrix(&D_800FA63C.m, &matrix, &matrix);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    SetFarColor(0, 0, 0);

    MBarrierRenderDesc.unkA = var_s1;
    MBarrierBufferPtr =
        func_800D29D4(&MBarrierRenderDesc, g_cDb->unk70, 12, MBarrierBufferPtr);

    if (D_80062D98 == 0) {
        barrier->AnimationFrame++;
        if (barrier->AnimationFrame >= 16) {
            barrier->StartFrame = -1;
        }
    }
}

static void MBarrierSequenceManage(void) {
    MBarrierData* barrier = &D_80162978[D_8015169C]; // model instance
    MBarrierData* next;

    if (D_80062D98 != 0) {
        return;
    }

    if (barrier->AnimationFrame == 0) {
        next = &D_80162978[func_800BBEAC(MBarrierRenderShield)];
        next->Pos = barrier->Pos;
        next->Rot = barrier->Rot;
    }

    if (barrier->AnimationFrame == 4) {
        next = &D_80162978[func_800BBEAC(MBarrierRenderShield)];
        next->Pos = barrier->Pos;
        next->Rot = barrier->Rot;
    }

    if (barrier->AnimationFrame == 8) {
        next = &D_80162978[func_800BBEAC(MBarrierRenderShield)];
        next->Pos = barrier->Pos;
        next->Rot = barrier->Rot;
    }

    if (barrier->AnimationFrame == 16) {
        func_800D5774(barrier->TargetId);
        barrier->StartFrame = -1;
    }

    barrier->AnimationFrame++;
}

static void MBarrierAttachToTarget(int arg0) {
    MBarrierData* barrier = &D_80162978[func_800BBEAC(MBarrierSequenceManage)];

    func_800D3994(arg0, D_801518E4[arg0].D_8015190F, &barrier->Pos);
    barrier->Pos.vx -=
        (rsin(D_801518E4[arg0].unk160.vy) * D_801518E4[arg0].unk12) >> 12;
    barrier->Pos.vz -=
        (rcos(D_801518E4[arg0].unk160.vy) * D_801518E4[arg0].unk12) >> 12;
    barrier->Rot = D_801518E4[arg0].unk160;
    barrier->TargetId = arg0;
}

static void MBarrierDoubleBufferFlip(void) {
    MBarrierData* barrier = &D_80162978[D_8015169C];

    MBarrierBufferPtr = &MBarrierPrimBuffer[barrier->AnimationFrame * 65536];
    barrier->AnimationFrame ^= 1;

    if (D_80162080 < 2) {
        barrier->StartFrame = -1;
    }
}

static void MBarrierMainSetup(int arg0, int arg1) {
    MBarrierBaseScale = 0x2000;
    MBarrierBlend = 0;
    func_800BBEAC(MBarrierDoubleBufferFlip);
    func_800D5444(arg0, arg1, 0, MBarrierAttachToTarget);
    func_800D55F4(0x20, 0x40, 0x43);
}
