---
title: 9A_LBYTE
---

- Opcode: **0x9A**
- Short name: **LBYTE**
- Handler: `OpcodeFuncLbyte`
- Status: verified -- compiled from C in a green build
- Length: **4** bytes (`PC_INC(4)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0x9A | *B2* | *arg3* | *?* |
|------|------|--------|-----|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg3*: at byte 0x03, addressed through *B2*.

