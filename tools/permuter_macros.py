#!/usr/bin/env python3
"""Directed decomp-permuter searches: symbol alignment, PERM macro recipes.

    .venv/bin/python3 tools/permuter_macros.py align nonmatchings/OpcodeFuncSwcol
    .venv/bin/python3 tools/permuter_macros.py suggest nonmatchings/OpcodeFuncSwcol
    .venv/bin/python3 tools/permuter_macros.py recipes
    .venv/bin/python3 tools/permuter_macros.py recipe temp-hop
    .venv/bin/python3 tools/permuter_macros.py count nonmatchings/OpcodeFuncSwcol
    .venv/bin/python3 tools/permuter_macros.py lint nonmatchings/OpcodeFuncSwcol
    .venv/bin/python3 tools/permuter_macros.py weights nonmatchings/OpcodeFuncSwcol

Run `align` and `lint` once after every `import.py` + `permuter_strip_asm.py`,
then paste a recipe from `recipe` into the scratch's base.c and check the size
of the search you just described with `count`.

Why bother, when the permuter randomizes on its own?

  * Randomization is an unbounded random walk with no memory: the same
    hypothesis gets re-derived over and over, and nothing tells you when the
    space you care about has been covered. A base.c holding only PERM macros
    has a *finite* space, which decomp-permuter enumerates exhaustively and
    then stops (src/perm/eval.py: `if not perm.is_random(): break`). Sixteen
    candidates that each encode a real hypothesis about gcc 2.6.3 beat ten
    thousand random ones.

  * The randomizer's passes are generic. This project's near-misses are not:
    they cluster into a handful of shapes (narrow typed temporaries, stack
    slot order, address forms, store targets) that the recipes below encode
    directly. See `recipes` for the catalogue and docs/decomp/PERMUTER_MACROS.md for
    the evidence behind each one.

`align` exists because of a trap specific to this repo: the scorer compares
*symbol names*, and the generated asm/ names every global after its address
(`D_8009D820`) while the C uses the project's real name (`g_DebugLevel`). Every
such reference costs 5 points forever, so a byte-perfect function can sit at a
nonzero score and `--stop-on-zero` never fires. On OpcodeFuncSwcol -- a
function this repo has since matched -- the committed, byte-identical C scores
50 in an un-aligned scratch, 20 after `align`, and 0 after `align --strings`.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from typing import Dict, List, Optional, Set, Tuple

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# `NAME = 0x8009D820;` with an optional trailing comment, the shared format of
# config/sym_extern.us.txt, config/sym_ovl_export.us.txt and config/symbols.*.txt.
SYM_LINE_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")

# Symbols spimdisasm invents for un-named addresses, which is what the target .s
# almost always uses.
ADDR_NAME_RE = re.compile(r"^(?:D|func|jtbl|jpt)_([0-9A-Fa-f]{8})$")

# Symbol references in a target .s: %hi()/%lo() relocations, jumps and .word.
TARGET_SYM_RES = [
    re.compile(r"%hi\(([A-Za-z_]\w*)"),
    re.compile(r"%lo\(([A-Za-z_]\w*)"),
    re.compile(r"^\s*(?:jal|j|b)\s+([A-Za-z_]\w*)", re.M),
    re.compile(r"^\s*\.word\s+([A-Za-z_]\w*)", re.M),
]

PERM_MACRO_RE = re.compile(r"(PERM_[A-Z_]+)\(")
B64_RE = re.compile(r"#pragma _permuter b64literal (\S+)\s*$")

# Measured on this project's scratches: a 24-core box running
# `permuter.py -j 8` on a ~15-line field.c function sustains roughly this many
# candidates per second, container startup included. Scale it with -j.
CANDIDATES_PER_SEC_PER_JOB = 0.8


# --------------------------------------------------------------------------
# recipes
# --------------------------------------------------------------------------

class Recipe:
    def __init__(self, name: str, symptom: str, count: str, body: str, note: str):
        self.name = name
        self.symptom = symptom
        self.count = count
        self.body = body.strip("\n")
        self.note = note.strip("\n")


RECIPES: List[Recipe] = [
    Recipe(
        "temp-hop",
        "one or two rows differ only in which register holds a value, or a load "
        "sits on the wrong side of a store",
        "len(types) x 2",
        """
/* Sweep the type of an intermediate that catches a call result or a load
 * before it is used. Place the PERM_VAR anywhere above the function. */
PERM_VAR(HOP, PERM_GENERAL(u8, s16, u16, s32))
...
    PERM_VAR(HOP) hop;
    ...
    PERM_GENERAL(corner = FieldEventReadMemoryU8(1, 3) * 3;,
                 hop = FieldEventReadMemoryU8(1, 3); corner = hop * 3;)
""",
        """
The single highest-yield shape in this repo, confirmed twice: OpcodeFuncSwcol
needed `s16 pane = FieldEventReadMemoryU8(1, 3);` before the multiply, and
FieldDebugIntToString needed a `u8` between an s32 array lookup and the store
(commit d0a549d). gcc 2.6.3 picks a different register, and sometimes a
different load/store order, once the value passes through a narrower slot.

PERM_VAR keeps one type choice consistent across every site that uses it, so
n sites cost len(types) candidates rather than len(types)**n.
""",
    ),
    Recipe(
        "decl-order",
        "`Stack Differences` in the penalty list, or sp-relative offsets differ",
        "n!",
        """
PERM_LINESWAP(
s16 pane;
s32 corner;
u8 flags;
)
""",
        """
Run with `--stack-diffs` while chasing this, always: the default scorer ignores
sp-relative offsets entirely, so without the flag the recipe has no gradient to
climb, every candidate looks equally good, and -- worse -- the search spends its
whole budget on the rest of the function while reporting improvements that
measure worse. That is not hypothetical; it cost a full overnight run on
func_801B009C.

Try it, but do not trust it: it is *not* true that gcc 2.6.3 assigns stack
slots in declaration order. On func_801B009C all five permutations of the
declaration block, and `register` on every subset of it, changed the allocation
by not one instruction. Slots are assigned by reload's spill order, which
follows allocation priority, not source order.

The lever that does move slot assignment is which values become reduced
induction variables and in what order gcc scans them -- so when this recipe
comes up flat, the residue is usually reachable through `giv-hoist` or
`cse-split` instead, and this block is the wrong axis rather than a hard one.
""",
    ),
    Recipe(
        "hoist",
        "`Reorderings` in the penalty list -- the right instructions, in the "
        "wrong order",
        "k (number of sites)",
        """
/* Same key in every slot: exactly one of them survives per candidate. */
PERM_ONCE(pos, part = model->modelData + model->partsOffset;)
model->partMatrices = pkts;
PERM_ONCE(pos, part = model->modelData + model->partsOffset;)
pkts += model->boneCount * 32;
PERM_ONCE(pos, part = model->modelData + model->partsOffset;)
""",
        """
Cheaper and far more targeted than letting perm_reorder_stmts find the same
move at random: k candidates, one per landing site, and the statement is
guaranteed to appear exactly once.

The field opcode handlers are full of this: whether `PC_INC(n)` sits before or
after the return value, and where the first GET_PARAM_U8 lands, both move whole
runs of instructions.
""",
    ),
    Recipe(
        "stmt-order",
        "`Reorderings`, with several mutually independent statements",
        "n!",
        """
PERM_LINESWAP(
D_80049208[corner] = FieldEventReadMemoryU8(2, 4);
D_80049208[corner + 1] = FieldEventReadMemoryU8(3, 5);
D_80049208[corner + 2] = FieldEventReadMemoryU8(4, 6);
)
""",
        """
Only sound when the statements really are independent -- the permuter will
happily produce semantically different code, and a candidate that matches for
the wrong reason still has to survive tools/checkfn.py and `make build`.

Use PERM_LINESWAP_TEXT instead when a line is not a complete statement.
""",
    ),
    Recipe(
        "cast-width",
        "an `andi rX,rX,0xff` / `sll+sra` pair is present in one side only",
        "len(casts)",
        """
PERM_VAR(CAST, PERM_GENERAL(, (u8), (s16), (s32)))
...
    total += PERM_VAR(CAST)value;
""",
        """
The empty first alternative is the no-cast case -- PERM_GENERAL takes empty
arguments, which is how you make an option optional. Sweeps sign/zero extension
placement without adding a variable, which is the difference between this and
temp-hop: same widths, but no new stack slot and no new live range.
""",
    ),
    Recipe(
        "addr-form",
        "`lui/addiu` of a base address is recomputed, or an `addu` appears on "
        "only one side",
        "len(alternatives)",
        """
PERM_GENERAL(
    for (i = 0; i < count; i++) { dst[i] = src[i]; },
    p = dst; q = src; for (i = 0; i < count; i++) { *p++ = *q++; },
    end = dst + count; while (dst < end) { *dst++ = *src++; }
)
""",
        """
FieldDebugIntToString only matched once the tail was written through an
explicit `end = out + count` pointer rather than indexing `out[count]`; the
index form kept the base address live in the wrong register. Index vs pointer
vs cached-base is a three-way choice worth one PERM_GENERAL rather than a long
randomizer walk.
""",
    ),
    Recipe(
        "struct-store",
        "loads are batched ahead of stores -- the instruction *count* matches "
        "but the interleaving does not",
        "2 (x sites, use PERM_VAR to link them)",
        """
PERM_VAR(VIA, PERM_GENERAL(struct, flat))
...
PERM_GENERAL(FieldState.command = cmd; FieldState.entity = ent;,
             D_8009ABF5 = cmd; D_8009ABF6 = ent;)
""",
        """
Documented in CLAUDE.md: gcc 2.6.3 only keeps a load and a store in source
order when *both* sides are struct references. Reading through a parameter
pointer and writing to standalone `extern` scalars lets it batch the loads
ahead of the stores. FieldEntityGatewayMapLoad needs the struct form, writing
through FieldState at 0x8009ABF4 rather than through six separate D_ symbols.

Both alternatives must name the same addresses, or the candidate is simply
wrong C that happens to compile.
""",
    ),
    Recipe(
        "branch-polarity",
        "`Branch Differences`, or the two arms of an if are swapped",
        "2-3",
        """
PERM_GENERAL(
    if (cond) { A } else { B },
    if (!(cond)) { B } else { A },
    if (!(cond)) { B return; } A
)
""",
        """
gcc 2.6.3 emits the fall-through arm first, so polarity decides which block
lands where and which branch instruction is used (`beqz` vs `bnez`). The
early-return form additionally removes the trailing jump.
""",
    ),
    Recipe(
        "loop-form",
        "the loop's compare/branch sits at the wrong end, or an extra branch "
        "guards the first iteration",
        "len(forms)",
        """
PERM_GENERAL(
    for (i = 0; i < n; i++) { B },
    i = 0; while (i < n) { B i++; },
    i = 0; do { B i++; } while (i < n),
    i = n; do { B } while (--i)
)
""",
        """
A `for` whose trip count gcc cannot prove non-zero keeps a guard branch that a
`do/while` does not. FieldDebugIntToString is a `do { ... } while (divisor >= 2)`
for exactly this reason. The last form also reverses the induction variable,
which changes register pressure -- only sound when the body does not use i.
""",
    ),
    Recipe(
        "param-copy",
        "a parameter lives in the wrong register, or is spilled on one side only",
        "2 per parameter",
        """
PERM_GENERAL(, FieldModelEntry *m = model;)   /* then use PERM_VAR(P) below */
PERM_VAR(P, PERM_GENERAL(model, m))
...
    PERM_VAR(P)->partMatrices = pkts;
""",
        """
gcc 2.6.3 will keep a parameter in its incoming a-register for as long as it
can; copying it to a local forces the allocator to choose again, which is what
frees up the a-registers for an interleaved call.
""",
    ),
    Recipe(
        "fake-arg",
        "a cached address or temp sits in a1/a2 in the target but a0/a1 in the "
        "candidate, and the function has a parameter it never uses",
        "2 per call site",
        """
void UpdateFieldExitArrows(s32 arg0)   /* arg0 unused by the body */
{
    ...
    PERM_GENERAL(DrawFieldExitArrow();, DrawFieldExitArrow(arg0);)
}
""",
        """
An unused parameter dies at function entry, which frees its a-register -- and
gcc 2.6.3 then hands that register to the next temporary it needs. If the
original keeps the parameter alive, the temp lands one register later.

Passing the parameter on to a callee whose declaration is unprototyped costs
zero instructions (the value is already sitting in a0) but extends its live
range to the call, occupying a0 the whole way. UpdateFieldExitArrows matched
on exactly this: the cached &D_8009D5A6 moved from a0 to a1 with no other
change. The original source very likely really did pass the argument down.

Variants worth adding as alternatives if the call form fails: using the
parameter in an arithmetic no-op inside a condition, or copying it to a local
that a later expression reads.
""",
    ),
    Recipe(
        "arm-swap",
        "a length that is wrong by one instruction per copy of an if/else, or "
        "a subexpression common to both arms rematerialised once too often",
        "2 ** len(sites), so keep it to three or four ifs",
        """
/* Which arm is the fall-through is not a style choice: the blocks come out
 * in source order, so it decides what reorg can steal into the two branch
 * delay slots and what combine can fold in the block that is now laid out
 * second. No decomp-permuter pass mutates this -- the CFG is identical, so
 * every AST-level pass sees an equivalent program. */
PERM_GENERAL(
if (GetGraphType() == 1 || GetGraphType() == 2) {
    tp = ((t & 0xC0) * 8) | ((f * 4) & 0x180);
    tpBit = (t >> 7) & 0x20;
} else {
    tp = ((t & 0xC0) * 2) | (f & 0x60);
    tpBit = ((t >> 4) & 0x100) >> 4;
}
,
if (GetGraphType() != 1 && GetGraphType() != 2) {
    tp = ((t & 0xC0) * 2) | (f & 0x60);
    tpBit = ((t >> 4) & 0x100) >> 4;
} else {
    tp = ((t & 0xC0) * 8) | ((f * 4) & 0x180);
    tpBit = (t >> 7) & 0x20;
}
)
""",
        """
Measured on FieldModelCreatePktsForPart in src/field/field2.c: the inverted
form is exactly 727 instructions, the plain one 731 -- one per copy of the
loop. Both differences are invisible in the arm you are looking at. `t & 0xC0`
is computed twice in the inverted form (once into each branch delay slot,
serving both arms) and three times in the plain one; and
`((t >> 4) & 0x100) >> 4` survives as three instructions in the inverted form
where combine folds it to `srl 8` / `andi 0x10` in the plain one.

Do not try to reach either effect directly. Every spelling of that shift
measures identically -- `(u16)` and `(s32)` casts, `/ 16`, `& 0x1000 >> 8`, a
named intermediate, two `>> 2` -- and hoisting the mask into a local costs
eight instructions, because it is then live across both calls and spills.

A `goto` chain that reproduces the target's branch polarity and block order
literally also measures 731, so read the length rather than the CFG. Use
PERM_VAR to link the arms when the same if/else appears in several loops:
they may or may not want to agree, and the macro can express both.
""",
    ),
    Recipe(
        "for-guard",
        "the body is a couple of instructions long per counted loop, and m2c "
        "printed `if (c != 0) { do { ... } while (i < c); }`",
        "2 ** len(loops)",
        """
/* gcc expands `for (i = 0; i < c; i++)` as an init, a zero-trip test and a
 * do-while -- which is exactly what m2c reconstructs from the CFG. Writing
 * the guard as well does not fold: the bound is a struct field whose load
 * gcc will not prove redundant, so the test is emitted twice. */
    count = part->gt4Count;
    PERM_GENERAL(, if (count != 0))
    for (i = 0; i < count; i++, out += sizeof(POLY_GT4), poly += 6) {
""",
        """
Worth 16 instructions across the eight loops of FieldModelCreatePktsForPart --
two thirds of everything that function was over length, from a transcription
that looked entirely faithful.

The macro is worth having rather than just deleting every guard, because the
opposite case is real too: a target that reaches its loop through its own
guard, spelled against a register it has just proved to be zero, wants the
hand-written `if`. Read whether the bound is reloaded inside the loop (write
the member) or hoisted into the preheader (write a local), then let the macro
settle the guard.
""",
    ),
    Recipe(
        "fold-order",
        "an `addu` whose two operands are the other way round from the target, "
        "at a base-plus-offset address",
        "2 ** len(sites)",
        """
/* fold canonicalises a pointer PLUS so the pointer is op0, and no spelling of
 * the pointer form moves it. Casting the base to s32 makes it an ordinary
 * integer PLUS, which keeps source order -- so the cast is the knob, and the
 * two orders are then both reachable. */
PERM_VAR(UV, PERM_GENERAL(uv + (v & 0xFF) * 2, (v & 0xFF) * 2 + (s32)uv))
    *(u16*)&p->u0 = *(u16*)(PERM_VAR(UV));
""",
        """
Four sites in FieldModelCreatePktsForPart, and it also decides which of the
two loads feeding the address is emitted first. Same lever as CLAUDE.md's
`(u_long*)(param * 32 + (s32)p)` bullet, applied to a whole function at once.

Link the sites with PERM_VAR when they are the same expression shape: they
usually agree, and 2**4 collapses to 2.
""",
    ),
    Recipe(
        "spill-slot-order",
        "spilled locals sit at the wrong `N(sp)` offsets, all of them, with the "
        "frame size already right",
        "permutations of the declaration block -- keep it to five or six lines",
        """
/* Reload hands out spill slots in pseudo order, and expand_decl creates one
 * pseudo per local at the top of the function in *declaration* order. So the
 * slot order is the declaration order, and PERM_LINESWAP enumerates it. */
PERM_LINESWAP(
u32 pass;
s32 uOff0;
s32 vOff0;
s32 uOff1;
s32 vOff1;
u32* texInfo;
)
""",
        """
Six spilled locals in FieldModelCreatePktsForPart land at 0x28...0x50 in the
target's own order the moment the declarations are in that order.

This does not contradict CLAUDE.md's standing rule that declaration order is
inert -- that rule is about hard registers, which are handed out by priority.
In the same function, moving those declarations for any other reason (a `u8`
to the end, four pointers to the front) is exactly inert. Only locals that
actually get a stack slot are affected, so use this recipe when the diff is
`N(sp)` rows and nothing else, and expect it to be inert otherwise.
""",
    ),
    Recipe(
        "flag-local",
        "a branch around an addition where the target computes it with "
        "`sltu` / `negu` / `and`",
        "2 ** len(sites)",
        """
/* A comparison used as a value reaches expand_expr underneath the `+`, and
 * gcc 2.6.3 emits a conditional jump around the arithmetic. Assign it to a
 * local first and it is a statement of its own, do_store_flag runs, and the
 * branchless sequence appears. `x & -(c != K)` is NOT a way to force it --
 * fold turns it straight back into the same COND_EXPR. */
PERM_GENERAL(
clut = (((t * 2) >> 23) + (((t & 0x3F) != 2) ? texY : 0)) << 6;
,
shift = (t & 0x3F) != 2;
clut = (((t * 2) >> 23) + (texY & -shift)) << 6;
)
""",
        """
Five instructions per site, four sites in FieldModelCreatePktsForPart. m2c
renders the target's sequence as `(v & -((x & M) != K))`, which reads like a
clever mask and is really just a named boolean one statement earlier.
""",
    ),
    Recipe(
        "conserved-pair",
        "a residue of pure reorderings where every single-site fix is exactly "
        "neutral and everything else is catastrophic -- no gradient",
        "sites x fillers (finite); never PERM_RANDOMIZE",
        """
/* The store has two homes and reorg re-steals it at whichever one you pick,
 * so permute the *sink site* and the *back-edge filler* jointly. A macro at
 * one site alone cannot see the trade: every candidate scores identically. */
    func_80027B84(pr);
    PERM_ONCE(sink, rect.x = 0;)
    PERM_GENERAL(, dy += 12;, i++;, rowY += 12;)   /* back-edge slot bait */
}
PERM_ONCE(sink, rect.x = 0;)
PERM_LINESWAP(
rect.y = 0;
rect.w = 0x100;
rect.h = 0x100;
)
""",
        """
Written for func_801D080C in src/menu/cnfgmenu.c, whose last four rows are two
such pairs, but the shape recurs wherever reorg competes with the scheduler.

Recognise it by the *shape of the score landscape*, which is the cheap part.
Sink the statement on its own: the rows at that site clear and the same number
reappear at a nearby branch, total unchanged. That conservation is the
signature. It means the two sites are one degree of freedom, and a macro that
offers a choice at only one of them spans a space in which every point has the
same score.

The landscape is then neutral-or-catastrophic with nothing in between, and both
halves are worth knowing. On this function 1.5M permuter iterations produced
30,504 candidates tied at the base score and not one below it; a directed batch
of 17 hand-written variants split the same way -- `value++` for `value + 1`,
`value += 1`, the call and the increment on one source line, and routing a
single field through `pr` all scored *exactly* the base, while every spelling
gcc could not fold scored 5, 6, 64, 89 or 149. gcc 2.7.2 folds the whole
neighbourhood of legal rephrasings to identical RTL, so the plateau is not the
search failing to find a gradient -- there is no gradient to find, and more
iterations buy nothing.

So do not reach for PERM_RANDOMIZE here, even scoped. An unbounded search over
a provably flat region is the one case where the permuter cannot help, and it
will happily burn a core-day proving it again. Spend the run on the finite
joint space above, or leave the function alone.

Size that joint space before running it, because it is usually small enough to
enumerate with tools/variant_eval.py instead -- exact scores, no import/strip/
retarget dance, and the result is a proof rather than a sample. On the function
this was written from it came to 48 points, which took under three minutes and
returned no improvement: the sink site and both back-edge fillers turned out
completely inert, leaving the field order as the only live axis and every move
along it a loss.

Two things to check before you conclude a pair is conserved rather than
reachable. Confirm the rows are *reorderings* -- asm-differ marks a moved
instruction with `<` against the row it left, so the pair shows up as one CHG
plus one INS with the same text, not as a real insertion. And read the
function's own comment block first: on a function this close to matching, the
mechanism has usually already been worked out and written down, and re-deriving
it costs a batch for nothing (it did here).
""",
    ),
    Recipe(
        "giv-hoist",
        "the target carries a value in a stack slot and increments it (`li "
        "t0,-42` in the preheader, `+= 2` at the loop bottom) where your build "
        "recomputes it inline as an `addiu`/`sll`/`addu` triple",
        "k sites + 1 (inline)",
        """
/* One PERM_ONCE key per value, with a site at each candidate hoist point and
 * the inline form as the last alternative. */
for (row = 0; row < 21; row++) {
    PERM_ONCE(vy, vy = (row - 16) * 2;)
    for (col = 0; col < 40; col++) {
        PERM_ONCE(vy, vy = (row - 16) * 2;)
        PERM_ONCE(vx, vx = (col - 21) * 2;)
        ...
        PERM_ONCE(vx, vx = (col - 21) * 2;)
        EscapeTiles[n].Vel.vx = vx + (rand() & 3);
""",
        """
gcc 2.6.3 strength-reduces an expression over a loop counter into an induction
variable with its own stack slot only when the value is a *named local computed
early and used late*. Inside one statement -- `(rand() & 3) + (col - 21) * 2`
-- the product is computed and consumed with nothing in between, and no
spelling of the arithmetic changes that: `col * 2 - 42`, `(col - 21) << 1`,
`2 * (col - 21)` and `col + col - 42` all compile to the same triple.

This is the opposite of the usual reflex, so it is easy to skip: naming the
temporary is what the compiler wanted. It took func_801B009C in
src/magic/escape.c from 104 rows to 22, for two values at once.

Do not confuse it with a hand-carried accumulator (`vy = -32` before the loop,
`vy += 2` at the bottom). That is a *biv*, gets a register rather than a slot,
and takes a low pseudo -- a different slot from the one the target uses. There
is still no way to spell keep-this-on-the-stack in C.

Hoist position matters and is not guessable, which is why it is a PERM_ONCE
sweep rather than a fixed rewrite: for the same function, the row-invariant
value had to go to the top of the outer loop and the column one to the top of
the inner loop. Moving either one line changed the slot count.
""",
    ),
    Recipe(
        "addr-eval-order",
        "one of several sibling `&arr[i][j + k]` addresses becomes a spilled "
        "giv that duplicates a multiply you already hold in a register, costing "
        "an extra stack slot and a reload per iteration",
        "k siblings",
        """
/* Which sibling is *evaluated* first is the lever; the temporary keeps the
 * store order unchanged, so the two are independent. */
{
    PERM_GENERAL(EscapeCell* c = &EscapeGrid[row][col + 1];,
                 EscapeCell* c = &EscapeGrid[row + 1][col];,
                 EscapeCell* c = &EscapeGrid[row + 1][col + 1];)
    EscapeTiles[n].Corner[0] = &EscapeGrid[row][col];
    EscapeTiles[n].Corner[1] = ...;
}
""",
        """
With `&EscapeGrid[row][col]` -- the zero-offset sibling -- evaluated first, gcc
builds a giv `&EscapeGrid + col * 12`, spills it, and reloads it once per
iteration, duplicating the `col * 12` giv it already keeps in a register. With
`&EscapeGrid[row][col + 1]` first, no giv forms at all and every sibling is
computed from the two multiples plus a rematerialised base, which is what the
target does. `[row + 1][col]` and `[row + 1][col + 1]` first both rebuild the
giv, so the live axis is a constant on the *inner* index, not just "not first".

Nothing else moved it on func_801B009C: a named `EscapeCell *`, a walked `c++`
pointer, `EscapeGrid[row] + col`, `[row + 0][col + 0]`, and explicit
`(u8 *)EscapeGrid + row * 492 + col * 12` byte arithmetic were all measured.
The ones that do avoid the giv do it by collapsing the row and column multiples
into a single flat giv, which costs more than it saves.

Evaluate into a temporary rather than reordering the assignments: gcc keeps
stores in source order, so reordering them to control evaluation order also
reorders the stores, and the target has both -- sibling 1 evaluated first,
stores still 0, 1, 2, 3.
""",
    ),
    Recipe(
        "cse-split",
        "the right instructions with the wrong addressing form: `sb v0,13(s0)` "
        "where the target has `lui at,%hi(sym + 0xD)` / `addu at,at,s1` / "
        "`sb v0,0(at)`, or the reverse, for some of several accesses to one "
        "indexed object",
        "len(patterns), via PERM_GENERAL over which accesses use the alias",
        """
/* A second pseudo holding the same index splits the address CSE, so accesses
 * through it take the base-register form and the rest keep maspsx's
 * symbol+offset(index) form. Sweep which accesses use which. */
    m = n;
    ...
    PERM_GENERAL(
      EscapeTiles[m].prim[0].v0 = EscapeTiles[n].prim[1].v0 =
          EscapeTiles[n].prim[0].v1 = EscapeTiles[m].prim[1].v1 = row * 8;,
      EscapeTiles[n].prim[0].v0 = EscapeTiles[m].prim[1].v0 =
          EscapeTiles[m].prim[0].v1 = EscapeTiles[n].prim[1].v1 = row * 8;)
""",
        """
gcc has two ways to reach `EscapeTiles[n].field`: through a pointer it has
already computed (`s0 = &EscapeTiles + n * 128`, then a small displacement), or
by handing maspsx `sym + off(index)` and letting it expand a lui/addiu/addu.
Which one each access gets is decided by CSE, and a second pseudo holding the
same index value splits the equivalence class.

Found on func_801B009C, where it is worth 10 of 22 rows -- but note two things
before reaching for it. It only worked *jointly* with addr-eval-order's
hoisted pointer: either change alone measured 202 rows against the retail
overlay, both together 12. And the alias has to be non-uniform; making every
access use it is as bad as using none (202), and a real pointer
(`EscapeTile *tp = &EscapeTiles[n];`) in place of the integer alias does not
work either (210).

That asymmetry is the tell that this is a compiler artefact rather than source.
Nobody wrote `m = n` and then used it for three of eight stores. Treat a
cse-split win as evidence that the original reached the same split some other
way -- a pointer parameter, a differently-shaped loop -- and keep looking for
that shape. It is a legitimate rung on the ladder, not a place to stop.
""",
    ),
    Recipe(
        "region-random",
        "the finite space is exhausted and the residue is a couple of rows you "
        "have no hypothesis for",
        "unbounded",
        """
    /* the part you have already matched, left alone */
    model->partMatrices = pkts;
    PERM_RANDOMIZE(
    for (i = 0; i < model->partCount; i++) {
        pkts = FieldModelCreatePktsForPart(part, pkts, 0, arg2);
        part += 0x20;
    }
    )
""",
        """
The last resort, and still better than a bare randomization run: mutation is
confined to the region that actually diverges, so the search cannot spend its
time rewriting code that already matches. Note that any PERM_RANDOMIZE makes
the whole space infinite -- the run no longer terminates on its own, so give it
`--stop-on-zero` and a floor you trust (see `align`).
""",
    ),
]

RECIPES_BY_NAME = {r.name: r for r in RECIPES}


def cmd_recipes(args: argparse.Namespace) -> int:
    width = max(len(r.name) for r in RECIPES)
    print("Symptom -> recipe. Read the symptom off the penalty list that")
    print("`permuter.py <dir> --debug` prints, or off tools/asm-differ.\n")
    for r in RECIPES:
        print(f"  {r.name.ljust(width)}  {r.symptom}")
    print("\nPrint one with: permuter_macros.py recipe <name>")
    return 0


def cmd_recipe(args: argparse.Namespace) -> int:
    r = RECIPES_BY_NAME.get(args.name)
    if r is None:
        print(f"no such recipe: {args.name}", file=sys.stderr)
        print("known: " + ", ".join(sorted(RECIPES_BY_NAME)), file=sys.stderr)
        return 1
    print(f"/* recipe: {r.name}")
    print(f" * symptom: {r.symptom}")
    print(f" * candidates: {r.count}")
    print(" */")
    print(r.body)
    print()
    for line in r.note.splitlines():
        print(f"// {line}" if line else "//")
    return 0


# --------------------------------------------------------------------------
# perm_count
# --------------------------------------------------------------------------

class CountError(Exception):
    pass


def _split_by_comma(text: str) -> List[str]:
    """Split on commas outside parentheses, as src/perm/parse.py does."""
    level = 0
    current = ""
    args: List[str] = []
    for c in text:
        if c == "," and level == 0:
            args.append(current)
            current = ""
        else:
            if c == "(":
                level += 1
            elif c == ")":
                level -= 1
                if level < 0:
                    raise CountError("bad nesting in PERM macro arguments")
            current += c
    if level != 0:
        raise CountError("mismatched parentheses in PERM macro arguments")
    args.append(current)
    return args


def _consume_arg_parens(text: str) -> Tuple[str, str]:
    level = 0
    for i, c in enumerate(text):
        if c == "(":
            level += 1
        elif c == ")":
            level -= 1
            if level == -1:
                return text[:i], text[i + 1 :]
    raise CountError("unclosed parenthesis in a PERM macro")


def _factorial(n: int) -> int:
    res = 1
    for i in range(2, n + 1):
        res *= i
    return res


class Counted:
    """The subset of src/perm/perm.py's bookkeeping that a size estimate needs.

    `count` mirrors Perm.perm_count; `once_keys` collects PERM_ONCE keys so the
    root can apply its len(options) factor; `random` mirrors is_random().
    """

    def __init__(self) -> None:
        self.count = 1
        self.random = False
        self.once_keys: Dict[str, int] = {}

    def combine_product(self, other: "Counted") -> None:
        self.count *= other.count
        self.random = self.random or other.random
        for k, v in other.once_keys.items():
            self.once_keys[k] = self.once_keys.get(k, 0) + v


def _count_text(text: str) -> Counted:
    """Count the perm space of a stretch of source, mirroring _rec_perm_parse."""
    res = Counted()
    remain = text
    while True:
        m = PERM_MACRO_RE.search(remain)
        if m is None:
            break
        kind = m.group(1)
        args_text, remain = _consume_arg_parens(remain[m.end() :])
        res.combine_product(_count_macro(kind, args_text))
    return res


def _count_macro(kind: str, text: str) -> Counted:
    res = Counted()
    if kind in ("PERM_GENERAL",):
        total = 0
        for arg in _split_by_comma(text):
            sub = _count_text(arg)
            total += sub.count
            res.random = res.random or sub.random
            for k, v in sub.once_keys.items():
                res.once_keys[k] = res.once_keys.get(k, 0) + v
        res.count = total
    elif kind in ("PERM_LINESWAP", "PERM_LINESWAP_TEXT"):
        lines = [ln for ln in text.split("\n") if ln.strip()]
        res.count = _factorial(len(lines))
        for line in lines:
            res.combine_product(_count_text(line))
    elif kind == "PERM_INT":
        parts = [p.strip() for p in _split_by_comma(text)]
        if len(parts) != 2:
            raise CountError("PERM_INT takes 2 arguments")
        try:
            lo, hi = int(parts[0], 0), int(parts[1], 0)
        except ValueError:
            raise CountError("PERM_INT arguments must be integer constants")
        res.count = hi - lo + 1
    elif kind == "PERM_ONCE":
        parts = _split_by_comma(text)
        if len(parts) not in (1, 2):
            raise CountError("PERM_ONCE takes 1 or 2 arguments")
        key = parts[0].strip()
        res.combine_product(_count_text(parts[-1]))
        res.once_keys[key] = res.once_keys.get(key, 0) + 1
    elif kind == "PERM_VAR":
        parts = _split_by_comma(text)
        if len(parts) not in (1, 2):
            raise CountError("PERM_VAR takes 1 or 2 arguments")
        for part in parts:
            res.combine_product(_count_text(part))
    elif kind == "PERM_RANDOMIZE":
        res.combine_product(_count_text(text))
        res.random = True
    elif kind in ("PERM_FORCE_SAMELINE", "PERM_IGNORE", "PERM_PRETEND"):
        res.combine_product(_count_text(text))
    else:
        raise CountError(f"unknown macro {kind} -- permuter.py will refuse this file")
    return res


def perm_count(source: str) -> Tuple[int, bool, Dict[str, int]]:
    """Return (perm_count, is_random, once_key_counts) for a base.c."""
    res = _count_text(source)
    total = res.count
    for key, n in res.once_keys.items():
        if n == 1:
            raise CountError(
                f"PERM_ONCE({key}) occurs only once -- permuter.py raises on this"
            )
        total *= n
    return total, res.random, res.once_keys


def cmd_count(args: argparse.Namespace) -> int:
    path = resolve_base_c(args.path)
    source = read_text(path)
    macros = sorted(set(PERM_MACRO_RE.findall(source)))
    if not macros:
        print(f"{path}: no PERM macros -- this is a plain randomization run.")
        print("Pick a recipe (permuter_macros.py recipes) before spending cores.")
        return 0
    try:
        total, random, once_keys = perm_count(source)
    except CountError as e:
        print(f"{path}: {e}", file=sys.stderr)
        return 1
    print(f"{path}")
    print(f"  macros:      {', '.join(macros)}")
    for key, n in sorted(once_keys.items()):
        print(f"  PERM_ONCE({key}): {n} sites")
    if random:
        print(f"  perm_count:  {total} base sources x unbounded randomization")
        print("  The run will not terminate on its own; use --stop-on-zero.")
        return 0
    rate = args.jobs * CANDIDATES_PER_SEC_PER_JOB
    secs = total / rate if rate else 0
    print(f"  perm_count:  {total} (finite -- enumerated exhaustively, then stops)")
    print(f"  estimate:    {fmt_duration(secs)} at -j {args.jobs}")
    if total > 200000:
        print("  That is too big to enumerate. Split it: fix one dimension at a")
        print("  time, or drop the widest PERM_GENERAL to a two-way choice.")
    return 0


def fmt_duration(secs: float) -> str:
    if secs < 90:
        return f"{secs:.0f}s"
    if secs < 90 * 60:
        return f"{secs / 60:.1f}min"
    return f"{secs / 3600:.1f}h"


# --------------------------------------------------------------------------
# lint
# --------------------------------------------------------------------------

def cmd_lint(args: argparse.Namespace) -> int:
    path = resolve_base_c(args.path)
    scratch = os.path.dirname(path)
    source = read_text(path)
    problems: List[str] = []
    notes: List[str] = []

    for i, line in enumerate(source.splitlines(), 1):
        m = B64_RE.match(line)
        if m and "__maspsx_include_asm_hack_" in decode_b64(m.group(1)):
            problems.append(
                f"line {i}: INCLUDE_ASM blob still present -- every candidate "
                f"carries the whole overlay's asm and the score is meaningless. "
                f"Run tools/permuter_strip_asm.py {scratch}"
            )
            break

    for i, line in enumerate(source.splitlines(), 1):
        if B64_RE.match(line) and "PERM_" in line:
            problems.append(
                f"line {i}: PERM macro inside a b64literal pragma -- the parser "
                f"never sees it, so it silently does nothing"
            )

    macros = set(PERM_MACRO_RE.findall(source))
    known = {
        "PERM_GENERAL", "PERM_ONCE", "PERM_RANDOMIZE", "PERM_FORCE_SAMELINE",
        "PERM_VAR", "PERM_LINESWAP", "PERM_LINESWAP_TEXT", "PERM_INT",
        "PERM_IGNORE", "PERM_PRETEND",
    }
    for m in sorted(macros - known):
        problems.append(f"unknown macro {m} -- permuter.py will refuse this file")

    if not macros:
        notes.append(
            "no PERM macros: this run is pure randomization. See `recipes`."
        )
    else:
        try:
            total, random, _ = perm_count(source)
            if random and total > 1:
                notes.append(
                    f"PERM_RANDOMIZE is mixed with {total} finite base sources; "
                    f"the space is unbounded. Consider exhausting the finite "
                    f"macros first -- that run ends by itself."
                )
            elif not random and total == 1:
                problems.append(
                    "the PERM macros describe exactly one candidate (a "
                    "single-argument PERM_GENERAL?), so the run tests nothing"
                )
        except CountError as e:
            problems.append(str(e))

    func = scratch_func_name(scratch)
    if func and re.search(r"\b" + re.escape(func) + r"\s*\(", source) is None:
        problems.append(
            f"settings.toml names {func}, which does not appear in base.c -- "
            f"import.py was run against an INCLUDE_ASM stub, so there is "
            f"nothing to permute"
        )

    target_s = os.path.join(scratch, "target.s")
    if os.path.isfile(target_s):
        renames, offsets, unresolved = plan_alignment(source, read_text(target_s))
        if renames:
            notes.append(
                f"{len(renames)} symbol(s) named differently in base.c and "
                f"target.s ({', '.join(sorted(renames)[:3])}...): 5 score points "
                f"each, forever. Run `permuter_macros.py align {scratch}`"
            )
        if offsets:
            notes.append(
                f"{len(offsets)} target symbol(s) are interior addresses of a "
                f"base.c array; `permuter_macros.py align {scratch}` rewrites "
                f"them to the (&D_xxxxxxxx)[i] form the scorer expects"
            )
        if unresolved and re.search(r'"[^"]*"', source):
            notes.append(
                f"{len(unresolved)} unnamed target symbol(s) with string "
                f"literals in base.c: `align --strings` can pair them"
            )

    if "_S(" in source:
        notes.append(
            "_S(...) present: under -DFF7_STR it is not a macro at all, bin/str "
            "rewrites it after cpp. Leave it exactly as imported"
        )

    for p in problems:
        print(f"ERROR   {p}")
    for n in notes:
        print(f"note    {n}")
    if not problems and not notes:
        print(f"{path}: clean")
    return 1 if problems else 0


def decode_b64(blob: str) -> str:
    import base64
    try:
        return base64.b64decode(blob).decode("utf-8", "replace")
    except Exception:
        return ""


# --------------------------------------------------------------------------
# suggest
# --------------------------------------------------------------------------

PENALTY_RE = re.compile(
    r"^(Stack Differences|Branch Differences|Register Differences|"
    r"Reorderings|Insertions|Deletions):\s+(\d+)",
    re.M,
)
MIPS_REGS = set(
    "zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 t8 t9 "
    "s0 s1 s2 s3 s4 s5 s6 s7 k0 k1 gp sp fp ra".split()
)
LOAD_MNEMONICS = {"lw", "lb", "lbu", "lh", "lhu"}
STORE_MNEMONICS = {"sw", "sb", "sh"}


def parse_debug_output(text: str):
    """Parse `permuter.py <dir> --debug` output.

    Returns (penalties, rows) where rows are (target_line, candidate_line)
    pairs from the two-column diff -- target on the left, candidate on the
    right, either possibly empty for insertions/deletions.
    """
    text = re.sub(r"\x1b\[[0-9;]*m", "", text)  # strip ANSI colors
    penalties = {name: int(n) for name, n in PENALTY_RE.findall(text)}
    rows: List[Tuple[str, str]] = []
    for line in text.splitlines():
        if "\t" not in line:
            continue
        left, _, right = line.partition("\t")
        left, right = left.strip(), right.strip()
        # Instruction rows only: mnemonic in column one of either side.
        if re.match(r"^[a-z][a-z0-9.]*\s", left + " ") or re.match(
            r"^[a-z][a-z0-9.]*\s", right + " "
        ):
            rows.append((left, right))
    return penalties, rows


def _regs_of(line: str) -> List[str]:
    return [t for t in re.findall(r"[a-z]+[0-9]*", line) if t in MIPS_REGS]


def _mnemonic(line: str) -> str:
    return line.split()[0] if line.split() else ""


def find_unused_params(source: str, func: str) -> List[str]:
    """Parameters of `func` in base.c that its body never mentions."""
    m = re.search(
        r"^[^\n#]*?\b" + re.escape(func) + r"\s*\(([^)]*)\)\s*\{", source, re.M | re.S
    )
    if not m:
        return []
    params = []
    for part in m.group(1).split(","):
        toks = re.findall(r"[A-Za-z_]\w*", part)
        if toks and toks[-1] not in ("void",):
            params.append(toks[-1])
    # Body: brace-match from the opening brace.
    depth, start, end = 0, m.end() - 1, len(source)
    for i in range(start, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                end = i
                break
    body = source[start + 1 : end]
    return [p for p in params if not re.search(r"\b" + re.escape(p) + r"\b", body)]


def cmd_suggest(args: argparse.Namespace) -> int:
    scratch = args.path if os.path.isdir(args.path) else os.path.dirname(args.path)
    if args.debug_output == "-":
        text = sys.stdin.read()
    else:
        debug_path = args.debug_output or os.path.join(scratch, "debug.txt")
        if not os.path.isfile(debug_path):
            print(
                f"no debug output at {debug_path}.\nSave it first:\n"
                f"  permuter.py {scratch} --debug 2>&1 | tee {scratch}/debug.txt",
                file=sys.stderr,
            )
            return 1
        text = read_text(debug_path)
    penalties, rows = parse_debug_output(text)
    if not penalties:
        print("no penalty list found in the debug output", file=sys.stderr)
        return 1

    base_c = resolve_base_c(scratch)
    source = read_text(base_c) if os.path.isfile(base_c) else ""
    func = scratch_func_name(scratch) or ""
    unused = find_unused_params(source, func) if source and func else []

    # Signature scan over paired rows.
    reg_pairs: List[Tuple[str, str]] = []
    cast_asym = branch_flip = addr_recompute = 0
    saw_load = saw_store = False
    for left, right in rows:
        lm, rm = _mnemonic(left), _mnemonic(right)
        if lm in LOAD_MNEMONICS or rm in LOAD_MNEMONICS:
            saw_load = True
        if lm in STORE_MNEMONICS or rm in STORE_MNEMONICS:
            saw_store = True
        if not left or not right:
            if ("andi" in (lm, rm) and "0xff" in left + right) or {lm, rm} & {
                "sll",
                "sra",
            }:
                cast_asym += 1
            if lm in ("lui", "addiu") or rm in ("lui", "addiu"):
                addr_recompute += 1
            continue
        if lm != rm:
            pairs = {frozenset((lm, rm))}
            if pairs & {
                frozenset(("beq", "bne")),
                frozenset(("beqz", "bnez")),
                frozenset(("bltz", "bgez")),
                frozenset(("blez", "bgtz")),
            }:
                branch_flip += 1
            continue
        lr, rr = _regs_of(left), _regs_of(right)
        if len(lr) == len(rr):
            reg_pairs.extend((a, b) for a, b in zip(lr, rr) if a != b)

    a_shift = any(
        a.startswith("a") and b.startswith("a") and a != b for a, b in reg_pairs
    )

    # Rank recipes: (score, name, reason).
    sugg: List[Tuple[int, str, str]] = []
    regs = penalties.get("Register Differences", 0)
    if regs:
        if a_shift and unused:
            sugg.append((
                100,
                "fake-arg",
                f"a-register shift ({', '.join(sorted(set(f'{a}->{b}' for a, b in reg_pairs if a.startswith('a')))[:2])}) "
                f"and parameter '{unused[0]}' is never used -- keep it alive",
            ))
        sugg.append((80, "temp-hop", f"{regs} register difference(s): sweep an intermediate's type"))
        if cast_asym:
            sugg.append((75, "cast-width", f"{cast_asym} width-mask row(s) on one side only"))
        if a_shift and not unused:
            sugg.append((60, "param-copy", "a-register shift with all parameters used"))
    if penalties.get("Stack Differences", 0):
        sugg.append((
            90,
            "decl-order",
            f"{penalties['Stack Differences']} stack difference(s) -- rerun searches with --stack-diffs",
        ))
    if penalties.get("Reorderings", 0):
        n = penalties["Reorderings"]
        sugg.append((85, "hoist", f"{n} reordering(s): move one statement between landing sites"))
        sugg.append((65, "stmt-order", f"{n} reordering(s) among independent statements"))
        if saw_load and saw_store:
            sugg.append((
                87,
                "temp-hop",
                "a load moved relative to a store -- a narrow typed temp forces "
                "the load back in order (both of this repo's permuter matches)",
            ))
            sugg.append((70, "struct-store", "loads and stores present in the moved rows -- check CLAUDE.md's struct rule"))
    if penalties.get("Branch Differences", 0) or branch_flip:
        sugg.append((85, "branch-polarity", "branch mnemonics flipped between target and candidate"))
        sugg.append((55, "loop-form", "if the flipped branch is a loop's back edge"))
    if penalties.get("Insertions", 0) or penalties.get("Deletions", 0):
        ins = penalties.get("Insertions", 0) + penalties.get("Deletions", 0)
        sugg.append((88, "loop-form", f"{ins} inserted/deleted row(s): the C's shape differs, not just its allocation"))
        sugg.append((72, "addr-form", "index vs pointer vs cached-base changes instruction count"))
        if addr_recompute:
            sugg.append((68, "addr-form", f"{addr_recompute} unpaired lui/addiu row(s)"))

    if not sugg:
        if sum(penalties.values()) == 0:
            print("score is already 0 -- verify with --debug --stack-diffs, then checkfn.py")
        else:
            print("no signature recognized; see `recipes` for the full catalogue")
        return 0
    seen: Set[str] = set()
    print(f"penalties: {penalties}")
    if unused:
        print(f"unused parameter(s) in {func}: {', '.join(unused)}")
    print()
    for score, name, reason in sorted(sugg, key=lambda s: -s[0]):
        if name in seen:
            continue
        seen.add(name)
        print(f"  {name:16s} {reason}")
    print("\nPrint one with: permuter_macros.py recipe <name>")
    return 0


# --------------------------------------------------------------------------
# weights
# --------------------------------------------------------------------------

# Starting-point overrides for a PERM_RANDOMIZE run on this target, written by
# `weights`. The [gcc] profile in decomp-permuter's default_weights.toml merely
# halves the IDO-oriented passes; on gcc 2.6.3 / R3000A some of them cannot
# help at all, and the passes that have actually produced matches here deserve
# more of the budget.
TUNED_WEIGHTS: List[Tuple[str, float, str]] = [
    ("perm_float_literal", 0.0, "the R3000A has no FPU; FF7 code is fixed-point"),
    ("perm_sameline", 0.0, "IDO line-sensitivity; gcc 2.6.3 does not care"),
    ("perm_mult_zero", 0.0, "IDO-ism, no observed effect on this compiler"),
    ("perm_dummy_comma_expr", 0.0, "IDO-ism, no observed effect on this compiler"),
    ("perm_empty_stmt", 0.0, "IDO-ism, no observed effect on this compiler"),
    ("perm_xor_zero", 0.0, "IDO-ism, no observed effect on this compiler"),
    ("perm_temp_for_expr", 150.0, "both permuter matches here were temp insertions"),
    ("perm_randomize_internal_type", 25.0, "the temp's width decided both matches"),
    ("perm_reorder_stmts", 15.0, "statement position moves whole runs on gcc 2.6.3"),
]


def cmd_weights(args: argparse.Namespace) -> int:
    scratch = args.path if os.path.isdir(args.path) else os.path.dirname(args.path)
    settings = os.path.join(scratch, "settings.toml")
    if not os.path.isfile(settings):
        print(f"{settings} not found -- run import.py first", file=sys.stderr)
        return 1
    text = read_text(settings)
    # Drop any existing [weight_overrides] section (to its next header or EOF).
    text = re.sub(
        r"\n?\[weight_overrides\][^[]*", "\n", text, flags=re.S
    ).rstrip("\n") + "\n"
    lines = ["", "[weight_overrides]"]
    for name, value, why in TUNED_WEIGHTS:
        lines.append(f"{name} = {value}  # {why}")
    text += "\n".join(lines) + "\n"
    if args.dry_run:
        print("\n".join(lines))
        return 0
    write_text(settings, text)
    print(f"wrote tuned [weight_overrides] to {settings}")
    print("These only matter for PERM_RANDOMIZE / bare randomization runs;")
    print("finite PERM spaces ignore them.")
    return 0


# --------------------------------------------------------------------------
# align
# --------------------------------------------------------------------------

def load_symbol_map() -> Dict[str, int]:
    """name -> address, from every committed symbol list in config/."""
    out: Dict[str, int] = {}
    config = os.path.join(REPO_ROOT, "config")
    for entry in sorted(os.listdir(config)):
        if not entry.endswith(".txt"):
            continue
        for line in read_text(os.path.join(config, entry)).splitlines():
            m = SYM_LINE_RE.match(line)
            if m:
                out.setdefault(m.group(1), int(m.group(2), 16))
    return out


def target_symbols(target_s: str) -> Set[str]:
    out: Set[str] = set()
    for regex in TARGET_SYM_RES:
        out.update(regex.findall(target_s))
    return out


def plan_alignment(
    source: str, target_s: str
) -> Tuple[Dict[str, str], List[Tuple[str, str, int]], List[str]]:
    """Work out how base.c's names differ from target.s's.

    Returns (renames, interior, unresolved):
      renames    project name in base.c -> address name used by target.s
      interior   (target name, base.c array, byte offset) for A+k references
      unresolved target symbols with no name anywhere -- usually .rodata
    """
    syms = load_symbol_map()
    addr_to_names: Dict[int, List[str]] = {}
    for name, addr in syms.items():
        addr_to_names.setdefault(addr, []).append(name)

    in_source = set(re.findall(r"\b[A-Za-z_]\w*\b", source))
    # Addresses of the symbols base.c actually mentions, for the interior check.
    # Auto-named ones (D_80049208) carry their address in the name itself.
    source_addrs = {syms[n]: n for n in in_source if n in syms}
    for name in in_source:
        m = ADDR_NAME_RE.match(name)
        if m:
            source_addrs[int(m.group(1), 16)] = name

    renames: Dict[str, str] = {}
    interior: List[Tuple[str, str, int]] = []
    unresolved: List[str] = []
    for sym in sorted(target_symbols(target_s)):
        m = ADDR_NAME_RE.match(sym)
        if not m:
            continue
        addr = int(m.group(1), 16)
        if sym in in_source:
            continue
        hit = [n for n in addr_to_names.get(addr, []) if n in in_source]
        if hit:
            renames[hit[0]] = sym
            continue
        base = next(
            (
                (source_addrs[a], addr - a)
                for a in sorted(source_addrs, reverse=True)
                if 0 < addr - a <= 0x40
            ),
            None,
        )
        if base:
            interior.append((sym, base[0], base[1]))
        else:
            unresolved.append(sym)
    return renames, interior, unresolved



# --------------------------------------------------------------------------
# retarget
# --------------------------------------------------------------------------

def cmd_retarget(args: argparse.Namespace) -> int:
    """Rewrite target.s's symbol names to the spellings base.c's object uses.

    `align` fixes the candidate side, which works when the two names denote the
    same object under different names. It cannot fix an *interior* of a named
    struct: splat calls the halfword at 0x8009D7BE `D_8009D7BE`, the C calls it
    `Savemap.config`, and rewriting the C to reach it through a standalone
    extern stops gcc caching the struct base in a register -- codegen changes,
    which is the one thing alignment must never do.

    The scratch's target.s is ours, though. Rewriting `D_8009D7BE` there to
    `Savemap+0x10da` leaves an identical relocation against an identical
    address, so both sides of the diff render the same text and the rows stop
    being scored -- with no effect on the candidate at all.

    Which addresses get rewritten is taken from the candidate object rather
    than guessed from a symbol table: whatever base.o spells as `NAME+0xOFF` is
    what target.s should say for that address. So this can only ever make the
    two sides agree on something they already agree on numerically.
    """
    path = resolve_base_c(args.path)
    scratch = os.path.dirname(path)
    target_s = os.path.join(scratch, "target.s")
    base_o = os.path.join(scratch, "base.o")
    target_o = os.path.join(scratch, "target.o")
    for needed in (target_s, base_o):
        if not os.path.isfile(needed):
            print(f"{scratch}: no {os.path.basename(needed)}", file=sys.stderr)
            return 1

    objdump = "mipsel-linux-gnu-objdump -drz -m mips:3000"
    settings = os.path.join(scratch, "settings.toml")
    if os.path.isfile(settings):
        m = re.search(r'objdump_command\s*=\s*"([^"]+)"', read_text(settings))
        if m:
            objdump = m.group(1)

    syms = load_symbol_map()

    def address_of(name: str, off: int) -> Optional[int]:
        if name in syms:
            return syms[name] + off
        m = ADDR_NAME_RE.match(name)
        return int(m.group(1), 16) + off if m else None

    try:
        dump = subprocess.run(
            objdump.split() + [base_o],
            check=True, capture_output=True, text=True,
        ).stdout
    except (subprocess.CalledProcessError, FileNotFoundError) as exc:
        print(f"objdump failed: {exc}", file=sys.stderr)
        return 1

    # MIPS o32 is REL: objdump prints the relocation's symbol on its own line
    # and leaves the addend in the instruction's immediate, so the offset has
    # to be read back out of the operand. Only R_MIPS_LO16 is used -- it is the
    # half that carries the low 16 bits, which is the whole addend for every
    # offset under 0x8000. A larger one would need its HI16 partner too; rather
    # than reconstruct that, the computed address simply fails to match any
    # target symbol and nothing is rewritten, which is the safe way to be wrong.
    insn_re = re.compile(r"^\s*([0-9a-f]+):	[0-9a-f ]+	\s*\S+\s+(.*?)\s*$")
    reloc_re = re.compile(r"^\s*([0-9a-f]+): (R_MIPS_\w+)\s+(\S+)")
    imm_res = (
        re.compile(r",\s*(-?\d+)\((?:\$?\w+)\)$"),
        re.compile(r",\s*(-?\d+)$"),
    )
    spelling: Dict[int, str] = {}
    operands: Dict[str, str] = {}
    for line in dump.splitlines():
        m = insn_re.match(line)
        if m:
            operands[m.group(1)] = m.group(2)
            continue
        m = reloc_re.match(line)
        if not m or m.group(2) != "R_MIPS_LO16":
            continue
        ops = operands.get(m.group(1))
        if ops is None:
            continue
        imm = next((r.search(ops) for r in imm_res if r.search(ops)), None)
        if imm is None:
            continue
        off = int(imm.group(1))
        addr = address_of(m.group(3), off)
        if addr is not None and off:
            spelling.setdefault(addr, f"{m.group(3)}+0x{off:x}")

    text = read_text(target_s)
    source = read_text(path)
    rewrites: List[Tuple[str, str, int]] = []
    for sym in sorted(target_symbols(text)):
        m = ADDR_NAME_RE.match(sym)
        if not m:
            continue
        want = spelling.get(int(m.group(1), 16))
        if not want or want == sym:
            continue
        # `align` may already have rewritten base.c to use this very name --
        # the array-interior case, `(&D_801D252D)[...]`. The two sides then
        # agree, and "correcting" the target would break that agreement.
        #
        # A member access is not a reference to the global, though: the C in
        # src/magic/escape.c reaches the byte at 0x80151909 as
        # `D_801518E4[row].D_80151909`, so a plain word match sees the name and
        # skips a rewrite that was needed -- leaving 6 points of pure naming
        # noise, and with it a floor that `--stop-on-zero` can never reach.
        # Anything preceded by `.` or `->` is a field, not a symbol.
        if re.search(r"(?<![.>\w])" + re.escape(sym) + r"\b", source):
            print(f"skip     {sym}: base.c already names it")
            continue
        pattern = r"\b" + re.escape(sym) + r"\b"
        count = len(re.findall(pattern, text))
        text = re.sub(pattern, want, text)
        rewrites.append((sym, want, count))

    if not rewrites:
        print("nothing to retarget -- target.s already agrees with base.o")
        return 0
    for sym, want, count in rewrites:
        print(f"retarget {sym} -> {want}  ({count} references)")
    if args.dry_run:
        return 0

    if not os.path.exists(target_s + ".preretarget"):
        shutil.copyfile(target_s, target_s + ".preretarget")
    write_text(target_s, text)

    # target.o is what the permuter actually scores against, so it has to be
    # rebuilt or the rewrite changes nothing.
    shim = os.path.join(REPO_ROOT, "tools", "permuter-bin", "mips-linux-gnu-as")
    try:
        subprocess.run(
            [shim, target_s, "-o", target_o],
            check=True, capture_output=True, text=True, cwd=REPO_ROOT,
        )
    except (subprocess.CalledProcessError, FileNotFoundError) as exc:
        detail = getattr(exc, "stderr", "") or str(exc)
        print(f"reassembling target.o failed: {detail}", file=sys.stderr)
        print(f"target.s kept at {target_s}.preretarget", file=sys.stderr)
        return 1
    total = sum(n for _, _, n in rewrites)
    print(f"rewrote {total} references and rebuilt target.o")
    print("Re-measure the base score: those rows should stop being counted.")
    return 0


def cmd_align(args: argparse.Namespace) -> int:
    path = resolve_base_c(args.path)
    scratch = os.path.dirname(path)
    target_s = os.path.join(scratch, "target.s")
    if not os.path.isfile(target_s):
        print(f"{scratch}: no target.s to align against", file=sys.stderr)
        return 1
    source = read_text(path)
    renames, interior, unresolved = plan_alignment(source, read_text(target_s))

    new = source
    for project, addr_name in sorted(renames.items()):
        new = re.sub(r"\b" + re.escape(project) + r"\b", addr_name, new)
        print(f"rename  {project} -> {addr_name}")

    strings = re.findall(r'"(?:[^"\\]|\\.)*"', source)
    if args.strings and unresolved:
        if len(unresolved) != len(strings):
            print(
                f"note    {len(unresolved)} unnamed target symbol(s) but "
                f"{len(strings)} string literal(s); pairing skipped",
                file=sys.stderr,
            )
        else:
            decls = []
            for sym, literal in zip(unresolved, strings):
                new = new.replace(literal, sym, 1)
                decls.append(f"extern char {sym}[];")
                print(f"string  {literal} -> {sym}")
            new = insert_after_typedefs(new, decls)

    interior_decls: List[str] = []
    for sym, base, off in interior:
        elem, size = element_type(source, base)
        if elem is None or off % size:
            print(
                f"note    target uses {sym} = {base}+{off}; rewrite the access "
                f"as (&{sym})[...] by hand to drop those rows"
            )
            continue
        idx = off // size
        before = new
        # Only rewrite *expressions*. `extern volatile u32 D_8009AC3C[1];`
        # holds the string `D_8009AC3C[1]` as an array bound, and rewriting it
        # to `extern volatile u32 (&D_8009AC40)[0];` leaves the base symbol
        # undeclared -- gcc 2.6.3 folds it to 0 and the whole search then runs
        # against a program that is not the one being matched. Declarations in
        # an import.py scratch are all file-scope externs, so skipping those
        # lines is enough; permuter_scratch.sh's diagnostics check catches the
        # rest.
        plus_pat = (
            r"\b" + re.escape(base)
            + r"\s*\[\s*([^][]*?)\s*\+\s*" + str(idx) + r"\s*\]"
        )
        flat_pat = r"\b" + re.escape(base) + r"\s*\[\s*" + str(idx) + r"\s*\]"
        out_lines = []
        for line in new.split("\n"):
            if not re.match(r"\s*(extern|static)\b", line):
                line = re.sub(plus_pat, rf"(&{sym})[\1]", line)
                line = re.sub(flat_pat, f"(&{sym})[0]", line)
            out_lines.append(line)
        new = "\n".join(out_lines)
        if new != before:
            interior_decls.append(f"extern {elem} {sym};")
            print(f"interior {base}[...+{idx}] -> (&{sym})[...]")
        else:
            print(
                f"note    target uses {sym} = {base}+{off} but no matching "
                f"index expression found; rewrite by hand"
            )
    if interior_decls:
        new = insert_after_typedefs(new, interior_decls)

    if new == source:
        print("nothing to align")
        return 0
    if args.dry_run:
        print("(dry run, nothing written)")
        return 0
    backup = path + ".prealign"
    if not os.path.exists(backup):
        shutil.copyfile(path, backup)
    write_text(path, new)
    print(f"wrote {path} (original kept at {os.path.basename(backup)})")
    return 0


C_TYPE_SIZES = {
    "u8": 1, "s8": 1, "char": 1,
    "u16": 2, "s16": 2, "short": 2,
    "u32": 4, "s32": 4, "int": 4, "long": 4,
}


def element_type(source: str, array: str) -> Tuple[Optional[str], int]:
    """Element type and size of `array` as declared in base.c."""
    m = re.search(
        r"\b(?:extern\s+)?(?:unsigned\s+|signed\s+)?(\w+)\s*\*?\s*"
        + re.escape(array)
        + r"\s*\[",
        source,
    )
    if not m or m.group(1) not in C_TYPE_SIZES:
        return None, 1
    return m.group(1), C_TYPE_SIZES[m.group(1)]


def insert_after_typedefs(source: str, decls: List[str]) -> str:
    lines = source.splitlines(keepends=True)
    idx = 0
    for i, line in enumerate(lines):
        if line.startswith(("typedef ", "extern ", "#pragma ")):
            idx = i + 1
    return "".join(lines[:idx] + [d + "\n" for d in decls] + lines[idx:])


# --------------------------------------------------------------------------
# plumbing
# --------------------------------------------------------------------------

def resolve_base_c(path: str) -> str:
    if os.path.isdir(path):
        return os.path.join(path, "base.c")
    return path


def scratch_func_name(scratch: str) -> Optional[str]:
    settings = os.path.join(scratch, "settings.toml")
    if not os.path.isfile(settings):
        return None
    m = re.search(r'func_name\s*=\s*"([^"]+)"', read_text(settings))
    return m.group(1) if m else None


def read_text(path: str) -> str:
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


def write_text(path: str, text: str) -> None:
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__.split("\n\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="See docs/decomp/PERMUTER_MACROS.md for the method these serve.",
    )
    sub = parser.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("recipes", help="list the PERM macro recipes")
    p.set_defaults(func=cmd_recipes)

    p = sub.add_parser("recipe", help="print one recipe, ready to paste")
    p.add_argument("name")
    p.set_defaults(func=cmd_recipe)

    p = sub.add_parser("count", help="size of the search a base.c describes")
    p.add_argument("path", help="scratch directory or base.c")
    p.add_argument("-j", "--jobs", type=int, default=8, help="permuter.py -j value")
    p.set_defaults(func=cmd_count)

    p = sub.add_parser("lint", help="check a scratch for the known traps")
    p.add_argument("path", help="scratch directory or base.c")
    p.set_defaults(func=cmd_lint)

    p = sub.add_parser(
        "suggest", help="rank recipes from a saved `permuter.py --debug` output"
    )
    p.add_argument("path", help="scratch directory")
    p.add_argument(
        "debug_output",
        nargs="?",
        help="file with the --debug output, or '-' for stdin "
        "(default: <scratch>/debug.txt)",
    )
    p.set_defaults(func=cmd_suggest)

    p = sub.add_parser(
        "weights",
        help="write R3000A/gcc-2.6.3 tuned randomizer weights into a scratch",
    )
    p.add_argument("path", help="scratch directory")
    p.add_argument("-n", "--dry-run", action="store_true")
    p.set_defaults(func=cmd_weights)

    p = sub.add_parser("align", help="make base.c's symbol names match target.s")
    p.add_argument("path", help="scratch directory or base.c")
    p.add_argument(
        "--strings",
        action="store_true",
        help="also pair string literals with unnamed target .rodata symbols",
    )
    p.add_argument("-n", "--dry-run", action="store_true")
    p.set_defaults(func=cmd_align)

    p = sub.add_parser(
        "retarget", help="make target.s's symbol names match base.c's object"
    )
    p.add_argument("path", help="scratch directory or base.c")
    p.add_argument("-n", "--dry-run", action="store_true")
    p.set_defaults(func=cmd_retarget)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
