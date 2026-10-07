# Project-owned QEMU baseline

The project keeps a pinned upstream QEMU source integration so that the EDU
device can be developed and tested without changing Poky or a system QEMU
installation.

## Baseline

The `qemu/` submodule points to upstream QEMU `v8.2.7`, commit
`317c999868dfbc6e4630a39ca10cf189b81d17ad`. This is the QEMU version selected
by the active Yocto Scarthgap metadata. The `qemuarm64` machine uses
`qemu-system-aarch64`, `-machine virt`, and `-cpu cortex-a57`, so this baseline
matches the existing platform rather than selecting an unrelated newer
release.

## Initialize and build

Clone the repository and initialize the pinned source:

```sh
git clone <repository-url>
cd embedded-linux-platform
git submodule update --init --recursive
```

Build only the AArch64 system emulator out of tree:

```sh
tools/build-qemu.sh
```

The script verifies the exact submodule revision, configures QEMU once, and
builds it with Ninja. By default, build output is placed outside the repository
at:

```text
$XDG_CACHE_HOME/embedded-linux-platform/qemu-8.2.7/
```

Set `QEMU_BUILD_DIR` to choose another build directory, or
`QEMU_INSTALL_DIR` to choose the installation prefix. The resulting emulator
is `qemu-system-aarch64` in the selected build directory.

Required host tools are Python 3, Meson, Ninja, a C compiler, and the
dependencies reported by QEMU's configure step. The script does not install
packages or modify system configuration.

## EDU development boundary

The initial integration contains no project-specific QEMU changes. The future
EDU model should be implemented in the pinned QEMU source and kept as a
focused project change, with device-model tests alongside the QEMU test
framework. Upstream baseline changes should be made as explicit submodule
updates, while EDU changes should remain isolated in a project patch series or
maintained fork so they can be reviewed independently.
