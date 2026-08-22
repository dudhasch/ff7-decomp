---
title: 73_PGTDR
---

- Opcode: **0x73**
- Short name: **PGTDR**
- Handler: `OpcodeFuncPgtdr`
- Status: verified -- compiled from C in a green build
- Length: **4** bytes (`PC_INC(4)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x73 | *arg2* | *?* | *?* |
|------|--------|-----|-----|

#### Arguments

- **const Byte** *arg2*: literal at byte 0x02.

