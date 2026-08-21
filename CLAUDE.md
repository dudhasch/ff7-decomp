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

It hands `variant_eval.py` a no-op edit per `MASPSX_OVERRIDE` body, so each
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

### 2. Seed with m2c

```shell
./mako.sh dec func_800A85FC              # add --fix-structs for Savemap accesses
```

This replaces the function's `INCLUDE_ASM(...)` line in the `.c` with approximate
decompiled C, using the project's existing types and symbol names. It is a
starting point, not an answer — it will usually compile but rarely match.

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
reading `diff.py` by eye — see *Seven ways a clean-looking diff lies* below.

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
* **`*p = 0xFF` gives `li -1` through an `s8*` and `li 0xff` through a `u8*`.**
  The constant is narrowed to QImode against the store, and an HImode/QImode
  `const_int` is sign-extended, so `0xFF` becomes -1 and gcc materialises it
  with `addiu <r>,zero,-1` rather than `ori <r>,zero,0xff`. The stored byte is
  identical and the row is a real difference. Same trap as the `0xC600`
  narrowing bullet above, one width down.
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
* **A chained assignment stores right to left.** `m[0][0] = m[1][1] =
  m[2][2] = 0x1000;` is `m[0][0] = (m[1][1] = (m[2][2] = 0x1000))`, so the
  stores come out `m[2][2]`, `m[1][1]`, `m[0][0]` — descending. A target that
  fills a struct in descending address order, in groups, is three chained
  statements, not twelve separate ones; twelve give ascending order and a
  loop-hoisted constant besides. This is what the identity matrix in
  `HandleKawaiDataInModel` needs.
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
* **A local declared in an inner block takes a later stack slot.** Slots go in
  declaration order, and a block-scope declaration is "declared" where the block
  is: moving `MATRIX mtx;` from the function's locals into the `if` that uses it
  moved it from `0x18` to `0x20`, behind the `long` declared after it at
  function scope. Use it when a diff is nothing but `N(sp)` offsets in one
  branch.
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
* **A pointer bumped once per outer iteration belongs in the `for` increment.**
  `for (i = 0; i < n; i++, p += stride)` emits `i++` ahead of `p += stride`;
  written as the body's last statement the two come out in the other order and
  land in different branch delay slots. Eight rows across four loops, and no
  other spelling reaches it.
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
* **A hand-written `tag = base + 7` is an ordinary insn; `base[7]` is a giv.**
  The giv's initialiser is emitted into the loop preheader by
  `strength_reduce` and reorg leaves it there, while the hand-written pointer
  is an ordinary assignment reorg happily steals into the guard branch's delay
  slot. So a target with a `nop` after `beqz <count>` and the `addiu` *after*
  it wants the offset written at the use site, not carried in a second
  variable. Eight rows across `KawaiSetModelTransparency`'s eight loops.
* **A giv's increment follows its biv's, in written order.** With
  `for (j = 0; j < n; j++, base += stride)` the `j++` comes first and the
  reduced `base + 7` increment after it; `base += stride, j++` swaps both.
  Same lever as the two-counter bullet above, one level down. A loop whose
  `base` is dead afterwards matches either way, because gcc drops the biv
  increment entirely — so a run of loops where all but the last are two rows
  out is this.
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

* **A `do { } while (0);` is a free test for which pass you are fighting.** It
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

* **Do not guess at `n_refs` and `live_length` -- cc1 prints them.** The
  `.lreg` dump names every pseudo with exactly the two numbers
  `allocno_compare` ranks on, and `.greg`'s post-reload RTL shows which hard
  register each one ended up in, so a residue that reads as register naming can
  be turned into arithmetic instead of a guess:

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

  What you get per function is
  `Register 73 used 47 times across 538 insns; ...; pointer.` -- `n_refs` and
  `live_length` -- and `;; N regs to allocate: ...` at the top of the `.greg`
  entry, which is the allocno list already sorted by priority. Map a pseudo to
  its hard register by finding an insn number in `.greg` (post-reload, prints
  `(reg:SI 13 t5)`) and looking the same insn number up in `.lreg`
  (pre-allocation, prints `(reg:SI 73)`).

  **Caller-saved ties are `local_alloc`, and its formula is a different one.**
  A pseudo the dump describes as "in block N" never reaches `global_alloc` at
  all -- it is allocated by `block_alloc`, which ranks *quantities* by
  `QTY_CMP_PRI = floor_log2(n_refs) * n_refs * size / (death - birth)` and then
  hands each the lowest-numbered free register. So a rotation among `$v0`,
  `$v1`, `$a0`..`$a3` is a statement about that ratio and nothing else, and the
  `.lreg` dump prints both terms. `OpcodeFuncMove`'s three-quantity tie scores
  0.75 / 0.62 / 0.33 where the target needs the reverse order outright.

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

#### Seven ways a clean-looking diff lies

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

### 4. Last-mile: decomp-permuter

If step 3 stalls — zero diff rows are out of reach by hand, or you're stuck
permuting variable declarations/expression order yourself — hand the function
to [decomp-permuter](https://github.com/simonlindholm/decomp-permuter), an
external tool that brute-forces AST-level permutations of a function until the
compiled output matches. It is a search tool, not a substitute for
understanding: only reach for it once the C is *semantically* correct and the
remaining diff looks like compiler-specific register/ordering noise.

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
| `tools/rodata_owner.py` | Whether a function can be decompiled without shifting `.rodata` |
| `tools/asm_widths.py` | Per-symbol access width from a target `.s` — what an m2c seed's byte offsets have to be cast to |
| `tools/psx_jtbl_align.py` | Jump-table alignment fixup for units whose `.rodata` base is 4 mod 8 |
| `tools/affected_overlays.py` | Changed files → the overlays CI has to rebuild |
| `tools/permuter_macros.py` | Permuter scratch alignment, `PERM_*` recipes, search sizing |
| `tools/permuter_scratch.sh` | Build a permuter scratch and prove it is scoreable before searching |
| `tools/unpark.py` | Make one `MASPSX_OVERRIDE`'d body the live one, for import or measurement |
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
creating a fresh one per task.

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
