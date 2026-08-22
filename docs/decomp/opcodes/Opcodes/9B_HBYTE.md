---
title: 9B_HBYTE
---

- Opcode: **0x9B**
- Short name: **HBYTE**
- Handler: `OpcodeFuncHbyte`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 4 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x9B | *B2* | *arg3* | *?* |
|------|------|--------|-----|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Short** *arg3*: at byte 0x03, addressed through *B2*.

