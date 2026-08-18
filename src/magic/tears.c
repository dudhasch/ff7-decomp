//! PSYQ=3.3 CC1=2.6.3

// MAGIC/TEARS.BIN -- "tears" (涙). Not one of the 54 Materia spells: the
// overlay rains three droplets per frame for 30 frames, each spawned at a
// random angle and distance around the target and falling until it passes
// the ground plane. Which ability or status it belongs to has not been
// established.

#include "common.h"
#include "../battle/battle.h"

// Battle effect instance, as this overlay lays it out. The manager slot uses
// only Id/AnimationFrame and the spawn origin at +4; each drop it spawns uses
// the same slot shape as a falling particle.
typedef struct TearsData {
    /* 0x00 */ s16 Id;
    /* 0x02 */ s16 AnimationFrame;
    /* 0x04 */ s16 x;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 z;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s16 unkC;
    /* 0x0E */ s16 vy;
    /* 0x10 */ s16 unk10;
    /* 0x12 */ s16 unk12;
    /* 0x14 */ s16 Scale;
    /* 0x16 */ char pad16[0xA];
} TearsData;

// The sprite descriptor func_800D6260 renders from. The colour and command
// byte are written together as a word on the steady-state path, and one
// channel at a time while the drop fades in, so they need both views.
typedef union TearsColor {
    struct {
        u8 r;
        u8 g;
        u8 b;
        u8 code;
    } c;
    u32 word;
} TearsColor;

typedef struct TearsSprite {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ u8 TexPage;
    /* 0x5 */ u8 unk5;
    /* 0x6 */ u8 unk6;
    /* 0x7 */ u8 unk7;
    /* 0x8 */ TearsColor col;
    /* 0xC */ s16 unkC;
    /* 0xE */ s16 unkE;
} TearsSprite;

// Battle effect instances.
extern TearsData D_80162978[];

static TearsSprite TearsSpriteDesc = {
    -16, -16, 0, 0, 0x1F, 0x1F, {{0x80, 0x80, 0x80, 0x2E}}, 0x3A, 0x7947};
static char TearsPrimBuffer[0x20000];
static void* TearsBufferPtr;

// tears.c forward declarations
static void TearsMainSetup(int arg0, int arg1);

void MAGIC_Tears(int arg0, int arg1) { TearsMainSetup(arg0, arg1); }

static void TearsRenderDrop(void) {
    TearsData* tear = &D_80162978[D_8015169C];
    s32 level;

    if (tear->AnimationFrame < 5) {
        level = (tear->AnimationFrame * 128) / 5;
        TearsSpriteDesc.col.c.b = level;
        TearsSpriteDesc.col.c.g = level;
        TearsSpriteDesc.col.c.r = level;
    } else {
        TearsSpriteDesc.col.word = 0x2E808080;
    }

    func_800D4368(&tear->x, tear->Scale, 0);

    TearsSpriteDesc.TexPage = ((tear->AnimationFrame + tear->Id) & 7) * 32;
    TearsBufferPtr =
        func_800D6260(&TearsSpriteDesc, g_cDb->unk70, 12, TearsBufferPtr);

    if (D_80062D98 == 0) {
        tear->y += tear->vy;
        tear->AnimationFrame++;
        tear->vy = tear->vy;
        if (tear->y > 0) {
            tear->Id = -1;
        }
    }
}

static void TearsSpawnDrops(void) {
    TearsData* src = &D_80162978[D_8015169C];
    TearsData* drop;
    s32 i;
    s32 angle;
    s32 dist;

    if (D_80062D98 != 0) {
        return;
    }

    if (src->AnimationFrame < 30) {
        for (i = 0; i < 3; i++) {
            drop = &D_80162978[func_800BBEAC(TearsRenderDrop)];
            angle = rand() & 0xFFF;
            dist = rand() % 1000;
            drop->x = src->x + ((rsin(angle) * dist) >> 12);
            drop->y = (rand() % 100) - 0x834;
            drop->z = src->z + ((rcos(angle) * dist) >> 12);
            drop->vy = 0x64;
            drop->Scale = 0x1000;
            drop->Id = rand() & 7;
        }
    } else {
        func_800D5774(src->Id);
        src->Id = -1;
    }

    src->AnimationFrame++;
}

static void TearsAttachToTarget(int arg0) {
    TearsData* src = &D_80162978[func_800BBEAC(TearsSpawnDrops)];

    src->Id = arg0;
    func_800D3994(arg0, D_801518E4[arg0].D_8015190F, &src->x);
}

static void TearsDoubleBufferFlip(void) {
    TearsData* src = &D_80162978[D_8015169C];

    TearsBufferPtr = &TearsPrimBuffer[src->AnimationFrame * 65536];
    src->AnimationFrame ^= 1;

    if (D_80162080 < 2) {
        src->Id = -1;
    }
}

static void TearsMainSetup(int arg0, int arg1) {
    func_800BBEAC(TearsDoubleBufferFlip);
    func_800D5444(arg0, arg1, 0, TearsAttachToTarget);
    func_800D55F4(32, func_800D574C(arg0), 0x11E);
}
