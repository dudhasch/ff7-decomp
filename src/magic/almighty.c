//! PSYQ=3.3 CC1=2.6.3

// MAGIC/ALMIGHTY.BIN -- "almighty" (全能). Not one of the 54 Materia spells:
// the overlay bursts three or six shards out of the target, each integrating
// its own velocity with decay and a pitch that eases downward. Which ability
// it belongs to has not been established.

#include "common.h"
#include "../battle/battle.h"

// Battle effect instance, as this overlay lays it out. The manager slot uses
// Id/AnimationFrame/TargetId and the spawn origin; every shard it spawns uses
// Pos/Rot plus its own speed, scale and spin.
typedef struct AlmightyData {
    /* 0x00 */ s16 Id;
    /* 0x02 */ s16 AnimationFrame;
    /* 0x04 */ s16 TargetId;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ SVECTOR Pos;
    /* 0x10 */ SVECTOR Rot;
    /* 0x18 */ s16 Speed;
    /* 0x1A */ s16 Scale;
    /* 0x1C */ s16 Spin;
    /* 0x1E */ s16 unk1E;
} AlmightyData;

// The sprite descriptor func_800D6260 renders from; the colour and command
// byte need both a word and a per-channel view.
typedef union AlmightyColor {
    struct {
        u8 r;
        u8 g;
        u8 b;
        u8 code;
    } c;
    u32 word;
} AlmightyColor;

typedef struct AlmightySprite {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ u8 TexPage;
    /* 0x5 */ u8 unk5;
    /* 0x6 */ u8 unk6;
    /* 0x7 */ u8 unk7;
    /* 0x8 */ AlmightyColor col;
    /* 0xC */ s16 unkC;
    /* 0xE */ s16 unkE;
} AlmightySprite;

// Battle effect instances.
extern AlmightyData D_80162978[];
extern s16 D_800F836C;

static AlmightySprite AlmightySpriteDesc = {
    -16, -16, 0, 0x40, 0x1F, 0x1F, {{0x80, 0x80, 0x80, 0x2E}}, 0x3A, 0x7947};
// Texture page per animation frame: the shard brightens, flickers, then blows
// out over its 17-frame life.
static u8 AlmightyFadeTable[20] = {
    0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0x80, 0x60, 0x80, 0xA0,
    0x80, 0x60, 0x80, 0xA0, 0xC0, 0xE0, 0x00, 0x00, 0x00, 0x00};
static SVECTOR AlmightySpawnRot = {0, 0, 0, 0};
static s32 D_801B05A0 = 100;
static s32 D_801B05A4 = 0;
static s32 AlmightyShardCount;
static char AlmightyPrimBuffer[0x20000];
static void* AlmightyBufferPtr;

// almighty.c forward declarations
static void AlmightyMainSetup(int arg0, int arg1);

void MAGIC_Almighty(int arg0, int arg1) { AlmightyMainSetup(arg0, arg1); }

static void AlmightyRenderShard(void) {
    SVECTOR* vec = (SVECTOR*)0x1F800000;
    MATRIX* matrix = (MATRIX*)0x1F800008;
    AlmightyData* shard = &D_80162978[D_8015169C];

    AlmightySpriteDesc.TexPage = AlmightyFadeTable[shard->AnimationFrame];
    func_800D4368(&shard->Pos, shard->Scale, 0);
    AlmightyBufferPtr =
        func_800D6260(&AlmightySpriteDesc, g_cDb->unk70, 12, AlmightyBufferPtr);

    if (D_80062D98 == 0) {
        shard->AnimationFrame++;
        if (shard->AnimationFrame >= 0x11) {
            shard->Id = -1;
        } else {
            vec->vx = shard->Speed;
            vec->vy = 0;
            vec->vz = 0;
            RotMatrixYXZ(&shard->Rot, matrix);
            ApplyMatrixSV(matrix, vec, vec);

            shard->Pos.vx += vec->vx >> 4;
            shard->Pos.vy += vec->vy >> 4;
            shard->Pos.vz += vec->vz >> 4;
            shard->Speed -= shard->Speed >> 3;
            shard->Rot.vz += (-0x400 - shard->Rot.vz) >> 2;
            shard->Rot.vx += shard->Spin;
        }
    }
}

static void AlmightySpawnShards(void) {
    AlmightyData* src = &D_80162978[D_8015169C];
    AlmightyData* shard;
    s32 i;

    if (D_80062D98 != 0) {
        return;
    }

    if (src->AnimationFrame < 10) {
        AlmightySpawnRot.vy = rand() & 0xFFF;

        for (i = 0; i < AlmightyShardCount; i++) {
            shard = &D_80162978[func_800BBEAC(AlmightyRenderShard)];
            shard->Pos = src->Pos;
            shard->Scale = (rand() % 8192) + 0x1000;
            shard->Rot = AlmightySpawnRot;
            shard->Speed = ((rand() % 400) + 100) * 16;
            shard->Spin = (rand() % 100) - 0x32;
            AlmightySpawnRot.vy += 0x1000 / AlmightyShardCount;
        }
    } else {
        func_800D5774(src->TargetId);
        src->Id = -1;
    }

    src->AnimationFrame++;
}

static void AlmightyAttachToTarget(int arg0) {
    AlmightyData* src = &D_80162978[func_800BBEAC(AlmightySpawnShards)];

    src->TargetId = arg0;
    func_800D3994(arg0, D_801518E4[arg0].D_8015190F, &src->Pos);
    src->Pos.vy = 0;
    func_800D55F4(32, func_800D56A8(&src->Pos), 0x8B);
}

static void AlmightyDoubleBufferFlip(void) {
    AlmightyData* src = &D_80162978[D_8015169C];

    AlmightyBufferPtr = &AlmightyPrimBuffer[src->AnimationFrame * 65536];
    src->AnimationFrame ^= 1;

    if (D_80162080 < 2) {
        src->Id = -1;
    }
}

static void AlmightyMainSetup(int arg0, int arg1) {
    s32 kind;

    func_800BBEAC(AlmightyDoubleBufferFlip);
    kind = func_800D54BC(arg0);
    AlmightyShardCount = (kind == 1) ? 6 : 3;
    func_800D5444(arg0, arg1, (kind >= 4) ? 0xA : 0, AlmightyAttachToTarget);
    D_800F836C = 0;
}
