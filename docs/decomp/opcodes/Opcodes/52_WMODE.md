---
title: 52_WMODE
---

- Opcode: **0x52**
- Short name: **WMODE**
- Handler: `OpcodeFuncWmode`
- Status: verified -- compiled from C in a green build
- Length: **4** bytes (`PC_INC(4)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x52 | *arg1* | *arg2* | *arg3* |
|------|--------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: literal at byte 0x03.

