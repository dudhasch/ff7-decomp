//! PSYQ=3.3 CC1=2.7.2
#include <game.h>

static s32 D_801D1AA8 = -1;
static u8 D_801D1AAC[] = {
    0x07, 0x07, 0x05, 0x07, 0x06, 0x07, 0x04, 0x07, 0x03, 0x0A, 0x00, 0x0A,
    0x01, 0x0A, 0x02, 0x0A, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x09, 0x07,
    0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08, 0x00, 0x10, 0x00, 0x20, 0x00,
    0x40, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
static u8 D_801D1ADB = 8;
static u8 D_801D1ADC[] = {
    0x05, 0x06, 0x04, 0x07, 0x02, 0x03, 0x00, 0x01, 0x08, 0x0B, 0x00, 0x00};
static unsigned char D_801D1AE8[][0x30] = {
    _S("Window color"),   // 0
    _S("Sound"),          // 1
    _S("Controller"),     // 2
    _S("Cursor"),         // 3
    _S("ATB"),            // 4
    _S("Battle speed"),   // 5
    _S("Battle message"), // 6
    _S("Field message"),  // 7
    _S("Camera angle"),   // 8
    _S("Select"),         // 9
    _S("Cancel"),         // 10
    _S("Menu"),           // 11
    _S("Window OFF"),     // 12
    _S("Pause"),          // 13
    _S("Battle help"),    // 14
    _S("Mono"),           // 15
    _S("Stereo"),         // 16
    _S("Wide"),           // 17
};
static unsigned char D_801D1E48[][0x30] = {
    _S("Normal"),
    _S("Customize"),
};
static unsigned char D_801D1EA8[][0x30] = {
    _S("Initial"),
    _S("Memory"),
};
static unsigned char D_801D1F08[][0x30] = {
    _S("Active"),      // 0
    _S("Recommended"), // 1
    _S("Wait"),        // 2
    _S("Auto"),        // 3
    _S("Fix"),         // 4
    _S(""),            // 5
    _S("Slow"),        // 6
    _S("Fast"),        // 7
    _S("RED"),         // 8
    _S("GREEN"),       // 9
    _S("BLUE"),        // 10
    _S("Magic order"), // 11
    _S("restore"),     // 12
    _S("attack"),      // 13
    _S("indirect"),    // 14
    _S("forbidden"),   // 15
    _S("No."),         // 16
};
static unsigned char D_801D2238[][0x30] = {
    _S("Left2"),  // 0
    _S("Right2"), // 1
    _S("Left1"),  // 2
    _S("Right1"), // 3
    _S("Menu"),   // 4
    _S("Select"), // 5
    _S("Cancel"), // 6
    _S("EXT"),    // 7
    _S("Help"),   // 8
};
static unsigned char D_801D23E8[][0x30] = {
    {0x30, 0x52, 0x45, 0x53, 0x53, 0x00, 0xB2, 0x33, 0x34, 0x21, 0x32, 0x34,
     0xB3, 0x00, 0x54, 0x4F, 0x00, 0x43, 0x55, 0x53, 0x54, 0x4F, 0x4D, 0x49,
     0x5A, 0x45, 0x0E, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};
static unsigned char D_801D2418[][0x30] = {
    _S("Press Directional button Left or Right to end."),
    _S("Pause"),
};
static u8 D_801D2478[][6] = {
    {0xC5, 0xB8, 0xB7, 0xFF, 0x00, 0x00}, // RED
    {0xBA, 0xC5, 0xB8, 0xB8, 0xC1, 0xFF}, // BLUE
    {0xB5, 0xBF, 0xC8, 0xB8, 0xFF, 0x00}, // GREEN
};
static u8 D_801D248C[] = {
    0x00, 0x01, 0x02, 0x00, 0x02, 0x01, 0x01, 0x02, 0x00, 0x01,
    0x00, 0x02, 0x02, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00,
};
static RECT D_801D24A0[2] = {
    {0, 5, 364, 24},
    {0, 29, 364, 195},
};
static u8 D_801D24B0[] = {
    // maybe this is an unused string?
    0x53, 0x69, 0x17, 0x61, 0xA1, 0x69, 0xFF,
};
struct Pad {
    u16 press;
    u16 prev;
};
static struct Pad D_801D24B8 = {0, 0};
// Deliberately not a third member of struct Pad: as a member gcc caches the
// address in a saved register across the reads in switch case 3, and the
// original reloads it each time.
static u16 D_801D24BC = 0;
// The three button ids (Left1, Right1, Menu) that must stay mapped for a
// customized controller layout to be accepted.
static s32 D_801D24C0[] = {4, 5, 6};

// from now on this is the BSS section; this can be moved up if necessary
static Unk80026448 D_801D24CC[4];
static s32 unused[5];
static s32 D_801D2528;
static u8 D_801D252C[LEN(D_80049208)];

static void func_801D0040(u16 arg0) {
    D_8009A000[0] = 0x30;
    D_8009A004[0] = arg0;
    D_8009A008[0] = arg0;
    SystemAkaoExecute();
}

static void func_801D0080(s32 arg0) {
    switch (arg0) {
    case 0:
        D_8009A000[0] = 0x81;
        D_8009A004[0] = 0x81;
        D_8009A008[0] = 0x81;
        break;
    case 1:
        D_8009A000[0] = 0x80;
        D_8009A004[0] = 0x80;
        D_8009A008[0] = 0x80;
        break;
    case 2:
        D_8009A000[0] = 0x82;
        D_8009A004[0] = 0x82;
        D_8009A008[0] = 0x82;
        break;
    }
    SystemAkaoExecute();
}

static u8 func_801D0118(u16 arg0) {
    s32 i;
    for (i = 0; i < 12; i++) {
        if ((arg0 >> i) & 1) {
            return i;
        }
    }
    return 0xFF;
}

// x and y reach func_80028CA0 as 16-bit values; the casts are what produce the
// sign-extension on entry. They cannot be s16 parameters instead: that would
// make func_801D080C narrow every argument at the call site, which it does not.
static void func_801D014C(s32 x, s32 y, s32 value) {
    u8 uv = D_801D1AAC[value * 2 + 0];
    u8 page = D_801D1AAC[value * 2 + 1];
    func_80028CA0((s16)x, (s16)y, (uv & 3) * 16 + 96, (uv >> 2) * 16 + 64, 16,
                  16, page, 0);
}

static void func_801D01C8(void) {
    RECT rect;
    s32 temp_s4;
    s32 y;
    s32 x;
    s32 var_s2;
    s32 i;
    s32 _setting;
    u8* setting;

    y = D_801D24A0[1].y + 33;
    func_80026F44(40, D_801D24A0[1].y + 14, D_801D1AE8[0], 5);
    for (i = 1; i < 9; i++) {
        func_80026F44(40, y + (i - 1) * 18, D_801D1AE8[i], 5);
    }
    func_80026F44(40, y + (i - 1) * 18, D_801D1AE8[33], 5);
    for (i = 0; i < 2; i++) {
        func_80026F44(165 + i * 65, y, D_801D1AE8[15 + i],
                      -((Savemap.config & 3) == i) & 7);
    }
    for (i = 0; i < 2; i++) {
        func_80026F44(165 + i * 65, y + 18, D_801D1E48[i],
                      -(((Savemap.config >> 2) & 3) == i) & 7);
    }
    for (i = 0; i < 2; i++) {
        func_80026F44(165 + i * 65, y + 0x24, D_801D1EA8[i],
                      -(((Savemap.config >> 4) & 3) == i) & 7);
    }
    temp_s4 = y + 54;
    func_80026F44(
        165, temp_s4, D_801D1F08[0], -(((Savemap.config >> 6) & 3) == 0) & 7);
    x = func_80026B70(D_801D1F08[0]);
    func_80026F44(x + 175, temp_s4, D_801D1F08[1],
                  -(((Savemap.config >> 6) & 3) == 1) & 7);
    func_80026F44(x + func_80026B70(D_801D1F08[1]) + 185, temp_s4,
                  D_801D1F08[2], -(((Savemap.config >> 6) & 3) == 2) & 7);
    for (i = 0; i < 2; i++) {
        func_80026F44(165 + i * 65, y + 126, D_801D1F08[3 + i],
                      -(((Savemap.config >> 8) & 3) == i) & 7);
    }
    for (i = 0; i < 3; i++) {
        _setting = D_801D248C[((Savemap.config >> 10) & 7) * 3 + i];
        setting = &D_801D248C[((Savemap.config >> 10) & 7) * 3];
        func_80026F44(189 + i * 52, y + 0x90, D_801D1F08[12 + setting[i]], 7);
    }
    func_80026F44(149, y + 0x90, D_801D1F08[16], 7);

    // battle speed value
    rect.y = y + 72;
    rect.w = 8;
    rect.h = 11;
    rect.x = (Savemap.battle_speed >> 1) + 184;
    func_80028030(&rect);

    // battle message speed value
    rect.y = y + 0x5A;
    rect.w = 8;
    rect.h = 11;
    rect.x = (Savemap.battle_msg_speed >> 1) + 184;
    func_80028030(&rect);

    // field message speed value
    rect.y = y + 108;
    rect.w = 8;
    rect.h = 11;
    rect.x = (Savemap.field_msg_speed >> 1) + 184;
    func_80028030(&rect);

    // speed values
    for (i = 0; i < 3; i++) {
        func_80026F44(157, y + 72 + i * 18 + 2, D_801D1F08[7], 7);
        func_80026F44(324, y + 72 + i * 18 + 2, D_801D1F08[6], 7);
    }

    // speed bars
    for (i = 0; i < 3; i++) {
        rect.x = 184;
        rect.y = y + 72 + i * 18;
        rect.w = 136;
        rect.h = 11;
        func_80027B84(&rect);
    }

    // magic order ID
    func_80029114(173, y + 146, ((Savemap.config >> 0xA) & 7) + 1, 1, 7);

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x100;
    rect.h = 0x100;
    func_80026A34(0, 1, 0x1F, &rect);
}

// exported, see 800493C8
void func_801D069C(void) {
    volatile s32 dummy;
    s32 i;
    func_80026448(&D_801D24CC[0], 0, 0, 1, 10, 0, 0, 1, 10, 0, 0, 0, 1, 0);
    func_80026448(&D_801D24CC[1], 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0);
    func_80026448(&D_801D24CC[2], 0, 0, 1, 3, 0, 0, 1, 3, 0, 0, 0, 1, 0);
    func_80026448(&D_801D24CC[3], 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0);
    D_801D1AA8 = 0;
    for (i = 0; i < LEN(D_80049208); i++) {
        D_801D252C[i] = D_80049208[i];
    }
}

// exported, see 8004941C
//
// SCRATCH SETUP (docs/PERMUTER_MACROS.md, plus tools/permuter_externise.py):
//   import.py -> permuter_strip_asm.py -> permuter_macros.py align --strings
//   -> permuter_externise.py <scratch>/base.c
// Aligned that way the base scores 805 against a noise floor of 280 -- 48 rows
// of Savemap interiors and compiler-generated jump tables that have no name to
// align to, plus 8 for D_801D24BC. So --stop-on-zero cannot fire here; watch
// for 280 instead. Folding D_801D24BC into an aggregate with D_801D24B8 would
// clear its 8 rows but is not codegen-neutral -- see permuter_externise.py.
//
// 12 rows of 1205 instructions still differ. Everything structural is right:
// same instruction count, byte-identical .data/.rodata/.bss, and every saved
// register where the original put it. Everything from the second switch's jump
// table onwards now matches except two rows in the ATB case, so what is left is
// concentrated in case 2 of the first switch.
//
// Measure variants with tools/variant_eval.py rather than checkfn.py: it scores
// a variant against a private object, so many can run at once, and one takes
// about three seconds instead of a full ninja round trip. Baseline is 12.
//
// FOUR THINGS HERE LOOK ODD AND ARE LOAD-BEARING. Each was measured alone and
// in combination; they are additive, and they cost 14 rows between them.
//
//   * `value` holds the button pointer in case 3 (`value = (s32)D_801D1ADC`,
//     `*(u8*)value`). The walking pointer has to occupy the *same allocno* as
//     the `value` used by the first switch -- that is what donates it enough
//     loop-weighted references to keep the s0/s1/s2 ranking right. It is not a
//     question of style: a separate `u8 *button` is 91, `corner` in place of
//     `value` is 91, indexing with `value` is 97, indexing with `i` is 120, and
//     the natural `value = *button;` inside the loop is 180. So the original
//     used one variable as both an integer and a byte pointer -- the same shape
//     as func_801D01C8's `s32 _setting` / `u8 *setting` pair further up.
//
//   * `w` names the 0x100 that case 3's loop stores into rect.w/rect.h, and
//     sits in for-init slot 2. Naming it keeps it out of the loop-invariant set
//     so gcc cannot emit it at the very end of the preheader, which is where
//     the literal ends up and where the original does not have it. The slot
//     matters: `w` second scores 12, `w` third scores 19, the literal 26.
//
//   * The sound case writes Savemap.config with `&=` then `|=`. gcc still folds
//     it to one `sh`, but the two-statement form is what makes the result
//     coalesce into the masked value's register rather than setting's.
//
//   * The mask loop's counter is `setting`, not a block-local. The counter sits
//     in a callee-saved register while nothing in the loop nest calls anything,
//     and gcc only refuses call-clobbered registers for an allocno that crosses
//     a call -- so the counter must be a variable that is live across calls
//     elsewhere. This only pays off together with the `value` pointer above:
//     on its own it costs 50+ rows, because `setting` picks up loop-weighted
//     references and overtakes `value` in the allocation order.
//
// It also replaced the `i = Savemap.config;` hack that used to sit in the
// magic-order case: that existed only to force an i/setting conflict, and the
// mask loop now creates the real one.
//
// WHAT IS LEFT, all instruction scheduling in case 2 except the last:
//   * `li s2,0x21` (dy) and `addiu s6,sp,0x20` (&rect) swap places in the loop
//     preheader. Mechanism understood: the preheader is one ~41-instruction
//     basic block, the two initialisers tie on scheduling priority, and the
//     order falls out of their position in the RTL chain. Loop-invariant motion
//     emits &rect immediately before the loop, i.e. after any for-init, so a
//     for-init `dy` always wins the early slot. Writing `dy = i * 12 + 0x21;`
//     in the loop body makes dy a strength-reduced induction variable, whose
//     initialiser is emitted after the invariants -- that reproduces the
//     target's order exactly and clears these rows. It costs 8 elsewhere,
//     because strength reduction allocates a fresh pseudo that loses the two
//     references the `x = ...; dy = ...;` hoist donates, dropping dy below x
//     and rowY. Adding one in-loop reference restores the exact target
//     allocation, so a zero-cost way to donate ~2 references to that giv closes
//     both this and the next item.
//   * The `D_801D24A0[1].x` and `.y` loads, and `x` versus `rowY`, come out in
//     the opposite order. Not reachable by statement order: gcc has already
//     CSE'd both loads before the scheduler runs, and x-first, rowY-first and
//     rowY-inlined all produce byte-identical output.
//   * `sh zero,0x20(sp)` in the block after case 2's loop. These 2 rows are a
//     conserved quantity: writing the block so it clears entirely moves the
//     cost to the loop back-edge, where the store takes the branch delay slot.
//     Only something that changes what fills that slot can spend them.
//   * The ATB case's delay slot (`li a0,0x1` versus `addiu s0,s0,1`). Every
//     reordering tried scores 97, alone or on top of everything above, so the
//     cascade is independent of the rest.
//   * One row is the compiler-generated jump table, which has no symbol to
//     align to. The floor is 1, not 0.
//
// Also ruled out by measurement: making rowY an induction variable (changes the
// saved-register count); calling func_801D0040 before the increment in the ATB
// case; `setting | (Savemap.config & 0xFFFC)` in the sound case; merging
// `corner` into `value`; declaration order of `value` and `setting`; naming
// &rect and initialising it in case 2's for-init (15 -- it moves &rect early,
// but the target wants both it and dy late); and hoisting the
// `unkA * 3 + unkB * 6` index in case 2, which the target recomputes each
// iteration rather than hoisting.
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/menu/nonmatchings/cnfgmenu", func_801D080C);
#else
void func_801D080C(s32 arg0) {
    RECT window;
    RECT rect;
    volatile s32 padding[4];
    s32 i;
    s32 corner;
    s32 value;
    u16 setting;
    u16 pad;

    pad = func_8001C808();
    D_801D24B8.press = pad;
    D_801D24BC = pad & (D_801D24B8.prev ^ pad);
    func_800230C4(D_80062F58);

    switch (D_801D1AA8) {
    case 0:
        func_80026F44(
            0x10, 0xB, func_80015248(8, D_801D24CC[0].unkB + 0x37, 8), 7);
        if (D_801D24CC[0].unkB == 0) {
            func_8001EB2C(0xC, D_801D24A0[1].y + 0x10);
        } else {
            s32 rowTop = D_801D24A0[1].y;
            s32 cursorY = (D_801D24CC[0].unkB - 1) * 18 + 0x23;
            func_8001EB2C(0xC, cursorY + rowTop);
        }
        break;

    case 1:
        if (arg0 & 2) {
            if (D_801D24CC[0].unkB == 0) {
                func_8001EB2C(0xC, D_801D24A0[1].y + 0x10);
            } else {
                s32 rowTop = D_801D24A0[1].y;
                s32 cursorY = (D_801D24CC[0].unkB - 1) * 18 + 0x25;
                func_8001EB2C(0xC, rowTop + cursorY);
            }
        }
        {
            s32 cursorY = D_801D24CC[1].unkB * 18 + 4;
            func_8001EB2C(
                D_801D24CC[1].unkA * 61 + 0x8D, D_801D24A0[1].y + cursorY);
        }
        break;

    case 2: {
        s32 y;
        s32 x;
        s32 dy;
        s32 rowY;
        s32 knob;

        if (arg0 & 2) {
            {
                s32 cursorY = D_801D24CC[0].unkB * 18 + 0x10;
                func_8001EB2C(0xC, D_801D24A0[1].y + cursorY);
            }
            {
                s32 valueY = D_801D24CC[1].unkB * 19 + 4;
                x = D_801D24CC[1].unkA * 61 + 0x8D;
                dy = D_801D24A0[1].y + valueY;
                func_8001EB2C(x, dy);
            }
        }
        {
            s32 channelY = D_801D24CC[2].unkB * 12 + 0x24;
            func_8001EB2C(0x93, D_801D24A0[1].y + channelY);
        }
        y = D_801D24A0[1].y;
        x = D_801D24A0[1].x + 0xAD;
        rowY = y + 0x23;
        func_8002708C(x, rowY, D_801D2478[0][0], 2);
        func_8002708C(x, y + 0x2F, D_801D2478[1][0], 4);
        func_8002708C(x, y + 0x3B, D_801D2478[2][0], 1);
        for (i = 0, dy = 0x21; i < 3; i++, dy += 12) {
            func_80029114(
                x + 0x10, rowY,
                D_801D252C[D_801D24CC[1].unkA * 3 + D_801D24CC[1].unkB * 6 + i],
                3, 7);
            rowY += 12;

            // the movable knob of one colour slider
            knob = D_801D252C[D_801D24CC[1].unkA * 3 + D_801D24CC[1].unkB * 6 +
                              i] >>
                   1;
            rect.y = D_801D24A0[1].y + dy;
            rect.h = 11;
            rect.w = 8;
            rect.x = knob + 0xD5;
            func_80028030(&rect);

            // and the bar it slides along
            rect.x = 0xD5;
            rect.w = 0x88;
            rect.h = 11;
            rect.y = D_801D24A0[1].y + dy;
            func_80027B84(&rect);
        }
        rect.y = 0;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 0x100;
        func_80026A34(0, 1, 0x1F, &rect);

        func_8001DE0C(&window, D_801D24A0[1].x + 0xA5, D_801D24A0[1].y + 0x1D,
                      0xC0, 0x2B);
        func_8001E040(&window);
        func_8001DE0C(
            &window, D_801D24A0[1].x + 0xF8, D_801D24A0[1].y + 7, 0x1A, 0x12);
        {
            s32 rgb = D_801D24CC[1].unkB * 6 + D_801D24CC[1].unkA * 3;
            func_8001DF24(&window, D_801D252C[rgb], D_801D252C[rgb + 1],
                          D_801D252C[rgb + 2]);
        }
        func_8001DE0C(
            &window, D_801D24A0[1].x + 0xF5, D_801D24A0[1].y + 4, 0x20, 0x18);
        func_8001E040(&window);
        break;
    }

    case 3: {
        s32 rowY;
        s32 x;
        s32 y;
        s32 dy;
        s32 labelDy;
        s32 w;

        if (arg0 & 2) {
            s32 cursorY = D_801D24CC[0].unkB * 21 + 0xA;
            func_8001EB2C(0xC, D_801D24A0[1].y + cursorY);
        }
        if (D_801D2528 == 0) {
            func_80026F44(0x10, 0xB, D_801D23E8[0], 7);
        } else {
            func_80026F44(0x10, 0xB, D_801D2418[0], 7);
        }
        x = D_801D24A0[1].x + 0x8C;
        y = D_801D24A0[1].y + 3;
        if (D_801D2528 != 0) {
            s32 cursorY = (D_801D2528 - 1) * 18 + 0xA;
            func_8001EB2C(D_801D24A0[1].x + 0x78, y + cursorY);
        }
        for (i = 0, dy = 0xA; i < 10; i++, dy += 18) {
            func_8002708C(x + 0x22, y + dy, 0xDA, 5);
        }
        func_8001DE0C(&window, x, y, 0x78, 0xBB);
        y += 6;
        x += 0xA;
        for (i = 0, w = 0x100, value = (s32)D_801D1ADC, labelDy = 4, rowY = y;
             i < 10; i++, labelDy += 18) {
            func_801D014C(x, rowY, *(u8*)value);
            func_801D014C(x + 0x2C, rowY, Savemap.button_config[*(u8*)value]);
            rowY += 18;
            rect.x = 0;
            rect.y = 0;
            rect.w = w;
            rect.h = w;
            func_80026A34(0, 1, 0x1F, &rect);
            func_80026F44(x + 0x3E, y + labelDy - 2,
                          D_801D2238[Savemap.button_config[*(u8*)value]], 6);
            value++;
        }
        func_8001E040(&window);
        break;
    }
    }

    func_801D01C8();
    rect.x = 0;
    rect.y = 0;
    rect.w = 0xFF;
    rect.h = 0xFF;
    func_80026A34(0, 1, 0x1F, &rect);
    func_8001DE70();
    func_8001DEF0(D_801D252C);
    func_8001DE0C(
        &window, D_801D24A0[1].x + 0xA5, D_801D24A0[1].y + 4, 0x40, 0x18);
    func_8001E040(&window);
    func_8001DEB0();
    for (i = 0; i < 2; i++) {
        func_8001E040(&D_801D24A0[i]);
    }

    if (func_80023050() == 0) {
        func_800264A8(&D_801D24CC[D_801D1AA8]);
        switch (D_801D1AA8) {
        case 0:
            if (D_80062D7E & 0x40) {
                func_801D0040(4);
                func_8002305C(5, 0);
                func_8002120C(0);
                break;
            }
            switch (D_801D24CC[0].unkB) {
            case 0: // window colour
                if (D_80062D7C & 0x20) {
                    D_801D1AA8 = 1;
                    func_801D0040(1);
                }
                break;

            case 1: // sound
                value = Savemap.config & 3;
                setting = value;
                if ((D_80062D7E & 0x2000) && setting != 1) {
                    func_801D0040(1);
                    setting = value + 1;
                    Savemap.config = (Savemap.config & 0xFFFC) | (value + 1);
                    func_801D0080(setting);
                }
                if ((D_80062D7E & 0x8000) && setting != 0) {
                    func_801D0040(1);
                    setting = setting - 1;
                    Savemap.config &= 0xFFFC;
                    Savemap.config |= setting;
                    func_801D0080(setting);
                }
                break;

            case 2: // controller
                value = (Savemap.config >> 2) & 3;
                setting = value;
                if (D_80062D7C & 0x20) {
                    if (setting == 1) {
                        func_801D0040(1);
                        D_801D1AA8 = 3;
                        D_801D2528 = 0;
                    }
                } else {
                    if ((D_80062D7E & 0x2000) && setting != 1) {
                        func_801D0040(1);
                        setting = value + 1;
                        Savemap.config =
                            (Savemap.config & 0xFFF3) | ((value + 1) << 2);
                    }
                    if ((D_80062D7E & 0x8000) && setting != 0) {
                        func_801D0040(1);
                        Savemap.config =
                            (Savemap.config & 0xFFF3) | ((setting - 1) << 2);
                    }
                }
                break;

            case 3: // cursor
                value = (Savemap.config >> 4) & 3;
                setting = value;
                if ((D_80062D7E & 0x2000) && setting != 1) {
                    func_801D0040(1);
                    setting = value + 1;
                    Savemap.config =
                        (Savemap.config & 0xFFCF) | ((value + 1) << 4);
                }
                if ((D_80062D7E & 0x8000) && setting != 0) {
                    func_801D0040(1);
                    Savemap.config =
                        (Savemap.config & 0xFFCF) | ((setting - 1) << 4);
                }
                break;

            case 4: // ATB
                value = (Savemap.config >> 6) & 3;
                setting = value;
                if ((D_80062D7E & 0x2000) && setting != 2) {
                    value = value + 1;
                    func_801D0040(1);
                    setting = value;
                    value = value << 6;
                    Savemap.config = (Savemap.config & 0xFF3F) | value;
                }
                if ((D_80062D7E & 0x8000) && setting != 0) {
                    func_801D0040(1);
                    Savemap.config =
                        (Savemap.config & 0xFF3F) | ((setting - 1) << 6);
                }
                break;

            case 5: // battle speed
                if ((D_80062D78 & 0x2000) && Savemap.battle_speed != 0xFF) {
                    if (arg0 & 4) {
                        func_801D0040(1);
                    }
                    Savemap.battle_speed++;
                }
                if (D_80062D78 & 0x8000) {
                    if (Savemap.battle_speed != 0) {
                        if (arg0 & 4) {
                            func_801D0040(1);
                        }
                        Savemap.battle_speed--;
                    }
                }
                break;

            case 6: // battle message speed
                if ((D_80062D78 & 0x2000) && Savemap.battle_msg_speed != 0xFF) {
                    if (arg0 & 4) {
                        func_801D0040(1);
                    }
                    Savemap.battle_msg_speed++;
                }
                if (D_80062D78 & 0x8000) {
                    if (Savemap.battle_msg_speed != 0) {
                        if (arg0 & 4) {
                            func_801D0040(1);
                        }
                        Savemap.battle_msg_speed--;
                    }
                }
                break;

            case 7: // field message speed
                if ((D_80062D78 & 0x2000) && Savemap.field_msg_speed != 0xFF) {
                    if (arg0 & 4) {
                        func_801D0040(1);
                    }
                    Savemap.field_msg_speed++;
                }
                if (D_80062D78 & 0x8000) {
                    if (Savemap.field_msg_speed != 0) {
                        if (arg0 & 4) {
                            func_801D0040(1);
                        }
                        Savemap.field_msg_speed--;
                    }
                }
                break;

            case 8: // camera angle
                value = (Savemap.config >> 8) & 3;
                setting = value;
                if ((D_80062D7E & 0x2000) && setting != 1) {
                    func_801D0040(1);
                    setting = value + 1;
                    Savemap.config =
                        (Savemap.config & 0xFCFF) | ((value + 1) << 8);
                }
                if ((D_80062D7E & 0x8000) && setting != 0) {
                    func_801D0040(1);
                    Savemap.config =
                        (Savemap.config & 0xFCFF) | ((setting - 1) << 8);
                }
                break;

            case 9: // magic order
                value = (Savemap.config >> 10) & 7;
                setting = value;
                if ((D_80062D7E & 0x2000) && setting != 5) {
                    func_801D0040(1);
                    setting = value + 1;
                    Savemap.config =
                        (Savemap.config & 0xE3FF) | ((value + 1) << 10);
                }
                if ((D_80062D7E & 0x8000) && setting != 0) {
                    func_801D0040(1);
                    Savemap.config =
                        (Savemap.config & 0xE3FF) | ((setting - 1) << 10);
                }
                break;
            }
            break;

        case 1:
            if (D_80062D7C & 0x40) {
                for (i = 0; i < LEN(D_80049208); i++) {
                    D_80049208[i] = D_801D252C[i];
                }
                func_801D0040(4);
                D_801D1AA8 = 0;
            } else if (D_80062D7C & 0x20) {
                D_801D1AA8 = 2;
                func_801D0040(1);
            }
            break;

        case 2:
            corner = D_801D24CC[1].unkA * 3 + D_801D24CC[1].unkB * 6 +
                     D_801D24CC[2].unkB;
            if (D_80062D7C & 0x40) {
                func_801D0040(4);
                D_801D1AA8 = 1;
            } else if (D_80062D78 & 0x2000) {
                if (D_801D252C[corner] != 0xFF) {
                    if (arg0 & 4) {
                        func_801D0040(1);
                    }
                    D_801D252C[corner] = D_801D252C[corner] + 1;
                }
            } else if (D_80062D78 & 0x8000) {
                if (D_801D252C[corner] != 0) {
                    if (arg0 & 4) {
                        func_801D0040(1);
                    }
                    D_801D252C[corner] = D_801D252C[corner] - 1;
                }
            }
            break;

        case 3:
            if (D_801D24BC & 0x40) {
                if (D_801D2528 == 0) {
                    func_801D0040(4);
                    D_801D1AA8 = 0;
                    break;
                }
            } else if (D_801D2528 == 0) {
                if (D_801D24BC & 0x800) {
                    D_801D2528 = 1;
                }
                break;
            }
            if (D_801D24BC == 0) {
                break;
            }
            if (func_801D0118(D_801D24BC) != 0xFF) {
                Savemap.button_config[(&D_801D1ADB)[D_801D2528]] =
                    func_801D0118(D_801D24BC);
            } else if ((D_80062D7E & 0x1000) && D_801D2528 != 1) {
                func_801D0040(1);
                D_801D2528--;
            } else if ((D_80062D7E & 0x4000) && D_801D2528 != 10) {
                func_801D0040(1);
                D_801D2528++;
            } else if (D_80062D7E & 0xA000) {
                // Left1, Right1 and Menu must each still be bound to some
                // button before the layout can be accepted.
                s32 mapped = 0;
                for (setting = 0; setting < 3; setting++) {
                    for (i = 0; i < 16; i++) {
                        if (D_801D24C0[setting] == Savemap.button_config[i]) {
                            mapped |= 1 << setting;
                        }
                    }
                }
                if (mapped == 7) {
                    func_801D0040(1);
                    D_801D2528 = 0;
                    D_801D1AA8 = 0;
                } else {
                    func_801D0040(3);
                }
            }
            break;

        case 4:
            break;
        }
    }
    D_801D24B8.prev = D_801D24B8.press;
}
#endif

static void func_801D1AA0(void) {}
