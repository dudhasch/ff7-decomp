# Status board

**Board:** <https://github.com/users/dudhasch/projects/5> — *FF7 Decompilation (PS1)*

The board is **generated, not hand-maintained**. Every number on it comes from
`build/report.json`, which `make report` produces from an actual build via
objdiff. Nothing is typed in by hand, so the board cannot drift from reality —
if it looks wrong, re-run the build and sync.

## Keeping it current

```shell
./tools/podman-build.sh 'make report'   # build + measure  (in the container)
make board                              # sync to GitHub   (on the host, needs gh)
```

`make board` is idempotent: it creates what is missing, updates what changed,
and leaves everything else alone. Running it twice in a row is a no-op.

## What is tracked

One issue per **C translation unit** — the real unit of decomp work. There are
21 of them. The other 19 objdiff units are generated data/bss asm stubs that are
matched by construction; they are summarised in the project README instead of
being given issues that would always sit at 100%.

Each issue carries:

* a progress bar and the matched-code percentage,
* functions matched / remaining,
* the remaining functions listed **smallest first** — usually the easiest to
  match, mirroring how `./mako.sh rank` orders work,
* the exact commands to work on that unit.

Labels are `decomp` plus `overlay:<name>`, so the issue list can be filtered per
overlay without the board.

## Board fields

| Field | Meaning |
| --- | --- |
| **Status** | `Todo` (nothing matched) / `In Progress` / `Done` (100%) |
| **Overlay** | Which of the 13 overlays the unit belongs to |
| **Match %** | Percentage of code bytes matching the original |
| **Functions Left** | Functions still to decompile — the work-remaining metric |
| **Functions Done** | Functions already matching |
| **Code Size** | Size of the unit's `.text` in bytes |

Issues reaching 100% are closed automatically; if a regression drops one below
100%, the next sync reopens it.

### Suggested views

The default view lists everything. These are worth adding in the GitHub UI:

* **By overlay** — group by `Overlay`, sort by `Functions Left` descending.
* **Quick wins** — filter `Functions Left` ≤ 5, sort ascending. Good first tasks.
* **Board** — group by `Status` for the classic kanban layout.

## Retargeting

The script defaults to `dudhasch/ff7-decomp`. For a different fork:

```shell
python3 tools/board_sync.py --repo OWNER/NAME
python3 tools/board_sync.py --dry-run          # show what would change
```

The project title (`FF7 Decompilation (PS1)`) is matched by name, so the script
finds an existing board rather than creating duplicates. Issues are matched by a
hidden `<!-- ff7-board:unit=... -->` marker in the body, not by title — renaming
an issue will not cause a duplicate, but deleting that marker will.

Note that GitHub disables issues on forks by default. Enable them once with:

```shell
gh api -X PATCH repos/OWNER/NAME -F has_issues=true
```
