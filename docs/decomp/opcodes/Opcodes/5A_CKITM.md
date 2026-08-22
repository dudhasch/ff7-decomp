---
title: 5A_CKITM
---

- Opcode: **0x5A**
- Short name: **CKITM**
- Handler: `OpcodeFuncCkitm`
- Status: verified -- compiled from C in a green build
- Length: **5** bytes (`PC_INC(5)`)
- The in-game debugger prints 4 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x5A | *B1* | *arg2* | *?* |
|------|------|--------|-----|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Short** *arg2*: at byte 0x02, addressed through *B1*.

