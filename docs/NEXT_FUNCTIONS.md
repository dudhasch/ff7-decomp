# Next Functions — Ranked by Logic Simplicity

A static callgraph ranking of every remaining `INCLUDE_ASM` in the tree
(1615 functions as of 2026-08-17). The ordering criterion is **logic
simplicity**, not file size:

- **Leaf** — the function makes no calls to other game functions and has no
  `jalr` (no indirect calls through function pointers or jump tables).
- **Helper** — calls only 1–3 other functions, all direct.
- Penalties applied for: external (SDK) calls, indirect calls, jump tables
  (`jtbl_` / `.rdata` reference), and hardware division (`div`/`divu` — sign
  handling in gcc 2.6.3 makes modulo/division a common near-miss cause).

Columns: `i` = instruction count in the target `.s`, `ext` = calls to
SDK/unknown symbols, `int` = calls to other game functions, `div` =
`div`/`divu` instructions.

Always cross-check the pick with `./mako.sh rank <file>` — the difficulty
model weighs different signals than this static analysis — and run
`tools/rodata_owner.py <file> <fn>` before writing any C (see CLAUDE.md §3b).

---

## Tier 0 — `src/main/psxsdk.c`: the SDK library batch (515 open)

A third of everything left is PsyQ runtime library. This is a separate
category from game logic: the functions are documented in the PsyQ Technical
References and their semantics are known exactly (see
[PSYQ_SDK_SOURCES.md](PSYQ_SDK_SOURCES.md)).

| bucket (leaf, no jtbl) | count |
| --- | --- |
| ≤ 8 instructions (pure syscall/`jr $ra` wrappers) | 140 |
| 9–30 instructions | 86 |
| 31–100 instructions | 58 |
| > 100 instructions | 12 |
| non-leaf or jump table | 219 |
| **total** | **515** |

The 140 sub-8-instruction functions are mostly named already: `SetIR0`,
`SetData32`, `SetDQA`/`SetDQB`, `Lzc`, `DecDCTBufSize`, `SetVertex0/1/2`,
`ReadGeomScreen`, `SetGeomScreen`, `AverageSZ3/4`, `ReadLZC`, `InitHeap`,
`FlushCache`, `_bu_init`, `_96_remove`, `SetMem`, `SystemError`,
`OpenEvent`, `CloseEvent`, `WaitEvent`, `TestEvent`, `EnableEvent`,
`DisableEvent`, `DeliverEvent`, `UnDeliverEvent`, `InitPAD`, `StartPAD`,
`StopPAD`, `PAD_init`, `PAD_dr`, `ReturnFromException`, `ResetEntryInt`,
`HookEntryInt`, `EnterCriticalSection`, …

These match against the PsyQ reference sources nearly mechanically. Note:
many are leaf *syscalls*, i.e. the "C" is a one-line inline-asm or a
`syscall`-number thunk — check how already-matched neighbours in the file
express them before writing bodies.

## Tier 1 — Pure game-logic leafs, ≤ 20 instructions, no calls at all

The cheapest real game code. No internal calls, no external calls, no jump
tables, no division. Most are getters, setters, flag flips or tiny struct
writes.

| i | function | file |
| --- | --- | --- |
| 3 | `func_80023050` | `src/main/1255C.c` |
| 3 | `func_80026A00` | `src/main/1255C.c` |
| 3 | `func_80026B64` | `src/main/1255C.c` |
| 3 | `func_800155A4` | `src/main/18B8.c` |
| 4 | `func_800A1ED4` | `src/ending/ending.c` |
| 4 | `func_800A22D4` | `src/ending/ending.c` |
| 4 | `func_80023AC4` | `src/main/1255C.c` |
| 5 | `func_80026A0C` | `src/main/1255C.c` |
| 5 | `func_80026A20` | `src/main/1255C.c` |
| 5 | `func_800148A0` | `src/main/18B8.c` |
| 6 | `func_800A32D8` | `src/ending/ending.c` |
| 6 | `func_800269D0` | `src/main/1255C.c` |
| 6 | `func_800269E8` | `src/main/1255C.c` |
| 6 | `func_800294A4` | `src/main/akao.c` |
| 6 | `func_8002C8C4` | `src/main/akao.c` |
| 7 | `func_800A886C` | `src/world/world.c` |
| 7 | `func_800B57C0` | `src/world/world.c` |
| 8 | `func_800A0514` | `src/brom/brom.c` |
| 9 | `func_800B579C` | `src/world/world.c` |
| 9 | `func_800B7838` | `src/world/world.c` |
| 10 | `func_800D3520` | `src/battle/battle2.c` |
| 11 | `func_80014C44` | `src/main/18B8.c` |
| 11 | `func_800B77F4` | `src/world/world.c` |
| 12 | `func_800A2274` | `src/ending/ending.c` |
| 12 | `func_800A22A4` | `src/ending/ending.c` |
| 12 | `func_800254E4` | `src/main/1255C.c` |
| 12 | `func_80029998` | `src/main/akao.c` |
| 12 | `func_800A16E0` | `src/world/world.c` |
| 12 | `func_800A5970` | `src/world/world.c` |
| 13 | `SystemMenuAddPartyGold` | `src/main/1255C.c` |
| 13 | `func_8001DEF0` | `src/main/18B8.c` |
| 14 | `SystemMenuRemovePartyGold` | `src/main/1255C.c` |
| 14 | `func_80019E4C` | `src/main/18B8.c` |
| 15 | `func_80018E90` | `src/main/18B8.c` |
| 15 | `func_8001FF50` | `src/main/18B8.c` |
| 15 | `func_8002001C` | `src/main/18B8.c` |
| 15 | `func_80033420` | `src/main/akao.c` |
| 15 | `func_800B01C4` | `src/world/world.c` |
| 16 | `func_800A2934` | `src/ending/ending.c` |
| 16 | `func_8001DE70` / `func_8001DEB0` | `src/main/18B8.c` |
| 16 | `func_8002C884` / `func_80033224` | `src/main/akao.c` |
| 16 | `func_800A6B8C`, `func_800AF2A4/AF324/AF364` | `src/world/world.c` |
| 17 | `func_800A22E4` | `src/ending/ending.c` |
| 17 | `func_80019338`, `func_80019608`, `func_8001964C` | `src/main/18B8.c` |
| 17 | `func_800A5A94` | `src/world/world.c` |
| 18 | `func_800B1368` | `src/battle/battle.c` |
| 18 | `func_80015AFC`, `func_8001FF8C`, `func_8001FFD4` | `src/main/18B8.c` |
| 18 | `func_800A40F0`, `func_800AD928`, `func_800B017C`, `func_800B37E0` | `src/world/world.c` |
| 19 | `func_80014B08`, `func_800193F4` | `src/main/18B8.c` |
| 19 | `func_80031AB0` | `src/main/akao.c` |
| 20 | `func_80025310` | `src/main/1255C.c` |
| 20 | `func_80015D14`, `func_8001C498` | `src/main/18B8.c` |
| 20 | `func_8002A748`, `func_8002A798`, `func_8002B1A8`, `func_8003252C` | `src/main/akao.c` |
| 20 | `func_800B5DD8` | `src/world/world.c` |

`SystemMenuAddPartyGold` / `SystemMenuRemovePartyGold` are already named —
semantics known, ideal first picks. Same-file siblings
`SystemMenuAddHpByPartyId` / `SystemMenuAddMpByPartyId` (51i each) are the
natural follow-up pair.

## Tier 2 — Small helpers (1–3 internal calls, ≤ 40 instructions)

One call deep, still trivially reviewable. Good second wave after Tier 1,
because matching the helper first often names the leafs that call it (and
vice versa).

| i | int | function | file |
| --- | --- | --- | --- |
| 9 | 1 | `func_800A1FA4`, `func_800A20D4` | `src/ending/ending.c` |
| 9 | 1 | `func_800293D0` | `src/main/akao.c` |
| 10 | 1 | `func_800A23F8` | `src/ending/ending.c` |
| 12 | 1 | `func_800293F4` | `src/main/akao.c` |
| 13 | 1 | `func_8001A384` | `src/main/18B8.c` |
| 13 | 1 | `func_800B64D8`, `func_800B6570` | `src/world/world.c` |
| 14 | 1 | `func_8002BBB4` | `src/main/akao.c` |
| 15 | 1 | `func_800A2190` | `src/ending/ending.c` |
| 15 | 1 | `func_800B65A4` | `src/world/world.c` |
| 16 | 1 | `func_800B0200` | `src/world/world.c` |
| 17 | 2 | `func_800A015C` (also 2 ext) | `src/brom/brom.c` |
| 18 | 1 | `func_800A208C` | `src/ending/ending.c` |
| 18 | 2 | `func_800211C4` | `src/main/18B8.c` |
| 19 | 1 | `func_800B77A8` | `src/world/world.c` |
| 19 | 2 | `func_800A1FC8` | `src/ending/ending.c` |
| 19 | 2 | `func_8001A980` | `src/main/18B8.c` |
| 23 | 1 | `func_800A1EEC`, `func_800A1F48` | `src/ending/ending.c` |
| 23 | 1 | `func_800B7AC0` | `src/world/world.c` |
| 24 | 1 | `func_800A5348`, `func_800AF0B0` | `src/world/world.c` |
| 24 | 2 | `func_80026A34` | `src/main/1255C.c` |
| 24 | 2 | `func_80014980` | `src/main/18B8.c` |
| 25 | 1 | `func_800B5CD4` | `src/battle/battle1.c` |
| 25 | 1 | `func_80015C3C` | `src/main/18B8.c` |
| 26 | 1 | `func_80014E0C` | `src/main/18B8.c` |
| 27 | 1 | `func_800D76B8` | `src/battle/battle2.c` |
| 28 | 2 | `func_800A85FC` | `src/battle/battle.c` |
| 30 | 1 | `func_800A2380` | `src/ending/ending.c` |
| 30 | 1 | `func_80018E18` | `src/main/18B8.c` |
| 32 | 1 | `func_800D3F0C` | `src/battle/battle2.c` |
| 32 | 1 | `func_800A5B88` | `src/world/world.c` |
| 34 | 1 | `func_800D6734`, `func_800D7B1C`, `func_800D7BA4` | `src/battle/battle2.c` |
| 34 | 1 | `func_800B7620` | `src/world/world.c` |
| 36 | 1 | `func_800B8FCC` | `src/battle/battle1.c` |
| 36 | 3 | `func_800A00CC` | `src/brom/brom.c` |
| 37 | 1 | `func_800C2FD4` | `src/battle/battle1.c` |
| 37 | 1 | `func_800D7A88` | `src/battle/battle2.c` |
| 38 | 1 | `func_800DF530` | `src/battle/battle3.c` |
| 39 | 1 | `func_800DF9F8` | `src/battle/battle3.c` |
| 39 | 1 | `func_800BBA84` | `src/battle/battle1.c` |

## Tier 3 — Finish-a-file clusters

Files where a handful of matches close out the whole translation unit.

| file | open | notes |
| --- | --- | --- |
| `src/menu/bginmenu.c` | 3 | All three are leafs: `func_801D027C` (42i, 1 ext), `func_801D0324` (57i), `func_801D0408` (62i). No jump tables. **Easiest file to finish outright.** |
| `src/brom/brom.c` | 5 | `func_800A0514` (8i leaf), `func_800A015C` (17i), `func_800A00CC` (36i) are the cheap three; `func_800A01A0` (221i, 33 int) and `func_800A0000` (790i, 69 int) are the boot loaders — leave for last. |
| `src/menu/savemenu.c` | 3 | `func_801D1C2C` (60i, 1 ext, 2 int) is approachable. ⚠ `savemenu` is one of the four units whose `.rodata` base is 4 mod 8 (jump-table phase, CLAUDE.md §3b) — check `rodata_owner.py` first. |
| `src/battle/batini.c` | 3 | All large (272–402i) but none have jump tables; `func_801B1E0C` (319i) is a pure leaf — long but linear. |
| `src/menu/title.c` | 2 | `func_801D3478` (124i, 5 ext, 3 int) and `func_801D2DA8` (326i leaf). |
| `src/menu/itemmenu.c` | 2 | `func_801D3260` (0–1i stub) is free; `func_801D0E80` (1723i, 93 int, jtbl) is one of the hardest functions left — pair them only at the end. |

## Tier 4 — Named medium leafs worth batching

Semantically obvious, no calls, moderate size — good throughput once the
tiny ones are done.

| i | function | file |
| --- | --- | --- |
| 30 | `FieldDebugPageAddPos`, `FieldDebugPageAddSize` | `src/field/field.c` |
| 86 | `FieldModelScalePartVrtxs` | `src/field/field.c` |
| 97 | `FieldEntityCollisionCheck` | `src/field/field.c` |
| 99 | `FieldEntityAnimationUpdate` | `src/field/field.c` |
| 123 | `FieldModelStructInit` | `src/field/field.c` |
| 125 | `FieldEnablePartyModels` | `src/field/field.c` |
| 129 | `FieldEntitySqrDistToLine` (1 div) | `src/field/field.c` |
| 137 | `FieldCalcPointOnLine` (4 div) | `src/field/field.c` |
| 168 | `FieldRainUpdate` | `src/field/field.c` |
| 200 | `FieldModelScaleAnimTranslat` | `src/field/field.c` |
| 51 | `SystemMenuAddHpByPartyId`, `SystemMenuAddMpByPartyId` | `src/main/1255C.c` |

## Explicitly NOT easy (small remaining counts, but blocked)

- `src/menu/cnfgmenu.c` — `func_801D080C`: 1189 instructions + jump table.
  Use `tools/variant_eval.py` (already hardcoded to this function).
- `src/dschange/dschange.c` — 2 functions, 586i/790i with 54/69 internal
  calls: the densest callgraph nodes in the repo.
- `src/field/field.c` rodata cluster — `AddStrNextDebugRow` /
  `AddColorStrNextDebugRow` (`$at` re-materialisation, no known fix),
  `OpcodeFuncMenu2` (borrows rodata from `OpcodeFuncMenu`), and the
  `IfCheck`/`If2Check*`/opcode group that needs the file split on the
  original seams (`docs/PC_HANDOVER_XREF.md` has the PC partition evidence).

## Suggested working order

1. `psxsdk.c` ≤ 8i batch (with the SDK references — see PSYQ_SDK_SOURCES.md)
2. Tier 1 leafs, file by file: `1255C.c` → `ending.c` → `akao.c` → `18B8.c` → `world.c`
3. `bginmenu.c` + `brom.c` cheap three (file completions)
4. Tier 2 helpers (they name their neighbours)
5. Tier 4 named medium leafs
6. Then re-run this analysis — each match renames symbols and changes the
   picture.
