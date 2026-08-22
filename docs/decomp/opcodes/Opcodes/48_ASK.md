---
title: 48_ASK
---

- Opcode: **0x48**
- Short name: **ASK**
- Handler: `OpcodeFuncAsk`
- Status: verified -- compiled from C in a green build
- Length: **7** bytes (`PC_INC(7)`)
- The in-game debugger prints 6 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x48 | *B2* | *arg2* | *arg3* | *arg4* | *arg5* | *arg6* |
|------|------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: literal at byte 0x03.
- **const Byte** *arg4*: literal at byte 0x04.
- **const Byte** *arg5*: literal at byte 0x05.
- **const Byte** *arg6*: at byte 0x06, addressed through *B2*.

