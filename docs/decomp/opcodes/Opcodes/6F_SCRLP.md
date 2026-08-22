---
title: 6F_SCRLP
---

- Opcode: **0x6F**
- Short name: **SCRLP**
- Handler: `OpcodeFuncScrlp`
- Status: verified -- compiled from C in a green build
- Length: **6** bytes (`PC_INC(6)`)
- The in-game debugger prints 0 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x6F | *B2* | *arg2* | *arg4* | *arg5* |
|------|------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Short** *arg2*: at byte 0x02, addressed through *B2*.
- **const Byte** *arg4*: literal at byte 0x04.
- **const Byte** *arg5*: literal at byte 0x05.

