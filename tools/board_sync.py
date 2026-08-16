#!/usr/bin/env python3
"""Sync a GitHub Projects board from the objdiff progress report.

The board is generated, never hand-maintained: every number comes from
build/report.json, which `make report` produces from an actual build. Re-running
this script after a build brings the board back in sync -- it creates what is
missing and updates what changed, and does nothing else.

One issue is tracked per C translation unit (the real unit of decomp work).
Data/bss units are pure asm stubs that are matched by construction, so they are
summarised in the project README rather than given issues.

Usage:
    python3 tools/board_sync.py [--dry-run] [--repo OWNER/NAME] [--report PATH]

Requires the `gh` CLI, authenticated with the `repo` and `project` scopes.
Runs on the host (not inside the build container, which has no gh/credentials).
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from typing import Any

DEFAULT_REPO = "dudhasch/ff7-decomp"
DEFAULT_REPORT = "build/report.json"
PROJECT_TITLE = "FF7 Decompilation (PS1)"

MARKER = "<!-- ff7-board:unit={unit} -->"
MARKER_RE = re.compile(r"<!-- ff7-board:unit=(?P<unit>[^ ]+) -->")

# Human-readable names for the overlays, so the board reads as something other
# than a list of segment ids.
OVERLAY_TITLES = {
    "main": "Main executable (SCUS_941.63)",
    "batini": "Battle init",
    "battle": "Battle engine",
    "brom": "Battle ROM loader",
    "dschange": "Disc change",
    "ending": "Ending sequence",
    "field": "Field",
    "bginmenu": "Background menu",
    "cnfgmenu": "Config menu",
    "savemenu": "Save menu",
    "itemmenu": "Item menu",
    "world": "World map",
    "barrier": "Barrier magic",
}

# Custom project fields, on top of the built-in Status field.
NUMBER_FIELDS = ["Match %", "Functions Left", "Functions Done", "Code Size"]

MAX_LISTED_FUNCS = 20


# --------------------------------------------------------------------------
# shell / gh helpers
# --------------------------------------------------------------------------

def run(args: list[str], check: bool = True) -> str:
    proc = subprocess.run(args, capture_output=True, text=True)
    if check and proc.returncode != 0:
        raise RuntimeError(
            f"command failed ({proc.returncode}): {' '.join(args)}\n{proc.stderr.strip()}"
        )
    return proc.stdout.strip()


def gh_json(args: list[str]) -> Any:
    out = run(["gh"] + args)
    return json.loads(out) if out else None


def gql(query: str, **variables: Any) -> Any:
    args = ["gh", "api", "graphql", "-f", f"query={query}"]
    for key, value in variables.items():
        # -F does type inference (ints stay ints); -f forces string.
        args += ["-F" if isinstance(value, (int, float)) else "-f", f"{key}={value}"]
    return json.loads(run(args))


# --------------------------------------------------------------------------
# report parsing
# --------------------------------------------------------------------------

def unit_is_source(unit: dict) -> bool:
    """True for C translation units, false for generated data/bss asm stubs."""
    return unit.get("metadata", {}).get("source_path", "").endswith(".c")


def summarise(report: dict) -> tuple[list[dict], dict]:
    """Return (source units enriched with derived stats, overall measures)."""
    units = []
    for unit in report.get("units", []):
        if not unit_is_source(unit):
            continue
        m = unit.get("measures", {})
        total_fn = int(m.get("total_functions", 0) or 0)
        done_fn = int(m.get("matched_functions", 0) or 0)
        overlay = unit["name"].split("/", 1)[0]
        unmatched = [
            f for f in unit.get("functions", [])
            if float(f.get("fuzzy_match_percent", 0) or 0) < 100.0
        ]
        unmatched.sort(key=lambda f: int(f.get("size", 0) or 0))
        units.append({
            "unit": unit["name"],
            "overlay": overlay,
            "source_path": unit.get("metadata", {}).get("source_path", ""),
            "match_pct": round(float(m.get("matched_code_percent", 0) or 0), 2),
            "fn_total": total_fn,
            "fn_done": done_fn,
            "fn_left": total_fn - done_fn,
            "code_size": int(m.get("total_code", 0) or 0),
            "unmatched": unmatched,
        })
    units.sort(key=lambda u: (-u["fn_left"], u["unit"]))
    return units, report.get("measures", {})


def status_for(unit: dict) -> str:
    if unit["fn_left"] == 0:
        return "Done"
    if unit["fn_done"] > 0:
        return "In Progress"
    return "Todo"


# --------------------------------------------------------------------------
# issue body
# --------------------------------------------------------------------------

def bar(pct: float, width: int = 24) -> str:
    filled = int(round(pct / 100 * width))
    return "█" * filled + "░" * (width - filled)


def issue_title(unit: dict) -> str:
    name = unit["source_path"].rsplit("/", 1)[-1] or unit["unit"]
    return f"[{unit['overlay']}] {name}"


def issue_body(unit: dict, repo: str) -> str:
    src = unit["source_path"]
    lines = [
        MARKER.format(unit=unit["unit"]),
        "",
        f"Decompile the remaining functions in [`{src}`](../blob/main/{src}).",
        "",
        f"**Overlay:** `{unit['overlay']}` — {OVERLAY_TITLES.get(unit['overlay'], unit['overlay'])}  ",
        f"**objdiff unit:** `{unit['unit']}`",
        "",
        "## Progress",
        "",
        f"`{bar(unit['match_pct'])}` **{unit['match_pct']:.2f}%** of code matched",
        "",
        "| Metric | Value |",
        "| --- | --- |",
        f"| Functions matched | {unit['fn_done']} / {unit['fn_total']} |",
        f"| Functions remaining | **{unit['fn_left']}** |",
        f"| Code size | {unit['code_size']:,} bytes |",
        "",
    ]

    if unit["unmatched"]:
        shown = unit["unmatched"][:MAX_LISTED_FUNCS]
        lines += [
            f"## Remaining functions ({len(unit['unmatched'])})",
            "",
            "Smallest first — usually the easiest to match.",
            "",
            "| Function | Size |",
            "| --- | ---: |",
        ]
        for f in shown:
            lines.append(f"| `{f['name']}` | {int(f.get('size', 0) or 0):,} b |")
        if len(unit["unmatched"]) > len(shown):
            lines.append(f"| _… and {len(unit['unmatched']) - len(shown)} more_ | |")
        lines.append("")
    else:
        lines += ["## Remaining functions", "", "None — this unit is fully matched. 🎉", ""]

    lines += [
        "## Working on it",
        "",
        "```shell",
        f"./tools/podman-build.sh './mako.sh rank {src}'   # rank what is left, easiest first",
        "./tools/podman-build.sh './mako.sh dec <function_name>'",
        "./tools/podman-build.sh 'make build'                 # must end with every overlay OK",
        "```",
        "",
        "<sub>Generated by `tools/board_sync.py` from `build/report.json`. "
        "Edits to this body are overwritten on the next sync.</sub>",
    ]
    return "\n".join(lines)


# --------------------------------------------------------------------------
# labels & issues
# --------------------------------------------------------------------------

def ensure_labels(repo: str, overlays: set[str], dry: bool) -> None:
    existing = {l["name"] for l in gh_json(
        ["label", "list", "--repo", repo, "--limit", "200", "--json", "name"]) or []}
    wanted = {"decomp": ("0e8a16", "Decompilation work")}
    for ov in sorted(overlays):
        wanted[f"overlay:{ov}"] = ("1d76db", OVERLAY_TITLES.get(ov, ov))
    for name, (color, desc) in wanted.items():
        if name in existing:
            continue
        print(f"  + label {name}")
        if not dry:
            run(["gh", "label", "create", name, "--repo", repo,
                 "--color", color, "--description", desc], check=False)


def existing_issues(repo: str) -> dict[str, dict]:
    """Map objdiff unit name -> issue, discovered via the embedded marker."""
    issues = gh_json(["issue", "list", "--repo", repo, "--state", "all",
                      "--limit", "300", "--json", "number,title,body,url,state"]) or []
    found = {}
    for issue in issues:
        m = MARKER_RE.search(issue.get("body") or "")
        if m:
            found[m.group("unit")] = issue
    return found


def sync_issues(repo: str, units: list[dict], dry: bool) -> dict[str, dict]:
    have = existing_issues(repo)
    result = {}
    for unit in units:
        body = issue_body(unit, repo)
        title = issue_title(unit)
        labels = ["decomp", f"overlay:{unit['overlay']}"]
        issue = have.get(unit["unit"])
        if issue is None:
            print(f"  + issue {title}  ({unit['fn_left']} left)")
            if dry:
                result[unit["unit"]] = {"url": f"(dry-run) {title}"}
                continue
            url = run(["gh", "issue", "create", "--repo", repo, "--title", title,
                       "--body", body, "--label", ",".join(labels)]).splitlines()[-1]
            result[unit["unit"]] = {"url": url}
            if unit["fn_left"] == 0:
                run(["gh", "issue", "close", url.rsplit("/", 1)[-1], "--repo", repo,
                     "--reason", "completed"], check=False)
        else:
            if issue["body"].strip() != body.strip() or issue["title"] != title:
                print(f"  ~ issue #{issue['number']} {title}  ({unit['fn_left']} left)")
                if not dry:
                    run(["gh", "issue", "edit", str(issue["number"]), "--repo", repo,
                         "--title", title, "--body", body])
            else:
                print(f"  = issue #{issue['number']} {title}")
            result[unit["unit"]] = {"url": issue["url"], "number": issue["number"]}
            # Close/reopen to reflect completion.
            want_closed = unit["fn_left"] == 0
            is_closed = issue["state"].upper() == "CLOSED"
            if want_closed and not is_closed and not dry:
                run(["gh", "issue", "close", str(issue["number"]), "--repo", repo,
                     "--reason", "completed"], check=False)
            elif not want_closed and is_closed and not dry:
                run(["gh", "issue", "reopen", str(issue["number"]), "--repo", repo], check=False)
    return result


# --------------------------------------------------------------------------
# project board
# --------------------------------------------------------------------------

def find_or_create_project(owner: str, dry: bool) -> dict:
    projects = gh_json(["project", "list", "--owner", owner, "--format", "json",
                        "--limit", "100"])["projects"]
    for p in projects:
        if p["title"] == PROJECT_TITLE:
            print(f"  = project #{p['number']} {PROJECT_TITLE}")
            return p
    print(f"  + project {PROJECT_TITLE}")
    if dry:
        return {"number": 0, "id": "(dry-run)", "url": "(dry-run)"}
    p = gh_json(["project", "create", "--owner", owner, "--title", PROJECT_TITLE,
                 "--format", "json"])
    return p


def project_fields(owner: str, number: int) -> dict[str, dict]:
    data = gh_json(["project", "field-list", str(number), "--owner", owner,
                    "--format", "json", "--limit", "100"])
    return {f["name"]: f for f in data["fields"]}


def ensure_fields(owner: str, number: int, overlays: list[str], dry: bool) -> dict[str, dict]:
    fields = project_fields(owner, number) if not dry else {}
    for name in NUMBER_FIELDS:
        if name in fields:
            continue
        print(f"  + field {name} (number)")
        if not dry:
            run(["gh", "project", "field-create", str(number), "--owner", owner,
                 "--name", name, "--data-type", "NUMBER"])
    if "Overlay" not in fields:
        print("  + field Overlay (single select)")
        if not dry:
            run(["gh", "project", "field-create", str(number), "--owner", owner,
                 "--name", "Overlay", "--data-type", "SINGLE_SELECT",
                 "--single-select-options", ",".join(overlays)])
    return project_fields(owner, number) if not dry else {}


ITEMS_QUERY = """
query($id: ID!, $cursor: String) {
  node(id: $id) {
    ... on ProjectV2 {
      items(first: 100, after: $cursor) {
        pageInfo { hasNextPage endCursor }
        nodes {
          id
          content { ... on Issue { number url } }
        }
      }
    }
  }
}
"""


def project_items(project_id: str) -> dict[int, str]:
    """Map issue number -> project item id."""
    out: dict[int, str] = {}
    cursor = None
    while True:
        args = ["gh", "api", "graphql", "-f", f"query={ITEMS_QUERY}", "-f", f"id={project_id}"]
        if cursor:
            args += ["-f", f"cursor={cursor}"]
        data = json.loads(run(args))["data"]["node"]["items"]
        for node in data["nodes"]:
            content = node.get("content") or {}
            if content.get("number"):
                out[content["number"]] = node["id"]
        if not data["pageInfo"]["hasNextPage"]:
            break
        cursor = data["pageInfo"]["endCursor"]
    return out


def set_number(project_id: str, item_id: str, field_id: str, value: float) -> None:
    run(["gh", "project", "item-edit", "--id", item_id, "--project-id", project_id,
         "--field-id", field_id, "--number", str(value)])


def set_single_select(project_id: str, item_id: str, field_id: str, option_id: str) -> None:
    run(["gh", "project", "item-edit", "--id", item_id, "--project-id", project_id,
         "--field-id", field_id, "--single-select-option-id", option_id])


def sync_board(owner: str, units: list[dict], issues: dict[str, dict],
               overlays: list[str], dry: bool) -> dict:
    project = find_or_create_project(owner, dry)
    number, pid = project["number"], project["id"]
    fields = ensure_fields(owner, number, overlays, dry)
    if dry:
        return project

    items = project_items(pid)
    opt_id = {}
    if "Overlay" in fields:
        opt_id = {o["name"]: o["id"] for o in fields["Overlay"].get("options", [])}
    status_opt = {}
    if "Status" in fields:
        status_opt = {o["name"]: o["id"] for o in fields["Status"].get("options", [])}

    for unit in units:
        issue = issues.get(unit["unit"])
        if not issue:
            continue
        num = issue.get("number")
        if num is None:
            num = int(issue["url"].rsplit("/", 1)[-1])
        item_id = items.get(num)
        if item_id is None:
            res = gh_json(["project", "item-add", str(number), "--owner", owner,
                           "--url", issue["url"], "--format", "json"])
            item_id = res["id"]
            print(f"  + board item for #{num}")
        for name, value in (
            ("Match %", unit["match_pct"]),
            ("Functions Left", unit["fn_left"]),
            ("Functions Done", unit["fn_done"]),
            ("Code Size", unit["code_size"]),
        ):
            if name in fields:
                set_number(pid, item_id, fields[name]["id"], value)
        if opt_id.get(unit["overlay"]):
            set_single_select(pid, item_id, fields["Overlay"]["id"], opt_id[unit["overlay"]])
        want = status_for(unit)
        if status_opt.get(want):
            set_single_select(pid, item_id, fields["Status"]["id"], status_opt[want])
    return project


# --------------------------------------------------------------------------
# project readme
# --------------------------------------------------------------------------

def build_readme(units: list[dict], overall: dict, report: dict) -> str:
    cats = {c["id"]: c["measures"] for c in report.get("categories", [])}
    data_units = [u for u in report.get("units", []) if not unit_is_source(u)]
    lines = [
        "## Final Fantasy VII (PS1, USA) — decompilation status",
        "",
        f"**{float(overall.get('matched_code_percent', 0)):.2f}% of code matched** · "
        f"{int(overall.get('matched_functions', 0)):,} / {int(overall.get('total_functions', 0)):,} "
        f"functions · {int(overall.get('matched_code', 0)):,} / {int(overall.get('total_code', 0)):,} bytes",
        "",
        "### By overlay",
        "",
        "| Overlay | Matched | Functions | Progress |",
        "| --- | ---: | ---: | --- |",
    ]
    for ov in sorted(cats, key=lambda o: -float(cats[o].get("matched_code_percent", 0) or 0)):
        m = cats[ov]
        pct = float(m.get("matched_code_percent", 0) or 0)
        lines.append(
            f"| **{ov}** — {OVERLAY_TITLES.get(ov, ov)} | {pct:.2f}% | "
            f"{int(m.get('matched_functions', 0))}/{int(m.get('total_functions', 0))} | `{bar(pct, 16)}` |"
        )
    lines += [
        "",
        f"Tracked here: **{len(units)} C translation units**. "
        f"The other {len(data_units)} objdiff units are generated data/bss asm stubs, "
        "matched by construction and not tracked as issues.",
        "",
        "Generated by `tools/board_sync.py` from `build/report.json`.",
    ]
    return "\n".join(lines)


def set_readme(owner: str, number: int, readme: str, dry: bool) -> None:
    print("  ~ project README")
    if not dry:
        run(["gh", "project", "edit", str(number), "--owner", owner, "--readme", readme])


# --------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--repo", default=DEFAULT_REPO)
    ap.add_argument("--report", default=DEFAULT_REPORT)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    if not shutil.which("gh"):
        print("error: gh CLI not found on PATH", file=sys.stderr)
        return 1
    try:
        report = json.load(open(args.report))
    except FileNotFoundError:
        print(f"error: {args.report} not found — run `make report` first", file=sys.stderr)
        return 1

    owner = args.repo.split("/", 1)[0]
    units, overall = summarise(report)
    overlays = sorted({u["overlay"] for u in units})

    print(f"report: {float(overall.get('matched_code_percent', 0)):.2f}% code, "
          f"{overall.get('matched_functions')}/{overall.get('total_functions')} functions, "
          f"{len(units)} source units")

    print("labels:")
    ensure_labels(args.repo, set(overlays), args.dry_run)
    print("issues:")
    issues = sync_issues(args.repo, units, args.dry_run)
    print("board:")
    project = sync_board(owner, units, issues, overlays, args.dry_run)
    set_readme(owner, project["number"], build_readme(units, overall, report), args.dry_run)

    print(f"\ndone — {project.get('url')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
