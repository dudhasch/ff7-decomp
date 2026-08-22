---
title: 9E_GETX
---

- Opcode: **0x9E**
- Short name: **GETX**
- Handler: `OpcodeFuncGetx`
- Status: verified -- compiled from C in a green build
- Length: **7** bytes (`PC_INC(7)`)
- The in-game debugger prints 6 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x9E | *B2* | *arg3* | *?* | *?* | *?* |
|------|------|--------|-----|-----|-----|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg1*: literal at byte 0x01.
- **const Short** *arg3*: at byte 0x03, addressed through *B2*.

