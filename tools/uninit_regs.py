"""Find registers a compiled function reads before anything writes them.

    sh tools/variant_disasm.sh .variants/abr_base.json /tmp/f.dis
    .venv/bin/python3 tools/uninit_regs.py /tmp/f.dis AddBackgroundToRender

A hit means the C reads a local whose only assignment sits on a path gcc
proved unreachable. gcc 2.6.3 accepts such a statement, `jump_optimize`
deletes the block, the pseudo keeps its *use* and loses its *def*, and
`global_alloc` hands the undefined value a hard register -- so the function
compiles, links, and scores. It can score *better* than the correct program,
because an allocno whose live range runs back to the function entry perturbs
the whole conflict graph. `AddBackgroundToRender` in `src/field/field.c` read
`0($s3)` this way and measured 65 rows against the correct body's 72; the
seven rows were carried in its park note as a lever for three sessions.

The shape that produces it is a statement between the close of an infinite
loop and a label that loop only reaches by `goto`:

    for (;;) { ... goto layer3; ... }   /* no break: never falls through */

    layer3Slot = &D_8009ACA2.layer3;    /* unreachable, silently dropped */
layer3:

Run it on every parked body before trusting a row count, and on any diff that
shows *your* build using a callee-saved register where the target uses a temp
-- that is what an allocno with no definition looks like.

The scan is linear over the disassembly, so it can over-report on a function
whose only write to a register sits below a backward branch into code above
it. A hit is a lead to confirm by reading, not a verdict. It does not
under-report: a register with no write anywhere is always flagged.

Confirming a hit takes one grep -- find who branches to the flagged block:

    grep -n '<0x-of-the-block> <' <dis>      # e.g. "367c <"

`FieldDialogCopyTextFromField` in `src/field/field5.c` is the worked example
of the false positive: it reads `$s6` at 0x3684 and `$s8` at 0x3698 where the
writes are at 0x3748 and 0x3740, and the block looks unreachable because the
instruction above it is the delay slot of a `j`. It is reached, by a `beq` at
0x3824 -- below both definitions. Nothing to fix. A *real* hit has no branch
into it from below the write, which is what makes `AddBackgroundToRender`'s
`0($s3)` a bug: nothing in the function writes `$s3` at all.

Run it across a whole unit by compiling with `-DNON_MATCHINGS`, which unparks
every body at once. The offsets are wrong in that configuration (each parked
body is a different length than its `.s`) and that does not matter here -- an
undefined register read is visible whatever the neighbours do. All five
`src/field/*.c` units were audited that way; only the false positive above
came back.
"""
import re
import sys

# Live on entry by the ABI, so reading them proves nothing.
INCOMING = set("a0 a1 a2 a3 sp ra zero at gp fp s8 k0 k1 t9".split())
REG = r"\b(v[01]|a[0-3]|t[0-9]|s[0-8]|at|ra|sp|zero|k[01]|gp|fp)\b"
STORES = ("sw", "sh", "sb", "swl", "swr", "swc1", "sdc1")
BRANCHES = ("b", "j")


def body_of(lines, fname):
    """The instruction lines of one function, by objdump's own labels."""
    i = 0
    while i < len(lines) and ("<%s>:" % fname) not in lines[i]:
        i += 1
    if i == len(lines):
        return None
    out = []
    i += 1
    while i < len(lines):
        ln = lines[i]
        # A new top-level symbol ends the function; gcc's own -g line labels
        # (<LMnnn>:) and interior <fn+0xN> labels do not.
        if re.match(r"^[0-9a-f]{8} <", ln) and "<LM" not in ln and "+0x" not in ln:
            break
        m = re.match(r"^\s+[0-9a-f]+:\s+[0-9a-f]{8}\s+(\S+)\s*(.*)$", ln)
        if m:
            out.append((m.group(1), m.group(2)))
        i += 1
    return out


def scan(body):
    written, bad = set(INCOMING), []
    for op, ops in body:
        # A call defines the return registers. This has to come before the
        # "no registers named" shortcut below, because `jal 0 <Callee>` names
        # none -- getting that order wrong reports every function that stores
        # a call result as reading $v0 uninitialised.
        if op in ("jal", "jalr"):
            written.add("v0")
            written.add("v1")
            continue
        regs = re.findall(REG, ops)
        if not regs:
            continue
        # A callee-saved prologue save reads the *caller's* value and defines
        # nothing. Counting it as a definition is how an earlier version of
        # this script reported the known-bad body as clean.
        if op == "sw" and ops.endswith("(sp)") and re.match(r"^s[0-8],", ops):
            continue
        if op.startswith(STORES) or op.startswith(BRANCHES) or op == "nop":
            dst, srcs = None, regs
        else:
            dst, srcs = regs[0], regs[1:]
        for r in srcs:
            if r not in written:
                bad.append((op, ops, r))
                written.add(r)  # report each register once
        if dst:
            written.add(dst)
    return bad


def main(argv):
    if len(argv) < 3:
        sys.exit("usage: uninit_regs.py <objdump-output> <FuncName> [FuncName...]")
    lines = open(argv[1]).read().split("\n")
    rc = 0
    for fname in argv[2:]:
        body = body_of(lines, fname)
        if body is None:
            print("%s: not found in %s" % (fname, argv[1]))
            rc = 2
            continue
        bad = scan(body)
        print("%s: %d instructions" % (fname, len(body)))
        if not bad:
            print("  clean")
            continue
        rc = 1
        for op, ops, r in bad:
            print("  READ-BEFORE-WRITE  $%-3s in   %s %s" % (r, op, ops))
    return rc


if __name__ == "__main__":
    sys.exit(main(sys.argv))
