//! PSYQ=3.3 CC1=2.6.3

// MAGIC/ESCAPE.BIN -- "Escape": ends the battle with no reward, cast from
// Exit Materia at level 1. See docs/spells/magic/escape.md.
//
// The effect grabs the framebuffer, cuts it into a 21x40 grid of textured
// quads and blows the grid apart. EscapeCaptureScreen copies the two halves of
// the drawing area into the texture pages the quads sample from;
// EscapeBuildGrid lays the grid out and gives every tile its own random drift;
// EscapeUpdateGrid runs the two phases of the animation.

#include "common.h"
#include "../battle/battle.h"

// One cell of the radial distance table: the sqrt of the squared distance from
// the screen centre, sampled on the same 8-pixel lattice as the grid.
typedef struct EscapeCell {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s32 Dist;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
} EscapeCell;

// One tile of the shattered screen: the two POLY_FT4s it is drawn with (one
// per frame buffer) followed by its own position, velocity and spin.
typedef struct EscapeTile {
    /* 0x00 */ POLY_FT4 prim[2];
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ s16 unk60;
    /* 0x62 */ s16 unk62;
    /* 0x64 */ s16 unk64;
    /* 0x66 */ s16 unk66;
    /* 0x68 */ s16 unk68;
    /* 0x6A */ s16 unk6A;
    /* 0x6C */ s16 unk6C;
    /* 0x6E */ s16 unk6E;
    /* 0x70 */ s16 unk70;
    /* 0x72 */ s16 unk72;
    /* 0x74 */ s16 unk74;
    /* 0x76 */ s16 unk76;
    /* 0x78 */ s16 unk78;
    /* 0x7A */ s16 unk7A;
    /* 0x7C */ s16 unk7C;
    /* 0x7E */ s16 unk7E;
} EscapeTile;

// Battle effect instance, as this overlay lays it out: the slot is only used
// to flag the effect finished.
typedef struct EscapeData {
    s16 Id;
    char pad2[0x1E];
} EscapeData;

// Battle effect instances.
extern EscapeData D_80162978[];
extern u8 D_800F8380;

// .data and .bss are each declared as one object so the two functions still
// assembled from asm/ can reach every field through a single symbol. Split
// them up once those land.
typedef struct EscapeStateT {
    /* 0x00 */ s32 Frame;
    /* 0x04 */ s32 unk04;
    /* 0x08 */ s32 unk08;
    /* 0x0C */ s32 unk0C;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
} EscapeStateT;

typedef struct EscapeWorkT {
    /* 0x00000 */ EscapeCell DistTable[22][41];
    /* 0x02A48 */ EscapeTile Tiles[21][40];
    /* 0x1CE48 */ POLY_F4 BackPrims[2];
} EscapeWorkT;

EscapeStateT EscapeState = {0, 0, 0x4000, 0x4000, 0x4000, 0, 0};
EscapeWorkT EscapeWork;

// Still assembled from asm/; declared so calling them and taking their address
// works.
void func_801B0020(void);
void func_801B009C(void);
void func_801B063C(void);

static void EscapeMainSetup(void);

void MAGIC_Escape(void) { EscapeMainSetup(); }

// Copies the drawing area into the two texture pages the grid samples from.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B0020);
#else
/* 16 rows, all scheduling: the target emits the two x stores before the w/h
 * pairs and fills the load-delay slot after `lhu DispX` with `move a2,zero`
 * (a MoveImage argument), where we fill it with the `li 0xA0` for the widths
 * and sink the x stores below them. Register allocation follows from that --
 * the target keeps g_cDb in v1 and overwrites it with DispY, we keep it in a2.
 * Every instruction is otherwise identical and the function is the right
 * length. Tried, in order: separate field reads (4 loads instead of 2); s32
 * locals for x and y (turns lhu into lh); adjacent reads without locals (still
 * 2 loads of DispX); both chain directions of `a.w = b.w = 0xA0` (the
 * left-to-right one is correct and is kept below); the embedded-assignment
 * form `right.x = (left.x = ...) + 0xA0`; and one `RECT half[2]` instead of
 * two RECTs. None moves the x stores up. */
static void EscapeCaptureScreen(void) {
    RECT left;
    RECT right;

    left.x = g_cDb->DispX;
    right.x = left.x + 0xA0;
    left.w = right.w = 0xA0;
    left.h = right.h = 0xA8;
    left.y = right.y = g_cDb->DispY;
    MoveImage(&left, 0x300, 0);
    MoveImage(&right, 0x280, 0x100);
    DrawSync(0);
}
#endif

INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B009C);

INCLUDE_ASM("asm/us/magic/nonmatchings/escape", func_801B063C);

// Runs once the grid has finished flying apart: releases every battle model
// and marks the effect done, which is what actually ends the battle.
static void EscapeFinish(void) {
    EscapeData* effect = &D_80162978[D_8015169C];
    s32 i;

    D_800F8380 |= 7;

    for (i = 0; i < 10; i++) {
        D_801518E4[i].D_80151909 |= 2;
    }

    effect->Id = -1;
}

static void EscapeMainSetup(void) {
    func_801B009C();
    func_801B0020();
    func_800BBEAC(func_801B063C);
    func_800BBEAC(EscapeFinish);
    func_800D55F4(0x20, 0x40, 0x5D);
}
