---
title: EA_MPPAL
---

- Opcode: **0xEA**
- Short name: **MPPAL**
- Handler: `OpcodeFuncMppal2`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xEA | *B1 / B2* | *B3 / B4* | *B5* | *arg4* | *arg5* | *arg6* | *arg7* | *arg8* | *arg9* |
|------|-----------|-----------|------|--------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B5*: bank selector, high nibble of byte 0x03; zero means the value is a literal.
- **const Byte** *arg4*: at byte 0x04, addressed through *B1*.
- **const Byte** *arg5*: at byte 0x05, addressed through *B2*.
- **const Byte** *arg6*: at byte 0x06, addressed through *B3*.
- **const Byte** *arg7*: at byte 0x07, addressed through *B4*.
- **const Byte** *arg8*: at byte 0x08, addressed through *B5*.
- **const Byte** *arg9*: literal at byte 0x09.

