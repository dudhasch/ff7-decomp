#include "common.h"

typedef struct {
    u16 magic[4];
    u16 unk8;
    u16 unkA;
    u16 colors[1];
} BromStruct;

extern s32 func_80017108(void* src, void* dst); // gzip decompress
extern void LoadImage(void* rect, void* p);

s32 func_800A0514(s32 arg0);
void func_800A0534(BromStruct* arg0);
u16 func_800A05D4(BromStruct* img, s32 arg1, s32 arg2);

INCLUDE_ASM("asm/us/brom/nonmatchings/brom", func_800A0000);

INCLUDE_ASM("asm/us/brom/nonmatchings/brom", func_800A00CC);

void func_800A015C(void) {
    if (func_80017108((void*)0x801B0000, (void*)0x801C0000) > 0) {
        func_800A0534((BromStruct*)0x801C0000);
        func_800A05D4((BromStruct*)0x801C0000, -1, -1);
    }
}

// The BRO browser's main loop: pick an index with the d-pad, gzip-inflate that
// file over the overlay at 0x801B0000 through func_800A015C, and draw it into
// one of two OTs while the other is on screen. Seeded and taken from 87 rows to
// 59; started over rather than left half-applied. What is established:
//
//   * m2c collapses the stack buffer to a bare `u8`, which gets the frame wrong
//     (0x40 against 0x98). Only sp+0x10 is ever named -- everything else goes
//     through registers -- and the saved registers occupy 0x70..0x94, so the
//     locals are one 0x60-byte buffer at 0x10. `u8 buf[0x60]` restores the
//     frame; the first 8 bytes hold the template copied from D_800A06BC and
//     the digits are written back into bytes 3..0.
//   * The index and its scratch copy are s32, not m2c's u16. The target
//     compares them full-width (`bgez s3`, `slti v0,s3,0x400`), so every u16
//     spelling costs an `andi 0xffff` at each test -- that alone was 5 rows.
//   * s8 holds `sp + 0x10` as a loop invariant. gcc will instead spend s8 on
//     the 0xCCCCCCCD magic for the `/ 10` in the digit loop, so the two compete
//     for the register and that is the thing to aim at next.
//   * The outer loop is guarded, not a do/while: the target tests the exit flag
//     before entering even though it was just zeroed. Writing `while (!done)`
//     is not enough on its own -- gcc still folds the test -- so the guard has
//     to come from somewhere else.
INCLUDE_ASM("asm/us/brom/nonmatchings/brom", func_800A01A0);

s32 func_800A0514(s32 arg0) {
    s32 g = arg0 & 0x3E0;
    s32 b = arg0 & 0x1F;
    s32 r = (u32)(arg0 & 0x7C00) >> 10;

    return (b << 10) | (r | g);
}

void func_800A0534(BromStruct* arg0) {
    s32 total_pixels;
    s32 i;
    s32 limit;
    s32 padding[2];
    s32 w, h;

    w = (arg0->unk8 >> 8) | (arg0->unk8 << 8);
    h = (arg0->unkA >> 8) | (arg0->unkA << 8);
    total_pixels = (w & 0xFFFF) * (h & 0xFFFF);
    if (total_pixels > 0) {
        i = 0;
        limit = total_pixels;
        do {
            u16 color = arg0->colors[i];
            arg0->colors[i] =
                func_800A0514(((color >> 8) | (color << 8)) & 0xFFFF);
            i++;
        } while (i < limit);
    }
}

u16 func_800A05D4(BromStruct* img, s32 arg1, s32 arg2) {
    s16 rect[4];
    s32 w, h;
    s32 y;
    u16 temp_v1;

    if ((temp_v1 = img->magic[0]) != 0x4152)
        return temp_v1;
    if ((temp_v1 = img->magic[1]) != 0x2057)
        return temp_v1;
    if ((temp_v1 = img->magic[2]) != 0x4752)
        return temp_v1;
    if ((temp_v1 = img->magic[3]) != 0x2042)
        return temp_v1;

    w = (img->unk8 >> 8) | (img->unk8 << 8);
    h = (img->unkA >> 8) | (img->unkA << 8);
    if (arg1 == -1) {
        arg1 = (0x140 - (w & 0xFFFF)) / 2;
    }
    y = arg2 + 0xE8;
    if (arg2 == -1) {
        arg2 = (0xF0 - (h & 0xFFFF)) / 2;
        y = arg2 + 0xE8;
    }
    rect[2] = w;
    rect[0] = arg1;
    rect[1] = y;
    rect[3] = h;
    LoadImage(rect, img->colors);
    return 0;
}
