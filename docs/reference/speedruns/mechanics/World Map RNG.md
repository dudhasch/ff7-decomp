The World Map module use its own implementation of a [Random Number Generator](/index.php/Random_Number_Generator "Random Number Generator") to generate unpredictable values. This random number generator is used to generate random encounters on the world map, as well as to control the behavior of the Midgar Zolom and the spawning position of Emerald Weapon, possibly among other things.

World Map RNG is seeded by the current [In-Game Time](/index.php?title=In-Game_Time&action=edit&redlink=1 "In-Game Time \(page does not exist\)") when the world map module is loaded.

## Implementation

World Map RNG consists of a 521-byte table generated when the world map module loads, and an index into this list.

    byte WorldRandBuffer[521];
    uint WorldRandIndex;

    void WorldSeedRand(uint seed) {
      int i, j;
      uint randBuffer [521];
      uint x;

      x = 0;
      for (i = 0; i < 0x11; i = i + 1) {
        for (j = 0; j < 0x20; j = j + 1) {
          seed = seed * 0x5d588b65 + 1;
          x = x >> 1 | seed & 0x80000000;
        }
        randBuffer[i] = x;
      }

      randBuffer[0x10] = randBuffer[0x10] << 0x17 ^ randBuffer[0] >> 9 ^ randBuffer[0xF];

      for (i = 0x11; i < 0x209; i = i + 1) {
        randBuffer[i] = randBuffer[i - 0x11] << 0x17 ^ randBuffer[i - 0x10] >> 9 ^ randBuffer[i - 1];
      }

      for (i = 0; i < 0x209; i = i + 1) {
        WorldRandBuffer[i] = (byte) randBuffer[i];
      }

      WorldScrambleRand();
      WorldScrambleRand();
      WorldScrambleRand();

      WorldRandIndex = 0x208;
      return;
    }

    void WorldScrambleRand() {
      int i;

      for (i = 0; i < 0x20; i = i + 1) {
        WorldRandBuffer[i] = WorldRandBuffer[i] ^ WorldRandBuffer[i + 0x1e9];
      }

      for (i = 0x20; i < 0x209; i = i + 1) {
        WorldRandBuffer[i] = WorldRandBuffer[i] ^ WorldRandBuffer[i - 0x20];
      }

      return;
    }

    byte WorldRand() {
      WorldRandIndex = WorldRandIndex + 1;

      if (0x208 < WorldRandIndex) {
        WorldScrambleRand();
        WorldRandIndex = 0;
      }

      return WorldRandBuffer[WorldRandIndex];
    }

## Uses

On every frame that any of (Left, Right, L1, R1) is pressed while Cloud is able to move, World Map RNG is called and its return value is ignored. This is most likely implemented as a mechanism to scrmable World Map RNG, but it can be abused to control world map manipulations.

World Map RNG is used in determining World Map Random Encounters.

World Map RNG is used by the Midgar Zolom to determine its random movement. This happens periodically while the player is within the Zolom Box, and is dependent on whether the player is standing on a Swamp triangle (id 7).

World Map RNG is used by Emerald Weapon to determine its spawn location out of 9 possibilities. This happens as the first RNG call made when the underwater map loads if Emerald Weapon exists in the world.

World Map RNG is used by Ruby Weapon to determine its spawn location out of 3 possibilities. This happens as the first RNG call made when the main map loads if Ruby Weapon exists in the world.