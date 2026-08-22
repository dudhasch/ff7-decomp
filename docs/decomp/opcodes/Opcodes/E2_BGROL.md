---
title: E2_BGROL
---

- Opcode: **0xE2**
- Short name: **BGROL**
- Handler: `OpcodeFuncBgrol`
- Status: verified -- compiled from C in a green build
- Length: **3** bytes (`PC_INC(3)`)
- The in-game debugger prints 3 argument byte(s); this saturates at 8 and is not the instruction length.

#### Memory layout

| 0xE2 | *B2* | *arg2* |
|------|------|--------|

#### Arguments

- **const Bit\[4\]** *B2*: bank selector, low nibble of byte 0x01; zero means the value is a literal.
- **const Byte** *arg2*: at byte 0x02, addressed through *B2*.

