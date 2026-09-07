# EDU device driver

This directory contains the authoritative Linux kernel module recipe and
source for the EDU virtual device. The driver intentionally separates the
platform-device lifecycle from the userspace-facing character-device
interface.

## Design areas

- `files/edu_main.c`: platform driver, probe/remove, and registration.
- `files/edu_chrdev.c`: file operations and ioctl handling.
- `files/edu_fifo.c`: lock-aware ring buffer implementation.
- `files/edu_irq.c`: IRQ handling and workqueue processing.
- `files/edu_hw.c`: MMIO resource mapping and hardware abstraction.
- `files/edu_sysfs.c`: sysfs attribute export.
- `files/edu_debugfs.c`: debugfs diagnostic entries.
- `files/edu_recovery.c`: fault injection and recovery logic.
- `files/edu_pm.c`: suspend/resume hooks.
