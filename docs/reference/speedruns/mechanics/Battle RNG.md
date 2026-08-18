The Battle Engine use its own implementation of a [Random Number Generator](/index.php/Random_Number_Generator "Random Number Generator") to generate unpredictable values. This random number generator is used for most, but not all, apparently random elements in battles.

## Implementation

### Battle RNG Table

Battle RNG consists of a lookup table consisting of the following 256 bytes:

    63 06 F0 23 F8 E5 A8 01 C1 AE 7F 48 7B B1 DC 09
    22 6D 7D EE 9D 58 D5 55 24 39 7A DF 8E 54 6C 1B
    C0 0B D0 43 D8 9A 47 5D 21 02 17 4B DB 11 AF 70
    CD 4D 34 49 72 91 2D 62 97 59 45 F7 6E 46 AA 0A
    A3 C8 31 92 38 FA D4 E6 CB F3 DE 6B BB F1 1C 3C
    D6 AD B2 A9 DD 57 42 95 0C 79 25 1F BC E7 AC 5B
    83 28 76 F2 18 DA 87 A1 61 6F BE 5A 5E 51 EF B0
    C9 15 74 89 BD D1 A2 75 D7 99 85 4C 4F D2 BF 4A
    20 08 56 A0 50 3A 67 26 41 33 B7 BA FB 30 CF 7C
    84 2C 32 E9 1D 16 82 78 A4 80 65 5F 0E 27 B9 19
    C3 A7 B6 00 3B FC 88 E1 C6 93 FE 8B D9 B8 13 69
    2F 64 12 37 FD 77 E2 B5 04 E0 1A 8C 8F B4 CC F9
    60 EB 29 E3 90 A5 68 3D 81 73 3F AB 7E B3 0F CE
    C4 35 94 96 86 71 D3 2A E4 9F 9C EC 4E 14 F5 EA
    40 A6 F6 03 98 C5 07 F4 2B C2 3E E8 9B 36 53 2E
    8D 0D 52 10 66 1E ED 8A 44 9E 05 FF 5C C7 6A CA

This list does not appear to be directly hardcoded but it is not yet known how it is generated.

### State

The current Battle RNG state consists of:

  * Eight 1-byte indexes that refer to a position in the RNG table
  * One 1-byte index that refers to one of the eight above indexes
  * One 4-byte value that influences how Rand16 calls behave

#### Index Array

The Battle RNG Index Array consists of eight 1-byte indexes that each refers to a position in the Battle RNG Table. One of these indexes is selected to be the Active Index by another index. The Active Index is usually incremented when Rand16 is called.

The Index Array is seeded when a battle starts, and the Active Index is always initialized to 0.

#### Counter

There is also another value called the Battle RNG Counter that increments on every Rand16 call and influences its behavior. This value is initialized to 0 when the game starts, but it is never reseeded or reinitialized afterwards.

## Seeding

Battle RNG is seeded by the return value of a single call to [System RNG](/index.php/System_RNG "System RNG") which is used to populate the Index Array as follows:

    void BattleSRand(int seed)

    {
      int i;

      for (i = 0; i < 8; i = i + 1) {
        BattleRandArr[i] = (byte)seed;
        seed = seed >> 1;
      }
      BattleRandIdx = 0;
      return;
    }

### Seeding Differences between PSX and PC

Both PC/HD and PSX use a 15-bit value to seed Battle RNG, but this value comes from different sources between the versions.

On PC/HD, a call to System RNG is made for the value that will be used. This leads to the possibility of [System Clock Manipulation](/index.php?title=System_Clock_Manipulation&action=edit&redlink=1 "System Clock Manipulation \(page does not exist\)").

On PSX, the lowest 15 bits of the global frame counter are instead used.

## Interfaces

There are several interfaces used to call Battle RNG or otherwise influence its state:

### Primary Interfaces

#### BattleRand8

    byte BattleRand8(void)

    {
      byte out;

      out = BattleRandTable[BattleRandArr[BattleRandIdx]];
      BattleRandArr[BattleRandIdx] = BattleRandArr[BattleRandIdx] + 1;
      return out;
    }

#### BattleRandIdxIncrement

    void BattleRandIdxIncrement(void)

    {
      BattleRandIdx = BattleRandIdx + 1 & 7;
      return;
    }

#### BattleRand16

    ushort BattleRand16(void)

    {
      byte lo;
      byte hi;
      uint joker;

      lo = BattleRand8();
      joker = BattleRandJoker & 7;
      BattleRandJoker = BattleRandJoker + 1;
      if (joker != 0) {
        BattleRandIdxIncrement();
      }
      hi = BattleRand8();
      return (hi << 8) | lo;
    }