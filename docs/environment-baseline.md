# Environment baseline

This baseline records the environment observed during repository separation.
Poky remains an external dependency and is not part of this repository.

## Host

- WSL: WSL 2
- Distribution: Ubuntu 26.04 LTS (codename `resolute`)
- Poky path: `~/Projects/Embedded/Yocto/poky`
- Poky repository branch: `scarthgap`
- Poky repository: official `https://git.yoctoproject.org/poky.git`

## Yocto

- BitBake: 2.8.1
- MACHINE: `qemux86-64`
- DISTRO: `poky`
- Build directory: `~/Projects/Embedded/Yocto/poky/build`
- Kernel recipe: `linux-yocto_6.6.bb`
- Kernel source version from the checked-out recipe: `6.6.142`

Configured layers in the Poky build configuration at audit time:

- `meta`
- `meta-poky`
- `meta-yocto-bsp`
- `meta-abhijeet`
- `meta-embedded-lab`

The configured layer set did not parse successfully because
`meta-embedded-lab/conf/layer.conf` declared an `openembedded-layer`
dependency that was not enabled. That dependency has not been changed during
repository separation.

## Tool availability

- `qemu-system-x86_64`: not found in `PATH`
- `qemu-system-aarch64`: not found in `PATH`
- `qemu-system-arm`: not found in `PATH`
- `aarch64-poky-linux-gcc`: not found in `PATH`
- `arm-poky-linux-gnueabi-gcc`: not found in `PATH`

These values are a point-in-time baseline, not a claim that the tools cannot
be installed or produced by a Yocto SDK later.
