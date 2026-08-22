---
title: CA_PRTYE
---

- Opcode: **0xCA**
- Short name: **PRTYE**
- Handler: `OpcodeFuncPrtye`
- Status: verified -- compiled from C in a green build
- Length: **4** bytes (`PC_INC(4)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xCA | *arg1* | *?* | *?* |
|------|--------|-----|-----|

#### Arguments

- **const Byte** *arg1*: literal at byte 0x01.

