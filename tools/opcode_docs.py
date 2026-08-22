#!/usr/bin/env python3
"""Generate one markdown page per field-script opcode, from the source.

    .venv/bin/python3 tools/opcode_docs.py -o docs/decomp/opcodes

Everything here is *derived*, never hand-written, so the pages can be
regenerated after any decomp change and diffed. Four facts come out of the
retail code itself:

  opcode number   the index into `g_FieldOpcodes`, the 256-entry dispatch
                  table in the field overlay's data segment.
  short name      the string the handler passes to `DebugPrintOpcode`, which
                  is the mnemonic the game's own debugger prints.
  total length    `PC_INC(k)`, the amount the interpreter advances the script
                  pointer past this instruction. Where a handler has exactly
                  one such value that is the length; several (a handler that
                  bails out early down one arm) or none (a jump, which assigns
                  the PC instead) leaves it unknown and the values are listed.
                  Note `DebugPrintOpcode(name, n)`'s n is *not* the length --
                  it is how many argument bytes the in-game debugger prints,
                  and it saturates at 8, so SPLIT prints 8 and is 15 bytes
                  long. It is reported separately because it is still a useful
                  cross-check on short opcodes.
  arguments       `GET_PARAM_U8(k)` and `GET_PARAM_S16(v, k)` are literal
                  bytes and shorts at byte offset k; `FieldEventReadMemoryU8(b,
                  k)` and `...S16(b, k)` are bank-addressed, where b is 1..6
                  and selects a nibble of bytes 1..3 exactly as B1..B6 do in
                  the community wiki -- see FieldEventReadMemoryS16's own
                  switch, which is where that mapping is defined.

The page layout mirrors ff7-mods/ff7-flat-wiki's `docs/FF7/Field/Script`, so
the two can be compared file by file.

Each page carries the handler's status, because only some of it is evidence.
A handler compiled from C in a green build is byte-identical to the retail
overlay, so everything derived from it is a fact about the shipped game; a
handler still pinned to its assembly (MASPSX_OVERRIDE) has C beside it that
never compiles, so its page is a reading of *unverified* source. A handler
with no C body at all gets a page saying so, and the index stays a complete
0x00..0xFF map either way.
"""
import argparse
import glob
import io
import os
import re
import sys

TABLE = re.compile(r"^glabel g_FieldOpcodes\b", re.M)
WORD = re.compile(r"^\s*/\*[^*]*\*/\s*\.word\s+(\w+)\s*$")
FUNC = re.compile(r"^(?:s32|void|u8|s16|u16|u32)\s+(\w+)\s*\(\s*void\s*\)\s*\{",
                  re.M)
DEBUG = re.compile(r'DebugPrintOpcode\(\s*"([A-Za-z0-9_]+)"\s*,\s*(\d+)\s*\)')
PCINC = re.compile(r"PC_INC\(\s*(\d+)\s*\)")
G_U8 = re.compile(r"GET_PARAM_U8\(\s*(\d+)\s*\)")
G_S16 = re.compile(r"GET_PARAM_S16\(\s*\w+\s*,\s*(\d+)\s*\)")
BANK = re.compile(r"FieldEventReadMemory(U8|S16)\(\s*(\d+)\s*,\s*(\d+)\s*\)")
PARKED = re.compile(r'MASPSX_OVERRIDE\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', re.S)
INCASM = re.compile(r'INCLUDE_ASM\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', re.S)


def read_table(asm_dir):
    """opcode index -> handler name, from the .word list after the glabel."""
    pattern = os.path.join(asm_dir, "**", "*.data.s")
    for path in sorted(glob.glob(pattern, recursive=True)):
        text = io.open(path, encoding="utf-8", errors="replace").read()
        m = TABLE.search(text)
        if not m:
            continue
        out = []
        for line in text[m.end():].split("\n")[1:]:
            w = WORD.match(line)
            if not w:
                break
            out.append(w.group(1))
        if out:
            return out
    return []


def read_bodies(src_glob):
    """handler name -> body text, including bodies parked under #else."""
    bodies = {}
    for path in sorted(glob.glob(src_glob)):
        text = io.open(path, encoding="utf-8", errors="replace").read()
        for m in FUNC.finditer(text):
            i = text.index("{", m.start())
            depth, j = 0, i
            while j < len(text):
                if text[j] == "{":
                    depth += 1
                elif text[j] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                j += 1
            bodies.setdefault(m.group(1), text[i:j + 1])
    return bodies


def read_status(src_glob):
    """handler name -> parked | asm, for names that are not plain C."""
    st = {}
    for path in sorted(glob.glob(src_glob)):
        text = io.open(path, encoding="utf-8", errors="replace").read()
        for m in PARKED.finditer(text):
            st[m.group(1)] = "parked"
        for m in INCASM.finditer(text):
            st.setdefault(m.group(1), "asm")
    return st


def strip_comments(s):
    s = re.sub(r"/\*.*?\*/", " ", s, flags=re.S)
    return re.sub(r"//[^\n]*", " ", s)


def bank_byte(b):
    """B1..B6 -> (byte offset, high|low), matching FieldEventReadMemory."""
    return 1 + (b - 1) // 2, "high" if b % 2 else "low"


def analyse(name, body):
    """Everything derivable about one handler."""
    b = strip_comments(body)
    d = DEBUG.search(b)
    info = {
        "handler": name,
        "short": d.group(1).upper() if d else None,
        "argbytes": int(d.group(2)) if d else None,
        "pcinc": sorted({int(x) for x in PCINC.findall(b)}),
        "args": {},
    }
    # One PC_INC is the instruction length; several or none is not a length.
    info["length"] = info["pcinc"][0] if len(info["pcinc"]) == 1 else None
    for off in sorted({int(x) for x in G_U8.findall(b)}):
        info["args"].setdefault(off, {"size": 1, "banks": []})
    for off in sorted({int(x) for x in G_S16.findall(b)}):
        info["args"][off] = {"size": 2, "banks": []}
    for kind, bk, off in BANK.findall(b):
        off, bk = int(off), int(bk)
        a = info["args"].setdefault(off, {"size": 0, "banks": []})
        a["size"] = max(a["size"], 2 if kind == "S16" else 1)
        if bk not in a["banks"]:
            a["banks"].append(bk)
    for a in info["args"].values():
        a["banks"].sort()
    return info


def bank_cells(info):
    """The B-nibble bytes this opcode uses, as {byte offset: [b, ...]}."""
    cells = {}
    for a in info["args"].values():
        for bk in a["banks"]:
            off = bank_byte(bk)[0]
            cells.setdefault(off, [])
            if bk not in cells[off]:
                cells[off].append(bk)
    for v in cells.values():
        v.sort()
    return cells


def layout_row(code, info):
    """The wiki-style `| 0xNN | *B1 / B2* | *argN* |` memory-layout row."""
    banks = bank_cells(info)
    cells = ["0x%02X" % code]
    covered = set()
    for off in sorted(set(list(banks) + list(info["args"]))):
        if off in covered:
            continue
        if off in banks:
            cells.append("*%s*" % " / ".join("B%d" % b for b in banks[off]))
            covered.add(off)
            continue
        a = info["args"][off]
        cells.append("*arg%X*" % off)
        covered.update(range(off, off + max(1, a["size"])))
    total = info["length"]
    if total is not None:
        for _ in range(max(0, total - 1 - len(covered))):
            cells.append("*?*")
    return cells


def render(code, info, unknown_reason=None):
    short = info["short"] if info and info["short"] else "UNKNOWN"
    lines = ["---", "title: %02X_%s" % (code, short), "---", ""]
    lines.append("- Opcode: **0x%02X**" % code)
    lines.append("- Short name: **%s**" % short)
    lines.append("- Handler: `%s`" % info["handler"])
    lines.append("- Status: %s" % {
        "parked": "**unverified** -- pinned to its assembly, the C beside it "
                  "is never compiled",
        "asm": "**not decompiled** -- still INCLUDE_ASM",
    }.get(info.get("status"), "verified -- compiled from C in a green build"))
    if info["length"] is not None:
        lines.append("- Length: **%d** bytes (`PC_INC(%d)`)"
                     % (info["length"], info["length"]))
    elif info["pcinc"]:
        lines.append("- Length: not a single value; `PC_INC` is %s"
                     % ", ".join(str(x) for x in info["pcinc"]))
    else:
        lines.append("- Length: unknown -- the handler has no `PC_INC`")
    if info["argbytes"] is not None:
        lines.append("- The in-game debugger prints %d argument byte(s); this "
                     "saturates at 8 and is not the instruction length."
                     % info["argbytes"])
    lines.append("")
    if unknown_reason:
        lines += ["#### Memory layout", "", unknown_reason, ""]
        return "\n".join(lines) + "\n"

    row = layout_row(code, info)
    lines += ["#### Memory layout", "",
              "| " + " | ".join(row) + " |",
              "|" + "|".join("-" * (len(c) + 2) for c in row) + "|", ""]
    lines += ["#### Arguments", ""]
    banks = bank_cells(info)
    if not info["args"]:
        lines += ["None.", ""]
    else:
        for off in sorted(banks):
            for b in banks[off]:
                bo, half = bank_byte(b)
                lines.append(
                    "- **const Bit\\[4\\]** *B%d*: bank selector, %s nibble "
                    "of byte 0x%02X; zero means the value is a literal."
                    % (b, half, bo))
        for off in sorted(info["args"]):
            a = info["args"][off]
            ty = "Short" if a["size"] == 2 else "Byte"
            if a["banks"]:
                lines.append(
                    "- **const %s** *arg%X*: at byte 0x%02X, addressed "
                    "through %s." % (ty, off, off,
                                     " / ".join("*B%d*" % b
                                                for b in a["banks"])))
            else:
                lines.append("- **const %s** *arg%X*: literal at byte 0x%02X."
                             % (ty, off, off))
        lines.append("")
    if len(info["pcinc"]) > 1:
        lines += ["#### Notes", "",
                  "- The handler advances the script pointer by different "
                  "amounts on different paths: %s."
                  % ", ".join(str(x) for x in info["pcinc"]), ""]
    return "\n".join(lines) + "\n"


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("-o", "--out", default="docs/decomp/opcodes")
    ap.add_argument("--asm", default="asm/us/field/data")
    ap.add_argument("--src", default="src/field/*.c")
    a = ap.parse_args(argv)

    table = read_table(a.asm)
    if not table:
        print("no g_FieldOpcodes table found under %s" % a.asm, file=sys.stderr)
        return 1
    bodies = read_bodies(a.src)
    status = read_status(a.src)
    outdir = os.path.join(a.out, "Opcodes")
    if not os.path.isdir(outdir):
        os.makedirs(outdir)

    index = []
    for code, handler in enumerate(table):
        body = bodies.get(handler)
        reason = None
        if body is None:
            info = {"handler": handler, "short": None, "argbytes": None,
                    "pcinc": [], "length": None, "args": {}, "status": "asm"}
            reason = ("Not known: `%s` has no C body yet (still INCLUDE_ASM)."
                      % handler)
        else:
            info = analyse(handler, body)
            info["status"] = status.get(handler)
            if info["short"] is None:
                reason = ("Not known: `%s` calls no DebugPrintOpcode, so the "
                          "mnemonic and length are not recoverable from it."
                          % handler)
        short = info["short"] or "UNKNOWN"
        name = "%02X_%s" % (code, short)
        io.open(os.path.join(outdir, name + ".md"), "w",
                encoding="utf-8", newline="\n").write(render(code, info, reason))
        index.append((code, short, name, handler, info["length"],
                      info["argbytes"], info.get("status") or "verified"))

    lines = ["---", "title: Opcodes", "---", "",
             "Generated by `tools/opcode_docs.py` from the field overlay's",
             "`g_FieldOpcodes` dispatch table and the decompiled handlers.",
             "Do not edit by hand.", "",
             "Length is `PC_INC`; `dbg` is how many argument bytes the "
             "in-game debugger prints (it saturates at 8 and is not the "
             "length). Only `verified` rows are byte-exact evidence.", "",
             "| Opcode | Short name | Length | dbg | Status | Handler |",
             "|--------|------------|--------|-----|--------|---------|"]
    for code, short, name, handler, length, dbg, st in index:
        lines.append("| 0x%02X | [%s](Opcodes/%s) | %s | %s | %s | `%s` |"
                     % (code, short, name,
                        "?" if length is None else str(length),
                        "?" if dbg is None else str(dbg), st, handler))
    lines.append("")
    io.open(os.path.join(a.out, "Opcodes.md"), "w",
            encoding="utf-8", newline="\n").write("\n".join(lines))
    print("%d opcode pages + index -> %s" % (len(index), a.out))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
