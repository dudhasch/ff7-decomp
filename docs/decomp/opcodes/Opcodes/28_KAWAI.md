---
title: 28_KAWAI
---

- Opcode: **0x28**
- Short name: **KAWAI**
- Handler: `OpcodeFuncKawai`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x28 | *arg1* | *arg2* | *arg3* |
|------|--------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: literal at byte 0x03.

