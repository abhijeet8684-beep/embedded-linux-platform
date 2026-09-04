# edu-cli

`edu-cli` is a small userspace diagnostic tool used to interact with `/dev/edu0`.

## Commands

- `reset`: resets the device state.
- `status`: reads the device status word.
- `write <value>`: writes a word into the simulated FIFO.
- `read`: reads one value from the FIFO.
- `fault <code>`: injects a simulated fault condition.
- `help`: prints usage.

## Notes

The CLI is designed to validate the kernel-side contract and is intentionally lightweight. It is not a full production application, but it demonstrates the expected userspace workflow for a production embedded device interface.
