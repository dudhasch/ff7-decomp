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

The spec names its own function, so one tool serves the whole repo:

    {"tag": "...", "source": "src/field/field4.c",
     "func": "KawaiSetVertexColorFromLighting", "edits": [[...]]}

Compiler, assembler and jump-table flags are read out of `build.ninja`'s edge
for that object rather than duplicated here -- the `//!` header's meaning lives
in tools/ninja/gen.py and a second copy of it would drift. The reference is
`expected/build/us/<source>.o`.

Pass several specs to run them concurrently:

    .venv/bin/python3 tools/variant_eval.py .variants/*.json --jobs 8

Pin a base per source before sweeping (once, and re-pin after landing a change):

    .venv/bin/python3 tools/variant_eval.py --pin src/field/field4.c
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
NINJA = os.path.join(REPO, "build.ninja")


def base_paths(source):
    """One pinned snapshot per source file, so several functions in different
    units can be swept at the same time without sharing a base."""
    stem = re.sub(r"\W", "_", source)
    d = os.path.join(REPO, ".variants")
    return os.path.join(d, "_base_%s.c" % stem), os.path.join(d, "_base_%s.sha256" % stem)


def resolve_build(source):
    """Read the object's cc1 / cc_flags / as_flags / jtbl_flags out of
    build.ninja. The `//!` header's meaning lives in tools/ninja/gen.py; a
    second parser here would drift the first time someone adds a PSYQ version,
    and the failure would be a wrong verdict rather than an error."""
    if not os.path.exists(NINJA):
        die("no build.ninja -- run `make build` once so the flags can be read")
    obj = "build/us/%s.o" % source
    with open(NINJA, encoding="utf-8") as fh:
        text = fh.read()
    m = re.search(r"^build %s: psx-cc .*?(?=^build )" % re.escape(obj),
                  text, re.M | re.S)
    if not m:
        die("build.ninja has no psx-cc edge for %s" % obj)
    edge = m.group(0)
    var = lambda k, d: (re.search(r"^\s+%s = (.*)$" % k, edge, re.M) or [None, d])[1]
    return {
        "cc1": var("cc1", "cc1-psx-272").strip(),
        "cc_flags": var("cc_flags", "-O2 -G0 -g -gcoff").split(),
        "as_flags": var("as_flags", "--expand-div --aspsx-version=2.34").split(),
        "jtbl_flags": var("jtbl_flags", "").split(),
        "ref_obj": "expected/%s" % obj,
    }


def die(msg):
    sys.stderr.write("variant_eval: %s\n" % msg)
    sys.exit(2)


def read_base(source):
    base_c, base_sha = base_paths(source)
    if not os.path.exists(base_c):
        die("no base snapshot for %s -- pin one with\n"
            "         .venv/bin/python3 tools/variant_eval.py --pin %s"
            % (source, source))
    with open(base_c, encoding="utf-8", newline="") as fh:
        text = fh.read()
    if os.path.exists(base_sha):
        want = open(base_sha).read().strip()
        got = hashlib.sha256(text.encode("utf-8")).hexdigest()
        if want != got:
            die("the base snapshot for %s has changed under us (%s != %s).\n"
                "         Every variant measured against it is now "
                "incomparable; stop and re-pin." % (source, got[:12], want[:12]))
    return text


def pin(source):
    d = os.path.join(REPO, ".variants")
    if not os.path.isdir(d):
        os.makedirs(d)
    base_c, base_sha = base_paths(source)
    with open(os.path.join(REPO, source), encoding="utf-8", newline="") as fh:
        text = fh.read()
    with open(base_c, "w", encoding="utf-8", newline="") as fh:
        fh.write(text)
    with open(base_sha, "w", encoding="utf-8") as fh:
        fh.write(hashlib.sha256(text.encode("utf-8")).hexdigest() + "\n")
    sys.stderr.write("pinned %s -> %s\n" % (source, os.path.relpath(base_c, REPO)))


def unpark(text, name):
    """Make one MASPSX_OVERRIDE-parked body the live one, exactly as
    tools/unpark.py does to the real file.

    Compiling the whole unit with -DNON_MATCHINGS instead -- which is what this
    tool used to do -- replaces *every* parked function's pinned .s with its C
    body, and those bodies are the wrong length. Everything after the first one
    then sits at the wrong offset, so a byte-perfect function reads as a wall of
    branch-target rows off by a constant. Measured: KawaiLightingApplyToPolyColor
    scored 22 rows that way while checkfn said MATCH, all of them branch targets
    4 bytes low, because the parked KawaiSetVertexColorFromLighting above it is
    one instruction longer as C than as asm.
    """
    lines = text.split(chr(10))
    ov = None
    for i, ln in enumerate(lines):
        if re.match(r"\s*MASPSX_OVERRIDE\s*\(", ln):
            blob = chr(10).join(lines[i:i + 3])
            if re.search(r"\b%s\s*\)" % re.escape(name), blob):
                ov = i
                break
    if ov is None:
        return text          # already live; nothing to do
    start = ov
    while not lines[start].startswith("#ifndef NON_MATCHINGS"):
        start -= 1
    els = ov
    while not lines[els].startswith("#else"):
        els += 1
    end, depth = els + 1, 0
    while True:
        ln = lines[end]
        if ln.startswith("#if"):
            depth += 1
        elif ln.startswith("#endif"):
            if depth == 0:
                break
            depth -= 1
        end += 1
    return chr(10).join(lines[:start] + lines[els + 1:end] + lines[end + 1:])


def apply_edits(text, edits):
    """Each edit is [old, new] and must match exactly once, or [old, new, n]
    and must match exactly n times -- a deliberate repeat, for a lever that
    applies to several identical arms of a switch. The count is required
    either way so that a typo cannot silently match nothing and score as
    "no change"."""
    for k, edit in enumerate(edits):
        if len(edit) == 2:
            (old, new), want = edit, 1
        elif len(edit) == 3:
            old, new, want = edit
        else:
            die("edit %d is not [old, new] or [old, new, count]" % k)
        count = text.count(old)
        if count != want:
            die("edit %d matched %d times, expected exactly %d:\n---\n%s\n---"
                % (k, count, want, old))
        text = text.replace(old, new)
    return text


# The compile line ends in the assembler, so the pipeline's exit status is the
# *assembler's*: gcc 2.7.2 reports an undeclared identifier, folds it to 0 and
# keeps generating code, and nothing downstream fails. Same trap checkfn.py
# guards against -- see CLAUDE.md.
DIAG_RE = re.compile(r"^\S.*:\d+: (?!warning:)")


def compile_variant(src, obj, work, cfg, orig):
    # The variant is written to a temp directory, so a quoted include like
    # "field_private.h" no longer resolves next to the source. Put the real
    # source's directory on the search path, which is where cpp would have
    # found it in the normal build.
    cpp = subprocess.run(
        ["mipsel-linux-gnu-cpp", "-I" + os.path.dirname(orig), "-Iinclude", "-Iinclude/psxsdk",
         "-DUSE_INCLUDE_ASM", "-DFF7_STR",
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
        "| python3 tools/psx_jtbl_align.py %s "
        "| mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 "
        "-no-pad-sections -O1 -G0 -o %s"
        % (cfg["cc1"], " ".join(cfg["cc_flags"]),
           " ".join(cfg["as_flags"]), " ".join(cfg["jtbl_flags"]), obj))
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


def score(obj, want, rows_wanted, context, cfg, func, out):
    proc = subprocess.run(
        [PYTHON, DIFF, "-o", "--format=json", "-f", obj, "-F", cfg["ref_obj"],
         func, "--max-lines", str(want * 2 + 128)],
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
    out.append("TOTAL %d  (%d changed, %d inserted, %d deleted; target has %d insns)"
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
            out.append("     ...")
        prev = n
        line = (scoped[n].get("current") or {}).get("src_line", "")
        out.append("%-3s %-4s %-40s | %-40s %s"
                   % (kinds.get(n, ""), n,
                      checkfn.text_of(scoped[n].get("base")) or "",
                      checkfn.text_of(scoped[n].get("current")) or "", line))
    return chg + ins + dele


def run_one(spec_path, rows_wanted, keep, context):
    """Score one spec. Everything it touches is private to this call: its own
    temp directory, its own object, and read-only repo inputs -- so N of these
    run concurrently without a shared build directory to corrupt."""
    out = []
    with open(spec_path, encoding="utf-8") as fh:
        spec = json.load(fh)
    tag = spec.get("tag") or os.path.basename(spec_path).rsplit(".", 1)[0]
    source = spec.get("source")
    func = spec.get("func")
    if not source or not func:
        die("%s: spec needs \"source\" and \"func\"" % spec_path)

    cfg = resolve_build(source)
    text = unpark(apply_edits(read_base(source), spec.get("edits", [])), func)
    want = checkfn.target_insn_count(os.path.join(REPO, source), func)

    work = tempfile.mkdtemp(prefix="variant-%s-" % re.sub(r"\W", "_", tag))
    try:
        src = os.path.join(work, os.path.basename(source))
        with open(src, "w", encoding="utf-8", newline="") as fh:
            fh.write(text)
        obj = os.path.join(work, "variant.o")
        compile_variant(src, obj, work, cfg, source)
        out.append("VARIANT %s  [%s %s]" % (tag, source, func))
        total = score(obj, want, rows_wanted, context, cfg, func, out)
    finally:
        if keep:
            out.append("kept %s" % work)
        else:
            shutil.rmtree(work, ignore_errors=True)
    return tag, total, out


def main(argv):
    args = argv[1:]
    if not args:
        sys.stderr.write(__doc__)
        return 2

    if args[0] == "--pin":
        if len(args) < 2:
            die("--pin needs a source path, e.g. src/field/field4.c")
        for s in args[1:]:
            pin(s)
        return 0

    rows_wanted = "--rows" in args
    keep = "--keep" in args
    context = 3
    jobs = 1
    specs = []
    for a in args:
        if a.startswith("--context="):
            context = int(a.split("=", 1)[1])
        elif a.startswith("--jobs="):
            jobs = int(a.split("=", 1)[1])
        elif a in ("--rows", "--keep"):
            continue
        else:
            specs.append(a)
    if not specs:
        die("no spec files given")

    # A failed spec must not take the batch down with it: die() exits the
    # process, so each spec runs in its own worker and a non-zero exit is
    # reported against that tag alone.
    results = []
    if jobs > 1 and len(specs) > 1:
        import multiprocessing.pool
        with multiprocessing.pool.ThreadPool(min(jobs, len(specs))) as pool:
            futs = [(s, pool.apply_async(_guarded, (s, rows_wanted, keep, context)))
                    for s in specs]
            for s, f in futs:
                results.append(f.get())
    else:
        results = [_guarded(s, rows_wanted, keep, context) for s in specs]

    worst = 0
    for tag, total, out in results:
        for line in out:
            print(line)
        if total is None:
            worst = 2
        print("")
    if len(results) > 1:
        print("=== summary, best first")
        for tag, total, _ in sorted(results, key=lambda r: (r[1] is None, r[1])):
            print("  %-40s %s" % (tag, "FAILED" if total is None else total))
    return worst


def _guarded(spec_path, rows_wanted, keep, context):
    import io as _io
    import contextlib
    tag = os.path.basename(spec_path).rsplit(".", 1)[0]
    err = _io.StringIO()
    try:
        with contextlib.redirect_stderr(err):
            return run_one(spec_path, rows_wanted, keep, context)
    except SystemExit:
        return tag, None, ["VARIANT %s" % tag] +             ["  " + l for l in err.getvalue().rstrip().splitlines()]
    except Exception as exc:  # noqa: BLE001 - one bad spec must not kill the batch
        return tag, None, ["VARIANT %s" % tag, "  %s: %s" % (type(exc).__name__, exc)]


if __name__ == "__main__":
    sys.exit(main(sys.argv))
