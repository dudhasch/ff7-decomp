(content fetched from https://raw.githubusercontent.com/ff7-mods/ff7-flat-wiki/master/docs/reference/ff7-wiki/Armor_data.md)

## KERNEL.BIN - Section 7: Armor data format

This
section contains the armor data. Each record is 36 bytes long.

| Offset | Length
| Description |  |
|:--:|----|----|----|
| 0x00 | 1 byte | Unknown |  |
| 0x01 |
1 byte | Damage Type, Based off values of Elemental Type |  |
|   |  | 0xFF |
Normal |
|  |  | 0x00 | Absorb |
|  |  | 0x01 | Nullify |
|  |  | 0x02 | Halve
|
| 0x02 | 1 byte | Defense |  |
| 0x03 | 1 byte | Magic Defense |  |
| 0x04 | 1 byte | Defense % |  |
| 0x05 | 1 byte | Magic Defense % |  |
| 0x06 | 1 byte |
... (truncated)