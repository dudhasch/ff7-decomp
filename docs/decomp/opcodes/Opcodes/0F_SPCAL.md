---
title: 0F_SPCAL
---

- Opcode: **0x0F**
- Short name: **SPCAL**
- Handler: `OpcodeFuncSpcal`
- Status: verified -- compiled from C in a green build
- Length: not a single value; `PC_INC` is 2, 3, 4, 6
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x0F | *arg1* | *B3 / B4* | *arg3* | *arg5* |
|------|--------|-----------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg2*: literal at byte 0x02.
- **const Byte** *arg3*: at byte 0x03, addressed through *B3* / *B4*.
- **const Byte** *arg5*: literal at byte 0x05.

#### Notes

- The handler advances the script pointer by different amounts on different paths: 2, 3, 4, 6.

