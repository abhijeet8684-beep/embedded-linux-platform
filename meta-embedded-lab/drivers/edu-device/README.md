# EDU driver source tree

This directory contains the Linux kernel-side implementation of the EDU virtual device. The driver intentionally separates the platform-device lifecycle from the userspace-facing character-device interface.

## Design areas

- `edu_main.c`: platform driver, probe/remove, and registration.
- `edu_chrdev.c`: file operations and ioctl handling.
- `edu_fifo.c`: lock-aware ring buffer implementation.
- `edu_irq.c`: simulated IRQ generation and workqueue handling.
- `edu_hw.c`: virtual hardware abstraction layer.
- `edu_sysfs.c`: sysfs attribute export.
- `edu_debugfs.c`: debugfs diagnostic entries.
- `edu_recovery.c`: fault injection and recovery logic.
- `edu_pm.c`: suspend/resume hooks.

The virtual device intentionally models a real hardware contract without pretending to provide equivalent electrical behavior.
