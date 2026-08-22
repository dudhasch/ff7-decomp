---
title: 2F_WSIZW
---

- Opcode: **0x2F**
- Short name: **WSIZW**
- Handler: `OpcodeFuncWsizw`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x2F | *arg1* |
|------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

