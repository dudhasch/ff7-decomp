---
title: D3_SLINE
---

- Opcode: **0xD3**
- Short name: **SLINE**
- Handler: `OpcodeFuncSline`
- Status: verified -- compiled from C in a green build
- Length: **16** bytes (`PC_INC(16)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xD3 | *B1 / B2* | *B3 / B4* | *B5 / B6* | *arg4* | *arg6* | *arg8* | *argA* | *argC* | *argE* |
|------|-----------|-----------|-----------|--------|--------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B5*: bank selector, high nibble of byte 0x03; zero means the value is a literal.
- **const Bit\[4\]** *B6*: bank selector, low nibble of byte 0x03; zero means the value is a literal.
- **const Short** *arg4*: at byte 0x04, addressed through *B1*.
- **const Short** *arg6*: at byte 0x06, addressed through *B2*.
- **const Short** *arg8*: at byte 0x08, addressed through *B3*.
- **const Short** *argA*: at byte 0x0A, addressed through *B4*.
- **const Short** *argC*: at byte 0x0C, addressed through *B5*.
- **const Short** *argE*: at byte 0x0E, addressed through *B6*.

