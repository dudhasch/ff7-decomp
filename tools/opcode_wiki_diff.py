#!/usr/bin/env python3
"""Compare this repo's derived opcode table against the community wiki's.

    .venv/bin/python3 tools/opcode_wiki_diff.py --wiki <checkout>/docs/FF7/Field/Script

`tools/opcode_docs.py` derives opcode number, mnemonic, length and argument
layout from the retail code -- the `g_FieldOpcodes` dispatch table, the string
each handler hands `DebugPrintOpcode`, and the `GET_PARAM_*` /
`FieldEventReadMemory*` calls in its body. ff7-mods/ff7-flat-wiki documents the
same instruction set from the PC version, by hand. Where the two disagree, one
of them is wrong, and this prints the disagreements so they can be checked
against the assembly one at a time.

The wiki states no length directly, so it is summed from the `#### Memory
layout` row: one byte for the opcode, then each cell costs the size of the
argument named in it, with a cell holding only `Bit[n]` selectors costing one
byte because they are nibbles packed together. That is the same arithmetic a
reader does by eye, made explicit.

Both a missing page and a mismatch are worth knowing, so nothing is filtered:
a name that differs may be this repo mis-transcribing the debugger string, and
a length that differs may be a PC-versus-PS1 difference rather than an error.
"""
import argparse
import io
import os
import re
import sys

TITLE = re.compile(r"^([0-9A-Fa-f]{2})_(.+)\.md$")
LAYOUT = re.compile(r"^#### Memory layout\s*\n\s*\n\|(.+?)\|\s*\n", re.M)
ARG = re.compile(r"^- \*\*const ([^*]+)\*\* \*([^*]+)\*", re.M)
OURS = re.compile(r"^\| 0x([0-9A-F]{2}) \| \[([^\]]+)\]\([^)]*\) \| ([0-9?]+) \|"
                  r" [0-9?]+ \| (\S+) \| `([^`]+)` \|$", re.M)

SIZE = {"UByte": 1, "Byte": 1, "Bute": 1, "UByte\\[3\\]": 3,
        "Short": 2, "UShort": 2, "UShort (Bit field)": 2,
        "SWord": 4, "UWord": 4, "ULong": 4}


def arg_size(ty):
    if ty.startswith("Bit"):
        return 0            # a nibble; its byte is counted by the cell
    return SIZE.get(ty)


def wiki_pages(d):
    """{opcode: (short, length or None, note)} from the wiki markdown."""
    out = {}
    for fn in sorted(os.listdir(d)):
        m = TITLE.match(fn)
        if not m:
            continue
        code, short = int(m.group(1), 16), m.group(2)
        text = io.open(os.path.join(d, fn), encoding="utf-8",
                       errors="replace").read()
        types = {n.strip(): t.strip() for t, n in ARG.findall(text)}
        lay = LAYOUT.search(text)
        if not lay:
            out[code] = (short, None, "no memory-layout table")
            continue
        cells = [c.strip() for c in lay.group(1).split("|")]
        total, note = 1, ""
        # The leading cell repeats the opcode byte; several pages carry a
        # neighbour's row verbatim, which is a self-contained contradiction
        # with the page's own title and needs no PS1 evidence to call.
        lead = re.match(r"0x([0-9A-Fa-f]{2})$", cells[0])
        if lead and int(lead.group(1), 16) != code:
            note = "layout row says 0x%s" % lead.group(1).upper()
        for cell in cells[1:]:
            names = [n.strip() for n in cell.strip("*").split(" / ") if n.strip()]
            sizes = [arg_size(types.get(n, "")) for n in names]
            if not names or any(s is None for s in sizes):
                note = note or "unparsed cell %r" % cell
                total = None
                break
            total += max(1, sum(sizes))
        out[code] = (short, total, note)
    return out


def our_pages(index_md):
    out = {}
    text = io.open(index_md, encoding="utf-8", errors="replace").read()
    for code, short, length, st, handler in OURS.findall(text):
        out[int(code, 16)] = (short, None if length == "?" else int(length),
                              "%s [%s]" % (handler, st))
    return out


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("--wiki", required=True,
                    help="path to the wiki's docs/FF7/Field/Script directory")
    ap.add_argument("--ours", default="docs/decomp/opcodes/Opcodes.md")
    ap.add_argument("--markdown", help="also write the report here")
    ap.add_argument("--wiki-rev", default="", help="wiki commit, for the report")
    a = ap.parse_args(argv)

    wiki = wiki_pages(os.path.join(a.wiki, "Opcodes"))
    ours = our_pages(a.ours)
    print("wiki pages: %d    our table slots: %d" % (len(wiki), len(ours)))

    only_ours, only_wiki, name_diff, len_diff, unparsed, agree = \
        [], [], [], [], [], 0
    for code in sorted(set(wiki) | set(ours)):
        w, o = wiki.get(code), ours.get(code)
        if o and not w:
            only_ours.append((code, o))
            continue
        if w and not o:
            only_wiki.append((code, w))
            continue
        if w[1] is None:
            unparsed.append((code, w))
        if o[0].upper() != w[0].upper():
            name_diff.append((code, o[0], w[0], o[2]))
        elif w[1] is not None and o[1] is not None and o[1] != w[1]:
            len_diff.append((code, o[0], o[1], w[1], o[2]))
        elif w[1] is not None and o[1] is not None:
            agree += 1

    def show(title, rows, fmt):
        print("\n== %s: %d" % (title, len(rows)))
        for r in rows:
            print("   " + fmt(r))

    print("\nname and length agree on %d opcodes" % agree)
    show("in our dispatch table, no wiki page", only_ours,
         lambda r: "0x%02X  %-10s len=%-4s %s"
                   % (r[0], r[1][0], r[1][1], r[1][2]))
    show("wiki page, not reached by our table", only_wiki,
         lambda r: "0x%02X  %-10s len=%s" % (r[0], r[1][0], r[1][1]))
    show("mnemonic differs", name_diff,
         lambda r: "0x%02X  ours=%-10s wiki=%-10s  %s" % r)
    show("length differs", len_diff,
         lambda r: "0x%02X  %-10s ours=%-3d wiki=%-3d  %s" % r)
    show("wiki length not parseable", unparsed,
         lambda r: "0x%02X  %-10s %s" % (r[0], r[1][0], r[1][2]))

    stale = [(c, w) for c, w in sorted(wiki.items())
             if w[2].startswith("layout row says")]
    show("wiki layout row contradicts its own opcode", stale,
         lambda r: "0x%02X  %-10s %s" % (r[0], r[1][0], r[1][2]))

    if a.markdown:
        write_report(a.markdown, a.wiki_rev, wiki, ours, agree, only_ours,
                     name_diff, len_diff, stale)
        print("\nreport -> %s" % a.markdown)
    return 0


def write_report(path, rev, wiki, ours, agree, only_ours, name_diff, len_diff,
                 stale):
    L = ["# Field opcodes: this decomp against ff7-flat-wiki", ""]
    L += ["Generated by `tools/opcode_wiki_diff.py`. Our side is derived by",
          "`tools/opcode_docs.py` from the PS1 (USA) field overlay; the wiki",
          "documents the PC version by hand%s." % (rev and " (%s)" % rev), "",
          "A row marked `verified` comes from a handler compiled from C in a",
          "green build, so it is byte-identical to the retail overlay and the",
          "derived fact is a fact about the shipped game. `parked` and `asm`",
          "rows are not evidence.", "",
          "| | count |", "|---|---|",
          "| wiki pages | %d |" % len(wiki),
          "| our dispatch-table slots | %d |" % len(ours),
          "| name and length agree | %d |" % agree, ""]

    L += ["## 1. Wiki layout row contradicts the page's own opcode", "",
          "Self-contained: the `#### Memory layout` row repeats a *different*",
          "opcode byte than the page title and `- Opcode:` line. Needs no PS1",
          "evidence to confirm.", "",
          "| page | title says | layout row says |", "|---|---|---|"]
    for c, w in stale:
        L.append("| %02X_%s | 0x%02X | %s |"
                 % (c, w[0], c, w[2].replace("layout row says ", "")))
    L.append("")

    L += ["## 2. Length disagreements on verified handlers", "",
          "Length here is `PC_INC`, the amount the PS1 interpreter advances",
          "the script pointer. The wiki's is summed from its layout row.", "",
          "| opcode | name | PS1 | wiki | handler |", "|---|---|---|---|---|"]
    for c, short, o, w, h in len_diff:
        L.append("| 0x%02X | %s | %d | %d | `%s` |" % (c, short, o, w, h))
    L.append("")

    L += ["## 3. Opcodes with no wiki page", "",
          "Excluding the invalid-opcode stub `OpcodeFuncBad`.", "",
          "| opcode | name | length | status | handler |", "|---|---|---|---|---|"]
    for c, o in only_ours:
        if o[2].startswith("OpcodeFuncBad"):
            continue
        h, st = o[2].rsplit(" [", 1)
        L.append("| 0x%02X | %s | %s | %s | `%s` |"
                 % (c, o[0], o[1] if o[1] is not None else "?",
                    st.rstrip("]"), h))
    L.append("")

    L += ["## 4. Mnemonic differences", "",
          "Ours is the string the handler passes to `DebugPrintOpcode`, i.e.",
          "what the game's own debugger prints. Most rows are a naming",
          "convention rather than an error -- the wiki prefers expanded names",
          "(SPECIAL for SPCAL, MESSAGE for MES). The ones worth checking are",
          "where the two differ by a transposition.", "",
          "| opcode | ours | wiki | handler |", "|---|---|---|---|"]
    for c, o, w, h in name_diff:
        L.append("| 0x%02X | %s | %s | `%s` |" % (c, o, w, h))
    L.append("")
    io.open(path, "w", encoding="utf-8", newline="\n").write("\n".join(L))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
