---
title: 6A_VWOFT
---

- Opcode: **0x6A**
- Short name: **VWOFT**
- Handler: `OpcodeFuncVwoft`
- Status: **unverified** -- pinned to its assembly, the C beside it is never compiled
- Length: unknown -- the handler has no `PC_INC`
- The in-game debugger prints 6 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x6A | *B1 / B2* | *arg2* | *arg4* | *arg6* |
|------|-----------|--------|--------|--------|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Short** *arg2*: at byte 0x02, addressed through *B1*.
- **const Short** *arg4*: at byte 0x04, addressed through *B2*.
- **const Byte** *arg6*: literal at byte 0x06.

