# Building on SteamOS (and other immutable-rootfs distros)

This guide covers SteamOS / Steam Deck, and applies equally to any host with a
read-only root filesystem (Fedora Silverblue, Bazzite, NixOS, ...).

## Why this is different

The native setup in the [README](../README.md) assumes you can install the
mipsel cross-toolchain into the system. On SteamOS you cannot, for two reasons:

1. **The root filesystem is immutable.** `steamos-readonly status` reports
   `enabled`. You can disable it and `pacman -S`, but every SteamOS update
   re-images `/usr` and silently destroys the toolchain.
2. **Toolchain version matters for byte-identical output.** The project is
   validated against `binutils-mipsel-linux-gnu` from Ubuntu questing. Ubuntu
   24.04's 2.42 ships a `mipsel-linux-gnu-ld` that does *not* match the game
   (see the comment in `.github/workflows/build.yaml`). Arch/AUR packages are
   not what CI validates against.

The fix is to run the build inside a container built from this repository's own
`Dockerfile` — the same Ubuntu base and the same questing binutils that CI uses.
**The container supplies the toolchain only; it does not change a single build
step.** Output is byte-identical to the native and CI builds, and the build
proves it: the final ninja target runs `sha1sum -c build/us/check.sha1`, which
compares every produced overlay against the SHA-1 of the retail disc executable
recorded in `config/us.yaml`.

SteamOS ships **podman** (rootless) preinstalled, so no Docker install is needed.

## One-time setup

### 1. Clone with submodules

```shell
git clone https://github.com/Xeeynamo/ff7-decomp.git --recursive && cd ff7-decomp
```

If you already cloned without `--recursive`:

```shell
git submodule update --init --recursive
```

### 2. Build the container image

```shell
podman build --platform=linux/amd64 --tag ff7-build:latest .
```

Takes a few minutes and about 1.5 GB. Only needed once — it survives SteamOS
updates, because it lives in `~/.local/share/containers`, not in `/usr`.

### 3. Provide the disc image

The `Makefile` expects an exact filename, and a `bin`/`cue` pair:

```
disks/Final Fantasy VII (USA) (Disc 1).bin
disks/Final Fantasy VII (USA) (Disc 1).cue
```

Two things commonly trip people up:

* **A `.iso` that is really a `.bin`.** A raw MODE2/2352 track is often
  distributed with an `.iso` extension. Check it: the file size is an exact
  multiple of 2352 and the first 12 bytes are the CD sync pattern
  `00 FF FF FF FF FF FF FF FF FF FF 00`. If so, it is a `.bin` — rename it,
  do not try to mount it.

  ```shell
  # size divisible by 2352 -> raw 2352-byte sectors, i.e. a .bin
  python3 -c 'import os,sys; s=os.path.getsize(sys.argv[1]); print(s, s%2352)' "your.iso"
  od -An -tx1 -N12 "your.iso"
  ```

* **Disk space.** Rather than copying 713 MB, hardlink it (works only if the
  source is on the same filesystem as the repo — on the Deck, both under
  `/home`):

  ```shell
  mkdir -p disks
  ln "/path/to/Final Fantasy VII (Disc 1).iso" "disks/Final Fantasy VII (USA) (Disc 1).bin"
  printf 'FILE "Final Fantasy VII (USA) (Disc 1).bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n' \
      > "disks/Final Fantasy VII (USA) (Disc 1).cue"
  ```

  The `.cue` must name the `.bin` next to it, or `bchunk` will fail.

Confirm you have the right disc — it must be **USA Disc 1**, boot id
`SCUS_941.63`. `make disks` (next step) extracts it, after which:

```shell
sha1sum disks/us/SCUS_941.63
# must print a95e8b16b97071203b953bb81a33980509262f30 (the value in config/us.yaml)
```

If that SHA-1 differs, you have a different region or revision and the build
will not match.

### 4. Python venv and disc extraction

```shell
./tools/podman-build.sh 'make requirements && make disks'
```

`make disks` runs `bchunk` to convert the `bin`/`cue` into an ISO, then extracts
its 3377 files into `disks/us/`. Budget roughly 1.3 GB on top of the `.bin`.

## Everyday use

`tools/podman-build.sh` is the podman twin of `tools/docker-build.sh`. Prefix any
normal command with it:

```shell
./tools/podman-build.sh 'make build'      # build; ends with the sha1 check
./tools/podman-build.sh 'make format'     # clang-format the codebase
./tools/podman-build.sh 'make report'     # progress report -> build/report.json
./tools/podman-build.sh './mako.sh rank src/battle'
./tools/podman-build.sh './mako.sh dec func_800A6858'
./tools/podman-build.sh bash              # interactive shell in /ff7
```

A successful `make build` ends with:

```
[91/91] check
build/us/main.exe: OK
build/us/battle.exe: OK
... (13 overlays)
```

Every `OK` is one overlay reproduced byte-for-byte from source. Anything else
means the build does not match.

### Why the wrapper needs `--userns=keep-id`

Rootless podman maps the container's UID 1000 (the image's `ubuntu` user) to a
subordinate UID on the host, which would leave every file written into the
bind-mounted repository owned by UID 100999 and unusable. `--userns=keep-id`
keeps UID 1000 equal to your host user, so build artifacts get normal ownership.
The wrapper handles this for you.

## Notes on repository changes made for this platform

Only two, neither of which affects generated code:

* `Dockerfile`: added `wget` and `ca-certificates` (the `bin/%.gz` rule in the
  `Makefile` downloads the `cc1-psx` compilers with `wget`, and `ubuntu:noble`
  ships neither), and moved the `chown` of `/ff7` ahead of `USER ubuntu`. Docker
  creates a `WORKDIR` owned by the current `USER`, buildah/podman creates it
  root-owned, so the chown had to happen while still root. The resulting image
  is identical under Docker.
* `tools/podman-build.sh`: new file, the podman equivalent of the existing
  `tools/docker-build.sh`.

The native and Docker workflows are untouched.

## Troubleshooting

| Symptom | Cause / fix |
| --- | --- |
| `error: image 'ff7-build:latest' not found` | Run the `podman build` step above. |
| `bchunk: ... No such file or directory` | The `.cue` names a `.bin` that isn't there. The `FILE` line must match the actual filename. |
| `sha1sum: WARNING: 1 computed checksum did NOT match` | The build does not reproduce the game. Check `disks/us/SCUS_941.63` has the SHA-1 above; otherwise it is a genuine regression in the source. |
| Files in the repo owned by a high UID (100000+) | You ran podman without `--userns=keep-id`. Use `tools/podman-build.sh`. |
| Out of disk space | `disks/` needs ~2 GB, the image ~1.5 GB, plus `asm/`, `build/` and `expected/` around 45 MB. `disks/*.iso` can be deleted after `make disks`. |
