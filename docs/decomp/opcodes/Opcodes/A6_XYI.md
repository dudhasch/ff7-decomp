---
title: A6_XYI
---

- Opcode: **0xA6**
- Short name: **XYI**
- Handler: `OpcodeFuncXyi`
- Status: verified -- compiled from C in a green build
- Length: **9** bytes (`PC_INC(9)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xA6 | *B1 / B2* | *B3* | *arg3* | *arg5* | *arg7* |
|------|-----------|------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Short** *arg3*: at byte 0x03, addressed through *B1*.
- **const Short** *arg5*: at byte 0x05, addressed through *B2*.
- **const Short** *arg7*: at byte 0x07, addressed through *B3*.

