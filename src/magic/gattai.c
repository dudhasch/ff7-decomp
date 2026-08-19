//! PSYQ=3.3 CC1=2.6.3

// MAGIC/GATTAI.BIN -- "gattai" (合体, "merge"). Not one of the 54 Materia
// spells: this overlay only flags the two party members other than the
// caster and starts a 30-frame countdown, which is the screen setup an
// ability plays before taking the camera. Which ability that is has not been
// established -- the overlay-to-ability map needs the CD directory, not the
// file name.

#include "common.h"
#include "../battle/battle.h"

// Battle effect instance, as this overlay lays it out: it only needs the
// teardown flag and a frame countdown.
typedef struct GattaiData {
    s16 StartFrame;
    s16 unk2;
    s16 Timer;
    char pad6[0x1A];
} GattaiData;

// The battle's other 0x20-byte job array, the one func_800BC04C allocates from.
typedef struct GattaiJob {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 PartyId;
    char pad8[0x18];
} GattaiJob;

// Battle effect instances, and the job slots func_800BC04C hands out.
extern GattaiData D_80162978[];
extern GattaiJob D_801621F0[];
extern u8 D_800F8374;

static void GattaiCountdown(void) {
    if (D_80162978[D_8015169C].Timer == 0) {
        D_80162978[D_8015169C].StartFrame = -1;
    }

    D_80162978[D_8015169C].Timer--;
}

int MAGIC_Gattai(int arg0, int arg1) {
    s32 job;

    D_80162978[func_800BBEAC(GattaiCountdown)].Timer = 30;

    job = func_800BC04C(func_800C55B8);
    D_801621F0[job].unk4 = 0x10;
    D_801621F0[job].PartyId = arg1;
    D_801621F0[job].unk2 = 0x100;

    D_800F8374 = 0xE;
    func_800BBA40(0x29);

    if (arg1 == 0) {
        D_801518E4[1].D_80151922 |= 2;
        D_801518E4[2].D_80151922 |= 2;
    }

    if (arg1 == 1) {
        D_801518E4[0].D_80151922 |= 2;
        D_801518E4[2].D_80151922 |= 2;
    }

    if (arg1 == 2) {
        D_801518E4[0].D_80151922 |= 2;
        D_801518E4[1].D_80151922 |= 2;
    }

    return 0;
}
