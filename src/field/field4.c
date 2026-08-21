//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* One model part's polygon groups, as LoadLocalFieldModelAndInitAll splices
 * them: eight per-primitive-kind counts packed two words to a record, an
 * offset to the polygon block and the relocated pointer to the model data the
 * offset is measured from. */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ u32
        polyCounts0; // gouraud quad, gouraud tri, then two flat quads
    /* 0x08 */ u32 polyCounts1; // two flat tris, then a gouraud tri and quad
    /* 0x0C */ u16 unkC;
    /* 0x0E */ u16 polyOffset;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u8* data;
    /* 0x1C */ u8* unk1C;
} FieldModelPart;

extern SVECTOR D_800DF520[]; // light normals, indexed by a colour's code byte

/* Unit 4 of 5, split out of field.c. .rodata 0x800A0104-0x800A0F10, base 4 mod
 * 8 -> --phase 4. The large middle run: 20 of the overlay's jump tables, all 4
 * mod 8. */

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiExecute);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetCustomLightToModelPkts);

extern u8 D_800DF114;

/* Apply the GTE lighting to each vertex colour of a model's *packets*: for
 * every polygon of every kind, run NormalColorColSingle on the vertex normal
 * the polygon table names and write the result straight into the packet's RGB.
 * The sibling KawaiLightingApplyToPolyColor below does the same GTE work in
 * place in the polygon table; this one is the pass that pushes the result out
 * to the GPU packets, so it walks two cursors at once.
 *
 * 202 rows / 12 insertions at 294 instructions against 293, from an m2c seed
 * that did not compile and could not have been measured: every GTE op came
 * out as an `M2C_ERROR` holding the raw text `unknown instruction: lwc2`,
 * `nccs`, `swc2`, which
 * emits nothing, so the body was 35 instructions short before any codegen
 * question could be asked. What the rewrite established:
 *
 *   - The parameter is a `FieldModelPart*`, and the typedef lives 350 lines
 *     below in this unit; the seed's `void* arg0` with `->unk1C` is why gcc
 *     rejected the whole file with the body unparked. Lifted the typedef.
 *   - The GTE sequence is the sibling's, verbatim:
 *     `gte_ldv0(normals + c[k * 4 + 7] * 8)`, `gte_ldrgb(&c[k * 4 + 4])`,
 *     `gte_nccs()`, then `gte_strgb` -- but into `pkt + 4` rather than the
 *     scratchpad, with no three-byte copy back, because the packet *is* the
 *     destination.
 *   - `swc2` writes four bytes, so it clobbers the packet's `code` byte at +7.
 *     The target saves it (`lbu t5,0(t2)`) before the inner loop and restores
 *     it (`sb t5,0(t2)`) after -- which is what m2c renders as the inert
 *     `*var_t2 = *var_t2;`.
 *   - Eight loops, and both strides differ per kind. The polygon-table
 *     strides are the sibling's exactly (0x18, 0x14, 0xC, 0xC, 8, 8, 0x10,
 *     0x14); the packet strides are 0x34, 0x28, 0x28, 0x20, 0x14, 0x18, 0x1C,
 *     0x24, and the per-vertex colour step is 0xC for the untextured
 *     primitives and 8 for the textured ones. Read them off the `addiu` in
 *     each outer loop's branch delay slot.
 *   - Each iteration is guarded by `if (*(u32*)pkt != 0)` -- the packet's tag
 *     word, i.e. only linked packets are lit.
 *   - Two cursors over the polygon table, not one. The target takes a fixed
 *     snapshot (`move t4,t1`) that the rgb giv is measured from *and* a
 *     walking one (`move a0,t1`, stepped by 4) that supplies the normal index
 *     at [7]. One `c = poly;` gives a single base with two givs. Worth 4 rows
 *     and 2 instructions, but only together with the next item.
 *   - The snapshot is taken *before* the saved code byte: the target's
 *     preheader is `t4 = t1 / t5 = *t2 / v1 = 0 / a2 = a3 + 4`. Reordering
 *     alone is 198/+3, two cursors alone 206/+1, both 202/+1 -- and the
 *     comparison that matters is within a length class, where 202 beats 206.
 *
 * Measured and rejected: copying the parameter into a `FieldModelPart* p` and
 * reading every field through it, to reproduce the target's `move t8,a0`, is
 * *exactly* inert -- gcc coalesces the copy away. That one instruction plus
 * two more elsewhere is the whole of the +1.
 *
 * A warning for whoever picks this up: an earlier pass here measured 168 rows
 * and -3 instructions, and it was a different program. The variant generator's
 * anchor had been broken by the reordering above, so the walking cursor `n`
 * was used at four sites and assigned at none. An uninitialised local is legal
 * C, gcc says nothing, and the number looks like the best result of the
 * session. Grep the body for an assignment to every local before believing a
 * sweep.
 *
 * 202 rows -> 169, from the lever that matched the sibling
 * KawaiLightingApplyToPolyColor below: the *first* inner loop gets its own
 * pair of cursor variables, so the shared pair's reference count drops by a
 * loop and the allocation changes underneath. Both cursors have to be split
 * -- giving loop 1 its own `c1` while it still shares `n` is exactly inert.
 * Which loop is split does not matter and neither does splitting more of
 * them: loops {1}, {1,2}, {1,4}, {2,3,4} and all four measure 169 to the
 * row, so 169 is a plateau rather than a step. In the sibling the same
 * change was worth the match outright; here it leaves the $t8 parameter copy
 * and one instruction still unaccounted for.
 *
 * What is left is register naming with the length within one, which is
 * decomp-permuter's job. Codegen pinned via MASPSX_OVERRIDE; the #else is the
 * verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field4", KawaiSetVertexColorFromLighting);
#else
void KawaiSetVertexColorFromLighting(FieldModelPart* part) {
    u8* normals;
    u8* poly;
    u8* pkt;
    u8* c;
    u8* n;
    u8* c1;
    u8* n1;
    u8* rgb;
    u8 code;
    u32 counts;
    u32 count;
    u32 i;
    u32 k;

    poly = (u8*)(part->polyOffset + (u32)part->data);
    normals = (u8*)D_800DF520;
    pkt = part->unk1C;
    if (D_800DF114 != 0) {
        pkt += part->unk16;
    }
    counts = part->polyCounts0;

    count = counts & 0xFF;
    for (i = 0; i < count; i++, pkt += 0x34, poly += 0x18) {
        if (*(u32*)pkt != 0) {
            c1 = poly;
            n1 = poly;
            code = pkt[7];
            rgb = pkt + 4;
            for (k = 0; k < 4; k++) {
                gte_ldv0(normals + n1[7] * 8);
                gte_ldrgb(&c1[k * 4 + 4]);
                gte_nccs();
                gte_strgb(rgb);
                rgb += 0xC;
                n1 += 4;
            }
            pkt[7] = code;
        }
    }

    count = (counts & 0xFF00) >> 8;
    for (i = 0; i < count; i++, pkt += 0x28, poly += 0x14) {
        if (*(u32*)pkt != 0) {
            c = poly;
            n = poly;
            code = pkt[7];
            rgb = pkt + 4;
            for (k = 0; k < 3; k++) {
                gte_ldv0(normals + n[7] * 8);
                gte_ldrgb(&c[k * 4 + 4]);
                gte_nccs();
                gte_strgb(rgb);
                rgb += 0xC;
                n += 4;
            }
            pkt[7] = code;
        }
    }

    count = (counts >> 16) & 0xFF;
    for (i = 0; i < count; i++, pkt += 0x28, poly += 0xC) {
        if (*(u32*)pkt != 0) {
            code = pkt[7];
            gte_ldv0(normals + poly[7] * 8);
            gte_ldrgb(&poly[4]);
            gte_nccs();
            gte_strgb(&pkt[4]);
            pkt[7] = code;
        }
    }

    count = counts >> 24;
    for (i = 0; i < count; i++, pkt += 0x20, poly += 0xC) {
        if (*(u32*)pkt != 0) {
            code = pkt[7];
            gte_ldv0(normals + poly[7] * 8);
            gte_ldrgb(&poly[4]);
            gte_nccs();
            gte_strgb(&pkt[4]);
            pkt[7] = code;
        }
    }

    counts = part->polyCounts1;

    count = counts & 0xFF;
    for (i = 0; i < count; i++, pkt += 0x14, poly += 8) {
        if (*(u32*)pkt != 0) {
            code = pkt[7];
            gte_ldv0(normals + poly[7] * 8);
            gte_ldrgb(&poly[4]);
            gte_nccs();
            gte_strgb(&pkt[4]);
            pkt[7] = code;
        }
    }

    count = (counts & 0xFF00) >> 8;
    for (i = 0; i < count; i++, pkt += 0x18, poly += 8) {
        if (*(u32*)pkt != 0) {
            code = pkt[7];
            gte_ldv0(normals + poly[7] * 8);
            gte_ldrgb(&poly[4]);
            gte_nccs();
            gte_strgb(&pkt[4]);
            pkt[7] = code;
        }
    }

    count = (counts >> 16) & 0xFF;
    for (i = 0; i < count; i++, pkt += 0x1C, poly += 0x10) {
        if (*(u32*)pkt != 0) {
            c = poly;
            n = poly;
            code = pkt[7];
            rgb = pkt + 4;
            for (k = 0; k < 3; k++) {
                gte_ldv0(normals + n[7] * 8);
                gte_ldrgb(&c[k * 4 + 4]);
                gte_nccs();
                gte_strgb(rgb);
                rgb += 8;
            }
            pkt[7] = code;
        }
    }

    count = counts >> 24;
    for (i = 0; i < count; i++, pkt += 0x24, poly += 0x14) {
        if (*(u32*)pkt != 0) {
            c = poly;
            n = poly;
            code = pkt[7];
            rgb = pkt + 4;
            for (k = 0; k < 4; k++) {
                gte_ldv0(normals + n[7] * 8);
                gte_ldrgb(&c[k * 4 + 4]);
                gte_nccs();
                gte_strgb(rgb);
                rgb += 8;
            }
            pkt[7] = code;
        }
    }
}
#endif

/* Applies one unaligned little-endian u16 per colour channel to every part of
 * the model, and puts the seventh byte in the scratchpad for the handwritten
 * KawaiSetColorToPartPkts to pick up.
 *
 * The twelve bytes of locals are load-bearing, in two separate ways.
 *
 * `unused` is not a variable the original had a name for -- it is 8 bytes of
 * frame that nothing ever stores to, and its only job here is to make the
 * prologue `addiu sp,sp,-0x40` rather than -0x38. gcc counts an aggregate
 * local into the frame whether or not it is read, and a scalar one does not
 * survive at all, so an array is the only way to reproduce it. What the
 * original declared there is unrecoverable from the object code.
 *
 * The six byte reads have to be their own statements, high byte before low
 * byte in each pair, because that is the order the loads issue in. Folding
 * them back into the three ORs leaves the operands right and the schedule
 * wrong; hoisting only data[5] gets every register correct and still emits
 * its load first instead of fifth. */
s32 KawaiSetColorToModelPkts(FieldModelEntry* model, u8* data) {
    u8 unused[8];
    u8* parts;
    u32 count;
    u32 i;
    s32 r;
    s32 g;
    s32 b;
    u32 redLo;
    u32 greenLo;
    u32 blueLo;
    u32 redHi;
    u32 greenHi;
    u32 blueHi;

    count = model->partCount;
    parts = model->modelData + model->partsOffset;
    redHi = data[1];
    redLo = data[0];
    greenHi = data[3];
    greenLo = data[2];
    blueHi = data[5];
    blueLo = data[4];
    r = redLo | (redHi << 8);
    g = greenLo | (greenHi << 8);
    b = blueLo | (blueHi << 8);
    *(u32*)0x1F800200 = data[6];
    for (i = 0; i < count; i++) {
        KawaiSetColorToPartPkts(&parts[i * 32], r, g, b);
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetColorToPartPkts);

/* Load this model's animated eye/mouth textures into VRAM. The face selector
 * is four bytes: two mouth frames, one eye frame, and the model's own slot
 * index, which both places the tiles in VRAM and gates the whole function --
 * slots 0x21 and up have no tiles reserved for them and the call is a no-op.
 * Each variant is looked up in a per-textureFaceId index table (mouth: stride
 * 7, eye: stride 3) and the matching 0x200-byte page of the shared model
 * texture block is uploaded to that slot's tile.
 *
 * 23 rows out, and the instruction count is exact -- the function ends at the
 * same address as the target, so nothing here is a missing or extra insn.
 * The whole residue is one allocation choice and the scheduling that follows
 * it: the selector byte `faceSel[k]` lands in a0 (blocks 1 and 3) or v1
 * (block 2) where the target puts it in a1 every time. Block 1 is otherwise
 * instruction-for-instruction exact, including both divisions and all four
 * rect stores; blocks 2 and 3 differ only in where the D_800DFCA0 load and
 * the `sll ..,9` get scheduled around that one register.
 *
 * What took it from 101 rows to 23, all of which are real findings:
 *   - `slot / 4` and `slot / 8`, not `>> 2` and `>> 3`. The target has the
 *     signed-division rounding fixup (`bgez` over `addiu s2,3`), which a
 *     shift does not emit.
 *   - The LoadImage source has to be a named local assigned *before* the four
 *     rect stores. Written inline as the second argument it is evaluated with
 *     the call, so the whole lookup lands after the `sh`s: 66 rows.
 *   - Two quotient variables, not one. A single `q` reused by the mouth and
 *     the eye block is live across both LoadImage calls and takes a
 *     callee-saved register, where the target's eye quotient is caller-saved
 *     (v1) because its range is inside the last block. Worth 4 rows.
 *   - D_800DFCA0 and model->textureFaceId are re-read at every use. Caching
 *     either in a local collapses three loads into one.
 *   - The index tables are two-dimensional. `D_800DFCA4[faceId * 7 + sel]`
 *     folds the whole index into one `addu`; `[faceId][sel]` gives the
 *     target's two, symbol-then-selector.
 *
 * Measured and inert (all 23 rows): `u8*` instead of `u_long*` for the source
 * pointer, `* 0x200` instead of `<< 9`, three separate source locals, `(s32)`
 * on the faceId subscript, all five declaration orders of the five locals,
 * `*(&D_800DFCA4[faceId][0] + sel)`, and `+ 0x300 + 8` for the second x.
 * Measured and worse: the selector byte hoisted into a local (50), either
 * quotient computed before the source pointer (67 and 38), `&rect` hoisted
 * into a `RECT*` (60), `faceSel` copied to a local pointer (35), and the
 * pointer-plus spelling of the table index (58). Codegen pinned via
 * MASPSX_OVERRIDE; the #else is the verified C. */
extern u8 D_800DFCA4[][7]; /* mouth texture page index, per face, per frame */
extern u8 D_800DFD94[][3]; /* eye texture page index, per face, per frame */

/* Reload one model's eye and mouth textures into VRAM: three LoadImage calls
 * into a per-model 16x32 slot, the two mouth halves side by side at x+0x300
 * and x+0x308 and the eyes on the row below at y+0x1A0. `faceSel` is the
 * per-frame selection -- [0] and [1] index the mouth table, [2] the eye
 * table, [3] the VRAM slot.
 *
 * Three things this needed, and none of them is readable off the target:
 *   - `sel = faceSel[k];` into an **s32** local before each `src`. Written
 *     inline, the byte the table is indexed by is loaded into whatever
 *     register is free and the three sites disagree with the target's $a1;
 *     a `u8` local is exactly inert (cse coalesces it with the load), an
 *     `s32` one is worth 28 rows to 7. Same cost rule as the `u8`-through-a-
 *     local bullet in CLAUDE.md: widening is what stops the substitution.
 *   - the two divisions are written **inline at every use**, not carried in
 *     `q` and `eyeQ` locals. Re-deriving `slot / 8` for the eye row rather
 *     than reusing the local is the last row: with the local, gcc shifts it
 *     in place (`sll v1,v1,5`, clobbering the quotient) and schedules the
 *     result one slot early; re-derived, cse shares the division but gives
 *     the shift its own destination, which is the target's `sll v0,v1,5`
 *     after `addiu a0,sp,0x10`. Dropping `q` the same way is also exact.
 *   - nothing else. Both divisions really are divisions (`>> 2`/`>> 3` is six
 *     instructions short), rect.w and rect.h really are re-stored at all
 *     three sites (setting them once is eight short), and the fill order
 *     really is x, y, w, h -- all six permutations were measured worse.
 *
 * `D_800DFCA0` is read three times, once per site, because each LoadImage
 * call invalidates it; writing it inline at every use is what reproduces
 * that. `D_800DFCA4` and `D_800DFD94` are each addressed once and hoisted. */
s32 KawaiLoadEyesMouthTexToVram(FieldModelEntry* model, u8* faceSel) {
    RECT rect;
    u_long* src;
    s32 slot;
    s32 sel;

    slot = faceSel[3];
    if (slot < 0x21) {
        sel = faceSel[0];
        src = (u_long*)((u8*)D_800DFCA0 + D_800DFCA0->pageOffset +
                        (D_800DFCA4[model->textureFaceId][sel] << 9));
        rect.x = ((slot - (slot / 4) * 4) << 4) + 0x300;
        rect.y = ((slot / 4) << 5) + 0x100;
        rect.w = 8;
        rect.h = 0x20;
        LoadImage(&rect, src);
        sel = faceSel[1];
        src = (u_long*)((u8*)D_800DFCA0 + D_800DFCA0->pageOffset +
                        (D_800DFCA4[model->textureFaceId][sel] << 9));
        rect.x = ((slot - (slot / 4) * 4) << 4) + 0x308;
        rect.y = ((slot / 4) << 5) + 0x100;
        rect.w = 8;
        rect.h = 0x20;
        LoadImage(&rect, src);
        sel = faceSel[2];
        src = (u_long*)((u8*)D_800DFCA0 + D_800DFCA0->pageOffset +
                        (D_800DFD94[model->textureFaceId][sel] << 9));
        rect.x = ((slot - (slot / 8) * 8) << 3) + 0x300;
        rect.y = ((slot / 8) << 5) + 0x1A0;
        rect.w = 8;
        rect.h = 0x20;
        LoadImage(&rect, src);
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiLightingApplyToModel);

/* Light one model part in place. Every polygon colour word carries the index
 * of its vertex normal in the byte the GPU would read as `code`, so each is
 * fed to NormalColorColSingle and the result written back over the same three
 * bytes. The eight groups differ only in the primitive's stride and how many
 * colour words it carries. Bit 1 of the part's data header marks it lit, so a
 * second call with redo == 0 returns immediately.
 *
 * Three things about this one are not guessable and are what the matching
 * body needed, in the order they were worth instructions:
 *
 *   - The inner loops index a *snapshot* of the polygon cursor, not the
 *     cursor itself: `c = poly;` at the top of the outer body is where the
 *     target's `move <reg>,<reg>` at each inner-loop preheader comes from,
 *     and it was worth 44 rows.
 *   - **The first loop needs its own snapshot variable.** With one `c`
 *     shared by all four inner loops the `move`s are all there but `c` and
 *     `count` hold each other's register ($t2/$t3) and every row in the diff
 *     is that swap. Giving loop 1 a `c1` of its own drops `c`'s reference
 *     count from four loops to three, `count` wins $t2, and the function
 *     matches. Removing the snapshot from loop 1 altogether -- which is what
 *     decomp-permuter found, at 23 rows against 32 -- fixes the same swap and
 *     loses the `move`, so it is one instruction short; the two facts
 *     together are the answer. Declaration order of `c1` against `c` is
 *     inert, as usual.
 *   - The scratchpad pointer is assigned after `normals` and before the
 *     early-return guard. The target materialises `lui t1,0x1F80` in that
 *     branch's delay slot, so it is the last thing the entry block computes;
 *     first (34/2), between `data` and `normals` (34/2) and after the guard
 *     (38/2) are all worse than here (32/0).
 *
 * The sibling KawaiSetVertexColorFromLighting above shares the GTE idiom and
 * the polygon strides exactly and is still parked; it wants the same two
 * cursors, and this function is the worked example. */
void KawaiLightingApplyToPolyColor(FieldModelPart* part, s32 redo) {
    u8* scratch;
    u8* normals;
    u8* data;
    u8* poly;
    u8* c;
    u8* c1;
    u32 counts;
    u32 count;
    u32 i;
    u32 k;

    data = part->data;
    normals = (u8*)D_800DF520;
    scratch = (u8*)0x1F800000;
    if ((*(u32*)data & 2) && redo == 0) {
        return;
    }

    poly = (u8*)(part->polyOffset + (u32)data);
    counts = part->polyCounts0;

    count = counts & 0xFF;
    for (i = 0; i < count; i++, poly += 0x18) {
        c1 = poly;
        for (k = 0; k < 4; k++) {
            gte_ldv0(normals + c1[k * 4 + 7] * 8);
            gte_ldrgb(&c1[k * 4 + 4]);
            gte_nccs();
            gte_strgb(scratch);
            c1[k * 4 + 4] = scratch[0];
            c1[k * 4 + 5] = scratch[1];
            c1[k * 4 + 6] = scratch[2];
        }
    }

    count = (counts & 0xFF00) >> 8;
    for (i = 0; i < count; i++, poly += 0x14) {
        c = poly;
        for (k = 0; k < 3; k++) {
            gte_ldv0(normals + c[k * 4 + 7] * 8);
            gte_ldrgb(&c[k * 4 + 4]);
            gte_nccs();
            gte_strgb(scratch);
            c[k * 4 + 4] = scratch[0];
            c[k * 4 + 5] = scratch[1];
            c[k * 4 + 6] = scratch[2];
        }
    }

    count = (counts >> 16) & 0xFF;
    for (i = 0; i < count; i++, poly += 0xC) {
        gte_ldv0(normals + poly[7] * 8);
        gte_ldrgb(&poly[4]);
        gte_nccs();
        gte_strgb(scratch);
        poly[4] = scratch[0];
        poly[5] = scratch[1];
        poly[6] = scratch[2];
    }

    count = counts >> 24;
    for (i = 0; i < count; i++, poly += 0xC) {
        gte_ldv0(normals + poly[7] * 8);
        gte_ldrgb(&poly[4]);
        gte_nccs();
        gte_strgb(scratch);
        poly[4] = scratch[0];
        poly[5] = scratch[1];
        poly[6] = scratch[2];
    }

    counts = part->polyCounts1;

    count = counts & 0xFF;
    for (i = 0; i < count; i++, poly += 8) {
        gte_ldv0(normals + poly[7] * 8);
        gte_ldrgb(&poly[4]);
        gte_nccs();
        gte_strgb(scratch);
        poly[4] = scratch[0];
        poly[5] = scratch[1];
        poly[6] = scratch[2];
    }

    count = (counts & 0xFF00) >> 8;
    for (i = 0; i < count; i++, poly += 8) {
        gte_ldv0(normals + poly[7] * 8);
        gte_ldrgb(&poly[4]);
        gte_nccs();
        gte_strgb(scratch);
        poly[4] = scratch[0];
        poly[5] = scratch[1];
        poly[6] = scratch[2];
    }

    count = (counts >> 16) & 0xFF;
    for (i = 0; i < count; i++, poly += 0x10) {
        c = poly;
        for (k = 0; k < 3; k++) {
            gte_ldv0(normals + c[k * 4 + 7] * 8);
            gte_ldrgb(&c[k * 4 + 4]);
            gte_nccs();
            gte_strgb(scratch);
            c[k * 4 + 4] = scratch[0];
            c[k * 4 + 5] = scratch[1];
            c[k * 4 + 6] = scratch[2];
        }
    }

    count = counts >> 24;
    for (i = 0; i < count; i++, poly += 0x14) {
        c = poly;
        for (k = 0; k < 4; k++) {
            gte_ldv0(normals + c[k * 4 + 7] * 8);
            gte_ldrgb(&c[k * 4 + 4]);
            gte_nccs();
            gte_strgb(scratch);
            c[k * 4 + 4] = scratch[0];
            c[k * 4 + 5] = scratch[1];
            c[k * 4 + 6] = scratch[2];
        }
    }

    *(u32*)part->data |= 2;
}

/* Set the semi-transparency/shade bits of every packet of every part of one
 * model. Walks each part's double-buffered packet area (the two ordering-table
 * copies) and toggles the ABE and shade bits of each primitive's tag byte, in
 * eight unrolled blocks, one per primitive type (strides 34/28/28/20/14/18/
 * 1C/24).
 *
 * Five levers took this from 153 rows to zero, and four of them are about who
 * gets to be an induction variable:
 *
 *   * `partCount' is not a local. The outer loop's bound is re-read from
 *     `model->partCount' at the end test, and its zero-trip guard *is* the
 *     early return -- a separate `if (partCount == 0) return 1;' in front of
 *     the loop gives two `beqz's where the target has one.
 *   * the part pointer is `&parts[i * 0x20]', not `part += 0x20'. Bumped, it
 *     is a biv, and gcc reduces its eight field accesses onto a second base
 *     at `parts + 0xb' -- invisible in the body, which still reads perfectly.
 *     Indexed off the counter, gcc builds the walking pointer itself and it
 *     comes out `move t1,v1' / `addiu t1,t1,0x20' exactly as the target has.
 *   * there is no `tag' pointer. Writing `base[7]' and letting gcc reduce it
 *     puts the `addiu v1,a2,7' in the loop preheader as a giv initialiser,
 *     which reorg then leaves alone; a hand-written `tag = base + 7' in the
 *     `for' init is an ordinary insn and gets stolen into the guard branch's
 *     delay slot, costing eight rows across the eight loops.
 *   * the increment list is `base += stride, j++', not `j++, base += stride'.
 *     The giv's increment follows its biv's, so the written order decides
 *     whether `addiu v1,v1,stride' lands before or after `addiu a1,a1,1'.
 *     Two rows per loop, seven loops -- the eighth matches either way because
 *     `base' is dead after it and gcc drops the biv increment entirely.
 *   * `parts' is built offset-first: `model->partsOffset + (s32)model->
 *     modelData'. Pointer PLUS puts the pointer first. */
s32 KawaiSetModelTransparency(FieldModelEntry* model, u8* data) {
    u8* parts;
    u8* part;
    u8* base;
    u32 enable;
    u32 i;
    u32 ot;
    u32 j;
    u32 n;

    parts = (u8*)(model->partsOffset + (s32)model->modelData);
    enable = data[0];
    for (i = 0; i < model->partCount; i++) {
        part = &parts[i * 0x20];
        for (ot = 0; ot < 2; ot++) {
            base = *(u8**)(part + 0x1C);
            if (ot != 0) {
                base += *(u16*)(part + 0x16);
            }
            n = part[4];
            for (j = 0; j < n; base += 0x34, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[5];
            for (j = 0; j < n; base += 0x28, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[6];
            for (j = 0; j < n; base += 0x28, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[7];
            for (j = 0; j < n; base += 0x20, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[8];
            for (j = 0; j < n; base += 0x14, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[9];
            for (j = 0; j < n; base += 0x18, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[10];
            for (j = 0; j < n; base += 0x1C, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
            n = part[11];
            for (j = 0; j < n; base += 0x24, j++) {
                if (enable) {
                    base[7] |= 2;
                } else {
                    base[7] &= ~2;
                }
                if (enable) {
                    base[7] |= 1;
                } else {
                    base[7] &= ~1;
                }
            }
        }
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetColorToPktsBelowLvl);

INCLUDE_ASM(
    "asm/us/field/nonmatchings/field4", KawaiSetColorToPartPktsBelowLvl);

/* Per-KAWAI-slot colour fade record (16 slots, 0x3C each; only the first 0x14
 * bytes are used by KawaiFadeModelColor). */
typedef struct {
    /* 0x00 */ s16 curR;
    /* 0x02 */ s16 curG;
    /* 0x04 */ s16 curB;
    /* 0x06 */ s16 targetR;
    /* 0x08 */ s16 targetG;
    /* 0x0A */ s16 targetB;
    /* 0x0C */ s16 deltaR;
    /* 0x0E */ s16 deltaG;
    /* 0x10 */ s16 deltaB;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 done;
    /* 0x14 */ u8 unused[0x28];
} KawaiColorFadeSlot;

extern u8 g_KawaiFadeScratch[]; /* scratch RGB quad, 0x20 before the table */
extern KawaiColorFadeSlot g_KawaiColorFadeSlots[16];

/* The slot table starts 0x20 past the scratch quad. Reaching it that way,
 * rather than through its own g_KawaiColorFadeSlots symbol, is what lets cse
 * hand the scratch's own address back as `-0x20($a2)` off the table base
 * register. */
#define KawaiFadeSlots ((KawaiColorFadeSlot*)(g_KawaiFadeScratch + 0x20))

/* Fade a model's vertex colour over time (KAWAI sub-command). data[0]==0 inits
 * the slot from the descriptor and returns 1; data[0]==1 exports the current
 * colour to the scratch quad, pushes it to the packets, advances each channel
 * toward its target with clamping, and returns 0 (1 if the slot was already
 * finished); any other sub-command returns 1.
 *
 * Six things this needed, and the first three were semantics, not codegen:
 *
 *   - the slot stride is 0x3C, not the 0x14 of live fields. The target's
 *     `sll v0,v1,4 / subu v0,v0,v1 / sll v0,v0,2` is x*60; a 0x14 struct gives
 *     x*20 and every later offset is wrong.
 *   - the dispatch is a `switch`, not two `if`s. `beqz v1,case0` /
 *     `li v0,1` / `beq v1,v0,case1` / `j default` is exactly what
 *     expand_end_case emits for a two-case compare chain, and it reads
 *     data[0] once.
 *   - the return values are 1 / 0 / 1, not 1 / 1 / 0. The default's `1` is the
 *     `li v0,0x1` the switch already materialised as its compare constant.
 *   - the slot table is reached as `g_KawaiFadeScratch + 0x20`, not through its
 * own g_KawaiColorFadeSlots symbol. cse links two constants only when they
 * share a symbol_ref base, so spelling it this way is what lets it hand the
 *     scratch quad's own address back as `-0x20($a2)` off the table base
 *     register -- both for the first scratch store and for the call argument.
 *     Named through g_KawaiColorFadeSlots, gcc materialises a second base
 * register.
 *   - `done = 0` sits at the top of the arm, before the packet push. It is
 *     dead there, but it makes the variable live across the call, which is
 *     what puts it in $s1 rather than a caller-saved register -- and the whole
 *     frame layout follows. sched2 then sinks the `move s1,zero` into the
 *     delay slot of the already-finished test, which is where the target has
 *     it.
 *   - each channel's clamp is one block reached by two `goto`s, not a body
 *     duplicated in both arms. Duplicated, cross-jumping merges only the tail
 *     and cse folds `done |= 1` to `li a1,1` because it can still see that
 *     `done` is 0; shared, the block has two predecessors, cse knows nothing,
 *     and the `ori s1,s1,0x1` the target has survives.
 *
 * The blue channel needs its `goto` written out the long way. R and G take
 * the natural `if (cur < target) goto skip;` and come out with the branch
 * inverted around a jump to the clamp, which is what the target has; B with
 * the same spelling gets the direct `bnez` instead, and no operand order or
 * ternary rewrite moves it. Spelling B's positive arm as an explicit
 * `goto clampB` plus `goto skipB` reproduces the target's polarity. */
s32 KawaiFadeModelColor(FieldModelEntry* model, u8* data) {
    KawaiColorFadeSlot* slot;
    s32 done;
    u8 unusedLocals[0x38];

    slot = &KawaiFadeSlots[data[1]];
    switch (data[0]) {
    case 0:
        slot->curR = data[0x02] | (data[0x03] << 8);
        slot->curG = data[0x04] | (data[0x05] << 8);
        slot->curB = data[0x06] | (data[0x07] << 8);
        slot->targetR = data[0x08] | (data[0x09] << 8);
        slot->targetG = data[0x0A] | (data[0x0B] << 8);
        slot->targetB = data[0x0C] | (data[0x0D] << 8);
        slot->deltaR = data[0x0E] | (data[0x0F] << 8);
        slot->deltaG = data[0x10] | (data[0x11] << 8);
        slot->deltaB = data[0x12] | (data[0x13] << 8);
        slot->unk12 = data[0x14];
        slot->done = 0;
        return 1;
    case 1:
        done = 0;
        g_KawaiFadeScratch[0] = slot->curR;
        g_KawaiFadeScratch[1] = slot->curR >> 8;
        g_KawaiFadeScratch[2] = slot->curG;
        g_KawaiFadeScratch[3] = slot->curG >> 8;
        g_KawaiFadeScratch[4] = slot->curB;
        g_KawaiFadeScratch[5] = slot->curB >> 8;
        g_KawaiFadeScratch[6] = slot->unk12;
        KawaiSetColorToModelPkts(model, g_KawaiFadeScratch);
        if (slot->done != 0) {
            return 1;
        }
        slot->curR += slot->deltaR;
        if (slot->deltaR >= 0) {
            if (slot->curR < slot->targetR) {
                goto skipR;
            }
        } else if (slot->curR > slot->targetR) {
            goto skipR;
        }
        slot->curR = slot->targetR;
        done |= 1;
    skipR:
        slot->curG += slot->deltaG;
        if (slot->deltaG >= 0) {
            if (slot->curG < slot->targetG) {
                goto skipG;
            }
        } else if (slot->curG > slot->targetG) {
            goto skipG;
        }
        slot->curG = slot->targetG;
        done |= 2;
    skipG:
        slot->curB += slot->deltaB;
        if (slot->deltaB >= 0) {
            if (slot->curB >= slot->targetB) {
                goto clampB;
            }
            goto skipB;
        } else if (slot->curB > slot->targetB) {
            goto skipB;
        }
    clampB:
        slot->curB = slot->targetB;
        done |= 4;
    skipB:
        if (done == 7) {
            slot->done++;
        }
        return 0;
    }
    return 1;
}

/* Store/apply a custom GTE lighting setup (KAWAI sub-command). data[0]==0
 * copies the 0x1E-byte descriptor into the slot -- twelve loose bytes, then
 * nine LE u16 words -- and returns 1; data[0]==1 expands the slot into the
 * g_KawaiFadeScratch scratch buffer and calls the handwritten GTE driver,
 * returning 0; any other sub-command returns 0. The slot reuses the
 * KawaiFadeModelColor table's 0x3C stride with a flat lighting-blob layout.
 *
 * Same recipe as KawaiFadeModelColor: a switch rather than two ifs, the table
 * reached as `g_KawaiFadeScratch + 0x20` so cse can hand the scratch quad's
 * address back as `-0x20($a3)`, and 0x38 of stack reserved for locals the
 * original allocates and never uses. The seed also had the byte/word boundary
 * one byte short in both arms, and read each pair through a `u16 pair` local:
 * the target reads the low half as a byte (`lbu`) and the high half through its
 * own `lhu` plus `srl`, which is what two separate reads of the same u16
 * field give -- assigning a u16 into a u8 narrows the load. */
s32 KawaiSetCustomLighting(FieldModelEntry* model, u8* data) {
    u8* slot;
    u8 unusedLocals[0x38];

    slot = (u8*)&KawaiFadeSlots[data[1]];
    switch (data[0]) {
    case 0:
        slot[0x00] = data[0x02];
        slot[0x01] = data[0x03];
        slot[0x02] = data[0x04];
        slot[0x03] = data[0x05];
        slot[0x04] = data[0x06];
        slot[0x05] = data[0x07];
        slot[0x06] = data[0x08];
        slot[0x07] = data[0x09];
        slot[0x08] = data[0x0A];
        slot[0x09] = data[0x0B];
        slot[0x0A] = data[0x0C];
        slot[0x0B] = data[0x0D];
        *(u16*)(slot + 0x0C) = data[0x0E] | (data[0x0F] << 8);
        *(u16*)(slot + 0x0E) = data[0x10] | (data[0x11] << 8);
        *(u16*)(slot + 0x10) = data[0x12] | (data[0x13] << 8);
        *(u16*)(slot + 0x12) = data[0x14] | (data[0x15] << 8);
        *(u16*)(slot + 0x14) = data[0x16] | (data[0x17] << 8);
        *(u16*)(slot + 0x16) = data[0x18] | (data[0x19] << 8);
        *(u16*)(slot + 0x18) = data[0x1A] | (data[0x1B] << 8);
        *(u16*)(slot + 0x1A) = data[0x1C] | (data[0x1D] << 8);
        *(u16*)(slot + 0x1C) = data[0x1E] | (data[0x1F] << 8);
        return 1;
    case 1:
        g_KawaiFadeScratch[0x00] = slot[0x00];
        g_KawaiFadeScratch[0x01] = slot[0x01];
        g_KawaiFadeScratch[0x02] = slot[0x02];
        g_KawaiFadeScratch[0x03] = slot[0x03];
        g_KawaiFadeScratch[0x04] = slot[0x04];
        g_KawaiFadeScratch[0x05] = slot[0x05];
        g_KawaiFadeScratch[0x06] = slot[0x06];
        g_KawaiFadeScratch[0x07] = slot[0x07];
        g_KawaiFadeScratch[0x08] = slot[0x08];
        g_KawaiFadeScratch[0x09] = slot[0x09];
        g_KawaiFadeScratch[0x0A] = slot[0x0A];
        g_KawaiFadeScratch[0x0B] = slot[0x0B];
        g_KawaiFadeScratch[0x0C] = *(u16*)(slot + 0x0C);
        g_KawaiFadeScratch[0x0D] = *(u16*)(slot + 0x0C) >> 8;
        g_KawaiFadeScratch[0x0E] = *(u16*)(slot + 0x0E);
        g_KawaiFadeScratch[0x0F] = *(u16*)(slot + 0x0E) >> 8;
        g_KawaiFadeScratch[0x10] = *(u16*)(slot + 0x10);
        g_KawaiFadeScratch[0x11] = *(u16*)(slot + 0x10) >> 8;
        g_KawaiFadeScratch[0x12] = *(u16*)(slot + 0x12);
        g_KawaiFadeScratch[0x13] = *(u16*)(slot + 0x12) >> 8;
        g_KawaiFadeScratch[0x14] = *(u16*)(slot + 0x14);
        g_KawaiFadeScratch[0x15] = *(u16*)(slot + 0x14) >> 8;
        g_KawaiFadeScratch[0x16] = *(u16*)(slot + 0x16);
        g_KawaiFadeScratch[0x17] = *(u16*)(slot + 0x16) >> 8;
        g_KawaiFadeScratch[0x18] = *(u16*)(slot + 0x18);
        g_KawaiFadeScratch[0x19] = *(u16*)(slot + 0x18) >> 8;
        g_KawaiFadeScratch[0x1A] = *(u16*)(slot + 0x1A);
        g_KawaiFadeScratch[0x1B] = *(u16*)(slot + 0x1A) >> 8;
        g_KawaiFadeScratch[0x1C] = *(u16*)(slot + 0x1C);
        g_KawaiFadeScratch[0x1D] = *(u16*)(slot + 0x1C) >> 8;
        KawaiSetCustomLightToModelPkts(model, g_KawaiFadeScratch);
        return 0;
    }
    return 0;
}

/* Fade a model's vertex colour over time below a light level (KAWAI
 * sub-command). Four channels (cur@0/2/4/6, target@8/A/C/E, delta@10/12/14/16,
 * unk18@0x18, done@0x19 in the 0x3C-stride slot table). data[0]==0 inits the
 * slot from twelve LE u16 descriptor words and returns 1; data[0]==1 exports
 * the four cur channels + unk18 to the g_KawaiFadeScratch scratch buffer,
 * pushes them to the below-level packets, advances each channel toward its
 * target with the sign-aware clamp, bumps done once all four reach 0xF, and
 * returns 0; any other sub-command returns 1.
 *
 * The four-channel twin of KawaiFadeModelColor and it needs every one of that
 * function's spellings: the switch, the table reached as `g_KawaiFadeScratch +
 * 0x20`, `done = 0` at the top of the arm so it lives across the packet push,
 * one shared clamp block per channel reached by two gotos, and the last
 * channel's positive arm written out as an explicit `goto cur3clamp` / `goto
 * cur3done` pair. 0x50 of dead locals here rather than 0x38.
 *
 * One extra: the init arm writes `slot->unk18` before `slot->done`, even
 * though the target stores 0x19 before 0x18. Two stores through the same
 * pointer at different constant offsets do not alias, so sched2 reorders them
 * freely -- reading the store order back out of the target and writing it down
 * puts the `lbu` of data[0x1A] one slot too late, and is the only row that
 * separates the two spellings.
 */
typedef struct {
    /* 0x00 */ u16 cur0;
    /* 0x02 */ u16 cur1;
    /* 0x04 */ u16 cur2;
    /* 0x06 */ u16 cur3;
    /* 0x08 */ s16 target0;
    /* 0x0A */ s16 target1;
    /* 0x0C */ s16 target2;
    /* 0x0E */ s16 target3;
    /* 0x10 */ s16 delta0;
    /* 0x12 */ s16 delta1;
    /* 0x14 */ s16 delta2;
    /* 0x16 */ s16 delta3;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 done;
} KawaiFadeBelowLvlSlot;

s32 KawaiColorFadeBelowLvl(FieldModelEntry* model, u8* data) {
    KawaiFadeBelowLvlSlot* slot;
    s32 done;
    u8 unusedLocals[0x50];

    slot = (KawaiFadeBelowLvlSlot*)&KawaiFadeSlots[data[1]];
    switch (data[0]) {
    case 0:
        slot->cur0 = data[0x02] | (data[0x03] << 8);
        slot->cur1 = data[0x04] | (data[0x05] << 8);
        slot->cur2 = data[0x06] | (data[0x07] << 8);
        slot->cur3 = data[0x08] | (data[0x09] << 8);
        slot->target0 = data[0x0A] | (data[0x0B] << 8);
        slot->target1 = data[0x0C] | (data[0x0D] << 8);
        slot->target2 = data[0x0E] | (data[0x0F] << 8);
        slot->target3 = data[0x10] | (data[0x11] << 8);
        slot->delta0 = data[0x12] | (data[0x13] << 8);
        slot->delta1 = data[0x14] | (data[0x15] << 8);
        slot->delta2 = data[0x16] | (data[0x17] << 8);
        slot->delta3 = data[0x18] | (data[0x19] << 8);
        slot->unk18 = data[0x1A];
        slot->done = 0;
        return 1;
    case 1:
        done = 0;
        g_KawaiFadeScratch[0] = slot->cur0;
        g_KawaiFadeScratch[1] = slot->cur0 >> 8;
        g_KawaiFadeScratch[2] = slot->cur1;
        g_KawaiFadeScratch[3] = slot->cur1 >> 8;
        g_KawaiFadeScratch[4] = slot->cur2;
        g_KawaiFadeScratch[5] = slot->cur2 >> 8;
        g_KawaiFadeScratch[6] = slot->cur3;
        g_KawaiFadeScratch[7] = slot->cur3 >> 8;
        g_KawaiFadeScratch[8] = slot->unk18;
        KawaiSetColorToPktsBelowLvl(model, g_KawaiFadeScratch);
        if (slot->done != 0) {
            return 1;
        }
        slot->cur0 += slot->delta0;
        if (slot->delta0 >= 0) {
            if ((s16)slot->cur0 < slot->target0) {
                goto cur0done;
            }
        } else if ((s16)slot->cur0 > slot->target0) {
            goto cur0done;
        }
        slot->cur0 = slot->target0;
        done |= 1;
    cur0done:
        slot->cur1 += slot->delta1;
        if (slot->delta1 >= 0) {
            if ((s16)slot->cur1 < slot->target1) {
                goto cur1done;
            }
        } else if ((s16)slot->cur1 > slot->target1) {
            goto cur1done;
        }
        slot->cur1 = slot->target1;
        done |= 2;
    cur1done:
        slot->cur2 += slot->delta2;
        if (slot->delta2 >= 0) {
            if ((s16)slot->cur2 < slot->target2) {
                goto cur2done;
            }
        } else if ((s16)slot->cur2 > slot->target2) {
            goto cur2done;
        }
        slot->cur2 = slot->target2;
        done |= 4;
    cur2done:
        slot->cur3 += slot->delta3;
        if (slot->delta3 >= 0) {
            if ((s16)slot->cur3 >= slot->target3) {
                goto cur3clamp;
            }
            goto cur3done;
        } else if ((s16)slot->cur3 > slot->target3) {
            goto cur3done;
        }
    cur3clamp:
        slot->cur3 = slot->target3;
        done |= 8;
    cur3done:
        if (done == 0xF) {
            slot->done++;
        }
        return 0;
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetLightingToModelPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetLightingToPartPkts);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetSplashToPktsBelowLvl);

extern s32 D_800E0200;

/* Build the 30 splash-sprite packet pairs for one field model's render slot:
 * two sprites per part, both 0x2C-coded semi-transparent, sharing the texture
 * page and CLUT, with the part's y offset negated into the second sprite. */
void KawaiInitSplashPkts(void* arg0, s32 arg1) {
    s16 clut;
    s16 tex;
    s32 i;
    u8* pkt;
    u16* parts;
    u8* base;
    s32 count;

    base = (u8*)D_800E0200 + arg1 * 0xAC8;
    tex = 0x6C2C;
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        clut = 0x22B;
    } else {
        clut = 0x9B;
    }
    count = 0x1F;
    parts = (u16*)(*(u32*)((u8*)arg0 + 0x1C) + 4);
    for (i = 1; i < count; i++) {
        pkt = &base[i * 0x5C];
        pkt[0x3] = 9;
        pkt[0x2B] = 9;
        pkt[0x7] = 0x2C;
        pkt[0x2F] = 0x2C;
        pkt[0x2E] = 0x80;
        pkt[0x6] = 0x80;
        pkt[0x2D] = 0x80;
        pkt[0x5] = 0x80;
        pkt[0x2C] = 0x80;
        pkt[0x4] = 0x80;
        *(s16*)(pkt + 0x36) = tex;
        *(s16*)(pkt + 0xE) = tex;
        *(s16*)(pkt + 0x3E) = clut;
        *(s16*)(pkt + 0x16) = clut;
        *(s16*)(pkt + 0x50) = 0;
        *(s16*)(pkt + 0x52) = 0;
        *(s16*)(pkt + 0x54) = 0;
        pkt[0x7] |= 2;
        pkt[0x2F] |= 2;
        *(s16*)(pkt + 0x58) = -*(s16*)parts;
        *(s16*)(pkt + 0x5A) = 0;
        parts += 2;
    }
}

s32 KawaiSetPartAttribute(FieldModelEntry* model, u8* data) {
    u8* parts;
    s32 count;
    s32 i;
    s32 partIdx;

    count = data[0];
    if (count > 0) {
        parts = model->modelData + model->partsOffset;
        for (i = 0; i < count; i++) {
            partIdx = data[i * 2 + 1];
            if (partIdx < model->partCount) {
                parts[partIdx * 32] = data[i * 2 + 2];
            }
        }
    }
    return 1;
}

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiApplyBoneTransform);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiRenderClippedPart);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiDirectionalColorGradient);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiGradientColor);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiAnimatedPointLight);

/////////////////////////////////////////////////
// Begin of field_event.c
/////////////////////////////////////////////////

extern u8 g_FieldMusicLock;
void FieldWindowResetAll(void);
void FieldInitDefaultValues(void);
void FieldEventRunInit(void);

/* Installs the field's state, model and script pointers, checks the script
 * header's version bytes, then brings the event system up. */
void FieldEventInit(
    FieldState* state, FieldEntity* models, FieldScriptHeader* scripts) {
    s32 flags;

    /* The high half of FieldState's 0x68 word. The low half is the
     * controller-1 key bits (see OpcodeFuncKeyEx, which matches against
     * activeKeys as a u32), so this cannot be a named field without splitting
     * that member. Widening to s32 is what makes the load lh rather than lhu:
     * held in an s16 the value is only ever masked, and gcc narrows it. */
    flags = *(s16*)((u8*)state + 0x6A);
    g_FieldState = state;
    g_FieldModels = models;
    g_FieldScripts = scripts;
    D_80095DCC = 0;
    D_8007EBE0 = 1;
    D_8009FE8C = 0;
    if (flags & 0x100) {
        D_80095DCC = 1;
        g_FieldScriptRunState = 4;
    }
    if (scripts->eventDataVersion < 2) {
        SystemError('K', 10);
    }
    if (scripts->eventDataVersion > 2 || scripts->eventVersion > 5) {
        SystemError('K', 12);
    }
    if (scripts->eventVersion < 5) {
        SystemError('K', 11);
    }
    FieldWindowResetAll();
    FieldInitDefaultValues();
    FieldEventRunInit();
    if (g_FieldMusicLock == 0) {
        FieldEventClearAkaoStruct();
        *D_8009A000 = 0xF2;
        SystemAkaoExecute();
    }
}

void InitFieldDebugPages(void);
void FieldEventUpdate(s32 arg0) {
    if (D_8007EBE0) {
        FieldWindowResetTextAll();
        ResetFieldRenderState();
        FieldDebugInitBuffers();
        InitFieldDebugPages();
        D_80095DCC = 0;
        D_8009FE8C = 0;
        D_8007EBE0 = 0;
        if (g_FieldScripts->eventVersion < 5) {
            SystemError('K', 11);
        }
        if (g_FieldScripts->eventDataVersion < 2) {
            SystemError('K', 10);
        }
        if (g_FieldScripts->eventDataVersion > 2 ||
            g_FieldScripts->eventVersion > 5) {
            SystemError('K', 12);
        }
    }
    if (g_FieldScriptRunState != 4) {
        if (g_FieldScriptRunState != 5 || g_FieldDebugStepRequest != 0) {
            FieldEventOpcodeCycle();
        }
    }
    if (g_WindowCount) {
        SystemMenuDrawDialog(
            g_WindowData, 4, arg0, g_FieldState->renderBuffer ^ 1);
    }
    UpdateFieldExitArrows(arg0);
}

extern u8 D_8007078C[];      // per-entity, reset to 0xFF
extern s16 D_800716DC[];     // per-entity
extern s16 D_80071748[][8];  // per-entity, one halfword per script bank
extern u8 D_80071A88[][8];   // per-entity, one byte per script bank
extern s8 D_80075F23;        // top of a 0x100-byte block cleared downward
extern FieldLine D_8007E7AC; // 32 interaction lines
extern u8 D_8007EB98[];      // per-entity, reset to 0xFF
extern u8 D_80081D90[];      // per-entity
extern u8 D_800833F8[][8];   // per-entity, one byte per script bank
extern s16 D_80095D84;
extern u8 D_8009A1C4[]; // per-entity, reset to 7
extern u8 D_8009AD38;   // top of a 9-byte block set to 0xFF downward

/* Zero and default-initialise the whole field runtime state: the entity table,
 * the per-model flags, the script state, and the various counters.
 *
 * 82 rows, and -- the reason this is a good permuter target rather than a
 * codegen problem -- the instruction *count* is exact: 18 insertions against
 * 18 deletions, all of them one-slot shifts. Every remaining row is naming or
 * placement.
 *
 * The three symbols m2c could not infer are settled: D_8009C6E0 is
 * g_FieldState, D_8009C6DC is g_FieldScripts and D_8009C544 is g_FieldModels,
 * all three pointer globals that the target re-reads before nearly every
 * store. Writing them inline, as here, is what reproduces that; a cached local
 * gives one load for the whole function.
 *
 * What took it from unmeasurable to 82:
 *   - Two loop counters for the whole function, not one per loop: the target
 *     puts every *outer* and every sequential loop on one variable ($a3) and
 *     both *inner* loops -- the script-bank walk and the palette clear -- on a
 *     second ($a2). Writing a third counter for the bank loop is 34 rows, all
 *     of them register renaming, and it reads as noise. This is CLAUDE.md's
 *     counter-merging idiom applied across a whole function rather than a run
 *     of adjacent loops.
 *   - D_8009AD38 is `u8`, not `s8`. `*p = 0xFF` through an `s8*` narrows the
 *     constant to QImode and gcc materialises it as `li v1,-1`; the target has
 *     `li v1,0xff`. Same stored byte, different instruction.
 *   - The FieldLine array is reached as byte offsets from D_8007E7AC, which is
 *     what gives the target's `lui at/addiu at/addu at,at,v1` per field. The
 *     interior labels the .s names (D_8007E7BD and friends) have no
 *     definition, so they cannot be declared -- checkfn resolves
 *     `%lo(D_8007E7AC+0x11)` against `%lo(D_8007E7BD)` as an alias, and the
 *     link works only in the offset form.
 *
 * What is left is one phenomenon, in nine places: a group of stores that share
 * one loaded pointer is attached in the target to the *first* statement of the
 * group and here to the *last*. The preamble wants
 * {0x2,0x26,0x32,0x2e,0x2a,0x2c,0x30,0x28} on the pointer loaded for 0x2 and
 * gets it on the one loaded for 0x30; the entity loop wants
 * {0x36,0x66,0xc,0x10,0x14,0x72,0x74} on 0x36's and gets it on 0x38's; the
 * same shift recurs at 0x60, 0x37, and the Kawai fields at 0/2/4. The number
 * of loads is identical either way, which is why the instruction count is
 * exact.
 *
 * Measured and rejected:
 *   - m2c's temps written out as explicit `FieldEntity*` locals, exactly at
 *     the group boundaries the target has: 126 rows. The plain
 *     `g_FieldModels[i].member` spelling is right and the grouping is not
 *     something a local can pin.
 *   - the palette clear as `D_80095DE0[i * 0x20 + j * 2]`, to stop
 *     `check_dbra_loop` reversing the outer counter: 85. As a walked `u8* pal`
 *     with `pal += 0x20`: 83. As `&D_80095DE0[i * 0x20]` hoisted to the inner
 *     preheader (below): 82, and the outer loop is still reversed -- the
 *     target counts up with `slti v0,a3,0x40`, we count down with `bgez`.
 *   - the FieldLine loop indexed `i * 0x18` instead of a separate `off` biv:
 *     146 rows. The scaled subscript folds the symbol into the address
 *     register and the whole `$at` form is lost -- CLAUDE.md's
 *     scaled-subscript rule, seen from the wrong side.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldInitDefaultValues);
#else
void FieldInitDefaultValues(void) {
    s32 i;
    s32 j;
    s8* p;
    u8* q;
    s16* cell;
    s32 off;

    g_FieldState->eventCmd = 0;
    g_FieldState->eventCmdParam = 0;
    g_FieldState->movieCommandState = 0;
    g_FieldState->characterLock = 0;
    g_FieldState->walkAnimId = 1;
    g_FieldState->pcModelId = 0;
    g_FieldState->idleAnimId = 0;
    g_FieldState->runAnimId = 2;
    D_80081DC4 = 0;
    g_FieldState->modelCount = g_FieldScripts->numModels;
    g_FieldState->suspendWalkAndAnim = 0;
    g_FieldState->menuDisabled = 0;
    g_FieldState->unk35 = 0;
    g_FieldState->battlesDisabled = 0;
    g_FieldState->mapJumpDisabled = 0;
    g_FieldState->scrloSet = 0;
    g_FieldState->battleMode1 = 0;
    g_FieldState->nextFieldMusic = 0;
    g_FieldState->nextBattleMusic = 0;
    g_FieldState->unk40 = 0;
    g_FieldState->battleMode2 = 0;
    g_FieldState->encounterTableId = 0;
    g_FieldState->viewOffsetNumSteps = 0;
    g_FieldState->viewOffsetCurrentStep = 0;
    g_FieldState->viewOffsetMode = 0;
    g_FieldState->shakeX.enabled = 0;
    g_FieldState->viewOffsetStart = 0;
    g_FieldState->viewOffsetTarget = 0;
    g_FieldState->shakeY.enabled = 0;
    g_FieldState->shakeX.segmentActive = 0;
    g_FieldState->shakeY.segmentActive = 0;
    g_FieldState->backgroundMovieEnabled = 0;
    g_FieldState->shakeX.amplitude = 0;
    g_FieldState->shakeY.amplitude = 0;
    g_FieldState->shakeX.numStepsPerSegment = 0;
    g_FieldState->shakeY.numStepsPerSegment = 0;
    g_FieldState->shakeX.currentStep = 0;
    g_FieldState->shakeY.currentStep = 0;
    g_FieldState->cameraScrollMode = 0;
    g_FieldState->currentFieldScale = g_FieldScripts->scale;

    p = &D_80075F23;
    i = 0xFF;
    do {
        *p = 0;
        i--;
        p--;
    } while (i >= 0);

    for (i = 0; i < 8; i++) {
        for (j = 0; j < g_FieldScripts->numEntities; j++) {
            D_80071748[j][i] = 0;
            D_800833F8[j][i] = 0;
            D_80071A88[j][i] = 0xFF;
            SavedScriptIds[j][i] = 0;
        }
    }
    for (i = 0; i < g_FieldScripts->numEntities; i++) {
        D_8009A1C4[i] = 7;
        D_8007EB98[i] = 0xFF;
        D_800716DC[i] = 0;
        D_80081D90[i] = 0;
        D_8007078C[i] = 0xFF;
        g_FieldScriptDebugEntities[i] = 0;
    }
    for (i = 0; i < g_FieldScripts->numModels; i++) {
        g_FieldModels[i].MoveDir = 0;
        g_FieldModels[i].charId = 0;
        g_FieldModels[i].PosX = 0;
        g_FieldModels[i].PosY = 0;
        g_FieldModels[i].PosZ = 0;
        g_FieldModels[i].PosI = 0;
        g_FieldModels[i].MoveEndI = 0;
        g_FieldModels[i].Dir = 0;
        g_FieldModels[i].TurnType = 0;
        g_FieldModels[i].TurnSteps = 0;
        g_FieldModels[i].TurnStep = 0;
        g_FieldModels[i].OfsType = 0;
        g_FieldModels[i].TurnStart = 0;
        g_FieldModels[i].TurnEnd = 0;
        g_FieldModels[i].OffsetX = 0;
        g_FieldModels[i].OffsetY = 0;
        g_FieldModels[i].OffsetZ = 0;
        g_FieldModels[i].OffsetStartX = 0;
        g_FieldModels[i].OffsetStartY = 0;
        g_FieldModels[i].OffsetStartZ = 0;
        g_FieldModels[i].OffsetEndX = 0;
        g_FieldModels[i].OffsetEndY = 0;
        g_FieldModels[i].OffsetEndZ = 0;
        g_FieldModels[i].OffsetSteps = 0;
        g_FieldModels[i].OffsetStep = 0;
        g_FieldModels[i].activeAnimId = 0;
        g_FieldModels[i].animSpeed = 0x10;
        g_FieldModels[i].visible = 0;
        g_FieldModels[i].MoveEndX = 0;
        g_FieldModels[i].MoveEndY = 0;
        g_FieldModels[i].MoveEndZ = 0;
        g_FieldModels[i].animCurrentFrame = 0;
        g_FieldModels[i].animLastFrame = 0;
        g_FieldModels[i].scriptedMoveMode = 0;
        g_FieldModels[i].MoveSpeed = g_FieldState->currentFieldScale * 2;
        g_FieldModels[i].requestTalkScript = 0;
        g_FieldModels[i].ActionArg = 0;
        g_FieldModels[i].ActionState = 0;
        g_FieldModels[i].requestPushScript = 0;
        g_FieldModels[i].SolidOff = 0;
        g_FieldModels[i].TalkOff = 0;
        g_FieldModels[i].DirLock = 0;
        g_FieldModels[i].SolidRange =
            g_FieldState->currentFieldScale * 0x1E / 512;
        g_FieldModels[i].TalkRange =
            g_FieldState->currentFieldScale * 0x50 / 512;
        D_8008325C[i] = 0;
        D_800756E8[i] = 0;
        D_8009D828[i] = 0x10;
        D_80082248[i] = 0x10;
        g_FieldModels[i].BlinkOn = 0;
        g_FieldModels[i].KawaiOp1 = 0;
        g_FieldModels[i].KawaiOp0 = 0;
        g_FieldModels[i].KawaiDataOffset = 0;
        g_FieldModels[i].KawaiA = 0;
    }
    i = 0;
    do {
        g_FieldState->backgroundLayerVisibility[i] = 0;
        i++;
    } while (i < 0x40);
    i = 0;
    do {
        g_FieldState->blockedAccesses[i] = 0;
        i++;
    } while (i < 0x40);
    i = 0;
    do {
        j = 0xF;
        cell = (s16*)(&D_80095DE0[i * 0x20] + 0x1E);
        do {
            *cell = 0;
            j--;
            cell--;
        } while (j >= 0);
        i++;
    } while (i < 0x40);
    i = 0;
    off = 0;
    do {
        *((u8*)&D_8007E7AC + 0x11 + off) = 0;
        *((u8*)&D_8007E7AC + 0x10 + off) = 0;
        *((u8*)&D_8007E7AC + 0xF + off) = 0;
        *((u8*)&D_8007E7AC + 0xE + off) = 0;
        *((u8*)&D_8007E7AC + 0x12 + off) = 0;
        *((u8*)&D_8007E7AC + 0x13 + off) = 0;
        *((u8*)&D_8007E7AC + 0xC + off) = 0;
        *((u8*)&D_8007E7AC + 0xD + off) = 0;
        *((u8*)&D_8007E7AC + 0x16 + off) = 0;
        *(s16*)((u8*)&D_8007E7AC + off) = 0;
        *(s16*)((u8*)&D_8007E7AC + 2 + off) = 0;
        *(s16*)((u8*)&D_8007E7AC + 4 + off) = 0;
        *(s16*)((u8*)&D_8007E7AC + 6 + off) = 0;
        *(s16*)((u8*)&D_8007E7AC + 8 + off) = 0;
        *(s16*)((u8*)&D_8007E7AC + 0xA + off) = 0;
        i++;
        off += 0x18;
    } while (i < 0x20);
    D_80095D84 = 0;
    i = 8;
    q = &D_8009AD38;
    do {
        *q = 0xFF;
        i--;
        q--;
    } while (i >= 0);
    g_EntityForSplitJoin = 0xFF;
    g_FieldMovieOpcodeActive = 0;
    Savemap.memory_bank_1[31] |= 3;
}
#endif

/* Walks every entity's first script and runs its initialisation opcodes
 * (everything up to the terminating 0). The script-offset table sits past the
 * entity-name table and the extras table in the script header.
 *
 * PARKED at 16 rows, down from 66, and three of the four causes are worth
 * carrying to the other script walkers:
 *
 *   * The inner opcode loop keeps &g_FieldScriptPC in its *own* callee-saved
 *     register, separate from the one the outer loop uses. `loop_optimize'
 *     runs innermost-loop-first, so the inner loop's copy is hoisted into the
 *     inner preheader before the outer's exists. A `u16* pcTable' assigned in
 *     front of the `do' is what makes gcc keep one live across the opcode
 *     call at all (36 rows) -- but it is assigned in the preheader, so cse
 *     copies the outer register into it (`move s0,s2') where the target
 *     rebuilds the address. Writing the access inline instead, in either the
 *     byte-offset or the subscript form, loses the register entirely and is
 *     30 rows worse.
 *   * `g_FieldScripts' has to be a local, and it must *not* be live across the
 *     opcode call. Read inline it is reloaded three times inside one
 *     statement group; cached at the top of the outer body and used in the
 *     inner loop too it takes a callee-saved register, which the target does
 *     not have. Assigned after the debug block and used only up to the inner
 *     loop, it lands in $a2 like the target's $a0.
 *   * The header address is built base-last: `scriptBase + n*8 + numExtras +
 *     (s32)scripts', not `(u8*)scripts + scriptBase + ...'. Pointer PLUS puts
 *     the pointer first; integer PLUS keeps source order. Same for the debug
 *     name, which is `(u8*)g_FieldScripts + 0x20 + entity*8'. Together those
 *     were 15 rows.
 *
 * What is left is the `move s0,s2' above. It is worth more than it looks:
 * `lui'+`addiu' is two insns and the copy is one, so every branch target after
 * it is 4 bytes short and three of the sixteen rows are that shift alone; the
 * rest is register renaming. The one other real row is the pair of shifts at
 * the top of the outer body -- the target issues `sll a3,v1,1' (the PC slot)
 * before `sll v1,v1,6' (the script base) and this build issues them the other
 * way round, which is scheduling, not statement order: moving the `slot'
 * assignment above `scriptBase' measures 25.
 *
 * Both address sums are already right, and the diff hides it -- their three
 * `addu's differ only in register names, and the operand order (base, then
 * entity count, then scripts, then extras for the second one) is what the
 * body below already writes. Swapping the last two addends is byte-identical,
 * so fold canonicalises the chain and the source cannot reach it anyway.
 *
 * Rejected for the copy, all measured: assigning `pcTable' inside the inner
 * loop rather than in its preheader (58), writing the inner access inline in
 * the subscript form (46) or the byte-offset form (46), spelling the
 * assignment `(u16*)((u8*)g_FieldScriptPC + 0)' (no change, folded), and
 * writing the outer `slot' as `&g_FieldScriptPC[g_CurrentEntity]' (no
 * change). What the target has is the inner loop's own hoisted movable, which
 * only exists if `loop_optimize' finds the symbol inside the inner body --
 * and every spelling that puts it there folds `%hi'/`%lo' into the address
 * instead, which loses the register altogether.
 *
 * Also rejected, and it is the one spelling the list above was missing: the
 * *outer* accesses reached through the neighbouring symbol, `(u16*)((entity *
 * 2) + (u8*)g_WindowToEntity - 0x70)`, so that cse has no shared `symbol_ref`
 * to relate the inner reference to and `loop_optimize` is forced to give the
 * inner loop its own movable. Both outer accesses 59/3, the post-loop one
 * alone 59/3, the pre-loop one alone 26/1 -- against 16/0. The neighbouring-
 * object idiom is real (see `FieldLoadMimToVram`) and it is the wrong tool
 * here: it does not stop the copy, it just costs the outer loop its own base.
 *
 * The length is already exact -- 0 inserted, 0 deleted at 152 instructions --
 * so this is a clean permuter target and score 0 is reachable.
 *
 * 16 rows -> 15, and the step was decomp-permuter's: a `u8* pcBase =
 * (u8*)g_FieldScriptPC;` assigned as the **first statement of the do-body**,
 * above the debug block, and used for the *first* `slot` only. Routing `slot2`
 * through it as well gives the row straight back (16), so the asymmetry is the
 * find, not tidiable noise -- the same shape as `FieldArrowsAddToRender`'s
 * five-of-seven index sites.
 *
 * That run is also the sharpest instance yet of the warning in CLAUDE.md step
 * 4 about the permuter's score: it went from a base of 395 to 150 -- a 62%
 * drop, the best of **26** improvements over **221,676** candidates, run to a
 * wall-clock limit rather than to exhaustion -- for exactly **one** row in the
 * real build. The other 25 outputs are between 160 and 390 and none of them is
 * worth opening. Do not read a score that size as progress; re-measure every
 * output with variant_eval. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventRunInit);
#else
void FieldEventRunInit(void) {
    s16 numExtras;
    s32 scriptBase;
    u8* pcBase;
    u16 pc;
    u16* slot;
    u16* slot2;
    u8 lo;
    u8 op;
    u8 op2;
    u16* pcTable;
    FieldScriptHeader* scripts;

    g_FieldModelCount = 0;
    g_CurrentEntity = 0;
    if (g_FieldScripts->numEntities != 0) {
        do {
            pcBase = (u8*)g_FieldScriptPC;
            if (g_FieldScriptDebugFlags & 3) {
                FieldDebugStringCopy(g_DebugText, &D_800E0628);
                FieldDebugStringConcat(g_DebugText, (u8*)g_FieldScripts + 0x20 +
                                                        (g_CurrentEntity * 8));
                if (g_FieldScriptDebugFlags & 1) {
                    SetStrToDebugRow(4, 0, g_DebugText);
                }
                if (g_FieldScriptDebugFlags & 2) {
                    DebugPrintToFieldWindow(g_DebugText);
                }
            }
            scripts = g_FieldScripts;
            scriptBase = g_CurrentEntity << 6;
            numExtras = scripts->numExtras * 4;
            lo = ((u8*)(scriptBase + (scripts->numEntities * 8) + numExtras +
                        (s32)scripts))[0x20];
            slot = (u16*)((g_CurrentEntity * 2) + pcBase);
            *slot = (u16)lo;
            *slot = lo | (((u8*)(scriptBase + (scripts->numEntities * 8) +
                                 numExtras + (s32)scripts))[0x21]
                          << 8);
            op = *((u8*)scripts + *slot);
            g_FieldCurrentOpcode = op;
            if (op != 0) {
                pcTable = g_FieldScriptPC;
                do {
                    g_FieldOpcodes[g_FieldCurrentOpcode]();
                    op2 = *((u8*)g_FieldScripts + pcTable[g_CurrentEntity]);
                    g_FieldCurrentOpcode = op2;
                } while (op2 != 0);
            }
            slot2 = (u16*)((g_CurrentEntity * 2) + (u8*)g_FieldScriptPC);
            pc = *slot2;
            g_CurrentEntity += 1;
            *slot2 = pc + 1;
        } while ((u8)g_CurrentEntity < (u8)g_FieldScripts->numEntities);
        g_CurrentEntity = 0;
    }
}
#endif

/* Enable the loaded field models that correspond to party members, then
 * disable (make non-solid, non-talkable, invisible) every model whose loader
 * slot was not claimed.
 *
 * Four things, and only the last is codegen. The second loop's bound is read
 * from memory on every iteration -- caching it in a `modelCount' local and
 * guarding the loop with `if (modelCount != 0)' is 35 rows, because the
 * hand-written guard is not the same code as the `for's own zero-trip test.
 * The inner loop walks *entities*, so its bound is `numEntities' (offset 2),
 * not `numModels' (offset 3); the two are adjacent `u8's and the whole
 * function still diffs to two rows with the wrong one. And the redundant
 * `if (i < numModels)' wrapper around it is not in the original -- the target
 * reaches the inner loop through its own zero-trip guard, which it spells
 * `slt v0,v1,v0' against the npcFlag register it has just proved to be zero.
 *
 * The codegen one: `g_EntityToModel[modelId]' has to go through an `s32'
 * local. Read inline it is re-loaded for the second test (three rows) and the
 * comparison against the u16 `count' comes out `sltu' -- gcc 2.6.3 promotes
 * unsigned short to *unsigned* int, so one unsigned operand makes the whole
 * comparison unsigned. The `(s32)' cast on `count' is what puts `slt' back. */
void FieldEnablePartyModels(void) {
    s16 i;
    s16 j;
    u8 charId;
    u8 modelId;
    s32 entityModel;

    /* Mark the loader slot of each present party member's model as an NPC. */
    for (i = 0; i < 3; i++) {
        charId = Savemap.memory_bank_2[9 + i];
        if (charId == 0xFF) {
            continue;
        }
        modelId = g_CharIdToEntity[charId];
        if (modelId == 0xFF) {
            continue;
        }
        entityModel = g_EntityToModel[modelId];
        if (entityModel == 0xFF) {
            continue;
        }
        if (entityModel < (s32)((FieldModelFileDesc*)D_8007E770)->count) {
            g_FieldModelLoaderData[entityModel].npcFlag = 1;
        }
    }

    /* Disable every model whose loader slot was not claimed above. */
    for (i = 0; i < ((FieldModelFileDesc*)D_8007E770)->count; i++) {
        if (g_FieldModelLoaderData[i].npcFlag == 0) {
            for (j = 0; j < g_FieldScripts->numEntities; j++) {
                if (g_EntityToModel[j] == i) {
                    g_EntityToModel[j] = 0xFF;
                    g_FieldModels[i].visible = 0;
                    g_FieldModels[i].SolidOff = 1;
                    g_FieldModels[i].TalkOff = 1;
                }
            }
        }
    }
}

// Inline as empty string when more is decompiled. Checksum fails now.
const char D_800A013C[8] = {0};

void FieldEventOpcodeCycle(void) {
    s32 i, j, count;
    u16 hours, seconds;
    s32 talkDone = 0;

    // Update display values for play time and countdown.
    hours = Savemap.time / 3600;
    if (hours > 255) {
        hours = 255;
    }
    Savemap.memory_bank_1[16] = hours;
    hours = Savemap.time % 3600;
    Savemap.memory_bank_1[17] = hours / 60;
    seconds = hours % 60;
    if (Savemap.memory_bank_1[18] != seconds) {
        Savemap.memory_bank_1[18] = seconds;
        Savemap.memory_bank_1[19] = 0;
    } else {
        Savemap.memory_bank_1[19]++;
    }

    hours = Savemap.countdown_timer_seconds / 3600;
    if (hours > 255) {
        hours = 255;
    }
    Savemap.memory_bank_1[20] = hours;
    hours = Savemap.countdown_timer_seconds % 3600;
    Savemap.memory_bank_1[21] = hours / 60;
    seconds = hours % 60;
    if (Savemap.memory_bank_1[22] != seconds) {
        Savemap.memory_bank_1[22] = seconds;
        Savemap.memory_bank_1[23] = 30;
    } else if (Savemap.memory_bank_1[23]) {
        Savemap.memory_bank_1[23]--;
    }

    count = g_FieldScripts->numModels;
    for (i = 0; i < count; i++) {
        if (g_FieldModels[i].requestTalkScript) {
            if (!g_FieldState->characterLock && !talkDone) {
                FieldEventRequestRun(g_FieldModels[i].entityId, 1, 1);
                talkDone = 1;
            }
            g_FieldModels[i].requestTalkScript = 0;
        }
        if (g_FieldModels[i].requestPushScript) {
            FieldEventRequestRun(g_FieldModels[i].entityId, 1, 2);
            g_FieldModels[i].requestPushScript = 0;
        }
    }
    for (i = 0; i < g_FieldLineCount; i++) {
        if (g_FieldLines[i].requestTalkScript) {
            if (!g_FieldState->characterLock) {
                FieldEventRequestRun(g_FieldLines[i].entityId, 1, 1);
            }
            g_FieldLines[i].requestTalkScript = 0;
        }
        if (g_FieldLines[i].requestPushScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 2);
            g_FieldLines[i].requestPushScript = 0;
        }
        if (g_FieldLines[i].across) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 3);
            g_FieldLines[i].across = 0;
        }
        if (g_FieldLines[i].requestTouchOnScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 5);
            g_FieldLines[i].requestTouchOnScript = 0;
        }
        if (g_FieldLines[i].requestTouchOffScript) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 6);
            g_FieldLines[i].requestTouchOffScript = 0;
        }
        if (g_FieldLines[i].touch) {
            FieldEventRequestRun(g_FieldLines[i].entityId, 1, 4);
        }
    }

    // Loop through all entities in field map and execute up to 8 opcodes of
    // each entity's active script.
    count = g_FieldScripts->numEntities;
    do {
        if (g_CurrentEntity >= g_FieldScripts->numEntities) {
            g_CurrentEntity = 0;
        }
        if (g_FieldScriptDebugFlags & 3) {
            DebugUpdateActor(4, g_CurrentEntity);
        }

        // Skip entities involved in a split or join animation
        // (g_EntitySplitJoinState[entity] != 0) except the entity they're
        // splitting from or joining to (g_EntityForSplitJoin).
        if (g_EntitySplitJoinState[g_CurrentEntity] == 0 ||
            g_EntityForSplitJoin == g_CurrentEntity) {
            for (j = 8; j != 0; j--) {
                if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                    (!(g_FieldScriptDebugFlags & 4) ||
                     g_FieldScriptDebugEntities[g_CurrentEntity])) {
                    for (i = 1; i < 9; i++) {
                        SetStrToDebugRow(3, i, D_800A013C);
                    }
                }
                g_FieldCurrentOpcode =
                    ((u8*)g_FieldScripts)[g_FieldScriptPC[g_CurrentEntity]];

                // Script can yield early if opcode returns 1.
                if (g_FieldOpcodes[g_FieldCurrentOpcode]()) {
                    if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                        (!(g_FieldScriptDebugFlags & 4) ||
                         g_FieldScriptDebugEntities[g_CurrentEntity])) {
                        g_CurrentEntity++;
                        goto done;
                    }
                    break;
                }
                if (g_FieldScriptRunState == 5 && g_DebugLevel & 1 &&
                    (!(g_FieldScriptDebugFlags & 4) ||
                     g_FieldScriptDebugEntities[g_CurrentEntity])) {
                    if (++g_FieldDebugStepCounter >= 8) {
                        g_FieldDebugStepCounter = 0;
                        g_CurrentEntity++;
                    }
                    goto done;
                }
            }
        }
        g_CurrentEntity++;
        count--;
        if (g_FieldScriptRunState == 5 && g_FieldScriptDebugFlags & 1 &&
            (!(g_FieldScriptDebugFlags & 4) ||
             g_FieldScriptDebugEntities[g_CurrentEntity])) {
            break;
        }
    } while (count != 0);

done:
    if (g_FieldScriptRunState == 5) {
        g_FieldDebugStepRequest = 0;
    }
    FieldUpdateAnimationState();
}

/* Per-frame animation state machine for every loaded model: skip the player's
 * own model unless the character is locked, then dispatch on the model's
 * animation state and wrap, hold or transition its frame counter. State 2
 * falls through into 3/4 once the animation has run out.
 *
 * Five typing and shape decisions carry this one, and the first two are the
 * lever the four parked g_FieldModelData callers were stuck on:
 *
 *   - `entryIdx` is a *separate `s32` local*. Written inline the whole lookup
 *     is byte-identical to the matching StartModelAnimation and still comes out
 *     with `entryIdx` and `g_FieldModelData` holding each other's register; as
 *     a `u8` local it is inert. Only the split plus the widening moves it.
 *   - `modelIdx` is `s32`, not the `u8` the array element is. Same lever as
 *     OpcodeFuncCanim's divisor: the QImode pseudo loses allocno_compare to the
 *     cse-made ones and every argument register rotates by one.
 *   - the frame test is `(s32)((u16)x << 16) >> 20`, not `x >> 4`. Read through
 *     the plainly-signed member gcc folds the pair of shifts into one `sra 4`
 *     and the load stays `lh`; the `u16` view leaves the `lhu` and the
 *     `sll`/`sra` pair the target has. animCurrentFrame is a 12.4 fixed-point
 *     counter, so the two spellings are the same value.
 *   - the `- 1` on the animation's frame count goes through an `s32` local.
 *     Inline, combine narrows the addend against the `s16` store, -1 becomes
 *     0xFFFF, which is not a legal `addiu` immediate, so gcc materialises it
 *     with `li` and loop.c then hoists that out of the loop.
 *   - every arm repeats `g_FieldModels[g_EntityToModel[i]]` rather than sharing
 *     one pointer local, and the state-0 arm's "animation changed" branch
 *     repeats `g_EntityToModel[i]` rather than reusing the `modelIdx` it
 *     already read. One local per use site is 36 rows; reusing modelIdx across
 *     the branch is the last a0/a1 swap. */
void FieldUpdateAnimationState(void) {
    s32 i;
    s32 modelIdx;
    u8* anims;
    s32 lastFrame;
    FieldEntity* entity;
    FieldEntity* wrap;
    s32 entryIdx;
    FieldModelEntry* model;

    for (i = 0; i < g_FieldScripts->numEntities; i++) {
        if (g_EntityToModel[i] != 0xFF &&
            (g_FieldState->pcModelId != g_EntityToModel[i] ||
             g_FieldState->characterLock != 0)) {
            switch (D_800756E8[g_EntityToModel[i]]) {
            case 0:
                modelIdx = g_EntityToModel[i];
                entity = &g_FieldModels[modelIdx];
                if (entity->activeAnimId != D_8008325C[modelIdx]) {
                    entity->activeAnimId = D_8008325C[modelIdx];
                    g_FieldModels[g_EntityToModel[i]].animSpeed =
                        D_80082248[g_EntityToModel[i]];
                    g_FieldModels[g_EntityToModel[i]].animCurrentFrame = 0;
                    entryIdx = g_FieldModelLoaderData[g_EntityToModel[i]]
                                   .modelEntryIndex;
                    model = &g_FieldModelData->modelEntries[entryIdx];
                    anims = model->modelData + model->animationOffset;
                    lastFrame = *(u16*)&anims[g_FieldEntity[g_EntityToModel[i]]
                                                  .activeAnimId *
                                              16] -
                                1;
                    g_FieldModels[g_EntityToModel[i]].animLastFrame = lastFrame;
                } else {
                    entryIdx = g_FieldModelLoaderData[modelIdx].modelEntryIndex;
                    model = &g_FieldModelData->modelEntries[entryIdx];
                    anims = model->modelData + model->animationOffset;
                    lastFrame =
                        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId *
                                      16] -
                        1;
                    entity->animLastFrame = lastFrame;
                    wrap = &g_FieldModels[g_EntityToModel[i]];
                    if (((s32)((u16)wrap->animCurrentFrame << 16) >> 20) >=
                        wrap->animLastFrame) {
                        wrap->animCurrentFrame = 0;
                    }
                }
                break;
            case 1:
                wrap = &g_FieldModels[g_EntityToModel[i]];
                if (((s32)((u16)wrap->animCurrentFrame << 16) >> 20) >=
                    wrap->animLastFrame) {
                    wrap->animCurrentFrame = 0;
                }
                break;
            case 2:
                if (((s32)((u16)g_FieldModels[g_EntityToModel[i]]
                               .animCurrentFrame
                           << 16) >>
                     20) < g_FieldModels[g_EntityToModel[i]].animLastFrame) {
                    break;
                }
                D_800756E8[g_EntityToModel[i]] = 4;
                /* fallthrough */
            case 3:
            case 4:
                g_FieldModels[g_EntityToModel[i]].animCurrentFrame =
                    (u16)g_FieldModels[g_EntityToModel[i]].animLastFrame * 16;
                break;
            case 5:
                if (((s32)((u16)g_FieldModels[g_EntityToModel[i]]
                               .animCurrentFrame
                           << 16) >>
                     20) >= g_FieldModels[g_EntityToModel[i]].animLastFrame) {
                    D_800756E8[g_EntityToModel[i]] = 0;
                }
                break;
            case 6:
                if (((s32)((u16)g_FieldModels[g_EntityToModel[i]]
                               .animCurrentFrame
                           << 16) >>
                     20) >= g_FieldModels[g_EntityToModel[i]].animLastFrame) {
                    D_800756E8[g_EntityToModel[i]] = 3;
                }
                break;
            }
        }
    }
}

u8 FieldEventRequestRun(s16 entityId, s16 priority, s16 scriptId) {
    u16 offset;
    s32 scriptOffset;
    s32 entityDataSize;
    s32 extrasHeaderSize;

    if (g_DebugLevel & 3) {
        switch (scriptId) {
        case 1: // Pressed OK.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Talk=");
            break;
        case 2: // Pushed / within entity's collision radius.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Push=");
            break;
        case 3: // Across line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Acrs=");
            break;
        case 4: // Touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "Toch=");
            break;
        case 5: // Started touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "TochON =");
            break;
        case 6: // Ended touching line.
            FieldDebugStringCopy(g_DebugMessageBuffer, "TochOFF=");
            break;
        }
        // Prints entity name.
        FieldDebugStringConcat(
            g_DebugMessageBuffer,
            (char*)g_FieldScripts + sizeof(FieldScriptHeader) + entityId * 8);
        FieldDebugAddParseValueToPage2(g_DebugMessageBuffer, 0, 0);
    }

    // Only request script if active script has lower priority.
    if (g_FieldScriptPriority[entityId] > priority) {

        // Entity is busy waiting for another script to return.
        if (g_FieldScriptSyncState[entityId][priority] != SYNC_NONE) {
            return g_FieldScriptSyncState[entityId][priority];
        }

        scriptOffset = scriptId * 2;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);
        entityDataSize = entityId * 64;
        entityDataSize += g_FieldScripts->numEntities * 8;

        offset = *((u8*)(scriptOffset + entityDataSize + extrasHeaderSize +
                         (s32)g_FieldScripts) +
                   sizeof(FieldScriptHeader));
        offset |=
            *((u8*)(scriptOffset + (entityDataSize + (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        // Empty event scripts consist of just a RET (0x00) opcode.
        if (((u8*)g_FieldScripts)[offset] != 0) {

            // Save position of current active script of lower priority and
            // replace with new script.
            SavedScriptIds[entityId][priority] = scriptId;
            g_SavedFieldScriptPC[entityId][g_FieldScriptPriority[entityId]] =
                g_FieldScriptPC[entityId];
            g_FieldScriptPC[entityId] = offset;
            g_FieldScriptPriority[entityId] = priority;

            // Clear running animation if entity has a model.
            if (g_EntityToModel[entityId] != 0xFF) {
                if (g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode ==
                    SMODE_WALK) {
                    g_FieldModels[g_EntityToModel[entityId]].activeAnimId = 0;
                    g_FieldModels[g_EntityToModel[entityId]].animCurrentFrame =
                        0;
                    g_FieldModels[g_EntityToModel[entityId]].animLastFrame = 0;
                }
                g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode =
                    SMODE_NONE;
            }

            // Reset wait counter.
            g_FieldWaitCounter[entityId] = 0;

            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("=recieved", 0, 0);
            }
        } else {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("=ret", 0, 0);
            }
        }
        return 1;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("=ignored", 0, 0);
    }
    return 0;
}

void ResetFieldRenderState(void) {
    s16 tpage;

    g_FieldExitArrowPktIdx = 0;
    g_FieldExitArrowX = 0x7FFF;
    g_FieldExitArrowY = 0x7FFF;
    setPolyFT4(&g_FieldExitArrowPkts[0]);
    setPolyFT4(&g_FieldExitArrowPkts[1]);
    setSemiTrans(&g_FieldExitArrowPkts[0], 0);
    setSemiTrans(&g_FieldExitArrowPkts[1], 0);
    setShadeTex(&g_FieldExitArrowPkts[0], 1);
    setShadeTex(&g_FieldExitArrowPkts[1], 1);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x2F;
    } else {
        tpage = 0x1F;
    }
    g_FieldExitArrowPkts[1].tpage = tpage;
    g_FieldExitArrowPkts[0].tpage = tpage;
    g_FieldExitArrowPkts[1].clut = 0x7850;
    g_FieldExitArrowPkts[0].clut = 0x7850;
    g_FieldExitArrowPkts[0].r0 = 0;
    g_FieldExitArrowPkts[1].r0 = 0;
    g_FieldExitArrowPkts[0].g0 = 0;
    g_FieldExitArrowPkts[1].g0 = 0;
    g_FieldExitArrowPkts[0].b0 = 0;
    g_FieldExitArrowPkts[1].b0 = 0;
}

/* Unprototyped on purpose: the original passes nothing, but arg0 has to stay
 * live across the call for the cached &g_FieldExitArrowState to land in $a1. */
void DrawFieldExitArrow();

/* Select toggles the exit arrows on and off (bit 0); bit 1 is a debug override
 * that shows them regardless of the toggle and of the movement lock. */
void UpdateFieldExitArrows(s32 arg0) {
    if (g_FieldState->newActiveKeys2 & (1 << 8)) {
        g_FieldExitArrowState[0] ^= 1;
    }
    if (((g_FieldExitArrowState[0] == 1) &&
         (g_FieldState->characterLock == 0)) ||
        (g_FieldExitArrowState[0] & 2)) {
        DrawFieldExitArrow(arg0);
    }
}

/* Draw the field-exit arrow: a single textured quad at the exit's projected
 * screen position, double-buffered through g_FieldExitArrowPktIdx so the packet
 * being added is never the one the GPU is reading. The clamp keeps it on
 * screen, and the two `if`s pick which corner of the 16x16 texture cell maps to
 * which vertex -- the arrow flips horizontally past x = 0x123 and vertically
 * above y = 0x11, so it always points inward from the edge it sits on.
 *
 * 255 rows and 5 insertions -> MATCH, from an m2c seed that addressed every
 * packet field as `*(&D_800E4900 + slot * 0x28)`. Three things did it:
 *   - the sixteen `D_800E49xx` placeholders are POLY_FT4 members of
 *     `g_FieldExitArrowPkts[]`: +8/+0xA x0,y0, +0xC/+0xD u0,v0, +0x10/+0x12
 * x1,y1, +0x14/+0x15 u1,v1, and so on. The externs are gone.
 *   - every access is the full
 * `g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].<field>` expression. Taking the
 * obvious `POLY_FT4* arrow = &g_FieldExitArrowPkts[g_FieldExitArrowPktIdx];`
 * once and writing through it measures **300** rows -- worse than the m2c seed
 * -- because the packet base then lives in one register across the whole
 *     function where the target rebuilds it at every store. Same house rule as
 *     the opcode handlers' `g_EntityToModel[g_CurrentEntity]`, and it is worth
 *     286 rows here. The `addPrim` argument is the one place the index appears
 *     without a member, and it is written inline, not through a local: a local
 *     costs 4 rows because it makes gcc evaluate the index before the array
 *     base.
 *   - the y clamp is an `if`/`else`, the x clamp is not. `y =
 * g_FieldExitArrowY; if (g_FieldExitArrowY >= 0xE1) y = 0xE0;` loads the global
 * twice -- an `lhu` for the s16 local and an `lh` for the comparison -- while
 *     `if (g_FieldExitArrowY >= 0xE1) { y = 0xE0; } else { y =
 * g_FieldExitArrowY; }` assigns the local *after* the comparison has already
 * loaded it, so cse rewrites the second load as `move a3,v1` and reorg puts the
 * copy in a delay slot. That is the documented three-spellings rule for an s16
 * local, and the asymmetry is real: the x clamp wants the plain form and
 * measures worse as an if/else. */
void DrawFieldExitArrow(s32* ot) {
    s16 x;
    s16 y;

    if (g_FieldMovieOpcodeActive == 0 &&
        (g_FieldExitArrowX != 0x7FFF ||
         g_FieldExitArrowY != g_FieldExitArrowX)) {
        x = g_FieldExitArrowX;
        if (g_FieldExitArrowX >= 0x141) {
            x = 0x140;
        }
        if (g_FieldExitArrowX < 0) {
            x = 0;
        }
        if (g_FieldExitArrowY >= 0xE1) {
            y = 0xE0;
        } else {
            y = g_FieldExitArrowY;
        }
        if (g_FieldExitArrowY < 0) {
            y = 0;
        }
        g_FieldExitArrowPktIdx ^= 1;
        if (x >= 0x123) {
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u0 = 0x8F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u1 = 0x7F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u2 = 0x8F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u3 = 0x7F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x0 = x - 0x10;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x1 = x;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x2 = x - 0x10;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x3 = x;
        } else {
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u0 = 0x80;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u1 = 0x90;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u2 = 0x80;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].u3 = 0x90;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x0 = x;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x1 = x + 0x10;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x2 = x;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].x3 = x + 0x10;
        }
        if (y < 0x11) {
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v0 = 0x6F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v1 = 0x6F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v2 = 0x5F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v3 = 0x5F;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y0 = y;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y1 = y;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y2 = y + 0x10;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y3 = y + 0x10;
        } else {
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v0 = 0x60;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v1 = 0x60;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v2 = 0x70;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].v3 = 0x70;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y0 = y - 0x10;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y1 = y - 0x10;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y2 = y;
            g_FieldExitArrowPkts[g_FieldExitArrowPktIdx].y3 = y;
        }
        addPrim(ot, &g_FieldExitArrowPkts[g_FieldExitArrowPktIdx]);
    }
}

/////////////////////////////////////////////////
// Begin of field_event_debug.c
/////////////////////////////////////////////////

void FieldDebugPageSetColor(s16 page, u8 r, u8 g, u8 b);
s32 SetDebugStrRowColor(s16 page, s16 row, u8 color);
extern u8 D_800716C8;
extern s16 D_80071E38;
extern s16 D_80071E3C;
extern u16 D_80075E12;
extern s16 D_8009AC1E;
extern u8 D_8009CBDD;
extern u8 D_8009CBDE;
extern u8 D_8009D289;
extern u8 D_8009D29B;
extern u8 D_8009D392;
extern u8 D_8009D393;
extern u16 D_8009D78A;
extern s16* D_800E4274;
extern char D_800A02B8[];

/* The per-actor debug readout: twenty rows of entity, model, line, walkmesh
 * and memory state, built up in g_DebugText and pushed to a debug page or the
 * field window.
 *
 * 148 rows / 4 insertions at 2104 instructions against 2102, from a raw m2c
 * seed that did not compile.  What the rewrite established, in the order it
 * was worth instructions:
 *
 *   - **Every if/else that feeds a call has the call written out in both
 *     arms.**  The target's arms each carry the *whole* argument setup --
 *     `lui a0,%hi(g_DebugText)/addiu a0/lui a1,%hi(str)/addiu a1/j` -- and
 *     cross-jumping merges only the `jal` and its delay slot.  Hoisting the
 *     call after the if/else, which is what m2c writes and what reads better,
 *     emits the setup once and is 11 instructions short per site.  There are
 *     36 such arms (the eleven `D_8009D78A` flag characters, the three
 *     transparency markers, the two field-state markers) and they were worth
 *     119 instructions.  The colour ladders are the same lever: each arm of
 *     `SetDebugStrRowColor(arg0, 0x12, N)`'s seven-way chain re-does the
 *     `(s16)arg0` sign-extension and materialises its own constant.
 *   - **The " Tg=" dispatch is a `switch`, not m2c's nest of ifs.**
 *     `ori v0,1/beq` then `slti v0,v1,2/beqz` then `beqz v1` is
 *     `expand_end_case`'s compare chain, and the arms are laid out 0, 1, 2,
 *     default in address order, which is the order they were written.  The
 *     default re-reads `SavedScriptIds[actorId][D_8009A1C4[actorId]]` because
 *     its block has two predecessors and cse knows nothing on entry -- 14
 *     instructions.
 *   - **Both parameters are `s16`.**  The prologue copies each argument
 *     register with a plain `addu` and re-does `sll 16/sra 16` at every use;
 *     a `u8` parameter would mask on entry.  m2c's `u8 actorId` cost 8
 *     instructions.  Measured: (s16,s16) 256 rows, (s32,s16) 259, (s32,s32)
 *     261, (s16,u8) 278, (s32,u8) 281.
 *   - **The FieldLine fields want a pre-scaled byte offset in a local,
 *     re-derived at every access.**  `*(s16*)((u8*)&D_8007E7AC + 0x4 +
 *     D_8007078C[actorId] * 0x18)` hands gcc a MULT, `associate` lifts the
 *     bare symbol out of the sum, and one `lui/addiu` of D_8007E7AC then
 *     serves all six fields from a callee-saved register.  Assigning
 *     `lineOff = D_8007078C[actorId] * 0x18;` immediately above each access
 *     gives `(plus (symbol) (reg))`, which stays in the mem and comes out as
 *     the assembler's `lui at/addiu at/addu at` per field -- which is what
 *     the target has, and what the already-matching FieldInitDefaultValues in
 *     this unit uses.  35 rows.  Hoisting one `lineOff` for the whole block
 *     instead deletes the re-derivations and measures -42 instructions.
 *     Spelling the interior address as `&D_8007E7AC.pos.z1` is exactly inert.
 *   - **`&D_8009AC1E` is a named `s16*` local, assigned at the top of the
 *     block that uses it.**  The target sets `$s3` once and reads `lh 0(s3)`
 *     nine times; direct reads rematerialise `lui %hi` at each one, eight
 *     instructions.  `volatile` also reaches the register form and is wrong
 *     here -- it turns every `lh` into `lhu`/`sll`/`sra` and measures 204
 *     rows / +27.  Assigning the pointer at the top of the *function* instead
 *     of at the top of the block is 179/+4 against 148/+2.
 *   - m2c's `var = x << 0x10; ... temp = var >> 0x10;` pairs are gcc
 *     re-extending a short parameter; with the parameters typed they are pure
 *     stack slots and the frame is 0x10 too large.
 *   - g_FieldModels is a `FieldEntity*` and every one of m2c's eighteen
 *     `unkNN` is a named member -- PosX/PosY/PosZ at 0xC/0x10/0x14, Dir 0x38,
 *     TalkOff/visible/activeAnimId at 0x5B/0x5C/0x5E, animSpeed/
 *     animCurrentFrame/animLastFrame at 0x60/0x62/0x64, SolidRange/TalkRange/
 *     MoveSpeed/PosI at 0x6C/0x6E/0x70/0x72.  The printed labels confirm each
 *     one ("X=", "am", "MS", "AS", ":TR", ".SR", " I=").
 *   - The three "B-R"/"R-G"/"G-B" rows are the walkmesh triangle
 *     `g_FieldEntity[*camTri].PosI` selects: three vertices, three s16 each,
 *     12 halfwords apart off `D_800E4274`, the spelling field2.c already
 *     matches with.
 *   - `D_800A0238`/`023C`/`0240`/`02C0`/`02C4` are one-character strings
 *     (".", "V", "T", "*", "B") and `D_800A0270` is "".  m2c renders them as
 *     `static s8` objects, which is a second definition of bytes the .s
 *     already owns.
 *
 * What is left is register allocation, in two shapes and nothing else.  The
 * target keeps g_DebugText's address in $s1 and (s16)actorId in $s2; we get
 * them the other way round, and that single swap is most of the 148 rows.
 * The four insertions are all `move a0,s0` in the row-clearing block, where
 * the target sign-extends arg0 straight into $a0 at each of the five calls
 * and we materialise it into $s0 and copy.  Both are `allocno_compare`
 * outcomes on a program that is now two instructions from the target's
 * length, which is where CLAUDE.md says to stop reading and let
 * decomp-permuter search -- raise perm_temp_for_expr and perm_ins_block.
 *
 * When it lands: this function OWNS D_800A0270 ("") and D_800A02B8 ("/"),
 * which FieldDebugRenderPage and FieldEventRequest borrow by name.  Both
 * become local labels the moment it becomes C, so they need lines in
 * config/sym_extern.us.txt -- see the LENDS recipe in CLAUDE.md.  m2c's
 * `s8 D_800A02B8[4] = {0x2F,...}` is a *definition* and has to stay an
 * `extern` while this is pinned. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", DebugUpdateActor);
#else
void DebugUpdateActor(s16 arg0, s16 actorId) {
    s16* camTri;
    s32 lineOff;
    s32 nameOff;
    s8* temp_s5;
    s8* temp_s7;

    if (arg0 == 4) {
        if (!(g_FieldScriptDebugFlags & 4) ||
            (g_FieldScriptDebugEntities[actorId] != 0)) {
            D_800716C8 = actorId;
            if (g_FieldScriptDebugEntities[actorId] != 0) {
                FieldDebugPageSetColor(4, 0x7F, 1, 0x7F);
            } else {
                FieldDebugPageSetColor(4, 7, 0xF, 0x1F);
            }
            FieldDebugStringCopy(g_DebugText, &D_800E0628);
            goto block_8;
        }
    } else {
        FieldDebugStringCopy(g_DebugText, "ctrl:");
    block_8:
        nameOff = actorId * 8 + 0x20;
        FieldDebugStringConcat(
            g_DebugText, (char*)(nameOff + (s32)g_FieldScripts));
        if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
            SetStrToDebugRow(arg0, 0, g_DebugText);
        }
        if (g_FieldScriptDebugFlags & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
        FieldDebugStringCopy(g_DebugText, "RqLv=");
        FieldDebugStringU8hex((s32)D_8009A1C4[actorId], g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, " Tg=");
        switch (SavedScriptIds[actorId][D_8009A1C4[actorId]]) {
        case 0:
            FieldDebugStringConcat(g_DebugText, "dft");
            break;
        case 1:
            FieldDebugStringConcat(g_DebugText, "tlk");
            break;
        case 2:
            FieldDebugStringConcat(g_DebugText, "psh");
            break;
        default:
            FieldDebugStringU16hex(
                (s32)SavedScriptIds[actorId][D_8009A1C4[actorId]],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            break;
        }
        if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
            SetStrToDebugRow(arg0, 1, g_DebugText);
        }
        if (g_FieldScriptDebugFlags & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
        if (g_EntityToModel[actorId] == 0xFF) {
            if (D_8007078C[actorId] == g_EntityToModel[actorId]) {
                FieldDebugStringCopy(g_DebugText, "Abst");
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetDebugStrRowColor(arg0, 2, 6);
                    if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                        SetStrToDebugRow(arg0, 2, g_DebugText);
                    }
                }
            } else {
                FieldDebugStringCopy(g_DebugText, "line=");
                FieldDebugStringU16hex(
                    (s32)D_8007078C[actorId], g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                lineOff = D_8007078C[actorId] * 0x18;
                if (*((u8*)&D_8007E7AC + 0xC + lineOff) != 0) {
                    FieldDebugStringConcat(g_DebugText, " on");
                } else {
                    FieldDebugStringConcat(g_DebugText, " off");
                }
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetDebugStrRowColor(arg0, 2, 3);
                    if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                        SetStrToDebugRow(arg0, 2, g_DebugText);
                    }
                }
            }
        } else {
            FieldDebugStringCopy(g_DebugText, "man=");
            FieldDebugStringU16hex(
                (s32)g_EntityToModel[actorId], g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, " dir=");
            FieldDebugStringU16hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].Dir,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetDebugStrRowColor(arg0, 2, 2);
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetStrToDebugRow(arg0, 2, g_DebugText);
                }
            }
        }
        if (g_FieldScriptDebugFlags & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
        if (g_EntityToModel[actorId] != 0xFF) {
            FieldDebugStringCopy(g_DebugText, "X=");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].PosX >> 0xC,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, " Y=");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].PosY >> 0xC,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 3, g_DebugText);
                SetDebugStrRowColor(arg0, 3, 1);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "Z=");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].PosZ >> 0xC,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, " I=");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].PosI,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 4, g_DebugText);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringU8hex(
                (s32)D_800756E8[g_EntityToModel[actorId]], g_DebugText);
            FieldDebugStringConcat(g_DebugText, "am");
            FieldDebugStringU16hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].activeAnimId,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, ".");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].animCurrentFrame,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, ".");
            FieldDebugStringU16hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].animLastFrame,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 5, g_DebugText);
                SetDebugStrRowColor(arg0, 5, 7);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            if (g_FieldModels[g_EntityToModel[actorId]].visible != 0) {
                FieldDebugStringCopy(g_DebugText, "V");
            } else {
                FieldDebugStringCopy(g_DebugText, ".");
            }
            if (g_FieldModels[g_EntityToModel[actorId]].TalkOff != 0) {
                FieldDebugStringConcat(g_DebugText, ".");
            } else {
                FieldDebugStringConcat(g_DebugText, "T");
            }
            if (g_FieldModels[g_EntityToModel[actorId]].SolidOff != 0) {
                FieldDebugStringConcat(g_DebugText, ".");
            } else {
                FieldDebugStringConcat(g_DebugText, "S");
            }
            FieldDebugStringConcat(g_DebugText, ":TR");
            FieldDebugStringU16hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].TalkRange,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, ".SR");
            FieldDebugStringU16hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].SolidRange,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 6, g_DebugText);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "MS");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].MoveSpeed,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, " AS");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].animSpeed,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 7, g_DebugText);
                SetDebugStrRowColor(arg0, 7, 7);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
        } else {
            if (D_8007078C[actorId] != g_EntityToModel[actorId]) {
                FieldDebugStringCopy(g_DebugText, "AX");
                lineOff = D_8007078C[actorId] * 0x18;
                FieldDebugStringU32hex(
                    (s32) * (s16*)((u8*)&D_8007E7AC + 0x0 + lineOff),
                    g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, " AY");
                lineOff = D_8007078C[actorId] * 0x18;
                FieldDebugStringU32hex(
                    (s32) * (s16*)((u8*)&D_8007E7AC + 0x2 + lineOff),
                    g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetStrToDebugRow(arg0, 3, g_DebugText);
                }
                if (g_FieldScriptDebugFlags & 2) {
                    DebugPrintToFieldWindow(g_DebugText);
                }
                FieldDebugStringCopy(g_DebugText, "AZ");
                lineOff = D_8007078C[actorId] * 0x18;
                FieldDebugStringU32hex(
                    (s32) * (s16*)((u8*)&D_8007E7AC + 0x4 + lineOff),
                    g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetStrToDebugRow(arg0, 4, g_DebugText);
                }
                if (g_FieldScriptDebugFlags & 2) {
                    DebugPrintToFieldWindow(g_DebugText);
                }
                FieldDebugStringCopy(g_DebugText, "BX");
                lineOff = D_8007078C[actorId] * 0x18;
                FieldDebugStringU32hex(
                    (s32) * (s16*)((u8*)&D_8007E7AC + 0x6 + lineOff),
                    g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, " BY");
                lineOff = D_8007078C[actorId] * 0x18;
                FieldDebugStringU32hex(
                    (s32) * (s16*)((u8*)&D_8007E7AC + 0x8 + lineOff),
                    g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetStrToDebugRow(arg0, 5, g_DebugText);
                }
                if (g_FieldScriptDebugFlags & 2) {
                    DebugPrintToFieldWindow(g_DebugText);
                }
                FieldDebugStringCopy(g_DebugText, "BZ");
                lineOff = D_8007078C[actorId] * 0x18;
                FieldDebugStringU32hex(
                    (s32) * (s16*)((u8*)&D_8007E7AC + 0xA + lineOff),
                    g_DebugMessageBuffer);
                FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
                if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                    SetStrToDebugRow(arg0, 6, g_DebugText);
                }
                if (g_FieldScriptDebugFlags & 2) {
                    DebugPrintToFieldWindow(g_DebugText);
                }
                goto block_91;
            }
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 3, "");
                SetStrToDebugRow(arg0, 4, "");
                SetStrToDebugRow(arg0, 5, "");
                SetStrToDebugRow(arg0, 6, "");
            block_91:
                SetStrToDebugRow(arg0, 7, "");
            }
        }
        if (arg0 != 4) {
            FieldDebugStringCopy(g_DebugText, "SX");
            FieldDebugStringU32hex((s32)D_80071E38, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, " SY");
            FieldDebugStringU32hex((s32)D_80071E3C, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 8, g_DebugText);
                SetDebugStrRowColor(arg0, 8, 3);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            camTri = &D_8009AC1E;
            FieldDebugStringCopy(g_DebugText, "B-R    X=");
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 0],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 9, g_DebugText);
                SetDebugStrRowColor(arg0, 9, 2);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            temp_s7 = "Y=";
            FieldDebugStringCopy(g_DebugText, temp_s7);
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 1],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            temp_s5 = " Z=";
            FieldDebugStringConcat(g_DebugText, temp_s5);
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 2],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0xA, g_DebugText);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "R-G    X=");
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 4],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0xB, g_DebugText);
                SetDebugStrRowColor(arg0, 0xB, 4);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, temp_s7);
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 5],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, temp_s5);
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 6],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0xC, g_DebugText);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "G-B    X=");
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 8],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0xD, g_DebugText);
                SetDebugStrRowColor(arg0, 0xD, 3);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, temp_s7);
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 9],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, temp_s5);
            FieldDebugStringU32hex(
                (s32)D_800E4274[g_FieldEntity[*camTri].PosI * 12 + 10],
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0xE, g_DebugText);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "Offset X=");
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].OffsetX,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0xF, g_DebugText);
                SetDebugStrRowColor(arg0, 0xF, 2);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, temp_s7);
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].OffsetY,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, temp_s5);
            FieldDebugStringU32hex(
                (s32)g_FieldModels[g_EntityToModel[actorId]].OffsetZ,
                g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0x10, g_DebugText);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "SF");
            FieldDebugStringU32hex(
                *D_8009D288 | (D_8009D289 << 8), g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (g_FieldState->characterLock != 0) {
                if (D_80081DC4 != 0) {
                    FieldDebugStringConcat(g_DebugText, ".");
                } else {
                    FieldDebugStringConcat(g_DebugText, "/");
                }
            } else if (D_80081DC4 != 0) {
                FieldDebugStringConcat(g_DebugText, "+");
            } else {
                FieldDebugStringConcat(g_DebugText, "*");
            }
            FieldDebugStringConcat(g_DebugText, "B");
            FieldDebugStringU8hex((s32)*D_8009CBDC, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringU8hex((s32)D_8009CBDD, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringU8hex((s32)D_8009CBDE, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (g_FieldState->battlesDisabled != 0) {
                FieldDebugStringConcat(g_DebugText, ">");
            } else {
                FieldDebugStringConcat(g_DebugText, "*");
            }
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0x11, g_DebugText);
                SetDebugStrRowColor(arg0, 0x11, 6);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringCopy(g_DebugText, "DP ");
            FieldDebugStringU32hex((s32)D_80075E12, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, " ");
            FieldDebugStringU32hex(
                (s32)g_FieldModelBufferTop, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (g_FieldMusicLock != 0) {
                FieldDebugStringConcat(g_DebugText, "M");
            }
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0x12, g_DebugText);
                if (g_FieldModelBufferTop > 0x801AFFFFU) {
                    if (D_8009D29B & 0x10) {
                        SetDebugStrRowColor(arg0, 0x12, 5);
                    } else {
                        SetDebugStrRowColor(arg0, 0x12, 3);
                    }
                }
                if (g_FieldModelBufferTop > 0x801ADFFFU) {
                    SetDebugStrRowColor(arg0, 0x12, 5);
                } else if (g_FieldModelBufferTop > 0x801AAFFFU) {
                    SetDebugStrRowColor(arg0, 0x12, 4);
                } else if (g_FieldModelBufferTop > 0x801A7FFFU) {
                    SetDebugStrRowColor(arg0, 0x12, 1);
                } else if (g_FieldModelBufferTop > 0x801A3FFFU) {
                    SetDebugStrRowColor(arg0, 0x12, 3);
                } else if (g_FieldModelBufferTop > 0x8019FFFFU) {
                    SetDebugStrRowColor(arg0, 0x12, 2);
                } else if (g_FieldModelBufferTop > 0x80197FFFU) {
                    SetDebugStrRowColor(arg0, 0x12, 0);
                } else {
                    SetDebugStrRowColor(arg0, 0x12, 7);
                }
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
            FieldDebugStringU8hex((s32)*D_8009D391, g_DebugMessageBuffer);
            FieldDebugStringCopy(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringU8hex((s32)D_8009D392, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            FieldDebugStringU8hex((s32)D_8009D393, g_DebugMessageBuffer);
            FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
            if (D_8009D78A & 1) {
                FieldDebugStringConcat(g_DebugText, "C");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 2) {
                FieldDebugStringConcat(g_DebugText, "B");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 4) {
                FieldDebugStringConcat(g_DebugText, "T");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 8) {
                FieldDebugStringConcat(g_DebugText, "E");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x10) {
                FieldDebugStringConcat(g_DebugText, "R");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x20) {
                FieldDebugStringConcat(g_DebugText, "Y");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x40) {
                FieldDebugStringConcat(g_DebugText, "K");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x80) {
                FieldDebugStringConcat(g_DebugText, "V");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x100) {
                FieldDebugStringConcat(g_DebugText, "D");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x200) {
                FieldDebugStringConcat(g_DebugText, "U");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (D_8009D78A & 0x400) {
                FieldDebugStringConcat(g_DebugText, "F");
            } else {
                FieldDebugStringConcat(g_DebugText, ".");
            }
            if (((u8)D_8009FE8C | (g_FieldScriptDebugFlags & 1)) != 0) {
                SetStrToDebugRow(arg0, 0x13, g_DebugText);
                SetDebugStrRowColor(arg0, 0x13, 0);
            }
            if (g_FieldScriptDebugFlags & 2) {
                DebugPrintToFieldWindow(g_DebugText);
            }
        }
    }
}
#endif

/* Traces one field-script opcode to debug page 3 and/or the on-screen window:
 * the mnemonic first, then one "arg<n>=<byte>" line per operand read straight
 * back out of the script stream. Bit 4 of g_FieldScriptDebugFlags restricts
 * tracing to the entities flagged in g_FieldScriptDebugEntities. */
void DebugPrintOpcode(char* name, u32 numArgs) {
    u32 total;
    u32 i;

    if ((g_FieldScriptDebugFlags & 4) &&
        !g_FieldScriptDebugEntities[g_CurrentEntity]) {
        return;
    }
    FieldDebugStringCopy(g_DebugText, &D_800E0630);
    FieldDebugStringConcat(g_DebugText, name);
    if (g_DebugLevel & 1) {
        SetStrToDebugRow(3, 0, g_DebugText);
    }
    if (g_DebugLevel & 2) {
        DebugPrintToFieldWindow(g_DebugText);
    }
    total = numArgs + 1;
    while (numArgs != 0) {
        i = total - numArgs;
        FieldDebugStringCopy(g_DebugText, "arg");
        FieldDebugStringU8hex(i, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, "=");
        FieldDebugStringU16hex(GET_PARAM_U8(i), g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        if (g_DebugLevel & 1) {
            SetStrToDebugRow(3, i, g_DebugText);
        }
        if (g_DebugLevel & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
        numArgs--;
    }
}

void FieldDebugAddParseValueToPage2(const char* str, s32 val, s32 kind) {
    if (!(g_FieldScriptDebugFlags & 4) ||
        g_FieldScriptDebugEntities[g_CurrentEntity]) {
        FieldDebugStringCopy(g_DebugText, str);
        switch (kind) {
        case 1:
            FieldDebugStringU8hex(
                val, g_DebugMessageBuffer); // to single hex digit
            break;
        case 2:
            FieldDebugStringU16hex(
                val, g_DebugMessageBuffer); // to double hex digit
            break;
        case 4:
            FieldDebugStringU32hex(
                val, g_DebugMessageBuffer); // to four hex digits
            break;
        default:
            FieldDebugStringCopy(g_DebugMessageBuffer, D_800A0270);
            break;
        }
        FieldDebugStringConcat(g_DebugText, g_DebugMessageBuffer);
        if (g_DebugLevel & 1) {
            AddStrNextDebugRow(2, g_DebugText);
        }
        if (g_DebugLevel & 2) {
            DebugPrintToFieldWindow(g_DebugText);
        }
    }
}

/////////////////////////////////////////////////
// Begin of field_event_memory_bank.c
/////////////////////////////////////////////////

/* The four FieldEvent*Memory* accessors are one unit and had to be unparked in
 * the same change. They share a dozen debug strings, and while any of them was
 * still pinned its `.s` supplied those strings under `D_` symbols while the
 * unparked ones emitted their own local copies -- so each looked 16 to 22 rows
 * out for reasons that were entirely about which copy of "S indx=" the
 * relocation named. Unparked together the literals fold and all four are
 * byte-identical with no source change at all.
 *
 * The lesson generalises: when a group of near-clone functions all print the
 * same diagnostics, measure them together before believing any one of their
 * residues. Doing them one at a time reads as four separate regalloc walls. */
u8 FieldEventReadMemoryU8(s16 mb_half, s16 offset) {
    s32 indx;
    u8 value;
    u8 bankId;

    switch (mb_half) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 0:
        value = GET_PARAM_U8(offset);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G cons=", value, 2);
        }
        return value;
    case 1:
    case 2:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 3:
    case 4:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 11:
    case 12:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 13:
    case 14:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 15:
    case 7:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 2);
        }
        return value;
    case 5:
    case 6:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 2);
        }
        return value;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
    return 0;
}

void FieldEventWriteMemoryU8(s16 arg0, s16 arg1, u8 value) {
    u8 bankId;
    s32 indx;

    switch (arg0) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 1:
    case 2:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 3:
    case 4:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 11:
    case 12:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 13:
    case 14:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 15:
    case 7:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 5:
    case 6:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 2);
        }
        return;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
}

s16 FieldEventReadMemoryS16(s16 bank_id, s16 offset) {
    u8 bankId;
    s32 indx;
    s16 value;

    switch (bank_id) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 0:
        GET_PARAM_S16(value, offset);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G cons=", value, 4);
        }
        return value;
    case 1:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 2:
        indx = GET_PARAM_U8(offset);
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 3:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 4:
        indx = GET_PARAM_U8(offset) | 0x100;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 11:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 12:
        indx = GET_PARAM_U8(offset) | 0x200;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 13:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 14:
        indx = GET_PARAM_U8(offset) | 0x300;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 15:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 7:
        indx = GET_PARAM_U8(offset) | 0x400;
        value = Savemap.memory_bank_1[indx];
        value |= Savemap.memory_bank_1[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G glov=", value, 4);
        }
        return value;
    case 5:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 4);
        }
        return value;
    case 6:
        indx = GET_PARAM_U8(offset);
        value = g_FieldMapVars[indx];
        value |= g_FieldMapVars[indx + 1] << 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("G indx=", indx, 4);
            FieldDebugAddParseValueToPage2("G mapv=", value, 4);
        }
        return value;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
    return 0;
}

void FieldEventWriteMemoryS16(s16 arg0, s16 arg1, s16 value) {
    u8 bankId;
    s32 indx;

    switch (arg0) {
    case 1:
        bankId = GET_PARAM_U8(1) >> 4;
        break;
    case 2:
        bankId = GET_PARAM_U8(1) & 0xF;
        break;
    case 3:
        bankId = GET_PARAM_U8(2) >> 4;
        break;
    case 4:
        bankId = GET_PARAM_U8(2) & 0xF;
        break;
    case 5:
        bankId = GET_PARAM_U8(3) >> 4;
        break;
    case 6:
        bankId = GET_PARAM_U8(3) & 0xF;
        break;
    }

    switch (bankId) {
    case 1:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 2:
        indx = GET_PARAM_U8(arg1);
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 3:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 4:
        indx = GET_PARAM_U8(arg1) | 0x100;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 11:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 12:
        indx = GET_PARAM_U8(arg1) | 0x200;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 13:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 14:
        indx = GET_PARAM_U8(arg1) | 0x300;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 15:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 2);
        }
        return;
    case 7:
        indx = GET_PARAM_U8(arg1) | 0x400;
        Savemap.memory_bank_1[indx] = (u8)value;
        Savemap.memory_bank_1[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S glov=", value, 4);
        }
        return;
    case 5:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = (u8)value;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 2);
        }
        return;
    case 6:
        indx = GET_PARAM_U8(arg1);
        g_FieldMapVars[indx] = (u8)value;
        g_FieldMapVars[indx + 1] = value >> 8;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("S indx=", indx, 4);
            FieldDebugAddParseValueToPage2("S mapv=", value, 4);
        }
        return;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S data err=", bankId, 2);
    }
    FieldEventDebugError("Bad Event arg!");
}

//////////////////////////////////////////////////
// Start of field_opcode_system.c
/////////////////////////////////////////////////

// This is called when there the script tries to execute an invalid opcode
// called for opcodes:
// 0C 0D 1A 1B 1C 1D 1E 1F 44 46 4C 4E BE
s32 OpcodeFuncBad(void) {
    if (g_DebugLevel & 3) {
        FieldDebugStringU16hex(g_FieldCurrentOpcode, g_DebugMessageBuffer);
        FieldDebugStringConcat(g_DebugMessageBuffer, "???");
        DebugPrintOpcode(g_DebugMessageBuffer, 8);
        FieldDebugPageSetColor(3, 0x7F, 0, 0);
    } else {
        FieldEventDebugError("Bad Event code!");
    }
    return 1;
}

/**
 @brief Opcode 0x5F - **WAIT1* - Wait 1 frame

 Memory layout:

 | 0x5F |
 @details
 Waits one frame and returns 1
 @note
 This does not emit a debug message.
 */
s32 OpcodeFuncWait1(void) {
    PC_INC(1);
    return 1;
}

/**
 * @brief Opcode 0x24 - **WAIT** - Wait
 *
 * Memory layout:
 *
 * | 0x24 | A |
 *
 * - const UShort A: Amount (number of frames) to wait.
 * @details
 * g_FieldWaitCounter[g_CurrentEntity] == 0 by default. The opcode then
 * sets it to how many frames to wait before returning 1, which halts
 * execution of the script until next frame.
 *
 * If parameter == 0, the opcode behaves the same way as NOP.
 *
 * The opcode is then called once per frame, decrementing the counter until it
 * reaches 1, at which point it's set to 0 and 0 is returned, which
 * tells the script parser to continue executing next opcode.
 */

s32 OpcodeFuncWait(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wait", 2);
    }

    if (g_FieldWaitCounter[g_CurrentEntity] == 0) {
        GET_PARAM_S16(g_FieldWaitCounter[g_CurrentEntity], 1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2(
                "wait_st=", g_FieldWaitCounter[g_CurrentEntity], 4);
        }
        if (g_FieldWaitCounter[g_CurrentEntity] == 0) {
            PC_INC(3);
            return 1;
        }
        return 1;
    }

    if (g_FieldWaitCounter[g_CurrentEntity] == 1) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("wait_end=", 1, 4);
        }
        g_FieldWaitCounter[g_CurrentEntity] = 0;
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "wait=", g_FieldWaitCounter[g_CurrentEntity], 4);
    }

    g_FieldWaitCounter[g_CurrentEntity]--;
    return 1;
}

//////////////////////////////////////////////////
// Start of field_opcode_vars.c
/////////////////////////////////////////////////

s32 OpcodeFuncSet(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("set", 3);
    }
    FieldEventWriteMemoryU8(1, 2, FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncSet2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("set2", 4);
    }
    FieldEventWriteMemoryS16(1, 2, FieldEventReadMemoryS16(2, 3));
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncLbyte(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lbyte", 3);
    }
    FieldEventWriteMemoryU8(1, 2, FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncHbyte(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("hbyte", 4);
    }
    FieldEventWriteMemoryU8(1, 2, (u8)(FieldEventReadMemoryS16(2, 3) >> 8));
    PC_INC(5);
    return 0;
}

s32 OpcodeFunc2byte(void) {
    s16 lhs;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("2byte", 5);
    }
    lhs = FieldEventReadMemoryU8(2, 4);
    FieldEventWriteMemoryS16(1, 3, lhs | (FieldEventReadMemoryU8(4, 5) << 8));
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncSetx(void) {
    s16 offset;
    u8 bank;
    u8 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("setx", 6);
    }
    bank = GET_PARAM_U8(1) >> 4;
    offset = GET_PARAM_U8(3) + FieldEventReadMemoryS16(2, 3);
    value = FieldEventReadMemoryU8(4, 5);
    switch (bank) {
    case 15:
        offset += 256;
    case 13:
        offset += 256;
    case 11:
        offset += 256;
    case 3:
        offset += 256;
    case 1:
        if (offset >= 1280) {
            offset = 1279;
        }
        Savemap.memory_bank_1[offset] = value;
        break;
    case 5:
        if (offset >= 256) {
            offset = 255;
        }
        g_FieldMapVars[offset] = value;
        break;
    }
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncGetx(void) {
    s16 offset;
    u8 bank;
    u8 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("getx", 6);
    }
    bank = GET_PARAM_U8(1) >> 4;
    offset = GET_PARAM_U8(3) + FieldEventReadMemoryS16(2, 3);
    switch (bank) {
    case 15:
        offset += 256;
    case 13:
        offset += 256;
    case 11:
        offset += 256;
    case 3:
        offset += 256;
    case 1:
        if (offset >= 1280) {
            offset = 1279;
        }
        value = Savemap.memory_bank_1[offset];
        break;
    case 5:
        if (offset >= 256) {
            offset = 255;
        }
        value = g_FieldMapVars[offset];
        break;
    }

    FieldEventWriteMemoryU8(4, 5, value);
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncSrchx(void) {
    s16 end;
    s16 start;
    s16 where;
    u8 bank;
    u8 value;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("srchx", 8);
    }
    bank = GET_PARAM_U8(1) >> 4;
    start = GET_PARAM_U8(4) + FieldEventReadMemoryS16(2, 5);
    end = GET_PARAM_U8(4) + FieldEventReadMemoryS16(3, 7);
    value = FieldEventReadMemoryU8(4, 9);
    switch (bank) {
    case 15:
        start += 256;
        end += 256;
    case 13:
        start += 256;
        end += 256;
    case 11:
        start += 256;
        end += 256;
    case 3:
        start += 256;
        end += 256;
    case 1:
        if (start >= 1280) {
            start = 1279;
        }
        if (end >= 1280) {
            end = 1279;
        }
        for (i = start; i <= end; i++) {
            if (Savemap.memory_bank_1[i] == value) {
                FieldEventWriteMemoryS16(6, 10, i);
                PC_INC(11);
                return 0;
            }
        }
        break;
    case 5:
        if (start >= 256) {
            start = 255;
        }
        if (end >= 256) {
            end = 255;
        }
        for (i = start; i <= end; i++) {
            if (g_FieldMapVars[i] == value) {
                FieldEventWriteMemoryS16(6, 10, i);
                PC_INC(11);
                return 0;
            }
        }
        break;
    }
    FieldEventWriteMemoryS16(6, 10, -1);
    PC_INC(11);
    return 0;
}

s32 OpcodeFuncBiton(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("biton", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) | (1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBitof(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bitof", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) & ~(1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBitxr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bitxr", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) ^ (1 << FieldEventReadMemoryU8(2, 3)));
    PC_INC(4);
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_line.c
/////////////////////////////////////////////////

s32 OpcodeFuncLine(void) {
    s16 value;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("line", 8);
    }

    if (g_FieldLineCount >= 32) {
        FieldEventDebugError("many lineobj!");
        PC_INC(13);
        return 0;
    }

    g_EntityToLine[g_CurrentEntity] = g_FieldLineCount;
    GET_PARAM_S16(value, 1);
    g_FieldLines[g_FieldLineCount].pos.x1 = value;
    GET_PARAM_S16(value, 3);
    g_FieldLines[g_FieldLineCount].pos.y1 = value;
    GET_PARAM_S16(value, 5);
    g_FieldLines[g_FieldLineCount].pos.z1 = value;
    GET_PARAM_S16(value, 7);
    g_FieldLines[g_FieldLineCount].pos.x2 = value;
    GET_PARAM_S16(value, 9);
    g_FieldLines[g_FieldLineCount].pos.y2 = value;
    GET_PARAM_S16(value, 11);
    g_FieldLines[g_FieldLineCount].pos.z2 = value;
    g_FieldLines[g_FieldLineCount].isActive = 1;
    g_FieldLines[g_FieldLineCount].entityId = g_CurrentEntity;
    g_FieldLineCount++;
    PC_INC(13);
    return 0;
}

s32 OpcodeFuncSline(void) {
    u8 lineId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sline", 8);
    }
    lineId = g_EntityToLine[g_CurrentEntity];
    g_FieldLines[lineId].pos.x1 = FieldEventReadMemoryS16(1, 4);
    g_FieldLines[lineId].pos.y1 = FieldEventReadMemoryS16(2, 6);
    g_FieldLines[lineId].pos.z1 = FieldEventReadMemoryS16(3, 8);
    g_FieldLines[lineId].pos.x2 = FieldEventReadMemoryS16(4, 10);
    g_FieldLines[lineId].pos.y2 = FieldEventReadMemoryS16(5, 12);
    g_FieldLines[lineId].pos.z2 = FieldEventReadMemoryS16(6, 14);
    PC_INC(16);
    return 0;
}

s32 OpcodeFuncLinon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("linon", 1);
    }
    g_FieldLines[g_EntityToLine[g_CurrentEntity]].isActive = GET_PARAM_U8(1);
    if (GET_PARAM_U8(1) == 0) {
        g_FieldLines[g_EntityToLine[g_CurrentEntity]].touch = 0;
    }
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode SLIP: Enables or disables slipping along a line
 *
 * Slipping allows the player to slide along a wall when running
 * against it instead of stopping. The wall must previously have a
 * line defined alongside it with opcode LINE.
 */

s32 OpcodeFuncSlip(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("slip", 1);
    }
    g_FieldLines[g_EntityToLine[g_CurrentEntity]].slipDisabled =
        GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_if.c
/////////////////////////////////////////////////

/*
 * Field-script opcode IF: If, 1 byte, unsigned
 *
 * Compares two u8 using a given logical operator.
 * Jumps given number of bytes ahead if the comparison is false.
 */

s32 OpcodeFuncIf(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if", 5);
    }
    if (IfCheck()) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if=true", 0, 0);
        }
        // If comparison is true, continue executing next opcode.
        PC_INC(6);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if=false", 0, 0);
        }
        // If comparison is false, jump number of bytes give in last parameter
        // from last parameter.
        PC_INC(GET_PARAM_U8(5) + 5);
    }
    return 0;
}

/*
 * Field-script opcode LIF: Long If, 1 byte, unsigned
 *
 * Compares two u8 using a given logical operator.
 * Identical to IF except that the jump parameter is s16, allowing for longer
 * jumps.
 */

s32 OpcodeFuncLif(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif", 6);
    }
    if (IfCheck()) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif=true", 0, 0);
        }
        PC_INC(7);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif=false", 0, 0);
        }
        GET_PARAM_S16(param, 5);
        PC_INC(param + 5);
    }
    return 0;
}

u32 IfCheck(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(4);
    switch (ope) {
    case IF_EQ:
        result = FieldEventReadMemoryU8(1, 2) == FieldEventReadMemoryU8(2, 3);
        break;
    case IF_NOT_EQ:
        result = FieldEventReadMemoryU8(1, 2) != FieldEventReadMemoryU8(2, 3);
        break;
    case IF_GT:
        result = FieldEventReadMemoryU8(1, 2) > FieldEventReadMemoryU8(2, 3);
        break;
    case IF_LT:
        result = FieldEventReadMemoryU8(1, 2) < FieldEventReadMemoryU8(2, 3);
        break;
    case IF_GTE:
        result = FieldEventReadMemoryU8(1, 2) >= FieldEventReadMemoryU8(2, 3);
        break;
    case IF_LTE:
        result = FieldEventReadMemoryU8(1, 2) <= FieldEventReadMemoryU8(2, 3);
        break;
    case IF_AND:
        result = FieldEventReadMemoryU8(1, 2) & FieldEventReadMemoryU8(2, 3);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryU8(1, 2) ^ FieldEventReadMemoryU8(2, 3);
        break;
    case IF_OR:
        result = FieldEventReadMemoryU8(1, 2) | FieldEventReadMemoryU8(2, 3);
        break;
    case IF_BIT:
        result =
            FieldEventReadMemoryU8(1, 2) & (1 << FieldEventReadMemoryU8(2, 3));
        break;
    case IF_NOT_BIT:
        result =
            FieldEventReadMemoryU8(1, 2) & (1 << FieldEventReadMemoryU8(2, 3));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}

/*
 * Field-script opcode IF2: If, 2 bytes, signed
 *
 * Compares two s16 using a given logical operator.
 */
s32 OpcodeFuncIf2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if2", 7);
    }
    if (If2CheckSigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=true", 0, 0);
        }
        PC_INC(8);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(7) + 7);
    }
    return 0;
}

/*
 * Field-script opcode LIF2: Long if, 2 bytes, signed
 *
 * Compares two s16 using a given logical operator.
 */
s32 OpcodeFuncLif2(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif2", 8);
    }
    if (If2CheckSigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=true", 0, 0);
        }
        PC_INC(9);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=false", 0, 0);
        }
        GET_PARAM_S16(param, 7);
        PC_INC(param + 7);
    }
    return 0;
}

u32 If2CheckSigned(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(6);
    switch (ope) {
    case IF_EQ:
        result = FieldEventReadMemoryS16(1, 2) == FieldEventReadMemoryS16(2, 4);
        break;
    case IF_NOT_EQ:
        result = FieldEventReadMemoryS16(1, 2) != FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GT:
        result = FieldEventReadMemoryS16(1, 2) > FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LT:
        result = FieldEventReadMemoryS16(1, 2) < FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GTE:
        result = FieldEventReadMemoryS16(1, 2) >= FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LTE:
        result = FieldEventReadMemoryS16(1, 2) <= FieldEventReadMemoryS16(2, 4);
        break;
    case IF_AND:
        result = FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 4);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 4);
        break;
    case IF_OR:
        result = FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 4);
        break;
    case IF_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        break;
    case IF_NOT_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}

/*
 * Field-script opcode IF2U: If, 2 bytes, unsigned
 *
 * Compares two u16 using a given logical operator.
 */
s32 OpcodeFuncIf2u(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("if2", 7);
    }
    if (If2CheckUnsigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=true", 0, 0);
        }
        PC_INC(8);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("if2=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(7) + 7);
    }
    return 0;
}

/*
 * Field-script opcode LIF2U: Long if, 2 bytes, unsigned
 *
 * Compares two u16 using a given logical operator.
 */
s32 OpcodeFuncLif2u(void) {
    s16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lif2", 8);
    }
    if (If2CheckUnsigned() != 0) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=true", 0, 0);
        }
        PC_INC(9);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("lif2=false", 0, 0);
        }
        GET_PARAM_S16(param, 7);
        PC_INC(param + 7);
    }
    return 0;
}

u32 If2CheckUnsigned(void) {
    u8 ope;
    u8 result;

    ope = GET_PARAM_U8(6);
    switch (ope) {
    case IF_EQ:
        result = (u16)FieldEventReadMemoryS16(1, 2) ==
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_NOT_EQ:
        result = (u16)FieldEventReadMemoryS16(1, 2) !=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GT:
        result = (u16)FieldEventReadMemoryS16(1, 2) >
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LT:
        result = (u16)FieldEventReadMemoryS16(1, 2) <
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_GTE:
        result = (u16)FieldEventReadMemoryS16(1, 2) >=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_LTE:
        result = (u16)FieldEventReadMemoryS16(1, 2) <=
                 (u16)FieldEventReadMemoryS16(2, 4);
        break;
    case IF_AND:
        result = FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 4);
        break;
    case IF_XOR:
        result = FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 4);
        break;
    case IF_OR:
        result = FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 4);
        break;
    case IF_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        break;
    case IF_NOT_BIT:
        result = FieldEventReadMemoryS16(1, 2) &
                 (1 << FieldEventReadMemoryS16(2, 4));
        result = result < 1;
        break;
    default:
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("ope err=", ope, 2);
        }
        break;
    }
    return result;
}

//////////////////////////////////////////////////
// Start of field_opcode_controller.c
/////////////////////////////////////////////////

/*
 * Field-script opcode KEY!: Key check
 *
 * Jumps ahead given number of bytes if given key(s) are not active.
 * All key opcodes only check the lower half word which contains the keys
 * for controller 1.
 */

s32 OpcodeFuncKeyEx(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("key!", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->activeKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->activeKeys);
    }
}

/*
 * Field-script opcode KEYON: Key On
 *
 * Checks keys that player pressed this frame.
 */

s32 OpcodeFuncKeyon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("keyon", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->newActiveKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->newActiveKeys);
    }
}

/*
 * Field-script opcode KEYOF: Key Off
 *
 * Checks keys that player released this frame.
 */

s32 OpcodeFuncKeyof(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("keyof", 3);
    }
    if (GET_PARAM_U8(2) & 2) {
        return KeyCheck((u16)g_FieldState->newInactiveKeys2);
    } else {
        return KeyCheck((u16)g_FieldState->newInactiveKeys);
    }
}

s32 KeyCheck(u16 keys) {
    u16 param;

    GET_PARAM_S16(param, 1);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("key now=", keys, 4);
        FieldDebugAddParseValueToPage2("key chk=", param, 4);
    }
    if (keys & param) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("key=true", 0, 0);
        }
        PC_INC(4);
    } else {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("key=false", 0, 0);
        }
        PC_INC(GET_PARAM_U8(3) + 3);
    }
    return 0;
}

//////////////////////////////////////////////////
// Start of field_opcode_request.c
/////////////////////////////////////////////////

s32 OpcodeFuncReq(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("req", 2);
    }
    return FieldEventRequest(1, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncReqsw(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("reqsw", 2);
    }
    return FieldEventRequest(2, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncReqew(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("reqew", 2);
    }
    return FieldEventRequest(3, GET_PARAM_U8(1), GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPreq(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("preq", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(1, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPrqsw(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prqsw", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(2, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

s32 OpcodeFuncPrqew(void) {
    u8 charId;
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prqew", 2);
    }
    charId = Savemap.memory_bank_2[GET_PARAM_U8(1) + 9];
    if (charId == 0xFF) {
        entityId = 0xFF;
    } else {
        entityId = g_CharIdToEntity[charId];
    }
    return FieldEventRequest(3, entityId, GET_PRIORITY(GET_PARAM_U8(2)),
                             GET_SCRIPTID(GET_PARAM_U8(2)));
}

/* "/" lives at 0x800A02B8, in DebugUpdateActor's .s, and is passed by name
 * rather than written as a literal: a second copy would be emitted into this
 * unit's pool and shift every later .rodata offset. Turn it back into "/" when
 * DebugUpdateActor becomes C -- gcc folds identical literals within one
 * translation unit, so the two would then share the one definition. */
s32 FieldEventRequest(s16 type, u8 target, u8 priority, u8 scriptId) {
    s32 scriptOffset;
    s32 entityDataSize;
    s32 extrasHeaderSize;

    if (target == 0xFF) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rqew=no one", 0, 0);
        }
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugStringCopy(g_DebugMessageBuffer, "rq=");
        FieldDebugStringConcat(
            g_DebugMessageBuffer, (char*)((s32)g_FieldScripts) +
                                      sizeof(FieldScriptHeader) + (target * 8));
        FieldDebugStringConcat(g_DebugMessageBuffer, D_800A02B8);
        FieldDebugAddParseValueToPage2(g_DebugMessageBuffer, scriptId, 2);
    }

    if (type > 0) {
        if (type >= 3) {
            if (type == 3 && g_FieldScriptSyncWaitEntity[target][priority] ==
                                 g_CurrentEntity) {
                switch (g_FieldScriptSyncState[target][priority]) {
                case SYNC_WAITING:
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("rqew=wait", 0, 0);
                    }
                    return 1;
                case SYNC_DONE:
                    if (g_DebugLevel & 3) {
                        FieldDebugAddParseValueToPage2("rqew=end", 0, 0);
                    }
                    g_FieldScriptSyncState[target][priority] = SYNC_NONE;
                    g_FieldScriptSyncWaitEntity[target][priority] = 0xFF;
                    PC_INC(3);
                    return 0;
                }
            }
        }
    }

    if (g_FieldScriptPriority[target] == priority) {
        switch (type) {
        case 1:
            PC_INC(3);
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("rq=skip", 0, 0);
            }
            return 0;
        case 2:
        case 3:
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("rqw=busy", 0, 0);
            }
        }
        return 1;
    } else if (g_FieldScriptPriority[target] < priority) {
        if (g_SavedFieldScriptPC[target][priority] != 0) {
            switch (type) {
            case 1:
                PC_INC(3);
                if (g_DebugLevel & 3) {
                    FieldDebugAddParseValueToPage2("rq=skip", 0, 0);
                }
                return 0;
            case 2:
            case 3:
                if (g_DebugLevel & 3) {
                    FieldDebugAddParseValueToPage2("rqw=busy", 0, 0);
                }
            }
            return 1;
        }
        scriptOffset = scriptId * 2;
        entityDataSize = target * 64;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

        g_SavedFieldScriptPC[target][priority] =
            *((u8*)(scriptOffset +
                    (entityDataSize + (g_FieldScripts->numEntities << 3)) +
                    extrasHeaderSize + (s32)g_FieldScripts) +
              sizeof(FieldScriptHeader));
        g_SavedFieldScriptPC[target][priority] |=
            *((u8*)(scriptOffset +
                    ((entityDataSize + (g_FieldScripts->numEntities << 3)) +
                     (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rq=send", 0, 0);
        }

        if (type <= 0) {
            return 1;
        }

        if (type >= 3) {
            if (type != 3) {
                return 1;
            }
        } else {
            PC_INC(3);
            return 0;
        }

        g_FieldScriptSyncWaitEntity[target][priority] = g_CurrentEntity;
        g_FieldScriptSyncState[target][priority] = SYNC_WAITING;
        return 1;
    } else if (g_FieldScriptSyncState[target][priority] == SYNC_NONE) {
        s32 scriptOffset;
        s32 entityDataSize;
        s32 extrasHeaderSize;

        SavedScriptIds[target][priority] = scriptId;
        g_SavedFieldScriptPC[target][g_FieldScriptPriority[target]] =
            g_FieldScriptPC[target];

        scriptOffset = scriptId * 2;
        entityDataSize = target * 64;
        extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

        g_FieldScriptPC[target] =
            *((u8*)(scriptOffset +
                    (entityDataSize + (g_FieldScripts->numEntities << 3)) +
                    extrasHeaderSize + (s32)g_FieldScripts) +
              sizeof(FieldScriptHeader));
        g_FieldScriptPC[target] |=
            *((u8*)(scriptOffset +
                    ((entityDataSize + (g_FieldScripts->numEntities << 3)) +
                     (s32)g_FieldScripts) +
                    extrasHeaderSize) +
              sizeof(FieldScriptHeader) + 1)
            << 8;

        g_FieldScriptPriority[target] = priority;

        if (g_EntityToModel[target] != 0xFF) {
            g_FieldModels[g_EntityToModel[target]].scriptedMoveMode =
                SMODE_NONE;
        }
        g_FieldWaitCounter[target] = 0;

        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("rq=send", 0, 0);
        }

        if (type <= 0) {
            return 1;
        }

        if (type >= 3) {
            if (type != 3) {
                return 1;
            }
        } else {
            PC_INC(3);
            return 0;
        }

        g_FieldScriptSyncWaitEntity[target][priority] = g_CurrentEntity;
        g_FieldScriptSyncState[target][priority] = SYNC_WAITING;
        return 1;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("rqw=busy*", 0, 0);
    }
    return 1;
}

s32 OpcodeFuncRet(void) {
    u16* fieldScriptPC;
    u16(*savedPC)[8];
    u16* savedRow;
    u16 scriptPc;
    u32 entity;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ret", 0);
    }
    if (g_FieldScriptPriority[g_CurrentEntity] >= 7) {
        return 1;
    }

    if (g_FieldScriptSyncState[g_CurrentEntity]
                              [g_FieldScriptPriority[g_CurrentEntity]] ==
        SYNC_WAITING) {
        g_FieldScriptSyncState[g_CurrentEntity]
                              [g_FieldScriptPriority[g_CurrentEntity]] =
                                  SYNC_DONE;
    }

    g_FieldScriptPriority[g_CurrentEntity]++;

    entity = g_CurrentEntity;
    savedPC = g_SavedFieldScriptPC;
    fieldScriptPC = g_FieldScriptPC;

    savedRow = savedPC[entity];
    scriptPc =
        *(u16*)((g_FieldScriptPriority[entity] * sizeof(u16)) + (s32)savedRow);
    fieldScriptPC[entity] = scriptPc;

    while (scriptPc == 0 && g_FieldScriptPriority[entity] < 7) {
        u16* activePcSlot;
        u16* loopSavedRow;
        u16 nextPc;

        g_FieldScriptPriority[g_CurrentEntity]++;
        entity = g_CurrentEntity;

        activePcSlot =
            (u16*)((entity * sizeof(*fieldScriptPC)) + (s32)fieldScriptPC);

        loopSavedRow = (u16*)((entity * sizeof(*savedPC)) + (s32)savedPC);

        nextPc = *(u16*)((g_FieldScriptPriority[entity] * sizeof(u16)) +
                         (s32)loopSavedRow);

        *activePcSlot = nextPc;
        scriptPc = nextPc;
    }

    g_SavedFieldScriptPC[g_CurrentEntity]
                        [g_FieldScriptPriority[g_CurrentEntity]] = 0;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "ret=", g_FieldScriptPriority[g_CurrentEntity], 2);
    }
    return 0;
}

s32 OpcodeFuncRetto(void) {
    s16 scriptId;
    u8 priority;
    s32 extrasHeaderSize;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("retto", 1);
    }

    priority = GET_PRIORITY(GET_PARAM_U8(1));
    scriptId = GET_SCRIPTID(GET_PARAM_U8(1));

    while (g_FieldScriptPriority[g_CurrentEntity] < (priority - 1) &&
           g_FieldScriptPriority[g_CurrentEntity] < 7) {
        if (g_FieldScriptSyncState[g_CurrentEntity]
                                  [g_FieldScriptPriority[g_CurrentEntity]] ==
            SYNC_WAITING) {
            g_FieldScriptSyncState[g_CurrentEntity]
                                  [g_FieldScriptPriority[g_CurrentEntity]] =
                                      SYNC_DONE;
        }
        g_FieldScriptPriority[g_CurrentEntity]++;
        g_SavedFieldScriptPC[g_CurrentEntity]
                            [g_FieldScriptPriority[g_CurrentEntity]] = 0;
    }
    SavedScriptIds[g_CurrentEntity][priority] = scriptId;
    scriptId *= 2;
    extrasHeaderSize = (s16)(g_FieldScripts->numExtras * 4);

    g_FieldScriptPC[g_CurrentEntity] =
        *((u8*)(scriptId +
                ((g_FieldScripts->numEntities * 8) + (g_CurrentEntity * 64)) +
                extrasHeaderSize + (s32)g_FieldScripts) +
          sizeof(FieldScriptHeader));
    g_FieldScriptPC[g_CurrentEntity] |=
        *((u8*)(scriptId +
                (((g_FieldScripts->numEntities * 8) + (g_CurrentEntity * 64)) +
                 (s32)g_FieldScripts) +
                extrasHeaderSize) +
          sizeof(FieldScriptHeader) + 1)
        << 8;

    g_FieldScriptPriority[g_CurrentEntity] = priority;
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2(
            "ret=", g_FieldScriptPriority[g_CurrentEntity], 2);
    }
    return 0;
}

s32 OpcodeFuncBack(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("back", 1);
    }
    PC_DEC(GET_PARAM_U8(1));
    return 1;
}

s32 OpcodeFuncLback(void) {
    u16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lback", 2);
    }
    GET_PARAM_S16(param, 1);
    PC_DEC(param);
    return 1;
}

s32 OpcodeFuncSkip(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("skip", 1);
    }
    PC_INC(GET_PARAM_U8(1) + 1);
    return 0;
}

s32 OpcodeFuncLskip(void) {
    u16 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lskip", 2);
    }
    GET_PARAM_S16(param, 1);
    PC_INC(param + 1);
    return 0;
}

/* MJUMP (0x60): jump to another field map at an explicit position, direction
 * and walkmesh triangle.
 *
 * `pcDirection` has to be stored as a halfword: the target's `sh v0,0x24(a2)`
 * covers pcDirection and the unk25 byte behind it in one store, the same way
 * FieldEntityGatewayMapLoad writes it, and a plain `= GET_PARAM_U8(9)` gives
 * `sb` and was the only diff row.
 *
 * This function's `.s` used to own D_800A0848, the "evt cmd=" string
 * src/field/field5.c reaches by symbol, which is why it sat parked as a
 * verified match for as long as it did. The literal is a local label now;
 * field5's reference is satisfied by an absolute definition in
 * config/sym_extern.us.txt, which the linker takes as a script. See CLAUDE.md
 * on landing a LENDS function.
 */
s32 OpcodeFuncMjump(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mjump", 8);
    }

    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_FIELD_MAP_CHANGE;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        GET_PARAM_S16(g_FieldState->eventCmdParam, 1);
        GET_PARAM_S16(g_FieldState->pcPosX, 3);
        GET_PARAM_S16(g_FieldState->pcPosY, 5);
        GET_PARAM_S16(g_FieldState->pcWalkMeshId, 7);
        *(u16*)&g_FieldState->pcDirection = GET_PARAM_U8(9);
        return 1;
    case EVTCMD_FIELD_MAP_CHANGE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(10);
            g_FieldState->eventCmd = EVTCMD_NONE;
            return 0;
        }
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("evt cmd=", g_FieldState->eventCmd, 2);
    }
    return 1;
}

s32 OpcodeFuncPmjmp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmjmp", 8);
    }
    GET_PARAM_S16(g_FieldPreloadMapId, 1);
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncPmjmp2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmjmp", 8);
    }
    if (g_isFieldLoading != 2) {
        return 1;
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncMgame(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mgame", 8);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_LOAD_MINIGAME;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        GET_PARAM_S16(g_FieldState->eventCmdParam, 1);
        GET_PARAM_S16(g_FieldState->pcPosX, 3);
        GET_PARAM_S16(g_FieldState->pcPosY, 5);
        GET_PARAM_S16(g_FieldState->pcWalkMeshId, 7);
        *(s16*)&g_FieldState->pcDirection = GET_PARAM_U8(9);
        *(u8*)((u8*)g_FieldState + 0xF2) = GET_PARAM_U8(10);
        return 1;
    case EVTCMD_LOAD_MINIGAME:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(11);
            g_FieldState->eventCmd = EVTCMD_NONE;
            return 0;
        }
        return 1;
    }
    return 1;
}

s32 OpcodeFuncBatle(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("batle", 3);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        FieldWindowResetTextAll();
        g_FieldState->eventCmd = EVTCMD_ENTERING_BATTLE;
        g_FieldState->eventCmdParam = FieldEventReadMemoryS16(2, 2);
        D_8007EBE0 = 1;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        return 1;
    case EVTCMD_ENTERING_BATTLE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            PC_INC(4);
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            return 0;
        }
        break;
    }
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_akao_sound.c
/////////////////////////////////////////////////

void FieldEventClearAkaoStruct(void) {
    s32 i;
    s16* p;

    D_8009A000[0] = 0;
    for (i = 5, p = &D_8009A000[10]; i >= 0; i--) {
        *(s32*)(p + 2) = 0;
        p -= 2;
    }
}

s32 OpcodeFuncAkao(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("akao", 3);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(4);
    *D_8009A004 = FieldEventReadMemoryU8(1, 5);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(2, 6);
    g_FieldAkaoArg3 = (s16)FieldEventReadMemoryS16(3, 8);
    g_FieldAkaoArg4 = (s16)FieldEventReadMemoryS16(4, 10);
    g_FieldAkaoArg5 = (s16)FieldEventReadMemoryS16(6, 12);
    SystemAkaoExecute();
    PC_INC(14);
    return 0;
}

s32 OpcodeFuncAkao2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("akao2", 3);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(4);
    *D_8009A004 = (s16)FieldEventReadMemoryS16(1, 5);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(2, 7);
    g_FieldAkaoArg3 = (s16)FieldEventReadMemoryS16(3, 9);
    g_FieldAkaoArg4 = (s16)FieldEventReadMemoryS16(4, 11);
    g_FieldAkaoArg5 = (s16)FieldEventReadMemoryS16(6, 13);
    SystemAkaoExecute();
    PC_INC(15);
    return 0;
}

s32 OpcodeFuncSe(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("se", 3);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x20;
    D_8009A004[0] = FieldEventReadMemoryU8(2, 4);
    D_8009A008[0] = (s16)FieldEventReadMemoryS16(1, 2);
    SystemAkaoExecute();
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncMusic(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("music", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x10;
    return SetAndApplyAkao();
}

s32 OpcodeFuncMusvt(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("musvt", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x14;
    return SetAndApplyAkao();
}

s32 OpcodeFuncMusvm(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("musvm", 1);
    }
    FieldEventClearAkaoStruct();
    D_8009A000[0] = 0x15;
    return SetAndApplyAkao();
}

s32 OpcodeFuncCmusc(void) {
    u32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cmusc", 5);
    }
    FieldEventClearAkaoStruct();
    *D_8009A000 = GET_PARAM_U8(3);
    *D_8009A008 = (s16)FieldEventReadMemoryS16(3, 4);
    g_FieldAkaoArg3 = (s16)FieldEventReadMemoryS16(4, 6);
    result = SetAndApplyAkao();
    PC_INC(6);
    return result;
}

s32 SetAndApplyAkao(void) {
    // Indexes into AKAO block of field file which contains the list of music
    // tracks available for current field.
    u8 akaoId;

    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("music=", akaoId, 2);
        }
        *D_8009A004 = (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
        g_FieldState->nextFieldMusic = *D_8009A004;
        SystemAkaoExecute();
    }
    PC_INC(2);
    return 0;
}

u32 GetAkaoBlockOffset(s16 akaoId) {
    s32 akaoData;
    u32 akaoOffset;

    akaoData =
        akaoId * 4 + g_FieldScripts->numEntities * 8 + (s32)g_FieldScripts;
    akaoOffset = ((u8*)akaoData)[sizeof(FieldScriptHeader)];
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 1] << 8;
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 2] << 16;
    akaoOffset |= ((u8*)akaoData)[sizeof(FieldScriptHeader) + 3] << 24;
    return akaoOffset;
}

s32 OpcodeFuncBmusc(void) {
    u8 akaoId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bmusc", 1);
    }
    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("bmusic=", akaoId, 2);
        }
        g_FieldState->nextBattleMusic =
            (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
    } else {
        g_FieldState->nextBattleMusic = 0;
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncFmusc(void) {
    u8 akaoId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fmusc", 1);
    }
    if (g_FieldMusicLock == 0) {
        akaoId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("bmusic=", akaoId, 2);
        }
        g_FieldState->nextFieldMusic =
            (u8*)((s32)g_FieldScripts + GetAkaoBlockOffset(akaoId));
    } else {
        g_FieldState->nextFieldMusic = 0;
    }
    PC_INC(2);
    return 0;
}

// In Akao because it uses the AKAO block area
/* TUTOR (0x21): open the main menu and play the tutorial with the given id.
 * First call arms the PARTY_MENU event command, flags the menu overlay and
 * resolves the tutorial's block into g_FieldTutorialAkaoBlock for the main loop
 * to stream; once the menu reports MOVCMD_DONE, clear the command and advance
 * past the operand. */
extern u8* g_FieldTutorialAkaoBlock;

/* This function's `.s` used to own D_800A08D0, the "evt result=" string
 * src/field/field5.c reaches by symbol, which is what kept a verified match
 * parked. The literal is a local label now; field5's reference is satisfied by
 * an absolute definition in config/sym_extern.us.txt. See CLAUDE.md on landing
 * a LENDS function. */
s32 OpcodeFuncTutor(void) {
    s16 tutorialId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tutor", 1);
    }
    if (g_FieldState->eventCmd == EVTCMD_NONE) {
        g_FieldState->eventCmd = EVTCMD_PARTY_MENU;
        D_8007EBE0 = 1;
        g_FieldState->eventCmdParam = 1;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        tutorialId = GET_PARAM_U8(1);
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("data=", tutorialId, 2);
        }
        g_FieldTutorialAkaoBlock =
            (u8*)g_FieldScripts + GetAkaoBlockOffset(tutorialId);
        return 1;
    }
    if (g_FieldState->eventCmd == EVTCMD_PARTY_MENU) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2(
                "evt result=", g_FieldState->movieCommandState, 2);
        }
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(2);
            return 0;
        }
    }
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_movie_overlay.c
/////////////////////////////////////////////////

/*
 * Field-script opcode MULCK (0xF5): set the music lock from the opcode operand.
 *
 * While g_FieldMusicLock is nonzero the MUSIC/FMUSC opcodes skip handing the
 * song to the sound engine, so field music stops responding until a later
 * MULCK 0 (or a reset) clears it again.
 *
 * The operand is read straight out of the running script:
 *   g_FieldScripts          - the current map's script bytecode
 *   g_FieldScriptPC[entity]  - that entity's program counter (byte offset into
 * it) g_CurrentEntity          - the entity whose script is currently executing
 * so g_FieldScripts[pc + 1] is the 1-byte operand. The program counter is then
 * stepped past the 2-byte instruction (opcode + operand).
 */
s32 OpcodeFuncMulck(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mulck", 1);
    }
    g_FieldMusicLock = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncBgmovie(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgmovie", 1);
    }
    g_FieldState->backgroundMovieEnabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncScrlo(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlo", 1);
    }
    g_FieldState->scrloSet = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode DSKCG: request a disc change.
 *
 * Runs as a small state machine on the field main-loop step (opcode):
 * on first execution it stores the requested disc number and switches the
 * field loop into the disc-change step (13), then keeps returning 1
 * (opcode not finished) until the loop reports the swap is done
 * (movieCommandState == 2). Only then does the script advance past the opcode.
 */
s32 OpcodeFuncDskcg(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dskcg", 1);
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_CD_CHANGE;
        g_FieldDiscChangeRequest = GET_PARAM_U8(1);
        return 1;
    case EVTCMD_CD_CHANGE:
        if (g_FieldState->movieCommandState == MOVCMD_DONE) {
            g_FieldState->eventCmd = EVTCMD_NONE;
            PC_INC(2);
            return 0;
        }
        return 1;
    default:
        return 1;
    }
}

/*
 * Field-script opcode UC: lock or unlock player control.
 *
 * A nonzero operand freezes the player character; on unlock the
 * per-model flag of the player's model is cleared as well.
 */
s32 OpcodeFuncUc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("uc", 1);
    }
    g_CharacterLock = g_FieldState->characterLock = GET_PARAM_U8(1);
    if (g_CharacterLock == 0) {
        D_800756E8[g_FieldState->pcModelId] = 0;
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncBtlon(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("btlon", 1);
    }
    g_FieldState->battlesDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMpdsp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpdsp", 1);
    }
    g_FieldState->mpdspSet = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMvcam(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mvcam", 1);
    }
    g_FieldState->movieCamDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncGmovr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gmovr", 0);
    }
    g_FieldState->eventCmd = EVTCMD_GAME_OVER;
    g_FieldState->movieCommandState = MOVCMD_IDLE;
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_char_control.c
/////////////////////////////////////////////////

/*
 * Field-script opcode CC: hand player control to another entity.
 *
 * The operand is a script entity id; if that entity has a field model
 * assigned (g_EntityToModel entry != 0xFF) it becomes the new player model.
 */
s32 OpcodeFuncCc(void) {
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cc", 1);
    }
    charId = GET_PARAM_U8(1);
    if (g_EntityToModel[charId] != 0xFF) {
        g_FieldState->pcModelId = g_EntityToModel[charId];
    }
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode CHAR: attach a field model to the current entity.
 *
 * Allocates the next model slot (g_FieldModelCount) for the executing entity,
 * records the mapping in g_EntityToModel and initializes the model with the
 * model id from the opcode operand and the owning entity id.
 */
s32 OpcodeFuncChar(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("char", 1);
    }
    g_EntityToModel[g_CurrentEntity] = g_FieldModelCount++;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].charId = GET_PARAM_U8(1);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].entityId = g_CurrentEntity;
    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_animate.c
/////////////////////////////////////////////////

/*
 * Field-script opcode DFANM: set a model's default (looping) animation.
 *
 * Stores the animation id and playback speed (per-model base speed divided
 * by the speed operand) for the model attached to the executing entity.
 * A model holding the last frame of a script animation (state 3) is
 * released so the new default animation starts playing.
 */
s32 OpcodeFuncDfanm(void) {
    u8 modelIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dfanm", 2);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        D_8008325C[g_EntityToModel[g_CurrentEntity]] = GET_PARAM_U8(1);
        D_80082248[g_EntityToModel[g_CurrentEntity]] =
            D_8009D828[g_EntityToModel[g_CurrentEntity]] / GET_PARAM_U8(2);
        modelIdx = g_EntityToModel[g_CurrentEntity];
        if (D_800756E8[modelIdx] == 3) {
            D_800756E8[modelIdx] = 0;
        }
    }
    PC_INC(3);
    return 1;
}

/*
 * Field-script opcode CCANM: set one of the player animation ids
 * (0: idle, 1: walk, 2: run) used while the player controls a model.
 */
s32 OpcodeFuncCcanm(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ccanm", 3);
    }
    switch (GET_PARAM_U8(3)) {
    case 0:
        g_FieldState->idleAnimId = GET_PARAM_U8(1);
        break;
    case 1:
        g_FieldState->walkAnimId = GET_PARAM_U8(1);
        break;
    case 2:
        g_FieldState->runAnimId = GET_PARAM_U8(1);
        break;
    }
    PC_INC(4);
    return 0;
}

/*
 * Starts the animation requested by the current ANIME-style opcode on the
 * model attached to the executing entity: animation id from the first
 * operand, playback speed from the per-model base speed divided by the
 * second operand, frame counter rewound and the last frame looked up in
 * the animation header of the model's file.
 */
void StartModelAnimation(void) {
    u8 modelIdx;
    u8* anims;
    FieldModelEntry* model;

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].activeAnimId =
        GET_PARAM_U8(1);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed =
        D_8009D828[g_EntityToModel[g_CurrentEntity]] / GET_PARAM_U8(2);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
    modelIdx = g_EntityToModel[g_CurrentEntity];
    model =
        &g_FieldModelData
             ->modelEntries[g_FieldModelLoaderData[modelIdx].modelEntryIndex];
    anims = model->modelData + model->animationOffset;
    g_FieldModels[modelIdx].animLastFrame =
        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
}

/*
 * Field-script opcode ANIME1/ANIME2: play an animation on the entity's
 * model. g_FieldCurrentOpcode distinguishes which opcode invoked the handler:
 * the asynchronous variant (0xAE, ANIME2) marks the model as playing (state 5)
 * and lets the script continue, while ANIME1 blocks (state 2) until the
 * animation system reports completion (state 4), then resets the model to
 * its default animation.
 */
s32 OpcodeFuncAnime(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("anime", 2);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(3);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        StartModelAnimation();
        if (g_FieldCurrentOpcode == 0xAE) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 5;
            PC_INC(3);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
        PC_INC(3);
        return 0;
    }
    return 1;
}

/*
 * Field-script opcode ANIM!1/ANIM!2: like ANIME1/ANIME2 but the model
 * keeps holding the last frame once the animation completes (state 3)
 * instead of returning to its default animation. 0xAE becomes 0xAF and
 * state 5 becomes 6 to tell the two opcode pairs apart.
 */
s32 OpcodeFuncAnimEx(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("anim!", 2);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(3);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        StartModelAnimation();
        if (g_FieldCurrentOpcode == 0xAF) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 6;
            PC_INC(3);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
        PC_INC(3);
        return 0;
    }
    return 1;
}

/* CANIM1/CANIM2 (change animation): play an animation whose id, start frame
 * and end frame all come from the script scaled by a divisor in operand 4,
 * then clamp the end frame to the animation's own length. The D_800756E8
 * state machine and the 0xB0 test are ANIME's, so the outer shape is
 * OpcodeFuncAnime's and the body is StartModelAnimation's with the divisor
 * threaded through. Twin of OpcodeFuncCanmEx.
 *
 * 10 rows out, from an m2c seed that did not compile (`void*` parameters
 * dereferenced as `->unkNN`), so this is the first number the function has
 * ever had. What the rewrite established:
 *   - operand 4 is the only parameter the original caches. Operands 1, 2 and
 *     3 are each re-read through a fresh `lhu` of the PC, which is what
 *     GET_PARAM_U8 spells; operand 4 is loaded once into a register that
 *     stays live across all three divisions. That is the opposite of the
 *     house rule for opcode handlers and the `.s` is unambiguous about it.
 *   - the divisor has to be `s16`. As `u8` gcc masks it (`andi 0xff`) and
 *     divides *unsigned*, which also deletes the three division-overflow
 *     checks: 40 rows and a different program. As `s32` the arithmetic is
 *     right but the allocator ranks its pseudo below the two cse-made ones
 *     for the script base and the PC address, and all three registers rotate
 *     (43). `s16` is what puts them back in a1/a2/a3.
 *   - the frame bound has to be `s16` too (`s32` measures 43, `s16` 36).
 *   - the arm ends with `break`, not `return 1`. With the explicit return gcc
 *     keeps $v0 reserved across the arm and the state constant 2 lands in
 *     $v1; with `break` and the single `return 1` after the switch the
 *     constant gets $v0, which is the documented idiom and is worth 26 rows
 *     here (36 -> 10).
 *
 *   - and the last ten rows: **the third division is computed after the model
 *     lookup, not before it.** `lastFrame = GET_PARAM_U8(3) / divisor;`
 *     written where the other two divisions are -- which is where it reads
 *     naturally, and where m2c put it -- leaves `lastFrame` live across the
 *     whole `&g_FieldModelData->modelEntries[loader[modelIdx].modelEntryIndex]`
 *     lookup. That is one extra quantity competing in that block, and it
 *     flips the order local-alloc processes the two quantities the lookup
 *     itself creates: the loader's entryIdx and the g_FieldModelData pointer
 *     swap $v0 and $a0, and the two clamp copies downstream of them swap
 *     $a0/$v1 for $v1/$a3. Moving the division down to sit immediately above
 *     the clamp that consumes it takes it out of the block and all ten rows
 *     go at once.
 *
 * That last one is worth stating as a rule, because four functions in this
 * file were parked on the same-looking rotation and the earlier notes all
 * blamed the lookup expression: when a diff is nothing but two or three
 * registers rotating inside one basic block, look for a value that is
 * *computed* before the block and *used* after it. It is not in the block's
 * source at all, which is exactly why every spelling of the block measures
 * the same. Everything tried on the expression was inert here and is recorded
 * for the two that are still parked -- see the longer note on OpcodeFuncMove,
 * where no such value exists and the rotation survives. */
s32 OpcodeFuncCanim(void) {
    u8 modelIdx;
    u8* anims;
    FieldModelEntry* model;
    s16 divisor;
    s16 lastFrame;
    s16 maxFrame;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("canim", 4);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(5);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        divisor = GET_PARAM_U8(4);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].activeAnimId =
            GET_PARAM_U8(1);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed =
            D_8009D828[g_EntityToModel[g_CurrentEntity]] / divisor;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame =
            (GET_PARAM_U8(2) / divisor) * 16;
        modelIdx = g_EntityToModel[g_CurrentEntity];
        model = &g_FieldModelData->modelEntries[g_FieldModelLoaderData[modelIdx]
                                                    .modelEntryIndex];
        anims = model->modelData + model->animationOffset;
        maxFrame = *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
        lastFrame = GET_PARAM_U8(3) / divisor;
        if (maxFrame < lastFrame) {
            g_FieldModels[modelIdx].animLastFrame = maxFrame;
        } else {
            g_FieldModels[modelIdx].animLastFrame = lastFrame;
        }
        if (g_FieldCurrentOpcode == 0xB0) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 5;
            PC_INC(5);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
        PC_INC(5);
        return 0;
    }
    return 1;
}

/* CANM!1/CANM!2 (change animation, hold the last frame): OpcodeFuncCanim with
 * three constants changed -- opcode 0xB1 instead of 0xB0, the asynchronous
 * state 6 instead of 5, and state 4 resetting to 3 rather than 0, the same
 * ANIME/ANIM! pairing one opcode down -- plus one real difference: the start
 * frame in operand 2 is *not* divided by the divisor here, only multiplied by
 * 16. Everything else, including the `s16` divisor and frame bound and the
 * `break` at the end of the arm, is CANIM's; read that note for why.
 *
 * Was an m2c seed that did not compile, then 10 rows out with the same two
 * register-naming ties as CANIM, row for row -- and it fell to the same one
 * line, the third division moved below the model lookup. Whatever moves one
 * of these two moves both. */
s32 OpcodeFuncCanmEx(void) {
    u8 modelIdx;
    u8* anims;
    FieldModelEntry* model;
    s16 divisor;
    s16 lastFrame;
    s16 maxFrame;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("canm!", 4);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(5);
        return 0;
    }

    switch (D_800756E8[g_EntityToModel[g_CurrentEntity]]) {
    case 0:
    case 1:
    case 3:
        divisor = GET_PARAM_U8(4);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].activeAnimId =
            GET_PARAM_U8(1);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed =
            D_8009D828[g_EntityToModel[g_CurrentEntity]] / divisor;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame =
            GET_PARAM_U8(2) * 16;
        modelIdx = g_EntityToModel[g_CurrentEntity];
        model = &g_FieldModelData->modelEntries[g_FieldModelLoaderData[modelIdx]
                                                    .modelEntryIndex];
        anims = model->modelData + model->animationOffset;
        maxFrame = *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
        lastFrame = GET_PARAM_U8(3) / divisor;
        if (maxFrame < lastFrame) {
            g_FieldModels[modelIdx].animLastFrame = maxFrame;
        } else {
            g_FieldModels[modelIdx].animLastFrame = lastFrame;
        }
        if (g_FieldCurrentOpcode == 0xB1) {
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 6;
            PC_INC(5);
            return 0;
        }
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 2;
        break;
    case 4:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
        PC_INC(5);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncAnimw(void) {
    u8 modelIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("animw", 0);
    }
    modelIdx = g_EntityToModel[g_CurrentEntity];
    if (modelIdx == 0xFF) {
        PC_INC(1);
        return 0;
    }
    switch (D_800756E8[modelIdx]) {
    case 2:
    case 5:
    case 6:
        return 1;
    case 4:
        D_800756E8[modelIdx] = 0;
        break;
    }
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncAnimb(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("animb", 0);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animLastFrame =
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame >>
            4;
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 3;
    }
    PC_INC(1);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_move.c
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// Start of field_opcode_model_move.c
/////////////////////////////////////////////////

/* MOVE: walk the entity's model to an (x, y) target read from the event
 * memory banks, in 12.4 fixed point. The animation is picked from the map's
 * scale -- three times currentFieldScale against the model's MoveSpeed
 * chooses run (2) over walk (1) -- and only restarted when it actually
 * changes, which is what the two `goto started` arms are for. The tail is the
 * per-frame half: state 1 means still walking, state 2 means arrived, and the
 * opcode only advances the PC on arrival.
 *
 * 14 rows out, from an m2c seed that did not compile. What the rewrite
 * established:
 *   - the ActionState test is a `switch`, not two `if`s. The target loads the
 *     halfword once and branches `beq 1` / `beq 2` / `j default`, and cse
 *     supplies the constant 1 out of the register the mode test just proved,
 *     which is expand_end_case's compare chain. Two `if`s cross-jump the
 *     first arm straight into the shared epilogue instead: 27 rows and one
 *     insertion against 21.
 *   - the two `FieldEntity*` locals are two variables, not one reused. They
 *     describe different things (the model being retargeted, then the model
 *     being polled) and merging them stretches one live range across the
 *     animation block, which costs the pointer its register and 7 rows.
 *
 * The residue is one register-naming tie, and it is the *same* one CANIM and
 * CANM! are parked on: the model-entry lookup allocates modelIdx / entryIdx /
 * g_FieldModelData as a0 / a1 / v1 in the target and a1 / v1 / a0 in ours, a
 * three-cycle that then decides the two halves of `anims` as well. The
 * expression is byte-for-byte the one in StartModelAnimation, which matches,
 * so the spelling is not the problem -- the tie is broken by what else is
 * live in the block. Measured and inert: modelIdx as u8/s16/s32, a named
 * entryIdx local, declaration order. Inlining `anims` costs 8 rows.
 *
 * FieldUpdateAnimationState had a residue that read *identically* -- the same
 * three registers in the same rotation -- and it fell to `s32 entryIdx` split
 * into its own statement plus `s32 modelIdx`. Applied here, to CANIM, to
 * CANM! and to FieldMoveToEntityUpdate, both levers measure to the row, as do
 * an `s32 lastFrame` local for the `- 1`, replacing modelIdx with the repeated
 * `g_EntityToModel[g_CurrentEntity]`, and every combination of the four. So
 * this is *not* that tie; it only looks like it. Whatever breaks it is
 * outside the lookup.
 *
 * CANIM and CANM! then landed, and they confirm that diagnosis exactly: their
 * rotation was caused by `lastFrame = GET_PARAM_U8(3) / divisor;` being
 * computed above the lookup and consumed below it, so one extra quantity was
 * live across the block; moving that one line down to the clamp matched both
 * functions outright. MOVE and MOVA have no such value -- nothing they compute
 * before the lookup survives past it -- which is why the same rotation is
 * still here, and it means the remaining lever is not another spelling of the
 * lookup either.
 *
 * The rotation is now `modelIdx / entryIdx / g_FieldModelData` as
 * $a0 / $a1 / $v1 in the target against $a1 / $v1 / $a0 in ours, with the two
 * halves of `anims` following as $a1/$a0 against $a0/$a1. Every quantity here
 * reuses a register that has just died -- $a0 from g_CurrentEntity, $a1 from
 * the last FieldEventReadMemoryS16 argument, $v1 from g_FieldModelLoaderData
 * -- so both allocations are locally sensible and only the processing order
 * differs. Measured on top of everything above, all still 14: `anims` with
 * its two operands swapped; `model` inlined into `anims`; the modelIdx local
 * repeated as the full expression in the lookup only, in the animLastFrame
 * store only, or in both; and modelIdx declared first. Moving statements the
 * way CANIM's fix does is much worse, because here they are stores and not a
 * dead value -- the animSpeed store below the lookup costs 36 rows and 11
 * insertions, animCurrentFrame 34/10, both 59/7, and hoisting modelIdx to
 * just after the 0xFF guard 47/3. Inlining `anims` is 22/2 and repeating the
 * expression inside the lookup alone 18/5.
 *
 * That RTL has now been read (`-dl` on an unparked field4.c), and it turns the
 * tie into arithmetic. All three quantities are block-local, so this is
 * `local_alloc`'s `block_alloc`, not `global_alloc`, and its ranking is
 * `QTY_CMP_PRI = floor_log2(n_refs) * n_refs * size / (death - birth)`. The
 * dump gives:
 *
 *   reg 196  modelIdx            4 refs / 13 insns   ->  2*4/13 = 0.62
 *   reg 202  entryIdx            3 refs /  4 insns   ->  1*3/4  = 0.75
 *   reg 195  g_FieldModelData    2 refs /  6 insns   ->  1*2/6  = 0.33
 *
 * so ours allocates entryIdx, then g_FieldModelData, then modelIdx, taking
 * $v1/$a0/$a1 off the free list in that order. The target's assignment --
 * g_FieldModelData $v1, modelIdx $a0, entryIdx $a1 -- is the exact **reverse**
 * order, so the whole three-way ranking has to invert. `n_refs` is fixed by
 * the arithmetic (entryIdx has three references because `* 36` decomposes into
 * `x*8 + x` then `<< 2`), which leaves `live_length` as the only term, and
 * every way of stretching one costs an instruction: entryIdx read above the
 * two animation stores is 40/4, above the whole animation block 43/4, modelIdx
 * read above the stores 37/3, and both together 48/8, against 14/0.
 *
 * 14 -> 10, and the step is decomp-permuter's `perm_ins_block`: the whole
 * animation-restart block duplicated into both arms of `if (g_EntityToModel)
 * { ... } else { ... }`. The condition is the address of an array, so it is
 * always true and pure; jump_optimize and flow delete the test and one copy,
 * the length is unchanged, and what survives is a different block structure
 * for `anims` -- whose two halves now land on the target's $a1/$a0. This is
 * the `AddBackgroundToRender` dead-conditional idiom, and it is worth noting
 * that the permuter's score moved only 85 -> 55 for it, so it would have been
 * easy to discard: re-measure every output with variant_eval.
 *
 * The remaining 10 are the three-way rotation above, unchanged, and the
 * allocno arithmetic in this note still applies to it: neither term of
 * `QTY_CMP_PRI` is reachable from C without emitting an instruction. What the
 * dead conditional shows is that the *block structure* around the lookup is
 * reachable even when the expression is not, so that is where the next
 * search should look. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncMove);
#else
s32 OpcodeFuncMove(void) {
    FieldEntity* entity;
    FieldEntity* moving;
    u8 modelIdx;
    u8* anims;
    FieldModelEntry* model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("move", 5);
    }

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(6);
        return 0;
    }

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        (FieldEventReadMemoryS16(1, 2) << 16) >> 4;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        (FieldEventReadMemoryS16(2, 4) << 16) >> 4;

    entity = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (g_FieldState->currentFieldScale * 3 < entity->MoveSpeed) {
        if (entity->activeAnimId == 2) {
            goto started;
        }
        entity->activeAnimId = 2;
    } else {
        if (entity->activeAnimId == 1) {
            goto started;
        }
        entity->activeAnimId = 1;
    }
    if (g_EntityToModel) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed = 0x10;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
        modelIdx = g_EntityToModel[g_CurrentEntity];
        model = &g_FieldModelData->modelEntries[g_FieldModelLoaderData[modelIdx]
                                                    .modelEntryIndex];
        anims = model->modelData + model->animationOffset;
        g_FieldModels[modelIdx].animLastFrame =
            *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
    } else {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed = 0x10;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
        modelIdx = g_EntityToModel[g_CurrentEntity];
        model = &g_FieldModelData->modelEntries[g_FieldModelLoaderData[modelIdx]
                                                    .modelEntryIndex];
        anims = model->modelData + model->animationOffset;
        g_FieldModels[modelIdx].animLastFrame =
            *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
    }

started:
    D_800756E8[g_EntityToModel[g_CurrentEntity]] = 1;
    moving = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (moving->scriptedMoveMode == 1) {
        switch (moving->ActionState) {
        case 1:
            return 1;
        case 2:
            moving->scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
            PC_INC(6);
            return 0;
        }
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    return 1;
}
#endif

/* FMOVE (0xAD): walk the current entity to a target point, letting it turn to
 * face the way it is going -- it clears DirLock, where CMOVE below sets it.
 * If a move is in flight (scriptedMoveMode 1) it polls: ActionState 1 means
 * still walking (return 1), 2 means arrived (clear the mode and step the PC),
 * and anything else falls through to the same block the not-in-flight path
 * reaches, which arms a fresh move.
 *
 * The parked body this replaces was three separate programs' worth of wrong,
 * and none of it was visible as anything but register noise: it read the two
 * coordinates from (2,4)/(3,6) instead of (1,2)/(2,4), cleared MoveDirAdd
 * (0x35) instead of DirLock (0x37), and -- the 25-row one -- wrote the
 * ActionState dispatch as `if (state == 1) state = 2; else if (state == 2)
 * ...' with its own `return 1', where the target sets nothing in the first
 * arm and sends its *default* into the shared tail block. That is the
 * "block reached by both a failed test and a switch default is a
 * fallthrough" idiom: two predecessors mean cse knows nothing on entry, which
 * is why the target rebuilds the whole g_FieldModels *0x84 index there
 * instead of reusing the base it already had. Read the branch structure off
 * the target before blaming that rebuild on register allocation. */
s32 OpcodeFuncFmove(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fmove", 5);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(6);
        return 0;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        (s32)FieldEventReadMemoryS16(1, 2) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        (s32)FieldEventReadMemoryS16(2, 4) << 12;
    if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode == 1) {
        switch (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState) {
        case 1:
            return 1;
        case 2:
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode =
                0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
            PC_INC(6);
            return 0;
        }
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    return 1;
}

/* CMOVE (0xA9): walk the current entity to a target point holding its facing
 * -- it sets DirLock, where FMOVE below clears it, and clears it again on
 * arrival. Same three-way poll as FMOVE: ActionState 1 keeps waiting, 2
 * finishes and steps the PC, anything else falls into the shared block that
 * arms a fresh move. Landed from FMOVE's reading of the target; see that
 * function's note for why the default has to reach the tail block rather than
 * return on its own. */
s32 OpcodeFuncCmove(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cmove", 5);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(6);
        return 0;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        (s32)FieldEventReadMemoryS16(1, 2) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        (s32)FieldEventReadMemoryS16(2, 4) << 12;
    if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode == 1) {
        switch (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState) {
        case 1:
            return 1;
        case 2:
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode =
                0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
            PC_INC(6);
            return 0;
        }
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    return 1;
}

s32 OpcodeFuncFcfix(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fcfix", 1);
    }
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock =
            GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

/* JUMP (0xC0): make the current entity jump to a target over a number of
 * frames. ActionState 1 keeps polling (return 1), 2 finishes -- and *returns*,
 * stepping the PC past the opcode -- and anything else falls through to arm a
 * fresh jump. The parked body had the ActionState 2 arm fall through into the
 * arming code instead of returning, which is a different program and was worth
 * 17 of the 23 rows.
 *
 * The last row was the `beq' to the ActionState 1 arm having an unfilled delay
 * slot: written as two `if's the following `li v0,2' is live on the taken path
 * (v0 is the return register and that path returns 1), so reorg will not steal
 * it. Written as a `switch', `expand_end_case' materialises the same constant
 * as its own compare setup and the slot is filled. Same two arms either way --
 * see OpcodeFuncFmove above, which needs the switch for a different reason. */
s32 OpcodeFuncJump(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("jump", 8);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(11);
        return 0;
    }
    if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode ==
        SMODE_JUMP) {
        switch (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState) {
        case 1:
            return 1;
        case 2:
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode =
                SMODE_NONE;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
            PC_INC(11);
            return 0;
        }
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode =
        SMODE_JUMP;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        (s32)FieldEventReadMemoryS16(1, 3) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        (s32)FieldEventReadMemoryS16(2, 5) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndI =
        FieldEventReadMemoryS16(3, 7);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveSteps =
        FieldEventReadMemoryS16(4, 9);
    return 1;
}

/*
 * Field-script opcode LADER: send a model up or down a ladder or climb path.
 *
 * The direction operand picks the climb mode (4 = one pair of animations,
 * 5 = the other) and which end of it the model starts from. The three
 * coordinate operands are the destination in 1/16th units, and a fourth
 * names the walk mesh triangle it lands on. While a climb is already running
 * the opcode blocks on the model's action state, then clears it and steps
 * past its 15 bytes.
 */
s32 OpcodeFuncLader(void) {
    s32 mode;
    u8 modelIdx;
    FieldModelEntry* entry;
    u8* anims;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lader", 8);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(0xF);
        return 0;
    }
    mode = g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode;
    if (mode < 6) {
        if (mode >= 4) {
            switch (
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState) {
            case 1:
                return 1;
            case 2:
                g_FieldModels[g_EntityToModel[g_CurrentEntity]]
                    .scriptedMoveMode = 0;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
                PC_INC(0xF);
                return 0;
            }
        }
    }
    switch (GET_PARAM_U8(0xB)) {
    case 0:
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 4;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 0;
        break;
    case 1:
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 4;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 1;
        break;
    case 2:
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 5;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 0;
        break;
    case 3:
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 5;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 1;
        break;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        FieldEventReadMemoryS16(1, 3) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        FieldEventReadMemoryS16(2, 5) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndZ =
        FieldEventReadMemoryS16(3, 7) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndI =
        FieldEventReadMemoryS16(4, 9);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].activeAnimId =
        GET_PARAM_U8(0xC);
    modelIdx = g_EntityToModel[g_CurrentEntity];
    g_FieldModels[modelIdx].animSpeed =
        D_8009D828[modelIdx] / GET_PARAM_U8(0xE);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
    modelIdx = g_EntityToModel[g_CurrentEntity];
    entry =
        &g_FieldModelData
             ->modelEntries[g_FieldModelLoaderData[modelIdx].modelEntryIndex];
    anims = entry->modelData + entry->animationOffset;
    g_FieldModels[modelIdx].animLastFrame =
        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
    D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir = GET_PARAM_U8(0xD);
    return 1;
}

s32 OpcodeFuncPmova(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmova", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = g_CharIdToEntity[partyId];
    }
    return FieldMoveToEntityUpdate(actorId);
}

s32 OpcodeFuncMova(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mova", 1);
    }
    return FieldMoveToEntityUpdate(GET_PARAM_U8(1));
}

/* MOVA/PMOVA: retarget the current entity's scripted move at another entity,
 * every frame, so the mover follows a moving target. The destination is the
 * target's live position and its solid radius becomes the stop distance; the
 * rest is OpcodeFuncMove's state machine, which is why the walk/run choice
 * and the animation restart are the same code. Returns the dispatcher's
 * "opcode consumed" flag, so MOVA and PMOVA pass it through rather than being
 * void.
 *
 * 14 rows out, from a hand-written body that did not compile and measured 154
 * once it did. What the rewrite established:
 *   - it returns 1 from the "start the move" tail, not 0 (the target's last
 *     insn before the epilogue is `li v0,1`), and the two callers pass it on.
 *   - offset 0x68 is ActionArg, not MoveEndI: the stop distance is written
 *     where OpcodeFuncMove clears it.
 *   - the walk/run test reads FieldState.currentFieldScale (0x10), not
 *     currentMovieFrame.
 *   - every model access is the full `g_FieldModels[g_EntityToModel[
 *     g_CurrentEntity]]` expression, except the run around the
 *     scriptedMoveMode/ActionState dispatch which is one `FieldEntity*`
 *     local. The hand-written body cached `cur` and `target` across the whole
 *     function and that alone was most of its 154 rows.
 *   - the two 0xFF guards are one `||` with an early `PC_INC(2); return 0;`,
 *     and the ActionState-2 arm duplicates that tail; cross-jumping keeps the
 *     arm's copy and the guard jumps into it, which is what the target has.
 *     (Contrast the turn opcodes, where the tail has to be the function's
 *     fall-through end -- read the target for which one it wants.)
 *
 * The 14 rows are the model-entry allocation tie, row for row the same one
 * OpcodeFuncMove is parked on -- read that note, which now carries the whole
 * measurement history. CANIM and CANM! shared it and landed once the value
 * that was live across their lookup was moved out of the block; MOVE and MOVA
 * have no such value, so two of the original four remain. Also measured here
 * and inert: `g_FieldModelData->modelEntries + idx` instead of `&...[idx]`,
 * and a local for `g_FieldModelData`; a local for `modelEntries` costs 6 more
 * rows. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldMoveToEntityUpdate);
#else
s32 FieldMoveToEntityUpdate(s32 targetEntityId) {
    FieldEntity* moving;
    FieldModelEntry* model;
    u8* anims;
    u8 modelIdx;

    if (g_EntityToModel[g_CurrentEntity] == 0xFF ||
        g_EntityToModel[targetEntityId & 0xFF] == 0xFF) {
        PC_INC(2);
        return 0;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg =
        g_FieldModels[g_EntityToModel[targetEntityId & 0xFF]].SolidRange;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        g_FieldModels[g_EntityToModel[targetEntityId & 0xFF]].PosX;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        g_FieldModels[g_EntityToModel[targetEntityId & 0xFF]].PosY;

    moving = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (moving->scriptedMoveMode == 1) {
        switch (moving->ActionState) {
        case 1:
            if (g_FieldState->currentFieldScale * 3 < moving->MoveSpeed) {
                if (moving->activeAnimId == 2) {
                    goto started;
                }
                moving->activeAnimId = 2;
            } else {
                if (moving->activeAnimId == 1) {
                    goto started;
                }
                moving->activeAnimId = 1;
            }
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].animSpeed = 0x10;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame =
                0;
            modelIdx = g_EntityToModel[g_CurrentEntity];
            model =
                &g_FieldModelData->modelEntries[g_FieldModelLoaderData[modelIdx]
                                                    .modelEntryIndex];
            anims = model->modelData + model->animationOffset;
            g_FieldModels[modelIdx].animLastFrame =
                *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId * 16] - 1;
        started:
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 1;
            return 1;
        case 2:
            moving->scriptedMoveMode = 0;
            D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
            PC_INC(2);
            return 0;
        }
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    return 1;
}
#endif

void OpcodeFuncDira(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dira", 1);
    }
    FieldEventSetDirByActorId(GET_PARAM_U8(1));
}

void OpcodeFuncPdira(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pdira", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = g_CharIdToEntity[partyId];
    }
    FieldEventSetDirByActorId(actorId);
}

/* Face the current entity towards another entity: read both models' fixed
 * point positions, compute the direction with FieldEntityDirByVec, and snap
 * the current entity's Dir to it, cancelling any turn in progress. This is the
 * whole of the DIRA/PDIRA opcode, not a helper -- it steps the script PC past
 * the two bytes itself and returns 1, or 0 when either entity has no model.
 *
 * Three things the parked body had wrong, worth 150 rows between them and all
 * of them readable off the target: the parameter is `s16', not `u8' (the
 * `sll'/`sra' pair ahead of the g_EntityToModel index is the sign extension);
 * the PC_INC(2) and the return value live here rather than in the callers;
 * and when the two positions share an x and a y the source nudges from.vx by
 * one before calling FieldEntityDirByVec, which is the pair of `bne's guarding
 * the `addiu v0,t0,0x1'. A degenerate direction vector would otherwise make
 * the callee's atan2 meaningless. Every model access is the full indexed
 * expression -- caching g_EntityToModel[g_CurrentEntity] in a local is what
 * made this look like a register-allocation problem. */
s32 FieldEventSetDirByActorId(s16 actorId) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;

    if (g_EntityToModel[g_CurrentEntity] == 0xFF ||
        g_EntityToModel[actorId] == 0xFF) {
        PC_INC(2);
        return 0;
    }
    from.vx = g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX >> 12;
    from.vy = g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY >> 12;
    from.vz = g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ >> 12;
    to.vx = g_FieldModels[g_EntityToModel[actorId]].PosX >> 12;
    to.vy = g_FieldModels[g_EntityToModel[actorId]].PosY >> 12;
    to.vz = g_FieldModels[g_EntityToModel[actorId]].PosZ >> 12;
    if (from.vx == to.vx && from.vy == to.vy) {
        from.vx = from.vx + 1;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir =
        FieldEntityDirByVec(&from, &to, &sqrDist);
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
    PC_INC(2);
    return 1;
}

s32 OpcodeFuncTura(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tura", 3);
    }
    return FieldEntityTurnToEntity(GET_PARAM_U8(1));
}

s32 OpcodeFuncPtura(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ptura", 3);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = g_CharIdToEntity[partyId];
    }
    return FieldEntityTurnToEntity(actorId);
}

/*
 * Turn the current entity's model to face another entity: snapshot the
 * current direction, derive the target facing from the position delta with
 * FieldEntityDirByVec, and hand the turn to the state machine. Operand 3
 * picks the direction the same way TURNR's does -- 0 clockwise, 1
 * anticlockwise, 2 whichever way is shorter. TurnType 3 means the turn
 * finished, and only then does the PC advance; the 0/1 this returns is the
 * dispatcher's "opcode consumed" flag, which is why TURA and PTURA pass it
 * through rather than being void.
 */
s32 FieldEntityTurnToEntity(s16 actorId) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    FieldEntity* entity;
    FieldEntity* snapshot;
    FieldEntity* turning;
    FieldEntity* stepping;
    s16 delta;
    s16 dist;

    if (g_EntityToModel[g_CurrentEntity] == 0xFF ||
        g_EntityToModel[actorId] == 0xFF) {
        PC_INC(4);
        return 0;
    }
    entity = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (entity->TurnType == 3) {
        entity->TurnType = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
        PC_INC(4);
        return 0;
    }
    if (entity->TurnStep == 0 || entity->TurnType != 2 ||
        entity->TurnSteps != GET_PARAM_U8(2)) {
        snapshot = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
        snapshot->TurnStart = snapshot->Dir;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = 2;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps =
            GET_PARAM_U8(2);
        from.vx = g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX >> 12;
        from.vy = g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY >> 12;
        from.vz = g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ >> 12;
        to.vx = g_FieldModels[g_EntityToModel[actorId]].PosX >> 12;
        to.vy = g_FieldModels[g_EntityToModel[actorId]].PosY >> 12;
        to.vz = g_FieldModels[g_EntityToModel[actorId]].PosZ >> 12;
        if (from.vx == to.vx) {
            if (from.vy == to.vy) {
                from.vx = from.vx + 1;
            }
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnEnd =
            FieldEntityDirByVec(&from, &to, &sqrDist) & 0xFF;
        switch (GET_PARAM_U8(3)) {
        case 2:
            turning = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
            delta = turning->TurnEnd - turning->TurnStart;
            dist = delta;
            if (delta < 0) {
                dist = ~delta + 1;
            }
            if (dist >= 0x81) {
                if (turning->TurnEnd > turning->TurnStart) {
                    turning->TurnEnd = turning->TurnEnd - 0x100;
                } else {
                    turning->TurnEnd = turning->TurnEnd + 0x100;
                }
            }
            break;
        case 1:
            stepping = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
            if (stepping->Dir < stepping->TurnEnd) {
                stepping->TurnEnd = stepping->TurnEnd - 0x100;
            }
            break;
        case 0:
            stepping = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
            if (stepping->TurnEnd < stepping->Dir) {
                stepping->TurnEnd = stepping->TurnEnd + 0x100;
            }
            break;
        }
    }
    return 1;
}

extern u8 D_800722C4;
extern /*?*/ s32 D_800831FC;
extern u8 D_8009D820;

/* OFSTD/OFSTL/OFSTC: start a positional offset on the current entity's model.
 * The three names are one handler: operand 3 is the interpolation mode and
 * also selects which name the debug print uses. Mode 0 snaps the offset to
 * its target immediately; the others record the current offset as the start
 * of an interpolation. The PC always advances, so this returns 0
 * unconditionally.
 *
 * 11 rows out with 3 insertions, from an m2c seed that did not compile, and
 * that is the first shot -- the body is written the way the turn opcodes
 * wanted: everything inside `if (g_EntityToModel[g_CurrentEntity] != 0xFF)`
 * with `PC_INC(0xC); return 0;` as the fall-through end, the mode read once
 * into a local and both stored and tested, and every model access spelled as
 * the full indexed expression rather than through a pointer (m2c's six
 * separate `temp_v0` pointers are cse re-deriving the base after each store,
 * not six source variables).
 *
 * The last 11 rows were the mode read, and they were not scheduling at all --
 * they were the order gcc expands a store. Written as two statements,
 * `ofsType = GET_PARAM_U8(3);` then `...OfsType = ofsType;`, the value is a
 * whole statement of its own and is emitted first: the PC load and the
 * g_FieldScripts base come out ahead of the `g_EntityToModel` lookup that
 * addresses the store. Written as one chained assignment,
 * `g_FieldModels[...].OfsType = ofsType = GET_PARAM_U8(3);`,
 * `expand_assignment` computes the destination address *before* it expands
 * the right-hand side, so the lookup goes first and the PC read lands
 * immediately above the `sb` -- which is exactly the target's block. Same
 * instructions, same count, one statement. `s32 ofsType` is byte-identical
 * to the `u8`, so only the chaining matters.
 *
 * Measured and worse: storing `GET_PARAM_U8(3)` directly with the parameter
 * re-read for the test, and the same with a separate local assigned after the
 * store (31/13 both -- two reads of the PC, and cse will not fold them across
 * the store). Measured and identical: passing the string literals to
 * DebugPrintOpcode directly instead of through a `char*` local. */
s32 OpcodeFuncOfstd(void) {
    u8 ofsType;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            switch (GET_PARAM_U8(3)) {
            case 0:
                DebugPrintOpcode("ofstd", 5);
                break;
            case 1:
                DebugPrintOpcode("ofstl", 5);
                break;
            case 2:
                DebugPrintOpcode("ofstc", 5);
                break;
            }
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStep = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetSteps =
            FieldEventReadMemoryS16(4, 0xA);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetEndX =
            FieldEventReadMemoryS16(1, 4);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetEndY =
            FieldEventReadMemoryS16(2, 6);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetEndZ =
            FieldEventReadMemoryS16(3, 8);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].OfsType = ofsType =
            GET_PARAM_U8(3);
        if (ofsType != 0) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStartX =
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetX;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStartY =
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetY;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStartZ =
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetZ;
        } else {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetX =
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetEndX;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetY =
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetEndY;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetZ =
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetEndZ;
        }
    }
    PC_INC(0xC);
    return 0;
}

/* Block until this entity's offset animation finishes. OfsType 3 means the last
 * step ran, so clear it and fall through; 0 means there was never one. */
s32 OpcodeFuncOfstw(void) {
    FieldEntity* model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ofstw", 0);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(1);
        return 0;
    }
    model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (model->OfsType != 0 && model->OfsType != 3) {
        return 1;
    }
    model->OfsType = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetStep = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].OffsetSteps = 0;
    PC_INC(1);
    return 0;
}

/* Block until this entity's turn finishes. Returning 1 without advancing the
 * PC re-runs the opcode next frame; TurnType 3 means the turn just completed,
 * so clear it and fall through.
 *
 * Every early exit writes its own `PC_INC(1); return 0;` -- three copies of
 * the tail in the source. gcc's cross-jumping then merges the common *suffix*
 * only, from the `%hi(g_FieldScriptPC)` on, and the two early copies reuse the
 * `g_CurrentEntity` value that is still live in $a0 while the copy after the
 * stores reloads it (the stores may alias). Written the other way -- one
 * trailing tail with the body wrapped in `if (idx != 0xFF)` -- there is a
 * single tail with a single reload, and the two branches that should enter the
 * suffix at its second instruction enter it at its first: 26 rows. Duplicating
 * the tail is not the same as defeating cross-jumping; it is how you choose
 * *where* the merge point lands. */
s32 OpcodeFuncTurnw(void) {
    FieldEntity* model;

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(1);
        return 0;
    }
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("turnw", 0);
    }
    model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (model->TurnType == 0) {
        PC_INC(1);
        return 0;
    }
    if (model->TurnType != 3) {
        return 1;
    }
    model->TurnType = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
    PC_INC(1);
    return 0;
}

/* TURN/TURNC: turn the current entity's model to an absolute direction read
 * from the event memory banks. Sibling of TURNR (which takes a relative
 * direction) and of FieldEntityTurnToEntity (which derives one from another
 * entity's position); the state machine and the 0/1 return are the same in
 * all three.
 *
 * 11 rows out, from 29. Two structural findings, both of which also moved
 * TURNR:
 *   - the whole body lives inside `if (g_EntityToModel[g_CurrentEntity] !=
 *     0xFF) { ... return 1; }` with `PC_INC(6); return 0;` as the function's
 *     fall-through end. That is where the target parks the single merged
 *     PC-advance block -- after every `return 1` path, entered by a `j` from
 *     the TurnType-3 arm with g_CurrentEntity reloaded in the arm itself.
 *     Written as two early returns gcc keeps the copy at the arm instead
 *     (28/7); written as a `goto` to a label after `return 1` the label's two
 *     jump predecessors go opaque to cse and the reload moves inside the tail
 *     (29/2, which is where this function sat).
 *   - the restart test's early exit is `goto done;` with a single `return 1`
 *     at the end, not its own `return 1`: the target materialises the 1 in a
 *     branch delay slot and again at a shared block, which two independent
 *     returns cannot produce. 13 -> 11.
 *   Also measured and worse: `FieldEntity*` locals instead of repeating the
 *   indexed expression (109 -- the opposite of what FieldEntityTurnToEntity
 *   wants, so read the target), `s32 dir` (14), the TurnEnd comparison
 *   reversed (12).
 *
 * The last 11 rows were block layout, not reorg, though they read as reorg:
 * the target's `done:` block sits *after* the shared `PC_INC(6); return 0;`
 * tail, not before it. Written as `... done: return 1; } PC_INC(6); return 0;`
 * the `return 1` block is laid out first, so it is the fall-through of the
 * store block and the `goto done` branch lands on a `j` -- and the two delay
 * slots reorg wants to fill (the last `&&` test's, with a duplicated
 * `li v0,1`, and the preceding `bne`'s, with the `sll` from the block below)
 * both stay empty. Closing the `if` with an explicit `goto done;` and putting
 * `done: return 1;` after the PC_INC tail puts the blocks in the target's
 * order, and reorg does the rest: the store block ends in `j` with the `sh`
 * duplicated into its delay slot, and the early exit jumps straight to the
 * epilogue with `li v0,1` duplicated into its own.
 *
 * This is the third variation on where an opcode handler's tail goes, and the
 * three are not interchangeable -- OpcodeFuncTurnw wants the tail duplicated
 * at every early return, this one wants two tails in a specific *order*, and
 * FieldMoveToEntityUpdate wants one tail that the guard jumps into. Read the
 * target's block addresses before choosing. */
s32 OpcodeFuncTurn(void) {
    s16 dir;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            switch (GET_PARAM_U8(5)) {
            case 1:
                DebugPrintOpcode("turn", 5);
                break;
            case 2:
                DebugPrintOpcode("turnc", 5);
                break;
            }
        }
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType == 3) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
            PC_INC(6);
            return 0;
        }
        dir = FieldEventReadMemoryS16(2, 2);
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep != 0 &&
            (s16)dir ==
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnEnd &&
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType ==
                GET_PARAM_U8(5) &&
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps ==
                GET_PARAM_U8(4)) {
            goto done;
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStart =
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType =
            GET_PARAM_U8(5);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps =
            GET_PARAM_U8(4);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnEnd = dir;
        goto done;
    }
    PC_INC(6);
    return 0;
done:
    return 1;
}

/* TURNR/TURNL/TRNRC/TRNLC: start (or restart) a turn on the current entity's
 * model. Operand 5 selects the turn kind and operand 3 the direction --
 * 0 clockwise, 1 anticlockwise, 2 whichever way is shorter -- which is why
 * one handler prints four different opcode names. The turn is only restarted
 * when the kind or the step count changed, so a script can spam the opcode
 * every frame; TurnType 3 means the turn system reported completion, and only
 * then does the PC advance.
 *
 * 18 rows out with 1 insertion, from an m2c seed that did not compile.
 * FieldEntityTurnToEntity is the same function with the target read from an
 * actor id instead of the script, and it MATCHES -- everything that got it
 * there is applied here:
 *   - both the debug-name pick and the direction dispatch are `switch`es, and
 *     the direction arms are written in source order **2, 1, 0**. m2c prints
 *     case bodies in address order and that is the order the original wrote
 *     them; 0,1,2 measures 69 rows and 0,2,1 measures 53.
 *   - the four uses of `&g_FieldModels[g_EntityToModel[g_CurrentEntity]]` are
 *     four separate `FieldEntity*` locals, not one reused: the TurnType
 *     checks, the TurnStart snapshot, the shortest-way arm, and the two
 *     single-step arms. On FieldEntityTurnToEntity merging just two of them
 *     cost 30 rows and merging three cost 42 -- splitting all four is what
 *     took it from 42 to MATCH.
 *   - `delta < 0`, not `delta & 0x8000`, and `TurnEnd > TurnStart`, not
 *     `TurnStart < TurnEnd`: gcc evaluates a comparison's operands in source
 *     order and the target reads TurnEnd first.
 *
 *   - the whole body lives inside `if (g_EntityToModel[g_CurrentEntity] !=
 *     0xFF) { ... return 1; }` with `PC_INC(6); return 0;` as the function's
 *     fall-through end. That is the only shape that puts the merged
 *     PC-advance block where the target has it -- after every `return 1`
 *     path, entered by a `j` from the TurnType-3 arm which reloads
 *     g_CurrentEntity itself. Two early returns leave the block at the arm
 *     (35/7); a `goto advance` label after `return 1` moves the block but
 *     makes cse lose g_CurrentEntity, so the reload lands inside the tail
 *     (37/3). Worth 35/7 -> 18/1. Also measured: caching g_CurrentEntity in
 *     a local (94/23).
 *
 *   - the `return 1` and the `PC_INC(6); return 0;` are two blocks and their
 *     *order* matters: `done: return 1;` goes after the PC_INC tail, reached
 *     by an explicit `goto done;` that closes the `if`. Laid out the other way
 *     round -- which is what writing `return 1;` inside the `if` gives -- the
 *     store block falls straight into the return and the two delay slots reorg
 *     wants stay empty. 18/1 -> 2. Same lever as OpcodeFuncTurn; see its note.
 *   - the debug-name pick is an `if`/`else`, not `name = "turnr"; if (...)
 *     name = "turnl";`. Both compile to the identical instructions, so this is
 *     invisible in a register diff -- what it decides is the order the four
 *     string literals land in `.rodata`, because gcc calls
 *     `output_constant_def` as it expands each STRING_CST and an `if`/`else`
 *     expands its *true* arm first. The target's pool is turnl, turnr, trnlc,
 *     trnrc: the conditional name first, which only the `if`/`else` (or a
 *     ternary, byte-identical) produces. The straight-line form puts turnr
 *     first, every `%lo` in the function is then 8 bytes off, and -- this is
 *     the trap -- `checkfn.py` still reports the function as 2 rows out from
 *     "register naming" while `make build` fails the whole overlay's SHA-1.
 *     Defaulting to the l-name and testing `== 0` gets the pool right and the
 *     branch polarity wrong: 4 rows. */
s32 OpcodeFuncTurnr(void) {
    FieldEntity* entity;
    FieldEntity* snapshot;
    FieldEntity* turning;
    FieldEntity* stepping;
    char* name;
    s16 delta;
    s16 dist;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            switch (GET_PARAM_U8(5)) {
            case 1:
                if (GET_PARAM_U8(3) != 0) {
                    name = "turnl";
                } else {
                    name = "turnr";
                }
                DebugPrintOpcode(name, 5);
                break;
            case 2:
                if (GET_PARAM_U8(3) != 0) {
                    name = "trnlc";
                } else {
                    name = "trnrc";
                }
                DebugPrintOpcode(name, 5);
                break;
            }
        }

        entity = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
        if (entity->TurnType == 3) {
            entity->TurnType = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
            PC_INC(6);
            return 0;
        }
        if (entity->TurnStep == 0 || entity->TurnType != GET_PARAM_U8(5) ||
            entity->TurnSteps != GET_PARAM_U8(4)) {
            snapshot = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
            snapshot->TurnStart = snapshot->Dir;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType =
                GET_PARAM_U8(5);
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps =
                GET_PARAM_U8(4);
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnEnd =
                FieldEventReadMemoryU8(2, 2) & 0xFF;
            switch (GET_PARAM_U8(3)) {
            case 2:
                turning = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
                delta = turning->TurnEnd - turning->TurnStart;
                dist = delta;
                if (delta < 0) {
                    dist = ~delta + 1;
                }
                if (dist >= 0x81) {
                    if (turning->TurnEnd > turning->TurnStart) {
                        turning->TurnEnd = turning->TurnEnd - 0x100;
                    } else {
                        turning->TurnEnd = turning->TurnEnd + 0x100;
                    }
                }
                break;
            case 1:
                stepping = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
                if (stepping->Dir < stepping->TurnEnd) {
                    stepping->TurnEnd = stepping->TurnEnd - 0x100;
                }
                break;
            case 0:
                stepping = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
                if (stepping->TurnEnd < stepping->Dir) {
                    stepping->TurnEnd = stepping->TurnEnd + 0x100;
                }
                break;
            }
        }
        goto done;
    }
    PC_INC(6);
    return 0;
done:
    return 1;
}

/* Snap this entity to a facing, cancelling any turn in progress. Returns 1 when
 * the entity actually has a model, unlike most opcodes. */
s32 OpcodeFuncDir(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("dir", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir =
            FieldEventReadMemoryU8(2, 2);
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
        PC_INC(3);
        return 1;
    }
    PC_INC(3);
    return 0;
}

/* SLIDR: set this entity's collision radius. The script value is in map units,
 * so it is scaled by the field's own scale and divided back down by 512. */
s32 OpcodeFuncSlidr(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("slidR", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidRange =
            (FieldEventReadMemoryU8(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(3);
    return 0;
}

/* SLDR2: SLIDR with a 16-bit radius. */
s32 OpcodeFuncSldr2(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("sldR2", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidRange =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

/* TALKR: set this entity's talk radius, scaled the same way as SLIDR. */
s32 OpcodeFuncTalkr(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("talkR", 2);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkRange =
            (FieldEventReadMemoryU8(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(3);
    return 0;
}

/* TLKR2: TALKR with a 16-bit radius. */
s32 OpcodeFuncTlkr2(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("tlkR2", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkRange =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Start of field_opcode_model_state.c
/////////////////////////////////////////////////

/* MSPED: set this entity's movement speed, scaled like the radius opcodes. */
s32 OpcodeFuncMsped(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("msped", 3);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveSpeed =
            (FieldEventReadMemoryS16(2, 2) * g_FieldState->currentFieldScale) /
            512;
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncAsped(void) {
    u8 modelIdx;
    s16 speed;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("asped", 3);
        }
        speed = FieldEventReadMemoryS16(2, 2);
        modelIdx = g_EntityToModel[g_CurrentEntity];
        g_FieldModels[modelIdx].animSpeed = speed;
        D_8009D828[modelIdx] = speed;
    }
    PC_INC(4);
    return 0;
}

/* GTDIR: write another entity's facing direction back into a memory bank. */
s32 OpcodeFuncGtdir(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("gtdir", 3);
        }
        FieldEventWriteMemoryU8(
            2, 3, g_FieldModels[g_EntityToModel[entityId]].Dir);
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncPgtdr(void) {
    u8 slot;
    u8 partyId;
    u8 actorId;

    slot = GET_PARAM_U8(2);
    if (slot < 3) {
        partyId = D_8009D391[slot];
        if (partyId != 0xFF) {
            actorId = g_CharIdToEntity[partyId];
            if (actorId != 0xFF) {
                if (g_EntityToModel[actorId] != 0xFF) {
                    if (g_DebugLevel & 3) {
                        DebugPrintOpcode("pgtdr", 3);
                    }
                    FieldEventWriteMemoryU8(
                        2, 3, g_FieldModels[g_EntityToModel[actorId]].Dir);
                }
            }
        }
    }
    PC_INC(4);
    return 0;
}

/* GETAI: write another entity's walkmesh triangle id back into a memory bank.
 */
s32 OpcodeFuncGetai(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("getai", 3);
        }
        FieldEventWriteMemoryS16(
            2, 3, g_FieldModels[g_EntityToModel[entityId]].PosI);
    }
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncGetaxy(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(2);
    if (g_EntityToModel[entityId] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("getaxy", 4);
        }
        FieldEventWriteMemoryS16(
            1, 3, g_FieldModels[g_EntityToModel[entityId]].PosX >> 12);
        FieldEventWriteMemoryS16(
            2, 4, g_FieldModels[g_EntityToModel[entityId]].PosY >> 12);
    }
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncAxyzi(void) {
    u8 entityId;

    entityId = GET_PARAM_U8(3);
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("axyzi", 7);
    }
    if (g_EntityToModel[entityId] != 0xFF) {
        FieldEventWriteMemoryS16(
            1, 4, g_FieldModels[g_EntityToModel[entityId]].PosX >> 12);
        FieldEventWriteMemoryS16(
            2, 5, g_FieldModels[g_EntityToModel[entityId]].PosY >> 12);
        FieldEventWriteMemoryS16(
            3, 6, g_FieldModels[g_EntityToModel[entityId]].PosZ >> 12);
        FieldEventWriteMemoryS16(
            4, 7, g_FieldModels[g_EntityToModel[entityId]].PosI);
    }
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncPxyzi(void) {
    u8 slot;
    u8 partyId;
    u8 actorId;

    slot = GET_PARAM_U8(3);
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pxyzi", 7);
    }
    if (slot < 3) {
        partyId = D_8009D391[slot];
        if (partyId < 9) {
            actorId = g_CharIdToEntity[partyId];
            if (g_EntityToModel[actorId] != 0xFF) {
                FieldEventWriteMemoryS16(
                    1, 4, g_FieldModels[g_EntityToModel[actorId]].PosX >> 12);
                FieldEventWriteMemoryS16(
                    2, 5, g_FieldModels[g_EntityToModel[actorId]].PosY >> 12);
                FieldEventWriteMemoryS16(
                    3, 6, g_FieldModels[g_EntityToModel[actorId]].PosZ >> 12);
                FieldEventWriteMemoryS16(
                    4, 7, g_FieldModels[g_EntityToModel[actorId]].PosI);
            }
        }
    }
    PC_INC(8);
    return 0;
}

s32 OpcodeFuncVisi(void) {
    u8 model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("visi", 1);
    }
    model = g_EntityToModel[g_CurrentEntity];
    if (model != 0xFF) {
        g_FieldModels[model].visible = GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncTlkon(void) {
    u8 model;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tlkon", 1);
    }
    model = g_EntityToModel[g_CurrentEntity];
    if (model != 0xFF) {
        g_FieldModels[model].TalkOff = GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncXyzi(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyzi", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ =
            FieldEventReadMemoryS16(3, 7) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosI =
            FieldEventReadMemoryS16(4, 9);
    }
    PC_INC(11);
    return 1;
}

s32 OpcodeFuncXyz(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyz", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosZ =
            FieldEventReadMemoryS16(3, 7) << 12;
    }
    PC_INC(9);
    return 1;
}

s32 OpcodeFuncXyi(void) {
    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("xyi", 8);
        }
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosX =
            FieldEventReadMemoryS16(1, 3) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosY =
            FieldEventReadMemoryS16(2, 5) << 12;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].PosI =
            FieldEventReadMemoryS16(3, 7);
    }
    PC_INC(9);
    return 1;
}

/////////////////////////////////////////////////
// Start of field_opcode_message.c
/////////////////////////////////////////////////

s32 OpcodeFuncMes(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mes", 2);
    }
    if (FieldDialogMessageUpdateStates(GET_PARAM_U8(1), GET_PARAM_U8(2)) != 0) {
        PC_INC(3);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncMpnam(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpnam", 1);
    }
    CopyDialogToMapName(GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

/*
 * Field-script opcode ASK: run a menu prompt and store the chosen row.
 *
 * Blocks (returning 1 and holding the player) until FieldDialogAskUpdateStates
 * reports the prompt is finished; the answer is written back to the script
 * memory bank either way.
 */
s32 OpcodeFuncAsk(void) {
    s16 answer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ask", 6);
    }
    answer = FieldEventReadMemoryU8(2, 6);
    if (FieldDialogAskUpdateStates(
            GET_PARAM_U8(2), GET_PARAM_U8(3), GET_PARAM_U8(4), GET_PARAM_U8(5),
            &answer) != 0) {
        FieldEventWriteMemoryU8(2, 6, answer);
        g_FieldState->characterLock = D_80081DC4;
        PC_INC(7);
        return 0;
    } else {
        FieldEventWriteMemoryU8(2, 6, answer);
        g_FieldState->characterLock = 1;
        return 1;
    }
}

/////////////////////////////////////////////////
// Start of field_opcode_window.c
/////////////////////////////////////////////////

s32 OpcodeFuncWclsEx(void) {
    s16 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wcls!", 0);
    }
    window = GET_PARAM_U8(1);
    if (g_WindowToEntity[window] == 0xFF) {
        PC_INC(2);
        return 0;
    }
    FieldWindowSetStateToClose(window);
    FieldDialogMessageUpdateStates(window, 0);
    return 1;
}

s32 OpcodeFuncWsizw(void) {
    s16 window;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wsizw", 8);
    }
    window = GET_PARAM_U8(1);
    if (g_WindowToEntity[window] == 0xFF) {
        return OpcodeFuncWsize();
    }
    if (g_WindowToEntity[window] == g_CurrentEntity) {
        FieldWindowSetStateToClose(window);
        FieldDialogMessageUpdateStates(window, 0);
    }
    return 1;
}

s32 OpcodeFuncWsize(void) {
    s16 x;
    s16 y;
    s16 w;
    s16 h;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wsize", 8);
    }
    GET_PARAM_S16(x, 2);
    GET_PARAM_S16(y, 4);
    GET_PARAM_S16(w, 6);
    GET_PARAM_S16(h, 8);
    FieldDialogSetSize(GET_PARAM_U8(1), x, y, w, h);
    PC_INC(10);
    /* Not cosmetic: the statement boundary stops gcc sinking `move v0,zero`
     * into the load delay slot of the PC_INC read, which is what forces the
     * original's $v0 for the incremented value and its trailing `nop`.
     * Most likely a macro in the original. Found by decomp-permuter. */
    do {
        return 0;
    } while (0);
}

s32 OpcodeFuncWrow(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wrow", 2);
    }
    FieldDialogSetWindowHeight(GET_PARAM_U8(1), (GET_PARAM_U8(2) << 4) | 9);
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncWmove(void) {
    s16 dx;
    s16 dy;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wmove", 8);
    }
    GET_PARAM_S16(dx, 2);
    GET_PARAM_S16(dy, 4);
    FieldDialogMove(GET_PARAM_U8(1), dx, dy);
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncWrest(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wrest", 1);
    }
    FieldWindowReset(GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncWclse(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wclse", 1);
    }
    if (FieldWindowSetStateToClose(GET_PARAM_U8(1)) != 0) {
        PC_INC(2);
        return 0;
    }
    return 1;
}

s32 OpcodeFuncWmode(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("wmode", 3);
    }
    FieldDialogSetWindowStyleCbc(
        GET_PARAM_U8(1), GET_PARAM_U8(2), GET_PARAM_U8(3));
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_math.c
/////////////////////////////////////////////////

/**
 * @brief Opcode 0x8F - **AND** - Bitwise AND (8-bit)
 *
 * Memory layout:
 *
 * | 0x8F | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise AND and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise AND.
 * @details
 * Performs a bitwise AND operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the "Oper" is the
 * operand to AND with. If the Source Bank is an 8 bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncAnd(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("and", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        FieldEventReadMemoryU8(1, 2) & FieldEventReadMemoryU8(2, 3) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x90 - **AND2** - Bitwise AND (16-bit)
 *
 * Memory layout:
 *
 * | 0x90 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise AND, and
 * that which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise AND, or address of the
 * second operand, if S is non-zero.
 * @details
 * Performs a bitwise AND operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then the "Oper" is the
 * operand to AND with. If the Source Bank is a 16-bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncAnd2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("and2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) & FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x91 - **OR** - Bitwise OR (8-bit)
 *
 * Memory layout:
 *
 * | 0x91 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise OR and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise OR.
 * @details
 * Performs a bitwise OR operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the "Oper" is the
 * operand to OR with. If the Source Bank
 * is an 8 bit bank, then the "Oper" is the address in that bank where
 * the operand is.
 */
s32 OpcodeFuncOr(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("or", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) | FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x92 - **OR2** - Bitwise OR (16-bit)
 *
 * Memory layout:
 *
 * | 0x92 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise OR, and that
 * which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise OR, or address of the
 * second operand, if S is non-zero
 * @details
 * Performs a bitwise OR operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then "Oper" is the operand
 * to OR with. If the Source Bank is a 16-bit bank, then the "Oper" is the
 * address in that bank where the operand is.
 */
s32 OpcodeFuncOr2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("or2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) | FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x93 - **XOR** - Bitwise XOR (8-bit)
 *
 * Memory layout:
 *
 * | 0x93 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains an operand of the bitwise XOR and receives the
 * result.
 * - const UByte Oper: The second operand of the bitwise XOR.
 * @details
 * Performs a bitwise XOR operation between "Dest" and "Oper" and stores the
 * result back into "Dest". If the Source Bank is 0 then the Operis the operand
 * to XOR with. If the Source Bank is an 8 bit bank, then the "Oper" is the
 * address in that bank where the operand is.
 */
s32 OpcodeFuncXor(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("xor", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) ^ FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x94 - **XOR2** - Bitwise XOR (16-bit)
 *
 * Memory layout:
 *
 * | 0x94 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank, or zero if "Oper" is specified as a literal
 * value.
 * - const UByte Dest: Address containing an operand of the bitwise XOR, and
 * that which receives the result.
 * - const UShort Oper: 16-bit operand of the bitwise XOR, or address of the
 * second operand, if S is non-zero.
 * @details
 * Performs a bitwise XOR operation between  "Dest"  and "Oper" and stores the
 * result back into  "Dest" . If the Source Bank is 0 then the "Oper" is the
 * operand to XOR with. If the Source Bank is a 16-bit bank, then the "Oper" is
 * the address in that bank where the operand is.
 */
s32 OpcodeFuncXor2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("xor2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) ^ FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x85 - **PLUS** - Addition (8-bit)
 *
 * Memory layout:
 *
 * | 0x85 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const UByte Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into  "Dest" . The
 * result of the addition wraps around into the range of 0-255. If the Source
 * Bank is 0 then the "Oper" is added to the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncPlus(void) {
    u16* temp_a0;
    u8 temp_s0;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) + FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x76 - **PLUS!** - Saturated Addition (8-bit)
 *
 * Memory layout:
 *
 * | 0x76 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const UByte Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into "Dest". The result
 * of the addition is capped at 255. If the Source Bank is 0 then the "Oper" is
 * added to the destination value. If the Source Bank is an 8 bit bank, then the
 * "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncPlusEx(void) {
    u8 a;
    u8 b;
    s16 sum;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus!", 3);
    }

    a = FieldEventReadMemoryU8(1, 2);
    b = FieldEventReadMemoryU8(2, 3);
    sum = a + b;
    if (sum > 255) {
        sum = 255;
    }

    FieldEventWriteMemoryU8(1, 2, sum);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x86 - **PLUS2** - Addition (16-bit)
 *
 * Memory layout:
 *
 * | 0x86 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const SWord Oper: The operand, added to the destination.
 * @details
 * Adds two numbers together and stores the result back into  "Dest" . The
 * result of the addition wraps around into the 16-bit range. If the Source Bank
 * is 0 then the "Oper" is added to the destination value. If the
 * Source Bank is an 16 bit bank, then the "Oper" is the address in that bank
 * where the operand is.
 */
s32 OpcodeFuncPlus2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("plus2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) + FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x77 - **PLS2!** - Saturated Addition (16-bit)
 *
 * Memory layout:
 *
 * | 0x77 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is added.
 * - const SWord Oper: The operand, added to the destination
 * @details
 * Adds two numbers together and stores the result back into "Dest" The result
 * of the addition is capped at 32767. The result is not capped at the negative
 * end, however (-32768), so adding two large negative numbers together will
 * still produce wrap-around. If the Source Bank is 0 then the "Oper" is added
 * to the destination value. If the Source Bank is an 16 bit bank, then the
 * "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncPls2Ex(void) {
    s16 a;
    s16 b;
    s32 sum;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pls2!", 3);
    }
    a = FieldEventReadMemoryS16(1, 2);
    b = FieldEventReadMemoryS16(2, 3);
    sum = a + b;
    if (sum > 0x7FFF) {
        sum = 0x7FFF;
    }
    FieldEventWriteMemoryS16(1, 2, sum);
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x87 - **MINUS** - Subtraction (8-bit)
 *
 * Memory layout:
 *
 * | 0x87 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const UByte Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts two numbers and stores the result back into  "Dest" . The result of
 * the subtraction wraps around into the range of 0-255. If the Source Bank is 0
 * then the "Oper" is subtracted from the destination value. If the Source Bank
 * is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMinus(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("minus", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) - FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x78 - **MINS!** - Saturated Subtraction (8-bit)
 *
 * Memory layout:
 *
 * | 0x78 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const UByte Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts "Oper" from "Dest" and stores the result back into "Dest". The
 * result of the subtraction is capped at 0. If the Source Bank is 0 then the
 * "Oper" is subtracted from the destination value. If the Source Bank is an 8
 * bit bank, then the "Oper" is the address in that bank where the operand is.
 */
s32 OpcodeFuncMinsEx(void) {
    u8 a;
    u8 b;
    s16 differ;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mins!", 3);
    }
    a = FieldEventReadMemoryU8(1, 2);
    b = FieldEventReadMemoryU8(2, 3);
    differ = a - b;
    if (differ < 0) {
        differ = 0;
    }
    FieldEventWriteMemoryU8(1, 2, differ & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x88 - **MINS2** - Subtraction (16-bit)
 *
 * Memory layout:
 *
 * | 0x98 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const SWord Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts two numbers and stores the result back into "Dest". The result of
 * the subtraction wraps around into the 16-bit range. If the Source Bank is 0
 * then the "Oper" is subtracted from the destination value. If the Source Bank
 * is an 16 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMins2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mins2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) - FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x79 - **MNS2!** - Saturated Subtraction (16-bit)
 *
 * Memory layout:
 *
 * | 0x79 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * subtracted.
 * - const SWord Oper: The operand to be subtracted from the destination.
 * @details
 * Subtracts "Oper" from "Dest" and stores the result back into "Dest". The
 * result of the subtraction is capped at -32768. The result is not capped at
 * the positive end (32767), so subtracting a large negative number from a large
 * positive number will still produce wrap-around. If the Source Bank is 0 then
 * the "Oper" is subtracted from the destination value. If the
 * Source
 * Bank is an 16 bit bank, then the "Oper" is the address in that bank
 * where the operand is.
 */
s32 OpcodeFuncMns2Ex(void) {
    s16 a;
    s16 b;
    s32 differ;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mns2!", 3);
    }
    a = FieldEventReadMemoryS16(1, 2);
    b = FieldEventReadMemoryS16(2, 3);
    differ = a - b;
    if (differ <= 0x7FFF) {
        differ = 0x8000;
    }
    FieldEventWriteMemoryS16(1, 2, differ);
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x89 - **MUL** - Multiplication (8-bit)
 *
 * Memory layout:
 *
 * | 0x89 | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * multiplied.
 * - const UByte Oper: The operand, which is multiplied with the destination.
 * @details
 * Multiplies two numbers together and stores the result back into "Dest". The
 * result of the Multiplication is capped at 255. If the Source Bank is 0 then
 * the the value "Oper" is multiplied with the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMul(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mul", 3);
    }
    FieldEventWriteMemoryU8(
        1, 2,
        (FieldEventReadMemoryU8(1, 2) * FieldEventReadMemoryU8(2, 3)) & 0xFF);
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8A - **MUL2** - Multiplication (16-bit)
 *
 * Memory layout:
 *
 * | 0x8A | D/S | Dest | Oper |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: The destination variable, to which the operand is
 * multiplied.
 * - const SWord Oper: The operand, which is multiplied with the destination.
 * @details
 * Multiplies two numbers together and stores the result back into "Dest". The
 * result of the Multiplication is capped at 32767. If the Source Bank is 0 then
 * the the value "Oper" is multiplied with the destination value. If the Source
 * Bank is an 8 bit bank, then the "Oper" is the address in that bank where the
 * operand is.
 */
s32 OpcodeFuncMul2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mul2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) * FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x8B - **DIV** - Division (8-bit)
 *
 * Memory layout:
 *
 * | 0x8B | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * quotient.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the result back into "Dest". The result of
 * the division is rounded towards zero to the nearest integer. If the Source
 * Bank is 0 then the "Den" is the denominator. If the Source Bank is an 8 bit
 * bank, then the "Den" is the address in that bank where the denominator is.
 */
s32 OpcodeFuncDiv(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("div", 3);
    }
    FieldEventWriteMemoryU8(1, 2,
                            (u8)((u32)(FieldEventReadMemoryU8(1, 2) & 0xFF) /
                                 (u32)(FieldEventReadMemoryU8(2, 3) & 0xFF)));
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8C - **DIV2** - Division (16-bit)
 *
 * Memory layout:
 *
 * | 0x8C | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * quotient.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the result back into "Dest". The
 * result of the division is rounded towards zero to the nearest integer. If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 8 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncDiv2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("div2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) / FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x8D - **REMAI** - Modulus (8-bit)
 *
 * Memory layout:
 *
 * | 0x8D | D/S | Dest | Den |
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * remainder.
 * - const UByte Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the remainder back into "Dest". If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 8 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncRemai(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("remai", 3);
    }
    FieldEventWriteMemoryU8(1, 2,
                            (u8)((u32)(FieldEventReadMemoryU8(1, 2) & 0xFF) %
                                 (u32)(FieldEventReadMemoryU8(2, 3) & 0xFF)));
    PC_INC(4);
    return 0;
}

/**
 * @brief Opcode 0x8E - **REMA2** - Modulus (16-bit)
 *
 * Memory layout:
 *
 * | 0x8E | D/S | Dest | Den
 *
 * - const Bit[4] D: Destination bank
 * - const Bit[4] S: Source bank
 * - const UByte Dest: Contains the nominator of the division and receives the
 * remainder.
 * - const SWord Den: The denominator of the division.
 * @details
 * Divides "Dest" by "Den" and stores the remainder back into "Dest". If the
 * Source Bank is 0 then the "Den" is the denominator. If the Source Bank is an
 * 16 bit bank, then the "Den" is the address in that bank where the denominator
 * is.
 */
s32 OpcodeFuncRema2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rema2", 3);
    }
    FieldEventWriteMemoryS16(
        1, 2,
        (s16)(FieldEventReadMemoryS16(1, 2) % FieldEventReadMemoryS16(2, 3)));
    PC_INC(5);
    return 0;
}

/**
 * @brief Opcode 0x95 - **INC** - Increment (8-bit)
 *
 * Memory layout:
 *
 * | 0x95 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Increments the 8-bit value found at bank B, address A. If the value is 0xFF,
 * it will roll over to 0x00. If you specify a 16-bit bank, only the lower byte
 * will be incremented, and if the lower byte is 0xFF, the higher byte will be
 * unaffected whilst the lower byte will return to 0x00.
 */
s32 OpcodeFuncInc(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc", 2);
    }
    FieldEventWriteMemoryU8(2, 2, (FieldEventReadMemoryU8(2, 2) + 1) & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7A - **INC!** - Saturated Increment (8-bit)
 *
 * Memory layout:
 *
 * | 0x7A | 0/D | Dest |
 *
 * - const Bit[4] D: Destination bank
 * - const UByte Dest: The destination address in the bank where the variable is
 * incremented.
 * @details
 * Increments the value in "Dest" by 1. The result is capped at 255.
 */
s32 OpcodeFuncIncEx(void) {
    s16 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc!", 2);
    }
    result = (FieldEventReadMemoryU8(2, 2) & 0xFF) + 1;
    if (result >= 0x100) {
        result = 0xFF;
    }
    FieldEventWriteMemoryU8(2, 2, result & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode  0x96 - **INC2** - Increment (16-bit)
 *
 * Memory layout:
 *
 * | 0x96 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Increments the 16-bit value found at bank B, address A. If the value is
 * 0xFFFF, it will roll over to 0x0000.
 */
s32 OpcodeFuncInc2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc2", 3);
    }
    FieldEventWriteMemoryS16(2, 2, (s16)(FieldEventReadMemoryS16(2, 2) + 1));
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode  0x7B - **INC2!** - Saturated Increment (16-bit)
 *
 * Memory layout:
 *
 * | 0x7B | 0/D | Dest |
 *
 * - const UByte B: Destination bank.
 * - const UByte Dest: The destination address in the bank where the variable is
 * incremented.
 * @details
 * Increments the value in "Dest" by 1. The result is capped at
 * 32767.
 */
s32 OpcodeFuncInc2Ex(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("inc2!", 3);
    }
    result = FieldEventReadMemoryS16(2, 2) + 1;
    if (result > 0x7FFF) {
        result = 0x7FFF;
    }
    FieldEventWriteMemoryS16(2, 2, result);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x97 - **DEC** - Decrement (8-bit)
 *
 * Memory layout:
 *
 * | 0x97 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Decrements the 8-bit value found at bank B, address A. If the value is
 * 0x00, it will roll over to 0xFF. If you specify a 16-bit bank, only the
 * lower byte will be decremented, and if the lower byte is 0x00, the higher
 * byte will be unaffected whilst the lower byte will return to 0xFF.
 */
s32 OpcodeFuncDec(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec", 2);
    }
    FieldEventWriteMemoryU8(2, 2, (FieldEventReadMemoryU8(2, 2) - 1) & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7C - **DEC!** - Saturated Decrement (8-bit)
 *
 * Memory layout:
 *
 * | 0x7C | 0/D | Dest |
 *
 * - const UByte B: Destination bank.
 * - const UByte Dest: The destination address in the bank where the variable is
 * deccremented.
 * @details
 * Decreases the value in "Dest" by 1. The result is capped at 0.
 */
s32 OpcodeFuncDecEx(void) {
    s16 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec!", 2);
    }
    result = (FieldEventReadMemoryU8(2, 2) & 0xFF) - 1;
    if (result < 0) {
        result = 0;
    }
    FieldEventWriteMemoryU8(2, 2, result & 0xFF);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x98 - **DEC2** - Decrement (16-bit)
 *
 * Memory layout:
 *
 * | 0x98 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Address.
 * @details
 * Decrements the 16-bit value found at bank B, address A. If the value is
 * 0x0000, it will roll over to 0xFFFF.
 */
s32 OpcodeFuncDec2(void) {

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec2", 3);
    }
    FieldEventWriteMemoryS16(2, 2, (s16)(FieldEventReadMemoryS16(2, 2) - 1));
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7D - **DEC2!** - Saturated Decrement (16-bit)
 *
 * Memory layout:
 *
 * | 0x7D | 0/D | Dest |
 *
 * - const Bit[4] D: Destination bank
 * - const UByte Dest: The destination address in the bank where the variable is
 * Decremented.
 * @details
 * Decreases the value in "Dest" by 1. The result is capped at -32768.
 */
s32 OpcodeFuncDec2Ex(void) {
    s32 result;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dec2!", 3);
    }
    result = FieldEventReadMemoryS16(2, 2) - 1;
    if (result <= 0x7FFF) {
        result = 0x8000;
    }
    FieldEventWriteMemoryS16(2, 2, result);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x99 - **RANDM** - Random
 *
 * Memory layout:
 *
 * | 0x99 | B | A |
 *
 * - const UByte B: Destination bank.
 * - const UByte A: Destination address.
 * @details
 * Places a random 8-bit value into the destination bank and address specified.
 * If you specify a 16-bit bank, only the lower byte is randomised.
 */
s32 OpcodeFuncRandm(void) {
    u16* temp_v1;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("randm", 2);
    }
    g_RandomTableIndex += g_RandomTableStep;
    FieldEventWriteMemoryU8(2, 2, g_RandomTable[g_RandomTableIndex]);
    PC_INC(3);
    return 0;
}

/**
 * @brief Opcode 0x7F - **RDMSD** - Seed Random Generator
 *
 * Memory layout:
 *
 * | 0x7F | B | S |
 *
 * - const UByte B: Bank in which the seed value is stored, or zero if S is
 * specified as a literal value.
 * - const UByte A: Destination address.
 * @details
 * Seeds the random number generator used by RANDOM. The lower four bits of the
 * arguments are used as the seed value by altering the offset used to take a
 * value from the table of pseudo-random numbers.
 */
s32 OpcodeFuncRdmsd(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rdmsd", 2);
    }
    g_RandomTableStep = (FieldEventReadMemoryU8(2, 2) << 4) + 1;
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_background.c
/////////////////////////////////////////////////

s32 OpcodeFuncBgon(void) {
    u8 layer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgon", 3);
    }
    layer = FieldEventReadMemoryU8(1, 2);
    g_FieldState->backgroundLayerVisibility[layer] |=
        1 << FieldEventReadMemoryU8(2, 3);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBgoff(void) {
    u8 layer;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgoff", 3);
    }
    layer = FieldEventReadMemoryU8(1, 2);
    g_FieldState->backgroundLayerVisibility[layer] &=
        ~(1 << FieldEventReadMemoryU8(2, 3));
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncBgclr(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgclr", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] = 0;
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBgrol(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgrol", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] <<= 1;
    PC_INC(3);
    return 0;
}

s32 OpcodeFuncBgrol2(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("bgrol", 3);
    }
    g_FieldState->backgroundLayerVisibility[FieldEventReadMemoryU8(2, 2)] >>= 1;
    PC_INC(3);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_movie.c
////////////////////////////////////////////////

/* Preload the movie named by the parameter, blocking until the load finishes.
 * Same post-then-poll shape as OpcodeFuncMovie, one event command earlier. */
s32 OpcodeFuncPmvie(void) {
    s16 movieId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmvie", 1);
    }
    if (g_FieldMovieLock != 0) {
        PC_INC(2);
        return 0;
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_LOAD_MOVIE:
        switch (g_FieldState->movieCommandState) {
        case MOVCMD_ACTIVE:
            break;
        case MOVCMD_DONE:
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(2);
            return 0;
        }
        return 1;
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_LOAD_MOVIE;
        movieId = GET_PARAM_U8(1);
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        g_FieldState->eventCmdParam = movieId;
        break;
    }
    return 1;
}

/* Play the field map's movie, blocking until it finishes. Returning 1 without
 * advancing the PC re-runs the opcode next frame, so the request is posted once
 * as an event command and then polled. */
s32 OpcodeFuncMovie(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("movie", 0);
    }
    g_FieldMovieOpcodeActive = 1;
    if (g_FieldMovieLock != 0) {
        g_FieldMovieLockFrame = 0;
        PC_INC(1);
        return 0;
    }
    switch (g_FieldState->eventCmd) {
    case EVTCMD_PLAY_MOVIE:
        switch (g_FieldState->movieCommandState) {
        case MOVCMD_ACTIVE:
            break;
        case MOVCMD_DONE:
            g_FieldState->eventCmd = EVTCMD_NONE;
            g_FieldState->movieCommandState = MOVCMD_IDLE;
            PC_INC(1);
            return 0;
        }
        return 1;
    case EVTCMD_UNK14:
        PC_INC(1);
        return 0;
    case EVTCMD_NONE:
        g_FieldState->eventCmd = EVTCMD_PLAY_MOVIE;
        g_FieldState->movieCommandState = MOVCMD_IDLE;
        break;
    }
    return 1;
}

s32 OpcodeFuncMvief(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mvief", 2);
    }
    if (g_FieldMovieLock != 0) {
        FieldEventWriteMemoryS16(2, 2, g_FieldMovieLockFrame);
        g_FieldMovieLockFrame++;
        PC_INC(3);
        return 0;
    } else {
        FieldEventWriteMemoryS16(2, 2, g_FieldState->currentMovieFrame);
        PC_INC(3);
        return 0;
    }
}

s32 OpcodeFuncMpjpo(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mpjpo", 0);
    }
    g_FieldState->mapJumpDisabled = GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_scroll.c
////////////////////////////////////////////////

s32 OpcodeFuncScr2d(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2d", 5);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_INSTANT;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 2);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 4);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncScrlc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlc", 0);
    }
    g_FieldState->cameraScrollMode = GET_PARAM_U8(4);
    g_FieldState->cameraScrollTargetId = g_FieldState->pcModelId;
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(5);
    return 0;
}

/* Scroll the camera to an entity over a number of frames. Unlike SCR2D the
 * target is an entity id, so a missing model makes the opcode a no-op. */
s32 OpcodeFuncScrla(void) {
    u8 entityId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrla", 0);
    }
    entityId = GET_PARAM_U8(4);
    if (g_EntityToModel[entityId] != 0xFF) {
        g_FieldState->cameraScrollMode = GET_PARAM_U8(5);
        g_FieldState->cameraScrollTargetId = g_EntityToModel[entityId];
        g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
        g_FieldState->cameraScrollState = 0;
    }
    PC_INC(6);
    return 0;
}

/* SCRLP is SCRLA addressed by party slot rather than by entity: the slot picks
 * a character, the character picks the field entity that represents them.
 *
 * The copy back into partyId is load-bearing, not redundant. Indexing
 * g_EntityToModel with actorId directly widens it in place as
 * `andi a1,v0,0xff`, where the original holds the resolved actor in v0 and
 * copies it out with a plain `move`. Going through the (by now dead) slot
 * variable is what produces that copy. Found by decomp-permuter. */
s32 OpcodeFuncScrlp(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlp", 0);
    }
    partyId = D_8009D391[GET_PARAM_U8(4)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = g_CharIdToEntity[partyId];
    }
    partyId = actorId;
    if (g_EntityToModel[partyId] != 0xFF) {
        g_FieldState->cameraScrollMode = GET_PARAM_U8(5);
        g_FieldState->cameraScrollTargetId = g_EntityToModel[partyId];
        g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(2, 2);
        g_FieldState->cameraScrollState = 0;
    }
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncScrcc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrcc", 0);
    }
    g_FieldState->cameraScrollMode = SCRL_OFF;
    g_FieldState->cameraScrollTargetId = g_FieldState->pcModelId;
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(1);
    return 0;
}

s32 OpcodeFuncScr2dc(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2dc", 8);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_SMOOTH;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 3);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 5);
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(4, 7);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(9);
    return 0;
}

s32 OpcodeFuncScr2dl(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scr2dl", 8);
    }
    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_LINEAR;
    g_FieldState->cameraScrollTargetX = FieldEventReadMemoryS16(1, 3);
    g_FieldState->cameraScrollTargetY = FieldEventReadMemoryS16(2, 5);
    g_FieldState->cameraScrollNumSteps = FieldEventReadMemoryS16(4, 7);
    g_FieldState->cameraScrollState = SCRLST_INIT;
    PC_INC(9);
    return 0;
}

s32 OpcodeFuncScrlw(void) {
    s32 mode;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("scrlw", 0);
    }
    if (g_FieldState->cameraScrollState == SCRLST_DONE) {
        mode = g_FieldState->cameraScrollMode;
        if (mode != SCRL_OFF) {
            if (mode < SCRL_TO_COORDS_INSTANT) {
                g_FieldState->cameraScrollMode = SCRL_TO_ENTITY_INSTANT;
            } else if (mode < 7) {
                if (mode >= SCRL_TO_COORDS_LINEAR) {
                    g_FieldState->cameraScrollMode = SCRL_TO_COORDS_INSTANT;
                }
            }
        }
        g_FieldState->cameraScrollState = SCRLST_INIT;
        PC_INC(1);
        return 0;
    }
    return 1;
}

/////////////////////////////////////////////////
// Begin of field_opcode_palette.c
////////////////////////////////////////////////

s32 OpcodeFuncStpal(void) {
    RECT rect;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpal", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = FieldEventReadMemoryU8(1, 2) + 0x1E0;
    rect.x = 0;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    StoreImage(&rect, (u_long*)&D_80095DE0[FieldEventReadMemoryU8(2, 3) * 32]);
    PC_INC(5);
    return 0;
}

/* STPAL with a start entry: the run of colours saved out of VRAM begins `x`
 * entries into the palette rather than at entry 0.
 *
 * Four instructions out, and the residue is which addend &D_80095DE0 joins.
 * The original groups the address as pal*32 + (base + x*2); gcc groups it
 * (base + pal*32) + x*2, materialising the symbol after the palette id is
 * loaded rather than before.
 *
 * Rewriting the expression does not move it. Twenty-odd phrasings -- the two
 * offsets as one index or as separate addends, either operand order, a u16 or
 * u_long view of the base, the pointer sum cast through u32, the palette id
 * hoisted into a local -- all compile to the same four rows, which says gcc
 * canonicalises the address tree before it lays anything out. Assigning
 * (base + x*2) to a local *does* produce the target's grouping, but the extra
 * pseudo shifts the script pointer out of $a1 and costs twenty rows elsewhere.
 * decomp-permuter got 265 -> 185 and no further in 13k iterations, and its one
 * find was retyping the extern to short, which is not the same address. */
s32 OpcodeFuncStpls(void) {
    RECT rect;
    s16 x;
    u8* p;
    s32 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(1) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    param = GET_PARAM_U8(2);
    p = &D_80095DE0[x * 2];
    StoreImage(&rect, (u_long*)(param * 32 + (s32)p));
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncLdpal(void) {
    RECT rect;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpal", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = FieldEventReadMemoryU8(2, 3) + 0x1E0;
    rect.x = 0;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    LoadImage(&rect, (u_long*)&D_80095DE0[FieldEventReadMemoryU8(1, 2) * 32]);
    PC_INC(5);
    return 0;
}

/* LDPAL with a start entry; same address-grouping residue as OpcodeFuncStpls
 * above, and the same phrasings have been tried against it. */
s32 OpcodeFuncLdpls(void) {
    RECT rect;
    s16 x;
    u8* p;
    s32 param;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(2) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    param = GET_PARAM_U8(1);
    p = &D_80095DE0[x * 2];
    LoadImage(&rect, (u_long*)(param * 32 + (s32)p));
    PC_INC(5);
    return 0;
}

void FieldEventRectClear(s16* arg0) {
    arg0[0] = 0;
    arg0[1] = 0;
    arg0[2] = 0;
    arg0[3] = 0;
}

/* Copy the first `count` entries of one 16-colour palette over another. The
 * palette store is a flat byte array of 32-byte pages, so both ends have to be
 * re-cast to u16 to walk entries rather than bytes. Declaring the two pointers
 * inside the loop is what makes gcc hoist each as one invariant; written above
 * the loop they land ahead of the zero-trip guard, and written inline gcc
 * reassociates the base out and the body needs a third `addu`.
 *
 * Same base-address recipe as ADPAL below: widen the palette id into an `s32`,
 * then take `u8* base = D_80095DE0;` as its own statement, then compute the two
 * pointers off `base`. The three invariant statements are hoisted in source
 * order, which is the order the target's preheader has them. */
s32 OpcodeFuncCppal(void) {
    s16 count;
    u8 src;
    u8 dst;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cppal", 4);
    }
    count = GET_PARAM_U8(4) + 1;
    src = FieldEventReadMemoryU8(1, 2);
    dst = FieldEventReadMemoryU8(2, 3);
    for (i = 0; i < count; i++) {
        s32 dp = dst;
        u8* base = D_80095DE0;
        u16* dstPal = (u16*)(base + dp * 32);
        u16* srcPal = (u16*)(base + src * 32);

        dstPal[i] = srcPal[i];
    }
    PC_INC(5);
    return 0;
}

/* As CPPAL, but source and destination each get their own start entry, so the
 * copy can shift a run of colours within or between palettes.
 *
 * Same recipe as CPPAL above; the store base is the one that has to be widened
 * and computed first, since that is the order this function's target builds
 * them in. The two are a .rodata unit -- CPPAL owns the "cppal" string CPPAL2
 * prints -- so they had to land in the same change. */
s32 OpcodeFuncCppal2(void) {
    s16 count;
    s16 srcPal;
    s16 dstPal;
    s16 src;
    s16 dst;
    s16 end;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("cppal", 7);
    }
    count = FieldEventReadMemoryU8(4, 7) + 1;
    srcPal = GET_PARAM_U8(3);
    dstPal = GET_PARAM_U8(4);
    src = FieldEventReadMemoryU8(1, 5);
    dst = FieldEventReadMemoryU8(2, 6);
    end = src + count;
    while (src < end) {
        s32 dp = dstPal;
        u8* base = D_80095DE0;
        u16* to = (u16*)(base + dp * 32);
        u16* from = (u16*)(base + srcPal * 32);

        to[dst] = from[src];
        src++;
        dst++;
    }
    PC_INC(8);
    return 0;
}

/* Rotate a palette: the run of colours ending at `count` is written back
 * starting `start` entries along, and the tail that falls off the end wraps
 * around to entry 0. Two passes, both walking the same pair of indices -- `i`
 * the source entry, `j` the destination one.
 *
 * Two things beyond the base-address recipe from ADPAL below (widen the id,
 * then `u8* base = D_80095DE0;`, then the two pointers off `base`), which is
 * what fixes both preheaders.
 *
 * The first loop's `for` counter is the store index and its body increments
 * the load index; the second loop's is the other way round. Spelled that way
 * -- `j++` as the last statement of the body -- the second loop expands its
 * two scaled addresses in the opposite order to the target and swaps $a0 and
 * $v1 all the way to the `lhu`/`sh`. gcc emits the body's increment before the
 * `for`'s, so the body form puts them in the order j, i where the target has
 * i, j; moving it into the `for` (`i++, j++`) puts both in the increment list,
 * in written order, and the whole loop falls into place. Hoisting the load
 * into a `u16` temp ahead of the store -- the obvious way to force the load
 * index first -- does not move a single row: cse folds the temp away. */
s32 OpcodeFuncRtpal(void) {
    s16 count;
    u8 src;
    u8 dst;
    s16 start;
    s16 i;
    s16 j;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rtpal", 6);
    }
    count = GET_PARAM_U8(6) + 1;
    src = FieldEventReadMemoryU8(1, 3);
    dst = FieldEventReadMemoryU8(2, 4);
    start = FieldEventReadMemoryU8(4, 5);
    i = 0;
    for (j = start; j <= count; j++) {
        s32 dp = dst;
        u8* base = D_80095DE0;
        u16* to = (u16*)(base + dp * 32);
        u16* from = (u16*)(base + src * 32);

        to[j] = from[i];
        i++;
    }
    j = 0;
    for (i = count - start; i <= count; i++, j++) {
        s32 dp = dst;
        u8* base = D_80095DE0;
        u16* to = (u16*)(base + dp * 32);
        u16* from = (u16*)(base + src * 32);

        to[j] = from[i];
    }
    PC_INC(7);
    return 0;
}

/* As RTPAL, but source and destination each get their own start entry, so the
 * rotation can move a run between two palettes as well as within one.
 *
 * Identical in shape to OpcodeFuncRtpal above and wants exactly the same two
 * fixes; read that note. */
s32 OpcodeFuncRtpal2(void) {
    s16 end;
    u8 src;
    u8 dst;
    s16 srcStart;
    s16 dstStart;
    s16 i;
    s16 j;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("rtpal", 7);
    }
    end = FieldEventReadMemoryU8(4, 7) + 1;
    src = GET_PARAM_U8(3);
    dst = GET_PARAM_U8(4);
    srcStart = FieldEventReadMemoryU8(1, 5);
    dstStart = FieldEventReadMemoryU8(2, 6);
    end += srcStart;
    i = srcStart;
    for (j = dstStart; j <= end; j++) {
        s32 dp = dst;
        u8* base = D_80095DE0;
        u16* to = (u16*)(base + dp * 32);
        u16* from = (u16*)(base + src * 32);

        to[j] = from[i];
        i++;
    }
    j = srcStart;
    for (i = end - dstStart; i <= end; i++, j++) {
        s32 dp = dst;
        u8* base = D_80095DE0;
        u16* to = (u16*)(base + dp * 32);
        u16* from = (u16*)(base + src * 32);

        to[j] = from[i];
    }
    PC_INC(8);
    return 0;
}

/* Add a signed per-channel delta to every colour of a palette. The three
 * deltas arrive as bytes, so a set sign bit is widened by hand -- `x ^= 0xFF00`
 * on a value already known to have bit 7 set is the original's sign extension.
 * Each channel is clamped to 0..0x1F on its own, and a colour that lands on
 * zero but did not start there is forced to 0x8000, since an all-zero entry is
 * the PS1's transparent pixel rather than black.
 *
 * The base address needs TWO statements of its own inside the loop, in this
 * order: the byte palette id widened into an `s32`, then `u8* base =
 * D_80095DE0;`, then the two pointers computed off `base`. That is the whole
 * of what parked this function (and its ADPAL2 twin, and the CPPAL, MPPAL,
 * RTPAL and PLS families) for so long, so it is worth spelling out.
 *
 * The residue was one transposition: the target issues `lui`/`addiu` of
 * &D_80095DE0 *between* the `andi` that widens srcPal and the `sll` that
 * scales it; gcc issued the `sll` first. Nothing about the address expression
 * moves it -- `(u8*)D_80095DE0 + (id << 5)`, `&((u16*)D_80095DE0)[id * 16]`,
 * `&D_80095DE0[id * 32]`, `id * 32 + D_80095DE0` and the plain form all
 * compile to the identical bytes, because fold canonicalises the tree to
 * `(mult) + (symbol)` and expand then evaluates the multiply first.
 *
 * The order in the preheader is `move_movables` emitting the loop's invariant
 * insns in the order `scan_loop` recorded them, which is insn order in the
 * body. So the fix is to give the body three separate invariant statements
 * whose natural order is the one wanted: widen, then base, then index. Widening
 * alone does nothing (gcc folds it back into the address), and a `base` local
 * alone puts the `lui`/`addiu` *before* the `andi` -- one row the other way.
 * Together they match. `(base + (sp << 5))` works as well as `sp * 32`, and
 * widening the second id too is harmless.
 *
 * Two other things this function needed, both still true:
 *   - `count` as s16, not u16. The s16->int widening collapses to exactly the
 *     `move a0,s4` the target has ahead of the zero-trip guard, and makes that
 *     guard `beqz` rather than `blez`. u16 folds the copy away and loses two
 *     rows. OpcodeFuncMppal2 below has the same loop shape and wants u16 --
 *     it has no such copy -- so this is not a house style, check each one.
 *   - `from` declared before `to` inside the loop; that is the order the
 *     target computes the two bases in, and it is worth 14 rows. RTPAL and
 *     RTPAL2 above compute the store base first and want the opposite order.
 * Rejected and measured: hoisting the two pointers above the loop (3 rows).
 *
 * OpcodeFuncAdpal2 below shares the "adpal" literal with this one, so the two
 * had to land together -- a lone C copy emits a second string and shifts every
 * later .rodata offset. (`rodata_owner.py` says SHARES for such a pair, which
 * is wrong while one of them is pinned: it reads the `#else` body and cannot
 * tell that MASPSX_OVERRIDE means the .s still supplies the literal.) */
s32 OpcodeFuncAdpal(void) {
    s16 count;
    u8 srcPal;
    u8 dstPal;
    s16 addB;
    s16 addG;
    s16 addR;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("adpal", 8);
    }
    count = GET_PARAM_U8(9) + 1;
    srcPal = FieldEventReadMemoryU8(1, 4);
    dstPal = FieldEventReadMemoryU8(2, 5);
    addB = FieldEventReadMemoryU8(3, 6);
    addG = FieldEventReadMemoryU8(4, 7);
    addR = FieldEventReadMemoryU8(5, 8);
    if (addB & 0x80) {
        addB ^= 0xFF00;
    }
    if (addG & 0x80) {
        addG ^= 0xFF00;
    }
    if (addR & 0x80) {
        addR ^= 0xFF00;
    }
    for (i = 0; i < count; i++) {
        s32 sp = srcPal;
        u8* base = D_80095DE0;
        u16* from = (u16*)(base + sp * 32);
        u16* to = (u16*)(base + dstPal * 32);
        u16 color = from[i];
        s16 r;
        s16 g;
        s16 b;

        r = (color & 0x1F) + addR;
        if (r >= 0x20) {
            r = 0x1F;
        }
        if (r < 0) {
            r = 0;
        }
        g = ((color >> 5) & 0x1F) + addG;
        if (g >= 0x20) {
            g = 0x1F;
        }
        if (g < 0) {
            g = 0;
        }
        b = ((color >> 10) & 0x1F) + addB;
        if (b >= 0x20) {
            b = 0x1F;
        }
        if (b < 0) {
            b = 0;
        }
        to[i] = (b << 10) | (g << 5) | r | (color & 0x8000);
        if (to[i] == 0 && color != 0) {
            to[i] = 0x8000;
        }
    }
    PC_INC(0xA);
    return 0;
}

/* ADPAL over a sub-range: the run starts `start` entries in and the two
 * palettes come from the script rather than from event memory.
 *
 * This body MATCHES -- `checkfn.py` reports MATCH on it, zero rows. It is
 * parked anyway because it cannot be compiled alone: it prints the "adpal"
 * that OpcodeFuncAdpal above owns, and with ADPAL still pinned the literal
 * would exist twice and shift the rest of .rodata. Unpin both together the
 * moment ADPAL's last row falls; nothing here needs to change.
 *
 * Both palette ids arrive by `lbu` from the script rather than through
 * FieldEventReadMemoryU8, so there is no `andi` for &D_80095DE0's `lui`/
 * `addiu` to straddle -- which is precisely why this one matches and its
 * sibling does not. */
s32 OpcodeFuncAdpal2(void) {
    s16 count;
    u8 srcPal;
    u8 dstPal;
    s16 start;
    s16 addB;
    s16 addG;
    s16 addR;
    s16 i;
    s16 end;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("adpal", 8);
    }
    count = FieldEventReadMemoryU8(6, 0xA) + 1;
    srcPal = GET_PARAM_U8(4);
    dstPal = GET_PARAM_U8(5);
    start = FieldEventReadMemoryU8(1, 6);
    addB = FieldEventReadMemoryU8(2, 7);
    addG = FieldEventReadMemoryU8(3, 8);
    addR = FieldEventReadMemoryU8(4, 9);
    if (addB & 0x80) {
        addB ^= 0xFF00;
    }
    if (addG & 0x80) {
        addG ^= 0xFF00;
    }
    if (addR & 0x80) {
        addR ^= 0xFF00;
    }
    end = start + count;
    for (i = start; i < end; i++) {
        u8* pal = D_80095DE0;
        u16* from = (u16*)(pal + srcPal * 32);
        u16* to = (u16*)(pal + dstPal * 32);
        u16 color = from[i];
        s16 r;
        s16 g;
        s16 b;

        r = (color & 0x1F) + addR;
        if (r >= 0x20) {
            r = 0x1F;
        }
        if (r < 0) {
            r = 0;
        }
        g = ((color >> 5) & 0x1F) + addG;
        if (g >= 0x20) {
            g = 0x1F;
        }
        if (g < 0) {
            g = 0;
        }
        b = ((color >> 10) & 0x1F) + addB;
        if (b >= 0x20) {
            b = 0x1F;
        }
        if (b < 0) {
            b = 0;
        }
        to[i] = (b << 10) | (g << 5) | r | (color & 0x8000);
        if (to[i] == 0 && color != 0) {
            to[i] = 0x8000;
        }
    }
    PC_INC(0xB);
    return 0;
}

/* Scale every colour of a palette per channel. The factor is a 1.7 fixed-point
 * byte, so the channel is doubled before the multiply and the product shifted
 * back down by 7. A transparent entry stays transparent -- the whole body is
 * skipped -- and one that scales down to zero is forced to 0x8000.
 *
 * Read the channel extraction off the target, do not derive it: all three are
 * "shift, then mask six bits", so the doubling is folded into the shift and
 * the mask keeps the neighbouring low bit. Red is `(color << 1) & 0x3E`, not
 * `(color & 0x1F) << 1` -- the same two instructions in the other order, and
 * two rows. Green and blue are `(color >> 4) & 0x3F` and `(color >> 9) & 0x3F`,
 * not `>> 5`/`>> 10` masked to 0x1F: those are genuinely different values, one
 * bit wider at the bottom. And the factor is the *left* operand of the
 * multiply. Writing the doubling as `* 2` anywhere in the expression lets gcc
 * reassociate it onto the loop-invariant factor and hoist `factor * 2` out of
 * the loop, which is three rows on its own; `<< 1` does not reassociate.
 *
 * The arithmetic is unsigned end to end: the target shifts the products with
 * `srl` and clamps with `sltiu`, so `color' and the three channel results are
 * u32. Declared u16/s32 -- which is what the m2c seed and every earlier
 * revision of this note had -- the same source compiles to `sra'/`slti' and
 * sits eight rows further out. `r | ((b << 10) | (g << 5))' is the target's
 * or-tree; the natural `(b << 10) | (g << 5) | r' reverses one `or' operand.
 *
 * This pair has to be measured with *both* members unparked: MPPAL2 owns the
 * "mppal" literal MPPAL prints, so with either one pinned the other emits a
 * second copy and every `.rodata' offset after it shifts.
 *
 * Four corrections took the pair from 23/15 rows to zero, and every one of
 * them is a type or a placement, not a scheduling accident:
 *   - `color' is `u16', so the promotion to int is a real `andi 0xffff' insn
 *     that the `!= 0' test and both right shifts share. As `u32' gcc knows the
 *     `lhu' already zero-extends and no such insn exists anywhere.
 *   - the products are unsigned -- the target shifts them with `srl' and
 *     clamps with `sltiu'. Two `u16' factors promote to *signed* int, so the
 *     unsignedness has to come from the mask: `& 0x3EU', not `& 0x3E'.
 *   - `(u16)(color << 1)' keeps the doubling in HImode, so it reads the raw
 *     `lhu' register the way the target does; without the cast the shift takes
 *     the zero-extended copy cse already has and the whole function is one row
 *     out. This is the same lever as the `&0x8000' at the bottom, which
 *     combine narrows on its own because it is a plain mask.
 *   - `to' is computed at the store, not beside `from'. `move_movables' emits
 *     the preheader hoists in the order the loop body first uses them, so the
 *     to-base has to come after the two factor masks; declaring it beside
 *     `from' puts both bases adjacent and costs seven rows plus the rename
 *     cascade. It still has to be a *statement* in the `if' arm rather than an
 *     initialiser inside it -- an initialiser lands before the arithmetic
 *     again (30 rows).
 *
 * The factor types are not uniform and the reason is mechanical: this function
 * returns `u8', so a `u16' local needs `andi 0xff' at the assignment and
 * `andi 0xffff' at each promotion, a `u32' local needs only the `andi 0xff',
 * and a `u8' local needs neither at the assignment and `andi 0xff' at the use.
 * Read which of the three the target has and declare accordingly -- here mulR
 * and mulG carry both masks and mulB only the first, so mulB is `u32'.
 *
 * MPPAL2's `count' is `s16': `i < count' is `slt', which a `u16' bound cannot
 * produce (gcc 2.6.3 promotes unsigned short to *unsigned* int), and its
 * zero-trip guard is `beqz' rather than `blez' because the bound also has to
 * be a narrow type. MPPAL's is `s16' for the same reason. */
s32 OpcodeFuncMppal2(void) {
    s16 count;
    u8 srcPal;
    u8 dstPal;
    u32 mulB;
    u16 mulG;
    u16 mulR;
    s16 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mppal", 8);
    }
    count = GET_PARAM_U8(9) + 1;
    srcPal = FieldEventReadMemoryU8(1, 4);
    dstPal = FieldEventReadMemoryU8(2, 5);
    mulB = FieldEventReadMemoryU8(3, 6);
    mulG = FieldEventReadMemoryU8(4, 7);
    mulR = FieldEventReadMemoryU8(5, 8);
    for (i = 0; i < count; i++) {
        s32 sp = srcPal;
        u8* base = D_80095DE0;
        u16* from = (u16*)(base + sp * 32);
        u16 color = from[i];

        if (color != 0) {
            u32 r = (mulR * ((u16)(color << 1) & 0x3EU)) >> 7;
            u32 g = (mulG * ((color >> 4) & 0x3FU)) >> 7;
            u32 b = (mulB * ((color >> 9) & 0x3FU)) >> 7;
            u16* to;

            if (b >= 0x20) {
                b = 0x1F;
            }
            if (g >= 0x20) {
                g = 0x1F;
            }
            if (r >= 0x20) {
                r = 0x1F;
            }
            to = (u16*)(base + dstPal * 32);
            to[i] = r | ((b << 10) | (g << 5)) | (color & 0x8000);
            if (to[i] == 0) {
                to[i] = 0x8000;
            }
        }
    }
    PC_INC(0xA);
    return 0;
}

/* MPPAL over a sub-range; the two palettes come from the script.
 *
 * Note the pair is named the wrong way round against the ADPAL and RTPAL
 * pairs: OpcodeFuncMppal2 above is the plain form and comes first in the
 * overlay, and this one -- the sub-range form -- is second. The addresses say
 * so, and so does the fact that MPPAL2 owns the "mppal" literal this prints.
 *
 * Same body as OpcodeFuncMppal2 above; read that note for why `color' is u16,
 * why the masks are unsigned, why the doubling carries a `(u16)' cast and why
 * `to' is computed at the store. Here the two palette indices come from the
 * script bytes rather than from banked memory, so they are plain `lbu' loads
 * and need no narrowing at all. */
s32 OpcodeFuncMppal(void) {
    s16 count;
    u8 srcPal;
    u8 dstPal;
    s16 start;
    u32 mulB;
    u16 mulG;
    u16 mulR;
    s16 i;
    s16 end;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mppal", 8);
    }
    count = FieldEventReadMemoryU8(6, 0xA) + 1;
    srcPal = GET_PARAM_U8(4);
    dstPal = GET_PARAM_U8(5);
    start = FieldEventReadMemoryU8(1, 6);
    mulB = FieldEventReadMemoryU8(2, 7);
    mulG = FieldEventReadMemoryU8(3, 8);
    mulR = FieldEventReadMemoryU8(4, 9);
    end = start + count;
    for (i = start; i < end; i++) {
        s32 sp = srcPal;
        u8* base = D_80095DE0;
        u16* from = (u16*)(base + sp * 32);
        u16 color = from[i];

        if (color != 0) {
            u32 r = (mulR * ((u16)(color << 1) & 0x3EU)) >> 7;
            u32 g = (mulG * ((color >> 4) & 0x3FU)) >> 7;
            u32 b = (mulB * ((color >> 9) & 0x3FU)) >> 7;
            u16* to;

            if (b >= 0x20) {
                b = 0x1F;
            }
            if (g >= 0x20) {
                g = 0x1F;
            }
            if (r >= 0x20) {
                r = 0x1F;
            }
            to = (u16*)(base + dstPal * 32);
            to[i] = r | ((b << 10) | (g << 5)) | (color & 0x8000);
            if (to[i] == 0) {
                to[i] = 0x8000;
            }
        }
    }
    PC_INC(0xB);
    return 0;
}

static void SetPcModel(void) {
    if (Savemap.memory_bank_2[9] != 0xFF &&
        g_CharIdToEntity[Savemap.memory_bank_2[9]] != 0xFF &&
        g_EntityToModel[g_CharIdToEntity[Savemap.memory_bank_2[9]]] != 0xFF) {
        g_FieldState->pcModelId =
            g_EntityToModel[g_CharIdToEntity[Savemap.memory_bank_2[9]]];
    }
}

s32 OpcodeFuncPc(void) {
    u8 charId;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pc", 1);
    }

    charId = GET_PARAM_U8(1);
    g_CharIdToEntity[charId] = g_CurrentEntity;

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            if (i != 0) {
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 0;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff = 1;
                g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkOff = 1;
            } else {
                g_FieldState->pcModelId = g_EntityToModel[g_CurrentEntity];
            }

            PC_INC(2);
            return 0;
        }
    }

    g_CharIdToEntity[charId] = g_CurrentEntity;

    g_FieldModels[g_EntityToModel[g_CurrentEntity]].visible = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TalkOff = 1;

    PC_INC(2);
    return 0;
}

s32 OpcodeFuncPrtyp(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtyp", 1);
    }

    charId = GET_PARAM_U8(1);
    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            PC_INC(2);
            SetPcModel();
            PartyFromBank2ToSave(0);
            return 0;
        }
    }

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == 0xFF) {
            ADD_PARTY_MEMBER(i, charId);

            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2(
                    "p+ ef=", g_CharIdToEntity[charId], 2);
            }
            PC_INC(2);
            SetPcModel();
            PartyFromBank2ToSave(1);
            return 0;
        }
    }

    ADD_PARTY_MEMBER(2, charId);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("p+ lf=", g_CharIdToEntity[charId], 2);
    }
    PC_INC(2);
    SetPcModel();
    PartyFromBank2ToSave(1);
    return 0;
}

s32 OpcodeFuncPrtym(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtym", 1);
    }

    charId = GET_PARAM_U8(1);

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            Savemap.memory_bank_2[9 + i] = 0xFF;
            PartyFromBank2ToSave(1);
            SetPcModel();
            PC_INC(2);
            return 0;
        }
    }

    PartyFromBank2ToSave(1);
    SetPcModel();
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncPrtye(void) {
    u8 newParty[3];
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtye", 3);
    }

    for (i = 0; i < 3; i++) {
        newParty[i] = (&GET_PARAM_U8(1))[i];
    }

    PartyReplace(newParty);
    PC_INC(4);
    return 0;
}

s32 OpcodeFuncSptye(void) {
    u8 newParty[3];
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("sptye", 5);
    }

    for (i = 0; i < 3; i++) {
        newParty[i] = FieldEventReadMemoryU8(1 + i, 3 + i);
    }

    PartyReplace(newParty);
    PC_INC(6);
    return 0;
}

s32 OpcodeFuncGptye(void) {
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gptye", 5);
    }

    for (i = 0; i < 3; i++) {
        FieldEventWriteMemoryU8(1 + i, 3 + i, Savemap.memory_bank_2[9 + i]);
    }
    PC_INC(6);
    return 0;
}

// Partial replace of bank 2 party with newParty.
// Any free slots in newParty are taken up by members of bank 2 party.
// The result is then transferred to main party in save.
void PartyReplace(u8* newParty) {
    s32 i, j;

    // Remove requested members from old party.
    for (i = 0; i < 3; i++) {
        if (newParty[i] != 0xFF) {
            for (j = 0; j < 3; j++) {
                if (newParty[i] == Savemap.memory_bank_2[9 + j]) {
                    Savemap.memory_bank_2[9 + j] = 0xFF;
                }
            }
        }
    }

    // Add remaining members of old party to empty slots in new party.
    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] != 0xFF) {
            for (j = 0; j < 3; j++) {
                if (newParty[j] == 0xFF) {
                    newParty[j] = Savemap.memory_bank_2[9 + i];
                    j = 3;
                }
            }
        }
    }

    // Overwrite old party with new party.
    for (i = 0; i < 3; i++) {
        // Convert forced empty slots to regular empty slots.
        if (newParty[i] == 0xFE) {
            newParty[i] = 0xFF;
        }

        ADD_PARTY_MEMBER(i, newParty[i]);
    }

    PartyFromBank2ToSave(1);
    SetPcModel();
}

// Compares two sets of parties and returns which members don't exist in both.
static void PartyCompare(
    u8* party1, u8* party2, u8* party2Only, u8* party1Only) {
    s32 i, j, k;

    for (i = 0; i < 3; i++) {
        party2Only[i] = 0xFF;
        party1Only[i] = 0xFF;
    }

    k = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party2[i] == party1[j]) {
                goto foundInParty1;
            }
        }
        party2Only[k++] = party2[i];
    foundInParty1:;
    }

    k = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party1[i] == party2[j]) {
                goto foundInParty2;
            }
        }
        party1Only[k++] = party1[i];
    foundInParty2:;
    }
}

// Transfers party from bank 2 to save while preserving order in save of
// characters existing in both parties.
void PartyFromBank2ToSave(s32 unused) {
    u8 notInSave[3];
    u8 notInBank2[3];

    PartyCompare(
        Savemap.partyID, &Savemap.memory_bank_2[9], notInSave, notInBank2);
    PartyRemove(Savemap.partyID, notInBank2);
    PartyAdd(Savemap.partyID, notInSave);
    g_PartyUpdatedByFieldScript = 1;
}

// Transfers party from save to bank 2 while preserving order in bank 2 of
// characters existing in both parties.
void PartyFromSaveToBank2(void) {
    u8 notInBank2[3];
    u8 notInSave[3];

    PartyCompare(
        &Savemap.memory_bank_2[9], Savemap.partyID, notInBank2, notInSave);
    PartyRemove(&Savemap.memory_bank_2[9], notInSave);
    PartyAdd(&Savemap.memory_bank_2[9], notInBank2);
}

void PartyRemove(u8* party, u8* toRemove) {
    s32 i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (toRemove[i] == party[j]) {
                party[j] = 0xFF;
            }
        }
    }
}

// Adds characters from toAdd to the first free slots in party.
// Does not use force freed slots.
void PartyAdd(u8* party, u8* toAdd) {
    s32 i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (party[j] == 0xFF) {
                party[j] = toAdd[i];
                break;
            }
        }
    }
}

s32 OpcodeFuncPrtyq(void) {
    s32 i;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("prtyq", 2);
    }

    charId = GET_PARAM_U8(1);

    for (i = 0; i < 3; i++) {
        if (Savemap.memory_bank_2[9 + i] == charId) {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("prty=TRUE", 0, 0);
            }
            PC_INC(3);
            return 0;
        }
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("prty=FALSE", 0, 0);
    }
    PC_INC(GET_PARAM_U8(2) + 2);
    return 0;
}

s32 OpcodeFuncMembq(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("membq", 2);
    }

    if ((1 << GET_PARAM_U8(1)) & Savemap.phs_visibility_mask) {
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("memb=TRUE", 0, 0);
        }
        PC_INC(3);
        return 0;
    }

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("memb=FALSE", 0, 0);
    }
    PC_INC(GET_PARAM_U8(2) + 2);
    return 0;
}

s32 OpcodeFuncMmbPlusMinus(void) {
    s16 i;
    s16 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmb+-", 3);
    }

    charId = GET_PARAM_U8(2);

    if (GET_PARAM_U8(1)) {
        Savemap.phs_visibility_mask |= 1 << charId;
    } else {
        Savemap.phs_visibility_mask &= ~(1 << charId);
        for (i = 0; i < 3; i++) {
            if (Savemap.memory_bank_2[9 + i] == charId) {
                Savemap.memory_bank_2[9 + i] = 0xFF;
            }
        }
    }

    PC_INC(3);
    return 0;
}

s32 OpcodeFuncMmblk(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmblk", 3);
    }

    Savemap.phs_locking_mask |= 1 << GET_PARAM_U8(1);
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncMmbuk(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mmbuk", 3);
    }

    Savemap.phs_locking_mask &= ~(1 << GET_PARAM_U8(1));
    PC_INC(2);
    return 0;
}

s32 OpcodeFuncSolid(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("solid", 1);
    }

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].SolidOff =
            GET_PARAM_U8(1);
    }
    PC_INC(2);
    return 0;
}

/* Set the camera's view offset. A non-zero mode eases from the current offset
 * to the target over N steps; mode 0 applies it immediately and clears the
 * animation state. */
/* Every instruction matches except the tail merge: gcc cross-jumps the whole
 * shared PC_INC(7) tail, where the original keeps the
 * &g_FieldScriptPC[g_CurrentEntity] computation duplicated in both arms. */
/* VWOFT: set the view offset, either as a ramp (mode nonzero: start, target,
 * step count and mode, with the current step reset) or immediately (mode 0:
 * everything cleared and the offset written straight).
 *
 * 24 rows / -4 -> 12 / exact, on two things, and both had been measured before
 * against a body that made them look worse than they are:
 *
 *   - the four instructions this was short are a *second* copy of
 *     `&g_FieldScriptPC[g_CurrentEntity]`, one per arm, with only the
 *     `lhu`/`addiu`/`sh` shared. Duplicating `PC_INC(7); return 0;` cannot
 *     produce that -- cross-jumping runs after reload, both copies are the
 *     identical eight-insn sequence, and the merge walks all the way back
 *     through `lui a0` (58 rows here, 48 before). A named `u16* pc` assigned
 *     as the last statement of each arm, with `*pc += 7;` after the if/else,
 *     does: the else arm's last insns are then its two `sh zero` stores and
 *     the if arm's is the address, so the suffixes differ at the first insn
 *     and nothing merges. Worth 3 of the 4.
 *   - the fourth is the else arm's statement order. `viewOffsetNumSteps = 0;`
 *     written before `viewOffset = FieldEventReadMemoryS16(1, 2);` makes gcc
 *     load g_FieldState before the call and put the store in its delay slot;
 *     with the call first, g_FieldState is not touched until after it and one
 *     load serves both the 0x12 and 0x16 stores, which is the target's
 *     grouping. 28 -> 12. Hoisting the call result into a local instead is 20
 *     -- it gets the call up but keeps the two loads apart.
 *
 * The 12 left are one quantity-ordering tie, twice: the target computes the
 * PC index in place (`lbu a0` / `sll a0,a0,1` / `addu a0,a0,s0`) and gives
 * g_FieldState $v0 for the trailing stores, where this build puts the index
 * in $v0 and g_FieldState in $v1. Both are three-reference block-local
 * quantities born a few insns apart, so this is `block_alloc`'s tie-break on
 * qty number, i.e. insn order -- and source order does not reach it: `pc`
 * assigned after `viewOffsetMode`, after `viewOffsetStart`, or last in the
 * arm all measure exactly 12, and first in the arm 18. Every spelling of the
 * address is inert too (`g_FieldScriptPC + g_CurrentEntity`, the
 * `n + (s32)p` cast form, `pc[0] += 7`). Permuter food, and now at the exact
 * length, so score 0 is reachable. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncVwoft);
#else
s32 OpcodeFuncVwoft(void) {
    u16* pc;
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("vwoft", 6);
    }
    if (GET_PARAM_U8(6)) {
        g_FieldState->viewOffsetStart = g_FieldState->viewOffset;
        g_FieldState->viewOffsetTarget = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetNumSteps = FieldEventReadMemoryS16(2, 4);
        g_FieldState->viewOffsetMode = GET_PARAM_U8(6);
        g_FieldState->viewOffsetCurrentStep = 0;
        pc = &g_FieldScriptPC[g_CurrentEntity];
    } else {
        g_FieldState->viewOffset = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetNumSteps = 0;
        g_FieldState->viewOffsetCurrentStep = 0;
        g_FieldState->viewOffsetMode = 0;
        g_FieldState->viewOffsetStart = 0;
        g_FieldState->viewOffsetTarget = 0;
        pc = &g_FieldScriptPC[g_CurrentEntity];
    }
    *pc += 7;
    return 0;
}
#endif

/////////////////////////////////////////////////
// Begin of field_opcode_party_manage.c
/////////////////////////////////////////////////

s32 FieldEventJoinSet(s16, s16); // extern

/* JOIN (0xC3): walk the two followers back onto the party leader, then relock
 * the party. Returns 1 while either of them is still moving.
 *
 * Two declarations carry the whole codegen. The per-follower flags are `s16'
 * -- the target truncates each one (`sll v0,s0,16') before testing it, which
 * an `s32' does not do -- and both guards are `if (x == 0xFF) ok = 1; else ok
 * = call;'. The obvious `ok = 1; if (x != 0xFF) ok = call;' for the first one
 * lets cse share the 0xFF between the two comparisons; the shared pseudo then
 * has to survive a call, so it lands in $s1 and the frame grows by a save and
 * a restore. Written as two if/elses gcc materialises `li v0,0xff' twice, as
 * the target does. That was 20 rows and the frame. */
s32 OpcodeFuncJoin(void) {
    s16 joinOk;
    s16 splitOk;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("join", 1);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    if (Savemap.memory_bank_2[10] == 0xFF) {
        joinOk = 1;
    } else {
        joinOk = FieldEventJoinSet(
            g_CharIdToEntity[Savemap.memory_bank_2[10]], GET_PARAM_U8(1));
    }
    if (Savemap.memory_bank_2[11] != 0xFF) {
        splitOk = FieldEventJoinSet(
            g_CharIdToEntity[Savemap.memory_bank_2[11]], GET_PARAM_U8(1));
    } else {
        splitOk = 1;
    }
    if (joinOk && splitOk) {
        for (i = 0; i < 3; i++) {
            charId = Savemap.memory_bank_2[9 + i];
            if (charId != 0xFF) {
                g_EntitySplitJoinState[g_CharIdToEntity[charId]] = 0;
                if (i == 0) {
                    modelId = g_CharIdToEntity[charId];
                    if (modelId != 0xFF) {
                        g_FieldModels[g_EntityToModel[modelId]].SolidOff = 0;
                    }
                }
            }
        }
        g_FieldState->characterLock = g_CharacterLock;
        g_EntityForSplitJoin = 0xFF;
        PC_INC(2);
        return 0;
    }
    g_FieldState->characterLock = 1;
    if (Savemap.memory_bank_2[9] != 0xFF) {
        modelId = g_CharIdToEntity[Savemap.memory_bank_2[9]];
        if (modelId != 0xFF) {
            g_EntitySplitJoinState[modelId] = 1;
            g_FieldModels[g_EntityToModel[modelId]].scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[modelId]].ActionState = 0;
            g_FieldModels[g_EntityToModel[modelId]].SolidOff = 1;
        }
    }
    return 1;
}

s32 FieldEventSplitSet(s16, s16, s16, s16, s16); // extern
/* SPLIT (0xC2): walk the two followers away from the leader to their own
 * destinations, then unlock the party. Returns 1 while either is still moving.
 * Twin of OpcodeFuncJoin, and it wants the same two things -- s16 flags and
 * the `if (x == 0xFF) ok = 1; else ok = call;' shape that keeps gcc from
 * sharing the 0xFF across a call in a callee-saved register.
 *
 * On top of that, **one variable holds the party byte and then the result**.
 * The byte has to survive the three FieldEventReadMemory* calls, because
 * `g_CharIdToEntity[...]' is the argument evaluated last, so it needs a
 * callee-saved register either way; writing it as a separate local leaves gcc
 * with three long-lived pseudos for two registers and it picks the other pair
 * (7 rows of pure $s2/$s3 swap, and nothing about the *types* of the locals
 * moves it -- u8, s16 and merging both bytes into the existing `charId' were
 * all measured). Assigning the byte to `splitOkA' and then overwriting it with
 * the call's result is one pseudo, and the allocation falls out. */
s32 OpcodeFuncSplit(void) {
    s16 splitOkA;
    s16 splitOkB;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("split", 8);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    splitOkA = Savemap.memory_bank_2[10];
    if (splitOkA == 0xFF) {
        splitOkA = 1;
    } else {
        splitOkA = FieldEventSplitSet(
            g_CharIdToEntity[splitOkA], FieldEventReadMemoryS16(1, 4),
            FieldEventReadMemoryS16(2, 6), FieldEventReadMemoryU8(3, 8) & 0xFF,
            GET_PARAM_U8(14));
    }
    splitOkB = Savemap.memory_bank_2[11];
    if (splitOkB == 0xFF) {
        splitOkB = 1;
    } else {
        splitOkB = FieldEventSplitSet(
            g_CharIdToEntity[splitOkB], FieldEventReadMemoryS16(4, 9),
            FieldEventReadMemoryS16(5, 11),
            FieldEventReadMemoryU8(6, 13) & 0xFF, GET_PARAM_U8(14));
    }
    if (splitOkA && splitOkB) {
        for (i = 0; i < 3; i++) {
            charId = Savemap.memory_bank_2[9 + i];
            if (charId != 0xFF) {
                g_EntitySplitJoinState[g_CharIdToEntity[charId]] = 0;
                if (i == 0) {
                    modelId = g_CharIdToEntity[charId];
                    if (modelId != 0xFF) {
                        g_FieldModels[g_EntityToModel[modelId]].SolidOff = 0;
                    }
                }
            }
        }
        g_FieldState->characterLock = g_CharacterLock;
        g_EntityForSplitJoin = 0xFF;
        PC_INC(15);
        return 0;
    }
    g_FieldState->characterLock = 1;
    if (Savemap.memory_bank_2[9] != 0xFF) {
        modelId = g_CharIdToEntity[Savemap.memory_bank_2[9]];
        if (modelId != 0xFF) {
            g_EntitySplitJoinState[modelId] = 1;
            g_FieldModels[g_EntityToModel[modelId]].scriptedMoveMode = 0;
            g_FieldModels[g_EntityToModel[modelId]].ActionState = 0;
            g_FieldModels[g_EntityToModel[modelId]].SolidOff = 1;
        }
    }
    return 1;
}

/* Drive one party member through a JOIN: state 0 turns them toward the party
 * leader, state 2 waits for that turn and then walks them onto the leader,
 * state 1 waits for the walk and then makes them intangible and invisible,
 * state 3 is done. Returns 1 once this member has finished. Twin of
 * FieldEventSplitSet -- written from the target rather than permuted, and the
 * two share a shape worth knowing: an `if' guard chain, then a four-case
 * switch whose non-terminal arms `break' to one `return 0'.
 *
 * `leaderEntity' is s16. It is only ever a `lbu' of g_CharIdToEntity, so u8 is
 * the obvious declaration and it costs one row -- the copy into the index
 * register comes out `andi s1,s0,0xff' instead of `move s1,s0'. s32 is worse
 * still (26 rows): it drops the sign extension the two 0xFF comparisons want.
 */
s32 FieldEventJoinSet(s16 entityId, s16 steps) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    s16 leaderEntity;

    if (Savemap.memory_bank_2[9] == 0xFF) {
        return 1;
    }
    leaderEntity = g_CharIdToEntity[Savemap.memory_bank_2[9]];
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("join p0=", leaderEntity, 2);
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("join p1=", entityId, 2);
    }
    if (leaderEntity == 0xFF) {
        return 1;
    }
    if (entityId == 0xFF) {
        return 1;
    }
    switch (g_EntitySplitJoinState[entityId]) {
    case 0:
        from.vx = g_FieldModels[g_EntityToModel[entityId]].PosX >> 12;
        from.vy = g_FieldModels[g_EntityToModel[entityId]].PosY >> 12;
        from.vz = g_FieldModels[g_EntityToModel[entityId]].PosZ >> 12;
        to.vx = g_FieldModels[g_EntityToModel[leaderEntity]].PosX >> 12;
        to.vy = g_FieldModels[g_EntityToModel[leaderEntity]].PosY >> 12;
        to.vz = g_FieldModels[g_EntityToModel[leaderEntity]].PosZ >> 12;
        FieldEventSplitJoinSetTurn(
            entityId, g_FieldModels[g_EntityToModel[entityId]].Dir,
            FieldEntityDirByVec(&from, &to, &sqrDist) & 0xFF);
        g_EntitySplitJoinState[entityId] = 2;
        break;
    case 1:
        if (FieldEventSplitJoinEndMove(entityId) == 0) {
            break;
        }
        g_FieldModels[g_EntityToModel[entityId]].SolidOff = 1;
        g_FieldModels[g_EntityToModel[entityId]].TalkOff = 1;
        g_FieldModels[g_EntityToModel[entityId]].visible = 0;
        g_EntitySplitJoinState[entityId] = 3;
        return 1;
    case 2:
        if (FieldEventSplitJoinEndTurn(entityId) == 0) {
            break;
        }
        FieldEventSplitJoinSetMove(
            entityId,
            (g_FieldModels[g_EntityToModel[leaderEntity]].PosX * 16) >> 16,
            (g_FieldModels[g_EntityToModel[leaderEntity]].PosY * 16) >> 16,
            steps, 0);
        g_EntitySplitJoinState[entityId] = 1;
        if (g_DebugLevel & 3) {
            FieldDebugAddParseValueToPage2("end setmove", 0, 0);
        }
        break;
    case 3:
        return 1;
    }
    return 0;
}

/* Drive one party member through a SPLIT: state 0 starts the move, state 1
 * waits for the move then starts the turn, state 2 waits for the turn, state 3
 * is done. Returns 1 once this member has finished, 0 while a step is still in
 * progress.
 *
 * The parked body had the last two parameters the wrong way round -- the
 * fourth is the facing to turn to (used as `turnDir & 0xFF' in state 1) and
 * the fifth is the step count handed to FieldEventSplitJoinSetMove, whose own
 * fifth argument is the constant 1, not this function's. All five are s16;
 * `entityId' was `u8' and the `sll'/`sra' pair before the debug call and the
 * state index is what says otherwise. That was 35 of the 39 rows.
 *
 * The last four were the register the state constant goes in. Every arm that
 * ends the frame writes `g_EntitySplitJoinState[entityId]' and then returns 0,
 * and spelling that as an explicit `return 0' per arm makes gcc keep $v0 for
 * the return value and put the constant in $v1; spelling it as `break' with a
 * single `return 0' after the switch lets the constant have $v0 and re-zero it
 * on the way out, which is what the target does. Same for the two early exits
 * on FieldEventSplitJoinEndMove/EndTurn returning 0. */
s32 FieldEventSplitSet(s16 entityId, s16 x, s16 y, s16 turnDir, s16 steps) {
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("split p1=", entityId, 2);
    }
    if (entityId == 0xFF) {
        return 1;
    }
    switch (g_EntitySplitJoinState[entityId]) {
    case 0:
        FieldEventSplitJoinSetMove(entityId, x, y, steps, 1);
        g_EntitySplitJoinState[entityId] = 1;
        break;
    case 1:
        if (FieldEventSplitJoinEndMove(entityId) == 0) {
            break;
        }
        g_FieldModels[g_EntityToModel[entityId]].SolidOff = 0;
        g_FieldModels[g_EntityToModel[entityId]].TalkOff = 0;
        FieldEventSplitJoinSetTurn(
            entityId, g_FieldModels[g_EntityToModel[entityId]].Dir,
            turnDir & 0xFF);
        g_EntitySplitJoinState[entityId] = 2;
        break;
    case 2:
        if (FieldEventSplitJoinEndTurn(entityId) == 0) {
            break;
        }
        g_EntitySplitJoinState[entityId] = 3;
        return 1;
    case 3:
        return 1;
    }
    return 0;
}

/* Start one party member walking to (x, y) as part of a SPLIT or JOIN.
 *
 * The follower is made solid and visible again, optionally snapped onto the
 * party leader's position first, and given a move speed scaled so the walk
 * takes `steps` frames. Anything faster than 0x601 switches the model to the
 * run animation, anything slower to the walk one; the animation clock is only
 * reset when the animation actually changes.
 *
 * Zero instructions out. The residue is ten rows of register naming, all of
 * them in the animation-reset block: the target keeps the model index in $a0
 * and &g_FieldModelData in $v1, gcc the other way round. The identical block
 * in OpcodeFuncLader above, written the same way, matches -- here it appears
 * in both arms of the speed test and gcc cross-jumps them, which is what
 * changes the allocation.
 *
 * Two things did land it here from 23 rows, both worth knowing:
 *   - `leaderId` is s16, not u8. A u8 local is masked at the point of *use*
 *     (`andi a2,s1,0xff` before indexing); an s16 one assigned from a `lbu`
 *     needs no conversion at either end, which is the target's plain `move`.
 *   - Fill each VECTOR in field order -- from.vx/vy/vz then to.vx/vy/vz --
 *     not in the order the stores come out of the target. m2c reconstructs
 *     the schedule (vx, vy, vx, vy, vz, vz) and writing that down reproduces
 *     a different one. See the EscapeCaptureScreen note in CLAUDE.md.
 * Measured and rejected: modelIdx as s16 rather than u8, and dropping the
 * modelIdx local for the inlined `g_EntityToModel[entityId]` -- gcc CSEs it
 * to the same thing and neither changes a single instruction.
 *
 * The last ten rows were the model-entry index: it needs its own `s32
 * entryIdx` local in *both* arms, not just the one the diff pointed at.
 * Written inline inside the `&g_FieldModelData->modelEntries[...]` subscript
 * the load and the g_FieldModelData base compete for $a0/$a1/$v1 and the
 * naming propagates through the rest of the block; split out, each gets its
 * own pseudo. Fixing only the arm the diff named changes nothing -- the
 * allocator sees the whole function. */
void FieldEventSplitJoinSetMove(
    s16 entityId, s16 x, s16 y, s16 steps, u16 snapToLeader) {
    s32 entryIdx;
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    s16 leaderId;
    u8 modelIdx;
    FieldModelEntry* entry;
    u8* anims;

    if (D_8009D391[0] != 0xFF) {
        leaderId = g_CharIdToEntity[D_8009D391[0]];
        if (leaderId != 0xFF) {
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("set move x=", x, 4);
            }
            if (g_DebugLevel & 3) {
                FieldDebugAddParseValueToPage2("set move y=", y, 4);
            }
            g_FieldModels[g_EntityToModel[entityId]].visible = 1;
            g_FieldModels[g_EntityToModel[entityId]].SolidOff = 1;
            g_FieldModels[g_EntityToModel[entityId]].TalkOff = 1;
            if (snapToLeader != 0) {
                g_FieldModels[g_EntityToModel[entityId]].PosX =
                    g_FieldModels[g_EntityToModel[leaderId]].PosX;
                g_FieldModels[g_EntityToModel[entityId]].PosY =
                    g_FieldModels[g_EntityToModel[leaderId]].PosY;
                g_FieldModels[g_EntityToModel[entityId]].PosZ =
                    g_FieldModels[g_EntityToModel[leaderId]].PosZ;
                g_FieldModels[g_EntityToModel[entityId]].PosI =
                    g_FieldModels[g_EntityToModel[leaderId]].PosI;
            }
            g_FieldModels[g_EntityToModel[entityId]].ActionArg = 0;
            g_FieldModels[g_EntityToModel[entityId]].DirLock = 0;
            g_FieldModels[g_EntityToModel[entityId]].MoveEndX = x << 12;
            g_FieldModels[g_EntityToModel[entityId]].MoveEndY = y << 12;
            modelIdx = g_EntityToModel[entityId];
            g_FieldModelSavedMoveSpeed[modelIdx] =
                g_FieldModels[modelIdx].MoveSpeed;
            from.vx = g_FieldModels[g_EntityToModel[entityId]].PosX >> 12;
            from.vy = g_FieldModels[g_EntityToModel[entityId]].PosY >> 12;
            from.vz = g_FieldModels[g_EntityToModel[entityId]].PosZ >> 12;
            to.vx = x;
            to.vy = y;
            to.vz = g_FieldModels[g_EntityToModel[entityId]].PosZ >> 12;
            FieldEntityDirByVec(&from, &to, &sqrDist);
            g_FieldModels[g_EntityToModel[entityId]].MoveSpeed =
                (sqrDist << 8) / steps;
            if (g_FieldModels[g_EntityToModel[entityId]].MoveSpeed >= 0x601) {
                if (g_FieldModels[g_EntityToModel[entityId]].activeAnimId !=
                    2) {
                    g_FieldModels[g_EntityToModel[entityId]].activeAnimId = 2;
                    g_FieldModels[g_EntityToModel[entityId]].animSpeed = 0x10;
                    g_FieldModels[g_EntityToModel[entityId]].animCurrentFrame =
                        0;
                    modelIdx = g_EntityToModel[entityId];
                    entryIdx = g_FieldModelLoaderData[modelIdx].modelEntryIndex;
                    entry = &g_FieldModelData->modelEntries[entryIdx];
                    anims = entry->modelData + entry->animationOffset;
                    g_FieldModels[modelIdx].animLastFrame =
                        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId *
                                      16] -
                        1;
                }
            } else {
                if (g_FieldModels[g_EntityToModel[entityId]].activeAnimId !=
                    1) {
                    g_FieldModels[g_EntityToModel[entityId]].activeAnimId = 1;
                    g_FieldModels[g_EntityToModel[entityId]].animSpeed = 0x10;
                    g_FieldModels[g_EntityToModel[entityId]].animCurrentFrame =
                        0;
                    modelIdx = g_EntityToModel[entityId];
                    entryIdx = g_FieldModelLoaderData[modelIdx].modelEntryIndex;
                    entry = &g_FieldModelData->modelEntries[entryIdx];
                    anims = entry->modelData + entry->animationOffset;
                    g_FieldModels[modelIdx].animLastFrame =
                        *(u16*)&anims[g_FieldEntity[modelIdx].activeAnimId *
                                      16] -
                        1;
                }
            }
            D_800756E8[g_EntityToModel[entityId]] = 1;
            g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode = 1;
            g_FieldModels[g_EntityToModel[entityId]].ActionState = 0;
        }
    }
}

/* Poll one party member's walk during a SPLIT or JOIN. ActionState 2 means the
 * move just finished, so release the scripted-move lock and restore the
 * model's default speed. */
s32 FieldEventSplitJoinEndMove(s16 entityId) {
    if (g_FieldModels[g_EntityToModel[entityId]].ActionState != 2) {
        return 0;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("end move", 0, 0);
    }
    g_FieldModels[g_EntityToModel[entityId]].scriptedMoveMode = 0;
    g_FieldModels[g_EntityToModel[entityId]].ActionState = 0;
    D_800756E8[g_EntityToModel[entityId]] = 0;
    g_FieldModels[g_EntityToModel[entityId]].MoveSpeed =
        g_FieldModelSavedMoveSpeed[g_EntityToModel[entityId]];
    return 1;
}

/* Begin a party member's turn to a facing during a SPLIT or JOIN. Sets the
 * turn target and step budget, then if the raw delta would exceed half a
 * circle wraps the target the short way round.
 *
 * Two things that read as noise and are not. The first store goes through the
 * array, not through `model' -- taking the pointer there makes gcc reuse the
 * one base for the store that follows, where the original recomputes the *0x84
 * index. And the wrap test is spelled `TurnEnd > TurnStart', not the more
 * natural `TurnStart < TurnEnd': gcc 2.6.3 emits the two `sll'/`sra' casts in
 * source-operand order, so the reversed spelling is the same test with the two
 * sign-extensions -- and every register downstream of them -- the other way
 * round. It was the whole 16-row residue. Reversing the *arms* instead
 * (`>=' with the bodies swapped) does not do it; only the operands move. */
void FieldEventSplitJoinSetTurn(s16 entityId, s16 startDir, s16 endDir) {
    FieldEntity* model;
    s16 delta;

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("set turn=", endDir & 0xFF, 2);
    }
    if (g_EntityToModel[entityId] != 0xFF) {
        g_FieldModels[g_EntityToModel[entityId]].TurnStart = startDir & 0xFF;
        g_FieldModels[g_EntityToModel[entityId]].TurnType = 2;
        g_FieldModels[g_EntityToModel[entityId]].TurnStep = 0;
        g_FieldModels[g_EntityToModel[entityId]].TurnSteps = 0x10;
        g_FieldModels[g_EntityToModel[entityId]].TurnEnd = endDir & 0xFF;
        model = &g_FieldModels[g_EntityToModel[entityId]];
        delta = model->TurnEnd - model->TurnStart;
        if (delta < 0) {
            delta = ~delta + 1;
        }
        if (delta >= 0x81) {
            if ((s16)model->TurnEnd > (s16)model->TurnStart) {
                model->TurnEnd -= 0x100;
            } else {
                model->TurnEnd += 0x100;
            }
        }
    }
}

/* Poll one party member's turn during a SPLIT or JOIN. Returns 1 once the
 * entity has finished turning -- or has no model to turn -- and 0 while it is
 * still in progress. */
s32 FieldEventSplitJoinEndTurn(s16 entityId) {
    if (g_EntityToModel[entityId] == 0xFF) {
        return 1;
    }
    if (g_FieldModels[g_EntityToModel[entityId]].TurnType != 3) {
        return 0;
    }
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("end turn", 0, 0);
    }
    g_FieldModels[g_EntityToModel[entityId]].TurnType = 0;
    g_FieldModels[g_EntityToModel[entityId]].TurnStep = 0;
    g_FieldModels[g_EntityToModel[entityId]].TurnSteps = 0;
    return 1;
}

/////////////////////////////////////////////////
// Begin of field_opcode_fade.c
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// Begin of field_opcode_fade.c
/////////////////////////////////////////////////

/* FADE (0x6B): start a screen fade. Reads the fade type and per-channel target
 * colours, then the speed. The jump table picks the fadeAdjust start value per
 * fade family: the odd types (1/5/7/9) start one above the parameter, the even
 * ones (2/6/8/10) at it, and 0/3/4 do not touch fadeAdjust at all. */
s32 OpcodeFuncFade(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fade", 8);
    }
    g_FieldState->fadeType = GET_PARAM_U8(7);
    switch ((s16) * (volatile u16*)&g_FieldState->fadeType) {
    case FFT_INV4_TO_FIELD_SUB:
    case FFT_STANDARD_TO_FIELD_ADD:
    case FFT_INSTANT_INV1_SUB_HOLD_FIELD:
    case FFT_INSTANT_STANDARD_ADD_HOLD_FIELD:
        g_FieldState->fadeAdjust = GET_PARAM_U8(8) + 1;
        break;
    case FFT_FIELD_TO_INV4_SUB:
    case FFT_FIELD_TO_STANDARD_ADD:
    case FFT_INSTANT_INV1_SUB_HOLD_COLOR:
    case FFT_INSTANT_STANDARD_ADD_HOLD_COLOR:
        g_FieldState->fadeAdjust = GET_PARAM_U8(8);
        break;
    case FFT_INSTANT:
    case FFT_SYS_FADE_TO_BLACK_FIELD_CHANGE:
    case FFT_INSTANT_BLACK:
        break;
    }
    *(volatile s16*)&g_FieldState->fadeSpeed = GET_PARAM_U8(6);
    g_FieldState->fadeRed = FieldEventReadMemoryU8(1, 3);
    g_FieldState->fadeGreen = FieldEventReadMemoryU8(2, 4);
    g_FieldState->fadeBlue = FieldEventReadMemoryU8(4, 5);
    PC_INC(9);
    return 0;
}

/* The two volatile casts are delay-slot barriers, not a claim about the
 * hardware. gcc reorg happily sinks a plain store sitting just ahead of a call
 * into that call's delay slot; the original does not, leaving the first
 * FieldEventReadMemoryU8's slot empty outright and filling the
 * FieldEventReadMemoryS16's with the `ori a1,7` from the argument setup
 * instead. A volatile store is the one thing reorg refuses to move, so casting
 * exactly the two stores that precede a call pins them. The other four
 * assignments are plain: they are not adjacent to a call and schedule the same
 * either way. (A do/while barrier costs six extra instructions here by
 * breaking the g_FieldState CSE.) */
s32 OpcodeFuncNfade(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("nfade", 8);
    }
    *(volatile u16*)&g_FieldState->fadeType = GET_PARAM_U8(3);
    g_FieldState->nFadeRedTarget = FieldEventReadMemoryU8(1, 4);
    g_FieldState->nFadeGreenTarget = FieldEventReadMemoryU8(2, 5);
    g_FieldState->nFadeBlueTarget = FieldEventReadMemoryU8(3, 6);
    *(volatile s16*)&g_FieldState->fadeAdjust = 0;
    g_FieldState->fadeSpeed = FieldEventReadMemoryS16(4, 7);
    PC_INC(9);
    return 0;
}

/* FADEW (0x6C): block the script until the active screen fade completes.
 * Returns 1 while waiting, 0 (advancing the PC) once done. What counts as
 * "complete" depends on the fade's direction, so the switch on fadeType
 * collapses to three tests: a fade to black is done when fadeAdjust has run
 * down to 0, a fade from black when it has reached 0xFF, and the NFADE forms
 * when it equals fadeSpeed. Types 0 and 4 are not fades and fall straight
 * through to the PC_INC.
 *
 * 26 rows -> 4, and three of the four corrections were semantic, read straight
 * off the jump table rather than guessed:
 *   - the table has ELEVEN entries, 0..10, not twelve. `sltiu v0,v1,0xb`.
 *   - FFT_INSTANT and FFT_INSTANT_BLACK advance unconditionally -- their table
 *     slots point at the PC_INC tail itself, not at any test.
 *   - the "to inv4 / to standard" fades wait for `fadeAdjust >= 0xFF`, not for
 *     `== 0`. The tell is `slti v0,v0,0xff` in an arm the old body had testing
 *     against zero, and it is a different program, not a different schedule.
 *   - fadeType, fadeAdjust and fadeSpeed are read through a
 *     `(volatile FieldState*)` cast. Plainly typed, gcc folds the widening into
 *     the load and gives `lh`; volatile pins the load as `lhu` and leaves the
 *     `sll`/`sra` as separate insns, which is what the target has -- and only
 *     where a signed compare needs them, so the `!= 0` arm keeps its bare
 *     `beqz`.
 *
 * 4 rows -> 2, and the correction is that the two waiting arms are not
 * spelled alike. The `>= 0xFF` arm reads the member straight into its
 * comparison with no local at all, which is what puts its `lhu` in $v0; the
 * `!= 0` arm keeps `adjust`, the same local the NFADE arm uses for its first
 * read. Sharing one local across those two arms is what pins it to $v1, and
 * the NFADE arm needs $v1 there because its base register is still live for
 * the `fadeSpeed` load two insns later -- so the shared local is right for
 * one arm and wrong for the other, and only the arm that does not use it can
 * be fixed.
 *
 * The two rows left are that: the `!= 0` arm loads fadeAdjust into $v1 where
 * the target uses $v0. The register is not reachable, because it is what
 * stops gcc cross-jumping. Both waiting arms end `beqz <reg>,<PC_INC> /
 * li v0,1 / j <epilogue>`; the moment the compare register agrees, those three
 * insns are identical hard-register patterns and the post-reload jump_optimize
 * merges the `!= 0` arm into the other, deleting them and leaving a bare `j`
 * into the middle of it. The target has both copies, so the original was held
 * apart by something that is not visible in the arms themselves.
 *
 * Measured and rejected, all 9 rows: both arms read inline with no local;
 * arms 1-2 inline with arm 3 keeping locals; a second local for arm 3 with
 * arms 1-2 sharing `adjust`; three distinct locals; block-scoped locals inside
 * arm 3; all three arms inline; and the `!= 0` arm reusing `type`, the switch
 * selector, which is the one pseudo already in $v0 at that point. Retyping the
 * shared local is 9 in every width -- s32, u32, u16 -- because arm 3's
 * `!= speed` compare loses or gains a widening pair. Distinct `goto` labels
 * for the two arms' exits do not help either (one, the other, or both, with
 * the labels placed immediately before the PC_INC) -- jump_optimize collapses
 * labels that resolve to the same address before cross-jumping ever looks at
 * the blocks.
 *
 * Case order is not a lever either, and it is expensive to get wrong: the
 * blocks are laid out in the order the `case` labels are written, so moving
 * the NFADE/default arm between the two waiting arms costs 20 rows and an
 * insertion, doing that with the `!= 0` arm inline costs 24, and simply
 * swapping the two waiting arms costs 15. Also rejected earlier, at 22 rows:
 * switching on a plain (s16) cast of the member, on an s32 temp, and on an s16
 * temp, all three of which fold the lhu/sll/sra into a single lh. And one that
 * does not touch the arms at all: declaring `adjust` ahead of `type` is
 * byte-identical, the two not competing, so declaration order is inert here as
 * usual.
 *
 * A basic-block barrier does not do it either, and that is worth knowing
 * because it is the one lever CLAUDE.md recommends for exactly this shape:
 * `do { } while (0);` inside the `if` before the `return`, after the `if`
 * before the `break`, with the arm inline and with the arm keeping its own
 * local, all four measure 9 rows and -3 instructions -- byte-identical to
 * the plain inline form. The merge is decided by whether the tails match as
 * hard-register patterns, and ending a block between them changes neither
 * the patterns nor the fact that both `j` the same label.
 *
 * Reading the target settles what the arms look like and deepens the
 * puzzle rather than solving it: both waiting arms end with the *identical*
 * four insns -- `beqz v0,<PC_INC> / ori v0,zero,0x1 / j <epilogue> / nop` --
 * on the same register, and gcc still emitted both copies. So the original
 * is not holding them apart by register or by block structure, and no
 * spelling of these two tests reaches it.
 *
 * Permuter food: what is needed is a shape that keeps the two arms apart for
 * some reason other than the register, not another spelling of the same two
 * tests.
 *
 * And the permuter has now had a go, with the result recorded rather than
 * repeated: base score **10** -- exactly the two register rows at
 * `allocno_compare`'s 5 points each, so the scratch is measuring the right
 * thing -- and **no improvement in 191,751 candidates** with
 * `perm_ins_block=20` and `perm_temp_for_expr=200`, run to a wall-clock limit
 * rather than to exhaustion. Not one candidate ever scored below 10, so the
 * run left no `output-` directory at all. That is a strong negative
 * and it fits the diagnosis: 10 is a local optimum with a cliff on both sides,
 * because the one edit that fixes the register also makes the two tails
 * identical hard-register patterns and cross-jumping then deletes three
 * insns, which the scorer charges 300 for. Nothing the randomizer can reach in
 * one step crosses that.
 *
 * Note the scratch only became scoreable at all with
 * `tools/permuter_latedefines.py` and `tools/permuter_rodata_local.py` (see
 * CLAUDE.md step 4): before those, `PC_INC` was compiled as a call and the
 * "fadew" literal and jump table scored as permanent differences, and the base
 * read as 372 bytes against 396 with six mismatched relocation symbols. Do not
 * re-derive that. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncFadew);
#else
s32 OpcodeFuncFadew(void) {
    s16 type;
    s16 adjust;
    s16 speed;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fadew", 0);
    }
    type = ((volatile FieldState*)g_FieldState)->fadeType;
    switch (type) {
    case FFT_INSTANT:
    case FFT_INSTANT_BLACK:
        break;
    case FFT_INV4_TO_FIELD_SUB:
    case FFT_STANDARD_TO_FIELD_ADD:
    case FFT_INSTANT_INV1_SUB_HOLD_FIELD:
    case FFT_INSTANT_STANDARD_ADD_HOLD_FIELD:
        adjust = ((volatile FieldState*)g_FieldState)->fadeAdjust;
        if (adjust != 0) {
            return 1;
        }
        break;
    case FFT_FIELD_TO_INV4_SUB:
    case FFT_FIELD_TO_STANDARD_ADD:
    case FFT_INSTANT_INV1_SUB_HOLD_COLOR:
    case FFT_INSTANT_STANDARD_ADD_HOLD_COLOR:
        if (((volatile FieldState*)g_FieldState)->fadeAdjust < 0xFF) {
            return 1;
        }
        break;
    case FFT_SYS_FADE_TO_BLACK_FIELD_CHANGE:
    default:
        adjust = ((volatile FieldState*)g_FieldState)->fadeAdjust;
        speed = ((volatile FieldState*)g_FieldState)->fadeSpeed;
        if (adjust != speed) {
            return 1;
        }
        break;
    }
    PC_INC(1);
    return 0;
}
#endif

/////////////////////////////////////////////////
// Begin of field_opcode_intersect.c
/////////////////////////////////////////////////

/* IDLCK: set or clear the "player may not cross this walkmesh edge" bit for
 * one triangle. blockedAccesses is a bitfield, eight triangles per byte. */
s32 OpcodeFuncIdlck(void) {
    s16 triId;
    s32 byteIdx;
    s32 bitIdx;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("idlck", 3);
    }
    GET_PARAM_S16(triId, 1);
    byteIdx = triId / 8;
    bitIdx = triId - byteIdx * 8;
    if (GET_PARAM_U8(3)) {
        g_FieldState->blockedAccesses[byteIdx] |= 1 << bitIdx;
    } else {
        g_FieldState->blockedAccesses[byteIdx] &= ~(1 << bitIdx);
    }
    PC_INC(4);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_window_color.c
/////////////////////////////////////////////////

s32 OpcodeFuncGwcol(void) {
    s16 pane;
    s32 corner;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("gwcol", 6);
    }
    pane = FieldEventReadMemoryU8(1, 3);
    corner = pane * 3;
    FieldEventWriteMemoryU8(2, 4, g_FieldWindowColors[corner]);
    FieldEventWriteMemoryU8(3, 5, g_FieldWindowColors[corner + 1]);
    FieldEventWriteMemoryU8(4, 6, g_FieldWindowColors[corner + 2]);
    PC_INC(7);
    return 0;
}

s32 OpcodeFuncSwcol(void) {
    s16 pane;
    s32 corner;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("swcol", 6);
    }
    pane = FieldEventReadMemoryU8(1, 3);
    corner = pane * 3;
    g_FieldWindowColors[corner] = FieldEventReadMemoryU8(2, 4);
    g_FieldWindowColors[corner + 1] = FieldEventReadMemoryU8(3, 5);
    g_FieldWindowColors[corner + 2] = FieldEventReadMemoryU8(4, 6);
    PC_INC(7);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_field_effect.c
/////////////////////////////////////////////////

s32 OpcodeFuncLstmp(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("lstmp", 2);
    }
    FieldEventWriteMemoryS16(2, 2, g_FieldState->prevFieldId);
    PC_INC(3);
    return 0;
}

/* SHAKE: arm the randomized camera shake on either axis. Bit 0 of parameter 3
 * enables the X shake, bit 1 the Y shake; a clear bit disables that axis. */
s32 OpcodeFuncShake(void) {
    s32 axes;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("shake", 7);
    }
    axes = GET_PARAM_U8(3);
    if (axes & 1) {
        g_FieldState->shakeX.enabled = 1;
        g_FieldState->shakeX.amplitude = FieldEventReadMemoryU8(1, 4);
        g_FieldState->shakeX.numStepsPerSegment = FieldEventReadMemoryU8(2, 5);
    } else {
        g_FieldState->shakeX.enabled = 0;
    }
    if (axes & 2) {
        g_FieldState->shakeY.enabled = 1;
        g_FieldState->shakeY.amplitude = FieldEventReadMemoryU8(3, 6);
        g_FieldState->shakeY.numStepsPerSegment = FieldEventReadMemoryU8(4, 7);
    } else {
        g_FieldState->shakeY.enabled = 0;
    }
    PC_INC(8);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_items.c
/////////////////////////////////////////////////

s32 OpcodeFuncStitm(void) {
    s32 itemHi;
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stitm", 4);
    }
    itemHi = FieldEventReadMemoryU8(2, 4);
    itemId = (itemHi & 0xFF) << 9;
    itemId |= FieldEventReadMemoryS16(1, 2);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("S item=", itemId, 4);
    }
    func_80025380(itemId);
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncDlitm(void) {
    s32 itemHi;
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("dlitm", 4);
    }
    itemHi = FieldEventReadMemoryU8(2, 4);
    itemId = (itemHi & 0xFF) << 9;
    itemId |= FieldEventReadMemoryS16(1, 2);
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("G item=", itemId, 4);
    }
    func_80025288(itemId);
    PC_INC(5);
    return 0;
}

s32 OpcodeFuncCkitm(void) {
    u16 itemId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ckitm", 4);
    }
    itemId = func_80025310(FieldEventReadMemoryS16(1, 2));
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("C item=", itemId, 4);
    }
    if (itemId == 0xFFFF) {
        itemId = 0;
    }
    FieldEventWriteMemoryU8(2, 4, itemId >> 9);
    PC_INC(5);
    return 0;
}

/////////////////////////////////////////////////
// Begin of field_opcode_special.c
/////////////////////////////////////////////////

extern u8 g_FieldMessageSpeed[1];
void func_80033A90(void);
void SystemMessageSetCharName(u8 charId, u8 nameId);

/* The "special" opcode: one byte of sub-opcode selects among eleven unrelated
 * jobs, from clearing the item/materia inventories to writing a character's
 * name into the message name buffer. Sub-opcodes run 0xF5..0xFF, which is what
 * the jump table's `(u32)(sub - 0xF5) < 0xB` guard checks.
 *
 * Four findings, all costly to re-derive:
 *   - `g_FieldMessageSpeed` is an array, not a scalar. As a plain `extern u8`
 * the store in the SMSPD arm is not MEM_IN_STRUCT_P, so true_dependence lets
 * the `g_FieldScriptPC[g_CurrentEntity]` load PC_INC needs float above it: gcc
 *     issues the `nor` immediately after the call, has to park it in $a1
 *     because $v0 has gone to &g_FieldScriptPC, and sinks the store past the
 *     PC address. Declared `u8[1]` and written `g_FieldMessageSpeed[0]`, the
 * store is in a struct too, the load is pinned below it, the `nor` lands in the
 *     load-delay slot of the `lbu` and keeps $v0 -- which is the target. That
 *     was the last row, after a park note had called it "post-reload
 *     scheduling with equal priorities on both sides"; it is aliasing, and it
 *     is the same lever AddBackgroundToRender needs run in the other
 *     direction. A named local for the complement, an `(s32)` cast on the
 *     operand and `-x - 1` in place of `~x` are all exactly inert, which is
 *     what says the expression was never the problem.
 *   - `itemId` must be an s32 local. Passing `i | 0xC600` straight to
 *     func_80025288(u16) lets combine narrow the ior to HImode, where the
 *     constant becomes -0x3a00 and can no longer be an `ori` immediate; gcc
 *     then hoists it into a callee-saved register and the frame grows to
 *     -0x38. A u16 local does not help -- it has to be s32.
 *   - the name copy walks (`*name++`), it does not index (`name[i]`). Indexing
 *     makes gcc build a separate giv and copy the base into it.
 *   - `len` is read before the switch. Reading it after, or folding it and the
 *     switch index into one variable, both cost ~13 rows. */
s32 OpcodeFuncSpcal(void) {
    u8* name;
    s32 itemId;
    u16 offset;
    u16 len;
    s32 i;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("spcal", 8);
    }
    switch (GET_PARAM_U8(1)) {
    case 0xFF:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("clitm", 8);
        }
        for (i = 0; i < 0x200; i++) {
            itemId = i | 0xC600;
            func_80025288(itemId);
        }
        PC_INC(2);
        return 0;
    case 0xFE:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("rsglb", 8);
        }
        func_80033A90();
        PC_INC(2);
        return 0;
    case 0xFD:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("spcnm", 8);
        }
        SystemMessageSetCharName(GET_PARAM_U8(2), GET_PARAM_U8(3));
        PC_INC(4);
        return 0;
    case 0xFC:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("mvlck", 2);
        }
        g_FieldMovieLock = GET_PARAM_U8(2);
        PC_INC(3);
        return 0;
    case 0xFB:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("btlck", 2);
        }
        g_FieldBattleLock = GET_PARAM_U8(2);
        PC_INC(3);
        return 0;
    case 0xFA:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("flitm", 8);
        }
        for (i = 0; i < 0x200; i++) {
            itemId = i | 0xC600;
            func_80025380(itemId);
        }
        PC_INC(2);
        return 0;
    case 0xF9:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("flmat", 8);
        }
        for (i = 0; i < 0x50; i++) {
            func_8002542C(i);
        }
        PC_INC(2);
        return 0;
    case 0xF8:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("smspd", 3);
        }
        g_FieldMessageSpeed[0] = ~FieldEventReadMemoryU8(4, 3);
        PC_INC(4);
        return 0;
    case 0xF7:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("gmspd", 3);
        }
        FieldEventWriteMemoryU8(4, 3, ~g_FieldMessageSpeed[0]);
        PC_INC(4);
        return 0;
    case 0xF6:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("pname", 8);
        }
        name = GetCharacterName(FieldEventReadMemoryU8(3, 3));
        offset = 0;
        len = GET_PARAM_U8(5);
        switch (GET_PARAM_U8(2) & 0xF) {
        case 15:
            offset += 0x100;
        case 13:
            offset += 0x100;
        case 11:
            offset += 0x100;
        case 3:
            offset += 0x100;
        }
        for (i = 0; i < len; i++) {
            ((u8*)D_8009D288)[offset + i] = *name++;
        }
        ((u8*)D_8009D288)[offset + i] = 0xFF;
        PC_INC(6);
        return 0;
    case 0xF5:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("arrow", 8);
        }
        g_FieldMovieOpcodeActive = GET_PARAM_U8(2);
        PC_INC(3);
        return 0;
    }
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("?????", 8);
    }
    PC_INC(2);
    return 0;
}
