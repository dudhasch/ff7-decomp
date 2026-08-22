//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 1 of 5, split out of field.c. .rodata 0x800A0000-0x800A0054, base 0
 * mod 8. Holds jtbl_800A0008, whose `.align 3` in FieldMain.s is a no-op at
 * offset 0x8. */

const u32 D_800A0000[] = {0, 0x01D801E0};

/////////////////////////////////////////////////
// Begin of field_main.c
/////////////////////////////////////////////////

typedef struct {
    u32 datSector; // +0x00
    u32 datSize;   // +0x04
    u32 mimSector; // +0x08
    u32 mimSize;   // +0x0C
    u32 bsxSector; // +0x10
    u32 bsxSize;   // +0x14
} FieldFileInfo;

extern FieldFileInfo g_FieldFileInfo[];
extern void SystemLzsDecompress(void* dst, void* src);
extern s32* g_FieldModelsP;
extern s32 g_FieldTriggers;
extern s32 g_FieldEncounters;
extern s32 D_8007E770;
extern s16 g_CurrentFieldIndex;
extern s32* g_FieldTriggersP;
extern s32* g_FieldEncountersP;
extern u32 g_FieldLzsInfo[];

void FieldLoadMimDatFiles(void) {
    s32 temp;

    if (g_isFieldLoading == 0) {
        DS_read(g_FieldLzsInfo[g_CurrentFieldIndex * 6],
                g_FieldLzsInfo[g_CurrentFieldIndex * 6 + 1], (u32*)0x80128000,
                NULL);
        while (SystemCdromReadChain() != 0) {
        }
    } else {
        while (SystemCdromReadChain() != 0) {
        }
        SystemLzsDecompress((void*)0x801B0000, (void*)0x80128000);
    }
    DS_read(((u32*)g_FieldFileInfo)[g_CurrentFieldIndex * 6],
            ((u32*)g_FieldFileInfo)[g_CurrentFieldIndex * 6 + 1],
            (u32*)0x80114FE4, NULL);
    while (SystemCdromReadChain() != 0) {
    }
    g_FieldTriggers = *g_FieldTriggersP;
    g_FieldEncounters = *g_FieldEncountersP;
    temp = *g_FieldModelsP;
    D_8007E770 = temp;
    g_FieldModelLoaderData = temp + 4;
}

void StopFieldMapPreload(void) {
    if (g_isFieldLoading == 1) {
        SystemCdromAbortLoading();
    }
    D_80071A5C = 0; // needs to be called g_preloadedFieldMapId;
    g_isFieldLoading = 0;
}

/* 0x800DA5C4 == &g_FieldFileInfo[0].mimSize: the preload pulls the MIM, so
 * the size is at [id * 6] and its sector one word before it. */
extern u32 g_FieldFileTable[];
extern volatile u16 g_FieldMoviePlayed;
extern u16 g_FieldPreloadMapId;
extern s32 g_WmPreSector;
extern u32 g_WmPreSize;

/* 13 rows, and the shape of them is worth writing down. The target
 * materialises `0x801B0000` once and reaches it from both paths; this build
 * rematerialises it in each (two extra `lui a2,0x801b`), and the two gateway
 * pointers' `addiu ...,0x18` increments come out in the other order with the
 * argument register on the wrong one. decomp-permuter found a candidate that
 * passes the load address through the dead loop counter -- `i = 0x801B0000;
 * SystemLoadFileBySector(sector, size, i, 0);` -- and it measures no change at
 * all against the overlay (13/2, byte-identical to the body below); the same
 * run's other find, writing the scratchpad through `((s32*)0x1F800000)[n]`
 * rather than a named pointer, is 29/5. Both are permuter noise. See the
 * two-bivs bullet in CLAUDE.md for the increment order: the one incremented
 * *last* keeps the incoming argument register, and this loop needs one bumped
 * first *and* holding it, which two plain increments cannot express. */
/* Pick the nearest gateway on this map and start streaming its MIM off the CD,
 * so a map transition does not have to wait for the read. The gateway table is
 * twelve 0x18-byte records; destFieldId == 0x7FFF marks an unused slot. The
 * distance is squared, in map units, from the player position stashed in the
 * scratchpad at 0x1F800000.
 *
 * Three things this needed that are not obvious from the asm.
 *
 * One walking pointer, not two. The target reads the record base with
 * `lh 0(a3)` and destFieldId with `lhu 0(a1)` where a1 = a3 + 0x12, and reads
 * pos.y1 as `lh -0x10(a1)` -- off the *far* base, at an offset that only makes
 * sense as a negative displacement from it. That reads like two pointers kept
 * 9 halfwords apart, and it is not: it is one biv (`gateway`) plus one
 * strength-reduced giv, with combine_givs merging the two constant offsets
 * (+2 and +0x12) onto a single register based at +0x12 -- the more-referenced
 * of the two. Written as two pointers with an explicit `gateway += 9;` the
 * `addiu a1,a1,0x12` lands *before* the loop's hoisted invariants instead of
 * among them (it is an ordinary statement, not a giv initialiser), and the two
 * increments come out in the wrong order with the argument register on the
 * wrong one -- 13 rows and 2 insertions, and no arrangement of two increments
 * reaches it. `gateway[9]` with the parameter walking is 10.
 *
 * Read g_FieldFileTable through the symbol at every use, not through a `u32*
 * table` local. The target computes the record address twice, in opposite
 * operand orders -- `addu v1,s0,v0` for the sector at -4 and `addu v0,v0,s0`
 * for the size at 0. Through one local both accesses are the same rtx on the
 * same pseudo and cse deletes the second; read through the symbol each time,
 * expand emits two address computations before cse can relate the loads, and
 * both survive. Worth 7 rows, and nothing about the *spelling* of the index
 * reaches it: `(s32)table + id * 24 - 4` against `id * 24 + (s32)table`,
 * pointer-first against index-first, and either one alone are all
 * byte-identical to the plain subscript.
 *
 * Write the call out in both arms of the final if/else. With one call after
 * the if, reorg fills the `bnez` delay slot from the fall-through thread
 * (`sll v0,v1,0x1`), is left with the call's third argument to place, puts it
 * in the first arm's `j` delay slot and duplicates it into the second arm --
 * three rows and an insertion that read as scheduling noise. Duplicated,
 * cross-jumping merges the two calls into one tail, the shared `lui a2,0x801b`
 * ends up ahead of the branch, and fill_simple_delay_slots takes it for the
 * `bnez` on its first pass, which is what the target has. A `u_long* dest`
 * local assigned before the `if` does not reach it (byte-identical -- cse
 * folds the constant back to the call site), and neither does inverting the
 * branch (10/5) nor pre-assigning the world-map arm and overwriting it (12/3).
 *
 * Also load-bearing, from the earlier passes: g_FieldFileTable (0x800DA5C4) is
 * &g_FieldFileInfo[0].mimSize, so the size is `table[id * 6]` and its sector is
 * the word before it; g_FieldMoviePlayed has to be `volatile u16`, or combine
 * folds the (s16) conversion into the load and emits `lh` where the target
 * loads `lhu` and sign-extends separately; and the (s16) cast on
 * g_FieldPreloadMapId has to appear on the table index as well as on the
 * comparison, or gcc reloads the global unsigned and the two uses stop sharing
 * a register. */

// External Declarations
extern u8 D_8009ABF5;
extern u8 g_FieldAnimLock;
extern s16 D_80071A5C;

// D_8009ABF5 = g_FieldState -> command

void PreloadNextFieldMap(FieldEntity* Player, u16* gateway) {
    s32* scratchpad;
    s32 minDist;
    s32 i;
    s32 diffX, diffY, dist;
    s32 sector;
    u32 size;

    minDist = 0x7FFFFFFF;

    scratchpad = (s32*)0x1F800000;
    scratchpad[0] = Player->PosX >> 12;
    scratchpad[1] = Player->PosY >> 12;
    scratchpad[2] = Player->PosZ >> 12;

    if (g_FieldAnimLock == 0) {
        for (i = 0; i < 12; i++, gateway += 12) {
            if (gateway[9] != 0x7FFF) {
                diffX = ((s16*)gateway)[0] - scratchpad[0];
                diffY = ((s16*)gateway)[1] - scratchpad[1];
                dist = diffX * diffX + diffY * diffY;
                if (dist < minDist) {
                    minDist = dist;
                    g_FieldPreloadMapId = gateway[9];
                }
            }
        }
    }

    if (D_8009ABF5 == 3 || (s16)g_FieldMoviePlayed == 1 || D_8009ABF5 == 2) {
        StopFieldMapPreload();
        return;
    }

    if (D_80071A5C == (s16)g_FieldPreloadMapId) {
        return;
    }

    if (0x4DFFF < g_FieldFileTable[(s16)g_FieldPreloadMapId * 6]) {
        return;
    }

    StopFieldMapPreload();
    D_80071A5C = g_FieldPreloadMapId;

    if (D_80071A5C >= 0x41) {
        SystemLoadFileBySector(
            g_FieldFileTable[D_80071A5C * 6 - 1],
            g_FieldFileTable[D_80071A5C * 6], 0x801B0000, NULL);
    } else {
        SystemLoadFileBySector(g_WmPreSector, g_WmPreSize, 0x801B0000, NULL);
    }
    g_isFieldLoading = 1;
}

extern DISPENV g_FieldDispEnv[2];
extern DRAWENV g_FieldDrawEnv[2];
extern DRAWENV g_FieldDrawEnvBg[2];
extern DRAWENV D_80113FE4[2];
extern DRAWENV D_8011409C[2];
extern DRAWENV D_80114154[2];
extern DRAWENV D_8011420C[2];
extern s8 D_800716D0;
extern s16 D_8007173C;
extern u8 D_80071A58;
extern s16 D_8007E768;
extern s32* D_8007EB64;
extern u8 D_8007EBC8;
extern u8 D_8009C6D8;
extern s8 D_8009A057;
extern s16 D_8009A100;
extern volatile u16 D_8009AC18;
/* volatile: the target holds each of these addresses in a general register
 * (lui/addiu/op 0(reg)) where a plain declaration gives maspsx's two-
 * instruction $at macro. A volatile MEM is barred from cse's table, so the
 * address is never folded back into it. Worth 12 rows; see the note on
 * FieldMain. Both symbols are written only here. */
extern volatile s16 D_8009AC1A[1];
extern volatile s16 D_8009AC1E;
extern u16 D_8009AC40[1];
extern volatile u32 D_8009AC3C[1];
extern void func_800128B8(void);
extern void func_800129D0(void);
extern void DebugRunEveryLoop(void);
extern void FieldEventInit(FieldState* state, FieldEntity* entities, s32 arg2);
extern void FieldEntityBgTriggerInit(void* triggers);
extern void FieldEnablePartyModels(void);
extern void FieldEntityLineClear(FieldLine* line);
extern void FieldArrowsInit(SPRT_16* sprites, DR_MODE* dm);
extern void FieldLoadMimToVram(s32 arg0, u8* mim);
/* The walk mesh: a u16 triangle count, then that many 24-byte triangles. */
typedef struct {
    /* 0x00 */ u16 triCount;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ s16 tris[1];
} FieldWalkmesh;

extern s32 FieldMainLoop(void);

/* The field's trigger block opens with a header; only the camera height
 * bias at +0xA is read here. Spelling it as a struct member -- rather than
 * *(u16*)(g_FieldTriggers + 0xA) -- is what lets gcc hoist the load above
 * the scalar stores in front of it: a struct load does not alias a store
 * to a plain extern. */
typedef struct {
    /* 0x00 */ u8 unk00[0xA];
    /* 0x0A */ u16 camHeightBias;
} FieldTriggerHeader;

extern FieldLine D_8007E7AC;
extern volatile s16 g_FieldNextModule;
extern FieldWalkmesh** D_8009A044;
extern FieldBgData** D_8009D848;
extern s16 D_80071E38;
extern s16 D_80071E3C;

/* Parked at 77 changed / 6 inserted of 793 instructions (78 further rows are
 * symbol aliases -- the C reaches g_FieldRenderData's members where the .s
 * names D_800E8F7C and friends). Everything down to func_800128B8 is
 * byte-identical, and so are the jump table, all seven of its arms and the
 * whole tail dispatch. What was learned, and what is left:
 *
 *   - the local `RECT clip` aggregate initialiser is what puts the 8-byte
 *     blob at .rodata+0 and the jump table at +8, which is the layout the
 *     overlay needs. Writing the four fields instead moves the table to 0.
 *     (While the function is parked the .s carries only the table, so the
 *     blob is the file-scope D_800A0000 at the top of this unit.)
 *   - eventCmd is read through a `volatile u8*`. Volatile is not decoration:
 *     it is what makes the target re-load the byte at every test and emit a
 *     separate `andi 0xff` for the zero-extension. Non-volatile, cse folds
 *     all six tests into one lbu and the function is ten instructions short.
 *   - that same pointer is the base register the whole tail addresses off
 *     (0(s1) eventCmd, 1(s1) eventCmdParam, 0x4b fadeType, 0x4d fadeAdjust,
 *     0x63 prevFieldId, 0xf1 the switch selector -- every FieldState offset
 *     minus one). Spelling those as g_FieldStateData.<member> instead costs a
 *     second anchor register (gcc picks &prevFieldId) and 37 rows.
 *   - D_8009AC1E and D_8009AC18 must be volatile: the target loads both with
 *     lhu, and a plain u16 with an (s16) cast folds to lh, a plain u16 stored
 *     into a u8 field narrows to lbu.
 *   - g_FieldBGCameraHeightBias is read as a struct member, not as
 *     *(u16*)(g_FieldTriggers + 0xA): only the struct form lets gcc hoist the
 *     load above the three scalar stores in front of it (7 rows).
 *   - the switch selector goes into a u8 local, not a u32 one -- the u32 form
 *     loses the andi.
 *
 *   - the pre-loop fadeType store is `g_FieldStateData.fadeType = 0`, not
 *     `D_8009AC40[0] = 0`. Same address, same halfword, one row apart: the
 *     member shares a symbol_ref with the fade block inside the loop, so cse
 *     relates the two and `ev` comes out as `addiu s1,<reg>,-0x4b` off the
 *     fade base, which is the form the target has. Through its own symbol the
 *     two are unrelated and `ev` needs its own %hi/%lo pair.
 *
 * UNPARKING THIS FUNCTION MUST ALSO DELETE `const u32 D_800A0000[]` AT THE
 * TOP OF THIS UNIT. It is the same 8-byte blob the local `RECT clip`
 * initialiser emits -- {0, 0x01D801E0} is {0, 0, 480, 472} -- and it exists
 * only because the pinned FieldMain.s references it by name. Left in place
 * alongside the local initialiser the unit emits the RECT twice, jtbl_800A0008
 * lands at .rodata+0x10 instead of +0x8, and every jump-table entry and every
 * branch target after it reads wrong: 7 rows that look like ordinary noise
 * and a red `make build`. This alone was 8 of the 84 rows this note used to
 * quote. Nothing else in the overlay names D_800A0000.
 *
 * =====================================================================
 * **27 rows / 4 insertions -> 8 / 1, at the exact 786.** Two changes, and the
 * second of them is a spelling this note had measured twice and rejected
 * twice -- at 81 rows and then at 73 -- because both measurements were taken
 * against a program the first change had not been applied to. Read that as
 * the standing rule rather than as a fact about `ev`: **a rejected spelling
 * is only rejected against the body it was measured on**, and the two
 * numbers agreeing with each other is not evidence, because they were taken
 * against two bodies that differed only in ways unrelated to the lever.
 *
 *   - **the fade block is written through `(volatile T*)` casts** (cluster 2,
 *     7 rows). The note below calls that dimension exhausted after all 24
 *     source orders of the six stores, every one of which emits `sh v0,4(s3)`
 *     before `sh s2,0(s3)`, and concludes it is sched2's tie-break between
 *     two ready stores. It is -- and sched2 may not reorder two *volatile*
 *     MEMs against each other, which is a knob the source has and the store
 *     order does not. `*(volatile u16*)&g_FieldStateData.fadeType = 1;` and
 *     friends is **27 -> 20**, and casting the pre-loop `fadeType = 0` store
 *     the same way is **20 -> 19**. Two things about the spelling are
 *     load-bearing: it has to be the *member-wise* cast, since
 *     `((volatile FieldState*)&g_FieldStateData)->fadeType` makes the whole
 *     base volatile, costs the hoisted `s3` and is **+7 instructions**; and
 *     only the first two stores need it (1, 2, 3 and all 6 casts all measure
 *     20, so all six is kept -- one qualifier per object, not per store).
 *     The same cast applied to the `(D_800965EC == 1 || == 3)` guard's fade
 *     accesses is 20, i.e. a row worse, so it stops at the two blocks that
 *     write the fade state unconditionally.
 *   - **`ev` is not a variable** (cluster 1, 11 rows, and it closes the
 *     cluster outright). Dropping the local and writing
 *     `((volatile u8*)&g_FieldStateData.fadeType - 0x4B)` at all seventeen
 *     use sites is **19 -> 8**: gcc then makes the address an ordinary loop
 *     invariant, `move_movables` hoists it into the preheader as
 *     `addiu s1,s3,-0x4b` derived from the fade base's own hoist, and the
 *     five `beq` offsets, the `v0`/`v1` swap in the pre-loop block and the
 *     `li s4,0xd` / fade-base ordering all go with it. This is CLAUDE.md's
 *     un-naming lever (`FieldModelBsxTdbModify`, `AddBackgroundToRender`'s
 *     wrap tests) and it reads as a style regression: seventeen copies of a
 *     nine-token expression where a `volatile u8* ev` would do. The target
 *     says otherwise, and the whole of the "no in-loop position for `ev`
 *     satisfies both requirements" analysis below is answered by there being
 *     no assignment at all.
 *
 * Everything the earlier sweeps had rejected was re-measured against the
 * 19-row body before `ev` was un-named, and the spread is worth keeping
 * because it shows how far a stale rejection can be off: `ev` at the loop
 * top 87 at +7, after the `== 1 || == 3` block 65 at +1, after the
 * `!= 5 && != 0xD` block 65 at +1, after the ClearImage guard 13, before
 * `func_800128B8()` 20, the two pre-loop statements swapped 20 -- and
 * un-named **8**. The one the note had called the shape "that would let cse
 * hoist it as a movable derived from the `&D_8009AC40` movable, which is
 * what the target's preheader shows" was right about the mechanism and wrong
 * about the number, by 65 rows.
 *
 * Also measured against the 19-row body and rejected: the fade block reached
 * through `D_8009AC40` rather than through `g_FieldStateData` is *exactly
 * inert* when every fade access moves together (27 at the time) and **+7
 * instructions** for any mixture of the two symbols, so the target's
 * relocation naming that address is a `.s` fact and not a source one; the
 * first `if`'s six statements written through `ev` the way the `*ev == 1`
 * and `g_FieldNextModule == 5` arms write theirs -- the sibling-asymmetry
 * reading, and the most plausible-looking hypothesis of the session -- is
 * +4 instructions and 93 rows, its read alone 28, its two stores alone +5.
 *
 * The residue is **8 rows / 1 insertion at the exact 786**, and it is
 * clusters 3 and 4 below and nothing else. Re-swept at this base:
 *   - cluster 3 (the rain fill loop, 4 rows): `i`/`fill` swapped 12, `fill`
 *     hoisted above the if/else 55 at -1, `i` hoisted 49 at -1, the arms
 *     inverted 14, the `do`/`while` as a `for` 12, `fill` as a giv
 *     (`(&D_8009A057)[i - 0xF]`) 42 at +1, `i = 0xF` duplicated into both
 *     arms inert, an empty `do { } while (0); ` after the if/else inert.
 *     Two inert results on constructs that end a basic block say the residue
 *     is reorg's, not sched2's: reorg copies the join's `li v1,0xf` into the
 *     `j`'s delay slot and redirects past it, where the target leaves the
 *     slot a `nop`.
 *   - cluster 4 (the `*ev == 1` arm, 4 rows): the `preloadId` read moved
 *     after the 0x63 store inert, moved between the store and the `+1` read
 *     inert, `preloadId` dropped and `D_80071A5C` read inline at the compare
 *     inert, the 0x63 store non-volatile 34 at -1, the `+1` read
 *     non-volatile 34 at -1, the `+1` read before the 0x63 store 34 at -1,
 *     the compare before the `g_CurrentFieldIndex` store 15, a `u16` local
 *     for the saved index 35 at -1, a barrier between the store and the read
 *     34 at +5, `preloadId` read through a `volatile s16*` 32 at +2.
 *   - `width_sweep.py` over all six scalar locals, 30 variants: **flat**.
 *     Nothing ties 8 and the best alternatives are 15.
 *
 * Both clusters are now closed by *mechanism* rather than by exhaustion, and
 * the two verdicts are different:
 *
 *   - **cluster 3 belongs to reorg and nothing in the source reaches it.**
 *     Five more basic-block boundaries -- at the rain join before `i = 0xF`,
 *     at the end of the then arm, `fillVal` moved to the join, `fillVal`
 *     moved below `fill`, and both `i` and `fill` duplicated into the two
 *     arms so the join has no first insn at all -- are **exactly 8, every
 *     one**. An empty `do { } while (0); ` cannot change an allocno's rank
 *     or a reference count; all it does is end a basic block, so a sweep of
 *     them that is inert at every placement says the residue is not a
 *     scheduling or a block-layout fact. What is left is
 *     `fill_slots_from_thread` copying the join's `li v1,0xf` into the `j`'s
 *     delay slot and redirecting past it, which it may do because `v1` is
 *     dead on the fall-through path -- and `v1` is dead there in the target
 *     too. Park it and say so.
 *   - **cluster 4 is one RTL insn in the wrong slot, and the registers are
 *     already right.** `tools/qty_pri.py` names both quantities and both
 *     match the target: the `D_80071A5C` load is `pri 10.667, 4 refs / life
 *     3 -> $v1` and the `g_CurrentFieldIndex` load `pri 4.000, 4 refs / life
 *     4, size 2 -> $v0`. So this is not the `FieldBackgroundInitPackets`
 *     anti-dependence shape -- nothing is holding `$v1` -- it is sched2
 *     picking the other of two ready insns. Note that `lh v1,%lo(D_80071A5C)`
 *     is *one* RTL insn that maspsx expands to `lui`+`lh`, so what has to
 *     move is a single insn across two volatile MEMs, and the three boundary
 *     placements that would force it (before the read, after it, after the
 *     whole group) cost +5, +7 and +2 instructions.
 *
 * The residue is **33 rows / 6 insertions**, measured with that object
 * deleted. Measured with it still in place it reads 40/6, and the extra seven
 * are the `.rodata` offsets the object itself causes: it is 8 bytes that the
 * unparked function emits as its own RECT blob, so while both exist the jump
 * table sits at `.rodata+0x10` against the target's `+0x8` and every table
 * slot reads wrong. That is the CLAUDE.md trap seen from the measuring side
 * rather than the landing side. Delete the object in the scratch copy before
 * quoting a number for this function; everything below is against 33/6 unless
 * it says otherwise.
 *
 * The lever that took it from 65 to 40 is `volatile` on the *casts*, not on
 * the pointer: `ev` is a `volatile u8*`, but `*(u16*)(ev + 0x63)` casts the
 * qualifier away, so gcc was free to hoist the `ev + 1` load above the
 * `ev + 0x63` store and fill both of the target's load-delay slots with it.
 * `*(volatile u16*)(ev + N)` at all eight sites pins them in source order and
 * the two `nop`s appear. It also removes an `andi 0xffff`: with the load
 * hoisted, `fieldId`'s pseudo had two uses and combine would not fold the
 * zero-extension into the `lhu`. Measured against it and worse: `fieldId` as
 * `s32` or `u32` (54/5 with the casts, 69/4 without), `preloadId` as `s32` or
 * `u32` (60/6).
 *
 * The qualifier is wanted at every site, which is the part that does not
 * follow from the reasoning: dropping it from the `ev + 1` reads alone is
 * 58/5, from the `ev + 0x4B`/`0x4D` stores 44/6, from both 61/5, from the
 * `ev + 0x63` stores 62/3, and from `0x63` plus `0x4B` 62/3. That matters
 * because one row in the tail argues the other way -- the target's
 * `lhu v0,1(s1)` feeds both the `g_CurrentFieldIndex` store and the compare
 * with no mask between them, and a *volatile* HImode MEM cannot be folded
 * into a `zero_extend`, so `expand_expr` reads it into an HImode pseudo and
 * widens it with an `andi 0xffff`. That reads as proof the target's `ev + 1`
 * is not volatile, and every combination of the plain read with a wider local
 * has been measured and is worse: plain + `s32 fieldId` 62/5, + `u32` 62/5,
 * + `s32 fieldId` and `s32 preloadId` 59/5, + `s32 preloadId` alone 60/6, and
 * `s32` on both with the volatile kept 53/5. So the `andi` is real and buying
 * it back costs more than it saves; whatever the original wrote is not
 * reachable by moving the qualifier or the two local types.
 *
 * The live clusters, against the 40/6 base:
 *   1. the pre-loop block. The target's preheader hoists five invariants into
 *      callee-saved registers -- `ori s2,zero,1`, `ori s5,zero,3`,
 *      `ori s4,zero,0xd`, the `%hi`/`%lo` of D_8009AC40 into $s3, and then
 *      `addiu s1,s3,-0x4b` for `ev` -- and the pre-loop `fadeType = 0` store
 *      builds its own copy of the address in $v0 and derives nothing. Here
 *      `ev` is derived at that pre-loop store instead, one instruction early.
 *      For gcc to hoist it, `ev` has to be assigned inside the loop, and every
 *      placement measured is much worse: loop top 100/13 against the current
 *      base (114/12 against the older one), before the fade `if` 140/22, after
 *      the fade `if` 87/7, before its first use 89/7. 5 rows, 2 insertions.
 *      The same block also shows `addiu s0,sp,0x18` -- the target computes
 *      `&clip` into a callee-saved register before the five-way guard chain
 *      and passes it with `move a0,s0`, where this build computes it straight
 *      into $a0 at the call. A named `RECT* clipP` assigned anywhere in the
 *      pre-loop block is exactly inert at all five placements measured: the
 *      address is `sp + 0x18`, cse rematerialises it, and a pointer local
 *      cannot pin it.
 *      Every in-loop placement of `ev` re-measured on the 40/6 base and all
 *      much worse: loop top 100/13, after `g_FieldPreloadMapId = 0` 100/13,
 *      before the `D_800965EC == 2` block 53/8, before the `D_80095DD4` spin
 *      134/19, before the fade `if` 133/15, before FieldEnablePartyModels
 *      127/15. Dropping the local and spelling the expression inline at all
 *      seventeen use sites -- the shape that would let cse hoist it as a
 *      movable derived from the `&D_8009AC40` movable, which is what the
 *      target's preheader shows -- is 81/6. The local is right and its
 *      position is wrong, and no position reaches it -- but the loop-top
 *      placement now has a diagnosis rather than just a number. There `ev`
 *      *is* hoisted into the preheader, ahead of the three constants, as
 *      `lui s1,%hi(g_FieldState) / addiu s1,s1,%lo(g_FieldState+0x1)` -- its
 * own two-instruction address rather than the target's one-instruction `addiu
 * s1,s3,-0x4b` -- and the fade base's own `lui s3 / addiu s3` *disappears*,
 * because the fade stores are then addressed as `0x4b(s1)` off it. `scan_loop`
 * records movables in insn order and `move_movables` hoists them in that order,
 * so whichever of the two is referenced first in the loop body becomes the
 * anchor and the other is folded into it.
 *      `*ev = 0;` sits in the `D_800965EC == 2` block, which is ahead of the
 *      fade block, so `ev` wins -- and the target wants the fade base to win.
 *      The only placement that hoists at all is the very top of the loop
 *      (anything after a conditional branch is not on the always-executed
 *      path and stays in the body: measured, `lui s1` appears inline there),
 *      and at the top there is no earlier computation of the fade address in
 *      the block for cse's `use_related_value` to relate it to. The pre-loop
 *      assignment gets the `-0x4b` form precisely because the store next to it
 *      has just materialised that address. Those two requirements -- hoisted,
 *      and related to a fade address computed earlier in the same block -- have
 *      no position in this function that satisfies both.
 *      Also measured against 33/6: the two pre-loop statements swapped 34/7,
 *      `ev` assigned before `func_800128B8()` 34/7, the pre-loop store written
 *      through `ev` as `*(volatile u16*)(ev + 0x4B) = 0` 34/5, and written as
 *      `D_8009AC40[0] = 0` 34/7.
 *   2. CLOSED. D_8009AC1A[0] = 2 and the D_8009AC3C read wanted `volatile` on
 *      the declarations. The target materialises each address into a general
 *      register (lui/addiu/op 0(reg)) where a plain declaration gives maspsx's
 *      $at macro; a volatile MEM is never entered in cse's table, so the
 *      address stays in its own pseudo instead of being folded back into the
 *      MEM. This note and CLAUDE.md both used to record volatile as measured
 *      and rejected here -- it is not, and it was worth 12 rows, not the 4 the
 *      cluster was quoted at, because the two extra registers renamed a third
 *      of the function. A volatile cast at the access site (`*(volatile
 *      s16*)D_8009AC1A = 2`) measures the same; spelling them as the struct
 *      members they are (movieCommandState +0x26, nextFieldMusic +0x48) is
 *      codegen-identical to the plain form and does not reach it, and named
 *      pointer locals are worse (80/6).
 *   3. the fade block. The target stores fadeType, fadeAdjust, fadeSpeed
 *      (offsets 0, 4, 2 from $s3); this build stores fadeAdjust, fadeType,
 *      fadeSpeed. All six source orders have been measured and the dimension
 *      is finished, not merely unexplored: fadeType/fadeSpeed/fadeAdjust --
 *      what is written here -- is 40, fadeType/fadeAdjust/fadeSpeed 42, and
 *      the four that do not write fadeType first are 98 to 100. So fadeType
 *      first is load-bearing and the remaining swap is sched2's, not source
 *      order's: three constant stores at distinct offsets off one hoisted
 *      base, all ready at once, and the tie goes the other way. 6 rows.
 *   4. the tail. Half of this closed: the `(s16)` cast on `fieldId` in
 *      `if ((s16)fieldId != preloadId)` was producing a sll/sra pair on a
 *      value that `lhu` had already zero-extended. Dropping it -- the two
 *      operands are `u16` and `s16`, and for an equality test the signedness
 *      of the promotion cannot matter -- is worth a row and an insertion.
 *      What is left is that the target compares the raw `lhu` result against
 *      an `lh` of D_80071A5C, where this build still masks with
 *      `andi v1,a0,0xffff` because the `u16` local is live across the
 *      g_CurrentFieldIndex store as well. Measured against this base and all
 *      worse: `fieldId` as s32 or u32 (68/4), as s16 (65/4), dropping the
 *      `preloadId` local and reading D_80071A5C inline (66/4), the compare
 *      re-reading `*(u16*)(ev + 1)` (66/5), both (66/3), `preloadId !=
 *      fieldId` (68/4). Dropping the `fieldId` local from the 0xC arm alone
 *      measures 63/4 against 64/3 -- the same total, so it is left out.
 *      Earlier, against the pre-volatile base: s32 temp 76/7, no temp 76/7,
 *      temp after the store 76/7, dropping the fieldId local 76/6. 9 rows.
 *      A `do { } while (0);` barrier after the 0x63 store -- decomp-permuter's
 *      best find on the aligned scratch, 2080 against a base of 2505 -- is
 *      deliberately *not* taken: it measures 59/5 against 65/3, six changed
 *      rows turned into matches at the cost of two more instructions than the
 *      target has, and a body that is longer is further from matching however
 *      the row count reads. One barrier is the whole of it; two, three, four
 *      and a barrier before the store all measure the same 59/5 or worse, as
 *      does a temporary for the addPrim address the same candidate carried
 *      (inert on its own). Its next find after the target.s rewrite (1920
 *      against 2505) is four mutations and all four are inert or worse here:
 *      `preloadId` as `u16` 67/4, a `DRAWENV*` temp for the SetDefDrawEnv
 *      argument inert, reusing `fillVal` for the `D_80095DD4 = 1` store in the
 *      0x40-range arm inert, and reusing it again as the `ev + 1` offset
 *      inert. The `otSlot` trick that was worth 34 rows in
 *      AddBackgroundToRender does not transfer to this function. Its finds on
 *      the repaired-and-aligned scratch (1455, 1475, 1490 against a base of
 *      1780) are all `preloadId` and `fieldId` retypings plus two dead
 *      assignments; measured here, `int preloadId` is 60/6, `newVar = 3;`
 *      before `func_800129D0()` for the fadeType store is inert, an extra
 *      `u16 pad;` local is inert, and reusing `i` for the
 *      `g_FieldModelsP = (s32*)0x80114FFC` store is 65/6.
 *      Nor does the inserted-block class, swept a second time across the
 *      whole function rather than around the addPrim: after the pre-loop
 *      `fadeType = 0` store 88/2, after the `ev` assignment 87/1, after the
 *      rain fill loop inert, after the fade block's last store inert. The two
 *      that move anything move it the wrong way -- they cut the insertion
 *      count by pulling the `ev` derivation to the *top* of the block, where
 *      the target derives it from the fade base further down, which is
 *      cluster 1 undone. The 87 is not 87 independent faults either: one
 *      instruction of length difference renames every branch offset after it,
 *      so a body that is one insertion closer can read twenty rows worse.
 *   5. the fill loop's two constants. `fillVal = -1;` written *above* the rain
 *      if/else is what puts `li a0,-1` in the guard's delay slot, the way the
 *      target has it -- the same lever as the `white` local in
 *      FieldBackgroundInitPackets, and the reason the older note here recorded
 *      "a named local changes nothing" is that it was assigned just before the
 *      loop rather than above the branch. Position, not existence. The type is
 *      inert (s8, s16 and s32 all measure 77/4). What is left is the other
 *      constant: the target's `j` out of the first rain arm has a `nop` in its
 *      delay slot and `li v1,0xf` at the join, where reorg steals the `li`
 *      into the slot here. Measured and rejected: `fill` assigned before `i`
 *      (80/5), `i = 0xF` hoisted above the branch too (92/4), `fill` hoisted
 *      instead (97/5), the arms written as a goto rather than an else (77/4,
 *      byte-identical). 3 rows.
 *      Re-swept against 33/6: the `do`/`while` written as
 *      `for (; i >= 0; i--)` is byte-identical, the rain arms inverted 38/7,
 *      `fill` assigned before `i` 36/7, `fillVal` moved below the rain
 *      if/else 32/7 (a tie on total rows, one more insertion), and `fillVal`
 *      moved below `fill` 36/7.
 *   6. six rows that were never real. The target names six interior addresses
 *      of g_FieldRenderData by their own splat labels (D_8010068C, D_8010080C,
 *      D_80100818, D_8010081C, D_80100820, D_80100860) where this C reaches
 *      them as `g_FieldRenderData + 0x1BA28` and the like. objdump renders a
 *      %lo operand as the instruction's signed immediate rather than as the
 *      relocation's addend, so those printed as `%lo(g_FieldRenderData
 *      -0x45d8)` and checkfn resolved two addresses 0x20000 apart for one
 *      address and one halfword of object code. checkfn now compares a %lo by
 *      the halfword it emits and the six are aliases; the residue is 65/3, not
 *      71/3, and no source change was involved.
 *   7. what decomp-permuter has said so far is worth nothing, twice over.
 *      Its scratch was built by `permuter_macros.py align`, which rewrote
 *      `extern volatile u32 D_8009AC3C[1];` into a second declaration of
 *      `D_8009AC40` -- so `D_8009AC3C` was undeclared at
 *      `D_8009A004[0] = D_8009AC3C[0];`, gcc folded it to 0, and the search
 *      ran against a program five instructions shorter than this one. Its best
 *      candidate there (2515 against a base of 3100) measures 79/6 here, and
 *      each of its six edits measured separately is a regression too:
 *      `long preloadId` + `int exitKind` 73/5, `ev =
 * &g_FieldStateData.eventCmd` with `D_8009AC40[0] = 0` 72/4, the `fillVal`
 * local folded back inline 70/4, a `do { } while (0)` around the
 * `g_CurrentFieldIndex` store 70/5, the `(s16)` cast back on the compare 72/4,
 * dropping `volatile` from D_8009AC1A 81/3. On the repaired base its next find
 * -- `idxOfs = 0x63;` at the top of the frame loop, used at one of the two
 *      `*(u16*)(ev + 0x63)` stores -- scored 2085 against 2585 and measures
 *      103/6 at the first store, 79/6 at the second, 90/6 at both.
 *      The scratch's target.s has since been rewritten to name the six
 *      g_FieldRenderData interiors and D_8009AC40 the way this C does, so its
 *      score now describes the code; only the RECT blob and the jump table are
 *      still scored as aliases, and those are constant across candidates.
 *   8. also measured against this base and rejected: `(s32)fieldId !=
 *      preloadId` (inert), `preloadId != (s32)fieldId` (69/4), `s32 preloadId`
 *      (67/5), a `do { } while (0)` barrier between the fadeType and fadeSpeed
 *      stores (106/11), and writing the fade block fadeType/fadeAdjust/
 *      fadeSpeed rather than fadeType/fadeSpeed/fadeAdjust (67/3).
 * The remaining rows are branch-target addresses, which follow from the
 * length difference and cost nothing once the clusters above are closed.
 *
 * **81 rows / +1 instruction -> 54 changed / 8 inserted, at the exact 786.**
 * Two local *widths* in the tail, and both were within the dimension this
 * note calls finished. The note's numbers for that block were quoted against
 * a 65/3 base and the function measures 81/9 now, so every one of them was
 * stale -- that is the whole lesson here, and it is the same one
 * FieldEntityWalkmechCross and LoadLocalFieldModelAndInitAll taught this
 * session.
 *
 *   - **`preloadId` is `s32`, not `s16`.** The note records `u16` (67/4) and
 *     never tried the signed wide form. `D_80071A5C` is an `s16`, so an `s32`
 *     local takes it with a sign-extending `lh` -- the one `lh` in the target
 *     that this build did not have anywhere -- instead of `lhu` plus a
 *     separate widening. 81/+1 -> 62 and the length exact.
 *   - **`fieldId` is `s32` too** (`u32` is byte-identical). 62 -> 54.
 *
 * `insn_histogram.py` is what made this tractable and is worth re-running
 * before anything else here: at 81 rows it read `ori +2 / addiu -2`,
 * `lhu +1 / lh -1`, `lui +1`, `andi +1`, `nop -1` -- seven opcode counts, of
 * which the `lh` is a *declaration* fact rather than codegen, and it named
 * the one local in the function whose type had never been swept wide.
 *
 * **47 rows / 8 insertions -> 27 / 4**, measured the honest way with
 * `D_800A0000` deleted as an unparking build must. On the other footing --
 * the object left in place, which is what the 54 quoted above was -- the
 * same pair reads **54 -> 34**; the seven-row gap is the `.rodata` offsets
 * the duplicated RECT blob causes and is constant across every measurement
 * below. Length stayed at the exact 786 throughout. Two changes did it, and
 * the first of them is a lesson about how this note was being read:
 *
 *   - **Two clusters had to move together, and each is a regression alone.**
 *     The `FieldEventInit` argument (`ori +2 / addiu -2` above) is worth the
 *     `addiu a0,a0,-0xa6` delay slot and costs an instruction: 47 -> 51 at
 *     **785**. `s16 fieldId` -- the width the paragraph above had just
 *     changed to `s32` -- buys the target's `sll`/`sra` sign-extension and
 *     costs the `andi`, +1 instruction: 47 -> 74 at **787**. Applied
 *     together they cancel exactly and the pair is 47 -> **39** at 786. The
 *     previous pass had measured each on its own, read "-1 instruction" and
 *     "+1 instruction" as two independent failures, and parked both. When
 *     two residues in one function have opposite length signs, try the
 *     product before believing either sum.
 *     The argument spelling that works is plain `&g_FieldStateData`: the old
 *     `&g_FieldState` is a *different* `symbol_ref` for the same address, so
 *     cse's `use_related_value` could not relate it to the `%lo` the scroll
 *     stores had already materialised. checkfn aliases the two names, which
 *     is why the row only ever read as register naming.
 *   - **The `*ev == 0xC` arm needs its own field-id local.** It shared
 *     `fieldId` with the `*ev == 1` arm; giving it `s16 exitId` is **39 ->
 *     27** and closes the 0xC arm outright *and* three of the 0x1 arm's
 *     rows. This is CLAUDE.md's "split what describes two things" run on a
 *     scalar: one pseudo serving two unrelated live ranges in two unrelated
 *     blocks loses `$v0` to the other quantity in both, and every row of it
 *     reads as `v0`/`v1` noise. Declaration position of `exitId` is inert
 *     (first or last in the local list, both 27).
 *
 * The residue is **27 rows / 4 insertions at 786**, in four clusters. Every
 * one of them is now measured to the end of its dimension:
 *
 *   1. the pre-loop `ev` derivation -- 12 rows, and unchanged in kind from
 *      cluster 1 above. Ours derives `ev` in the pre-loop block (one
 *      instruction before the ClearImage guard, which is why five `beq`
 *      offsets read 0x77c against 0x778); the target derives it in the
 *      preheader as `addiu s1,s3,-0x4b` after the fade base, and puts
 *      `li s4,0xd` *before* the fade base rather than after. Re-swept at
 *      this base: pre-loop (27, best), function top 29, in-loop after the
 *      fade `if` 40, own-symbol pre-loop store + that 40, in-loop before the
 *      `D_800965EC == 2` block 42, between the ClearImage guard and the loop
 *      78, loop top 106, before the `D_80095DD4` spin 140. `ev` spelled
 *      `&g_FieldStateData.eventCmd` is byte-identical to the `-0x4B` form
 *      (27). `D_8009AC40[0] = 0` for the pre-loop store is 41 at the 39-base
 *      and 40 combined with an in-loop `ev`. The two requirements -- hoisted
 *      by `move_movables`, *and* derived from a fade base the same block has
 *      already materialised -- still have no position that satisfies both.
 *   2. the fade block, 7 rows: the target emits `sh s2,0(s3)` (fadeType)
 *      before `sh v0,4(s3)` (fadeSpeed) and this build emits them the other
 *      way round. Nothing else in the block differs. **The source-order
 *      dimension is now exhausted**: all 24 permutations with R/G/B in
 *      order were measured, plus the six of type/speed/adjust from the
 *      earlier sweep. Every one of them emits `sh v0,4` first, so no
 *      spelling reaches it -- this is sched2's tie-break between two ready
 *      stores off one hoisted base, one of which needs the constant the
 *      branch delay slot already carries. Four orders score **24**
 *      (T,S,R,G,B,A and T,R,S,G,B,A and T,R,G,S,B,A and T,R,G,B,S,A) and
 *      **none of them is taken**: they win three rows only by moving the
 *      `fadeAdjust` store to the end of the block, where the target does not
 *      have it, so the emitted order goes from one fault (0/4 swapped) to
 *      two (0/4 swapped *and* 2 displaced past 6/8/0xa). At equal length a
 *      row count is the metric, but it is the metric for the *same* set of
 *      faults; a body one transposition from correct is closer than a body
 *      three, whatever the differ's alignment says. The natural order --
 *      type, speed, adjust, red, green, blue, which is what anyone would
 *      type -- is kept.
 *      Barriers are worse in every position: after fadeType 104, after
 *      fadeSpeed 91, after fadeAdjust 92 (they cost the `li v0,0x10` its
 *      branch delay slot). A chained `fadeRed = fadeGreen = fadeBlue = 0` is
 *      inert (27), which is the descending-store rule holding.
 *   3. the rain fill loop, 4 rows: reorg copies the join's `li v1,0xf` into
 *      the delay slot of the first arm's `j` and retargets it, where the
 *      target leaves the slot a `nop`. Measured and rejected at this base:
 *      `fill = &D_8009A057;` before `i = 0xF;` 43, the `do`/`while` written
 *      as a `for` with the same swap 43, the arms inverted 45, both inits
 *      hoisted above the if/else 48, `i` alone hoisted 68, `fill` alone
 *      hoisted 74, `i = 0x10` with `while (--i != 0)` 28 at +1 instruction,
 *      `while (--i != -1)` 56 at +1, `*fill = fillVal; fill--;` split into
 *      two statements inert, the if/else written as an explicit
 *      `goto rainDone;` inert, a `do { } while (0);` between the if/else and
 *      `i = 0xF` inert. Inert barriers say this is reorg's, not sched2's.
 *   4. the `*ev == 1` arm, 4 rows: the target puts `lui`/`lh D_80071A5C`
 *      into the load-delay slot after `lhu %lo(g_CurrentFieldIndex)` and
 *      this build puts it four instructions later, into the *other* delay
 *      slot. Same instructions, same count, two `nop`s trading places.
 *      Every statement order measured is exactly inert at 27: `preloadId`
 *      read after the 0x63 store, after the `ev + 1` load, read inline at
 *      the compare, the compare operands swapped, the `(u16)` cast dropped,
 *      a `u16 curId` local for the `g_CurrentFieldIndex` read. `preloadId`
 *      as `s16` is 60, read through a `volatile s16*` 51, and moving the
 *      `g_CurrentFieldIndex` store inside the `if` 64. A `do { } while (0);`
 *      in the arm is +1 instruction and 61.
 *
 * Two further things measured at this base and worth not re-deriving:
 * `exitKind` as `s32` or `u32` is inert; reading it before the `ev + 1` load
 * is 57 and dropping the 0xC arm's field-id local entirely is 47.
 *
 * `insn_histogram.py` agrees with the rows now and did not before. It used to
 * report `ori +2 / addiu -1 / nop -1`, which reads as three faults cancelling
 * to the exact 786 -- two constants materialised that the target does not
 * have, against an `addiu` and a load-delay `nop` it has and we do not. One of
 * those three was the tool: objdump renders this function's single
 * `addiu $a0,$zero,-0x1` as `li a0,-1` and the fold was by mnemonic name, so
 * every negative `li` was credited to `ori` and an equal `addiu` deficit
 * invented (fixed in 7e68f5f, folding by encoding). Decoded, the table is
 * `ori +1 / nop -1` and that is **cluster 3 and nothing else** -- one extra
 * `li v1,0xf` in the `j`'s delay slot against the target's `nop`. So the
 * opcode table now carries no information the four clusters do not, which is
 * itself worth knowing: there is no fifth fault hiding behind the exact
 * length.
 * The three `%hi` rows it still prints (`.rodata` x2 ours against
 * `D_800A0000` and `jtbl_800A0008` x1 each) are the local-label naming gap in
 * that tool, not the parked-blob artifact -- they are unchanged with
 * `const u32 D_800A0000[]` deleted.
 *
 * Re-swept at the 27-row base, and every one of these is a *new* dimension or
 * a re-measurement of a stale number rather than a repeat:
 *   - cluster 1, `ev` in the loop body so `move_movables` can hoist it into
 *     the preheader the way the target does. This is the whole hypothesis for
 *     those 12 rows and it is now closed from both ends. After the
 *     `D_800965EC == 1 || 3` block (the first in-body use of the fade base,
 *     so the discovery order would be right) is **40 at the exact 786** --
 *     a different fault set of the same length, not a longer one. Inside the
 *     `== 2` arm 138, before the `!= 0xD` fade block 149 at +8, after that
 *     block 134 at +1. The +1/+8 are the point: `ev` is used at `*ev = 0`
 *     earlier in the body than those two placements, so the pseudo is live
 *     across the back edge and `move_movables` will not touch it -- gcc
 *     rematerialises `lui`/`addiu` instead of deriving `addiu s1,s3,-0x4b`.
 *     No in-body position both dominates every use and is discovered after
 *     the fade base, which is the same wall the earlier sweep hit from the
 *     spelling side.
 *   - `ev` as a declaration *initialiser* placed above `RECT clip` -- a
 *     different knob from an assignment statement, since the aggregate's blob
 *     copy is a scheduling barrier and a declaration above it is emitted
 *     first (CLAUDE.md, FieldEntityTriggerCheck). Exactly inert, 27. As an
 *     initialiser below the aggregate, also 27. Swapping the pre-loop store
 *     and the `ev` assignment is 29.
 *   - cluster 3, the loop's *shape* rather than its statement order: a `for`
 *     with `fill--` in the increment list 31, `fill = ...` before `i = 0xF`
 *     31 (the old note says 43 -- stale, the base moved under it), a
 *     `for (;;)` with `if (--i < 0) break;` 60 at +3. Making `fill` a giv
 *     instead of a hand-walked pointer -- `(&D_8009A057)[i - 0xF]` and
 *     `(&D_8009A057 - 0xF)[i]`, which is the lever that decides delay-slot
 *     stealing in KawaiSetModelTransparency -- is **61** either way, and 61
 *     as a `do`/`while` too. So the giv/ordinary-insn distinction is not what
 *     separates these two bodies; reorg steals the join's `ori v1,zero,0xf`
 *     here and leaves it in the target for a reason no loop shape reaches.
 *   - `width_sweep.py` over all eight scalar locals, 30 variants: **flat**.
 *     Nothing ties the 34-row parked base and everything else is worse
 *     (best alternatives `exitId`/`exitKind`/`fillVal`/`preloadId` at 34,
 *     then 35, then 54+). The width dimension is closed.
 *   - the paired-lever cross from CLAUDE.md -- a placement that moves the
 *     length one way against a width that moves it the other, which is what
 *     took this function from 47 to 39. Twelve crosses of the four `ev`
 *     placements with `fieldId`->`u16` (-1), `exitKind`->`u16` (-2) and
 *     `i`->`u16` (-8): every one is worse than both of its components
 *     (60, 63, 118, 124, 136, 140, 151, 151). The pattern is real and it does
 *     not fire here.
 *
 * Swept once more at the 27-row honest base, this time asking the
 * pseudo-structure question rather than the spelling one (the residue is a
 * permutation at the exact 786, so CLAUDE.md's 0-insertion rule applies and
 * the answer, if there is one, is a count of variables). Nothing moved:
 *
 *   - cluster 1's hoist order read as a `scan_loop` fact rather than as an
 *     `ev` placement. `move_movables` emits in the order `scan_loop` recorded
 *     the movables, which is insn order in the loop body, and the target's
 *     preheader is `1, 3, 0xd, fadeBase, ev` against this build's
 *     `1, 3, fadeBase, 0xd` -- so in the original the constant 0xD is
 *     referenced *before* the fade base, i.e. before `fadeType == 0` at the
 *     top of the loop. Writing that guard as a nested `if` so the fade base's
 *     reference sits inside a conditional arm (where `move_movables` will not
 *     take it) is **exactly inert at 27**; putting the fadeType test first in
 *     the `&&` instead is 43/9. The only source shape that would record 0xD
 *     first is one where the `D_800965EC != 5 && != 0xD` block precedes the
 *     fade block, and that is a different program.
 *   - cluster 3 asked as "how many `i = 0xF` assignments does the original
 *     have?", since reorg can only steal the join's `li v1,0xf` if there is a
 *     join to steal it from. Duplicating `i = 0xF;` into both rain arms is
 *     inert (27); duplicating both `i` and `fill` into both arms is inert
 *     (27) -- cross-jumping merges the copies straight back and the join is
 *     rebuilt. The if/else collapsed to `g_RainForce = (g_RainControl & 0x80)
 *     == 0 ? 0 : 0xFF;` is 80 at -7 instructions, so the branch is real.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldMain);
#else

void FieldMain(void) {
    RECT clip = {0, 0, 480, 472};
    s8* fill;
    s32 fillVal;
    s32 i;
    s16 fieldId;
    s32 preloadId;
    s16 exitId;
    u8 exitKind;

    ClearOTagR(&g_FieldRenderData[0].OtFadeDrenv, 1);
    ClearOTagR(&g_FieldRenderData[1].OtFadeDrenv, 1);
    SetDrawEnv(&g_FieldRenderData[0].FadeDrenv, &g_FieldDrawEnv[0]);
    SetDrawEnv(&g_FieldRenderData[1].FadeDrenv, &g_FieldDrawEnv[1]);
    addPrim(&g_FieldRenderData[0].OtFadeDrenv, &g_FieldRenderData[0].FadeDrenv);
    addPrim(&g_FieldRenderData[1].OtFadeDrenv, &g_FieldRenderData[1].FadeDrenv);
    SetDefDrawEnv(&g_FieldDrawEnvBg[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&g_FieldDrawEnvBg[1], 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(&D_80114154[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_80114154[1], 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv(&D_8011420C[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_8011420C[1], 0, 0xF0, 0x140, 0xE0);
    g_FieldDrawEnvBg[0].dtd = 1;
    g_FieldDrawEnvBg[1].dtd = 1;
    D_80114154[0].dtd = 1;
    D_80114154[1].dtd = 1;
    D_8011420C[0].dtd = 1;
    D_8011420C[1].dtd = 1;
    g_FieldDrawEnvBg[0].isbg = 0;
    g_FieldDrawEnvBg[1].isbg = 0;
    D_80114154[0].isbg = 0;
    D_80114154[1].isbg = 0;
    D_8011420C[0].isbg = 0;
    D_8011420C[1].isbg = 0;
    ClearOTagR(&g_FieldRenderData[0].OtSceneDrenv, 1);
    ClearOTagR(&g_FieldRenderData[1].OtSceneDrenv, 1);
    SetDrawEnv(&g_FieldRenderData[0].SceneDrenv, &g_FieldDrawEnvBg[0]);
    SetDrawEnv(&g_FieldRenderData[1].SceneDrenv, &g_FieldDrawEnvBg[1]);
    addPrim(
        &g_FieldRenderData[0].OtSceneDrenv, &g_FieldRenderData[0].SceneDrenv);
    addPrim(
        &g_FieldRenderData[1].OtSceneDrenv, &g_FieldRenderData[1].SceneDrenv);
    SetDefDrawEnv(&D_80113FE4[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_80113FE4[1], 0, 0xF0, 0x140, 0xE0);
    D_80113FE4[0].isbg = 0;
    D_80113FE4[1].isbg = 0;
    D_80113FE4[0].dtd = 1;
    D_80113FE4[1].dtd = 1;
    SetDefDrawEnv(&D_8011409C[0], 0, 8, 0x140, 0xE0);
    SetDefDrawEnv(&D_8011409C[1], 0, 0xF0, 0x140, 0xE0);
    D_8011409C[0].isbg = 0;
    D_8011409C[1].isbg = 0;
    D_8011409C[0].dtd = 1;
    D_8011409C[1].dtd = 1;
    func_800128B8();
    *(volatile u16*)&g_FieldStateData.fadeType = 0;
    if (D_800965EC != 1 && D_800965EC != 2 && D_800965EC != 3 &&
        D_800965EC != 5 && D_800965EC != 0xD) {
        ClearImage(&clip, 0, 0, 0);
    }

    for (;;) {
        DebugRunEveryLoop();
        D_80071A5C = 0;
        g_FieldPreloadMapId = 0;
        if ((D_800965EC == 1 || D_800965EC == 3) &&
            g_FieldStateData.fadeType == 0) {
            func_800129D0();
            g_FieldStateData.fadeType = 3;
            D_80071A58 = 3;
            g_FieldStateData.fadeAdjust = 0;
            D_8007E768 = 0;
            D_80095DD4 = 1;
        }
        if (D_800965EC != 5 && D_800965EC != 0xD) {
            D_8007EB64 = (s32*)0x80114FE4;
            D_8009A044 = (FieldWalkmesh**)0x80114FE8;
            D_8009D848 = (FieldBgData**)0x80114FEC;
            D_80083578 = (MATRIX**)0x80114FF0;
            g_FieldTriggersP = (s32*)0x80114FF4;
            g_FieldEncountersP = (s32*)0x80114FF8;
            g_FieldModelsP = (s32*)0x80114FFC;
            FieldLoadMimDatFiles();
        }
        if (D_800965EC == 2) {
            D_8007EBE0 = 1;
            if (D_8007EBC8 == 1) {
                D_8007EBC8 = 0;
                D_8009C6D8 = 0;
                D_8007173C = 0;
                *((volatile u8*)&g_FieldStateData.fadeType - 0x4B) = 0;
            }
        }
        while (D_80095DD4 != 0) {
        }
        while (DrawSync(1) != 0) {
        }
        if (D_800965EC != 0xD) {
            *(volatile u16*)&g_FieldStateData.fadeType = 1;
            *(volatile s16*)&g_FieldStateData.fadeSpeed = 0x10;
            *(volatile s16*)&g_FieldStateData.fadeAdjust = 0x100;
            *(volatile s16*)&g_FieldStateData.fadeRed = 0;
            *(volatile s16*)&g_FieldStateData.fadeGreen = 0;
            *(volatile s16*)&g_FieldStateData.fadeBlue = 0;
        }
        if (D_800965EC == 0 || D_800965EC == 1 || D_800965EC == 3 ||
            D_800965EC == 6 || D_800965EC == 8 || D_800965EC == 7 ||
            D_800965EC == 9 || D_800965EC == 0xB || D_800965EC == 0xA) {
            g_FieldStateData.layer2_bgScrollXSpeed = 0;
            g_FieldStateData.layer2_bgScrollYSpeed = 0;
            g_FieldStateData.layer3_bgScrollXSpeed = 0;
            g_FieldStateData.layer3_bgScrollYSpeed = 0;
            g_FieldStateData.layer3_depth = 1;
            g_FieldStateData.layer2_depth = 0xFFF;
            D_8009A100 = 0;
            D_80071E38 = 0;
            D_80071E3C = 0;
            g_FieldBGCameraHeightBias =
                ((FieldTriggerHeader*)g_FieldTriggers)->camHeightBias;
            FieldEventInit(&g_FieldStateData, g_FieldEntity, *D_8007EB64);
            g_FieldEntity[D_8009AC1E].Dir = D_8009AC18;
            fillVal = -1;
            if ((g_RainControl & 0x80) == 0) {
                g_RainForce = 0;
            } else {
                g_RainForce = 0xFF;
            }
            i = 0xF;
            fill = &D_8009A057;
            do {
                *fill-- = fillVal;
            } while (--i >= 0);
            FieldEntityBgTriggerInit((void*)(g_FieldTriggers + 0x158));
        } else {
            D_8009AC1A[0] = 2;
        }
        FieldEnablePartyModels();
        FieldEntityLineClear(&D_8007E7AC);
        D_800716D0 = 0;
        FieldArrowsInit(
            g_FieldRenderData[0].Arrows, &g_FieldRenderData[0].ArrowsDm);
        FieldArrowsInit(
            g_FieldRenderData[1].Arrows, &g_FieldRenderData[1].ArrowsDm);
        if (D_800965EC != 5 && D_800965EC != 0xD) {
            FieldLoadMimToVram(0, (u8*)0x80128000);
        }
        if (D_800965EC == 2) {
            D_8009A000[0] = 0xF5;
            SystemAkaoExecute();
            D_8009A000[0] = 0x18;
            D_8009A008[0] = 4;
            D_8009A004[0] = D_8009AC3C[0];
            SystemAkaoExecute();
        }
        FieldMainLoop();
        while (DrawSync(1) != 0) {
        }
        VSync(1);
        g_FieldDispEnv[0].isrgb24 = 0;
        g_FieldDispEnv[1].isrgb24 = 0;
        PutDispEnv(&g_FieldDispEnv[(s16)D_80075DEC]);
        PutDrawEnv(&g_FieldDrawEnv[(s16)D_80075DEC]);
        D_800965EC = 1;
        if (*((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 0xA ||
            *((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 0x1A ||
            *((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 5) {
            break;
        }
        if (*((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 1) {
            preloadId = D_80071A5C;
            *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType - 0x4B) +
                             0x63) = (u16)g_CurrentFieldIndex;
            fieldId =
                *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType -
                                  0x4B) +
                                 1);
            g_CurrentFieldIndex = fieldId;
            if (fieldId != preloadId) {
                StopFieldMapPreload();
            }
            if ((u32)((u16)g_CurrentFieldIndex - 1) < 0x40) {
                g_FieldNextModule = 3;
                func_800129D0();
                *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType -
                                  0x4B) +
                                 0x4B) = 3;
                D_80071A58 = 3;
                *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType -
                                  0x4B) +
                                 0x4D) = 0;
                D_8007E768 = 0;
                D_80095DD4 = 1;
                break;
            }
        }
        if (*((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 0xC) {
            *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType - 0x4B) +
                             0x63) = (u16)g_CurrentFieldIndex;
            exitId =
                *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType -
                                  0x4B) +
                                 1);
            exitKind = ((volatile u8*)&g_FieldStateData.fadeType - 0x4B)[0xF1];
            g_CurrentFieldIndex = exitId;
            switch (exitKind) {
            case 0:
                g_FieldNextModule = 6;
                break;
            case 1:
                g_FieldNextModule = 7;
                break;
            case 2:
                g_FieldNextModule = 8;
                break;
            case 3:
                g_FieldNextModule = 9;
                break;
            case 4:
                g_FieldNextModule = 0xA;
                break;
            case 5:
                g_FieldNextModule = 0xB;
                break;
            case 6:
                g_FieldNextModule = 0xE;
                break;
            }
            break;
        }
        if (*((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 2 ||
            *((volatile u8*)&g_FieldStateData.fadeType - 0x4B) == 0xD) {
            break;
        }
        if (g_FieldNextModule == 5) {
            func_800129D0();
            *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType - 0x4B) +
                             0x4B) = 0xD;
            D_80071A58 = 0xD;
            *(volatile u16*)(((volatile u8*)&g_FieldStateData.fadeType - 0x4B) +
                             0x4D) = 0;
            D_8007E768 = 0;
            D_80095DD4 = 1;
            break;
        }
        if (g_FieldNextModule == 0xD) {
            break;
        }
        if (g_FieldNextModule == 0x10) {
            break;
        }
    }
    VSync(0);
}

#endif

/* The field module's per-frame loop: flip the double buffer, clear both OTs,
 * run the event script and the entity/background updates, then hand the frame
 * to the GPU. Returns only when the event script asks for a different module.
 *
 * This function was one instruction out for a long time, and the instruction
 * was not codegen -- maspsx dropped the load-delay nop the original assembler
 * put at the loop-top join label:
 *
 *     lhu  v0,%lo(D_80075DEC)(v0)    <- dead re-read, from `D_80075DEC++`
 *   .Ljoin:
 *     nop
 *     lui  v0,%hi(D_80075DEC)        <- the next `lhu v0,D_80075DEC` macro
 *
 * A load with a symbolic operand and no base register is expanded by the
 * assembler through its own destination register, not $at, so its `lui` lands
 * in the delay slot of a preceding load into that same register. maspsx only
 * asked whether the next instruction *reads* the loaded register, and nothing
 * in the text of `lhu $2,D_80075DEC` does. Fixed in tools/maspsx
 * (_next_load_clobbers_reg, with tests in tests/test_symbol_load_clobber.py);
 * that change adds exactly one instruction to the whole build, this one.
 *
 * Everything else in here was derived and is worth keeping:
 *   - `s32` return, not `void`. v0 is then live at the epilogue, so gcc's
 *     delay-slot pass cannot steal the following `li v0,0xc` into the delay
 *     slot of the `eventCmd == 1` branch (the only branch here that jumps
 *     straight to the epilogue). Declared void, that slot gets filled and the
 *     function is one instruction short.
 *   - `D_80075DEC++`, not `D_80075DEC = D_80075DEC + 1`. The variable is
 *     volatile, and only the increment form re-reads it afterwards - three
 *     instructions the plain assignment does not emit.
 *   - the six RECTs are locals with aggregate initialisers, which is what
 *     emits their blobs into .rodata at 0x800A0024..0x800A0054 and copies them
 *     in with lwl/lwr. Do not also declare them at file scope: while this
 *     function was parked the .s referenced six `const u32 D_800A0024[]`
 *     definitions, and leaving those in beside the compiled C emits every
 *     blob twice and grows the overlay by 48 bytes.
 *   - g_FieldMovieDrawBg must NOT be volatile: volatile pins its load ahead of
 * the `li v0,1`, and the target has the constant first (in the branch delay
 *     slot of the movie-stream test).
 *   - the C uses g_FieldStateData.eventCmd throughout, never the D_8009ABF5
 * alias; the alias costs a %hi/%lo pair per use where gcc wants 1(s2), and it
 * is what lets s4 become the `addiu s4,s2,1` base the target uses for
 *     pcPosX/pcPosY/pcWalkMeshId.
 *   - `/ 4096`, not `>> 12`: the target has the bgez/addiu 0xfff rounding. */

extern FieldWalkmesh** D_8009A044;
extern s16* D_800E4274;
extern s16* D_80114458;
extern s32 g_FieldMovieDrawBg;
extern volatile s32 g_FieldMovieVSyncMode;
extern u8 g_FieldLineCheckResult;
extern s16 D_80071E38;
extern s16 D_80071E3C;
extern MATRIX* D_80071E40;
extern u32 g_FieldOTHead[2];
extern FieldLine D_8007E7AC;
extern s32 g_FieldVSyncBeforeDraw;
extern s32 g_FieldVSyncAfterDraw;
extern volatile s16 g_FieldNextModule;
extern s32 g_FieldScreenCenterX;
extern s32 g_FieldScreenCenterY;
extern DISPENV g_FieldDispEnv[2];
extern DRAWENV g_FieldDrawEnv[2];
extern DRAWENV g_FieldDrawEnvBg[2];
extern DISPENV* g_FieldCurDispEnv;
extern DRAWENV* g_FieldCurDrawEnv;

s32 FieldMainLoop(void) {
    RECT clip24Top = {0, 0, 480, 8};
    RECT clip24Mid = {0, 232, 480, 8};
    RECT clip24Bot = {0, 464, 480, 8};
    RECT clip16Top = {0, 0, 320, 8};
    RECT clip16Mid = {0, 232, 320, 8};
    RECT clip16Bot = {0, 464, 320, 8};
    struct FieldRenderData* buf;
    s16* tris;
    s16 first;

    g_FieldScreenCenterX = 160;
    g_FieldScreenCenterY = 120;
    if (D_800965EC != 5 && D_800965EC != 0xD) {
        FieldModelLoadAndInit();
    }
    tris = (*D_8009A044)->tris;
    D_800E4274 = tris;
    D_80114458 = (s16*)((*D_8009A044)->triCount * 24 + (s32)tris);
    if (D_800965EC != 5 && D_800965EC != 2 && D_800965EC != 0xD) {
        FieldEntityInitPos();
    }
    FieldBackgroundInitPackets(
        g_FieldRenderData[0].Bg1, g_FieldRenderData[0].Bg2,
        (u8*)g_FieldRenderData[0].BgAnim, g_FieldRenderData[0].BgDm);
    first = 1;
    FieldBackgroundInitPackets(
        g_FieldRenderData[1].Bg1, g_FieldRenderData[1].Bg2,
        (u8*)g_FieldRenderData[1].BgAnim, g_FieldRenderData[1].BgDm);
    FieldRainInit(&g_FieldRenderData[0]);
    FieldRainInit(&g_FieldRenderData[1]);
    g_FieldMovieStreamActive = 0;
    g_FieldMovieStreamDone = 0;
    g_FieldMoviePlayed = 0;
    g_FieldLineCheckResult = 0;
    g_isFieldLoading = 0;

    for (;;) {
        if (first == 0) {
            D_80075DEC++;
        }
        D_80075DEC = D_80075DEC & 1;
        g_FieldStateData.renderBuffer = D_80075DEC;
        buf = &g_FieldRenderData[(s16)D_80075DEC];
        ClearOTagR(buf->ot, 0x1000);
        ClearOTagR(&buf->OtUi, 1);
        FieldCameraAssign();
        g_FieldPadRaw = FieldButtonsUpdate(&D_80071E38, &D_80071E3C);
        g_FieldStateData.currentMovieFrame = g_MovieStream->currentFrame;
        FieldEventUpdate((s32)&buf->OtUi);
        g_PlayerModelId = g_FieldStateData.pcModelId;
        FieldBGScrollInit();
        FieldBGScrollUpdate();
        FieldBGShakeUpdate(&g_FieldStateData.shakeX);
        FieldBGShakeUpdate(&g_FieldStateData.shakeY);
        FieldBGUpdateDrawenv(buf);
        PreloadNextFieldMap(&g_FieldEntity[g_PlayerModelId],
                            (FieldLine*)(g_FieldTriggers + 0x38));
        if ((g_FieldStateData.activeKeys & 0x90F) == 0x90F) {
            g_FieldStateData.eventCmd = 0xA;
            func_80035658();
            StopFieldMapPreload();
            return;
        }
        if (g_FieldStateData.eventCmd == 1) {
            return;
        }
        if (g_FieldStateData.eventCmd == 0xC) {
            StopFieldMapPreload();
            return;
        }
        if (g_FieldStateData.eventCmd == 0xD) {
            StopFieldMapPreload();
            g_FieldNextModule = 0xC;
            return;
        }
        if (g_FieldStateData.eventCmd == 0x19) {
            g_FieldNextModule = 0x10;
            StopFieldMapPreload();
            return;
        }
        if (g_FieldStateData.eventCmd == 0xF ||
            g_FieldStateData.eventCmd == 0x10 ||
            g_FieldStateData.eventCmd == 0x11 ||
            g_FieldStateData.eventCmd == 0x15 ||
            g_FieldStateData.eventCmd == 0x16 ||
            g_FieldStateData.eventCmd == 0x17 ||
            g_FieldStateData.eventCmd == 0x18) {
            g_FieldNextModule = 0xD;
            StopFieldMapPreload();
            return;
        }
        if (g_FieldStateData.eventCmd == 6 || g_FieldStateData.eventCmd == 7 ||
            g_FieldStateData.eventCmd == 9 ||
            g_FieldStateData.eventCmd == 0xE ||
            g_FieldStateData.eventCmd == 8 ||
            g_FieldStateData.eventCmd == 0x12 ||
            g_FieldStateData.eventCmd == 0x13) {
            g_FieldNextModule = 5;
            StopFieldMapPreload();
            return;
        }
        if ((g_FieldPadRaw & 0x10) && g_FieldStateData.menuDisabled == 0 &&
            g_FieldMoviePlayed == 0 && g_FieldMovieStreamActive == 0) {
            g_FieldNextModule = 5;
            g_FieldStateData.eventCmd = 9;
            g_FieldStateData.eventCmdParam = 0;
            StopFieldMapPreload();
            return;
        }
        if (g_FieldStateData.eventCmd == 5 ||
            g_FieldStateData.eventCmd == 0x1A) {
            StopFieldMapPreload();
            return;
        }
        if (g_FieldStateData.eventCmd == 2) {
            g_FieldStateData.pcPosX =
                g_FieldEntity[g_PlayerModelId].PosX / 4096;
            g_FieldStateData.pcPosY =
                g_FieldEntity[g_PlayerModelId].PosY / 4096;
            g_FieldNextModule = 2;
            g_FieldStateData.pcWalkMeshId = g_FieldEntity[g_PlayerModelId].PosI;
            StopFieldMapPreload();
            return;
        }
        FieldEntityMovementUpdate(g_FieldPadRaw);
        FieldEntityLineInteract(&g_FieldEntity[g_PlayerModelId], &D_8007E7AC);
        FieldEntityCheckTalk();
        if (g_FieldMovieStreamActive == 0 || g_FieldMovieDrawBg == 1) {
            AddBackgroundToRender(buf);
        }
        HandleKawaiDataInModel(buf);
        FieldRainUpdate();
        FieldRainAddToRender(buf->ot, buf->Rain, D_80071E40, &buf->RainDm);
        FieldArrowsAddToRender(buf, D_80071E40, g_FieldTriggers + 0x38);
        func_800138EC();
        g_FieldVSyncBeforeDraw = VSync(1);
        while (DrawSync(1) != 0) {
        }
        g_FieldVSyncAfterDraw = VSync(1);
        if (g_FieldMovieStreamActive != 0 && g_FieldMovieVSyncMode != 1) {
            VSync(3);
        } else {
            VSync(2);
        }
        if (first != 0) {
            first--;
            if (first == 0) {
                SetDispMask(1);
            }
        }
        ResetGraph(1);
        if (g_FieldMovieStreamActive == 0) {
            if (g_FieldMovieStreamDone == 0) {
                g_FieldDispEnv[(s16)D_80075DEC].isrgb24 = 0;
            } else {
                g_FieldMovieStreamDone = 0;
            }
        }
        PutDispEnv(&g_FieldDispEnv[(s16)D_80075DEC]);
        PutDrawEnv(&g_FieldDrawEnv[(s16)D_80075DEC]);
        if (g_FieldMovieStreamActive == 0) {
            ClearImage(&g_FieldDrawEnv[(s16)D_80075DEC].clip, 0, 0, 0);
        } else if (g_FieldDispEnv[(s16)D_80075DEC].isrgb24 == 0) {
            ClearImage(&clip16Top, 0, 0, 0);
            ClearImage(&clip16Mid, 0, 0, 0);
            ClearImage(&clip16Bot, 0, 0, 0);
        } else {
            ClearImage(&clip24Top, 0, 0, 0);
            ClearImage(&clip24Mid, 0, 0, 0);
            ClearImage(&clip24Bot, 0, 0, 0);
        }
        g_FieldCurDispEnv = &g_FieldDispEnv[(s16)D_80075DEC];
        g_FieldCurDrawEnv = &g_FieldDrawEnvBg[(s16)D_80075DEC];
        FieldUpdateMovieStream();
        if (g_FieldStateData.mpdspSet == 0) {
            DrawOTag(&buf->OtSceneDrenv);
            DrawOTag(&buf->ot[0xFFF]);
            DrawOTag(&buf->OtFadeDrenv);
            if (D_8009AC40[0] != 0) {
                DrawOTag(&g_FieldOTHead[(s16)D_80075DEC]);
            }
        }
        DrawOTag(&buf->OtUi);
    }
}

/* Parse a MIM (field background map image) header and upload its palette and
 * two texture pages to VRAM. `mim` points at the loaded file; three
 * variable-length records follow one another, each opening with a 32-bit byte
 * length, and each seeds a slice of the state block at g_FieldMimPalData. The
 * palette goes up with LoadImage, the two pages with LoadTPage, with a DrawSync
 * between every step.
 *
 * The body was rewritten from the target; the previous one did not compile
 * (LoadTPage takes seven arguments, not five) and so had never been measured.
 * Five things it gets right:
 *
 *   - `mim` itself advances. The target reaches the second record's fields as
 *     0(a1) and 2(a1) with `addiu a1,a1,4` between the pairs, which is a
 *     source-level walk; constant displacements off one base would come out as
 *     a single `addiu a1,a1,0xC`.
 *   - the second LoadTPage is guarded by `if (*(u32*)&g_FieldMimTex1Size[0] !=
 * 0)` -- the `lw v1,g_FieldMimTex1Size` / `beqz v1` right after the first
 * LoadTPage.
 *   - every value read back for the LoadTPage argument lists is spelled as an
 *     offset from g_FieldMimPalData rather than through its own symbol. Naming
 * the symbol twice -- once for the store, once for the read -- makes gcc
 *     materialise its %hi/%lo into a register, and with nine such symbols the
 *     function grew nine callee-saved registers and a 0x50 frame. Reached as
 *     `(u8*)g_FieldMimPalData + 0x1C` the address is named once and the
 * assembler rebuilds it at the use, which is what the target does. Worth 38
 * rows, and the same mechanism as the byte-offset idiom in CLAUDE.md.
 *   - `unusedLocals` reserves the 0x28 of stack the original allocates after
 *     `rect` and never touches; see FieldDebugInitBuffers for the same thing.
 *   - each block's length is stored to the state word first and then read back
 *     out of it for the `(len >> 2) * 4 - 0xC` skip. Held in a local instead,
 *     the pseudo is still live at the shift, so the chain needs two more
 *     registers and sched2 cannot spread it into the load-delay slots of the
 *     three `sh`s in front of it -- 15 rows of nops, and no declaration order,
 *     variable count or spelling of the arithmetic moves them. Read back
 *     through the global, cse hands the just-stored register straight back,
 *     the shift is done in place, and the slots fill. Re-reading the *source*
 *     word instead of the destination is not the same thing and costs 18.
 *   - the LoadImage source is `(u8*)g_FieldMimPalSize - 4`, not
 * `g_FieldMimPalData` by name, even though the two are the same address and the
 * store just above uses the name. Naming it twice gives cse a second reference,
 * it promotes the address to a callee-saved register and the frame grows -- 7
 * rows. The
 *     `.s` names g_FieldMimPalData there, so the relocation reads
 * `g_FieldMimPalSize-0x4` against the target's `g_FieldMimPalData`; the linked
 * bytes are identical and `tools/checkfn.py` resolves the negative addend. */
void FieldLoadMimToVram(s32 arg0, u8* mim) {
    RECT rect;
    u8 unusedLocals[0x28];
    u32 next;
    u16 unk0A;

    *(u32*)&g_FieldMimPalSize[0] = *(u32*)mim;
    next = (*(u32*)&g_FieldMimPalSize[0] >> 2) * 4 - 0xC;
    *(u16*)&g_FieldMimPalX[0] = *(u16*)(mim + 4);
    *(u16*)&g_FieldMimPalY[0] = *(u16*)(mim + 6);
    *(u16*)&g_FieldMimPalW[0] = *(u16*)(mim + 8);
    unk0A = *(u16*)(mim + 0xA);
    mim += 0xC;
    *(u8**)&g_FieldMimPalData[0] = mim;
    *(u16*)&g_FieldMimPalH[0] = unk0A;
    mim += next;

    /* First texture page block. */
    *(u32*)&g_FieldMimTex0Size[0] = *(u32*)mim;
    next = (*(u32*)&g_FieldMimTex0Size[0] >> 2) * 4 - 0xC;
    mim += 4;
    *(u16*)&g_FieldMimTex0X[0] = *(u16*)mim;
    *(u16*)&g_FieldMimTex0Y[0] = *(u16*)(mim + 2);
    mim += 4;
    *(u16*)&g_FieldMimTex0Rect[0] = *(u16*)mim * 2;
    *(u16*)((u8*)g_FieldMimTex0Rect + 2) = *(u16*)(mim + 2);
    mim += 4;
    *(u8**)&g_FieldMimTex0Data[0] = mim;
    mim += next;

    /* Second texture page block. */
    *(u32*)&g_FieldMimTex1Size[0] = *(u32*)mim;
    mim += 4;
    *(u16*)&g_FieldMimTex1X[0] = *(u16*)mim;
    *(u16*)&g_FieldMimTex1Y[0] = *(u16*)(mim + 2);
    mim += 4;
    *(u16*)&g_FieldMimTex1Rect[0] = *(u16*)mim * 2;
    *(u16*)((u8*)g_FieldMimTex1Rect + 2) = *(u16*)(mim + 2);
    mim += 4;
    *(u8**)&g_FieldMimTex1Data[0] = mim;

    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = 0x100;
    rect.h = 0x10;
    DrawSync(0);
    LoadImage(&rect, *(u_long**)((u8*)g_FieldMimPalSize - 4));
    DrawSync(0);
    *(u16*)&g_FieldMimTex0Tpage[0] = LoadTPage(
        *(u_long**)((u8*)g_FieldMimPalData + 0x14), 1, 0,
        *(s16*)((u8*)g_FieldMimPalData + 0x1C),
        *(s16*)((u8*)g_FieldMimPalData + 0x1E),
        *(u16*)((u8*)g_FieldMimPalData + 0x20),
        *(u16*)((u8*)g_FieldMimPalData + 0x22));
    if (*(u32*)((u8*)g_FieldMimPalData + 0x48) != 0) {
        DrawSync(0);
        *(u16*)&g_FieldMimTex1Tpage[0] = LoadTPage(
            *(u_long**)((u8*)g_FieldMimPalData + 0x44), 1, 0,
            *(s16*)((u8*)g_FieldMimPalData + 0x4C),
            *(s16*)((u8*)g_FieldMimPalData + 0x4E),
            *(u16*)((u8*)g_FieldMimPalData + 0x50),
            *(u16*)((u8*)g_FieldMimPalData + 0x52));
    }
    DrawSync(0);
}

/* Latch both pads: keep the raw state, the previous state, and the edges
 * (newly pressed / newly released) derived from the two, and hand the caller
 * back pad 2's raw state.
 *
 * The return value is not decoration -- it is what makes the function match.
 * Declared void, gcc puts `~pad2` in v0 (the dying source of the `nor`) and
 * the last two instructions come out as `nor v0,zero,v0` / `and v1,v1,v0`.
 * The target writes `nor a0,zero,v0`, reusing a0 freed by the
 * g_FieldPad2Pressed store one instruction earlier, which it can only do
 * because v0 must still hold pad2 at the epilogue. The identical pad-1 block,
 * whose pad1 really is dead, does use v0 -- so the two halves being allocated
 * differently is the tell. FieldMainLoop's call site agrees: it stores the
 * result into g_FieldPadRaw. */
u32 FieldButtonsUpdate(void) {
    u32 pad1;
    u32 pad2;
    u32 old1;
    u32 old2;

    pad1 = func_8001C808();
    old1 = g_FieldPad1State;
    g_FieldPadRaw = pad1;
    g_FieldPad1State = pad1;
    g_FieldPad1PrevState = old1;
    g_FieldPad1Pressed = (pad1 ^ old1) & pad1;
    g_FieldPad1Released = (pad1 ^ old1) & ~pad1;

    pad2 = func_8001C8D4();
    old2 = g_FieldPad2State;
    g_FieldPadRaw = pad2;
    g_FieldPad2State = pad2;
    g_FieldPad2PrevState = old2;
    g_FieldPad2Pressed = (pad2 ^ old2) & pad2;
    g_FieldPad2Released = (pad2 ^ old2) & ~pad2;
    return pad2;
}

extern FieldBgData** D_8009D848;
extern FieldBgTile3* D_8007EBD4;
extern s16 D_8011448C;
extern s16 D_801144C8;
extern s16 D_801144D0;

/* Build the sprite packets for the field background's four layers. Layers 1
 * and 2 are 16x16 sprites, layers 3 and 4 are 32x32; each layer walks the run
 * list from where the previous one stopped, emitting one packet per tile and a
 * DR_MODE whenever a run asks for a different texture page. `pairs` collects
 * the two per-sprite parameter bytes the animation code later edits in place.
 *
 * **40 changed / 10 inserted, at 397 instructions against the target's 395,
 * and that is deliberate.** The body here is two `nop`s long and scores two
 * rows worse than the 45-row body it replaces, and it is still the better
 * one, because the *alignment-free* comparison collapsed from seven mismatch
 * categories to one:
 *
 *     old body (45 rows, 395):  sw +3  sh -3  lw +3  lh -2  lhu -1
 *                               move -2  nop +2
 *     this body (50 rows, 397): nop +2
 *
 * Frame size, every stack slot offset, every access width, every `%hi`
 * materialisation and every opcode count now agree with the target. What is
 * left is one scheduling artefact, at two identical sites -- see lever 14.
 * Read `tools/insn_histogram.py`, not the row count, when judging a change to
 * this function: the row count is dominated by whichever cluster happens to
 * be misaligned and it pointed the wrong way for three sessions.
 *
 * **Do not unpark this at 397.** The two extra instructions would shift every
 * symbol after it in the overlay. The body is here as documentation and as
 * permuter input; it costs the build nothing while it is pinned.
 *
 * Levers 1-7 below are unchanged and still hold. Lever 8 is **withdrawn** --
 * it was a pair of frame hacks that existed only to buy a frame size that
 * levers 12-13 now buy for free, and while they were in place they forced the
 * three memory-resident values into the wrong stack slots, which is what made
 * the note's "not reachable" verdict on those slots look true. Levers 12-14
 * are new.
 *
 * 1. The four run walks are `for (;;)` loops left by `goto`, not backward
 *    gotos. This note previously concluded the opposite, and it had the
 *    measurements to prove it -- but only for two of the three loop spellings:
 *    `while` (191/43) and `for (;;) { if (...) break; }` (177/36) both get the
 *    0x7FFF test copied to the bottom by duplicate_loop_exit_test. The third
 *    shape, `for (;;) { if (...) goto <next layer>; ... }`, is the one
 *    CLAUDE.md already records from AddBackgroundToRender: still a loop, so
 *    the body's references are loop-depth weighted and invariants hoist, but
 *    the exit is not the loop's own end test, so nothing is duplicated and the
 *    single top test is reached by a plain `j`. Worth 48 rows here, and it is
 *    what hands $s8 to `modes`: `REG_N_REFS += loop_depth` in flow, so putting
 *    all four walks at depth 1 lifts modes' weighted reference count over the
 *    floor_log2 step at 16, while the two sprite counters -- whose references
 *    are already inside the inner do-whiles -- gain nothing they did not have.
 *    The counters then spill to 0x20(sp)/0x28(sp) and `modes` gets the
 *    register, which is the whole of the old note's "this is the whole
 *    function". Layers 1+2 alone as loops measures 181/23 and layers 3+4 alone
 *    171/24 -- it only pays when all four are loops together.
 *
 * 2. The SetSemiTrans if/else is written the other way round: the target's
 *    branch is `bnez` to the `li a1,1` block, so the *then* arm is the zero
 *    one and the condition is `(flags & 0x80) == 0`. All three sites. 5 rows.
 *
 * 3. Layers 3 and 4 write `clut` *after* `w` and `h`. combine_givs bases the
 *    body's address giv on the last sprt offset referenced in insn order, so
 *    clut-then-w-then-h bases it at sprt+0x12 (`h`) and every store in the
 *    body carries a displacement 4 bytes off the target's sprt+0xe. 22 rows
 *    for moving one line, and it reads as pure scheduling noise in the diff
 *    because the emitted *order* is w, h, clut either way -- sched2 sinks the
 *    clut store regardless. Layers 1 and 2 already end with clut and already
 *    matched at 0xe.
 *
 * 4. Layer 2 writes r0, g0, b0 in that order, and layer 2's `modes++` sits
 *    immediately after its SetDrawMode rather than after the SetSemiTrans
 *    if/else. 12 rows between them.
 *
 * 5. Both sprite counters are incremented late in their inner loop body --
 *    after the record pointer and the packet pointer are bumped, but *before*
 *    `pairs += 2`. 8 rows. The dimension was swept twice: to the end of the
 *    body first (7 rows, and only worth it for both counters together -- one
 *    at a time measures 83/19 and 84/15 against 80/17), and then finely once
 *    everything around it had settled. Against the seven-lever base, moving
 *    each counter up one bump from the end is 55/14, up two bumps 62/12, up
 *    two in layers 1+2 only 58/14, up two in layers 3+4 only 59/13, and at
 *    the very front of the body much worse. Positions inside the field-store
 *    run (after u0, v0 or clut) all measure 55/17.
 *
 * 6. `tpages++` is a statement of its own, not part of the SetDrawMode
 *    argument: `SetDrawMode(modes, 0, 1, tpages[0], NULL); tpages++;` at all
 *    four sites. 13 rows. Written as `*tpages++` the load and the spilled
 *    pointer's read-modify-write are one dependency chain that sched2 keeps
 *    together, and the `addiu s8,s8,0xc` the target puts in the load-delay
 *    slot has nowhere to go. Note this only pays with `modes` written out
 *    separately -- `SetDrawMode(modes++, 0, 1, tpages[0], NULL)` measures 71
 *    against 67, and with `*tpages++` still in the argument it is 71 either
 *    way.
 *
 * 7. `data` is three variables, one per layer, not one reused three times.
 *    The layer-4 walk never reads it at all. This is the opposite of the
 *    counter-merging rule and the same side as the one-pointer-pair-per-loop
 *    rule: merge counters that describe the same walk, split pointers that
 *    describe different ones. 12 rows, and it is the change that made the
 *    whole prologue line up -- everything down to the first spill store now
 *    matches instruction for instruction. Splitting `count` the same way is
 *    worse in both arrangements (80 against 67 with `data` merged, 69 against
 *    55 with it split), which is the counter-merging rule holding.
 *
 * `white` is not a tidiness variable. The target materialises 0x80 into a
 * callee-saved register once at the layer-3 entry and again at the layer-4
 * entry (`ori $s1,$zero,0x80` at both, the second one alone in its own block),
 * and reads it for all six r0/g0/b0 stores in those two walks; written as
 * literals gcc rematerialises it inside the loops. That is why layer 4 needs
 * the extra `layer4run:` label -- the walk's back edge targets the run test,
 * not the assignment, so the constant is set once on entry and not per
 * iteration. Layer 1 must keep its literals: hoisting a `white` there as well
 * costs 51 rows.
 *
 * 8. **Withdrawn.** `u8 unusedLocals[0x10];` plus a never-dereferenced
 *    `&sprite34Count`. The pair bought the target's 0x78 frame -- declared
 *    locals are allocated during expand and reload's spills after them, so a
 *    declared local always sits below the spills, and taking a scalar's
 *    address moves it into the declared pool -- and the note that stood here
 *    concluded, from a full eleven-cell grid, that "no combination reaches
 *    the target's offsets at the right length", and that the 0x10 the target
 *    reserves above its three slots "is not expressible in C".
 *
 *    Both claims were true of the grid and false of the function. The grid
 *    swept the two frame hacks against each other while holding the *guard*
 *    (lever 13) and `spriteCount`'s *width* (lever 12) at values that were
 *    themselves wrong, and the guard turns out to add exactly the missing
 *    0x10 of frame by itself. With levers 12-13 in, deleting both hacks gives
 *    frame 0x78 with the three memory-resident values at 0x18 / 0x20 / 0x28
 *    -- the target's offsets, in the target's order -- and the fourteen rows
 *    of "not reachable" stack offset go away. This is the second time a fully
 *    swept dimension on this function turned out to have been swept with
 *    something else held wrong.
 *
 *    The grid itself, re-measured on the old base before the withdrawal, so
 *    that the history is not lost: with the pad and the address-take,
 *    `s32 spriteCount` 45, `s16` 47, `u16` 47; with the address-take alone,
 *    68 / 70 / 70; with the pad alone, 65 / 67 / 67, all at 397; with
 *    neither, 78 / 78 / 78, all at 397. Every cell of it is a local minimum
 *    of the wrong function.
 *
 *    Two facts from the old grid do survive, and they bound where a future
 *    frame hack can go. Declared-pool slots are packed at the type's own
 *    alignment while reload's spill slots are rounded to 8: address-taking
 *    `tpages`, `spriteCount` and `sprite34Count` in that order puts them at
 *    0x18 / 0x1C / 0x1E, not 0x18 / 0x20 / 0x28, so the target's 8-byte
 *    spacing is itself proof that all three are reload spills. And an
 *    aggregate declared in an inner block *is* allocated where the block is
 *    reached, after any scalar whose address was taken earlier -- but since
 *    reload's spills come after every declared local, there is still no way
 *    to put dead frame *above* a spill slot. Padding the declared pool to
 *    force 0x18 / 0x20 / 0x28 by hand (two `s32` fillers between the three
 *    address-taken values, a trailing pad in an inner block, five pad sizes)
 *    reproduces the offsets exactly and measures 144 rows at every pad size,
 *    because an address-taken pointer is a MEM at every use and gcc reloads
 *    it more often than the target does. Address-taking `tpages` alone --
 *    the one combination that gets all three offsets right with a single
 *    hack, since a 4-byte declared slot at 0x18 leaves the two spills at
 *    0x20 and 0x28 -- is 102, and adding `&sprite34Count` after it 121,
 *    before it 122.
 *
 * 12. `spriteCount` is `u16`, not `s32`. It is a spilled pseudo, and reload
 *    spills in the pseudo's own mode, so the width of the *declaration*
 *    decides whether the counter's three stores and three loads come out
 *    `sh`/`lhu` or `sw`/`lw`. That is the whole of the old body's
 *    `sw +3 / sh -3 / lw +3` cluster, which read as an allocation problem
 *    and was a declaration. `s16` is byte-identical here; `u16` is what the
 *    target's `lhu` at the layer-2 and layer-3 entries says it is.
 *    `tools/width_sweep.py` had swept this dimension and reported `s32` as
 *    the winner by two rows -- true, and true only while lever 8's pad was
 *    holding the counter in a slot at the wrong offset.
 *
 * 13. Layers 3 and 4 store `run[1]` *before* reading `run[2]`:
 *
 *        run[1] = sprite34Count;
 *        count = run[2];
 *        if (count != 0) {
 *
 *    Two things come out of the swap. The target's guard is
 *    `lh v0,4(s4) / lhu t0,0x28(sp) / move s3,v0 / beqz v0 / sh t0,2(s4)` --
 *    an `lh` into a scratch register, a copy into the counter, and the test
 *    on the scratch -- where the other order narrows the load to `lhu`
 *    straight into the counter and pays a `nop`. That is CLAUDE.md's "a value
 *    the target copies with `move` out of a register it just loaded" idiom,
 *    worth `lh +2 / move +2 / nop -2` across the two sites. The second thing
 *    is what nobody was looking for: the swap also **adds 0x10 to the
 *    frame**, which is exactly what lever 8's pad was there to supply.
 *    Testing `run[2]` rather than `count` in the guard is inert -- the two
 *    spellings give byte-identical objects -- so write the readable one.
 *
 *    The old note recorded this dimension as closed, with
 *    `run[1] = sprite34Count; count = run[2]; if (count != 0)` measured at
 *    65/4 against 43/4. That number was taken with lever 12 wrong. On the
 *    old base the six guard spellings measure: re-reading `run[2]` in the
 *    test with the old statement order 79 (at 401), the swap with the
 *    re-read 67, the swap with the plain test 67, `count` assigned inside
 *    the arm 82, `run[1]` moved inside the arm 80 and 80, against 45 for the
 *    old spelling -- so on that base every one of them looks like a
 *    regression, and on the corrected base two of them are the answer.
 *    Re-run a "closed" dimension after any change to a declaration.
 *
 * 14b. **Lever 14 re-diagnosed: it is not a sched2 priority tie, it is an
 *    anti-dependence on a register `local_alloc` reused.** Read the two
 *    schedules side by side rather than the row count. The target spreads the
 *    `--count` chain through three load-delay slots -- `addiu a0,s3,-1` in
 *    `lbu v0,5(v0)`'s, `move s3,a0` in `lhu v0,6(v1)`'s, `sll a0,a0,0x10` in
 *    `lbu v0,8(v1)`'s -- and issues the spilled counter's `lhu`/`addiu`/`sh`
 *    afterwards. This build cannot do the first of those *at all*, because
 *    its decrement temp is `$v1` and `$v1` is the D_8007EBD4 pointer until
 *    `lbu v0,8(v1)`: `addiu v1,s3,-1` anti-depends on that read and can only
 *    be issued after it. The counter chain then fills the slots the decrement
 *    was going to have. So the schedule is a consequence and the register is
 *    the cause, and every sweep of *statement position* was sweeping the
 *    wrong dimension -- which is why all of them came back inert.
 *
 *    `tools/qty_pri.py` prints both quantities and they are a dead tie:
 *
 *      pri 12.000  refs  9  life  9  -> v1  (plus (subreg (reg/v:HI count)) -1)
 *      pri 12.000  refs 12  life 12  -> v1  (mem (symbol_ref "D_8007EBD4"))
 *
 *    -- and a tie is not the point, because the two do not *conflict*: the
 *    pointer dies at its last read and the temp is born after it, so
 *    `block_alloc` hands the temp the same lowest-free register whatever the
 *    order. For the target's `$a0` the temp has to conflict with both `$v0`
 *    and `$v1`, i.e. its live range has to **start before the last
 *    D_8007EBD4 pointer use**. That is the specification a candidate has to
 *    meet, and `--count` in a `do`/`while` test is emitted at the bottom of
 *    the body by construction, so no statement order reaches it.
 *
 *    Swept against that specification and all worse: keeping a pointer
 *    quantity alive to the loop bottom by moving `pairs[1]` after the three
 *    bumps (63 at **-10** instructions), both `pairs` stores after them (116
 *    at -6), `D_8007EBD4++` after `sprt++`/`pairs += 2` (exactly inert, 50),
 *    `sprite34Count++` last of all (48 at +4). Pinning the counter instead --
 *    a `volatile u16` local, which is the lever that closed FieldMain's fade
 *    block -- costs instructions at every spelling: `spriteCount` volatile
 *    with `x = x + 1` 94 at +4, with `x++` 104 at +6, `sprite34Count`
 *    volatile with `x++` 62 at +4, both 128 at +8. A volatile *local* is a
 *    declared stack slot reloaded at every use, so it is not the same tool as
 *    a volatile MEM on a global.
 *
 *    One trap found while sweeping this, and it is the exact-length-by-
 *    cancellation shape again: moving `sprt->clut` below `pairs[0]` scores
 *    **51 rows at the exact 395**, which reads as the length finally fitting.
 *    It is not -- layer 4 loses the clut store's `lhu v0,6(v1)`/`move`/`sh`
 *    outright (cse shares the pointer load with `pairs[0]`'s) and two `nop`s
 *    elsewhere pay for it. Check the rows before believing a length.
 *
 * 14. **What is left**: two `nop`s, one each in the layer-3 and layer-4 tile
 *    loops, in the load-delay slot of
 *
 *        lbu   $v0, 0x5($v0)     ; D_8007EBD4->v
 *        addiu $a0, $s3, -0x1    ; --count, which the target issues here
 *        sb    $v0, -0x1($s0)    ; sprt->v0
 *
 *    Everything else in those blocks is instruction-for-instruction right;
 *    the two schedules differ by a single swap. sched2 ranks by the longest
 *    dependence path to the end of the block, and the spilled
 *    `sprite34Count++` is a three-insn `lhu`/`addiu`/`sh` chain whose load
 *    latency outranks the four-insn `addiu`/`move`/`sll`/`bnez` decrement
 *    chain, so ours issues the counter first and the target issues the
 *    decrement first. Nothing in the source reaches it, which is the
 *    signature of a `perm_ins_block` / `perm_temp_for_expr` residue: the
 *    permuter is the tool for it, and with the histogram this clean the
 *    scratch's base score should be low enough for `--stop-on-zero` to be
 *    reachable.
 *
 *    Measured against it and rejected, all on this body:
 *      - every position of `sprite34Count++` among the layer-3/4 loop's six
 *        tail statements: before `pairs[0]` 50, current 50, after `pairs[1]`
 *        50, after `D_8007EBD4++` 50, after `sprt++` 48 *at 399*, after
 *        `pairs += 2` 48 *at 399*. The two 48s buy two rows with two more
 *        `nop`s each -- the same trap as the withdrawn lever 11.
 *      - every position of `spriteCount++` in the layer-1/2 loops: ahead of
 *        the tile bump 56, between the two bumps 53, current 50, after
 *        `pairs += 2` 49 -- all at 397, so none of them recovers a `nop`.
 *      - all five non-identity permutations of `D_8007EBD4++`, `sprt++` and
 *        `pairs += 2`: **exactly 50, every one.** sched2 reorders the three
 *        bumps regardless of source order, so that block is inert and is not
 *        worth another sweep.
 *      - `do { } while (0);` barriers in the layer-3/4 body: after
 *        `pairs[0]` 44 rows *at 403*, after `sprite34Count++` 46 *at 399*,
 *        after the clut store 48 *at 399*, after `sprt->v0` 54, after
 *        `pairs[1]` 58, after `sprt->h` 59. The 44 is the most expensive
 *        false positive this function has to offer -- eight `nop`s for six
 *        rows. Removing layer 4's existing barrier is 55, so that one stays.
 *      - store order in the layer-3/4 body: `v0` before `u0` 58, `w`/`h`
 *        before `u0`/`v0` 61, `w`/`h` between `v0` and `clut` 63, `u0`/`v0`
 *        first 82, `x0`/`y0` last 84. The order here is the target's.
 *
 * 9-11, all three withdrawn. A body built on the deleted-lever base reached
 *    26 changed / 11 inserted, against 43/4 here, and every one of those rows
 *    was bought with instructions:
 *
 *      9.  `run[1] = sprite34Count; count = run[2]; if (run[2] != 0)` in
 *          layers 3 and 4, re-reading the guard rather than testing `count`.
 *      10. the counter increments moved past both pointer bumps.
 *      11. a `do { } while (0);` at the end of the layer-3 and layer-4 inner
 *          loop bodies.
 *
 *    Each costs two instructions and the three together cost six, so the
 *    "better" body is 401 instructions against the target's 395. `checkfn.py`
 *    now prints that figure and it is what exposed this; before it did, the
 *    row count was the only number in view and it pointed the wrong way for a
 *    full session. Lever 11 is the clearest case: the barriers emit six
 *    `nop`s, and what they buy is the three-instruction `--count` chain
 *    landing in a block of its own where the diff happens to align it. Six
 *    nops for an alignment coincidence.
 *
 *    Re-measured against the restored base, every one of the three is a
 *    regression on its own as well: lever 9 alone 65/4, lever 10 alone 47/8,
 *    lever 9 and 10 together 69/9, all three 78/15 at 401. The guard
 *    dimension is now covered rather than sampled -- `count = run[2];
 *    run[1] = sprite34Count; if (count != 0)` is 43/4 and wins; the same two
 *    statements swapped is 65/4; `run[1] = sprite34Count; if (run[2] != 0) {
 *    count = run[2];` -- the CLAUDE.md idiom for producing the target's
 *    `move`, assigning inside the arm so cse rewrites the redundant load --
 *    is 76/8 at 397; a redundant reload inside the arm on top of the old
 *    guard 66/9 at 400; the `run[1]` store moved inside the guard 68/14 at
 *    403. So the *order* of the two statements before the test is
 *    load-bearing and already right, and the re-read is not what the target
 *    wants. The layer-1/2 counter position was re-swept on this base too:
 *    ahead of `tile1++`/`tile2++` 46/7, between the tile and packet bumps
 *    46/4, between the packet bump and `pairs += 2` 43/4 (the current
 *    spelling), after `pairs += 2` 43/5.
 *
 * **Everything from here down was written against the pre-lever-12 body and
 * is history, not evidence.** It described three residual clusters -- the
 * fourteen rows of stack offset "not reachable", the twenty-two rows of loop
 * guard, and the layer-1/2 counter read-modify-write. The first two are gone
 * (levers 12-13); the third went with them. Only lever 14 is left. The
 * numbers below are still useful as *relative* evidence within each
 * paragraph, and one of them -- the arm-inversion sweep -- is still live.
 *
 * Read off the target's own layer-3 body rather than off the diff, the tail is
 *
 *     lbu   $v0, 0x5($v1)      ; D_8007EBD4->v
 *     addiu $a0, $s3, -0x1     ; --count, in that load's delay slot
 *     sb    $v0, -0x1($s0)     ; sprt->v0
 *     ...
 *     ori   $v0, $zero, 0x20   ; one materialisation for both
 *     sh    $v0, 0x2($s0)      ; sprt->w
 *     sh    $v0, 0x4($s0)      ; sprt->h
 *     lhu   $v0, 0x6($v1)
 *     addu  $s3, $a0, $zero    ; count = a0
 *     sh    $v0, 0x0($s0)      ; sprt->clut
 *     lbu   $v0, 0x8($v1)
 *     sll   $a0, $a0, 16
 *     sb    $v0, 0x0($s5)      ; pairs[0]
 *     lhu   $t0, 0x28($sp)     ; sprite34Count, between the two pairs stores
 *     ...
 *     sb    $v0, 0x1($s5)      ; pairs[1]
 *
 * which confirms three things the diff alone does not. The giv is based at
 * sprt+0xe (clut at 0, w at 2, h at 4, v0 at -1), so lever 3 is right. The
 * counter's read-modify-write is *emitted* between the two `pairs` stores even
 * though writing it there in the source measures worse -- so its emitted
 * position is sched2's and says nothing about where it was written. And the
 * 0x20 is materialised once inside the loop and used for both `w` and `h`,
 * which is what decomp-permuter's best find here is groping at by assigning
 * 0x20 to a live variable and clobbering it. Hoisting it honestly, as a `size`
 * local beside `white` at the layer-3 and layer-4 entries, was 38/14 as `u8`,
 * 42/14 as `s32` and 38/14 assigned ahead of `white`; writing `h` before `w`
 * was 38/13. All measured on the withdrawn base, so all worth one re-measure
 * before being believed again.
 *
 * The barrier dimension was covered on that base and the numbers are still
 * useful as relative evidence even though the base is gone: inside the
 * layer-3/4 bodies, a barrier after `SetSprt`, after `SetShadeTex` or after
 * `sprt->b0 = white` is exactly inert -- a barrier next to a call boundary
 * emits nothing and changes nothing -- while one after `sprt->v0`, after
 * `sprt->h`, bracketing `sprite34Count++`, or before the clut store all move
 * the diff. What that says is that a `do { } while (0);` here is a scheduling
 * lever and nothing else, which is exactly why it cannot pay for itself: the
 * six nops it costs are real instructions and the alignment it buys is not.
 *
 * The arm-inversion dimension is closed too, and it is worth stating because
 * CLAUDE.md records it as a real lever elsewhere: inverting the three
 * `(flags & 0x80) == 0` SetSemiTrans tests and swapping their arms is
 * 53/7, inverting the three `run[0] == 0x7FFE` sentinel tests 93/36, and
 * both together 101/39, against 43/4. Every branch polarity in this
 * function is already the target's.
 *
 * decomp-permuter has now been run against this exact body (base score
 * 1937, 126k iterations, `perm_pad_var_decl` at 200 since the residue is
 * stack layout) and its best candidate, 1547, measures **43/4** -- exactly
 * the body it started from. The change was `perm_temp_for_expr`'s: an `int`
 * temporary between `count` and `run[2]` in one of the two layers. Applied
 * to layer 3, to layer 4, and as an `s16` rather than an `int`, it is
 * inert; applied to both layers at once it is 68/4. So a fifth of the
 * permuter's score moved for no change in the build at all, on a scratch
 * whose relocations are clean and whose base.c compiles to exactly 395
 * instructions. Re-measure before believing any run on this function.
 *
 * `while (--count)` and `while (0 != --count)` are byte-identical to
 * `while (--count != 0)`; `while (count-- != 1)` is 52/17 and
 * `while (count-- > 1)` 61/16. Writing the decrement as the loop body's
 * *first* statement -- `do { count--; ... } while (count != 0);` -- is
 * catastrophic (185/26): with no `--count` on the back edge gcc does not
 * recognise the biv and the whole loop is rebuilt.
 *
 * A row count is only comparable between two bodies of the same length, and
 * this function is the demonstration. Measured against each other, the
 * withdrawn 26/11 body and the 43/4 body here differ by seventeen rows and by
 * six instructions, and the seventeen are mostly the six: three `nop`s per
 * barrier, the `--count` chain displaced behind them, and eight branch
 * offsets that shift because everything after the first barrier moved by
 * 0xc. Strip those and the two bodies are within a couple of rows of each
 * other, with the shorter one having the correct length and the correct
 * frame. The rule this cost a session to re-learn: fit the length first, and
 * treat a row count that improves while the length grows as evidence against
 * the change, not for it.
 *
 * The pad-size grid was re-measured on the restored base as part of lever 8
 * above and the numbers there supersede an earlier grid (none 64, 0x8 77,
 * 0x10 55, 0x18 through 0x30 all 77) taken on a body that no longer exists.
 * Two other levers are also gone with the base they were measured on: an
 * eight-row `do { } while (0);` after layer 4's inner loop, and
 * `sprite34Count++` between the two `pairs` stores. Both are covered by the
 * withdrawal of lever 11 and lever 10 respectively; neither survives on this
 * body.
 *
 * Also measured and rejected, on one base or another -- the row counts are
 * only comparable within a bullet:
 *   - every ordering of `count = run[2];` and `run[1] = sprite34Count;` and
 *     every spelling of the guard between them; see lever 9 above, where the
 *     dimension is closed.
 *   - `SetDrawMode(modes++, ...)` at the layer-1/3/4 sites (97 against 87
 *     before lever 6, 71 against 67 after it).
 *   - layer 2 written b0, r0, g0 (98 against 87).
 *   - layer 4 alone written back as a backward goto walk, keeping 1-3 as
 *     loops: 134/20 against 67/15. The four walks stand or fall together --
 *     see lever 1. What prompted it is real, though, and is still open: the
 *     layer-4 loop hoists `&run[1]` into its preheader as a giv
 *     (`addiu s2,s4,2`, bumped by 6 each iteration) where the target and our
 *     own layer 3 both address it as `2(s4)`. Two rows.
 *   - the counters moved to just after `pairs[0]` (71/13) or just after the
 *     clut store (70/17) instead of to the end of the body.
 *   - `tpages++` written ahead of `modes++` rather than after (88 against 67).
 * **47 rows -> 45**, from `spriteCount` declared `s32` rather than `u16`
 * (`u32` is byte-identical). `tools/width_sweep.py` found it, and the full
 * 47-variant cross-product over `count`/`spriteCount`/`sprite34Count`/`white`
 * then closes the dimension: 45 is the floor, `count` must stay `s16` (every
 * `s32 count` variant is **-4 instructions** and 110 rows or worse), and
 * `sprite34Count` and `white` are free at any width. (45 was the floor *of
 * that base only*; lever 12 reverses the `spriteCount` half of it. The
 * `count` half stands -- `s16` is still right, for the `sll a0,a0,0x10` in
 * the target's loop test.)
 *
 * The frame-pad and address-take pair was re-swept afterwards, because
 * changing a counter's width is exactly the sort of thing that makes a
 * finished sweep stale -- and this one survived. All eleven combinations of
 * pad size (none, 4, 8, 0x10, 0x18, 0x20) against taking or not taking
 * `&sprite34Count` measure 65 rows or worse, several at +2 instructions,
 * against 45 for the pairing already here. So CLAUDE.md's note that the two
 * lock each other still holds at the new width.
 *
 * -- and that conclusion is the one lever 8 above withdraws. The sweep was
 * honest and the dimension was genuinely exhausted; it was exhausted with
 * the guard held wrong, and the guard is what supplies the frame the pad was
 * buying. Two exhaustive sweeps of the same two knobs, a session apart, both
 * concluding "the pair is load-bearing", and both wrong for the same reason:
 * when a swept dimension will not move, suspect the thing you are holding
 * fixed, not the sweep.
 *
 * What is left reads as three spills: `sw +3 / lw +3` against `sh -3`, plus
 * `nop +2`, `lh -2`, `addu -2`, `lhu -1`. The target keeps in halfwords what
 * this body spills as words, and no width reaches it -- which points at
 * *which* values are in memory rather than how wide they are. (It was the
 * width after all: `spriteCount` is a spilled pseudo and reload spills in
 * the pseudo's mode. The reason no width reached it *here* is that the pad
 * had put the counter in the declared pool's shadow at 0x38, where its
 * offset was wrong whatever its width. See lever 12.)
 *
 *   - `D_801144C8 = spriteCount` deferred to the layer-4 entry: it does flip
 *     the allocation on its own (147/22, before the loop change was found) but
 *     the store then lands in the wrong block -- the target stores it at the
 *     layer-3 entry, from the stack slot -- and on top of the loop change it
 *     measures 123 against 118.
 *   - `s32`/`s16`/`u32` counters (no change or worse); declaring the counters
 *     ahead of the pointers (no change); `s32 count` (216/25); `white`
 *     declared ahead of `count` (no change); a `white` at the layer-2 entry
 *     (no change). Re-measured on the lever 8-10 base, since the body has
 *     moved: `s16` counters inert, `s32` and `u32` counters 36/12, `u16 count`
 *     82/12 and `s32 count` 98/11, against 34/13. The counter types are
 *     genuinely free and `count` is genuinely `s16`.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldBackgroundInitPackets);
#else
void FieldBackgroundInitPackets(
    SPRT_16* sprt16, SPRT* sprt, u8* pairs, DR_MODE* modes) {
    FieldBgData* data1;
    FieldBgData* data2;
    FieldBgData* data3;
    FieldBgTile1* tile1;
    FieldBgTile2* tile2;
    u16* tpages;
    s16* run;
    s16 count;
    u8 white;
    u16 spriteCount;
    u16 sprite34Count;

    spriteCount = 0;
    sprite34Count = 0;
    D_8011448C = 0;
    D_801144D0 = 0;
    data1 = *D_8009D848;
    run = data1->runs;
    tile1 = (FieldBgTile1*)((u8*)data1 + data1->layer1Offset);
    tpages = (u16*)((u8*)data1 + data1->tpageOffset);

    for (;;) {
        if (run[0] == 0x7FFF) {
            run++;
            goto layer2;
        }
        if (run[0] == 0x7FFE) {
            SetDrawMode(modes, 0, 1, tpages[0], NULL);
            tpages++;
            D_8011448C++;
            modes++;
        } else {
            count = run[2];
            if (count != 0) {
                do {
                    SetSprt16(sprt16);
                    SetShadeTex(sprt16, 1);
                    SetSemiTrans(sprt16, 0);
                    sprt16->r0 = 0x80;
                    sprt16->g0 = 0x80;
                    sprt16->b0 = 0x80;
                    sprt16->x0 = tile1->x;
                    sprt16->y0 = tile1->y;
                    sprt16->u0 = tile1->u;
                    sprt16->v0 = tile1->v;
                    sprt16->clut = tile1->clut;
                    tile1++;
                    sprt16++;
                    spriteCount++;
                    pairs += 2;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer2:
    D_8011448C = spriteCount - D_8011448C;
    data2 = *D_8009D848;
    tile2 = (FieldBgTile2*)((u8*)data2 + data2->layer2Offset);

    for (;;) {
        if (run[0] == 0x7FFF) {
            run++;
            goto layer3;
        }
        count = run[2];
        if (count != 0) {
            do {
                SetDrawMode(modes, 0, 1, tile2->tpage, NULL);
                modes++;
                D_801144D0++;
                SetSprt16(sprt16);
                SetShadeTex(sprt16, 1);
                if ((tile2->flags & 0x80) == 0) {
                    SetSemiTrans(sprt16, 0);
                } else {
                    SetSemiTrans(sprt16, 1);
                }
                sprt16->r0 = tile2->rg;
                sprt16->g0 = tile2->rg >> 8;
                sprt16->b0 = 0x80;
                sprt16->x0 = tile2->x;
                sprt16->y0 = tile2->y;
                sprt16->u0 = tile2->u;
                sprt16->v0 = tile2->v;
                sprt16->clut = tile2->clut;
                pairs[0] = tile2->flags;
                pairs[1] = tile2->param;
                tile2++;
                sprt16++;
                spriteCount++;
                pairs += 2;
            } while (--count != 0);
        }
        run += 3;
    }

layer3:
    white = 0x80;
    D_801144C8 = spriteCount;
    data3 = *D_8009D848;
    D_8007EBD4 = (FieldBgTile3*)((u8*)data3 + data3->layer34Offset);

    for (;;) {
        if (run[0] == 0x7FFF) {
            run++;
            goto layer4;
        }
        if (run[0] == 0x7FFE) {
            SetDrawMode(modes, 0, 1, tpages[0], NULL);
            tpages++;
            modes++;
        } else {
            run[1] = sprite34Count;
            count = run[2];
            if (count != 0) {
                do {
                    SetSprt(sprt);
                    SetShadeTex(sprt, 1);
                    if ((D_8007EBD4->flags & 0x80) == 0) {
                        SetSemiTrans(sprt, 0);
                    } else {
                        SetSemiTrans(sprt, 1);
                    }
                    sprt->r0 = white;
                    sprt->g0 = white;
                    sprt->b0 = white;
                    sprt->x0 = D_8007EBD4->x;
                    sprt->y0 = D_8007EBD4->y;
                    sprt->u0 = D_8007EBD4->u;
                    sprt->v0 = D_8007EBD4->v;
                    sprt->w = 0x20;
                    sprt->h = 0x20;
                    sprt->clut = D_8007EBD4->clut;
                    pairs[0] = D_8007EBD4->flags;
                    sprite34Count++;
                    pairs[1] = D_8007EBD4->param;
                    D_8007EBD4++;
                    sprt++;
                    pairs += 2;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer4:
    white = 0x80;
    for (;;) {
        if (run[0] == 0x7FFF) {
            return;
        }
        if (run[0] == 0x7FFE) {
            SetDrawMode(modes, 0, 1, tpages[0], NULL);
            tpages++;
            modes++;
        } else {
            run[1] = sprite34Count;
            count = run[2];
            if (count != 0) {
                do {
                    SetSprt(sprt);
                    SetShadeTex(sprt, 1);
                    if ((D_8007EBD4->flags & 0x80) == 0) {
                        SetSemiTrans(sprt, 0);
                    } else {
                        SetSemiTrans(sprt, 1);
                    }
                    sprt->r0 = white;
                    sprt->g0 = white;
                    sprt->b0 = white;
                    sprt->x0 = D_8007EBD4->x;
                    sprt->y0 = D_8007EBD4->y;
                    sprt->u0 = D_8007EBD4->u;
                    sprt->v0 = D_8007EBD4->v;
                    sprt->w = 0x20;
                    sprt->h = 0x20;
                    sprt->clut = D_8007EBD4->clut;
                    pairs[0] = D_8007EBD4->flags;
                    sprite34Count++;
                    pairs[1] = D_8007EBD4->param;
                    D_8007EBD4++;
                    sprt++;
                    pairs += 2;
                } while (--count != 0);
                do {
                } while (0);
            }
        }
        run += 3;
    }
}
#endif

/* Background layer wrap distances, in the field's trigger block. Layers 3 and
 * 4 scroll on their own and a tile that leaves the camera window is moved a
 * whole layer width or height rather than being redrawn. */
typedef struct {
    /* 0x00 */ u8 unk00[0x18];
    /* 0x18 */ u16 wrapX3;
    /* 0x1A */ u16 wrapY3;
    /* 0x1C */ u16 wrapX4;
    /* 0x1E */ u16 wrapY4;
} FieldBgWrap;

/* Right and bottom edge of the camera window, one entry per scrolling group:
 * [0] layers 1 and 2, [1] layer 3, [2] layer 4. Reading these through the
 * array is what keeps them out of a register across the tile loops -- as a
 * plain scalar gcc decides they cannot alias the struct stores and hoists the
 * load, where the original reloads the bound every iteration. */
typedef struct {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
} FieldBgCamera;

extern FieldBgCamera D_80071A48[3];
/* The ordering-table slots layers 3 and 4 link their sprites into. Read
 * through the struct for the same reason as FieldBgCamera above: as scalars
 * gcc hoists the load out of the walk and keeps the slot in a register. */
typedef struct {
    /* 0x00 */ u16 layer4;
    /* 0x02 */ u16 layer3;
} FieldBgOtSlot;

extern FieldBgOtSlot D_8009ACA2;

/* Link this frame's visible background tiles into the field's ordering table.
 * The four layers share one run list -- the same one FieldBackgroundInitPackets
 * walked to build the packets -- with a 0x7FFF word between them, so each layer
 * starts where the previous one stopped. A 0x7FFE run is a texture-page change
 * and contributes only its DR_MODE.
 *
 * Layers 1 and 2 are static: a run is skipped whole when its row is off the
 * camera, and each tile is culled again on x. Layer 2 additionally carries the
 * OT slot inside the sprite's own r0/g0 bytes and links a DR_MODE behind every
 * tile. Layers 3 and 4 scroll independently, so a tile that has left the
 * window is wrapped in place, and both are bracketed by their own draw
 * environments.
 *
 * Four findings are folded in above and are worth more than this function --
 * see CLAUDE.md, where each has its own bullet:
 *   - the camera bounds, the two OT slots and the animation pairs are read
 *     through an array or a struct, not as scalars, so their loads stay inside
 *     the tile loops instead of being hoisted (MEM_IN_STRUCT_P aliasing);
 *   - the four walks are `for (;;)` loops left by `goto`, which keeps the
 *     invariant hoisting a backward goto would lose without paying for
 *     duplicate_loop_exit_test's rotation;
 *   - the globals are read directly rather than through locals, so their loads
 *     join the other movables in each phase's preheader;
 *   - the wrap test is an `||` of two `<=`, not `!(a && b)`; gcc 2.6.3 does not
 *     apply De Morgan and the two spellings schedule differently.
 *
 * ================= READ THIS BEFORE ANY NUMBER BELOW =================
 *
 * **The 65-row body this note used to describe read an undefined register,
 * and every measurement taken against it is worth nothing.** Lever 6 was
 * `layer3Slot = &D_8009ACA2.layer3;` written on the line above the `layer3:`
 * label -- and the layer-2 walk is a `for (;;)` that is left only by
 * `goto layer3`, so that line is *unreachable*. jump_optimize deleted the
 * block, the pseudo kept its use and lost its def, and global_alloc gave the
 * undefined value `$s3`: `lhu v1,0(s3)` at 0x292c with no write to `s3`
 * anywhere above it in the function. The seven rows lever 6 was credited with
 * were an allocation perturbation bought by undefined behaviour, and the
 * bodies the sweeps below were measured against all carry it.
 * The check that finds this class in one command, on any body whose diff has
 * a callee-saved register in it that the target does not use for that value:
 * disassemble the object and grep the function for writes to that register.
 * A register read and never written is not a codegen residue, it is a bug.
 *
 * The honest baseline with the pointer deleted and the addPrim macro written
 * plainly is **72 rows, 0 insertions, 0 deletions, at the exact 658**, and
 * from there one lever takes it to **64**:
 *
 *   - **layer 2's mask test wants the same `otSlot` shape as layers 3 and 4.**
 *     `if (entity == 0 || (g_FieldEntityBgTrigger[entity] &
 *     (otSlot = buf->BgAnim[sprite].mask)))` -- the assignment-inside-the-test
 *     that lever 2 already applies to the two scrolling layers -- is **72 ->
 *     64** and closes the whole layer-2 entity block (the `v1`/`a0` and
 *     `v0`/`v1` rows at 137-151 that this note never attributed to anything).
 *     `otSlot` has to stay *one* variable across all three layers: a separate
 *     `maskSlot` for the three mask reads is 67. That is the counter-merging
 *     rule on a scalar -- the three layers describe the same quantity.
 *
 * Re-measured against the corrected 64-row body and all inert or worse:
 * `entity` split into one local per layer (inert, 64), `otSlot = run[0]` in
 * layer 1's kind test (inert), `(otSlot = buf->Bg1[sprite].x0)` in layer 1's
 * or layer 2's x test (inert), `(otSlot = D_80071A48[0].y)` in layer 1's or
 * layer 2's y test (71 / 70), `otSlot = run[1]` for layer 1's BgDm addPrim
 * (149 at -5 instructions). `width_sweep.py` over all four scalar locals, 20
 * variants: **flat** -- nothing ties 64 and the next alternatives are 70+.
 *
 * The five levers 1-5 below were all re-measured on the *corrected* body and
 * all still pay: without the layer-2 dead conditional 141, without the
 * layer-4 dead `else if` 85, without the layer-3 `do { } while (0);` 73,
 * without the two mask-test assignments 116 at +1 instruction, with the
 * layer-3 mask test's operands unswapped 73.
 *
 * **The reference multiplier cannot reach this residue, and the reason is a
 * mechanism rather than a measurement.** CLAUDE.md offers exactly one
 * construct that adds references to a pseudo without emitting an instruction
 * -- a `do { X } while (0);` with a body, whose loop notes make `flow` count
 * everything inside at `loop_depth + 1`. Lever 4 below *is* one of those, so
 * the construct is live on this function. It still cannot move the two
 * quantities the ranking needs raised, because the trivial loop is a real
 * loop to `loop_optimize`: nothing is modified on its back edge, so *every*
 * expression inside it is invariant, `move_movables` lifts it into the
 * do-while's own preheader, and its references land back at the outer depth.
 * Only insns that cannot be hoisted -- stores, calls, and anything reading a
 * value set inside -- keep the deeper weight.
 *
 * Measured, not inferred: wrapping layer 1's `&buf->BgDm[run[1]]` addPrim in
 * one, two, three and four nested `do { } while (0);` leaves pseudo 96 (the
 * 0x124DC BgDm offset, the quantity that has to reach >= 12 references) at
 * exactly **5 refs / 80 insns** in `.lreg` at every depth, while `buf` goes
 * 136 -> 152 and three ot-slot quantities rise with it, because the addPrim's
 * *stores* stay inside. Same for `&D_80071A48`, unchanged at 4/146. So the
 * specification below -- `run` at <= 16 refs, or the layer-1 constant at
 * >= 12 -- is not merely unmet, it is unreachable by the only construct that
 * could have met it: both of those quantities are compiler-generated address
 * constants whose every reference is in a hoistable insn.
 *
 * The three sweeps that closed on this base (all >= 64, none an improvement):
 *
 *   - **`do { } while (0);` nesting and placement**, 21 variants. Depth on
 *     the layer-3 0x7FFE addPrim: 0 -> 73, 1 -> **64**, 2 -> 76, 3 -> 81,
 *     4 -> 82. The same wrapper on layer 4's 0x7FFE addPrim -- the direct
 *     sibling the old sweep never tried -- is exactly inert (64), at depth 2
 *     76 and depth 3 87. On the layer-3 and layer-4 sprite addPrims exactly
 *     inert; on layer 2's first addPrim 80/2, its second 71/1, both 75/1; on
 *     layer 1's 0x7FFE addPrim 127, its sprite addPrim 71/1 (at depth 2 75,
 *     depth 3 70, depth 4 70); on BgDrenv4S 72, BgDrenv3S 103/1, BgDrenv3E
 *     211/12, BgDrenv4E 202/3.
 *   - **the sibling asymmetries between the four layers.** Layer 4's dead
 *     `else if` (lever 3) added to layer 3's wrapX3 70, to layer 3's wrapY3
 *     70, to layer 4's own wrapY4 77; with the other axis as the guard 76 and
 *     83. Layer 2's dead exit conditional (lever 1) replicated at layer 3's
 *     exit 136/1 and at layer 4's 144/17. Six alternative guards for lever 1
 *     itself: `D_80071A48[0].y` 129/4, `.x` 129/4, `&buf->BgDm[0]` 64,
 *     `run[0]` 133, `otSlot` 222/2, `entity` 192/1 -- so `sprite ||` is not
 *     merely one working guard, it is the only one.
 *   - **`run`'s own spelling**, which no earlier sweep varied even though
 *     `run` is the quantity whose rank has to fall: `run += 3` moved into the
 *     `for` clause is exactly inert in layers 1, 2 and 3 (64 each).
 *
 * =====================================================================
 *
 * 204 rows: 1 instruction out and 203 rows of register naming. The four wrap
 * tests read `buf->Bg2[sprite].x0` / `.y0` directly at every use rather than
 * through an `s16 x` / `s16 y` temporary -- decomp-permuter found it (score
 * 1050 -> 765) and it measures 244 rows -> 204 against the overlay, the single
 * largest step this function has taken. It is the same lever as the "read a
 * loop-invariant global directly" bullet in CLAUDE.md, applied to a struct
 * field: the temporary is a source statement that lands among the surrounding
 * code, where the repeated reference is a common subexpression cse places
 * itself. Inlining only the layer-3 pair (the permuter's literal find) is worth
 * 18 of the 40 rows; all four is the rest.
 *
 * What is left is delay slots: the target wastes a slot after each of the four
 * wrap tests' camera loads (it evaluates the sprite coordinate first, which no
 * spelling of the comparison reproduced -- five were measured), and fills the
 * entity-check branch in layer 3 with the join block's `sll a0,a1,0x2` where
 * this build emits a nop and the `sll` after the join. That one `sll` is the
 * *only* non-register row in 889 instructions, and it is a consequence rather
 * than a cause: the target has $a0 free there because it addresses the trigger
 * byte through $v1, this build through $a0.
 * The &run[2]-versus-&run[1] induction variable this note used to blame has
 * been measured and is not the problem: both builds compute the same two
 * bases, `addiu <r>,v0,0x10` and `addiu <r>,v0,0x14` at 0x2328, and differ
 * only in which register each lands in.
 * 96 rows, no insertions, and still nothing but register naming -- but half
 * of what the note below called unreachable is now closed, and the two levers
 * that did it are both things no reading of the target suggests.
 *
 * Both came out of decomp-permuter and both are *rank* levers, which is what
 * the analysis below said was needed. gcc assigns $t0..$t9 in allocno-priority
 * order, priority is `floor_log2(n_refs) * n_refs / live_length * size` with
 * n_refs weighted by loop depth, so an extra reference at depth 0 -- even one
 * whose code is deleted again -- reorders the whole caller-saved set.
 *
 *   1. a dead conditional in front of the layer-2 walk's exit:
 *
 *          if (sprite || ((P_TAG*)&buf->ot[D_8009ACA2.layer4])->addr) {
 *              run++;
 *              goto layer3;
 *          }
 *          run++;
 *          goto layer3;
 *
 *      Both paths do the same thing and both operands are pure, so nothing of
 *      it survives to the object -- the insertion count is 0 -- and it is
 *      worth 56 rows on its own. The condition is load-bearing in every part:
 *      `sprite ||` alone is 112, the `->addr` term alone is 161, `count` in
 *      place of `sprite` is 154, dropping the cast so the load is the plain
 *      `u_long` is 151/2, and a bare `do { } while (0);` -- the barrier that
 *      finished FieldArrowsAddToRender -- is 159. The `else` arm the permuter
 *      wrote is not needed (96 either way); the duplicated tail is.
 *      Repeating the same construct at the layer-1 exit is 98 and at the
 *      layer-3 exit 165/29, so it belongs to that one exit.
 *   2. the layer-3 and layer-4 mask tests written through `otSlot`:
 *
 *          if (entity == 0 ||
 *              ((otSlot = buf->BgAnim[sprite + D_801144C8].mask) &
 *               g_FieldEntityBgTrigger[entity])) {
 *
 *      34 rows for the pair, and it has to be `otSlot` -- a fresh `s32 mask`
 *      local in its place is 99. One site alone is 159 (layer 3) or 184
 *      (layer 4).
 *
 *   3. a dead `else if` splitting the layer-4 wrapX4 subtraction:
 *
 *          } else if (buf->Bg2[sprite].x0) {
 *              <the same subtraction>
 *          } else {
 *              <the same subtraction>
 *          }
 *
 *      13 more rows, again with no insertion -- cross-jumping merges the two
 *      arms and the test is deleted, and what is left is the reference it
 *      added. Same class as lever 1, and also a decomp-permuter find; the two
 *      together are what took this function from 193 to 83.
 *
 *   4. a `do { } while (0);` wrapped around the layer-3 walk's 0x7FFE addPrim,
 *      nine more rows and again no insertion. The placement is the whole of
 *      it: the same wrapper on the layer-1 0x7FFE addPrim is 136, on the
 *      layer-1 sprite addPrim 80/1, on BgDrenv3E 208/12, on BgDrenv3S 112/1,
 *      on BgDrenv4E 208/3, and on the layer-3 sprite addPrim exactly inert.
 *      193 -> 83 -> 74 on three inserted blocks, none of which survives to the
 *      object.
 *   5. both mask tests written `g_FieldEntityBgTrigger[entity] & (otSlot =
 *      ...)` rather than the other way round: gcc evaluates a comparison's
 *      operands in source order, so the swap decides which of the two loads
 *      goes first. One row per site and they are independent (73 either way,
 *      72 for both). Nothing else of that kind pays -- `run[0] >
 *      D_80071A48[0].y - 0x100` for either wrap test or both, and the same
 *      rewrite of the layer-1 x tests, are all exactly inert.
 *   6. WITHDRAWN -- this was the undefined `layer3Slot` read described at the
 *      top of this note, not a lever. For the record, on a *correct* program
 *      the same idea costs rows rather than saving them: the assignment moved
 *      down to just after the `layer3:` label, where it is reachable and
 *      dominates its one use, is 70, and routing all eight layer-3 reads of
 *      the slot through the pointer is 113 at -1 instruction. The target reads
 *      all eight through **one** register (`0($t1)`, and it is
 *      `&D_8009ACA2.layer3` = D_8009ACA4), which is what the plain `addPrim`
 *      spelling gives; two source variables for it is the fault, not the fix.
 *
 * What is left is a three-cycle plus two swaps in the layer-1 preheader, and
 * it says the same thing the old analysis did, one place less far out:
 *
 *   target: 0x124DC, &D_80071A48, run;   here: run, 0x124DC, &D_80071A48
 *
 * so `run` is still ranked one place too high. D_8011448C against $s0, the two
 * 0xFF000000 materialisations and g_FieldTriggers all follow from it.
 *
 * The three quantities can be read straight out of cc1's own dumps rather than
 * guessed at (see CLAUDE.md for the incantation -- the dump has to be written
 * inside the container, a bind-mounted `-dumpbase` silently produces 0-byte
 * files). In `.lreg`, pseudo 73 is `run`, 97 is the 0x124DC constant and 142
 * is `&D_80071A48`, and `.greg`'s post-reload RTL shows them taking $t5, $t6
 * and $t7 here:
 *
 *     Register  73 used 47 times across 538 insns   pri (235-538)/538 = -0.56
 *     Register  97 used  5 times across  80 insns   pri ( 10- 80)/ 80 = -0.88
 *     Register 142 used  4 times across 146 insns   pri (  8-146)/146 = -0.95
 *
 * so on `allocno_compare`'s ranking alone `run` is not one place too high, it
 * is at the top by a wide margin: for the 0x124DC constant to overtake it its
 * reference count would have to go from 5 to 12, and for `&D_80071A48` from 4
 * to 16. That is not a tweak, and the obvious way to get there does not work
 * -- a `FieldBgCamera* cam = D_80071A48;` local used at all 22 sites is 165/3
 * whether it is assigned at the top of the function or beside `run`, because
 * it also turns every `mem` with a `symbol_ref` address into a based access
 * and the relocation set changes with it (2 symbol aliases against 14).
 *
 * That "other half of `find_reg`" escape hatch the note used to offer -- maybe
 * the conflict sets differ rather than the ranking -- is **closed, and the
 * dump closes it**. `find_reg` really does hand out the lowest-numbered
 * non-conflicting hard register, and this function's own `.greg` proves it
 * against the obvious counter-example: pseudo 105 is allocated seven places
 * *after* `run` and still gets `$t2`, which reads like a cost model until you
 * check its conflict list -- it conflicts with 149/150 (holding $a2/$a1) and
 * with 101 and 1219 (holding $t0/$t1) but with nothing in $t2, so $t2 *is* its
 * lowest free register. Every disposition in the function is consistent with
 * lowest-free. So the register each of these three quantities gets is decided
 * by `allocno_compare`'s order and nothing else, and the numbers above are a
 * proof rather than an estimate.
 *
 * Which makes the residue a genuinely sharp statement about the original, and
 * the numbers are worth having exactly: with 0 insertions and 0 deletions the
 * RTL is the same on both sides (CLAUDE.md's rule), so the *only* thing that
 * can differ is how many pseudos the source cuts the function into. For the
 * target's order to come out, `run` needs **<= 16 references** (floor_log2 x
 * n_refs < 67 at length 538) where it has 47, or the layer-1 0x124DC constant
 * needs **>= 12** (at length 80) where it has 5. Nothing in between helps and
 * nothing about the spelling of an expression moves either number, because
 * REG_N_REFS is counted on post-cse RTL.
 *
 * The one structure that would deliver it -- a per-loop cursor, `r = run;` at
 * the top of each walk with the body reading `r[0]`/`r[1]`/`r[2]` and only
 * `run = r + 1` / `run = r + 3` touching `run` -- **does not survive cse**:
 * measured on layer 1 it takes `run` from 47 references to **55**, because cse
 * propagates `r` straight back into every subscript and the two assignments
 * are pure additions to `run`'s own count. 135 rows at -1 instruction. A
 * `do { } while (0);` after `r = run;` to give cse a basic-block boundary is
 * exactly inert -- 135 again, the same -1 -- so the barrier does not save it
 * either. Whatever the original wrote, it is not a second pointer.
 *
 * Re-measured against *this* base, since a park note's negatives are only good
 * for the state they were taken in: a per-layer `sprite` is 151, a per-layer
 * `count` 119, both split 157. The old note's numbers for the same three
 * (207/203/207) were taken against the 193-row base and are consistent -- the
 * counter-merging rule holds either way.
 *
 * Measured and rejected across three sessions, all still against a body with
 * these two levers absent, so re-check before trusting any of them:
 * goto loops (434 rows, no hoisting at all), `break` out of the loops (396),
 * s32 wrap temporaries (frame 0x30, 320 rows), a separate s32 temporary
 * loaded before the wrap test (278), locals for
 * D_8011448C/D_801144C8/g_FieldTriggers (305), both `!(a && b)` and
 * `a > lo && a < hi` for the wrap test (251), referencing run[2] before
 * run[1] in all four walks (236 against 233), a per-layer `count` (203), a
 * per-layer `sprite` (207), both split (207), `entity` as u32 or s32
 * (inert), `otSlot` as s16 (214/2), swapping the `otSlot`/`entity`
 * declaration order (inert), and folding `data` away into
 * `run = (*D_8009D848)->runs;` (inert). The per-layer splits are the
 * interesting negatives: the same change on FieldBackgroundInitPackets'
 * `data` was worth 12 rows, so "split pointers that describe different
 * things" is not a rule that transfers -- there the three pointers really did
 * address three different objects, here `count` and `sprite` describe the
 * same walk four times over and merging them is correct.
 *
 * The `perm_ins_block` class does not touch this function at all, which is
 * itself the confirmation that the residue is allocation and not scheduling:
 * a `do { } while (0);` at the end of any one of the four inner loop bodies,
 * or of all four, is exactly inert -- 65 rows either way -- where the same
 * barrier is worth eight rows in FieldBackgroundInitPackets. A barrier emits
 * nothing and adds no references, so it can move a schedule and cannot move
 * an allocno's rank.
 *
 * The obvious way to change the rank -- a reference to one of the two
 * constants, added inside an inner loop where flow weights it double -- has
 * been tried in the shape that worked for levers 1 and 3 and does not work
 * here. A dead `if (x) { addPrim } else { addPrim }` around the layer-1
 * sprite addPrim is 125/1, around the layer-2 one 74/2, both 132/3, and with
 * `D_80071A48[0].x` as the condition instead of `buf->Bg1[sprite].y0` 125/1
 * and 78/2. Every one of them costs an insertion, which is the difference
 * from levers 1 and 3: there the two arms were byte-identical after reload
 * and cross-jumping deleted the whole construct, here the guard's load
 * survives. So the reference-count lever is real and the spelling that
 * delivers it without emitting anything is not one this reading found.
 *
 * Both of `insn_histogram.py`'s tables are **clean** on this body -- 658
 * against 658 instructions, opcodes `(identical)`, `%hi` materialisations per
 * address `(identical)` -- so the residue cannot be an addressing or an
 * instruction-mix fault; there is nothing left for it to be except which
 * register each quantity got.
 *
 * That used to be written here as "so the `allocno_compare` arithmetic is the
 * only remaining explanation, and a residue that reduces to allocno arithmetic
 * is a park, not a search". **That inference is backwards and it is what kept
 * this function parked for three sessions.** Two clean tables and a diff with
 * 0 insertions and 0 deletions mean the two objects have the *same instruction
 * stream*, which means the same RTL, which means the same `n_refs` and
 * `live_length`, which means `allocno_compare` produces the *same order on
 * both sides*. A ranking that is provably equal cannot be the thing that
 * differs. What differs is how many pseudos the source cuts the block into --
 * and that is reachable from C. The layer-2 `otSlot` lever at the top of this
 * note is exactly such a change, found by asking "how many variables?" rather
 * than "which register?", and it was worth 8 rows on a body this note had
 * already declared finished.
 *
 * So the arithmetic below is still worth having -- it says precisely what a
 * candidate structure has to deliver (`run` at <= 16 refs, or the layer-1
 * constant at >= 12) -- but as a *specification for the search*, not as a
 * verdict that no search is possible.
 *
 * (Read that with the tool's own history in mind. Before commits 2ac13ad /
 * 7e68f5f the same tables reported `D_80071A48 +6` against five interior
 * halfwords at -1 each, which reads exactly like "the original declares six
 * scalars where we declare an array" -- a whole program-level hypothesis,
 * arrived at from a table, contradicting this note. It was arithmetic on
 * symbol *names*: `%hi(D_80071A48+2)` and `%hi(D_80071A4A)` are the same
 * address and the same linked bytes. The totals gave it away -- 8 against 8,
 * 2 against 2 -- and totals are worth checking on any table whose rows are
 * complementary.)
 *
 * This one belongs to decomp-permuter, and the scratch must be re-imported
 * against the **corrected 64-row body** -- every scratch built before this
 * note was rewritten was importing a program with an undefined register read
 * in it, which is the same class of mistake as the `align`-broke-a-declaration
 * failure recorded for FieldMain. The pass that matters is `perm_refer_to_var`
 * -- it adds a reference to a variable, which is exactly the term in
 * `allocno_compare` that has to move -- with `perm_var_cond_block`,
 * `perm_dummy_comma_expr` and `perm_add_self_assignment` behind it; a 55,000
 * iteration run at the default weights, which spend most of their budget on
 * `perm_temp_for_expr`, found nothing at all. Note that `perm_temp_for_expr`
 * is nevertheless the pass that found the layer-2 `otSlot` shape by hand, so
 * weight it second rather than dropping it.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", AddBackgroundToRender);
#else
void AddBackgroundToRender(struct FieldRenderData* buf) {
    FieldBgData* data;
    s16* run;
    s16 count;
    s16 sprite;
    s32 otSlot;
    u8 entity;

    data = *D_8009D848;
    run = data->runs;

    for (;;) {
        if (run[0] == 0x7FFF) {
            run++;
            goto layer2;
        }
        if (run[0] == 0x7FFE) {
            addPrim(&buf->ot[0xFFF], &buf->BgDm[run[1]]);
        } else if (
            D_80071A48[0].y - 0x100 < run[0] && run[0] < D_80071A48[0].y) {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (D_80071A48[0].x - 0x150 < buf->Bg1[sprite].x0 &&
                        buf->Bg1[sprite].x0 < D_80071A48[0].x) {
                        addPrim(&buf->ot[0xFFF], &buf->Bg1[sprite]);
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer2:
    for (;;) {
        if (run[0] == 0x7FFF) {
            if (sprite || ((P_TAG*)&buf->ot[D_8009ACA2.layer4])->addr) {
                run++;
                goto layer3;
            }
            run++;
            goto layer3;
        }
        if (D_80071A48[0].y - 0x100 < run[0] && run[0] < D_80071A48[0].y) {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (D_80071A48[0].x - 0x150 < buf->Bg1[sprite].x0 &&
                        buf->Bg1[sprite].x0 < D_80071A48[0].x) {
                        entity = buf->BgAnim[sprite].entity & 0x3F;
                        if (entity == 0 ||
                            (g_FieldEntityBgTrigger[entity] &
                             (otSlot = buf->BgAnim[sprite].mask))) {
                            otSlot = buf->Bg1[sprite].r0 +
                                     (buf->Bg1[sprite].g0 << 8);
                            addPrim(&buf->ot[otSlot], &buf->Bg1[sprite]);
                            addPrim(&buf->ot[otSlot],
                                    &buf->BgDm[sprite - D_8011448C]);
                        }
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer3:
    addPrim(&buf->ot[D_8009ACA2.layer3], &buf->BgDrenv3E);
    for (;;) {
        if (run[0] == 0x7FFF) {
            addPrim(&buf->ot[D_8009ACA2.layer3], &buf->BgDrenv3S);
            run++;
            goto layer4;
        }
        if (run[0] == 0x7FFE) {
            do {
                addPrim(&buf->ot[D_8009ACA2.layer3],
                        &buf->BgDm[run[1] + D_801144D0]);
            } while (0);
        } else {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (buf->Bg2[sprite].x0 <= D_80071A48[1].x - 0x160 ||
                        D_80071A48[1].x <= buf->Bg2[sprite].x0) {
                        if (buf->Bg2[sprite].x0 < D_80071A48[1].x - 0xA0) {
                            buf->Bg2[sprite].x0 =
                                buf->Bg2[sprite].x0 +
                                ((FieldBgWrap*)g_FieldTriggers)->wrapX3;
                        } else {
                            buf->Bg2[sprite].x0 =
                                buf->Bg2[sprite].x0 -
                                ((FieldBgWrap*)g_FieldTriggers)->wrapX3;
                        }
                    }
                    if (buf->Bg2[sprite].y0 <= D_80071A48[1].y - 0x100 ||
                        D_80071A48[1].y <= buf->Bg2[sprite].y0) {
                        if (buf->Bg2[sprite].y0 < D_80071A48[1].y - 0x70) {
                            buf->Bg2[sprite].y0 =
                                buf->Bg2[sprite].y0 +
                                ((FieldBgWrap*)g_FieldTriggers)->wrapY3;
                        } else {
                            buf->Bg2[sprite].y0 =
                                buf->Bg2[sprite].y0 -
                                ((FieldBgWrap*)g_FieldTriggers)->wrapY3;
                        }
                    }
                    entity = buf->BgAnim[sprite + D_801144C8].entity & 0x3F;
                    if (entity == 0 ||
                        (g_FieldEntityBgTrigger[entity] &
                         (otSlot = buf->BgAnim[sprite + D_801144C8].mask))) {
                        addPrim(&buf->ot[D_8009ACA2.layer3], &buf->Bg2[sprite]);
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }

layer4:
    addPrim(&buf->ot[D_8009ACA2.layer4], &buf->BgDrenv4E);
    for (;;) {
        if (run[0] == 0x7FFF) {
            addPrim(&buf->ot[D_8009ACA2.layer4], &buf->BgDrenv4S);
            return;
        }
        if (run[0] == 0x7FFE) {
            addPrim(
                &buf->ot[D_8009ACA2.layer4], &buf->BgDm[run[1] + D_801144D0]);
        } else {
            sprite = run[1];
            count = run[2];
            if (count != 0) {
                do {
                    if (buf->Bg2[sprite].x0 <= D_80071A48[2].x - 0x160 ||
                        D_80071A48[2].x <= buf->Bg2[sprite].x0) {
                        if (buf->Bg2[sprite].x0 < D_80071A48[2].x - 0xA0) {
                            buf->Bg2[sprite].x0 =
                                buf->Bg2[sprite].x0 +
                                ((FieldBgWrap*)g_FieldTriggers)->wrapX4;
                        } else if (buf->Bg2[sprite].x0) {
                            buf->Bg2[sprite].x0 =
                                buf->Bg2[sprite].x0 -
                                ((FieldBgWrap*)g_FieldTriggers)->wrapX4;
                        } else {
                            buf->Bg2[sprite].x0 =
                                buf->Bg2[sprite].x0 -
                                ((FieldBgWrap*)g_FieldTriggers)->wrapX4;
                        }
                    }
                    if (buf->Bg2[sprite].y0 <= D_80071A48[2].y - 0x100 ||
                        D_80071A48[2].y <= buf->Bg2[sprite].y0) {
                        if (buf->Bg2[sprite].y0 < D_80071A48[2].y - 0x70) {
                            buf->Bg2[sprite].y0 =
                                buf->Bg2[sprite].y0 +
                                ((FieldBgWrap*)g_FieldTriggers)->wrapY4;
                        } else {
                            buf->Bg2[sprite].y0 =
                                buf->Bg2[sprite].y0 -
                                ((FieldBgWrap*)g_FieldTriggers)->wrapY4;
                        }
                    }
                    if (D_80071A48[2].x - 0x160 < buf->Bg2[sprite].x0 &&
                        buf->Bg2[sprite].x0 < D_80071A48[2].x) {
                        entity = buf->BgAnim[sprite + D_801144C8].entity & 0x3F;
                        if (entity == 0 ||
                            (g_FieldEntityBgTrigger[entity] &
                             (otSlot =
                                  buf->BgAnim[sprite + D_801144C8].mask))) {
                            addPrim(
                                &buf->ot[D_8009ACA2.layer4], &buf->Bg2[sprite]);
                        }
                    }
                    sprite++;
                } while (--count != 0);
            }
        }
        run += 3;
    }
}
#endif

s32 FieldCalcLinearStep(s32 start, s32 target, s32 duration, s32 step) {
    s32 delta = target - start;

    if ((u32)(delta + 0x7FFFF) <= 0xFFFFE) {
        start += (delta * step) / duration;
    } else {
        start += (delta / duration) * step;
    }

    return start;
}

s32 FieldCalcEaseInOut(s32 from, s32 to, s32 total, s32 step) {
    s32 angle;
    s32 diff;

    angle = ((step << 12) / total) / 32 - 0x80;
    diff = to - from;
    return from +
           ((FieldEntityGetDirVectorY(angle & 0xFF) + 0x1000) * diff) / 0x2000;
}

s32 FieldCalcWorldToScreenPos(SVECTOR* worldPos, long* screenPos) {
    long flag;
    long depth;
    s32 ret;

    PushMatrix();
    SetRotMatrix(D_80071E40);
    SetTransMatrix(D_80071E40);
    SetGeomOffset(0, 0);
    ret = RotTransPers(worldPos, screenPos, &flag, &depth);
    PopMatrix();
    return ret;
}

/* Advance one axis of the SHAKE camera effect. The shake runs as a chain of
 * segments: each one eases the background offset from where the previous
 * segment left off to a fresh random target, negated relative to the last so
 * the image swings either side of centre. Clearing `enabled` does not stop it
 * dead -- it eases one final segment back to zero first.
 *
 * Every instruction matches except the four that increment currentStep: gcc
 * coalesces `step + 1` into a3 (the argument register) where the original
 * keeps it in v0, which lets the original schedule the store into the call's
 * delay slot. */
void FieldBGShakeUpdate(FieldShakeData* shake) {
    s16 step;
    s16 target;

    if (shake->enabled == 1) {
        if (shake->segmentActive == 0) {
            shake->currentStep = 0;
            shake->start = 0;
            shake->target =
                (s16)(g_RandomTable[shake->rngId] * shake->amplitude) / 256;
            shake->segmentActive = 1;
            shake->rngId++;
            return;
        }
        step = shake->currentStep;
        if (shake->numStepsPerSegment < step) {
            target = shake->target;
            shake->currentStep = 0;
            shake->start = target;
            if (target < 0) {
                shake->target =
                    (s16)(g_RandomTable[shake->rngId] * shake->amplitude) / 256;
            } else {
                shake->target =
                    -(s16)(g_RandomTable[shake->rngId] * shake->amplitude) /
                    256;
            }
            shake->rngId++;
            return;
        }
    } else if (shake->segmentActive == 1) {
        step = shake->currentStep;
        if (shake->numStepsPerSegment < step) {
            shake->currentStep = 0;
            shake->start = shake->target;
            shake->target = 0;
            shake->segmentActive = 0;
            shake->rngId++;
            return;
        }
    } else {
        step = shake->currentStep;
        if (shake->numStepsPerSegment == step) {
            shake->currentOffset = 0;
            return;
        }
    }
    /* The two arms are deliberately identical. gcc cross-jumps them back into
     * one block, but only after it has allocated `step + 1` to its own
     * register instead of coalescing it into a3 -- which is what lets the
     * store of currentStep be scheduled into the call's delay slot, as the
     * original does. Written once, the tail is four instructions off. */
    if (step != 0) {
        step = step + 1;
        shake->currentStep = step;
        shake->currentOffset = FieldCalcEaseInOut(
            shake->start, shake->target, shake->numStepsPerSegment, step);
    } else {
        step = step + 1;
        shake->currentStep = step;
        shake->currentOffset = FieldCalcEaseInOut(
            shake->start, shake->target, shake->numStepsPerSegment, step);
    }
}
