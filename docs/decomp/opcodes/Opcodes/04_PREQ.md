---
title: 04_PREQ
---

- Opcode: **0x04**
- Short name: **PREQ**
- Handler: `OpcodeFuncPreq`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 2 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x04 | *arg1* | *arg2* |
|------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.

