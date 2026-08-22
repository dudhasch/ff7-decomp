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
for u in field field2 field3 field4 field5; do
  src="src/field/$u.c"
  [ -f "$src" ] || continue
  names=$(tr '\n' ' ' < "$src" | grep -o 'MASPSX_OVERRIDE([^)]*)' \
          | sed 's/.*, *//;s/)//' | tr -d ' ')
  [ -z "$names" ] && continue
  ninja -t commands "build/us/$src.o" | tail -1 \
    | sed -e 's/-o build[^ ]*/-o \/tmp\/nm.o/' \
          -e 's/cc1-psx-26 /cc1-psx-26 -DNON_MATCHINGS /' > /tmp/c.sh
  sh /tmp/c.sh >/dev/null 2>&1 || { echo "$u: NON_MATCHINGS build failed"; continue; }
  mipsel-linux-gnu-objdump -d /tmp/nm.o > /tmp/nm.dis 2>/dev/null
  echo "== $u"
  .venv/bin/python3 tools/uninit_regs.py /tmp/nm.dis $names 2>&1
done
