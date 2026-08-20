//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 4 of 5, split out of field.c. .rodata 0x800A0104-0x800A0F10, base 4 mod
 * 8 -> --phase 4. The large middle run: 20 of the overlay's jump tables, all 4
 * mod 8. */

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiExecute);

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiSetCustomLightToModelPkts);

extern u8 D_800DF114;

/* Apply the GTE lighting to each vertex colour of a model's packets: for each
 * part's polygons run the NormalColorColSingle GTE op and write the result
 * into the packet's RGB. m2c seed; the residual is the GTE intrinsic codegen
 * (lwc2/nccs/swc2) and the per-part stride walking. Pinned pending a permuter
 * pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field4", KawaiSetVertexColorFromLighting);
#else
void KawaiSetVertexColorFromLighting(void* arg0) {
    s32* var_a3;
    u32 temp_t3;
    u32 temp_t3_2;
    u32 temp_t3_3;
    u32 temp_t3_4;
    u32 temp_t3_5;
    u32 temp_t3_6;
    u32 temp_t3_7;
    u32 temp_t3_8;
    u32 temp_t7;
    u32 temp_t7_2;
    u32 var_t0;
    u32 var_t0_2;
    u32 var_t0_3;
    u32 var_t0_4;
    u32 var_t0_5;
    u32 var_t0_6;
    u32 var_t0_7;
    u32 var_t0_8;
    u32 var_v1;
    u32 var_v1_2;
    u32 var_v1_3;
    u32 var_v1_4;
    u8* var_a1;
    u8* var_a1_2;
    u8* var_a1_3;
    u8* var_a1_4;
    u8* var_t2;
    u8* var_t2_2;
    u8* var_t2_3;
    u8* var_t2_4;

    var_a3 = arg0->unk1C;
    if (D_800DF114 != 0) {
        var_a3 += arg0->unk16;
    }
    temp_t7 = arg0->unk4;
    temp_t3 = temp_t7 & 0xFF;
    var_t0 = 0;
    if (temp_t3 != 0) {
        var_t2 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                var_v1 = 0;
                do {
                    M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $6, ($v0) */);
                    M2C_ERROR(/* unknown instruction: nccs */);
                    M2C_ERROR(/* unknown instruction: swc2 $22, ($a2) */);
                    var_v1 += 1;
                } while (var_v1 < 4U);
                *var_t2 = *var_t2;
            }
            var_t0 += 1;
            var_t2 += 0x34;
            var_a3 += 0x34;
        } while (var_t0 < temp_t3);
    }
    temp_t3_2 = (u32)(temp_t7 & 0xFF00) >> 8;
    var_t0_2 = 0;
    if (temp_t3_2 != 0) {
        var_t2_2 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                var_v1_2 = 0;
                do {
                    M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $6, ($v0) */);
                    M2C_ERROR(/* unknown instruction: nccs */);
                    M2C_ERROR(/* unknown instruction: swc2 $22, ($a2) */);
                    var_v1_2 += 1;
                } while (var_v1_2 < 3U);
                *var_t2_2 = *var_t2_2;
            }
            var_t0_2 += 1;
            var_t2_2 += 0x28;
            var_a3 += 0x28;
        } while (var_t0_2 < temp_t3_2);
    }
    temp_t3_3 = (temp_t7 >> 0x10) & 0xFF;
    var_t0_3 = 0;
    if (temp_t3_3 != 0) {
        var_a1 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $6, ($a2) */);
                M2C_ERROR(/* unknown instruction: nccs */);
                M2C_ERROR(/* unknown instruction: swc2 $22, ($a0) */);
                *var_a1 = *var_a1;
            }
            var_t0_3 += 1;
            var_a1 += 0x28;
            var_a3 += 0x28;
        } while (var_t0_3 < temp_t3_3);
    }
    temp_t3_4 = temp_t7 >> 0x18;
    var_t0_4 = 0;
    if (temp_t3_4 != 0) {
        var_a1_2 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $6, ($a2) */);
                M2C_ERROR(/* unknown instruction: nccs */);
                M2C_ERROR(/* unknown instruction: swc2 $22, ($a0) */);
                *var_a1_2 = *var_a1_2;
            }
            var_t0_4 += 1;
            var_a1_2 += 0x20;
            var_a3 += 0x20;
        } while (var_t0_4 < temp_t3_4);
    }
    temp_t7_2 = arg0->unk8;
    temp_t3_5 = temp_t7_2 & 0xFF;
    var_t0_5 = 0;
    if (temp_t3_5 != 0) {
        var_a1_3 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $6, ($a2) */);
                M2C_ERROR(/* unknown instruction: nccs */);
                M2C_ERROR(/* unknown instruction: swc2 $22, ($a0) */);
                *var_a1_3 = *var_a1_3;
            }
            var_t0_5 += 1;
            var_a1_3 += 0x14;
            var_a3 += 0x14;
        } while (var_t0_5 < temp_t3_5);
    }
    temp_t3_6 = (u32)(temp_t7_2 & 0xFF00) >> 8;
    var_t0_6 = 0;
    if (temp_t3_6 != 0) {
        var_a1_4 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                M2C_ERROR(/* unknown instruction: lwc2 $6, ($a2) */);
                M2C_ERROR(/* unknown instruction: nccs */);
                M2C_ERROR(/* unknown instruction: swc2 $22, ($a0) */);
                *var_a1_4 = *var_a1_4;
            }
            var_t0_6 += 1;
            var_a1_4 += 0x18;
            var_a3 += 0x18;
        } while (var_t0_6 < temp_t3_6);
    }
    temp_t3_7 = (temp_t7_2 >> 0x10) & 0xFF;
    var_t0_7 = 0;
    if (temp_t3_7 != 0) {
        var_t2_3 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                var_v1_3 = 0;
                do {
                    M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $6, ($v0) */);
                    M2C_ERROR(/* unknown instruction: nccs */);
                    M2C_ERROR(/* unknown instruction: swc2 $22, ($a2) */);
                    var_v1_3 += 1;
                } while (var_v1_3 < 3U);
                *var_t2_3 = *var_t2_3;
            }
            var_t0_7 += 1;
            var_t2_3 += 0x1C;
            var_a3 += 0x1C;
        } while (var_t0_7 < temp_t3_7);
    }
    temp_t3_8 = temp_t7_2 >> 0x18;
    var_t0_8 = 0;
    if (temp_t3_8 != 0) {
        var_t2_4 = var_a3 + 7;
        do {
            if (*var_a3 != 0) {
                var_v1_4 = 0;
                do {
                    M2C_ERROR(/* unknown instruction: lwc2 $0, ($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $1, 0x4($v0) */);
                    M2C_ERROR(/* unknown instruction: lwc2 $6, ($v0) */);
                    M2C_ERROR(/* unknown instruction: nccs */);
                    M2C_ERROR(/* unknown instruction: swc2 $22, ($a2) */);
                    var_v1_4 += 1;
                } while (var_v1_4 < 4U);
                *var_t2_4 = *var_t2_4;
            }
            var_t0_8 += 1;
            var_t2_4 += 0x24;
            var_a3 += 0x24;
        } while (var_t0_8 < temp_t3_8);
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
 * (arg1) is four bytes: two mouth frames, one eye frame, and a "has animation"
 * flag; values 0x21+ in the flag mean the model has no animated face and the
 * function is a no-op. Each present variant is looked up in a per-textureFaceId
 * index table (mouth: stride 7, eye: stride 3, 0x7E = none) and the matching
 * 0x200-byte page of the model's texture block is uploaded to its VRAM tile.
 *
 * Semantically right, codegen pinned via MASPSX_OVERRIDE: the verified C is the
 * #else. The target keeps the table base in a callee-saved register and
 * strength-reduces faceId*7 / faceId*3; gcc picks different registers. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field4", KawaiLoadEyesMouthTexToVram);
#else
extern u8 D_800DFCA4[]; /* mouth texture index table, stride 7 per face */
extern u8 D_800DFD94[]; /* eye texture index table, stride 3 per face */

s32 KawaiLoadEyesMouthTexToVram(FieldModelEntry* model, u8* faceSel) {
    RECT rect;
    u8* texBlock;
    u8* texData;
    s32 n;
    s32 q;
    u8 faceId;

    n = faceSel[3];
    if (n >= 0x21) {
        return 1;
    }
    texBlock = (u8*)D_800DFCA0;
    faceId = model->textureFaceId;

    /* First mouth frame. */
    q = n >> 2;
    rect.x = ((n - q * 4) << 4) + 0x300;
    rect.y = (q << 5) + 0x100;
    rect.w = 8;
    rect.h = 0x20;
    texData = *(u8**)(texBlock + 8);
    LoadImage(
        &rect, (u_long*)(texData + (D_800DFCA4[faceId * 7 + faceSel[0]] << 9)));

    /* Second mouth frame, one tile to the right. */
    rect.x = ((n - q * 4) << 4) + 0x308;
    rect.y = (q << 5) + 0x100;
    rect.w = 8;
    rect.h = 0x20;
    texData = *(u8**)(texBlock + 8);
    LoadImage(
        &rect, (u_long*)(texData + (D_800DFCA4[faceId * 7 + faceSel[1]] << 9)));

    /* Eye frame. */
    q = n >> 3;
    rect.x = ((n - q * 8) << 3) + 0x300;
    rect.y = (q << 5) + 0x1A0;
    rect.w = 8;
    rect.h = 0x20;
    texData = *(u8**)(texBlock + 8);
    LoadImage(
        &rect, (u_long*)(texData + (D_800DFD94[faceId * 3 + faceSel[2]] << 9)));
    return 1;
}
#endif

INCLUDE_ASM("asm/us/field/nonmatchings/field4", KawaiLightingApplyToModel);

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

/* Light one model part in place. Every polygon colour word carries the index
 * of its vertex normal in the byte the GPU would read as `code`, so each is
 * fed to NormalColorColSingle and the result written back over the same three
 * bytes. The eight groups differ only in the primitive's stride and how many
 * colour words it carries. Bit 1 of the part's data header marks it lit;
 * `redo` forces the pass to run again.
 *
 * The first function in the repo to use inline GTE ops -- gte_ldrgb, gte_nccs
 * and gte_strgb were added to include/psxsdk/libgte.h for it, in the style of
 * the ones already there. Three findings are folded in and each has a CLAUDE.md
 * bullet:
 *   - the normal table's base is a `u8*` scaled by hand, not an `SVECTOR*`
 *     indexed with []; as a typed pointer gcc rebuilds its %hi/%lo in all eight
 *     loop preheaders instead of keeping one register for the function;
 *   - `c = poly` snapshots the base the inner loop indexes off, which is what
 *     the target's `move t3,t0` at each inner preheader is;
 *   - the per-polygon bump lives in the `for` increment, so `i++` is emitted
 *     ahead of `poly += stride` as the target has it.
 *
 * Instruction for instruction identical bar one register-allocation decision:
 * the target puts the loop count in t2 and the snapshot in t3, this build the
 * other way round, which is every one of the 32 remaining rows. The snapshot
 * wins because it is referenced at loop depth 2 and the count at depth 1, and
 * nothing that changes their reference counts moves it -- measured: the count
 * inlined into the loop condition (77 rows), a u8 count (77), eight separate
 * count variables (34, gcc coalesces them), and the snapshot narrowed to the
 * gte_ldrgb address alone or widened to all eight loops (34 each). The other
 * two rows are the prologue: the target fills the delay slot of the part-data
 * load with the normal table's address, this build with a nop and the address
 * after; neither statement order reproduces it. A permuter candidate.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE(
    "asm/us/field/nonmatchings/field4", KawaiLightingApplyToPolyColor);
#else
void KawaiLightingApplyToPolyColor(FieldModelPart* part, s32 redo) {
    u8* scratch;
    u8* normals;
    u8* data;
    u8* poly;
    u8* c;
    u32 counts;
    u32 count;
    u32 i;
    u32 k;

    scratch = (u8*)0x1F800000;
    data = part->data;
    normals = (u8*)D_800DF520;
    if ((*(u32*)data & 2) && redo == 0) {
        return;
    }

    poly = (u8*)(part->polyOffset + (u32)data);
    counts = part->polyCounts0;

    count = counts & 0xFF;
    for (i = 0; i < count; i++, poly += 0x18) {
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
#endif

/* Set the semi-transparency/shade bits of every packet of every part of one
 * model. Walks each part's double-buffered packet area (the two ordering-table
 * copies) and toggles the ABE and shade bits of each primitive's tag byte. The
 * 8 unrolled primitive-type blocks (strides 34/28/28/20/14/18/1C/24) and the
 * dual walking-pointer tag/base induction are the wall; codegen pinned via
 * MASPSX_OVERRIDE, #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", KawaiSetModelTransparency);
#else
s32 KawaiSetModelTransparency(FieldModelEntry* model, u8* data) {
    u8* parts;
    u8* part;
    u8* base;
    u8* tag;
    u32 partCount;
    u32 enable;
    u32 i;
    u32 ot;
    u32 j;
    u32 n;

    parts = model->modelData + model->partsOffset;
    partCount = model->partCount;
    enable = data[0];
    if (partCount == 0) {
        return 1;
    }
    part = parts;
    for (i = 0; i < partCount; i++, part += 0x20) {
        for (ot = 0; ot < 2; ot++) {
            base = *(u8**)(part + 0x1C);
            if (ot != 0) {
                base += *(u16*)(part + 0x16);
            }
            n = part[4];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x34, base += 0x34) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[5];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x28, base += 0x28) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[6];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x28, base += 0x28) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[7];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x20, base += 0x20) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[8];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x14, base += 0x14) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[9];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x18, base += 0x18) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[10];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x1C, base += 0x1C) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
            n = part[11];
            for (j = 0, tag = base + 7; j < n; j++, tag += 0x24, base += 0x24) {
                if (enable) {
                    *tag |= 2;
                } else {
                    *tag &= ~2;
                }
                if (enable) {
                    *tag |= 1;
                } else {
                    *tag &= ~1;
                }
            }
        }
    }
    return 1;
}
#endif

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

extern u8 D_800DFE1C[]; /* scratch RGB quad, 0x20 before the table */
extern KawaiColorFadeSlot D_800DFE3C[16];

/* The slot table starts 0x20 past the scratch quad. Reaching it that way,
 * rather than through its own D_800DFE3C symbol, is what lets cse hand the
 * scratch's own address back as `-0x20($a2)` off the table base register. */
#define KawaiFadeSlots ((KawaiColorFadeSlot*)(D_800DFE1C + 0x20))

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
 *   - the slot table is reached as `D_800DFE1C + 0x20`, not through its own
 *     D_800DFE3C symbol. cse links two constants only when they share a
 *     symbol_ref base, so spelling it this way is what lets it hand the
 *     scratch quad's own address back as `-0x20($a2)` off the table base
 *     register -- both for the first scratch store and for the call argument.
 *     Named through D_800DFE3C, gcc materialises a second base register.
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
        D_800DFE1C[0] = slot->curR;
        D_800DFE1C[1] = slot->curR >> 8;
        D_800DFE1C[2] = slot->curG;
        D_800DFE1C[3] = slot->curG >> 8;
        D_800DFE1C[4] = slot->curB;
        D_800DFE1C[5] = slot->curB >> 8;
        D_800DFE1C[6] = slot->unk12;
        KawaiSetColorToModelPkts(model, D_800DFE1C);
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
 * D_800DFE1C scratch buffer and calls the handwritten GTE driver, returning 0;
 * any other sub-command returns 0. The slot reuses the KawaiFadeModelColor
 * table's 0x3C stride with a flat lighting-blob layout.
 *
 * Same recipe as KawaiFadeModelColor: a switch rather than two ifs, the table
 * reached as `D_800DFE1C + 0x20` so cse can hand the scratch quad's address
 * back as `-0x20($a3)`, and 0x38 of stack reserved for locals the original
 * allocates and never uses. The seed also had the byte/word boundary one byte
 * short in both arms, and read each pair through a `u16 pair` local: the
 * target reads the low half as a byte (`lbu`) and the high half through its
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
        D_800DFE1C[0x00] = slot[0x00];
        D_800DFE1C[0x01] = slot[0x01];
        D_800DFE1C[0x02] = slot[0x02];
        D_800DFE1C[0x03] = slot[0x03];
        D_800DFE1C[0x04] = slot[0x04];
        D_800DFE1C[0x05] = slot[0x05];
        D_800DFE1C[0x06] = slot[0x06];
        D_800DFE1C[0x07] = slot[0x07];
        D_800DFE1C[0x08] = slot[0x08];
        D_800DFE1C[0x09] = slot[0x09];
        D_800DFE1C[0x0A] = slot[0x0A];
        D_800DFE1C[0x0B] = slot[0x0B];
        D_800DFE1C[0x0C] = *(u16*)(slot + 0x0C);
        D_800DFE1C[0x0D] = *(u16*)(slot + 0x0C) >> 8;
        D_800DFE1C[0x0E] = *(u16*)(slot + 0x0E);
        D_800DFE1C[0x0F] = *(u16*)(slot + 0x0E) >> 8;
        D_800DFE1C[0x10] = *(u16*)(slot + 0x10);
        D_800DFE1C[0x11] = *(u16*)(slot + 0x10) >> 8;
        D_800DFE1C[0x12] = *(u16*)(slot + 0x12);
        D_800DFE1C[0x13] = *(u16*)(slot + 0x12) >> 8;
        D_800DFE1C[0x14] = *(u16*)(slot + 0x14);
        D_800DFE1C[0x15] = *(u16*)(slot + 0x14) >> 8;
        D_800DFE1C[0x16] = *(u16*)(slot + 0x16);
        D_800DFE1C[0x17] = *(u16*)(slot + 0x16) >> 8;
        D_800DFE1C[0x18] = *(u16*)(slot + 0x18);
        D_800DFE1C[0x19] = *(u16*)(slot + 0x18) >> 8;
        D_800DFE1C[0x1A] = *(u16*)(slot + 0x1A);
        D_800DFE1C[0x1B] = *(u16*)(slot + 0x1A) >> 8;
        D_800DFE1C[0x1C] = *(u16*)(slot + 0x1C);
        D_800DFE1C[0x1D] = *(u16*)(slot + 0x1C) >> 8;
        KawaiSetCustomLightToModelPkts(model, D_800DFE1C);
        return 0;
    }
    return 0;
}

/* Fade a model's vertex colour over time below a light level (KAWAI
 * sub-command). Four channels (cur@0/2/4/6, target@8/A/C/E, delta@10/12/14/16,
 * unk18@0x18, done@0x19 in the 0x3C-stride slot table). data[0]==0 inits the
 * slot from twelve LE u16 descriptor words and returns 1; data[0]==1 exports
 * the four cur channels + unk18 to the D_800DFE1C scratch buffer, pushes them
 * to the below-level packets, advances each channel toward its target with the
 * sign-aware clamp, bumps done once all four reach 0xF, and returns 0; any
 * other sub-command returns 1.
 *
 * The four-channel twin of KawaiFadeModelColor and it needs every one of that
 * function's spellings: the switch, the table reached as `D_800DFE1C + 0x20`,
 * `done = 0` at the top of the arm so it lives across the packet push, one
 * shared clamp block per channel reached by two gotos, and the last channel's
 * positive arm written out as an explicit `goto cur3clamp` / `goto cur3done`
 * pair. 0x50 of dead locals here rather than 0x38.
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
        D_800DFE1C[0] = slot->cur0;
        D_800DFE1C[1] = slot->cur0 >> 8;
        D_800DFE1C[2] = slot->cur1;
        D_800DFE1C[3] = slot->cur1 >> 8;
        D_800DFE1C[4] = slot->cur2;
        D_800DFE1C[5] = slot->cur2 >> 8;
        D_800DFE1C[6] = slot->cur3;
        D_800DFE1C[7] = slot->cur3 >> 8;
        D_800DFE1C[8] = slot->unk18;
        KawaiSetColorToPktsBelowLvl(model, D_800DFE1C);
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
 * page and CLUT, with the part's y offset negated into the second sprite.
 *
 * 12 rows out, and all twelve are one thing: the packet base is
 * strength-reduced onto `pkt + 0x58` (`addiu a0,s0,0xb4` and negative
 * displacements throughout) where the original keeps the plain base. That is
 * combine_givs picking the last address in the body as its representative --
 * the `-*parts` store at +0x58 -- and it happens for every spelling measured:
 * `pkt = &base[i * 0x5C]` at the top of the body, the same index written
 * inline at all 22 accesses with no pointer local at all, and a walked
 * `pkt += 0x5C`. The walked form gets the loop bound into a register
 * (`li t3,0x1f` / `slt`, which the target has and the indexed form's `slti`
 * does not) but costs a fourth saved register, so it measures 31 rows against
 * 12; a named `count` local for the bound made it 42.
 *
 * What the same pass did fix, from 43 rows: `tex` is a local holding 0x6C2C
 * assigned before the GetGraphType tests, which is what makes it live across
 * the second call and puts it in $s1 -- read back out of the asm as a literal
 * in the loop it lands in a caller-saved register and the whole preheader
 * renumbers. And the packet address must be indexed off the counter, not
 * walked, for the frame and saved-register list to come out right. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", KawaiInitSplashPkts);
#else
void KawaiInitSplashPkts(void* arg0, s32 arg1) {
    s16 clut;
    s16 tex;
    s32 i;
    u8* pkt;
    u16* parts;
    u8* base;

    base = (u8*)D_800E0200 + arg1 * 0xAC8;
    tex = 0x6C2C;
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        clut = 0x22B;
    } else {
        clut = 0x9B;
    }
    parts = (u16*)(*(u32*)((u8*)arg0 + 0x1C) + 4);
    for (i = 1; i < 0x1F; i++) {
        pkt = &base[i * 0x5C];
        pkt[0x7] = 0x2C;
        pkt[0x2F] = 0x2C;
        pkt[0x3] = 9;
        pkt[0x2B] = 9;
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
        *(s16*)(pkt + 0x5A) = 0;
        *(s16*)(pkt + 0x58) = -*(s16*)parts;
        parts += 2;
    }
}
#endif

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

extern u8 D_800716D4;
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
    if (D_800716D4 == 0) {
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
        if (g_FieldScriptRunState != 5 || D_80070788 != 0) {
            FieldEventOpcodeCycle();
        }
    }
    if (g_WindowCount) {
        SystemMenuDrawDialog(
            g_WindowData, 4, arg0, g_FieldState->renderBuffer ^ 1);
    }
    UpdateFieldExitArrows(arg0);
}

extern /*?*/ s32 D_8007078C;
extern s16 D_800716DC;
extern /*?*/ s32 D_80071748;
extern /*?*/ s32 D_80071A88;
extern s8 D_80075F23;
extern /*?*/ s32 D_8007E7AC;
extern /*?*/ s32 D_8007E7AE;
extern /*?*/ s32 D_8007E7B0;
extern /*?*/ s32 D_8007E7B2;
extern /*?*/ s32 D_8007E7B4;
extern /*?*/ s32 D_8007E7B6;
extern /*?*/ s32 D_8007E7B8;
extern /*?*/ s32 D_8007E7B9;
extern /*?*/ s32 D_8007E7BA;
extern /*?*/ s32 D_8007E7BB;
extern /*?*/ s32 D_8007E7BC;
extern /*?*/ s32 D_8007E7BD;
extern /*?*/ s32 D_8007E7BE;
extern /*?*/ s32 D_8007E7BF;
extern /*?*/ s32 D_8007E7C2;
extern /*?*/ s32 D_800833F8;
extern s16 D_80095D84;
extern /*?*/ s32 D_8009A1C4;
extern s8 D_8009AD38;

/* Zero and default-initialise the whole field runtime state: the entity table,
 * the per-model flags, the script state, and the various counters. m2c seed
 * (three locals m2c left untyped -- see the note in the body). Residual is
 * the wide store scheduling.
 * Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldInitDefaultValues);
#else
void FieldInitDefaultValues(void) {
    /* m2c could not infer these three; u8* is a placeholder chosen so the
     * unit parses, not a claim about the real type. The strides the body
     * walks them by (8, 8, 0x10) and the byte store through var_t0 are
     * consistent with it. Settle the real types when this body is worked. */
    u8* var_t0;
    u8* var_t1;
    u8* var_t2;
    s16* var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a3;
    s32 var_a3_2;
    s32 var_a3_3;
    s32 var_a3_4;
    s32 var_a3_5;
    s32 var_a3_6;
    s32 var_a3_7;
    s32 var_a3_8;
    s32 var_a3_9;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1_3;
    s8* temp_a0;
    s8* temp_v1;
    s8* var_a0;
    s8* var_v0_4;
    u8(*var_t3)[8];
    u8* temp_a1;
    u8* var_v0_3;
    u8* var_v1_2;
    void* temp_v0;
    void* temp_v0_2;
    void* temp_v0_3;
    void* temp_v0_4;
    void* temp_v0_5;
    void* temp_v0_6;
    void* temp_v1_2;
    void* temp_v1_3;
    void* temp_v1_4;

    D_8009C6E0->unk1 = 0;
    D_8009C6E0->unk2 = 0;
    D_8009C6E0->unk26 = 0;
    D_8009C6E0->unk32 = 0;
    D_8009C6E0->unk2E = 1;
    D_8009C6E0->unk2A = 0;
    D_8009C6E0->unk2C = 0;
    D_8009C6E0->unk30 = 2;
    D_80081DC4 = 0;
    D_8009C6E0->unk28 = (s16)D_8009C6DC->unk3;
    D_8009C6E0->unk33 = 0;
    D_8009C6E0->unk34 = 0;
    D_8009C6E0->unk35 = 0;
    D_8009C6E0->unk3B = 0;
    D_8009C6E0->unk36 = 0;
    D_8009C6E0->unk37 = 0;
    D_8009C6E0->unk3D = 0;
    D_8009C6E0->unk48 = 0;
    D_8009C6E0->unk44 = 0;
    D_8009C6E0->unk40 = 0;
    D_8009C6E0->unk3E = 0;
    D_8009C6E0->unk3C = 0;
    D_8009C6E0->unk12 = 0;
    D_8009C6E0->unk13 = 0;
    D_8009C6E0->unk14 = 0;
    D_8009C6E0->unk8A = 0;
    D_8009C6E0->unk18 = 0;
    D_8009C6E0->unk1A = 0;
    D_8009C6E0->unk98 = 0;
    D_8009C6E0->unk8B = 0;
    D_8009C6E0->unk99 = 0;
    D_8009C6E0->unk3A = 0;
    var_a3 = 0xFF;
    D_8009C6E0->unk8E = 0;
    D_8009C6E0->unk9C = 0;
    D_8009C6E0->unk94 = 0;
    D_8009C6E0->unkA2 = 0;
    D_8009C6E0->unk96 = 0;
    D_8009C6E0->unkA4 = 0;
    D_8009C6E0->unk1D = 0;
    var_a0 = &D_80075F23;
    D_8009C6E0->unk10 = (u16)D_8009C6DC->unk8;
    do {
        *var_a0 = 0;
        var_a3 -= 1;
        var_a0 -= 1;
    } while (var_a3 >= 0);
    var_a3_2 = 0;
    do {
        var_a2 = 0;
        if ((s32)D_8009C6DC->unk2 > 0) {
            var_t3 = SavedScriptIds;
            var_t2 = &D_80071A88;
            var_t1 = &D_800833F8;
            var_t0 = &D_80071748;
            do {
                temp_a1 = &(*var_t3)[var_a3_2];
                var_t3 += 8;
                temp_a0 = var_t2 + var_a3_2;
                var_t2 += 8;
                temp_v1 = var_t1 + var_a3_2;
                var_t1 += 8;
                *((var_a3_2 * 2) + var_t0) = 0;
                *temp_v1 = 0;
                *temp_a0 = 0xFF;
                *temp_a1 = 0;
                var_a2 += 1;
                var_t0 += 0x10;
            } while (var_a2 < (s32)D_8009C6DC->unk2);
        }
        var_a3_2 += 1;
    } while (var_a3_2 < 8);
    var_a3_3 = 0;
    if ((s32)D_8009C6DC->unk2 > 0) {
        var_v1 = &D_800716DC;
        do {
            *(&D_8009A1C4 + var_a3_3) = 7;
            *(&D_8007EB98 + var_a3_3) = 0xFF;
            *var_v1 = 0;
            *(&D_80081D90 + var_a3_3) = 0;
            *(&D_8007078C + var_a3_3) = 0xFF;
            g_FieldScriptDebugEntities[var_a3_3] = 0;
            var_a3_3 += 1;
            var_v1 += 2;
        } while (var_a3_3 < (s32)D_8009C6DC->unk2);
    }
    var_a3_4 = 0;
    if ((s32)D_8009C6DC->unk3 > 0) {
        var_a1 = 0;
        do {
            temp_v0 = var_a1 + D_8009C544;
            temp_v0->unk36 = 0;
            temp_v0->unk66 = 0;
            temp_v0->unkC = 0;
            temp_v0->unk10 = 0;
            temp_v0->unk14 = 0;
            temp_v0->unk72 = 0;
            temp_v0->unk74 = 0;
            (var_a1 + D_8009C544)->unk38 = 0;
            (var_a1 + D_8009C544)->unk3B = 0;
            (var_a1 + D_8009C544)->unk39 = 0;
            (var_a1 + D_8009C544)->unk3A = 0;
            temp_v0_2 = var_a1 + D_8009C544;
            temp_v0_2->unk56 = 0;
            temp_v0_2->unk3C = 0;
            temp_v0_2->unk3E = 0;
            temp_v0_2->unk40 = 0;
            temp_v0_2->unk46 = 0;
            temp_v0_2->unk4C = 0;
            temp_v0_2->unk42 = 0;
            temp_v0_2->unk48 = 0;
            temp_v0_2->unk4E = 0;
            temp_v0_2->unk44 = 0;
            temp_v0_2->unk4A = 0;
            temp_v0_2->unk50 = 0;
            temp_v0_2->unk52 = 0;
            temp_v0_2->unk54 = 0;
            (var_a1 + D_8009C544)->unk5E = 0;
            temp_v0_3 = var_a1 + D_8009C544;
            temp_v0_3->unk60 = 0x10;
            temp_v0_3->unk5C = 0;
            temp_v0_3->unk78 = 0;
            temp_v0_3->unk7C = 0;
            temp_v0_3->unk80 = 0;
            temp_v0_3->unk62 = 0;
            temp_v0_3->unk64 = 0;
            temp_v0_4 = var_a1 + D_8009C544;
            temp_v0_4->unk5D = 0;
            temp_v1_2 = var_a1 + D_8009C544;
            temp_v0_4->unk70 = (s16)((s16)D_8009C6E0->unk10 * 2);
            temp_v1_2->unk5A = 0;
            temp_v1_2->unk68 = 0;
            temp_v1_2->unk6A = 0;
            (var_a1 + D_8009C544)->unk58 = 0;
            (var_a1 + D_8009C544)->unk59 = 0;
            (var_a1 + D_8009C544)->unk5B = 0;
            (var_a1 + D_8009C544)->unk37 = 0;
            var_v0 = (s16)D_8009C6E0->unk10 * 0x1E;
            temp_v1_3 = var_a1 + D_8009C544;
            if (var_v0 < 0) {
                var_v0 += 0x1FF;
            }
            temp_v1_3->unk6C = (s16)(var_v0 >> 9);
            var_v0_2 = (s16)D_8009C6E0->unk10 * 0x50;
            if (var_v0_2 < 0) {
                var_v0_2 += 0x1FF;
            }
            temp_v1_3->unk6E = (s16)(var_v0_2 >> 9);
            D_8008325C[var_a3_4] = 0;
            D_800756E8[var_a3_4] = 0;
            D_8009D828[var_a3_4] = 0x10;
            D_80082248[var_a3_4] = 0x10;
            temp_v1_4 = var_a1 + D_8009C544;
            temp_v1_4->unk8 = 0;
            temp_v1_4->unk0 = 0;
            temp_v1_4->unk2 = 0;
            temp_v1_4->unk4 = 0;
            (var_a1 + D_8009C544)->unk9 = 0;
            var_a3_4 += 1;
            var_a1 += 0x84;
        } while (var_a3_4 < (s32)D_8009C6DC->unk3);
    }
    var_a3_5 = 0;
    do {
        temp_v0_5 = D_8009C6E0 + var_a3_5;
        var_a3_5 += 1;
        temp_v0_5->unkF2 = 0;
    } while (var_a3_5 < 0x40);
    var_a3_6 = 0;
    do {
        temp_v0_6 = D_8009C6E0 + var_a3_6;
        var_a3_6 += 1;
        temp_v0_6->unkB2 = 0;
    } while (var_a3_6 < 0x40);
    var_a3_7 = 0;
    var_v1_2 = D_80095DE0;
    do {
        var_a2_2 = 0xF;
        var_v0_3 = var_v1_2 + 0x1E;
    loop_23:
        *var_v0_3 = 0;
        var_a2_2 -= 1;
        var_v0_3 -= 2;
        if (var_a2_2 >= 0) {
            goto loop_23;
        }
        var_a3_7 += 1;
        var_v1_2 += 0x20;
    } while (var_a3_7 < 0x40);
    var_a3_8 = 0;
    var_v1_3 = 0;
    do {
        *(&D_8007E7BD + var_v1_3) = 0;
        *(&D_8007E7BC + var_v1_3) = 0;
        *(&D_8007E7BB + var_v1_3) = 0;
        *(&D_8007E7BA + var_v1_3) = 0;
        *(&D_8007E7BE + var_v1_3) = 0;
        *(&D_8007E7BF + var_v1_3) = 0;
        *(&D_8007E7B8 + var_v1_3) = 0;
        *(&D_8007E7B9 + var_v1_3) = 0;
        *(&D_8007E7C2 + var_v1_3) = 0;
        *(&D_8007E7AC + var_v1_3) = 0;
        *(&D_8007E7AE + var_v1_3) = 0;
        *(&D_8007E7B0 + var_v1_3) = 0;
        *(&D_8007E7B2 + var_v1_3) = 0;
        *(&D_8007E7B4 + var_v1_3) = 0;
        *(&D_8007E7B6 + var_v1_3) = 0;
        var_a3_8 += 1;
        var_v1_3 += 0x18;
    } while (var_a3_8 < 0x20);
    D_80095D84 = 0;
    var_a3_9 = 8;
    var_v0_4 = &D_8009AD38;
    do {
        *var_v0_4 = 0xFF;
        var_a3_9 -= 1;
        var_v0_4 -= 1;
    } while (var_a3_9 >= 0);
    g_EntityForSplitJoin = 0xFF;
    g_FieldMovieOpcodeActive = 0;
    Savemap.memory_bank_1[31] |= 3;
}
#endif

/* Walks every entity's first script and runs its initialisation opcodes
 * (everything up to the terminating 0). The script-offset table sits past the
 * entity-name table and the extras table in the script header. Semantically
 * correct; codegen pinned via MASPSX_OVERRIDE: gcc 2.6.3 fixes the address
 * arithmetic order (the <<6/<<3/<<1 sequence) and re-materialises the script
 * pointer, a conserved-pair the permuter plateaus on (best 1075 after the
 * override-strip fix, iter 55k). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventRunInit);
#else
void FieldEventRunInit(void) {
    s16 numExtras;
    s32 scriptBase;
    u16 pc;
    u16* slot;
    u16* slot2;
    u8 lo;
    u8 op;
    u8 op2;

    g_FieldModelCount = 0;
    g_CurrentEntity = 0;
    if (g_FieldScripts->numEntities != 0) {
        do {
            if (g_FieldScriptDebugFlags & 3) {
                FieldDebugStringCopy(g_DebugText, &D_800E0628);
                FieldDebugStringConcat(
                    g_DebugText,
                    (u8*)g_FieldScripts + (g_CurrentEntity * 8) + 0x20);
                if (g_FieldScriptDebugFlags & 1) {
                    SetStrToDebugRow(4, 0, g_DebugText);
                }
                if (g_FieldScriptDebugFlags & 2) {
                    DebugPrintToFieldWindow(g_DebugText);
                }
            }
            scriptBase = g_CurrentEntity << 6;
            numExtras = g_FieldScripts->numExtras * 4;
            lo = ((u8*)g_FieldScripts + scriptBase +
                  (g_FieldScripts->numEntities * 8) + numExtras)[0x20];
            slot = (u16*)((g_CurrentEntity * 2) + (u8*)g_FieldScriptPC);
            *slot = (u16)lo;
            *slot = lo | (((u8*)g_FieldScripts + scriptBase +
                           (g_FieldScripts->numEntities * 8) + numExtras)[0x21]
                          << 8);
            op = *((u8*)g_FieldScripts + *slot);
            g_FieldCurrentOpcode = op;
            if (op != 0) {
                do {
                    g_FieldOpcodes[g_FieldCurrentOpcode]();
                    op2 = *((u8*)g_FieldScripts +
                            *((u16*)((g_CurrentEntity * 2) +
                                     (u8*)g_FieldScriptPC)));
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
 * slot was not claimed. Codegen pinned via MASPSX_OVERRIDE: the #else body is
 * the verified-correct C; its bytes come from the reference .s (the
 * s16-walking-counter strength-reduction wall). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEnablePartyModels);
#else
void FieldEnablePartyModels(void) {
    s16 i;
    s16 j;
    s16 modelCount;
    u8 charId;
    u8 modelId;

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
        if (g_EntityToModel[modelId] == 0xFF) {
            continue;
        }
        if (g_EntityToModel[modelId] <
            ((FieldModelFileDesc*)D_8007E770)->count) {
            g_FieldModelLoaderData[g_EntityToModel[modelId]].npcFlag = 1;
        }
    }

    /* Disable every model whose loader slot was not claimed above. */
    modelCount = ((FieldModelFileDesc*)D_8007E770)->count;
    if (modelCount != 0) {
        for (i = 0; i < modelCount; i++) {
            if (g_FieldModelLoaderData[i].npcFlag == 0) {
                if (i < g_FieldScripts->numModels) {
                    for (j = 0; j < g_FieldScripts->numModels; j++) {
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
    }
}
#endif

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
                    if (++D_8009A064 >= 8) {
                        D_8009A064 = 0;
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
        D_80070788 = 0;
    }
    FieldUpdateAnimationState();
}

extern /*?*/ s32 D_80074F02;
extern void* D_8009C6E0;
extern u8 D_8007EB98[];
extern s32 D_8009C544;
extern /*?*/ s32 D_8009C6DC;

/* Per-frame animation state machine for every loaded model: dispatch on the
 * model's animation state, advance the current frame by the playback speed,
 * and wrap/transition at the last frame. m2c seed; residual is regalloc across
 * the switch and the per-model address CSE. Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldUpdateAnimationState);
#else
void FieldUpdateAnimationState(void) {
    FieldModelEntry* temp_v0;
    FieldModelEntry* temp_v0_2;
    s32 temp_a3;
    s32 temp_v1_4;
    s32 var_t2;
    s32* var_t0;
    u8 temp_a0;
    u8 temp_a0_2;
    u8 temp_a0_3;
    u8 temp_a0_4;
    u8 temp_a0_5;
    u8 temp_a0_6;
    u8 temp_v1;
    u8 temp_v1_2;
    u8 temp_v1_3;
    void* temp_a2;
    void* temp_v1_5;
    void* temp_v1_6;
    void* temp_v1_7;
    void* temp_v1_8;
    void* var_a0;

    var_t2 = 0;
    if ((s32)D_8009C6DC->unk2 > 0) {
        var_t0 = &D_8007EB98;
        do {
            temp_a0 = *var_t0;
            if ((temp_a0 != 0xFF) &&
                ((D_8009C6E0->unk2A != temp_a0) || (D_8009C6E0->unk32 != 0))) {
                temp_v1 = D_800756E8[temp_a0];
                switch (temp_v1) {
                case 0:
                    temp_a0_2 = *var_t0;
                    temp_a3 = temp_a0_2 * 0x84;
                    temp_a2 = temp_a3 + D_8009C544;
                    temp_v1_2 = D_8008325C[temp_a0_2];
                    if (temp_a2->unk5E != temp_v1_2) {
                        temp_a2->unk5E = temp_v1_2;
                        temp_v1_3 = *var_t0;
                        ((temp_v1_3 * 0x84) + D_8009C544)->unk60 =
                            (u16)D_80082248[temp_v1_3];
                        ((*var_t0 * 0x84) + D_8009C544)->unk62 = 0;
                        temp_a0_3 = *var_t0;
                        temp_v0 = &g_FieldModelData->modelEntries
                                       [g_FieldModelLoaderData[temp_a0_3]
                                            .modelEntryIndex];
                        temp_v1_4 = temp_a0_3 * 0x84;
                        (temp_v1_4 + D_8009C544)->unk64 =
                            (s16)(*((*(&D_80074F02 + temp_v1_4) * 0x10) +
                                    &temp_v0->modelData
                                         [temp_v0->animationOffset]) -
                                  1);
                    } else {
                        temp_v0_2 = &g_FieldModelData->modelEntries
                                         [g_FieldModelLoaderData[temp_a0_2]
                                              .modelEntryIndex];
                        temp_a2->unk64 =
                            (s16)(*((*(&D_80074F02 + temp_a3) * 0x10) +
                                    &temp_v0_2->modelData
                                         [temp_v0_2->animationOffset]) -
                                  1);
                        var_a0 = (*var_t0 * 0x84) + D_8009C544;
                    block_11:
                        if (((s32)(var_a0->unk62 << 0x10) >> 0x14) >=
                            var_a0->unk64) {
                            var_a0->unk62 = 0U;
                        }
                    }
                    break;
                case 1:
                    var_a0 = (*var_t0 * 0x84) + D_8009C544;
                    goto block_11;
                case 2:
                    temp_a0_4 = *var_t0;
                    temp_v1_5 = (temp_a0_4 * 0x84) + D_8009C544;
                    if (((s32)(temp_v1_5->unk62 << 0x10) >> 0x14) >=
                        temp_v1_5->unk64) {
                        D_800756E8[temp_a0_4] = 4;
                    case 3:
                    case 4:
                        temp_v1_6 = (*var_t0 * 0x84) + D_8009C544;
                        temp_v1_6->unk62 = (s16)(temp_v1_6->unk64 * 0x10);
                    }
                    break;
                case 5:
                    temp_a0_5 = *var_t0;
                    temp_v1_7 = (temp_a0_5 * 0x84) + D_8009C544;
                    if (((s32)(temp_v1_7->unk62 << 0x10) >> 0x14) >=
                        temp_v1_7->unk64) {
                        D_800756E8[temp_a0_5] = 0;
                    }
                    break;
                case 6:
                    temp_a0_6 = *var_t0;
                    temp_v1_8 = (temp_a0_6 * 0x84) + D_8009C544;
                    if (((s32)(temp_v1_8->unk62 << 0x10) >> 0x14) >=
                        temp_v1_8->unk64) {
                        D_800756E8[temp_a0_6] = 3;
                    }
                    break;
                }
            }
            var_t2 += 1;
            var_t0 += 1;
        } while (var_t2 < (s32)D_8009C6DC->unk2);
    }
}
#endif

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

    D_80114490 = 0;
    D_80114464 = 0x7FFF;
    D_80114468 = 0x7FFF;
    setPolyFT4(&D_800E48F4[0]);
    setPolyFT4(&D_800E48F4[1]);
    setSemiTrans(&D_800E48F4[0], 0);
    setSemiTrans(&D_800E48F4[1], 0);
    setShadeTex(&D_800E48F4[0], 1);
    setShadeTex(&D_800E48F4[1], 1);
    if (GetGraphType() == 1 || GetGraphType() == 2) {
        tpage = 0x2F;
    } else {
        tpage = 0x1F;
    }
    D_800E48F4[1].tpage = tpage;
    D_800E48F4[0].tpage = tpage;
    D_800E48F4[1].clut = 0x7850;
    D_800E48F4[0].clut = 0x7850;
    D_800E48F4[0].r0 = 0;
    D_800E48F4[1].r0 = 0;
    D_800E48F4[0].g0 = 0;
    D_800E48F4[1].g0 = 0;
    D_800E48F4[0].b0 = 0;
    D_800E48F4[1].b0 = 0;
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

extern /*?*/ s32 D_800E48FC;
extern /*?*/ s32 D_800E48FE;
extern /*?*/ s32 D_800E4900;
extern /*?*/ s32 D_800E4901;
extern /*?*/ s32 D_800E4904;
extern /*?*/ s32 D_800E4906;
extern /*?*/ s32 D_800E4908;
extern /*?*/ s32 D_800E4909;
extern /*?*/ s32 D_800E490C;
extern /*?*/ s32 D_800E490E;
extern /*?*/ s32 D_800E4910;
extern /*?*/ s32 D_800E4911;
extern /*?*/ s32 D_800E4914;
extern /*?*/ s32 D_800E4916;
extern /*?*/ s32 D_800E4918;
extern /*?*/ s32 D_800E4919;

/* Draw the field-exit arrow sprite (a POLY_FT4) at the given OT slot, pulsing
 * its colour over time. m2c seed; residual is the packet-field store ordering.
 * Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", DrawFieldExitArrow);
#else
void DrawFieldExitArrow(s32* arg0) {
    POLY_FT4* temp_v1_5;
    s16 temp_v1;
    s16 temp_v1_2;
    s16 temp_v1_3;
    s16 temp_v1_4;
    s16 var_a2;
    s16 var_a3;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_v0;

    if ((g_FieldMovieOpcodeActive == 0) &&
        ((D_80114464 != 0x7FFF) || (D_80114468 != D_80114464))) {
        var_a2 = D_80114464;
        if (D_80114464 >= 0x141) {
            var_a2 = 0x140;
        }
        if (D_80114464 < 0) {
            var_a2 = 0;
        }
        var_a3 = D_80114468;
        if (D_80114468 >= 0xE1) {
            var_a3 = 0xE0;
        }
        if (D_80114468 < 0) {
            var_a3 = 0;
        }
        D_80114490 ^= 1;
        if (var_a2 >= 0x123) {
            *(&D_800E4900 + (D_80114490 * 0x28)) = 0x8F;
            *(&D_800E4908 + (D_80114490 * 0x28)) = 0x7F;
            *(&D_800E4910 + (D_80114490 * 0x28)) = 0x8F;
            *(&D_800E4918 + (D_80114490 * 0x28)) = 0x7F;
            temp_v0 = D_80114490 * 0x28;
            temp_v1 = var_a2 - 0x10;
            *(&D_800E48FC + temp_v0) = temp_v1;
            *(&D_800E4904 + temp_v0) = var_a2;
            *(&D_800E490C + temp_v0) = temp_v1;
            *(&D_800E4914 + temp_v0) = var_a2;
            var_v0 = var_a3 << 0x10;
        } else {
            *(&D_800E4900 + (D_80114490 * 0x28)) = 0x80;
            *(&D_800E4908 + (D_80114490 * 0x28)) = 0x90;
            *(&D_800E4910 + (D_80114490 * 0x28)) = 0x80;
            *(&D_800E4918 + (D_80114490 * 0x28)) = 0x90;
            temp_v0_2 = D_80114490 * 0x28;
            temp_v1_2 = var_a2 + 0x10;
            *(&D_800E48FC + temp_v0_2) = var_a2;
            *(&D_800E4904 + temp_v0_2) = temp_v1_2;
            *(&D_800E490C + temp_v0_2) = var_a2;
            *(&D_800E4914 + temp_v0_2) = temp_v1_2;
            var_v0 = var_a3 << 0x10;
        }
        if ((var_v0 >> 0x10) < 0x11) {
            *(&D_800E4901 + (D_80114490 * 0x28)) = 0x6F;
            *(&D_800E4909 + (D_80114490 * 0x28)) = 0x6F;
            *(&D_800E4911 + (D_80114490 * 0x28)) = 0x5F;
            *(&D_800E4919 + (D_80114490 * 0x28)) = 0x5F;
            temp_v0_3 = D_80114490 * 0x28;
            temp_v1_3 = var_a3 + 0x10;
            *(&D_800E48FE + temp_v0_3) = var_a3;
            *(&D_800E4906 + temp_v0_3) = var_a3;
            *(&D_800E490E + temp_v0_3) = temp_v1_3;
            *(&D_800E4916 + temp_v0_3) = temp_v1_3;
        } else {
            *(&D_800E4901 + (D_80114490 * 0x28)) = 0x60;
            *(&D_800E4909 + (D_80114490 * 0x28)) = 0x60;
            *(&D_800E4911 + (D_80114490 * 0x28)) = 0x70;
            *(&D_800E4919 + (D_80114490 * 0x28)) = 0x70;
            temp_v0_4 = D_80114490 * 0x28;
            temp_v1_4 = var_a3 - 0x10;
            *(&D_800E48FE + temp_v0_4) = temp_v1_4;
            *(&D_800E4906 + temp_v0_4) = temp_v1_4;
            *(&D_800E490E + temp_v0_4) = var_a3;
            *(&D_800E4916 + temp_v0_4) = var_a3;
        }
        temp_v1_5 = &D_800E48F4[D_80114490];
        temp_v1_5->tag = (temp_v1_5->tag & 0xFF000000) | (*arg0 & 0xFFFFFF);
        *arg0 = (*arg0 & 0xFF000000) | ((s32)temp_v1_5 & 0xFFFFFF);
    }
}
#endif

/////////////////////////////////////////////////
// Begin of field_event_debug.c
/////////////////////////////////////////////////

INCLUDE_ASM("asm/us/field/nonmatchings/field4", DebugUpdateActor);

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

/* The #else body is a VERIFIED MATCH -- checkfn reports MATCH for it -- but it
 * cannot land alone. This function LENDS: its `.s` owns D_800A032C, the
 * "bank" diagnostic that FieldEventWriteMemoryU8, FieldEventReadMemoryS16 and
 * FieldEventWriteMemoryS16 all print, and all three are still pinned. Writing
 * the literal here emits a second copy under a local label and the link fails
 * with `undefined reference to D_800A032C`. The four have to land together;
 * the other three are at 16, 22 and 22 rows. (rodata_owner.py says SAFE, which
 * is its MASPSX_OVERRIDE blind spot -- it reads the `#else` bodies and cannot
 * tell that the pinned siblings still assemble their `.s`.) */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventReadMemoryU8);
#else
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
#endif

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventWriteMemoryU8);
#else
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
#endif

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventReadMemoryS16);
#else
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
#endif

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventWriteMemoryS16);
#else
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
#endif

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

// Depends on decomp of DebugUpdateActor due to shared string.
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventRequest);
#else
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
        FieldDebugStringConcat(g_DebugMessageBuffer, "/");
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
#endif

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

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncMjump);
#else
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
        g_FieldState->pcDirection = GET_PARAM_U8(9);
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
#endif

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
    D_8009A00C = (s16)FieldEventReadMemoryS16(3, 8);
    D_8009A010 = (s16)FieldEventReadMemoryS16(4, 10);
    D_8009A014 = (s16)FieldEventReadMemoryS16(6, 12);
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
    D_8009A00C = (s16)FieldEventReadMemoryS16(3, 9);
    D_8009A010 = (s16)FieldEventReadMemoryS16(4, 11);
    D_8009A014 = (s16)FieldEventReadMemoryS16(6, 13);
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
    D_8009A00C = (s16)FieldEventReadMemoryS16(4, 6);
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
 * resolves the tutorial's block into D_800E48E0 for the main loop to stream;
 * once the menu reports MOVCMD_DONE, clear the command and advance past the
 * operand. */
extern u8* D_800E48E0;

/* The #else body is a VERIFIED MATCH but cannot land alone: this function's
 * `.s` owns D_800A08D0, and src/field/field5.c reaches that string by symbol
 * (field_private.h:208). A C literal here becomes a local label, so field5
 * fails to link. Landing it means giving the string a home both units can
 * reach -- not a codegen problem. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncTutor);
#else
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
        D_800E48E0 = (u8*)g_FieldScripts + GetAkaoBlockOffset(tutorialId);
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
#endif

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
        D_8009D588 = GET_PARAM_U8(1);
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

/* CANIM (change animation): switch the current model's animation to a new id
 * read from the script, recompute the frame rate from the base speed and the
 * script's divisor, and clamp the last frame. Twin of OpcodeFuncCanmEx. m2c
 * seed; residual is the per-model address CSE and the divide scheduling.
 * Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncCanim);
#else
s32 OpcodeFuncCanim(void) {
    FieldModelEntry* temp_v1_5;
    s16 temp_v0;
    s32 temp_a1_4;
    s32 temp_lo;
    u16* temp_a0;
    u16* temp_a3;
    u16* temp_v1_2;
    u8 temp_a1;
    u8 temp_a1_2;
    u8 temp_a1_3;
    u8 temp_v1;
    u8 temp_v1_4;
    void* temp_v1_3;

    if (D_8009D820 & 3) {
        DebugPrintOpcode("canim", 4U);
    }
    temp_a1 = *(&D_8007EB98 + D_800722C4);
    if (temp_a1 != 0xFF) {
        temp_v1 = D_800756E8[temp_a1];
        if (temp_v1 != 3) {
            if ((s32)temp_v1 >= 4) {
                if (temp_v1 != 4) {
                    return 1;
                }
                D_800756E8[temp_a1] = 0;
                temp_v1_2 = (D_800722C4 * 2) + &D_800831FC;
                *temp_v1_2 += 5;
                return 0;
            }
            if ((s32)temp_v1 < 2) {
                if ((s32)temp_v1 >= 0) {
                    goto block_10;
                }
                // Duplicate return node #19. Try simplifying control flow for
                // better match
                return 1;
            }
            return 1;
        }
    block_10:
        temp_v1_3 = D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2));
        temp_a1_2 = temp_v1_3->unk4;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk5E =
            (u8)temp_v1_3->unk1;
        temp_v1_4 = *(&D_8007EB98 + D_800722C4);
        ((temp_v1_4 * 0x84) + D_8009C544)->unk60 =
            (s16)((s16)D_8009D828[temp_v1_4] / (s32)temp_a1_2);
        temp_a3 = (D_800722C4 * 2) + &D_800831FC;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk62 =
            (s16)(((s32)(D_8009C6DC + *temp_a3)->unk2 / (s32)temp_a1_2) * 0x10);
        temp_lo = (s32)(D_8009C6DC + *temp_a3)->unk3 / (s32)temp_a1_2;
        temp_a1_3 = *(&D_8007EB98 + D_800722C4);
        temp_v1_5 =
            &g_FieldModelData->modelEntries[g_FieldModelLoaderData[temp_a1_3]
                                                .modelEntryIndex];
        temp_a1_4 = temp_a1_3 * 0x84;
        temp_v0 = *((*(&D_80074F02 + temp_a1_4) * 0x10) +
                    &temp_v1_5->modelData[temp_v1_5->animationOffset]) +
                  0xFFFF;
        if (temp_v0 < temp_lo) {
            (temp_a1_4 + D_8009C544)->unk64 = temp_v0;
        } else {
            (temp_a1_4 + D_8009C544)->unk64 = (s16)temp_lo;
        }
        if (g_FieldCurrentOpcode == 0xB0) {
            D_800756E8[*(&D_8007EB98 + D_800722C4)] = 5;
            goto block_15;
        }
        D_800756E8[*(&D_8007EB98 + D_800722C4)] = 2;
        return 1;
    }
block_15:
    temp_a0 = (D_800722C4 * 2) + &D_800831FC;
    *temp_a0 += 5;
    return 0;
}
#endif

/* CANM! (change animation, extended): switch the current model's animation to
 * a new id read from the script, recompute the frame rate from the base speed
 * and the script's divisor, and clamp the last frame. The D_800756E8 state
 * drives the if-else chain. m2c seed; residual is the per-model address CSE
 * and the divide scheduling. Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncCanmEx);
#else
s32 OpcodeFuncCanmEx(void) {
    FieldModelEntry* temp_v1_4;
    s16 temp_v0;
    s32 temp_a1_4;
    s32 temp_lo;
    u16* temp_a0;
    u16* temp_a1_2;
    u8 temp_a1;
    u8 temp_a1_3;
    u8 temp_a3;
    u8 temp_v1;
    u8 temp_v1_3;
    void* temp_v1_2;

    if (D_8009D820 & 3) {
        DebugPrintOpcode("canm!", 4U);
    }
    temp_a1 = *(&D_8007EB98 + D_800722C4);
    if (temp_a1 != 0xFF) {
        temp_v1 = D_800756E8[temp_a1];
        if (temp_v1 != 3) {
            if ((s32)temp_v1 >= 4) {
                if (temp_v1 != 4) {
                    return 1;
                }
                D_800756E8[temp_a1] = 3;
                goto block_17;
            }
            if ((s32)temp_v1 < 2) {
                if ((s32)temp_v1 >= 0) {
                    goto block_10;
                }
                // Duplicate return node #20. Try simplifying control flow for
                // better match
                return 1;
            }
            return 1;
        }
    block_10:
        temp_v1_2 = D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2));
        temp_a3 = temp_v1_2->unk4;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk5E =
            (u8)temp_v1_2->unk1;
        temp_v1_3 = *(&D_8007EB98 + D_800722C4);
        ((temp_v1_3 * 0x84) + D_8009C544)->unk60 =
            (s16)((s16)D_8009D828[temp_v1_3] / (s32)temp_a3);
        temp_a1_2 = (D_800722C4 * 2) + &D_800831FC;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk62 =
            (s16)((D_8009C6DC + *temp_a1_2)->unk2 * 0x10);
        temp_lo = (s32)(D_8009C6DC + *temp_a1_2)->unk3 / (s32)temp_a3;
        temp_a1_3 = *(&D_8007EB98 + D_800722C4);
        temp_v1_4 =
            &g_FieldModelData->modelEntries[g_FieldModelLoaderData[temp_a1_3]
                                                .modelEntryIndex];
        temp_a1_4 = temp_a1_3 * 0x84;
        temp_v0 = *((*(&D_80074F02 + temp_a1_4) * 0x10) +
                    &temp_v1_4->modelData[temp_v1_4->animationOffset]) +
                  0xFFFF;
        if (temp_v0 < temp_lo) {
            (temp_a1_4 + D_8009C544)->unk64 = temp_v0;
        } else {
            (temp_a1_4 + D_8009C544)->unk64 = (s16)temp_lo;
        }
        if (g_FieldCurrentOpcode == 0xB1) {
            D_800756E8[*(&D_8007EB98 + D_800722C4)] = 6;
        block_17:
            goto block_18;
        }
        D_800756E8[*(&D_8007EB98 + D_800722C4)] = 2;
        return 1;
    }
block_18:
    temp_a0 = (D_800722C4 * 2) + &D_800831FC;
    *temp_a0 += 5;
    return 0;
}
#endif

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

/* MOVE (0x??): start a scripted move of the current entity toward a target
 * read from the script, picking the walk/run animation by distance and setting
 * the scripted-move state machine going. m2c seed using the raw D_ symbols;
 * residual is the full-expression g_FieldModels[...] address CSE. Pinned
 * pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncMove);
#else
s32 OpcodeFuncMove(void) {
    FieldModelEntry* temp_v0;
    s16 temp_a0_2;
    s32 temp_v1_2;
    s32 var_a0;
    u16* temp_a0_3;
    u8 temp_a0;
    u8 temp_a1_2;
    u8 temp_v1;
    void* temp_a1;
    void* temp_v1_3;

    if (D_8009D820 & 3) {
        DebugPrintOpcode("move", 5U);
    }
    temp_v1 = *(&D_8007EB98 + D_800722C4);
    if (temp_v1 == 0xFF) {
        var_a0 = D_800722C4 * 2;
        goto block_16;
    }
    ((temp_v1 * 0x84) + D_8009C544)->unk68 = 0;
    ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk37 = 0;
    ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk78 =
        (s32)((s32)(FieldEventReadMemoryS16(1, 2) << 0x10) >> 4);
    ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk7C =
        (s32)((s32)(FieldEventReadMemoryS16(2, 4) << 0x10) >> 4);
    temp_a1 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
    if ((D_8009C6E0->unk10 * 3) < (s32)temp_a1->unk70) {
        if (temp_a1->unk5E != 2) {
            temp_a1->unk5E = 2U;
            goto block_9;
        }
    } else if (temp_a1->unk5E != 1) {
        temp_a1->unk5E = 1U;
    block_9:
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk60 = 0x10;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk62 = 0;
        temp_a0 = *(&D_8007EB98 + D_800722C4);
        temp_v0 =
            &g_FieldModelData->modelEntries[g_FieldModelLoaderData[temp_a0]
                                                .modelEntryIndex];
        temp_v1_2 = temp_a0 * 0x84;
        (temp_v1_2 + D_8009C544)->unk64 =
            (s16)(*((*(&D_80074F02 + temp_v1_2) * 0x10) +
                    &temp_v0->modelData[temp_v0->animationOffset]) -
                  1);
    }
    D_800756E8[*(&D_8007EB98 + D_800722C4)] = 1;
    temp_v1_3 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
    temp_a1_2 = temp_v1_3->unk5D;
    if (temp_a1_2 == 1) {
        temp_a0_2 = temp_v1_3->unk6A;
        if (temp_a0_2 != temp_a1_2) {
            if (temp_a0_2 != 2) {
                goto block_17;
            }
            temp_v1_3->unk5D = 0U;
            ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk6A = 0;
            D_800756E8[*(&D_8007EB98 + D_800722C4)] = 0;
            var_a0 = D_800722C4 * 2;
        block_16:
            temp_a0_3 = var_a0 + &D_800831FC;
            *temp_a0_3 += 6;
            return 0;
        }
        return 1;
    }
block_17:
    ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk5D = 1;
    ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk6A = 0;
    return 1;
}
#endif

/* FMOVE (0xAD): move the current entity to a target while keeping its facing.
 * If a move is in flight (scriptedMoveMode 1), poll it (return 1) until
 * ActionState 2 marks it done, then clear the mode. Otherwise start the move.
 * Verified C kept as the #else; codegen pinned via MASPSX_OVERRIDE (the
 * g_FieldModels *0x84 base regalloc wall). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncFmove);
#else
s32 OpcodeFuncFmove(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fmove", 5);
    }
    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(6);
        return 0;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionArg = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveDirAdd = 0;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndX =
        (s32)FieldEventReadMemoryS16(2, 4) << 12;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].MoveEndY =
        (s32)FieldEventReadMemoryS16(3, 6) << 12;
    if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode == 1) {
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState == 1) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 2;
        } else if (
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState == 2) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode =
                0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
            PC_INC(6);
            return 0;
        }
        return 1;
    }
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    return 1;
}
#endif

/* CMOVE (0xA9): start (or continue) a scripted walk of the current entity to a
 * target point. Unlike JUMP it never blocks -- it arms the walk mode and steps
 * over its own 6 bytes every call; the field model update drives the walk
 * per-frame. The g_FieldModels *0x84 base regalloc is the wall; codegen pinned
 * via MASPSX_OVERRIDE, #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncCmove);
#else
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
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState == 1) {
            PC_INC(6);
            return 0;
        }
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState == 2) {
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
    PC_INC(6);
    return 0;
}
#endif

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
 * frames. If a jump is already in flight, poll it (return 1) until ActionState
 * 2 marks it done, then clear the move mode. Otherwise start a new jump. The
 * scalar clear-stores go through the full g_FieldModels[...] indexed expression
 * (the original rematerialises the address). Codegen pinned via
 * MASPSX_OVERRIDE; the #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncJump);
#else
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
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState == 1) {
            return 1;
        }
        if (g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState == 2) {
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode =
                SMODE_NONE;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
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
#endif

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

void OpcodeFuncPmova(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("pmova", 1);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldMoveToEntityUpdate(actorId);
}

void OpcodeFuncMova(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("mova", 1);
    }
    FieldMoveToEntityUpdate(GET_PARAM_U8(1));
}

/* Set up (or finish) a scripted move of the current entity toward a target
 * entity: the current entity's move destination becomes the target's current
 * position. Returns 1 while a move is being set up / is in progress, 0 when it
 * just finished or when either entity has no model. Every global access
 * re-materialises the g_EntityToModel / g_FieldModels bases through $at (the
 * $at remat wall); codegen pinned via MASPSX_OVERRIDE, #else is the verified
 * C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldMoveToEntityUpdate);
#else
s32 FieldMoveToEntityUpdate(s32 targetEntityId) {
    FieldEntity* cur;
    FieldEntity* target;
    FieldModelEntry* entry;
    u8 curModel;
    u8 targetModel;
    u8 animCount;

    curModel = g_EntityToModel[g_CurrentEntity];
    if (curModel == 0xFF) {
        goto advance;
    }
    targetModel = g_EntityToModel[targetEntityId & 0xFF];
    if (targetModel == 0xFF) {
        goto advance;
    }
    cur = &g_FieldModels[curModel];
    target = &g_FieldModels[targetModel];
    cur->MoveEndI = target->SolidRange;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].DirLock = 0;
    cur->MoveEndX = target->PosX;
    cur->MoveEndY = target->PosY;
    cur = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (cur->scriptedMoveMode != 1) {
        goto setmode;
    }
    if (cur->ActionState == 1) {
        if (g_FieldState->currentMovieFrame * 3 < cur->MoveSpeed) {
            if (cur->activeAnimId == 2) {
                goto done_anim;
            }
            cur->activeAnimId = 2;
        } else {
            if (cur->activeAnimId == 1) {
                goto done_anim;
            }
            cur->activeAnimId = 1;
        }
        cur->animSpeed = 0x10;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animCurrentFrame = 0;
        curModel = g_EntityToModel[g_CurrentEntity];
        entry = &g_FieldModelLoaderData[curModel];
        animCount = D_80074F02[curModel];
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].animLastFrame =
            *(u16*)(g_FieldModelData->modelEntries[entry->modelEntryIndex]
                        .animationOffset +
                    g_FieldModelData->modelEntries[entry->modelEntryIndex]
                        .modelData +
                    animCount * 0x10) -
            1;
    done_anim:
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 1;
        return 1;
    }
    if (cur->ActionState == 2) {
        cur->scriptedMoveMode = 0;
        D_800756E8[g_EntityToModel[g_CurrentEntity]] = 0;
        goto advance;
    }
    goto out;
setmode:
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].scriptedMoveMode = 1;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].ActionState = 0;
    goto out;
advance:
    g_FieldScriptPC[g_CurrentEntity] += 2;
    return 0;
out:
    return 0;
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
        actorId = D_8009AD30[partyId];
    }
    FieldEventSetDirByActorId(actorId);
}

/* Face the current entity towards another entity. Reads both models' fixed
 * point positions, computes the direction with FieldEntityDirByVec, and snaps
 * the current entity's Dir to it, cancelling any turn in progress. No-op when
 * either entity has no model. The g_FieldModels *0x84 base regalloc is the
 * wall; codegen pinned via MASPSX_OVERRIDE, #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventSetDirByActorId);
#else
void FieldEventSetDirByActorId(u8 actorId) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    u8 curModel;
    u8 targetModel;

    curModel = g_EntityToModel[g_CurrentEntity];
    if (curModel == 0xFF) {
        return;
    }
    targetModel = g_EntityToModel[actorId];
    if (targetModel == 0xFF) {
        return;
    }
    from.vx = g_FieldModels[curModel].PosX >> 12;
    from.vy = g_FieldModels[curModel].PosY >> 12;
    from.vz = g_FieldModels[curModel].PosZ >> 12;
    to.vx = g_FieldModels[targetModel].PosX >> 12;
    to.vy = g_FieldModels[targetModel].PosY >> 12;
    to.vz = g_FieldModels[targetModel].PosZ >> 12;
    g_FieldModels[curModel].Dir = FieldEntityDirByVec(&from, &to, &sqrDist);
    g_FieldModels[curModel].TurnType = 0;
    g_FieldModels[curModel].TurnStep = 0;
}
#endif

void OpcodeFuncTura(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("tura", 3);
    }
    FieldEntityTurnToEntity(GET_PARAM_U8(1));
}

void OpcodeFuncPtura(void) {
    u8 partyId;
    u8 actorId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ptura", 3);
    }
    partyId = D_8009D391[GET_PARAM_U8(1)];
    if (partyId == 0xFF) {
        actorId = 0xFF;
    } else {
        actorId = D_8009AD30[partyId];
    }
    FieldEntityTurnToEntity(actorId);
}

/* Turn the current entity to face a target entity: snapshot the current
 * direction, compute the target facing from the position delta, and set the
 * turn state machine going (choosing the short way around). m2c seed; residual
 * is the per-model address CSE and the turn-delta sign logic. Pinned pending a
 * permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEntityTurnToEntity);
#else
void FieldEntityTurnToEntity(u8 actorId) {
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp30;
    s16 temp_v0_2;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 var_a0_2;
    s16 var_v0;
    s32 temp_a1_2;
    s32 temp_a3;
    s32 temp_t0;
    s32 temp_t1;
    s32 var_a0;
    u16 temp_a1_3;
    u16 temp_a3_2;
    u16* temp_a0;
    u8 temp_v1;
    u8 temp_v1_2;
    u8 temp_v1_3;
    void* temp_a1;
    void* temp_a2;
    void* temp_v0;
    void* var_a0_3;

    temp_v1 = *(&D_8007EB98 + D_800722C4);
    if ((temp_v1 == 0xFF) || (*(&D_8007EB98 + (s16)actorId) == 0xFF)) {
        var_a0 = D_800722C4 * 2;
        goto block_5;
    }
    temp_a1 = (temp_v1 * 0x84) + D_8009C544;
    temp_v1_2 = temp_a1->unk3B;
    if (temp_v1_2 == 3) {
        temp_a1->unk3B = 0U;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk3A = 0;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk39 = 0;
        var_a0 = D_800722C4 * 2;
    block_5:
        temp_a0 = var_a0 + &D_800831FC;
        *temp_a0 += 4;
        return;
    }
    if ((temp_a1->unk3A == 0) || (temp_v1_2 != 2) ||
        (temp_a1->unk39 !=
         (D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk2)) {
        temp_v0 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
        temp_v0->unk3C = (s16)temp_v0->unk38;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk3B = 2;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk39 =
            (u8)(D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk2;
        temp_t0 =
            (s32)((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unkC >>
            0xC;
        sp10 = temp_t0;
        temp_t1 =
            (s32)((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk10 >>
            0xC;
        sp14 = temp_t1;
        sp18 =
            (s32)((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk14 >>
            0xC;
        temp_a1_2 =
            (s32)((*(&D_8007EB98 + (s16)actorId) * 0x84) + D_8009C544)->unkC >>
            0xC;
        sp20 = temp_a1_2;
        temp_a3 =
            (s32)((*(&D_8007EB98 + (s16)actorId) * 0x84) + D_8009C544)->unk10 >>
            0xC;
        sp24 = temp_a3;
        sp28 =
            (s32)((*(&D_8007EB98 + (s16)actorId) * 0x84) + D_8009C544)->unk14 >>
            0xC;
        if (temp_t0 == temp_a1_2) {
            if (temp_t1 == temp_a3) {
                sp10 = temp_t0 + 1;
            }
        }
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk3E =
            (s16)(FieldEntityDirByVec((VECTOR*)&sp10, (VECTOR*)&sp20, &sp30) &
                  0xFF);
        temp_v1_3 = (D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk3;
        switch (temp_v1_3) { // irregular
        case 2:
            temp_a2 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_a1_3 = temp_a2->unk3E;
            temp_a3_2 = temp_a2->unk3C;
            temp_v1_4 = temp_a1_3 - temp_a3_2;
            var_a0_2 = temp_v1_4;
            if (temp_v1_4 & 0x8000) {
                var_a0_2 = ~temp_v1_4 + 1;
            }
            if (var_a0_2 >= 0x81) {
                if ((s16)temp_a3_2 < (s16)temp_a1_3) {
                    temp_a2->unk3E = (u16)(temp_a1_3 - 0x100);
                } else {
                    temp_a2->unk3E = (u16)(temp_a1_3 + 0x100);
                }
            }
            break;
        case 1:
            var_a0_3 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v1_5 = var_a0_3->unk3E;
            if ((s32)var_a0_3->unk38 < temp_v1_5) {
                var_v0 = temp_v1_5 - 0x100;
            block_27:
                var_a0_3->unk3E = var_v0;
            }
            break;
        case 0:
            var_a0_3 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0_2 = var_a0_3->unk3E;
            var_v0 = temp_v0_2 + 0x100;
            if (temp_v0_2 < (s32)var_a0_3->unk38) {
                goto block_27;
            }
            break;
        }
    }
}
#endif

extern u8 D_800722C4;
extern /*?*/ s32 D_800831FC;
extern u8 D_8009D820;

/* OFSTD (0x?? offset-start): set up an offset animation for the current
 * entity's model. Reads the four target offsets from the script, stores the
 * mode byte, and either snapshots the current offsets as the start (mode!=0)
 * or copies the ends into the starts (mode==0). Residual is the
 * full-expression g_FieldModels[...] address CSE; pinned pending permuter. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncOfstd);
#else
s32 OpcodeFuncOfstd(void) {
    s8* var_a0;
    u16* temp_v1_3;
    u8 temp_v1;
    u8 temp_v1_2;
    void* temp_v0;
    void* temp_v0_2;
    void* temp_v0_3;
    void* temp_v0_4;
    void* temp_v0_5;
    void* temp_v0_6;

    if (*(&D_8007EB98 + D_800722C4) != 0xFF) {
        if (D_8009D820 & 3) {
            temp_v1 = (D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk3;
            switch (temp_v1) { // irregular
            case 0:
                var_a0 = "ofstd";
            block_11:
                DebugPrintOpcode(var_a0, 5U);
                break;
            case 1:
                var_a0 = "ofstl";
                goto block_11;
            case 2:
                var_a0 = "ofstc";
                goto block_11;
            }
        }
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk54 = 0;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk52 =
            FieldEventReadMemoryS16(4, 0xA);
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk44 =
            FieldEventReadMemoryS16(1, 4);
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk4A =
            FieldEventReadMemoryS16(2, 6);
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk50 =
            FieldEventReadMemoryS16(3, 8);
        temp_v1_2 = (D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk3;
        ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk56 = temp_v1_2;
        if (temp_v1_2 != 0) {
            temp_v0 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0->unk42 = (u16)temp_v0->unk40;
            temp_v0_2 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0_2->unk48 = (u16)temp_v0_2->unk46;
            temp_v0_3 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0_3->unk4E = (u16)temp_v0_3->unk4C;
        } else {
            temp_v0_4 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0_4->unk40 = (u16)temp_v0_4->unk44;
            temp_v0_5 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0_5->unk46 = (u16)temp_v0_5->unk4A;
            temp_v0_6 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0_6->unk4C = (u16)temp_v0_6->unk50;
        }
    }
    temp_v1_3 = (D_800722C4 * 2) + &D_800831FC;
    *temp_v1_3 += 0xC;
    return 0;
}
#endif

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
 * Instruction-for-instruction identical; what is left is where gcc cross-jumps
 * the PC_INC tail. The original merges the three paths *after* the reload of
 * g_CurrentEntity, so the model == 0xFF path reuses the copy loaded at the top
 * of the function; gcc merges two instructions earlier and reloads. Writing the
 * tail out twice stops it cross-jumping at all (9 extra instructions), and
 * inverting the test to an early return flips the branch to `bne`. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncTurnw);
#else
s32 OpcodeFuncTurnw(void) {
    FieldEntity* model;

    if (g_EntityToModel[g_CurrentEntity] != 0xFF) {
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("turnw", 0);
        }
        model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
        if (model->TurnType != 0) {
            if (model->TurnType != 3) {
                return 1;
            }
            model->TurnType = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
            g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
        }
    }
    PC_INC(1);
    return 0;
}
#endif

/* TURN (0xB5): turn the current entity to a target direction over a number of
 * steps. If the previous turn finished (TurnType 3) clear it and advance. If an
 * identical turn is already in flight, keep waiting. Otherwise (re)arm the
 * turn: TurnStart = current Dir, TurnEnd = target. Verified C kept as the
 * #else; codegen pinned via MASPSX_OVERRIDE (the g_FieldModels *0x84 base
 * regalloc wall). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncTurn);
#else
s32 OpcodeFuncTurn(void) {
    s16 dir;
    FieldEntity* model;
    u8 turnType;
    u8 turnSteps;

    if (g_EntityToModel[g_CurrentEntity] == 0xFF) {
        PC_INC(6);
        return 0;
    }
    turnSteps = GET_PARAM_U8(4);
    turnType = GET_PARAM_U8(5);
    if (g_DebugLevel & 3) {
        if (turnType == 1) {
            DebugPrintOpcode("turn", 5);
        } else if (turnType == 2) {
            DebugPrintOpcode("turnc", 5);
        }
    }
    model = &g_FieldModels[g_EntityToModel[g_CurrentEntity]];
    if (model->TurnType == 3) {
        model->TurnType = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnStep = 0;
        g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = 0;
        PC_INC(6);
        return 0;
    }
    dir = FieldEventReadMemoryS16(2, 2);
    if (model->TurnStep != 0 && (s16)dir == model->TurnEnd &&
        model->TurnType == turnType && model->TurnSteps == turnSteps) {
        return 1;
    }
    model->TurnStart = g_FieldModels[g_EntityToModel[g_CurrentEntity]].Dir;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnType = turnType;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnSteps = turnSteps;
    g_FieldModels[g_EntityToModel[g_CurrentEntity]].TurnEnd = dir;
    return 1;
}
#endif

/* TURNR (turn toward a direction, with the trnrc/trnlc turn-clockwise /
 * counter-clockwise variants): set up the current entity's turn toward a
 * target direction read from the script, choosing the short way around. The
 * turn state lives in the FieldEntity TurnStart/TurnEnd/TurnStep fields. m2c
 * seed; residual is the per-model address CSE and the turn-delta sign logic.
 * Pinned pending a permuter pass. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncTurnr);
#else
s32 OpcodeFuncTurnr(void) {
    s16 temp_v0_2;
    s16 temp_v1_3;
    s16 temp_v1_4;
    s16 var_a0_2;
    s8* var_a0;
    u16 temp_a1_2;
    u16 temp_a3;
    u16* temp_a0_6;
    u8 temp_a0;
    u8 temp_a0_2;
    u8 temp_a2;
    u8 temp_v1_2;
    void* temp_a0_3;
    void* temp_a0_4;
    void* temp_a0_5;
    void* temp_a1;
    void* temp_a2_2;
    void* temp_v0;
    void* temp_v1;

    temp_a0 = D_800722C4;
    if (*(&D_8007EB98 + temp_a0) != 0xFF) {
        if (D_8009D820 & 3) {
            temp_v1 = D_8009C6DC + *(&D_800831FC + (temp_a0 * 2));
            temp_a0_2 = temp_v1->unk5;
            switch (temp_a0_2) { // irregular
            case 1:
                var_a0 = "turnr";
                if (temp_v1->unk3 != 0) {
                    var_a0 = "turnl";
                }
            block_9:
                DebugPrintOpcode(var_a0, 5U);
                break;
            case 2:
                var_a0 = "trnrc";
                if (temp_v1->unk3 != 0) {
                    var_a0 = "trnlc";
                }
                goto block_9;
            }
        }
        temp_a0_3 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
        temp_a2 = temp_a0_3->unk3B;
        if (temp_a2 == 3) {
            temp_a0_3->unk3B = 0U;
            ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk3A = 0;
            ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk39 = 0;
            goto block_30;
        }
        if ((temp_a0_3->unk3A == 0) ||
            (temp_a1 = D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)),
             (temp_a2 != temp_a1->unk5)) ||
            (temp_a0_3->unk39 != temp_a1->unk4)) {
            temp_v0 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v0->unk3C = (s16)temp_v0->unk38;
            ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk3B =
                (u8)(D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk5;
            ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk39 =
                (u8)(D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk4;
            ((*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544)->unk3E =
                (s16)(FieldEventReadMemoryU8(2, 2) & 0xFF);
            temp_v1_2 = (D_8009C6DC + *(&D_800831FC + (D_800722C4 * 2)))->unk3;
            if (temp_v1_2 != 1) {
                if ((s32)temp_v1_2 < 2) {
                    if (temp_v1_2 != 0) {
                        return 1;
                    }
                    temp_a0_4 =
                        (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
                    temp_v0_2 = temp_a0_4->unk3E;
                    if (temp_v0_2 < (s32)temp_a0_4->unk38) {
                        temp_a0_4->unk3E = (s16)(temp_v0_2 + 0x100);
                    }
                    goto block_31;
                }
                if (temp_v1_2 == 2) {
                    temp_a2_2 =
                        (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
                    temp_a1_2 = temp_a2_2->unk3E;
                    temp_a3 = temp_a2_2->unk3C;
                    temp_v1_3 = temp_a1_2 - temp_a3;
                    var_a0_2 = temp_v1_3;
                    if (temp_v1_3 & 0x8000) {
                        var_a0_2 = ~temp_v1_3 + 1;
                    }
                    if (var_a0_2 >= 0x81) {
                        if ((s16)temp_a3 < (s16)temp_a1_2) {
                            temp_a2_2->unk3E = (u16)(temp_a1_2 - 0x100);
                        } else {
                            temp_a2_2->unk3E = (u16)(temp_a1_2 + 0x100);
                        }
                    }
                }
                // Duplicate return node #32. Try simplifying control flow for
                // better match
                return 1;
            }
            temp_a0_5 = (*(&D_8007EB98 + D_800722C4) * 0x84) + D_8009C544;
            temp_v1_4 = temp_a0_5->unk3E;
            if ((s32)temp_a0_5->unk38 < temp_v1_4) {
                temp_a0_5->unk3E = (s16)(temp_v1_4 - 0x100);
            }
        block_31:
            // Duplicate return node #32. Try simplifying control flow for
            // better match
            return 1;
        }
        return 1;
    }
block_30:
    temp_a0_6 = (temp_a0 * 2) + &D_800831FC;
    *temp_a0_6 += 6;
    return 0;
}
#endif

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
            actorId = D_8009AD30[partyId];
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
            actorId = D_8009AD30[partyId];
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
    if (D_8008326C[window] == 0xFF) {
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
    if (D_8008326C[window] == 0xFF) {
        return OpcodeFuncWsize();
    }
    if (D_8008326C[window] == g_CurrentEntity) {
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
    if (D_800716CC != 0) {
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
    if (D_800716CC != 0) {
        D_801144D4 = 0;
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
    if (D_800716CC != 0) {
        FieldEventWriteMemoryS16(2, 2, D_801144D4);
        D_801144D4++;
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
        actorId = D_8009AD30[partyId];
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
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncStpls);
#else
s32 OpcodeFuncStpls(void) {
    RECT rect;
    s16 x;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("stpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(1) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    StoreImage(&rect, (u_long*)&D_80095DE0[GET_PARAM_U8(2) * 32 + x * 2]);
    PC_INC(5);
    return 0;
}
#endif

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
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncLdpls);
#else
s32 OpcodeFuncLdpls(void) {
    RECT rect;
    s16 x;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("ldpls", 4);
    }
    FieldEventRectClear((s16*)&rect);
    rect.y = GET_PARAM_U8(2) + 0x1E0;
    x = GET_PARAM_U8(3);
    rect.x = x;
    rect.w = GET_PARAM_U8(4) + 1;
    rect.h = 1;
    LoadImage(&rect, (u_long*)&D_80095DE0[GET_PARAM_U8(1) * 32 + x * 2]);
    PC_INC(5);
    return 0;
}
#endif

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
 * Three rows, no extra instructions -- all three are the same choice. The
 * target materialises &D_80095DE0 between the `andi` that widens the palette
 * id and the `sll` that scales it; gcc emits the `sll` first. That is the
 * residue OpcodeFuncCppal and OpcodeFuncStpls above are parked on, and it
 * costs one row per loop preheader here, so two. Naming the base in a local
 * (`u8* pal = D_80095DE0;` inside the loop) moves the pair to *before* the
 * `andi` rather than between -- one row off in the other direction, measured
 * here at 27 rows against 13 for the plain form, though it is what the two
 * ADPALs below need. The third row is the loop body swapping $a0 and $v1
 * between the two scaled indices, which is downstream of the same choice. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncRtpal);
#else
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
        u16* to = (u16*)(D_80095DE0 + dst * 32);
        u16* from = (u16*)(D_80095DE0 + src * 32);

        to[j] = from[i];
        i++;
    }
    j = 0;
    for (i = count - start; i <= count; i++) {
        u16* to = (u16*)(D_80095DE0 + dst * 32);
        u16* from = (u16*)(D_80095DE0 + src * 32);

        to[j] = from[i];
        j++;
    }
    PC_INC(7);
    return 0;
}
#endif

/* As RTPAL, but source and destination each get their own start entry, so the
 * rotation can move a run between two palettes as well as within one.
 *
 * One row, the same &D_80095DE0 placement as OpcodeFuncRtpal above. Here the
 * named-base spelling is the better of the two (11 rows against 23) and is
 * kept, so the pair lands one slot early rather than one slot late. Both
 * palettes come from the script here rather than from event memory, so there
 * is no `andi` to straddle in the second preheader and only one row is left. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncRtpal2);
#else
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
        u8* pal = D_80095DE0;
        u16* to = (u16*)(pal + dst * 32);
        u16* from = (u16*)(pal + src * 32);

        to[j] = from[i];
        i++;
    }
    j = srcStart;
    for (i = end - dstStart; i <= end; i++) {
        u8* pal = D_80095DE0;
        u16* to = (u16*)(pal + dst * 32);
        u16* from = (u16*)(pal + src * 32);

        to[j] = from[i];
        j++;
    }
    PC_INC(8);
    return 0;
}
#endif

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
 * Three rows left. Two are &D_80095DE0's placement, as in the RTPALs above.
 * The third is not source-addressable: the target converts mulR and mulG to
 * u16 in the preheader (`andi t3,a0,0xffff`) but uses mulB straight out of
 * $s4, and the three are read identically from the same call. gcc is being
 * inconsistent with itself there, so no single declared type reproduces it --
 * u16 for all three gets the first two right and the third wrong. Related:
 * the target extracts red from the *untruncated* $a3 while taking the other
 * two channels from the truncated copy, which is the same inconsistency seen
 * from the other end. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncMppal2);
#else
s32 OpcodeFuncMppal2(void) {
    u16 count;
    u8 srcPal;
    u8 dstPal;
    u16 mulB;
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
        u16* from = (u16*)(D_80095DE0 + srcPal * 32);
        u16* to = (u16*)(D_80095DE0 + dstPal * 32);
        u16 color = from[i];

        if (color != 0) {
            s32 r = (mulR * ((color << 1) & 0x3E)) >> 7;
            s32 g = (mulG * ((color >> 4) & 0x3F)) >> 7;
            s32 b = (mulB * ((color >> 9) & 0x3F)) >> 7;

            if (b >= 0x20) {
                b = 0x1F;
            }
            if (g >= 0x20) {
                g = 0x1F;
            }
            if (r >= 0x20) {
                r = 0x1F;
            }
            to[i] = (b << 10) | (g << 5) | r | (color & 0x8000);
            if (to[i] == 0) {
                to[i] = 0x8000;
            }
        }
    }
    PC_INC(0xA);
    return 0;
}
#endif

/* MPPAL over a sub-range; the two palettes come from the script.
 *
 * Note the pair is named the wrong way round against the ADPAL and RTPAL
 * pairs: OpcodeFuncMppal2 above is the plain form and comes first in the
 * overlay, and this one -- the sub-range form -- is second. The addresses say
 * so, and so does the fact that MPPAL2 owns the "mppal" literal this prints.
 *
 * Two rows, both &D_80095DE0's placement. Same channel extraction as
 * OpcodeFuncMppal2 above; read that note before touching this one. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncMppal);
#else
s32 OpcodeFuncMppal(void) {
    s16 count;
    u8 srcPal;
    u8 dstPal;
    s16 start;
    u16 mulB;
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
        u16* from = (u16*)(D_80095DE0 + srcPal * 32);
        u16* to = (u16*)(D_80095DE0 + dstPal * 32);
        u16 color = from[i];

        if (color != 0) {
            s32 r = (mulR * ((color << 1) & 0x3E)) >> 7;
            s32 g = (mulG * ((color >> 4) & 0x3F)) >> 7;
            s32 b = (mulB * ((color >> 9) & 0x3F)) >> 7;

            if (b >= 0x20) {
                b = 0x1F;
            }
            if (g >= 0x20) {
                g = 0x1F;
            }
            if (r >= 0x20) {
                r = 0x1F;
            }
            to[i] = (b << 10) | (g << 5) | r | (color & 0x8000);
            if (to[i] == 0) {
                to[i] = 0x8000;
            }
        }
    }
    PC_INC(0xB);
    return 0;
}
#endif

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
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncVwoft);
#else
s32 OpcodeFuncVwoft(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("vwoft", 6);
    }
    if (GET_PARAM_U8(6)) {
        g_FieldState->viewOffsetStart = g_FieldState->viewOffset;
        g_FieldState->viewOffsetTarget = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetNumSteps = FieldEventReadMemoryS16(2, 4);
        g_FieldState->viewOffsetMode = GET_PARAM_U8(6);
        g_FieldState->viewOffsetCurrentStep = 0;
    } else {
        g_FieldState->viewOffsetNumSteps = 0;
        g_FieldState->viewOffset = FieldEventReadMemoryS16(1, 2);
        g_FieldState->viewOffsetCurrentStep = 0;
        g_FieldState->viewOffsetMode = 0;
        g_FieldState->viewOffsetStart = 0;
        g_FieldState->viewOffsetTarget = 0;
    }
    PC_INC(7);
    return 0;
}
#endif

/////////////////////////////////////////////////
// Begin of field_opcode_party_manage.c
/////////////////////////////////////////////////

s32 FieldEventJoinSet(u8, u8); // extern

#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncJoin);
#else
/* 25 rows: gcc hoists the 0xFF constant into a saved reg ($s1) for the two
 * memory_bank_2[10]/[11] compares and reuses it across both FieldEventJoinSet
 * calls; target reloads `li v0,0xff` per compare and keeps the stack frame at
 * -0x18 (no $s1 save). GET_PARAM_U8(1) shared by both calls is the hoist
 * trigger. g_FieldModels idiom (not g_FieldEntity) was the key fix that cut
 * 66->25 rows. Polarity flip and block-scope arg temp both plateau at 25. */
s32 OpcodeFuncJoin(void) {
    s32 joinOk;
    s32 splitOk;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("join", 1);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    joinOk = 1;
    if (Savemap.memory_bank_2[10] != 0xFF) {
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
#endif

s32 FieldEventSplitSet(u8, s16, s16, s32, s32); // extern
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncSplit);
#else
/* 25 rows: same $s1 0xFF-constant hoist as OpcodeFuncJoin (twin function).
 * if==0xFF polarity matches target; the != form regressed to 39. g_FieldModels
 * idiom applied. Solve Join and the recipe transfers here. */
s32 OpcodeFuncSplit(void) {
    s32 splitOkA;
    s32 splitOkB;
    s16 i;
    u8 modelId;
    u8 charId;

    if (g_DebugLevel & 3) {
        DebugPrintOpcode("split", 8);
    }
    g_EntityForSplitJoin = g_CurrentEntity;
    if (Savemap.memory_bank_2[10] == 0xFF) {
        splitOkA = 1;
    } else {
        splitOkA = FieldEventSplitSet(
            g_CharIdToEntity[Savemap.memory_bank_2[10]],
            FieldEventReadMemoryS16(1, 4), FieldEventReadMemoryS16(2, 6),
            FieldEventReadMemoryU8(3, 8) & 0xFF, GET_PARAM_U8(14));
    }
    if (Savemap.memory_bank_2[11] == 0xFF) {
        splitOkB = 1;
    } else {
        splitOkB = FieldEventSplitSet(
            g_CharIdToEntity[Savemap.memory_bank_2[11]],
            FieldEventReadMemoryS16(4, 9), FieldEventReadMemoryS16(5, 11),
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
#endif

extern /*?*/ s32 D_80081D90;

/* Drive one party member through a JOIN: state 0 starts the turn toward the
 * leader, state 2 waits for the turn then starts the move, state 1 waits for
 * the move then marks done, state 3 is done. Returns 1 while a step is in
 * progress. Twin of FieldEventSplitSet. m2c seed; residual is the g_FieldModels
 * *0x84 base regalloc and the s16 arg-widening. Pinned pending a permuter pass.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventJoinSet);
#else
s32 FieldEventJoinSet(u8 arg0, u8 arg1) {
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp38;
    s32 var_v0;
    u8 temp_s0;
    u8 temp_v1;
    void* temp_v0;

    if (*Savemap.memory_bank_2[9] != 0xFF) {
        temp_s0 = D_8009AD30[*Savemap.memory_bank_2[9]];
        if (D_8009D820 & 3) {
            FieldDebugAddParseValueToPage2("join p0=", (s32)temp_s0, 2);
            if (D_8009D820 & 3) {
                FieldDebugAddParseValueToPage2("join p1=", (s32)(s16)arg0, 2);
            }
        }
        if ((temp_s0 != 0xFF) && ((s16)arg0 != 0xFF)) {
            temp_v1 = *(&D_80081D90 + (s16)arg0);
            if (temp_v1 != 1) {
                if ((s32)temp_v1 < 2) {
                    if (temp_v1 != 0) {
                        return 0;
                    }
                    sp18 =
                        (s32)((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)
                            ->unkC >>
                        0xC;
                    sp1C =
                        (s32)((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)
                            ->unk10 >>
                        0xC;
                    sp20 =
                        (s32)((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)
                            ->unk14 >>
                        0xC;
                    sp28 = (s32)((*(&D_8007EB98 + temp_s0) * 0x84) + D_8009C544)
                               ->unkC >>
                           0xC;
                    sp2C = (s32)((*(&D_8007EB98 + temp_s0) * 0x84) + D_8009C544)
                               ->unk10 >>
                           0xC;
                    sp30 = (s32)((*(&D_8007EB98 + temp_s0) * 0x84) + D_8009C544)
                               ->unk14 >>
                           0xC;
                    FieldEventSplitJoinSetTurn(
                        (s16)arg0,
                        ((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)
                            ->unk38,
                        FieldEntityDirByVec(
                            (VECTOR*)&sp18, (VECTOR*)&sp28, &sp38) &
                            0xFF);
                    *(&D_80081D90 + (s16)arg0) = 2;
                    return 0;
                }
                if (temp_v1 != 2) {
                    var_v0 = 1;
                    if (temp_v1 != 3) {
                        return 0;
                    }
                    // Duplicate return node #21. Try simplifying control flow
                    // for better match
                    return var_v0;
                }
                if (FieldEventSplitJoinEndTurn((s16)arg0) != 0) {
                    temp_v0 = (*(&D_8007EB98 + temp_s0) * 0x84) + D_8009C544;
                    FieldEventSplitJoinSetMove(
                        (s16)arg0, (s32)(temp_v0->unkC * 0x10) >> 0x10,
                        (s32)(temp_v0->unk10 * 0x10) >> 0x10, (s16)arg1, 0);
                    *(&D_80081D90 + (s16)arg0) = 1;
                    if (D_8009D820 & 3) {
                        FieldDebugAddParseValueToPage2("end setmove", 0, 0);
                        return 0;
                    }
                }
                goto block_20;
            }
            if (FieldEventSplitJoinEndMove((s16)arg0) != 0) {
                ((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)->unk59 = 1;
                ((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)->unk5B = 1;
                ((*(&D_8007EB98 + (s16)arg0) * 0x84) + D_8009C544)->unk5C = 0;
                *(&D_80081D90 + (s16)arg0) = 3;
                return 1;
            }
        block_20:
            var_v0 = 0;
            return var_v0;
        }
        goto block_19;
    }
block_19:
    return 1;
}
#endif

/* Drive one party member through a SPLIT: state 0 starts the move, state 1
 * waits for the move then starts the turn, state 2 waits for the turn, state 3
 * is done. Returns 1 while a step is still in progress. The g_FieldModels
 * *0x84 base regalloc and the s16 arg-widening (<<0x10/>>0x10) are the wall;
 * codegen pinned via MASPSX_OVERRIDE, #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventSplitSet);
#else
s32 FieldEventSplitSet(u8 entityId, s16 x, s16 y, s32 turnDir, s32 a4) {
    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("split p1=", entityId, 2);
    }
    if (entityId == 0xFF) {
        return 1;
    }
    switch (g_EntitySplitJoinState[entityId]) {
    case 0:
        FieldEventSplitJoinSetMove(entityId, x, y, turnDir, a4);
        g_EntitySplitJoinState[entityId] = 1;
        return 0;
    case 1:
        if (FieldEventSplitJoinEndMove(entityId) == 0) {
            return 0;
        }
        g_FieldModels[g_EntityToModel[entityId]].SolidOff = 0;
        g_FieldModels[g_EntityToModel[entityId]].TalkOff = 0;
        FieldEventSplitJoinSetTurn(
            entityId, g_FieldModels[g_EntityToModel[entityId]].Dir, a4 & 0xFF);
        g_EntitySplitJoinState[entityId] = 2;
        return 0;
    case 2:
        if (FieldEventSplitJoinEndTurn(entityId) == 0) {
            return 0;
        }
        g_EntitySplitJoinState[entityId] = 3;
        return 1;
    case 3:
        return 1;
    }
    return 0;
}
#endif

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
 * to the same thing and neither changes a single instruction. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventSplitJoinSetMove);
#else
void FieldEventSplitJoinSetMove(
    s16 entityId, s16 x, s16 y, s16 steps, u16 snapToLeader) {
    VECTOR from;
    VECTOR to;
    s32 sqrDist;
    s16 leaderId;
    u8 modelIdx;
    FieldModelEntry* entry;
    u8* anims;

    if (D_8009D391[0] != 0xFF) {
        leaderId = D_8009AD30[D_8009D391[0]];
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
            D_800E42A8[modelIdx] = g_FieldModels[modelIdx].MoveSpeed;
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
                    entry =
                        &g_FieldModelData->modelEntries
                             [g_FieldModelLoaderData[modelIdx].modelEntryIndex];
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
                    entry =
                        &g_FieldModelData->modelEntries
                             [g_FieldModelLoaderData[modelIdx].modelEntryIndex];
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
#endif

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
        D_800E42A8[g_EntityToModel[entityId]];
    return 1;
}

/* Begin a party member's turn to a facing during a SPLIT or JOIN. Sets the
 * turn target and step budget, then if the raw delta would exceed half a
 * circle wraps the target the short way round. Codegen pinned via
 * MASPSX_OVERRIDE: the #else body is the verified-correct C; its bytes come
 * from the reference .s (the g_FieldModels *0x84 base register allocation). */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", FieldEventSplitJoinSetTurn);
#else
void FieldEventSplitJoinSetTurn(s16 entityId, s16 startDir, s16 endDir) {
    FieldEntity* model;
    s16 delta;

    if (g_DebugLevel & 3) {
        FieldDebugAddParseValueToPage2("set turn=", endDir & 0xFF, 2);
    }
    if (g_EntityToModel[entityId] != 0xFF) {
        model = &g_FieldModels[g_EntityToModel[entityId]];
        model->TurnStart = startDir & 0xFF;
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
            if ((s16)model->TurnStart < (s16)model->TurnEnd) {
                model->TurnEnd -= 0x100;
            } else {
                model->TurnEnd += 0x100;
            }
        }
    }
}
#endif

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
 * fade family (subtractive fades start at the speed, additive at 0). The
 * .rodata phase wall (jump table). Verified C kept as the #else; codegen pinned
 * via MASPSX_OVERRIDE. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncFade);
#else
s32 OpcodeFuncFade(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fade", 8);
    }
    g_FieldState->fadeType = GET_PARAM_U8(7);
    switch (g_FieldState->fadeType) {
    case FFT_INV4_TO_FIELD_SUB:
    case FFT_FIELD_TO_INV4_SUB:
    case FFT_STANDARD_TO_FIELD_ADD:
    case FFT_FIELD_TO_STANDARD_ADD:
        g_FieldState->fadeAdjust = GET_PARAM_U8(8) + 1;
        break;
    default:
        g_FieldState->fadeAdjust = GET_PARAM_U8(8);
        break;
    }
    g_FieldState->fadeSpeed = FieldEventReadMemoryS16(1, 1);
    g_FieldState->fadeRed = FieldEventReadMemoryU8(2, 3);
    g_FieldState->fadeGreen = FieldEventReadMemoryU8(3, 4);
    g_FieldState->fadeBlue = FieldEventReadMemoryU8(4, 5);
    PC_INC(9);
    return 0;
}
#endif

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

/* PARKED: 9 rows, all in one switch arm. The jump table and all four loads
 * match; what is left is cross-jumping. The original keeps the case 1/5/7/9
 * arm as its own block (lhu / nop / beqz / li 1 / j); gcc merges it into the
 * case 2/6/8/10 arm because both end in the same `return 1'. Not an alignment
 * problem -- this function's table is correctly placed since the split.
 *
 * Rejected, all 22 rows: switching on a plain (s16) cast of the member, on an
 * s32 temp, and on an s16 temp. All three let gcc fold the lhu + sll + sra
 * into a single lh. Only the volatile u16 read below reproduces the load form,
 * and it is what took the diff from 22 rows to 9.
 *
 * Next step is the permuter, not another hand-shaped attempt. */
/* FADEW: block the script until the fade started by FADE/NFADE has finished.
 * What counts as finished depends on the fade's direction, so the switch is on
 * fadeType and the eleven arms collapse to three tests: a fade to black is done
 * when fadeAdjust has run down to 0, a fade from black when it has run up to
 * 0xFF, and the NFADE forms when it has reached fadeSpeed. Types 0 and 4 are
 * not fades and fall straight through.
 *
 * Every read of the three fields goes through a volatile u16. The original
 * loads each one zero-extended and then sign-extends it in registers
 * (lhu / sll / sra); reading the s16 members directly lets gcc fold the two
 * into a single lh, which is a byte shorter everywhere it appears. volatile is
 * the one thing that keeps the load in the form the member's own type implies.
 * The `!= 0` arm takes no cast because the original does not sign-extend
 * there -- a zero test does not need it. */
#ifndef NON_MATCHINGS
/* FADEW (0x6C): block the script until the active screen fade completes. The
 * wait test depends on the fade type: subtractive fades complete when
 * fadeAdjust reaches 0, the hold-colour fades when fadeAdjust reaches
 * fadeSpeed, and the rest when fadeAdjust hits 0. Returns 1 while waiting, 0
 * (advancing the PC) once done. Jump-table fadeType dispatch; the .rodata phase
 * wall. Codegen pinned via MASPSX_OVERRIDE, #else is the verified C. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncFadew);
#else
s32 OpcodeFuncFadew(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fadew", 0);
    }
    switch (g_FieldState->fadeType) {
    case FFT_INV4_TO_FIELD_SUB:
    case FFT_FIELD_TO_INV4_SUB:
    case FFT_STANDARD_TO_FIELD_ADD:
    case FFT_FIELD_TO_STANDARD_ADD:
        if (g_FieldState->fadeAdjust == 0) {
            PC_INC(1);
            return 0;
        }
        return 1;
    case FFT_INSTANT:
    case FFT_INSTANT_BLACK:
    case FFT_INSTANT_INV1_SUB_HOLD_FIELD:
    case FFT_INSTANT_INV1_SUB_HOLD_COLOR:
    case FFT_INSTANT_STANDARD_ADD_HOLD_FIELD:
    case FFT_INSTANT_STANDARD_ADD_HOLD_COLOR:
        if (g_FieldState->fadeAdjust == 0) {
            PC_INC(1);
            return 0;
        }
        return 1;
    case FFT_SYS_FADE_TO_BLACK_FIELD_CHANGE:
    case FFT_FIELD_TO_STANDARD_ADD_HOLD_COLOR:
    default:
        if (g_FieldState->fadeAdjust == g_FieldState->fadeSpeed) {
            PC_INC(1);
            return 0;
        }
        return 1;
    }
}
#endif
#else
s32 OpcodeFuncFadew(void) {
    if (g_DebugLevel & 3) {
        DebugPrintOpcode("fadew", 0);
    }
    switch ((s16) * (volatile u16*)&g_FieldState->fadeType) {
    case 1:
    case 5:
    case 7:
    case 9:
        if (*(volatile u16*)&g_FieldState->fadeAdjust != 0) {
            return 1;
        }
        break;
    case 2:
    case 6:
    case 8:
    case 10:
        if ((s16) * (volatile u16*)&g_FieldState->fadeAdjust < 0xFF) {
            return 1;
        }
        break;
    case 0:
    case 4:
        break;
    default:
        if ((s16) * (volatile u16*)&g_FieldState->fadeAdjust !=
            (s16) * (volatile u16*)&g_FieldState->fadeSpeed) {
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
    FieldEventWriteMemoryU8(2, 4, D_80049208[corner]);
    FieldEventWriteMemoryU8(3, 5, D_80049208[corner + 1]);
    FieldEventWriteMemoryU8(4, 6, D_80049208[corner + 2]);
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
    D_80049208[corner] = FieldEventReadMemoryU8(2, 4);
    D_80049208[corner + 1] = FieldEventReadMemoryU8(3, 5);
    D_80049208[corner + 2] = FieldEventReadMemoryU8(4, 6);
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

extern u8 D_8009D7D0;
void func_80033A90(void);
void SystemMessageSetCharName(u8 charId, u8 nameId);

/* The "special" opcode: one byte of sub-opcode selects among eleven unrelated
 * jobs, from clearing the item/materia inventories to writing a character's
 * name into the message name buffer. Sub-opcodes run 0xF5..0xFF, which is what
 * the jump table's `(u32)(sub - 0xF5) < 0xB` guard checks. */
/* One instruction out, and it is a scheduling choice, not a semantic one: for
 * the smspd sub-opcode the target computes `nor v0,zero,v0` into the load-delay
 * slot of the `lbu` that PC_INC needs for g_CurrentEntity, and stores to
 * D_8009D7D0 before materialising &g_FieldScriptPC. This build emits the `nor`
 * into a1 immediately after the call and sinks the store past the PC address.
 * Everything else -- all eleven sub-opcodes, the 13 .rodata literals and the
 * jump table at .rodata+0xde0 -- is byte-exact.
 *
 * Three findings got it this far, all of them costly to re-derive:
 *   - `itemId` must be an s32 local. Passing `i | 0xC600` straight to
 *     func_80025288(u16) lets combine narrow the ior to HImode, where the
 *     constant becomes -0x3a00 and can no longer be an `ori` immediate; gcc
 *     then hoists it into a callee-saved register and the frame grows to
 *     -0x38. A u16 local does not help -- it has to be s32.
 *   - the name copy walks (`*name++`), it does not index (`name[i]`). Indexing
 *     makes gcc build a separate giv and copy the base into it.
 *   - `len` is read before the switch. Reading it after, or folding it and the
 *     switch index into one variable, both cost ~13 rows.
 * Measured and rejected: a u8 temp for the FieldEventReadMemoryU8 result
 * (no change), a u16 itemId (no change). Permuter scratch imported at base
 * score 475. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", OpcodeFuncSpcal);
#else
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
        D_800716CC = GET_PARAM_U8(2);
        PC_INC(3);
        return 0;
    case 0xFB:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("btlck", 2);
        }
        D_80071E30 = GET_PARAM_U8(2);
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
        D_8009D7D0 = ~FieldEventReadMemoryU8(4, 3);
        PC_INC(4);
        return 0;
    case 0xF7:
        if (g_DebugLevel & 3) {
            DebugPrintOpcode("gmspd", 3);
        }
        FieldEventWriteMemoryU8(4, 3, ~D_8009D7D0);
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
#endif
