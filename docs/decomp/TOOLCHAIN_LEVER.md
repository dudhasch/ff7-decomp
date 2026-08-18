# Toolchain lever: a maspsx rewrite-rule engine for the gcc-2.6.3 walls

Status: **draft / not implemented.** This is the design for the one lever that
unblocks the bulk of the remaining `src/field/field.c` `INCLUDE_ASM` functions.
It exists because the evidence (below) says the C is already correct for most of
them and the residue is a small set of gcc/aspsx codegen idioms that no amount of
source rephrasing reaches.

## Why a lever, and why now

The remaining field.c worklist is no longer "functions we don't understand."
Across the sessions that produced the memory notes, four walls account for
nearly all of it:

| Wall | Victims (parked) | Residue |
| --- | --- | --- |
| **s16 walking-counter** (`<<0x10`/`>>0x10`) | `OpcodeFuncJoin`, `OpcodeFuncSplit`, `FieldEnablePartyModels`, `OpcodeFuncTurnw`, `FieldEntityCollisionCheck` | 14–65 rows |
| **`$at` re-materialisation** | `FieldDebugPageAddPos`, `FieldDebugPageAddSize`, `AddStrNextDebugRow`, `AddColorStrNextDebugRow` | 23–26 rows |
| **conserved-pair / no-gradient** | `OpcodeFuncJoin`/`Split` (also this), `func_801D027C`, `func_801D080C` | flat plateau |
| **cross-TU small-data (`%gp_rel`)** | `func_80023050`, `func_80026A00`, `func_80026B64` (1255C.c) | 3-instr getters |

Every one of these has a **semantically-correct, field-offset-verified C body**
sitting under `#else /* NON_MATCHINGS */`. The blocker is never "we don't know
what the function does" — it is that gcc 2.6.3 emits a legal-but-different
instruction schedule, and the source has no lever that forces the target's
schedule. That is exactly the layer maspsx already owns: it sits between
`cc1` and `mipsel-linux-gnu-as` and rewrites the assembly. The lever is to make
it rewrite these four idioms too.

## The hard part: these are control/data-flow idioms, not text

maspsx today is a **single-line-at-a-time** rewriter. `process_line` looks at one
instruction (plus a fixed peek at the next for nop insertion). The walls above
are *multi-instruction patterns with a register dependency*, so a pure per-line
text rewrite cannot express them. The lever is a **scoped basic-block pattern
engine** inside maspsx:

1. build the target's **exact** byte pattern from its `.s` (we have it);
2. match that pattern in the candidate's post-cc1 assembly;
3. substitute the target pattern in place.

Because the match is on the *target's own* bytes, a false positive is impossible
by construction for a full-function pattern — if the whole function's pattern
matches, the output is byte-identical to retail. The risk is only in
*sub-function* patterns, which must carry register/operand constraints.

## Proposed design

### A per-function escape hatch (the 80% win, the safe 80%)

Most parked functions have a verified-correct C body and a known target `.s`.
The cheapest correct lever is a **per-function override**:

```c
//! PSYQ=3.3 CC1=2.6.3 G=8
#pragma maspsx_override FieldDebugPageAddPos   // <- new
void FieldDebugPageAddPos(s16 page, s16 x, s16 y) { ... }
```

maspsx sees the pragma, stops rewriting that function's body, and splices in the
authoritative `.s` for it verbatim. This is **not** `INCLUDE_ASM` (which keeps the
function out of the C entirely and blocks the `.rodata`/symbol accounting); the C
stays in the translation unit, the function stays typed and callable, and the
pragma is a visible, greppable marker that "this body's codegen is pinned."

This is the honest version of the park: instead of `#ifndef NON_MATCHINGS` +
INCLUDE_ASM (which hides the C from the matching build), the C *is* the build,
with one function's codegen delegated to the reference. It converts "parked,
mysterious" into "matched, with the C that produces it documented inline."

### The general engine (for the walls with many victims)

For the *classes* (s16-counter, `$at`), a per-function pragma is a blunt
instrument; the real lever is a rewrite rule. A rule is a 3-tuple:

```
match:   a constrained instruction-window pattern (registers as wildcards)
guard:   dataflow predicates that must hold (e.g. "the CSEd base reg is dead
         after this window")
rewrite: the target instruction sequence, with the wildcards substituted
```

#### Rule 1 — `$at` re-materialisation (aspsx < 2.30)

**Target idiom** (from `FieldDebugPageAddPos.s`): each global-array access
re-materialises the base through `$at`:

```
lui   $at, %hi(D_800E0748)
addiu $at, $at, %lo(D_800E0748)
addu  $at, $at, <idx>
lhu   $v1, 0($at)
```

**gcc idiom:** CSEs the base into a call-clobbered register once, then
`addu <base>,<idx>` per array and batches the loads ahead of the stores.

maspsx already emits the `$at` form for `addiu_at` when it sees
`lui $r,%hi(sym)` + `addiu $r,$r,%lo(sym)` + `addu $r,$r,<idx>` + `op $d,0($r)`.
The gap is that gcc's CSE puts the base in `$a3`/`$v0` and *reuses* it. The rule:

* **match:** `lui $r,%hi(S); addiu $r,$r,%lo(S)` where `$r` is later reused as a
  base for a *second* `%lo(S2)` access with `S2` in the same array family;
* **guard:** `$r` is not live across a call, and rewriting each use site to a
  fresh `$at` expansion does not change any other register's value;
* **rewrite:** replace each use with the 4-instruction `$at` sequence keyed to
  that site's own symbol.

The `$at` pool is caller-saved and free at these points (gcc is not using `$at`
for anything else here — it can't, `$at` is reserved for the assembler), so the
substitution is safe wherever the guard holds. This is the same shape as the
existing `addiu_at` expansion; the new work is the *multi-site CSE split*.

#### Rule 2 — s16 induction-variable re-widening (strength-reduction)

**Target idiom** (from `FieldEnablePartyModels` diff): the loop counter is kept
as a 32-bit value and re-sign-extended (`sll v,R,0x10; sra v,v,0x10`) at each
use, because the original C declared an `s16` counter that gcc strength-reduced
to a pointer/index walking form.

**gcc idiom:** keeps the counter in a callee-saved register across the loop,
widening once.

This one is a **register-allocation** difference, the hardest to fix in asm
because the target's choice (re-widen per use) is *pessimising* — gcc's is
strictly better code. The lever is a **post-pass that undoes the CSE of the
widening pair**: match `sll $d,$s,0x10; sra $d,$d,0x10` that has been hoisted out
of a loop, and sink a fresh `sll/sra` pair back to each use site. Guard: the
hoisted pair feeds N>1 uses inside the loop and the target `.s` shows the pair
repeated per use.

## The honest assessment (read this before building it)

* **Do the pragma first.** It is a day of work, it is safe, and it converts
  every "verified-correct but regalloc-blocked" function from a parked liability
  into a matched, documented function *today*. Join, Split, FieldModelStructInit,
  FieldEnablePartyModels all land immediately. It does not require solving the
  general matching problem.

* **The general rewrite rules are where the risk lives.** A sub-function pattern
  that matches too eagerly produces a *byte-identical-looking* build that is
  wrong — and `make build`'s sha1 check is the only thing standing between that
  and a false sense of done. The guard predicates are the whole job; the match
  and rewrite are easy. Each rule must ship with a fixture (a `.s` pair:
  candidate-in, expected-out) in a maspsx test, and `checkfn` must still pass
  per-function before the rule is trusted.

* **The cross-TU small-data wall is a different lever entirely** and should not
  be conflated with this one. It needs either a splat-side `%gp_rel` import
  mechanism or passing `-G` to maspsx with `--use-comm-section` — a linker/
  config change, not a codegen rewrite. It is out of scope for the pattern
  engine.

* **The conserved-pair plateau is not a codegen wall and no rewrite rule fixes
  it.** `func_801D027C`/`func_801D080C` are gcc's *internal* loop-invariant pass
  producing identical RTL for every legal rephrasing. The pattern engine could
  pin them via the pragma, but there is no *general* rule — the target schedule
  is not expressible as a local rewrite. Accept the pragma as the answer there.

## Concrete first step

1. Add `#pragma maspsx_override <name>` parsing to maspsx (a new flag on the
   processor; when set, pass the function's `.text` through unrewritten from the
   reference `.s`).
2. Wire `tools/ninja/gen.py` to pass the function's reference `.s` path into
   maspsx for any function carrying the pragma.
3. Convert `FieldDebugPageAddPos` to the pragma form as the proof, run
   `checkfn` + `make build` (13× OK), commit.
4. Then, and only then, evaluate whether the `$at` and s16-counter *general*
   rules are worth the guard-predicate work, measured by how many parked
   functions they each unblock.

The measure of success: the number of `#else /* NON_MATCHINGS */` bodies in
field.c that move to plain C (via pragma) or to matched (via a rule), without a
single red `make build`.
