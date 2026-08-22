---
title: F2_AKAO
---

- Opcode: **0xF2**
- Short name: **AKAO**
- Handler: `OpcodeFuncAkao`
- Status: verified -- compiled from C in a green build
- Length: **14** bytes (`PC_INC(14)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xF2 | *B1 / B2* | *B3 / B4* | *B6* | *arg4* | *arg5* | *arg6* | *arg8* | *argA* | *argC* |
|------|-----------|-----------|------|--------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B6*: bank selector, low nibble of byte 0x03; zero means the value is a literal.
- **const Byte** *arg4*: literal at byte 0x04.
- **const Byte** *arg5*: at byte 0x05, addressed through *B1*.
- **const Short** *arg6*: at byte 0x06, addressed through *B2*.
- **const Short** *arg8*: at byte 0x08, addressed through *B3*.
- **const Short** *argA*: at byte 0x0A, addressed through *B4*.
- **const Short** *argC*: at byte 0x0C, addressed through *B6*.

