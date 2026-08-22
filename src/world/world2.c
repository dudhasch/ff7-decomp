#include "world.h"

/* One VRAM upload in the group func_800C0808 walks: a rect plus the byte
 * offset of the pixel data from the start of the group. */
typedef struct {
    /* 0x0 */ u16 w;
    /* 0x2 */ u16 h;
    /* 0x4 */ u16 x;
    /* 0x6 */ u16 y;
    /* 0x8 */ u32 offset;
} WorldTim; /* size: 0xC */

typedef struct {
    /* 0x0 */ u32 unk0;
    /* 0x4 */ u8 count;
    /* 0x5 */ u8 unk5;
    /* 0x6 */ u16 unk6;
    /* 0x8 */ WorldTim entries[1];
} WorldTimGroup;

u8* func_800BFCAC(WorldModelPart* part, u8* buf, s32 arg2, s32 arg3);
void func_800C1D58(WorldModel* model, s16 arg1, s32 arg2);

u8* func_800BFBF0(WorldModel* model, u8* buf, s32 arg2) {
    u32 i;
    WorldModelPart* part;

    model->unk20 = buf;
    buf += model->unk2 * 32;
    part = (WorldModelPart*)(model->unk18 + (s32)model->unk1C);
    for (i = 0; i < model->unk3; i++) {
        buf = func_800BFCAC(&part[i], buf, 0, arg2);
    }
    func_800C1D58(model, model->unk16, 0);
    return buf;
}

/* 727 instructions, and it used to be two: spimdisasm split it in the middle
 * of an expression at 0x800C02F4, which had no prologue, carried this
 * function's epilogue, and branched backwards into it.  `worklist.py` counted
 * both halves as work.  `config/symbols.world.txt` now pins this function's
 * size at 0xB5C so splat emits one `.s`; see CLAUDE.md, "A `.s` with no
 * prologue is not a function". */
INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800BFCAC);

void func_800C0808(WorldTimGroup* group) {
    RECT rect;
    u32 i;
    s32 count;
    WorldTim* tim;

    count = group->count;
    tim = group->entries;
    for (i = 0; i < count; i++) {
        rect.x = tim[i].x;
        rect.y = tim[i].y;
        rect.w = tim[i].w;
        rect.h = tim[i].h;
        LoadImage(&rect, (u_long*)((u8*)group + tim[i].offset));
    }
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C08A8);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C0B48);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C1490);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C1D58);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C1FD8);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C2130);

void func_800C2524(WorldModelPart* part, s16 arg1, s16 arg2, s16 arg3);

s32 func_800C2450(WorldModel* model, u8* p) {
    u32 i;
    s32 count;
    WorldModelPart* part;
    s16 c0;
    s16 c1;
    s16 c2;
    /* The target's frame is 8 bytes larger than this body needs, with the
     * extra between the outgoing-argument area and the register saves.  The
     * identity of the local is not recoverable. */
    u8 unusedLocals[8];

    count = model->unk3;
    part = (WorldModelPart*)(model->unk18 + (s32)model->unk1C);
    c0 = (p[1] << 8) | p[0];
    c1 = (p[3] << 8) | p[2];
    c2 = (p[5] << 8) | p[4];
    *(u32*)0x1F800200 = p[6];
    for (i = 0; i < count; i++) {
        func_800C2524(&part[i], c0, c1, c2);
    }
    return 1;
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C2524);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C31F0);

extern SVECTOR D_800C7938[];

/* PARKED: 68 changed / 0 inserted at the **exact** length (282 against 282).
 * Every remaining row is a register name, and they are one 3-cycle plus one
 * swap, repeated once per list:
 *   target  a1=k a2=i a3=off   t2=count t3=base
 *   ours    a1=off a2=k a3=i   t2=base  t3=count
 * so the fix has to change these quantities' `QTY_CMP_PRI` ranking, and
 * nothing spelled in C does.  Measured and exactly inert at 68:
 *   - `tools/width_sweep.py`, 25 variants over 5 scalar locals: every 32-bit
 *     alternative is inert, every narrow one costs 15 to 119 rows and the
 *     length
 *   - declaration order (`off`/`k`/`i` permuted)
 *   - `base = light` moved into the `for` init list
 *   - `gte_ldrgb(&base[off])` for `gte_ldrgb(base + off)`
 *   - `off += 4` moved from the body into the `for` increment list
 *   - `base = light + 0`
 * and worse: dropping `base` and indexing `light` directly (92 rows, -4),
 * `gte_ldrgb(src + 4)` for the ldrgb address (120 rows, -8).
 *
 * The lever that got it from 88 to 68 is worth keeping: the four
 * multi-vertex lists index off `base` with `k * 4 + N`, they do **not** walk
 * a `src` pointer.  With a walking `src += 4`, `combine_givs` bases the
 * loop's addresses on the last offset referenced -- `src[6]`, the last store
 * -- and since `src` is dead after the loop gcc rewrites the biv itself, so
 * the preheader reads `addiu v1,t0,6` and every displacement in the body is
 * 6 low.  The target has `move v1,t0`, i.e. base 0, which no store order
 * reaches (4/5/6 is 88 rows, 6/5/4 is 96, 5/6/4 is 100, 4/6/5 is 96).
 * Indexing off the counter leaves the addresses as `base + const` and no giv
 * forms at all.  The four *flat* lists keep the walking pointer and do base
 * at +6 in the target, because `light` is live into the next list and the
 * biv cannot be rewritten -- that asymmetry is what the target states.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/world/nonmatchings/world2", func_800C3948);
#else
void func_800C3948(WorldModelPart* part, s32 arg1) {
    u32 i;
    u32 k;
    s32 count;
    s32 off;
    u32 w;
    u8* light;
    u8* base;
    u8* src;
    u8* normals;
    u8* scratch;

    normals = (u8*)D_800C7938;
    scratch = (u8*)0x1F800000;
    if ((*(u32*)part->unk18 & 2) && arg1 == 0) {
        return;
    }
    light = (u8*)(part->unkE + (s32)part->unk18);

    w = *(u32*)&part->numPrims[0];

    count = w & 0xFF;
    for (i = 0; i < count; i++, light += 0x18) {
        base = light;
        for (k = 0, off = 4; k < 4; k++) {
            gte_ldv0(normals + base[k * 4 + 7] * 8);
            gte_ldrgb(base + off);
            gte_nccs();
            gte_strgb(scratch);
            base[k * 4 + 4] = scratch[0];
            base[k * 4 + 5] = scratch[1];
            base[k * 4 + 6] = scratch[2];
            off += 4;
        }
    }

    count = (w & 0xFF00) >> 8;
    for (i = 0; i < count; i++, light += 0x14) {
        base = light;
        for (k = 0, off = 4; k < 3; k++) {
            gte_ldv0(normals + base[k * 4 + 7] * 8);
            gte_ldrgb(base + off);
            gte_nccs();
            gte_strgb(scratch);
            base[k * 4 + 4] = scratch[0];
            base[k * 4 + 5] = scratch[1];
            base[k * 4 + 6] = scratch[2];
            off += 4;
        }
    }

    count = (w >> 16) & 0xFF;
    for (i = 0; i < count; i++, light += 0xC) {
        gte_ldv0(normals + light[7] * 8);
        gte_ldrgb(light + 4);
        gte_nccs();
        gte_strgb(scratch);
        light[4] = scratch[0];
        light[5] = scratch[1];
        light[6] = scratch[2];
    }

    count = w >> 24;
    for (i = 0; i < count; i++, light += 0xC) {
        gte_ldv0(normals + light[7] * 8);
        gte_ldrgb(light + 4);
        gte_nccs();
        gte_strgb(scratch);
        light[4] = scratch[0];
        light[5] = scratch[1];
        light[6] = scratch[2];
    }

    w = *(u32*)&part->numPrims[4];

    count = w & 0xFF;
    for (i = 0; i < count; i++, light += 8) {
        gte_ldv0(normals + light[7] * 8);
        gte_ldrgb(light + 4);
        gte_nccs();
        gte_strgb(scratch);
        light[4] = scratch[0];
        light[5] = scratch[1];
        light[6] = scratch[2];
    }

    count = (w & 0xFF00) >> 8;
    for (i = 0; i < count; i++, light += 8) {
        gte_ldv0(normals + light[7] * 8);
        gte_ldrgb(light + 4);
        gte_nccs();
        gte_strgb(scratch);
        light[4] = scratch[0];
        light[5] = scratch[1];
        light[6] = scratch[2];
    }

    count = (w >> 16) & 0xFF;
    for (i = 0; i < count; i++, light += 0x10) {
        base = light;
        for (k = 0, off = 4; k < 3; k++) {
            gte_ldv0(normals + base[k * 4 + 7] * 8);
            gte_ldrgb(base + off);
            gte_nccs();
            gte_strgb(scratch);
            base[k * 4 + 4] = scratch[0];
            base[k * 4 + 5] = scratch[1];
            base[k * 4 + 6] = scratch[2];
            off += 4;
        }
    }

    count = w >> 24;
    for (i = 0; i < count; i++, light += 0x14) {
        base = light;
        for (k = 0, off = 4; k < 4; k++) {
            gte_ldv0(normals + base[k * 4 + 7] * 8);
            gte_ldrgb(base + off);
            gte_nccs();
            gte_strgb(scratch);
            base[k * 4 + 4] = scratch[0];
            base[k * 4 + 5] = scratch[1];
            base[k * 4 + 6] = scratch[2];
            off += 4;
        }
    }

    *(u32*)part->unk18 |= 2;
}
#endif

void func_800C3DB0(WorldModelPart* part, s32 flag) {
    u32 i;
    u32 j;
    s32 count;
    u8* p;
    /* 8 bytes of locals this body does not declare; identity not
     * recoverable.  Frame is 0x40 against the 0x38 the code needs. */
    u8 unusedLocals[8];

    for (j = 0; j < 2; j++) {
        p = part->unk1C;
        i = 0;
        if (j != 0) {
            p += part->unk16;
        }

        /* The first list is spelled as an explicit guard plus a do/while so
         * that `i = 0` lands in the block above the `if` -- reorg then fills
         * that branch's delay slot with it, which is what the target has.  A
         * plain `for (i = 0; ...)` puts the init in the join block and the
         * slot goes to the count guard instead (2 rows), and moving the init
         * out of a `for` without the explicit guard costs the zero-trip fold
         * (51 rows, +1 instruction).  The other seven keep the `for`. */
        count = part->numPrims[0];
        if (count != 0) {
            do {
                setSemiTrans(p, flag);
                setShadeTex(p, flag);
                i++;
                p += 0x34;
            } while (i < count);
        }
        count = part->numPrims[1];
        for (i = 0; i < count; i++, p += 0x28) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[2];
        for (i = 0; i < count; i++, p += 0x28) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[3];
        for (i = 0; i < count; i++, p += 0x20) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[4];
        for (i = 0; i < count; i++, p += 0x14) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[5];
        for (i = 0; i < count; i++, p += 0x18) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[6];
        for (i = 0; i < count; i++, p += 0x1C) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
        count = part->numPrims[7];
        for (i = 0; i < count; i++, p += 0x24) {
            setSemiTrans(p, flag);
            setShadeTex(p, flag);
        }
    }
}

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C4148);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C4FB4);

INCLUDE_ASM("asm/us/world/nonmatchings/world2", func_800C5CD4);

extern u8 D_800C752C;

/* PARKED: 204 changed / 0 inserted, and **one instruction short** (292
 * against 293).  The instruction stream is otherwise identical row for row --
 * every one of the 204 is a register name, and `insn_histogram.py` reports a
 * single opcode difference, `addu -1`.  That one insn is `move t8,a0`: the
 * target copies the parameter out of its incoming register and reads every
 * field through the copy, and gives `$a0` to the inner loop's `src` instead.
 *
 * The register lists line up exactly if you delete `part` from ours:
 *   target  a0=src a1=off a2=dst a3=p t0=i t1=light t2=p+7 t3=count t4=base
 *           t5=code t6=normals t7=w t8=part
 *   ours    a0=part a1=src a2=off a3=dst t0=p t1=i t2=code t3=light t4=base
 *           t5=p+7 t6=count t7=normals t8=w
 * i.e. `part` won `$a0` from its copy preference and pushed the whole list
 * down one.  `find_reg` honours that preference unless a higher-priority
 * allocno has already taken `$a0`, so the lever would be to make `src` outrank
 * `part` -- and `part` is already at the bottom of `allocno_compare`
 * (6 refs over the whole 293-insn body).
 *
 * Measured and rejected, all at exactly 204 rows / -1 instruction:
 *   - a local copy of the parameter, every field read through it (cse deletes
 *     the copy)
 *   - a `void*` parameter cast into a typed local (same)
 *   - `src` declared first among the locals (allocno-number tie-break)
 *   - `do { gte_ldv0(...); } while (0);` around the one `src` reference in
 *     the inner loop, i.e. the free reference multiplier -- exactly inert
 *   - `width_sweep.py`: 25 variants over 5 scalar locals, nothing below 204
 *     and nothing at the exact length
 * and at 206 / -2: both count words read up front so `part` dies early.
 *
 * What *did* get it here, and is worth keeping if this is re-derived:
 *   - `u8* normals = (u8*)D_800C7938;` with `normals + idx * 8`.  As
 *     `&D_800C7938[idx]` on an `SVECTOR[]` the symbol's `%hi`/`%lo` is
 *     rematerialised in all eight loop preheaders: +19 instructions.
 *   - `k`, `dst`, `off` and `src` initialised in the `for`'s init list, not as
 *     statements above it -- the target emits `k = 0` between `code` and
 *     `dst`, which no statement ordering reaches (4 rows).
 *   - a `dst` local in the four flat lists; `gte_strgb(p + 4)` written inline
 *     computes the address at the store and leaves the guard's delay slot
 *     empty (4 insertions).
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/world/nonmatchings/world2", func_800C6104);
#else
void func_800C6104(WorldModelPart* part) {
    u32 i;
    u32 k;
    s32 count;
    s32 off;
    u32 w;
    u8 code;
    u8* p;
    u8* light;
    u8* normals;
    u8* base;
    u8* src;
    u8* dst;

    normals = (u8*)D_800C7938;
    light = (u8*)(part->unkE + (s32)part->unk18);
    p = part->unk1C;
    if (D_800C752C != 0) {
        p += part->unk16;
    }

    w = *(u32*)&part->numPrims[0];

    count = w & 0xFF;
    for (i = 0; i < count; i++, p += 0x34, light += 0x18) {
        if (*(u32*)p != 0) {
            base = light;
            code = p[7];
            for (k = 0, dst = p + 4, off = 4, src = light; k < 4; k++,
                src += 4) {
                gte_ldv0(normals + base[k * 4 + 7] * 8);
                gte_ldrgb(base + off);
                gte_nccs();
                gte_strgb(dst);
                dst += 0xC;
                off += 4;
            }
            p[7] = code;
        }
    }

    count = (w & 0xFF00) >> 8;
    for (i = 0; i < count; i++, p += 0x28, light += 0x14) {
        if (*(u32*)p != 0) {
            base = light;
            code = p[7];
            for (k = 0, dst = p + 4, off = 4, src = light; k < 3; k++,
                src += 4) {
                gte_ldv0(normals + base[k * 4 + 7] * 8);
                gte_ldrgb(base + off);
                gte_nccs();
                gte_strgb(dst);
                dst += 0xC;
                off += 4;
            }
            p[7] = code;
        }
    }

    count = (w >> 16) & 0xFF;
    for (i = 0; i < count; i++, p += 0x28, light += 0xC) {
        if (*(u32*)p != 0) {
            dst = p + 4;
            code = p[7];
            gte_ldv0(normals + light[7] * 8);
            gte_ldrgb(light + 4);
            gte_nccs();
            gte_strgb(dst);
            p[7] = code;
        }
    }

    count = w >> 24;
    for (i = 0; i < count; i++, p += 0x20, light += 0xC) {
        if (*(u32*)p != 0) {
            dst = p + 4;
            code = p[7];
            gte_ldv0(normals + light[7] * 8);
            gte_ldrgb(light + 4);
            gte_nccs();
            gte_strgb(dst);
            p[7] = code;
        }
    }

    w = *(u32*)&part->numPrims[4];

    count = w & 0xFF;
    for (i = 0; i < count; i++, p += 0x14, light += 8) {
        if (*(u32*)p != 0) {
            dst = p + 4;
            code = p[7];
            gte_ldv0(normals + light[7] * 8);
            gte_ldrgb(light + 4);
            gte_nccs();
            gte_strgb(dst);
            p[7] = code;
        }
    }

    count = (w & 0xFF00) >> 8;
    for (i = 0; i < count; i++, p += 0x18, light += 8) {
        if (*(u32*)p != 0) {
            dst = p + 4;
            code = p[7];
            gte_ldv0(normals + light[7] * 8);
            gte_ldrgb(light + 4);
            gte_nccs();
            gte_strgb(dst);
            p[7] = code;
        }
    }

    count = (w >> 16) & 0xFF;
    for (i = 0; i < count; i++, p += 0x1C, light += 0x10) {
        if (*(u32*)p != 0) {
            base = light;
            code = p[7];
            for (k = 0, dst = p + 4, off = 4, src = light; k < 3; k++,
                src += 4) {
                gte_ldv0(normals + base[k * 4 + 7] * 8);
                gte_ldrgb(base + off);
                gte_nccs();
                gte_strgb(dst);
                dst += 8;
                off += 4;
            }
            p[7] = code;
        }
    }

    count = w >> 24;
    for (i = 0; i < count; i++, p += 0x24, light += 0x14) {
        if (*(u32*)p != 0) {
            base = light;
            code = p[7];
            for (k = 0, dst = p + 4, off = 4, src = light; k < 4; k++,
                src += 4) {
                gte_ldv0(normals + base[k * 4 + 7] * 8);
                gte_ldrgb(base + off);
                gte_nccs();
                gte_strgb(dst);
                dst += 8;
                off += 4;
            }
            p[7] = code;
        }
    }
}
#endif

s32 func_800C6598(WorldModel* model) {
    u32 i;
    s32 size;
    WorldModelPart* part;

    size = model->unk2 * 32;
    part = (WorldModelPart*)(model->unk18 + (s32)model->unk1C);
    for (i = 0; i < model->unk3; i++) {
        size += part->unk16 * 2;
        part++;
    }
    return size;
}
