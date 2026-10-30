#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
TMP="${TMPDIR:-/tmp}/syc-loop-nest-transforms"
mkdir -p "$TMP"

compile_pair() {
  local source="$1"
  local skip_env="$2"
  "$COMPILER" -S -O1 -o "$TMP/$source.default.s" \
    "$ROOT/tests/riscv/$source.sy" 2>"$TMP/$source.default.log"
  env "$skip_env" "$COMPILER" -S -O1 -o "$TMP/$source.skip.s" \
    "$ROOT/tests/riscv/$source.sy" 2>"$TMP/$source.skip.log"
}

expect_unchanged() {
  local source="$1"
  if ! cmp -s "$TMP/$source.default.s" "$TMP/$source.skip.s"; then
    echo "unsafe loop transform changed assembly: $source" >&2
    exit 1
  fi
}

run_pair() {
  local source="$1"
  for mode in default skip; do
    riscv64-linux-gnu-gcc -static -o "$TMP/$source.$mode.elf" \
      "$TMP/$source.$mode.s" "$RUNTIME" -lm
    timeout 20 qemu-riscv64 "$TMP/$source.$mode.elf" \
      >"$TMP/$source.$mode.out" 2>/dev/null
  done
  diff -u "$TMP/$source.skip.out" "$TMP/$source.default.out"
}

source=loop_fusion_read_only_positive
compile_pair "$source" SYC_SKIP_HIR_LOOP_FUSION=1
expect_unchanged "$source"
# strict 构建不包含尚未闭环的 loop-fusion 实现，研究开关必须无效。
SYC_ENABLE_HIR_LOOP_FUSION=1 "$COMPILER" -S -O1 \
  -o "$TMP/$source.strict-enable.s" "$ROOT/tests/riscv/$source.sy"
cmp "$TMP/$source.default.s" "$TMP/$source.strict-enable.s"
SYC_TRACE_HIR_LOOP_FUSION=1 "$COMPILER" -S -O1 \
  -o "$TMP/$source.trace.s" "$ROOT/tests/riscv/$source.sy" \
  2>"$TMP/$source.trace.log"
if grep -Fq '[loop-fusion]' "$TMP/$source.trace.log"; then
  echo "sibling loop fusion is unexpectedly reachable" >&2
  exit 1
fi
SYC_TRACE_HIR_LOOP_FUSION=1 SYC_SKIP_HIR_LOOP_FUSION=1 "$COMPILER" \
  -S -O1 -o "$TMP/$source.skip-trace.s" \
  "$ROOT/tests/riscv/$source.sy" 2>"$TMP/$source.skip-trace.log"
if grep -Fq '[loop-fusion]' "$TMP/$source.skip-trace.log"; then
  echo "loop fusion skip did not suppress its trace" >&2
  exit 1
fi
run_pair "$source"

source=loop_fusion_dependence_negative
compile_pair "$source" SYC_SKIP_HIR_LOOP_FUSION=1
expect_unchanged "$source"
run_pair "$source"

source=loop_interchange_rectangular_negative
compile_pair "$source" SYC_SKIP_HIR_LOOP_INTERCHANGE=1
expect_unchanged "$source"
# strict 构建同样忽略 loop-interchange 的研究开关。
SYC_ENABLE_HIR_LOOP_INTERCHANGE=1 "$COMPILER" -S -O1 \
  -o "$TMP/$source.strict-enable.s" "$ROOT/tests/riscv/$source.sy"
cmp "$TMP/$source.default.s" "$TMP/$source.strict-enable.s"
SYC_TRACE_HIR_LOOP_INTERCHANGE=1 "$COMPILER" -S -O1 \
  -o "$TMP/$source.trace.s" "$ROOT/tests/riscv/$source.sy" \
  2>"$TMP/$source.trace.log"
if grep -Fq '[loop-interchange]' "$TMP/$source.trace.log"; then
  echo "rectangular loop interchange was accepted" >&2
  exit 1
fi
run_pair "$source"

source=loop_interchange_side_effect_negative
compile_pair "$source" SYC_SKIP_HIR_LOOP_INTERCHANGE=1
expect_unchanged "$source"

# continue 边界变换尚未形成通用正向证据，strict 构建不应响应研究开关。
source=loop_fusion_read_only_positive
"$COMPILER" -S -O1 -o "$TMP/$source.continue-default.s" \
  "$ROOT/tests/riscv/$source.sy"
SYC_ENABLE_HIR_LOOP_CONTINUE_BOUND=1 "$COMPILER" -S -O1 \
  -o "$TMP/$source.continue-enable.s" "$ROOT/tests/riscv/$source.sy"
cmp "$TMP/$source.continue-default.s" "$TMP/$source.continue-enable.s"
run_pair "$source"

echo "loop nest transforms: PASS"
