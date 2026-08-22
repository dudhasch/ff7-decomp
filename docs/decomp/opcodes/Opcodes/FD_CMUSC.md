---
title: FD_CMUSC
---

- Opcode: **0xFD**
- Short name: **CMUSC**
- Handler: `OpcodeFuncCmusc`
- Status: verified -- compiled from C in a green build
- Length: **6** bytes (`PC_INC(6)`)
- The in-game debugger prints 5 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xFD | *B3 / B4* | *arg3* | *arg4* | *arg6* |
|------|-----------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B3*: bank selector, high nibble of byte 0x02; zero means the value is a literal.
- **const Bit\[4\]** *B4*: bank selector, low nibble of byte 0x02; zero means the value is a literal.
- **const Byte** *arg3*: literal at byte 0x03.
- **const Short** *arg4*: at byte 0x04, addressed through *B3*.
- **const Short** *arg6*: at byte 0x06, addressed through *B4*.

