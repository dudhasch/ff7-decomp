# PC handover cross-reference

What `ff7-pc-decomp/research/handover/psx-decomp-paths.md` (assert/`__FILE__`
paths extracted from the 1998 retail `ff7.exe`) does and does not give this
project, checked against the actual state of the tree. The handover is
naming/module evidence, not decompilation evidence — nothing in it changes a
`checkfn.py` verdict — but two sections of it line up with real open problems.

## 1. The field file split — the one direct hit

`src/field/field.c` is several original `.c` files glued together (its own
`// Begin of field_*.c` comments mark 33 inferred seams). Because the original
build compiled each as its own object with its own `.rodata` base, the merged
unit carries jump tables in *both* 8-byte-alignment phases at once, and one
`--phase` setting cannot satisfy both. This is what keeps these stuck as
`INCLUDE_ASM` even though their instructions match:

- `FieldEntityBgTriggerInit` (comment at line ~739: "every instruction of the C
  matches, but gcc precedes the switch's jump table with `.align 3`")
- `IfCheck`, `If2CheckSigned`, `If2CheckUnsigned`
- `OpcodeFuncSetx`, `OpcodeFuncGetx`, `OpcodeFuncSrchx`
- `OpcodeFuncFade`, `OpcodeFuncFadew`, `OpcodeFuncSpcal`
- `FieldEventWriteMemoryU8`, `FieldEventRequestRun`
- `OpcodeFuncMenu2` (additionally `BORROWS` a string owned by `OpcodeFuncMenu`)

The eventual fix is splitting `field.c` on the original TU boundaries. The
repo's current seams are *inferred from code content*; the handover's source
tree is *binary-grounded* — the 12 `ad_*.cpp` files under `field\src\`
(`ad_app`, `ad_bk`, `ad_cdr`, `ad_data`, `ad_ddraw`, `ad_human`, `ad_image`,
`ad_list`, `ad_obj`, `ad_pal`, `ad_tile`, `tutaddr`) are how Square actually
partitioned the field module on the PC side.

**Caveat:** the PC `ad_*` split is the PC port's module layout, not the PSX
overlays' TU boundaries. It is evidence of how the field code was cut up, not a
1:1 map. Use it to sanity-check the inferred seams when the split happens, not
to replace them. The `ad_` prefix is the field module's internal namespace and
worth keeping in mind for symbol naming.

## 2. Battle module attribution — useful for renaming

`src/battle/*.c` currently has no `// Begin of` seams and mostly address-based
`func_*`/`D_*` names. The handover confirms the known Square battle-team split:

| PC source dir | Attribution |
|---|---|
| `src\battle\yama\` (`coloss.cpp`, `inits.cpp`, `init.cpp`) | Yama — summon/Colossus effects |
| `src\battle\yasui\` (`deadsef.cpp`, `sting.cpp`, `vahamut0.cpp`) | Yasui — `vahamut0` = Bahamut |
| `src\battle\myoshiok\` (`lasboss3.cpp`) | Myoshioka — final bosses |

When `func_*` in `battle.c`/`battle1.c`/`battle3.c` get renamed, these author
directories are the best surviving evidence for module attribution.

## 3. MAGIC effect names — confirmatory only

`battle2.c` already models `MAGIC/*.BIN` as an overlay entrypoint table
(`D_800EEBB8[] // MAGIC/*.BIN overlay`, plus the `func_801B*` entrypoints).
The handover's 48 effect names (`MAGIC/BARIA` … `MAGIC/WALL`, plus the
lowercase summon/alt variants like `magic/altema`, `magic/faiga`) can anchor
the magic-effect ID map, but they confirm what the repo already assumes rather
than add anything. The `magic-barrier` overlay's symbol file is nearly empty —
if that overlay gets worked, the effect-name list is a reasonable naming source.

## 4. Everything else — no transfer value

These handover sections do not help a matching decomp:

- **Condor Fort RSD table (292 paths)** and **chocobo/minigame HRC/ANM tables** —
  asset inventories. `src/world/` has no condor/wmfile code yet; asset names
  don't affect instruction matching.
- **`.sf2` SoundFonts, MIDI, DirectX, ACM, registry keys** — PC-port-only, no
  PSX counterpart.
- **`data\tim\class_*`, `kao`, `lsprite`/`ssprite`** — battle-menu UI TIMs;
  relevant to assets, not to `src/menu/*.c` code matching.
- The 29 bare battle-model HRC IDs (`aaa aba aia …`) — confirm the battle-model
  ID set the engine touches, but again an asset fact.

## Bottom line

Keep the handover around for **symbol naming and module attribution**, surfaced
at two moments: when `src/field/field.c` is split (cross-check the `ad_*`
partition), and when battle `func_*` are renamed (use the
`yama`/`yasui`/`myoshiok` attribution). It will not unblock any
currently-failing function on its own.
