---
title: 41_MPARA
---

- Opcode: **0x41**
- Short name: **MPARA**
- Handler: `OpcodeFuncMpara`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 4 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x41 | *B1* | *arg2* | *arg3* | *arg4* |
|------|------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: at byte 0x03, addressed through *B1*.
- **const Byte** *arg4*: literal at byte 0x04.

