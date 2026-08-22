---
title: 5D_CMTRA
---

- Opcode: **0x5D**
- Short name: **CMTRA**
- Handler: `OpcodeFuncCmtra`
- Status: verified -- compiled from C in a green build
- Length: **10** bytes (`PC_INC(10)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x5D | *B1 / B2* | *B3 / B4* | *arg4* | *arg5* | *arg6* | *arg7* | *arg8* | *?* | *?* |
|------|-----------|-----------|--------|--------|--------|--------|--------|-----|-----|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg4*: at byte 0x04, addressed through *B1*.
- **const Byte** *arg5*: at byte 0x05, addressed through *B2*.
- **const Byte** *arg6*: at byte 0x06, addressed through *B3*.
- **const Byte** *arg7*: at byte 0x07, addressed through *B4*.
- **const Byte** *arg8*: literal at byte 0x08.

