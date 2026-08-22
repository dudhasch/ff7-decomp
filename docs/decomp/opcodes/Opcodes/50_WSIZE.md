---
title: 50_WSIZE
---

- Opcode: **0x50**
- Short name: **WSIZE**
- Handler: `OpcodeFuncWsize`
- Status: verified -- compiled from C in a green build
- Length: **10** bytes (`PC_INC(10)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x50 | *arg1* | *arg2* | *arg4* | *arg6* | *arg8* |
|------|--------|--------|--------|--------|--------|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.
- **const Short** *arg2*: literal at byte 0x02.
- **const Short** *arg4*: literal at byte 0x04.
- **const Short** *arg6*: literal at byte 0x06.
- **const Short** *arg8*: literal at byte 0x08.

