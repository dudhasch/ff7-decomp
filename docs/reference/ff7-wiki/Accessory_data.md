(content fetched from https://raw.githubusercontent.com/ff7-mods/ff7-flat-wiki/master/docs/reference/ff7-wiki/Accessory_data.md)

## KERNEL.BIN - Section 8: Accessory data

This section contains the accessory data. Each record is 16 bytes long.

| Offset | Length | Description |  |
|:--:|----|----|----|
| 0x00 | 2 bytes | Stat
Bonus |  |
|   |  | 0xFF | None |
|  |  | 0x00 | Strength |
|  |  | 0x01 | Vitality
|
|  |  | 0x02 | Magic |
|  |  | 0x03 | Spirit |
|  |  | 0x04 | Dexterity |
| 
|  | 0x05 | Luck |
| 0x02 | 2 bytes | Bonus Amount |  |
| 0x04 | 1 byte |
Elemental Strength |  |
|   |  | 0x00 | Absorb |
|  |  | 0x01 | Nullify |
|  |  | 0x02
... (truncated)