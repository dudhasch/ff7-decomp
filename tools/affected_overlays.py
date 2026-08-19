#!/usr/bin/env python3
"""Map a list of changed files onto the overlays that need rebuilding.

CI uses this to scope a pull request: only the overlays that own a changed
source file get split, compiled, linked and sha1-checked. Everything else is
skipped, which is the bulk of a full `make build`.

The source-file -> overlay mapping is derived from config/us.yaml rather than
hardcoded, because it is not a directory mapping: src/menu/ alone feeds four
different overlays (bginmenu, cnfgmenu, savemenu, itemmenu), each named by a
`c` subsegment of the overlay that owns it.

The verdict is one of three modes:

  full     rebuild everything -- the change can move bytes anywhere
  partial  rebuild the named overlays only
  none     no build needed at all (documentation and other inert paths)

`full` is the default for anything unrecognised, so a new top-level directory
or a new source file gets the safe answer rather than a silently narrow one.

Usage:
    tools/affected_overlays.py <path> [<path> ...]
    tools/affected_overlays.py --from-file changed.txt
    git diff --name-only main... | tools/affected_overlays.py --github-output
"""

import argparse
import fnmatch
import os
import sys

import yaml

# A change to any of these can move bytes in any overlay -- shared headers, the
# symbol tables every overlay links against, the compiler driver itself -- so
# there is nothing to narrow down.
#
# fnmatch's `*` crosses `/`, so `config/*` already covers everything below it.
FULL_BUILD_GLOBS = [
    "config/*",
    "include/*",
    "tools/*",
    ".github/workflows/*",
    "Dockerfile",
    "Makefile",
    "mako.sh",
    "go.work",
    "go.work.sum",
    "requirements.txt",
]

# Paths that cannot affect a build at all. Checked before FULL_BUILD_GLOBS, so
# that e.g. a README under tools/ stays inert. Anything neither listed here nor
# resolvable to an overlay falls through to a full build.
NO_BUILD_GLOBS = [
    "*.md",
    "docs/*",
    ".gitignore",
    ".gitattributes",
    ".gitmodules",
    ".editorconfig",
    ".clang-format",
    "LICENSE",
    ".vscode/*",
    ".idea/*",
    ".claude/*",
]


def matches(path, globs):
    return any(fnmatch.fnmatch(path, g) for g in globs)


def load_config(config_path):
    with open(config_path) as f:
        return yaml.load(f, Loader=yaml.SafeLoader)


def source_owners(cfg):
    """source file path -> set of overlays compiling it.

    A `c` (or `.data`) subsegment names a translation unit; tools/ninja/gen.py
    resolves it to <src_path>/<base_path>/<name>.c, so this mirrors that.
    """
    src_root = cfg.get("src_path", "src")
    owners = {}
    for ovl in cfg["overlays"]:
        base = ovl.get("base_path", ovl["name"])
        for sub in ovl.get("segments", []):
            if not isinstance(sub, list) or len(sub) < 3:
                continue
            if str(sub[1]) not in ("c", ".data"):
                continue
            path = f"{src_root}/{base}/{sub[2]}.c"
            owners.setdefault(path, set()).add(ovl["name"])
    return owners


def dependents(cfg):
    """overlay -> overlays that link against its exported symbol list.

    battle writes config/sym_export_battle.us.txt, which batini and the magic
    overlays link against, so a battle change has to be checked against them
    too.
    """
    deps = {}
    for ovl in cfg["overlays"]:
        for sym_path in ovl.get("symbol_addrs_path") or []:
            name = os.path.basename(str(sym_path))
            if not name.startswith("sym_export_"):
                continue
            provider = name[len("sym_export_") :].split(".")[0]
            deps.setdefault(provider, set()).add(ovl["name"])
    return deps


def resolve(paths, cfg, log=lambda msg: None):
    """Return (mode, sorted overlay list)."""
    owners = source_owners(cfg)
    deps = dependents(cfg)
    known = {ovl["name"] for ovl in cfg["overlays"]}

    selected = set()
    for path in paths:
        path = path.strip().replace("\\", "/")
        if not path:
            continue
        if path in owners:
            log(f"{path}: {', '.join(sorted(owners[path]))}")
            selected |= owners[path]
        elif matches(path, NO_BUILD_GLOBS):
            log(f"{path}: not a build input")
        elif matches(path, FULL_BUILD_GLOBS):
            log(f"{path}: shared input, full build")
            return "full", sorted(known)
        else:
            log(f"{path}: unrecognised, full build")
            return "full", sorted(known)

    # main links against config/sym_ovl_export.us.txt, regenerated from the ELF
    # of every other overlay, so scoping it to a subset is not possible.
    if "main" in selected:
        log("main is affected, which pulls in every other overlay")
        return "full", sorted(known)

    for provider, users in deps.items():
        if provider in selected:
            log(f"{provider} exports symbols to {', '.join(sorted(users))}")
            selected |= users

    if not selected:
        return "none", []
    if selected == known:
        return "full", sorted(known)
    return "partial", sorted(selected)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("paths", nargs="*", help="changed file paths")
    ap.add_argument("--from-file", help="read changed paths from a file, one per line")
    ap.add_argument("--config", default="config/us.yaml", help="splat build config")
    ap.add_argument(
        "--github-output",
        action="store_true",
        help="print `mode=` and `overlays=` lines for $GITHUB_OUTPUT",
    )
    ap.add_argument("--quiet", action="store_true", help="do not explain on stderr")
    args = ap.parse_args()

    paths = list(args.paths)
    if args.from_file:
        with open(args.from_file) as f:
            paths += f.read().splitlines()
    if not paths and not sys.stdin.isatty():
        paths += sys.stdin.read().splitlines()

    cfg = load_config(args.config)
    log = (
        (lambda msg: None) if args.quiet else (lambda msg: print(msg, file=sys.stderr))
    )
    mode, overlays = resolve(paths, cfg, log)

    if args.github_output:
        print(f"mode={mode}")
        # `full` deliberately leaves this empty: an explicit list would have to
        # be kept in step with config/us.yaml by hand.
        print(f"overlays={','.join(overlays) if mode == 'partial' else ''}")
    else:
        print(",".join(overlays))
    return 0


if __name__ == "__main__":
    sys.exit(main())
