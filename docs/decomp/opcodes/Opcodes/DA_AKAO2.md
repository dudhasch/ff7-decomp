---
title: DA_AKAO2
---

- Opcode: **0xDA**
- Short name: **AKAO2**
- Handler: `OpcodeFuncAkao2`
- Status: verified -- compiled from C in a green build
- Length: **15** bytes (`PC_INC(15)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xDA | *B1 / B2* | *B3 / B4* | *B6* | *arg4* | *arg5* | *arg7* | *arg9* | *argB* | *argD* |
|------|-----------|-----------|------|--------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B6*: bank selector, low nibble of byte 0x03; zero means the value is a literal.
- **const Byte** *arg4*: literal at byte 0x04.
- **const Short** *arg5*: at byte 0x05, addressed through *B1*.
- **const Short** *arg7*: at byte 0x07, addressed through *B2*.
- **const Short** *arg9*: at byte 0x09, addressed through *B3*.
- **const Short** *argB*: at byte 0x0B, addressed through *B4*.
- **const Short** *argD*: at byte 0x0D, addressed through *B6*.

