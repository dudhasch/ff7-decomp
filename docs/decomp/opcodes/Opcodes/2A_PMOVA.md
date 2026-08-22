---
title: 2A_PMOVA
---

- Opcode: **0x2A**
- Short name: **PMOVA**
- Handler: `OpcodeFuncPmova`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 1 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x2A | *arg1* |
|------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

