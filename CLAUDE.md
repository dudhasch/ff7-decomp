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
See [docs/STEAMOS.md](docs/STEAMOS.md).

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
.venv/bin/python3 tools/worklist.py src/field/field.c -o docs/worklist-field.md
```

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
bytes). It exits non-zero unless every function named matched. Prefer it to
reading `diff.py` by eye — see *Two ways a clean-looking diff lies* below.

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
* **Wrong compiler** — check the `//!` header (see *Compiler selection*).

One near-miss that currently has no known fix: gcc hoists a global array's
address out of a loop where the original re-materialises it through the
assembler's `$at` macro each time. The C is otherwise byte-for-byte correct, so
`AddStrNextDebugRow` stays `INCLUDE_ASM`. Its former twin
`FieldDebugPagesResetPosSize` has since been solved and is plain C.

#### Four ways a clean-looking diff lies

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

#### Sweeping many variants at once

`checkfn.py` builds through ninja into `build/us/...`, a single shared path, so
it cannot be run concurrently: two agents editing the same `.c` overwrite each
other's object and read each other's verdict. When you want to sweep dozens of
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
silently score as "no change". The tool is currently hardcoded to
`func_801D080C`; point the constants at another function to reuse it.

### 3b. Check .rodata ownership before writing the C

Some functions cannot be decompiled alone, and the failure shows up as a red
`make build` while every function still diffs perfectly. Check first:

```shell
.venv/bin/python3 tools/rodata_owner.py src/field/field.c OpcodeFuncMenu2
# BORROWS  OpcodeFuncMenu2 -> D_800A0F38, owned by OpcodeFuncMenu (still INCLUDE_ASM)
```

* **BORROWS** — the function prints a string that another `.s` owns. Writing the
  literal makes gcc emit a *second* copy, shifting every later `.rodata` offset
  and breaking the overlay. Decompile the owner in the same change, or skip.
* **LENDS** — the function owns a label other `.s` files still reference.
  Decompiling it alone deletes the definition and the link fails with an
  undefined reference. `IfCheck` owns the `"ope err="` that both `If2Check*` use,
  so those three are one unit.
* **SHARES** — the owner is already C, so the two identical literals fold into
  one. Fine, as long as you pass exactly the same string.

Pass `--all` to triage a whole file at once.

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

**The residue is a file-split problem, not an alignment one.** The original
build compiled many small `.c` files, each with its own object and its own
`.rodata` base, and splat merges them into one unit. `src/field/field.c` is
several original files glued together — its own comments mark the seams
(`// Begin of field_event_memory_bank.c`) — so its `.rodata` carries *both*
phases at once: `jtbl_800A052C` is 4 mod 8 while the tables of already-matched
functions are 0 mod 8. One `--phase` setting cannot satisfy both, so these stay
`INCLUDE_ASM` until the file is split on the original boundaries: `IfCheck`,
`If2CheckSigned`, `If2CheckUnsigned`, `OpcodeFuncSetx`, `OpcodeFuncGetx`,
`OpcodeFuncSrchx`, `OpcodeFuncFade`, `OpcodeFuncFadew`, `OpcodeFuncSpcal`,
`FieldEventWriteMemoryU8` and `FieldEventRequestRun`.

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
by walking up from the target directory, so no extra flags are needed. Typical
usage:

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
see [docs/PERMUTER_MACROS.md](docs/PERMUTER_MACROS.md).

**The `permuter_strip_asm.py` step is not optional**, and skipping it is not
obvious from the output — the search just never converges. `import.py`
preprocesses the `.c`, so every `INCLUDE_ASM` has already expanded into a
`__maspsx_include_asm_hack_*` function holding `.include "<fn>.s"`, and those
land in `base.c` as `#pragma _permuter b64literal` blobs. The permuter decodes
them into every candidate, so each candidate object carries the whole overlay's
assembly — ~44,000 instructions for `src/field/field.c` — while `target.o` holds
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
(22 of 361 instructions), parked under `#else /* NON_MATCHINGS */` with its diff
and a hundred and thirty measured rejected phrasings written down. They are the
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
   when something structural changes. Rank candidates by the shape and confirm
   with the raw count.

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

**Write the finding down the first time.** Every gcc idiom, every environment
trap, every "this looked like a match but wasn't" belongs in this file or in
`docs/PERMUTER_MACROS.md` **in the same change that discovered it** — not at
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
| `tools/rodata_owner.py` | Whether a function can be decompiled without shifting `.rodata` |
| `tools/psx_jtbl_align.py` | Jump-table alignment fixup for units whose `.rodata` base is 4 mod 8 |
| `tools/permuter_macros.py` | Permuter scratch alignment, `PERM_*` recipes, search sizing |
| `docs/worklist-*.md` | Generated by `worklist.py` — regenerate per batch, never hand-edit |
| `disks/us/` | Extracted game files (generated, gitignored) |
| `asm/`, `build/`, `expected/` | All generated — never edit |

Overlays: `main`, `batini`, `battle`, `brom`, `dschange`, `ending`, `field`,
`bginmenu`, `cnfgmenu`, `savemenu`, `itemmenu`, `world`, `barrier`.

## Progress

```shell
make report     # true match% into build/report.json (SKIP_ASM build vs expected/)
make board      # sync the GitHub Projects board — HOST only, needs gh
```

The board at <https://github.com/users/dudhasch/projects/5> is **generated** from
`build/report.json`. Do not hand-edit issue bodies; the next sync overwrites them.
See [docs/BOARD.md](docs/BOARD.md).

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
splat emits 83 of `src/field/field.c`'s 260 files and the build dies with a
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
