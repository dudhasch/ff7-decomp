---
title: C8_PRTYP
---

- Opcode: **0xC8**
- Short name: **PRTYP**
- Handler: `OpcodeFuncPrtyp`
- Status: verified -- compiled from C in a green build
- Length: **2** bytes (`PC_INC(2)`)
- The in-game debugger prints 1 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xC8 | *arg1* |
|------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

