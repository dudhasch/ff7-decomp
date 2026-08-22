---
title: 42_MPRA2
---

- Opcode: **0x42**
- Short name: **MPRA2**
- Handler: `OpcodeFuncMpra2`
- Status: verified -- compiled from C in a green build
- Length: **6** bytes (`PC_INC(6)`)
- The in-game debugger prints 5 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x42 | *B1* | *arg2* | *arg3* | *?* | *?* |
|------|------|--------|--------|-----|-----|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: at byte 0x03, addressed through *B1*.

