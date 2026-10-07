#!/bin/sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
qemu_source="$project_root/qemu"
qemu_commit=317c999868dfbc6e4630a39ca10cf189b81d17ad
build_root=${QEMU_BUILD_DIR:-"${XDG_CACHE_HOME:-$HOME/.cache}/embedded-linux-platform/qemu-8.2.7"}
prefix=${QEMU_INSTALL_DIR:-"$build_root/install"}

if [ ! -f "$qemu_source/configure" ]; then
    printf '%s\n' "QEMU source is missing. Run: git submodule update --init --recursive" >&2
    exit 1
fi

actual_commit=$(git -C "$qemu_source" rev-parse HEAD)
if [ "$actual_commit" != "$qemu_commit" ]; then
    printf '%s\n' "QEMU submodule is not pinned to the approved revision." >&2
    printf 'Expected: %s\nActual:   %s\n' "$qemu_commit" "$actual_commit" >&2
    exit 1
fi

command -v python3 >/dev/null 2>&1 || {
    printf '%s\n' "Missing prerequisite: python3" >&2
    exit 1
}
command -v meson >/dev/null 2>&1 || {
    printf '%s\n' "Missing prerequisite: meson" >&2
    exit 1
}
command -v ninja >/dev/null 2>&1 || {
    printf '%s\n' "Missing prerequisite: ninja" >&2
    exit 1
}

mkdir -p "$build_root"
if [ ! -f "$build_root/config-host.mak" ]; then
    cd "$build_root"
    "$qemu_source/configure" \
        --target-list=aarch64-softmmu \
        --prefix="$prefix" \
        --disable-werror \
        --disable-docs \
        --disable-gtk \
        --disable-sdl \
        --disable-vnc \
        --disable-tools
fi

ninja -C "$build_root"
printf 'QEMU binary: %s\n' "$build_root/qemu-system-aarch64"
