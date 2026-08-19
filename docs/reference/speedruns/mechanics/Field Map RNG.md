Field Maps use their own implementation of a [Random Number Generator](/index.php/Random_Number_Generator "Random Number Generator") to generate unpredictable values. This random number generator is used in field map scripts, but it is also somewhat used to generate field map random encounters.

## Implementation

### Field RNG Table

Field Map RNG consists of a lookup table into the following hardcoded list of 256 bytes:

    B1 CA EE 6C 5A 71 2E 55 D6 00 CC 99 90 6B 7D EB
    4F A0 07 AC DF 8A 56 9E F1 9A 63 75 11 91 A3 B8
    94 73 F7 54 D9 6E 72 C0 F4 80 DE B9 BB 8D 66 26
    D0 36 E1 E9 70 DC CD 2F 4A 67 5D D2 60 B5 9D 7F
    45 37 50 44 78 04 19 2C EF FD 64 81 03 DA 95 4C
    7A 0B AD 1F BA DD 3E F9 D7 1A 29 F8 18 B3 20 F6
    D1 5E 34 92 7B 24 43 88 97 D4 0F 35 AA 83 68 27
    A8 D5 BE FA 14 31 AF 10 0D D8 6A CE 23 61 F3 3D
    A4 08 33 E3 A9 38 E6 93 1D 1C F0 0E 87 59 65 82
    BC FF FE 7E 8F C1 1E F5 CB 49 02 32 09 C4 8E C6
    2B 40 A7 17 76 3B 16 2A C8 FB B2 58 A5 15 AE 25
    CF 46 C7 48 B4 0A 3F C9 06 85 51 89 62 4D 12 8C
    EA A2 98 4B 79 6F 5C 47 30 1B E7 C5 22 9C E8 96
    3A E4 7C E0 69 A1 B7 05 39 74 01 9F BD C3 84 FC
    77 86 13 4E BF F2 53 5B ED 21 8B 6D C2 41 B6 DB
    3C D3 28 EC 2D E2 9B A6 42 52 57 5F E5 AB B0 0C

Each value from 0 to 255 appears exactly once in this list.

There are two values that determine the state of Field Map RNG: Index and Increment. These are colloquially known as List and Stone.

### Index

Index, also known as List, RNGLi, or RNG List Index, is a byte value that is an index into the Field RNG Table. It can take any value from 0 to 255.

This value is updated every time Field RNG is called.

### Increment

Increment, usually known as Stone, is a byte value that determines by how much Index is incremented each time RNG is called.

It can take on any value in the form **16k+1** for integer k from 0 to 15 inclusive, which are the following values:

k value  | Increment
---|---
0  | 1
1  | 17
2  | 33
3  | 49
4  | 65
5  | 81
6  | 97
7  | 113
8  | 129
9  | 145
10  | 161
11  | 177
12  | 193
13  | 209
14  | 225
15  | 241

This value is updated in field map scripts, but it is only updated at three points in the game:

  * When beginning the Squats game, Increment is set to 33 (**k = 2**)*
  * In the Shinra Elevator, it is set every time the button is pressed immediately before generating a floor to land on
  * In Bone Village, it updates to (**k = IGT second mod 16**) when starting to dig**.

*In Squats, k is actually 50, but k mod 16 is used which is equal to 2

**Bone Village specifically uses the seconds position of IGT, not the total number of seconds. It can be expressed in terms of the total seconds as ((s mod 60) mod 16).

### Code

    byte index, increment;

    int rand(){
        index = index + increment;
        return buffer[index];
    }

## Other references to the Field RNG Table

The Field RNG Table is also used to determine [field map random encounters](/index.php/Field_map_encounter_mechanics "Field map encounter mechanics")

There also appears to be another use for this table in another separate RNG that appears to determine when field models blink their eyes.