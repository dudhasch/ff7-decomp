---
title: 75_PXYZI
---

- Opcode: **0x75**
- Short name: **PXYZI**
- Handler: `OpcodeFuncPxyzi`
- Status: verified -- compiled from C in a green build
- Length: **8** bytes (`PC_INC(8)`)
- The in-game debugger prints 7 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x75 | *arg3* | *?* | *?* | *?* | *?* | *?* | *?* |
|------|--------|-----|-----|-----|-----|-----|-----|

#### Arguments

- **const Byte** *arg3*: literal at byte 0x03.

