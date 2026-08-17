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

## The decompilation loop

### 1. Pick a function

```shell
./mako.sh rank src/battle/battle.c
# 0.355: func_800A85FC.s     <- lowest score first
```

The score is a difficulty model (0 = easy, 1 = hard) trained to predict one-shot
decompilation success. **Always work in ascending score order.** Do not start at
the top of the file; do not pick by name.

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
* **Wrong compiler** — check the `//!` header (see *Compiler selection*).

#### Two ways a clean-looking diff lies

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

#### Known blocker: a string immediately followed by a jump table

gcc emits `.rdata` / `.align 3` before every jump table, so GNU `as` puts the
table on the next 8-byte boundary. Where the original has a string constant
immediately before the table it instead sits 4 bytes further on, with 4 zero
bytes in between (`spimdisasm` renders them as a stray `.asciz ""`). An
8-byte alignment cannot produce that offset, so the original toolchain must
have used 4-byte alignment plus a real 4-byte item, and maspsx has no knob for
it. Until that is resolved these stay `INCLUDE_ASM` however good the C is —
in `src/field/field.c` that is `IfCheck`, `If2CheckSigned`, `If2CheckUnsigned`,
`OpcodeFuncSetx`, `OpcodeFuncGetx`, `OpcodeFuncSrchx`, `OpcodeFuncFade`,
`OpcodeFuncFadew`, `OpcodeFuncSpcal`, `FieldEventWriteMemoryU8` and
`FieldEventRequestRun`. Functions whose jump table does *not* follow a string
are unaffected and match normally.

### 4. Last-mile: decomp-permuter

If step 3 stalls — zero diff rows are out of reach by hand, or you're stuck
permuting variable declarations/expression order yourself — hand the function
to [decomp-permuter](https://github.com/simonlindholm/decomp-permuter), an
external tool that brute-forces AST-level permutations of a function until the
compiled output matches. It is a search tool, not a substitute for
understanding: only reach for it once the C is *semantically* correct and the
remaining diff looks like compiler-specific register/ordering noise.

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
.venv/bin/python3 ../decomp-permuter/permuter.py nonmatchings/func_800A85FC \
    -j"$(($(nproc) - 2))"
```

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
| `tools/checkfn.py` | Per-function match verdict; use instead of eyeballing `diff.py` |
| `tools/rodata_owner.py` | Whether a function can be decompiled without shifting `.rodata` |
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
