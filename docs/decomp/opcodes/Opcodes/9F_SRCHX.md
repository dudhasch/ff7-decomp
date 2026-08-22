---
title: 9F_SRCHX
---

- Opcode: **0x9F**
- Short name: **SRCHX**
- Handler: `OpcodeFuncSrchx`
- Status: verified -- compiled from C in a green build
- Length: **11** bytes (`PC_INC(11)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x9F | *B2* | *B3 / B4* | *arg4* | *arg5* | *arg7* | *arg9* | *?* | *?* |
|------|------|-----------|--------|--------|--------|--------|-----|-----|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg1*: literal at byte 0x01.
- **const Byte** *arg4*: literal at byte 0x04.
- **const Short** *arg5*: at byte 0x05, addressed through *B2*.
- **const Short** *arg7*: at byte 0x07, addressed through *B3*.
- **const Byte** *arg9*: at byte 0x09, addressed through *B4*.

