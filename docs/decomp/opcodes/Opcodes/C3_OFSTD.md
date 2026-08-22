---
title: C3_OFSTD
---

- Opcode: **0xC3**
- Short name: **OFSTD**
- Handler: `OpcodeFuncOfstd`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 5 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xC3 | *B1 / B2* | *B3* | *arg3* | *arg4* | *arg6* | *arg8* |
|------|-----------|------|--------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg3*: literal at byte 0x03.
- **const Short** *arg4*: at byte 0x04, addressed through *B1*.
- **const Short** *arg6*: at byte 0x06, addressed through *B2*.
- **const Short** *arg8*: at byte 0x08, addressed through *B3*.

