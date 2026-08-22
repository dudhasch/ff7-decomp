# CLAUDE.md

Guidance for AI agents working in this repository.

## What this project is

A matching decompilation of Final Fantasy VII (PS1, USA, `SCUS_941.63`). The goal
is C source that compiles to **byte-identical** machine code. "Looks correct" and
"behaves the same" are both failures — only bit-exact output counts.

**The one invariant:** `make build` ends with `sha1sum -c`, comparing every built
overlay against the SHA-1 of the retail executable. A green build means every
decompiled function matches exactly. There is no partial-credit state that passes.
Never claim a function is done without a green build.

## Running commands

On a host with a read-only root filesystem (SteamOS/Steam Deck, Silverblue, ...)
the toolchain lives in a container. Prefix every build command:

```shell
./tools/podman-build.sh 'make build'
./tools/podman-build.sh './mako.sh rank src/battle/battle.c'
```

On a normally-provisioned Linux host, run the commands directly. Everything below
is written unprefixed; add the prefix if `mipsel-linux-gnu-as` is not on PATH.
See [docs/decomp/STEAMOS.md](docs/decomp/STEAMOS.md).

`gh`-based commands (`make board`) run on the **host**, never in the container —
it has no credentials.

### Where `build/` and `.venv/` live in a container build

`tools/docker-build.sh` (and its untracked Windows twin `tools/docker-build.ps1`)
mount the repository at `/ff7` but keep two paths in **Docker named volumes**
rather than on the host bind mount:

| Path in container | Volume | Why |
| --- | --- | --- |
| `/ff7/.venv` | `ff7_venv` | The venv is baked into the image; the bind mount would otherwise hide it. |
| `/ff7/build` | `ff7_build` | Thousands of small object files, far too slow over a macOS/Windows bind mount. |
| `/gocache` | `go_cache` | Go module and build cache, shared across runs. |

The consequence: **`build/` does not exist on the host filesystem.** Anything
that reads it — `asm-differ`, `ninja`, `make report`'s `build/report.json` — must
run inside the container. `asm/`, `expected/`, `config/` and `src/` are on the
bind mount and are visible from both sides.

`tools/podman-build.sh` mounts only `/gocache`, so on that path `.venv` and
`build/` are ordinary host directories and `make requirements` must be run once.

To inspect or copy something out of a volume:

```shell
docker run --rm -v ff7_build:/build ff7-build:latest -lc 'ls /build/us'
```

Rebuild the image from a minimal context (just `Dockerfile` + `requirements.txt`
in a temp directory) — a build from the repository root uploads the ~750 MB disc
image in `disks/` as build context for no reason.

### On a Windows host

Every build command goes through the **PowerShell tool** running the untracked
`tools/docker-build.ps1`. None of the following is discoverable from a failure
message, and all of it has been re-learned from scratch more than once:

```powershell
Set-Location C:\ff7-daten-kopie\ff7-decomp; .\tools\docker-build.ps1 'make build'
```

* **Prefix `Set-Location <repo>;` every single time.** The PowerShell tool's
  working directory drifts between calls; a bare `.\tools\docker-build.ps1`
  eventually runs from somewhere else and fails with a path error that reads
  like a missing file.
* **Do not invoke `docker-build.ps1` through the Bash tool.** It dies with
  `execution of scripts is disabled on this system`. PowerShell tool only.
* **`git` runs on the host, never inside the container.** In the container it
  fails with `fatal: not a git repository: /ff7/C:/...` or, on a bind-mounted
  path that does resolve, commits as `Author identity unknown`.
* **PowerShell has no heredocs.** `git commit -m @'...'@` is a parse error
  (`Missing file specification`). Write the message to a scratchpad file and
  use `git commit -F <file>`.
* **Each `docker-build.ps1` invocation is a fresh container**, so `/tmp` does
  not survive between calls. Anything a later command needs must land under the
  bind-mounted repo or a named volume.
* **The Bash tool's `python` is the Windows interpreter**, not the container's.
  It writes CRLF and cp1252 by default, which corrupts sources and makes files
  churn in git. Scripted edits either run inside the container, or use the Edit
  tool, or pass `encoding="utf-8", newline="\n"` explicitly.
* **`file format not recognized` on an object during the link is a transient
  volume flake, not a source problem.** On Windows the build volume can serve
  the linker a partial view of an object ninja compiled moments earlier, and
  `make build` dies with

  ```
  mipsel-linux-gnu-ld: build/us/src/main/psxsdk.c.o: file not recognized: file format not recognized
  ```

  naming a file you did not touch. Re-running `make build` fixes it with no
  other change -- the object is a valid ELF on disk by then, so a sweep for
  truncated objects finds nothing and reads as a contradiction. It happened
  twice in one session, on `18B8.c.o` and `psxsdk.c.o`, while other work was
  running against the same volume. Do not go looking for a cause in `src/`.
* **One Docker build volume per worktree.** `build/`, `config/sym_export_*.txt`
  and `.ninja_*` are shared mutable state; two agents on one volume corrupt each
  other. Point each worktree at its own (`ff7_build_<name>`) — see
  *Working in parallel*.

## The decompilation loop

### 0. The budget, before anything else

Every function gets **three shaped attempts and one permuter run.** A shaped
attempt is a deliberate change with a stated hypothesis ("the target keeps the
value in a callee-saved register, so make it a local") — not a retry with the
operand order flipped and no theory. When the budget is spent, **park the
function and move to the next one**:

```c
#ifndef NON_MATCHINGS
INCLUDE_ASM("asm/us/field/nonmatchings/field", FieldButtonsUpdate);
#else
/* 2 rows: target keeps &D_8009D5A6 in a1, we get a0. Cached-address choice —
 * see the addr-form recipe. Permuter plateaued at 10 over 40k candidates. */
void FieldButtonsUpdate(void) { ... }
#endif
```

The note is the deliverable, not the failure. A parked function with the diff
rows and the rejected hypotheses written down is worth more to the next pass
than a function nobody wrote anything about.

**This rule exists because the history says so.** Across the sessions that
built this file, 38% of all `checkfn` invocations went to sixteen functions that
never landed; four of them — `FieldButtonsUpdate`, `func_801D080C`,
`OpcodeFuncStpls`, `OpcodeFuncLdpls` — absorbed 290 attempts between them and
were parked anyway. Throughput over the same period fell from 28 functions/hour
to 5 without the approach ever changing. Attempt eleven is not closer than
attempt four; it is the same guess with a different seed.

Two exits from the budget, both of which must be *earned*, not assumed:

* The diff shrank on this attempt. Shrinking rows means the hypothesis is
  live — keep going, the budget resets.
* The remaining rows are a documented idiom (see step 3's list). Then it is not
  a search, it is a lookup: apply it and check.

### 1. Pick a function

```shell
.venv/bin/python3 tools/worklist.py src/field/field.c -o docs/decomp/worklist-field.md
```

One list per `.c`, and the field overlay is five of them — `field.c`,
`field2.c` … `field5.c`. Regenerate all five, or you will keep re-picking a
function that lives in a unit you did not look at.

This is the start of every batch. It intersects the four things that otherwise
get re-derived by hand — and re-derived again after every compaction:

* remaining `INCLUDE_ASM` in the `.c` (**not** what `mako.sh rank` prints, which
  names every `.s` in the overlay including the ones already decompiled — a trap
  that has cost real time more than once)
* the handwritten screen: `.s` files marked `/* Handwritten function */` can
  never become matching C, and are not remaining work
* the `.rodata` verdict from `tools/rodata_owner.py` — BORROWS/LENDS functions
  are tabled separately as groups, not as individual picks
* a static cost proxy (instruction count, call count, indirect calls, division,
  jump table) that orders the rest cheapest-first

Functions already carrying a parked near-miss body are marked `P` and sort to
the top: a written hypothesis is a head start, and finishing them is the
cheapest work available.

**Check the pick is a function at all before budgeting for it.** splat takes
`type:func` in a `config/symbols.*.txt` line as fact, so one hand-written line
can put a block of data on the work list forever, dressed as an `INCLUDE_ASM`
of plausible size. `func_801D3260` was the last "function" in
`src/menu/itemmenu.c` and is 1492 bytes of FF7-encoded dialogue plus a
per-item sort table -- every line of its `.s` is a `.word` marked
`/* invalid instruction */`, and `func_801D0E80` loads its address with
`lui`/`addiu` rather than calling it. `grep -c '\.word' <target.s>` against the
file's line count answers it in one command. The fix is a config change, not
a decompilation: move the `data` subsegment boundary in `config/us.yaml` down
to the region's start and rename the symbol out of `func_` -- `make build`
stays green because the bytes and their order do not move, and splat rewrites
the callers' `.s` with the new name for free.

**`src/main/psxsdk.c` is fenced, and the fence is a comment in the middle of
the file that no tool reads.** At the head of the SDK region it says

```c
// NOTE: please do not decompile any of these functions.
// Please refer to psyz/decomp for decompiled PSX SDK functions:
// https://github.com/Xeeynamo/psyz/tree/main/decomp
```

with no closing delimiter, and **417 of the unit's 459 remaining
`INCLUDE_ASM` lines sit below it** -- everything from `_SpuInit` to the end of
the file, i.e. all of libspu, libgpu, libetc, libapi, libcd, libc and the
kernel stubs. `worklist.py` reports the unit as 432 actionable functions and
knows nothing about it. Only the 42 above the note (FF7's own CD-streaming and
LZS code, `func_80033BE0` through `ChangeClearSIO`) are unambiguously in
scope. Confirm with whoever owns the repo before spending a session below the
line; the note reads as either "leave these alone" or "import them from psyz
rather than re-deriving them", and both answers make hand-decompiling them
the wrong work.

**A library unit's actionable count is inflated by things that can never be
C, and `worklist.py`'s handwritten screen does not catch them.** splat marks
`/* Handwritten function */` on its own heuristics, and in `psxsdk.c` it marks
83 of 515 while **54 more are BIOS syscall stubs** --
`addiu $t2, $zero, 0xA0 / jr $t2 / addiu $t1, $zero, N`, which is a jump
through a register and has no C spelling -- and **16 more are bare COP2
wrappers** (`AverageSZ4` is one `avsz4` instruction; `SetVertex0` is two
`lwc2`). Those 70 sort to the very top of the cheapest-first list, because
they are one to four instructions long. Screen them before picking:

```shell
grep -lE 'jr +\$t[0-9]' asm/us/<ovl>/nonmatchings/<unit>/*.s      # syscall stubs
grep -lE 'lwc2|swc2|ctc2|cfc2|mtc2|mfc2|avsz|rtps|nclip' .../*.s  # COP2 wrappers
```

**`P` says a note exists, not how close it is -- run `parked_queue.py` for
that.** The row count written into a park note is true of the moment it was
written and of nothing after: anything that changes the function's own body,
its callees' prototypes, or a struct it reads moves it, and a stale number is
indistinguishable from a fresh one in the text.

```shell
.venv/bin/python3 tools/parked_queue.py 'src/field/field*.c' --jobs 4
#   HandleKawaiDataInModel   2
#   OpcodeFuncFadew          2
#   FieldEntityTriggerCheck  9
#   ...
```

It hands `variant_eval.py` a no-op edit per parked body, so each
score is that body's own, measured with every guard `variant_eval` already
has -- the target unparked alone, diagnostics fatal, aliases discounted. This
matters more than it sounds: a queue ordered by remembered numbers sent a full
budget at a function its note called "-2 instructions" and the tool measured at
110 rows, while the two functions actually two rows from matching sat eight
places further down. Regenerate it at the start of a batch, next to
`worklist.py`, and read it with the length caveat -- rows only compare between
bodies of the same length.

Then cross-check the pick with the trained model, which weighs different
signals:

```shell
./mako.sh rank src/battle/battle.c
# 0.355: func_800A85FC.s     <- lowest score first
```

The score is a difficulty model (0 = easy, 1 = hard) trained to predict one-shot
decompilation success. **Always work in ascending score order.** Do not start at
the top of the file; do not pick by name. Pass a saved copy of this output to
`worklist.py --rank` to get both judgements in one table.

Work in **file order** within a batch where you can: a function whose compiled
size changes shifts every later function's branch targets, so a downstream
verdict is only trustworthy once everything before it matches.

**The symptom of that is a mismatch whose rows are *only* branch targets, all
off by the same constant — and it means the function is finished.**
`diff.py` renders a branch's destination as an absolute offset into the object,
so a function sitting N bytes from where the target puts it reports every
internal branch, every `j` and every loop back edge as a changed row while
being byte-identical instruction for instruction. Landing 22 functions in one
pass of `src/main/akao.c` produced eleven such verdicts at once, eight of them
reading `1 changed`, and every one cleared with no source change when the one
genuinely wrong function ahead of them was fixed. Read the *operands*: if the
only difference on every row is the hex address after a `beqz`/`j`/`bnez` and
the deltas all agree, do not touch the function — go fix whichever earlier
function `checkfn` reports a length for.

### 2. Seed with m2c

```shell
./mako.sh dec func_800A85FC              # add --fix-structs for Savemap accesses
```

This replaces the function's `INCLUDE_ASM(...)` line in the `.c` with approximate
decompiled C, using the project's existing types and symbol names. It is a
starting point, not an answer — it will usually compile but rarely match.

**One missing prototype poisons every later `mako.sh dec` in the unit, not
just the function that needs it.** When m2c cannot type a callee it writes
`/*?*/ void func_800A4F60(u8, ?);` into the `.c`, and `?` is not C — so the
*next* run fails at the C-context parse with

```
Decompilation failure:
Syntax error when parsing C context.
before: ? at line 2713, column 26
      void func_800A4F60(u8, ?);
```

naming a line number in a preprocessed file and a function that has nothing to
do with the one you asked for. Three separate batches in
`src/battle/battle3.c` died this way. The fix is always the same: declare the
callee properly in the overlay's private header and re-run the whole batch.
Read the parameter types off the callee's own prologue — `andi $a0,$a0,0xff`
is a `u8` parameter, `sll`/`sra 16` is `s16`, a plain `slti $v0,$a0,N` with no
mask is `s32`. Doing it up front for every `jal` target the unit has is a few
minutes and it is the cheapest work in a fresh file:

```shell
grep -ho 'jal *\w*' asm/us/<ovl>/nonmatchings/<unit>/*.s | sort -u
```

**And the declaration does not always belong in `game.h`.** A main-overlay
function that main *defines* with a different signature than the callers use
is a `conflicting types` error the moment `game.h` is included by both units,
and gcc 2.6.3/2.7.2 keeps generating code afterwards (see *A compile error
that ninja calls a success*). Two in `src/battle/battle3.c`:
`func_80026B5C` is `void func_80026B5C(void) {}` in `1255C.c` while every
battle caller passes an argument, and `func_8001DE0C` is defined in `18B8.c`
against `main_private.h`'s `Unk8001DE0C`, which the battle overlay cannot see.
Declare those in `battle_private.h` (redeclaring the struct locally if you
must) and leave `game.h` for the ones that agree.

### 3. Iterate against the assembly

Rebuild the one object and get a verdict:

```shell
.venv/bin/python3 tools/checkfn.py src/battle/battle.c func_800A85FC
# MATCH    func_800A85FC  (6 symbol aliases)
```

`checkfn.py` rebuilds the object, then compares only the instructions the target
`.s` actually declares, discounting differences that are purely a symbol *name*
(the `.s` says `D_800722C4`, your C says `g_CurrentEntity`; same address, same
bytes). It exits non-zero unless every function named matched. Pass `--rows`
and it prints *every* differing instruction rather than just the first, with
the alias rows already filtered out — which is what you want on a near-miss,
since separating the real rows from the aliases in `diff.py` output by eye is
the slow half of reading a diff (`FieldEventRequest`: 3 real rows against 95
aliases). Prefer it to
reading `diff.py` by eye — see *Twelve ways a clean-looking diff lies* below.

To look at the actual instructions once it reports a mismatch:

```shell
ninja build/us/src/battle/battle.c.o
.venv/bin/python3 tools/asm-differ/diff.py -o --format=plain func_800A85FC
```

Use `--format=json` when you need to parse the result programmatically; it gives
per-row diff classes (register mismatches, insertions, reorderings) rather than
text you have to scrape.

Repeat edit → `checkfn` until it reports MATCH. Typical causes of
a near-miss, in rough order of frequency:

* **Register allocation differs** — usually means a temporary should (or should
  not) be a local variable, or the operand order in an expression is reversed.
* **Instruction order differs** — restructure the C (hoist/sink an assignment,
  change `if`/`else` polarity) rather than fighting the compiler.
* **Stack layout differs** — a local's type or declaration order is wrong.
* **Load/store order differs** — gcc 2.6.3 only keeps a load and a store in
  source order when *both* sides are struct references. Reading through a
  parameter pointer and writing to standalone `extern` scalars lets it batch the
  loads ahead of the stores, which changes the instruction count. Writing the
  destination as members of the real struct object fixes it — this is what
  `FieldEntityGatewayMapLoad` needs to write through `FieldState` at
  `0x8009ABF4` rather than through six separate `D_` symbols.
* **One extra `move` into a callee-saved register, at the top of a loop** —
  the target computes a base address into a caller-saved temp and then copies
  it (`addu t3,t3,a3` / `move s0,t3`), where a hand-written walking pointer
  computes straight into the callee-saved register and is one instruction
  short. That copy is gcc *strength-reducing an indexed access*: the base is
  dead after loop setup, so it stays in a temp, and the induction variable is
  initialised from it. Write the access as `&parts[i * 32]` and let gcc create
  the walking pointer, rather than writing `parts += 0x20` yourself. This is
  what `FieldModelCreatePktsAndScale` and `KawaiSetColorToModelPkts` need.
  When the value *stored* is itself the loop counter, the reduction goes the
  other way — gcc walks a pointer for the address and you have to write that
  pointer by hand (`OpcodeFuncMhmmx`'s first store loop).
* **One variable across several sequential loops, not one per loop.** Where a
  function walks the same thing more than once — a grid built, then copied,
  then transformed — the original usually reuses a single counter, and that is
  not a style detail. gcc 2.6.3 allocates one pseudo per variable and orders
  allocation by roughly `refs / live_length`, so merging three loops' counters
  into one variable sums their reference counts and wins the pseudo a
  callee-saved register early; writing three variables splits the count three
  ways and gcc spills the one that loses, adding a `lw`/`addiu`/`sw` to every
  iteration. `func_801B063C` in `src/magic/escape.c` matched on exactly this:
  one `row` serving the ripple loop, the corner-copy loop and the tile loop.
  The tell is a loop counter living on the stack in your build and in a
  register in the target, while the register counts are otherwise equal.
* **But a bound *derived* differently is a different variable, however
  identical the loops look.** The rule above is about counters that describe
  one walk; a bound computed from an unrelated quantity is not one of them.
  `LoadLocalFieldModelAndInitAll` in `src/field/field2.c` reused one `s32 n`
  for the word copy's `words / 4` and for each of the three record loops'
  `rec->boneCount`/`partCount`/`animCount` — which reads as exactly this idiom
  and is not. The target keeps the copy count in `$t1` where the merged
  variable lands in `$t3`, and giving the copy loop its own `nw` is worth
  4 rows; splitting the three record counts instead is 49 rows *worse*, and
  splitting the copy loops' shared `w` is 10 worse. The partition is real and
  has to be read off the target rather than applied uniformly, and the
  cheapest reading is which of the loops' counts share a register there.
* **Put a call first in a sum: `f() + expr`, not `expr + f()`.** Written second,
  the call forces `expr` to be computed *before* it and to survive it, so the
  value needs a callee-saved register or a spill slot; written first, `expr` is
  computed afterwards into a caller-saved temp. Same arithmetic, two
  instructions apart per statement — `(rand() & 3) + (col - 21) * 2` is what
  `func_801B009C` needs, twice.
* **Fill a struct in field order, not in the order the stores come out.** The
  scheduler moves stores; reading them back out of the `.s` gives you an order
  no one wrote, and writing that order down reproduces a *different* schedule.
  `EscapeCaptureScreen` sets two `RECT`s and the target's stores read x, w, h,
  y — so the C said x, w, h, y and sat 16 rows out. The give-away was a load in
  the wrong place: the target reads `DispY` *before* the w/h stores, using them
  to cover its load-delay slot, and fills the earlier slot with the `move
  a2,zero` that sets up `MoveImage`'s third argument. Written in the order a
  person would — x, y, w, h — the read lands where the target has it, gcc sinks
  the y stores past the w/h stores by itself, and the function matches. When a
  diff is "all scheduling", check whether the source order is one the compiler
  invented.
* **Two allocator knobs that do nothing, so stop reaching for them.** Neither
  the order of local declarations (all five permutations tried on
  `func_801B009C`) nor `register` on any subset of them changes gcc 2.6.3's
  allocation by a single instruction. And a hand-carried accumulator — `x = -42`
  before the loop, `x += 2` in the body — does not buy you a spill slot: gcc
  treats it as a basic induction variable, gives it a *register*, and spills
  something else instead. There is no way to spell "keep this on the stack" in C.
* **Two spilled induction variables holding each other's slot: read cc1's RTL,
  do not guess.** Which stack slot each reduced giv of a loop gets is the order
  `strength_reduce` creates their new pseudos, and that is the **reverse** of
  the order it discovers them — reverse insn order in the loop body. You can
  see it directly rather than inferring it from a diff:

  ```shell
  mipsel-linux-gnu-cpp <the flags from `ninja -t commands`> src/magic/escape.c \
    | bin/str | iconv -f UTF-8 -t Shift-JIS > /tmp/e.i
  bin/cc1-psx-26 -quiet -mcpu=3000 -mgas -O2 -G0 -dumpbase dumps/e.c -dL /tmp/e.i -o /dev/null
  ```

  The `.loop` dump ends each loop's preheader with one `(set (reg:SI N) <init>)`
  per reduced giv; `N` ascending is the slot order. (`-dL` is loop, `-dl` is
  local-alloc — the lowercase one silently gives you the wrong dump. `-dg` adds
  the post-global-alloc RTL.) The consequence for the C: a value whose defining
  insn is at the top of the loop is discovered *first* and so numbered *last*.
  To pull it to a lower slot its def has to sit after the other giv's — which is
  impossible when its use dominates, and that is what parks `func_801B009C`.
  Two escapes, each costing one extra slot: defining it late with a pre-loop
  init keeps the original pseudo alive beside the giv, and hoisting the *other*
  value into a row-top local works only if you spell it so gcc does not see a
  shared subexpression — `(row + 1) * 8`, never `row * 8 + 8`, or `row * 8`
  becomes a third giv of its own.
* **A value the target keeps in a spill slot wants to be a named local,
  computed early and used late.** gcc 2.6.3 only strength-reduces an expression
  over a loop counter when the value has a live range to speak of. Written
  inline — `Vel.vx = (rand() & 3) + (col - 21) * 2;` — the product is computed
  and consumed inside one statement and stays an `addiu`/`sll`/`addu` triple in
  the loop body, in every spelling. Hoisted to the top of the loop as
  `vx = (col - 21) * 2;` it becomes an induction variable with its own stack
  slot: `li t0,-42` in the preheader, `+= 2` at the loop bottom, `lw`/`addu` at
  the use. That is a two-instruction-per-iteration difference, and it is what
  `func_801B009C` in `src/magic/escape.c` needs for both `(col - 21) * 2` and
  `(row - 16) * 2`. This is the opposite of the usual reflex: naming the
  temporary is what the compiler wanted. It is also not the same thing as the
  accumulator in the bullet above — an accumulator is a *biv*, gets a register,
  and lands in a different slot.
* **Which sibling address expression is evaluated first decides whether gcc
  reduces any of them.** Four stores of `&EscapeGrid[row][col + d]` for
  d = 0, 1, 41, 42: with the `d = 0` form evaluated first, gcc builds a giv
  `&EscapeGrid + col * 12`, spills it and reloads it once per iteration —
  duplicating the `col * 12` giv it already has and costing a whole extra stack
  slot. With the `d = 1` form evaluated first no giv forms at all and every
  corner is computed from the two multiples plus a rematerialised base, which
  is what the target does. Nothing else moved it: a named `EscapeCell *`,
  `EscapeGrid[row] + col`, `[row + 0][col + 0]`, a walked `c++` pointer and a
  flat `EscapeGrid + row * 41 + col` were all measured, and the ones that avoid
  the giv do it by collapsing the row and column multiples into one flat giv,
  which costs more than it saves. Evaluate the sibling into a temporary if the
  *store* order has to stay 0, 1, 2, 3 — the compiler keeps stores in source
  order, so the temporary is the only way to have both.
* **`(x & 7) << 5` narrows the loads, `(x & 7) * 32` does not.** With the
  shift, combine's `force_to_mode` pushes the 3-bit mask back through the
  `plus` and into the `mem`s, so two `s16` fields load as `lbu`/`lbu` (or
  `lhu` if an `s16` temp holds the sum). Written as a multiply, the mask never
  reaches the loads and they stay `lh`/`lh` — which is what the target does in
  `TearsRenderDrop`'s texture-page index. The two forms are the same
  arithmetic; only the operator spelling decides the load width.
* **`x % n` on an `s16` sign-extends the result** — gcc 2.6.3 does the modulo
  in `HImode` and adds a `sll`/`sra` pair to widen it back. Write it as
  `value - quotient * n` with an `s32` quotient. Hoist both the quotient and
  the remainder into locals ahead of an `if`/`else` that uses them, or gcc
  duplicates the division into both arms (`OpcodeFuncIdlck`).
* **Before blaming register allocation, read the target's opcodes for the
  types.** Three of them are declarations in disguise, and each is worth tens
  of rows that all look like allocation noise:
  * `lw` on a symbol where you wrote `addiu` means the symbol is a **pointer
    to** the thing, not the thing. `D_800E4274` in `src/field/field2.c` is a
    `s16*` holding the walk mesh address; declaring it `s16[]` cost 27
    instructions and a 64-byte frame overrun before the `lw` gave it away.
    Run the other way it types a *callback*: `addiu a1,a1,%lo(D_800A00CC)`
    where your build has `lw a1,%lo(D_800A00CC)(a1)` means the symbol **is**
    the entry point, `extern void D_800A00CC(void);`, not an
    `extern void (*D_800A00CC)(void);` the caller dereferences. Two sites in
    `func_800146A4` in `src/main/18B8.c`, worth two rows and no length; the
    give-away that the declaration is wrong rather than the expression is that
    `config/sym_ovl_export.us.txt` names the same address `func_800A00CC`.
  * `lhu` against your `lh` on a struct field means the field is unsigned
    (`lbu`/`lb` likewise). `FieldEntity.PosI` was declared `s16` and every
    read in the target is `lhu`.
  * `blez` on a loop guard cannot come from an unsigned bound, and `beqz`
    cannot come from a plainly signed one — so the guard alone types the
    bound. `FieldState.modelCount` was declared `u16` and the target's guard
    is `blez`.
  Fix the declaration, not the expression: a cast at the use site produces
  `lh` plus an `andi`, which is a third wrong answer. Check who else uses the
  field first — both of the above had few enough users to retype outright,
  and `make build` is the arbiter.

  **Before retyping anything, check that the *expression* is not the reason.**
  The load opcode for a global is not decided by its declaration alone:
  `D_80162080++` on an `extern s16` compiles to `lhu`, because only the low
  16 bits reach the `sh` and combine's `force_to_mode` narrows the load, while
  `D_80162080 == 0` on the same declaration keeps `lh`. So one unit can want
  `lhu` at one site and `lh` at another with a single signed declaration, and
  an `lhu` in the target is *not* by itself evidence that a scalar global is
  unsigned. Retyping it to `u16` matched the function under test and silently
  broke a sibling that had been matching for months.

  That failure mode is worth as much as the rule, because nothing in the
  per-function tooling can see it: `checkfn` and `variant_eval` are both
  scoped to the function you name, so a header retype that fixes yours and
  breaks a neighbour reads as MATCH on both runs. `make build` fails with
  every object still exactly the right *size*, which reads like a link
  problem rather than a codegen one. The two commands that find it in under a
  minute:

  ```shell
  for f in build/us/src/<ovl>/*.o; do cmp -s $f expected/$f || echo $f; done
  mipsel-linux-gnu-objdump -dr <ours> | grep -vE '^$|<LM[0-9]+>:' > /tmp/a
  mipsel-linux-gnu-objdump -dr <expected> | grep -vE '^$|<LM[0-9]+>:' > /tmp/b
  diff /tmp/a /tmp/b
  ```

  Strip the `LM` debug labels: `-g -gcoff` emits one every few instructions in
  a *C* function and none in an assembled `.s`, so an unfiltered diff of a
  half-decompiled unit is thousands of lines of nothing. What is left is one
  row per real difference, with the relocation naming the symbol.
* **`x < 0` gives `srl x,31` through an `s32` local and `slti x,0` through a
  `u8` one.** `do_store_flag` has a special case for a sign test against zero
  and emits the logical shift whenever the destination is the comparison's own
  mode; a QImode destination is not, so the shift path is skipped and the
  ordinary `slti` survives. Both are one instruction and both feed a following
  `sll` identically, so the diff is a single row that reads like an
  optimisation you cannot spell -- and the fix is the local's width, not the
  comparison's spelling. `0 > v`, `(v < 0) ? 1 : 0`, `neg * 8` and `s16`/`u32`
  locals were all measured and are the `srl` form; only `u8` reaches `slti`.
  `func_800BA360` in `src/battle/battle1.c` matched on that one word.
* **A `u16` parameter gets no entry mask when a prototype is in scope, so an
  `andi 0xffff` at the *use* is the source's own `& 0xFFFF`.** With
  `PROMOTE_PROTOTYPES` gcc trusts the caller to have extended the argument and
  `assign_parms` emits nothing, so the entry-conversion rule ("`move` cannot
  come from a `u8` parameter") only holds for the *un*prototyped case, which
  is not how this codebase compiles. Read the parameter's width off the
  **callers** instead: a caller loading the argument with `lhu` passes it to a
  `u16` parameter and one loading `lh` to an `s32`, and the callee's own body
  cannot tell you which. `func_800BBA84`'s `s32` reading matched the function
  under test perfectly and moved two `lhu` to `lh` in `battle2.c`, which is
  only visible in `make build` -- see the objdump recipe above.
* **A range test written with `&&` is folded; nested `if`s are not.**
  `x < 6 && x >= 4` becomes `(unsigned)(x - 4) < 2` — one instruction where
  the target has two `slti` against the same register. `fold_range_test` only
  fires on the `TRUTH_ANDIF` node, so writing the two comparisons as nested
  `if`s with a shared fallthrough produces the target's pair. The tell is a
  lone `addiu <reg>,<reg>,-N` feeding an `sltiu` where the target compares
  twice. `OpcodeFuncLader` in `src/field/field4.c` needs this.
* **A `u8` field compared inline gives `sltiu`; through an `s32` local it
  gives `slti`.** Read into the comparison directly, combine folds the
  `zero_extend` into the compare, proves the sign bit clear, and switches to
  the unsigned opcode. Assign it to a signed local first and the extension
  sits in its own insn, so the comparison stays signed. Same load (`lbu`
  either way), same constant, different opcode — so when the only rows left
  are `slti` against `sltiu`, the fix is a local, not a cast. A cast changes
  the load instead and makes it worse.
* **A block reached by both a failed test and a `switch` default is a
  fallthrough, not an `else`.** When every arm of the switch returns, the
  `else` form looks equivalent and reads better, but it inverts the guard
  branch ahead of it and gcc lays the blocks out in the other order —
  `OpcodeFuncLader` measured ten instructions apart on this alone. Follow the
  target's branch polarity: `bne` to the body means the guard is spelled as
  an early `return`, `beq` means an `if` wrapping everything. Duplicated tail
  statements after such a guard are fine; cross-jumping merges them, which is
  what the target's shared `j` into the common epilogue already shows.
* **But a `&&` guard over a `goto` is *not* one of those: its polarity is
  fixed after the front end, and reasoning from `do_jump` will send you to
  five byte-identical bodies.** `do_jump` threads an `if_true_label` and an
  `if_false_label` through `TRUTH_ANDIF`, and the shape a target with
  `bgez <fast> / j <slow>` and both delay slots empty appears to want — the
  last comparison emitted with *both* labels non-null — is reachable in theory
  only from a negated guard whose body is the slow path, with no label at all
  on the fast one. It is not reachable in practice: on
  `FieldEntityWalkmechCross` the `goto done` form, `!(a >= 0 && b >= 0 &&
  c >= 0)` with the chain as the if-body and no `done:`, the `a < 0 || b < 0
  || c < 0` form of the same structure, `!(a) || !(b) || !(c)`, and a
  `goto chain; goto done; chain:` inversion are **all byte-identical**.
  `jump_optimize` normalises block layout across every one of them, so the
  guard's *structure* is exactly as inert as this file already records its
  *condition* to be. The `OpcodeFuncLader` bullet above is about a `switch`
  default's block order, which `expand_end_case` fixes at expand time; do not
  generalise it to an ordinary conditional.
* **A loop bound as `s16` buys a `move` that `u16` folds away.** For
  `for (i = 0; i < count; i++)` with `count` assigned from a `lbu` plus a
  constant, gcc knows the value is small and non-negative, so the widening to
  `int` collapses to a plain register copy and the zero-trip guard becomes
  `beqz` rather than `blez`. Declared `u16` there is no widening node at all
  and no copy is emitted. The two spellings therefore differ by exactly the
  `move a0,s4` / `move t3,a0` pair a target may or may not have, and *neither
  is the house style* — `OpcodeFuncAdpal` and `OpcodeFuncMppal2` in
  `src/field/field4.c` have identical loop shapes and want opposite types.
  Read the guard in the target: `blez` means the bound is plainly signed,
  `beqz` plus a copy means `s16`, `beqz` with no copy means `u16`.
* **`x * 2 * invariant` reassociates and hoists; `(x << 1) * invariant` does
  not.** gcc folds the constant onto the loop-invariant operand and lifts
  `invariant * 2` into the preheader, so a per-iteration `sll` in the target
  becomes a preheader `sll` plus a changed multiply — three rows for what
  reads as the same arithmetic. Spelling the doubling as a shift keeps it in
  the body. The three MPPAL opcodes need this.
* **Read a bitfield extraction off the target, do not derive it.** A channel
  that is doubled before use is spelled with the doubling folded into the
  shift and a mask one bit wider: `(color << 1) & 0x3E`, `(color >> 4) & 0x3F`,
  `(color >> 9) & 0x3F` — *not* `((color >> 5) & 0x1F) * 2`, which is a
  genuinely different value, not just a different phrasing. Deriving the
  "obvious" form gives correct-looking C that is two instructions out per
  channel and, for the middle channels, semantically wrong.
* **Which of two pointers is declared first decides which base the target
  computes first.** For a loop copying between two bases, declaring the load
  base before the store base (or the reverse) is worth a dozen rows of
  register naming, and the answer is not consistent even between siblings:
  in `src/field/field4.c` the ADPALs and MPPALs compute the load base first,
  the RTPALs the store base. Find which base feeds the `lhu` in the target
  and declare that one first.
* **A loop bound that lives in memory has to be a local.** `for (i = 0; i <
  src->partCount; i++) { d[k] = s[k]; ... }` reloads `partCount` on every
  iteration — the stores may alias it, and gcc 2.6.3 does no aliasing
  analysis worth the name, so the bound is re-read after each one. The
  target loads it once into a register before the loop, which is what
  `count = src->partCount;` gives. The tell is an `lbu`/`lhu` of a struct
  field appearing *inside* the loop body in your build and only in the
  preheader in the target. `FieldModelLoadBcx` had three of these and they
  were worth 20 rows.
* **Copy an unrolled record with `d[i * 8 + k]`, never with `d += 8`.** Both
  spell the same walk, but the pointer form makes `d` and `s` bivs, and gcc
  then strength-reduces the eight element addresses onto a *second* base
  register — `addiu a3,a1,0x18` / `addiu v1,a0,0x18` in the preheader, with
  the body reaching back through negative offsets. It costs two instructions
  per loop and it is invisible in the body, which still looks perfect. Which
  element becomes the new base is a giveaway that the mechanism is giv
  benefit counting: whichever address is referenced most (the field that is
  copied and then fixed up) or, failing a tie-break, the last one. Indexing
  off the counter leaves the addresses as `biv + constant`, which gcc leaves
  alone. This is the same lever as the `&parts[i * 32]` bullet above; here it
  was worth four rows per loop across four loops in `FieldModelLoadBcx`.
* **A folded range test in front of a `switch` deletes the switch's own
  bounds check.** `if (id >= 1 && id <= 9)` becomes `(u32)(id - 1) < 9`
  through `fold_range_test` — bit-for-bit the comparison `do_tablejump`
  emits for cases 1..9 — and cse, which follows the fall-through path out of
  a conditional branch and records its outcome, folds the second copy to a
  constant and drops it. The tell is unmistakable and otherwise
  inexplicable: code that belongs *before* the switch (here `if (unk6 ==
  0)`) sits between the range check and the `jr` through the jump table.
  Nothing you do to the switch alone can produce that, because
  `expand_end_case` emits the range check and the tablejump as one unit.
  Note this is the exact inverse of the `OpcodeFuncLader` bullet, where the
  fold had to be *defeated* with nested `if`s — read the target for which
  one it wants.
* **An argument to a narrow parameter needs a wide local, or combine folds the
  constant into the wrong mode.** `func(i | 0xC600)` where the prototype takes
  `u16` lets `force_to_mode` do the `ior` in HImode, and an HImode `const_int`
  is *sign-extended*: 0xC600 becomes -0x3a00, which is no longer a legal `ori`
  immediate. gcc then materialises it with `li` into a register, hoists that
  out of the loop as an invariant, and — because the loop already uses one
  callee-saved register — grows the frame. What you see in the diff is a
  changed stack size and a couple of `sw`/`lw` in the prologue, which reads
  like ordinary allocation noise and sends you looking at the wrong loop. The
  fix is an `s32` local: `itemId = i | 0xC600; func(itemId);` keeps the `ior`
  in SImode and the constant as a plain `ori`. A `u16` local does **not** work
  — it is the same HImode trap. `OpcodeFuncSpcal`'s two inventory loops need
  this, and it was worth four rows and the whole frame layout.
* **A loop whose exit test is duplicated at the bottom wants to be a backward
  `goto`.** `loop.c`'s `duplicate_loop_exit_test` copies the loop's first
  conditional jump to the end, turning a top-test loop into a rotated one — so
  your build has the test twice (once in the preheader, once on the back edge)
  where the target reaches a single top test through a plain `j`. Both `while
  (cond) { ... }` and `for (;;) { if (cond) break; ... }` get rotated; a
  backward `goto` does not, because gcc emits no NOTE_INSN_LOOP_BEG for it and
  `loop_optimize` never sees a loop there. The cost is two rows per loop plus
  whatever the changed register pressure does downstream —
  `FieldBackgroundInitPackets`' four run walks measured 43 rows as `while`, 38
  as `for (;;)` and 25 as `goto`. The same absence of a loop note also means no
  invariant hoisting and no strength reduction inside that loop, so check the
  inner loops still look right after the change; here the real loops are the
  inner `do`/`while`s and only the outer walks are gotos.

  **Read the absence of hoisting as the tell, not the duplicated test, and the
  whole of `src/battle/battle1.c` becomes cheap.** The rotation is two rows;
  the loop *notes* are worth tens, because without them there is no invariant
  hoisting, no strength reduction and no giv. So a counted walk whose target
  rebuilds a symbol's address at every iteration, re-reads a bound that is
  obviously invariant, keeps an `sll`/`sra` conversion inside the body, or
  computes `i * stride` per iteration where you emit a walking pointer, is a
  backward `goto` -- and *all four* of those show up together, which makes it
  easy to recognise from one reading of the diff. Three functions in
  `src/battle/battle1.c` measured 39 -> 14, 32 -> 27 and 29 -> 27 rows on the
  spelling alone, and `func_800B45F0` went on to match. The corollary is that
  the usual advice about `for` increments does not apply in such a loop: with
  no loop note there are no bivs and no givs, so every counter is an ordinary
  variable and the increments come out in written order wherever you put them.

  In a goto walk the one lever left is **which symbol addresses are
  materialised before the loop and in what order**, since nothing will be
  moved for you. A named `u8* base = (u8*)SYM;` assigned *between* the two
  counters' initialisations and the arithmetic that uses it is what took
  `func_800B45F0` from 14 rows to a match: written inside the pointer
  expression instead, the `lui`/`addiu` is emitted among the multiplies rather
  than ahead of them, and every register downstream renames.

  **Run the other way -- the *target* has the test twice and you have one --
  no spelling of the loop reaches it, because the transformation has a
  precondition on the test's contents.** `duplicate_loop_exit_test` starts
  from `next_nonnote_insn (loop_start)` and gives up unless that insn is
  already the exit `JUMP_INSN`, so it fires only on a loop whose guard needs
  no set-up: everything the test computes must be live before
  `NOTE_INSN_LOOP_BEG`. A guard that reloads two globals and subtracts, like
  `FieldDebugRenderPage`'s `while (y < PageY[page] + PageH[page] - 8)`,
  leaves those loads at the top of the loop and the pass never runs, in every
  spelling -- `while`, `for(;;){break}`, `for(;;){goto after}` and a backward
  `goto` measured 758, 758, 762 and 759 rows there. So a `j +N / beqz -N`
  imbalance in `insn_histogram.py` is a statement about *what the guard
  recomputes*, not about how the loop is written, and the loop-shape sweep
  that seems to be indicated is a whole dimension of nothing.
* **A byte offset assigned once and used across the whole function is a
  global allocno; the target's caller-saved register says it was several
  variables.** The `$at` addressing idiom above leaves the scaled offset in a
  register at every access, so which register it gets is repeated on every
  row of the diff -- `addu at,at,s1` against the target's `addu at,at,t7`,
  ninety times. `$t7` is caller-saved, which `global_alloc` never hands to a
  value that spans basic blocks, so the original had a *block-local* offset:
  the same `page * 0x17A` written into a fresh local at each block head. One
  variable assigned six times is still one pseudo and still global, so
  re-assigning it more often does not help -- the locals have to be
  different. Which of the assignments want their own is not derivable and not
  uniform: all 32 subsets of `FieldDebugRenderPage`'s five re-assignments
  were scored and they range over 753 to 802 rows and -4 to +3 instructions,
  with the best at the exact length being a three-of-five split whose
  neighbours in the table are 5 and 8 rows worse. Sweep the subset; it is one
  batch and the reasoning does not narrow it.
* **`f(x, cond ? 1 : 0)` folds to a shift; the target's branch means two
  calls.** `SetSemiTrans(p, (tile->flags & 0x80) ? 1 : 0)` compiles to
  `srl a1,a1,0x7` — one instruction, no branch. Where the target tests the bit
  and branches, the source called the function twice from an `if`/`else` and
  cross-jumping merged the two `jal`s into one, leaving only the two argument
  setups. The tell is a pair of blocks that each load the same `a0` and a
  different `a1` before a shared call.
* **A loop-invariant constant is hoisted only if its defining insn is on the
  loop's always-executed path.** gcc 2.6.3's `move_movables` will not lift a
  constant whose only uses sit inside a conditional arm, so
  `if (x == 0) { p[0] = 2; p[1] = 2; }` keeps its `li v0,2` in the loop (usually
  in a branch delay slot, so it costs nothing visible) while a constant whose
  first use is the loop's unconditional test — `if (q[i].flag == 1)` — is lifted
  into a callee-saved register. When the target hoists one the C does not, the
  frame grows by one saved register and *every* s-register is renumbered: 55
  rows of pure renaming from one missing `li`. Assigning the constant to a local
  at the top of the loop body is what makes it hoistable
  (`blinkClosed = 2;`, then `p[0] = blinkClosed;`). Neither the count of uses nor
  which arm holds it matters — both were measured. `HandleKawaiDataInModel` in
  `src/field/field2.c` needs this, and is parked 2 rows out because
  `move_movables` emits the hoists in insn order: a loop-top assignment is
  always lifted *before* a constant the compiler found later in the body, and
  making the other constant a local too fixes the order but hands the allocator
  a different register.

  The rule is not about constants: a global's `%hi`/`%lo` address is a movable
  too, so a global written only inside a conditional arm gets its address
  rebuilt at every use. `LoadLocalFieldModelAndInitAll` toggles `D_800DF114`
  twice inside an `if` in its third loop; taking `flip = &D_800DF114;` before
  the loop and writing `*flip ^= 1;` hoists the address the way the target has
  it, and is worth 19 rows.
* **`addPrim`'s second argument has to be built offset-first.** The macro uses
  the pointer twice -- once as an address, for `setaddr(p, ...)`, and once as a
  *value*, for `setaddr(ot, p)` -- and the value the target computes is
  `base + (i * stride + K)`: `addiu v0,<idx>,K` then `addu v0,<base>,v0`. Every
  pointer spelling, `&buf->Arrows[i + 0xC]` included, folds that to
  `(base + K) + i * stride` and emits the two instructions the other way round,
  which changes the whole loop's allocation behind it. Write it as
  `(SPRT_16*)(i * 0x10 + 0x40C0 + (s32)buf)`; the integer PLUS keeps the source
  association where the pointer PLUS reassociates. It was worth 70 of
  `FieldArrowsAddToRender`'s 86 rows, and parenthesising the pointer form
  (`(s32)buf + (i * 0x10 + 0x40C0)`) does *not* reach it -- fold reassociates
  regardless once the base is a pointer.
* **Assign the field whose value takes longest to compute first, and sched2
  fills the load's delay slot with the short one.** `p->v0 = 0xD0;` before
  `p->u0 = (D_8011446C * 4 & 0x30) + 0x30;` issues the two-insn v0 store
  immediately and leaves a `nop` after the `lhu`; written the other way round
  the six-insn u0 chain starts first and the v0 store lands in its shadow,
  which is what the target has. Two rows and, here, the last insertion. The
  tell is a `nop` right after a global load with an unrelated short store
  sitting two slots above it.
* **`addPrim`'s first argument is used twice, and the two uses can be spelled
  differently.** `addPrim(ot, p)` expands to `setaddr(p, getaddr(ot)),
  setaddr(ot, p)`, so an ordering-table slot read out of a global appears twice
  in one statement — and routing only the *second* use through a pointer local
  is a lever worth seven rows:

  ```c
  layer3Slot = &D_8009ACA2.layer3;      /* immediately above the layer3: label */
  ...
  setaddr(&buf->Bg2[sprite], getaddr(&buf->ot[D_8009ACA2.layer3]));
  setaddr(&buf->ot[*layer3Slot], &buf->Bg2[sprite]);
  ```

  The macro cannot express that, so the expansion has to be written by hand;
  writing it out *without* the pointer is exactly inert, which is the check
  that says the pointer is doing the work rather than the expansion. Where the
  pointer is assigned decides everything — immediately above the walk's entry
  label it is 65 rows, at the top of the function 124/3. This is the same shape
  as the "same packet reached two ways in one loop" idiom: when one object is
  reached twice in one statement, the two spellings are independent knobs.
  `AddBackgroundToRender` in `src/field/field.c` needs it.
* **The same packet reached two ways in one loop is not untidiness.** In
  `FieldArrowsAddToRender` the target writes `u0`/`v0` through
  `((struct FieldRenderData*)(i * 0x10 + (s32)buf))->Arrows[K]` -- base
  register `i * 0x10 + buf`, displacement 0x400C/0x40CC -- and `x0`/`y0`/`clut`
  through the plain `buf->Arrows[i + K]`, which computes `(i + K) * 0x10`.
  Same byte, two different address computations, both present in one loop
  body. When m2c emits two temporaries for what is obviously one object,
  reproduce both rather than picking the tidier one.
* **A hardware address used a dozen times wants a named pointer local; gcc
  will not spend a callee-saved register on a constant by itself.** Written as
  `*(s32*)0x1F800004` at every access, gcc rematerialises `lui <t>,0x1f80`
  ahead of each one -- `CONST_COSTS` makes a two-instruction constant cheaper
  than a register that has to be saved and restored -- so a function that
  touches the PS1 scratchpad fifteen times pays fifteen `lui`s where the target
  pays one and addresses everything as `N($s0)`. `s32* scratch =
  (s32*)0x1F800000;` in front of the loop is worth 51 rows in
  `FieldEntityWalkmechCross`. Where the local is *assigned* is load-bearing in
  the other direction: accesses written before the assignment keep the `$at`
  macro form, which is exactly what the target has for the two stores that
  precede its first use.
* **m2c's nested reconstruction of an if/else-if chain is not the source, and
  transcribing it lets cse delete the target's redundant tests.** A chain
  `if (a) X else if (b) Y else if (c) Z else W` behind a fast-path
  `if (!a && !b && !c) goto done;` compiles with each fast-path branch
  *threaded* straight at its arm, leaving the chain's own tests behind as
  apparently-redundant re-tests of the same registers. m2c reads that CFG back
  as a nest in which the re-tests sit inside the arm that already proved them,
  and written that way cse folds them and the arms come out in a different
  order -- 19 rows in `FieldEntityWalkmechCross`, and the give-away is an arm
  falling through immediately after the tests where the target has a different
  one there. When m2c produces a nest with a test that looks obviously
  redundant, try the flat chain first.
* **The counter-merging rule scales to a whole function, and the partition is
  outer-versus-inner, not adjacent-versus-distant.** `FieldInitDefaultValues`
  in `src/field/field4.c` runs eleven loops over three hundred lines, and the
  target puts *every* outer and sequential loop on one variable (`$a3`) and
  *both* inner loops -- a script-bank walk and a palette clear, far apart in
  the source and unrelated in purpose -- on a second (`$a2`). Writing a third
  counter for the one loop that reads as genuinely different (the bank walk's
  outer index) split the reference count three ways and cost **34 rows**, all
  of it register renaming that reads as allocator noise. When a function's diff
  is dozens of rows of `a3` against `a2` on loop counters and nothing else,
  count the target's counter registers before looking at anything: two
  registers means two variables, however many loops there are.
* **Counting *down* is a different walk from counting up, and a loop whose
  biv gcc eliminates outright still has to have its own variable.** The
  partition above is about which loops share a counter; this is about a set
  that must not. `func_801B0050` in `src/battle/batini.c` runs three
  descending clears (`for (k = 0x3F; k >= 0; k--)`) among a dozen ascending
  `for (i = 0; i < n; i++)` loops. The descending ones keep no counter at all
  in the object -- the biv's only other use is the guard, so `strength_reduce`
  retests the giv (`bgez` on the byte offset) and deletes it -- yet spelling
  them with the same `i` as the ascending loops is **3 rows**, because the
  *pseudo* still spans them and its live range decides what the allocator has
  free afterwards. The consequence lands nowhere near the loops: a `-1`
  materialised for a store two statements later loses `$v0` to a callee-saved
  register, and a callee-saved register is one reorg may legally sink into the
  preceding call's delay slot where `$v0` -- clobbered by the call -- may not,
  so an unrelated `move sN,zero` gets displaced. The tell is a constant in a
  callee-saved register with no reason to be there, plus one insn that has
  swapped places with a `jal`'s delay slot.
* **A caller-saved register cannot be moved into a preceding call's delay
  slot, and a callee-saved one can.** `fill_simple_delay_slots` also scans
  *forward* from a `jal` for a candidate, and rejects anything the call
  clobbers -- so `li v0,K` immediately after a call stays put while a
  `move sN,zero` from further down is hoisted over it into the slot. Read a
  delay slot that holds an insn from suspiciously far below as evidence about
  the *register class* of everything in between, not about statement order.
* **`*p = 0xFF` gives `li -1` through an `s8*` and `li 0xff` through a `u8*`.**
  The constant is narrowed to QImode against the store, and an HImode/QImode
  `const_int` is sign-extended, so `0xFF` becomes -1 and gcc materialises it
  with `addiu <r>,zero,-1` rather than `ori <r>,zero,0xff`. The stored byte is
  identical and the row is a real difference. Same trap as the `0xC600`
  narrowing bullet above, one width down.
* **That same narrowing lets an all-ones constant *share a movable* with an
  unrelated `-1`, and the hoist it earns then rewrites the whole function.**
  The bullet above is about one row; this is the same fact with a loop around
  it, and it was worth **59 rows and the frame** on `func_801B1E0C` in
  `src/battle/batini.c`. `c->unk52 = 0xFFFF` through an **`s16`** member is
  `addiu <r>,zero,-1`, which is bit-identical to the `c->unk8 = -1` two lines
  up and to the `(s16)enemyID != -1` guard below it. Three matching movables
  in one loop body, all on the always-executed path, is enough for
  `move_movables` to lift the constant into a callee-saved register -- so the
  register the target spends on a loop-invariant *address* is gone, that
  address is rebuilt at every use instead, and the pseudo the address
  displaced (here the `i * 0x60` snapshot two inner loops index off) spills,
  taking the frame from 0x30 to 0x38. Declared **`u16`** the constant stays
  `ori <r>,zero,0xffff`, matches nothing, is not hoisted, and all of it goes.
  The diff reads as a wall of callee-saved renaming plus a spill, which is the
  shape that sends you looking at register pressure; the tell that it is a
  *constant* is a hoisted `li <s-reg>,-1` in your preheader that the target
  does not have, with the target materialising -1 two or three times in the
  body instead. Check the signedness of every member a function stores `0xFFFF`
  or `0xFF` to before reading a single allocation row -- `grep` the member name
  first, since one with a single user is free to retype.
* **m2c's temporaries are a *readout* of gcc's grouping, not a source-level
  fact, and writing them back as locals makes things worse.** A run of stores
  that share one loaded pointer appears in m2c output as `temp_v0->a = 0;
  temp_v0->b = 0; ...` with the singletons spelled inline, which looks exactly
  like a source structure worth reproducing. Spelling those groups out as
  explicit `T* e = &arr[i];` locals -- at precisely the boundaries the target
  has -- took `FieldInitDefaultValues` from 82 rows to **126**. The plain
  `arr[i].member` spelling at every store is what the original wrote; the
  grouping is decided after the front end and a local cannot pin it.
* **One pointer per copy loop, even when the loops are far apart.** Reusing a
  `u32* s` / `u32* d` pair for two unrelated copies in the same function
  stretches both live ranges across everything between them, and the allocator
  pays for it elsewhere — in `LoadLocalFieldModelAndInitAll` the second copy
  came out with two base registers instead of one, 20 rows. This is the
  opposite of the counter-merging idiom above: merge *loop counters* that
  describe the same walk, split *pointers* that describe different ones.
* **Two levers rejected for moving the length in *opposite* directions can
  cancel, so test the pair before believing either rejection.** The length
  rule -- fit the length first, treat a row count that improves while the
  length grows as evidence against the change -- is right per change and
  wrong as a filter on a *set* of changes. `FieldMain` carried two clusters
  whose fixes were each measured alone, each moved the length by one in a
  different direction, and each was parked as a regression: spelling the
  `FieldEventInit` argument `&g_FieldStateData` so cse can relate it to the
  `%lo` the neighbouring stores just materialised is 51 rows at **785**, and
  `s16 fieldId` for the sign-extension the target has before the `preloadId`
  compare is 74 rows at **787**. Applied together they are **39 at the exact
  786**. Neither is findable from the other's rejection line. When a note
  lists two entries at -1 and +1, cross them before moving on -- it is one
  extra measurement and it was worth 20 rows here.

* **A parked function whose `.rodata` blob is carried as a file-scope object
  measures worse than it will be when it lands, and the queue does not know
  it.** A local aggregate initialiser emits its constant into the unit's
  `.rodata`; while the function is pinned that object has to exist under its
  own name, and `parked_queue.py` / `variant_eval.py` measure it *in place*.
  Every `%lo` in the function then reads against a shifted pool. `FieldMain`
  reports 34 rows with `const u32 D_800A0000[]` present and **27** with it
  deleted -- a constant 7 rows that vanish the moment the function is
  unparked, which is exactly when the object must be deleted anyway (see
  *Twelve ways a clean-looking diff lies*). So for any parked function with a
  local aggregate initialiser, take the honest baseline by scoring one
  variant that deletes the object, and record both numbers in the note.

* **Read variable *identity* off the destination registers, and read it in
  both directions.** The bullet below says a value in *different* registers in
  two arms cannot be one variable, and that is half the rule; this project
  applied only that half for a long time and it cost 35 rows on one function.
  The other half: a value in the **same** register in both arms *is* one
  variable, and there is a sharper form of the test that names which pseudos
  are global. `local_alloc` hands out `$v0,$v1,$a0..$a3,$t0,$t1,...` in that
  order, so a block-local quantity takes the lowest register free in its
  block. If the target writes a *high* register that a block-local could never
  have reached -- `negu $t2,$a1` in one arm and `negu $t2,$a3` in the other,
  with `$t1` untouched anywhere in the first arm -- then `local_alloc`
  *refused* that pseudo, which means it spans two basic blocks and
  `global_alloc` placed it. That is a global allocno, i.e. one source
  variable across both arms. `FieldCalcPointOnLine` went **60 rows to 25** on
  merging three per-arm pairs back into single variables, and the third global
  is also what forces the two long-lived pointers up to `$t3`/`$t4` where a
  two-global body leaves them at `$t2`/`$t3`.

  The caveat is worth as much as the rule, and it is the difference between a
  reconstruction and a lever. Two of those three merges are what the target
  states: each value holds *one* register across both arms. The third does
  not -- the target has it in `$v0` in one arm and `$a1` in the other -- so
  merging it manufactures a fourth global that displaces the others and buys
  14 rows while leaving another value in the wrong register. Keep it if you
  are hill-climbing, but record in the note that it is a codegen lever and not
  what the original wrote, or the next pass will reason from a false premise.

* **A sweep is exhaustive only over the dimension it varies, and "every axis is
  closed" is a claim about the axes you thought of.** `FieldCalcPointOnLine`'s
  note recorded roughly 200 measured points -- 98 barrier placements, a
  150-point width sweep, statement orders, declaration orders, operand orders
  -- and concluded the function was finished. Every one of those ranged over
  *spelling with the variable set held fixed*, and the variable set was the
  answer. Both enumerations are genuinely flat over a two-global body and say
  nothing whatever about a five-global one. So when a note says a dimension is
  closed, ask what it held constant; and after any change that alters how many
  variables exist, treat every earlier sweep as stale.

* **Which register a value gets in each arm tells you how many variables the
  original had.** A pseudo gets exactly one hard register, so if a value lands
  in a *different* register in two arms of an `if`, it cannot be one variable
  -- and if it lands in the *same* register in both, it is. Read the two arms
  side by side before touching anything: `FieldCalcPointOnLine` in
  `src/field/field2.c` has minX in `$t0` then `$t1`, dx in `$a2` then `$t0`,
  dy in `$a0` then `$a1`, den in `$a2` then `$a0` -- every value on a
  different register -- where the C reused one local per value and got one
  register for each in both arms. Declaring a separate set per arm is worth
  3 insertions -> 2 on its own and, with the term-naming bullet below, 76
  rows -> 60 and the exact length. This is the same lever as "one pointer
  pair per loop", read off the register assignment rather than guessed: merge
  what describes one walk, split what describes two.
* **`A - (B + C)` is folded to `(A - C) - B`, and a named local is what stops
  it.** gcc 2.6.3's `fold` associates every such sum: `maxX - (minX + 0x140)`
  comes out as `addiu -0x140` off maxX and `minY - (maxY - 0xF0)` as
  `addiu +0xf0` off minY, both the mirror image of what the target emits.
  `split_tree` strips only conversions that keep the machine mode, so no cast
  in SImode blocks it and an `(s16)` cast costs a truncation pair (+7
  instructions, measured). What does block it is a VAR_DECL: `bx = minX +
  0x140; dx = limits->maxX - bx;`. combine does not put it back, because the
  two insns cannot merge into one. The cost is not just two swapped operands
  -- the association changes the load's slack, so maspsx stops emitting a
  load-delay `nop` the target has and the function comes out an instruction
  *short*. When a diff shows the same opcodes with the constant carried on
  the other operand, name the inner sum.

  The trap in the middle of this is worth as much as the rule. With the arms
  split and only *some* terms named, `FieldCalcPointOnLine` measures 75 rows
  against the finished body's 60 -- and that 75-row body can never match,
  because one arm still emits `addiu +0xf0` where the target has
  `addiu -0xf0`. Two bodies of the same length are comparable by rows only
  when both compute the target's arithmetic; a lower count reached by leaving
  a fold in place is a local minimum with no path out of it.
* **The same lever runs the other way: a named local also stops fold
  *distributing* a multiply over a difference onto a symbol's addend.**
  `*(s16*)((u8*)SYM + (arg0 - 4) * 12)` does not compile to a subtraction and
  a multiply -- fold rewrites it as `arg0 * 12 - 0x30` and folds the `-0x30`
  onto the symbol, so the address comes out `%lo(SYM-0x30)` with a bare
  `arg0 * 12` index. Same byte, three instructions in a different order, and
  every spelling of the sum reaches it: `-0x30 + arg0 * 12` written
  explicitly measures identical, because both trees fold to the same thing.
  `k = arg0 - 4;` on its own line is a VAR_DECL fold cannot see through, so
  the subtraction stays its own insn and the symbol keeps its own address.
  The tell is an addend on the `%lo` that the target does not have, with an
  `addiu <reg>,<param>,-N` in the target that your build has nowhere.
  `func_800B8FCC` in `src/battle/battle1.c` matched on this alone, from 7
  rows.
* **A `lui`/`%lo(reg)` pair that splat prints as `%hi/%lo(<symbol>)` may be a
  plain constant, and the target tells you which.** splat disassembles a
  *linked* overlay, so it has no relocations -- it pairs a `lui` with a later
  `%lo`-shaped use heuristically and prints the symbol whose address the two
  halves add up to. A bare `lui a0,0x801b` feeding `lw v0,0(a0)` is therefore
  rendered `lui a0,%hi(func_801B0000)` / `lw v0,%lo(func_801B0000)(a0)`, and
  writing C that names the symbol cannot reproduce it. The give-away is the
  target adding that same register straight back as a pointer value
  (`addu a0,v0,a0`): a real symbol reference has to become an address through
  an `addiu %lo` first, so no correct compiler emits it, whereas a constant
  whose low half is zero needs exactly one `lui` and can be added as-is.
  Reach for the literal -- `(u_long*)0x801B0000` -- and expect `checkfn` to
  report the operand as rows it cannot alias away; the bytes are identical
  and the overlay SHA-1 is the arbiter. `func_800BB4F8` and `func_800B5CD4`
  in `src/battle/battle1.c` are both this, at 3 and 5 such rows.
  Two corroborating signs in the same function: splat prints an *unpaired*
  `lui` as `lui $a1, (0x801B0000 >> 16)`, so a function that shows both
  spellings of one address is telling you gcc materialised the constant twice
  rather than that two different objects are involved; and the symbol form
  costs an extra `addiu` that shows up as a length overrun, not as noise.
* **A chained assignment stores right to left.** `m[0][0] = m[1][1] =
  m[2][2] = 0x1000;` is `m[0][0] = (m[1][1] = (m[2][2] = 0x1000))`, so the
  stores come out `m[2][2]`, `m[1][1]`, `m[0][0]` — descending. A target that
  fills a struct in descending address order, in groups, is three chained
  statements, not twelve separate ones; twelve give ascending order and a
  loop-hoisted constant besides. This is what the identity matrix in
  `HandleKawaiDataInModel` needs.
* **Two fields of one struct fed by the same load want the chained form, and
  the reason is aliasing, not style.** `c->maxHP = e->maxHP; c->curHP =
  e->maxHP;` emits the load **twice** -- the first store is a struct MEM and
  so is the load, `true_dependence` cannot disambiguate them, and gcc 2.6.3
  re-reads. Written `c->curHP = c->maxHP = e->maxHP;` there is one load, and
  the stores come out 0x30 then 0x2c, which is the descending order of the
  bullet above. Two instructions per pair (the load and its delay-slot `nop`),
  and `func_801B1E0C` in `src/battle/batini.c` had two such pairs. The tell is
  a load of the *same* member appearing twice in your build and once in the
  target, with a `nop` behind each.
* **A walking pointer's initialiser belongs in the `for` init clause.**
  `q = base + 0xF; for (j = 0xF; j >= 0; j--, q--)` emits the pointer's
  `addiu` *before* the counter's `li`; `for (j = 0xF, q = base + 0xF; j >= 0;
  j--, q--)` emits them in written order, which is what a target with
  `li a2,0xf` ahead of `addiu v1,t4,0xf` is telling you. Free -- same
  instructions, different order -- and worth two rows per loop, three loops in
  `func_801B1E0C`. Same lever as the two-counter `for`-increment bullet below,
  applied to the init clause. Reusing *one* pointer variable for two such
  loops costs three more rows there, which is the "one pointer pair per loop"
  rule again.
* **`p[k]` on a pointer is already an INDIRECT_REF, so rewriting it as
  `*(p + k)` to clear `MEM_IN_STRUCT_P` is exactly inert.** The C front end
  lowers a subscript on a pointer to `*(p + k)` in `build_array_ref`; only a
  subscript on a real *array* builds an ARRAY_REF. So the aliasing bullets
  above that turn on ARRAY_REF apply to `arr[i]`, never to `ptr[i]`, and a
  budget spent re-spelling pointer subscripts buys nothing -- measured to the
  row on all seven stores through a `u8* atk` in `func_801B1E0C`.
* **A whole-struct copy schedules; an element-wise copy through pointers does
  not.** `*(MATRIX*)dst = *src;` is a block move gcc knows cannot overlap, so it
  interleaves the loads and stores across three registers. Eight
  `dst[k] = src[k];` through two `s32*` may alias, so gcc emits `lw`/`nop`/`sw`
  through one register per word. A run of load-nop-store pairs in the target
  means element-wise C, not a struct assignment.
* **A local aggregate initializer is a `.rodata` blob copy, not stores.**
  `MATRIX mtx = {{{0x1000,0,0},...}};` makes cc1-psx-26 emit the constant into
  `.rodata` and copy it in with `lw`/`sw` pairs — which also shifts every later
  `.rodata` offset in the unit. Write the fields.
* **That blob copy is also a scheduling barrier, and the only way past it is
  to *declare* something above it.** The copy is emitted where the aggregate
  is declared, so every statement in the function body is scheduled after it
  -- including a constant materialisation that the target issues as the very
  first insn of the body. `s32* nearest; ... nearest = (s32*)0x1F800020;`
  puts the `lui`/`ori` after the copy; `s32* nearest = (s32*)0x1F800020;`
  written as an initialiser on a declaration placed *above* the aggregate
  puts it first, which in turn drags the save of that callee-saved register
  (`sw s3,0x2c(sp)`) in front of `sw ra` in the prologue. Only the position
  relative to the aggregate matters: initialised at a declaration *below* it
  the diff is unchanged. The tell is a register save reordered ahead of `ra`
  with the constant that clobbers that register sitting at the top of the
  target's body and at the bottom of the entry block in yours.
  `FieldEntityTriggerCheck` in `src/field/field2.c` matched on this after
  five statement orderings of the same three assignments had all measured
  exactly 7 rows -- which is what made the residue read as a scheduling knot
  with no lever. When statement order is provably inert, try declaration
  order with an initialiser; they are different knobs.
* **A local declared in an inner block takes a later stack slot.** Slots go in
  declaration order, and a block-scope declaration is "declared" where the block
  is: moving `MATRIX mtx;` from the function's locals into the `if` that uses it
  moved it from `0x18` to `0x20`, behind the `long` declared after it at
  function scope. Use it when a diff is nothing but `N(sp)` offsets in one
  branch.
* **A store to a `u8` struct member kills the cached pointer that reached it;
  a store to an `s16`/`s32` one does not — so a run of stores sharing one
  loaded pointer is a *source* run that ends at its first byte store.**
  `true_dependence`'s struct/non-struct disambiguation carries the guard
  `GET_MODE (x) != QImode` on both arms, because a byte access may be part of
  any object. So `g_FieldState->someU8 = 0;` conflicts with the plain
  `extern FieldState* g_FieldState` load in cse's table and forces a reload at
  the next statement, while `g_FieldState->someS16 = 0;` does not. What that
  buys is a way to read a target's *source order* out of its asm without any
  reasoning about registers: group the stores by which `lw` of the pointer
  they use, and within each group the one `sb` is the last statement the
  original wrote. **Its emitted position says nothing** — sched2 routinely
  hoists it five or six slots up, into the load-delay slot of the very load
  the *next* group needs, which is exactly what makes the run look like it
  starts with a byte store. `FieldInitDefaultValues` in `src/field/field4.c`
  is twenty such runs across two objects (`g_FieldState` and `g_FieldModels`);
  reading them back moved ten statements one position each and took it from
  87 rows to 23 at the exact length, after two sessions had recorded the same
  residue as "cse's grouping, not reachable from statement order".

  Two corollaries. A store to a *different* scalar global in the middle of a
  run is free — `memrefs_conflict_p` proves two `symbol_ref`s distinct — so
  `D_80081DC4 = 0;` sits inside the opening run and only its being *in* the
  run matters, not where (nine placements measured 21 rows, the tenth, just
  after it, 40). And an `sw` behaves like `sh`: only QImode is special.
* **A scalar global does not alias a struct store; the same address reached
  through an array or a struct member does.** gcc 2.6.3's `true_dependence`
  disambiguates on `MEM_IN_STRUCT_P`, so a load of a plain `extern s16 x` is
  loop-invariant even in a loop that stores through a struct pointer — gcc
  hoists it into a register and reloads nothing. Write the same address as
  `cam[1].x` and the load may alias, stays in the loop, and only its `%hi`/`%lo`
  *address* is hoisted. So when the target reloads a global on every iteration
  and your build hoists it, the global is an array or struct element in the
  original, not a scalar. The tell is exact: `lui`/`lh` of the symbol inside the
  loop in the target against a single load in the preheader in yours, or an
  `addiu <reg>,<reg>,%lo(sym)` in the preheader with `lh <r>,0(<reg>)` in the
  body. `AddBackgroundToRender` reads three such globals — a camera window, two
  ordering-table slots and a per-sprite animation byte pair — and typing them as
  arrays and structs was worth 172 rows and the whole frame layout.
* **A scalar `extern` store lets the very next statement's array load float
  above it, and a one-element array pins it.** The store side of the
  aliasing rule below, and it is worth a whole function: `D_8009D7D0 = ~f();`
  followed by `PC_INC(4)` puts `g_FieldScriptPC[g_CurrentEntity]`'s load
  *before* the store, because a plain `extern u8` is not `MEM_IN_STRUCT_P` and
  `true_dependence` waves the struct load past it. gcc then issues the `nor`
  immediately after the call and has to park it in a second register, since
  `$v0` has already gone to the array's address. Declared `extern u8 D_[1];`
  and written `D_[0] = ~f();`, the store is in a struct too, the load stays
  below it, and the `nor` falls into the `lbu`'s load-delay slot keeping
  `$v0` -- which is the target. `OpcodeFuncSpcal` in `src/field/field4.c`
  matched on this after its park note had called the residue "post-reload
  scheduling with equal priorities on both sides". The tell is a store that
  the target issues *earlier* than you do with an address materialisation
  in between, and the give-away that it is aliasing rather than scheduling is
  that every spelling of the stored *value* -- a named local, a cast, `-x - 1`
  for `~x` -- measures exactly the same.

* **`MEM_IN_STRUCT_P` is set by an ARRAY_REF as well as a COMPONENT_REF, so
  `((s32*)p)[k]` is not the non-struct spelling it looks like.** Every
  aliasing bullet here turns on that flag, and there is exactly one way to
  clear it: a plain INDIRECT_REF on a cast pointer, `*(s32*)((u8*)p + 4)`.
  Rewriting `outEdge->vx` as `((s32*)outEdge)[0]` to stop a load being shared
  measured *exactly* inert in `FieldEntityWalkmechCross` for this reason. The
  same fact read forwards is the cheaper fix: to stop a struct load floating
  above a run of raw `*(s32*)0xNNNN` stores, spell the stores as an array
  (`scratch[12]`) rather than the load as a cast -- same effect, and the
  array indexing is usually what the original wrote anyway.
* **A store written through a pointer local pins every later load; the same
  store written to the symbol does not, and the two are not always both
  available.** `true_dependence` cannot disambiguate a store through a
  computed pointer from anything, so `*p = table[idx];` stops a struct load
  below it from being hoisted, while `SYM = table[idx];` -- a plain `extern`,
  not `MEM_IN_STRUCT_P` -- lets it float. That is the usual rule; the part
  worth knowing is that it can *collide* with the address-form rule one
  section down. `func_801D2408` in `src/menu/savemenu.c` needs both halves at
  once and cannot have them: the target stores the first play-clock digit
  through a base register (`sb v1,0(s0)`, with the packet destination reached
  later as `addiu s0,s0,0x66`), which requires `&D_801E6D52` to have a second
  symbol reference for cse to relate -- and routing the store through the
  pointer is exactly what stops `Savemap.header.time`'s second read floating
  above it, which is what puts the clock's base in the target's `$s0` rather
  than `$s1`. Measured: `*p = ...` is 24 rows, `SYM = ...` is 68. When two
  documented idioms each fix half a residue and each breaks the other's half,
  say so in the note -- it is a much more useful statement than either
  measurement alone.
* **A store's struct-ness decides whether a later load may float above it.**
  The companion to the aliasing bullet above, run forwards instead of
  backwards: `true_dependence` lets a `MEM_IN_STRUCT_P` load move past a store
  that is *not* in a struct, and refuses when both are. So a run of scratchpad
  writes followed by a call whose argument reads a struct member schedules
  three different ways for the same four assignments. All of them written as
  casts (`*(u8*)0x1F800001 = 1;`) lets the member load float to the top of the
  block, where it lands right behind the pointer load it depends on and costs a
  load-delay `nop`; all of them written through a record pins the load below
  every one of them; and writing all but the last through the record settles it
  in between. `FieldModelLoadAndInit`'s KAWAI loop needs exactly that last
  shape — three `KawaiFaceSel` member stores and one plain cast — and the three
  spellings measure 5, 2 and 0 rows. When a diff is one `nop` and one load that
  is a couple of slots early, look at what the stores around it are declared
  as, not at the load.
* **A `for (;;)` loop left by `goto` keeps the invariant hoisting that a
  backward `goto` loses, without paying for the rotation a `break` costs.**
  Three shapes, three different results for the same body: a backward `goto`
  emits no `NOTE_INSN_LOOP_BEG`, so `loop_optimize` never runs and every
  invariant is rematerialised at its use; `for (;;) { if (c) break; ... }` is a
  loop, so invariants hoist, but `duplicate_loop_exit_test` copies the exit test
  to the bottom, hoists an entry test into the preheader and reorders the
  blocks around it; `for (;;) { if (c) goto next; ... }` with `next:` after the
  loop is still a loop and still hoists, but the exit jump is not the loop's own
  end test, so nothing is duplicated and the single top test is reached by a
  plain `j` — which is what a target with one top test *and* a populated
  preheader is telling you. Measured on `AddBackgroundToRender`'s four walks:
  434 rows as backward gotos, 396 with `break`, 268 with `goto`. This is the
  companion to the `FieldBackgroundInitPackets` bullet above, not a replacement
  — there the bodies were small and the rotation was the whole cost, so a
  backward goto won.
  **The first test tells you which of the four shapes it is, and it does so
  from one operand.** A `while` puts its zero-trip test in the *entry* block,
  where cse still knows every value the preheader just set — so a loop
  starting `bit = 1` comes out `andi v0,v1,0x1`, the constant folded into the
  compare. Every other shape puts the first test at a label with two
  predecessors, cse stops there, and the same test is `and v0,v1,a2` against
  the register. So: a compare against a *register* whose value is a loop-entry
  constant rules `while` out outright, before any row is read.
  `func_80033128` in `src/main/akao.c` measures 19 rows as `while`, 19 as
  `for (;;) { … break; }`, 10 as a backward `goto` and **0** as
  `for (;;) { … goto done; }` with the label after the loop — and its target's
  `and` against `$a2` said so from the start.
  The choice also decides register allocation, not just hoisting, and that is
  the larger half. `REG_N_REFS += loop_depth` in flow, so every reference in a
  walk that gcc recognises as a loop counts double — and `allocno_compare`
  ranks by `floor_log2(n_refs) * n_refs / live_length`, whose `floor_log2` step
  makes crossing a power of two worth about 30% of an allocno's priority. So a
  value referenced only in the *outer* walks (a cursor advanced once per run, a
  pointer passed to a call there) gains nothing while the walks are backward
  gotos, loses to any counter incremented inside an inner `do`/`while`, and is
  the one that spills. Turn the outer walks into `for (;;)` loops and it wins
  the register instead. `FieldBackgroundInitPackets` in `src/field/field.c` is
  the case: with four backward-goto walks the target's `modes` pointer spills
  and each of its four `modes++` costs an `lw`/`addiu`/`sw`, and every register
  downstream renames — 48 rows. All four walks have to change together (layers
  1+2 alone measure 181, layers 3+4 alone 171, all four 118), because it is the
  *total* weighted count that has to cross 16. The park note there had measured
  `while` and `for (;;) { … break; }`, found both worse, and concluded the goto
  walks were load-bearing; it never tried the third spelling. When a diff is
  dozens of rows of renaming plus a spilled pointer the target keeps in a
  callee-saved register, count the loop-weighted references before touching the
  expression.
* **`combine_givs` bases a loop body's address giv on the *last* offset
  referenced in insn order, so the field a struct-filling loop writes last
  decides every displacement in it.** A packet loop writing r0…v0, then `clut`,
  `w`, `h` bases on `sprt+0x12`; writing `w`, `h`, then `clut` bases on
  `sprt+0xe`, and every `sb`/`sh` in the body moves by 4. The two are the same
  program and the *emitted* order is `w`, `h`, `clut` either way — sched2 sinks
  the clut store regardless — so the diff reads as pure scheduling noise with a
  constant offset, which is the tell. Read the base off the target
  (`addiu s0,s6,0xe` against your `addiu s0,s6,0x12`), subtract, and move the
  field at that offset to the end of the source block. Worth 22 rows across two
  loops in `FieldBackgroundInitPackets`, where layers 1 and 2 already ended
  with `clut` and had matched all along, which is what made layers 3 and 4 look
  like an allocation problem.
* **`f(…, *p++, …)` and `f(…, p[0], …); p++;` are not the same schedule when
  `p` is spilled.** As an argument, the load and the spilled pointer's
  read-modify-write form one dependency chain sched2 keeps together, so the
  load-delay slot after the `lw` of `p` gets a `nop`; split into two statements
  the chain breaks and an unrelated invariant increment fills the slot. Thirteen
  rows across four call sites in `FieldBackgroundInitPackets`, and it only pays
  once the *other* pointer in the call is written out separately too —
  `f(q++, …, p[0], …)` measures back where it started.
* **Read a loop-invariant global directly; do not hoist it into a local
  yourself.** Both spellings load it once outside the loop, but a local is a
  source statement and lands among the surrounding code, while letting gcc hoist
  it makes it a movable that is inserted into the preheader beside the other
  movables — different order, different register class, and a different number
  of callee-saved registers at the end of it. Replacing four such locals with
  direct reads of `D_8011448C`, `D_801144C8` and `g_FieldTriggers` took
  `AddBackgroundToRender` from 305 rows and a frame 8 bytes short to 254 rows
  with the frame exact. The inverse case is the conditional-arm bullet above: a
  value gcc will *not* hoist on its own still needs the local.
* **`!(a && b)` and `x <= lo || hi <= x` are not the same to gcc 2.6.3.** Fold
  does not apply De Morgan, so the two spellings hand `expand_expr` different
  trees and their operands are evaluated in different orders — which decides
  which of two loads goes first, and therefore which load-delay slots the
  scheduler can fill. Same instructions, same branch polarity, four rows apart
  per test. Both are worth trying whenever a range test's diff is scheduling.
* **A value the target copies with `move` out of a register it just loaded is
  an `s16` local assigned inside the arm.** Three spellings of the same
  temporary: `s32 t = p->field;` before the `if` gives one `lh` and no copy, gcc
  having coalesced the local into the loaded register; `s16 t = p->field;`
  before the `if` gives *two* loads, an `lhu` for the `movhi` and an `lh` that
  combine folded out of the sign-extension the comparison needed; and `s16 t =
  p->field;` *inside* the arm, after the comparison has already loaded the field
  with `lh`, gives the copy — cse rewrites the redundant load as a register move
  and the HImode pseudo does not coalesce with the SImode one. The last is what
  a lone `move` in a branch delay slot means.
* **An array's base address in a typed pointer local is rematerialised in
  every loop that indexes it; as a `u8*` scaled by hand it stays in one
  register.** `SVECTOR* normals = D_800DF520;` then `&normals[i]` gives gcc a
  `(plus (reg) (mult (reg) 8))` it is happy to rebuild from the symbol at each
  loop's preheader — eight `lui`/`addiu` pairs in `KawaiLightingApplyToPolyColor`
  where the target has one at function entry. `u8* normals = (u8*)D_800DF520;`
  with `normals + i * 8` keeps a single pseudo live across the whole function
  and, as a bonus, emits the `addu` base-first the way the target does. Worth 14
  instructions there. The tell is the same symbol's `%hi`/`%lo` appearing once
  per loop in your build and once per function in the target.
* **A base an inner loop indexes off, while the outer loop advances it, wants
  an explicit snapshot local.** `for (i...) { for (k...) { p[k * 4 + 7] ... }
  p += stride; }` bases the inner loop's induction variables straight on `p`;
  the original writes `c = p;` at the top of the outer body and indexes `c`,
  which is where the target's `move <reg>,<reg>` at each inner-loop preheader
  comes from. It is not a redundant copy — gcc keeps the snapshot in its own
  register and the whole allocation follows. `KawaiLightingApplyToPolyColor`
  went from 87 rows to 43 on this one line.
* **A nested clear wants the row base hoisted as a *subscript* on the outer
  counter and the inner access as a subscript on the inner one — writing
  either walk by hand fails in a different direction.** For the target's
  `addiu <row>,<row>,0x20` in the outer branch's delay slot with
  `addiu <cell>,<row>,0x1e` in the inner preheader and `addiu <cell>,<cell>,-2`
  as the inner step, the source is

  ```c
  do {
      j = 0xF;
      cell = (s16*)&D_80095DE0[i * 0x20];   /* outer giv -> walking pointer */
      do { cell[j] = 0; j--; } while (j >= 0);   /* inner giv -> +0x1e, -2 */
      i++;
  } while (i < 0x40);
  ```

  and the two obvious alternatives each get exactly one of the two halves.
  A hand-written `u8* row` walked `row += 0x20` reaches the walking pointer
  and costs **two instructions**: `i` then has no use outside the exit test,
  so `check_dbra_loop` reverses it (`bgez` in place of `slti`/`bnez`), *and*
  the `+0x1E` folds into `row`'s initial value so the inner preheader's
  `addiu` disappears — measured identically for `row + 0x1E` with `cell--`,
  for `row + j * 2`, and for either increment order. Any form that keeps the
  index in the address (`D_80095DE0[i * 0x20 + j * 2]`,
  `&D_80095DE0[i * 0x20] + j * 2`, `((s16*)&D_80095DE0[i * 0x20])[j]`) gets
  the inner giv right and leaves the outer row rebuilt with `sll`/`addu`,
  **+1 instruction**. `FieldInitDefaultValues` in `src/field/field4.c`; the
  tell is `addiu -2 / addu +1 / sll +1` in the opcode histogram with a `%hi`
  naming the base *plus the inner constant* where the target names the base
  alone.
* **Two globals a fixed short distance apart, touched at the same stride, are
  one record array — and the negative displacement tells you which field the
  original wrote last.** `lbu $v0,0($a1)` / `lbu $v0,0($v1)` with a single
  `lui`/`addiu` of one symbol and `addiu $a1,$v1,-1` between them is
  `combine_givs` merging the loop's two address givs onto the offset referenced
  *last* in insn order, and reaching the other by displacement. Declaring the
  record and indexing it (`arr[i].count`, `arr[i].value`) is the only spelling
  that reproduces it. Two pointers derived from each other in C cannot: the
  front end folds `&SYM - 1` into `%lo(SYM-0x1)` before RTL, so you get a
  second `lui`/`addiu` pair and the two walking pointers land in each other's
  registers — 12 rows on `func_80019338` in `src/main/18B8.c` against 0 for the
  record. And walking the record by hand with `rec++` gets the single base but
  bases it on the **first** field (`addiu $v1,$a1,1` rather than
  `addiu $a1,$v1,-1`), which is 5 rows: the hand-written biv is the record
  pointer, so the giv it keeps is the one at offset 0.

  Two more things about that loop shape, both worth a row each. **A counted
  `for` over an index emits no zero-trip guard where a pointer compare against
  an end pointer does** — gcc cannot prove `&SYM[0] < &SYM[0x10]` at expand
  time, so `for (p = SYM; p < &SYM[0x10]; p++)` costs an `sltu`/`beqz`/`nop`
  the target does not have, while `for (i = 0; i < 0x10; i++)` strength-reduces
  to the same walk with the guard elided. It also gives `slt` against the
  reduced end register rather than `sltu`, which is what a signed counter
  produces. And **`arr[i] = delta + arr[i]` is not enough to put `delta` first
  in the `addu`**: fold ranks the ARRAY_REF as the more complex operand and
  makes it `op0` regardless of how the sum is written, so the value has to be
  read into a local first (`v = arr[i]; arr[i] = delta + v;`) — two operands
  fold considers equal, source order stands. `func_800193F4` needed both.
* **A row base walked in the `for` clause and the same base assigned from a
  subscript inside the body are different givs, and the difference is which
  offset the displacements are measured from.** Walked -- `p = (u8*)&arr[0].m;
  for (i = 0; i < n; i++, p += sizeof(arr[0]))` -- `p` is a *biv*, the body's
  `p + 0x28` and `p + 0x2C` are two givs, and `combine_givs` merges them onto
  the later one: the base register comes out at `sym+0x16c` with displacements
  `-4` and `0`. Assigned from the subscript at the top of the body --
  `p = (u8*)&arr[i].m;` -- `p` is itself the giv, its add_val is `sym+0x140`,
  and both accesses stay plain displacements `0x28`/`0x2c` off it. Same
  instruction count, same increment, three rows apart, and it is what
  `func_800D54EC` in `src/battle/battle2.c` needs.

  This is the escape when the `combine_givs` bullet above cannot be applied.
  That rule says to move the field at the target's base offset to the end of
  the source block -- which is impossible when the target's base is an offset
  *no field uses*, as here, where the target bases at `+0x140` and reads only
  `+0x168` and `+0x16c`. A base that is not any of the referenced offsets is
  the tell that the register is a biv or a subscript giv rather than a
  combined one. Three other spellings of the walk (`BattleModel* b` with
  `b++`, an `s16*` walked and indexed `p[0x14]`, a shared
  `(SVECTOR*)(p + 0x28)`) all give the merged form and measure the same 3
  rows, so this is not reachable by re-spelling the *accesses*.
* **A pointer bumped once per outer iteration belongs in the `for` increment.**
  `for (i = 0; i < n; i++, p += stride)` emits `i++` ahead of `p += stride`;
  written as the body's last statement the two come out in the other order and
  land in different branch delay slots. Eight rows across four loops, and no
  other spelling reaches it.

  **But a value the loop only *reads* is the opposite case: derive it from the
  counter inside the body, never carry it as a second `for` increment.** Two
  distinct things go wrong at once when it is carried.
  `for (i = 0, slot = 0x13F; i < 5; i++, slot -= 6)` leaves `i` with no use
  outside the exit test, so `check_dbra_loop` reverses the whole loop and it
  counts *down* from 4 — `addiu s0,s0,-1` / `bgez` where the target has
  `addiu s0,s0,1` / `slti 5`. And the second counter's initialiser is an
  ordinary preheader insn, so it is emitted *before* whatever `move_movables`
  and `strength_reduce` put there, while the target has it after. Written
  `slot = 0x13F - i * 6;` as the first statement of the body, `i` keeps a
  reference, `slot` becomes a reduced giv with the `addiu <r>,<r>,-6` the
  target has, and the giv's initialiser lands in the preheader behind the
  hoists. `func_80025174` in `src/main/1255C.c` is 18 rows and one instruction
  short the first way and **matches** the second; `func_800260DC` in the same
  unit is the same lever for a `cy` that is only passed to a call (2 rows to
  0, with the fully inline `i * 0x30 + 0x138` at the call site measuring
  byte-identical to the named local). Read the target's loop direction before
  anything else: a down-counting loop where you expect an up-counting one is
  `check_dbra_loop` telling you a variable the source had is missing.
* **A residue of exactly one `nop` may be the assembler, not the C — and
  `tools/maspsx` is a fork under this project's own account, so it is
  fixable.** This one has been fixed, and the shape of it is the lesson.
  maspsx decided a load needed no delay-slot `nop` by asking whether the next
  instruction *reads* the loaded register. Nothing in the text of
  `lhu $2,D_80075DEC` does — but a load with a symbolic operand and no base
  register is expanded by the assembler through its own destination register
  rather than `$at`, so its `lui $2,%hi(...)` lands in the delay slot of a
  preceding load into `$2` and the original assembler protected it. The
  give-away in maspsx's own output is
  `#nop # DEBUG: '...' does not load from $2` sitting between two loads into
  the same register. `_next_load_clobbers_reg` now catches it, with tests in
  `tools/maspsx/tests/test_symbol_load_clobber.py`; across the whole build
  the rule fires exactly once, in `FieldMainLoop`, which is now a match.

  The method generalises. Before spending a budget on a one-instruction
  residue, prove where it comes from: pipe cc1 through maspsx to a file,
  insert the instruction by hand, assemble with the flags from
  `ninja -t commands`, and re-run `diff.py`. Zero rows means the C is finished.
  Then fix maspsx and run its own suite (`cd tools/maspsx && python3 -m
  unittest discover -s tests -t .`) plus a full `make build` — a hazard rule is
  global, and the only evidence that it is narrow enough is nineteen `OK`s. A
  submodule change is not covered by `make submit`; stage it by hand, and note
  that the worktrees carry the submodule with its `.git` renamed to
  `.git-disabled`, so the commit has to be made in the main checkout.
* **A `void` function fills the delay slot of a branch to its own epilogue; a
  non-`void` one cannot.** gcc's delay-slot pass steals the insn *after* a
  conditional branch when that insn is dead on the taken path — and `li v0,K`
  is dead on the way to a `void` epilogue but live on the way to one that
  returns a value. So a function whose diff is a single unfilled delay slot,
  on exactly the branch that jumps to `jr ra`, was declared with a return type
  it never uses. `FieldMainLoop` needs `s32`; K&R implicit `int` is the likely
  original spelling. Only branches straight to the epilogue are affected —
  branches into a shared tail still get their slots filled either way.

  The rule runs in both directions, and the other one is easier to miss
  because the return type is invisible in a function that never sets `$v0`.
  `FieldEntityCheckTalk` in `src/field/field2.c` sat one row out with
  `ori v1,zero,0x40` landing one instruction *after* the branch-to-epilogue
  whose delay slot the target fills -- and a park note that had reasoned its
  way to "a pure sched2 permutation, since `sll -> sra -> beq` is a longer
  dependence chain than `ori -> beq`", with six spellings measured
  byte-identical and three measured worse. Declaring the function non-`void`
  matches outright. `s32`, `u8` and `s16` all match, so the `.s` cannot tell
  them apart. decomp-permuter found it in 721 iterations through
  `perm_randomize_internal_type`; nothing about reading the target suggests
  it, which is exactly the case the permuter is for.

  A third instance, and this one *is* readable off the target:
  `func_801D3478` in `src/menu/title.c` sat one instruction short with the
  `li v0,1` of the next statement sitting in the delay slot of the
  `bnez` that leaves for the epilogue, where the target has a `nop` there.
  One row, one word, `void` -> `s32`, MATCH. The function sets no return
  value on any path and no caller reads one, so K&R implicit `int` is the
  likely original spelling; write `s32` and say so in a comment, because a
  reader will otherwise "fix" it back.

* **A block a conditional branch jumps *forward* to gets swapped into the
  fall-through unless it contains a call.** For `if (c) { checks } else { A }`
  followed by `B`, gcc emits `beqz c,A / checks / j B / A: / B:` and then
  moves `A` up -- `bnez c,checks / A / checks / B` -- which deletes the `j B`
  *and* puts `B` in the same extended basic block as the checks, so cse
  substitutes a register for `B`'s first load and reorg fills a delay slot
  with it. Four instructions per site, and it reads as scheduling noise a
  long way from the `if`. Nothing about the *spelling* reaches it: the
  if/else, its inversion, the explicit `if (c) goto A;` goto chain, a
  combined `c1 || c2` range test and a `do { } while (0);` barrier at either
  end all measured exactly 100 rows on `func_801D2DA8` in `src/menu/title.c`.
  What reaches it is giving `A` a **call**: writing `A`'s early-out as
  `func_801D2B58(1); return;` rather than `goto` into the shared tail that
  already ends in that call is 4 instructions per arm and 100 rows -> 87.
  The tell is your build being short by `j`s and `nop`s while every
  instruction still looks right, with the target's version of `B` reloading a
  field yours keeps in a register.

* **Which of two stores to the same field is the conditional one decides
  whether reorg can duplicate it into a delay slot.** `if (c) { p->f = 0; }
  p->f = 0;` and `p->f = 0; if (c) { p->f = 0; }` are the same program and
  four instructions apart. With the unconditional store *second*, cse shares
  the field the test reads with the test above it and sched2 sinks the store
  past two unrelated loads, so the branch's delay slot ends up a `nop`; with
  it *first*, the conditional store is the branch's target, reorg copies it
  into the delay slot and redirects the branch past it, and both copies
  survive. Two `sb $zero` to one address with a branch between them is that
  shape, not a compiler artefact to be tidied away -- `func_801D2DA8` in
  `src/menu/title.c` needs it, and the six spellings measured 0 (this one),
  4, 4, 4, 57 and 58 rows.

* **`beq`/`li K`/`beq`/`j default` with *both* tests ahead of *both* arms is a
  `switch`; an `if`/`else if` chain interleaves them.** `expand_end_case`
  emits the whole compare chain first and the case bodies after it, so a
  target whose two comparisons sit adjacent and whose arms follow is a switch
  even when there are only two cases -- and the `li` for the second compare
  ends up in the first branch's delay slot. Written as `else if` the second
  test lands *after* the first arm instead. Worth 29 rows and the whole tail
  of `func_801D2DA8`. This is the same reading as the `OpcodeFuncJump` bullet
  above, one level up: there a `switch` bought one delay slot, here it buys
  the block order.

* **A compiler-generated loop-invariant -- a division's magic multiplier --
  is hoisted or not depending on how many giv *bases* the loop needs, and the
  lever is a pre-loop local that cse folds straight back to the symbol.** A
  loop walking two arrays off one symbol carries two giv base registers, and
  `move_movables` then leaves the `lui`/`ori` of the `x / 0xFFFF` magic
  constant in the loop body; naming one of the two walks' base in a local
  (`Unk801D026C* ratio = (Unk801D026C*)D_8009D3FC;`, indexed `&ratio[i]`)
  drops it to one and the constant is lifted into a callee-saved register --
  which is **+2 instructions**, its save and restore. So the tell is a target
  *longer* than your build by exactly one `sw`/`lw` pair with a `lui`/`ori`
  of a division magic number in its preheader where yours has it in the loop.
  `func_801D027C` in `src/menu/bginmenu.c` needed it. Note the local is
  inert as far as the emitted address goes -- cse folds it back, and the
  preheader is byte-identical either way -- so this is not the "cache a
  re-read global pointer" idiom and reads as a no-op change.

* **A store in the `jr ra` delay slot is what a *non*-`volatile` store looks
  like; a target with a `nop` there and the store above it wants the
  qualifier.** reorg's `fill_simple_delay_slots` happily takes the last insn
  of the body into the return's slot, so a two-statement accessor
  (`old = *p; *p = v; return old;`) comes out one instruction *shorter* than a
  target that keeps the `nop`. `volatile u_short* p` is what stops it, and the
  cost is nothing else: the loads and the store are the same three
  instructions either way. `SetIntrMask` in `src/main/psxsdk.c` is the case
  (6 instructions against 7, the whole residue), and it is worth reading
  together with the `void`/non-`void` bullet above -- both are "the delay slot
  after `jr ra` is a *declaration* fact", reached from the return type in one
  case and from the pointer's qualifier in the other. Note this is a
  *hardware* register behind the pointer, so `volatile` is also what the
  original meant; do not reach for it on an ordinary global, where the
  qualifier costs `lhu` plus a separate `sll`/`sra` at every read.

* **For a function whose whole body is one two-way return, the early exit and
  the `if`/`else` lay the two blocks out in opposite orders.**
  `if (ok) { work; return 0; } return -1;` puts the *failure* block in the
  fall-through and the success block at the branch target; the inverted
  `if (!ok) return -1; work; return 0;` puts success in the fall-through,
  which is what a target with `beqz` over `move v0,zero / <work> / j` is
  telling you. Same program, same instruction count, 4 rows apart, and the
  reflex reading -- "the target's `beqz` means the guard is spelled the
  positive way" -- is backwards. `MargePrim` in `src/main/psxsdk.c` needs
  `if (len > 0x20) return -1;`. Read which block sits immediately after the
  conditional branch, not which way the comparison points.

* **A convenience macro in `include/psxsdk/` is not evidence about the library
  function of the same name.** The headers carry the PSY-Q macro forms
  (`setPolyF3`, `setLineG3`, `addPrim`), and for most of the primitive
  helpers the retail function really is one macro expansion -- 31 of 33
  matched that way at the first attempt. Two do not: `setLineG3` and
  `setLineG4` also clear `p2` and `p2`/`p3`, and the retail `SetLineG3` /
  `SetLineG4` emit no such stores. Diff the macro's store list against the
  target's before using it, since a macro with one store too many reads as an
  insertion in an otherwise perfect five-instruction function.

* **`x++` on a `volatile` re-reads it; `x = x + 1` does not.** The increment is
  an expression whose value gcc materialises even when the statement discards
  it, and for a volatile that means a fourth instruction pair (`lui`/`lhu`)
  after the store. Three rows separate the two spellings of the same line —
  `D_80075DEC++` at the top of `FieldMainLoop`'s frame loop.
* **`volatile` on a global also pins it against constant materialisation.**
  For `if (a == 0 || g == 1)` gcc puts `li v0,1` in the first branch's delay
  slot and the `lw` of `g` after it; declare `g` volatile and the load is
  nailed to its source position, the `li` slides down into the load-delay slot
  instead, and both slots read wrong. Two rows, and the only fix is to drop the
  `volatile` — `D_8009A060`.
* **A field re-read at every test, each `lbu` trailed by a redundant
  `andi 0xff`, is a `volatile`.** Without the qualifier cse folds a run of
  `if (x == A) … if (x == B) …` on the same byte into a single load, and
  combine merges the zero-extension into it, so the C comes out ten
  instructions short of a target that reloads six times. The `andi` is the
  half that cannot be explained any other way: `lbu` already zero-extends, and
  the separate extension only survives because the volatile MEM is barred from
  the cse table. `FieldMain` reads `FieldState.eventCmd` this way.
  The corollary is worth as much as the rule: reading it through a
  `volatile u8*` local makes that pointer the base register the *whole*
  surrounding block is then addressed off — `0(s1)` for the byte itself and
  every other member of the struct at its offset minus one. Spelling those
  members as `D_8009ABF4.<member>` instead lets gcc pick its own anchor (it
  chose `&prevFieldId`), which costs a second callee-saved register and 37
  rows.
* **`volatile` on a 16-bit global gives `lhu` plus a separate `sll`/`sra`.**
  The plain type folds the sign-extension into the load and gives `lh`, and a
  plain `u16` read into a `u8` field narrows all the way to `lbu`. So when the
  target loads a halfword unsigned and *then* widens it by hand, the
  declaration is volatile, not the expression — an `(s16)` cast on a
  non-volatile `u16` produces `lh` and is a third wrong answer.
  `D_800965EC`, `g_FieldNextModule`, `D_8009AC1E` and `D_8009AC18` are all
  this in `src/field/field.c`.
* **A local aggregate initializer's `.rodata` blob is emitted *before* the
  function's jump table.** gcc writes the constant pool ahead of the function
  body and the table during `final`, so a unit whose table has to sit at
  `.rodata+8` wants the local written as `RECT clip = {0, 0, 480, 472};` and
  *not* as four field stores — the opposite of the "write the fields" advice
  two bullets up, which is about a target that has no blob copy at all. The
  tell is `lwl`/`lwr` from a `.rodata` symbol into the frame. `FieldMain` puts
  `jtbl_800A0008` at offset 8 exactly this way.
* **Read a global through a struct pointer when the target hoists its load
  above nearby stores.** `*(u16*)(g_FieldTriggers + 0xA)` is a load through a
  computed pointer and may alias the plain `extern` stores in front of it, so
  gcc leaves it where it is; `((FieldTriggerHeader*)g_FieldTriggers)->
  camHeightBias` sets `MEM_IN_STRUCT_P` and `true_dependence` then lets the
  load float to the top of the block. Seven rows in `FieldMain`, and it is the
  same lever as the `AddBackgroundToRender` aliasing bullet run the other way:
  there a scalar had to *become* a struct to stop being hoisted, here a
  pointer deref has to become a struct member to *start*.
* **A `u8` switch selector wants a `u8` local, not a `u32` one.** The switch
  needs the value twice — once for `do_tablejump`'s bounds check and once as
  the table index — so combine cannot fold the zero-extension into the load
  and the target has `lbu` plus `andi`. Assign it to a `u32` and the extension
  disappears into the load. Three rows in `FieldMain`'s exit dispatch.
* **`lui r/addiu r/op 0(r)` against your `$at` macro is usually a second
  reference — and when there is no second reference, it is `volatile`.** gcc
  keeps a symbol's address in a general register once cse has a second use for
  it, and maspsx's two-instruction `%hi`/`%lo(at)` expansion is what a single
  use produces. But a **volatile** MEM is never entered in cse's table at all,
  so its address is not folded back into the MEM and stays in its own pseudo —
  the register form, off one reference. `FieldMain` writes `D_8009AC1A` once
  and reads `D_8009AC3C` once, and both need `extern volatile`; a volatile
  cast at the access site (`*(volatile s16*)D_8009AC1A = 2`) measures the same.
  It was worth 12 rows there, not the 4 the cluster was quoted at, because the
  two extra registers renamed a third of the function — and an earlier version
  of this bullet recorded volatile as measured and rejected, which was wrong.
  What genuinely does *not* move it: scalar `extern` against a one-element
  array, the struct-member spelling of the same address, and a named pointer
  local (worse). The other way to reach the register form is the symbol_ref
  bullet above — give the address a second reference by spelling a neighbour
  as an offset from it.

  **There is a third route and it is a fact about the symbol rather than
  about the function: a `D_` scalar the target addresses through a
  `lui`/`addiu` pair with `0(reg)` is a *member of a larger object*, not a
  standalone global.** An operand of the form `<sym>+<offset>` is not the same
  thing to maspsx as a bare `<sym>`, so a member reference comes out as one
  `lui`/`addiu` in a register that serves the load *and* the store, where the
  bare symbol is re-expanded through `$at` at every access. `D_8009D260` in
  `src/main/1255C.c` is `Savemap.gil` — `Savemap` is `0x8009C6E4` and `gil` is
  at `+0xB7C` — and rewriting the two `SystemMenuAddPartyGold` /
  `SystemMenuRemovePartyGold` bodies to say `Savemap.gil` matched both
  **outright**, from 10 rows / +2 instructions and 8 rows / +1. The two park
  notes there were forty lines of allocator theory (`$v0` against `$a1`,
  "gcc 2.7.2's refusal", "needs a permuter run or a toolchain lever") that had
  measured a file-scope pointer, a `volatile` pointer, a `section(".data")`
  attribute and four spellings of the local — and not once asked *what the
  symbol is*. The base symbol is what matters, not struct-ness: a
  `(u32*)&Savemap.gil` pointer local reaches the same form.

  So the check, before any codegen reasoning about an address that will not go
  into a register: subtract the `D_` address from every big object declared in
  `include/game.h` and `src/<ovl>/<ovl>_private.h` and see whether the
  difference lands on a member. `grep -n 'Savemap = ' config/sym_extern.us.txt`
  gives the bases; the struct's own `/* 0xNNN */` offset comments give the
  rest. It costs a minute and it is the difference between a match and a park.

* **`%gp_rel(<sym>)($gp)` in the target where your build has `%hi`/`%lo` is a
  *declaration* fact: in a `-G<n>` unit the small globals are written as
  tentative definitions, not as `extern`s.** cc1 does not decide the
  addressing form — it emits every access as the plain `lw $2,<sym>` macro and
  lets the assembler expand it. What `-G<n>` decides is where cc1 puts the
  objects *this unit declares*: an object no larger than the threshold goes to
  `.sdata`/`.sbss`, and a **tentative definition** (`s32 D_80062DF8;`, file
  scope, no `extern`, no initialiser — the ordinary 1990s spelling for "this
  global lives somewhere in the program") becomes a small `.comm`. maspsx then
  addresses anything in `.sdata`/`.sbss`/small-`.comm` through `$gp`. Declared
  `extern` instead, cc1 emits only `.extern <sym>,<size>`, nothing lands in
  small data, and every access is two or three instructions where the target
  has one.

  Two build-side pieces are needed and both are in `tools/ninja/gen.py`:
  `-G<n>` has to be passed to **maspsx** as well as to cc1 (it is what sets
  maspsx's `sdata_limit`; the default 0 means "no symbol is small"), and
  `--use-comm-section` has to go with it, or maspsx emits its own `.sbss`
  block for the tentative definition and the link sees two definitions of a
  symbol `asm/us/<ovl>/data/*.bss.s` already defines.

  **Do not reach for `.extern <sym>,<size>` instead, however much it looks
  like the intended mechanism.** Teaching maspsx to treat a small `.extern` as
  gp-addressable is a four-line change that makes the accessors match and then
  fails the link: of the 39 small externs across `src/main/18B8.c` and
  `src/main/1255C.c`, only 7 are inside the ±32K window around `_gp`, and the
  other 32 come back as `relocation truncated to fit: R_MIPS_GPREL16`. The
  set of gp-addressed symbols is chosen by the *source* — which globals a unit
  spells as tentative definitions — and it is not recoverable from sizes.
  maspsx needs no change; it already does the right thing for `.comm`.

  The tell costs one command, and the answer is a list rather than a verdict:

  ```shell
  grep -ho '%gp_rel([A-Za-z_0-9]*)' asm/us/main/nonmatchings/1255C/*.s \
    | sed 's/%gp_rel(//;s/)//' | sort -u
  ```

  Every name it prints is a global that unit declares as a tentative
  definition; every other data symbol it touches is an `extern`. In
  `src/main/1255C.c` that is 39 names against 107, and it is why the file's
  existing C already opens with `s8 D_80062EBC = 0;` and friends rather than
  with a block of `extern`s. **37 of that file's 52 remaining functions touch
  at least one gp-addressed symbol**, so until the flag and the spelling are
  both right none of them can match, and every one of them reads as an
  ordinary `%hi`/`%lo` addressing residue.
* **A scaled subscript folds the symbol into the address register; a
  pre-scaled byte offset does not.** This is the whole of the "`$at`
  rematerialisation wall" that a dozen park notes in `src/field/` describe.
  `arr[i * 189]` makes the address `(plus (symbol) (mult (reg) 2))`, so gcc has
  to compute the scaled index into a pseudo anyway and folds `%hi`/`%lo` of the
  symbol into that same `addu` — one base register then serves every access in
  the function. Hand gcc the byte offset instead and the address stays
  `(plus (symbol) (reg))`, which it leaves in the `mem`; the assembler then
  rebuilds it through the `$at` macro at each use, three instructions a time,
  which is what the original does:

  ```c
  off = page * 378;                              /* bytes, not elements */
  px = *(s16*)((u8*)D_800E0748 + off);
  *(s16*)((u8*)D_800E0748 + off) = px + x;
  ```

  The tell is `lui at / addiu at / addu at,at,<reg> / op 0(at)` repeated in the
  target where your build has one `lui`/`addiu` pair and an `addu` into a
  named register. Measured on `FieldDebugPageAddPos`: 26 rows as
  `D_800E0748[page * 189] += x`, 0 in the form above. It also unblocked
  `AddStrNextDebugRow`, which CLAUDE.md previously recorded as having no known
  fix.

  **It applies to packet arrays too, and that is where the load-delay `nop`s
  live.** `((LINE_F3*)D_800E3B28)[rb * 24 + rrect]` makes gcc compute
  `(rb * 24 + rrect) * 24` as one chain; the byte-offset form
  `*(LINE_F3*)(D_800E3B28 + rb * 0x240 + rrect * 0x18)` computes the two
  products separately, and gcc then reaches for the target's own shift
  decompositions (`(x<<3 + x)<<6` and `(x<<1 + x)<<3`). Across 45 sites in
  `FieldDebugRenderPage` that is **913 rows to 780**, and it took the excess
  load-delay nops from `+14` to `+3` — the separate products give the
  scheduler something to put in the slots. When a diff is mostly `nop` surplus
  against an `addu`/`sll` deficit, this is the first thing to try.

  **And count the target's expansions before choosing how often to assign the
  index local.** How many times the original re-derived `page * 0x17A` is
  countable, not guessable: `grep -A1 'sll .*, 6' <target.s> | grep subu`
  gives the number and the addresses — six for `FieldDebugRenderPage` against
  the one our source had. The five extra assignments are *exactly additive*
  (+6/+7/+5/+5/+4 instructions), which is precisely why a subset of them that
  happens to hit the length is not evidence of anything.
* **A compound assignment computes the address once, by construction.**
  `a[i] += x` expands the lvalue a single time and reuses the result for the
  load and the store, so no spelling of the index can give you two `$at`
  expansions. Split it into a load into a named local and a separate store
  first, then apply the byte-offset form above; on `FieldDebugPageAddPos` the
  split alone was worth an instruction count (26 changed / 3 inserted → 25
  changed / 0 inserted) before the addressing was touched at all.
* **A base an already-live symbol register can be adjusted to is written as an
  offset from *that* symbol, and it needs a struct to keep the two constants
  apart.** `addiu s0,s0,-0x10` / `addu s0,s1,s0` / `sh v0,0xc(s0)` is not cse
  relating two symbols — it is one source expression `D_800E0758 - 0x10 + off`
  reaching a field at `+0xC`. Spelled as pointer arithmetic (`rows - 4`) the
  front end folds both constants in the tree and you get `sh v0,-4(s0)`
  instead; the split survives only when the `+0xC` arrives at RTL as a
  `COMPONENT_REF` on top of an address that already is `(plus (const) (reg))`,
  because `plus_constant` will not fold a constant past a register. So declare
  the record and cast to it:

  ```c
  hdr = (FieldDebugPageHdr*)(D_800E0758 - 0x10 + off);
  hdr->headRow = *(s16*)((u8*)D_800E0754 + off) + 1;
  ```

  A named local for the pointer is required as well — inline, the same
  expression folds. This is what `AddStrNextDebugRow` needs and it is worth 9
  rows; `AddColorStrNextDebugRow` reaches its colour byte the same way, as
  `D_800E0758 + 0x150 + off`.
* **One pointer pair per loop, even for loops that do unrelated work.** A
  single `p0`/`p1` pair reused by four setup loops is live across every call
  between them, so gcc gives it a callee-saved register and every buffer base
  is materialised into one; the original's first bases die in their own
  preheader and sit in `$v0`. Separate variables per loop is the fix, and in
  `FieldDebugInitBuffers` it was the difference between 59 rows and 12. Pair it
  with the `&base[i * stride]` addressing from the `FieldModelLoadBcx` bullet
  above — bumped pointers are bivs and gcc biases the reduced base to whichever
  byte offset is referenced most (here `+7`, from `setcode` plus
  `setShadeTex`'s read-modify-write), which turns `3(v1)`/`7(v1)` into
  `-4(a0)`/`0(a0)` and is invisible in the loop body.
* **Taking a local's address is the only way to move its stack slot from one
  pool to the other.** A function's frame holds two kinds of slot and they are
  filled at different times: declared locals get theirs from `assign_stack_local`
  during expand, reload's spills get theirs afterwards, so every declared local
  sits below every spill and a `u8 unusedLocals[0xNN]` can only push the spill
  slots *up*. When a target keeps a counter at a lower offset than your build
  does and no pad size reaches it, the counter is *addressable* in the original:
  `TREE_ADDRESSABLE` sends it through `put_var_into_stack` and it is allocated
  with the declared locals instead. One never-dereferenced pointer is enough —

  ```c
  u16 sprite34Count;
  u16* addrOfSprite34Count;
  ...
  addrOfSprite34Count = &sprite34Count;   /* first statement; never read */
  ```

  — and it costs no instruction, because the store to the pointer is dead.
  `FieldBackgroundInitPackets` in `src/field/field.c` needs it for exactly one
  of its two run counters, which lands at `0x28(sp)` where the target has it;
  applied to the other counter, to the loop count, to `white`, to `tpages` or
  to `run` it measures 74 to 165 rows against 55. Two spellings that look
  equivalent and are not: dereferencing the pointer at the use site as well is
  worse the moment there are two such sites (82/12 against 55/10, the second
  read going through a pointer that may alias), and declaring the counter as a
  one-element array indexed at every use is completely inert — gcc keeps a
  one-element local array in a register, so an array is not a way to spell
  "put this on the stack". Neither the counters' declaration order nor the
  pad's position among them changes anything.

  The tell is precise, and it is worth reading before reaching for the pad: a
  run of `N(sp)` rows all off by the *same* constant means the pools are the
  right sizes and one value is in the wrong pool, where a diff that also moves
  the frame and the saved-register offsets means the pad itself is wrong.

  **But read the slot *spacing* before deciding a value is in the declared
  pool at all.** `expand_decl` packs a declared local at its own type's
  alignment, so three address-taken scalars come out at 0x18 / 0x1C / 0x1E;
  reload's `alter_reg` rounds every spill slot to `BIGGEST_ALIGNMENT`, so
  three spilled pseudos come out at 0x18 / 0x20 / 0x28. Eight-byte spacing
  between memory-resident scalars is therefore proof they are reload spills
  and *cannot* be reached by taking addresses -- and since spills are
  allocated after every declared local, there is no way to put a dead pad
  above one. A pad and an address-take that between them fix the frame size
  will do it by pushing the real values into the wrong slots, which is the
  `FieldBackgroundInitPackets` trap in the bullet above seen from the frame
  side.
* **A spilled pseudo is spilled in its own mode, so a counter's declared
  width decides `sh`/`lhu` against `sw`/`lw` at every one of its uses.** When
  a value lives on the stack because reload put it there -- not because its
  address was taken -- `alter_reg` builds the MEM from `PSEUDO_REGNO_MODE`,
  so `s32 count` gives `sw`/`lw` and `u16 count` gives `sh`/`lhu` for the
  identical program. A diff whose whole residue is `sw +N / sh -N / lw +N` on
  one stack slot is a *declaration*, and it reads exactly like an allocation
  problem: `FieldBackgroundInitPackets` carried that cluster for three
  sessions, and `tools/width_sweep.py` had swept the dimension and reported
  the wrong width as the winner -- correctly, because a frame pad was holding
  the counter at an offset that was wrong at every width. Read the target's
  opcode at the slot (`lhu` means `u16`, `lh` means `s16`) rather than
  ranking widths by rows.
* **A frame that is larger than your code needs, with the extra between the
  outgoing-argument area and the register saves, is a local you cannot see.**
  gcc's `expand_decl` gives every aggregate local a stack slot whether or not
  any use of it survives, so a function can carry dead bytes that no
  instruction names. Read the saved-register offsets: `sw ra,0x48(sp)` against
  your `sw ra,0x28(sp)`, with the same five registers saved, means exactly 0x20
  of locals you have not declared. Reserve the slot — `u8 unusedLocals[0x20];`
  — and say in the comment that its identity is not recoverable, rather than
  inventing a type for it. `FieldDebugInitBuffers` needs 0x20 this way, and the
  frame alone was 12 of its 70 rows.
* **`andi <reg>,<reg>,0xffff` on a value that is only ever passed on is a `u16`
  local.** An `s32` has no widening node and emits nothing; the mask is the
  declaration. `FieldDebugInitBuffers`' `tpage` is the case, and reading it as
  noise costs the whole tail of the function.
* **In a `-G` unit, `%gp_rel` against your `%hi`/`%lo` is a *size* fact about
  the object, and the escape is to index through a cast.** `src/main/18B8.c`
  carries `//! G=8`, so gcc 2.6.3's `ENCODE_SECTION_INFO` marks every object it
  can see whose size is 1..8 bytes as small data and addresses it in one
  instruction off `$gp`. Anything larger, and anything whose size it does *not*
  know — an `extern T x[];` with no bound — keeps the ordinary `lui`/`%lo`
  pair. So a target that reaches two neighbouring globals two different ways
  (`sb $a0,%lo(D_80062E10)($at)` for one, `sw $zero,%gp_rel(D_80062E18)($gp)`
  for the next word along) is telling you the original compiled them in
  *different translation units*, with the `%hi`/`%lo` one declared `extern` and
  unsized. splat has merged those units into one `.c`, so the definition is now
  visible and gcc small-datas it.

  Indexing through a cast is what gets the addressing back without moving the
  definition: `((u8*)&D_80062E10)[i] = x;` on an `s32 D_80062E10 = 0;` emits
  `lui at,%hi / addu at,at,<i> / sb %lo(at)`, exactly the target's form, where
  a walked `u8* p = (u8*)&D_80062E10;` makes `p` and the bound both givs and
  costs an instruction (`slt` against a computed end pointer instead of
  `slti 8`). `func_80014C44` measured 0 rows through the cast and 7 rows /
  +1 instruction through the pointer; the park note beside it had said the
  function "needs to be in a different file than D_80062E10", which was the
  right diagnosis and the wrong conclusion. Note the size rule is a *ceiling*,
  not a floor: declaring the same eight bytes as `u8 D_80062E10[8]` would still
  be small data, because 8 is `<= G`.

  **The other direction — `%gp_rel` in the target where you emit `lui`/`%lo` —
  is not reachable from C at all, and it is worth knowing before spending a
  budget on it.** cc1 does *not* emit the gp form: with `-mgas` it writes a
  bare `sw $2,D_80062E18` and puts the object in `.sdata`, and the **assembler**
  decides. `mipsel-linux-gnu-as` gp-addresses a symbol only when it can see it
  defined in a small-data section *in the same object*, and `tools/ninja/gen.py`
  passes `as` `-G0`, so an `extern` never qualifies however it is declared. The
  three spellings that look like they should work all measure to the row:
  `extern u8 SYM[1];` read as `SYM[0]`, a tentative definition `u8 SYM;` in the
  unit (which becomes a `.comm` that `-G0` puts in `.bss`, not `.sbss`), and a
  volatile cast at each access. So a diff whose every row is
  `%gp_rel(SYM)($gp)` against `lui/%lo` — one extra instruction per access, and
  otherwise instruction-for-instruction correct — is a **build-configuration**
  finding, not a codegen one: the original defined those objects in the
  translation unit that referenced them, and reproducing that needs the
  overlay's `.bss` imported into C plus `-G8` on the assembler. Park and say so.
  `src/main/18B8.c` has three parked bodies in exactly this state and **30 of
  its 154 remaining functions reach a `.bss` symbol this way** —
  `grep -rho '%gp_rel([A-Za-z0-9_]*)' asm/<ovl>/nonmatchings/<unit>/*.s` against
  the unit's `.sdata` symbol list is the one command that sizes it.
* **Two extra instructions around an indexed-symbol store — an `addiu at,at,%lo`
  before the `addu` and a `nop` after the load — are maspsx's aspsx-version
  switch, not your C.** Both sides reach the same `$at` macro; `maspsx.py` sets
  `nop_at_expansion` and `addiu_at` for `--aspsx-version` **below 2.30**, so the
  target's `lui at / addiu at,at,%lo / addu at,at,<idx> / sb 0(at)` and your
  `lui at,%hi / addu at,at,<idx> / sb %lo(at)` are the same pseudo-instruction
  through two assemblers. Nothing in C changes which macro gcc emits, so this is
  a park with a one-line diagnosis rather than a search. Check the version knob
  before assuming the unit is simply mis-declared, though: `//!` sets one
  assembler for the whole translation unit, and a splat-merged unit (the `file
  cut between ...` comments in `src/main/18B8.c`) can legitimately contain
  functions from modules assembled at different versions — `func_8001DEF0`
  wants < 2.30 while 72 of its neighbours match at 2.34.

  **Count both sides before concluding the unit's version is right, because
  the count can say the opposite.** In `src/main/18B8.c` **77 of the 151
  remaining functions** contain `addiu $at, $at, %lo(...)` — the < 2.30
  expansion — and **none of the 72 already-matching ones do**. That is not a
  mixed unit; it is a unit whose whole assembler version may be wrong, with
  the evidence hidden because the functions that would show it are exactly the
  ones nobody has written yet. `tools/ninja/gen.py` now takes `ASPSX=<ver>` in
  the `//!` header to set the assembler alone (`PSYQ=` moves cc1 with it,
  which is not what you want here), and under `//! G=8 ASPSX=2.21`
  `build/us/src/main/18B8.c.o` is **byte-identical** to the 2.34 build —
  measured with `md5sum`, twice, with the ninja output visible. So the change
  is free for everything currently written and would unblock 77 functions.

  It is *not* landable yet, and the reason is a feedback loop in the build
  rather than anything about codegen. Changing the `//!` line regenerates
  `build.ninja`, which makes ninja re-run **splat for `main`**; splat's
  `symbol_addrs_path` for `main` includes the untracked
  `config/sym_export.us.txt`, which is itself generated from `main.elf`, so
  the pass is not a fixpoint. After it, `sym_export` names `0x80062f58`
  `g_AkaoVolMulMusicSlideStep`, splat therefore stops writing
  `D_80062F58 = 0x80062F58;` into `build/us/undefined_syms.cnfgmenu.txt`, and
  `src/menu/cnfgmenu.c` and `src/menu/bginmenu.c` — which reference the
  address by its `D_` name — fail to link. The give-away that the assembler
  flag is innocent: `rm build/us/undefined_syms.cnfgmenu.txt` and rebuild with
  the header **unchanged** reproduces exactly the same failure, and a plain
  `touch src/main/18B8.c` (which relinks `main` but does not regenerate
  `build.ninja`) does **not**. The underlying discrepancy is one address —
  `config/symbols.main.us.txt` puts `g_AkaoVolMulMusicSlideStep` at
  `0x80062F2C` while `main.elf` links it at `0x80062F58`. Fix that and the
  assembler switch is a 77-function unblock. Until then, leave `//! G=8`
  alone — and know that **any** change that makes splat re-run for `main` can
  surface this out of nowhere, in an overlay you did not touch.

  All of the above was re-measured from scratch on an isolated Docker build
  volume, because the first pass was taken while seven worktrees were sharing
  one (see *Working in parallel*) and produced a *different* and wrong story —
  a corrupt `psxsdk.c.o`, a `sym_export` that appeared to change on its own,
  and two "reproducible" failures that landed on different overlays. **A
  build-system finding measured on a shared volume is not a finding.** If a
  failure cannot be attributed to a file you edited, check the volume before
  spending anything on the symptom.
* **`setShadeTex` is `ori 0x1`, `setSemiTrans` is `ori 0x2`.** Both are a
  read-modify-write of byte 7 of the packet and look identical in a diff except
  for the constant; do not assume a primitive-setup loop sets transparency
  because the neighbouring loop does.
* **cse links two symbolic constants only when they share a `symbol_ref`
  base, so an address written off the neighbouring object comes back for
  free.** `sb v0,-0x20($a2)` where `$a2` holds one global and the store is to
  a *different* global 0x20 below it is not cse being clever about addresses —
  it is one source expression. Spell the second object as an offset from the
  first (`(KawaiColorFadeSlot*)(D_800DFE1C + 0x20)` for a table that sits just
  past a scratch buffer) and cse's `use_related_value` hands the scratch's own
  address back as `-0x20($a2)` at every use, including as a call argument.
  Named through its own symbol the two are unrelated and gcc materialises a
  second base register. The three KAWAI colour/lighting handlers in
  `src/field/field4.c` all need this.
* **A call with a stack argument precomputes *every* argument into a pseudo,
  so a bare constant or a symbol address passed to it is competing for a
  register.** `expand_call` takes the must-preallocate path as soon as one
  argument goes on the stack, and cc1's `-dr` dump then shows
  `(set (reg 88) (symbol_ref "D_800A15E4"))` and `(set (reg 89) (const_int
  480))` emitted ahead of the call -- for a five-argument
  `SetDefDrawEnv(&env, 0, 0, 0x280, 0x1E0)`, both. That is where an otherwise
  inexplicable `li s1,0x1e0` in a target's prologue comes from, and it is not
  a source-level hoist you have to reproduce.
  What you *do* have to reproduce is whether the pseudo dies. With exactly one
  use it dies at the `move a0,<pseudo>`, combine folds the pair into
  `lui a0`/`addiu a0`, and the pseudo costs nothing; with a second use anywhere
  in the same extended basic block it lives, takes a callee-saved register, and
  every s-register after it renames. The second use is usually cse substituting
  the pseudo for the same symbol later in the function -- so a global whose
  address is both passed to such a call *and* indexed further down is one
  callee-saved register more expensive than the target, and the whole diff
  reads as allocation noise. `func_800A0C58` in `src/dschange/dschange.c` is
  parked on exactly this. Neither the array-versus-scalar spelling of the
  object, nor a `do { } while (0);` boundary between the call and the later
  use, reaches it (both measured, both inert): what the original must be doing
  is denying cse the match, which usually means the later use names a
  *different* symbol -- in that target, the byte store `&env[0].isinter` kept
  in a register and the base derived from it as `addiu s1,s1,-0x10`.
* **A dead assignment before a call is how a value gets a callee-saved
  register.** `done = 0;` at the top of an arm, where nothing reads it until
  after the call, makes the variable live across the call, so global-alloc puts
  it in `$s1` instead of a caller-saved temp — and the frame, the saved-register
  list and every later allocation follow. sched2 then sinks the `move s1,zero`
  down to wherever it fits, typically a delay slot after the call, which is why
  the target looks like the assignment was written there. Reading the position
  back out of the asm gives you a caller-saved register and a different frame.
  `KawaiFadeModelColor` and `KawaiColorFadeBelowLvl` both need it.
* **`beqz` / `li 1` / `beq` / `j default` on one loaded byte is a `switch`,
  not a chain of `if`s.** That is `expand_end_case`'s compare chain for a
  two-case switch: the selector is loaded once, and the `li v0,1` it
  materialises as the second compare constant is still in `v0` on the default
  path, so a default that returns 1 costs nothing. Two separate `if`s reload
  the selector and lay the blocks out the other way round. Read the return
  values off the arms while you are there — all three KAWAI handlers had them
  wrong in the m2c seed, and no amount of codegen work reaches a function that
  returns the wrong number.
* **A body duplicated in both arms of an `if`/`else` is not the same as one
  block reached by two `goto`s.** Duplicated, cross-jumping merges only the
  common tail and cse still knows what the variables held on the way in — so
  `done |= 1` right after `done = 0` folds to `li a1,1`. Shared, the block has
  two predecessors, cse knows nothing, and the target's `ori s1,s1,0x1`
  survives. Write the guard as `if (cond) goto skip;` in both arms with the
  clamp after them. The tell is a constant materialised where the target
  or-s into a register.
* **A tail that several arms share is cross-jumped into one, and the way out
  is to give each arm its own local for the value it stores.** The standing
  bullet below says the only lever is the ref count of the index; that is
  true when the arms genuinely store the *same* pseudo, and the commoner case
  is that they need not. Three arms ending `D_801144CC = N; D_80113F28 =
  *triId;` merge into a single store site because the second statement is
  register-identical in all three; written `triN = *triId; D_801144CC = N;
  D_80113F28 = triN;` with one `triN` per arm, each arm reloads into its own
  register and gcc merges only the arms that happen to agree -- which is what
  a target with more `%hi` references to a symbol than your source has store
  sites is telling you. `FieldEntityWalkmechCross` in `src/field/field2.c`
  needed it, and where the read sits matters: before the neighbouring store
  it is worth three instructions more than after it. Count the target's
  `%hi`/`%lo` pairs per symbol first -- six against four is three store sites
  against two, and that is a fact about cross-jumping, not about registers.
* **Two switch arms that end in the same store are cross-jumped into one —
  and the only lever is the ref count of the index.** gcc runs `jump_optimize`
  with cross-jumping *after* reload, so two arms merge exactly when their
  emitted tails are identical **including registers**; a target that stores
  twice, with a different index register each time, is telling you its
  allocator happened to pick differently, not that it did anything structural.
  Nothing about the control flow reaches it — case order, `goto` polarity,
  operand order, separate per-arm index locals and both spellings of the
  stored value all measured 21 rows on `FieldEntityBgTriggerActivate` in
  `src/field/field2.c`. What does reach it is deleting the `old` local and
  reading the array element inline at each use: the extra reference raises that
  quantity's priority in `block_alloc`, it wins a register before the index
  does, and the two arms come out on different registers, which is what the
  original has. The tell is a `j` into the *other* arm's store with the stored
  value in its delay slot, and a jump-table bounds check whose branch target is
  four instructions short of the target's.
* **The same `goto` spelling does not always give the same branch polarity.**
  In `KawaiFadeModelColor` the red and green channels take
  `if (cur < target) goto skip;` and come out with the branch inverted around a
  jump to the clamp, exactly like the target; the blue channel, identical in
  every other way, gets a direct `bnez` instead, and neither operand order nor
  a ternary moves it. Writing that one arm out the long way — `if (cur >=
  target) goto clamp;` followed by `goto skip;`, with the label before the
  clamp — reproduces the target's polarity. When one instance of a repeated
  pattern refuses to follow the others, spell its control flow explicitly
  rather than looking for a reason.
* **A one-bit test against a *variable* shift wants the mask in a named
  local, or combine turns the mask round.** `if ((w & (1 << (15 - sh))) == 0)`
  written inline compiles to `srav <t>,w,<n>` / `andi <t>,<t>,1` -- combine's
  `simplify_comparison` rewrites `(x & (1 << n)) != 0` into `(x >> n) & 1`,
  which is one instruction shorter and inverts the branch with it. Hoisting
  the mask to `s32 mask = 1 << (0xF - sh);` gives the target's
  `li 1` / `sllv` / `and` / `beqz`, because the shift then has its own insn
  and there is no `and` for the comparison to fold into. `func_800D3548` in
  `src/battle/battle2.c` measured 19 rows inline, 13 with the mask named but
  the arms the other way round, 7 with the arms right and the mask inline,
  and **matches** with both. Two knobs that are each partial and jointly
  decisive, which is the paired-lever shape again -- and neither is visible
  as anything but register noise, since the wrong form is a *shorter*
  instruction sequence that computes the same flag.
* **`x * 16` and `x << 4` are not the same expression to cse.** Both expand to
  an `ashift`, but MULT_EXPR and LSHIFT_EXPR reach RTL as distinct rtx and cse
  unifies only identical ones — so a value written the same way twice is
  computed once and kept live across the branch between the two uses, and a
  value written `* 16` in the comparison and `<< 4` in the store is computed
  twice. The tell is a `move` preserving the *unshifted* value past a compare
  that clobbers it, plus a second shift in the branch delay slot (reorg pulls
  it out of the fall-through block). Spelling both the same way instead leaves
  the delay slot as a `nop`. `FieldEntityAnimationUpdate` in `src/field/field2.c`
  needs the two spellings, and it is worth ten rows of register naming.
* **A dead local does not have to be an aggregate to reserve frame.** The
  `unusedLocals[0xNN]` idiom above is about a *sized* hole; when the target's
  frame is the minimum 8 bytes and your code needs none, a single unused scalar
  (`s32 unusedLocal;`) is enough, and `u8 unusedLocals[1]` gives the identical
  frame. Do not scale the array to the frame size in that case —
  `u8 unusedLocals[8]` measures 0x10, not 8. **In a leaf function neither of
  those reserves anything**: with no call and no saved register the prologue is
  elided outright and a 1-byte or scalar local goes with it. `u8
  unusedLocals[4]` is the smallest that gives the 8-byte frame there, and [8]
  and `s32[2]` measure the same 8 — `FieldDebugRenderString` in
  `src/field/field5.c` needs it. Read the incoming fifth argument's offset to
  tell: `lw <r>,0x18(sp)` against your `0x10(sp)` is 8 bytes of frame you have
  not declared.
* **fold swaps the operands of a sum of two multiplies, so write the index the
  other way round.** `arr[g_A * 0x1580 + g_B * 0x10]` evaluates `g_B` first —
  the two loads, the two shift chains and every register downstream follow — and
  no parenthesisation reaches it, because the swap happens in the tree, not in
  expand. To get the target's order, spell the operands in the *opposite* order
  to the emitted code. Worth 35 rows across two sites in
  `FieldDebugRenderString`; it does not apply when the two halves are separate
  statements assigning locals, which come out in source order.

  The same swap fires on a sum where only *one* side carries a constant, and
  there it is easier to miss because the arithmetic still reads correct.
  `x + (w - 2)` is folded to `(x - 2) + w`: same value, but the constant has
  moved to the other load's chain, so the two `lhu`s are emitted in the
  opposite order and the `addu`'s operands swap with them. Read which of the
  two globals the target loads *first* and put that one first with the
  constant on it -- `(w - 2) + x`. Five sites and 17 rows in
  `FieldDebugRenderPage`, at no change in length, which is what makes it look
  like scheduling noise.
* **A `u8` tested through a local and used through the pointer gives one load
  for the tests and a fresh load per use.** `c = *p; if (c < A) x = *p + 1;
  else if (c >= B) x = *p + 2; else x = *p + 3;` compiles to one `lbu` for both
  comparisons and three more for the arithmetic — cse substitutes the register
  into a compare for free, but into an add it would need an `andi` to widen the
  QImode pseudo, which ties `lbu` on cost, and cse only substitutes when
  strictly cheaper. Write `*p` in the tests as well and all five fold into one
  load, 16 rows out. The tell is a `lbu` of the same address at the top of a
  block *and* again in each arm.
* **A narrowed addend is not just a wrong constant, it is a hoisted register.**
  The `force_to_mode` trap above has a second, louder form inside a loop:
  `dst->animLastFrame = *(u16*)&anims[...] - 1;` narrows the `-1` against the
  `s16` store, and −1 in HImode is 0xFFFF, which is not a legal `addiu`
  immediate — so gcc materialises it with `li`, `move_movables` lifts that out
  of the loop as an invariant, and the frame gains a register that renames
  everything. The fix is an `s32` local **assigned immediately before the
  store**: `lastFrame = *(u16*)&anims[...] - 1; dst->animLastFrame = lastFrame;`.
  Assigned earlier, with other statements in between, it does not help — that is
  what makes the same idiom inert in `FieldDebugRenderString`'s `- 0x80`.
  `FieldUpdateAnimationState` needed it twice.
* **`x >> 4` on a signed `s16` member and `(s32)((u16)x << 16) >> 20` are the
  same value and different code.** Read through the plainly-signed member gcc
  folds the sign-extending load and the two shifts into `lh` plus one `sra 4`;
  read through a `u16` view the extension is a `zero_extend` that cannot be
  folded past the `ashift`, so the target's `lhu` / `sll 16` / `sra 20` triple
  survives. When a fixed-point counter's compare is three instructions in the
  target and two in your build, it is the *view* that is wrong, not the shift
  count. `FieldUpdateAnimationState` reads `animCurrentFrame` this way five
  times; retyping the member instead would touch every other user of it.
* **Split the model-entry lookup into an `s32` index local — but only where it
  measures.** `model = &g_FieldModelData->modelEntries[g_FieldModelLoaderData
  [modelIdx].modelEntryIndex];` compiles to the same instructions as the
  matching `StartModelAnimation` and still puts `entryIdx` and
  `g_FieldModelData` in each other's registers. Writing it as two statements
  with `s32 entryIdx` (a `u8` local is inert — the widening is the lever, not
  the split) fixed it in `FieldUpdateAnimationState` together with `s32
  modelIdx`. It is **not** a general fix for that expression: applied to
  `OpcodeFuncCanim`, `OpcodeFuncCanmEx`, `OpcodeFuncMove` and
  `FieldMoveToEntityUpdate`, which are parked on a residue that looks
  identical, both levers measure to the row. Whatever those four need is a
  different thing that only looks like this one.
* **A store through a *variable* offset invalidates the whole struct in cse,
  so read an incremented field back with `++x` used as a value.**
  `arr[i]++;` followed by `if (arr[i] == n)` looks like the store-and-read-back
  idiom and is not: the store's address is `(plus (reg base) (reg idx))`, which
  `memrefs_conflict_p` cannot disambiguate from anything, so cse drops every
  entry for that object — the re-read reloads the *index* as well as the
  element and costs four instructions. `if (++arr[i] == n)` uses the increment
  as an expression, so the value stays in the register cse already has, and the
  `andi <r>,<r>,0xffff` the target has immediately after its `sh` is that
  register's `u16`-to-`int` promotion rather than anything to explain away.
  `func_80033264` in `src/main/akao.c` is +2 instructions written the first way
  and matches written the second; a `u16*` local for the element address does
  not reach it either (11 rows), because the reload is about the store's
  address form, not about how the address was spelled.
* **A store to a symbol at a variable offset invalidates every scalar global in
  cse.** `*(u16*)(SYM + rbOff + charOff) = v;` is `(plus (symbol) (reg))`, which
  `memrefs_conflict_p` cannot disambiguate from anything, so every global read
  after it reloads — two extra `lh`/`lhu` pairs in `FieldDebugRenderString`. The
  fix is not to move the store: read the globals into locals *before* it and use
  those afterwards (`rb = g_FieldDebugRb; chars = g_FieldDebugRChars;` …
  `g_FieldDebugRChars = chars + 1;`), which is also what lets the increment
  reuse the load the index already made. Moving `x++` above the store instead
  drags its `sh` up with it and costs more.
* **A symbol base shared by two sibling address expressions wants a named local
  assigned inside the first one.** `(u16*)((pal = D_80095DE0) + dstPal * 32)`
  followed by `(u16*)(pal + srcPal * 32)` gives the symbol's address a pseudo
  whose live range starts at the first use, which is what decides whether the
  preheader holds the address in `$v0` or `$v1`. A plain `u8* pal = D_80095DE0;`
  statement at the top of the loop body is *not* the same thing — it defines the
  pseudo ahead of the index computation and costs three rows — and the
  declaration's position in the local list is load-bearing: for
  `OpcodeFuncCppal2` in `src/field/field4.c` only the slot between `src` and
  `dst` matches, every other slot scores three rows. Reach for this when the
  whole function is instruction-for-instruction right and two preheader
  registers are swapped.
* **A store whose value is an arithmetic expression may need the value in a
  named local computed *before* the address.** Written as one statement, the
  address computation and the value computation compete for the slots after a
  call and the scheduler issues a constant materialisation one slot early;
  split, the address computation fills those slots instead.
  `AddColorStrNextDebugRow`'s `hdr->headRow = row + 1` needs `s16 next = row + 1;`
  ahead of the `hdr = ...` assignment — and it has to be `s16`, since an `s32`
  local puts a widening node back into the store. It is not only arithmetic values:
  a plain **constant** stored through an indexed lvalue needs the same split,
  because `expand_assignment` computes the destination address first either
  way. `g_FieldEntity[bestId].requestTalkScript = 1;` emits the index
  arithmetic ahead of the `li`, and `talk = 1;` as its own statement emits the
  `li` first — one row in `FieldEntityCheckTalk`, and the same `s16`-not-`s32`
  caveat applies.
* **Two sibling pointers off one global base want three separate statements:
  widen, base, index.** For `p = (u16*)(SYM + id * 32)` gcc's fold canonicalises
  the tree to `(mult) + (symbol)` and expand then evaluates the multiply first,
  so the `lui`/`addiu` of the symbol lands *after* the `sll`. Every spelling of
  the address compiles to the identical bytes — `(u8*)SYM + (id << 5)`,
  `&((u16*)SYM)[id * 16]`, `&SYM[id * 32]`, `id * 32 + SYM` were all measured —
  because the difference is not in the expression at all. It is the order
  `move_movables` hoists the loop's invariant insns, which is the order
  `scan_loop` recorded them, which is insn order in the body. So write the body
  so that order is the one you want:

  ```c
  for (i = 0; i < count; i++) {
      s32 sp = srcPal;            /* the andi */
      u8* base = D_80095DE0;      /* the lui/addiu */
      u16* from = (u16*)(base + sp * 32);
      u16* to = (u16*)(base + dstPal * 32);
  ```

  Neither half works alone: widening on its own gcc folds straight back into the
  address, and a `base` local on its own puts the `lui`/`addiu` *before* the
  `andi` — one row, in the other direction. This is what unparked
  `OpcodeFuncAdpal`, `OpcodeFuncAdpal2`, `OpcodeFuncCppal` and
  `OpcodeFuncCppal2` in `src/field/field4.c` after each had absorbed a long
  budget on the address expression alone. Which of the two pointers is declared
  first still matters separately (see the ADPAL note in that file).
* **`p + n` puts the pointer first; `n + (s32)p` lets you choose.** For a
  pointer plus a scaled integer, fold canonicalises the operands so the pointer
  is `op0`, and no C spelling of the pointer form moves it —
  `param * 32 + p`, `(u8*)p + (param << 5)`, a `u_long*` base with `param * 8`
  and an `&arr[...]` base were all measured and produce the identical `addu`.
  Casting the pointer to `s32` makes it an ordinary integer `PLUS_EXPR`, fold
  keeps source order, and the multiply becomes `op0`:
  `(u_long*)(param * 32 + (s32)p)`. That single row is the whole of what parked
  `OpcodeFuncStpls` and `OpcodeFuncLdpls` in `src/field/field4.c`, the two
  functions this file records as having absorbed the largest failed budget in
  the project. Their other four rows came from splitting the address into two
  statements — the parameter load first, then the base — which is the same
  `scan_loop` ordering lever as the bullet above, applied to a straight-line
  block instead of a loop.
* **`p + i * 4` and `p + (i << 2)` are the same arithmetic and put the `addu`'s
  operands the other way round.** The bullet below says fold ranks the more
  complex operand first and that a named local is the way out; there is a
  second way out that costs nothing and does not create an induction
  variable. A `MULT_EXPR` outranks the pointer, so `base + i * 4` emits
  `addu <idx>,<base>`; an `LSHIFT_EXPR` ties with it, source order stands,
  and you get `addu <base>,<idx>`. All three of `func_800D08B8`,
  `func_800D0958` and `func_800D09D0` in `src/battle/battle2.c` are 2-9 rows
  written with `* 4` and **match** written with `<< 2`, at the same length
  and with no other change.

  Reach for the shift before the local, because the local is not free here:
  `off = i * 4;` as its own statement inside the loop becomes a giv, the loop
  grows an `addiu off,off,4`, and `func_800D08B8` measures **14** rows that
  way. Four other spellings are all exactly inert at 2 rows -- an `(s32)`
  cast on the base, `((u32*)base)[i + 0x2F]`, `0xBC + (s32)(base + i * 4)`,
  and a struct type with the two arrays declared at their real offsets --
  which is what makes this look like a wall rather than a one-word fix. Note
  the limit: this is about which *operand* is first, not about the `x * 16`
  against `x << 4` cse rule further up, which is about two spellings of the
  same value in one function failing to unify.
* **Between two integer operands fold puts the *more complex* one first, so a
  named local is how a plain variable stays `op0` of an `addu`.** The
  companion to the bullet above with no pointer in sight: `base + (p->count *
  8)` is `VAR + MULT`, fold swaps them, and the multiply's register is the
  first operand -- `addu v0,v0,<base>` where the target has
  `addu v0,<base>,v0`. Two same-class operands tie and source order stands, so
  assigning the product to a local first is what fixes it. When the target
  *recomputes* that product (because an intervening store invalidated the
  load it came from), the local is **assigned twice**, once before each use:

  ```c
  numEnt = scripts->numEntities * 8;
  lo = (scriptBase + numEnt + numExtras + (s32)scripts)[0x20];
  *slot = lo;
  numEnt = scripts->numEntities * 8;              /* the target's reload */
  *slot = lo | ((scriptBase + numEnt + (s32)scripts + numExtras)[0x21] << 8);
  ```

  One assignment is **-3 instructions** (the reload disappears) and inline is
  12 rows; the pair is what also lets the second sum accumulate *into*
  `scriptBase`'s own register, the way the target does. `FieldEventRunInit`
  in `src/field/field4.c`. Note the two sums' *later* addends keep source
  order in both spellings, which is why a target whose two copies of one
  address add their operands in different orders is telling you the original
  wrote them differently -- swapping them is a real lever and is not folded
  away.
* **`&((T*)(base + K))[i]` is an ARRAY_REF and survives fold; `i * sizeof(T) +
  K + (s32)base` is a PLUS_EXPR and does not.** For a packet at a fixed offset
  inside a big buffer, the integer sum is reassociated to `(base + K) + i *
  sizeof(T)` — `addiu <base>,K` then `addu <idx>` — where the target computes
  `addiu <idx>,K` then `addu <base>`. No spelling of the sum reaches it: casts
  to `s32` or `u32` (a conversion to a type the operand already has is not a
  NOP_EXPR at all, so the C front end's `convert` returns the operand unchanged
  and fold never sees a barrier), explicit parentheses, and named locals for
  either half were all measured and are inert or much worse. The subscript form
  works because an ARRAY_REF is not a PLUS_EXPR — there is nothing for
  `associate` to take apart. This is the lever that finished
  `FieldArrowsAddToRender` in `src/field/field2.c` after a park note had
  concluded that "the two remaining levers do not exist in C"; it had tried
  every spelling of the sum and none of the subscript. Note the limit: it only
  helps when the *whole* address is one ARRAY_REF. The same function reads
  `g_FieldTriggers + i * 0x10 + 0x230` two lines earlier, where the base is a
  runtime load that still has to be added to the scaled index, and there all
  six spellings — including `((VECTOR*)((u8*)g + 0x230))[i].vx` — give the
  identical `addu` operand order.
* **A scaled-index local can be right at *some* of its use sites and wrong at
  the rest, and the split is not guessable.** `off = i * 0x10;` used at every
  one of `FieldArrowsAddToRender`'s seven sites costs 43 rows, used at none
  costs 2, and used at exactly five of them — with `pos.vy` and one of two
  stores through the same cast keeping the inline multiply — matches. This is
  the "same packet reached two ways in one loop" idiom above, one level down:
  the asymmetry is the answer, not noise to be tidied away. Nothing about the
  target's asm predicts which sites, so this is decomp-permuter's job
  (`perm_temp_for_expr`), not a reading exercise — and it is only reachable
  once the function is already within a couple of rows, which is the practical
  case for CLAUDE.md's "correct the program first, then hand it to the
  permuter". The local's declaration slot among the other locals is
  load-bearing as well.
* **A basic-block boundary in the middle of an `if` body is sometimes
  load-bearing and no natural construct is known to produce one.**
  `FieldArrowsAddToRender` needs 7 rows' worth of one right after an addPrim,
  before the `if`'s join, and the only spellings found are `do { } while (0);`
  and `while (0) { }` — byte-identical to each other, so what the original
  wrote there emitted `NOTE_INSN_LOOP_BEG`/`END` and `expand_end_loop`'s exit
  `CODE_LABEL`. Plain braces, an inverted guard with a label before the loop
  increment, the barrier moved outside the `if`, rewriting the neighbouring
  `if` as a `goto` over its body, and moving the addPrim above its neighbours
  are all exactly 7 rows short. decomp-permuter's `perm_ins_block` is the pass
  that finds these. When one is needed, take it, and say in the comment that
  its identity is not recoverable — the same standing as the
  `unusedLocals[0xNN]` frame reservations.
* **A `u8` field and the byte behind it, stored together, are one halfword
  store.** `sh` where you have `sb`, at an offset whose field is a `u8`
  followed by a padding or `unkNN` byte, means the original wrote both at once:
  `*(u16*)&state->pcDirection = GET_PARAM_U8(9);`. `OpcodeFuncMjump` and
  `FieldEntityGatewayMapLoad` both do this with `FieldState.pcDirection`.
* **Duplicate the exit tail at every early return; cross-jumping decides where
  the merge lands.** A function whose body is wrapped in `if (cond) { ... }`
  with one trailing `PC_INC(1); return 0;` compiles to a single tail with a
  single reload of whatever the tail needs. Written instead as an early
  `if (!cond) { PC_INC(1); return 0; }` — the tail spelled out at each exit —
  cross-jumping merges only the common *suffix*, and the early copies reuse the
  value still live in a register while the copy after the body's stores reloads
  it (the stores may alias). The tell is a branch that enters the shared tail
  one or two instructions *past* its first insn. `OpcodeFuncTurnw` in
  `src/field/field4.c` matched on exactly this, from 26 rows; note this is not
  the same as the `KawaiFadeModelColor` bullet above, which is about *avoiding*
  duplication so cse keeps knowing what the variables held.

  **The sharper form: duplicate the exit tail in the arm that is *not* wrong,
  to stop two other arms merging.** Post-reload cross-jumping compares emitted
  tails including registers, so two switch arms both ending
  `beqz <r>,<tail> / li v0,1 / j <epilogue>` are merged and the function comes
  out three instructions short. Nothing you do to *those two* arms separates
  them once their values land in the same register. Writing a **third** arm's
  tail out in full — `if (fadeAdjust >= 0xFF) { PC_INC(1); return 0; }
  return 1;` instead of falling into the shared tail — gives that arm its own
  copy of the `PC_INC` block, so the conditional jumps carry different labels
  when cross-jumping compares them and the merge does not happen.
  `OpcodeFuncFadew` in `src/field/field4.c` matched on this, 2 rows to 0.
  The trap that hid it: on the shared-tail body every polarity flip of that
  guard is *exactly inert*, so the dimension reads as closed and swept — and
  the polarity only becomes load-bearing once the tail is duplicated, at which
  point the other spelling is 2 rows again. Two knobs that are each inert
  alone and jointly decisive, which is the paired-lever shape one more time.
* **A cast through a `volatile` pointer drops the qualifier, and the loads it
  frees fill the target's `nop`s.** `volatile u8* ev;` says nothing about
  `*(u16*)(ev + 1)` -- the cast produces a plain `u16*`, the MEM is not
  volatile, and gcc will hoist that load above the `*(u16*)(ev + 0x63)` store
  next to it, because two non-volatile MEMs through a computed pointer are
  exactly what `memrefs_conflict_p` cannot disambiguate in the *other*
  direction. What you see is a target with two `nop`s in load-delay slots that
  your build fills with useful work, plus an `andi 0xffff` your build has and
  the target does not: with the load hoisted its pseudo has two uses, and
  combine only folds a zero-extension into a `lhu` that feeds one. Spelling
  every access `*(volatile u16*)(ev + N)` restores the source order and both
  effects go. Worth 25 rows in `FieldMain`, from a body whose park note had
  already diagnosed the tail as "a load in the wrong place" and had gone
  looking at the *types* of the two locals instead. When a diff is denser than
  the target, check what the qualifiers survive.
* **A struct member read through a `(volatile T*)` cast gets the global-volatile
  treatment without touching the header.** `((volatile FieldState*)g_FieldState)
  ->fadeAdjust` loads `lhu` and leaves the `sll`/`sra` as separate insns, where
  the plainly-typed member folds the widening into the load and gives `lh` —
  and it leaves a bare `beqz` where no signed compare needs the extension.
  `OpcodeFuncFadew` needs this for three members at once. Reach for it when a
  diff is `lh` against `lhu` plus a shift pair on a *member*, since retyping the
  member would change every other function that touches it.
* **Read a jump table's arm count and its arm *tests* off the target before
  touching codegen.** `sltiu v0,v1,0xb` means eleven cases, not twelve; a table
  slot pointing straight at the function's tail means that case does the work
  unconditionally; and `slti v0,v0,0xff` in an arm your C tests against zero
  means the arm is a different *program*, not a different schedule. All three
  were wrong in `OpcodeFuncFadew`'s parked body and together were worth 22 of
  its 26 rows. A `.rodata+0xNN` grouping line in `diff.py` output tells you
  which table slots reach which arm — read it like a case list.
* **A base address the target rebuilds at every use is a control-flow fact,
  not an allocation fact.** Ten functions in `src/field/field4.c` were parked
  with a note saying "the `g_FieldModels` *0x84 base regalloc is the wall", and
  not one of them had an allocation problem: in every case the parked C was a
  different *program*, and the rebuilt base was the consequence. gcc 2.6.3's
  cse walks the dominator tree, so a block with **two predecessors** starts
  with nothing known — and the FF7 opcode handlers are full of such blocks,
  because the original writes the "start a fresh action" path as a block that
  *both* a failed test and a dispatch default fall into. Give the same block
  one predecessor by writing the default as its own `return` and gcc keeps the
  base live across it, which is a dozen rows out and reads exactly like
  register noise. `OpcodeFuncFmove`, `OpcodeFuncCmove`, `OpcodeFuncJump`,
  `OpcodeFuncTurn` and `FieldEventSetDirByActorId` were all this, 116 rows at
  the worst, and all five are plain matching C now. Read the branch structure
  first; the base rebuild is a symptom, and the `.s` shows you which blocks
  have how many predecessors for free.
* **Three habits that read as good C and are always wrong here.** Caching
  `g_EntityToModel[g_CurrentEntity]` (or the model pointer it indexes) in a
  local, hoisting a `GET_PARAM_U8(n)` into a named variable at the top of a
  handler, and taking a `FieldEntity*` instead of repeating the array
  expression. The original re-reads all of them at every use — `OpcodeFuncTurn`
  reads `GET_PARAM_U8(4)` and `(5)` three times each — and the locals are worth
  indexed expression, however ugly it looks.
* **But when the target *does* hold a `FieldEntity*` in a register, it holds
  one per use site, not one per function.** The exception to the bullet above
  is a run of accesses to the same model with a store in the middle: the
  target keeps a base register across them because they are one source
  expression, and it re-materialises the base for the *next* such run. Written
  as one reused local, the pseudo's live range spans everything between the
  runs and the allocator pays for it somewhere else. `FieldEntityTurnToEntity`
  needs **three** — the TurnType/TurnStep checks, the `TurnStart = Dir`
  snapshot, and the switch arms — and it matched only when all three were
  separate variables: merging the arms into the snapshot's costs 30 rows and
  merging the snapshot into the checks' costs 3. `OpcodeFuncMove` needs two
  and `OpcodeFuncTurnr` four. The tell is a `move`/`addu` into a *different*
  register than the one the previous run used, for the same address. This is
  the same lever as the "one pointer pair per loop" bullet, applied to
  straight-line code.
* **Where the merged `PC_INC(n); return 0;` tail lands is a source-shape
  decision, and it moves every branch offset in the function.** An opcode
  handler that bails out when the entity has no model reaches that tail twice
  — once from the `0xFF` test at the top, once from the state-machine arm —
  and the three ways to spell it give three different layouts. Two early
  `return 0`s: cross-jumping keeps the *later* copy, so the block sits at the
  arm near the top. A `goto advance;` with the label after `return 1`: the
  block moves to the end, but the label now has two jump predecessors, cse
  knows nothing on entry, and the `g_CurrentEntity` reload moves *inside* the
  tail. The whole body inside `if (g_EntityToModel[g_CurrentEntity] != 0xFF)
  { ... return 1; }` with `PC_INC(n); return 0;` as the function's
  fall-through end: the block lands after every return-1 path, entered by a
  `j` from the arm, with the reload left behind in the arm — which is what
  the originals do. Worth 18 rows on `OpcodeFuncTurn` and 17 on
  `OpcodeFuncTurnr`. The tell is a `lui`/`lbu` of `D_800722C4` immediately
  before a `j` whose delay slot is a `nop`.
* **m2c prints a switch's case bodies in address order, and that is the
  source order.** gcc emits the bodies as the switch body is expanded, so
  their layout is the order they were written; only the compare chain is
  rebuilt by `balance_case_nodes`. So when m2c renders `case 2:` before
  `case 1:` before `case 0:`, write them that way — it is not m2c being
  arbitrary. `FieldEntityTurnToEntity`'s three direction arms measure 65, 53
  and 42 rows as 0-1-2, 0-2-1 and 2-1-0, and only the last one goes on to
  match. The give-away without m2c is the last test in the chain: `bne
  <default>` with the arm as fall-through means that arm is written last.
* **A helper that "just" wraps an opcode usually is the opcode.** Two `void`
  handlers calling a `void` helper, each doing its own `PC_INC`, is the shape
  m2c and a first reading produce; the original often has the helper do the
  `PC_INC` and return the 0/1 the dispatcher wants, with the callers passing
  its value straight through. A `void` C function leaves `$v0` alone, so the
  callers stay byte-identical either way and nothing tells you — except the
  helper's own `.s`, which ends in `addiu v1,v1,N / sh v1,0(a0)` and an
  `ori v0,1`. `FieldEventSetDirByActorId` was 150 rows on this plus a `u8`
  parameter that is really `s16` (the `sll`/`sra` ahead of the index is the
  give-away) plus a missing degenerate-vector nudge.
* **`a < b` and `b > a` are the same test and different code.** gcc 2.6.3
  evaluates a comparison's operands in source order, so the two spellings emit
  their operand setup — two `sll`/`sra` sign-extension pairs, in the case that
  matters — the other way round, and every register downstream of them follows.
  `FieldEventSplitJoinSetTurn`'s entire 16-row residue was this one line.
  Reversing the *arms* instead (`>=` with the bodies swapped) does not do it.
* **A branch whose delay slot the target fills and yours does not may want to
  be a `switch`.** Written as two `if`s, the `li v0,K` setting up the second
  comparison is live on the first branch's taken path whenever that path
  returns a value in `$v0`, so reorg refuses to steal it and you get a `nop`.
  Written as a `switch`, `expand_end_case` emits the same constant as part of
  its own compare chain and the slot fills. Same two arms, one row —
  `OpcodeFuncJump`.
* **Repeat the whole indexed expression in a packet-filling function too, not
  just in the opcode handlers.** `POLY_FT4* arrow = &D_800E48F4[D_80114490];`
  written once and used for twenty-four member stores measures **300** rows
  against 14 for `D_800E48F4[D_80114490].<field>` repeated at every store —
  worse than the unmodified m2c seed. The cached pointer keeps the packet base
  in one register across the whole function; the target rebuilds it at each
  store. The exception is an argument that needs the address without a member,
  where the inline `&D_800E48F4[D_80114490]` is right and a local costs 4 rows
  by making gcc evaluate the index before the array base.
* **A value that a call clobbers and a result that replaces it are one
  variable, not two.** `charId = party[n]; if (charId == 0xFF) ok = 1; else ok
  = f(map[charId], g(), h());` needs `charId` alive across `g()` and `h()`,
  because the argument that uses it is evaluated last — so it takes a
  callee-saved register, and so does `ok`. Two long-lived pseudos for the two
  available registers is a coin flip, and gcc calls it the other way: pure
  `$s2`/`$s3` swap, seven rows, immune to every declared type. Write
  `ok = party[n];` and let the call's result overwrite it and there is one
  pseudo and no choice to get wrong. `OpcodeFuncSplit` needs this on both
  followers.
* **A `0xFF` sentinel compared on both sides of a call gets a callee-saved
  register.** `ok = 1; if (x != 0xFF) ok = f();` twice in a row lets cse share
  the constant between the two comparisons; the shared pseudo has to survive
  the call, so it lands in `$s1` and the frame grows by a save and a restore.
  `if (x == 0xFF) ok = 1; else ok = f();` — the same test written as an
  if/else — materialises `li v0,0xff` twice, which is what the target does.
  Worth 20 rows and the frame on `OpcodeFuncJoin`.
* **An arm that ends the frame by writing a state byte and returning a
  constant wants `break`, not `return`.** With an explicit `return 0` per arm
  gcc keeps `$v0` reserved for the return value across the arm and the state
  constant goes in `$v1`; with `break` and a single `return 0` after the
  switch, the constant gets `$v0` and is re-zeroed on the way out, which is
  what the target does. `FieldEventSplitSet`'s last four rows.
* **Two counters walking together: put both increments in the `for`.** gcc
  emits the body's `i++` before the `for`'s `j++`, so `for (j = ...; ...; j++)
  { d[j] = s[i]; i++; }` hands `expand` the increments in the order j, i — and
  with them the two scaled addresses, which then swap registers all the way to
  the `lhu`/`sh`. `for (i = ...; ...; i++, j++)` puts both in the increment
  list in written order and the loop falls into place. The tell is a loop whose
  only fault is that its two index computations are in the opposite order to
  the target; forcing the load first with a named temp does *not* work, because
  cse folds the temp away. Both `OpcodeFuncRtpal` and `OpcodeFuncRtpal2` needed
  this, and only in their second loop — the first, whose `for` counter is the
  store index, matched all along.
* **A `nop` in a zero-trip guard's delay slot means the loop's setup is
  written *inside* the guard.** In a `for`, the init list is emitted in the
  entry block ahead of the test, so reorg finds it sitting on the fall-through
  and steals it into the branch's delay slot — one instruction shorter than
  the target, with the register assignment dragged along behind it. Written as
  `p = base; if (n != 0) { bit = 1; do { … } while (n != 0); }` the `bit = 1`
  is in the loop preheader, a block reorg does not reach back past, and the
  target's `nop` survives. `func_8002A748` and `func_8002A798` in
  `src/main/akao.c` need it, and the measurement is what makes it a rule
  rather than a guess: **eleven** spellings — `for` with the init in the init
  list, `while`, a backward `goto`, both increment orders, both declaration
  orders, an index instead of a walking pointer, `s32` and `u32` for the bit,
  and `x = x | 3` for `x |= 3` — all come out at exactly 10 rows and -1
  instruction with the loop body byte-identical, and the twelfth matches
  outright. This is the mirror of the bullet below: there the hand-written
  guard is the wrong answer, here it is the only one, and the target's delay
  slot is what separates them.
* **A loop bound read from memory is not the same as a cached one, and a
  hand-written zero-trip guard is not the same as the `for`'s.**
  `count = hdr->count; if (count != 0) for (i = 0; i < count; i++)` looks like
  what `for (i = 0; i < hdr->count; i++)` compiles to and is 35 rows away from
  it — this is the exact inverse of the `FieldModelLoadBcx` bullet above, so
  read the target for which one it wants: the bound reloaded inside the loop
  means write the member, hoisted into the preheader means write the local. A
  redundant `if (i < bound)` in front of an inner loop is the same trap seen
  from the other side: the target reaches the loop through its own guard, and
  spells it against whatever register it has just proved to be zero.
* **`unsigned char` promotes to *signed* int, unlike `unsigned short`.** A
  `u8` struct field as a loop bound therefore gives `slt` and `blez`, and no
  spelling of the bound reaches the `sltu`/`beqz` pair a target may have —
  because the fix is on the other side of the comparison. Declare the *counter*
  `u32` and the compare goes unsigned: `for (i = 0; i < data->modelCount; i++)`
  with a `u32 i` emits `beqz` on the zero-trip guard and `sltu` on the back
  edge, exactly where an `s32 i` emits `blez` and `slt`. Two of
  `FieldModelLoadAndInit`'s loops need this, and it is worth reading against
  the `u16`/`s16` loop-bound bullet above: there the *bound*'s type is the
  knob, here it is the counter's.
* **A narrow-returning call's result carries a different mask for every width
  of local you store it in, and the target tells you which.** A function
  declared to return `u8` has its result masked at the call site only as far as
  the destination pseudo's mode needs: a `u8` local is a plain `move`, a `u16`
  local costs `andi 0xff` at the assignment *and* `andi 0xffff` at every
  promotion to int, and a `u32`/`s32` local costs only the `andi 0xff`. So
  three factors that all look interchangeable are not — in `OpcodeFuncMppal2`
  the target masks two of them twice and the third once, which types them
  `u16`, `u16`, `u32` with no other evidence needed. Count the `andi`s per
  value before touching anything else.
* **`(u16)(x << 1) & 0x3EU` reads the raw halfword; `(x << 1) & 0x3E` reads the
  zero-extended copy.** For a `u16` local whose promotion to int is already
  materialised (because a comparison or a right shift needed it), cse hands
  that copy to the left shift too, and combine will not narrow it back — where
  the target shifts the `lhu` destination directly. Casting the shift result
  back to `u16` keeps the whole expression in HImode and the raw register is
  used. A plain mask (`x & 0x8000`) narrows on its own and needs no cast, so a
  diff with one shift on the wrong register and every mask on the right one is
  this. The `U` suffix is a separate matter: two `u16` factors promote to
  *signed* int, so a product that the target shifts with `srl` and clamps with
  `sltiu` needs its unsignedness from the mask constant.
* **gcc 2.6.3 promotes `unsigned short` to *unsigned* int.** One `u16` operand
  therefore makes a whole comparison unsigned — `sltu` where the target has
  `slt` — even when the other side is a plain `s32`. An explicit `(s32)` cast
  on the `u16` is what puts the signed compare back; it costs no instruction,
  because the value is already zero-extended by its `lhu`.
* **A counter reset between two loops has to be reset where one definition
  dominates the second loop, or the second loop's giv initialiser is not a
  constant.** `strength_reduce` computes a reduced giv's starting value from
  the biv's value at the loop entry, so `for (...) {...} i = 0; for (...)
  { models[i]... }` gives `move a2,a0` — the array base itself — only when gcc
  can fold `i` to 0 there. Put the reset *inside* the first loop's guarding
  `if` and the zero-trip path skips it; the two paths join, the value is
  unknown, and the preheader grows an `sll`/`addu` pair computing
  `base + i * stride` from a register. The tell is two extra insns in the
  second loop's preheader and nothing else wrong with the loop.
  `FieldModelStructInit` in `src/field/field2.c` needs the reset after the
  `if`, not at the end of its body.
* **A register parameter keeps its incoming register for free, and a target
  that copies it out is telling you an allocno that *conflicts* with it took
  that register anyway — which `find_reg`'s first pass is built to prevent.**
  gcc emits `(set (reg P) (reg a0))` at entry for every register parameter,
  and `global_alloc`'s `set_preferences` turns that into
  `;; P preferences: 4`. The consequence is not just that P likes `$a0`: on
  its first pass `find_reg` ORs `regs_someone_prefers` into the used set, so
  **every allocno that conflicts with P avoids `$a0`**, and P — however low
  its priority, however late it is allocated — gets it at the end. The tell in
  your own build is that the global allocnos start at `$a1` and the
  parameter sits in `$a0` with no copy, where the target has `move t8,a0` at
  insn 0 and a loop cursor in `$a0`.

  Reaching the target's assignment needs the first pass to *fail*, so
  `find_reg` retries without the restriction — i.e. real pressure at the
  moment that allocno is allocated. If it is allocated 8th of 42 there is
  none, and the residue is not reachable from C at all: a `p = part;` copy is
  propagated away by cse, and keeping `part` live past the copy to defeat that
  costs a whole register rather than a preference.
  `KawaiSetVertexColorFromLighting` in `src/field/field4.c` is parked on
  exactly this one instruction, with `insn_histogram` reporting a single
  opcode out (`addu 34 against 35`) and every `%hi` identical. `cc1 -dg`
  prints the two lines that settle it — the allocno's `conflicts` list and
  its `preferences` line — so this is a five-minute question, not a search.

* **Count the increments in a loop's `bnez` delay slot and the two `addiu`s
  above it before reading anything else.** A counted inner loop with a walking
  pointer has one `addiu` per induction variable, and they are all in the last
  three slots of the body. A missing one is a *wrong program*, not codegen:
  `KawaiSetVertexColorFromLighting`'s last two loops were written without
  `n += 4;`, so every vertex of a textured triangle or quad was lit from the
  first vertex's normal, and the whole symptom in the histogram was
  `addiu -2`. The same reading gives the strides for free — the outer loop's
  own `addiu` is in its branch delay slot.

* **`p = base + K;` as its own statement is computed early enough to fill a
  load-delay slot; `f(&base[K])` at the use site is not.** Same address, same
  instruction, and the statement form lets sched2 issue it in the slot after
  the neighbouring load while the use-site form lands after the work in
  between and leaves a `nop`. Four `gte_strgb(&pkt[4])` calls rewritten as
  `rgb = pkt + 4;` … `gte_strgb(rgb)` are four `nop`s in
  `KawaiSetVertexColorFromLighting`. This is the inverse of the standing
  advice to repeat an indexed expression rather than name it: name it when
  the target computes the address *early*, repeat it when the target
  recomputes it at each use.
* **A parameter the target keeps in its incoming register *and* copies to a
  temporary is two source variables, not one.** A pseudo gets one hard
  register, so a target that stores through `$a1` at entry, computes a derived
  pointer into `$a1` in place (`addu a1,a1,v0`) and reads every struct field
  through a `$t0` copy cannot come from a single pointer — write
  `d = data;` after the first use and read through `d` from then on. The copy
  survives because cse only propagates within an extended basic block, and the
  uses are all in later blocks. What it buys is the argument register for the
  derived value: with one pointer the parameter keeps `$a1` and the derived
  pointer takes a `$t` register, renaming a dozen rows. The same shape shows
  up for the *first* parameter whenever a target computes something like
  `models = desc->models` into `$a0`.
* **A nested loop keeps its own copy of a shared base address.**
  `loop_optimize` walks loops innermost-first, so an invariant used inside an
  inner loop is hoisted into the *inner* preheader before the outer loop's
  copy exists, and the function ends up with two callee-saved registers holding
  the same `%hi`/`%lo`. If your build has one and the target has two, the inner
  loop's use is written in a form that lets cse relate it to the outer one.
  `FieldEventRunInit` is parked on exactly this row.
* **A global pointer re-read three times in one statement group wants a local
  — but only if the local dies before the next call.** gcc reloads a pointer
  global after any store that might alias it, so `(u8*)g_FieldScripts + ...`
  written three times is three `lui`/`lw` pairs where the target has one. Cache
  it and the reloads go, but cache it *too early* — before a call, or across
  one — and the local needs a callee-saved register the target does not have,
  which costs a save, a restore and the whole allocation. Assign it after the
  last call that precedes its uses. Worth 12 rows either way in
  `FieldEventRunInit`, in opposite directions.
* **A `move` of a variable into a scratch register immediately before a
  compare against a constant is a post-increment in the test.**
  `addu v0,s6,zero / slti v0,v0,9 / addiu s6,s6,1` cannot come from
  `if (repeat >= 9) … repeat++;` — `slti v0,s6,9` is legal and gcc emits it,
  so the copy has no reason to exist. `if (repeat++ >= 9)` needs the *old*
  value live across the increment, which is exactly one extra pseudo and
  exactly that `move`. sched2 then puts the `addiu` in the branch's delay
  slot, which is what makes the increment look as though it belongs to one
  arm. Same shape for `--`, and for a post-increment used as any operand
  rather than as a comparand. `func_800A01A0` in `src/brom/brom.c` needs it
  for the pad auto-repeat counter.
* **`bcc rX,L / delay: <insn defining rX>` is reorg copying an insn *up* out
  of the branch target, so the target block's first statement runs on the
  taken path only — read it as an `if`/`else`, not as a statement before the
  test.** The pattern

  ```
  bgez  s3, L      ; delay: addiu s3,s3,0xA
  j     L          ; delay: move  s3,zero
  ```

  reads naturally as `index += 10; if (index < 0) index = 0;` and that is not
  what it is: `fill_simple_delay_slots` will not move an insn that defines the
  branch's own operand *down* into its slot, because the branch has already
  read it and the result would be a different program. What it will do is
  `fill_eager_delay_slots` — copy the *target* block's first insn up, and
  redirect the branch past it, whenever that insn is dead on the fall-through
  path. Here the fall-through overwrites `s3` with zero, so `s3 += 10` is
  dead there and qualifies. The source is
  `if (index < 0) { index = 0; } else { index += 10; }`, whose `bgez` skips
  the then-arm, whose then-arm ends in the `j` to the join, and whose else-arm
  is the single `addiu` reorg then steals. The give-away that it is eager
  filling rather than a hoisted statement is that the branch and the `j` have
  the *same* target: a statement genuinely above the test would leave the
  branch pointing at the join and the slot holding something else.
* **A hand-written `tag = base + 7` is an ordinary insn; `base[7]` is a giv.**
  The giv's initialiser is emitted into the loop preheader by
  `strength_reduce` and reorg leaves it there, while the hand-written pointer
  is an ordinary assignment reorg happily steals into the guard branch's delay
  slot. So a target with a `nop` after `beqz <count>` and the `addiu` *after*
  it wants the offset written at the use site, not carried in a second
  variable. Eight rows across `KawaiSetModelTransparency`'s eight loops.
* **The same rule decides whether a *frame* address is derived or
  rematerialised, and there it is worth the whole function.** `&buf[4]` on a
  local array is folded by the front end to `(plus (sp) 0x14)`, and gcc
  rebuilds it from `$sp` at the use — even when `&buf` is already sitting in a
  callee-saved register because `move_movables` hoisted it out of the
  surrounding loop. Written as an index the same address becomes a giv, and
  `strength_reduce` emits its preheader init *off the hoisted base*:
  `addiu a1,s8,4` rather than `addiu a1,sp,0x14`. So spell a walking cursor
  over a stack buffer as a subscript, not a pointer:

  ```c
  j = 4;
  do { buf[--j] += n % 10; n /= 10; } while (n != 0);   /* addiu a1,s8,4  */
  p = &buf[4];
  do { *--p += n % 10;     n /= 10; } while (n != 0);   /* addiu a1,sp,20 */
  ```

  `j` has no use outside the address, so gcc drops the biv entirely and the
  two forms are otherwise instruction-for-instruction identical — the tell is
  exactly one row, an `addiu` whose base register is `$sp` where the target's
  is the frame pointer the same loop nest already holds. `func_800A01A0` in
  `src/brom/brom.c` is 0 rows one way and 2 the other, at the same length,
  with identical opcode and `%hi` histograms.
* **A giv's increment follows its biv's, in written order.** With
  `for (j = 0; j < n; j++, base += stride)` the `j++` comes first and the
  reduced `base + 7` increment after it; `base += stride, j++` swaps both.
  Same lever as the two-counter bullet above, one level down. A loop whose
  `base` is dead afterwards matches either way, because gcc drops the biv
  increment entirely — so a run of loops where all but the last are two rows
  out is this.

  **For an unrolled copy between two walked pointers the same word decides
  which combined giv's initialiser is emitted first.** `record_giv` prepends,
  so `strength_reduce` walks the list in reverse discovery order and the two
  `addiu <base>,<base>,0xc` insns land in the preheader in the order the two
  `+= 4` statements are *not* written in. `LoadLocalFieldModelAndInitAll`'s
  word copy wants `d += 4; s += 4;` — the source base's initialiser first,
  which is what `addiu a2,s1,0xc` ahead of `addiu a1,a3,0xc` says. Two rows,
  and nothing inside the body reaches it: naming the loaded word, `d[3] = s[3]`
  first, `*d = *s`, both increments in the `for` clause, and the `s`/`d`
  declaration and assignment orders were all measured and are inert or worse.
  The tell is two preheader `addiu`s with identical registers in the opposite
  order.
* **`slt` against a register where you emit `slti` means the loop bound is a
  named local — and where you assign it decides everything.** A literal bound
  is folded into the compare immediate and no spelling of the loop reaches
  `slt`; a local does, because cse stops at the loop-top label (it has a second
  reference, the back edge) and never propagates the constant into the body.
  But the local has to be assigned *after* the last call before the loop:
  written as an initialiser, or anywhere above the calls, it is live across
  them, takes a callee-saved register and rewrites the whole frame — 42 rows
  against 12 on `KawaiInitSplashPkts`, which is what made this look like a dead
  end for two sessions. Assigned at the join right below the `if`/`else` it is
  7. A walked pointer reaches the same `slt` (the end pointer is a register by
  construction) but costs a fourth saved register, so it measures worse; the
  bound local is the cheap way to get there.
* **A store whose value is read back later in the same loop body is pulled to
  the top by the scheduler, so its emitted position says nothing about source
  order.** `pkt[7] = 0x2C; … pkt[7] |= 2;` comes out as `sb` at the very top
  with the `lbu` for the read-modify-write right behind it, whatever order the
  source used. What *does* carry the source order is the run of hoisted
  loop-invariant constants in the preheader: `move_movables` emits them in insn
  order, so `li t2,0x9` before `li t1,0x2c` means the 9-stores are written
  first even though the 0x2C-stores are emitted first. Read the preheader, not
  the body, when a diff is nothing but two constants swapping registers — it
  was 6 of `KawaiInitSplashPkts`' last 7 rows.
* **`move v0,<reg>` in the `jr ra` delay slot means one `return`, not two.**
  Two `return` statements let gcc coalesce each value straight into `$v0`, and
  the delay slot stays empty; a single exit keeps the value in an ordinary
  pseudo until the very end, so the copy into `$v0` is a real insn and the
  slot-filler takes it. Write the success path as `goto out;` with the failure
  value assigned below it and one `return` at `out:`. It has to be the *same*
  variable that held whatever the function computed on the way — a fresh
  `result` local measures 22 rows and pre-setting it before the tests 30, where
  reusing the existing one matches. `FieldEntitySqrDistToLine` in
  `src/field/field2.c` needed exactly this, and the tell is unmistakable: every
  `li <reg>,-1` in the failure paths names a caller-saved register that is not
  `$v0`.
* **A value stored to a global and then needed again is read back out of the
  global, not held in a local.** With a local the pseudo is still live at the
  second use, so the arithmetic needs its own registers and sched2 cannot lift
  it into the load-delay slots of the stores around it — you get a column of
  `nop`s where the target has the computation spread through them. Written as
  `*(u32*)&D_800E4D94[0] = *(u32*)mim;` and then
  `next = (*(u32*)&D_800E4D94[0] >> 2) * 4 - 0xC;`, cse hands the just-stored
  register straight back, the shift is done *in place* in it, and the slots
  fill. `FieldLoadMimToVram` in `src/field/field.c` does this three times and
  it was worth 15 rows; no declaration order, variable count, or spelling of
  the arithmetic reaches it. Re-reading the *source* word instead of the
  destination is a different thing and costs 18 — it is the store's register
  that has to be reused.
* **Two references to a global at offset 0 promote its address to a register;
  reaching one of them through a neighbour keeps the `$at` macro.** The
  companion to the `$at` rematerialisation idiom above: `*(u8**)&D_800E4D90[0]
  = mim;` followed later by a read of the same address makes cse relate them,
  gcc parks `%hi`/`%lo` in a callee-saved register and the frame grows (7 rows
  in `FieldLoadMimToVram`). Spelling the read `*(u_long**)((u8*)D_800E4D94 - 4)`
  — the same address, reached through the *next* object — leaves both as `$at`
  expansions. The relocation then names `D_800E4D94` with a −4 addend where the
  `.s` names `D_800E4D90`; the linked bytes are identical, and `checkfn.py`
  resolves negative addends the same way it already resolved positive ones.
* **`extern u8 D_800E4DB2[];` for an address inside another object links only
  while nothing references it, and `checkfn.py` will not warn you.** The
  interior label is in the symbol *config*, so checkfn resolves it and discounts
  `%lo(D_800E4DB2)` against the target's `%lo(D_800E4DB0+0x2)` as an alias —
  MATCH, with no hint that the symbol has no definition. The linker is the first
  thing to notice: `undefined reference to 'D_800E4DB2'`. Write the interior
  field as an offset from the object that owns it (`*(u16*)((u8*)D_800E4DB0 +
  2)`) and delete the extern. This is a third way a clean-looking verdict lies,
  and it only shows up in `make build`.
* **Of two pointers walked together, the one incremented *last* keeps the
  incoming argument register.** `for (i = 0; i < 12; i++, a += 12, b += 12)`
  and the same loop with `a` and `b` swapped emit the two `addiu`s in source
  order either way — but the allocator gives the parameter's own hard register
  to whichever is written second, and the other one gets a fresh register plus
  a `move` in the preheader to initialise it. Every use of both follows, so it
  is worth a dozen rows of pure renaming and nothing else moves it: both
  increments in the `for`, either one moved to the end of the body, and both
  moved there all reduce to the same two outcomes. `PreloadNextFieldMap` in
  `src/field/field.c` went 28 rows to 13 on this one line. Note this is a
  *different* knob from the giv-increment order bullet below — these are two
  bivs, and the emitted order and the register assignment are controlled by the
  same word, which is why a target that wants one bumped first *and* holding
  the argument register cannot be reached with two plain increments.
* **A value conditionally adjusted is written to its destination first and
  read back, not built in a temporary.** The array-element form of the
  store-and-read-back idiom above, and it is worth more than it looks:
  `quality[i] = (u8)(a - b); if (quality[i] >= 0x81) quality[i] = 0x100 -
  quality[i];` gives two stores — the unconditional one sinks into the branch
  delay slot and is overwritten on the other path — where an `if`/`else` over a
  temporary gives one store, a duplicated `andi`, and a `sltiu` instead of
  `slti` (the element is a signed `s16`, a masked temporary is a promoted
  `u8`). On `FieldEntityCheckTalk` it also reversed the order of the loop's two
  givs, so the two increments came out swapped as well: 15 rows to 3 on one
  statement. When a diff shows the target storing to the same address twice,
  stop looking for a scheduling explanation.
* **A dead conditional is a register-allocation lever, and it leaves no
  instructions behind.** When a diff is a permutation of the caller-saved set
  and nothing else, the fix is to change one value's *rank* — gcc assigns
  `$t0..$t9` in allocno-priority order, so the register a value gets is its
  rank — and `allocno_compare`'s only reachable term is `n_refs`, weighted by
  loop depth in flow. An extra reference at depth 0 therefore reorders the
  whole set, and it does not have to survive to the object:

  ```c
  if (sprite || ((P_TAG*)&buf->ot[D_8009ACA2.layer4])->addr) {
      run++;
      goto layer3;
  }
  run++;
  goto layer3;
  ```

  Both paths do the same thing and both operands are pure, so jump_optimize
  and flow delete all of it — the insertion count goes *down*, from 1 to 0 —
  and `AddBackgroundToRender` in `src/field/field.c` drops 56 rows. Every part
  of the condition matters and none of it is guessable: `sprite ||` on its own
  is 112 rows, the `->addr` term on its own 161, `count` in place of `sprite`
  154, dropping the cast so the load is a plain `u_long` 151/2, and the plain
  `do { } while (0); ` barrier that finished `FieldArrowsAddToRender` 159. The
  `else` arm decomp-permuter wrote it with is not needed; the duplicated tail
  is. Repeating the construct at the neighbouring walk's exit costs rows again.

  The companion form is cheaper to write and does survive: assigning a
  subexpression to an *existing* local inside a condition,
  `((otSlot = buf->BgAnim[...].mask) & mask)`, which adds references to that
  local rather than to a new one — a fresh local in its place gives back none
  of the 34 rows it is worth. Both of these are `perm_ins_block` and
  `perm_temp_for_expr` finds; neither is reachable by reasoning about the
  target, because neither changes what the code does. When the residue is pure
  renaming, raise those two weights and let the search run.

  The plain `do { } while (0);` form of the same pass pays on *scheduling*
  residue too, and there the placement is equally arbitrary: one after the
  layer-4 run walk's inner `do`/`while` in `FieldBackgroundInitPackets` -- inside
  the zero-trip `if`, after the loop closes -- is worth eight rows, while the
  same barrier after layer 2's inner loop is worth nothing, after layer 3's is
  inert on top of it, and doubling the layer-4 one gives the whole eight rows
  back. Three more in `AddBackgroundToRender`, each at one specific `addPrim`
  and worth 9 to 56 rows, with the same behaviour at every neighbouring site.
  Four of the six levers that took those two functions from 193/1 and 55/14 to
  65/0 and 43/4 are inserted blocks, and not one of them is predictable from
  the target -- the pass to raise is `perm_ins_block`, and the only way to
  place one is to search.
* **m2c's offsets are in *bytes*, so every `*(SYM + off)` it writes is scaled
  a second time by whatever `SYM` is declared as.** The seed reads the byte
  offset straight out of the `addu`, then renders the access as pointer
  arithmetic on an `extern` that the project has already typed -- so
  `*(g_FieldDebugPageY + page * 0x17A)` against an `s16[]` addresses
  `page * 0x2F4`, and against an `s32[]` it is four times out. Nothing
  complains: the C is valid, the function compiles, and the diff reads as a
  wall of register noise because every address in it is wrong. Two functions
  in `src/field/` were carrying this at 53 and 9 sites respectively, and in
  both cases fixing it was the single largest step.
  The fix is the byte-offset form this file already prescribes for the `$at`
  wall -- `*(s16*)((u8*)SYM + off)` -- which is also what the target has
  whenever it addresses interior labels: `FieldDebugRenderPage` spends **389
  of its 1341 instructions** inside `$at` macro expansions and
  `FieldBGUpdateDrawenv` 181 of 1266. Read the load width off the target per
  symbol rather than trusting the `extern` (`lw` at one offset and `lhu` at
  the next is normal), and check this before anything else on a fresh seed:
  `grep -c '\$at'` against the function's instruction count tells you in one
  command whether the original addresses this way at all.
  `tools/asm_widths.py <target.s>` prints the width per symbol, which is the
  other half you need and is not in the header.
* **The index temporaries m2c emits under different names are usually one
  variable, and whether it is computed once is worth tens of instructions in
  either direction.** `temp_t7 = page * 0x17A;` repeated at five block
  boundaries is one `off`. Merging the names is free; deciding how often to
  *assign* it is the lever, and the two extremes are both bad --
  `FieldDebugRenderPage` measures **+27** with the assignment repeated per
  block, **-77** with the expression written inline at all 93 uses (cse then
  folds it to far fewer than the target's six expansions), and lands closest
  computing it once. Sweep the three; do not assume.

* **Do not transcribe m2c's `if (c != 0) { do { … } while (i < c); }` -- that
  *is* a `for`, and writing both costs two instructions per loop.** gcc
  expands `for (i = 0; i < c; i++)` as an init, a zero-trip test and a
  do-while, which is exactly what m2c reconstructs from the CFG; add your own
  `if (c != 0)` around it and gcc does not fold the two tests, because the
  bound is a `u8` field whose load it will not prove redundant. It reads like
  a faithful transcription and it is a duplicate. `FieldModelCreatePktsForPart`
  in `src/field/field2.c` has eight such loops and the redundant guards were
  worth **16 instructions** -- two thirds of everything that function was over
  length. The same shape shows up wherever m2c prints a guarded do-while,
  which is every counted loop in the codebase.
* **A comparison used as a *value* needs its own statement, or gcc branches
  around the arithmetic instead of computing it.** `x + ((c != K) ? y : 0)`
  and `x + (y & -(c != K))` are the same tree after fold, and both reach
  `expand_expr` underneath the `+`, where gcc 2.6.3 emits a conditional jump
  around the addition -- five instructions and two extra blocks. Assign the
  comparison to a local first and it is a statement of its own, `do_store_flag`
  runs, and you get the target's branchless `andi` / `xori` / `sltu` / `negu` /
  `and`. So when m2c prints `(v & -((x & M) != K))` -- its rendering of exactly
  that sequence -- the source has a named boolean, not a clever mask. Four
  sites in `FieldModelCreatePktsForPart`.
* **Declaration order is inert for *registers* and decisive for *stack
  slots*.** The standing rule (allocation is by priority, not position) is
  about hard registers; reload's spill slots are handed out in pseudo order,
  and `expand_decl` creates one pseudo per local at the top of the function in
  declaration order. So a function whose spilled locals sit at the wrong
  offsets is telling you its declarations are in the wrong order, and the fix
  is free: read the target's slots, and declare the locals in that order.
  `FieldModelCreatePktsForPart` has six spilled locals at `0x28`…`0x50` and
  all six land at once. In the same function, moving those declarations for
  any *other* reason -- a `u8` to the end of the list, four pointers to the
  front -- is exactly inert, which is the register half of the rule holding.
* **Inverting an `if` and swapping its arms is not a no-op, even when the CFG
  comes out identical.** The blocks are emitted in source order, so which arm
  is the fall-through decides two things that look like they belong to other
  passes: which insns reorg can steal into the two branch delay slots, and
  therefore how many times a subexpression common to both arms is
  rematerialised; and whether combine gets to fold a shift-and-shift in the
  arm that is now laid out second. `FieldModelCreatePktsForPart`'s graph-type
  test measures 727 instructions written `!= 1 && != 2` with the arms swapped
  and 731 written `== 1 || == 2` -- one instruction per copy of the loop, from
  `t & 0xC0` being computed three times rather than twice and
  `((t >> 4) & 0x100) >> 4` folding to two instructions rather than staying at
  three. Neither is reachable by attacking it directly: every spelling of that
  shift measures identically, and hoisting the mask into a local is +8 because
  it then spans two calls and spills. A `goto` chain that reproduces the
  target's branch polarity and block order *literally* is also 731 -- so read
  the length, not the CFG.

* **A `%hi` table only means anything once addends and aliases are resolved,
  and getting that wrong invents faults.** MIPS uses REL relocations, so
  objdump prints the symbol name and nothing else -- the addend is split across
  the `lui`/`lo` immediate pair. Read naively, every `g_FieldEntity[i].member`
  looks like a reference to the base symbol rather than to
  `g_FieldEntity + 0x32`, which is the same byte the target calls
  `D_80074ED6`. `tools/insn_histogram.py` reassembles the addend and folds
  names that share an address (and folds objdump's `c2` back to the GTE
  mnemonics), because `checkfn.py` discounts exactly those differences. Before
  it did, it reported `243 against 4` on a function whose addressing was
  already correct, and a whole conversion was written and landed on that
  reading before the length and row counts contradicted it.

  **That paragraph was true as a specification and false as a description for
  as long as it existed, and the same wrong reading happened again on the
  strength of it.** The tool counted per symbol *name*: it discarded the
  addend on both sides (`split("+")[0]` against objdump, a bare-name regex
  against the `.s`) and never folded a renamed global against the `D_` name a
  frozen `.s` still uses. So `AddBackgroundToRender` reported eight
  materialisations of one base against six distinct neighbours -- which reads
  exactly like the original declaring six scalars where the C has an array --
  and every handler in `src/field/field4.c` reported twelve complementary
  rows. All phantom. Both were diagnosed, written up and dispatched before a
  direct look at the object showed the addends were zero and
  `g_EntityToModel` was `D_8007EB98`. It is fixed and validated now (both
  tables go to `(identical)` on a function `checkfn` calls pure register
  naming, while `FieldDebugRenderPage` keeps its real spread), but the
  transferable rule is the one that would have caught it in either decade:
  **a note saying a tool resolves something is not evidence that it does.**
  When a table hands you a structural diagnosis, confirm it against
  `objdump -drz` on the object before spending anything on it -- one grep of
  the relocations would have cost a minute and saved both.

  What the corrected tool then says about `AddBackgroundToRender` is worth
  recording on its own, because it is the rarer verdict: opcodes identical
  *and* addresses identical at the exact length, with 65 rows still
  differing. That is proof the residue is nothing but register naming, which
  **confirms** the park note's `allocno_compare` arithmetic rather than
  refuting it. A body with two clean histogram tables and rows remaining is a
  park, not a search, and specifically not a permuter target.

  **The *opcode* column had the same bug one table over, and it manufactured a
  second whole hypothesis before anyone checked it.** objdump prints an alias
  where the `.s` spells the real mnemonic, and the alias is ambiguous: `li` is
  `ori rD,$zero,K` for a constant that fits unsigned and `addiu rD,$zero,-K`
  for one that does not, and `move` is `addu rD,rS,$zero` or `or rD,rS,$zero`
  (splat emits 264 of the first and 32 of the second in the field overlay
  alone). Folding by *name* credited every negative `li` to `ori` and invented
  an `addiu` deficit of exactly the same size. `FieldMain` has one
  `addiu $a0,$zero,-0x1`, so it reported `ori +2 / nop -1 / addiu -1` -- three
  faults summing to zero, which is precisely the "an exact length can be errors
  cancelling" shape, and it was read that way and assigned as such. Decoded
  from the instruction word it is `ori +1 / nop -1`, which is *one* cluster the
  park note already names. Fixed in 7e68f5f; the `.s` side needs no folding at
  all.

  Two sessions hit this independently and in opposite units, which is how much
  it costs: `FieldModelStructInit` read the same way at the exact length with
  *one* row differing — precisely the shape the table exists to adjudicate —
  and the invented column said "findable spelling" where the truth is
  "register naming, park it". The fix decodes the primary-opcode and funct
  fields out of the 32-bit word, which cannot drift, and the same trap is
  latent in every name-keyed fold (`move` is `addu` *or* `or`, `b` is `beq`).
  Stated generally, because it is the same bug as counting `%hi` per name one
  table over: **a tool that normalises two sides of a comparison must
  normalise on something the *assembler* decided, not on something the
  *disassembler* chose to print.**

  **The check that catches both without knowing either: add the columns up.**
  A naming or folding artifact moves counts between rows and cannot change the
  total, so its rows are complementary and balance. `AddBackgroundToRender`
  read `D_80071A48 8/2` against five neighbours at `0/1` and one at `0/2` --
  8 against 8 -- and `D_8009ACA2 2/1` against `D_8009ACA4 0/1` -- 2 against 2.
  Two balanced groups, no fault. A real addressing error does not balance:
  `FieldDebugRenderPage` is `+3 +2 +2 +1 +1 +1` against `-1 -1 -1`. This costs
  one glance and it is the only structural check available before the tool
  itself is trusted.

  **But that check belongs to the `%hi` table only, and applying it to the
  opcode table inverts the answer.** The two tables are keyed differently and
  balance means opposite things in them. `%hi` rows are keyed by *address*, so
  there is no such thing as a legitimate substitution — a balanced pair can
  only be one address split across two keys, i.e. an artifact. Opcode rows are
  keyed by mnemonic, where a length-preserving substitution is the commonest
  *real* fault there is: the same program compiled with one local declared
  `s32` instead of `u16` emits `sw` where the target emits `sh`, and the
  columns balance exactly because the instruction count did not change.
  `FieldBackgroundInitPackets` is the worked case in both directions — its
  `lw +3 / sw +3 / sh -3 / lh -2 / lhu -1 / addu -2 / nop +2` sums to zero,
  survived both tool fixes unchanged, and every row of it was real. That is
  the same fact as the exact-length-by-cancellation rule above, read off the
  opcode table instead of the length: **a balanced opcode table is what a
  wrong declaration looks like, and a balanced address table is what a wrong
  tool looks like.**

  **And the two cancelling errors can be large.** `FieldDebugRenderPage` sat
  at the exact 1341 carrying **15 instructions** of double-scaled pointer
  arithmetic against **15 instructions** of missing re-expansion — an error
  either side of the truth, netting to a length that read as settled for
  several sessions. Decomposing them *cost* the exact length (1337, −4) and
  bought **149 rows**. So on a body hundreds of rows out at exactly the target
  length, the length is the least trustworthy number you have: look for the
  pair before treating it as a constraint to preserve.

  One row class in that table is naming and will stay: a string literal or a
  jump table is a local `.rodata` label in our object and a named symbol in the
  `.s`, so any function with either reports `.rodata` against `D_800A0000` /
  `jtbl_800A0008`. `checkfn.py` resolves it by calibrating the section base
  from paired diff rows, which needs an alignment this tool deliberately does
  not have. It is **not** the parked-`.rodata`-blob artifact -- on `FieldMain`
  the rows are identical with `const u32 D_800A0000[]` deleted.

* **On a function hundreds of rows out, count instructions before reading
  any.** `tools/insn_histogram.py <src> <func>` prints the compiled length and
  two tables: opcodes (objdump's aliases folded back to the `.s` spelling, or
  `move` against `addu ..,$zero` drowns the comparison) and `%hi`
  materialisations per symbol. Both are alignment-free, so unlike a row count
  they stay meaningful at any distance, and they compress a wall of diff into
  one sentence. `FieldDebugRenderPage` at +1 instruction reads
  `nop +14, lui +8` against `addu -12, sll -7, sra -3` — the target re-expands
  an index and gets its load-delay slots filled for it. `FieldBGUpdateDrawenv`
  reads `lui +30 / addiu -44`, which is the single fact that it rebuilds
  addresses where the target keeps them, and the per-symbol table then names
  the two globals responsible (27 materialisations against 9).

  The per-symbol table is also the honest check on a length fix. Reading one
  of those two globals through a pointer local takes that function to exactly
  the target's 1266 instructions — and collapses the symbol to **1**
  materialisation against 9, hitting the length only because a +18 error
  cancels a -8 one. A length that is right for the wrong reason is the
  `FieldBackgroundInitPackets` trap in advance; check the table before landing
  one.

* **An alias fold is only safe for aliases splat does *not* itself write, and
  the histogram's fold was not.** The opcode table decodes objdump's aliases
  back to the mnemonic the `.s` spells, which is right for `li`, `move` and
  `not` — splat never writes those — and was wrong for `negu` and `b`, which
  splat writes 261 and 13 times across `asm/us/` respectively (and it writes
  `subu $rD, $zero, $rT` **never**). Folding those renamed *our* side alone,
  so the table reported a balanced `subu +N / negu -N` pair that is pure
  artifact: `FieldModelCreatePktsForPart` read `+4/-4` with the `negu` at all
  four sites already byte-identical in the diff, and `FieldEntityMove`
  `+6/-6` — in both, the real cluster was elsewhere and this pair was the
  loudest row in the table. Fixed in the tool. The general shape is worth
  keeping: a *balanced* pair in the opcode table is either a wrong
  declaration or a naming asymmetry, and the cheap way to tell them apart is
  to grep the target `.s` for both mnemonics — if it spells one of them, the
  tool must not rewrite it.

* **A `%hi` entry at an address the target never materialises names a wrong
  address, and no row count can see it.** The per-address table is the only
  thing that catches a pointer difference that is scaled twice — the
  companion to the m2c byte-offset rule above, reached from the other end.
  `FieldBGUpdateDrawenv` passed ten `DRAWENV`s to `SetDrawEnv` as
  `(DRAWENV*)(&D_80113F34 - 8)`, and `D_80113F34` is an `extern s16`, so the
  pointer landed **0x10** below the symbol where the target's twenty-four
  `addiu $a1, $reg, -0x8` say 8. Correcting it measures *exactly* the same
  811 rows / 134 insertions as the wrong version, because either spelling
  folds the constant into the `%lo` and either row is a CHG against a target
  that holds the symbol's own address in a register. The table said it in one
  line: four `%hi` at `0x80113F24` against the target's zero.

  The fix for that whole family is a **per-site pointer local with the store
  routed through it** — `envN = &SYM; *envN = ...; f(x, (T*)((u8*)envN - 8));`
  — which reproduces `lui`/`addiu` of the symbol, `sh $v0,0($reg)` and
  `addiu $a1,$reg,-0x8` register-for-register, and took that function from
  **811 rows to 632**. Two halves are separately load-bearing and neither is
  guessable from the diff: the store must go through the pointer, since a
  pointer local that is never dereferenced is folded straight back into the
  address by cse and measures *exactly* inert; and it must be one local **per
  site**, since a single shared one becomes an allocno with 48 references,
  wins a callee-saved register, lives across the whole function and is 85
  rows worse than doing nothing.

* **Reload aligns a spill slot to `BIGGEST_ALIGNMENT`, which is 64 bits on
  MIPS, so spilled `short`s sit 8 bytes apart and do not look like `short`s.**
  Three halfword slots at `0x20`, `0x28`, `0x30` read like three 8-byte
  aggregates and are three spilled `u16` locals. Read them as evidence of
  *register pressure*, not of a missing struct: in `FieldBGUpdateDrawenv` the
  target spills exactly three halfwords because two of its callee-saved
  registers are spent holding two globals' addresses, which this project's
  body does not do. Fixing the addressing is what produces the spills, and
  the `TREE_ADDRESSABLE` lever is the wrong tool for them.

* **A symbol the target materialises *nine* times is a count of cse regions,
  and the pointer-local idiom applies per region, not per function.** The
  standing advice — a scalar global read many times wants a named pointer
  local — is right about the form and silent about the scope, and the scope
  is the whole question when the target's count is neither 1 nor N.
  `FieldBGUpdateDrawenv` reads each screen-centre global 31 times; the target
  materialises each address 9 times, which decomposes as **three**
  register-held regions (`lui`+`addiu` into a callee-saved register, then
  `lhu $a2,0($s3)`) plus six isolated `$at`-form reads in the tail. Those
  three regions are readable straight off the target — one set right after a
  call and covering both arms of the `if` below it, one inside each arm of a
  later `if` — so the fix is one pointer pair per region with the tail reads
  left direct. A single pair for the whole function gives 1 against 9 and
  overshoots the length by exactly what the six isolated reads cost (`pcX`
  alone +2, `pcY` alone +6, both **-18**).

* **`variant_eval.py` prints the compiled length too, and that is the number
  to read.** It reports `length <ours> against <target> (+N instructions)`
  whenever the two differ, taken from the function's ELF symbol size rather
  than from the diff, so it cannot be confused by alignment. Reach for it
  first: `FieldDebugRenderPage` reports **127 insertions** and is **twelve**
  instructions long, and three separate times this project has read an
  insertion count as a length and drawn the wrong conclusion from it.


  It compares against the **.text-only** count, not the whole `.s`. For a
  function with a jump table the `.s` also holds the table's `.word` entries
  -- same `/* offset addr bytes */` prefix, sitting in the file's `.rodata`
  ahead of the body -- and counting those as instructions makes the function
  read as short by exactly the number of cases. `OpcodeFuncFadew` reported
  **-11 instructions** for three sessions on that alone; its body is 73
  against the target's 73 and the 11 were `jtbl_800A0DF4`. The whole-`.s`
  count is still what scopes the diff, which does have to cover those rows,
  so the two numbers are both needed and are not interchangeable. Re-running
  the field triage after the fix moved four verdicts: `FieldDebugRenderString`
  from -33 to -1, `DebugUpdateActor` from -5 to +2, `FieldMain` from -6 to
  +1, and `OpcodeFuncFadew` to exact. A length that reads badly wrong on a
  function with a `switch` is worth checking against the `.s` before
  believing it.
* **`checkfn`'s "inserted" is not a length measurement — read the `+N
  instructions` figure instead.** An instruction that merely *moved* is
  reported as one insertion and one deletion, so the counts scale with how
  badly the diff aligned rather than with how wrong the body is.
  `FieldMain` reports six insertions at exactly the target's 786 instructions;
  `AddBackgroundToRender` reports none at exactly 658; `FieldBackgroundInitPackets`
  reports eleven and is genuinely six instructions long. `checkfn.py` now
  prints the compiled length whenever it differs, and that is the number to
  work on first: length is a hard invariant, so a body of the wrong size
  cannot match however few rows differ, and a "better" row count bought by
  adding an instruction is a step backwards.

  It cuts the other way too, and that is the more expensive mistake because
  it looks like diligence. `FieldEntityWalkmechCross`' park note recorded
  "99 changed / **+4 instructions**" -- the 4 was the insertion count, and the
  body was **11 instructions short**. Every lever that would fix it therefore
  *adds* instructions and reads as a regression by rows: the note had measured
  the one that mattered (a flat integer address sum, 148 rows against 141) and
  rejected it on exactly that evidence. Ranking by length instead took the
  same function from 99 rows to 34 at the exact 291. When a note quotes two
  numbers, check which one is the length before believing either.

  The two metrics disagree often enough to reverse a day's work. Three levers
  on `FieldBackgroundInitPackets` — a re-read loop guard, a moved counter and
  two `do { } while (0);` barriers — took it from 43 rows to 26 and were
  landed on that evidence; each of the three costs exactly two instructions,
  so the "better" body is 401 against the target's 395, and the seventeen rows
  are mostly the six: three `nop`s per barrier, the `--count` chain displaced
  behind them, and eight branch offsets that shift because everything after
  the first barrier moved by 0xc. All three were withdrawn. **A row count is
  only comparable between two bodies of the same length** — across lengths it
  measures how well the diff aligned, which is a property of where the
  insertion happened rather than of how wrong the body is. Fit the length
  first; treat a row count that improves while the length grows as evidence
  against the change.

* **But an exact length can be several errors cancelling, and only the opcode
  histogram can tell you which kind you have.** The rule above is right about
  *rows* and says nothing about the body underneath them: a length is one
  number, so a body with `sw +3 / sh -3 / lw +3 / addu -2 / lh -2 / lhu -1 /
  nop +2` sums to zero and reports "length exact" while seven separate things
  are wrong. `FieldBackgroundInitPackets` sat at exactly 395 that way for
  three sessions, and every attempt to improve it from there was fighting a
  frame pad and a wrong declared width that were holding the cancellation in
  place. The body that fixes them measures **+2 instructions and five rows
  worse** — and its histogram is `nop +2` and nothing else, with every `%hi`
  count per symbol identical. That is a strictly better position: one honest
  error rather than seven that happen to sum to none.

  So run `insn_histogram.py` before accepting *or* rejecting a length verdict.
  An exact length with a clean histogram is finished work; an exact length
  with opposing columns is a coincidence, and the way out of it necessarily
  passes through a body that reads as a regression by both rows and length.
  Say so in the park note, and say the function must not be unparked at the
  wrong length — the build stays green because the body is still pinned.

* **An *empty* `do { } while (0);` is a free test for which pass you are
  fighting** — and the word *empty* is load-bearing, see the reference-
  multiplier bullet below, which is the same construct with a body and is not
  free at all. Empty, it
  emits nothing, so it cannot change an allocno's reference count or live
  range; all it does is end a basic block, which is a scheduling boundary. So
  drop one at the end of a loop body and re-measure: if the row count moves at
  all, the residue is sched2's and source position is a live lever; if it is
  *exactly* inert, the residue is register allocation and every barrier,
  reordering and re-spelling you are about to try is inert too. Measured on
  two functions in `src/field/field.c` that both read as noise:
  `FieldBackgroundInitPackets` moves 34 rows to 26 on one barrier, and
  `AddBackgroundToRender` is 65 either way — at the end of any one of its four
  inner loop bodies or all four at once. That second result is worth as much
  as the first: it is what turns "65 rows of register naming" from a guess
  into a finding, and it costs one sweep.

  **It is also a cse barrier, not only a scheduling one, and that is a second
  use worth knowing.** A named address local is folded back into the subscript
  below it unless a basic-block boundary intervenes; the empty loop supplies
  one. Worth 4 rows in `FieldEventRunInit`, and it is what lets the local sit
  where the preheader hoist order needs it rather than where cse will tolerate
  it. So an "exactly inert" result closes the *scheduling* dimension only —
  re-test with the barrier placed between a definition and its use before
  concluding a local cannot be moved.

  **And it is a register-allocation probe, which is its most useful third
  use: it can tell you a residue is reachable before you know how.** Moving
  the one barrier `FieldEventRunInit` already carried from after `pcBase` to
  after `scriptBase` took it from 12 rows to **10** and reproduced the
  target's register assignment exactly -- the two competing quantities
  swapped, nine of the twelve rows went, and what was left was two `nop`s the
  split block could no longer fill. That is +2 instructions, so it is not a
  fix and must not be landed; what it *is* is proof that the assignment is
  reachable at all, plus a picture of which two quantities are competing,
  which is what turns "12 rows of register naming" into a question with an
  answer. Eleven placements were measured there and only one produced it, so
  sweep the boundary rather than picking one. The mechanism is that the
  boundary changes which quantities `local_alloc` sees as block-local, and
  therefore the order registers are handed out in — the same lever the
  `size` term reaches for free.

  **And it is not free at all once it has a body: the loop notes are a
  reference multiplier.** `expand_start_loop` emits `NOTE_INSN_LOOP_BEG` and
  `NOTE_INSN_LOOP_END` around whatever is inside, and `flow.c` counts
  `REG_N_REFS (regno) += loop_depth` — so every reference inside a
  `do { … } while (0);` is weighted one level deeper, and nesting two of them
  weights it two deeper, at no cost in instructions. **This is the only
  construct this project has found that adds references to a pseudo without
  emitting anything**, and it delivers exactly what an `allocno_compare`
  specification asks for. `FieldModelStructInit` in `src/field/field2.c` needs
  its counter `i` at 16 references to out-rank the data pointer at 15, and
  every candidate this file lists — a dead store, a reg-reg copy, `i = i + 0`,
  a duplicated tail — is either deleted before `flow` counts it or survives to
  the object. `do { do { i = 0; } while (0); } while (0);` around the first
  reset puts that one statement at loop_depth 3, takes `i` from 14 to 16 (read
  off `-dl`, not inferred) and makes **every register in the function
  correct**. The empty form measures inert precisely because it has no
  references inside it; do not read "emits nothing" as "changes nothing".

  The cost is the notes themselves. gcc's scheduler makes the insn after a
  loop note depend on everything before it, so the basic block they sit in is
  cut in two and the halves are then emitted in source order. Twenty-odd
  placements were measured on that function: the cheapest is 2 rows at the
  exact length, and the residue is always the pair of instructions the
  target's undivided block swaps and the cut cannot. So the weight is free
  where the target already has a block boundary at that point and costs at
  least a row where it does not — which makes "where does the target end a
  block?" the question to ask before choosing which reference to weight.

* **A diff of 0 insertions and 0 deletions refutes an `allocno_compare` or
  `QTY_CMP_PRI` diagnosis by construction, and that is a proof rather than an
  argument.** Same instruction stream means the same RTL, which means the same
  `n_refs` and `live_length`, which means the same ranking — so a ranking that
  is provably *equal* on both sides cannot be the thing that differs. What
  differs in that case is the **pseudo structure**: how many quantities the
  block is cut into, which is decided by how many variables the source spells
  and is therefore reachable from C. `OpcodeFuncMove` and
  `FieldMoveToEntityUpdate` in `src/field/field4.c` were parked three sessions
  on the opposite reading — both notes ended in the allocno arithmetic and
  concluded no term was reachable — and both matched on naming one
  intermediate. Read the insertion/deletion counts before believing any
  allocation diagnosis; this file's standing advice to park on allocno
  arithmetic applies to a residue with insertions in it, not to a pure
  permutation.

  **Applying it looks like asking "how many variables?" at every quantity the
  diff names, and the answer can be a *merge* as easily as a split.**
  `AddBackgroundToRender` in `src/field/field.c` is the purest case this repo
  has — 0 insertions, 0 deletions, the exact 658 instructions, and both
  `insn_histogram.py` tables identical — and its note had computed the allocno
  scores and declared it a park. The lever was that layer 2's entity-mask test
  was spelled `mask & trigger[entity]` while layers 3 and 4 spell it
  `trigger[entity] & (otSlot = mask)`: the same quantity, written as an
  assignment to the shared local in two of the three places it occurs. Making
  the third match is **72 rows → 64** and closes the whole layer-2 entity
  block. Splitting the mask reads off into their own `maskSlot` instead is 67,
  so the direction is merge-into-one, not split. Sweep every repeated
  construct in the function for the one that is spelled differently from its
  siblings before concluding anything from the allocno numbers.

* **`find_reg` really is lowest-free, and the counter-example that looks like a
  cost model is a conflict list you have not read.** The escape hatch a park
  note reaches for once the allocno arithmetic says "unreachable" is that
  gcc's `find_reg` might be picking by cost rather than by number, so the two
  programs could differ in conflicts rather than in ranking. It is not: in
  `AddBackgroundToRender`'s own `.greg`, pseudo 105 is allocated *seven places
  after* `run` and still gets the lower `$t2`, which reads as a cost model
  until you check `;; 105 conflicts:` — it conflicts with the allocnos holding
  `$a1`, `$a2`, `$t0` and `$t1` and with nothing in `$t2`, so `$t2` is its
  lowest free register. Check the conflict line before inventing a mechanism;
  the dump prints one per allocno, right under the allocation order.

  With that closed, the arithmetic becomes a **specification for the search**
  rather than a verdict against it. `allocno_compare` ranks by
  `floor_log2(n_refs) * n_refs / live_length`, so a diff whose whole residue is
  two quantities trading registers states exactly what a candidate structure
  must deliver: `AddBackgroundToRender` needs `run` at **≤ 16** references (it
  has 47 over 538 insns) *or* the layer-1 `0x124DC` constant at **≥ 12** (it
  has 5 over 80). Write that number into the note — it is what tells the next
  pass whether a proposed structure is even in range, and it is one `-dl` dump
  away.

* **A register the target never writes, read in your build, is a bug and not a
  residue — and an unreachable assignment is how you get one.**
  `AddBackgroundToRender` carried a `layer3Slot = &D_8009ACA2.layer3;` written
  on the line above the `layer3:` label, and the walk above it is a `for (;;)`
  left only by `goto layer3` — so the assignment was unreachable,
  `jump_optimize` deleted the block, the pseudo kept its use and lost its def,
  and `global_alloc` handed the undefined value `$s3`. It compiled, it scored
  **seven rows better** than the correct program, and it had been recorded as a
  lever and re-measured against for three sessions; every number taken on that
  body is worth nothing. The check costs one command — disassemble the object
  and look for a register read with no write above it:

  ```shell
  sh tools/variant_disasm.sh .variants/<spec>.json /tmp/f.dis
  .venv/bin/python3 tools/uninit_regs.py /tmp/f.dis AddBackgroundToRender
  #   READ-BEFORE-WRITE  $s3  in   lhu v1,0(s3)
  ```

  A read with no write above it is the answer. Reach for it whenever the diff
  shows your build using a *callee-saved* register for a value the target keeps
  in a temp: that is exactly what an allocno with no definition looks like,
  because its live range runs back to the function entry. A label after an
  infinite loop is the shape to distrust — everything between the loop's close
  and the label is dead code that the front end accepts silently.

* **You cannot move references off a walking pointer with a per-loop cursor —
  cse propagates it straight back and the count goes *up*.** The obvious way to
  drop a long-lived pointer's `REG_N_REFS` (the term `allocno_compare` ranks
  on) is to read the loop body through a copy: `r = run;` at the top, `r[0]`,
  `r[1]`, `r[2]` in the body, and only `run = r + 3` at the bottom. It does not
  work. cse substitutes `run` back into every subscript, so `r` ends up with a
  handful of references and the two assignments are pure *additions* to `run`'s
  own count: measured on `AddBackgroundToRender`'s first walk, `run` went from
  47 references to **55** and the body from 72 rows to 135. A
  `do { } while (0);` after the copy to give cse a basic-block boundary — the
  barrier that works for a named *address* local — is exactly inert here, 135
  again. So `REG_N_REFS` on a pointer is a property of the walk, not of the
  spelling, and a residue that needs it lowered needs a different program
  rather than a different phrasing.
  The caveat, from `FieldModelStructInit` in `src/field/field2.c`: a row whose
  two sides name *different registers for the same address* is not a pure
  permutation, because which pseudo an operand names decides that pseudo's
  `n_refs`. That function is 1 changed / 0 inserted and the fix genuinely is
  the ranking — reading the third `d->modelCount` through `d` rather than
  through `data` is what takes `d` from 13 references to 15 and puts it ahead
  of the loop counter. The rule holds when the differing rows are register
  *names* on values that are already the same pseudo; it does not when the
  row is a base register, since that is a reference count in disguise.

* **A statement duplicated into both arms of an `if`/`else` is free in the
  object and worth two insns of `live_length` to everything live around it.**
  Post-reload cross-jumping merges two identical tails, so the object keeps
  one copy — but `flow` ran long before that and counted both, and every
  pseudo live across the surrounding loop gets the extra insns in its
  `reg_live_length`. That is the only lever this project has found for the
  case where a residue needs a *longer* live range: a dead assignment is
  deleted by flow before it counts (measured twice), a reg-reg copy is folded
  by cse, and an `andi` that combine cannot reach because its consumer is in
  another basic block stays in the object. `HandleKawaiDataInModel` in
  `src/field/field2.c` matched on exactly this — `g_FieldEntity[i].KawaiA =
  blink;` written at the end of both arms rather than once after them, which
  takes `blinkClosed` from 5 refs over 84 insns (`2*5/84 = 1190`) to 5 over 86
  (1162) and `faceSel` from 19/644 (1180) to 19/646 (1176), so the two
  exchange `$s5` and `$s6` and the function lands. The target has the single
  merged store at the join, reached by a `j` from the first arm, which is what
  a duplicated-then-merged tail looks like from the `.s` side.

  It is not universal, and the failure is silent: the same trick on
  `FieldModelStructInit`'s `i += 1` leaves **both** copies in the object
  (+2 instructions, 15 rows). A one-insn tail whose arms differ in how they
  reach the join is not always merged, so check the length, never the rows.

* **Read the reset of a loop counter as a live-range lever, and put it inside
  the *guard*, not merely below the block.** A second `i = 0` between two
  loops sits in the same basic block as everything else between them, and
  `reg_live_length` for the counter then covers that whole block however far
  down the assignment is written — `FieldModelStructInit` measures **99
  insns** with the reset at the top of the middle block and 99 with it at the
  bottom, byte-identical. Written inside the second loop's guard,
  `if (count != 0) { i = 0; do { … } }`, the def moves into the loop's own
  preheader, the middle block leaves the counter's live range altogether, and
  it drops to **86** — enough to reverse `allocno_compare` against a pointer
  at 15 refs / 97 insns. The cost is that the `move a3,zero` is then emitted
  in the preheader rather than in the first loop's exit block, so the two are
  mutually exclusive; read the target for which one it wants before spending
  anything. The general form: *where* a def sits inside a block is inert, and
  *which* block it sits in is decisive.

* **Paired levers can both be *plateaus*, not regressions, and a note that
  measures one at a time rejects both.** The existing paired-levers rule is
  about two changes at +1 and −1 instruction that cancel. The commoner case
  moves nothing: `OpcodeFuncMove`'s dead conditional is 14 rows → 10 and the
  `s32 entryIdx` split is 14 → 14, so the second reads as exactly inert and
  every note recorded it as "measured and inert" — together they are **0**.
  `FieldEventRunInit` is the same shape (a moved local at −2 instructions, a
  barrier at +1, together 15 → 11). An inert measurement is evidence about
  that lever *in the presence of the others you happened to have*, and nothing
  more. Cross the note's inert entries with its partial wins before spending a
  budget on anything new.

* **A `perm_ins_block` dead conditional is free only when the duplicated block
  leaves nothing live at the join.** `OpcodeFuncMove`'s is deleted by flow at
  no cost in length; the same construct in `FieldEventRunInit`, wrapped around
  blocks whose locals are read afterwards, costs +5 and +6 instructions. Two
  data points and the mechanism is unread, so treat it as a rule of thumb:
  check the length, not just the rows, on every inserted block.

* **Before deciding two residues are two problems, check whether the lever for
  one is the cause of the other.** `HandleKawaiDataInModel` in
  `src/field/field2.c` sits 2 rows out on a movable *order* (the target hoists
  `li 1` before `li 2`), and the one spelling that reverses the order --
  `blinkOpen = 1;` written above `blinkClosed = 2;` at the loop top -- lands 13
  rows out on an unrelated-looking *register* swap, `faceSel` and
  `blinkClosed` trading `$s5`/`$s6`. Three sessions attacked those as
  independent, sweeping widths, declaration positions, reference counts and
  pointer placements against each. They are one insn: `move_movables` records
  in insn order, so reversing the order requires an insn in front of
  `blinkClosed`'s def, and that insn shortens `blinkClosed`'s live range by
  exactly one -- which is exactly the margin, since its priority is
  `2*5/84 = 1190` against `faceSel`'s `4*19/644 = 1180`, a 0.8% gap. Any fix
  for the order re-creates the swap by construction. The tell is a pair of
  residues where each lever's *side effect* is the other's cause; work out the
  arithmetic once and you save the whole cross-product. And note which way it
  cuts: this is the counter-case to the paired-levers rule above -- there two
  changes that each moved the length the wrong way cancelled, here two changes
  that each fix half are the same change and cannot be added.

* **Do not guess at `n_refs` and `live_length` -- cc1 prints them, and
  `variant_eval.py --rtl` prints them for the body you just scored.** The
  `.lreg` dump names every pseudo with exactly the two numbers
  `allocno_compare` ranks on, and `.greg`'s post-reload RTL shows which hard
  register each one ended up in, so a residue that reads as register naming can
  be turned into arithmetic instead of a guess:

  ```shell
  .venv/bin/python3 tools/variant_eval.py .variants/base.json --rtl
  #        rtl dumps -> .variants/rtl-base  (rtl.c.lreg)
  #   --rtl=L for the loop dump, --rtl=a for every pass
  ```

  It re-runs cc1 on the same preprocessed text with `-d<letters>` and copies
  the dumps out, which is the whole of the manual recipe below plus the two
  things that recipe gets wrong in practice -- the flags come from
  `build.ninja` rather than from memory, and the parked function is unparked
  the same way the score was taken, so the dump describes the body whose rows
  you are reading rather than the pinned `.s` beside it.

  ```shell
  mipsel-linux-gnu-cpp <the flags from `ninja -t commands`> src/field/field.c \
    | bin/str | iconv -f UTF-8 -t Shift-JIS > /tmp/f.i
  cd /tmp/d && /ff7/bin/cc1-psx-26 -quiet -mcpu=3000 -mgas -O2 -G0 \
    -dumpbase f.c -dl /tmp/f.i -o /dev/null      # -dg for .greg
  ```

  The dump has to be written **inside the container, to a container path**: a
  `-dumpbase` under the bind-mounted repository creates the files and leaves
  them 0 bytes, with no error, which reads exactly like a pass that did not
  run. `cd` into the target directory and copy the results out afterwards.
  (`--rtl` does this for you.)

  **Worked forwards, the arithmetic is a specification with one number in it,
  and the reference multiplier is how you pay it.** `func_801D1F40` in
  `src/menu/savemenu.c` sat 5 rows out with a colour loop counter in the wrong
  register; the `.lreg` dump named the two competing quantities and both
  terms -- `retries` at 7 refs over 14 insns (`2*7/14 = 1.00`) against `fd` at
  6 over 19 (`2*6/19 = 0.63`) -- so `retries` took `$s0` where the target has
  `fd`. Solving `floor_log2(n)*n/19 > 1.00` gives **n = 8** and nothing
  smaller, because 7 is still on the `floor_log2` step below. Two
  `do { close(fd); } while (0);` deliver exactly +2 references and the
  function matches; one barrier, either one, is 7 references and measures the
  same 8 rows as none. Predicting the *count* before editing is what makes
  this a lookup rather than a search -- and it is why "one more barrier"
  is a hypothesis rather than a retry.

  **`-dL` does the same job for `move_movables`, and it prints the decisions
  rather than the inputs.** The `.loop` dump gives `Loop from A to B: N real
  insns` and then, per candidate, either
  `Insn I: regno R (life L), savings S moved to M` or a `not desirable` line.
  `move_movables` keeps a movable iff `threshold * savings * life >=
  insn_count`, so **two** moved/not-moved pairs from the same loop bracket the
  threshold: on `FieldDebugRenderString` they give `11.6 <= threshold < 15.5`.
  That converts "66 rows of register noise" into a quantity — *this constant
  needs three more instructions of lifetime* — and then into a yes/no question
  about whether there is anything left to put there. Reach for it whenever a
  residue turns on which of two invariants got hoisted; it is far better than
  permuting the loop body and guessing from the outcome.

  **Do not stop at the threshold arithmetic, though — the dump has a second
  column and it is the one that matters.** Besides `moved`/`not desirable` a
  line can read `cond forces I` or `done matches I`, and those two bypass the
  formula entirely: a movable rides out of the loop when the movable it
  *forces* is already `done` (`m->forces && m->forces->done`), and a movable
  that `matches` an earlier identical one is deleted rather than judged. So an
  invariant address the target computes *inside* its loop is almost never a
  threshold miss — it is a `forces` link pointing at the wrong operand.
  `FieldDebugRenderString`'s park note had spent a session bracketing the
  threshold and concluded "neither term is reachable from C"; the answer was
  that its two `plus` insns forced the multiply chain instead of the symbol.
  `tools/loop_movables.py` prints the whole list with each insn named by its
  `SET_SRC` from the `.cse` dump, which is what makes the links readable.

  **Which operand a `plus` forces is decided by insn order, and *that* is
  reachable from C: an address expression emits its leaves first, a plain
  assignment emits in tree order.** `expand_expr` reaches an address with
  `EXPAND_SUM`, so the whole sum is built symbolically, every leaf (a
  sign-extension, a `symbol_ref`) is emitted as it is met, and `force_operand`
  emits the arithmetic afterwards. A symbol that is a leaf of the sum is
  therefore always *earlier* than the multiply chain it is added to — and
  `scan_loop` sets `m1->forces = m` for the **last** movable in list order
  whose single use is `m1`, so the multiply wins, the adds ride it out, and
  the symbol goes with them. Assign the same value in a statement of its own
  and the operands are expanded in tree order instead, so the symbol's
  movable comes last, the adds force *it*, and if it is not desirable they
  are reported `not safe` and stay:

  ```c
  colOff = page * 378;              /* still inline, so it still matches   */
  colBase = (s32)D_800E08A8;        /* now the later movable of the two    */
  v = D_800E4200[*(u8*)(colOff + colBase + rowIdx)];
  ```

  Note what each half is for. The multiply has to stay spelled out rather
  than reusing the loop's existing `off`, or it stops *matching* the loop's
  own copy, its savings halves and half the chain stops being hoisted (`off`
  measured +3 instructions). And `colBase` has to be a **variable**, because
  `fold` drives a symbolic constant to the end of any flat sum — seven
  spellings of the association, `(s32)` casts included, measured *exactly*
  the same 66 rows. Splitting the sum also stops `colOff + rowIdx` being
  hoisted as one invariant and keeps the symbol in a register instead of an
  `$at` macro expansion. 66 rows to 16 on that one change.

  **A local widened for a long live range is also a preheader-ordering knob,
  so sweep where it is assigned.** `move_movables` emits the hoists in
  movable-list order, i.e. insn order in the loop body, so `rowIdx = row;`
  written at the top of the loop puts the sign-extension first in the
  preheader where the target has it third. Moved down among the packet
  stores it is still hoisted (the live range is what matters, not the
  position) and the preheader comes out in the target's order: 16 rows to 9,
  with all eight placements between `rb = g_FieldDebugRb;` and the colour
  store identical.

  What you get per function is
  `Register 73 used 47 times across 538 insns; ...; pointer.` -- `n_refs` and
  `live_length` -- and `;; N regs to allocate: ...` at the top of the `.greg`
  entry, which is the allocno list already sorted by priority. Map a pseudo to
  its hard register by finding an insn number in `.greg` (post-reload, prints
  `(reg:SI 13 t5)`) and looking the same insn number up in `.lreg`
  (pre-allocation, prints `(reg:SI 73)`).

  **`.lreg` already prints the answer, though: `;; Register 73 in 3.` is
  `reg_renumber`, i.e. pseudo 73 got hard register 3 (`$v1`).** Those lines
  come after the per-function `Registers live at start:` block, one per
  pseudo `local_alloc` placed, and they make the dump self-contained — refs,
  live length and outcome in one file, with no `.greg` cross-reference and no
  guessing from the emitted asm. Name the pseudos by grepping the same
  function's RTL for `reg/v` (declared locals keep the `/v` and their mode:
  `(reg/v:SI 73)` is a pointer local, `(reg/v:HI 75)` a `u16` one), and read
  everything else off the insn that defines it. On `FieldEventRunInit` that
  turned a twelve-row diff into five numbers and one arithmetic comparison.

  **Caller-saved ties are `local_alloc`, and its formula is a different one.**
  A pseudo the dump describes as "in block N" never reaches `global_alloc` at
  all -- it is allocated by `block_alloc`, which ranks *quantities* by
  `QTY_CMP_PRI = floor_log2(n_refs) * n_refs * size / (death - birth)` and then
  hands each the lowest-numbered free register. So a rotation among `$v0`,
  `$v1`, `$a0`..`$a3` is a statement about that ratio and nothing else, and the
  `.lreg` dump prints both terms. `OpcodeFuncMove`'s three-quantity tie scores
  0.75 / 0.62 / 0.33 where the target needs the reverse order outright.

  **`size` in that formula is `GET_MODE_SIZE` in *bytes*, so a local's
  declared width is a register-allocation lever that emits no instruction.**
  This is the third term and this file spent four sessions treating the
  formula as if it had two. An HImode quantity is scored at *half* an SImode
  one, so `s16 x = expr;` and `s32 x = (s16)expr;` -- identical value,
  identical instructions, identical `n_refs` and `live_length` -- rank a
  factor of two apart. `FieldEventRunInit` in `src/field/field4.c` sat at 12
  rows because its `s16 numExtras` scored `2*6*2/11 = 2.18` against the
  competing `lo` at 4.0 and lost `$a1` to it; widening the declaration and
  moving the truncation into the expression flips the ranking and the
  function **matches**. Nothing else in the body changed.

  Two consequences worth having in front of you. A residue that reads as
  "two block-local quantities holding each other's register" is *always*
  worth a width sweep before anything else, because the sweep varies exactly
  this term and `variant_eval` scores the whole cross-product in a minute.
  And the check that a width is right for the right reason is still the
  target's extension opcode: `u16`/`u8 numExtras` reach the same allocation
  (3 rows) and cannot emit the `sll 18` / `sra 16` the target has, so they
  are a coincidence, where `s32` carrying the `(s16)` cast is the same
  arithmetic *and* the same ranking.

  `FieldDebugRenderString` is the second worked example and the cheapest one:
  its last eight rows were a straight `$t0`/`$t1` swap between `chars` and
  `charOff`, two quantities with **six references each** and lives of 30 and
  31 — 1.600 against 1.548, a 3% gap that no reference-count change can
  reach. `s16 chars` halves its `size`, drops it to 0.800, and the swap goes.
  `tools/qty_pri.py` prints `pri`, `n_refs`, `live_length`, `size` and the
  hard register each pseudo actually got, in one table sorted by priority, so
  the gap is a number before any edit is made; `width_sweep.py` then finds
  the fix in one run. Both were run *after* the body was already at 8 rows —
  a width sweep on the parked body had plateaued and the note recorded the
  widths as closed, which was true of that body and of no later one.

* **A value the target keeps in a *global* allocno's register has to be
  assigned to that variable at every step, not computed by one expression.**
  `local_alloc` runs before `global_alloc`, so every block-local pseudo takes
  its register first and the cross-block value gets what is left. A value
  computed in two arms and read in the shared tail is a global allocno; write
  its address as one expression and each intermediate is a *fresh block-local
  pseudo* that `local_alloc` hands `$v0`, which then pushes whatever else the
  block needs up a register. Assign the same variable at each step and the
  whole chain is one pseudo -- the global's -- and comes out in place:

  ```c
  pc = g_CurrentEntity;                  /* lbu  a0, ...      */
  pc = pc * 2;                           /* sll  a0,a0,1      */
  pc = pc + (s32)g_FieldScriptPC;        /* addu a0,a0,s0     */
  ```

  against `pc = &g_FieldScriptPC[g_CurrentEntity];`, which gives
  `lbu v0 / sll v0,v0,1 / addu a0,v0,s0` and costs the neighbouring
  `g_FieldState` pointer `$v0` as well. `OpcodeFuncVwoft` in
  `src/field/field4.c` was parked on exactly this for three sessions with a
  note that had correctly identified the tie and concluded neither term was
  reachable; 12 rows to 0. Two steps instead of three is 9 rows and **+1
  instruction** -- the load still needs its own register and the delay slot
  after it takes a `nop` -- so the chain has to start at the variable, not
  merely end there. The tell is an `addu` whose destination is a
  callee-of-the-join register and whose first source is a different one,
  where the target has the same register twice.

  **When it *is* reachable, one loop-weighted reference is the whole lever.**
  The counter-case to the paragraph below, and the numbers say which you are
  in before you edit anything. `FieldModelStructInit`'s two competing
  quantities scored 0.464 (`d`, 15 refs / 97 insns) against 0.424 (`i`, 14 /
  99) -- a 9% gap, where `OpcodeFuncMove`'s three had to invert outright.
  Routing *one* of two `d->modelCount` accesses through the parameter instead
  of the copy drops `d` to 14, and because that reference sits inside a loop
  (flow weights `REG_N_REFS` by depth) one is enough. Which of the two moves
  is irrelevant -- all three placements measure identically, which is the
  evidence that the count is the lever and not the expression. Dropping two
  *non-loop* references instead does not work: both pointers then stay live
  and each costs a load. 21 rows to 1, with a store reorder.

  **Neither term is reachable from C without emitting an instruction when the
  gap is large, so a residue that reduces to a wholesale priority inversion is
  a park, not a search.**
  `n_refs` is fixed by the arithmetic -- `entryIdx * 36` decomposes into
  `x*8 + x` then `<< 2`, which is three references whatever you call it -- and
  cse re-shares a constant however it is spelled, so an extra reference to a
  value that already exists is folded away. `live_length` can only be stretched
  by moving a definition earlier, and that either materialises a load that was
  free (`OpcodeFuncMove`: 37 to 48 rows and 3 to 8 extra instructions for four
  placements, against 14/0) or is dead code that flow deletes and changes
  nothing (`HandleKawaiDataInModel`: two placements, exactly 13 rows both
  times). Between them those two functions cover both terms and both outcomes.

  The payoff is knowing when to stop. `AddBackgroundToRender`'s residue is 65
  rows of pure register naming and the three quantities that have to permute
  score -0.56, -0.88 and -0.95; for the second to overtake the first its
  reference count would have to go from 5 to 12 and the third's from 4 to 16.
  That is not something a spelling change reaches, and two sessions of trying
  to add references one at a time would have found nothing. Note also what the
  numbers do *not* settle: `find_reg` gives an allocno the lowest-numbered
  register that does not conflict, so a different assignment can come from a
  different conflict set rather than a different ranking, and the target's
  `.s` cannot tell you which.

* **On a parked body, sweep every local's *width* before anything else, and
  let `insn_histogram.py` tell you whether to.** A wrong length plus a
  `lhu`/`lh` or `andi`/`sll`+`sra` imbalance in the histogram is a
  declaration fact, not codegen, and the fix is one word. `FieldMain` sat at
  81 rows and +1 instruction with a histogram reading `ori +2 / addiu -2`,
  `lhu +1 / lh -1`, `lui +1`, `andi +1`, `nop -1`; the lone `lh` the target
  has and this build had nowhere is a `u16` global read into a local, and
  declaring that local `s32` rather than `s16` takes it with a sign-extending
  `lh` -- 81/+1 to 62 and the length exact, then 62 to 54 for the second
  local in the same statement. Seven opcode counts named the fix where
  eighty-one rows had not. The widths are a small finite cross-product
  (`s16`/`u16`/`s32`/`u32` per local) and `variant_eval` scores the whole
  sweep in one run, so there is no reason to sample it.
  `tools/width_sweep.py` does exactly that -- every alternative width for
  every scalar local of one function, scored and sorted by length first --
  and it is cheap enough to run before reading a single diff row. **Re-pin
  before running it**: it takes the declaration block from the pinned base
  (it used to read the live source), because `variant_eval` applies every
  edit to the pin, so a block that has moved on since the pin matches nothing
  and *every* variant aborts. The table then comes back empty, which reads
  exactly like "no width is worth anything"; it now refuses rather than
  printing a header with no rows.
  `FieldCalcPointOnLine`'s 150 variants over 30 locals come back flat in
  about a minute, which closes a whole dimension that
  `perm_randomize_internal_type` would otherwise spend a search on.

  **Run it on a residue that reads as pure register naming too, not only on
  one that reads as a wrong declaration.** The width is a term in
  `QTY_CMP_PRI` (see the `size` bullet above), so a sweep is a sweep of the
  *allocation*, not only of the load opcodes -- and it is the cheapest way to
  vary that term, because every other way of changing a quantity's ranking
  emits an instruction. `FieldEventRunInit`'s last twelve rows were a
  two-quantity swap that nine hand-shaped attempts, a `.lreg` dump and eleven
  barrier placements had failed to move, and `width_sweep` named it on the
  first run: `numExtras s16 -> u16, 3 rows, exact`, with everything else in
  the table flat at 12. Reading *why* that row was 3 rather than 0 gave the
  matching spelling in one more measurement.

  (The `ori +2 / addiu -2` in the histogram quoted above was taken before the
  opcode fold was fixed and is the alias artifact described further down, not
  a real column; the `lhu +1 / lh -1` that actually named the fix is real,
  since loads are never aliased. The fix and its row counts stand.)

  **The positive check on a width is the target's extension *opcode*, not the
  row count.** `width_sweep` measures and does not reason, and a wrong width
  can score beautifully: `y` as `u8` scores 792 rows at the exact 1341 in
  `FieldDebugRenderPage`, and the target emits `sll 16 / sra 16` on that
  value — a sign-extension from 16 bits, which no `u8` or `u16` local can
  produce. So read what the target does to the value before taking the row:
  `sll 16/sra 16` means `s16`, `sll 24/sra 24` means `s8`, `andi 0xff` means
  `u8`, and a load folded to `lh`/`lbu` with no separate extension means the
  width is already the declared one. A width that scores well and cannot emit
  the extension the target has is a coincidence.

  Two things make it pay much more than one run suggests. **Re-run it after
  every change it finds** -- the sweep is stale the moment one width lands,
  and the wins compound: `FieldDebugRenderString` went 100 rows and -1
  instruction -> 98 and exact on `charOff`, then 98 -> 96 on `glyph`, which
  was invisible until `charOff` had moved. And **read the value's range and
  its tests before taking a row**, because the tool measures and does not
  check semantics. `FieldEntityMovementUpdate`'s best two candidates are
  `dy4` as `u8` (+21 -> +7 instructions, and it is a 20.12 fixed-point delta
  a byte truncates) and `frameB5` as `u16` (500 -> 498 rows, with
  `if (frameB5 < 0)` on the next line, which unsigned makes dead). Both would
  have looked like the find of the session.
* **A park note's numbers are only true of the body they were measured on,
  and the body moves.** This file already says to re-check a note's
  *diagnosis*; the stronger version is that its **rejected-spellings list
  expires too**, and three functions in one session turned on exactly that.
  `FieldEntityWalkmechCross`' note recorded per-arm locals as worth 13 rows;
  merging them back is worth 16 once the guard is nested. `FieldMain`'s tail
  paragraph quotes a 65/3 base while the function measured 81/9, so every
  number in it was stale and the one width it never tried was the fix.
  `LoadLocalFieldModelAndInitAll` carried "-1 instruction" for as long as the
  note existed and nobody chased it as a *length*; it was one statement
  order. The cheap habit: before spending a budget on a parked function,
  re-measure two or three of the note's own rejected entries. If they still
  agree, the note is live; if any has moved, re-run the whole list.
* **Read the allocno arithmetic *forwards*: it does not only tell you when to
  stop, it tells you what the fix has to deliver.** This file already records
  the priority formulae as a stopping criterion -- work out which term you can
  change, and park if the answer is none. Run the other way they are a
  specification. `FieldEntityWalkmechCross`' residue was 34 rows that read as
  pure register naming; cc1's `-dl` dump named the two quantities and both
  terms, `link` at 3 refs over 12 insns (`(3-12)/12 = -0.75`) against its
  sign-extension at 3 over 6 (`-0.50`), so the extension is allocated first
  and takes the lower register. That is not a verdict, it is an equation with
  a solution: `link` at **4** refs is `(2*4-12)/12 = -0.33` and wins, because
  4 crosses a `floor_log2` step. The question then stops being "what should I
  try" and becomes "what source change gives that variable more references",
  which has a small and enumerable answer set. Two changes fell straight out
  of it and took the function to 6 rows at the exact length.
* **Per-arm locals and one shared local are a lever in *both* directions, and
  which way it points depends on the surrounding guard shape.** This file
  records splitting one variable per arm as worth 13 rows on
  `FieldCalcPointOnLine` and on `FieldEntityWalkmechCross`, and that is true.
  It is also true that *merging* the same three locals back into one is worth
  16 rows on the same function once its guard is written as nested `if`s
  rather than `&&` -- because merging is how a HImode value gets from 3
  references to 9 and out-ranks the sign-extension competing with it. The two
  measurements are 99->86 and 22->6 on the same lines of the same function.
  So this is not a rule with a direction; it is a reference-count knob, and
  the `.lreg` dump is what says which way to turn it. Re-measure both
  directions after any change to the guard around them.
* **Nest a guard rather than `&&` when a value's live range is what you need
  to shorten.** `shift = link >> 3; if (link >= 0 && bit(shift) == 0)` and
  `if (link >= 0) { shift = link >> 3; if (bit(shift) == 0) }` are the same
  program; the second computes the sign-extension's consumer inside the test,
  which shortens that pseudo's live range and re-ranks it. Worth 12 rows
  across three arms in `FieldEntityWalkmechCross`, and it fixed a *different*
  register pair (the table base against the scaled index) as a side effect.
  Note this is the opposite lever from `OpcodeFuncLader`, where a range test
  had to be *un*-folded into nested `if`s to stop `fold_range_test`; here the
  nesting is about live ranges, not about folding.
* **When two locals hold each other's register and nothing you write moves
  them, you are looking at `allocno_compare`, and only two of its three terms
  are reachable from C.** gcc 2.6.3 sorts global allocnos by
  `floor_log2(n_refs) * n_refs / live_length * size` and breaks ties by
  *allocno number*, which follows pseudo creation order and therefore
  declaration order. So: if swapping the declarations changes nothing, the
  priorities are not tied and the lever has to be the reference count or the
  live range — not the order. And `n_refs` is weighted by loop depth
  (`REG_N_REFS += loop_depth` in flow), which is why a value referenced inside
  an inner loop beats a loop bound referenced only in the outer one. Three
  functions in `src/field/` are parked on exactly this and all three read as
  pure register renaming: `HandleKawaiDataInModel` (s5/s6, `faceSel` against a
  loop-top constant), `FieldModelStructInit` (a3/t0, the counter against a
  pointer copy) and `KawaiLightingApplyToPolyColor` (t2/t3, a loop count
  against an inner-loop snapshot). Before spending a budget on one of them,
  work out which term you can change; if the answer is none, park it.
* **Declaration order is inert *except* between two locals of the same type
  that are live at the same time.** CLAUDE.md's standing rule (measured on
  `func_801B009C`, five permutations, no change) holds for values that never
  compete: gcc allocates by priority, not by position. Two same-type locals
  live across the same loop *do* compete, and there the one declared first gets
  the higher-numbered register — `s16 bestId; s16 best;` against `s16 best;
  s16 bestId;` is 8 rows in `FieldEntityCheckTalk`. Try the swap when the diff
  is two registers trading places and nothing else.
* **`x / 32` and `x >> 5` are not the same instruction.** A signed division by
  a power of two has to round toward zero, so gcc emits `bgez`/`addiu N-1`
  ahead of the `sra`; a shift is the `sra` alone. Four instructions per site,
  and it reads as noise in the middle of a division sequence. When the target
  has a `bgez` over an `addiu` of a power-of-two-minus-one, the source wrote a
  division. `FieldEntityDirByVec` in `src/field/field2.c` has two.
* **`-x * 2` and `-(x * 2)` index differently.** Negating first makes gcc
  compute the index into its own register before it materialises the array
  base, so the shift is free to be stolen into the preceding branch's delay
  slot and the base is subtracted from; folding the negation outward builds
  the base first and leaves the slot empty. Same value, three rows across four
  arms — the last three rows of `FieldEntityDirByVec`.
* **An in/out pointer parameter written twice is what forces it into a
  callee-saved register.** `*p = a; x = f(*p); *p = x;` gives the `move s0,a2`
  and an extra saved register that the single-store version does not need. If
  the target saves one more register than your code and there is a spare store
  to a parameter in the diff, the function overwrites its own output — read
  what the callers compare it against before assuming the first store is the
  only one. `FieldEntityDirByVec` returns the *distance* through a parameter
  the seed had named `sqrDist`.
* **A pointer to a global object is spelled inline at every use, not held in
  a local — and the same goes for a struct field read more than once.** This
  is the inverse of the "name the temporary" idioms above and it is worth as
  much. `block = (FieldTexBlockHeader*)D_800DFCA0;` makes the pointer live
  across the whole arm, so gcc loads the global once and then has the *other*
  loads in the block free to fill its load-delay slots; written as
  `((FieldTexBlockHeader*)D_800DFCA0)->field` at each use, cse rematerialises
  the base per reference and the slots stay empty the way the target has them.
  22 rows to zero on `FieldModelBsxTdbModify` for what reads as a style
  regression. `AddBackgroundToRender`'s four wrap tests are the same lever on a
  struct field — `buf->Bg2[sprite].x0` read directly rather than through an
  `s16 x` — and worth 40 rows there. The tell is a target that reloads a value
  it could obviously have kept, with a nop where your build has useful work.
  Both were found by decomp-permuter, and neither is reachable by reasoning
  about register allocation, because the change is not about registers.
* **A struct whose *stride* is wrong still shows every field offset correct.**
  `rec[i].f` on a four-word record where the original is five words advances
  by 0x10 against the target's 0x14, and the diff's field offsets agree for
  `i == 0` — which is the row a reader checks. A park note claiming "every
  field offset is correct (verified in diff)" is therefore evidence of
  nothing. Read the scaled index instead: `sll v0,v1,0x3 / addu v0,v0,v1 /
  sll v0,v0,0x2` is ×36, `addiu <r>,<r>,0x14` on a walked cursor is a 0x14
  stride, and either one types the record outright.
* **Two levers that each fit one number can lock each other in place, and the
  only way out is to delete both at once.** `FieldBackgroundInitPackets` spent
  three sessions on a stack layout with three slots, of which the target had
  `0x18` / `0x20` / `0x28`. A `u8 unusedLocals[0x10];` bought the target's frame
  size, and a never-dereferenced `&sprite34Count` moved that counter into the
  declared-local pool where it landed at `0x28` — the target's offset. Both were
  real: each was measured against every alternative in its own dimension (seven
  pad sizes, every subset of counters address-taken, the two grids crossed), and
  the note concluded that the remaining two slots were "not reachable by taking
  more addresses". They were not reachable *because of* the two levers. Deleted
  together, the three slots come out `0x18` / `0x20` / `0x28` in the target's own
  order, the frame is `0x10` short, and the missing `0x10` turns out to be two
  more reload slots that appear only once a *third* thing is fixed — so the
  cluster the note called two independent problems was one cause, invisible from
  either end. The tell is a park note whose dimension has been swept exhaustively
  and whose residue did not move: a fully-swept dimension that still misses means
  the sweep was run with something else held wrong. Delete the levers, take the
  regression, and read the diff of the *plain* body.
* **A loop guard that re-reads the array gives `lh` plus a `move`; a guard on
  the local gives `lhu` into the local's register.** `count = p[2]; if (count
  != 0)` lets combine see the value used only for a zero test and a decrement,
  so it narrows the load and puts it straight in the counter's register.
  `count = p[2]; if (p[2] != 0)` hands the guard its own tree, which promotes
  the `s16` to `int` and needs the sign extension, so the load is `lh` into a
  temporary and cse rewrites the assignment as `move <counter>,<temp>`. Same
  two instructions either way; the target's is one load and one copy, ours was
  one load. Order is a third knob: the assignment ahead of the neighbouring
  store measures 66/9, moved inside the arm 44/12, and between the two 40/10.
* **A park note is not evidence.** Two of this repo's notes recorded diagnoses
  that were simply wrong and cost more than the functions did:
  `FieldModelStructInit`'s said "-0x38 frame, six callee-saved registers,
  needs the permuter to find the lean local set" — that frame belonged to the
  *next* function, which `diff.py -o` renders past the end of the one you
  asked for, and the real frame is `-0x10` with no saved registers at all.
  `FieldModelBsxTdbModify`'s said the residue was "gcc's inlined memcpy
  expansion, a scheduler/expansion coupling", when the record struct was the
  wrong size. Re-derive a note's claim from the `.s` before spending a budget
  on top of it; the rejected-spellings list in a note is worth keeping, the
  diagnosis is worth re-checking. And a rejected-spellings list is only as
  good as the set of spellings it enumerates: `FieldBackgroundInitPackets`'
  note recorded `while` (191/43) and `for (;;) { … break; }` (177/36) for its
  four run walks and concluded from those two numbers that "the goto walks are
  load-bearing". The third spelling, which this file documents two bullets
  above, is the one that works, and it was worth 48 rows plus the register
  allocation the same note called "the whole function" and had found no lever
  for. When a note rejects a lever, check it rejected *every* spelling of it.
  A note's *rejected-spellings* list carries the same risk one level up: it
  is only true of the program it was measured against. `FieldEntityLineCheck`
  had all three of the levers that finished it written down as measured and
  rejected -- all fields through the pointer, the parameter as the cursor, a
  local for the read-back -- at 79, 91 and 96 rows. Every one of those numbers
  was taken before four *semantic* corrections landed in the same function,
  and against the corrected body the same three measure 24, 9 and 0. When a
  note records a program fix and a codegen sweep in the same paragraph, the
  sweep predates the fix and has to be re-run.
* **Correct the program first, then hand it to the permuter.** These two are
  not alternatives and the order matters. `FieldModelBsxTdbModify` sat at
  44 changed / 7 inserted; a permuter run on that body is hill-climbing a
  function that computes the wrong addresses, and the four program fixes
  (guard, indexing, struct size, absolute destination) took it to 22 by hand
  in one sitting. Only then did the permuter close it, in 457 candidates and
  about seven minutes on five workers. The converse holds too: no amount of
  reading the target suggests *un-naming* a variable, which is what the last
  22 rows were.
* **`(u8)x` vanishes into a narrowing store; `x & 0xFF` does not.** Storing to
  an `s16` array element, `quality[i] = (u8)(a - b);` emits no mask — `sh`
  truncates anyway, so combine deletes the `andi` and maspsx fills the slot
  with a nop — while `quality[i] = (a - b) & 0xFF;` is arithmetic on the
  promoted `int` and survives to the store. Same value, one instruction apart.
  When the target masks a value it is about to store narrowly, the mask comes
  from the *expression*, not from any declaration: `FieldEntityCheckTalk` had
  every type for the subtrahend measured (u8, s16, u16, u32, s32, with and
  without a cast at the use) and all five compile identically, because none of
  them is where the `andi` comes from.
* **Where you put a constant's local decides which invariant is hoisted
  first.** Two levers already in this file combine into a third. A
  loop-invariant is hoisted only if its defining insn is on the loop's
  always-executed path, and `move_movables` emits the hoists in the order
  `scan_loop` recorded them — insn order in the loop body. So a *pointer*
  assigned above the loop is not a movable at all and lands ahead of every
  hoist; the same assignment as the loop's first statement becomes a movable
  but still precedes any constant whose first use is further down; and naming
  the constant and assigning it *above* the pointer puts the two movables in
  the other order. `FieldEntityLineInteract` in `src/field/field2.c` measured
  19, 17 and 0 rows for those three arrangements of the same two values —
  `active = 1;` then `pad2 = &g_FieldPad2State;`, both at the top of the loop
  body. The tell is two preheader initialisations in the wrong order with a
  callee-saved rename cascade behind them and nothing else wrong.

  The corollary is that a park note reading "this value is materialised early
  here and late in the target" is only half a diagnosis: check the *position*
  of the local before concluding the local itself is wrong. That note had
  rejected the one alternative it tried and concluded "the local is right" —
  it was right, and in the wrong place.

  The cheapest form of the same lever is a local for an *index*, and it is
  worth reaching for whenever the two hoisted constants are simply in the
  wrong order. `arg1[3 - i] = (t - q * 10) * 8 + 0x98;` after a `q = t / 10;`
  records the division's magic multiplier as a movable before the `3`, since
  `scan_loop` sees the division's insns first; `j = 3 - i;` as the loop
  body's first statement records the `3` first and nothing else moves.
  `func_800C2F20` in `src/battle/battle1.c` matched on that one line from 2
  rows, and the alternatives that look equivalent are not — writing the store
  before the division (so `expand_assignment` computes `&arg1[3 - i]` first)
  measures 20 rows, because it also changes where the quotient's live range
  starts.
* **A rotation of two or three registers inside one basic block is caused by a
  value that is not in the block.** When the whole residue is `$v0`/`$a0`
  trading places across a lookup — same opcodes, same order, same count — the
  extra pressure comes from something *computed above* the block and *used
  below* it, which is live across it and competes in local-alloc's quantity
  ordering. It is invisible in the block's source, which is precisely why every
  spelling of the block measures identically. `OpcodeFuncCanim` and
  `OpcodeFuncCanmEx` in `src/field/field4.c` were 10 rows out for exactly this:
  `lastFrame = GET_PARAM_U8(3) / divisor;` sat with the function's other two
  divisions, above `&g_FieldModelData->modelEntries[loader[i].modelEntryIndex]`,
  and was consumed by the clamp below it. Moving that one line down to the
  clamp matched both functions outright — after a named `entryIdx` local, every
  modelIdx type, repeating `g_EntityToModel[g_CurrentEntity]`, declaration
  order, `- 1` against `+ 0xFFFF` and `u8 unusedLocals[N]` had all been
  measured on the lookup itself and were all inert. Look for the crossing value
  before touching the expression; when there is no such value the rotation does
  not move at all, which is where `OpcodeFuncMove` and `FieldMoveToEntityUpdate`
  are still parked.
* **A chained assignment expands the destination address before the value; two
  statements expand the value first.** `expand_assignment` computes the address
  of the left-hand side and only then calls `store_expr` on the right, so
  `s->field = t = f();` emits the address computation ahead of `f()`'s
  operands, while `t = f(); s->field = t;` emits `f()` first because it is a
  complete statement of its own. When a store's address and its value are each
  several instructions — an indexed lookup on one side, a chain of loads on the
  other — the two spellings give the same instructions in two different orders,
  and no amount of scheduling advice reaches it. `OpcodeFuncOfstd` in
  `src/field/field4.c` was 11 rows out on exactly this: the target reads
  `g_EntityToModel[g_CurrentEntity]` before the script PC, and only the chained
  form produces that. This is a different lever from the right-to-left
  *store order* of `a = b = c = K` recorded above; both come from the same
  statement shape.
* **Two return blocks are laid out in source order, and reorg can only fill a
  delay slot from a block it *jumps* to.** An opcode handler whose body is
  `if (ok) { ... return 1; } PC_INC(n); return 0;` lays the `return 1` block
  first, so the body falls straight into it and every branch that wanted to
  reach it is a fall-through with nothing to steal. Close the `if` with an
  explicit `goto done;` and put `done: return 1;` *after* the `PC_INC` tail and
  the two blocks swap: the body now ends in a `j` whose delay slot takes the
  store above it, and an early `goto done` gets `li v0,1` duplicated into its
  own slot. `OpcodeFuncTurn` went 11 rows to MATCH on this and `OpcodeFuncTurnr`
  18/1 to 2. This is the third variation on where a handler's tail goes —
  `OpcodeFuncTurnw` wants the tail duplicated at every early return,
  `FieldMoveToEntityUpdate` wants one tail the guard jumps into — so read the
  target's block addresses rather than picking by taste.
* **A string literal's `.rodata` slot is decided by the order gcc expands the
  STRING_CST, and `if`/`else` expands its true arm first.** `name = "turnr";
  if (c) { name = "turnl"; }` and `if (c) { name = "turnl"; } else { name =
  "turnr"; }` compile to the same instructions in the same order — they differ
  only in which literal is defined first, and therefore in every `%lo` in the
  function. **This is the fourth way a clean-looking diff lies**: `checkfn.py`
  reports the shifted references as two rows of "register naming", the function
  looks one small step from done, and `make build` fails the whole overlay's
  SHA-1 because every later `.rodata` offset moved. When the only rows left are
  `%lo` of a string, read the `.s`'s `glabel` order and make the source expand
  the literals in that order; a ternary is byte-identical to the `if`/`else` if
  you prefer it. `OpcodeFuncTurnr` in `src/field/field4.c` needs turnl before
  turnr.
* **A raw m2c seed that addresses a struct as `*(&D_800E4900 + i * 0x28)` is a
  typing problem, not a codegen one, and it is the cheapest work in the file.**
  Sixteen `extern /*?*/ s32 D_800E49xx;` placeholders eight bytes apart are one
  `POLY_FT4[]`; resolving them against the PSY-Q layout and writing the members
  took `DrawFieldExitArrow` in `src/field/field4.c` from 255 rows and 5
  insertions to MATCH. Read the offsets against the header before assuming the
  residue is scheduling: +8/+0xA are x0,y0, +0xC/+0xD u0,v0, +0x10/+0x12 x1,y1,
  and so on. The same shape — a wall of `/*?*/` externs at a fixed stride —
  marks every remaining m2c seed worth rewriting.
* **A pointer into one member, spelled as an offset from a *different* member
  of the same object, is derived for free; through its own symbol it costs a
  whole `%hi`/`%lo` pair.** The companion to the neighbouring-object idiom
  above, applied to two members rather than two objects. `FieldMain` keeps a
  `volatile u8*` on `FieldState.eventCmd` (symbol+1) and also stores through
  `&FieldState.fadeType` (symbol+0x4C); written as `ev = &D_8009ABF4.eventCmd`
  the two are unrelated to cse, because the fade block is reached through its
  own `extern` (`D_8009AC40`) and the pointer through the struct — different
  `symbol_ref`s, two independent address materialisations. Spell the pre-loop
  store as `D_8009ABF4.fadeType = 0;` and the pointer as
  `(volatile u8*)&D_8009ABF4.fadeType - 0x4B;` and both sides name
  `D_8009ABF4`, `use_related_value` relates them, and the pointer comes out as
  `addiu <ev>,<fade>,-0x4b` — which is the form the target has. Worth a row
  and the whole address form; the tell is your build emitting two `lui`/`addiu`
  pairs where the target emits one plus an `addiu`.
* **A constant the target materialises into a callee-saved register at a goto
  walk's entry is a named local, and the walk then needs its own back-edge
  label.** `ori $s1,$zero,0x80` sitting alone in the block a `goto` falls into,
  with the walk's own back edge targeting the *next* label, is not cse being
  clever: it is `white = 0x80;` written between the walk's entry label and its
  test. Written as literals at the six `r0`/`g0`/`b0` stores gcc
  rematerialises the constant inside the loop. `FieldBackgroundInitPackets`
  needs one at the layer-3 entry and another at the layer-4 entry, which is
  what forces `layer4:` to split into `layer4:` / `layer4run:` — the back edge
  must skip the assignment. The local's type is inert (u8, s16 and s32 all
  measure the same). Do not generalise it to every walk: the same hoist in
  layer 1 costs 51 rows.
* **`n_refs` in `global_alloc` is weighted by loop depth, and the priority has
  a `- live_length` term.** The ranking is
  `(floor_log2(n_refs) * n_refs - live_length) / live_length * size`, and
  flow's `REG_N_REFS += loop_depth` means a reference inside a `do`/`while`
  counts twice. Both halves matter when two values are competing for the last
  callee-saved register: in `FieldBackgroundInitPackets` a counter incremented
  inside two inner loops beats a pointer bumped four times at depth 0, and
  moving one of those increments out of its loop — `spriteCount += count;`
  ahead of the `do`, identical value and identical *static* reference count —
  flips the whole allocation. Use that as a *probe*, not a fix: read the
  target for where the increment really is before keeping the change.

* **A second base register reached by a *negative* displacement is one biv plus
  a combined giv, not two pointers.** `lh 0(a3)` for a record's first field and
  `lh -0x10(a1)` for its second, with `a1 = a3 + 0x12` and both advanced by the
  record stride, reads like two pointers held a fixed distance apart — and
  writing it that way cannot match. It is one walking pointer with two
  strength-reduced address expressions (`p[1]` and `p[9]`), which `combine_givs`
  merges onto a *single* register based at the more-referenced offset, leaving
  the other as a negative displacement from it. The tells are that the far
  base's initialiser (`addiu a1,a1,0x12`) sits among the loop's hoisted
  invariants rather than before them — a giv initialiser goes in the preheader,
  an ordinary `p += 9;` statement goes before it — and that the near base is
  read at offset 0, which is the biv itself. `PreloadNextFieldMap` in
  `src/field/field.c` went 13 rows to 10 on this, and no arrangement of two
  increments reaches it, because the target wants one of them bumped first
  *and* holding the incoming argument register.
* **A pointer global read through its symbol at every use keeps two address
  computations that one cached local collapses.** The exact inverse of the
  "cache a re-read global pointer" idiom above, and the target tells you which
  it wants: two `addu`s of the same base and index, in *opposite operand
  orders*, mean the two addresses were expanded from two separate loads of the
  symbol. Through one `T* table = g_Sym;` local both are the same rtx on the
  same pseudo and cse deletes the second. Nothing about the index spelling
  reaches it — `(s32)table + i * 24 - 4` against `i * 24 + (s32)table`,
  pointer-first against index-first, and either one alone are all
  byte-identical to the plain subscript, because the operand order is decided
  by which pseudo holds the base, not by the source. Worth 7 rows in
  `PreloadNextFieldMap`. Reach for the local when the target *reloads* the
  global; reach for the symbol when it computes the same address twice.
* **A named local for a call's address argument, assigned *before* the
  statement above the call, is how that call's delay slot stays a `nop`.**
  reorg fills a `jal`'s slot from the insn immediately before it and will not
  take a store (a call may read memory), so a target with an empty slot after
  `jal` is telling you the last insn before the call is the *store*, with the
  argument move already issued. Written inline, `expand_call` emits
  `move a1,<addr>` right before the `jal` and reorg takes it. Hoisting the
  address into a local assigned one statement earlier -- `base =
  &Savemap.header.leader_level;` above the memcpy that precedes
  `D_80062D99 = 1; f(0x10F0, base);` -- makes sched2 issue the move first and
  leaves the flag store adjacent to the call. Worth the last instruction and
  six rows in `func_801D2408`. Where the assignment goes is the whole lever:
  one statement earlier is right, at the top of the block it is hoisted out
  and costs a second materialisation of the object's base, and empty
  `do { } while (0);` barriers at six points around the call are inert or
  worse -- which is the probe saying the residue was allocation, not
  scheduling, and that the position of a *definition* was the thing to move.
* **A call written out in both arms of an `if`/`else` puts its shared argument
  setup ahead of the branch.** With one call after the `if`, reorg fills the
  conditional branch's delay slot from the fall-through thread, is left with
  the call's remaining argument to place, drops it in the first arm's `j` slot
  and duplicates it into the second arm — which reads as three rows of
  scheduling noise. Duplicate the call and cross-jumping merges the two into
  one tail; the argument setup they share ends up *before* the branch, where
  `fill_simple_delay_slots` takes it on its first pass. That is what a target
  with a materialised constant in a conditional branch's delay slot and a
  `nop` in the following `j`'s slot is telling you. It closed
  `PreloadNextFieldMap`; a named local for the constant assigned before the
  `if` does not reach it, because cse folds the constant back to the call site.

* **m2c invents a prototype for every extern it sees called, and a narrowing
  one is worth hundreds of rows.** For `FieldCalcLinearStep(start, target,
  duration, step)` — whose real definition in `src/field/field.c` takes four
  `s32` — m2c wrote `s32 FieldCalcLinearStep(s16, s16, u8, u8);` at the top of
  `field2.c`, reading the parameter types back out of what the *caller*
  happened to pass. gcc then converts each argument at the call site and
  `force_to_mode` pushes the narrowing into the load, so an `s32` member reads
  `lh` instead of `lw` and an `s16` member reads `lbu` instead of `lh`. Every
  such call site is one wrong opcode, the values are still correct, and the
  diff reads as register noise. `FieldEntityMovementUpdate` had 44 of them:
  correcting the two prototypes took it from 608 rows to 572 and removed 28
  spurious `lbu` outright. Deleting the *casts* m2c also writes at the call
  sites (`(u8) e->MoveSteps`) is exactly inert — the prototype is what
  narrows, so fix the declaration. Check every `// extern` line m2c leaves
  behind against the real definition before reading a single diff row;
  `grep -rn "<name>" src/ include/` finds it, and the callee usually lives in
  a sibling unit of the same overlay.

  `tools/struct_access_audit.py` is the check that finds these without
  reading the diff: it counts every access to an array-of-struct global on
  both sides, keyed by member offset and opcode, alignment-free, so it stays
  meaningful while the function is still hundreds of rows out.

* **Arms that each carry the *whole* argument setup mean the call is written
  twice, not hoisted after the `if`/`else`.** The companion to the
  `PreloadNextFieldMap` bullet below, and the commoner shape: where that one
  duplicates a call so cross-jumping pushes the *shared* setup above the
  branch, this is the case where nothing is shared. `lui a0,%hi(g_DebugText)/
  addiu a0/lui a1,%hi(str)/addiu a1/j` in one arm and the same four
  instructions with a different `a1` in the other, with a single
  `jal`/delay-slot tail after the join, is `f(x, A)` and `f(x, B)` written out
  in the two arms -- cross-jumping merges only the `jal` and its delay slot,
  because the tails differ from the second instruction back. A single call
  after the `if`/`else` over a variable emits the setup once and is short by
  the whole duplicated prologue, four to eleven instructions *per site*.
  `DebugUpdateActor` in `src/field/field4.c` has 36 such arms -- eleven flag
  characters, three transparency markers, two colour ladders -- and they were
  worth 119 instructions, two thirds of everything it was under length. The
  tell is a pair of blocks that load the same `a0` and a different `a1`, which
  is the *same* tell as the `SetSemiTrans` bullet above; read whether the
  shared argument is loaded inside the arms (duplicate the call) or before the
  branch (hoist it).
* **The converse tell is a call whose arguments the target *rematerialises*:
  that call is written once, after the whole `if`/`else` chain, and the join
  is what empties cse's table.** A block with more than one predecessor
  starts with nothing known, so an address that any arm already had in a
  register is rebuilt with `lui`/`addiu` there, and a `short` parameter is
  re-extended with its own `sll`/`sra` rather than read out of the
  callee-saved register an arm parked it in. Both of those are visible in the
  `.s` and neither can happen if the call sits inside the arms.

  The second-order effect is what pays. Nested inside each arm, the call's
  `(s16)arg` has a *later* use in the same arm and so is live across the
  preceding call: it takes a callee-saved register and every argument setup
  becomes `sll`/`sra`/`move a0,s0`. Hoisted, the extension dies at the call
  in each arm and goes straight into `$a0`, which is one instruction per arm.
  `DebugUpdateActor` in `src/field/field4.c` had m2c's nest of three
  `if (guard) { SetDebugStrRowColor(...); if (guard) { SetStrToDebugRow(...,
  g_DebugText); } }` arms; lifting the inner call out to a single
  `if (guard) SetStrToDebugRow(..., g_DebugText);` after the chain took it
  from **144 rows and +2 instructions to 0**. The duplicated inner guard is
  the give-away that m2c reconstructed a nest from a CFG that was not one.

  Read the two rules together as one question -- *does the target build this
  argument in the arm or at the join?* -- and note that one function can want
  both: the same `DebugUpdateActor` wants its row-7 clear **duplicated** into
  the two paths that reach it, because there the two `a0` setups differ
  (`move a0,s0` on one path, `sll`/`sra` on the other) and cross-jumping
  merges only the `a2`/`a1`/`jal` suffix behind them.
* **A pre-scaled byte offset only defeats the folded base if it is a local.**
  Sharpening the `$at` rematerialisation bullet above: what `associate` takes
  apart is a `PLUS` whose operands are a symbolic constant and a `MULT`, and an
  index written inline is still a `MULT` however the address is spelled. So
  `*(s16*)((u8*)&SYM + 0x4 + arr[i] * 0x18)` hands gcc
  `(plus (symbol+4) (mult (reg) 0x18))`, fold lifts the bare `SYM` out as the
  common subexpression of all the fields, and one `lui`/`addiu` in a
  callee-saved register then serves every access. Assigning
  `off = arr[i] * 0x18;` as its own statement immediately above each access
  gives `(plus (symbol) (reg))`, which stays in the `mem` and comes out as the
  assembler's `lui at/addiu at/addu at` per field. Both halves are needed:
  hoisting *one* `off` for the whole block deletes the re-derivations the
  target has and measures -42 instructions, and spelling the interior address
  as a member (`&SYM.pos.z1`) rather than `&SYM + 0x4` is exactly inert.
  `DebugUpdateActor`'s six FieldLine reads needed it, 35 rows.
* **A scalar global whose *address* the target keeps in a register, read many
  times, is a named pointer local -- not `volatile`.** CLAUDE.md already gives
  two routes to the register form, a second reference and `volatile`; there is
  a third, and on a global read nine times it is the only cheap one.
  `lui s3,%hi(SYM)/addiu s3` once with `lh 0(s3)` at every read, against a
  `lui %hi` per read in your build, is `s16* p = &SYM;` and `*p`. `volatile`
  reaches the same address form and brings its own cost -- every `lh` becomes
  `lhu` plus a separate `sll`/`sra` -- so on a target that loads signed it is
  a wrong answer that *looks* right in the `lui` column alone (measured on
  `D_8009AC1E` in `DebugUpdateActor`: 148 rows as a pointer local, 204 and
  +27 instructions as `volatile`). Where the pointer is assigned is
  load-bearing as usual: at the top of the block that uses it, 148 rows; at
  the top of the function, 179.
* **Read the parameter types off the prologue before anything else.**
  `addu $s6,$a1,$zero` -- a plain copy -- cannot come from a `u8` parameter,
  which gcc masks on entry with `andi $s6,$a1,0xff`; and `sll/sra 16` at every
  use with no extended copy kept is what a `short` parameter looks like. m2c
  infers narrow parameter types from what the *caller* passes and then writes
  `(s16)` casts at every use to model the re-extension, which reads as
  faithful and is a different program. `DebugUpdateActor`'s declared
  `(s32, u8)` was `(s16, s16)`, worth 8 instructions and 25 rows; the four
  spellings measured 256 (s16,s16), 259 (s32,s16), 261 (s32,s32), 278
  (s16,u8), 281 (s32,u8).
* **A walked record's fields all go through the pointer, and the pointer is
  the parameter itself.** Two separate levers that both look like style. First,
  mixing `p->field` for some accesses and `arr[i].field` for others gives the
  loop a second address giv based at +0, where the target bases it at the
  offset of the field group it actually walks -- every displacement in the body
  then reads high by that constant and the diff looks like a wall of wrong
  offsets. Route every field through the pointer and the base moves.
  Second, `T* p = param;` walked with `p++` is not the same as walking `param`:
  gcc merges the two pseudos and emits the copy where the *loop* starts, while
  the parameter's own copy is emitted in the entry block -- so a target with
  `move s2,a1` among the register saves is telling you the parameter is the
  cursor. `FieldEntityLineCheck` in `src/field/field2.c` measured 40 rows
  mixed, 24 all-through-the-pointer, 9 once the pointer was the parameter.
* **A `u8`-returning call's result stored to a `u8` field is not reloaded when
  you read the field back -- gcc trusts the callee's extension, so cse
  substitutes the return register for free.** The target's `sb`/`lbu` pair at
  the same address therefore cannot be reached by any spelling of the read:
  `p->field`, a cast through `u8*`, and a `u8` local all give the same code,
  because a `u8` local coalesces with the return value. Reading the field into
  an `s16`/`s32` local is what defeats it -- substituting there would need an
  `andi` to widen, which ties `lbu` on cost, and cse only substitutes when
  strictly cheaper. This is the same cost rule as the `u8`-tested-through-a-
  local bullet above, reached from the store side. Worth the last row *and*
  the last instruction of `FieldEntityLineCheck`. A `volatile` cast at the use
  reaches the reload too and costs a row elsewhere.
* **A narrow value proved in range is masked only where the proof does not
  reach, so split the statement that carries the proof.** `roll =
  FieldGetNextRandomU8() >> 2;` lets combine see a `lbu`-normalised call result
  shifted right by two, so every later `(u8)roll` folds into the `sltu` and the
  target's `andi <r>,<r>,0xff` never appears. Nothing at the comparison reaches
  it -- `(u8)roll`, `(roll & 0xFF)`, both operand orders and every width of
  `roll` measure identically. Writing it as two statements, `roll =
  FieldGetNextRandomU8(); roll >>= 2;`, stores the result into the `u8` pseudo
  first, so the shift reads a QImode value whose bound gcc no longer carries
  past the block, and the masks come back. 27 rows and the exact length in
  `FieldBattleCheck`. The `u8` and the split are one lever: widening the
  variable after the split loses the masks again.
* **When a value has to sit in its own register, widen the local; a `u8` one
  is inert.** The general form of the two bullets above, and it has now paid
  three times. cse substitutes a register for a memory reference only when
  that is *strictly cheaper*, and a `u8` local is free to substitute -- it
  coalesces with whatever loaded it -- while an `s16`/`s32` local would need an
  `andi` to widen, which ties the `lbu` on cost and stops the substitution
  dead. So a `u8` local reads as "no change at all" and the same statement
  with an `s32` local is worth tens of rows: `sel = faceSel[k];` before each
  table index in `KawaiLoadEyesMouthTexToVram` is 28 rows as `u8` and 7 as
  `s32`, and `angle = line->proximityAngle;` in `FieldEntityLineCheck` is 9 as
  `u8` and 0 as `s32`. When a diff shows a byte in the wrong register, or the
  target reloading something you kept, name it in a *wide* local -- and do not
  conclude from a `u8` attempt that the local is not the lever.
* **A quotient carried in a local is shifted in place; re-derived, the shift
  gets its own destination.** `eyeQ = slot / 8; ... rect.y = (eyeQ << 5) +
  K;` emits `sll <q>,<q>,5`, clobbering the quotient and freeing the scheduler
  to issue it early; writing `rect.y = ((slot / 8) << 5) + K;` lets cse share
  the division but gives the shift a fresh destination, which is what a target
  with the quotient still live after the shift is telling you. Same value,
  same instruction count, and the last row of `KawaiLoadEyesMouthTexToVram`.
  This is the counter-case to the name-the-temporary idioms: here the local is
  what is wrong, and CLAUDE.md's standing advice to repeat the whole indexed
  expression applies to arithmetic too.
* **`ori` against the target's `addiu` off the same base is combine merging an
  address into its consumer, not a way of spelling the address.** For a pointer
  local whose value gcc knows (`s32* p = (s32*)0x1F800000;`), `p + 4` reaches
  RTL as `(set (reg X) (plus (reg p) (const_int 16)))`; combine folds that into
  the argument move `(set (reg a0) (reg X))` and rewrites `PLUS` to `IOR`,
  because `reg_nonzero_bits[p]` proves the low bits clear. The rewrite is
  visible as `86 {iorsi3}` in the `.combine` dump against `3 {addsi3_internal}`
  in `.flow`. Two facts follow and both are load-bearing. `reg_nonzero_bits` is
  a **function-global union over every set of the pseudo**, computed in
  combine's own first pass, so a second identical assignment teaches it nothing
  and there is no per-block escape — which is why the same function emits `ori`
  in every block once it emits it in one. And the rewrite happens only when
  combine can *merge*: an address with **two** uses has no LOG_LINK to fold and
  survives as `addsi3` — a three-line probe with `f(s, s + 4, s + 8)` inside a
  goto loop and `s + 4` used again after it compiles `s + 8` to
  `ori $6,$16,0x20` and `s + 4` to `addu $17,$16,16`. So when the target has
  `addiu` where you have `ori`, stop respelling the address (`fold` normalises
  every form of it) and ask what gave that address a second use — and price the
  extra pseudo, which conflicts with the base across the loop and usually costs
  a callee-saved register. `FieldEntityWalkmechCross` in `src/field/field2.c`
  is parked on exactly these two rows.
* **`jump2` merging two identical return tails is the same kind of verdict one
  pass earlier, and it is equally out of reach.** `func_801D1C2C` in
  `src/menu/savemenu.c` is 2 instructions short because the post-reload
  `jump_optimize` deletes the success path's own copy of the return value and
  the `j` that skipped over the `end:` copy -- `--rtl=a` shows insn 162
  (`(set (reg/i:SI 2 v0) (zero_extend:SI (reg/v:HI 19 s3)))`) and jump_insn
  164 present at `.greg` and gone at `.jump2`, with `.greg` otherwise
  instruction-for-instruction the target. The two insns are byte-identical
  after reload in the target as well, so no spelling of *this* function can be
  what stops the merge there: twenty-two were measured (three loop shapes,
  five tail shapes, a second local at either exit with and without a cse
  barrier, three widths for the returned variable) and they produce exactly
  three distinct row counts. When a dump shows the target's shape reached and
  then destroyed, stop -- the remaining question is about a pass, not about C.
* **A branch polarity that survives every spelling of its condition belongs to
  reorg, and one `-da` run tells you so instead of a sweep.** Track the
  `jump_insn` across the dumps: `FieldEntityWalkmechCross`'s fast path is
  `(if_then_else (ge:SI ...) (label_ref <done>))` followed by `j <chain>` —
  the target's shape, instruction for instruction — in `.jump`, `.loop`,
  `.combine`, `.lreg`, `.greg`, `.jump2` **and `.sched2`**, and only `.dbr` has
  it as `(lt:SI ...) (label_ref <chain>)` with the following jump retargeted at
  a fresh label one insn into `done`. That is reorg inverting the pair so the
  unconditional jump lands somewhere it can steal a delay-slot insn from. When
  the dumps show the branch correct at `sched2`, **no C spelling of the
  condition can reach it** — the residue is reorg's cost decision, and the only
  lever left is what sits at the branch target. Note the corollary about pass 1:
  expand emits a `TRUTH_ANDIF` guard as N `lt` branches to the drop-through
  plus a `j` to the then-clause, and `jump_optimize`'s
  conditional-jump-around-an-unconditional-jump rule inverts the last one
  immediately — so every guard spelling funnels into one shape before anything
  interesting happens. Thirteen were measured byte-identical on that function.
* **`fold` collapses `A ? B : 0` to `A && B`, so a ternary is not a way to
  reach `do_jump`'s COND_EXPR case.** That case is the one place in `expr.c`
  that passes *both* `if_true_label` and `if_false_label` down to a nested
  `TRUTH_ANDIF`, which is the only front-end route to `bcc <success>` /
  `j <failure>` rather than the usual `bcc <failure>` / `j <success>`. It
  cannot be reached from C: `!(a && b)` is De Morgan'd by `build_unary_op`'s
  `invert_truthvalue` at parse time, and every ternary spelling is folded back
  to an `ANDIF`/`ORIF` before `do_jump` runs.
* **A global's address in a hoisted register needs `&` *and* `volatile`, and
  neither alone.** This file records `volatile` as one route to the register
  form and says a `volatile` cast at the access site "measures the same" as
  the declaration. On a global read inside a loop that is wrong in both
  halves. `*(volatile u16*)&D_8009C862` is 0 rows on `func_800E53C8` in
  `src/battle/battle3.c`; `extern volatile u16 D_8009C862` with a plain read
  is **6** and one instruction short, and `*(u16*)&D_8009C862` with a
  non-volatile declaration is **6** as well. The `&` is what gives the
  address its own pseudo, which `move_movables` then lifts into the loop
  *preheader* — after the zero-trip guard, which is where the target has it
  and where a source-level pointer local (assigned before the `for`) cannot
  land. The `volatile` is what stops cse folding that pseudo back into the
  `mem`. The tell is `lui`/`addiu` in the preheader with `lhu 0(reg)` in the
  body, against your `lui %hi` / `lhu %lo(reg)` pair inside the loop.
* **A scattered run of `D_` scalars that exactly covers a known struct's size
  is that struct, and typing it is worth more than any codegen lever.** m2c
  emits one `extern` per address it sees touched, so a function that
  initialises a record comes out as thirteen unrelated globals. `D_800F92E2`
  through `D_800F92F3` in `src/battle/battle3.c` are one `Unk80026448`: every
  field width and offset lines up, and `func_800DE46C` hands `&D_800F92E2`
  straight to `func_800264A8(Unk80026448*)`, which is the confirmation. The
  check is cheap — take the lowest and highest address m2c invented, subtract,
  and grep the headers for a struct of that size whose member widths match the
  store opcodes. It was worth a row on the initialiser itself and made two
  other functions in the same unit readable at all.
* **`p[i]` and `*(p + i)` are not the same address sum.** For a `u8*` local
  and an `int` index, the subscript comes out `addu v0,v0,s1` (index first)
  and the pointer-plus form `addu v0,s1,v0` (pointer first). One row, and the
  last one on `func_800E010C`. The companion for a *scaled* index is the
  reverse: `&row[i]` puts the pointer first and the integer sum
  `i * sizeof(T) + (s32)row` puts the index first — `func_800DE46C` needs
  that one. Read which register the target's `addu` names first and pick the
  spelling accordingly; no cast or parenthesisation of the *other* form
  reaches it.
* **A post-increment read into a second global is three statements, not
  `b = a++`.** `D_8009D2FC = D_80163604++;` computes the increment first and
  emits the two `sb` in the order D_80163604, D_8009D2FC; a plain
  `D_8009D2FC = D_80163604; D_80163604 = D_80163604 + 1;` reuses the old
  value's register and drops one instruction. What the target has —
  `addiu v1,v0,1` with `v0` still live — is `next = D_80163604 + 1;` as its
  own statement *before* the D_8009D2FC store, and `next` has to be `u8`: an
  `s32` local costs 3 rows. `func_800E05E4`'s last two rows.
* **A compound assignment is the cheapest way to give a value two more
  references, and two can be the whole diff.** `block_alloc` ranks quantities
  by `floor_log2(n_refs) * n_refs * size / (death - birth)`, and the
  `floor_log2` step between 3 and 4 references is worth a factor of two -- so
  a value with 2 references sits at the *bottom* of the tie group and the same
  value with 4 sits at the top, which on a short function is the difference
  between `$a3` and `$a0` and, with the other three quantities each shifting
  up one register, every row of the diff.
  `lo = D_800F4AC0[pc++]; return lo | (hi << 8);` gives `lo` two references
  (def, use) and 13 rows of pure caller-saved naming;
  `lo |= hi << 8; return lo;` gives it four (def, use, def, use) and
  `func_800B1368` in `src/battle/battle.c` **matches** -- same instructions,
  same length, nothing else changed. Six other spellings (the expression
  inline, two named locals, `u8`/`u32` for the first byte, a pointer local for
  the struct, an explicit `pc` local) all measure 13. The tell is that the
  rotation is *cyclic*: every quantity exactly one register up means one of
  them crossed a `floor_log2` step, not that the allocator did anything
  structural. This is the same lever as the `do { } while (0);` reference
  multiplier and much cheaper, because it emits no instruction and needs no
  basic-block boundary.
* **A call that passes fewer arguments than the callee takes needs the callee
  defined K&R, and the diagnostic is the only symptom.** These sources do it
  deliberately -- a wrapper that forwards whatever the caller left in
  `$a0`/`$a1` without naming it -- and against a *prototyped* definition gcc
  2.6.3 reports `too few arguments to function`, substitutes for the missing
  argument and keeps generating code, so ninja is happy, the overlay's SHA-1
  is right, and the only casualty is that `checkfn.py` refuses every verdict
  in the file. An old-style definition (`void f(a, b) s32 a; s32 b; { ... }`)
  creates no prototype, so the call is legal again, and for `int`-width
  parameters it is codegen-identical -- the default argument promotions are
  the identity on `int`, and `clang-format` leaves the form alone.
  `src/battle/battle.c` carried three of these plus one genuinely conflicting
  forward declaration, and between them they blocked every measurement in a
  109-function file.
* **Read the *preheader order* to tell a cached loop bound from one read in
  the test.** Both spellings compile to a single load and the difference
  shows up as one `move` in the wrong place, which reads as noise.
  `count = D_800F3948;` ahead of the loop is an ordinary statement, so expand
  emits it into the preheader *before* anything `move_movables` hoists. The
  bound written into the test instead -- `do { ... } while (i < D_800F3948);`
  behind an explicit `if (D_800F3948 > 0)` guard, so the value is already in a
  register -- is a redundant load that cse rewrites as
  `move <bound>,<guard>` **inside** the loop, and `move_movables` then hoists
  it in insn order, i.e. *after* the constants the loop body uses at its top.
  So a preheader reading `move a0,zero / li a3,-1 / move t0,v0` -- a hoisted
  constant sandwiched between the two counter inits -- says the bound is in
  the test, and `move t0,v0 / li a3,-1` says it is a local.
  `func_800A304C` in `src/battle/battle.c`.
  The guard is load-bearing in both directions: without it there is no earlier
  load for cse to copy from, and with *both* an explicit guard and a `for`
  loop the two zero-trip tests are not folded and you get a duplicate `blez`
  (+1 instruction). Guard plus `do`/`while` is the shape that gives one test
  and one copy.
* **Several identical `lw` of one symbol with no store between them is
  `volatile`, and it is worth far more than the loads.** cse shares a
  global's load across a whole extended basic block, so a target that reloads
  the same scalar three times in straight-line code cannot come from a plain
  `extern` -- a volatile MEM is never entered in cse's table at all. What it
  costs is not the three instructions but everything derived from them:
  `D_800F499C` in `src/battle/battle.c` indexes three tables, and with one
  shared load gcc computes `idx * 40` once and hands it to both bases where
  the target computes it per load. That was 28 rows and **-8 instructions**
  on `func_800B0170`; `extern volatile s32` matches outright. The check is
  cheap and does not need a diff -- count the symbol's `%hi`
  materialisations in the target against the number of times your source
  names it. And check for a second, non-volatile `extern` of the same symbol
  further down the `.c`: gcc 2.6.3 accepts the redeclaration silently, so a
  file-local `extern` next to an already-matching function will quietly
  decide which behaviour that function gets.
* **Wrong compiler** — check the `//!` header (see *Compiler selection*).

The `$at` rematerialisation wall this section used to call unsolved -- gcc
hoisting a global array's address where the original rebuilds it through the
assembler's `$at` macro at each use -- is the scaled-subscript bullet above.
`AddStrNextDebugRow`, `FieldDebugPageAddPos`, `FieldDebugPageAddSize`,
`FieldDebugPagesResetPosSize` and `FieldDebugInitBuffers` are all plain
matching C now. Every park note that names that wall is worth re-reading
against those three bullets before spending a budget on the function.

**Measure a group of near-clone functions together, not one at a time.** When
several parked functions print the same diagnostics, whichever ones are still
`MASPSX_OVERRIDE` supply those strings from their `.s` under `D_` symbols while
the unparked ones emit their own local copies — so every relocation names a
different label and `checkfn.py` cannot alias them. Each function then reads as
16–22 rows of what looks like register allocation and is nothing of the kind.
The four `FieldEvent*Memory*` accessors in `src/field/field4.c` measured 0, 16,
22 and 22 rows individually and **all four MATCH with no source change at all**
once unparked in the same build. Before spending a budget on any member of a
clone family — the `Opcode*` palette ops, the `FieldEvent*` accessors, the
`Kawai*` handlers — unpark the whole family and re-measure.

**A relocation multiset that names a symbol more often than your source stores
to it counts store *sites*, and it is a structural reading of the target that
costs one command.** The check `permuter_scratch.sh` runs to prove a scratch is
scoreable prints both objects' relocation counts, and each `%hi`/`%lo` pair is
one site -- so six against four is three sites against two, which is a fact
about cross-jumping and block layout rather than about registers.
`FieldEntityWalkmechCross` in `src/field/field2.c` names `D_80113F28` four
times against the target's six. Note what that did **not** settle there:
writing the two stores out in all three arms rather than sharing a `store:`
tail behind an `edge` variable was worth 10 rows and left the count at four,
so gcc still merges the identical `D_80113F28 = *triId;` suffix and the source
shape that defeats it is still unknown. Read the multiset early -- it is
cheap, and it says how many sites the original had -- but treat a count you
cannot move as an open question, not as a lever you have already pulled.

**Three unrelated values spelled into one local is the counter-merging idiom
run backwards, and it is worth more than any register lever.** CLAUDE.md's
standing advice is to *merge* the counters of loops that describe the same
walk. The converse costs just as much: `FieldEntityWalkmechCross`'s seed used
one `px` for `pos->vx`, for `pos->vy` and then for the mesh-space x the cross
products consume. Three unrelated live ranges in one pseudo take a `$t`
register rather than `$v0`, and the scheduler hoists a load a block early to
cover the resulting delay slot -- which reads as scheduling noise a long way
from the declaration. A separate local for the conversion was **29 rows** and
cost no instruction. When a diff shows a value in a callee- or long-lived
register where the target uses `$v0`, count how many different things the
source spells with that name.

#### Twelve ways a clean-looking diff lies

**A stale object.** `make report` rewrites `build.ninja` to build into
`report/build/`. After it has run, `ninja build/us/...` finds no such target,
prints `no work to do`, and leaves the *previous* object in place — so
`diff.py` compares code you did not write and reports a match. An
`INCLUDE_ASM` function trivially matches itself this way. Run `make build` to
restore the normal configuration after `make report`; `checkfn.py` refuses to
run when `build.ninja` is in the report configuration, and also fails if the
object ends up older than the source.

**Neighbouring functions.** `diff.py -o <fn>` renders a window that continues
past the end of the function, so rows belonging to the *next* function appear
under the name you asked for. Scope by the target `.s`'s instruction count, as
`checkfn.py` does, before concluding anything.

**A function name two overlays both define.** `diff.py` with no `-f`/`-F`
picks the objects through `diff_settings.py`, which greps every
`build/us/*.map` for the function name — and when **more than one** overlay
defines it, `estimate_overlay_from_func_name` returns `None` and the caller
silently falls back to `main`. The auto-generated names collide by
construction, because several overlays share a load address: `func_800A0000`
is defined by both `brom` and `dschange`, both at `0x800A0000`, and `field`,
`ending` and `dschange` all start there. What comes back is a diff of a
function `main` does not have — no rows, score 0, which reads as a flawless
match. `checkfn.py` now passes `-f`/`-F` outright, the way `variant_eval.py`
already did, so no map is consulted and there is nothing to guess; before
that its instruction-count guard caught this one only by luck, because `main`
happens to have no function of that name. If you call `diff.py` by hand on an
overlay, pass the objects.

**A compile error that ninja calls a success.** The compile line is a pipeline
ending in `mipsel-linux-gnu-as`, so the shell's exit status is the
*assembler's*. gcc 2.6.3 reports an error such as

```
src/field/field.c:452: `D_8009AC26' undeclared (first use this function)
```

on stderr, substitutes 0 for the unknown value, and keeps generating code. The
assembler is perfectly happy with that code, ninja prints no failure, and you
are diffing a function with a whole `if` constant-folded out of it. Nothing in
the build shouts. `checkfn.py` now scans the compile output for non-warning
diagnostics and refuses to give a verdict, caching them beside the object so a
later `no work to do` run cannot look clean.

That check has to be scoped to the file under test, and for a long time it was
not. `ninja <obj>` builds everything the object's edge depends on, and for
`src/battle/batini.c` that is the *whole battle overlay* — the link needs
`config/sym_export_battle.us.txt`, which is generated from `battle.elf`. So the
captured output carries `battle.c`'s and `battle2.c`'s diagnostics, both of
which have several standing ones, and `checkfn.py` refused every verdict in
`batini.c` while quoting errors from files the caller was not editing. It now
splits the output on ninja's `[N/M] psx cc <source>` edge lines and keeps only
the section for the source it was asked about. The reason to fix it rather than
work around it is the second-order one: an alarm that fires on somebody else's
file is an alarm you learn to read past, and then it cannot tell you the day
your own file starts folding an `if` away.

The usual way to trip this is a symbol declared only inside a
`#else /* NON_MATCHINGS */` block — `PreloadNextFieldMap`'s externs near the top
of `src/field/field.c` are not compiled in the matching build. Declare the
symbol in the real extern block instead.

The mirror-image trap, moving a parked body between the field units: an
`extern` whose type comes from a `typedef struct` declared just above it has to
travel with that typedef. Left behind, gcc 2.6.3 does not stop at the unknown
type name — it makes the object a tentative definition in `.bss`, and the only
complaint is a `multiple definition` at link time against the real symbol in
the data segment.

**A function that is still pinned.** `MASPSX_OVERRIDE` assembles the reference
`.s` into the object, so the C beside it is never compiled and the comparison
is the target against itself: **MATCH, whatever the `#else` body says.** This
is not a hypothetical — it cost a red `make build` in the session that wrote
this paragraph. The parked body had been rewritten from scratch, `checkfn.py`
said MATCH, and the function it was unparked into turned out to be one
instruction longer than the target, with `slt` where the target has `sltu`;
every symbol in the overlay after it shifted by four bytes and only the SHA-1
noticed. Measuring a parked body means unparking it first — `checkfn.py` now
refuses to give a verdict while the name still appears in a `MASPSX_OVERRIDE`
in that `.c`, with the same multiline-aware match the rename check uses.

**And a park can be spelled the other way, which defeats every tool at once.**
`#ifndef NON_MATCHINGS / INCLUDE_ASM(...) / #else / <body> / #endif` pins the
bytes exactly as `MASPSX_OVERRIDE` does, but nothing named `MASPSX_OVERRIDE`
appears — so `checkfn.py` compared the target to itself and printed **MATCH
with exit 0**, `parked_queue.py` reported "no `MASPSX_OVERRIDE` bodies found",
`variant_eval.py`'s `unpark()` scored the pinned build, and `worklist.py`
counted the function as *unstarted work*. A near-miss body with a long park
note is invisible to the whole toolchain and reads as a match to the one tool
anybody runs. **Thirteen functions across `src/` were parked this way** — one
in `src/magic/escape.c`, two in `src/main/1255C.c`, seven in `src/main/18B8.c`,
one in `src/menu/cnfgmenu.c`, two in `src/menu/savemenu.c` — and the
`cnfgmenu` one had an exhaustive note recording roughly 1700 measured variants
and 1.5M permuter iterations that no tool could see.

Both halves are fixed. `checkfn.py`'s guard now also matches an `INCLUDE_ASM`
inside a `#ifndef NON_MATCHINGS` arm that has an `#else`, and refuses with the
same message; verified both ways, since a guard that over-triggers would
refuse every real match. And all thirteen are converted to `MASPSX_OVERRIDE`,
which is byte-neutral — the build stays green because both macros assemble the
same `.s`. **Spell a park `MASPSX_OVERRIDE` and nothing else.**

`unpark.py` refuses on this spelling too, so a body parked that way cannot even
be measured by hand without editing the source first. The check that costs
nothing, before believing any file's park count: `grep -n 'INCLUDE_ASM' <file>`
and look at what surrounds each hit — a genuinely unfinished function has no
`#ifndef NON_MATCHINGS` above it and no `#else` body below.

**And the pin macro is not always the guard's first line.** A park whose body
needs a forward declaration — an opcode-style helper defined further down the
unit, say — or that carries a one-line `// why` comment puts those *inside*
the guard, above the macro. `parked_queue.py`'s anchor was `#ifndef
NON_MATCHINGS\s*\n\s*(MASPSX_OVERRIDE|INCLUDE_ASM)\(`, which those two lines
break, so such a function is simply absent from the queue — not reported as
failed, not reported at all. Three of `src/main/18B8.c`'s seven parks are
spelled that way, and the queue read "4 parked bodies" against a `grep` that
finds seven; two of the three turned out to be five and eight rows out.
`checkfn.py`'s guard was already tolerant (it matches `#ifndef …(.*?)#else`),
which is what makes the discrepancy invisible — the one tool that refuses is
not the one that enumerates. Both now skip any line that is not itself a
preprocessor branch. **Cross-check a park count against `grep -c 'MASPSX_OVERRIDE'`
before believing a queue is empty.**

**A diff that stops early.** `diff.py --max-lines` defaults to **1024**, and it
truncates silently: on a longer function the tail simply never enters the
comparison, so it can differ freely while the visible rows look perfect.
`func_801D080C` is 1205 instructions, and its last ~150 — including a whole
loop that did not match — were invisible for as long as nobody passed the flag.
`checkfn.py` now sizes `--max-lines` from the target's instruction count and
refuses to give a verdict if it gets back fewer instructions than the `.s`
declares. Passing the flag by hand has a trap of its own: it must come *after*
the function name, because `diff.py -o --max-lines 2000 <fn>` binds `<fn>` to
the end-address positional and returns an empty diff with score 0 — which reads
as a flawless match.

**A shifted string literal.** Two spellings of a conditional debug name can
compile to identical instructions and still put the function's string literals
in `.rodata` in different orders — see the STRING_CST bullet above. Every
`%lo(<string>)` in the function then names a slot 8 bytes off, `checkfn.py`
reports it as a couple of rows of register naming, and the function reads as
one small step from done. It is not: `make build` fails the overlay's SHA-1,
because every later `.rodata` offset in the unit moved. When the only rows left
are `%lo` of a string, check the `.s`'s `glabel` order before anything else.
`OpcodeFuncTurnr` cost a full red build to learn this.

**A file-scope constant that is really the parked function's own blob.** A
function with a local aggregate initialiser emits that aggregate into the
unit's `.rodata`; while the function is pinned it emits nothing, so the same
bytes have to exist as a named object for the `.s` to reference. `field.c`
carries `const u32 D_800A0000[] = {0, 0x01D801E0};` for exactly that reason —
it is `FieldMain`'s `RECT clip = {0, 0, 480, 472}`. Unpark the function
without deleting the object and the unit emits the RECT **twice**:
`jtbl_800A0008` moves from `.rodata+0x8` to `+0x10`, every jump-table entry and
every branch target after it reads wrong, and `checkfn.py` renders the whole
thing as ordinary rows — 8 of `FieldMain`'s 84. `make build` fails the
overlay's SHA-1 and nothing points at the object. Before unparking any
function with a local aggregate initialiser, grep the unit's file-scope
`const` objects for the same bytes; `od -A d -t x4` on the initialiser is the
check. The same applies in reverse: re-parking one means putting the object
back.

**A parked body that reads an undefined register.** gcc 2.6.3 accepts a
statement that no path reaches, `jump_optimize` deletes the block, and a local
whose only assignment was in it keeps its *use* and loses its *def* — so
`global_alloc` gives the undefined value a hard register and the function
compiles, links and scores. It can score *better* than the correct program,
because an allocno whose live range runs back to the function entry perturbs
the whole conflict graph: `AddBackgroundToRender` read `0($s3)` with no write
to `$s3` anywhere above it and measured **65 rows against the correct body's
72**, and that 65 was carried in the park note as a lever and re-measured
against for three sessions. The shape to distrust is a statement between the
close of an infinite loop and a label the loop only reaches by `goto`:

```c
    for (;;) { ... goto layer3; ... }      /* no break, never falls through */

    layer3Slot = &D_8009ACA2.layer3;       /* unreachable — silently dropped */
layer3:
```

The check is two commands, and it belongs in the routine for every parked body
— not only for a diff that shows *your* build using a callee-saved register
where the target uses a temp, though that is the tell:

```shell
sh tools/variant_disasm.sh .variants/<spec>.json /tmp/f.dis
.venv/bin/python3 tools/uninit_regs.py /tmp/f.dis <Func> [<Func>...]
```

A read with no write above it is the answer, and no amount of codegen
reasoning will find it, because the residue is not codegen. Note the trap
inside the checker itself, which reported the known-bad body as clean twice
while it was being written: a callee-saved prologue `sw s3,0x4c(sp)` reads the
*caller's* value and defines nothing, and a `jal` defines `$v0`/`$v1` while
naming no register at all. Either one mishandled turns the check into a
constant answer.

**A row count read before the functions *above* it are settled.** Every branch
in a function is a PC-relative offset into its own `.text`, and `diff.py`
renders those as absolute offsets in the object — so one neighbour of the wrong
length shifts every branch row in every function after it. This is the file-
order rule stated as a measurement error rather than as advice, and on a fresh
unit it is the dominant one: parking three near-misses in `src/battle/battle3.c`
took `func_800E05E4` from **17 rows to 2**, `func_800DDE90` from 11 to **0**,
`func_800DE94C` from 20 to 4 and `func_800E010C` from 6 to 1, with no change to
any of their bodies. Two habits follow. Park or fix upstream *before*
measuring downstream — a `MASPSX_OVERRIDE` is exactly the target's length, so
pinning restores every offset below it. And when a diff is nothing but branch
rows whose two addresses differ by a constant, do not read it as a finding:
subtract the two, divide by four, and check that against the instruction
surplus of the functions above.

**A red `make build` that belongs to another agent's worktree.** The symptom
is a full build that fails once with a missing object
(`cannot find build/us/src/main/psxsdk.c.o`), a SHA-1 mismatch on
`main_final.elf`, or a `multiple definition` naming a unit you have never
touched (`src/menu/itemmenu.c`), then passes on an immediate re-run with no
source change. It reads exactly like a filesystem flake and it is not: it is
two worktrees sharing one Docker build volume, which is the corruption this
file's *Working in parallel* section warns about, arriving through the one
route nobody checks. `tools/docker-build.ps1` derives the volume with

```powershell
elseif ($repoRoot -match '[\\/]\.claude[\\/]worktrees[\\/]([^\\/]+)') {
```

and the character class **must** be `[\\/]`, not `[\/]` — in .NET regex `[\/]`
is just `[/]` and never matches a Windows path separator, so the match fails
silently and every worktree falls through to the shared `ff7_build`. The
script is gitignored, so each worktree carries its own copy and each one has
to be checked. Two commands prove it:

```powershell
docker volume ls | Select-String 'ff7_build'
.\tools\docker-build.ps1 'echo x'   # then look for ff7_build_<name> in the list
```

Everything measured on a shared volume is suspect — `checkfn.py`,
`insn_histogram.py` and `make build` all read `build/`. `variant_eval.py`
compiles to a private temp directory and is the one tool that is not exposed.
After fixing the regex, re-run `make build` (it starts from an empty volume
and rebuilds everything, which is expected) and re-run `checkfn.py` over every
function landed on the old volume before believing any of them.

#### Sweeping many variants at once

`checkfn.py` builds through ninja into `build/us/...`, a single shared path, so
it cannot be run concurrently: two agents editing the same `.c` overwrite each
other's object and read each other's verdict. The same collision bites a single
agent: a decomp-permuter **import** unparks the target into the live `.c`,
runs ninja, and only then restores it, so an import running in the background
and a variant sweep in the foreground will clobber each other's copy of the
source. The failure is silent in both directions — the import writes a scratch
built from someone else's variant, or dies with `Function <name> not found in
base.c`, and the sweep scores a body it did not write. Do the imports first,
wait for them, then sweep. When you want to sweep dozens of
phrasings of one function — or run several searches in parallel —
use `tools/variant_eval.py` instead:

```shell
cp src/menu/cnfgmenu.c .variants/_base.c
sha256sum .variants/_base.c | cut -d' ' -f1 > .variants/_base.sha256
.venv/bin/python3 tools/variant_eval.py .variants/my-idea.json --rows
```

It compiles a variant to a private temp object and diffs it with
`diff.py -f/-F`, which takes explicit object paths and so never consults the
map file or the build directory — nothing is shared but read-only inputs. A run
takes about three seconds against roughly forty for a ninja round trip, and the
verdict is the one `checkfn.py` would give (same alias discounting, same
scoping). A variant is described as *edits against a pinned base*, and each
`old` string must match exactly once or the run aborts, so a typo cannot
silently score as "no change".

**A unit with a standing diagnostic cannot be measured at all, so clear those
before picking a function.** `variant_eval.py` and `checkfn.py` treat any
non-warning line from cc1 in the file under test as fatal — rightly, since
gcc 2.6.3 substitutes 0 and carries on — and the check is per *file*, not per
function. So two long-standing `conflicting types` errors in
`src/battle/battle2.c` refused **every** verdict in a 60-function unit, and
the first six variants of a session came back `FAILED` with diagnostics about
declarations nobody had touched. This is the same second-order cost as the
`batini.c` scoping bug one section down: an alarm that always fires is an
alarm that stops meaning anything, and here it meant nothing could be scored.

Clearing one is cheap and provable. Fix the declaration, then score a variant
of a function in the same unit that **already matches** — it must come back
`0 changed, 0 inserted`, which is proof the fix is byte-neutral. Both of
battle2.c's were: a definition whose fourth parameter was `s32` where the
header said `void (*)(int)` (same mode, same code), and a local redeclaration
of `SystemAkaoExecute` with four arguments against `game.h`'s `(void)` — and
those four arguments turned out to be dead, since they are already live in
`$a0..$a3` where the call sits. The check to run before a batch:

```shell
ninja build/us/src/<ovl>/<unit>.c.o 2>&1 | grep ':[0-9]*:' | grep -v warning:
```

That "exactly once" is over the **whole unit**, not the function, and these
units are full of near-clones: `    s32 off;` occurs ten times in `field5.c`,
and `OpcodeFuncMove`'s model-entry lookup statement occurs four times in
`field4.c` — two of them inside already-matching C, where an edit would be a
silent regression rather than an abort. Anchor on something the function alone
contains: its signature, a whole block, or a neighbouring comment. A bare
declaration is never a safe anchor here.

Two flags, both hardened after they cost real time. `--jobs` takes either
`--jobs 8` or `--jobs=8` — only the second parsed for a long while, and this
file and the tool's own usage line both showed the first, so the documented
invocation read `8` as a spec path, ran single-threaded, and reported an extra
`FAILED` tag that looked like a broken variant. And any unrecognised `-flag`
is now a hard error instead of being appended to the spec list, which is the
general form of that failure: a mistyped option must not come back as a
plausible-looking result row.

**Name the spec files in one case.** `.variants/` sits on the repo's working
tree, which on Windows is case-insensitive, so a sweep that tags its variants
`fms_d` and `fms_D` writes *one* file and scores that lever twice — the
summary shows the same tag twice with the same number and reads as a
consistent measurement rather than as a lost one. A 20-variant cross-product
generated with one letter per lever hit this on the first run of the session
that wrote this paragraph; the shell glob passed 18 paths and 16 distinct
variants were scored. Use distinct lowercase tags, or assert the file count
matches the variant count before believing a sweep.

The spec names its own source and function, so one tool serves the whole
repo, and several specs given at once are scored concurrently:

```shell
.venv/bin/python3 tools/variant_eval.py --pin src/field/field4.c
.venv/bin/python3 tools/variant_eval.py .variants/*.json --jobs=8 --rows
```

The compiler, assembler and jump-table flags are read out of `build.ninja`'s
edge for that object rather than duplicated in the tool -- the `//!` header's
meaning lives in `tools/ninja/gen.py`, and a second parser would drift the
first time someone adds a PSYQ version, with a wrong verdict rather than an
error as the failure.

**A park is the `#ifndef NON_MATCHINGS` guard, not the `MASPSX_OVERRIDE`.**
Both pin macros put the target's `.s` in the object and compile the C beside
them out, and `src/menu/*.c` parks every near-miss with a bare `INCLUDE_ASM`
inside that guard rather than with `MASPSX_OVERRIDE`. `unpark.py`,
`variant_eval.py` and `parked_queue.py` used to recognise only the second, so
`parked_queue` reported "no bodies found" on a file holding two of them and
`variant_eval` scored the *pinned* `.s` against itself -- a silent MATCH for
any edit. All three now anchor on the guard, which is also what keeps an
ordinary file-scope `INCLUDE_ASM` (a function with no C body, i.e. work still
to do rather than a near-miss) from being read as a park.

**It unparks only the function under test, and that is load-bearing.**
Compiling the unit with `-DNON_MATCHINGS`, which is the obvious way to select
the C bodies, replaces *every* parked function's pinned `.s` with its C body
-- and those bodies are the wrong length. Everything after the first one then
sits at the wrong offset, so a byte-perfect function reads as a wall of
branch-target rows off by a constant: `KawaiLightingApplyToPolyColor` scored
22 that way while `checkfn.py` said MATCH, all of them branch targets 4 bytes
low, because the parked `KawaiSetVertexColorFromLighting` above it is one
instruction longer as C than as asm. The check that the setup is honest is a
no-edit spec on a function that already matches: it must score exactly 0.

The reference is `expected/build/us/<source>.o`, which holds the *target*
bytes, so the verdict agrees with `checkfn.py` to the row.

### 3b. Check .rodata ownership before writing the C

Some functions cannot be decompiled alone, and the failure shows up as a red
`make build` while every function still diffs perfectly. Check first:

```shell
.venv/bin/python3 tools/rodata_owner.py src/field/field.c OpcodeFuncMenu2
# BORROWS  OpcodeFuncMenu2 -> D_800A0F38, owned by OpcodeFuncMenu (still INCLUDE_ASM)
```

* **BORROWS** — the function prints a string that another `.s` owns. Writing the
  literal makes gcc emit a *second* copy, shifting every later `.rodata` offset
  and breaking the overlay. **Pass the borrowed string by name instead**:
  `extern char D_800A02B8[];` and `f(g_DebugMessageBuffer, D_800A02B8)` emits no
  literal at all and resolves against the owner's still-assembled `.s`, so the
  function lands on its own. Turn it back into `"/"` when the owner becomes C —
  gcc folds identical literals within one translation unit, so the two then
  share the one definition. `FieldEventRequest` in `src/field/field4.c` matched
  on exactly this, and it had been carrying a "depends on decomp of
  DebugUpdateActor" comment for as long as anyone had looked at it.
* **LENDS** — the function owns a label other `.s` files still reference.
  Decompiling it alone deletes the definition and the link fails with an
  undefined reference. `IfCheck` owns the `"ope err="` that both `If2Check*` use,
  so those three are one unit.
* **SHARES** — the owner is already C, so the two identical literals fold into
  one. Fine, as long as you pass exactly the same string.

Pass `--all` to triage a whole file at once.

**Most of the "blocked" column is not blocked — it is pairs.** A BORROWS and
a LENDS that name only *each other* are one unit and are actionable the moment
you take them together, which `worklist.py` files under "Blocked — decompile
the whole `.rodata` group or skip" without saying so. All six of
`src/field/field4.c`'s blocked functions were three such pairs
(`OpcodeFuncRtpal`/`Rtpal2`, `Adpal`/`Adpal2`, `Mppal`/`Mppal2`), each ~130
instructions and near-clones of the other, which made them the cheapest work
in the file rather than the most expensive. Scan the blocked table for
reciprocal pairs before believing the "actionable" count.

**A `SAFE` verdict is unreliable for the same reason, in the other
direction.** The tool cannot see that a *pinned* sibling still assembles its
`.s`, so it will not tell you the function you are about to land is the one
that **owns** a literal those siblings print. `FieldEventReadMemoryU8` and
`OpcodeFuncTutor` both report SAFE, both match instruction-for-instruction, and
both break the link the moment they land — the first because its three
`FieldEvent*Memory*` siblings reference `D_800A032C`, the second because
`src/field/field5.c` reaches `D_800A08D0` by symbol. A C string literal becomes
a *local* label, so the definition simply disappears. Before landing a function
that prints anything, grep `asm/` for the `glabel` of every `.rodata` symbol its
own `.s` defines and check who else names it:

```shell
grep -rln "glabel D_800A032C" asm/us/field/
grep -rln "D_800A032C" asm/us/field/ src/field/
```

**A LENDS function lands on its own: define the borrowed symbol in
`config/sym_extern.us.txt`.** That file is handed to `ld` as a linker script
(`-T`, every overlay, see `tools/ninja/gen.py`), so a line there defines an
absolute symbol. Write the C literal as a literal, let it become the local
label it wants to be, and give the *other* unit's reference an address:

```
D_800A0848 = 0x800A0848;
D_800A08D0 = 0x800A08D0;
```

Nothing else changes — the borrowing unit's source still says `D_800A0848`, so
its relocation resolves exactly as before and its codegen is untouched. The
bytes are identical because the pool still emits the same strings at the same
offsets, and `make build`'s SHA-1 is what proves it. `OpcodeFuncMjump` and
`OpcodeFuncTutor` in `src/field/field4.c` had both been *verified matches*
parked for this reason alone, and both landed this way with no source change to
their bodies.

Two constraints on that file. It is read by **both** splat and `ld`, and they
disagree about comments: splat asserts every non-blank line holds exactly one
semicolon (so `/* … */` fails), and `ld` does not understand `//` (so that
fails too). Keep it to bare assignments and put the explanation in the source.
And delete the line if the function is ever re-parked — the `.s` would define
the same symbol again and the link would see it twice.

The escape that does *not* work, and it was measured: giving the string a named
definition, `const char D_800A0848[] = "evt cmd=";`, produces a function that
matches and an overlay that does not. gcc emits a named object at its
*declaration point* but a string literal into the function's own constant pool
just before the function body, so the named string lands ahead of the
neighbouring literals rather than among them; and a `char[]` object carries the
array's alignment (1) where the pool uses `.align 2`, so it lands at the wrong
offset even in the right order.

**`rodata_owner.py` reads `MASPSX_OVERRIDE` as if it were C, and it is not.**
The tool decides a symbol is owned by C once it sees a body in the `.c`, so a
function parked under `#else /* NON_MATCHINGS */` reports **SHARES** — "fine,
as long as you pass exactly the same string". It is not fine: the compiled
form is the `MASPSX_OVERRIDE`, the `.s` is still `.include`d, and the `.s`
carries its own copy of the literal in `.rodata`. Writing the sibling as C
then emits the string a second time and every later offset shifts. This costs
a full `make build` to discover, because every function still diffs perfectly
and only the overlay's SHA-1 fails — `OpcodeFuncAdpal2` matches instruction
for instruction and still produced `build/us/field.exe: FAILED`. A pinned
function is *not* an owner; treat SHARES as BORROWS whenever the named owner
sits under a `MASPSX_OVERRIDE`.

#### An overlay's `.bss` is `extern`, never a C definition

An overlay whose `.bss` lies past the end of its disk file has no subsegment
for it in `config/us.yaml` — there is nothing to disassemble — and splat's
`create_undefined_syms_auto` writes `build/us/undefined_syms.<ovl>.txt`
instead, one bare `D_800A06CC = 0x800A06CC;` per address the *disassembly*
references. Two consequences, and both were needed to land `func_800A0000` in
`src/brom/brom.c`:

* **Declare them, do not define them.** `extern DRAWENV D_800A06CC;` resolves
  against that generated script and emits the same `%hi`/`%lo` the `.s` had.
  A real C object would land in `<ovl>.c.o(.bss)`, which the linker script
  places *inside* the overlay's own section — so `SIZEOF(.<ovl>)` grows, the
  emitted binary is longer than the retail file, and the SHA-1 fails for a
  reason nothing points at. (`dschange` is the other case: its zero region is
  *inside* the file, so it has a real `bss` subsegment and a real `.s`.)
* **splat keeps emitting them after the last referencing function becomes
  C.** It disassembles the whole segment regardless of which functions the
  `.c` still holds as `INCLUDE_ASM`, so the auto-generated definitions do not
  disappear underneath you. This is the thing worth checking rather than
  assuming — `undefined_syms.brom.txt` still lists all fourteen with every
  function in the unit written out as C.

An interior address (`D_800A0739` for `DISPENV.isrgb24`) needs no declaration
of its own: `D_800A0728.isrgb24 = 0;` relocates against `D_800A0728+0x11`,
which `checkfn.py` discounts against the target's `D_800A0739` as an alias
and which links to the same byte.

#### Jump table alignment, and the file splits it implies

gcc emits `.rdata` / `.align 3` before every jump table. GNU `as` honours that
**relative to the start of the object's `.rodata` section**, so the table lands
on an 8-byte boundary counted from offset 0. The original aligned on the
address the table would actually have. The two agree only when the section base
is itself 8-byte aligned — and four units start 4 bytes off:

| unit | `.rodata` base | | unit | `.rodata` base |
| --- | --- | --- | --- | --- |
| `battle1` | `0x800A05DC` | | `18B8` | `0x8001029C` |
| `battle3` | `0x800A0DD4` | | `savemenu` | `0x801D017C` |

`tools/psx_jtbl_align.py` handles those: `tools/ninja/gen.py` reads each
`.rodata` subsegment's offset out of the splat config and passes `--phase 4`
when it is odd, which demotes a jump table's `.align 3` to `.align 2` so the
table keeps its natural offset — 8-byte aligned once measured from a base that
is itself 4 mod 8. Only jump tables are touched; `.align 3` before a `double`
is left alone. This is what makes `func_800E5FB4` in `src/battle/battle3.c`
match.

**The residue was a file-split problem, not an alignment one, and the split has
happened.** The original build compiled many small `.c` files, each with its own
object and its own `.rodata` base, and splat merged them into one unit.
`src/field/field.c` was several original files glued together, so its `.rodata`
carried *both* phases at once: `jtbl_800A052C` is 4 mod 8 while the tables of
already-matched functions are 0 mod 8, and one `--phase` setting cannot satisfy
both.

It is now five units — `field.c`, `field2.c` … `field5.c`, sharing
`src/field/field_private.h` — with five `c` and five `.rodata` subsegments in
`config/us.yaml`. **The cut is on jump-table phase, not on the original module
boundaries**: the bases alternate 0/4/0/4/0 mod 8 so that every table sits in a
unit congruent to it, and `tools/ninja/gen.py` pins the two ambiguous ones
through `JTBL_PHASE_OVERRIDE`. Do not merge the units back together, and do not
"tidy" a boundary — moving one function across a seam can move a table into a
unit of the wrong phase, and the failure is a red `make build` with every
instruction still diffing perfectly.

**Each unit's `INCLUDE_ASM`/`MASPSX_OVERRIDE` path string has to name its own
unit.** splat writes `asm/us/field/nonmatchings/<unit>/` from the `c`
subsegment name, and `tools/checkfn.py` derives the same path from the `.c`
stem — but the macro's first argument is a free string, so a unit left saying
`.../field` after a split assembles from a directory splat no longer
maintains. Nothing fails at the time: the old copies are byte-identical the
day the split happens, and drift only once a `mako.sh symbols add` or a splat
change rewrites the live ones. `KawaiExecute` and `OpcodeFuncSpcal` had
already drifted by an `.align 3` before anyone looked, with `checkfn.py`
reading the fresh `field4/` copy while the build assembled the stale `field/`
one. Splitting a unit therefore has a third step after the config and the
source: repoint every path string, and **move** the `MASPSX_OVERRIDE` `.s`
files into the new directory by hand — splat never regenerates those, so they
do not appear there on their own.

Nine of the eleven functions this blocked are now plain matching C: `IfCheck`,
`If2CheckSigned`, `If2CheckUnsigned`, `OpcodeFuncSetx`, `OpcodeFuncGetx`,
`OpcodeFuncSrchx`, `OpcodeFuncFadew` and `FieldEventRequestRun`. The two that
remain — `OpcodeFuncFade` and `FieldEventWriteMemoryU8` — are stuck on
ordinary codegen, not on alignment; treat them as normal work.

The seam comments the old single file carried (`// Begin of
field_event_memory_bank.c`) name the *original* translation units, of which
there were 33, not 5. They are still a useful map of what belongs together, and
the real boundaries are recoverable without a build: every `.rodata` symbol's
address orders identically to its owning function's `.text` address, so
scanning the overlay for `lui`/`%lo` pairs into `0x800A0000..0x800A1368` and
bucketing by owner reproduces the per-unit rodata ranges exactly. If a future
change needs a finer split, derive it that way rather than by guessing.

The tell is `tools/checkfn.py` reporting a `.rodata` offset rather than an
instruction — `want: .rodata+0x294 / got: .rodata+0x298`. Every instruction can
be byte-perfect and the function still fails the link check.

#### `%gp_rel` in a target `.s` means a function this unit cannot compile

`-G` is the second per-original-translation-unit setting that survives into the
merged units, and unlike jump-table phase there is no `--phase` knob for it.
The original build put small objects in `.sdata`/`.sbss` and addressed them off
`$gp`; splat renders those as `%gp_rel(SYM)($gp)` — one instruction where a
`-G0` build emits `lui`/`lo`, so a function that touches one can never match.

The window is narrow and easy to spot: `main`'s `gp_value` is `0x80062D44`
(`config/us.yaml`), so every symbol in roughly `0x80062D44 + 0x200` is a
candidate and everything outside it uses `%hi`/`%lo` **in the same function**.
That mixture is the giveaway that it is an addressing mode and not a symbol
property — `D_80062FD8` is `%gp_rel` in `func_800294BC` and `%hi`/`%lo` in
`func_8002C884`, because the two came from different original files.

Whether maspsx emits the `$gp` form is decided by
`sdata_entries`/`sbss_entries`, which it populates from `.sdata`/`.sbss`
sections **in the compiler's own output** — i.e. only for objects the unit
*defines*. Every one of these globals is an `extern` here (they live in main's
`.bss` `.s`), so no `G=` on the `//!` line can reach them; the only route would
be to move the data's definition into the `.c`, which relocates the whole
small-data window and is not worth it for a handful of functions.

Measure before believing it — one `variant_eval` run is enough, and it is
unambiguous: `func_800293D0` in `src/main/akao.c` compiles to the target
instruction for instruction and scores **+1 instruction, 1 changed row**,
`sh zero,%gp_rel(D_80062E08)(gp)` against `lui at` + `sh zero,%lo(...)(at)`.
Seven functions at the top of `akao.c` (`func_800293D0`, `func_800293F4`,
`func_800294A4`, `func_800294BC`, `func_8002988C`, `func_80029998`,
`func_800299C8`) are blocked this way and none of them is worth a budget;
`grep -l gp_rel asm/us/<ovl>/nonmatchings/<unit>/*.s` lists them in one
command, and that grep belongs in the triage before `worklist.py`'s ordering,
not after three attempts on one of them.

**Correction, from the same session: the route the paragraph above dismisses
is the one that works, and it has already landed 23 functions.**
`src/main/1255C.c` carries `G=8` on its `//!` line *and* spells nine of its
globals as tentative definitions (`s32 D_80062DF8;`) rather than `extern`s —
which is exactly what puts them in the compiler's own `.sdata`/`.sbss` and so
into maspsx's `sdata_entries`. Its object emits **269** gp-relative accesses
against the 37 target `.s` files that want them, and 37 of that unit's 52
remaining functions went from unmatchable to ordinary work. The other half of
the fix is in `tools/ninja/gen.py`, which now passes `-G<n>` to **maspsx** as
well as cc1; cc1 does not choose the addressing form, it emits `lw $2,<sym>`
and leaves the expansion to the assembler, and maspsx's `sdata_limit`
defaulted to 0.

So the cost is not "relocating the whole small-data window" — it is declaring
the handful of globals that unit *owns*, and `make build`'s SHA-1 is the
arbiter. What is genuinely true is the narrower claim: an `extern` can never
get the `$gp` form, so the definition has to move into the `.c`.

For `akao.c` specifically the change is bounded, because the affected
functions are one original translation unit rather than a scatter. The seven
`%gp_rel` functions run `0x800293D0` to `0x800299C8` starting at the very
first function in the file, and the two that sit inside that range with no
`%gp_rel` at all — `func_800297A4` and `func_80029818`, both parked at 3 rows
on a `QTY_CMP_PRI` counter — are in the same block and were compiled with the
same `-G`, which is a live hypothesis for their residue too. `0x80029C48`
onward is a different original unit and must keep `-G0`. That is the same
shape as the `field.c` five-way split: one merged unit carrying two
incompatible per-unit settings, where the fix is a split at the boundary the
addressing mode reveals, not a header change applied to the whole file.

The general rule: **`grep -l gp_rel` first, then check whether the hits
cluster.** A contiguous run means an original translation unit and a bounded
fix; a scatter through the file means something else is going on and the
paragraph above applies.

### 4. Last-mile: decomp-permuter

If step 3 stalls — zero diff rows are out of reach by hand, or you're stuck
permuting variable declarations/expression order yourself — hand the function
to [decomp-permuter](https://github.com/simonlindholm/decomp-permuter), an
external tool that brute-forces AST-level permutations of a function until the
compiled output matches. It is a search tool, not a substitute for
understanding: only reach for it once the C is *semantically* correct and the
remaining diff looks like compiler-specific register/ordering noise.

**Never start a run without writing a `PERM_*` macro set for that function
first.** Left to itself the permuter *randomizes*: an unbounded walk with no
memory of what it has tried and no termination. Given macros it *enumerates* a
finite candidate set, tries each once, and stops. The yield difference on this
project is not marginal:

| function | candidates | outcome |
| --- | --- | --- |
| `FieldBattleCheck` | 3,150 | **match** |
| `LoadLocalFieldModelAndInitAll` | 98,144 | 5 rows |
| `OpcodeFuncMove` | ~120,000 | a candidate measuring **+43 instructions** |
| `OpcodeFuncVwoft` | 101,000 | nothing |
| `FieldModelStructInit` | 87,000 | nothing |

Write the macro against the rows the diff actually shows.
`tools/permuter_macros.py recipes` indexes the catalogue by penalty type
(`temp-hop`, `cast-width`, `addr-form`, `decl-order`, `stmt-order`,
`giv-hoist`, `cse-split`, ...) and `recipe <name>` prints one to paste; count
the candidate space before starting, and expect the run to end on its own. A
run you cannot bound is a run whose result you will not be able to interpret --
every one of the three failures above was a function whose park note had
already reduced the residue to `allocno_compare` arithmetic, which is not
reachable from C and therefore not reachable by a search that edits C.

**Feed it the parked queue, do not babysit it.** The permuter's input is the
set of functions parked under `#else /* NON_MATCHINGS */` — each already has
semantically correct C and a written hypothesis, which is exactly what the
search needs. Run it on those, unattended, whenever cores are free; check back
when a batch closes. What does *not* work is treating "keep a permuter running
at all times" as a goal in itself: in the sessions that built this file that
standing order produced 246 runs, 56 stop-and-restart cycles, and a stream of
turns spent asking whether one was already going, for a handful of landed
matches. The permuter is a background consumer of a queue, not a co-worker that
needs scheduling.

`tools/worklist.py` marks the parked functions with `P`; that column is the
queue.

This repo ships `config/permuter_settings.toml` (compiler type, build system,
and macros such as `gDma.*`/`_SHIFTL` that the permuter should treat as opaque
side effects rather than trying to permute); decomp-permuter auto-discovers it
by walking up from the target directory, so no extra flags are needed.

`tools/permuter_scratch.sh` runs that whole sequence and then the three checks
below, which is the form to reach for — the manual version is spelled out here
because every step of it has a failure mode worth knowing, not because it
should be typed:

```shell
git clone https://github.com/simonlindholm/decomp-permuter ../decomp-permuter
.venv/bin/pip3 install pynacl toml            # its deps, on top of requirements.txt
PERM_WEIGHTS=perm_ins_block=20.0,perm_temp_for_expr=200.0 \
    bash tools/permuter_scratch.sh FieldMain src/field/field.c
.venv/bin/python3 -u ../decomp-permuter/permuter.py nonmatchings/FieldMain \
    -j"$(($(nproc) - 2))" --stop-on-zero --better-only --stack-diffs
```

It takes the function and its `.c`, unparks a `MASPSX_OVERRIDE`'d body for the
import and restores the source afterwards, and prints the compile diagnostics,
the `.text` sizes and the relocation-symbol diff. It also merges `PERM_WEIGHTS`
into `settings.toml` rather than appending — `permuter_macros.py weights`
writes some of those keys already, toml rejects a duplicate key, and the run
then dies at startup with a `TomlDecodeError` and produces nothing at all,
which in a background job looks exactly like a search that found no
improvement.

```shell
git clone https://github.com/simonlindholm/decomp-permuter ../decomp-permuter
.venv/bin/pip3 install pynacl toml            # its deps, on top of requirements.txt
ninja build/us/src/battle/battle.c.o
export PATH="$PWD/tools/permuter-bin:$PATH"   # see "Toolchain overrides" below
.venv/bin/python3 ../decomp-permuter/import.py src/battle/battle.c \
    asm/us/battle/nonmatchings/battle/func_800A85FC.s
.venv/bin/python3 tools/permuter_strip_asm.py nonmatchings/func_800A85FC
.venv/bin/python3 tools/permuter_macros.py align nonmatchings/func_800A85FC --strings
.venv/bin/python3 tools/permuter_macros.py retarget nonmatchings/func_800A85FC
.venv/bin/python3 ../decomp-permuter/permuter.py nonmatchings/func_800A85FC \
    -j"$(($(nproc) - 2))"
```

`permuter_macros.py align` is the second correction the scratch needs, for the
same reason as the first: the score has to describe the code. The scorer diffs
`objdump` text, so a relocation shows up as the symbol's *name* — and the
generated `asm/` names globals after their address (`D_8009D820`) while the C
uses the project name (`g_DebugLevel`). Every such reference is a permanent
5-point penalty, so a byte-identical function can score 50 and `--stop-on-zero`
never fires. The same tool holds this project's `PERM_*` macro catalogue, which
turns the search from an unbounded random walk into a finite, exhaustive one;
see [docs/decomp/PERMUTER_MACROS.md](docs/decomp/PERMUTER_MACROS.md).

**The `permuter_strip_asm.py` step is not optional**, and skipping it is not
obvious from the output — the search just never converges. `import.py`
preprocesses the `.c`, so every `INCLUDE_ASM` has already expanded into a
`__maspsx_include_asm_hack_*` function holding `.include "<fn>.s"`, and those
land in `base.c` as `#pragma _permuter b64literal` blobs. The permuter decodes
them into every candidate, so each candidate object carries the whole overlay's
assembly — ~44,000 instructions for the field overlay — while `target.o` holds
only the function being permuted. The score is then almost entirely code the
permuter cannot affect.

The signature is a base score in the millions, and — the giveaway — base scores
that agree to within a percent across completely unrelated functions:

```
[FieldDebugPageAddPos]   base score = 4446740
[OpcodeFuncMpPlus]       base score = 4422490
[UpdateFieldExitArrows]  base score = 4361645   <- 45 after stripping
```

**Sanity-check the base score before trusting a run.** A correctly imported
scratch scores in the tens or low hundreds; `asm-differ` and `checkfn.py` tell
you roughly how far off the function is, and the base score should agree. If it
is in the millions, stop and strip.

Adding `-DSKIP_ASM=1` to the scratch's `compile.sh` does **not** fix this — cpp
consumed the macro at import time. It only ever appeared to help by accident,
when an import ran while `make report` had left `build.ninja` in its SKIP_ASM
configuration.

**Stripping is not enough on its own: check that the prune happened.** Even with
the blobs gone, `base.c` still holds every *matching C* function in the unit,
and the permuter compiles all of them into every candidate. `import.py` is
supposed to prevent that — it parses the preprocessed source with pycparser and
reduces it to the target function alone — but a syntax error anywhere in the
file makes it print

```
Syntax error in base.c.
before: ? at approximately line 2466, column 5
Proceeding anyway, but expect errors when permuting!
```

and skip the prune. Nothing downstream complains. `DrawFieldExitArrow`'s scratch
carried 107,120 bytes of `.text` against a 1,368-byte `target.o` — 98.7% of the
score was code the permuter cannot influence — and sixteen such runs spent three
to eight hours each hill-climbing that noise, one of them writing 12,113
"improvements", none converging.

Two things in this repo's own sources trigger it, both of which the matching
build never sees because they sit in `#else NON_MATCHINGS` arms:

* **a cast to a type declared later in the file, or in another unit.**
  `(FieldModelFileDesc*)` was used in `field2.c` 666 lines before its own
  `typedef`, and again from `field4.c`, which never saw it. Shared types belong
  in `src/field/field_private.h`.
* **raw m2c output that is not C.** `? *var_t0;` — m2c's placeholder for a type
  it could not infer — is a syntax error, and one of them blocks pruning for
  every function in the unit.

The cheap check is a size comparison, not a reading of the log:

```shell
mipsel-linux-gnu-size nonmatchings/<fn>/base.o nonmatchings/<fn>/target.o
```

A correctly pruned scratch has the two `.text` figures within a few percent of
each other — the whole object *is* the function under test. Anything else means
the prune did not happen, whatever the log said.

**That check reads the wrong object, though: `base.o` is written by `import.py`
and never rebuilt.** `permuter_strip_asm.py` runs *after* the import and edits
`base.c` only, so a correctly stripped scratch still has a fat `base.o` sitting
beside it — three field-overlay scratches all showed `base.o` at 7440 bytes
against targets of 3220, 2680 and 1628, which reads exactly like a prune
failure and is not one. Compile `base.c` with the scratch's own `compile.sh`
and size *that*:

```shell
./nonmatchings/<fn>/compile.sh nonmatchings/<fn>/base.c x /tmp/b.o
mipsel-linux-gnu-size /tmp/b.o nonmatchings/<fn>/target.o
```

Do it inside the container and through `tools/docker-build.ps1`, not a bare
`docker run` with a different mount point — `compile.sh` does `cd /ff7` and
then uses relative `-Iinclude`, so a scratch compiled against the image's baked
copy of the repo silently produces a 48-byte object.

**Run that compile with its diagnostics visible, every time, before starting a
search.** `permuter_macros.py align` rewrites declarations in `base.c` to match
the target's relocation names, and it can rewrite the *wrong* one: on
`FieldMain` it turned `extern volatile u32 D_8009AC3C[1];` into a second
declaration of `D_8009AC40`, so `D_8009AC3C` was undeclared at its only use.
gcc 2.6.3 substitutes 0 for an unknown identifier and keeps generating code
(see *A compile error that ninja calls a success*), the permuter's own log says
nothing, and the base score looks entirely plausible — 3100 for a function
sixty rows out. Every candidate in that run was hill-climbing a program five
instructions shorter than the one being matched. The check is one line:

```shell
./nonmatchings/<fn>/compile.sh nonmatchings/<fn>/base.c x /tmp/b.o 2>&1 \
  | grep -v 'warning:'
```

Silence is the pass condition.

**A permuter score can improve while the real diff gets worse, and symbol
aliases are why.** The scorer compares relocation *symbols*; `checkfn.py`
discounts a difference that is purely a symbol name for the same address. So
every alias the scratch still carries is a lever the search can pull for free
— it moves the score and moves nothing in the build. `FieldMain`'s best
candidate scored 2515 against a base of 3100 and measured **79 changed / 6
inserted** against 64/3 for the body it came from; decomposed into its six
individual edits, every one of them was a regression on its own as well
(70–81 rows). Two habits follow. Before trusting a run, diff the relocation
multisets and fix what `align` missed:

```shell
diff <(mipsel-linux-gnu-objdump -drz /tmp/b.o \
        | grep -oE 'R_MIPS_[A-Z0-9_]+[[:space:]]+[^[:space:]]+' \
        | awk '{print $2}' | sort | uniq -c) \
     <(mipsel-linux-gnu-objdump -drz nonmatchings/<fn>/target.o | ...)
```

**Two things make that diff lie, and `permuter_scratch.sh` now handles both.**
Neither is visible in the log, and each cost a scratch that read as broken:

* **A preserved macro is not expanded in `base.c`.** `import.py` leaves every
  `[preserve_macros]` entry as a `#pragma _permuter latedefine` block plus an
  opaque `void PC_INC();`, and only the permuter's own per-candidate
  preprocessing turns those back into `#define`s. cpp ignores an unknown pragma
  silently, so compiling `base.c` to check it emits a *call* to `PC_INC`: the
  object comes out short by a `%hi`/`%lo` pair per site and carries a `PC_INC`
  relocation the target cannot have. `OpcodeFuncFadew` read as 372 bytes
  against 396 with six mismatched symbols and was fine.
  `tools/permuter_latedefines.py` materialises them first. This affects every
  opcode handler in `src/field/`, which is most of that overlay's parked queue.
* **String literals and jump tables are local labels.** gcc emits them as local
  `.rodata` symbols, so a candidate relocates against `.rodata`; splat names the
  same bytes, so the target relocates against `D_800A0DE8` and
  `jtbl_800A0DF4`. Same address, same bytes, permanent penalty, so
  `--stop-on-zero` can never fire. `align --strings` cannot reach it: a literal
  has no declaration to rewrite, and naming it in C moves where gcc puts it
  (see the `sym_extern` note above). The target side has to move instead --
  the prelude's `glabel` macro takes a visibility argument, so
  `tools/permuter_rodata_local.py` demotes the `.rodata` ones to
  `glabel <sym>, local` and reassembles `target.o`.


An empty diff means the score describes the code — `AddBackgroundToRender` and
`FieldBackgroundInitPackets` are both clean this way, `FieldMain` is not: the
target names six interior members of `g_FieldRenderData` by their own
addresses, and `D_8009AC40` where the C reaches the same halfword through
`D_8009ABF4`. And after trusting it, still re-measure the winner with
`checkfn.py` before believing a single row of it.

**A parked body that does not compile is not permuter input.** 23 of the field
overlay's 72 `MASPSX_OVERRIDE` bodies are m2c seeds that gcc rejects — mostly
`request for member 'unkN' in something not a structure or union`, from
dereferencing a `void*` parameter. gcc 2.6.3 reports it and generates code
anyway, so the scratch builds and the run looks healthy; the base score is what
gives it away (`FieldCalcPointOnLine` scores 13,500 against neighbours in the
low hundreds). Fix the types first, or pick a different function. The check:

```shell
mipsel-linux-gnu-cpp -Iinclude -Iinclude/psxsdk -DNON_MATCHINGS -DFF7_STR \
    -lang-c -undef -fno-builtin src/field/field4.c | bin/str \
  | iconv -f UTF-8 -t Shift-JIS \
  | bin/cc1-psx-26 -quiet -mcpu=3000 -mgas -O2 -G0 -o /dev/null 2>&1 \
  | grep -v warning:
```

**A function that uses the GTE intrinsics could not be imported at all, and
the two reasons are now fixed in `config/`.** Every `Kawai*` lighting function
carried a park note saying "pinned pending a permuter pass" and none had ever
had one, because `import.py` died and deleted its own scratch on the way out:

* `mipsel-linux-gnu-as` does not know the GTE *mnemonics* -- `nccs`, `nclip`,
  `mvmva`, `gpl`, `avsz3` and the rest are `.macro`s in `include/gte.inc`,
  which `include/macro.inc` pulls in and which reach a normal build through
  `INCLUDE_ASM`'s `.include`. The *target* `.s` that import.py assembles into
  the reference object never got them, so the import failed with
  `Error: unrecognized opcode 'nccs'`. `config/permuter_prelude.inc` now does
  `.include "gte.inc"`; the file spells them upper-case and GAS matches macro
  names case-insensitively, so the lower-case forms in `asm/` resolve.
* pycparser cannot parse the `__asm__ volatile` that `gte_ldv0` and friends
  expand to, and **a syntax error anywhere in base.c makes import.py skip the
  prune silently** -- the exact trap this file already documents one section
  above, reached from a different direction. `config/permuter_settings.toml`
  now preserves `gte_[a-z0-9_]+` as opaque `void`, so the front end sees a
  call and the real macro is restored in the late-define block before every
  candidate compiles.

The check that the fix worked is the prune, not the import message:
`KawaiLightingApplyToPolyColor`'s base.c is 7 KB with exactly one function
definition in it. Note that the relocation-multiset check this file
recommends cannot pass for a function with preserved macros -- raw `base.c`
still calls `gte_nccs` as a function, and only the candidates get the real
`.word`. That is true of `PC_INC` and `GET_PARAM_U8` too; read the size and
the prune instead.

`import.py` takes `<c_file> <asm_file|func_name>` — there is no `-o` flag. The
`func_name` form only works if the function still has a `GLOBAL_ASM` stub, which
this repo does not use (it uses `INCLUDE_ASM`), so pass the target `.s` path
explicitly. `permuter.py` takes the scratch **directory**, not a bare function
name.

Run both through `.venv/bin/python3`, as with `diff.py` in step 3. The
permuter's own dependencies (`toml`, `pynacl`, `Levenshtein`) are installed into
this project's venv, so letting the scripts' `#!/usr/bin/env python3` shebang
pick the system interpreter fails with `ModuleNotFoundError: No module named
'toml'`.

**Always pass `-j`.** It is the only CPU knob the permuter has —
`config/permuter_settings.toml` does not carry one — and it **defaults to 1**,
so an untuned run searches on a single core and looks like the permuter is
simply bad at finding a match. Reserving two cores (`nproc - 2`) keeps the
machine usable during a search that can run for hours; use the full `nproc` on
a dedicated box.

Each worker is an independent compile-and-score loop, so heterogeneous cores
are fine — on a hybrid CPU the E-core workers just contribute fewer candidates
per minute than the P-core ones. Scale by core count, not by thread count: with
SMT enabled, two workers sharing a physical core each run near half speed.
Memory is not the limit (workers are well under 1 GB each), so the only reason
to go below `nproc - 2` is wanting the machine responsive for other work.

**In a worktree, the clone has to sit beside the *worktree*.**
`tools/docker-build.ps1` mounts `$repoRoot/../decomp-permuter` at
`/decomp-permuter`, and `$repoRoot` is the worktree — so the sibling it looks
for is `.claude/worktrees/decomp-permuter`, not the one next to the main
checkout. Copy it there (a few MB; delete the copied `.git`) or `import.py`
dies with `FileNotFoundError: 'mips-linux-gnu-as'` or is simply missing.

**Do not pipe the run through `tail`.** The permuter's progress line is the
only sign it is alive, and a pipe buffers it until the process exits — which,
without `--stop-on-zero` firing, is never. Run it with `python3 -u` and no
pipe. Either way the durable signal is on disk: every improvement lands in
`nonmatchings/<fn>/output-<score>-<n>/source.c`, so
`ls -d nonmatchings/*/output-* | sed 's/.*output-//' | sort -n | head` is the
score board, and `diff nonmatchings/<fn>/base.c <that>/source.c` is the finding.

**On this project the permuter's score does not track `checkfn`'s row count,
so harvest only a score of zero.** Three functions, three clean scratches --
relocations verified identical, `base.c` compiling to exactly the target's
instruction count, `compile.sh` identical to the ninja command but for
`-g`/`-gcoff` -- and in all three the best candidate of a long run measured no
better or worse in the build:

| function | base | best | `checkfn` on the winner |
| --- | --- | --- | --- |
| `FieldBackgroundInitPackets` | 1937 | 1547 | 43/4, i.e. *identical* to the body it started from |
| `FieldModelCreatePktsForPart` | 6030 | 4660 | 232/22 against 218/29, and one instruction short |
| `FieldMain` | 1780 | 1285 | semantically wrong (a truncated assignment) |
| `FieldEventRunInit` | 395 | 150 | 15/0 against 16/0 -- one row, from 221,676 candidates |

A fifth to a quarter of the score, for nothing. The two scorers weight
different things -- the permuter charges 100 per insertion and 5 per register
where `checkfn` counts rows and discounts symbol aliases outright -- and the
gap is wide enough that hill-climbing one says nothing about the other. What
still works is the terminating condition: score 0 is a real match, and
`--stop-on-zero` fires on it. So run the permuter as a *search for zero*, not
as a source of partial improvements; do not spend a session's attention on
`output-<score>/` directories, and re-measure with `checkfn.py` before
believing any of them. Both of the finds in the table above are the passes
this file recommends elsewhere (`perm_temp_for_expr` naming a subexpression,
an extra local), applied to exactly the residue a reading of the diff
identifies -- so being plausible is not evidence either.

**When to run it at all: the residue has to be block structure or a temporary,
not allocno arithmetic.** One session's record, every output re-measured with
`variant_eval` rather than read off the score:

| function | base -> best | what it was worth |
| --- | --- | --- |
| `FieldBattleCheck` (13 rows) | 85 -> 30 | 13 -> 6 rows |
| `FieldBattleCheck` (6 rows) | 30 -> **0** | **match**, 3,150 candidates |
| `LoadLocalFieldModelAndInitAll` | 1055 -> 580 | 46 -> 37 rows |
| `OpcodeFuncMove` (14 rows) | 85 -> 55 | 14 -> 10 rows |
| `OpcodeFuncMove` (10 rows) | 55 -> 35 | **69 rows / +43 instructions** |
| `OpcodeFuncVwoft` | 85 -> none | nothing in 101,000 |
| `FieldModelStructInit` | 5 -> none | nothing in 87,000 |

Every win is `perm_ins_block` or `perm_temp_for_expr` -- a dead conditional, a
duplicated block, an assignment to an existing local -- on a body that was
otherwise correct. Every failure is a function whose note had already reduced
the residue to `allocno_compare`/`QTY_CMP_PRI` and shown that neither term is
reachable from C without emitting an instruction. That is not a coincidence:
the permuter only edits C, so a residue proved unreachable from C is
unreachable for it too. **A note that ends in the allocno arithmetic is a park,
not a permuter target** -- and the same three functions have now absorbed
400,000 candidates between them across sessions for nothing.

The `OpcodeFuncMove` row is the other half of the warning: a 36% score
improvement bought a candidate that is 43 instructions longer, because the
`FieldModelData` temporary it introduced broke the arm-identity that lets flow
delete one copy of a duplicated block. Never land an output unmeasured.
**Re-import the scratch after every hand improvement — a stale base is why a
search runs forever.** The permuter hill-climbs from `base.c`, which `import.py`
froze at the moment the scratch was built, so every row you close by hand
afterwards is a row the search is still paying for. `FieldBattleCheck`'s search
from a 13-row body ran **192,000** candidates over hours and never got below a
score of 30; re-imported from the 6-row body it hit zero in **3,150** and
`--stop-on-zero` fired. The cost of re-importing is one `permuter_scratch.sh`
run, so do it whenever the body has moved, and treat a long search with no
improvement as a signal to check the base rather than to wait longer. Both
finds there were the assign-to-an-existing-local idiom -- `total = roll;` and
`D_8009ABF6 = (total = slot) & 0x3FF;`, two dead assignments to the same local
that emit nothing and only raise its reference count -- which is the pass to
weight (`perm_temp_for_expr`) when the residue is caller-saved naming.
**Read the finding, do not paste it — and re-measure it.** The permuter reaches
for two tricks that are noise rather than insight: rewriting every `a * b` as a
call to an `inline int inline_fn(a, b) { return a * b; }`, and introducing a
`new_var = 0` to pass where a literal `0` stood. Both are semantically identical
to what you wrote. The first is worse than inert: the helper is declared
`inline`, not `static inline`, so gcc 2.6.3 emits an out-of-line copy of it —
which on an overlay lands ahead of the function and shifts the whole address
range, and the scorer reads that as a large improvement. On `func_801B009C` it
reported 840 against a base of 2400; rewritten as `static inline` so no copy is
emitted, the same change measures *worse* than doing nothing. Score every
permuter output against the real overlay before believing it.

What *is* worth taking is a structural change you can restate in ordinary C —
naming a subexpression, splitting a statement. Even then a find is only true of
the state it was found in. The permuter's one real result on `ESCAPE.BIN`,
`n = (x * wave) >> 12;` hoisted out of the store that followed it, was worth 75
instructions when `.bss` was still one big `EscapeWork` object; after that was
split into the separate objects the original had, the plain inline expression is
what the target compiles and the named temporary costs eight rows. Re-derive a
find after any change to the layout it was measured against.

**Pass `--stack-diffs`, or the search optimises a different function than the
one you are matching.** The scorer's penalties are

| insertion | deletion | reordering | regalloc | branch | stack |
| --- | --- | --- | --- | --- | --- |
| 100 | 100 | 60 | 5 | 1 | 1 |

and the last column is only counted when `--stack-diffs` is given, which is
**off by default**: without it `src/objdump.py` normalises every `N(sp)` away
before the diff. A residue that is mostly stack-slot *naming* — two spilled
values holding each other's slot, so every `sw`/`lw`/`lbu` that touches them
reads the other offset — is therefore invisible, and the hill-climb spends its
whole budget on the rest. That is how `func_801B009C` produced a candidate the
permuter scored at 85 against a base of 250 which measured *worse* against the
retail overlay: it had traded two insertions (200 points) for one reordering
(60), while the twelve stack rows it left untouched were worth nothing either
way. Re-measure every output against the overlay, and pass `--stack-diffs`
whenever the diff has stack rows in it.

`perm_pad_var_decl` — "inserts an unused variable to adjust stack offsets" — is
the pass aimed at exactly that residue, and its default weight is 0.5. Raise it
in the scratch's `settings.toml` when stack layout is what you are hunting;
`perm_temp_for_expr` is the one that introduces a named local, which is the
lever behind the giv-versus-inline idiom above.

**A symbol the C reaches through a struct makes score 0 unreachable.** The
scorer compares relocation *symbols*, not addresses. `src/magic/escape.c`
writes `D_801518E4[row].D_80151909`, which relocates against
`D_801518E4 + 0x25`; the target `.s` names the byte directly as `D_80151909`.
Same address, same bytes, permanent penalty — so `--stop-on-zero` can never
fire even on a perfect candidate. `permuter_macros.py align` does not catch it,
because base.c *does* contain the string `D_80151909` — as a field name. Fix it
in the scratch, never in `asm/`: rewrite the two `%hi`/`%lo` operands in
`nonmatchings/<fn>/target.s` to the form the C produces and reassemble with the
`tools/permuter-bin` shim.

```shell
sed -i -e 's/%hi(D_80151909)/%hi(D_801518E4 + 0x25)/' \
       -e 's/%lo(D_80151909)/%lo(D_801518E4 + 0x25)/' nonmatchings/<fn>/target.s
mips-linux-gnu-as -march=vr4300 -mabi=32 nonmatchings/<fn>/target.s \
    -o nonmatchings/<fn>/target.o
```

**Drop `-g -gcoff` from the scratch's `compile.sh`.** The project build passes
both, so `import.py` copies them into the scratch, and cc1 then emits an `LMn`
line label every few instructions — 162 of them for `func_801B009C`. objdump
prints each as its own line *and* renders branch targets as `<LM4>` instead of
`<fn+0x38>`, so the candidate object is 650 lines against the target's 488.
Removing both flags is codegen-identical (verified instruction-for-instruction)
and takes that noise out of the comparison.

#### Toolchain overrides

decomp-permuter defaults to an N64 toolchain in three places, none of which
exist for this little-endian R3000A target. Two are fixed by settings, one is
not:

| Default | Override |
| --- | --- |
| `mips-linux-gnu-as -march=vr4300 -mabi=32` | `tools/permuter-bin/` on `PATH` |
| prelude opening `.set gp=64` | `asm_prelude_file` in `permuter_settings.toml` |
| `mips-linux-gnu-objdump -drz -m mips:4300` | `objdump_command` in the same file |

The assembler is the awkward one. `import.py` only overrides `DEFAULT_AS_CMDLINE`
by scraping an `asm-processor --assembler` flag out of the discovered build
command, and this project drives the assembler through maspsx, so nothing is
scraped. The `assembler_command` setting is not a way out either: `import.py`
asserts it is never combined with `build_system`, and dropping
`build_system = "ninja"` would force hand-writing `compiler_command` too —
discarding the ninja-derived compile line that is the whole reason the permuter
compiles exactly the way the real build does. So the assembler is overridden by
`PATH` instead, via a `mips-linux-gnu-as` shim that strips the N64 flags and
forwards to `mipsel-linux-gnu-as` with the `psx-as` flags from `build.ninja`.
Forget the `export PATH` line and `import.py` dies with
`FileNotFoundError: 'mips-linux-gnu-as'`.

`import.py` creates a self-contained scratch holding `base.c`, `target.s`,
`target.o`, `compile.sh` and a generated `settings.toml`. It is named after the
target `.s`'s parent directory, so for this project's
`asm/us/<ovl>/nonmatchings/...` layout it lands in **`nonmatchings/<func>/`** at
the repo root, not `permuter/` — both are gitignored. `permuter.py` then
searches until it prints a `(0)` base-score / perfect match or you stop it,
writing each improvement to an `output-<score>-<n>/` beside the scratch. Copy the
winning permutation back into the real `.c` file by hand — never commit anything
from the scratch directory — then re-run step 3 to confirm `asm-differ` shows
zero diff rows before moving to step 5.

The function must already have a C body in the `.c` for this to be useful: the
permuter mutates `base.c`, so if the function is still an `INCLUDE_ASM` stub
`import.py` warns `Function <name> not found in base.c` and there is nothing to
permute.

### 5. Verify for real

```shell
make build          # 13x OK — the commit gate
```

**Once per batch, not once per function.** `checkfn.py` is the per-function
signal and costs a few seconds; `make build` is the commit gate and costs
minutes. Running it after every function is both slower and misleading — see
the first rule below. The sessions that produced this file ran `make build` 441
times for ~147 landed functions, three full builds each, half of them red for
reasons that had nothing to do with the function under the cursor.

The batch shape that works:

1. `worklist.py` once — pick the batch (5–8 functions, file order).
2. Per function: `rodata_owner.py` → write C → `checkfn.py` until MATCH or
   budget spent → park with a note if spent.
3. Once at the end: `make build` → `make format` → commit.

## Adding a MAGIC spell overlay

`disks/us/MAGIC/` holds ~300 spell-effect overlays, all of which load into the
same slot at `0x801B0000`. Seven are in the build, all under `src/magic/`:
`BARRIER.BIN`, `MABARIA.BIN`, `REFREC.BIN`, `GATTAI.BIN`, `TEARS.BIN` and
`ALMIGHTY.BIN` are fully C; `ESCAPE.BIN` has one function left, `func_801B009C`
(12 of 361 instructions), parked under `#else /* NON_MATCHINGS */` with its diff
and two hundred measured rejected phrasings written down. They are the
cheapest work in the repo — a few kilobytes each, six or seven functions, no
`.rodata` entanglement — and the ones near barrier in size are near-clones of it,
so the matching C is largely a transcription with different field offsets.

The recipe, start to finish:

1. **Find the code/data boundary.** There is no header; the file is raw code
   from `0x801B0000`.

   ```shell
   mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL \
       --adjust-vma=0x801B0000 disks/us/MAGIC/MABARIA.BIN
   ```

   The last `jr $ra` plus its delay slot ends the text; everything after is
   data, and `.bss` starts at the file size. Every `jr $ra`/delay-slot boundary
   in between is also a function start, which predicts splat's split exactly.

2. **Size `.bss` from the `lui`/`addiu` pairs.** Grep the disassembly for
   `lui $x, 0x801b` and resolve the following `%lo`. References past the file
   size are `.bss`; the highest one plus its size is `bss_size`. Watch for the
   variable that sits *after* a 128 KB primitive buffer — it disassembles as
   `0x801D....`, which is just `0x801B0000 + 0x20...`, and forgetting it makes
   `bss_size` short by four.

3. **Add the overlay** to `config/us.yaml` (mirror the `barrier` block: `sha1`
   of the `.BIN`, `base_path: magic`, `vram_start: 0x801B0000`, and the three
   `c` / `.data` / `.bss` subsegments), add its name to the `MAGIC` group of the
   list in `tools/ninja/gen.py`, and create `config/symbols.magic-<name>.us.txt`
   — the path is read even when there is nothing to name, and `make format`
   rejects a file that holds only comments, so give it at least one real entry.

4. **Let `make build` run splat once.** It writes `src/magic/<name>.c` full of
   `INCLUDE_ASM` stubs and the `nonmatchings/*.s`, then fails to link on the
   `.bss` symbols, which nothing defines yet. That failure is the expected
   halfway point.

   **Then run `make build` again — not `ninja` — before reading any diff.**
   `tools/ninja/gen.py` reads the `//!` compiler line out of the `.c` at
   *build.ninja generation* time, and returns the defaults when the file does
   not exist yet. On the run that creates it, the overlay is therefore wired up
   as `cc1-psx-272` + aspsx 2.34, and `ninja <target>` alone never regenerates
   `build.ninja`, so it stays that way for every later iteration. Nothing in
   the output says so — you just diff against code from the wrong compiler and
   chase register-allocation ghosts. `ninja -t commands build/us/src/magic/
   <name>.c.o | tail -1` prints which `cc1` is really being used; check it the
   moment a diff looks structurally wrong.

5. **Write the whole file as C in one pass, using `barrier.c` as the
   template.** Do not try to land the `INCLUDE_ASM` build first: the `.s` files
   reference the `.bss`/`.data` symbols by their splat names, and the C
   definitions are `static`, so the intermediate state cannot link without
   renaming everything twice. The `.data` block is transcribed as
   `static s32 <name>_a1[] = {...}` from `od -A n -t x4 -v -j <off> -N <len>`,
   and the descriptor struct at its tail is a `Unk801B0C98`.

   **On an overlay too big for one pass, take the incremental route instead**
   (`ESCAPE.BIN` did): declare *one* symbol per contiguous region in
   `config/symbols.magic-<name>.us.txt` with a `// size:` annotation, and one
   matching global object in the `.c`. splat then renders every interior
   reference as `SYM+0x…`, so the functions still in `asm/` resolve while the
   ones you have written are C. Two things bite here. A global declared with no
   initialiser at all becomes a *common* symbol, which the linker places
   wherever it likes — give it an explicit initialiser, or fold it into an
   object that has one, or the whole `.data`/`.bss` slides and every address in
   the diff is wrong. (`= 0` is enough: cc1-psx-26 emits an explicitly
   zero-initialised global into `.data` in declaration order, so `EscapeBuf`,
   `EscapeWobble` and `EscapeUnk18` sit around the non-zero `EscapeScale`
   exactly where the original put them.) And an `INCLUDE_ASM` function whose
   *address* you take needs a real declaration: without one gcc substitutes 0
   for the identifier, emits `move a0,zero`, and the build says nothing (see
   *A compile error that ninja calls a success*).

   **Collapse the regions back into separate objects before tuning the diff.**
   One symbol per region is a scaffold for the intermediate state, not the
   shape the original had, and keeping it costs matches: gcc picks the base for
   a strength-reduced address from the whole object, so `EscapeWork.Tiles[n]`
   gives a giv based at `EscapeWork` and every access pays `%lo(EscapeWork +
   0x2ab8)` where the target pays `0x70(s0)` — plus four instructions per
   iteration to rebuild `&EscapeWork.Tiles[n]` for the calls that take its
   address. The tell is in the target: a base register used both with small
   offsets *and* bare (`move a0,s0`) means that base is its own object. Splitting
   `EscapeWork` into `EscapeGrid`/`EscapeTiles`/`EscapeBackPrims` took
   `func_801B009C` from 13695 to 4291 in one step. Splitting is safe — the splat
   rule is per-overlay (`add_splat_config` in `tools/ninja/gen.py`), so only
   that overlay's `asm/` is rewritten and no other overlay's `.s` is touched.

6. **The verdict is the overlay's SHA-1**, not `checkfn.py`. Once the file is
   plain C there are no `.s` files left for `checkfn` to compare against — it
   reports `no target asm`. `build/us/<name>.exe: OK` in the `make build` tail
   means every instruction *and* every data byte matched.

   That leaves no per-function signal while you are tuning one, and the three
   obvious substitutes each lie:

   * `checkfn.py` dies with `Not able to find .o file for function` — its
     `diff.py` call has no `-f`, so the map lookup never finds an overlay
     function. Call `diff.py` directly instead:
     `diff.py -o --format=plain -f build/us/src/magic/<ovl>.c.o <fn>`.
   * That compares against `expected/build/us/src/magic/<ovl>.c.o`, which is a
     *committed snapshot*. Rename one symbol and every reference to it reads as
     a difference for the rest of the session — `func_801B009C` scored 3986
     against an expected object that still said `EscapeWork`, when the real
     residue was 54 instructions. Use it for structure, never for a score.
   * Comparing `build/us/<ovl>.exe` against the retail `.BIN` word by word is
     ground truth only when the lengths already agree. One inserted instruction
     shifts the rest of the function and every later word "differs" — the same
     `func_801B009C` read as 250 differing words when 54 instructions differed.

   The honest measure is a diff of the *instruction sequence*, stripped of
   addresses and encodings, over the function's address range:

   ```shell
   dump() { mipsel-linux-gnu-objdump -D -b binary -m mips:3000 -EL \
       --adjust-vma=0x801B0000 "$1" | sed -n "/$2:/,/$3:/p" \
       | sed -e 's/^ *//' -e 's/\t/ /g' -e 's/^[0-9a-f]*: [0-9a-f]* //'; }
   dump disks/us/MAGIC/ESCAPE.BIN 801b063c 801b0dfc > /tmp/w.txt
   dump build/us/escape.exe       801b063c 801b0dfc > /tmp/g.txt
   diff -y --suppress-common-lines /tmp/w.txt /tmp/g.txt
   ```

   Equal line counts on both sides means the length is right; the diff is then
   the real work left, and it names the registers so the story is readable.

   Count it two ways. The raw diff is the verdict — zero is the only success —
   but as a *search signal* it lies, because a change of one stack slot moves
   the frame pointer offset of every `sw`/`lw` in the function and every branch
   target after it. `func_801B009C`'s residue is 52 instructions; the raw count
   reads 104 rows, of which more than half are the prologue, the epilogue and
   the spill traffic all shifted by eight bytes. Normalising

   ```shell
   norm() { sed -e 's/-\{0,1\}[0-9]\{1,\}(sp)/N(sp)/g' -e 's/0x801b[0-9a-f]*/L/g'; }
   ```

   before the diff gives a second number — call it the shape — that moves only
   when something structural changes.

   Count it a *third* way as soon as candidates change the function's length.
   An overlay is code followed by its own `.data`, so one extra instruction
   slides every later byte: each `addiu at,at,14732` becomes `…,14772`, the
   tail of the next function slides into the dumped address range, and a
   candidate three instructions away reads as two hundred rows. The shape does
   not save you — it normalises `N(sp)` and `0x801b…`, but objdump prints those
   displacements in *decimal*. Collapse them too:

   ```shell
   core() { norm | sed -e 's/-{0,1}[0-9]{4,}/K/g'; }
   ```

   Rank candidates by core, sanity-check with the shape, and confirm with the
   raw count — which stays the only verdict. This is not a nicety: every
   hoisted variant of `func_801B009C` scored 210 to 244 raw and looked equally
   hopeless, and the best of them was nine instructions out.

   **Sweep phrasings in batches, not one at a time.** `ninja build/us/<ovl>.exe`
   plus the two dumps is about two seconds, so a container invocation that loops
   over a directory of candidate `.c` files scores a dozen or more of them in a
   couple of minutes. Generate the candidates on the host (one `cp` of a pinned
   base plus a `perl -0pi` edit each — the same discipline `variant_eval.py`
   enforces for `checkfn`-scored functions), then run the loop inside the
   container and print `name rows= shape=` per line. Fifty-eight phrasings of
   `func_801B009C` were scored this way in six batches; the one that helped —
   `rand()` written as the first operand rather than the second — is not
   something anyone would have picked out of a list of guesses, and the ones
   that were worse are now in the park note instead of being re-tried next
   session. Gate each build on the compile diagnostics (`^src/.*:[0-9]+: `
   minus warnings), or a variant with a typo scores as "no change".

7. **A parked function's `.s` freezes at the symbol names of the moment it
   became C.** splat writes `nonmatchings/<fn>.s` only for functions the `.c`
   still holds as `INCLUDE_ASM`, so once you replace the stub the file stops
   being maintained. Rename a `.bss` symbol afterwards and the stale `.s` still
   refers to the old name; park the function again and the link fails with
   `undefined reference to EscapeWork` naming a symbol nothing defines any more.
   `touch config/symbols.magic-<name>.us.txt` re-runs splat and rewrites every
   `.s` in that overlay against the current names.

Naming: keep the entry point `MAGIC_<Spell>` and prefix the statics, since all
these overlays share the `0x801B0000` address space and `config/
sym_ovl_export.us.txt` collects them into one namespace. The entry point is not
always at offset 0 — `GATTAI.BIN` puts its per-frame tick there and the
`MAGIC_*` entry at `0x801B007C`.

**A `static` in `src/battle/` that a MAGIC overlay calls has to lose the
`static`.** The overlays link against `config/sym_export_battle.us.txt`, which
is generated from `battle.elf`'s *global* symbols, so a callee the repo made
file-local is simply undefined at link time. Dropping `static` changes nothing
about the emitted code — only the symbol's binding — and `battle.exe` stays
green; `func_800BBA40`, `func_800C55B8` and `func_800D56A8` were opened up this
way. Add the declaration to `src/battle/battle.h`, not to the overlay.

## Rules that prevent wasted work

**Do not use `make build` as the inner-loop signal.** While a function is
mid-work, `make build` fails for reasons unrelated to your correctness. A
function whose compiled size differs shifts every downstream address, and the
regenerated symbol export then collides:

```
error reading config/sym_export_battle.us.txt, line 929:
Duplicate symbol detected! D_800F83A4 clashes with g_BattleState at vram 0x800F83AC
```

This is expected and is **not** a broken repo, a bad toolchain, or something to
"fix". Use `ninja <one>.o` + `asm-differ` (step 3), which works fine while the
full build is red. Only run `make build` once the diff is clean.

**Never delete or stub an `INCLUDE_ASM` line to make the build pass.** That
silently drops a function. If you cannot match a function, revert your changes to
that function and leave the `INCLUDE_ASM` in place.

**A `.s` with no prologue is not a function, and the work list cannot tell.**
spimdisasm splits functions on its own heuristics and it gets it wrong: in
`src/world/world2.c`, `func_800BFCAC.s` ended with `andi $v0,$v0,0x3f` and no
`jr $ra`, and `func_800C02F4.s` began with `or $v1,$v1,$v0` and no
`addiu $sp`, carried the *other* function's epilogue, and branched backwards
into it (`bnez $v0, .L800BFD80`). They are one 727-instruction function that
splat had rendered as a 402-instruction one plus a 325-instruction one, and
`worklist.py` counted both as actionable work. Nothing else complains --
the bytes are right, the overlay's SHA-1 is right, and the halves diff
perfectly against themselves.

Three tells, any one of which is conclusive:

* the `.s` has no `addiu $sp, $sp, -N` at the top, or no `jr $ra` at the
  bottom (`head -6` / `tail -4` is the whole check)
* a branch target rendered as `.L800…` that lies outside the file's own
  address range -- so the two `.s` reference each other's labels
* no callers: `grep -rl "jal *<name>" asm/ src/` finds nothing, and the
  function is not an obvious entry point

The fix is to pin the real function's size in the overlay's symbol config,
which is read by splat only:

```
func_800BFCAC = 0x800BFCAC; // size:0xB5C
```

`// ignore:true` on the *spurious* symbol is **not** enough on its own -- it
suppresses the second `.s` but leaves the split, so the merged file then has
an undefined reference to the label that lived in the file that no longer
exists. With the size pinned, splat emits one `.s` and keeps the interior
`glabel`, so `config/sym_ovl_export.us.txt` does not churn and every other
overlay links unchanged. Delete the stale `asm/us/<ovl>/nonmatchings/<unit>/
<spurious>.s` by hand -- splat does not remove it -- and drop its
`INCLUDE_ASM` line from the `.c`.

Check for this before picking any function whose rank score is 1.000 and
whose caller search comes up empty; both halves of this pair scored 1.000.

**A second shape the work list cannot screen: an asm-only helper of a
handwritten function.** `/* Handwritten function */` is written into the `.s`
of the handwritten function itself, and its private helpers do not carry it —
so they arrive on the list looking like ordinary 40-instruction work.
`func_800D32B4`, `func_800D3354`, `func_800D8304` and `func_800D83A4` in
`src/battle/battle2.c` are four of them, and they were entries 8, 9, 15 and
17 of a 60-function list, i.e. among the cheapest-looking picks in the file.
Two tells, either one conclusive and both one `grep` away:

* **the function returns through `jr $at`.** No C function does; `$at` is the
  assembler's scratch register. These four have *two* exits — `jr $at` for the
  early-out and `jr $ra` for the fall-through — which is a hand-written
  two-result convention, not a compiled one.
* **it reads `$t0`/`$t1`/`$t2` (or any register that is not `$a0..$a3`) before
  writing them.** Arguments arrive in `$a0..$a3`; a first instruction of
  `sll $v0, $t0, 16` means the caller set `$t0` up by hand.

```shell
grep -l 'jr *\$at' asm/us/<ovl>/nonmatchings/<unit>/*.s
for f in *.s; do sed -n '5p' $f | grep -q '\$[ts][0-9]' && echo "$f"; done
```

Confirm by finding the callers: all four are called only from `func_800D29D4`
and `func_800D7D3C`, which are themselves on the handwritten list. A helper
whose only callers are handwritten is not remaining work.

**And an instruction *after* the `.size` directive is object padding, which a
merged unit cannot emit.** splat writes the bytes between the end of a
function and the start of the next one into the first function's `.s`, past
its own `.size`, so a function can be byte-exact in all its real instructions
and still be four bytes short when written as C. `func_800D3520.s` is the only
`.s` in `battle2.c` with one, and functions in that overlay are not 8-byte
aligned in general (`func_800D7B1C` sits at `...B1C`), so it is an original
translation-unit boundary rather than function alignment — `battle2.c` is
several of the original `.c` files merged, and the pad belongs to the end of
one of those objects. The failure mode is the cross-overlay one: 36 bytes
where the target has 40 shifts every later symbol in `battle.elf` and comes
back as `batini.c: undefined reference to D_800F7ED0`. The check costs one
command and belongs next to the handwritten screen:

```shell
for f in *.s; do awk '/^\.size /{s=1;next} s&&/\/\* [0-9A-F]+ [0-9A-F]+ /{c++}
  END{if(c)print FILENAME": "c" trailing"}' $f; done
```

`INCLUDE_ASM` hard-codes `.align 2`, so there is no in-source way to reserve
the pad; such a function is a park however exact its body is.

**Scope a rename by owner, not by name.** A struct member or a local is not
a symbol, so renaming one cannot change codegen -- but only if the thing being
renamed is really the one you meant. `unk10` occurs in six unrelated structs
across the tree and `unk6` in three within `include/game.h` alone; a pass that
renamed every `->unkNN` touched battle, main, world and magic, and every one of
those edits would have compiled. Both halves need scoping: the **declaration**
inside the owning typedef's braces -- anchored on the closing `} Name;` and
walked back to the nearest preceding `typedef struct ... {`, since a non-greedy
match from the first typedef in the file spans every struct above it -- and each
**use** by the expression that reaches it (`part->unk1C`, never `->unk1C`). The
same applies to locals: `var_v0_10` was dead in `FieldEntityMovementUpdate` and
live in `FieldEntityMove` twenty lines below, so a file-scoped dead-declaration
sweep deleted a live one. gcc 2.6.3 does not stop on the undeclared identifier
-- it substitutes 0 and keeps generating code -- so the only symptom would have
been the score.

**A mis-compile in one overlay surfaces as a link failure in a *different*
one, naming a symbol you never touched.** `config/sym_export_battle.us.txt` is
regenerated from `battle.elf` on every build and handed to `batini` and the
MAGIC overlays as a `-T` script. gcc 2.6.3 reports an error and keeps going, so
a `structure has no member named 'unk14'` in `battle.c` -- from an edit to a
struct in `battle.h` -- still produces a `battle.elf`, with its `.bss` moved.
Every name in the export file then shifts, and what you see is

```
build/us/src/battle/batini.c.o: in function `LM13':
undefined reference to `D_800FAFD0'
```

in an overlay whose source you did not change, about a symbol the edit has
nothing to do with. The second-order form is worse: if the shifted file happens
to still carry *some* name for the address, batini links against the wrong one
and comes out with a **one-byte** SHA-1 mismatch and no diagnostic at all.
Both were hit in one session from that single missing member.

So when a link fails on a symbol you did not touch, the first move is to
recompile every unit of the overlay the symbol comes from and read its
diagnostics -- not to go looking at the symbol config:

```shell
touch src/battle/battle*.c && ninja build/us/src/battle/battle.c.o ... 2>&1 \
  | grep -v 'warning:' | grep ':[0-9]*:'
```

And do **not** "fix" it by adding the name to `config/sym_extern.us.txt`. That
file is read by splat for every overlay and by `ld` for every link, so one line
there moved `main`, `battle`, `batini` and all seven MAGIC overlays off their
SHA-1s at once, which reads as a catastrophe rather than as a typo. A
per-overlay `config/symbols.<ovl>.txt` entry is the scoped tool; the global one
is for the `.rodata`-borrowing case this file documents above and nothing else.

While a build is in that state the generated export is *poisoned*, and reading
an address out of it is how the diagnosis goes wrong: `sym_export_battle.us.txt`
said `D_800FAFEC = 0x800fafd0` (0x1C low, from the broken `battle.elf`), which
reads exactly like a long-standing misnaming in the repo and is worth a
paragraph of wrong reasoning. Fix the compile first, then re-read the file.

**Never edit generated files.** `asm/`, `build/`, `expected/`,
`config/sym_export*.txt` and `build.ninja` are all produced by the build. Editing
them accomplishes nothing and hides real errors.

**`config/sym_ovl_export.us.txt` is different** — it is generated *and*
committed. If a build changes it, commit the change; CI checks it with
`git diff --exit-code`.

**Revert cleanly on failure.** `git checkout -- <file>` then re-run `make build`
to confirm you are back to 13× `OK` before moving on. Never leave the tree in a
state where the build is red.

**Park, do not grind.** Three shaped attempts and one permuter run per function
(step 0). A fourth attempt with no new hypothesis is not persistence, it is the
same guess reseeded. Write the near-miss note and take the next function.

**Read the park note before the first measurement, not after the third.** The
notes in `src/field/*.c` are long because they are exhaustive: they list the
rejected spellings with their row counts. Picking a parked function and
starting from the diff re-derives them one build at a time. In the session that
wrote this paragraph, a whole budget went into re-measuring `HandleKawaiDataInModel`'s
`blinkOpen` local and another into `OpcodeFuncVwoft`'s duplicated `PC_INC`
tail — both already written down, both to the row, including the 48/8 the
duplication measures. The note is the first tool, ahead of `checkfn.py`.

**Write the finding down the first time.** Every gcc idiom, every environment
trap, every "this looked like a match but wasn't" belongs in this file or in
`docs/decomp/PERMUTER_MACROS.md` **in the same change that discovered it** — not at
the end of the session, which does not arrive. The measure of whether this is
working: across the sessions that built this file, the stale-object trap was
re-derived 47 times, the permuter-scratch strip 38 times, and the PowerShell
heredoc limitation 29 times, because each was discovered, used, and then lost
with the context window. A fact that lives only in the conversation gets paid
for once per compaction.

**Regenerate the work list, do not remember it.** `worklist.py` at the start of
every batch. Reconstructing "what is left" by grepping `INCLUDE_ASM` and
eyeballing `mako.sh rank` output is a per-batch tax that also gets the
handwritten and `.rodata`-blocked functions wrong.

## Definition of done

Before committing, all four must hold:

| Check | Command | Required result |
| --- | --- | --- |
| Function matches | `tools/checkfn.py <src> <fn>` | `MATCH` |
| Nothing regressed | `make build` | 13× `OK` |
| Formatted | `make format` | empty `git diff` |
| Export committed | `git diff --exit-code -- config/sym_ovl_export.us.txt` | clean |

`make format` uses a SHA-256-pinned `clang-format` the builder downloads, so it
never drifts by local version. Run it, do not hand-format.

Commit only `config/`, `include/` and `src/` — `make submit` stages exactly those.

## Code conventions

### Compiler selection

The first line of each `.c` selects the compiler and flags:

```c
//! PSYQ=3.3 CC1=2.7.2 G=8
```

* `PSYQ=3.3|3.5|3.6|4.0` — sets both cc1 and the assembler version
* `CC1=2.6.3|2.7.2` — cc1 binary (`2.6.3` → `cc1-psx-26`, `2.7.2` → `cc1-psx-272`)
* `G=<n>` — `-G<n>` small-data threshold; `O=<n>` — optimisation level
* `g=false` / `gcoff=false` — drop `-g` / `-gcoff`

Default is `cc1-psx-272 -O2 -G0` with aspsx 2.34. **Do not change this line to
force a match** unless you have evidence the whole translation unit was built
differently — it affects every function in the file and will break the ones that
already match.

**Which headerless units are still unvalidated, measured.** A default is only
a coin flip while nothing has been compiled against it; once a unit has
matching C bodies, the default is *proven* for that unit and there is nothing
to find. Six units carry no `//!` line, and they split cleanly:

| unit | matching C bodies | verdict |
| --- | --- | --- |
| `src/battle/battle.c` | 145 | default proven correct |
| `src/battle/batini.c` | 20 | default proven correct |
| `src/world/world2.c` | many | default proven correct |
| `src/brom/brom.c` | all (100% C) | default proven correct |
| `src/main/ovl.c` | 0, but also 0 functions | nothing to decide |
| `src/dschange/dschange.c` | **0** | unvalidated — tested below |

`dschange.c` was the only real candidate and the answer is **negative**: its
one parked body (`func_800A0C58`, 586 instructions) measures **223 rows / 26
insertions / exact length under both `CC1=2.6.3` and the default `2.7.2`**,
and `insn_histogram` agrees to within one row (`addu +7 / addiu -3 / lui -3 /
sll -2` either way). The flags do reach the measurement — that one differing
row is the proof — the function simply is not compiler-sensitive. So its 223
rows are ordinary work, not a wrong toolchain, and the header stays absent.

Do the same check before spending anything on the hypothesis: **count the
unit's matching C bodies first.** If it has any, the default is already
validated by them and the compiler is not your problem.

**A file with no `//!` line at all has not been *decided*, it has been
defaulted — and on a fresh unit that is a coin flip you have to call before
writing anything.** `src/ending/ending.c` carried the default for as long as it
existed and the whole overlay is `//! PSYQ=3.3 CC1=2.6.3`; twelve functions
written against the default came out MISMATCH by one instruction each and read
as twelve separate scheduling puzzles. Three tells settle it in one command
apiece, and none of them needs a diff:

* **`jr ra` with the stack adjust in its delay slot is 2.7.2; `lw ra / addiu
  sp / jr ra / nop` is 2.6.3.** gcc 2.7.2 has `DELAY_SLOTS_FOR_EPILOGUE`, so
  reorg fills the return slot with `addiu sp` *and* moves the last body insn
  into the `lw ra` load-delay shadow — two instructions shorter than 2.6.3's
  epilogue for the same function. Every non-leaf function in the target then
  reads as "-1 instruction, a `nop` we do not have", which is exactly what a
  scheduling residue looks like. Count it across the target instead of reading
  one function:

  ```shell
  mipsel-linux-gnu-objdump -d expected/build/us/<src>.o \
    | grep -A1 'jr[[:space:]]*ra' | grep -c 'addiu[[:space:]]*sp'
  ```

  A file that is genuinely 2.7.2 still scores near zero on this (real code
  rarely offers reorg a candidate), so a *nonzero* count in **your** build
  against zero in the target is the signal, not the target's count alone.
* **`lui at / addiu at / addu at / op 0(at)` where you get `lui at / addu at /
  op %lo(at)` is aspsx < 2.30**, not a C-level addressing choice. cc1 emits
  `sh $0,SYM($2)` either way — the four-instruction form is maspsx's
  `addiu_at`, which `tools/maspsx/maspsx.py` turns on for `aspsx_version <
  (2, 30)`. Before spending a budget on the scaled-subscript-versus-byte-offset
  recipe, check the assembler version; the recipe cannot produce the extra
  `addiu` at all under 2.34.
* **An unexplained `nop` in a load-delay slot that an `$at` expansion's `lui`
  would have covered is `nop_at_expansion`, same version gate.**

The cheapest confirmation is not a build at all — run the unit through both
compilers and read one function:

```shell
mipsel-linux-gnu-cpp -Iinclude -Iinclude/psxsdk -DUSE_INCLUDE_ASM -DFF7_STR \
    -lang-c -undef -fno-builtin src/ending/ending.c \
  | bin/str | iconv -f UTF-8 -t Shift-JIS \
  | bin/cc1-psx-26 -quiet -mcpu=3000 -mgas -O2 -G0 -g -gcoff
```

**Changing the `//!` line and re-running `checkfn.py` does not test the change.**
`tools/ninja/gen.py` reads that line when **build.ninja is generated**, and
`ninja <target>` — which is what `checkfn.py` runs — never regenerates it. The
compile line keeps the old compiler, every verdict comes back byte-identical to
the previous run, and the honest-looking conclusion is "the compiler is not the
difference". It cost a whole hypothesis here. Regenerate first and verify:

```shell
make build OVERLAYS=<ovl>          # regenerates build.ninja
ninja -t commands build/us/<src>.o | tail -1 | grep -o 'cc1-psx-[0-9]*\|aspsx-version=[0-9.]*'
```

That is the same trap the MAGIC-overlay recipe records for a *newly created*
`.c`; it applies just as much to editing the header of one that already exists.

### Game strings

The game uses a custom encoding, not ASCII. Strings are written `_S("Hello")`.
To decode one from the original data:

```shell
make bin/str
bin/str disks/us/MENU/SAVEMENU.MNU 12DF8
```

### Naming

* `func_800A85FC` / `D_801D1AA8` — auto-generated, address-based. Renaming one to
  something meaningful is a genuine contribution.
* Rename via the symbol config, not with find-and-replace:

  ```shell
  ./mako.sh symbols add config/symbols.battle.txt g_BattleState 0x800F83AC 4
  ```

* Per-overlay symbol files are `config/symbols.<overlay>.txt` or
  `config/symbols.<overlay>.us.txt`.

**A symbol referenced by a `MASPSX_OVERRIDE` function cannot be renamed.**
splat writes `nonmatchings/<fn>.s` only for functions the `.c` still holds as
`INCLUDE_ASM`; a pinned function's `.s` is frozen at the symbol names of the
moment it stopped being one. Rename the symbol and splat rewrites every
*generated* `.s` to the new name while the frozen ones keep the old, which no
longer exists — so `make build` dies with

```
src/field/field4.c:(.text+0xc38): undefined reference to `D_800DF520'
```

naming the pinned function, not the symbol config. Check before renaming, and
check with a **multiline-aware** matcher: `clang-format` wraps a long
`MASPSX_OVERRIDE("asm/us/field/nonmatchings/field4", SomeLongName);` across two
lines, so a line-based `grep -o 'MASPSX_OVERRIDE([^)]*)'` silently under-counts
— it found 37 of the field overlay's 42, and the five it missed were exactly
the ones that broke the build.

```shell
python3 - <<'EOF'
import io, re, glob, os
frozen = []
for f in glob.glob('src/field/field*.c'):
    u = os.path.basename(f)[:-2]
    src = io.open(f, encoding='utf-8').read()
    for m in re.finditer(r'MASPSX_OVERRIDE\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', src, re.S):
        frozen.append('asm/us/field/nonmatchings/%s/%s.s' % (u, m.group(1)))
for sym in ('D_800DF520', 'D_800E0200'):
    print(sym, [p for p in frozen if re.search(r'%s' % sym,
                io.open(p, encoding='utf-8', errors='ignore').read())])
EOF
```

An empty list means the rename is safe; anything else means the symbol is
locked until that function matches. There is no alias escape — splat rejects
two names at one vram with `Duplicate symbol detected!`.

**A rename has to sweep `src/**/*.h`, not just the `.c` files.** Most of the
`extern` declarations for these globals live in the per-overlay private headers
(`src/field/field_private.h`, `src/main/main_private.h`), not in
`include/game.h`. Miss one and gcc 2.6.3 reports `'g_Foo' undeclared (first use
in this function)`, substitutes 0, and **keeps generating code** -- so the build
runs to completion and the only symptom is the overlay's SHA-1. Renaming
thirteen field globals this way silently took 736 bytes out of `field.exe`
(0x2BC from `field4.c`, 0x24 from `field5.c`) with every function still
diffing perfectly. The give-away is in `build/us/<ovl>.map`: a `.text` size that
shrank. Check the compile output for non-warning diagnostics before believing
a rename is inert.

**Where a renamed symbol's name has to be declared depends on which overlay
*defines* it.** Field's `vram_start` is `0x800A0000`, so every field global
below that address is defined in **main**'s `.bss`/`.data` and merely
referenced from field. Three files are involved and they do different jobs:

| file | job |
| --- | --- |
| `config/symbols.<ovl>.us.txt` | names references in the `.s` splat writes for that overlay |
| `config/sym_extern.us.txt` | `-T` linker script -- defines the absolute symbol, for *every* overlay's link |
| `config/sym_export.us.txt` | generated from `main.elf`; do not edit |

A main-owned global referenced from field needs an entry in **both** of the
first two (`g_FieldMusicLock` is the worked example). With only the field entry,
`main.elf` fails to link the moment main's own C references the new name --
`undefined reference to 'g_FieldMovieLock'` -- because main's `.bss` `.s` still
defines the old `D_` label. And `symbols.*.us.txt` is parsed strictly: a comment
containing a `:` is rejected with `error reading config/symbols.field.us.txt,
line N`, so keep the prose colon-free and put `// size:0xN` on its own.

**And it must *not* also go in `symbols.main.us.txt`, however natural that
looks.** splat reads that file and the linker script for the same overlay, so
naming a main-owned symbol in both makes it see one name at one vram twice and
the split dies before anything compiles:

```
error reading config/sym_extern.us.txt, line 100:
Duplicate symbol detected! g_PreemptiveRate clashes with g_PreemptiveRate
  defined at vram 0x80062F1B.
```

The tempting fix -- put it only in `symbols.main.us.txt`, so main's own
regenerated `.s` picks the name up too -- does not link: the name reaches
main.elf but not `sym_export.us.txt`, so every *other* overlay's C fails with
`undefined reference`. Both failure modes were hit in order while naming
`g_PreemptiveRate`, `g_AkaoChannelMask` and `g_AkaoPendingMask`. The
combination that works is the `g_FieldMusicLock` one and only that one:
`sym_extern.us.txt` defines the address for every link, plus a
`symbols.<ovl>.us.txt` entry for each overlay whose *assembly* should print the
new name. main's own `.s` keeps the `D_` label, which is harmless -- both names
resolve to the same address, and the overlay SHA-1s are the proof.

## Repository map

| Path | Contents |
| --- | --- |
| `src/<overlay>/*.c` | The decompiled source — what you edit |
| `include/` | Shared headers; `include/psxsdk/` is the PSY-Q SDK |
| `config/us.yaml` | splat config: overlays, segments, target SHA-1s |
| `config/symbols.*.txt` | Hand-maintained symbol names — edit via `mako.sh symbols` |
| `config/permuter_settings.toml` | Auto-discovered by decomp-permuter (see step 4) |
| `tools/builder/` | The Go build driver behind `./mako.sh` |
| `tools/worklist.py` | Per-file work list: what is left, blocked, handwritten, cheapest first |
| `tools/checkfn.py` | Per-function match verdict; use instead of eyeballing `diff.py` |
| `tools/parked_queue.py` | Every parked near-miss in a file, measured and ordered closest-first |
| `tools/insn_histogram.py` | A function against its target by opcode and by symbol — what is wrong, not how many rows |
| `tools/rodata_owner.py` | Whether a function can be decompiled without shifting `.rodata` |
| `tools/asm_widths.py` | Per-symbol access width from a target `.s` — what an m2c seed's byte offsets have to be cast to |
| `tools/psx_jtbl_align.py` | Jump-table alignment fixup for units whose `.rodata` base is 4 mod 8 |
| `tools/affected_overlays.py` | Changed files → the overlays CI has to rebuild |
| `tools/width_sweep.py` | Every alternative width for every scalar local of one function, scored |
| `tools/loop_movables.py` | `move_movables`' hoist decision per candidate, each insn named by its RTL |
| `tools/qty_pri.py` | `local_alloc`'s `pri`/`n_refs`/`live_length`/`size` and the register each pseudo got |
| `tools/permuter_macros.py` | Permuter scratch alignment, `PERM_*` recipes, search sizing |
| `tools/permuter_scratch.sh` | Build a permuter scratch and prove it is scoreable before searching |
| `tools/unpark.py` | Make one `MASPSX_OVERRIDE`'d body the live one, for import or measurement |
| `tools/variant_disasm.sh` | Score a variant and disassemble its object |
| `tools/uninit_regs.py` | Registers a compiled body reads before writing — finds unreachable-assignment bugs |
| `docs/decomp/worklist-*.md` | Generated by `worklist.py` — regenerate per batch, never hand-edit |
| `disks/us/` | Extracted game files (generated, gitignored) |
| `asm/`, `build/`, `expected/` | All generated — never edit |

Overlays: `main`, `batini`, `battle`, `brom`, `dschange`, `ending`, `field`,
`bginmenu`, `cnfgmenu`, `savemenu`, `itemmenu`, `world`, `barrier`.

## Building a subset of the overlays

```shell
make build OVERLAYS=field,world      # split, compile, link and sha1-check these two
```

The overlays left out are not split, compiled or linked at all. What is left in
is not a free choice, though — the dependencies are real:

* Every overlay links against `config/sym_export.us.txt`, generated from
  `main.elf`, so any subset still compiles the whole of `main`.
* `main` itself links against `config/sym_ovl_export.us.txt`, regenerated from
  the ELF of *every other* overlay. `OVERLAYS=main` therefore degrades to a full
  build rather than pretending to narrow anything.
* `batini` and the magic overlays link against `config/sym_export_battle.us.txt`,
  so a `battle` change has to be checked against them too.

`.github/workflows/build.yaml` uses this to scope a pull request:
`tools/affected_overlays.py` maps the PR's changed files onto overlays and the
build only checks those. The mapping is derived from `config/us.yaml`, not
hardcoded — `src/menu/` alone feeds four overlays, one per `c` subsegment. It
answers `full` for anything it does not recognise (a new source file, a change
under `config/`, `include/` or `tools/`), and `none` for documentation, which
skips the build job entirely.

Two consequences worth knowing:

* A partial build leaves `config/sym_ovl_export.us.txt` untouched, so CI runs
  the `git diff --exit-code` on it only after a full build. Every path that can
  change that file forces `full`, so nothing slips through.
* `make report` and `make submit` always build everything; the knob is only on
  `make build`.

## Progress

```shell
make report     # true match% into build/report.json (SKIP_ASM build vs expected/)
make board      # sync the GitHub Projects board — HOST only, needs gh
```

The board at <https://github.com/users/dudhasch/projects/5> is **generated** from
`build/report.json`. Do not hand-edit issue bodies; the next sync overwrites them.
See [docs/decomp/BOARD.md](docs/decomp/BOARD.md).

## Working in parallel

`build/`, `config/sym_export_*.txt` and `.ninja_*` are shared mutable state. Two
agents in the same working tree **will** corrupt each other's builds. For
concurrent work use one `git worktree` per agent, symlinking `disks/` in rather
than copying it (it is ~1.3 GB).

**Set this up before the second agent starts, not after the tree breaks.** The
failure mode is not a clear error: the other agent's half-finished renames make
every overlay fail to link, which reads as "the repo is broken" and sends
whoever hits it debugging the toolchain. If overlays start failing to link for
no reason you can attribute to your own edits, check for a second session
first.

A worktree needs three things the `git worktree add` does not give it:

* its **own Docker build volume** — `ff7_build_<name>`, since `build/` is the
  shared state that corrupts
* `asm/`, `bin/`, `disks/` and the generated `config/` files, none of which are
  tracked; symlink `disks/`, and either symlink `asm/` or regenerate it
* the tool submodules (`asm-differ`, `maspsx`, `builder`) — an empty `maspsx`
  fails **silently**, producing objects that diff cleanly against nothing

Cheapest path: keep one bootstrapped worktree around and reuse it rather than
creating a fresh one per task. Bootstrapping a new one from that copy is
~45 MB and four steps, and two of them are not guessable:

```shell
git worktree add -b claude/wt-<n> .claude/worktrees/wt-<n> HEAD
cp -r <bootstrapped>/{asm,bin,disks,expected} .claude/worktrees/wt-<n>/
cp -r <bootstrapped>/tools/{asm-differ,maspsx,builder,m2c} .claude/worktrees/wt-<n>/tools/
cp <bootstrapped>/config/sym_export*.us.txt .claude/worktrees/wt-<n>/config/
mkdir -p .claude/worktrees/wt-<n>/{.venv,build,.variants}
```

* **`tools/m2c` is a fourth submodule and it is the one nobody lists.** Without
  it `./mako.sh dec` dies with `ModuleNotFoundError: No module named 'm2c.main'`,
  which reads like a broken venv rather than an empty submodule directory --
  and it only bites on the first *fresh* function, long after the worktree has
  been proved by a green `make build`. Copy it with the other three and drop
  the `.git` file the copy brings along.
* **`expected/` is needed too**, not just `asm/` -- `variant_eval.py` and
  `checkfn.py` compare against `expected/build/us/**.o`, so a worktree without
  it can build green and still be unable to *measure* anything.
* **`cp` resets every mtime, and that makes `make` re-extract the toolchain.**
  The rule chain is `bin/%: bin/%.gz: bin/%.gz.sha256`, so with all three
  stamped the same second make re-downloads the `.gz`, gunzips it, and then
  dies on `chmod +x`: `Operation not permitted`, because the Windows bind mount
  will not take a mode change. It also deletes the `.gz` on the way out, so the
  second attempt fails differently. Stamp the prerequisites old and the
  products new and make leaves the toolchain alone:

  ```shell
  rm -f bin/*.gz
  touch -d 2020-01-01 bin/*.sha256 bin/*.tag bin/clang-format.gz tools/str.c
  touch bin/cc1-psx-26 bin/cc1-psx-272 bin/clang-format bin/objdiff-cli-linux-x86_64 bin/str
  ```

**`tools/docker-build.ps1` hardcodes `-v ff7_build:/ff7/build`**, so out of the
box every worktree on Windows shares one build volume -- which is the exact
corruption this section warns about, silently. The file is gitignored, so each
worktree carries its own copy and each has to be patched; derive the volume
from the checkout so the main clone keeps `ff7_build` and nothing it already
has is orphaned:

```powershell
$buildVol = if ($env:FF7_BUILD_VOLUME) { $env:FF7_BUILD_VOLUME }
  elseif ($repoRoot -match '[\\/]\.claude[\\/]worktrees[\\/]([^\\/]+)') {
    'ff7_build_' + ($Matches[1] -replace '[^A-Za-z0-9_.-]', '_') }
  else { 'ff7_build' }
```

**The character class has to be `[\\/]`, and an earlier version of this
snippet said `[\/]`, which silently does the opposite of what it claims.**
Inside a .NET character class `\/` is an escaped forward slash, so `[\/]` is
just `[/]` — and `$repoRoot` on Windows is `C:\...\.claude\worktrees\<name>`
with backslashes. The match fails, the `else` arm fires, and **the worktree
shares `ff7_build` with the main clone**: precisely the corruption this
section exists to prevent, set up by the snippet that is supposed to prevent
it. Verify it rather than reading it — the whole check is one line, and it
costs nothing:

```powershell
$repoRoot = "<your worktree path>"
if ($repoRoot -match '[\\/]\.claude[\\/]worktrees[\\/]([^\\/]+)') {
    "ff7_build_$($Matches[1])" } else { "SHARED ff7_build -- WRONG" }
docker volume ls | Select-String ff7_build
```

The symptom, if you do not, is a link that fails on an object the same build
produced minutes earlier: `build/us/src/main/psxsdk.c.o: file not recognized:
file format not recognized`, on a unit you never touched, with `main.elf`
having linked from that same object successfully at step 6 of the same run.
Deleting the object and rebuilding "fixes" it until the next collision, so it
reads as flakiness rather than as a shared volume. A second tell is
`docker volume ls`: every other agent's worktree has an `ff7_build_<name>`
and yours does not.

The recovery is the fix plus one clean build — the new volume starts empty, so
the rebuild is from scratch and its 19x `OK` is the strongest verification
available. Re-run `checkfn.py` over everything the session landed afterwards:
a shared volume can hand a verdict that was measured against another agent's
object.

Prove the result before handing work to anyone: `make build` to 19x `OK` **and**
one `variant_eval.py` run whose row count matches what the source worktree
reports for the same function. The build alone is not enough -- an empty
`maspsx` or a missing `expected/` fails in ways that look like a result.

**`asm/` cannot be regenerated from scratch any more — copy it.** splat only
writes `nonmatchings/<fn>.s` for functions the `.c` still references through
`INCLUDE_ASM`; it knows nothing about `MASPSX_OVERRIDE`, whose expansion
`.include`s the same `.s` at assembly time. So on a tree with an empty `asm/`,
splat emits 83 of the field overlay's 260 files and the build dies with a
wall of

```
{standard input}:43480: Error: can't open asm/us/field/nonmatchings/field/FieldDebugInitBuffers.s for reading
```

naming exactly the functions that were converted to `MASPSX_OVERRIDE`. Nothing
points at splat, and re-running `make build` does not help. `cp -r <bootstrapped
tree>/asm/. asm/` fixes it; the tree is 24 MB and the files are a pure function
of the binaries plus `config/us.yaml`, so any bootstrapped worktree will do.

Only the disk files named by a `disk_path:` in `config/us.yaml` are needed, not
all of `disks/` — that is ~1.4 MB, cheap enough to copy rather than symlink
(and a Windows symlink into another tree would not resolve inside the build
container anyway).
