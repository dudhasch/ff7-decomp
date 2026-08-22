---
title: 2C_BGDPH
---

- Opcode: **0x2C**
- Short name: **BGDPH**
- Handler: `OpcodeFuncBgdph`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x2C | *B1* | *arg2* | *arg3* |
|------|------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Short** *arg3*: at byte 0x03, addressed through *B1*.

