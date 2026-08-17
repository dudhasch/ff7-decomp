# Directed permuter searches

decomp-permuter has two modes. Left alone it **randomizes**: an unbounded random
walk over AST mutations, with no memory and no termination. Given `PERM_*`
macros it **enumerates**: the macros describe a finite set of candidates, the
permuter tries each one exactly once and then stops
(`src/perm/eval.py`, `if not perm.is_random(): break`).

This document is about using the second mode on this project — writing macros
that encode what gcc 2.6.3/2.7.2 actually does on this target, so the search is
a checklist rather than a lottery.

Two things have to be true before either mode is worth starting:

1. the scratch scores the **function**, not the overlay — `tools/permuter_strip_asm.py`, see CLAUDE.md step 4;
2. the scratch scores **code**, not symbol names — `tools/permuter_macros.py align`, below.

The second one is new, and it is the bigger trap of the two.

## Fix the score before you search

The scorer disassembles both objects with `objdump -drz` and diffs the text, so
a relocation renders as the *name* of the symbol it points at. The generated
`asm/` names every global after its address (`D_8009D820`); the C names it
`g_DebugLevel`. Same address, same bytes, different text — 5 points, on every
single reference, forever.

`OpcodeFuncSwcol` is a function this repo has already matched. Feeding its
committed, byte-identical C to an un-aligned scratch:

| base.c | score |
| --- | ---: |
| as imported | 50 |
| after `align` (3 globals renamed to their `D_` form) | 20 |
| after `align --strings` (string literal → the `.rodata` symbol target.s uses) | 10 |
| after the interior-address rewrite `align` suggests | **0** |

Three consequences, all of which cost real time if you don't know about them:

* `--stop-on-zero` never fires, so a finished search doesn't announce itself.
* Scores are not comparable between functions — one that touches six renamed
  globals starts 30 points above one that touches none.
* You cannot tell "5 points of noise" from "one wrong register", so you keep
  searching after the answer has already been found. The randomization log for
  this very function sat at 110 for 31 outputs; 50 of those 110 were noise.

So, once per scratch:

```bash
.venv/bin/python3 tools/permuter_macros.py align nonmatchings/<Func> --strings
.venv/bin/python3 tools/permuter_macros.py retarget nonmatchings/<Func>
```

It rewrites only the scratch's `base.c` (keeping the original as
`base.c.prealign`), never `src/`. Three kinds of mismatch, two of them
automatic:

* **renamed globals** — `g_DebugLevel` → `D_8009D820`, resolved through
  `config/sym_extern.us.txt` and `config/symbols.*.txt`. Applied.
* **string literals** — the target references a named `.rodata` symbol,
  your candidate emits its own anonymous one. `--strings` pairs them in order
  and adds `extern char D_800A0E30[];`. Applied. Check the pairing if the
  function has more than one literal.
* **interior addresses** — splat named `D_80049209` where your C writes
  `D_80049208[i + 1]`. Applied when the index expression is recognizable:
  the access becomes `(&D_80049209)[i]` with a matching `extern u8` declaration.
  Codegen is identical; only the relocation's name changes. Irregular
  expressions are reported for a hand rewrite instead.

The aligned `base.c` uses address names, so when a candidate wins, translate the
names back on the way into `src/` — `base.c.prealign` is the key.

### Two kinds `align` does not reach

A file whose data is `static` has a fourth kind, and some functions have a
fifth that cannot be fixed at all. Both were found on `func_801D080C`
(`src/menu/cnfgmenu.c`); `tools/permuter_externise.py` handles the first.

* **file-scope statics** — they are *defined* in the candidate object, so the
  assembler relocates against the section (`.data+0xa10`), while target.s,
  which holds only the function, relocates against the name
  (`D_801D24B8+0x4`). Dropping the definitions to `extern` makes them undefined
  in the candidate too. The scratch is never linked, so losing the data costs
  nothing, and it is codegen-neutral: it only changes how asm-differ
  *classifies* rows, turning an insert+delete pair into a reordering.

* **interiors of a named struct** — splat calls the field at 0x8009D7BE
  `D_8009D7BE`; the C calls it `Savemap.config`, relocating as
  `Savemap+0x10da`. **Do not rewrite these.** A standalone `extern u16` stops
  gcc CSE-ing the struct's base address into a register, so one `lui/addiu`
  plus `lhu 0(reg)` per access becomes a `lui`+`lo` pair: 805 → 9620 on that
  function. The same trap catches any attempt to merge two adjacent scalars
  into one aggregate so an offset rides in the relocation's addend.

Compiler-generated jump tables have no name to align to either. So a function
can have a **nonzero noise floor** — 280 for `func_801D080C` — and then
`--stop-on-zero` never fires however close you get. Read the floor off the
`--debug` two-column diff (rows differing only in symbol text) and watch for
that number instead of zero.

Worth knowing alongside this: the aligned score is a *finer* instrument than
`tools/checkfn.py`, whose alias discounting is address-based. On that function
it hid an 85-point regression entirely. Use checkfn as the gate and the aligned
score to compare variants.

## The loop

```bash
# 1. import, then the two corrections
.venv/bin/python3 ../decomp-permuter/import.py src/field/field.c \
    asm/us/field/nonmatchings/field/FieldButtonsUpdate.s
.venv/bin/python3 tools/permuter_strip_asm.py nonmatchings/FieldButtonsUpdate
.venv/bin/python3 tools/permuter_macros.py align nonmatchings/FieldButtonsUpdate --strings
.venv/bin/python3 tools/permuter_externise.py nonmatchings/FieldButtonsUpdate/base.c  # if the file's data is static
.venv/bin/python3 tools/permuter_macros.py lint nonmatchings/FieldButtonsUpdate

# 2. see what is actually wrong, in the scorer's own terms -- keep the output
.venv/bin/python3 ../decomp-permuter/permuter.py nonmatchings/FieldButtonsUpdate \
    --debug 2>&1 | tee nonmatchings/FieldButtonsUpdate/debug.txt

# 3. let the tool read the penalty list and rank the recipes
.venv/bin/python3 tools/permuter_macros.py suggest nonmatchings/FieldButtonsUpdate
.venv/bin/python3 tools/permuter_macros.py recipe temp-hop   # paste into base.c

# 4. check how big a search you just described
.venv/bin/python3 tools/permuter_macros.py count nonmatchings/FieldButtonsUpdate -j 8

# 5. run it -- a finite space ends by itself
.venv/bin/python3 ../decomp-permuter/permuter.py nonmatchings/FieldButtonsUpdate \
    -j 8 --stop-on-zero --best-only
```

`suggest` parses the saved `--debug` output (penalty list plus the two-column
diff, target on the left) and cross-references `base.c` — so it notices things
like an a-register shift combined with a parameter the body never uses, which
is the `fake-arg` signature. On the three functions solved so far it ranks the
winning recipe first each time. It is a heuristic reading of the same evidence
you would read by eye; treat it as a starting order, not an oracle.

Then copy the winner into `src/`, undo the alignment renames, and verify for
real: `tools/checkfn.py` → `make build` → `make format`. A candidate that scores
0 in the permuter is a hypothesis about the source, not a result.

`--debug` compiles and scores the base once and prints the penalty list —
register differences, reorderings, insertions, stack differences. That list is
the input to choosing a recipe; `recipes` is indexed by it.

Add `--stack-diffs` whenever the penalty list shows stack differences: without
it the scorer ignores sp-relative offsets entirely, so `decl-order` has no
gradient to climb and every candidate looks identical.

## Budgeting the search

Each candidate is a full cc1 + maspsx + as + objdump round trip. Measured on
this project's scratches, a 24-core box sustains roughly **0.8 candidates per
second per worker** on a short `src/field/field.c` function — about 8/s at
`-j 8`. `count` uses that figure; measure your own with a `-j 1` run if a
function is much larger.

| perm_count | at `-j 8` | verdict |
| ---: | --- | --- |
| ≤ 1,000 | ≤ 2 min | free; just run it |
| ≤ 50,000 | ≤ 2 h | fine for an overnight or a lunch break |
| > 200,000 | > 7 h | too big — fix one dimension at a time |

PERM macro spaces **multiply**. Four types × a 3-line `PERM_LINESWAP` (3! = 6) ×
two `PERM_GENERAL` alternatives is already 48; add a second 4-way type sweep and
it is 192. That is still nothing, which is the point: an exhaustive 192 is worth
more than 10,000 random candidates, because the 192 are the ones you had a
reason to try.

The discipline that keeps it small is `PERM_VAR`: define the choice once and
read it at every site, so *n* sites cost `len(choices)` candidates instead of
`len(choices)**n`.

```c
PERM_VAR(HOP, PERM_GENERAL(u8, s16, u16, s32))   /* set once, above the function */
    PERM_VAR(HOP) a;                             /* read anywhere below */
    PERM_VAR(HOP) b;
```

## The recipes

`tools/permuter_macros.py recipes` lists them, `recipe <name>` prints one ready
to paste, with the reasoning attached. Indexed by what `--debug` tells you:

| Penalty list says | Recipe |
| --- | --- |
| register differences, 1–2 rows | `temp-hop`, `cast-width`, `param-copy` |
| stack differences | `decl-order` (with `--stack-diffs`) |
| reorderings | `hoist`, `stmt-order` |
| branch differences | `branch-polarity`, `loop-form` |
| right instructions, wrong base register | `addr-form` |
| loads batched ahead of stores | `struct-store` |
| a-register off by one and a parameter is unused | `fake-arg` |
| nothing left you can explain | `region-random` |

The catalogue is not generic permuter advice — each entry comes from a
near-miss this repo actually hit. `temp-hop` is the one to reach for first:
`OpcodeFuncSwcol` needed an `s16` between a call result and a multiply, and
`FieldDebugIntToString` needed a `u8` between an array lookup and a store
(commit d0a549d). Both are a single narrow slot that changes which register
gcc picks, and in the second case the load/store order too.

### Worked example

`OpcodeFuncSwcol`, aligned as above, scoring 60. The penalty list showed
register differences only, so: sweep the type of an intermediate, and let the
declaration order float.

```c
PERM_VAR(HOP, PERM_GENERAL(u8, s16, u16, s32))
s32 OpcodeFuncSwcol(void)
{
  PERM_LINESWAP(
  PERM_VAR(HOP) hop;
  s32 corner;
  )
  if (D_8009D820 & 3)
  {
    DebugPrintOpcode(D_800A0E30, 6);
  }
  PERM_GENERAL(corner = FieldEventReadMemoryU8(1, 3) * 3;,
               hop = FieldEventReadMemoryU8(1, 3); corner = hop * 3;)
  ...
```

4 types × 2! declaration orders × 2 forms = 16 candidates. `count` agrees, and
so does the permuter: `Will run for 16 iterations.` It hit 0 on the 4th, in
under three seconds, and the winner is the `s16` form this repo committed by
hand months earlier.

Pure randomization on the identical aligned scratch also gets there — one
dimension, and `perm_temp_for_expr` is the randomizer's highest-weighted pass —
but it takes what a random walk takes: six runs on the same scratch finished in
1.9 s, 9.8 s, 37 s, 41 s, 68 s and about 100 s. The macro run is 2 s every time,
and, more usefully, it is *over* after 16 candidates whether or not it won.

### Worked example: a function randomization did not solve

`FieldModelCreatePktsAndScale` was `INCLUDE_ASM` in `src/field/field.c` when this
search was run. An earlier randomization run on it improved 205 → 165 and
stopped there. `--debug`
on the aligned scratch shows 21 register differences and one deletion — no
reorderings, no stack differences — so the C is structurally right and something
about the loop is holding a value in the wrong register.

Three hypotheses, one macro each: where `part` is computed (`hoist`, 3 sites),
declaration order (`decl-order`, 2), loop shape and index form (`loop-form` +
`addr-form`, 4), and the counter type (`temp-hop`'s `PERM_VAR` sweep, 4).
96 candidates, an estimated 15 seconds.

It found a zero on the 4th:

```c
u8 *FieldModelCreatePktsAndScale(FieldModelEntry *model, u8 *pkts, s32 arg2) {
    u32 i;
    u8 *part;

    part = model->modelData + model->partsOffset;
    model->partMatrices = pkts;
    pkts += model->boneCount * 32;
    for (i = 0; i < model->partCount; i++) {
        pkts = FieldModelCreatePktsForPart(part + (i * 0x20), pkts, 0, arg2);
    }
    FieldModelScaleModel(model, model->scale, 0);
    return pkts;
}
```

Three of the four dimensions moved: `part` is computed *before* the two stores,
the declarations are the other way round, and the loop indexes `part + i * 0x20`
instead of advancing a pointer. No single one of those is a match; the randomizer
had to land all three at once, and in 165-score terms it never did.

That is the whole argument for macros. Each extra lever multiplies the
randomizer's difficulty and merely *adds* to the enumeration: 96 candidates
covers three dimensions exhaustively, and a fourth would cost 384.

### Worked example: a fix the randomizer cannot express

`UpdateFieldExitArrows` (also `INCLUDE_ASM` at the time): after alignment, the entire
residue is five rows where the cached `&D_8009D5A6` sits in **a1** in the target
but **a0** in the candidate. The function's `arg0` is never used — it dies at
entry, freeing a0, and gcc 2.6.3 hands the freed register to the next temp.
`suggest` flags exactly this pairing and ranks `fake-arg` first.

The fix keeps the parameter alive by passing it through to the call —
`DrawFieldExitArrow(arg0)` — which costs zero instructions (the value is
already in a0, the callee's declaration is unprototyped and it ignores the
extra argument) but occupies a0 all the way to the call, forcing the temp into
a1. Two candidates; zero on the second:

```c
void UpdateFieldExitArrows(s32 arg0) {
    if (g_FieldState->newActiveKeys2 & (1 << 8)) {
        D_8009D5A6[0] ^= 1;
    }
    if (((D_8009D5A6[0] == 1) && (g_FieldState->characterLock == 0)) ||
        (D_8009D5A6[0] & 2)) {
        DrawFieldExitArrow(arg0);
    }
}
```

This one is qualitatively different from the others: randomization *cannot*
find it, at any budget, because no randomizer pass adds arguments to calls. A
tuned-weights randomization run on the same aligned scratch went 1,100+
candidates without progress. Some of gcc 2.6.3's levers simply live outside
the randomizer's mutation space — source-level hypotheses are the only way to
pull them.

(The original almost certainly really did pass the argument down; the callee
ignoring a parameter is ordinary code evolution, not a trick.)

### A score of 0 is not the same as a match

The scorer normalizes stack offsets and immediates away by default — `addiu
sp,sp,imm`, `lw ra,addr(sp)`. Re-check every winner with `--debug --stack-diffs`
before believing it, then take it through `tools/checkfn.py` and `make build`,
which is the only gate that counts.

## Writing macros without breaking the scratch

* Arguments split on commas **outside parentheses**. `f(1, 3)` inside an
  argument is safe; a bare `s16 a, b;` is not — write `(,)` for a literal comma.
* An empty argument is legal, and is how you make an alternative optional:
  `PERM_GENERAL(, (u8))`.
* `PERM_VAR(NAME, value)` expands to nothing and may sit at file scope; it must
  appear textually before every `PERM_VAR(NAME)` that reads it.
* `PERM_ONCE(key, stmt)` must appear at **two or more** sites with the same key
  — the permuter raises on a single occurrence.
* Any `PERM_RANDOMIZE` anywhere makes the whole space infinite; the run stops
  only on `--stop-on-zero` or your patience. Exhaust the finite macros first.
  Before a randomization run, `permuter_macros.py weights <scratch>` writes
  R3000A-tuned pass weights into the scratch's `settings.toml`: the FPU and
  IDO-specific passes zeroed (the R3000A has no FPU; gcc ignores
  line-placement), and the passes that have actually produced matches here
  boosted. Finite PERM spaces ignore weights entirely.
* Macros must be plain text in `base.c`. Inside a `#pragma _permuter b64literal`
  line they are invisible to the parser and silently do nothing.
* An alternative that leaves a variable unused is fine — gcc drops it. Do not
  hand-prune those until the function matches; then re-verify with `checkfn.py`.

`lint` checks the ones that fail silently: leftover INCLUDE_ASM blobs, macros
buried in b64 literals, unknown `PERM_` names, single-site `PERM_ONCE`, a space
that describes exactly one candidate, and a `func_name` that isn't in `base.c`
at all.

## `retarget`: fixing the side you are allowed to change

`align` makes the *candidate* use the target's names. It cannot help when the
target names an **interior of a named struct**: splat calls the halfword at
0x8009D7BE `D_8009D7BE`, the C calls it `Savemap.config`, and rewriting the C to
reach it through a standalone extern stops gcc caching the struct base in a
register — the score goes up, not down, because the codegen genuinely changed.
Alignment must never change codegen.

The scratch's `target.s` is not a generated file of this repo, though; it is
ours. Rewriting `D_8009D7BE` there to `Savemap+0x10da` leaves an identical
relocation against an identical address, so both sides render the same text and
those rows stop being scored — with no effect on the candidate whatsoever.

```shell
.venv/bin/python3 tools/permuter_macros.py retarget nonmatchings/<Func>
```

Which addresses get rewritten is read out of the candidate object, never
guessed from the symbol table. That matters: on `func_801D080C` the nearest
symbol preceding `Savemap+0x10da` is `g_RainControl`, an unrelated object 0xb1
bytes earlier, so a nearest-preceding heuristic would have produced a confidently
wrong `g_RainControl+0xb3`. Instead the map comes from the R_MIPS_LO16
relocations in `base.o`, whose addends live in the instruction immediates
(MIPS o32 is REL). An address the candidate does not already spell as
`NAME+0xOFF` is left alone, so the command can only ever make the two sides
agree on something they already agree on numerically. Symbols `align` has
already handled are skipped, and `target.o` is reassembled — it is what the
permuter actually scores against, so a rewrite that skipped it would do nothing.

On `func_801D080C` this rewrote 44 references and took the base score from 596
to 220: register differences 54 → 8, stack differences 6 → 0, and an
insertion/deletion pair collapsed into a reordering because the two sides now
line up. The share of the score that is real rather than naming noise went from
54% to 82%. Do this before measuring the floor, or the floor you measure is
mostly noise you could have removed.

## What the permuter is not

It searches the space you describe. It has no idea what the function means, and
a candidate that matches by luck is still wrong C until `checkfn.py` and
`make build` agree. Functions blocked for structural reasons — a string
immediately followed by a jump table, `.rodata` borrowed from an
`INCLUDE_ASM` neighbour (`tools/rodata_owner.py`) — stay blocked no matter how
good the score gets; check those before spending an afternoon on a search.
