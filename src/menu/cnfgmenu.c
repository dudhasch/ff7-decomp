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
// PERMUTER PLAN. Recipe `delay-slot-swap` in tools/permuter_macros.py was
// written for this function; print it with
//   .venv/bin/python3 tools/permuter_macros.py recipe delay-slot-swap
//
// Scratch setup (docs/PERMUTER_MACROS.md, plus tools/permuter_externise.py):
//   import.py -> permuter_strip_asm.py -> permuter_macros.py align --strings
//   -> permuter_externise.py <scratch>/base.c
//
// Two things about running it here, both of which differ from a normal run:
//
//   * Run `permuter_macros.py retarget` after `align`, then measure the floor.
//     Retarget rewrites the scratch's own target.s so the Savemap interiors are
//     spelled the way the C spells them; here that is 44 references and takes
//     the base score from 596 to 220, with the floor dropping from ~276 to ~40
//     (what is left is the two jump tables, which have no name either side).
//     --stop-on-zero still cannot fire, so watch for the floor. Measured
//     without retarget, 46% of the score is naming noise the search can nudge
//     without improving anything -- which is exactly what happened on the first
//     attempt: a 156-point "improvement" that was worth zero real rows and
//     whose candidate read an uninitialised variable.
//
//   * Scope the randomization to the two arms that diverge, never the whole
//     function. 1201 of 1205 instructions already match, so an unscoped run
//     spends its candidates rewriting correct code, and can quietly destroy the
//     load-bearing constructs listed below -- `value` doubling as a byte
//     pointer, the s8*/short* loop invariants, the named 0x100. Exhaust the
//     finite space first: PERM_ONCE over the two legal sites for `rect.x = 0`,
//     PERM_LINESWAP over the remaining three stores and PERM_GENERAL over the
//     call's rect argument is 24 candidates, seconds of work.
//
// What not to search: statement order around either call. reorg fills a call's
// delay slot from whatever the scheduler left adjacent to the jal, and the
// argument setup is emitted at the call, so an argument load is always adjacent
// and always wins. About 1700 directed variants across three rounds moved
// neither pair. Only something that changes RTL emission order without changing
// the instruction count can help, which is what the randomizer's
// temp-introduction passes do and hand-written alternatives do not.
//
// 4 rows of 1205 instructions still differ, and the floor is 0 -- a match is
// reachable. 1201 instructions are byte-identical, as are .data/.rodata/.bss
// and every register assignment. What is left is two pairs, each a scheduling
// order between two instructions: no wrong value, no wrong register, no
// difference in instruction count.
//
// (An earlier revision claimed 5 rows and a floor of 1, counting a jump-table
// relocation as unfixable. That was a scorer artefact: jtbl_801D0000 is at
// .rodata+0x0, so %lo(jtbl_801D0000) and %lo(.rodata) are the same address and
// the same bytes -- verified at object level, same relocation types and sites,
// identical instruction words. checkfn.py now handles offset-0 section
// references.)
//
// Measure variants with tools/variant_eval.py, not checkfn.py: it scores
// against a private object, so many run at once, ~3 seconds each. Baseline 4.
// against a private object, so many run at once, ~3 seconds each. Baseline 5.
//
// SEVEN THINGS HERE LOOK ODD AND ARE ALL LOAD-BEARING. Every one was measured
// against the alternative that reads better; the number after each is what the
// clean version costs.
//
//   * `value` holds case 3's button pointer (`value = (s32)D_801D1ADC`,
//     `*(u8*)value`). The walking pointer must occupy the *same allocno* as the
//     `value` the first switch uses -- that is what donates the loop-weighted
//     references that keep the s0/s1/s2 ranking right. A separate `u8 *button`
//     is 91, `corner` in its place 91, an index in `value` 97, an index in `i`
//     120, and the natural `value = *button;` 180. So the original used one
//     variable as both an integer and a byte pointer, the same shape as
//     func_801D01C8's `s32 _setting` / `u8 *setting` pair above.
//
//   * case 2's three loop invariants are *named* -- `pa`, `py`, `pr` -- and all
//     five preheader initialisers are spelled out in the for-init in the order
//     loop-invariant motion would have emitted them: i, pa, py, pr, dy. That is
//     what fixes the preheader ordering. LICM emits each hoisted invariant
//     immediately before the loop, i.e. after any for-init, so with `dy` in the
//     for-init its `li 0x21` always takes an early slot and pushes &rect late;
//     naming the invariants moves them into the for-init region where their
//     order is ours to choose, and putting dy last reproduces the target.
//     Naming only &rect and leaving the others hoisted is 14-16.
//
//   * `pa` is `s8 *` at `&D_801D24CC[1].unkA`, read as `pa[0]` / `pa[1]`, not a
//     `Unk80026448 *` read as `pa->unkA` / `pa->unkB` (10). `unkA` and `unkB`
//     are adjacent `s8` (game.h), so the loads are identical; only the base
//     register gcc keeps differs.
//
//   * `py` is `short *` at `&D_801D24A0[1].y`, not a `RECT *` read as `pw->y`
//     (149). RECT.y is a `short` at +2, so `*py` is the same `lh`.
//
//   * `x` is declared once at function scope rather than in both case 2 and
//     case 3. They are mutually exclusive arms of the same switch, both assign
//     x before reading it, and nothing after the switch reads it -- but sharing
//     the allocno is what clears the case-2 load ordering.
//
//   * `w` names the 0x100 that case 3's loop stores into rect.w/rect.h and sits
//     in for-init slot 2, keeping it out of the loop-invariant set. Slot 3 is
//     15 and the bare literal 14. (An earlier revision of this comment claimed
//     19 and 26; those were measured on a much earlier baseline and are wrong
//     for this source.)
//
//   * The sound case writes `Savemap.config &= 0xFFFC;` then `|= setting;`.
//     gcc folds it back to one `sh`; the two-statement form is what makes the
//     result coalesce into the masked value's register. The mask loop's counter
//     is `setting` for the same family of reasons -- it has to be an allocno
//     that crosses a call -- and only pays off together with the `value`
//     pointer above.
//
// WHAT THIS BLOCK IS. func_80026A34 is PSY-Q's SetDrawMode: its arguments are
// (dfe, dtd, tpage, RECT *texture_window), which src/main/18B8.c makes plain by
// calling it as func_80026A34(0, 1, (u16)GetTPage(0, 2, 0x3C0, 0x100), &rect).
// The RECT is a texture window and 0,0,0x100,0x100 means "no windowing", so the
// four stores plus the call are a *batch terminator*: after appending sprites
// that used one texture page, the code appends a draw-mode primitive resetting
// the page and clearing the window so later primitives are unaffected. The
// idiom appears ten times in this repo; in 18B8.c it is the last statement of
// each drawing helper. Case 2 here is the window-colour screen, whose loop
// draws the three R/G/B sliders before this terminator runs.
//
// That matters because it is a fixed idiom, not bespoke code, and every other
// func_80026A34 site in the repo writes the fields in natural x, y, w, h order.
// The y, w, x, h below is therefore a search artefact rather than source truth.
// Written naturally the block scores 6, and the emitted store order is then
// exactly the target's -- the only difference is that the target puts the x
// store ahead of the call's argument setup. Since the target's layout is three
// stores after the arguments and one before, the x store is probably not part
// of this block at all in the original: sinking `rect.x = 0;` to the end of the
// loop and writing the block y, w, h reproduces the target's whole block,
// argument for argument, and also scores 4. Both 4-row forms are equal; the
// sunk one localises the remaining difference to a single instruction, and the
// permuted one below is kept only because it is what was measured first.
//
// WHAT IS LEFT. Three rounds of parallel search -- roughly 1700 measured
// variants -- have not moved either pair, and both now have a mechanism
// argument for why source-level rearrangement cannot reach them. Read these
// before spending more time here.
//
// A permuter run has since settled the question for the search half. With the
// scratch retargeted (base score 220, floor ~40), 1.5M iterations produced
// 30,504 candidates tied at 220 and none below it, and a directed batch of 17
// variants split the same way: everything gcc folds scores exactly 4, and
// everything it cannot fold scores 5, 6, 64, 89 or 149. The neighbourhood is
// flat, not unsearched. Do not run the permuter on this function again --
// see `permuter_macros.py recipe conserved-pair`, which is written from it.
//
//   * 2 rows after case 2's loop: the target emits `sh zero,0x20(sp)`
//     (rect.x = 0) as the first instruction of the block, ahead of the call's
//     argument setup; we emit it with the other three stores. gcc 2.7.2's
//     scheduler is bottom-up: it repeatedly places the highest-priority ready
//     insn at the current tail, so the first-picked lands last, and ties break
//     on the lowest LUID. The call's argument moves are generated last and so
//     have the highest LUIDs, which is exactly why they always end up first.
//     A store's only successor in the block is the call, reached by a memory
//     dependence of cost 0, while an argument move reaches it by a data
//     dependence with a cycle of latency -- so an argument move outranks every
//     store feeding the same call, and no successor can lift a store above it
//     (a successor placed later necessarily has a shorter remaining path than
//     the call). Every attempt to give the x store a real successor is folded
//     by CSE, which knows the value it just stored; anything CSE cannot fold
//     costs an instruction. Confirmed by the rest of the function: there are
//     four other RECT-fill-then-call blocks here, all matching, and one of them
//     -- the block after the switch -- is this block's structural twin, four
//     stores plus a shared constant register feeding the same func_80026A34.
//     The original's own compiler puts every store after the arguments there.
//     So the hoisted store in case 2 probably is not one of that block's four
//     at all, and the answer is a different block structure rather than a
//     different statement order.
//     Sinking `rect.x = 0;` to the end of the loop body does clear these two
//     rows, but reorg then takes the store for the loop branch's delay slot
//     instead of `addiu s2,s2,0xc`, costing two rows at the back-edge. The
//     pair is conserved; total is 4 either way. That holds with `pr` present,
//     across every back-edge form, and in the induction-variable world.
//     Writing the block through `pr` does not help: `pr->x = 0` compiles to
//     `sh zero,0(a3)`, reusing the call's own argument register, which adds a
//     dependence on `move a3,s6` and pins the store harder behind the
//     arguments. Routing x alone re-measures as neutral, not +1 as recorded
//     here earlier -- that figure predates the checkfn alias fix, which moved
//     the baseline from 5 to 4. Routing more than one field still costs.
//
//   * 2 rows in the ATB case: the target fills the `beq`'s delay slot with
//     `li a0,0x1` and the `jal`'s with `addiu s0,s0,1`; we fill them the other
//     way round. Both slots are filled in both builds -- the two instructions
//     are simply swapped. `li a0,0x1` is emitted at the call, so it is always
//     the instruction immediately before the `jal`, and reorg's backward scan
//     always takes it for the call's slot; the branch then gets whatever is
//     left. Adding a third instruction to the pre-call region does not change
//     that. Lengthening the increment's dependence chain is capped: `li a0`'s
//     height is 1 + height(jal), and the increment's chain joins the call's
//     own `lhu -> andi -> or -> sh` two edges downstream, so it cannot
//     overtake. Routing the shift through `setting` does not even buy that
//     edge, because gcc CSEs it back to `value`. Any form that puts the call
//     before the increment leaves the `beq` slot empty and adds a `nop` -- the
//     "cascade" earlier rounds saw is that one extra instruction shifting every
//     later branch, not a register or layout problem. The cursor case confirms
//     the model from the other side: its increment lands in caller-saved v1, so
//     it can never take the call's slot, and the target carries a `nop` in the
//     branch slot there.
// Several variations are byte-identical to what is here and are free if they
// help something else: `value++` instead of `value = value + 1` in the ATB
// case, `rect.y` written first in the second block of case 2's loop, `dy`
// declared first among case 2's locals, `padding` without `volatile` or as
// `volatile RECT[2]`, `dy += 12` moved to the end of the loop body, and the
// ATB store split into `&=` / `|=`.
//
// Ruled out by measurement: making rowY an induction variable (changes the
// saved-register count); `dy = i * 12 + 0x21;` as a strength-reduced induction
// variable, which also clears the preheader ordering but costs 8 elsewhere
// because the fresh giv pseudo loses two references and drops below x and rowY
// -- and cannot regain them, since a giv takes no references from outside the
// loop and loop.c folds every post-loop use to a constant; hoisting case 2's
// `unkA * 3 + unkB * 6` index, which the target recomputes each iteration
// (181); calling func_801D0040 before the increment in the ATB case; merging
// `corner` into `value`; and the declaration order of `value` and `setting`.
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
    s32 x;

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
        s32 dy;
        s32 rowY;
        s32 knob;
        s8* pa;
        short* py;
        RECT* pr;

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
        for (i = 0, pa = &D_801D24CC[1].unkA, py = &D_801D24A0[1].y, pr = &rect,
            dy = 0x21;
             i < 3; i++, dy += 12) {
            func_80029114(
                x + 0x10, rowY, D_801D252C[pa[0] * 3 + pa[1] * 6 + i], 3, 7);
            rowY += 12;

            // the movable knob of one colour slider
            knob = D_801D252C[pa[0] * 3 + pa[1] * 6 + i] >> 1;
            rect.y = *py + dy;
            rect.h = 11;
            rect.w = 8;
            rect.x = knob + 0xD5;
            func_80028030(pr);

            // and the bar it slides along
            rect.x = 0xD5;
            rect.w = 0x88;
            rect.h = 11;
            rect.y = *py + dy;
            func_80027B84(pr);
        }
        rect.y = 0;
        rect.w = 0x100;
        rect.x = 0;
        rect.h = 0x100;
        func_80026A34(0, 1, 0x1F, pr);

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
