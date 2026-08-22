//! PSYQ=3.3 CC1=2.6.3

#include "common.h"
#include "game.h"
#include "psxsdk/libspu.h"

typedef struct {
    s32 unk0;
    s8 unk4;
    s8 pad5[3];
    s32 unk8;
    s8 unkC;
} Unk8002C5A8;

// Field names cross-checked against the independent qgears reverse-engineering
// project's AkaoChannel struct (same source as the g_Akao*SlideStep/Steps
// naming above): https://github.com/Akari1982/q-gears_reverse,
// ffvii/DISC/SCUS_941_akao.h. Spans neither this repo nor qgears resolves
// (LFO delay/rate sub-fields, the 0xA8-0xB8 gap) are left as unkNN.
typedef struct {
    u8* addr;
    u8* loop_addr[4];
    u8* drum_addr;
    u32 vibrato_wave;
    u32 tremolo_wave;
    u32 pan_lfo_wave;
    u32 overlay_voice;
    s32 alt_voice_id;
    s32 vol_master;
    s32 pitch_base;
    s32 pitch_slide;
    s32 update_flags;
    u32 pitch_mul_sound;
    s32 pitch_mul_sound_slide_step;
    s32 volume;
    s32 vol_slide_step;
    s32 pitch_slide_step;
    u32 unk50; // unresolved by qgears either (marked as 4 unknown bytes there
               // too)
    u16 type;
    u8 length_1;
    u8 length_2;
    u16 instr_id;
    u16 pitch_mul_sound_slide_steps;
    u16 vol_slide_steps;
    u16 vol_balance_slide_steps;
    s16 vol_pan;
    s16 vol_pan_slide_steps;
    s16 pitch_slide_steps_cur;
    u16 octave;
    u16 pitch_slide_steps;
    u16 key_stored;
    s16 portamento_steps;
    s16 sfx_mask;
    u8 unk70[0xC];
    s16 unk7C;
    s16 vibrato_depth;
    s16 vibrato_depth_slide_steps;
    s16 unk82;
    u8 unk84[0xC];
    s16 tremolo_depth;
    s16 tremolo_depth_slide_steps;
    s16 tremolo_depth_slide_step;
    s16 unk96;
    s16 pan_lfo_rate;
    s16 pan_lfo_rate_cur;
    s16 pan_lfo_type;
    s16 pan_lfo_depth;
    s16 pan_lfo_depth_slide_steps;
    s16 pan_lfo_depth_slide_step;
    s16 noise_switch_delay;
    s16 pitch_lfo_switch_delay;
    s16 unkA8;
    s16 unkAA;
    s16 unkAC;
    s16 unkAE;
    u8 unkB0[0x8];
    u16 loop_id;
    u16 loop_times[0x4];
    u16 length_stored;
    u16 length_fixed;
    s16 vol_balance;
    s16 vol_balance_slide_step;
    s16 vol_pan_slide_step;
    s16 transpose;
    s16 fine_tuning;
    s16 key;
    s16 key_add;
    s16 transpose_stored;
    s16 vibrato_pitch;
    s16 tremolo_vol;
    s16 pan_lfo_vol;
    s32 unkDC;
    s32 attr_mask;
    u32 unkE4;
    u32 unkE8;
    u32 unkEC;
    u32 unkF0;
    u32 unkF4;
    u16 unkF8;
    u16 unkFA;
    u16 unkFC;
    u16 unkFE;
    u16 unk100;
    u16 unk102;
    u8 unk104[0x4];
} AKAO_TRACK; // size:0x108

typedef struct {
    u8 pad0[0x4];
    u32 unk4;
    u8 pad8[0x10];
    u32 tempo;
    s32 tempo_slide_step;
    u8 pad20[0x4];
    u32 unk24;
    u32 unk28;
    u32 noise_mask;
    u32 reverb_mask;
    u32 pitch_lfo_mask;
    u8 pad38[0x8];
    u32 reverb_depth;
    s32 reverb_depth_slide_step;
    u16 tempo_slide_length;
    u16 song_id;
    u16 last_condition;
    u16 condition;
    u16 reverb_depth_slide_length;
    u16 noise_clock;
    u16 field_54;
    u16 beats_per_measure;
    u16 beat;
    u16 ticks_per_beat;
    u16 tick;
    u16 measure;
} AKAO_CONFIG;

typedef struct {
    /* 0x0 */ u8 unk0;
    /* 0x1 */ u8 unk1;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ u16 unkC;
    /* 0xE */ u16 unkE;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} Unk8002B7E0; // size:0x24

typedef struct {
    u8 pad0[0x3C];
    s32 unk3C;
    u8 pad1[0x1A];
    s16 unk5A;
    u8 pad2[0x2];
    s16 unk5E;
    s16 unk60;
    s16 unk62;
    u8 pad3[0x62];
    s16 unkC6;
    u8 pad4[0x18];
    s32 unkE0;
    u8 pad5[0x24];
} Unk80099788Half; // size 0x108

typedef struct {
    Unk80099788Half half0;
    Unk80099788Half half1;
} Unk80099788; // size 0x210

typedef struct {
    u8* unk0;
    u8 pad04[0x50];
    u16 unk54;
    u16 unk56;
    u8 pad58[0x88];
    s32 unkE0;
    u8 padE4[0x24];
} Unk80096608; // size 0x108

extern void (*D_80049548[])(Unk8002B7E0*);
extern u8 D_800499A8[]; // opcode lenghts
extern u8 D_80049C40[];
extern s32 g_AkaoWaveTableKey[];
extern s32 D_80062F00;
extern s32 D_80062F04;
extern u16 D_80062F1E;
extern u8 D_80075F34[];

// One 0x40-byte record per instrument. Only the first 16 bytes are read by
// the driver: two words of SPU envelope, then eight LFO/parameter bytes.
typedef struct {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ u8 unk8;
    /* 0x9 */ u8 unk9;
    /* 0xA */ u8 unkA;
    /* 0xB */ u8 unkB;
    /* 0xC */ u8 unkC;
    /* 0xD */ u8 unkD;
    /* 0xE */ u8 unkE;
    /* 0xF */ u8 unkF;
    /* 0x10 */ u8 unk10[0x30];
} AkaoInstrument; // size:0x40

extern AkaoInstrument D_80075F28[];
extern s32 D_80076C68[];
// Music-driver slide state: each MulMusic value is a fixed-point scalar for
// pitch/volume/tempo (current value in the upper 16 bits, lower 16 bits are
// fractional precision the driver accumulates every tick for a smooth
// ramp); *SlideStep is the per-tick delta added to it, *SlideSteps is the
// remaining tick count. Names/meaning confirmed one-off against the
// independent qgears reverse-engineering project (not part of this repo):
// https://github.com/q-gears/q-gears, src/main/SCUS_941_akao.cpp.
extern s32 g_AkaoPitchMulMusicSlideStep;
extern s32 g_AkaoVolMulMusicSlideStep;
extern s32 g_AkaoTempoMulMusicSlideStep;
extern s16 g_AkaoPitchMulMusicSlideSteps;
extern s16 g_AkaoVolMulMusicSlideSteps;
extern s16 g_AkaoTempoMulMusicSlideSteps;
extern s32 g_AkaoVolMulMusic;
extern u16 D_80062F70;
extern s32 D_80062F74;
extern s32 D_80062F84;
extern s32 D_80062F8C;
extern s32 D_80062FAC;
extern s32 D_80062FB0;
extern s32 g_AkaoCdVolSlideStep;
extern u16 D_80062FB8;
extern u16 g_AkaoCdVolSlideSteps;
extern s32 g_AkaoCdVol;
extern u16 D_80062FD6;
extern s32 D_80062FD8;
extern s32 g_AkaoPitchMulMusic;
extern s32 g_AkaoTempoMulMusic;
extern s32 D_80062FF8;
extern s32 D_80063004;
extern s32 D_80063010; // sound message queue count
extern u8 g_FieldMovieLock;
extern u8 D_8007EBE4[];
extern s32 D_8007EBE8;
extern s32 D_8007EBEC;
extern s32 D_8007EBF0;
extern s32 D_8007EBF4;
extern s32 D_8007EBF8;
extern s32 D_8007EBFC;
extern u16 D_8007EC00;
extern u16 D_8007EC02;
extern u16 D_8007EC04;
extern u16 D_8007EC06;
extern u16 D_8007EC08;
extern u16 D_8007EC0A;
extern s16 D_8007EC0C;
extern s16 D_8007EC0E;
extern s32 D_8007EC10;
extern s32 D_800804D0;
extern Unk8002B7E0 D_80081DC8[]; // sound messages queue
extern s32 D_80083334;
extern u16 D_8008337E;
extern s32 D_80083394;
extern u16 D_800833DE;
extern s32 D_80083580[];
extern Unk80096608 D_80096608[];
extern Unk80096608 D_800966E8[];
extern s32 D_80097768;
extern s32 D_80097870;
extern Unk80096608 D_80099868[];
extern Unk80099788 D_80099788[];
extern s32 g_AkaoChannelMask[];
extern s32 D_80099FD8;
extern s32 D_80099FDC;
extern s32 g_AkaoNoiseMask;
extern s32 g_AkaoReverbMask;
extern s32 g_AkaoPitchLfoMask;
extern u16 D_80099E0C;
extern s32 D_80099FD0;
extern s32 D_80099FD4;
extern u16 D_80099FFA;
extern u16 D_8009A14E;
extern u16 D_8009A152;
extern s32 D_8009A104;
extern s32 g_AkaoPendingMask;
extern s32 D_8009A10C;
extern s32 D_8009A110;
extern s32 D_8009A114;
extern s32 D_8009A118;
extern s32 D_8009A128;
extern s32 D_8009A12C;
extern s32 D_8009A13C;

extern u32 g_ReverbMode;
extern SpuReverbAttr g_ReverbAttr;

extern SpuCommonAttr D_8009C578;

#define READ_S8(addr) ((s8)(*(addr)++))
#define READ_S16(addr) ((s16)(*(addr)++ | (*(addr)++ << 8)))

void func_8002CF98(Unk8002B7E0* arg0);
void func_8002B1F8(Unk8002B7E0* arg0);
void func_8002B2F8(Unk8002B7E0* arg0);
void func_8002B3B4(Unk8002B7E0* arg0);
void func_8002B5A8(Unk8002B7E0* arg0);
void func_8002B608(Unk8002B7E0* arg0);
void func_8002B904(Unk8002B7E0* arg0);
void func_8002B6AC(Unk8002B7E0* arg0);
void func_8002B730(Unk8002B7E0* arg0);
void func_8002B7E0(Unk8002B7E0* arg0);
void func_8002B958(Unk8002B7E0* arg0);
void func_8002B9AC(Unk8002B7E0* arg0);
void func_8002BA08(Unk8002B7E0* arg0);
void func_8002B8B4(Unk8002B7E0* arg0);
void func_8002B668(Unk8002B7E0* arg0);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800293D0);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800293F4);

static void func_80029424(s32 arg0, s32 arg1) {
    func_800293F4();
    func_80038F04(arg0, arg1);
}

static void func_80029464(s32 arg0, s32 arg1) {
    func_800293F4();
    SpuRead(arg0, arg1);
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800294A4);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800294BC);

/* Kick off a DMA of the header at arg0 into SPU RAM, then stage the payload
 * into the instrument table while the transfer runs and wait for it.
 *
 * PARKED, 3 rows, 29 instructions (exact length). The whole residue is the
 * loop counter: the target holds it in $a1 and this holds it in $a0, so the
 * `li`, the `addiu -1` and the `bnez` all name the wrong register and
 * nothing else in the function differs. $a1 is where func_80029424's second
 * argument was, and it is free by the loop; $a0 is free too and is lower, so
 * gcc takes it.
 *
 * 21 spellings measured, every one of them **exactly 3 rows**: both
 * declaration orders, `s32`/`u32`/`u16` counters, `do`/`while` against
 * `for`, `--i` in the test against `i--` at the top of the body, the copy
 * split into four statements, a separate cursor for `data`, both orders of
 * the `i = 0x800` / `dst = ...` setup, named locals for either or both of
 * func_80029424's arguments (to give $a0 an occupant), `&src[3]`/`src[0]`
 * subscripts, and the call's second argument shared with the counter as one
 * variable (CLAUDE.md's "a value that a call clobbers and a result that
 * replaces it are one variable") -- 4, 7 and 8 rows for the three sharing
 * shapes, so sharing is a regression, not the answer.
 *
 * The pass is settled, not guessed: a `do { } while (0);` barrier before the
 * loop, inside the body and after the loop measures **exactly 3 rows in all
 * three positions**, which per CLAUDE.md means the residue is register
 * allocation and not sched2 -- so every reordering and re-spelling above was
 * inert by construction. What is left is `QTY_CMP_PRI`'s choice of hard
 * register for a block-local quantity, which is not reachable from C without
 * emitting an instruction; this is a park, not a permuter target.
 *
 * func_80029818 below is the same function with a different destination and
 * count and carries the identical residue -- fix one and the other follows.
 */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/akao", func_800297A4);
#else
void func_800297A4(s32* src, s32* data) {
    s32* dst;
    s32 i;

    SpuSetTransferStartAddr(*src++);
    func_80029424((s32)(src + 3), *src);
    i = 0x800;
    dst = (s32*)D_80075F28;
    do {
        i--;
        *dst++ = *data++;
    } while (i != 0);
    func_800294A4();
}
#endif

/* PARKED, 3 rows, 30 instructions (exact length). Identical residue to
 * func_800297A4 above -- see that note for the 21 measured spellings and the
 * barrier probe that says it is register allocation. */
#ifndef NON_MATCHINGS
MASPSX_OVERRIDE("asm/us/main/nonmatchings/akao", func_80029818);
#else
void func_80029818(s32* src, s32* data) {
    s32* dst;
    s32 i;

    SpuSetTransferStartAddr(*src++);
    func_80029424((s32)(src + 3), *src);
    i = 0x4B0;
    dst = (s32*)D_80076C68;
    do {
        i--;
        *dst++ = *data++;
    } while (i != 0);
    func_800294A4();
}
#endif

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002988C);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80029998);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800299C8);

// Key off the voices in D_80062F00 and clear the SPU transfer/IRQ callbacks.
static void func_80029A50(void) {
    SpuSetTransferCallback(0);
    SpuSetIRQ(0);
    SpuSetIRQCallback(0);
    SpuSetKey(0, D_80062F00);
    if (D_80062F00 & 0x10000) {
        D_80097768 = 0x1FF93;
    }
    if (D_80062F00 & 0x20000) {
        D_80097870 = 0x1FF93;
    }
    D_80062F00 = 0;
    func_80030038();
    func_80030148();
    func_8002FF4C();
}

void SetReverbMode(s32 in_ReverbMode) {
    func_80029A50();
    SpuGetReverbModeParam(&g_ReverbAttr);
    if (g_ReverbAttr.mode != in_ReverbMode) {
        g_ReverbMode = in_ReverbMode;
        SpuSetReverb(SPU_OFF);
        g_ReverbAttr.mode = in_ReverbMode | SPU_REV_MODE_CLEAR_WA;
        g_ReverbAttr.mask = SPU_REV_MODE;
        SpuSetReverbModeParam(&g_ReverbAttr);
        SpuSetReverb(SPU_ON);
    }
}

// Word-copies (arg1 >> 2) words from arg0 into staging buffer D_80083580.
static void func_80029B78(s32* arg0, u32 arg1) {
    s32* dst;
    u32 nwords;

    nwords = arg1 >> 2;
    dst = D_80083580;
    while (nwords != 0) {
        nwords -= 1;
        *dst = *arg0;
        arg0 += 1;
        dst += 1;
    }
}

void func_80031820(AKAO_TRACK*, s32);

static void SoundChannelInit(AKAO_TRACK* arg0, u8* arg1) {
    arg0->addr = arg1;
    arg0->vol_master = 0x78;
    func_80031820(arg0, 5);
    arg0->octave = 2;
    arg0->fine_tuning = 0;
    arg0->transpose = 0;
    arg0->portamento_steps = 0;
    arg0->pitch_slide = 0;
    arg0->key_add = 0;
    arg0->length_fixed = 0;
    arg0->length_stored = 0;
    arg0->pitch_slide_steps_cur = 0;
    arg0->volume = 0x32000000;
    arg0->vol_slide_steps = 0;
    arg0->update_flags = 0;
    arg0->loop_id = 0;
    arg0->sfx_mask = 0;
    arg0->pan_lfo_vol = 0;
    arg0->pan_lfo_depth = 0;
    arg0->tremolo_depth = 0;
    arg0->vibrato_depth = 0;
    arg0->pan_lfo_depth_slide_steps = 0;
    arg0->tremolo_depth_slide_steps = 0;
    arg0->vibrato_depth_slide_steps = 0;
    arg0->pitch_lfo_switch_delay = 0;
    arg0->noise_switch_delay = 0;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80029C48);

// Merges newly-requested bits (D_8009A128/D_8009A12C) into the pending mask
// g_AkaoPendingMask, then for each set bit points the matching D_80096608 slot
// at the default D_80049C40 sample and marks it (unk56 = 0x204), clearing the
// request bits as it goes.
static void func_80029E98(void) {
    s32 mask;
    s32 bit;
    Unk80096608* slot;
    s32 req0;
    s32 req1;

    if (g_AkaoPendingMask != 0) {
        slot = D_80096608;
        bit = 1;
        req0 = D_8009A128;
        req1 = D_8009A12C;
        D_8009A12C = 0;
        D_8009A128 = 0;
        D_8009A110 = 0;
        D_8009A10C = 0;
        req0 |= req1;
        mask = g_AkaoPendingMask;
        mask |= req0;
        g_AkaoPendingMask = mask;
        D_8009A114 |= mask;
        do {
            if (mask & bit) {
                mask ^= bit;
                slot->unk56 = 0x204;
                slot->unk0 = D_80049C40;
            }
            bit *= 2;
            slot += 1;
        } while (mask != 0);
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80029F44);

void func_8002A094(u16 arg0, s32 arg1, s32 arg2, s32 arg3);
INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002A094);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002A28C);

void func_8002A43C(void) {
    Unk80096608* p;
    u16 i;
    s32 keep;

    for (p = (Unk80096608*)D_80099788, i = 0x30; i < 0x38; i++, p++) {
        if (p->unk54 != 2) {
            p->unk56 = 0x204;
            p->unk0 = D_80049C40;
        }
    }
    if (D_80099E0C == 2) {
        D_80099FD0 &= 0xC00000;
        D_80099FD4 &= 0xC00000;
        keep = D_80099FD8 & 0xFF3FFFFF;
        D_80099FD8 = keep & g_AkaoChannelMask[0];
    } else {
        D_80099FD0 = 0;
        D_80099FD4 = 0;
        D_80099FD8 = g_AkaoChannelMask[0];
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002A510);

// Resolves a 10-bit note index into a pair of table entries: looks up
// D_80062F74[index] and D_80062F74[index+1] (u16), adding D_80062F84 unless the
// entry is the 0xFFFF "unused" sentinel (in which case the result is 0).
static void func_8002A6C4(s32* arg0, s32* arg1, u16 arg2) {
    u16 idx;
    s32 val0;
    s32 val1;
    u16 raw0;
    u16 raw1;

    idx = (arg2 & 0x3FF) * 2;
    raw0 = *(u16*)((idx * 2) + D_80062F74);
    if (raw0 != 0xFFFF) {
        val0 = raw0 + D_80062F84;
    } else {
        val0 = 0;
    }
    *arg0 = val0;
    idx = idx + 1;
    raw1 = *(u16*)((idx * 2) + D_80062F74);
    if (raw1 != 0xFFFF) {
        val1 = raw1 + D_80062F84;
    } else {
        val1 = 0;
    }
    *arg1 = val1;
}

// Flag every voice named by the pending mask as needing a hardware update.
void func_8002A748(void) {
    Unk80096608* p;
    s32 mask;
    u32 bit;

    mask = g_AkaoPendingMask;
    p = D_80096608;
    if (mask != 0) {
        bit = 1;
        do {
            if (mask & bit) {
                p->unkE0 |= 3;
                mask ^= bit;
            }
            p++;
            bit <<= 1;
        } while (mask != 0);
    }
}

void func_8002A798(void) {
    Unk80096608* p;
    s32 mask;
    u32 bit;

    mask = g_AkaoChannelMask[0];
    p = (Unk80096608*)D_80099788;
    if (mask != 0) {
        bit = 0x10000;
        do {
            if (mask & bit) {
                p->unkE0 |= 3;
                mask ^= bit;
            }
            p++;
            bit <<= 1;
        } while (mask != 0);
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002A7E8);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002A958);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002AABC);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002AFB8);

void func_8002B1A8(s32* src0, s32* dst0, s32* src1, s32* dst1) {
    u16 i;

    i = 0x630;
    do {
        i--;
        *dst0++ = *src0++;
    } while (i != 0);
    i = 0x18;
    do {
        i--;
        *dst1++ = *src1++;
    } while (i != 0);
}

void func_8002B1F8(Unk8002B7E0* arg0) {
    func_80029B78(arg0->unk4, arg0->unk8);
    if (D_8009A14E == 0xE) {
        func_8002A7E8();
        func_8002B1A8(&D_80096608, &D_800804D0, &D_8009A104, &D_80083394);
    }
    func_80029E98();
    if (D_8008337E && D_8008337E == arg0->unkC) {
        func_8002AABC(0);
    } else if (D_800833DE && D_800833DE == arg0->unkC) {
        func_8002AABC(1);
    } else {
        func_80029C48();
    }
    D_8009A14E = arg0->unkC;
}

void func_8002B2F8(Unk8002B7E0* arg0) {
    s32* var_a2;

    func_80029B78(arg0->unk4, arg0->unk8);
    func_8002A7E8();
    var_a2 = &D_8009A104;
    if (D_8009A14E) {
        if (D_8009A14E == 0xE) {
            func_8002B1A8(&D_80096608, &D_800804D0, var_a2, &D_80083394);
        } else {
            func_8002B1A8(&D_80096608, &D_8007EC10, var_a2, &D_80083334);
        }
    }
    func_80029E98();
    func_80029C48();
    D_8009A14E = arg0->unkC;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002B3B4);

extern u16 D_80062FC8;

void func_8002B5A8(Unk8002B7E0* arg0) {
    if (D_8009A14E) {
        D_80062FC8 = arg0->unk10 ? arg0->unk10 : 0x10;
        func_8002AFB8();
    }
    func_8002B1F8(arg0);
}

void func_8002B608(Unk8002B7E0* arg0) {
    if (D_8009A14E) {
        D_80062FC8 = arg0->unk10 ? arg0->unk10 : 0x10;
        func_8002AFB8();
    }
    func_8002B2F8(arg0);
}

void func_8002B668(Unk8002B7E0* arg0) {
    func_8002A510(4, 1);
    func_8002A094(0x40, 0x34, arg0->unk4, arg0->unk8);
}

void func_8002B6AC(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(4, 2);
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x32, sp10, sp14);
    func_8002A6C4(&sp10, &sp14, arg0->unkC);
    func_8002A094(arg0->unk4, 0x34, sp10, sp14);
}

void func_8002B730(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(4, 3);
    func_80029A50();
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x30, sp10, sp14);
    func_8002A6C4(&sp10, &sp14, arg0->unkC);
    func_8002A094(arg0->unk4, 0x32, sp10, sp14);
    func_8002A6C4(&sp10, &sp14, arg0->unk10);
    func_8002A094(arg0->unk4, 0x34, sp10, sp14);
}

void func_8002B7E0(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(6, 4);
    func_80029A50();
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x30, sp10, sp14);
    func_8002A6C4(&sp10, &sp14, arg0->unkC);
    func_8002A094(arg0->unk4, 0x32, sp10, sp14);
    func_8002A6C4(&sp10, &sp14, arg0->unk10);
    func_8002A094(arg0->unk4, 0x34, sp10, sp14);
    func_8002A6C4(&sp10, &sp14, arg0->unk14);
    func_8002A094(arg0->unk4, 0x36, sp10, sp14);
}

void func_8002B8B4(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(6, 1);
    func_8002A6C4(&sp10, &sp14, arg0->unk4);
    func_8002A28C(sp10, sp14);
}

void func_8002B904(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(4, 1);
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x34, sp10, sp14);
}

void func_8002B958(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(2, 1);
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x32, sp10, sp14);
}

void func_8002B9AC(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(0, 1);
    func_80029A50();
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x30, sp10, sp14);
}

void func_8002BA08(Unk8002B7E0* arg0) {
    s32 sp10, sp14;

    func_8002A510(6, 1);
    func_8002A6C4(&sp10, &sp14, arg0->unk8);
    func_8002A094(arg0->unk4, 0x36, sp10, sp14);
}

void AkaoC0VolumeSet(Unk8002B7E0* arg0) {
    g_AkaoVolMulMusicSlideSteps = 0;
    g_AkaoVolMulMusic = (arg0->unk4 & 0x7F) << 0x10;
    func_8002A748();
}

typedef struct {
    u32 unk0;
    s32 unk4;
    s32 unk8;
} Unk8002BA98;

// Starts a volume slide from the current g_AkaoVolMulMusic toward a target
// derived from arg0, over arg0's tick count.
void AkaoC1VolumeSlideFromCurrent(Unk8002BA98* arg0) {
    s32 temp_v0;
    s32 var_a1;

    temp_v0 = arg0->unk4;
    var_a1 = 1;
    if (temp_v0 != 0) {
        var_a1 = temp_v0;
    }
    g_AkaoVolMulMusicSlideSteps = var_a1;
    g_AkaoVolMulMusicSlideStep =
        (((arg0->unk8 & 0x7F) << 0x10) - g_AkaoVolMulMusic) / var_a1;
    func_8002A748();
}

typedef struct {
    u32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} Unk8002BB20;

// Starts a volume slide between two explicit targets from arg0 (rather than
// from the current g_AkaoVolMulMusic), over arg0's tick count.
void AkaoC2VolumeSlideBetweenTargets(Unk8002BB20* arg0) {
    s32 temp_v1;
    s32 var_a1;
    s32 temp_v0;

    temp_v0 = arg0->unk4;
    var_a1 = 1;
    if (temp_v0 != 0) {
        var_a1 = temp_v0;
    }
    temp_v0 = (arg0->unkC & 0x7F) << 0x10;
    temp_v1 = (arg0->unk8 & 0x7F) << 0x10;
    g_AkaoVolMulMusicSlideSteps = var_a1;
    g_AkaoVolMulMusic = temp_v1;
    g_AkaoVolMulMusicSlideStep = (temp_v0 - temp_v1) / var_a1;
    func_8002A748();
}

static void AkaoUpdateCdVolume(void);

typedef struct {
    u32 unk0;
    u16 unk4;
} Unk8002BBB4;

// Set the CD volume outright, cancelling any slide in progress.
void func_8002BBB4(Unk8002BBB4* arg0) {
    g_AkaoCdVolSlideSteps = 0;
    g_AkaoCdVol = arg0->unk4 << 16;
    AkaoUpdateCdVolume();
}

typedef struct {
    u32 unk0;
    s32 unk4;
    u16 unk8;
} Unk8002BBEC;

// Starts a CD-audio volume slide from the current g_AkaoCdVol toward a
// target derived from arg0, over arg0's tick count.
void AkaoC9CdVolumeSlideFromCurrent(Unk8002BBEC* arg0) {
    s32 temp_v0;
    s32 var_a1;

    temp_v0 = arg0->unk4;
    var_a1 = 1;
    if (temp_v0 != 0) {
        var_a1 = temp_v0;
    }
    g_AkaoCdVolSlideSteps = var_a1;
    g_AkaoCdVolSlideStep = ((arg0->unk8 << 0x10) - g_AkaoCdVol) / var_a1;
}

typedef struct {
    u32 unk0;
    s32 unk4;
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u16 unkE;
} Unk8002BC58;

// Starts a CD-audio volume slide between two explicit targets from arg0
// (rather than from the current g_AkaoCdVol), over arg0's tick count.
void AkaoCACdVolumeSlideBetweenTargets(Unk8002BC58* arg0) {
    s32 temp_v0;
    s32 temp_v1;
    s32 var_a1;
    s32 temp_v0_shifted;
    s32 temp_v1_shifted;

    temp_v0 = arg0->unk4;
    var_a1 = 1;
    if (temp_v0 != 0) {
        var_a1 = temp_v0;
    }
    temp_v0_shifted = arg0->unkC << 0x10;
    temp_v1_shifted = arg0->unk8 << 0x10;
    g_AkaoCdVolSlideSteps = var_a1;
    g_AkaoCdVol = temp_v1_shifted;
    g_AkaoCdVolSlideStep = (temp_v0_shifted - temp_v1_shifted) / var_a1;
}

// The voice record at arg1 is two identical-layout 0x108-byte halves. Write the
// note's transposed pitch (arg0+4) into a field in each half, clear another
// field in each half, and set flag bits 0x3 in each half's control word
// (+0xE0).
static void func_8002BCCC(void* arg0, void* arg1) {
    u16 val0;
    s32 v1;
    s32 v0_e0;
    Unk80099788* voice = (Unk80099788*)arg1;
    // The do{}while(0) affects register allocation and is required for the
    // match.
    do {
        val0 = *((u16*)((u8*)arg0 + 0x4));
        v1 = voice->half1.unkE0;
        voice->half1.unk5E = 0;
        voice->half0.unk5E = 0;
        voice->half1.unkC6 = (s16)((val0 & 0x7F) << 8);
    } while (0);
    voice->half0.unkC6 = (s16)((val0 & 0x7F) << 8);
    v0_e0 = voice->half0.unkE0;
    voice->half1.unkE0 = v1 | 3;
    voice->half0.unkE0 = v0_e0 | 3;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002BD04);

// Apply the paired handler to 4 blocks spaced 0x210 bytes apart.
void func_8002BDCC(void* arg0) {
    func_8002BCCC(arg0, &D_80099788[3]);
    func_8002BCCC(arg0, &D_80099788[2]);
    func_8002BCCC(arg0, &D_80099788[1]);
    func_8002BCCC(arg0, &D_80099788[0]);
}

// Apply the paired handler to 4 blocks spaced 0x210 bytes apart.
void func_8002BE2C(void* arg0) {
    func_8002BD04(arg0, &D_80099788[3]);
    func_8002BD04(arg0, &D_80099788[2]);
    func_8002BD04(arg0, &D_80099788[1]);
    func_8002BD04(arg0, &D_80099788[0]);
}

void func_8002BE8C(void* arg0) { func_8002BCCC(arg0, &D_80099788[2]); }

void func_8002BEB4(s32 arg0) { func_8002BD04(arg0, &D_80099788[2]); }

void func_8002BEDC(void* arg0) { func_8002BCCC(arg0, &D_80099788[1]); }

void func_8002BF04(s32 arg0) { func_8002BD04(arg0, &D_80099788[1]); }

void func_8002BF2C(void* arg0) { func_8002BCCC(arg0, &D_80099788[0]); }

void func_8002BF54(s32 arg0) { func_8002BD04(arg0, &D_80099788[0]); }

void func_8002BF7C(void* arg0) { func_8002BCCC(arg0, &D_80099788[3]); }

void func_8002BFA4(s32 arg0) { func_8002BD04(arg0, &D_80099788[3]); }

// Same shape as func_8002BCCC (two 0x108-byte halves, shared +0xE0 control
// word), at a different pitch/clear field within each half.
static void func_8002BFCC(void* arg0, void* arg1) {
    s16 temp_v0;
    s32 v1;
    Unk80099788* voice = (Unk80099788*)arg1;

    temp_v0 = (*(u16*)((u8*)arg0 + 0x4) & 0x7F) << 8;
    v1 = voice->half1.unkE0;
    voice->half1.unk62 = 0;
    voice->half0.unk62 = 0;
    voice->half1.unk60 = temp_v0;
    voice->half0.unk60 = temp_v0;
    voice->half0.unkE0 = voice->half0.unkE0 | 3;
    voice->half1.unkE0 = (v1 | 3);
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002C004);

// Apply the paired handler to 4 blocks spaced 0x210 bytes apart.
void func_8002C0CC(void* arg0) {
    func_8002BFCC(arg0, &D_80099788[3]);
    func_8002BFCC(arg0, &D_80099788[2]);
    func_8002BFCC(arg0, &D_80099788[1]);
    func_8002BFCC(arg0, &D_80099788[0]);
}

// Apply the paired handler to 4 blocks spaced 0x210 bytes apart.
void func_8002C12C(void* arg0) {
    func_8002C004(arg0, &D_80099788[3]);
    func_8002C004(arg0, &D_80099788[2]);
    func_8002C004(arg0, &D_80099788[1]);
    func_8002C004(arg0, &D_80099788[0]);
}

void func_8002C18C(void* arg0) { func_8002BFCC(arg0, &D_80099788[2]); }

void func_8002C1B4(s32 arg0) { func_8002C004(arg0, &D_80099788[2]); }

void func_8002C1DC(void* arg0) { func_8002BFCC(arg0, &D_80099788[1]); }

void func_8002C204(s32 arg0) { func_8002C004(arg0, &D_80099788[1]); }

void func_8002C22C(void* arg0) { func_8002BFCC(arg0, &D_80099788[0]); }

void func_8002C254(s32 arg0) { func_8002C004(arg0, &D_80099788[0]); }

void func_8002C27C(void* arg0) { func_8002BFCC(arg0, &D_80099788[3]); }

void func_8002C2A4(s32 arg0) { func_8002C004(arg0, &D_80099788[3]); }

// Same shape as func_8002BCCC/func_8002BFCC (two 0x108-byte halves, shared
// +0xE0 control word), at a third pitch/clear field, setting flag bit 0x10
// instead of 0x3.
static void func_8002C2CC(void* arg0, void* arg1) {
    s32 temp_v0;
    s32 temp_v1;
    s8* arg0_bytes = (s8*)arg0;
    Unk80099788* voice = (Unk80099788*)arg1;

    temp_v0 = arg0_bytes[4] << 8;
    temp_v1 = voice->half1.unkE0;
    voice->half1.unk5A = 0;
    voice->half0.unk5A = 0;
    voice->half1.unk3C = temp_v0;
    voice->half0.unk3C = temp_v0;
    voice->half0.unkE0 = voice->half0.unkE0 | 0x10;
    voice->half1.unkE0 = temp_v1 | 0x10;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002C300);

// Apply the paired handler to 4 blocks spaced 0x210 bytes apart.
void func_8002C3A8(void* arg0) {
    func_8002C2CC(arg0, &D_80099788[3]);
    func_8002C2CC(arg0, &D_80099788[2]);
    func_8002C2CC(arg0, &D_80099788[1]);
    func_8002C2CC(arg0, &D_80099788[0]);
}

// Apply the paired handler to 4 blocks spaced 0x210 bytes apart.
void func_8002C408(void* arg0) {
    func_8002C300(arg0, &D_80099788[3]);
    func_8002C300(arg0, &D_80099788[2]);
    func_8002C300(arg0, &D_80099788[1]);
    func_8002C300(arg0, &D_80099788[0]);
}

void func_8002C468(void* arg0) { func_8002C2CC(arg0, &D_80099788[2]); }

void func_8002C490(s32 arg0) { func_8002C300(arg0, &D_80099788[2]); }

void func_8002C4B8(void* arg0) { func_8002C2CC(arg0, &D_80099788[1]); }

void func_8002C4E0(s32 arg0) { func_8002C300(arg0, &D_80099788[1]); }

void func_8002C508(void* arg0) { func_8002C2CC(arg0, &D_80099788[0]); }

void func_8002C530(s32 arg0) { func_8002C300(arg0, &D_80099788[0]); }

void func_8002C558(void* arg0) { func_8002C2CC(arg0, &D_80099788[3]); }

void func_8002C580(s32 arg0) { func_8002C300(arg0, &D_80099788[3]); }

void func_8002C5A8(Unk8002C5A8* arg0) {
    s32 n = arg0->unk4;
    g_AkaoTempoMulMusicSlideSteps = 0;
    g_AkaoTempoMulMusic = n << 0x10;
}

typedef struct {
    s32 unk0;
    s32 unk4;
    s8 unk8;
} Unk8002C5C8;

// Starts a tempo slide toward a target derived from arg0, over arg0's tick
// count.
void AkaoD1TempoSlideFromCurrent(Unk8002C5C8* arg0) {
    s32 temp_v0;
    s32 var_a1;

    temp_v0 = arg0->unk4;
    var_a1 = 1;
    if (temp_v0 != 0) {
        var_a1 = temp_v0;
    }
    g_AkaoTempoMulMusicSlideStep =
        ((arg0->unk8 << 0x10) - g_AkaoTempoMulMusic) / var_a1;
    g_AkaoTempoMulMusicSlideSteps = var_a1;
}

// Starts a tempo slide between two explicit targets from arg0, over arg0's
// tick count.
void AkaoD2TempoSlideBetweenTargets(Unk8002C5A8* arg0) {
    long new_var;
    s32 temp_a2;
    s32 temp_v1;
    s32 var_a1;

    temp_v1 = arg0->unk8;
    temp_a2 = arg0->unk4 << 0x10;
    g_AkaoTempoMulMusic = temp_a2;
    var_a1 = 1;
    if (temp_v1 != 0) {
        var_a1 = temp_v1;
    }
    new_var = (arg0->unkC << 0x10) - temp_a2;
    g_AkaoTempoMulMusicSlideSteps = var_a1;
    g_AkaoTempoMulMusicSlideStep = new_var / var_a1;
}

void func_8002C6A8(Unk8002C5A8* arg0) {
    s32 n = arg0->unk4;
    g_AkaoPitchMulMusicSlideSteps = 0;
    g_AkaoPitchMulMusic = n << 0x10;
}

typedef struct {
    s32 unk0;
    s32 unk4;
    s8 unk8;
} Unk8002C6C8;

// Starts a pitch slide from the current g_AkaoPitchMulMusic toward a
// target derived from arg0, over arg0's tick count.
void AkaoD5PitchSlideFromCurrent(Unk8002C6C8* arg0) {
    s32 temp_v0;
    s32 var_a1;
    s32 temp_v1;

    temp_v0 = arg0->unk4;
    var_a1 = 1;
    if (temp_v0 != 0) {
        var_a1 = temp_v0;
    }
    temp_v1 = ((arg0->unk8 << 0x10) - g_AkaoPitchMulMusic) / var_a1;
    g_AkaoPitchMulMusicSlideSteps = var_a1;
    g_AkaoPitchMulMusicSlideStep = temp_v1;
}

// Starts a pitch slide between two explicit targets from arg0, over arg0's
// tick count.
void AkaoD6PitchSlideBetweenTargets(Unk8002C5A8* arg0) {
    s32 new_var;
    s32 temp_a2;
    s32 temp_v1;
    s32 var_a1;

    temp_v1 = arg0->unk8;
    temp_a2 = arg0->unk4 << 0x10;
    g_AkaoPitchMulMusic = temp_a2;
    var_a1 = 1;
    if (temp_v1 != 0) {
        var_a1 = temp_v1;
    }
    new_var = (arg0->unkC << 0x10) - temp_a2;
    g_AkaoPitchMulMusicSlideSteps = var_a1;
    g_AkaoPitchMulMusicSlideStep = new_var / var_a1;
}

void func_8002C7A8(void) { func_80029F44(); }

void func_8002C7C8(void) { func_8002A43C(); }

void func_8002C7E8(void) {
    D_8009A104 = 1;
    func_8002A748();
    func_8002A798();
}

void func_8002C81C(void) {
    D_8009A104 = 4;
    func_8002A748();
    func_8002A798();
}

static void Akao81SetMonoMode(void) {
    D_8009A104 = 2;
    func_8002A748();
    func_8002A798();
}

void func_8002C884(Unk8002B7E0* arg0) {
    u16 i;
    Unk80096608* p;

    D_80062FD8 = arg0->unk4;
    for (i = 0, p = D_800966E8; i < 0x18; i++, p++) {
        *(s32*)p |= 3;
    }
}

void func_8002C8C4(Unk8002B7E0* arg0) { D_8009A152 = arg0->unk4; }

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002C8DC);

// Retire the sound-effect voices queued in D_8009A118: mark each one for a
// full hardware refresh, hand the set to the pending mask and re-run the
// three key-on/key-off passes.
void func_8002C9E4(void) {
    s32 mask;
    Unk80096608* p;
    u32 bit;
    s32 queued;

    mask = D_8009A118;
    if (mask != 0) {
        bit = 1;
        p = D_800966E8;
        do {
            if (mask & bit) {
                *(s32*)p |= 0x2203;
                mask ^= bit;
            }
            bit <<= 1;
            p++;
        } while (mask != 0);
        queued = D_8009A118;
        D_8009A118 = 0;
        g_AkaoPendingMask = queued;
        func_8002FF4C();
        func_80030038();
        func_80030148();
    }
    D_80062FF8 &= ~1;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002CA84);

// As func_8002C9E4, for the music channels.
void func_8002CB78(void) {
    s32 mask;
    Unk80096608* p;
    u32 bit;
    s32 queued;

    mask = D_80099FDC;
    if (mask != 0) {
        bit = 0x10000;
        p = D_80099868;
        do {
            if (mask & bit) {
                *(s32*)p |= 0x2203;
                mask ^= bit;
            }
            bit <<= 1;
            p++;
        } while (mask != 0);
        queued = D_80099FDC;
        D_80099FDC = 0;
        g_AkaoChannelMask[0] = queued;
        func_8002FF4C();
        func_80030038();
        func_80030148();
    }
    D_80062FF8 &= ~2;
}

typedef struct {
    u32 unk0;
    u16 unk4;
} Unk8002CC18;

static void AkaoE0SetReverbPan(Unk8002CC18* arg0) {
    D_80062F70 = arg0->unk4 & 0x7F;
    D_8009A13C |= 0x80;
}

typedef struct {
    u32 unk0;
    u8 unk4;
} Unk8002CC44;

static void AkaoE4SetReverbMul(Unk8002CC44* arg0) {
    u8 temp_v0;
    s32 var_v0;
    s32 mask;

    temp_v0 = arg0->unk4;
    D_80062FB8 = (s16)temp_v0;
    mask = ~0x10;
    if (temp_v0 != 0) {
        var_v0 = D_80062FF8 | 0x10;
    } else {
        var_v0 = D_80062FF8 & mask;
    }
    D_80062FF8 = var_v0;
    func_80030038();
    D_8009A13C |= 0x80;
}

void func_8002CCBC(void) { D_8008337E = 0; }

void func_8002CCCC(void) { D_800833DE = 0; }

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002CCDC);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002CDD0);

static void AkaoF8StreamReverbMaskClear(void) {
    s32* addr;
    s32 temp_a0;
    s32 temp_v1;

    func_8002CFC0();
    addr = g_AkaoChannelMask;
    temp_a0 = g_AkaoReverbMask;
    temp_v1 = ~D_80062F00;
    *addr &= temp_v1;
    g_AkaoReverbMask = temp_v1 & temp_a0;
    func_80030038(temp_a0, addr);
}

static void AkaoF9StreamReverbMaskRestore(void) {
    s32 temp_a0;

    func_8002CFC0();
    temp_a0 = g_AkaoChannelMask[0];
    g_AkaoChannelMask[0] = ~D_80062F00 & temp_a0;
    g_AkaoReverbMask |= D_80062F00;
    func_80030038(temp_a0, g_AkaoChannelMask, D_80062F00);
}

static void func_8002CF78(void) { func_80029A50(); }

void func_8002CF98(Unk8002B7E0* arg0) {}

void func_8002CFA0() { SpuSetTransferCallback(0); }

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002CFC0);

void func_8002E23C(s32, void*);

// Configures the voice-attribute block for a mono CD-stream voice (ADSR
// envelope, pan, reverb-echo work area) and applies it via func_8002E23C.
static void AkaoStreamVoiceAttrMono(void) {
    D_8007EBE8 = 0x1FF93;
    D_8007EC02 = 0;
    D_8007EBEC = 0x77000;
    D_8007EBF0 = 0x77000;
    D_8007EC04 = 0xF;
    D_8007EC06 = 0xF;
    D_8007EC08 = 0x7F;
    D_8007EC0A = 6;
    D_8007EBF4 = 1;
    D_8007EBF8 = 3;
    D_8007EBFC = 3;
    D_8007EC0C = (D_80062FB0 ^ 0x7F) * D_80062FAC >> 7;
    D_8007EC00 = D_80062F1E;
    D_8007EC0E = D_80062FAC * D_80062FB0 >> 7;
    func_8002E23C(0x10, &D_8007EBE4);
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002D2D4);

void func_8002D530(void);

// CD-stream DMA transfer-complete callback (mono case). Keys on the stream
// voice(s) in D_80062F00; when D_80063004 (bytes remaining) is nonzero, first
// re-arms the SPU transfer IRQ with func_8002D530 to continue streaming.
static void AkaoStreamTransferCallbackMono(void) {
    SpuSetTransferCallback(0);
    if (D_80063004 != 0) {
        SpuSetIRQ(0);
        SpuSetIRQAddr(0x78000);
        SpuSetIRQCallback(&func_8002D530);
        SpuSetIRQ(1);
    }
    SpuSetKey(1, D_80062F00);
    D_80099FD8 &= ~D_80062F00;
}

void func_8002D7A0(void);

// CD-stream DMA transfer-complete callback (split/stereo case). Twin of
// AkaoStreamTransferCallbackMono above, using a different IRQ callback.
static void AkaoStreamTransferCallbackSplit(void) {
    SpuSetTransferCallback(0);
    if (D_80063004 != 0) {
        SpuSetIRQ(0);
        SpuSetIRQAddr(0x78000);
        SpuSetIRQCallback(&func_8002D7A0);
        SpuSetIRQ(1);
    }
    SpuSetKey(1, D_80062F00);
    D_80099FD8 &= ~D_80062F00;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002D530);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002D668);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002D7A0);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002D8E8);

void func_8002DA30(Unk8002B7E0** out_msg) {
    *out_msg = D_80081DC8;
    *out_msg = &D_80081DC8[D_80063010];
    D_80063010++;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", SystemAkaoExecute);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002DF88);

void func_8002E1A8(void) {
    Unk8002B7E0* msg;

    if (D_80062F8C == 0) {
        for (msg = D_80081DC8; D_80063010; D_80063010--, msg++) {
            D_80049548[msg->unk0](msg);
        }
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002E23C);

// Applies the current CD volume (g_AkaoCdVol) to the SPU's CD-input channel.
// Confirmed against qgears' independent reverse-engineering (system_psyq_spu_
// set_common_attr call, mask = SPU_COMMON_CDVOLL|CDVOLR|CDREV): D_8009C578's
// first field is a field-select mask, not a voice bitmask.
static void AkaoUpdateCdVolume(void) {
    D_8009C578.mask = 0x1C0;
    D_8009C578.unk14 = 0;
    D_8009C578.unk12 = D_80062FD6;
    D_8009C578.unk10 = D_80062FD6;
    SpuSetCommonAttr(&D_8009C578);
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002E478);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002E954);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002ED34);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002F24C);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002F738);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002F848);

// Fold the SPU voice bits owned by the tracks named in `mask` into `*out`,
// but only those that also appear in `filter`. A track is either playing on
// its overlay voice (0x100) or on the alternate voice it claimed (0x200);
// overlay voices past 24 wrap into the second bank.
void func_8002FDA0(AKAO_TRACK* track, s32* out, s32 mask, s32 filter) {
    u32 bit;
    u32 voicebit;
    u16 voice;
    s32 flags;

    bit = 1;
    *out |= mask;
    if (mask != 0) {
        do {
            if (mask & bit) {
                flags = track->update_flags;
                if (flags & 0x100) {
                    voice = track->overlay_voice;
                    if (track->overlay_voice >= 0x18) {
                        voice -= 0x18;
                    }
                    voicebit = 1 << voice;
                    if (filter & voicebit) {
                        *out |= voicebit;
                    }
                } else if (flags & 0x200) {
                    voicebit = 1 << track->alt_voice_id;
                    if (filter & voicebit) {
                        *out |= voicebit;
                    }
                }
                mask ^= bit;
            }
            bit <<= 1;
            track++;
        } while (mask != 0);
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002FE48);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_8002FF4C);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80030038);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80030148);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80030234);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80030380);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800308D4);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80030E7C);

// Load an instrument: copy its record out of the table into the track's
// shadow fields and flag every one of them dirty.
void func_80031820(AKAO_TRACK* track, s32 id) {
    AkaoInstrument* ins;

    track->instr_id = id;
    ins = &D_80075F28[(u16)id];
    track->unkE4 = ins->unk0;
    track->unkE8 = ins->unk4;
    track->unkEC = ins->unkD;
    track->unkF0 = ins->unkE;
    track->unkF4 = ins->unkF;
    track->unkFA = ins->unk8;
    track->unkFC = ins->unk9;
    track->unkFE = ins->unkA;
    track->unk100 = ins->unkB;
    track->unk102 = ins->unkC;
    track->attr_mask |= 0x1FF80;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800318BC);

u8 func_80031A70(u8** arg0) {
    u8 expected;
    u8 len;
    u8 opcode;
    u8* data;

    data = *arg0;
    expected = 0xCA;
    do {
        opcode = *data;
        len = D_800499A8[opcode];
        data += len;
    } while (len);
    return opcode == expected ? 0xCA : 0xA0;
}

// Tempo set: a 16-bit value landing in the top half of the 20.12 tempo
// accumulator, and the slide it cancels.
void func_80031AB0(AKAO_TRACK* track, AKAO_CONFIG* config) {
    config->tempo = *track->addr++ << 16;
    config->tempo |= *track->addr++ << 24;
    config->tempo_slide_length = 0;
}

// Tempo slide: keep the top half of the accumulator and ramp toward a new
// 16-bit value over the byte count that precedes it.
void func_80031AFC(AKAO_TRACK* track, AKAO_CONFIG* config) {
    s32 steps;
    s32 cur;
    s32 target;

    steps = *track->addr++;
    config->tempo_slide_length = steps;
    if (steps == 0) {
        config->tempo_slide_length = 0x100;
    }
    target = *track->addr++ << 16;
    target |= *track->addr++ << 24;
    cur = config->tempo & 0xFFFF0000;
    config->tempo = cur;
    config->tempo_slide_step = (target - cur) / config->tempo_slide_length;
}

void func_80031BA0(u8** cursor, AKAO_TRACK* track) {
    u8* p = *cursor;
    u8 v0;
    u8 v1;
    u32 combined;

    /* cc1-psx writes the cursor back after each byte, not once at the end --
       tested; a single trailing writeback regresses the gate. */
    *cursor = p + 1;
    v0 = p[0];
    *cursor = p + 2;
    v1 = p[1];
    combined = (u32)v0 << 0x10;
    combined |= (u32)v1 << 0x18;
    *(u16*)&track->unk50 =
        0; // only the first half of this 4-byte unknown field
    track->update_flags |= 0x80;
    track->pitch_mul_sound_slide_step = combined;
}

// As func_80031AFC, for the reverb depth accumulator.
void func_80031BE4(AKAO_TRACK* track, AKAO_CONFIG* config) {
    s32 steps;
    s32 cur;
    s32 target;

    steps = *track->addr++;
    config->reverb_depth_slide_length = steps;
    if (steps == 0) {
        config->reverb_depth_slide_length = 0x100;
    }
    target = *track->addr++ << 16;
    target |= *track->addr++ << 24;
    cur = config->reverb_depth & 0xFFFF0000;
    config->reverb_depth = cur;
    config->reverb_depth_slide_step =
        (target - cur) / config->reverb_depth_slide_length;
}

void func_80031C88(AKAO_TRACK* track) {
    track->vol_master = *track->addr++;
    track->attr_mask |= 3;
}

void func_80031CB0(AKAO_TRACK* track) {
    s32 val = (s8)*track->addr++;

    track->vol_slide_steps = 0;
    track->attr_mask |= 3;
    track->volume = val << 0x17;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80031CE0);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80031D6C);

void func_80031E98(AKAO_TRACK* track, AKAO_CONFIG* config) {
    u16 voice;
    s32 flags;

    voice = track->overlay_voice;
    if (D_80062F04 != 0) {
        voice -= 0x18;
    }
    flags = track->update_flags;
    if (flags & 0x100) {
        track->update_flags = flags & ~0x100;
        config->unk24 &= ~(1 << voice);
    }
}

void func_80031EEC(AKAO_TRACK* track) {
    u8 val = *track->addr++;

    track->vol_balance_slide_steps = 0;
    track->vol_balance = val << 8;
    if (track->update_flags & 0x100) {
        track->attr_mask |= 3;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80031F30);

void func_80031FC0(AKAO_TRACK* track) {
    track->vol_pan = *track->addr++ << 8;
    track->vol_pan_slide_steps = 0;
    track->attr_mask |= 3;
}

void func_80031FF0(AKAO_TRACK* track) {
    u8 ch;
    u16 var_a0;

    track->vol_pan_slide_steps = *track->addr++;
    if (track->vol_pan_slide_steps == 0) {
        track->vol_pan_slide_steps = 0x100;
    }
    ch = *track->addr++;
    track->vol_pan &= 0xFF00;
    var_a0 = track->vol_pan;
    track->vol_pan_slide_step =
        ((ch << 8) - var_a0) / (u16)track->vol_pan_slide_steps;
}

void func_80032078(AKAO_TRACK* track) { track->octave = *track->addr++; }

void func_80032094(AKAO_TRACK* track) {
    track->octave = (track->octave + 1) & 0xF;
}

void func_800320AC(AKAO_TRACK* track) {
    track->octave = (track->octave + 0xFFFF) & 0xF;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800320C4);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80032274);

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_800323CC);

void func_800324D8(AKAO_TRACK* track) { track->transpose = (s8)*track->addr++; }

void func_80032500(AKAO_TRACK* track) {
    track->transpose = (s8)*track->addr++ + track->transpose;
}

void func_8003252C(AKAO_TRACK* track) {
    s32 steps;

    steps = *track->addr++;
    track->pitch_slide_steps = steps;
    if (steps == 0) {
        track->pitch_slide_steps = 0x100;
    }
    track->key_add = (s8)*track->addr++;
}

void func_8003257C(AKAO_TRACK* track) {
    u8 val = *track->addr++;

    track->portamento_steps = (s16)val;
    if (val == 0) {
        track->portamento_steps = 0x100;
    }
    track->transpose_stored = 0;
    track->key_stored = 0;
    track->sfx_mask = 1;
}

void func_800325B8(AKAO_TRACK* track) { track->portamento_steps = 0; }

void func_800325C0(AKAO_TRACK* track) {
    track->fine_tuning = (s8)*track->addr++;
}

void func_800325E8(AKAO_TRACK* track) {
    track->fine_tuning = (s8)*track->addr++ + track->fine_tuning;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80032614);

void func_80032718(AKAO_TRACK* track) {
    u32 raw;
    s32 base;
    s32 depth;

    raw = *track->addr++ << 8;
    base = track->pitch_base;
    track->vibrato_depth = raw;
    depth = (raw & 0x7F00) >> 8;
    if (raw & 0x8000) {
        track->unk7C = (depth * base) >> 7;
    } else {
        track->unk7C = (depth * ((base * 16 - base) >> 8)) >> 7;
    }
}

void func_80032770(AKAO_TRACK* track) {
    s32 steps;
    s32 delta;

    steps = *track->addr++;
    if (steps == 0) {
        steps = 0x100;
    }
    delta = (*track->addr++ << 8) - (u16)track->vibrato_depth;
    track->vibrato_depth_slide_steps = steps;
    track->unk82 = delta / steps;
}

void func_800327E0(AKAO_TRACK* track) {
    track->vibrato_pitch = 0;
    track->update_flags &= ~1;
    track->attr_mask |= 0x10;
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80032804);

void func_800328D4(AKAO_TRACK* track) {
    track->tremolo_depth = *track->addr++ << 8;
}

static void AkaoDETremoloDepthSlideFromCurrent(AKAO_TRACK* track) {
    u16 rate;
    u8* addr;
    s32 delta;

    addr = track->addr;
    track->addr = addr + 1;
    rate = addr[0];
    if (rate == 0) {
        rate = 0x100;
    }
    track->addr = addr + 2;
    delta = ((addr[1] << 8) - *(u16*)&track->tremolo_depth) / rate;
    track->tremolo_depth_slide_steps = rate;
    track->tremolo_depth_slide_step = delta;
}

void func_80032968(AKAO_TRACK* track) {
    track->tremolo_vol = 0;
    track->update_flags &= ~2;
    track->attr_mask |= 3;
}

static void AkaoBCSetPanLfo(AKAO_TRACK* track) {
    u8* addr;
    u8* addr2;
    u8 rate;

    addr = track->addr;
    track->update_flags |= 4;
    track->addr = addr + 1;
    rate = *addr;
    track->pan_lfo_rate = rate;
    if (rate == 0) {
        track->pan_lfo_rate = 0x100;
    }
    addr2 = track->addr;
    track->addr = addr2 + 1;
    track->pan_lfo_type = *addr2;
    track->pan_lfo_wave = g_AkaoWaveTableKey[*(u16*)&track->pan_lfo_type];
    track->pan_lfo_rate_cur = 1;
}

void func_80032A04(AKAO_TRACK* track) {
    track->pan_lfo_depth = *track->addr++ << 7;
}

static void AkaoDFPanLfoDepthSlideFromCurrent(AKAO_TRACK* track) {
    u8* addr;
    s32 rate;
    s32 delta;

    addr = track->addr;
    track->addr = addr + 1;
    rate = *addr;
    if (rate == 0) {
        rate = 0x100;
    }
    track->addr = addr + 2;
    delta = ((addr[1] << 7) - *(u16*)&track->pan_lfo_depth) / rate;
    track->pan_lfo_depth_slide_steps = rate;
    track->pan_lfo_depth_slide_step = delta;
}

void func_80032A98(AKAO_TRACK* track) {
    track->pan_lfo_vol = 0;
    track->update_flags &= ~4;
    track->attr_mask |= 3;
}

void func_8002FF4C();

static void AkaoC4NoiseOn(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    if (track->type == 0) {
        config->noise_mask = mask | config->noise_mask;
    } else {
        g_AkaoNoiseMask |= mask;
    }
    D_8009A13C |= 0x10;
    func_8002FF4C();
}

static void AkaoC5NoiseOff(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    if (track->type == 0) {
        config->noise_mask &= ~mask;
    } else {
        g_AkaoNoiseMask &= ~mask;
    }
    D_8009A13C |= 0x10;
    func_8002FF4C();
    track->noise_switch_delay = 0;
}

void func_80030148();

static void AkaoC6PitchLfoOn(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    if (track->type == 0) {
        config->pitch_lfo_mask = mask | config->pitch_lfo_mask;
    } else if (!(mask & 0x555555)) {
        g_AkaoPitchLfoMask |= mask;
    }
    func_80030148();
}

static void AkaoC7PitchLfoOff(
    AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    if (track->type == 0) {
        config->pitch_lfo_mask &= ~mask;
    } else {
        g_AkaoPitchLfoMask &= ~mask;
    }
    func_80030148();
    track->pitch_lfo_switch_delay = 0;
}

void func_80030038();

static void AkaoC2ReverbOn(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    if (track->type == 0) {
        config->reverb_mask = mask | config->reverb_mask;
    } else {
        g_AkaoReverbMask |= mask;
    }
    func_80030038();
}

static void AkaoC3ReverbOff(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    if (track->type == 0) {
        config->reverb_mask = ~mask & config->reverb_mask;
    } else {
        g_AkaoReverbMask &= ~mask;
    }
    func_80030038();
}

void func_80032D44(AKAO_TRACK* track) { track->sfx_mask = 1; }

void func_80032D50(void) {}

void func_80032D58(AKAO_TRACK* track) { track->sfx_mask = 4; }

void func_80032D64(void) {}

// Noise clock: bits 6-7 set means the low 6 bits are a signed-ish delta on
// the current clock, otherwise they replace it. Sound effects keep their
// own copy in D_80099FFA rather than the song's config.
void func_80032D6C(AKAO_TRACK* track, AKAO_CONFIG* config) {
    u8 val;

    val = *track->addr++;
    if (track->type == 0) {
        if (val & 0xC0) {
            config->noise_clock = (config->noise_clock + (val & 0x3F)) & 0x3F;
        } else {
            config->noise_clock = val;
        }
    } else {
        if (val & 0xC0) {
            D_80099FFA = (D_80099FFA + (val & 0x3F)) & 0x3F;
        } else {
            D_80099FFA = val;
        }
    }
    D_8009A13C |= 0x10;
}

// The eight LFO-parameter opcodes are one shape: read a byte, flag the change
// in attr_mask, store it in the track, and -- when the track is already live
// (update_flags & 0x100) -- mirror it into the hardware-shadow record for this
// track's voice. The mirror's address is written as a byte offset off
// D_80096608 so the symbol stays in the mem and the assembler rebuilds it
// through the $at macro, which is what the target does.
void func_80032E08(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unkFA = *track->addr++;
    track->attr_mask |= 0x900;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u16*)((u8*)D_80096608 + off + 0xFA) = track->unkFA;
    }
}

void func_80032E6C(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unkFC = *track->addr++;
    track->attr_mask |= 0x1000;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u16*)((u8*)D_80096608 + off + 0xFC) = track->unkFC;
    }
}

void func_80032ED0(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unkFE = *track->addr++;
    track->attr_mask |= 0x8000;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u16*)((u8*)D_80096608 + off + 0xFE) = track->unkFE;
    }
}

void func_80032F34(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unk100 = *track->addr++;
    track->attr_mask |= 0x2200;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u16*)((u8*)D_80096608 + off + 0x100) = track->unk100;
    }
}

void func_80032F98(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unk102 = *track->addr++;
    track->attr_mask |= 0x4400;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u16*)((u8*)D_80096608 + off + 0x102) = track->unk102;
    }
}

void func_80032FFC(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unkEC = *track->addr++;
    track->attr_mask |= 0x100;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u32*)((u8*)D_80096608 + off + 0xEC) = track->unkEC;
    }
}

void func_80033060(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unkF0 = *track->addr++;
    track->attr_mask |= 0x200;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u32*)((u8*)D_80096608 + off + 0xF0) = track->unkF0;
    }
}

void func_800330C4(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s32 off;

    track->unkF4 = *track->addr++;
    track->attr_mask |= 0x400;
    if (track->update_flags & 0x100) {
        off = track->overlay_voice * 0x108;
        *(u32*)((u8*)D_80096608 + off + 0xF4) = track->unkF4;
    }
}

// Claim the lowest free voice slot for this track: walk the union of the
// three in-use masks until a clear bit is found, and take it if it is still
// inside the 24-voice range.
void func_80033128(AKAO_TRACK* track, AKAO_CONFIG* config) {
    u32 busy;
    u32 bit;
    u16 slot;

    track->unk102 = *track->addr++;
    if ((track->update_flags & 0x200) == 0) {
        slot = 0;
        bit = 1;
        busy = config->unk4 | config->unk24 | config->unk28;
        for (;;) {
            if ((busy & bit) == 0) {
                goto found;
            }
            bit <<= 1;
            slot++;
            if ((bit & 0xFFFFFF) == 0) {
                goto found;
            }
        }
    found:;
        if (bit & 0xFFFFFF) {
            config->unk28 |= bit;
            track->alt_voice_id = slot;
            track->update_flags |= 0x200;
        }
    }
}

void func_800331CC(AKAO_TRACK* track, AKAO_CONFIG* config) {
    s32 off;

    config->unk28 &= ~(1 << track->alt_voice_id);
    off = track->instr_id * 0x40;
    track->update_flags &= ~0x200;
    track->unk102 = *((u8*)D_80075F34 + off);
    track->attr_mask |= 0x4400;
}

// Loop start: open the next of four nested loop slots.
void func_80033224(AKAO_TRACK* track) {
    track->loop_id = (track->loop_id + 1) & 3;
    track->loop_addr[track->loop_id] = track->addr;
    track->loop_times[track->loop_id] = 0;
}

// Loop end with a repeat count: go round again until the count is reached,
// then close the slot.
void func_80033264(AKAO_TRACK* track) {
    s32 count;

    count = *track->addr++;
    if (count == 0) {
        count = 0x100;
    }
    if (++track->loop_times[track->loop_id] != count) {
        track->addr = track->loop_addr[track->loop_id];
    } else {
        track->loop_id = (track->loop_id - 1) & 3;
    }
}

// Branch out of the loop on the last-but-one pass (the count is tested
// without being advanced), otherwise skip the 16-bit displacement.
void func_800332EC(AKAO_TRACK* track) {
    s32 count;

    count = *track->addr++;
    if (count == 0) {
        count = 0x100;
    }
    if (track->loop_times[track->loop_id] + 1 != count) {
        track->addr += 2;
    } else {
        track->addr += READ_S16(track->addr);
    }
}

// As func_800332EC, but the branch also closes the loop slot.
void func_8003337C(AKAO_TRACK* track) {
    s32 count;

    count = *track->addr++;
    if (count == 0) {
        count = 0x100;
    }
    if (track->loop_times[track->loop_id] + 1 != count) {
        track->addr += 2;
    } else {
        track->addr += READ_S16(track->addr);
        track->loop_id = (track->loop_id - 1) & 3;
    }
}

// Loop end with no count: always go round again.
void func_80033420(AKAO_TRACK* track) {
    track->loop_times[track->loop_id]++;
    track->addr = track->loop_addr[track->loop_id];
}

void func_8003345C(AKAO_TRACK* track) {
    u16 val = *track->addr++;

    track->length_fixed = 0;
    /* sets length_1 and length_2 to the same byte in one halfword store;
       writing them as two separate field assignments regresses the gate. */
    *(s16*)&track->length_1 = (val << 8) | val;
    track->length_stored = val;
}

void func_80033488(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    short delta = READ_S8(track->addr);
    if (delta != 0) {
        delta += track->length_stored;
        if (delta < 1) {
            delta = 1;
        } else if (delta > 255) {
            delta = 255;
        }
    }
    track->length_fixed = delta;
}

void func_800334EC(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->drum_addr = track->addr + READ_S16(track->addr);
    track->update_flags |= 0x8;
}

void func_80033534(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->update_flags &= ~0x8;
}

void func_80033548(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    config->ticks_per_beat = *track->addr++;
    config->beats_per_measure = *track->addr++;
    config->tick = 0;
    config->beat = 0;
}

void func_80033588(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    config->measure = *track->addr++;
    config->measure |= *track->addr++ << 8;
}

void func_800335CC(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    config->field_54 = 1;
}

void func_800335D8(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    func_80032E6C(track, config, mask);
    func_80032ED0(track, config, mask);
}

void func_80033628(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    int delay = *track->addr++;
    if (delay == 0) {
        track->noise_switch_delay = 257;
    } else {
        track->noise_switch_delay = delay + 1;
    }
    AkaoC4NoiseOn(track, config, mask);
}

void func_8003366C(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s16 var_v0 = *track->addr++;
    if (var_v0 == 0) {
        var_v0 = 257;
    } else {
        var_v0++;
    }
    track->noise_switch_delay = var_v0;
}

void func_80033698(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    int delay = *track->addr++;
    if (delay == 0) {
        track->pitch_lfo_switch_delay = 257;
    } else {
        track->pitch_lfo_switch_delay = delay + 1;
    }
    AkaoC6PitchLfoOn(track, config, mask);
}

void func_800336DC(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    s16 var_v0 = *track->addr++;
    if (var_v0 == 0) {
        var_v0 = 257;
    } else {
        var_v0++;
    }
    track->pitch_lfo_switch_delay = var_v0;
}

void func_80033708(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->update_flags &= ~0x37;
    AkaoC5NoiseOff(track, config, mask);
    AkaoC7PitchLfoOff(track, config, mask);
    AkaoC3ReverbOff(track, config, mask);
    track->sfx_mask &= ~0x5;
}

void func_80033788(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->update_flags |= 0x10;
}

void func_8003379C(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->update_flags &= ~0x10;
}

void func_800337B0(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->update_flags |= 0x20;
}

void func_800337C4(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->update_flags &= ~0x20;
}

void func_800337D8(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    track->addr += READ_S16(track->addr);
}

void func_80033818(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    int cond = *track->addr++;
    if (config->condition != 0 && cond <= config->condition) {
        track->addr += READ_S16(track->addr);
        config->last_condition = cond;
    } else {
        track->addr += 2;
    }
}

INCLUDE_ASM("asm/us/main/nonmatchings/akao", func_80033894);

void func_80033A70(AKAO_TRACK* track, AKAO_CONFIG* config, u32 mask) {
    func_80033894(track, config, mask);
}

void func_80033A90(void) {
    s32 i;
    u8* bank;

    for (i = 1279, bank = &Savemap.memory_bank_5[255]; i >= 0; i--, bank--) {
        *bank = 0;
    }

    for (i = 0; i < 3; i++) {
        Savemap.partyID[i] = 0xFF;
        Savemap.memory_bank_2[i + 9] = 0xFF;
    }

    Savemap.phs_visibility_mask = 1; // Only Cloud is visible.
    g_FieldMusicLock = 0;
    g_FieldMovieLock = 0;
    g_FieldBattleLock = 0;
    Savemap.partyID[0] = 0;
    Savemap.memory_bank_2[9] = 0;
    Savemap.memory_bank_4[0x68] = 0xFF; // Start of location name.
    Savemap.memory_bank_1[0x1C] = 0xFF; // Menu visibility, 2 bytes.
    Savemap.memory_bank_1[0x1D] = 0xFF;
    Savemap.time = 0;
    Savemap.countdown_timer_seconds = 0;
    D_8009ABF4.nFadeRedStart = 0;
    D_8009ABF4.nFadeGreenStart = 0;
    D_8009ABF4.nFadeBlueStart = 0;
    D_8009ABF4.movieCamDisabled = 0;
    g_PartyUpdatedByFieldScript = 0;
}
