---
title: 09_SPLIT
---

- Opcode: **0x09**
- Short name: **SPLIT**
- Handler: `OpcodeFuncSplit`
- Status: verified -- compiled from C in a green build
- Length: **15** bytes (`PC_INC(15)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x09 | *B1 / B2* | *B3 / B4* | *B5 / B6* | *arg4* | *arg6* | *arg8* | *arg9* | *argB* | *argD* | *argE* |
|------|-----------|-----------|-----------|--------|--------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B5*: bank selector, high nibble of byte 0x03; zero means the value is a literal.
- **const Bit\[4\]** *B6*: bank selector, low nibble of byte 0x03; zero means the value is a literal.
- **const Short** *arg4*: at byte 0x04, addressed through *B1*.
- **const Short** *arg6*: at byte 0x06, addressed through *B2*.
- **const Byte** *arg8*: at byte 0x08, addressed through *B3*.
- **const Short** *arg9*: at byte 0x09, addressed through *B4*.
- **const Short** *argB*: at byte 0x0B, addressed through *B5*.
- **const Byte** *argD*: at byte 0x0D, addressed through *B6*.
- **const Byte** *argE*: literal at byte 0x0E.

