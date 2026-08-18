System RNG refers to the [Random Number Generator](/index.php/Random_Number_Generator "Random Number Generator") provided to the game from an external source. These are analogous to each other, but have different implementations and subtle differences in use between platforms.

All implementations are a form of [Linear Congruential Generator](https://en.wikipedia.org/wiki/Linear_congruential_generator "wikipedia:Linear congruential generator") that returns an integer value uniformly distributed in the range [0, 32767], but they have different constants.

## Implementations

### PSX

PSX System RNG is provided by the PlayStation's BIOS. It is a Linear Congruential Generator with a multiplier of 0x41C64E6D (1103515245), an addend of 0x3039 (12345), an implicit modulus of 4294967296 (2^32), and returns bits 30 through 16.

### PC

PC System RNG is provided to the PC version of Final Fantasy VII by the compiler used to build the game. It is a Linear Congruential Generator with a multiplier of 0x343FD (214013), an addend of 0x269EC3 (2531011), an implicit modulus of 4294967296 (2^32), and returns bits 30 through 16.

## Uses

### Seeding Battle RNG

On the PC version, [Battle RNG](/index.php/Battle_RNG "Battle RNG") is seeded using the result of a call to System RNG.