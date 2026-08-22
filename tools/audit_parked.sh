#!/bin/sh
# Audit every MASPSX_OVERRIDE body in the field overlay for the
# undefined-register bug -- a local whose only assignment sits on a path gcc
# proved unreachable, which keeps its use, loses its def, and still compiles,
# links and scores. See "ways a clean-looking diff lies" in CLAUDE.md and
# tools/uninit_regs.py.
#
#     ./tools/docker-build.ps1 'sh tools/audit_parked.sh'
#
# One build per unit: -DNON_MATCHINGS unparks every body at once. The offsets
# are wrong in that configuration and it does not matter here, because a
# register with no write anywhere is visible whatever the neighbours do.
#
# Run it before spending a budget on any parked body, and after any change to
# one. An invalid body can score BETTER than the correct program -- an allocno
# whose live range runs back to function entry perturbs the whole conflict
# graph -- so a good row count is not evidence of a valid program.
# Sources to audit: any given on the command line, else every unit known to
# carry parked bodies. Add a unit here when you park your first body in it.
srcs="$*"
[ -n "$srcs" ] || srcs="src/field/field.c src/field/field2.c src/field/field3.c
src/field/field4.c src/field/field5.c src/world/world2.c"

for src in $srcs; do
  [ -f "$src" ] || continue
  names=$(tr '\n' ' ' < "$src" | grep -o 'MASPSX_OVERRIDE([^)]*)' \
          | sed 's/.*, *//;s/)//' | tr -d ' ')
  [ -z "$names" ] && continue
  # The `//!` header picks the compiler per unit, so match whichever cc1 the
  # ninja edge names -- world2.c is cc1-psx-272 where the field units are -26,
  # and a hardcoded name silently produces a build without -DNON_MATCHINGS.
  ninja -t commands "build/us/$src.o" | tail -1 \
    | sed -e 's/-o build[^ ]*/-o \/tmp\/nm.o/' \
          -e 's/\(cc1-psx-[0-9]*\) /\1 -DNON_MATCHINGS /' > /tmp/c.sh
  sh /tmp/c.sh >/dev/null 2>&1 || { echo "$src: NON_MATCHINGS build failed"; continue; }
  mipsel-linux-gnu-objdump -d /tmp/nm.o > /tmp/nm.dis 2>/dev/null
  echo "== $src"
  .venv/bin/python3 tools/uninit_regs.py /tmp/nm.dis $names 2>&1
done
