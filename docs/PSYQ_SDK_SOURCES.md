# PsyQ SDK — Where to Find It and What to Use

Reference material for the ~515 remaining functions in `src/main/psxsdk.c`
and for validating the headers in `include/psxsdk/`. The project compiles
with PsyQ 3.3 / 3.5 / 3.6 / 4.0 toolchains (`//! PSYQ=...` header line), so
those SDK versions are the relevant ones.

> **Legal note:** the PsyQ SDK is Sony copyright. Standard practice in
> decompilation projects is to keep the SDK locally as a reference and
> commit only derived information (signatures, struct layouts, semantics) —
> never the SDK files themselves. Nothing from these archives belongs in
> this repository.

## 1. archive.org — `ps1_sdks` collection

<https://archive.org/details/ps1_sdks> — 7.8 GB, uploaded 2021-01-15 by
"PlayStation SDK and Service". The authoritative dump. File listing:
<https://archive.org/download/ps1_sdks>

Relevant items (verified present in the listing):

| File | Contents |
| --- | --- |
| `Programmer Tool - Runtime Library Version 3.3 (Japan)_DTL-S2190_redump.zip` (362 MB) | Runtime lib 3.3 — headers, libs, docs |
| `Programmer Tool - Runtime Library Version 3.5 (Japan) (En,Ja)_DTL-S2300_redump.zip` (372 MB) | Runtime lib 3.5 |
| `Programmer Tool - Runtime Library Version 3.6 (Japan)_DTL-S2310_redump.zip` (366 MB) | Runtime lib 3.6 |
| `Programmer Tool - Runtime Library Version 4.0 (Japan)_DTL-S2320_redump.zip` (392 MB) | Runtime lib 4.0 |
| `Programmer Tools - Run-time Library 4.0 (USA) (Release 2.0)_DTL-S2002_redump.zip` (374 MB) | USA 4.0 release |
| `Programmer Tools Runtime Library 3.3 DTL-S2190.rar` (278 MB) | Loose 3.3 repack |
| `Programmer Tools Runtime Library 3.5 CD Release 1.7.rar` (325 MB) | Loose 3.5 repack |
| `Programmer Tools Runtime Library 3.6 DTL-S2002 CD Release 1.0.zip` (356 MB) | Loose 3.6 repack |
| `GNU C Compiler Version 2.60 (World) (Disk 1/2).img` | The gcc 2.6.0 toolchain floppies |
| `Technical Reference - Release 2.0 (USA)_DTL-S2003_redump.zip` (165 MB) | Docs: libapi/libgte/libetc/libgs signatures |
| `Technical Reference 3.6 DTL-S2003 CD Release 1.8.zip` (281 MB) | Docs matching 3.6 |
| `Technical Reference 4.1 DTL-S2003 CD Release 2.1 Dec1997.zip` (340 MB) | Docs matching 4.1 |
| `Runtime Library 4.4.7z` (78 MB) / `Runtime Library 4.7.zip` (6 MB) | Newer libs |
| `[SONY].PlayStation Programer Tool.iso` (87 MB) | Programmer Tool disc — contains **C sources for parts of the runtime library** |
| `Net Yaroze Software Development Disc (USA, Europe)_redump.zip` (75 MB) | Yaroze lib sources — some libetc/libapi C source |

## 2. psx.arthus.net — PsyQ mirror (convenience copies)

<https://psx.arthus.net/sdk/Psy-Q/> — smaller, faster individual downloads:

| File | Size | Why it matters |
| --- | --- | --- |
| `psyq-4.7-converted-full.7z` | 1.5 MB | **Headers pre-converted to GCC-readable syntax** — the format PSX decomp projects standardise on |
| `psyq-4_7-converted-light.zip` | 660 KB | Same, minimal |
| `Psy-Q_46.zip` / `Psy-Q_47.zip` | 6.7 / 5.8 MB | Bare newer SDK drops |
| `Runtime Library 4.6.7z` | 5.5 MB | Runtime lib 4.6 |
| `PSYQ.SDevTC.Developers.Toolkit.For.PSX.v4.5-MFD.zip` | 9.1 MB | SDevTC 4.5 |
| `DOCS/` directory | — | Loose documentation |
| `sources/` directory | — | Loose source fragments |

Setup/howto links (context, not needed for matching):
<http://www.psxdev.net/help/psyq_install.html>,
<https://www.retroreversing.com/psyq-sdk-setup>

## 3. What to actually do with it

1. **Header diff (highest value / lowest effort).** Extract the 3.5 / 3.6 /
   4.0 `include/` trees and diff `libapi.h`, `libgte.h`, `libetc.h`,
   `libgs.h`, `kernel.h` against `include/psxsdk/`. Wrong prototypes are a
   recurring cause of `psxsdk.c` near-misses; the originals settle them.
2. **Function semantics for `psxsdk.c`.** The Technical Reference PDFs give
   exact signatures and behaviour for `OpenEvent`, `DeliverEvent`,
   `InitHeap`, `FlushCache`, the PAD functions, etc. The ≤ 8-instruction
   wrappers are syscall thunks whose C shape is fixed once the syscall
   number is known — with the reference open, they match almost
   mechanically.
3. **Library C sources where they exist.** The Yaroze discs and the
   Programmer Tool ISO ship C source for parts of libetc/libapi. Where a
   `psxsdk.c` function corresponds to shipped source, the decompilation
   becomes a comparison, not a reconstruction.
4. **Version pinning evidence.** If a whole translation unit resists
   matching, the SDK changelog/readme files help confirm which PsyQ version
   FF7's modules were built with — legitimate evidence for a
   `//! PSYQ=...` line change (CLAUDE.md forbids changing it without
   unit-wide evidence).

## 4. Do NOT commit

- SDK headers, libs, PDFs, or extracted trees (copyright).
- The ~1.3 GB of ISOs/RARs — keep them outside the repo or in a
  gitignored scratch directory.
