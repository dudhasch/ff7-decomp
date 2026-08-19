# Introduction

AKAO frames are most complicated
frames in FF7 sound system. ("AKAO" is frame magic, probably developed by Minoru
Akao, Square Enix sound programmer :) )

Frame is similar to MIDI sequence - it's
custom tracker format for playing sequence sound, well tuned specially for
PSX.

This frames are in all FF7 game modules: Field, Battle, Worldmap and in
minigames.

All files with exension *.SND are AKAO.

**MINI/ASERI2.SND** - Battle Arena
theme

**MINI/SENSUI.SND** - used in Submarine minigame

**ENEMY6/OVER2.SND** -
game over sequence

**ENEMY6/FAN2.SND** - battle win "fanfare"
sequence

**MOVIE/OVER2.SND** - same game over sequence, don't know, why to duplicate
data

Other AKAO frames are hard-wired in other files.

# AKAO frame structure

## Header
(size: 16 bytes)

struct
AkaoHeader

{

`   static const uint8_t magic[4]; // "AKAO" C-string aka frame *MAGIC*` 

`   uint16_t id;           // frame ID, used for playing sequence` 

`   uint16_t length;       // frame length - sizeof(header)` 

`   uint8_t unknown[8];    // some numbers, can't find their usage`

};

## Channel info (size: 4 bytes + 2 bytes * <channels
count>)

First there is 32-bit number (offset 0x10), which represents bitmask of
used channels in this frame, after this frame there is <channels count> offsets
to channel opcode data counting from current offset.

... (truncated for brevity)
