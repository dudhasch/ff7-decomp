---
title: 22_BTMD2
---

- Opcode: **0x22**
- Short name: **BTMD2**
- Handler: `OpcodeFuncBtmd2`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 2 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x22 | *arg1* | *arg2* | *arg3* | *arg4* |
|------|--------|--------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: literal at byte 0x03.
- **const Byte** *arg4*: literal at byte 0x04.

