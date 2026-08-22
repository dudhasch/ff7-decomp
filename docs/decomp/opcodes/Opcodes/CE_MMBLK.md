---
title: CE_MMBLK
---

- Opcode: **0xCE**
- Short name: **MMBLK**
- Handler: `OpcodeFuncMmblk`
- Status: verified -- compiled from C in a green build
- Length: **2** bytes (`PC_INC(2)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xCE | *arg1* |
|------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

