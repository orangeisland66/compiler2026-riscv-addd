#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
PERFORMANCE_ROOT="${PERFORMANCE_ROOT:?set PERFORMANCE_ROOT to the public performance cases}"
TMP="${TMPDIR:-/tmp}/syc-remaining-pass-semantics"
mkdir -p "$TMP"

compile_pair_path() {
  local name="$1"
  local source="$2"
  local skip_env="$3"
  local enable_env="${4:-SYC_TEST_NOOP=1}"
  # Pass every semantic switch explicitly.  Prefix assignments on shell
  # functions are dynamically scoped in bash and can otherwise leak into a
  # nested helper, making an isolation test depend on invocation order.
  env SYC_RESEARCH=1 "$enable_env" \
    "$COMPILER" -S -O1 -o "$TMP/$name.default.s" "$source"
  env SYC_RESEARCH=1 "$enable_env" "$skip_env" \
    "$COMPILER" -S -O1 -o "$TMP/$name.skip.s" "$source"
}

compile_pair() {
  compile_pair_path "$1" "$ROOT/tests/riscv/$1.sy" "$2"
}

expect_changed() {
  local name="$1"
  if cmp -s "$TMP/$name.default.s" "$TMP/$name.skip.s"; then
    echo "正例未触发：$name" >&2
    exit 1
  fi
}

expect_unchanged() {
  local name="$1"
  if ! cmp -s "$TMP/$name.default.s" "$TMP/$name.skip.s"; then
    echo "负例意外触发：$name" >&2
    exit 1
  fi
}

run_pair() {
  local name="$1"
  local input="${2:-}"
  for mode in default skip; do
    riscv64-linux-gnu-gcc -static -o "$TMP/$name.$mode.elf" \
      "$TMP/$name.$mode.s" "$RUNTIME" -lm
    if [[ -n "$input" ]]; then
      timeout 60 qemu-riscv64 "$TMP/$name.$mode.elf" \
        <"$input" >"$TMP/$name.$mode.out" 2>/dev/null
    else
      timeout 60 qemu-riscv64 "$TMP/$name.$mode.elf" \
        >"$TMP/$name.$mode.out" 2>/dev/null
    fi
  done
  diff -u "$TMP/$name.skip.out" "$TMP/$name.default.out"
}

# 模递推：固定加法正例触发，旧值参与乘法的近似形状拒绝。
printf '8 5\n' >"$TMP/modular_scalar_recurrence_positive.in"
compile_pair modular_scalar_recurrence_positive \
  SYC_SKIP_COREIR_MOD_SCALAR_RECURRENCE=1
expect_changed modular_scalar_recurrence_positive
run_pair modular_scalar_recurrence_positive \
  "$TMP/modular_scalar_recurrence_positive.in"
MOD_SCALAR_RECURRENCE_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/modular_scalar_recurrence_positive.trace.s" \
  "$ROOT/tests/riscv/modular_scalar_recurrence_positive.sy" \
  2>"$TMP/modular_scalar_recurrence_positive.trace.log"
grep -F '[mod-scalar-rec] fastpath' \
  "$TMP/modular_scalar_recurrence_positive.trace.log"

printf '5 1\n' >"$TMP/modular_scalar_recurrence_nonlinear_negative.in"
compile_pair modular_scalar_recurrence_nonlinear_negative \
  SYC_SKIP_COREIR_MOD_SCALAR_RECURRENCE=1
expect_unchanged modular_scalar_recurrence_nonlinear_negative
run_pair modular_scalar_recurrence_nonlinear_negative \
  "$TMP/modular_scalar_recurrence_nonlinear_negative.in"

# 函数特化：递归闭包原样传递控制参数时可特化，递归中改写该参数时拒绝。
printf '4\n' >"$TMP/function_specialization_recursive_positive.in"
compile_pair function_specialization_recursive_positive \
  SYC_SKIP_COREIR_FUNCTION_SPECIALIZATION=1
expect_changed function_specialization_recursive_positive
run_pair function_specialization_recursive_positive \
  "$TMP/function_specialization_recursive_positive.in"

printf '4\n' >"$TMP/function_specialization_recursive_mutation_negative.in"
compile_pair function_specialization_recursive_mutation_negative \
  SYC_SKIP_COREIR_FUNCTION_SPECIALIZATION=1
expect_unchanged function_specialization_recursive_mutation_negative
run_pair function_specialization_recursive_mutation_negative \
  "$TMP/function_specialization_recursive_mutation_negative.in"

# 重复覆盖循环：公开正例可折叠；外层 IV 参与可观察结果时拒绝。
compile_pair_path overwrite_repeat_positive \
  "$PERFORMANCE_ROOT/h-10-01.sy" \
  SYC_SKIP_COREIR_OVERWRITE_LOOP_COLLAPSE=1
expect_changed overwrite_repeat_positive
run_pair overwrite_repeat_positive "$PERFORMANCE_ROOT/h-10-01.in"
OVERWRITE_LOOP_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/overwrite_repeat_positive.trace.s" \
  "$PERFORMANCE_ROOT/h-10-01.sy" \
  2>"$TMP/overwrite_repeat_positive.trace.log"
grep -F '[overwrite-loop] apply func=' \
  "$TMP/overwrite_repeat_positive.trace.log"

compile_pair overwrite_repeat_iv_dependent_negative \
  SYC_SKIP_COREIR_OVERWRITE_LOOP_COLLAPSE=1
expect_unchanged overwrite_repeat_iv_dependent_negative
run_pair overwrite_repeat_iv_dependent_negative

# 除数特化：循环不变除数生成守卫版本；循环内更新除数时拒绝。
printf '3\n4 6 -7 8 9\n' >"$TMP/runtime_invariant_divisor.in"
compile_pair runtime_invariant_divisor \
  SYC_SKIP_COREIR_DIVISOR_SPECIALIZATION=1
expect_changed runtime_invariant_divisor
run_pair runtime_invariant_divisor "$TMP/runtime_invariant_divisor.in"
SYC_TRACE_COREIR_DIVISOR_SPECIALIZATION=1 "$COMPILER" -S -O1 \
  -o "$TMP/runtime_invariant_divisor.trace.s" \
  "$ROOT/tests/riscv/runtime_invariant_divisor.sy" \
  2>"$TMP/runtime_invariant_divisor.trace.log"
grep -F '[divisor-specialize] apply' \
  "$TMP/runtime_invariant_divisor.trace.log"

printf '2\n4 6 -7 8 9\n' >"$TMP/divisor_variant_negative.in"
compile_pair divisor_variant_negative \
  SYC_SKIP_COREIR_DIVISOR_SPECIALIZATION=1
expect_unchanged divisor_variant_negative
run_pair divisor_variant_negative "$TMP/divisor_variant_negative.in"

# row-locality：闭合矩阵规约触发；存储值额外依赖外层 IV 时拒绝。
compile_pair_path row_locality_k_block_positive \
  "$ROOT/tests/riscv/row_locality_k_block_positive.sy" \
  SYC_SKIP_COREIR_ROW_LOCALITY=1 SYC_ENABLE_COREIR_ROW_LOCALITY=1
expect_unchanged row_locality_k_block_positive
run_pair row_locality_k_block_positive
SYC_ENABLE_COREIR_ROW_LOCALITY=1 ROW_LOCALITY_TRACE=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1 "$COMPILER" -S -O1 \
  -o "$TMP/row_locality_k_block_positive.trace.s" \
  "$ROOT/tests/riscv/row_locality_k_block_positive.sy" \
  2>"$TMP/row_locality_k_block_positive.trace.log"
grep -F 'pass=row-locality reason=candidate-ir-verifier-failed' \
  "$TMP/row_locality_k_block_positive.trace.log"

compile_pair_path row_locality_outer_value_negative \
  "$ROOT/tests/riscv/row_locality_outer_value_negative.sy" \
  SYC_SKIP_COREIR_ROW_LOCALITY=1 SYC_ENABLE_COREIR_ROW_LOCALITY=1
expect_unchanged row_locality_outer_value_negative
run_pair row_locality_outer_value_negative

# 行广播融合：完整 observer/effect/live-out 闭包默认事务式提交；列号加权
# checksum 不能由未加权标量摘要替换，必须拒绝。
printf '0\n' >"$TMP/row_broadcast.in"
compile_pair_path row_broadcast_fusion_positive \
  "$ROOT/tests/riscv/row_broadcast_fusion_positive.sy" \
  SYC_SKIP_COREIR_ROW_BROADCAST_FUSION=1
expect_changed row_broadcast_fusion_positive
run_pair row_broadcast_fusion_positive "$TMP/row_broadcast.in"
SYC_TRACE_STAGE_FUSION=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1 "$COMPILER" -S -O1 \
  -o "$TMP/row_broadcast_fusion_positive.trace.s" \
  "$ROOT/tests/riscv/row_broadcast_fusion_positive.sy" \
  2>"$TMP/row_broadcast_fusion_positive.trace.log"
grep -F 'pass=row-broadcast-transpose-checksum-fusion reason=accepted' \
  "$TMP/row_broadcast_fusion_positive.trace.log"

# 对称 producer 正例同时证明 source/transpose 的四种仿射访问角色；该证明
# 不依赖 transpose-view forwarding 是否先改写 load。
compile_pair_path row_broadcast_symmetric_producer_positive \
  "$PERFORMANCE_ROOT/matmul1.sy" \
  SYC_SKIP_COREIR_ROW_BROADCAST_FUSION=1
expect_changed row_broadcast_symmetric_producer_positive
run_pair row_broadcast_symmetric_producer_positive \
  "$PERFORMANCE_ROOT/matmul1.in"
SYC_TRACE_STAGE_FUSION=1 SYC_TRACE_TRANSACTIONAL_PASSES=1 \
  "$COMPILER" -S -O1 \
  -o "$TMP/row_broadcast_symmetric_producer_positive.trace.s" \
  "$PERFORMANCE_ROOT/matmul1.sy" \
  2>"$TMP/row_broadcast_symmetric_producer_positive.trace.log"
grep -F 'materialize=symmetric-producer-rowmin' \
  "$TMP/row_broadcast_symmetric_producer_positive.trace.log"
grep -F 'pass=row-broadcast-transpose-checksum-fusion reason=accepted' \
  "$TMP/row_broadcast_symmetric_producer_positive.trace.log"

compile_pair_path row_broadcast_weighted_negative \
  "$ROOT/tests/riscv/row_broadcast_weighted_negative.sy" \
  SYC_SKIP_COREIR_ROW_BROADCAST_FUSION=1
expect_unchanged row_broadcast_weighted_negative
run_pair row_broadcast_weighted_negative "$TMP/row_broadcast.in"

# stencil：公开正例必须生成 interior 快路径；带额外可观察状态的模板拒绝。
compile_pair_path stencil_interior_positive \
  "$PERFORMANCE_ROOT/conv2d-1.sy" \
  SYC_SKIP_COREIR_STENCIL_INTERIOR=1 SYC_ENABLE_COREIR_STENCIL_INTERIOR=1
expect_unchanged stencil_interior_positive
run_pair stencil_interior_positive "$PERFORMANCE_ROOT/conv2d-1.in"
SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 SYC_TRACE_COREIR_STENCIL=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1 "$COMPILER" -S -O1 \
  -o "$TMP/stencil_interior_positive.trace.s" \
  "$PERFORMANCE_ROOT/conv2d-1.sy" \
  2>"$TMP/stencil_interior_positive.trace.log"
grep -F 'pass=stencil-interior reason=candidate-transform-rejected' \
  "$TMP/stencil_interior_positive.trace.log"

SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 SYC_TRACE_COREIR_STENCIL=1 "$COMPILER" -S -O1 \
  -o "$TMP/stencil_side_effect_negative.default.s" \
  "$ROOT/tests/riscv/stencil_side_effect_negative.sy" \
  2>"$TMP/stencil_side_effect_negative.trace.log"
SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 SYC_SKIP_COREIR_STENCIL_INTERIOR=1 "$COMPILER" -S -O1 \
  -o "$TMP/stencil_side_effect_negative.skip.s" \
  "$ROOT/tests/riscv/stencil_side_effect_negative.sy"
if grep -Fq '[stencil-interior] func=' \
  "$TMP/stencil_side_effect_negative.trace.log"; then
  echo 'stencil 负例意外物化快路径' >&2
  exit 1
fi
grep -F 'reason=load-coeff' "$TMP/stencil_side_effect_negative.trace.log"
printf '4\n' >"$TMP/stencil_side_effect_negative.in"
run_pair stencil_side_effect_negative "$TMP/stencil_side_effect_negative.in"

echo 'remaining pass semantics: PASS'
