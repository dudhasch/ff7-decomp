"""Make one parked body the live one, so import.py sees it.

    .venv/bin/python3 tools/unpark.py <file.c> <FuncName>

Removes the `#ifndef NON_MATCHINGS` / pin-macro / `#else` header of that one
function and the `#endif` that closes it. Restore with git checkout.

Both pin macros are recognised: `MASPSX_OVERRIDE`, and a bare `INCLUDE_ASM`
inside the same guard, which is how src/menu/*.c parks its near-misses. The
search anchors on the guard rather than on the macro, so an ordinary file-scope
`INCLUDE_ASM` (a function with no C body at all) is not mistaken for a park.
"""
import io, re, sys

PIN = re.compile(r"\b(?:MASPSX_OVERRIDE|INCLUDE_ASM)\s*\(")


def find_park(lines, name):
    """(#ifndef line, #else line, #endif line) of the block pinning `name`."""
    for i, ln in enumerate(lines):
        if not ln.startswith("#ifndef NON_MATCHINGS"):
            continue
        els = i + 1
        while els < len(lines) and not lines[els].startswith("#else"):
            els += 1
        if els >= len(lines):
            continue
        blob = "\n".join(lines[i + 1:els])
        if not PIN.search(blob):
            continue
        if not re.search(r"\b%s\s*\)" % re.escape(name), blob):
            continue
        end, depth = els + 1, 0
        while True:
            l2 = lines[end]
            if l2.startswith("#if"):
                depth += 1
            elif l2.startswith("#endif"):
                if depth == 0:
                    break
                depth -= 1
            end += 1
        return i, els, end
    return None


if __name__ == "__main__":
    path, name = sys.argv[1], sys.argv[2]
    s = io.open(path, encoding="utf-8", newline="").read()
    lines = s.split("\n")
    found = find_park(lines, name)
    if found is None:
        sys.exit("no parked body for %s in %s" % (name, path))
    start, els, end = found
    out = lines[:start] + lines[els + 1:end] + lines[end + 1:]
    io.open(path, "w", encoding="utf-8", newline="\n").write("\n".join(out))
    print("unparked %s (%d..%d, endif %d)" % (name, start, els, end))
