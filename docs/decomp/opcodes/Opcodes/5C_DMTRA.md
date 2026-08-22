---
title: 5C_DMTRA
---

- Opcode: **0x5C**
- Short name: **DMTRA**
- Handler: `OpcodeFuncDmtra`
- Status: verified -- compiled from C in a green build
- Length: **8** bytes (`PC_INC(8)`)
- The in-game debugger prints 7 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x5C | *B1 / B2* | *B3 / B4* | *arg3* | *arg4* | *arg5* | *arg6* | *arg7* |
|------|-----------|-----------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg3*: at byte 0x03, addressed through *B1*.
- **const Byte** *arg4*: at byte 0x04, addressed through *B2*.
- **const Byte** *arg5*: at byte 0x05, addressed through *B3*.
- **const Byte** *arg6*: at byte 0x06, addressed through *B4*.
- **const Byte** *arg7*: literal at byte 0x07.

