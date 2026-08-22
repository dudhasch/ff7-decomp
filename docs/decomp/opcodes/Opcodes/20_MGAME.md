---
title: 20_MGAME
---

- Opcode: **0x20**
- Short name: **MGAME**
- Handler: `OpcodeFuncMgame`
- Status: verified -- compiled from C in a green build
- Length: **11** bytes (`PC_INC(11)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x20 | *arg9* | *argA* | *?* | *?* | *?* | *?* | *?* | *?* | *?* | *?* |
|------|--------|--------|-----|-----|-----|-----|-----|-----|-----|-----|

#### Arguments

- **const Byte** *arg9*: literal at byte 0x09.
- **const Byte** *argA*: literal at byte 0x0A.

