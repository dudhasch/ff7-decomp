---
title: 62_SCRLC
---

- Opcode: **0x62**
- Short name: **SCRLC**
- Handler: `OpcodeFuncScrlc`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 0 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x62 | *B2* | *arg2* | *arg4* |
|------|------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Short** *arg2*: at byte 0x02, addressed through *B2*.
- **const Byte** *arg4*: literal at byte 0x04.

