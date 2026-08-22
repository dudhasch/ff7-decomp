---
title: C9_PRTYM
---

- Opcode: **0xC9**
- Short name: **PRTYM**
- Handler: `OpcodeFuncPrtym`
- Status: verified -- compiled from C in a green build
- Length: **2** bytes (`PC_INC(2)`)
- The in-game debugger prints 1 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xC9 | *arg1* |
|------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

