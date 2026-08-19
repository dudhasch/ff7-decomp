//! PSYQ=3.3 CC1=2.6.3
#include "field_private.h"

/* Unit 3 of 5, split out of field.c. .rodata 0x800A00E0-0x800A0104, base 0
 * mod 8. Exists only to give jtbl_800A00E0 -- which FieldModelLoadBcx.s
 * precedes with `.align 3`
 * -- a base it is congruent to. */

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelLoadBcx);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelPrepareRender);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelAddToRender);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelAnimCalcMtrxs);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelScaleModel);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelScalePartVrtxs);

INCLUDE_ASM("asm/us/field/nonmatchings/field3", FieldModelScaleAnimTranslat);

/////////////////////////////////////////////////
// Begin of field_kawai_char_model.c
/////////////////////////////////////////////////

void KawaiClearData(void) {
    u8* p = D_800DFDFC;
    s32 count = 16;
    s32 i;

    p[0] = count;
    for (i = 0; i < count; i++) {
        p[i * 2 + 2] = 0;
        p[i * 2 + 3] = 0;
    }
}
