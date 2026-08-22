---
title: BE_UNKNOWN
---

- Opcode: **0xBE**
- Short name: **UNKNOWN**
- Handler: `OpcodeFuncBad`
- Status: verified -- compiled from C in a green build
- Length: unknown -- the handler has no `PC_INC`

#### Memory layout

Not known: `OpcodeFuncBad` calls no DebugPrintOpcode, so the mnemonic and length are not recoverable from it.

