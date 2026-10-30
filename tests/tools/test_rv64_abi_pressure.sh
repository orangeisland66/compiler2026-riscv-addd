#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
TMP="${TMPDIR:-/tmp}/syc-rv64-abi-pressure"
mkdir -p "$TMP"

for mode in o0 o1; do
  args=(-S -o "$TMP/$mode.s" "$ROOT/tests/riscv/rv64_abi_pressure.sy")
  if [[ "$mode" == o1 ]]; then
    args=(-S -O1 -o "$TMP/$mode.s" "$ROOT/tests/riscv/rv64_abi_pressure.sy")
  fi
  "$COMPILER" "${args[@]}"
  riscv64-linux-gnu-gcc -static -mcmodel=medany -march=rv64gc \
    -o "$TMP/$mode.elf" "$TMP/$mode.s" "$RUNTIME" -lm
  timeout 20 qemu-riscv64 "$TMP/$mode.elf" >"$TMP/$mode.out"
  diff -u "$ROOT/tests/riscv/rv64_abi_pressure.out" "$TMP/$mode.out"
done

# 5200-byte local object forces offsets beyond signed 12-bit immediates.
grep -Eq 'li[[:space:]]+[^,]+, *-?[2-9][0-9]{3,}|add[[:space:]]' "$TMP/o0.s"
echo "rv64 ABI/RA/frame pressure: PASS"
