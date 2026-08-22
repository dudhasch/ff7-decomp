#include "game.h"

typedef struct {
    u16 magic[4];
    u16 unk8;
    u16 unkA;
    u16 colors[1];
} BromStruct;

extern s32 func_80017108(void* src, void* dst); // gzip decompress
extern void func_80014540(void);

extern u16 D_800707BC; // last browsed file index, kept across invocations
extern u16 D_800707BE;
extern u16 D_8009C560;

extern u8 D_800A06B4[]; // "BRO " label, FF7-encoded
extern u8 D_800A06BC[]; // "0000" + terminators, the index template
extern RECT D_800A06C4; // the strip the index is drawn into

/* .bss past the data segment: two primitive buffers and the two OTs that
 * index them.  D_800A473C is D_800A073C + 0x4000, i.e. just past both. */
extern u8 D_800A073C[2][0x2000];
extern u_long D_800A473C[2];

extern void func_8001C980(void);
extern s32 func_8001C808(void);
extern void func_80020058(s32 partyId);
extern void func_8001786C(u8 partyId);
extern void func_80017678(void);
extern void func_80014578(s32 file_no, void* dst, void (*cb)(void));
extern void func_800145BC(void (*cb)(void));
extern void func_80025174(void);
extern void func_80026090(void);
extern void func_800269C0(void* arg0);
extern void func_80026A00(void* arg0);
extern void func_80027354(s32 x, s32 y, u8* str, s32 arg3);

/* The overlay's own display/drawing environments, in .bss just past the data
 * segment; splat resolves them out of undefined_syms.brom.txt. */
extern DISPENV D_800A0728;
extern DRAWENV D_800A06CC;

void func_800A0000(void);
void func_800A01A0(void);
s32 func_800A0514(s32 arg0);
void func_800A0534(BromStruct* arg0);
u16 func_800A05D4(BromStruct* img, s32 arg1, s32 arg2);

void func_800A0000(void) {
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(1);
    SetDefDispEnv(&D_800A0728, 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv(&D_800A06CC, 0, 0xF0, 0x140, 0xE0);
    D_800A0728.isrgb24 = 0;
    D_800A06CC.dfe = 1;
    D_800A06CC.dtd = 0;
    D_800A06CC.isbg = 0;
    D_800A06CC.tpage = 0;
    VSync(0);
    PutDispEnv(&D_800A0728);
    PutDrawEnv(&D_800A06CC);
}

void func_800A00CC(void) {
    while (D_80095DD4 != 0) {
    }

    D_80075DEC = 1;
    func_800A0000();
    func_800A01A0();
    if (D_800707BE != 0) {
        D_8009C560 = 2;
    } else {
        D_8009C560 = 1;
        func_80014540();
    }
}

void func_800A015C(void) {
    if (func_80017108((void*)0x801B0000, (void*)0x801C0000) > 0) {
        func_800A0534((BromStruct*)0x801C0000);
        func_800A05D4((BromStruct*)0x801C0000, -1, -1);
    }
}

/* The BRO browser's main loop: pick an index with the d-pad, gzip-inflate that
 * file over the overlay at 0x801B0000 through func_800A015C, and draw it into
 * one of two OTs while the other is on screen. */
void func_800A01A0(void) {
    u8 buf[0x60];
    s32 index;
    s32 done;
    s32 repeat;
    s32 prev;
    s32 cur;
    s32 pressed;
    s32 otIdx;
    s32 i;
    u32 k;
    s32 n;
    s32 j;

    index = D_800707BC;
    D_800707BE = 0;
    done = 0;
    func_8001C980();
    func_8001C980();
    for (i = 0; i < 3; i++) {
        func_80020058(i);
        func_8001786C(i);
    }
    func_80017678();

    while (!done) {
        repeat = 0;
        prev = 0;
        func_80014578(4, (void*)0x801B0000, func_800A015C);
        func_800145BC(0);
        otIdx = 0;
        for (;;) {
            cur = func_8001C808();
            pressed = (prev ^ cur) & cur;
            ClearOTag(&D_800A473C[otIdx], 1);
            func_80026A00(&D_800A473C[otIdx]);
            func_800269C0(D_800A073C[otIdx]);
            if (prev == cur) {
                if (repeat++ >= 9) {
                    pressed = prev;
                }
            } else {
                repeat = 0;
            }
            prev = cur;
            if (pressed & 0x100) {
                done = 1;
                break;
            }
            if (pressed & 0x820) {
                D_800707BE = 0x8000;
                done = 1;
                break;
            }
            /* 0x20 is already covered by the 0x820 test above, so this arm is
             * unreachable -- but the compare is in the target, so it is in the
             * original. */
            if (!(pressed & 0x20)) {
                if (pressed & 0x10) {
                    func_80020058(0);
                    func_8001786C(0);
                    func_80020058(1);
                    func_8001786C(1);
                    func_80020058(2);
                    func_8001786C(2);
                    func_80017678();
                    func_80025174();
                    func_80026090();
                    func_80020058(0);
                    func_8001786C(0);
                    func_80020058(1);
                    func_8001786C(1);
                    func_80020058(2);
                    func_8001786C(2);
                    func_80017678();
                    break;
                } else if (pressed & 0x1000) {
                    index++;
                } else if (pressed & 0x4000) {
                    index--;
                } else if (pressed & 0x8000) {
                    if (index < 0) {
                        index = 0;
                    } else {
                        index += 10;
                    }
                } else if (pressed & 0x2000) {
                    index -= 10;
                }
            }
            if (index < 0) {
                index = 0;
            } else if (index >= 0x400) {
                index = 0x3FF;
            }
            n = index;
            for (k = 0; k < 8; k++) {
                buf[k] = D_800A06BC[k];
            }
            /* The digit cursor has to be an index, not a pointer: the target
             * initialises it as `addiu a1,s8,4` off the hoisted `&buf`, which
             * is a strength-reduced giv's preheader init. A `u8* p = &buf[4]`
             * is an ordinary assignment that the front end folds to
             * `(plus (sp) 0x14)` and rematerialises from sp -- 1 row.
             * `j` itself has no other use, so gcc drops the biv entirely. */
            if (n != 0) {
                j = 4;
                do {
                    buf[--j] += n % 10;
                    n /= 10;
                } while (n != 0);
            }
            func_80027354(D_800A06C4.x, 0xB2, D_800A06B4, 0);
            func_80027354(D_800A06C4.x + 0x30, 0xB2, buf, 0);
            ClearImage(&D_800A06C4, 0xFF, 0xFF, 0xFF);
            DrawOTag(&D_800A473C[otIdx]);
            otIdx ^= 1;
            DrawSync(0);
            VSync(2);
        }
    }
    D_800707BC = index;
}

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
