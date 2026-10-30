#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
TMP="${TMPDIR:-/tmp}/syc-default-recognizers"
mkdir -p "$TMP"

compile_case() {
  local name="$1" source="$2"
  shift 2
  env "$@" "$COMPILER" -S -O1 -o "$TMP/$name.s" "$ROOT/tests/riscv/$source" \
    2>"$TMP/$name.log"
}

link_run() {
  local name="$1"
  riscv64-linux-gnu-gcc -march=rv64gc -mabi=lp64d -mcmodel=medany \
    -static -o "$TMP/$name.elf" "$TMP/$name.s" "$RUNTIME" -lm
  qemu-riscv64 "$TMP/$name.elf" >"$TMP/$name.out"
}

compile_case bit.default bit_accumulator_positive.sy
compile_case bit.skip bit_accumulator_positive.sy \
  SYC_SKIP_COREIR_BIT_ACCUMULATOR=1
cmp "$TMP/bit.default.s" "$TMP/bit.skip.s" && exit 1 || true
BIT_PATTERN_TRACE=1 compile_case bit.trace bit_accumulator_positive.sy
grep -F '[bit-pattern] accept-helper bit_and_renamed and' "$TMP/bit.trace.log"
link_run bit.default; link_run bit.skip
test "$(tr -d '\r\n' <"$TMP/bit.default.out")" = 9
diff -u "$TMP/bit.default.out" "$TMP/bit.skip.out"

compile_case bit_effect.default bit_accumulator_side_effect_negative.sy
compile_case bit_effect.skip bit_accumulator_side_effect_negative.sy \
  SYC_SKIP_COREIR_BIT_ACCUMULATOR=1
cmp "$TMP/bit_effect.default.s" "$TMP/bit_effect.skip.s"
BIT_PATTERN_TRACE=1 compile_case bit_effect.trace \
  bit_accumulator_side_effect_negative.sy
if grep -F 'accept-helper bit_and_effect' "$TMP/bit_effect.trace.log"; then
  exit 1
fi
link_run bit_effect.default; link_run bit_effect.skip
diff -u "$TMP/bit_effect.default.out" "$TMP/bit_effect.skip.out"

compile_case modmul.default recursive_modmul_positive.sy
compile_case modmul.skip recursive_modmul_positive.sy \
  SYC_SKIP_COREIR_RECURSIVE_MODMUL=1
cmp "$TMP/modmul.default.s" "$TMP/modmul.skip.s" && exit 1 || true
BIT_PATTERN_TRACE=1 compile_case modmul.trace recursive_modmul_positive.sy
grep -F \
  '[bit-pattern] accept-modmul-fastpath multiply_mod_renamed recursive-halving' \
  "$TMP/modmul.trace.log"
link_run modmul.default; link_run modmul.skip
diff -u "$TMP/modmul.default.out" "$TMP/modmul.skip.out"

compile_case modmul_recip.default recursive_modmul_reciprocal_edges.sy
compile_case modmul_recip.skip recursive_modmul_reciprocal_edges.sy \
  SYC_SKIP_COREIR_RECURSIVE_MODMUL=1
grep -F 'modmul.recip.quotient' "$TMP/modmul_recip.default.s"
if grep -Fq 'modmul.recip.quotient' "$TMP/modmul_recip.skip.s"; then
  exit 1
fi
link_run modmul_recip.default; link_run modmul_recip.skip
diff -u "$TMP/modmul_recip.default.out" "$TMP/modmul_recip.skip.out"

compile_case modmul_effect.default recursive_modmul_side_effect_negative.sy
compile_case modmul_effect.skip recursive_modmul_side_effect_negative.sy \
  SYC_SKIP_COREIR_RECURSIVE_MODMUL=1
cmp "$TMP/modmul_effect.default.s" "$TMP/modmul_effect.skip.s"
BIT_PATTERN_TRACE=1 compile_case modmul_effect.trace \
  recursive_modmul_side_effect_negative.sy
if grep -F 'accept-modmul-fastpath multiply_mod_effect' \
  "$TMP/modmul_effect.trace.log"; then
  exit 1
fi
link_run modmul_effect.default; link_run modmul_effect.skip
diff -u "$TMP/modmul_effect.default.out" "$TMP/modmul_effect.skip.out"

compile_case linear.default linear_recurrence_positive.sy
compile_case linear.skip linear_recurrence_positive.sy \
  SYC_SKIP_COREIR_LINEAR_RECURRENCE=1
cmp "$TMP/linear.default.s" "$TMP/linear.skip.s" && exit 1 || true
SYC_TRACE_COREIR_LINEAR_RECURRENCE=1 compile_case linear.trace \
  linear_recurrence_positive.sy
grep -F 'func=recurrence_renamed' "$TMP/linear.trace.log"
link_run linear.default; link_run linear.skip
diff -u "$TMP/linear.default.out" "$TMP/linear.skip.out"

compile_case linear_negative.default linear_recurrence_non_linear_negative.sy
compile_case linear_negative.skip linear_recurrence_non_linear_negative.sy \
  SYC_SKIP_COREIR_LINEAR_RECURRENCE=1
cmp "$TMP/linear_negative.default.s" "$TMP/linear_negative.skip.s"
SYC_TRACE_COREIR_LINEAR_RECURRENCE=1 compile_case linear_negative.trace \
  linear_recurrence_non_linear_negative.sy
if grep -F 'func=recurrence_non_linear' "$TMP/linear_negative.trace.log"; then
  exit 1
fi
link_run linear_negative.default; link_run linear_negative.skip
diff -u "$TMP/linear_negative.default.out" \
  "$TMP/linear_negative.skip.out"

# 与内部 helper 同名的用户函数仍按普通调用处理，不能由名称改变语义。
compile_case helper_collision.default internal_helper_name_collision_negative.sy
compile_case helper_collision.skip internal_helper_name_collision_negative.sy \
  SYC_SKIP_COREIR_RECURSIVE_MODMUL=1
link_run helper_collision.default; link_run helper_collision.skip
printf '10\n' >"$TMP/helper_collision.expected"
diff -u "$TMP/helper_collision.expected" "$TMP/helper_collision.default.out"
diff -u "$TMP/helper_collision.default.out" "$TMP/helper_collision.skip.out"

compile_case scalar_dormant.default loop_scalar_accumulator_negative.sy
compile_case scalar_dormant.skip loop_scalar_accumulator_negative.sy \
  SYC_SKIP_HIR_LOOP_SCALAR_ACCUMULATOR=1
cmp "$TMP/scalar_dormant.default.s" "$TMP/scalar_dormant.skip.s"
# strict 构建不包含未形成正向证据的实现，研究开关不得改变汇编。
compile_case scalar_dormant.strict_enable loop_scalar_accumulator_negative.sy \
  SYC_ENABLE_HIR_LOOP_SCALAR_ACCUMULATOR=1
cmp "$TMP/scalar_dormant.default.s" \
  "$TMP/scalar_dormant.strict_enable.s"
SYC_TRACE_LOOP_SCALAR_ACCUMULATOR=1 compile_case scalar_dormant.trace \
  loop_scalar_accumulator_negative.sy
if grep -F '[loop-scalar-acc] apply' "$TMP/scalar_dormant.trace.log"; then
  exit 1
fi
link_run scalar_dormant.default; link_run scalar_dormant.skip
diff -u "$TMP/scalar_dormant.default.out" \
  "$TMP/scalar_dormant.skip.out"

echo 'riscv default recognizers: PASS'
