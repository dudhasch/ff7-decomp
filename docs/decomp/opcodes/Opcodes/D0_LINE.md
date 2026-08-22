---
title: D0_LINE
---

- Opcode: **0xD0**
- Short name: **LINE**
- Handler: `OpcodeFuncLine`
- Status: verified -- compiled from C in a green build
- Length: **13** bytes (`PC_INC(13)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xD0 | *arg1* | *arg3* | *arg5* | *arg7* | *arg9* | *argB* |
|------|--------|--------|--------|--------|--------|--------|

#### Arguments

- **const Short** *arg1*: literal at byte 0x01.
- **const Short** *arg3*: literal at byte 0x03.
- **const Short** *arg5*: literal at byte 0x05.
- **const Short** *arg7*: literal at byte 0x07.
- **const Short** *arg9*: literal at byte 0x09.
- **const Short** *argB*: literal at byte 0x0B.

