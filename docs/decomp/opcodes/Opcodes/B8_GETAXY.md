---
title: B8_GETAXY
---

- Opcode: **0xB8**
- Short name: **GETAXY**
- Handler: `OpcodeFuncGetaxy`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 4 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xB8 | *arg2* | *?* | *?* | *?* |
|------|--------|-----|-----|-----|

#### Arguments

- **const Byte** *arg2*: literal at byte 0x02.

