#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
CLI_BIN="$ROOT_DIR/userspace/edu-cli/edu-cli"

if [ ! -x "$CLI_BIN" ]; then
    make -C "$ROOT_DIR/userspace/edu-cli" >/dev/null
fi

"$CLI_BIN" --help >/dev/null

echo "edu_cli_smoke: PASS"
