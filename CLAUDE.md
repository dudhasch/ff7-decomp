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

Build **one object** and diff it:

```shell
ninja build/us/src/battle/battle.c.o
.venv/bin/python3 tools/asm-differ/diff.py -o --format=plain func_800A85FC
```

Use `--format=json` when you need to parse the result programmatically; it gives
per-row diff classes (register mismatches, insertions, reorderings) rather than
text you have to scrape.

Repeat edit → `ninja` → `diff` until there are zero diff rows. Typical causes of
a near-miss, in rough order of frequency:

* **Register allocation differs** — usually means a temporary should (or should
  not) be a local variable, or the operand order in an expression is reversed.
* **Instruction order differs** — restructure the C (hoist/sink an assignment,
  change `if`/`else` polarity) rather than fighting the compiler.
* **Stack layout differs** — a local's type or declaration order is wrong.
* **Wrong compiler** — check the `//!` header (see *Compiler selection*).

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
ninja build/us/src/battle/battle.c.o
../decomp-permuter/import.py -o build/us/src/battle/battle.c.o func_800A85FC
../decomp-permuter/permuter.py func_800A85FC -j"$(nproc)"
```

`import.py` creates a self-contained scratch under `permuter/func_800A85FC/`
(target `.s`, current `.c`, and compile command); `permuter.py` then searches
until it prints a `(0)` base-score / perfect match or you stop it. Copy the
winning permutation back into the real `.c` file by hand — never commit
anything from the `permuter/` scratch directory — then re-run step 3 to
confirm `asm-differ` shows zero diff rows before moving to step 5.

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
| Function matches | `diff.py -o <fn>` | zero diff rows |
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
