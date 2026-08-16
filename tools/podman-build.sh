#!/bin/bash
# Rootless-podman build wrapper for FF7 decomp.
#
# For hosts with an immutable / read-only root filesystem (SteamOS, Silverblue,
# Bazzite, ...) where the mipsel toolchain cannot be installed natively, or where
# the distro's binutils is not the version the project is validated against.
#
# This is the podman twin of tools/docker-build.sh. It runs the exact same
# ff7-build image, which is built from the repository's own Dockerfile
# (ubuntu:noble + binutils-mipsel-linux-gnu from questing), i.e. the same
# assembler and linker the GitHub Actions workflow uses. Output is therefore
# byte-identical to the native and CI builds -- the container only supplies the
# toolchain, it does not change any build step.
#
# Usage:
#   ./tools/podman-build.sh "make build"
#   ./tools/podman-build.sh "make format"
#   ./tools/podman-build.sh bash              # interactive shell
#
# One-time image build:
#   podman build --platform=linux/amd64 --tag ff7-build:latest .

set -euo pipefail

IMAGE="${FF7_IMAGE:-ff7-build:latest}"
REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

if ! podman image exists "$IMAGE"; then
    echo "error: image '$IMAGE' not found." >&2
    echo "Build it once with:" >&2
    echo "    cd '$REPO_ROOT' && podman build --platform=linux/amd64 --tag ff7-build:latest ." >&2
    exit 1
fi

# Rootless podman maps the container's UID 1000 (the image's 'ubuntu' user) to a
# subuid on the host, which would make every file written into the bind-mounted
# repository unreadable to the invoking user. --userns=keep-id keeps UID 1000 ==
# the host user, so build artifacts land with normal ownership.
run_opts=(
    --rm
    --userns=keep-id
    --workdir /ff7
    --volume "$REPO_ROOT":/ff7
    --volume ff7_go_cache:/gocache
)

# Allocate a TTY only when there is one, so piping/CI use stays clean.
if [ -t 0 ] && [ -t 1 ]; then
    run_opts+=(--interactive --tty)
fi

exec podman run "${run_opts[@]}" "$IMAGE" -lc "cd /ff7 && $*"
