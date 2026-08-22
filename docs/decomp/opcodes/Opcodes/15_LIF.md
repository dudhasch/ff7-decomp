---
title: 15_LIF
---

- Opcode: **0x15**
- Short name: **LIF**
- Handler: `OpcodeFuncLif`
- Status: verified -- compiled from C in a green build
- Length: **7** bytes (`PC_INC(7)`)
- The in-game debugger prints 6 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x15 | *arg5* | *?* | *?* | *?* | *?* |
|------|--------|-----|-----|-----|-----|

#### Arguments

- **const Short** *arg5*: literal at byte 0x05.

