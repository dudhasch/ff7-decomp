---
title: 32_KEYOF
---

- Opcode: **0x32**
- Short name: **KEYOF**
- Handler: `OpcodeFuncKeyof`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x32 | *arg2* |
|------|--------|

#### Arguments

- **const Byte** *arg2*: literal at byte 0x02.

