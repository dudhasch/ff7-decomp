---
title: 38_STTIM
---

- Opcode: **0x38**
- Short name: **STTIM**
- Handler: `OpcodeFuncSttim`
- Status: verified -- compiled from C in a green build
- Length: **6** bytes (`PC_INC(6)`)
- The in-game debugger prints 5 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x38 | *B1 / B2* | *B4* | *arg3* | *arg4* | *arg5* |
|------|-----------|------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg3*: at byte 0x03, addressed through *B1*.
- **const Byte** *arg4*: at byte 0x04, addressed through *B2*.
- **const Byte** *arg5*: at byte 0x05, addressed through *B4*.

