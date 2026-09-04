# meta-embedded-lab

This Yocto layer implements a portfolio-grade embedded Linux platform for a simulated EDU device. The design emphasizes a real Linux driver architecture, a production-style device model, and a reproducible build flow that can later be ported to real ARM hardware.

## Layer purpose

- Add a custom kernel device tree fragment for the EDU virtual device.
- Build a custom Linux kernel module that follows platform driver conventions.
- Expose the device through a character device interface and sysfs/debugfs entries.
- Provide a user-space CLI application for validation and diagnostics.
- Support reproducible image builds via a custom image recipe.

## Structure

- `conf/layer.conf`: Yocto layer metadata.
- `drivers/edu-device`: Linux kernel driver implementation as a virtual hardware abstraction.
- `userspace/edu-cli`: Command-line userspace test and diagnostic tool.
- `recipes-kernel/linux`: Kernel appends and device tree fragments.
- `recipes-drivers/edu-device`: Out-of-tree kernel module recipe.
- `recipes-apps/edu-cli`: Userspace package recipe.
- `recipes-core/images`: Embedded lab image recipe.
- `docs/architecture.md`: Architectural decision log.
- `tests`: Unit and integration checks.

## Design notes

This project intentionally models a virtual device rather than pretending to represent real GPIO hardware. The Linux driver still follows the same abstractions used in real embedded systems: platform driver matching, IRQ handling, char-device I/O, sysfs, debugfs, resource locking, and power-management hooks.

## Build notes

The layer is intentionally kept separate from `meta-yocto-bsp` and is meant to be added to a Poky build via `bitbake-layers add-layer ../meta-embedded-lab` or by appending it to `build/conf/bblayers.conf`.
