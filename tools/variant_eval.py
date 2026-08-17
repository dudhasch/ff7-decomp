#!/usr/bin/env python3
"""Score one variant of a function without touching the shared build.

    .venv/bin/python3 tools/variant_eval.py .variants/my-idea.json --rows

Made for running many experiments at once. `checkfn.py` builds through ninja
into `build/us/...`, which is a single shared path: two agents editing
`src/menu/cnfgmenu.c` and running it concurrently overwrite each other's object
and read each other's verdict. This compiles a variant to a private object in a
temp directory and diffs it with `diff.py -f/-F`, which takes explicit object
paths and so never consults the map file or the build directory. Nothing is
shared but read-only inputs, so N of these can run in parallel.

The variant is described as *edits against a pinned base*, not as a whole file:

    {
      "tag": "dy-as-giv",
      "edits": [
        ["for (i = 0, dy = 0x21; i < 3; i++, dy += 12) {",
         "for (i = 0; i < 3; i++) {\\n            dy = i * 12 + 0x21;"]
      ]
    }

Each `old` must occur exactly once in the base or the run aborts -- a typo that
silently matched nothing would otherwise score as "no change" and be read as a
result. The base is a snapshot (`.variants/_base.c`) with its hash pinned
beside it, so a stray edit to `src/` by one agent cannot quietly change what
everyone else is measuring against.

Verdict format matches checkfn.py: symbol references that resolve to the same
address are counted as aliases, not differences, and the comparison is scoped
to exactly the instructions the target `.s` declares.
"""
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(REPO, "tools"))
import checkfn  # noqa: E402

PYTHON = os.path.join(REPO, ".venv", "bin", "python3")
DIFF = os.path.join(REPO, "tools", "asm-differ", "diff.py")
BASE_C = os.path.join(REPO, ".variants", "_base.c")
BASE_SHA = os.path.join(REPO, ".variants", "_base.sha256")

# src/menu/cnfgmenu.c's line from build.ninja: cc1-psx-272, -O2 -G0 -g -gcoff,
# aspsx 2.21. NON_MATCHINGS selects the C body over the INCLUDE_ASM stub.
SOURCE = "src/menu/cnfgmenu.c"
FUNC = "func_801D080C"
REF_OBJ = "expected/build/us/src/menu/cnfgmenu.c.o"
CC1 = "cc1-psx-272"
CC_FLAGS = ["-O2", "-G0", "-g", "-gcoff"]
AS_FLAGS = ["--expand-div", "--aspsx-version=2.21"]


def die(msg):
    sys.stderr.write("variant_eval: %s\n" % msg)
    sys.exit(2)


def read_base():
    if not os.path.exists(BASE_C):
        die("no base snapshot at %s -- create it with\n"
            "         cp %s .variants/_base.c && sha256sum .variants/_base.c "
            "| cut -d' ' -f1 > .variants/_base.sha256" % (BASE_C, SOURCE))
    with open(BASE_C, encoding="utf-8", newline="") as fh:
        text = fh.read()
    if os.path.exists(BASE_SHA):
        want = open(BASE_SHA).read().strip()
        got = hashlib.sha256(text.encode("utf-8")).hexdigest()
        if want != got:
            die("the base snapshot has changed under us (%s != %s).\n"
                "         Every variant measured against it is now "
                "incomparable; stop and re-pin." % (got[:12], want[:12]))
    return text


def apply_edits(text, edits):
    for n, edit in enumerate(edits):
        if len(edit) != 2:
            die("edit %d is not a [old, new] pair" % n)
        old, new = edit
        count = text.count(old)
        if count != 1:
            die("edit %d matched %d times, expected exactly 1:\n---\n%s\n---"
                % (n, count, old))
        text = text.replace(old, new)
    return text


# The compile line ends in the assembler, so the pipeline's exit status is the
# *assembler's*: gcc 2.7.2 reports an undeclared identifier, folds it to 0 and
# keeps generating code, and nothing downstream fails. Same trap checkfn.py
# guards against -- see CLAUDE.md.
DIAG_RE = re.compile(r"^\S.*:\d+: (?!warning:)")


def compile_variant(src, obj, work):
    cpp = subprocess.run(
        ["mipsel-linux-gnu-cpp", "-Iinclude", "-Iinclude/psxsdk",
         "-DUSE_INCLUDE_ASM", "-DFF7_STR", "-DNON_MATCHINGS",
         "-MMD", "-MF", os.path.join(work, "dep.d"), "-lang-c",
         "-undef", "-Wall", "-fno-builtin", src],
        cwd=REPO, capture_output=True)
    if cpp.returncode != 0:
        die("cpp failed:\n%s" % cpp.stderr.decode("utf-8", "replace"))

    script = (
        "set -o pipefail; "
        "bin/str | iconv --from-code=UTF-8 --to-code=Shift-JIS "
        "| bin/%s -quiet -mcpu=3000 -mgas %s "
        "| python3 tools/maspsx/maspsx.py %s "
        "| mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 "
        "-no-pad-sections -O1 -G0 -o %s"
        % (CC1, " ".join(CC_FLAGS), " ".join(AS_FLAGS), obj))
    proc = subprocess.run(["bash", "-c", script], cwd=REPO,
                          input=cpp.stdout, capture_output=True)
    out = proc.stdout.decode("utf-8", "replace") + \
        proc.stderr.decode("utf-8", "replace")
    bad = [l for l in out.splitlines() if DIAG_RE.match(l)]
    if bad:
        die("the variant does not compile -- gcc substitutes 0 for what it "
            "cannot resolve\n         and carries on, so this would otherwise "
            "score as a near-miss:\n\n%s"
            % "\n".join("         " + l for l in bad))
    if proc.returncode != 0:
        die("compile pipeline failed:\n%s" % out)
    if not os.path.exists(obj):
        die("no object produced")


def score(obj, want, rows_wanted, context):
    proc = subprocess.run(
        [PYTHON, DIFF, "-o", "--format=json", "-f", obj, "-F", REF_OBJ,
         FUNC, "--max-lines", str(want * 2 + 128)],
        cwd=REPO, capture_output=True, text=True)
    if proc.returncode != 0:
        die("diff.py failed:\n%s" % proc.stderr.strip())
    rows = json.loads(proc.stdout)["rows"]

    scoped, seen = [], 0
    for row in rows:
        if seen >= want and row.get("base"):
            break
        scoped.append(row)
        if row.get("base"):
            seen += 1
    if seen < want:
        die("diff.py returned only %d of %d instructions" % (seen, want))

    syms = checkfn.load_symbols()
    bases = checkfn.calibrate_sections(scoped, syms)
    kinds = {}
    for n, row in enumerate(scoped):
        b = checkfn.text_of(row.get("base"))
        c = checkfn.text_of(row.get("current"))
        if b is None:
            kinds[n] = "INS"
        elif c is None:
            kinds[n] = "DEL"
        elif b == c:
            continue
        elif checkfn.normalise(b, syms, bases) == checkfn.normalise(c, syms, bases):
            continue
        else:
            kinds[n] = "CHG"

    chg = sum(1 for k in kinds.values() if k == "CHG")
    ins = sum(1 for k in kinds.values() if k == "INS")
    dele = sum(1 for k in kinds.values() if k == "DEL")
    # Rows, not instructions: asm-differ renders a moved instruction as a
    # changed row against a `<` marker rather than as a delete/insert pair, so
    # these do not subtract cleanly from the 1205 the .s declares.
    print("TOTAL %d  (%d changed, %d inserted, %d deleted; target has %d insns)"
          % (chg + ins + dele, chg, ins, dele, want))

    if not rows_wanted:
        return chg + ins + dele
    show = set()
    for n in kinds:
        show.update(range(n - context, n + context + 1))
    prev = -1
    for n in sorted(show):
        if not (0 <= n < len(scoped)):
            continue
        if n != prev + 1:
            print("     ...")
        prev = n
        line = (scoped[n].get("current") or {}).get("src_line", "")
        print("%-3s %-4s %-40s | %-40s %s"
              % (kinds.get(n, ""), n,
                 checkfn.text_of(scoped[n].get("base")) or "",
                 checkfn.text_of(scoped[n].get("current")) or "", line))
    return chg + ins + dele


def main(argv):
    if len(argv) < 2:
        sys.stderr.write(__doc__)
        return 2
    spec_path = argv[1]
    rows_wanted = "--rows" in argv
    keep = "--keep" in argv
    context = 3
    for a in argv:
        if a.startswith("--context="):
            context = int(a.split("=", 1)[1])

    with open(spec_path, encoding="utf-8") as fh:
        spec = json.load(fh)
    tag = spec.get("tag") or os.path.basename(spec_path).rsplit(".", 1)[0]
    text = apply_edits(read_base(), spec.get("edits", []))

    want = checkfn.target_insn_count(os.path.join(REPO, SOURCE), FUNC)
    work = tempfile.mkdtemp(prefix="variant-%s-" % re.sub(r"\W", "_", tag))
    try:
        src = os.path.join(work, "cnfgmenu.c")
        with open(src, "w", encoding="utf-8", newline="") as fh:
            fh.write(text)
        obj = os.path.join(work, "variant.o")
        compile_variant(src, obj, work)
        print("VARIANT %s" % tag)
        score(obj, want, rows_wanted, context)
    finally:
        if keep:
            sys.stderr.write("kept %s\n" % work)
        else:
            shutil.rmtree(work, ignore_errors=True)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
