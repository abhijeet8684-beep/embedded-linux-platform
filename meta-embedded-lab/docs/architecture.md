# EDU embedded lab architecture

## 1. Overview

The target platform is a small embedded Linux system that exposes a virtual EDU device to the kernel and userspace. The implementation is intentionally split into the following layers:

- Hardware abstraction layer: emulates virtual control/status registers.
- Kernel driver layer: platform driver, IRQ logic, char device, buffer management.
- Userspace API: command-line test harness that interacts via `ioctl` and read/write calls.
- Yocto packaging: module, userspace binary, and image definitions.

## 2. Virtual hardware model

A real embedded design would expose hardware registers through an MMIO region or bus probing flow. Because the lab runs in WSL/QEMU without real sensors or GPIO, the project models equivalent operations with a software-only register block:

- `CONTROL`
- `STATUS`
- `DATA`
- `IRQ_STATUS`
- `IRQ_ENABLE`
- `FIFO_LEVEL`
- `VERSION`
- `ERROR_STATUS`

This register abstraction does not claim to emulate the exact timing or electrical behavior of physical hardware. Instead, it models the same software contract a real platform device would expose.

## 3. Linux driver architecture

The driver is structured as a platform driver with a character device front end. This keeps the logical responsibilities clear:

- `probe/remove`: device lifecycle, resource registration.
- `chrdev`: user-facing file operations, `open`, `close`, `read`, `write`, `poll`, `ioctl`.
- `irq`: interrupt generation and workqueue handling.
- `fifo`: lock-aware ring buffer for producer/consumer flow.
- `sysfs`: runtime metrics and control knobs.
- `debugfs`: diagnostics for developers and boot-time troubleshooting.
- `recovery`: fault injection and self-healing paths.
- `pm`: suspend/resume and runtime-power behavior.

## 4. Concurrency model

The driver uses a ring buffer protected by a mutex and spinlock. Producer and consumer paths are separated to avoid lock contention. Interrupt and user-driven data paths are synchronized with `wait_event` and `work_struct` so that the device remains responsive under stress.

## 5. Error handling and recovery

The implementation includes explicit status words and watchdog-style checks for conditions such as:

- empty or full FIFO states
- repeated IRQ storms
- stale interrupt state
- invalid `ioctl` arguments
- simulated hardware faults

The recovery path raises errors to the debugfs and sysfs views while allowing the driver to continue operating safely.

## 6. Userspace interface

The `edu-cli` tool coordinates with the kernel device via ioctl and simple file operations. It is meant for:

- probing the device and reading metadata
- resetting the simulated device state
- writing sample payloads
- reading FIFO data
- validating error and IRQ status

## 7. QEMU and future porting

The system is intended to boot under QEMU and later be ported to a real ARM target by replacing the virtual hardware abstraction layer while keeping the rest of the driver API intact.

## 8. Portability guidance

To port to real hardware:

1. Replace the virtual register implementation with actual MMIO access.
2. Keep the platform device match table and `probe` contract.
3. Preserve the user-space ABI and ioctl values.
4. Rework the interrupt source, but keep the same driver internals.

This keeps the architecture sound without pretending that the simulation is identical to real silicon.
