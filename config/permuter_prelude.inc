# Assembly prelude prepended to the target .s that decomp-permuter scores
# candidates against. Referenced from permuter_settings.toml via
# `asm_prelude_file`.
#
# decomp-permuter's bundled prelude.inc is written for N64: it opens with
# `.set gp=64`, which mipsel-linux-gnu-as rejects on this project's R3000A
# target with `Error: 'gp=64' used with a 32-bit processor`. This is that
# prelude minus the 64-bit directive. The o32 float register aliases are also
# dropped -- the PS1 has no FPU, so no FF7 target .s ever references them.

.set noat
.set noreorder

# The GTE mnemonics (nccs, nclip, mvmva, ...) are macros, not opcodes:
# mipsel-linux-gnu-as does not know them, and every target .s that lights
# a model uses them. The candidate objects get these through macro.inc,
# which INCLUDE_ASM's expansion pulls in; the reference object assembled by
# import.py never did, so importing any Kawai* function died with
# `Error: unrecognized opcode 'nccs'` and left no scratch behind.
# gte.inc spells them upper-case and GAS matches macro names
# case-insensitively, so the lower-case forms in asm/ resolve.
.include "gte.inc"

.macro glabel label, visibility=global
    .\visibility \label
    .type \label, @function
    \label:
.endm

.macro endlabel label
    .size \label, . - \label
.endm

.macro alabel label, visibility=global
    .\visibility \label
    .type \label, @function
    \label:
.endm

.macro ehlabel label, visibility=global
    .\visibility \label
    \label:
.endm


.macro jlabel label, visibility=local
    \label:
.endm


.macro dlabel label, visibility=global
    .\visibility \label
    .type \label, @object
    \label:
.endm

.macro enddlabel label
    .size \label, . - \label
.endm


.macro nonmatching label, size=1
    .global \label\().NON_MATCHING
    .type \label\().NON_MATCHING, @object
    .size \label\().NON_MATCHING, \size
    \label\().NON_MATCHING:
.endm
