---
title: F1_SE
---

- Opcode: **0xF1**
- Short name: **SE**
- Handler: `OpcodeFuncSe`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xF1 | *B1 / B2* | *arg2* | *arg4* |
|------|-----------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Short** *arg2*: at byte 0x02, addressed through *B1*.
- **const Byte** *arg4*: at byte 0x04, addressed through *B2*.

