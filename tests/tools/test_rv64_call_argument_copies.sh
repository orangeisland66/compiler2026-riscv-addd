#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
COMPILER=${COMPILER:-"$ROOT/build-plan/compiler"}
RUNTIME=${RUNTIME:-"$ROOT/build-plan/libsysy_runtime.a"}
GCC=${GCC:-riscv64-linux-gnu-gcc}
QEMU=${QEMU:-qemu-riscv64-static}
WORK=$(mktemp -d /tmp/syc-call-args.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

has_immediate_argument_stack_roundtrip() {
  awk '
    /^[[:space:]]*sd[[:space:]]/ {
      previous_store = 1
      previous_offset = $3
      next
    }
    /^[[:space:]]*[a-z]/ {
      if (previous_store && $1 == "ld" && $2 ~ /^a[0-7],$/ &&
          $3 == previous_offset) {
        found = 1
        exit
      }
      previous_store = 0
    }
    END { exit found ? 0 : 1 }
  ' "$1"
}

compile_case() {
  local source=$1
  local output=$2
  "$COMPILER" -S -O1 -o "$output" "$source"
}

positive="$WORK/positive.s"
stack_negative="$WORK/stack-negative.s"
compile_case "$ROOT/tests/riscv/call_argument_parallel_copy_positive.sy" \
  "$positive"
compile_case "$ROOT/tests/riscv/call_argument_stack_overflow_negative.sy" \
  "$stack_negative"

if ! grep -Eq '^[[:space:]]*call[[:space:]]+putarray([[:space:]]|$)' "$positive"; then
  echo "positive call-argument fixture lost the putarray call" >&2
  exit 1
fi
if has_immediate_argument_stack_roundtrip "$positive" ||
   has_immediate_argument_stack_roundtrip "$stack_negative"; then
  echo "register argument was stored to a temporary stack slot and reloaded" >&2
  exit 1
fi

if ! grep -Eq '^[[:space:]]*call[[:space:]]+keep_ten([[:space:]]|$)' \
     "$stack_negative"; then
  echo "stack-argument fixture lost the ten-argument call" >&2
  exit 1
fi
if ! grep -Eq '^[[:space:]]*sd[[:space:]]+[^,]+,[[:space:]]*0\(sp\)' \
     "$stack_negative" ||
   ! grep -Eq '^[[:space:]]*sd[[:space:]]+[^,]+,[[:space:]]*8\(sp\)' \
     "$stack_negative"; then
  echo "true overflow stack arguments were not preserved at 0(sp)/8(sp)" >&2
  exit 1
fi

if [[ -f "$RUNTIME" ]] && command -v "$GCC" >/dev/null &&
   command -v "$QEMU" >/dev/null; then
  arch=(-march=rv64gc -mabi=lp64d -mcmodel=medany -static)
  "$GCC" "${arch[@]}" -o "$WORK/positive.elf" "$positive" "$RUNTIME" -lm
  "$GCC" "${arch[@]}" -o "$WORK/stack-negative.elf" "$stack_negative" \
    "$RUNTIME" -lm
  "$QEMU" "$WORK/positive.elf" >"$WORK/positive.out"
  "$QEMU" "$WORK/stack-negative.elf" >"$WORK/stack-negative.out"
  printf '2: 7 9\n' >"$WORK/positive.expected"
  printf '47\n' >"$WORK/stack-negative.expected"
  cmp "$WORK/positive.expected" "$WORK/positive.out"
  cmp "$WORK/stack-negative.expected" "$WORK/stack-negative.out"
fi

echo "rv64 call argument parallel-copy tests: PASS"
