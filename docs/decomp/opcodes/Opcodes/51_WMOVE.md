---
title: 51_WMOVE
---

- Opcode: **0x51**
- Short name: **WMOVE**
- Handler: `OpcodeFuncWmove`
- Status: verified -- compiled from C in a green build
- Length: **6** bytes (`PC_INC(6)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x51 | *arg1* | *arg2* | *arg4* |
|------|--------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Short** *arg2*: literal at byte 0x02.
- **const Short** *arg4*: literal at byte 0x04.

