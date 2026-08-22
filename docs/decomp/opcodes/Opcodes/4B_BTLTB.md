---
title: 4B_BTLTB
---

- Opcode: **0x4B**
- Short name: **BTLTB**
- Handler: `OpcodeFuncBtltb`
- Status: verified -- compiled from C in a green build
- Length: **2** bytes (`PC_INC(2)`)
- The in-game debugger prints 1 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x4B | *arg1* |
|------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

