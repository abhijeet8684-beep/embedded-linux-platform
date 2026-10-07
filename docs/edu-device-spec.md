# EDU Virtual Peripheral Specification

## 1. Purpose and scope

The EDU peripheral is a small virtual educational and test device for the
`embedded-linux-platform` project. It provides a stable MMIO, FIFO, and
interrupt contract that can be implemented by a QEMU device model and used
by the Linux platform driver.

The device is intended to exercise platform-driver probing, resource
mapping, register access, interrupt handling, FIFO synchronization, error
reporting, reset/recovery, sysfs/debugfs visibility, and userspace I/O.
It is not a model of real production hardware and has no electrical or
timing equivalence to a physical device.

This specification describes the target hardware contract. Existing driver
gaps are recorded in section 13 and are not fixed by this document.

## 2. System architecture

```text
edu-cli
   |
   v
/dev/edu0
   |
   v
Linux EDU platform driver
   |
   v
Device Tree
   |
   v
QEMU EDU peripheral
```

- **edu-cli**: A diagnostic client for reset, status, data, and fault tests.
- **`/dev/edu0`**: The character-device ABI exposed after successful probe.
- **Linux platform driver**: Maps MMIO, requests the IRQ, synchronizes FIFO
  access, translates hardware state to the character-device ABI, and exposes
  diagnostics.
- **Device Tree**: Describes the compatible device, MMIO region, and IRQ.
- **QEMU peripheral**: Owns register state, FIFO state, interrupt assertion,
  and deterministic fault behavior.

## 3. MMIO memory map

The peripheral occupies one 4 KiB region. The proposed base address is
`0x20000000`, matching the address currently described by the project DT
fragment for QEMU virt. The base is a design choice only; the DTS is not
changed by this specification.

All registers are 32-bit, little-endian, and naturally aligned. Reserved
bits read as zero and writes to reserved bits are ignored.

| Offset | Register | Access | Reset | Definition |
| --- | --- | --- | --- | --- |
| `0x00` | `CONTROL` | RW | `0x00000000` | Enable, reset, and IRQ enable |
| `0x04` | `STATUS` | RO | `0x00000001` | Ready, data, and error state |
| `0x08` | `DATA` | RW | `0x00000000` | FIFO producer/consumer port |
| `0x0C` | `IRQ_STATUS` | RW1C | `0x00000000` | Latched interrupt causes |
| `0x10` | `IRQ_ENABLE` | RW | `0x00000000` | Interrupt mask |
| `0x14` | `VERSION` | RO | `0x00010000` | Hardware contract version |
| `0x18` | `FIFO_LEVEL` | RO | `0x00000000` | Number of queued words |
| `0x1C` | `ERROR_STATUS` | RW1C | `0x00000000` | Latched error causes |

The remaining region is reserved and must return zero.

## 4. CONTROL register

`CONTROL` fields:

| Bit | Name | Reset | Meaning |
| --- | --- | --- | --- |
| 0 | `ENABLE` | 0 | Enables normal DATA/FIFO operation |
| 1 | `RESET` | 0 | Writing one performs a device reset; reads as zero |
| 2 | `IRQ_ENABLE` | 0 | Master interrupt enable |

`ENABLE=0` leaves the device idle. DATA reads and writes while disabled are
invalid operations and set `ERROR_STATUS.INVALID_OP`. Setting `RESET` performs
the reset in section 10 and does not remain set. `IRQ_ENABLE` gates the
interrupt output in addition to the per-cause `IRQ_ENABLE` register.

## 5. STATUS register

| Bit | Name | Meaning |
| --- | --- | --- |
| 0 | `READY` | Device is initialized and enabled |
| 1 | `DATA_AVAILABLE` | FIFO contains at least one word |
| 2 | `ERROR` | One or more error bits are latched |

`READY` is one only when `CONTROL.ENABLE` is one and the device is not in
reset. `DATA_AVAILABLE` tracks `FIFO_LEVEL != 0`. `ERROR` tracks
`ERROR_STATUS != 0`.

## 6. DATA register

When enabled, a DATA write appends one 32-bit word to the FIFO. A DATA read
removes and returns the oldest word. Reads from an empty FIFO set
`ERROR_STATUS.INVALID_OP` and return zero; writes to a full FIFO set
`ERROR_STATUS.FIFO_OVERFLOW` and do not modify the FIFO.

A successful write that changes the FIFO from empty to non-empty latches
`IRQ_STATUS.DATA_AVAILABLE` when that cause is enabled. DATA writes do not
generate an interrupt for every word, which keeps behavior deterministic.

## 7. Interrupt model

The device has one level-high interrupt output connected to the DT interrupt.
The output is asserted when:

```text
CONTROL.IRQ_ENABLE &&
(
  (IRQ_ENABLE.DATA_AVAILABLE && IRQ_STATUS.DATA_AVAILABLE) ||
  (IRQ_ENABLE.ERROR && IRQ_STATUS.ERROR)
)
```

`IRQ_STATUS` fields:

| Bit | Name | Cause |
| --- | --- | --- |
| 0 | `DATA_AVAILABLE` | FIFO transitioned from empty to non-empty |
| 1 | `ERROR` | An error bit was latched |

`IRQ_ENABLE` uses the same bit positions and is reset to zero. Causes are
acknowledged by writing one to the corresponding `IRQ_STATUS` bit (RW1C).
Acknowledging DATA_AVAILABLE while data remains queued does not immediately
re-latch it; a later empty-to-non-empty transition is required. Errors remain
visible through `STATUS.ERROR` until their `ERROR_STATUS` bits are cleared.

The Linux driver must read `IRQ_STATUS`, service the FIFO or error, write the
handled bits back to `IRQ_STATUS`, and avoid returning from the handler while
an enabled cause remains asserted.

## 8. FIFO model

The hardware FIFO is 128 entries deep, with each entry a 32-bit word. This
matches the intended DT `fifo-depth = <128>` property. The QEMU model owns
the hardware FIFO; the driver may maintain software bookkeeping but must not
invent a second hardware state.

- DATA writes are the producer operation.
- DATA reads are the consumer operation.
- `FIFO_LEVEL` reports the inclusive range 0 through 128.
- Empty reads are invalid and do not change FIFO level.
- Full writes are rejected, preserve all existing entries, and latch
  `FIFO_OVERFLOW`.
- `DATA_AVAILABLE` is set whenever the level is non-zero.
- FIFO reset discards all entries and sets level to zero.

The FIFO is intentionally small and word-oriented so ordering and overflow
can be tested without DMA or timing dependencies.

## 9. Error model

`ERROR_STATUS` fields:

| Bit | Name | Meaning |
| --- | --- | --- |
| 0 | `FIFO_OVERFLOW` | DATA write attempted while FIFO was full |
| 1 | `INVALID_OP` | DATA access while disabled or invalid empty read |
| 2 | `INJECTED` | Deterministic software-requested hardware fault |

Errors are sticky until cleared by RW1C writes or reset. Any newly latched
error sets `IRQ_STATUS.ERROR` and may assert the IRQ. `STATUS.ERROR` is a
summary of the register, not a separate latch. An injected error does not
corrupt FIFO contents or produce nondeterministic behavior.

## 10. Reset model

Power-on reset and `CONTROL.RESET=1` have the same effect:

- `CONTROL = 0`
- `STATUS = READY=0`, with no data or error
- FIFO is empty
- `FIFO_LEVEL = 0`
- `IRQ_STATUS = 0`
- `IRQ_ENABLE = 0`
- `ERROR_STATUS = 0`
- interrupt output is deasserted
- `VERSION = 0x00010000`

After reset, software writes `CONTROL.ENABLE=1` to enter the enabled state.
Reset is synchronous from the guest's point of view and completes before the
register write returns.

## 11. VERSION register

`VERSION` is the constant `0x00010000`. The upper 16 bits identify the major
register contract and the lower 16 bits identify the minor revision. A driver
may reject an unsupported major version and may expose the value through
sysfs for diagnostics.

## 12. Device Tree contract

The eventual DT node must provide:

```dts
edu-device@20000000 {
	compatible = "abhijeet,edu-device";
	reg = <0x0 0x20000000 0x0 0x1000>;
	interrupts = <GIC_SPI 12 IRQ_TYPE_LEVEL_HIGH>;
	interrupt-parent = <&gic>;
	status = "okay";
	fifo-depth = <128>;
};
```

The `compatible` string is project-specific and is the proposed final value.
The node requires a 4 KiB MMIO resource and one level-high GIC SPI interrupt.
The QEMU virt machine supplies the GIC; the EDU device is not itself an
interrupt controller. `fifo-depth` is descriptive and must agree with the
fixed hardware depth.

This is a contract only. The existing DTS and driver match string are not
modified in this step.

## 13. Driver contract

The driver should:

1. Match the compatible string and receive a platform device.
2. Map resource zero with `devm_ioremap_resource()` and validate its 4 KiB
   minimum size.
3. Read VERSION and establish the supported contract before enabling the
   device.
4. Reset the peripheral, configure IRQ causes, and set CONTROL.ENABLE.
5. Use `readl()`/`writel()` for all register access and serialize accesses
   with the existing mutex/spinlock strategy.
6. Request the one platform IRQ with device-managed lifetime.
7. Move DATA words between hardware FIFO and `/dev/edu0`, using FIFO_LEVEL and
   STATUS rather than a disconnected software-only queue.
8. In the IRQ handler, acknowledge only causes that were serviced and wake
   blocked readers or pollers.
9. Expose version, status, FIFO level, IRQ enable state, and error state
   through the existing diagnostic interfaces.
10. Use reset and error-clear operations for recovery without silently
    discarding unacknowledged error state.

## 14. Userspace contract

`edu-cli` will use `/dev/edu0` as follows:

- **information/status**: `status` obtains the driver-visible status summary;
- **data write**: `write <value>` submits one 32-bit FIFO word;
- **data read**: `read` consumes one 32-bit FIFO word;
- **reset**: `reset` returns the peripheral and driver to the reset baseline;
- **error reporting**: `fault <code>` requests a deterministic fault once the
  ioctl and hardware path support it.

The CLI must not access MMIO directly. Sysfs/debugfs remain diagnostic
interfaces rather than a replacement for the character-device ABI.

## 15. Expected end-to-end flows

### A. Device initialization

```text
DT node -> platform match -> map MMIO -> read VERSION -> reset
         -> configure IRQ -> ENABLE -> register /dev/edu0
```

### B. Normal data write/read

```text
edu-cli write -> write(2) -> driver writes DATA -> FIFO_LEVEL increases
edu-cli read  -> read(2)  -> driver reads DATA  -> FIFO_LEVEL decreases
```

### C. Interrupt-driven event

```text
FIFO empty -> DATA write -> DATA_AVAILABLE latch -> level IRQ
           -> driver reads IRQ_STATUS -> services DATA -> RW1C acknowledge
           -> wake readers/pollers -> IRQ deasserts
```

### D. FIFO full/overflow

```text
128 words queued -> next DATA write -> FIFO_OVERFLOW + ERROR IRQ
                 -> driver reports error -> software clears error/IRQ
```

### E. Hardware error

```text
invalid operation or injected fault -> ERROR_STATUS latch
                                    -> STATUS.ERROR + ERROR IRQ
                                    -> driver records and reports error
```

### F. Device reset/recovery

```text
reset request -> CONTROL.RESET -> FIFO/IRQ/errors cleared
              -> driver reinitializes configuration -> CONTROL.ENABLE
```

## 16. Reset values and state machine

```text
RESET --enable--> DISABLED --CONTROL.ENABLE--> ENABLED
                                      |
                                      +-- data available --> DATA_READY
                                      |
                                      +-- fault ----------> ERROR
```

- **RESET**: registers and FIFO have reset values; no IRQ is asserted.
- **DISABLED**: initialized but not accepting normal DATA operations.
- **ENABLED**: ready for FIFO and interrupt operation.
- **DATA_READY**: enabled with one or more queued words; it returns to
  ENABLED after the FIFO becomes empty.
- **ERROR**: enabled operation with one or more error bits; it returns to
  ENABLED after all errors and their IRQ causes are cleared.

Reset is legal from every state and returns to RESET. Disable is legal from
ENABLED, DATA_READY, or ERROR and must deassert normal operation without
implicitly clearing sticky errors.

## 17. Validation plan

Future implementation tests must cover:

- power-on and software reset register values;
- 32-bit MMIO reads/writes and reserved-bit behavior;
- enable/disable transitions;
- VERSION validation;
- FIFO ordering, empty reads, full writes, and level reporting;
- DATA_AVAILABLE and ERROR IRQ assertion, masking, and RW1C clearing;
- deterministic error injection and recovery;
- driver probe, MMIO mapping, and IRQ request;
- `/dev/edu0` creation and read/write/poll behavior;
- sysfs/debugfs status visibility;
- `edu-cli` reset, status, data, fault, and end-to-end flows;
- module unload/reload and PM suspend/resume behavior.

## 18. Design decisions

- **Fixed 4 KiB MMIO region**: aligns with the current resource validation and
  leaves room for future registers without changing the ABI.
- **32-bit word FIFO**: matches the current driver and CLI data types and is
  easy to inspect in QEMU and kernel traces.
- **128-entry depth**: matches the existing DT intent while remaining small
  enough for deterministic overflow tests.
- **Level-high, single IRQ**: maps directly to a QEMU virt GIC SPI and avoids
  edge-loss races during early driver development.
- **RW1C sticky causes**: makes interrupt acknowledgement and error recovery
  explicit and testable.
- **Constant VERSION**: permits compatibility checks without adding discovery
  complexity.
- **No DMA or timing model**: keeps the model reproducible and suitable for
  interview explanation while still exercising real Linux interfaces.

## 19. Future extensions

Possible later additions include a watchdog register, DMA simulation,
power-management state retention, multiple interrupt causes or lines, a
richer packet FIFO, and additional fault-injection controls. None is required
for the first QEMU implementation.

## 20. Compatibility review and known conflicts

The current project implementation is not yet a complete implementation of
this hardware contract:

1. The driver currently matches `"edu,device-v1"`, while this specification
   proposes `"abhijeet,edu-device"`.
2. The existing DT fragment uses `0x20000000`, IRQ 12, a 4 KiB region, and
   `fifo-depth = <128>`, which align with this proposal, but it uses the old
   compatible string.
3. `edu_mmio_init()` and the register helpers exist, but the current
   character-device, FIFO, recovery, and IRQ paths primarily operate on
   software fields and do not implement the register map above.
4. The driver FIFO is sized by `EDU_MAX_BUFSZ` (256), while the proposed
   hardware FIFO depth is 128 and the current DT advertises 128.
5. The current driver uses software status values (`READY=0x1`, data/IRQ and
   error bits) that are not yet synchronized with hardware STATUS,
   IRQ_STATUS, or ERROR_STATUS.
6. The current reset ioctl clears software state but does not write the
   proposed CONTROL reset bit or reconfigure hardware IRQ state.
7. `edu-cli` defines `EDU_IOC_FAULT_INJECT`, but the current driver ioctl
   switch does not implement that command.
8. The current driver sets `irq_enabled` as software state and requests the
   IRQ, but does not yet program the proposed CONTROL or IRQ_ENABLE registers
   or acknowledge hardware causes.
9. The current DT uses the standard `qemu,virt` root compatible and describes
   an overlay artifact; standard QEMU does not instantiate the EDU device
   until a QEMU model and DT loading path are integrated.
10. PM hooks currently update software status only; register retention and
    reset behavior across suspend/resume remain unspecified implementation
    work.

These are deliberate follow-up implementation items, not failures of the
specification. The specification is ready to guide the QEMU model and the
subsequent driver/DT alignment work.
