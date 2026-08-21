"""Make one MASPSX_OVERRIDE-parked body the live one, so import.py sees it.

    .venv/bin/python3 tools/unpark.py <file.c> <FuncName>

Removes the `#ifndef NON_MATCHINGS` / MASPSX_OVERRIDE(...) / `#else` header of
that one function and the `#endif` that closes it. Restore with git checkout.
"""
import io, re, sys

path, name = sys.argv[1], sys.argv[2]
s = io.open(path, encoding="utf-8", newline="").read()
lines = s.split("\n")

# find the MASPSX_OVERRIDE whose argument list names this function
ov = None
for i, ln in enumerate(lines):
    if re.match(r"\s*MASPSX_OVERRIDE\s*\(", ln):
        blob = "\n".join(lines[i:i + 3])
        if re.search(r"\b%s\s*\)" % re.escape(name), blob):
            ov = i
            break
if ov is None:
    sys.exit("no MASPSX_OVERRIDE for %s in %s" % (name, path))

# header: the #ifndef above it, and the #else below it
start = ov
while not lines[start].startswith("#ifndef NON_MATCHINGS"):
    start -= 1
els = ov
while not lines[els].startswith("#else"):
    els += 1

# the #endif that closes the block: first line == "#endif" after the body
end = els + 1
depth = 0
while True:
    ln = lines[end]
    if ln.startswith("#if"):
        depth += 1
    elif ln.startswith("#endif"):
        if depth == 0:
            break
        depth -= 1
    end += 1

out = lines[:start] + lines[els + 1:end] + lines[end + 1:]
io.open(path, "w", encoding="utf-8", newline="").write("\n".join(out))
print("unparked %s (%d..%d, endif %d)" % (name, start, els, end))
