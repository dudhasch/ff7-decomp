# Picking work without the game data

`./mako.sh rank` is the right way to choose a function: it scores each one with a
model trained to predict one-shot decompilation success. It reads `asm/`, which
splat produces from `disks/us/SCUS_941.63`, so it only runs on a machine that has
the retail executable. A fresh clone does not — the game data is not distributed
with the source, and CI pulls it from a separate credentialed repository.

`tools/worklist.py` answers a coarser version of the same question from committed
files alone. It is a way to decide *where to look*; it is not a ranker, and it
never replaces `tools/checkfn.py` as the verdict.

```shell
tools/worklist.py --summary                          # per-unit progress
tools/worklist.py --exclude-unit psxsdk --limit 30   # the useful head of the queue
tools/worklist.py --unit ending --isolated --json
```

## What it measures, and why the sizes are exact

Every function in an overlay is either an `INCLUDE_ASM` stub or a C definition —
anything else would fail to link — so scanning `src/` recovers the complete
function list. Addresses come from the auto-generated names (`func_800A85FC`
carries its own) or from `config/symbols.*.txt`. All 1631 stubs resolve; only 8
of 1345 C definitions do not, all file-static helpers in already-matched
overlays.

A function's size is then the distance to the next function in the same overlay.
Two corrections matter:

* **Overlays share VRAM.** `battle`, `brom`, `dschange`, `ending`, `field` and
  `world` all start at `0x800A0000`, and the four menu overlays all start at
  `0x801D0000`. Addresses are only comparable within one overlay.
* **Units are not always adjacent.** `world` places a 10956-byte `asm` segment
  (`world_unk`) between its two `c` segments. Without clamping each function at
  its own segment boundary, the last function of unit `world` absorbs the whole
  blob and reports 2889 instructions instead of its real size.

With both in place the result is checkable against `config/us.yaml`: the function
sizes in each unit should sum to that unit's segment span. They do, for **all 20
units, exactly, byte for byte** — 18B8, 1255C, akao, psxsdk, battle, battle1,
battle2, battle3, brom, dschange, ending, field, bginmenu, cnfgmenu, savemenu,
title, itemmenu, world, world2, barrier. That is a complete accounting of every
byte of code in the game, so the sizes are not estimates. The only slack is
alignment padding after a unit's final function, which can inflate that one
function by a few bytes.

As a spot check, `func_801D080C` measures 1189 instructions where CLAUDE.md
records 1205. The 16-instruction difference is consistent with a 16-entry jump
table: `migrate_rodata_to_functions` puts a function's jump table in its own
`.s`, so counting lines there counts table entries as well as code.

## Where the work actually is

45.2% of functions are matched (1345 of 2976). 1631 remain.

| instructions | remaining | no decompiled caller |
| ---: | ---: | ---: |
| ≤10 | 208 | 105 |
| 11–20 | 169 | 113 |
| 21–40 | 265 | 187 |
| 41–80 | 382 | 249 |
| 81–160 | 299 | 206 |
| 161–320 | 188 | 126 |
| 321+ | 120 | 68 |

Sorting by size puts `psxsdk` at the top of almost every page, and that is a
trap. Read the next section before acting on it.

## psxsdk is not game code

`src/main/psxsdk.c` is 2.6% matched with 515 functions left, and 277 of them are
under 20 instructions — 73% of every tiny function in the project. It looks like
the obvious place to start. It is not.

The 14 functions matched there so far all sit between `0x80033B70` and
`0x80034B44`: game CD-ROM helpers that happen to land in this unit. None of the
actual PSY-Q library has been touched, and the library arrived as prebuilt
objects whose source Sony never shipped.

Worse, much of the small end is not expressible in C at all. Four dense runs of
uniform stubs:

| range | count | instructions | what they are |
| --- | ---: | --- | --- |
| `0x8003B53C` | 17 | 3–8 | libgte register moves — `SetVertex0`, `SetIR0`, `SetDQA`, `SetMAC123` |
| `0x80042990` | 35 | **all 4** | libapi BIOS trampolines — `FlushCache`, `OpenEvent`, `InitPAD`, `SetMem` |
| `0x80042D38` | 10 | **all 4** | libc BIOS trampolines — `printf`, `memcpy`, `strcmp`, `setjmp`, `rand` |
| `0x80046898` | 17 | **all 5** | libgpu primitive setup — `SetPolyF3`, `SetSprt`, `SetTile` |

Thirty-five consecutive functions of *exactly* four instructions each, carrying
libapi names, is the signature of a jump-to-BIOS trampoline table; the libc run
is the same shape. Those 45 were hand-written assembly in the original and have
no C form. The libgte run writes GTE registers via `mtc2`/`ctc2`, which plain C
cannot emit either. The libgpu run is the exception — poking `len` and `code`
into a primitive header is ordinary C, and those 17 are real targets.

This classification is inferred from name, size and spacing, not from reading the
assembly, so confirm with `asm/` before committing to any of it. The conservative
reading is enough to act on: treat `psxsdk` as a specialist area, and pass
`--exclude-unit psxsdk` for everyday work.

## The genuinely approachable set

Outside `psxsdk` there are 1116 functions left, 100 of them under 20 instructions
and 260 under 40. Filtering to the ones no matched code calls yet:

```shell
tools/worklist.py --exclude-unit psxsdk --isolated --max-size 160
```

gives 195 functions of ≤40 instructions with no decompiled caller — 83 of which
are not overlay-exported either, so nothing outside their own file can observe
them. They cluster in `world` (28 tiny), `18B8` (23), `akao` (15), `ending` (14)
and `1255C` (13). `ending` and `world2` are near-untouched (9.7% and 0%), and
`dschange` has both of its functions outstanding.

## What this does not tell you

The columns are size, call sites and export status. Nothing here knows about
register pressure, control flow, stack layout, or which compiler the translation
unit wants — the things that actually decide whether a function matches.

In particular it says nothing about `.rodata` ownership. A function that prints a
string another `.s` still owns cannot be decompiled alone, however small it is.
Run `tools/rodata_owner.py` before writing any C, and finish with
`tools/checkfn.py` and a green `make build`, exactly as CLAUDE.md requires.
