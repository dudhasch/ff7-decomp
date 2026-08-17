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
