#!/usr/bin/env python3
"""Rank the remaining INCLUDE_ASM functions without needing disks/ or asm/.

`./mako.sh rank` reads the split assembly and scores each function with a
trained difficulty model. That needs the retail executable, so it cannot run in
an environment that has no `disks/us/SCUS_941.63` -- which includes every fresh
clone, since the game data is not (and cannot be) distributed with the source.

This tool answers a coarser version of the same question using only files that
are committed: the `.c` sources and `config/`. For every function still behind
an `INCLUDE_ASM` stub it reports

  size      bytes to the next function in the same overlay, stopped at the end
            of the function's own segment, so instruction count is size/4. The
            per-unit sizes sum exactly to the segment spans in config/us.yaml,
            so these are measurements rather than estimates. Size is the
            strongest predictor of difficulty available without reading the
            assembly, and a very small function is almost always a leaf.
  callers   how many times the name is referenced from already-decompiled C.
            Zero means no matched code depends on it yet -- an isolated helper
            that can be attempted, and reverted, without disturbing anything.
  export    whether the overlay exports the symbol (config/sym_ovl_export.us.txt)
            or the main executable does (config/sym_extern.us.txt).

None of this replaces `mako.sh rank`; it is a work queue for deciding *where to
look* when the ranker is unavailable. It knows nothing about register pressure,
control flow, or .rodata ownership -- run tools/rodata_owner.py before writing
any C, as always.

Usage:
    tools/worklist.py                       # every overlay, smallest first
    tools/worklist.py --overlay battle      # one overlay
    tools/worklist.py --unit battle3        # one splat unit
    tools/worklist.py --isolated --max-size 128
    tools/worklist.py --summary             # per-unit progress only
    tools/worklist.py --json                # machine-readable
"""

import argparse
import collections
import glob
import json
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# A function definition at the top level of a .c file. gcc-era style in this
# repo puts the return type and the name on one line, so anchoring at column 0
# and requiring the opening brace is enough to avoid matching calls and
# prototypes. `[^;]*?` keeps it from running across a `;` into the next
# construct.
C_DEFINITION = re.compile(r"^[A-Za-z_][\w \t\*]*?(\w+)\s*\([^;]*?\)\s*\{", re.M)
INCLUDE_ASM = re.compile(r'INCLUDE_ASM\("([^"]+)",\s*(\w+)\)')
SYMBOL_LINE = re.compile(r"\s*(\w+)\s*=\s*(0x[0-9a-fA-F]+)")
AUTO_NAME = re.compile(r"func_8[0-9A-Fa-f]{7}\Z")


def load_symbols():
    """Every `name = 0xADDR` in config/, first definition wins."""
    symbols = {}
    for path in sorted(glob.glob(os.path.join(REPO, "config", "sym*.txt"))):
        with open(path, errors="replace") as fh:
            for line in fh:
                m = SYMBOL_LINE.match(line)
                if m:
                    symbols.setdefault(m.group(1), int(m.group(2), 16))
    return symbols


def load_units():
    """Map each splat `c` unit to its overlay and its VRAM extent.

    Overlays share VRAM -- battle, brom, dschange, ending, field and world all
    start at 0x800A0000, and the four menu overlays all start at 0x801D0000 --
    so addresses are only comparable within one overlay.

    Units inside an overlay are *not* always adjacent: `world` puts a 10956-byte
    `asm` segment (world_unk) between its two `c` segments. Carrying each unit's
    end address lets measure() stop a function at its own segment boundary
    instead of swallowing whatever follows.

    Segment offsets are file offsets; the first segment of an overlay sits at
    vram_start, so vram = vram_start + offset - first_offset.
    """
    import yaml

    with open(os.path.join(REPO, "config", "us.yaml"), errors="replace") as fh:
        config = yaml.safe_load(fh)

    units = {}
    for overlay in config["overlays"]:
        segments = [s for s in overlay["segments"]
                    if isinstance(s, list) and s and s[0] is not None]
        if not segments:
            continue
        base = overlay["vram_start"] - segments[0][0]
        for index, segment in enumerate(segments):
            if len(segment) > 2 and segment[1] == "c":
                end = (segments[index + 1][0] if index + 1 < len(segments) else None)
                units[segment[2]] = {
                    "overlay": overlay["name"],
                    "start": base + segment[0],
                    "end": None if end is None else base + end,
                }
    return units


def address_of(name, symbols):
    """Auto-generated names carry their own address; the rest need config/."""
    if AUTO_NAME.match(name):
        return int(name[5:], 16)
    return symbols.get(name)


def collect(symbols, units):
    """Walk src/ and return every function, matched or not."""
    functions = []
    unresolved = []

    for source in sorted(glob.glob(os.path.join(REPO, "src", "**", "*.c"), recursive=True)):
        relative = os.path.relpath(source, REPO)
        text = open(source, errors="replace").read()
        unit = os.path.splitext(os.path.basename(source))[0]
        if unit not in units:
            # src/main/ovl.c is a .data segment with no `c` unit of its own.
            continue

        stubbed = set()
        for m in INCLUDE_ASM.finditer(text):
            name = m.group(2)
            stubbed.add(name)
            # The unit in the INCLUDE_ASM path is authoritative; a file glued
            # together from several original .c files still names each one.
            functions.append(
                {
                    "name": name,
                    "unit": os.path.basename(m.group(1)),
                    "file": relative,
                    "done": False,
                }
            )

        for m in C_DEFINITION.finditer(text):
            name = m.group(1)
            if name in stubbed:
                continue
            functions.append({"name": name, "unit": unit, "file": relative, "done": True})

    for function in functions:
        function["overlay"] = units.get(function["unit"], {}).get("overlay", "?")
        function["addr"] = address_of(function["name"], symbols)
        function["size"] = None
        if function["addr"] is None:
            unresolved.append(function)

    # Unaddressed functions stay in the list so per-unit totals are honest;
    # measure() skips them, so they simply never get a size. In practice they
    # are file-static helpers in a fully matched overlay, never work items.
    return functions, unresolved


def measure(functions, units):
    """Size of each function = distance to the next one in the same overlay,
    stopped at the end of the function's own `c` segment.

    Every function in an overlay is either an INCLUDE_ASM stub or a C
    definition -- anything else would fail to link -- so the address list is
    complete and consecutive deltas are exact. The clamp matters at the two
    kinds of boundary: a unit followed by a non-code segment, and the last
    function of a unit, which would otherwise have no successor at all.

    The only imprecision left is alignment padding after a unit's final
    function, which inflates that one function by at most a few bytes.
    """
    by_overlay = collections.defaultdict(list)
    for function in functions:
        if function["addr"] is not None:
            by_overlay[function["overlay"]].append(function)

    for group in by_overlay.values():
        group.sort(key=lambda f: f["addr"])
        for index, current in enumerate(group):
            limit = units.get(current["unit"], {}).get("end")
            if index + 1 < len(group):
                nxt = group[index + 1]["addr"]
                limit = nxt if limit is None else min(limit, nxt)
            current["size"] = None if limit is None else limit - current["addr"]


# A file-scope prototype: a declaration ending in `;` rather than a body. These
# are not call sites, and counting them would give every declared function a
# phantom caller.
PROTOTYPE = re.compile(r"^[A-Za-z_][\w \t\*]*?\w+\s*\([^;{]*\)\s*;")


def count_callers(functions):
    """Call sites in decompiled C, and whether a header declares the function.

    Only `src/**/*.c` is counted, and only lines that are neither the function's
    own INCLUDE_ASM stub nor a prototype. A count of zero means nothing that has
    been decompiled so far refers to the function -- it can be attempted, and
    reverted, without disturbing matched code.

    `declared` is tracked separately: a PSY-Q prototype in include/psxsdk/ tells
    you the exact signature, which is a large head start, but it is emphatically
    not a caller.
    """
    names = {f["name"] for f in functions}
    counts = collections.Counter()

    for path in sorted(glob.glob(os.path.join(REPO, "src", "**", "*.c"), recursive=True)):
        for line in open(path, errors="replace"):
            if "INCLUDE_ASM(" in line or PROTOTYPE.match(line):
                continue
            for token in re.findall(r"\b\w+\b", line):
                if token in names:
                    counts[token] += 1

    headers = "".join(open(h, errors="replace").read()
                      for h in glob.glob(os.path.join(REPO, "include", "**", "*.h"),
                                         recursive=True))
    declared = set(re.findall(r"\b(\w+)\s*\(", headers))

    for function in functions:
        # For a stub the only textual occurrence is the call sites; for a
        # matched function its own definition line also lands here, which is
        # why callers is only reported for stubs.
        function["callers"] = counts[function["name"]]
        function["declared"] = function["name"] in declared


def load_exports():
    exported = {}
    for path, label in (
        ("config/sym_ovl_export.us.txt", "ovl"),
        ("config/sym_extern.us.txt", "extern"),
    ):
        full = os.path.join(REPO, path)
        if not os.path.exists(full):
            continue
        with open(full, errors="replace") as fh:
            for line in fh:
                m = SYMBOL_LINE.match(line)
                if m:
                    exported.setdefault(m.group(1), label)
    return exported


def summarise(functions):
    rows = collections.defaultdict(lambda: [0, 0])
    for function in functions:
        rows[(function["overlay"], function["unit"])][function["done"]] += 1

    print(f"{'overlay':10s} {'unit':10s} {'done':>6s} {'left':>6s} {'total':>6s}   progress")
    total_done = total_left = 0
    for (overlay, unit), (left, done) in sorted(rows.items()):
        total = done + left
        total_done += done
        total_left += left
        share = done / total if total else 0.0
        bar = "#" * round(share * 24)
        print(f"{overlay:10s} {unit:10s} {done:6d} {left:6d} {total:6d}   {share:6.1%} {bar}")

    total = total_done + total_left
    print(f"{'':10s} {'TOTAL':10s} {total_done:6d} {total_left:6d} {total:6d}   "
          f"{total_done / total:6.1%}")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--overlay", help="restrict to one overlay")
    parser.add_argument("--unit", help="restrict to one splat unit")
    parser.add_argument("--exclude-unit", action="append", default=[], metavar="UNIT",
                        help="drop a unit; repeatable. `--exclude-unit psxsdk` is the "
                             "usual one, since the PSY-Q library dominates the small "
                             "end of the list without being game code")
    parser.add_argument("--isolated", action="store_true",
                        help="only functions no decompiled C references")
    parser.add_argument("--max-size", type=int, metavar="BYTES",
                        help="only functions at or below this size")
    parser.add_argument("--declared", action="store_true",
                        help="only functions a header already prototypes")
    parser.add_argument("--limit", type=int, default=40, help="rows to print (0 = all)")
    parser.add_argument("--summary", action="store_true", help="per-unit progress only")
    parser.add_argument("--json", action="store_true", help="emit JSON")
    args = parser.parse_args()

    symbols = load_symbols()
    units = load_units()
    functions, unresolved = collect(symbols, units)
    measure(functions, units)
    count_callers(functions)
    exported = load_exports()

    if args.summary:
        summarise(functions)
        if unresolved:
            print(f"\n{len(unresolved)} function(s) counted above but not sized, having "
                  f"no address in config/:",
                  ", ".join(sorted(f["name"] for f in unresolved)))
        return 0

    remaining = [f for f in functions if not f["done"]]
    if args.overlay:
        remaining = [f for f in remaining if f["overlay"] == args.overlay]
    if args.unit:
        remaining = [f for f in remaining if f["unit"] == args.unit]
    if args.exclude_unit:
        remaining = [f for f in remaining if f["unit"] not in args.exclude_unit]
    if args.isolated:
        remaining = [f for f in remaining if f["callers"] == 0]
    if args.declared:
        remaining = [f for f in remaining if f["declared"]]
    if args.max_size is not None:
        remaining = [f for f in remaining
                     if f["size"] is not None and f["size"] <= args.max_size]

    for function in remaining:
        function["export"] = exported.get(function["name"], "")
        function["named"] = not AUTO_NAME.match(function["name"])

    # Unsized functions sort last; they are overlay tails, not easy wins.
    remaining.sort(key=lambda f: (f["size"] is None, f["size"] or 0, f["name"]))

    if args.json:
        json.dump([
            {
                "name": f["name"],
                "overlay": f["overlay"],
                "unit": f["unit"],
                "file": f["file"],
                "addr": f"0x{f['addr']:08X}",
                "size": f["size"],
                "instructions": None if f["size"] is None else f["size"] // 4,
                "callers": f["callers"],
                "declared": f["declared"],
                "export": f["export"],
            }
            for f in remaining
        ], sys.stdout, indent=2)
        print()
        return 0

    shown = remaining if args.limit == 0 else remaining[:args.limit]
    print(f"{'insns':>6s} {'callers':>7s} {'proto':>5s} {'export':>6s}  "
          f"{'address':10s} {'unit':10s} name")
    for f in shown:
        insns = "?" if f["size"] is None else str(f["size"] // 4)
        print(f"{insns:>6s} {f['callers']:7d} {'yes' if f['declared'] else '':>5s} "
              f"{f['export']:>6s}  0x{f['addr']:08X} {f['unit']:10s} {f['name']}")
    print(f"\n{len(shown)} of {len(remaining)} matching function(s); "
          f"{sum(1 for f in functions if not f['done'])} remain overall.")
    return 0


if __name__ == "__main__":
    # The interesting invocations end in `| head`, which closes the pipe early.
    import signal

    signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    sys.exit(main())
