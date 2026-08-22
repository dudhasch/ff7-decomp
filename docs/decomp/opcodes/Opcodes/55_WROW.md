---
title: 55_WROW
---

- Opcode: **0x55**
- Short name: **WROW**
- Handler: `OpcodeFuncWrow`
- Status: verified -- compiled from C in a green build
- Length: **3** bytes (`PC_INC(3)`)
- The in-game debugger prints 2 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x55 | *arg1* | *arg2* |
|------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.

