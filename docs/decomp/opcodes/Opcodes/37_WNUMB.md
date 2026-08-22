---
title: 37_WNUMB
---

- Opcode: **0x37**
- Short name: **WNUMB**
- Handler: `OpcodeFuncWnumb`
- Status: verified -- compiled from C in a green build
- Length: **8** bytes (`PC_INC(8)`)
- The in-game debugger prints 7 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x37 | *B1 / B2* | *arg2* | *arg3* | *arg5* | *arg7* |
|------|-----------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Short** *arg3*: at byte 0x03, addressed through *B1*.
- **const Short** *arg5*: at byte 0x05, addressed through *B2*.
- **const Byte** *arg7*: literal at byte 0x07.

