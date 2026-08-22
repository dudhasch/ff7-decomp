---
title: B7_GTDIR
---

- Opcode: **0xB7**
- Short name: **GTDIR**
- Handler: `OpcodeFuncGtdir`
- Status: verified -- compiled from C in a green build
- Length: **4** bytes (`PC_INC(4)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xB7 | *arg2* | *?* | *?* |
|------|--------|-----|-----|

#### Arguments

- **const Byte** *arg2*: literal at byte 0x02.

