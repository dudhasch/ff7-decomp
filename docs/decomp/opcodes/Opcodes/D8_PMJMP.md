---
title: D8_PMJMP
---

- Opcode: **0xD8**
- Short name: **PMJMP**
- Handler: `OpcodeFuncPmjmp`
- Status: verified -- compiled from C in a green build
- Length: **3** bytes (`PC_INC(3)`)
- The in-game debugger prints 8 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xD8 | *arg1* |
|------|--------|

#### Arguments

- **const Short** *arg1*: literal at byte 0x01.

