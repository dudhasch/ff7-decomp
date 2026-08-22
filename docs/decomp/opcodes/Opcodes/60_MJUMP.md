---
title: 60_MJUMP
---

- Opcode: **0x60**
- Short name: **MJUMP**
- Handler: `OpcodeFuncMjump`
- Status: verified -- compiled from C in a green build
- Length: **10** bytes (`PC_INC(10)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x60 | *arg9* | *?* | *?* | *?* | *?* | *?* | *?* | *?* | *?* |
|------|--------|-----|-----|-----|-----|-----|-----|-----|-----|

#### Arguments

- **const Byte** *arg9*: literal at byte 0x09.

