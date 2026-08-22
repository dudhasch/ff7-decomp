---
title: 56_GWCOL
---

- Opcode: **0x56**
- Short name: **GWCOL**
- Handler: `OpcodeFuncGwcol`
- Status: verified -- compiled from C in a green build
- Length: **7** bytes (`PC_INC(7)`)
- The in-game debugger prints 6 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x56 | *B1* | *arg3* | *?* | *?* | *?* | *?* |
|------|------|--------|-----|-----|-----|-----|

#### Arguments

- **const Bit\[4\]** *B1*: bank selector, high nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg3*: at byte 0x03, addressed through *B1*.

