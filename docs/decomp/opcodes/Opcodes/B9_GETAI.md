---
title: B9_GETAI
---

- Opcode: **0xB9**
- Short name: **GETAI**
- Handler: `OpcodeFuncGetai`
- Status: verified -- compiled from C in a green build
- Length: **4** bytes (`PC_INC(4)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xB9 | *arg2* | *?* | *?* |
|------|--------|-----|-----|

#### Arguments

- **const Byte** *arg2*: literal at byte 0x02.

