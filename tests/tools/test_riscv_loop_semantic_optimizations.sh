#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
PERFORMANCE_ROOT="${PERFORMANCE_ROOT:-$ROOT/../../../workbench/riscv/performance}"
TMP="${TMPDIR:-/tmp}/syc-loop-semantic-optimizations"
mkdir -p "$TMP"

compile_pair_path() {
  local source="$1"
  local source_path="$2"
  local skip_env="$3"
  "$COMPILER" -S -O1 -o "$TMP/$source.default.s" \
    "$source_path"
  env "$skip_env" "$COMPILER" -S -O1 -o "$TMP/$source.skip.s" \
    "$source_path"
}

compile_pair() {
  local source="$1"
  local skip_env="$2"
  compile_pair_path "$source" "$ROOT/tests/riscv/$source.sy" "$skip_env"
}

expect_changed() {
  local source="$1"
  if cmp -s "$TMP/$source.default.s" "$TMP/$source.skip.s"; then
    echo "expected optimization did not trigger: $source" >&2
    exit 1
  fi
}

expect_unchanged() {
  local source="$1"
  if ! cmp -s "$TMP/$source.default.s" "$TMP/$source.skip.s"; then
    echo "unsafe optimization triggered: $source" >&2
    exit 1
  fi
}

run_pair() {
  local source="$1"
  local input="${2:-}"
  for mode in default skip; do
    riscv64-linux-gnu-gcc -static -o "$TMP/$source.$mode.elf" \
      "$TMP/$source.$mode.s" "$RUNTIME" -lm
    if [ -n "$input" ]; then
      timeout 20 qemu-riscv64 "$TMP/$source.$mode.elf" \
        <"$input" >"$TMP/$source.$mode.out" 2>/dev/null
    else
      timeout 20 qemu-riscv64 "$TMP/$source.$mode.elf" \
        >"$TMP/$source.$mode.out" 2>/dev/null
    fi
  done
  diff -u "$TMP/$source.skip.out" "$TMP/$source.default.out"
}

if [[ "${SYC_TEST_DORMANT_STATE_TRANSFORMS:-0}" == 1 ]]; then
  compile_pair pure_call_total_positive SYC_SKIP_HIR_PURE_CALL_LAST_VALUE=1
  expect_changed pure_call_total_positive
  run_pair pure_call_total_positive

  for source in pure_call_nontermination_negative pure_call_divisor_negative \
                pure_call_inclusive_negative; do
    compile_pair "$source" SYC_SKIP_HIR_PURE_CALL_LAST_VALUE=1
    expect_unchanged "$source"
  done
fi

compile_pair reduction_split_address_positive \
  SYC_SKIP_COREIR_REDUCTION_SPLIT=1
# 后续 canonicalization 可能把 default/skip 收敛成同一机器产物；用 pass 自身的
# accept 事实验证真触发，再用两侧动态输出验证语义。
SYC_TRACE_REDUCTION_SPLIT=1 "$COMPILER" -S -O1 \
  -o "$TMP/reduction_split_address_positive.default.trace.s" \
  "$ROOT/tests/riscv/reduction_split_address_positive.sy" \
  2>"$TMP/reduction_split_address_positive.default.log"
SYC_TRACE_REDUCTION_SPLIT=1 SYC_SKIP_COREIR_REDUCTION_SPLIT=1 "$COMPILER" \
  -S -O1 -o "$TMP/reduction_split_address_positive.skip.trace.s" \
  "$ROOT/tests/riscv/reduction_split_address_positive.sy" \
  2>"$TMP/reduction_split_address_positive.skip.log"
grep -F '[reduction-split] accept' \
  "$TMP/reduction_split_address_positive.default.log"
if grep -Fq '[reduction-split] accept' \
  "$TMP/reduction_split_address_positive.skip.log"; then
  echo 'reduction split skip did not suppress the pass' >&2
  exit 1
fi
run_pair reduction_split_address_positive

compile_pair reduction_split_odd_bound_negative \
  SYC_SKIP_COREIR_REDUCTION_SPLIT=1
expect_unchanged reduction_split_odd_bound_negative
run_pair reduction_split_odd_bound_negative

compile_pair reduction_split_j_side_effect_negative \
  SYC_SKIP_COREIR_REDUCTION_SPLIT=1
expect_unchanged reduction_split_j_side_effect_negative
run_pair reduction_split_j_side_effect_negative

# 重复规约正例来自公开性能根目录；负例让旧累加器参与乘法，验证 matcher
# 只接受 F(acc)=acc+delta 的平移转移。
compile_pair_path repeated_reduction_positive \
  "$PERFORMANCE_ROOT/many_mat_cal-1.sy" \
  SYC_SKIP_COREIR_REPEATED_REDUCTION=1
expect_changed repeated_reduction_positive
run_pair repeated_reduction_positive \
  "$PERFORMANCE_ROOT/many_mat_cal-1.in"

compile_pair repeated_reduction_acc_dependent_negative \
  SYC_SKIP_COREIR_REPEATED_REDUCTION=1
expect_unchanged repeated_reduction_acc_dependent_negative
printf '1 2 3 4\n' \
  >"$TMP/repeated_reduction_acc_dependent_negative.in"
run_pair repeated_reduction_acc_dependent_negative \
  "$TMP/repeated_reduction_acc_dependent_negative.in"
REPEATED_REDUCTION_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/repeated_reduction_acc_dependent_negative.trace.s" \
  "$ROOT/tests/riscv/repeated_reduction_acc_dependent_negative.sy" \
  2>"$TMP/repeated_reduction_acc_dependent_negative.trace.log"
grep -F 'reason=non-translation-accumulator' \
  "$TMP/repeated_reduction_acc_dependent_negative.trace.log"

compile_pair row_locality_k_block_positive \
  SYC_SKIP_COREIR_ROW_LOCALITY_K_BLOCKING=1
expect_changed row_locality_k_block_positive
run_pair row_locality_k_block_positive
ROW_LOCALITY_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/row_locality_k_block_positive.trace.s" \
  "$ROOT/tests/riscv/row_locality_k_block_positive.sy" \
  2>"$TMP/row_locality_k_block_positive.trace.log"
grep -F '[row-locality] func=' \
  "$TMP/row_locality_k_block_positive.trace.log"

compile_pair loop_blocking_irregular_child_negative \
  SYC_SKIP_HIR_LOOP_BLOCKING=1
expect_unchanged loop_blocking_irregular_child_negative
printf '4\n1 2 3 4\n' \
  >"$TMP/loop_blocking_irregular_child_negative.in"
run_pair loop_blocking_irregular_child_negative \
  "$TMP/loop_blocking_irregular_child_negative.in"
LOOP_BLOCKING_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/loop_blocking_irregular_child_negative.trace.s" \
  "$ROOT/tests/riscv/loop_blocking_irregular_child_negative.sy" \
  2>"$TMP/loop_blocking_irregular_child_negative.trace.log"
grep -F 'reason=nested-irregular-loop' \
  "$TMP/loop_blocking_irregular_child_negative.trace.log"

compile_pair array_recurrence_control_positive \
  SYC_SKIP_HIR_ARRAY_RECURRENCE=1
expect_changed array_recurrence_control_positive
run_pair array_recurrence_control_positive
ARRAY_RECURRENCE_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/array_recurrence_control_positive.default.trace.s" \
  "$ROOT/tests/riscv/array_recurrence_control_positive.sy" \
  2>"$TMP/array_recurrence_control_positive.default.log"
ARRAY_RECURRENCE_TRACE=1 SYC_SKIP_HIR_ARRAY_RECURRENCE=1 "$COMPILER" \
  -S -O1 -o "$TMP/array_recurrence_control_positive.skip.trace.s" \
  "$ROOT/tests/riscv/array_recurrence_control_positive.sy" \
  2>"$TMP/array_recurrence_control_positive.skip.log"
grep -F '[array_recurrence]' \
  "$TMP/array_recurrence_control_positive.default.log"
if grep -Fq '[array_recurrence]' \
  "$TMP/array_recurrence_control_positive.skip.log"; then
  echo "late array recurrence skip did not suppress its trace" >&2
  exit 1
fi

compile_pair array_recurrence_non_affine_negative \
  SYC_SKIP_HIR_ARRAY_RECURRENCE=1
expect_unchanged array_recurrence_non_affine_negative
run_pair array_recurrence_non_affine_negative
ARRAY_RECURRENCE_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/array_recurrence_non_affine_negative.default.trace.s" \
  "$ROOT/tests/riscv/array_recurrence_non_affine_negative.sy" \
  2>"$TMP/array_recurrence_non_affine_negative.default.log"
if grep -Eq '\[array_recurrence\].*cand=[1-9]' \
  "$TMP/array_recurrence_non_affine_negative.default.log"; then
  echo "non-affine array recurrence was accepted" >&2
  exit 1
fi

compile_pair transpose_view_forwarding_positive \
  SYC_SKIP_HIR_TRANSPOSE_FORWARDING=1
expect_unchanged transpose_view_forwarding_positive
run_pair transpose_view_forwarding_positive
TRANSPOSE_VIEW_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/transpose_view_forwarding_positive.trace.s" \
  "$ROOT/tests/riscv/transpose_view_forwarding_positive.sy" \
  2>"$TMP/transpose_view_forwarding_positive.trace.log"
if grep -Fq '[transpose-view]' \
  "$TMP/transpose_view_forwarding_positive.trace.log"; then
  echo 'dormant transpose forwarding unexpectedly ran' >&2
  exit 1
fi

compile_pair transpose_view_forwarding_small_negative \
  SYC_SKIP_HIR_TRANSPOSE_FORWARDING=1
expect_unchanged transpose_view_forwarding_small_negative
run_pair transpose_view_forwarding_small_negative

compile_pair mir_loop_address_singleton_positive \
  SYC_SKIP_MIR_LOOP_ADDRESS_REDUCTION=1
expect_changed mir_loop_address_singleton_positive
run_pair mir_loop_address_singleton_positive
SYC_TRACE_MIR_LOOP_ADDR=1 "$COMPILER" -S -O1 \
  -o "$TMP/mir_loop_address_singleton_positive.trace.s" \
  "$ROOT/tests/riscv/mir_loop_address_singleton_positive.sy" \
  2>"$TMP/mir_loop_address_singleton_positive.trace.log"
grep -E '\[mir-loop-(addr|symbol)\]' \
  "$TMP/mir_loop_address_singleton_positive.trace.log"

compile_pair mir_loop_address_call_negative \
  SYC_SKIP_MIR_LOOP_ADDRESS_REDUCTION=1
expect_unchanged mir_loop_address_call_negative
run_pair mir_loop_address_call_negative

compile_pair dead_global_array_positive \
  SYC_SKIP_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM=1
# 专用证明之后还可能执行通用的未观测全局清理，因此这里要求专用 trace
# 与运行输出一致；最终汇编删除可由专用 pass 或后续通用清理完成。
SYC_TRACE_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM=1 "$COMPILER" -S -O1 \
  -o "$TMP/dead_global_array_positive.default.trace.s" \
  "$ROOT/tests/riscv/dead_global_array_positive.sy" \
  2>"$TMP/dead_global_array_positive.default.log"
SYC_TRACE_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM=1 \
  SYC_SKIP_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM=1 "$COMPILER" -S -O1 \
  -o "$TMP/dead_global_array_positive.skip.trace.s" \
  "$ROOT/tests/riscv/dead_global_array_positive.sy" \
  2>"$TMP/dead_global_array_positive.skip.log"
grep -F '[dead-global-array-store-elim] phase=late' \
  "$TMP/dead_global_array_positive.default.log"
if grep -Fq '[dead-global-array-store-elim]' \
  "$TMP/dead_global_array_positive.skip.log"; then
  echo "dead-global skip did not suppress its late trace" >&2
  exit 1
fi
run_pair dead_global_array_positive

for source in dead_global_array_read_negative dead_global_array_escape_negative; do
  compile_pair "$source" SYC_SKIP_HIR_DEAD_GLOBAL_ARRAY_STORE_ELIM=1
  expect_unchanged "$source"
  run_pair "$source"
done

# 正常的闭区间下界与常量开区间上界会收缩为无符号比较；
# 开下界和动态上界继续保留原有比较组合。
"$COMPILER" -S -O1 -o "$TMP/nonnegative_range_compare.s" \
  "$ROOT/tests/riscv/nonnegative_range_compare.sy"
range_sltu_count=$(grep -Ec '^[[:space:]]+sltu[[:space:]]' \
  "$TMP/nonnegative_range_compare.s")
if [ "$range_sltu_count" -lt 2 ]; then
  echo "nonnegative range compare did not lower to sltu" >&2
  exit 1
fi
riscv64-linux-gnu-gcc -static -o "$TMP/nonnegative_range_compare.elf" \
  "$TMP/nonnegative_range_compare.s" "$RUNTIME" -lm
range_inputs=(
  $'-1 10\n'
  $'0 -1\n'
  $'0 1\n'
  $'16 17\n'
  $'17 17\n'
  $'22 30\n'
  $'23 30\n'
  $'2147483647 2147483647\n'
  $'-2147483648 2147483647\n'
)
range_outputs=(0000 1100 1101 1111 0100 0101 0001 0000 0000)
for index in "${!range_inputs[@]}"; do
  printf '%s' "${range_inputs[$index]}" | \
    qemu-riscv64 "$TMP/nonnegative_range_compare.elf" \
      >"$TMP/nonnegative_range_compare.$index.out" 2>/dev/null
  actual=$(tr -d ' \r\n' \
    <"$TMP/nonnegative_range_compare.$index.out")
  test "$actual" = "${range_outputs[$index]}"
done

# 无访存短递推采用三倍展开并批量处理两次尾迭代；缺少 streaming recurrence
# 证明的数组更新循环不得再通过旧 fallback 默认四倍展开。
for source in small_arithmetic_recurrence_unroll small_memory_update_unroll; do
  SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
    -o "$TMP/$source.s" "$ROOT/tests/riscv/$source.sy" \
    2>"$TMP/$source.log"
  riscv64-linux-gnu-gcc -static -o "$TMP/$source.elf" \
    "$TMP/$source.s" "$RUNTIME" -lm
done
grep -F 'phase=partial-pair-tail accept factor=3 cost=8' \
  "$TMP/small_arithmetic_recurrence_unroll.log"
if grep -Fq 'phase=partial accept factor=4' \
    "$TMP/small_memory_update_unroll.log"; then
  echo '缺少 recurrence 证明的数组更新循环意外采用四倍展开' >&2
  exit 1
fi

SYC_SKIP_HIR_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/small_arithmetic_recurrence_unroll.skip.s" \
  "$ROOT/tests/riscv/small_arithmetic_recurrence_unroll.sy"
riscv64-linux-gnu-gcc -static \
  -o "$TMP/small_arithmetic_recurrence_unroll.skip.elf" \
  "$TMP/small_arithmetic_recurrence_unroll.skip.s" "$RUNTIME" -lm
printf '4 4 5 6 7\n4 1 2 -3 4\n' | \
  qemu-riscv64 "$TMP/small_arithmetic_recurrence_unroll.elf" \
    >"$TMP/small_arithmetic_recurrence_unroll.out" 2>/dev/null
printf '4 4 5 6 7\n4 1 2 -3 4\n' | \
  qemu-riscv64 "$TMP/small_arithmetic_recurrence_unroll.skip.elf" \
    >"$TMP/small_arithmetic_recurrence_unroll.skip.out" 2>/dev/null
cmp "$TMP/small_arithmetic_recurrence_unroll.out" \
  "$TMP/small_arithmetic_recurrence_unroll.skip.out"
test "$(tr -d ' \r\n' \
  <"$TMP/small_arithmetic_recurrence_unroll.out")" = '1532-127256'
printf '8\n' | qemu-riscv64 "$TMP/small_memory_update_unroll.elf" \
  >"$TMP/small_memory_update_unroll.out" 2>/dev/null
test "$(tr -d ' \r\n' <"$TMP/small_memory_update_unroll.out")" = '310'

# 动态除法的多流循环使用八倍展开；超过访存压力边界的循环保持原形态。
for source in dense_division_stream_unroll dense_division_stream_pressure; do
  SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
    -o "$TMP/$source.s" "$ROOT/tests/riscv/$source.sy" \
    2>"$TMP/$source.log"
  SYC_SKIP_HIR_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
    -o "$TMP/$source.skip.s" "$ROOT/tests/riscv/$source.sy"
  for mode in default skip; do
    assembly="$TMP/$source.s"
    if [ "$mode" = skip ]; then
      assembly="$TMP/$source.skip.s"
    fi
    riscv64-linux-gnu-gcc -static -o "$TMP/$source.$mode.elf" \
      "$assembly" "$RUNTIME" -lm
    printf '8 3\n' | qemu-riscv64 "$TMP/$source.$mode.elf" \
      >"$TMP/$source.$mode.out" 2>/dev/null
  done
  cmp "$TMP/$source.default.out" "$TMP/$source.skip.out"
done
grep -F 'phase=partial accept factor=8 cost=24' \
  "$TMP/dense_division_stream_unroll.log"
if grep -F 'phase=partial accept factor=8 cost=24' \
    "$TMP/dense_division_stream_pressure.log"; then
  echo '动态除法压力夹具意外采用八倍展开' >&2
  exit 1
fi
test "$(tr -d ' \r\n' \
  <"$TMP/dense_division_stream_unroll.default.out")" = '1123'
test "$(tr -d ' \r\n' \
  <"$TMP/dense_division_stream_pressure.default.out")" = '1428'

# 短浮点更新循环使用八倍展开来摊薄地址递增和分支；含浮点除法的
# 相似循环继续保留小幅展开，避免复制长延迟除法序列。
compile_pair fp_streaming_update_unroll_positive \
  SYC_SKIP_HIR_FP_STREAMING_UPDATE_UNROLL8=1
expect_changed fp_streaming_update_unroll_positive
run_pair fp_streaming_update_unroll_positive \
  "$ROOT/tests/riscv/fp_streaming_update_unroll_positive.in"
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/fp_streaming_update_unroll_positive.trace.s" \
  "$ROOT/tests/riscv/fp_streaming_update_unroll_positive.sy" \
  2>"$TMP/fp_streaming_update_unroll_positive.trace.log"
grep -F 'phase=partial accept factor=8' \
  "$TMP/fp_streaming_update_unroll_positive.trace.log"

compile_pair fp_streaming_update_unroll_division_negative \
  SYC_SKIP_HIR_FP_STREAMING_UPDATE_UNROLL8=1
expect_unchanged fp_streaming_update_unroll_division_negative
run_pair fp_streaming_update_unroll_division_negative \
  "$ROOT/tests/riscv/fp_streaming_update_unroll_division_negative.in"
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/fp_streaming_update_unroll_division_negative.trace.s" \
  "$ROOT/tests/riscv/fp_streaming_update_unroll_division_negative.sy" \
  2>"$TMP/fp_streaming_update_unroll_division_negative.trace.log"
if grep -Fq 'phase=partial accept factor=8' \
    "$TMP/fp_streaming_update_unroll_division_negative.trace.log"; then
  echo '含浮点除法的循环意外采用了八倍展开' >&2
  exit 1
fi

# 子循环中已经提升的同一符号地址可以继续提升到父循环；
# 父循环含调用时保持原位置，避免延长跨调用活跃区间。
compile_pair hierarchical_loop_symbol_address_positive \
  SYC_SKIP_MIR_HIERARCHICAL_LA_HOIST=1
expect_changed hierarchical_loop_symbol_address_positive
run_pair hierarchical_loop_symbol_address_positive \
  "$ROOT/tests/riscv/hierarchical_loop_symbol_address_positive.in"
SYC_TRACE_MIR_LOOP_ADDR=1 "$COMPILER" -S -O1 \
  -o "$TMP/hierarchical_loop_symbol_address_positive.trace.s" \
  "$ROOT/tests/riscv/hierarchical_loop_symbol_address_positive.sy" \
  2>"$TMP/hierarchical_loop_symbol_address_positive.trace.log"
grep -E '\[mir-loop-symbol\].*symbol=node_values.*uses=2.*nested=1' \
  "$TMP/hierarchical_loop_symbol_address_positive.trace.log"
test "$(tr -d ' \r\n' \
  <"$TMP/hierarchical_loop_symbol_address_positive.default.out")" = '4'

compile_pair hierarchical_loop_symbol_address_call_negative \
  SYC_SKIP_MIR_HIERARCHICAL_LA_HOIST=1
expect_unchanged hierarchical_loop_symbol_address_call_negative
run_pair hierarchical_loop_symbol_address_call_negative \
  "$ROOT/tests/riscv/hierarchical_loop_symbol_address_call_negative.in"
SYC_TRACE_MIR_LOOP_ADDR=1 "$COMPILER" -S -O1 \
  -o "$TMP/hierarchical_loop_symbol_address_call_negative.trace.s" \
  "$ROOT/tests/riscv/hierarchical_loop_symbol_address_call_negative.sy" \
  2>"$TMP/hierarchical_loop_symbol_address_call_negative.trace.log"
if grep -Eq '\[mir-loop-symbol\].*nested=1' \
  "$TMP/hierarchical_loop_symbol_address_call_negative.trace.log"; then
  echo '父循环含调用时意外提升了符号地址' >&2
  exit 1
fi
test "$(tr -d ' \r\n' \
  <"$TMP/hierarchical_loop_symbol_address_call_negative.default.out")" = \
  '00004'

# 两个分支分别产生原值和逐位取反值时，直接用条件全位掩码完成选择；
# 非互补的近似表达式继续使用普通的菱形选择序列。
compile_pair ifconvert_complement_select_positive \
  SYC_SKIP_COREIR_COMPLEMENT_SELECT_IFCONV=1
run_pair ifconvert_complement_select_positive \
  "$ROOT/tests/riscv/ifconvert_complement_select_positive.in"
SYC_TRACE_COREIR_IFCONV=1 "$COMPILER" -S -O1 \
  -o "$TMP/ifconvert_complement_select_positive.trace.s" \
  "$ROOT/tests/riscv/ifconvert_complement_select_positive.sy" \
  2>"$TMP/ifconvert_complement_select_positive.trace.log"
grep -F '[if-convert] complement-select' \
  "$TMP/ifconvert_complement_select_positive.trace.log"
test "$(tr -d ' \r\n' \
  <"$TMP/ifconvert_complement_select_positive.default.out")" = '-5'

compile_pair ifconvert_complement_select_negative \
  SYC_SKIP_COREIR_COMPLEMENT_SELECT_IFCONV=1
expect_unchanged ifconvert_complement_select_negative
run_pair ifconvert_complement_select_negative \
  "$ROOT/tests/riscv/ifconvert_complement_select_negative.in"
test "$(tr -d ' \r\n' \
  <"$TMP/ifconvert_complement_select_negative.default.out")" = '-6'
SYC_TRACE_COREIR_IFCONV=1 "$COMPILER" -S -O1 \
  -o "$TMP/ifconvert_complement_select_negative.trace.s" \
  "$ROOT/tests/riscv/ifconvert_complement_select_negative.sy" \
  2>"$TMP/ifconvert_complement_select_negative.trace.log"
if grep -Fq '[if-convert] complement-select' \
    "$TMP/ifconvert_complement_select_negative.trace.log"; then
  echo '非互补分支意外采用了互补值选择' >&2
  exit 1
fi

COMPLEMENT_HOT_SOURCE="$PERFORMANCE_ROOT/h-1-01.sy"
COMPLEMENT_HOT_INPUT="$PERFORMANCE_ROOT/h-1-01.in"
if [ ! -f "$COMPLEMENT_HOT_SOURCE" ] || [ ! -f "$COMPLEMENT_HOT_INPUT" ]; then
  echo "missing complement-select performance fixture under $PERFORMANCE_ROOT" >&2
  exit 1
fi
compile_pair_path ifconvert_complement_select_hot_positive \
  "$COMPLEMENT_HOT_SOURCE" SYC_SKIP_COREIR_COMPLEMENT_SELECT_IFCONV=1
expect_changed ifconvert_complement_select_hot_positive
run_pair ifconvert_complement_select_hot_positive "$COMPLEMENT_HOT_INPUT"

HUFFMAN_SOURCE="$PERFORMANCE_ROOT/huffman-01.sy"
HUFFMAN_INPUT="$PERFORMANCE_ROOT/huffman-01.in"
if [ ! -f "$HUFFMAN_SOURCE" ] || [ ! -f "$HUFFMAN_INPUT" ]; then
  echo "missing Huffman compliance fixture under $PERFORMANCE_ROOT" >&2
  exit 1
fi

if [[ "${SYC_TEST_DORMANT_STATE_TRANSFORMS:-0}" == 1 ]]; then
  compile_pair_path fixpoint_strict_positive "$HUFFMAN_SOURCE" \
    SYC_SKIP_HIR_LOOP_STATE_FIXPOINT_BREAK=1
  expect_changed fixpoint_strict_positive
  run_pair fixpoint_strict_positive "$HUFFMAN_INPUT"

  sed 's/while (n > 0)/while (n >= 0)/' "$HUFFMAN_SOURCE" \
    >"$TMP/fixpoint_inclusive_negative.sy"
  compile_pair_path fixpoint_inclusive_negative \
    "$TMP/fixpoint_inclusive_negative.sy" \
    SYC_SKIP_HIR_LOOP_STATE_FIXPOINT_BREAK=1
  expect_unchanged fixpoint_inclusive_negative

  sed -e '/int bfinal, btype;/a\    int audit_output[2];' \
      -e '/decode_fixed_huffman();/a\        audit_output[btype] = bfinal;' \
      -e '/stoptime();/i\    putint(audit_output[0]); putint(audit_output[1]);' \
      "$HUFFMAN_SOURCE" >"$TMP/fixpoint_aggregate_negative.sy"
  compile_pair_path fixpoint_aggregate_negative \
    "$TMP/fixpoint_aggregate_negative.sy" \
    SYC_SKIP_HIR_LOOP_STATE_FIXPOINT_BREAK=1
  expect_unchanged fixpoint_aggregate_negative
fi

# 循环内直接读写的全局标量可以形成循环携带值；若循环中的调用会读取该
# 标量，则继续保留每轮的全局内存更新。
compile_pair loop_global_scalar_promotion_positive \
  SYC_SKIP_COREIR_LOOP_GLOBAL_SCALAR_PROMOTION=1
expect_changed loop_global_scalar_promotion_positive
run_pair loop_global_scalar_promotion_positive \
  "$ROOT/tests/riscv/loop_global_scalar_promotion_positive.in"
SYC_TRACE_COREIR_GLOBAL_SCALAR_PROMOTION=1 "$COMPILER" -S -O1 \
  -o "$TMP/loop_global_scalar_promotion_positive.trace.s" \
  "$ROOT/tests/riscv/loop_global_scalar_promotion_positive.sy" \
  2>"$TMP/loop_global_scalar_promotion_positive.trace.log"
grep -E '\[global-scalar\] apply func=main global=loop_total' \
  "$TMP/loop_global_scalar_promotion_positive.trace.log"
test "$(tr -d ' \r\n' \
  <"$TMP/loop_global_scalar_promotion_positive.default.out")" = '10'

compile_pair loop_global_scalar_promotion_call_negative \
  SYC_SKIP_COREIR_LOOP_GLOBAL_SCALAR_PROMOTION=1
expect_unchanged loop_global_scalar_promotion_call_negative
run_pair loop_global_scalar_promotion_call_negative \
  "$ROOT/tests/riscv/loop_global_scalar_promotion_call_negative.in"
SYC_TRACE_COREIR_GLOBAL_SCALAR_PROMOTION=1 "$COMPILER" -S -O1 \
  -o "$TMP/loop_global_scalar_promotion_call_negative.trace.s" \
  "$ROOT/tests/riscv/loop_global_scalar_promotion_call_negative.sy" \
  2>"$TMP/loop_global_scalar_promotion_call_negative.trace.log"
grep -E '\[global-scalar\] reject func=main global=visible_total reason=call' \
  "$TMP/loop_global_scalar_promotion_call_negative.trace.log"
test "$(tr -d ' \r\n' \
  <"$TMP/loop_global_scalar_promotion_call_negative.default.out")" = '10'

# Earlier affine rewrites can delete a loop header retained by a later
# diagnostic plan. Trace mode must report that stale observation safely.
AFFINE_ARRAY_ACC_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/affine_array_trace_liveness.s" \
  "$PERFORMANCE_ROOT/03_sort1.sy" \
  2>"$TMP/affine_array_trace_liveness.log"
grep -F 'header=<deleted>' "$TMP/affine_array_trace_liveness.log"

echo "loop semantic optimizations: PASS"
