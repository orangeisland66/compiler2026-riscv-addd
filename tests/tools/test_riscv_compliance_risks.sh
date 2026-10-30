#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1
# This suite exercises the legacy AST recursive-accumulator semantic corpus;
# research builds now require its semantic enable independently of master opt-in.
export SYC_ENABLE_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1
export SYC_ENABLE_AST_RADIX_SORT_LOWERING=1
export SYC_ENABLE_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
export SYC_ENABLE_AST_CROSS_CALL_ARRAY_SUMMARY=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
PERFORMANCE_ROOT="${PERFORMANCE_ROOT:-$ROOT/2026初赛RISCV赛道性能用例/performance}"
TMP="${TMPDIR:-/tmp}/syc-compliance-risks"
mkdir -p "$TMP"

compile_case() {
  local name="$1"
  local source="$2"
  shift 2
  env "$@" "$COMPILER" -S -O1 -o "$TMP/$name.s" "$source"
}

link_case() {
  local name="$1"
  riscv64-linux-gnu-gcc -static -o "$TMP/$name.elf" \
    "$TMP/$name.s" "$RUNTIME" -lm
}

run_case() {
  local name="$1"
  local input="$2"
  printf '%s' "$input" | qemu-riscv64 "$TMP/$name.elf" \
    >"$TMP/$name.out" 2>/dev/null
}

run_case_as() {
  local binary="$1"
  local output="$2"
  local input="$3"
  printf '%s' "$input" | qemu-riscv64 "$TMP/$binary.elf" \
    >"$TMP/$output.out" 2>/dev/null
}

if grep -Eq 'detectAndReplaceBitLoop|andCount|orCount' \
  "$ROOT/src/rewrite/bit_patterns.cpp"; then
  echo "instruction-count bit-loop heuristic is still present" >&2
  exit 1
fi

if grep -Eq 'tryLowerTimingBuiltinCall|_sysy_stoptime' \
  "$ROOT/src/backend_mir/lower.cpp"; then
  echo "callee-specific timing lowering is still present" >&2
  exit 1
fi

# 字节区间单测覆盖同址、部分重叠、相邻区间、负偏移和极值偏移；测试程序
# 只生成在临时目录，不向提交仓库增加第二个 main。
"${CXX:-c++}" -std=c++17 -I"$ROOT/src" -x c++ \
  -o "$TMP/local_store_range_test" - <<'CPP'
#include "hir/local_store_load_forward_detail.h"
#include <cassert>
#include <cstdint>
#include <limits>

int main() {
  using hir::detail::MemoryByteRange;
  using hir::detail::memoryByteRangesOverlap;
  assert(memoryByteRangesOverlap({0, 4}, {0, 4}));
  assert(memoryByteRangesOverlap({0, 8}, {4, 4}));
  assert(memoryByteRangesOverlap({-4, 8}, {0, 4}));
  assert(!memoryByteRangesOverlap({0, 4}, {4, 4}));
  assert(!memoryByteRangesOverlap({-4, 4}, {0, 4}));
  assert(!memoryByteRangesOverlap(
      {std::numeric_limits<int64_t>::min(), 4},
      {std::numeric_limits<int64_t>::max(), 4}));
  return 0;
}
CPP
"$TMP/local_store_range_test"

compile_case bit.default "$ROOT/tests/riscv/bit_loop_vote_negative.sy"
compile_case bit.skip "$ROOT/tests/riscv/bit_loop_vote_negative.sy" \
  SYC_SKIP_COREIR_BIT_PATTERNS=1
cmp "$TMP/bit.default.s" "$TMP/bit.skip.s"
for mode in default skip; do
  link_case "bit.$mode"
  run_case "bit.$mode" $'63 5\n'
  test "$(tr -d '\r\n' <"$TMP/bit.$mode.out")" = "6"
done

# 有符号除法和余数模拟的位运算必须覆盖负数与 INT_MIN，奇偶比较也要
# 区分余数 -1 和 1；正数、负数和边界输入均与未变换版本逐项对照。
compile_case bit_signed.default \
  "$ROOT/tests/riscv/bit_accumulator_signed_domain.sy"
compile_case bit_signed.skip \
  "$ROOT/tests/riscv/bit_accumulator_signed_domain.sy" \
  SYC_SKIP_COREIR_BIT_ACCUMULATOR=1 \
  SYC_SKIP_COREIR_BIT_PATTERNS=1 \
  SYC_SKIP_PARITY_CANONICALIZE=1
if cmp -s "$TMP/bit_signed.default.s" "$TMP/bit_signed.skip.s"; then
  echo "signed bit accumulator positive did not trigger" >&2
  exit 1
fi
BIT_PATTERN_TRACE=1 "$COMPILER" -S -O1 \
  -o "$TMP/bit_signed.trace.s" \
  "$ROOT/tests/riscv/bit_accumulator_signed_domain.sy" \
  2>"$TMP/bit_signed.log"
grep -F '[bit-pattern] accept-helper signed_and and' \
  "$TMP/bit_signed.log"
grep -F '[bit-pattern] accept-helper signed_or or' \
  "$TMP/bit_signed.log"
grep -F '[bit-pattern] accept-helper signed_xor xor' \
  "$TMP/bit_signed.log"
for mode in default skip; do
  link_case "bit_signed.$mode"
done
bit_signed_inputs=(
  $'13 11\n'
  $'-1 1\n'
  $'-3 -5\n'
  $'-2147483648 2147483647\n'
  $'0 -1\n'
)
for index in "${!bit_signed_inputs[@]}"; do
  run_case_as bit_signed.default "bit_signed.default.$index" \
    "${bit_signed_inputs[$index]}"
  run_case_as bit_signed.skip "bit_signed.skip.$index" \
    "${bit_signed_inputs[$index]}"
  diff -u "$TMP/bit_signed.skip.$index.out" \
    "$TMP/bit_signed.default.$index.out"
done

compile_case stage.default \
  "$ROOT/tests/riscv/stage_fusion_partial_map_negative.sy"
compile_case stage.skip \
  "$ROOT/tests/riscv/stage_fusion_partial_map_negative.sy" \
  SYC_SKIP_COREIR_STAGE_FUSION=1
cmp "$TMP/stage.default.s" "$TMP/stage.skip.s"
for mode in default skip; do
  link_case "stage.$mode"
  run_case "stage.$mode" $'2\n'
  test "$(tr -d '\r\n' <"$TMP/stage.$mode.out")" = "-13"
done

compile_case stage_effect.default \
  "$ROOT/tests/riscv/stage_fusion_side_effect_negative.sy"
compile_case stage_effect.skip \
  "$ROOT/tests/riscv/stage_fusion_side_effect_negative.sy" \
  SYC_SKIP_COREIR_STAGE_FUSION=1
cmp "$TMP/stage_effect.default.s" "$TMP/stage_effect.skip.s"
for mode in default skip; do
  link_case "stage_effect.$mode"
  run_case "stage_effect.$mode" $'2\n'
  test "$(tr -d '\r\n' <"$TMP/stage_effect.$mode.out")" = "-8"
done

compile_case stage_liveout.default \
  "$ROOT/tests/riscv/stage_fusion_liveout_negative.sy"
compile_case stage_liveout.skip \
  "$ROOT/tests/riscv/stage_fusion_liveout_negative.sy" \
  SYC_SKIP_COREIR_STAGE_FUSION=1
cmp "$TMP/stage_liveout.default.s" "$TMP/stage_liveout.skip.s"
for mode in default skip; do
  link_case "stage_liveout.$mode"
  run_case "stage_liveout.$mode" $'2\n'
  test "$(tr -d '\r\n' <"$TMP/stage_liveout.$mode.out")" = "0"
done

compile_case stage_partial_checksum.default \
  "$ROOT/tests/riscv/stage_fusion_partial_checksum_negative.sy"
compile_case stage_partial_checksum.skip \
  "$ROOT/tests/riscv/stage_fusion_partial_checksum_negative.sy" \
  SYC_SKIP_COREIR_STAGE_FUSION=1
cmp "$TMP/stage_partial_checksum.default.s" \
  "$TMP/stage_partial_checksum.skip.s"
for mode in default skip; do
  link_case "stage_partial_checksum.$mode"
  run_case "stage_partial_checksum.$mode" $'2\n'
  test "$(tr -d '\r\n' <"$TMP/stage_partial_checksum.$mode.out")" = "-3"
done

compile_case producer.default \
  "$ROOT/tests/riscv/producer_map_zero_trip_negative.sy" \
  SYC_ENABLE_COREIR_STENCIL_INTERIOR=1
compile_case producer.skip \
  "$ROOT/tests/riscv/producer_map_zero_trip_negative.sy" \
  SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 \
  SYC_SKIP_COREIR_STENCIL_INTERIOR=1
for mode in default skip; do
  link_case "producer.$mode"
  run_case_as "producer.$mode" "producer.$mode.zero" $'4 1\n'
  test "$(tr -d '\r\n' <"$TMP/producer.$mode.zero.out")" = "336"
  run_case_as "producer.$mode" "producer.$mode.multi" $'4 3\n'
  test "$(tr -d '\r\n' <"$TMP/producer.$mode.multi.out")" = "186"
done

SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 SYC_TRACE_COREIR_STENCIL=1 \
  "$COMPILER" -S -O1 \
  -o "$TMP/producer_liveout.default.s" \
  "$ROOT/tests/riscv/producer_map_liveout_negative.sy" \
  2>"$TMP/producer_liveout.default.log"
compile_case producer_liveout.skip \
  "$ROOT/tests/riscv/producer_map_liveout_negative.sy" \
  SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 \
  SYC_SKIP_COREIR_PRODUCER_MAP=1
cmp "$TMP/producer_liveout.default.s" "$TMP/producer_liveout.skip.s"
# 当前 producer/stencil 入口在更早的统一 loop-shape/effect 门槛即可拒绝该
# live-out 近似形态；不得要求执行到某个较晚 matcher 才算安全拒绝。
grep -F '[stencil-interior] reject' "$TMP/producer_liveout.default.log"
for mode in default skip; do
  link_case "producer_liveout.$mode"
  run_case "producer_liveout.$mode" $'4 3\n'
  test "$(tr -d '\r\n' <"$TMP/producer_liveout.$mode.out")" = "186"
done

# 递归 memoization 现在必须先经过机器 admission。当前 RV64 模型会因候选
# 栈帧增长拒绝这些高风险候选；拒绝必须保持 baseline 汇编和运行结果。
compile_case mixed_recursive_memo.default \
  "$ROOT/tests/riscv/mixed_recursive_memo.sy" \
  SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1
compile_case mixed_recursive_memo.skip \
  "$ROOT/tests/riscv/mixed_recursive_memo.sy" \
  SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_SKIP_COREIR_RECURSIVE_MEMOIZATION=1
cmp "$TMP/mixed_recursive_memo.default.s" \
  "$TMP/mixed_recursive_memo.skip.s"
mixed_expected_stdout="$(sed -n '1p' \
  "$ROOT/tests/riscv/mixed_recursive_memo.out" | tr -d '\r')"
mixed_expected_exit="$(tail -n 1 \
  "$ROOT/tests/riscv/mixed_recursive_memo.out" | tr -d '\r\n')"
for mode in default skip; do
  link_case "mixed_recursive_memo.$mode"
  run_case "mixed_recursive_memo.$mode" \
    "$(cat "$ROOT/tests/riscv/mixed_recursive_memo.in")"
  test "$(tr -d '\r\n' <"$TMP/mixed_recursive_memo.$mode.out")" = \
    "$mixed_expected_stdout"
done
test "$mixed_expected_exit" = "0"

# 唯一赋值和多状态用例同样验证 fail-closed 回退与输出一致。
SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1 COREIR_RECURSION_TRACE=1 \
  "$COMPILER" -S -O1 \
  -o "$TMP/memo_fixed_scalar.default.s" \
  "$ROOT/tests/riscv/recursive_memo_fixed_scalar_state.sy" \
  2>"$TMP/memo_fixed_scalar.log"
compile_case memo_fixed_scalar.skip \
  "$ROOT/tests/riscv/recursive_memo_fixed_scalar_state.sy" \
  SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_SKIP_COREIR_RECURSIVE_MEMOIZATION=1
cmp "$TMP/memo_fixed_scalar.default.s" "$TMP/memo_fixed_scalar.skip.s"
for mode in default skip; do
  link_case "memo_fixed_scalar.$mode"
  run_case "memo_fixed_scalar.$mode" $'3\n'
  test "$(tr -d '\r' <"$TMP/memo_fixed_scalar.$mode.out")" = \
    $'47\n76'
done

# 旧指纹对 (0, 1) 与 (1, 64) 产生相同结果；缓存上下文必须继续比较
# 两个完整状态值，第二次调用不能复用第一次调用的递归结果。
compile_case memo_state_collision.default \
  "$ROOT/tests/riscv/recursive_memo_state_collision.sy" \
  SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1
compile_case memo_state_collision.skip \
  "$ROOT/tests/riscv/recursive_memo_state_collision.sy" \
  SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_SKIP_COREIR_RECURSIVE_MEMOIZATION=1
cmp "$TMP/memo_state_collision.default.s" \
  "$TMP/memo_state_collision.skip.s"
for mode in default skip; do
  link_case "memo_state_collision.$mode"
  run_case "memo_state_collision.$mode" ''
  test "$(tr -d '\r' <"$TMP/memo_state_collision.$mode.out")" = \
    $'34\n2210'
done

# 多次外部调用使用可变 epoch 隔离缓存；生成代码必须包含回绕清表路径，
# 同时正常路径的两次调用仍与关闭记忆化的结果一致。
SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1 COREIR_RECURSION_TRACE=1 \
  "$COMPILER" -S -O1 \
  -o "$TMP/memo_epoch_wrap.default.s" \
  "$ROOT/tests/riscv/recursive_memo_epoch_wrap.sy" \
  2>"$TMP/memo_epoch_wrap.log"
compile_case memo_epoch_wrap.skip \
  "$ROOT/tests/riscv/recursive_memo_epoch_wrap.sy" \
  SYC_ENABLE_COREIR_RECURSIVE_MEMOIZATION=1 \
  SYC_SKIP_COREIR_RECURSIVE_MEMOIZATION=1
cmp "$TMP/memo_epoch_wrap.default.s" "$TMP/memo_epoch_wrap.skip.s"
for mode in default skip; do
  link_case "memo_epoch_wrap.$mode"
  run_case "memo_epoch_wrap.$mode" ''
  test "$(tr -d '\r' <"$TMP/memo_epoch_wrap.$mode.out")" = \
    $'26\n42'
done

compile_case calls "$ROOT/tests/riscv/single_register_void_calls.sy" \
  MIR_SIMPLE_RA=1
for callee in _sysy_starttime putint putfloat putch _sysy_stoptime; do
  grep -Eq "call[[:space:]]+$callee" "$TMP/calls.s"
done
if grep -Eq '(ld[[:space:]]+a0|flw[[:space:]]+fa0),[[:space:]]*0\(sp\)' \
  "$TMP/calls.s"; then
  echo "single-register void call still uses stack staging" >&2
  exit 1
fi
link_case calls
run_case_as calls calls.int $'1\n'
test "$(tr -d '\r\n' <"$TMP/calls.int.out")" = "7"
run_case_as calls calls.float $'2\n'
test "$(tr -d '\r\n' <"$TMP/calls.float.out")" = "0x1p+0"
run_case_as calls calls.char $'3\n'
test "$(od -An -tx1 "$TMP/calls.char.out" | tr -d ' \r\n')" = "0a"

# AST reflective-max 变换只存在于 research 构建。strict 编译器即使收到
# 对应环境变量，也必须生成完全相同的汇编。
compile_case reflective.default \
  "$ROOT/tests/riscv/reflective_max_research_negative.sy"
compile_case reflective.enabled \
  "$ROOT/tests/riscv/reflective_max_research_negative.sy" \
  SYC_ENABLE_AST_REFLECTIVE_MAX_SIMPLIFY=1
cmp "$TMP/reflective.default.s" "$TMP/reflective.enabled.s"

# 递归累加器归一化仍要求递归增量与 sentinel 关系成立，但允许递归调用
# 原样传递无关的标量或数组上下文参数。
compile_case recursive_acc_extra.default \
  "$ROOT/tests/riscv/recursive_accumulator_extra_params_positive.sy"
compile_case recursive_acc_extra.skip \
  "$ROOT/tests/riscv/recursive_accumulator_extra_params_positive.sy" \
  SYC_SKIP_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1
if cmp -s "$TMP/recursive_acc_extra.default.s" \
  "$TMP/recursive_acc_extra.skip.s"; then
  echo "recursive accumulator extra-parameter positive did not trigger" >&2
  exit 1
fi
SYC_TRACE_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1 "$COMPILER" -S -O1 \
  -o "$TMP/recursive_acc_extra.trace.s" \
  "$ROOT/tests/riscv/recursive_accumulator_extra_params_positive.sy" \
  2>"$TMP/recursive_acc_extra.log"
grep -F \
  '[ast-recursive-accumulator] function=walk_state accum=acc params=4' \
  "$TMP/recursive_acc_extra.log"
for mode in default skip; do
  link_case "recursive_acc_extra.$mode"
  run_case_as "recursive_acc_extra.$mode" "recursive_acc_extra.$mode.pos" \
    $'4 0\n'
  test "$(tr -d '\r\n' <"$TMP/recursive_acc_extra.$mode.pos.out")" = "4"
  run_case_as "recursive_acc_extra.$mode" "recursive_acc_extra.$mode.offset" \
    $'2 3\n'
  test "$(tr -d '\r\n' <"$TMP/recursive_acc_extra.$mode.offset.out")" = "5"
  run_case_as "recursive_acc_extra.$mode" "recursive_acc_extra.$mode.zero" \
    $'0 5\n'
  test "$(tr -d '\r\n' <"$TMP/recursive_acc_extra.$mode.zero.out")" = "5"
  run_case_as "recursive_acc_extra.$mode" "recursive_acc_extra.$mode.sentinel" \
    $'-1 9\n'
  test "$(tr -d '\r\n' <"$TMP/recursive_acc_extra.$mode.sentinel.out")" = "7"
done

# 用户可以使用编译器内部风格的函数名和形参名；归一化通过 fresh name
# 消除 helper 与包装局部变量冲突，不能因命名方式而放弃本来可用的变换。
compile_case recursive_acc_name.default \
  "$ROOT/tests/riscv/recursive_accumulator_name_collision_positive.sy"
compile_case recursive_acc_name.skip \
  "$ROOT/tests/riscv/recursive_accumulator_name_collision_positive.sy" \
  SYC_SKIP_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1
if cmp -s "$TMP/recursive_acc_name.default.s" \
  "$TMP/recursive_acc_name.skip.s"; then
  echo "recursive accumulator name-collision positive did not trigger" >&2
  exit 1
fi
SYC_TRACE_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1 "$COMPILER" -S -O1 \
  -o "$TMP/recursive_acc_name.trace.s" \
  "$ROOT/tests/riscv/recursive_accumulator_name_collision_positive.sy" \
  2>"$TMP/recursive_acc_name.log"
grep -F \
  '[ast-recursive-accumulator] function=__norm_walk_state accum=acc params=4' \
  "$TMP/recursive_acc_name.log"
for mode in default skip; do
  link_case "recursive_acc_name.$mode"
  run_case "recursive_acc_name.$mode" $'3 4\n'
  test "$(tr -d '\r' <"$TMP/recursive_acc_name.$mode.out")" = \
    $'7\n102'
done

compile_case recursive_acc_negative.default \
  "$ROOT/tests/riscv/recursive_accumulator_negative.sy"
compile_case recursive_acc_negative.skip \
  "$ROOT/tests/riscv/recursive_accumulator_negative.sy" \
  SYC_SKIP_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1
cmp "$TMP/recursive_acc_negative.default.s" \
  "$TMP/recursive_acc_negative.skip.s"
SYC_TRACE_AST_RECURSIVE_ACCUMULATOR_NORMALIZE=1 "$COMPILER" -S -O1 \
  -o "$TMP/recursive_acc_negative.trace.s" \
  "$ROOT/tests/riscv/recursive_accumulator_negative.sy" \
  2>"$TMP/recursive_acc_negative.log"
if grep -q '\[ast-recursive-accumulator\]' \
  "$TMP/recursive_acc_negative.log"; then
  echo "recursive accumulator accepted an unsafe near-miss" >&2
  exit 1
fi
for mode in default skip; do
  link_case "recursive_acc_negative.$mode"
  run_case "recursive_acc_negative.$mode" $'2 0 2\n'
  test "$(tr -d '\r' <"$TMP/recursive_acc_negative.$mode.out")" = \
    $'4\n2\n4\n2\n2'
done

# 这些算法级 AST pass 已从编译入口断开，等待迁移到 proof/planning 层后
# 再恢复触发型测试。保留语义 corpus，默认发布门禁不伪称它们仍已接线。
if [[ "${SYC_TEST_QUARANTINED_AST_PASSES:-0}" == 1 ]]; then
# 被调用函数不读取额外标量参数时，跨调用摘要可保留这些上下文参数。
# 正例还占用首选摘要数组名和标量名，验证 fresh name；被调用函数实际
# 使用额外值时维持原有调用路径。
compile_case cross_extra.default \
  "$ROOT/tests/riscv/cross_call_extra_scalar_positive.sy"
compile_case cross_extra.skip \
  "$ROOT/tests/riscv/cross_call_extra_scalar_positive.sy" \
  SYC_SKIP_AST_CROSS_CALL_ARRAY_SUMMARY=1
SYC_TRACE_AST_CROSS_CALL_ARRAY_SUMMARY=1 "$COMPILER" -S -O1 \
  -o "$TMP/cross_extra.trace.s" \
  "$ROOT/tests/riscv/cross_call_extra_scalar_positive.sy" \
  2>"$TMP/cross_extra.log"
grep -F \
  '[ast-array-summary] function=run_extra_summary callee=advance_rows_extra calls-per-round=4 args=5' \
  "$TMP/cross_extra.log"
grep -F 'order=row-major state=scalar' "$TMP/cross_extra.log"
for mode in default skip; do
  link_case "cross_extra.$mode"
  run_case "cross_extra.$mode" ''
done
diff -u "$TMP/cross_extra.skip.out" "$TMP/cross_extra.default.out"

# 摘要计算没有写回双缓冲矩阵，后续仍读取矩阵时必须拒绝变换。
compile_case cross_array_liveout.default \
  "$ROOT/tests/riscv/cross_call_array_liveout_negative.sy"
compile_case cross_array_liveout.skip \
  "$ROOT/tests/riscv/cross_call_array_liveout_negative.sy" \
  SYC_SKIP_AST_CROSS_CALL_ARRAY_SUMMARY=1
cmp "$TMP/cross_array_liveout.default.s" \
  "$TMP/cross_array_liveout.skip.s"
SYC_TRACE_AST_CROSS_CALL_ARRAY_SUMMARY=1 "$COMPILER" -S -O1 \
  -o "$TMP/cross_array_liveout.trace.s" \
  "$ROOT/tests/riscv/cross_call_array_liveout_negative.sy" \
  2>"$TMP/cross_array_liveout.log"
if grep -q '\[ast-array-summary\]' "$TMP/cross_array_liveout.log"; then
  echo "cross-call array live-out was incorrectly accepted" >&2
  exit 1
fi
for mode in default skip; do
  link_case "cross_array_liveout.$mode"
  run_case "cross_array_liveout.$mode" ''
done
diff -u "$TMP/cross_array_liveout.skip.out" \
  "$TMP/cross_array_liveout.default.out"

compile_case cross_extra_used.default \
  "$ROOT/tests/riscv/cross_call_extra_scalar_negative.sy"
compile_case cross_extra_used.skip \
  "$ROOT/tests/riscv/cross_call_extra_scalar_negative.sy" \
  SYC_SKIP_AST_CROSS_CALL_ARRAY_SUMMARY=1
cmp "$TMP/cross_extra_used.default.s" "$TMP/cross_extra_used.skip.s"

# 跨调用摘要覆盖不同的偶数调用次数；别名、循环变量后续使用、额外副作用、
# 非方阵、被调用函数副作用和近似校验和形态继续保留原有程序。
for count in two six; do
  compile_case "cross_count.$count.default" \
    "$ROOT/tests/riscv/cross_call_call_count_${count}_positive.sy"
  compile_case "cross_count.$count.skip" \
    "$ROOT/tests/riscv/cross_call_call_count_${count}_positive.sy" \
    SYC_SKIP_AST_CROSS_CALL_ARRAY_SUMMARY=1
  SYC_TRACE_AST_CROSS_CALL_ARRAY_SUMMARY=1 "$COMPILER" -S -O1 \
    -o "$TMP/cross_count.$count.trace.s" \
    "$ROOT/tests/riscv/cross_call_call_count_${count}_positive.sy" \
    2>"$TMP/cross_count.$count.log"
  expected_calls=2
  test "$count" = six && expected_calls=6
  grep -F \
    "[ast-array-summary] function=run_${count} callee=advance_rows_${count} calls-per-round=$expected_calls args=5" \
    "$TMP/cross_count.$count.log"
  grep -F 'order=row-major state=scalar' \
    "$TMP/cross_count.$count.log"
  for mode in default skip; do
    link_case "cross_count.$count.$mode"
    run_case "cross_count.$count.$mode" ''
  done
  diff -u "$TMP/cross_count.$count.skip.out" \
    "$TMP/cross_count.$count.default.out"
done

compile_case cross_near.default \
  "$ROOT/tests/riscv/cross_call_near_miss_negative.sy"
compile_case cross_near.skip \
  "$ROOT/tests/riscv/cross_call_near_miss_negative.sy" \
  SYC_SKIP_AST_CROSS_CALL_ARRAY_SUMMARY=1
cmp "$TMP/cross_near.default.s" "$TMP/cross_near.skip.s"
SYC_TRACE_AST_CROSS_CALL_ARRAY_SUMMARY=1 "$COMPILER" -S -O1 \
  -o "$TMP/cross_near.trace.s" \
  "$ROOT/tests/riscv/cross_call_near_miss_negative.sy" \
  2>"$TMP/cross_near.log"
if grep -q '\[ast-array-summary\]' "$TMP/cross_near.log"; then
  echo "cross-call near miss was incorrectly accepted" >&2
  exit 1
fi
for mode in default skip; do
  link_case "cross_near.$mode"
  run_case "cross_near.$mode" ''
  test "$(tr -d '\r\n' <"$TMP/cross_near.$mode.out")" = "0"
done

# demand-driven copy 允许未使用的标量上下文参数以及无副作用的空语句、
# 未初始化声明。正例还占用首选输出下标名以验证 fresh name；callee 读取
# 额外参数会改变原计算，必须拒绝。
compile_case demand_extra.default \
  "$ROOT/tests/riscv/demand_copy_extra_scalar_positive.sy"
compile_case demand_extra.skip \
  "$ROOT/tests/riscv/demand_copy_extra_scalar_positive.sy" \
  SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
SYC_TRACE_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1 "$COMPILER" -S -O1 \
  -o "$TMP/demand_extra.trace.s" \
  "$ROOT/tests/riscv/demand_copy_extra_scalar_positive.sy" \
  2>"$TMP/demand_extra.log"
grep -F \
  '[ast-demand-copy] function=run_demand_extra transform=reshape_lower_triangle_extra array=storage args=4' \
  "$TMP/demand_extra.log"
for mode in default skip; do
  link_case "demand_extra.$mode"
  run_case "demand_extra.$mode" ''
done
diff -u "$TMP/demand_extra.skip.out" "$TMP/demand_extra.default.out"

# 按需计算不会写回完整数组，校验和之后仍读取数组时必须拒绝变换。
compile_case demand_array_liveout.default \
  "$ROOT/tests/riscv/demand_copy_array_liveout_negative.sy"
compile_case demand_array_liveout.skip \
  "$ROOT/tests/riscv/demand_copy_array_liveout_negative.sy" \
  SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
cmp "$TMP/demand_array_liveout.default.s" \
  "$TMP/demand_array_liveout.skip.s"
SYC_TRACE_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1 "$COMPILER" -S -O1 \
  -o "$TMP/demand_array_liveout.trace.s" \
  "$ROOT/tests/riscv/demand_copy_array_liveout_negative.sy" \
  2>"$TMP/demand_array_liveout.log"
if grep -q '\[ast-demand-copy\]' "$TMP/demand_array_liveout.log"; then
  echo "demand-copy array live-out was incorrectly accepted" >&2
  exit 1
fi
for mode in default skip; do
  link_case "demand_array_liveout.$mode"
  run_case "demand_array_liveout.$mode" ''
done
diff -u "$TMP/demand_array_liveout.skip.out" \
  "$TMP/demand_array_liveout.default.out"

compile_case demand_zero.default \
  "$ROOT/tests/riscv/demand_copy_zero_trip_positive.sy"
compile_case demand_zero.skip \
  "$ROOT/tests/riscv/demand_copy_zero_trip_positive.sy" \
  SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
SYC_TRACE_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1 "$COMPILER" -S -O1 \
  -o "$TMP/demand_zero.trace.s" \
  "$ROOT/tests/riscv/demand_copy_zero_trip_positive.sy" \
  2>"$TMP/demand_zero.log"
grep -F \
  '[ast-demand-copy] function=run_zero transform=reshape_zero array=storage_zero args=4' \
  "$TMP/demand_zero.log"
for mode in default skip; do
  link_case "demand_zero.$mode"
  run_case "demand_zero.$mode" ''
done
diff -u "$TMP/demand_zero.skip.out" "$TMP/demand_zero.default.out"

compile_case demand_extra_used.default \
  "$ROOT/tests/riscv/demand_copy_extra_scalar_negative.sy"
compile_case demand_extra_used.skip \
  "$ROOT/tests/riscv/demand_copy_extra_scalar_negative.sy" \
  SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
cmp "$TMP/demand_extra_used.default.s" "$TMP/demand_extra_used.skip.s"

compile_case demand_tail.default \
  "$ROOT/tests/riscv/demand_copy_tail_side_effect_negative.sy"
compile_case demand_tail.skip \
  "$ROOT/tests/riscv/demand_copy_tail_side_effect_negative.sy" \
  SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
cmp "$TMP/demand_tail.default.s" "$TMP/demand_tail.skip.s"

# demand-driven copy 必须拒绝数组别名、调用循环 live-out、被丢弃但带
# side effect 的实参，以及非三角形 callee。
compile_case demand_near.default \
  "$ROOT/tests/riscv/demand_copy_near_miss_negative.sy"
compile_case demand_near.skip \
  "$ROOT/tests/riscv/demand_copy_near_miss_negative.sy" \
  SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1
cmp "$TMP/demand_near.default.s" "$TMP/demand_near.skip.s"
SYC_TRACE_AST_DEMAND_DRIVEN_COPY_TRANSFORM=1 "$COMPILER" -S -O1 \
  -o "$TMP/demand_near.trace.s" \
  "$ROOT/tests/riscv/demand_copy_near_miss_negative.sy" \
  2>"$TMP/demand_near.log"
if grep -q '\[ast-demand-copy\]' "$TMP/demand_near.log"; then
  echo "demand-copy near miss was incorrectly accepted" >&2
  exit 1
fi
for mode in default skip; do
  link_case "demand_near.$mode"
  run_case "demand_near.$mode" ''
done
diff -u "$TMP/demand_near.skip.out" "$TMP/demand_near.default.out"

# 递归 radix 降低允许未使用的标量上下文参数以及无副作用的空语句、
# 未初始化声明。正例同时占用 pass 的首选全局名和局部名，验证 fresh name；
# 负例让辅助函数读取额外参数并改变 digit，必须拒绝。
compile_case radix_extra.default \
  "$ROOT/tests/riscv/radix_sort_extra_scalar_positive.sy"
compile_case radix_extra.skip \
  "$ROOT/tests/riscv/radix_sort_extra_scalar_positive.sy" \
  SYC_SKIP_AST_RADIX_SORT_LOWERING=1
SYC_TRACE_AST_RADIX_SORT_LOWERING=1 "$COMPILER" -S -O1 \
  -o "$TMP/radix_extra.trace.s" \
  "$ROOT/tests/riscv/radix_sort_extra_scalar_positive.sy" \
  2>"$TMP/radix_extra.log"
grep -F \
  '[ast-radix] function=sort_by_digits_extra capacity=64 args=5' \
  "$TMP/radix_extra.log"
for mode in default skip; do
  link_case "radix_extra.$mode"
  run_case "radix_extra.$mode" ''
done
diff -u "$TMP/radix_extra.skip.out" "$TMP/radix_extra.default.out"

compile_case radix_extra_used.default \
  "$ROOT/tests/riscv/radix_sort_extra_scalar_negative.sy"
compile_case radix_extra_used.skip \
  "$ROOT/tests/riscv/radix_sort_extra_scalar_negative.sy" \
  SYC_SKIP_AST_RADIX_SORT_LOWERING=1
cmp "$TMP/radix_extra_used.default.s" "$TMP/radix_extra_used.skip.s"

for radix in base2 base16; do
  compile_case "radix.$radix.default" \
    "$ROOT/tests/riscv/radix_sort_${radix}_positive.sy"
  compile_case "radix.$radix.skip" \
    "$ROOT/tests/riscv/radix_sort_${radix}_positive.sy" \
    SYC_SKIP_AST_RADIX_SORT_LOWERING=1
  SYC_TRACE_AST_RADIX_SORT_LOWERING=1 "$COMPILER" -S -O1 \
    -o "$TMP/radix.$radix.trace.s" \
    "$ROOT/tests/riscv/radix_sort_${radix}_positive.sy" \
    2>"$TMP/radix.$radix.log"
  capacity=16
  test "$radix" = base16 && capacity=32
  grep -F \
    "[ast-radix] function=sort_by_digits_${radix} capacity=$capacity args=5" \
    "$TMP/radix.$radix.log"
  for mode in default skip; do
    link_case "radix.$radix.$mode"
    run_case "radix.$radix.$mode" ''
  done
  diff -u "$TMP/radix.$radix.skip.out" \
    "$TMP/radix.$radix.default.out"
done

compile_case radix.base8_multi.default \
  "$ROOT/tests/riscv/radix_sort_base8_multi_positive.sy"
compile_case radix.base8_multi.skip \
  "$ROOT/tests/riscv/radix_sort_base8_multi_positive.sy" \
  SYC_SKIP_AST_RADIX_SORT_LOWERING=1
SYC_TRACE_AST_RADIX_SORT_LOWERING=1 "$COMPILER" -S -O1 \
  -o "$TMP/radix.base8_multi.trace.s" \
  "$ROOT/tests/riscv/radix_sort_base8_multi_positive.sy" \
  2>"$TMP/radix.base8_multi.log"
grep -F \
  '[ast-radix] function=sort_values8 capacity=128 args=6' \
  "$TMP/radix.base8_multi.log"
for mode in default skip; do
  link_case "radix.base8_multi.$mode"
  run_case "radix.base8_multi.$mode" ''
done
diff -u "$TMP/radix.base8_multi.skip.out" \
  "$TMP/radix.base8_multi.default.out"
fi

# 尾递归在 AST 与 CoreIR 层各有独立开关，正例可据此归因到实际层级；
# 同时关闭两层变换后，近似但不满足条件的负例保持不变。
compile_case tail_scalar.default \
  "$ROOT/tests/riscv/tail_recursion_scalar_positive.sy"
compile_case tail_scalar.skip \
  "$ROOT/tests/riscv/tail_recursion_scalar_positive.sy" \
  SYC_SKIP_AST_TAIL_RECURSION=1 SYC_SKIP_COREIR_TAIL_RECURSION=1
if cmp -s "$TMP/tail_scalar.default.s" "$TMP/tail_scalar.skip.s"; then
  echo "AST tail recursion positive did not trigger" >&2
  exit 1
fi
SYC_TRACE_AST_TAIL_RECURSION=1 "$COMPILER" -S -O1 \
  -o "$TMP/tail_scalar.trace.s" \
  "$ROOT/tests/riscv/tail_recursion_scalar_positive.sy" \
  2>"$TMP/tail_scalar.log"
grep -F \
  '[ast-tail-recursion] function=walk_depth params=3 calls=1' \
  "$TMP/tail_scalar.log"
SYC_SKIP_AST_TAIL_RECURSION=1 SYC_TRACE_COREIR_TAIL_RECURSION=1 \
  "$COMPILER" -S -O1 -o "$TMP/tail_scalar.coreir.s" \
  "$ROOT/tests/riscv/tail_recursion_scalar_positive.sy" \
  2>"$TMP/tail_scalar.coreir.log"
grep -F '[coreir-tail-recursion] function=walk_depth' \
  "$TMP/tail_scalar.coreir.log"
for mode in default skip; do
  link_case "tail_scalar.$mode"
  run_case "tail_scalar.$mode" ''
done
diff -u "$TMP/tail_scalar.default.out" "$TMP/tail_scalar.skip.out"

compile_case tail_multi.default \
  "$ROOT/tests/riscv/tail_recursion_multi_edge_positive.sy"
compile_case tail_multi.skip \
  "$ROOT/tests/riscv/tail_recursion_multi_edge_positive.sy" \
  SYC_SKIP_AST_TAIL_RECURSION=1 SYC_SKIP_COREIR_TAIL_RECURSION=1
if cmp -s "$TMP/tail_multi.default.s" "$TMP/tail_multi.skip.s"; then
  echo "multi-edge tail recursion positive did not trigger" >&2
  exit 1
fi
SYC_TRACE_AST_TAIL_RECURSION=1 "$COMPILER" -S -O1 \
  -o "$TMP/tail_multi.trace.s" \
  "$ROOT/tests/riscv/tail_recursion_multi_edge_positive.sy" \
  2>"$TMP/tail_multi.log"
grep -F \
  '[ast-tail-recursion] function=walk_edges params=2 calls=2' \
  "$TMP/tail_multi.log"
for mode in default skip; do
  link_case "tail_multi.$mode"
  run_case "tail_multi.$mode" ''
done
diff -u "$TMP/tail_multi.default.out" "$TMP/tail_multi.skip.out"

compile_case tail_array.default \
  "$ROOT/tests/riscv/tail_recursion_coreir_array_positive.sy"
compile_case tail_array.skip \
  "$ROOT/tests/riscv/tail_recursion_coreir_array_positive.sy" \
  SYC_SKIP_COREIR_TAIL_RECURSION=1
if cmp -s "$TMP/tail_array.default.s" "$TMP/tail_array.skip.s"; then
  echo "CoreIR tail recursion array positive did not trigger" >&2
  exit 1
fi
SYC_TRACE_COREIR_TAIL_RECURSION=1 "$COMPILER" -S -O1 \
  -o "$TMP/tail_array.trace.s" \
  "$ROOT/tests/riscv/tail_recursion_coreir_array_positive.sy" \
  2>"$TMP/tail_array.log"
grep -F '[coreir-tail-recursion] function=visit_values' \
  "$TMP/tail_array.log"
for mode in default skip; do
  link_case "tail_array.$mode"
  run_case "tail_array.$mode" ''
done
diff -u "$TMP/tail_array.default.out" "$TMP/tail_array.skip.out"

for tail_negative in non_tail loop; do
  compile_case "tail_${tail_negative}.default" \
    "$ROOT/tests/riscv/tail_recursion_${tail_negative}_negative.sy"
  compile_case "tail_${tail_negative}.skip" \
    "$ROOT/tests/riscv/tail_recursion_${tail_negative}_negative.sy" \
    SYC_SKIP_AST_TAIL_RECURSION=1 SYC_SKIP_COREIR_TAIL_RECURSION=1
  cmp "$TMP/tail_${tail_negative}.default.s" \
    "$TMP/tail_${tail_negative}.skip.s"
  SYC_TRACE_AST_TAIL_RECURSION=1 SYC_TRACE_COREIR_TAIL_RECURSION=1 \
    "$COMPILER" -S -O1 \
    -o "$TMP/tail_${tail_negative}.trace.s" \
    "$ROOT/tests/riscv/tail_recursion_${tail_negative}_negative.sy" \
    2>"$TMP/tail_${tail_negative}.log"
  if grep -q 'tail-recursion' "$TMP/tail_${tail_negative}.log"; then
    echo "tail recursion accepted $tail_negative negative" >&2
    exit 1
  fi
  for mode in default skip; do
    link_case "tail_${tail_negative}.$mode"
    run_case "tail_${tail_negative}.$mode" ''
  done
  diff -u "$TMP/tail_${tail_negative}.default.out" \
    "$TMP/tail_${tail_negative}.skip.out"
done

compile_case radix_near.default \
  "$ROOT/tests/riscv/radix_sort_near_miss_negative.sy"
compile_case radix_near.skip \
  "$ROOT/tests/riscv/radix_sort_near_miss_negative.sy" \
  SYC_SKIP_AST_RADIX_SORT_LOWERING=1
cmp "$TMP/radix_near.default.s" "$TMP/radix_near.skip.s"
SYC_TRACE_AST_RADIX_SORT_LOWERING=1 "$COMPILER" -S -O1 \
  -o "$TMP/radix_near.trace.s" \
  "$ROOT/tests/riscv/radix_sort_near_miss_negative.sy" \
  2>"$TMP/radix_near.log"
if grep -q '\[ast-radix\]' "$TMP/radix_near.log"; then
  echo "radix near miss was incorrectly accepted" >&2
  exit 1
fi
for mode in default skip; do
  link_case "radix_near.$mode"
  run_case "radix_near.$mode" ''
  test "$(tr -d '\r\n' <"$TMP/radix_near.$mode.out")" = "0"
done

# 整体替换不得丢弃候选函数体中额外的可观察操作。
compile_case radix_body_effect.default \
  "$ROOT/tests/riscv/radix_sort_body_effect_negative.sy"
compile_case radix_body_effect.skip \
  "$ROOT/tests/riscv/radix_sort_body_effect_negative.sy" \
  SYC_SKIP_AST_RADIX_SORT_LOWERING=1
cmp "$TMP/radix_body_effect.default.s" \
  "$TMP/radix_body_effect.skip.s"
SYC_TRACE_AST_RADIX_SORT_LOWERING=1 "$COMPILER" -S -O1 \
  -o "$TMP/radix_body_effect.trace.s" \
  "$ROOT/tests/riscv/radix_sort_body_effect_negative.sy" \
  2>"$TMP/radix_body_effect.log"
if grep -q '\[ast-radix\]' "$TMP/radix_body_effect.log"; then
  echo "radix body effect was incorrectly accepted" >&2
  exit 1
fi
for mode in default skip; do
  link_case "radix_body_effect.$mode"
  run_case "radix_body_effect.$mode" ''
done
diff -u "$TMP/radix_body_effect.skip.out" \
  "$TMP/radix_body_effect.default.out"

# 动态步长正例必须真实触发并改变汇编，运行结果与关闭 loop-unroll 的
# 参照版本保持一致。
compile_case stride_positive.default \
  "$ROOT/tests/riscv/dynamic_stride_simple_positive.sy"
compile_case stride_positive.skip \
  "$ROOT/tests/riscv/dynamic_stride_simple_positive.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
if cmp -s "$TMP/stride_positive.default.s" "$TMP/stride_positive.skip.s"; then
  echo "dynamic simple-stride optimization did not trigger" >&2
  exit 1
fi
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/stride_positive.trace.s" \
  "$ROOT/tests/riscv/dynamic_stride_simple_positive.sy" \
  2>"$TMP/stride_positive.log"
grep -F 'phase=strided-partial accept factor=2' \
  "$TMP/stride_positive.log"
for mode in default skip; do
  link_case "stride_positive.$mode"
  run_case "stride_positive.$mode" ''
  test "$(tr -d '\r' <"$TMP/stride_positive.$mode.out")" = $'39\n26\n0\n0\n185'
done

compile_case stride_cfg_positive.default \
  "$ROOT/tests/riscv/dynamic_stride_unroll_positive.sy"
compile_case stride_cfg_positive.skip \
  "$ROOT/tests/riscv/dynamic_stride_unroll_positive.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
if cmp -s "$TMP/stride_cfg_positive.default.s" \
  "$TMP/stride_cfg_positive.skip.s"; then
  echo "dynamic CFG-stride optimization did not trigger" >&2
  exit 1
fi
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/stride_cfg_positive.trace.s" \
  "$ROOT/tests/riscv/dynamic_stride_unroll_positive.sy" \
  2>"$TMP/stride_cfg_positive.log"
grep -F 'phase=strided-cfg-partial accept factor=2' \
  "$TMP/stride_cfg_positive.log"
for mode in default skip; do
  link_case "stride_cfg_positive.$mode"
  run_case "stride_cfg_positive.$mode" ''
  test "$(tr -d '\r' <"$TMP/stride_cfg_positive.$mode.out")" = $'42\n28\n0\n0\n150993938'
done

compile_case stride_bound.default \
  "$ROOT/tests/riscv/dynamic_stride_bound_negative.sy"
compile_case stride_bound.skip \
  "$ROOT/tests/riscv/dynamic_stride_bound_negative.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
cmp "$TMP/stride_bound.default.s" "$TMP/stride_bound.skip.s"
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/stride_bound.trace.s" \
  "$ROOT/tests/riscv/dynamic_stride_bound_negative.sy" \
  2>"$TMP/stride_bound.log"
if grep -q 'phase=strided-.*accept' "$TMP/stride_bound.log"; then
  echo "dynamic stride expanded a loop with a mutable bound" >&2
  exit 1
fi
for mode in default skip; do
  link_case "stride_bound.$mode"
  run_case "stride_bound.$mode" ''
  test "$(tr -d '\r\n' <"$TMP/stride_bound.$mode.out")" = "2"
done

compile_case stride.default \
  "$ROOT/tests/riscv/dynamic_stride_unroll_negative.sy"
compile_case stride.skip \
  "$ROOT/tests/riscv/dynamic_stride_unroll_negative.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
for mode in default skip; do
  link_case "stride.$mode"
  run_case "stride.$mode" ''
  test "$(tr -d '\r' <"$TMP/stride.$mode.out")" = $'39\n26\n0\n0\n2147483643'
done

compile_case stride_cfg.default \
  "$ROOT/tests/riscv/dynamic_stride_cfg_unroll_negative.sy"
compile_case stride_cfg.skip \
  "$ROOT/tests/riscv/dynamic_stride_cfg_unroll_negative.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
for mode in default skip; do
  link_case "stride_cfg.$mode"
  run_case "stride_cfg.$mode" ''
  test "$(tr -d '\r' <"$TMP/stride_cfg.$mode.out")" = $'10\n7\n0\n0\n-2147483636'
done

# 多个常量除法和取余可使用动态步长 CFG 二倍展开；只要存在运行时除数，
# 成本模型就保留原循环。两条路径都与关闭循环展开的结果逐字比较。
compile_case stride_const_div.default \
  "$ROOT/tests/riscv/dynamic_stride_constant_div_cfg_positive.sy"
compile_case stride_const_div.skip \
  "$ROOT/tests/riscv/dynamic_stride_constant_div_cfg_positive.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/stride_const_div.trace.s" \
  "$ROOT/tests/riscv/dynamic_stride_constant_div_cfg_positive.sy" \
  2>"$TMP/stride_const_div.log"
grep -F 'phase=strided-cfg-partial accept factor=2' \
  "$TMP/stride_const_div.log"
for mode in default skip; do
  link_case "stride_const_div.$mode"
  run_case "stride_const_div.$mode" ''
done
cmp "$TMP/stride_const_div.default.out" \
  "$TMP/stride_const_div.skip.out"
test "$(tr -d '\r' <"$TMP/stride_const_div.default.out")" = \
  $'191\n161\n0'

compile_case stride_runtime_div.default \
  "$ROOT/tests/riscv/dynamic_stride_runtime_div_cfg_negative.sy"
compile_case stride_runtime_div.skip \
  "$ROOT/tests/riscv/dynamic_stride_runtime_div_cfg_negative.sy" \
  SYC_SKIP_HIR_LOOP_UNROLL=1
cmp "$TMP/stride_runtime_div.default.s" \
  "$TMP/stride_runtime_div.skip.s"
SYC_TRACE_LOOP_UNROLL=1 "$COMPILER" -S -O1 \
  -o "$TMP/stride_runtime_div.trace.s" \
  "$ROOT/tests/riscv/dynamic_stride_runtime_div_cfg_negative.sy" \
  2>"$TMP/stride_runtime_div.log"
grep -F 'phase=strided-cfg-partial reject=latency-bound-div-rem' \
  "$TMP/stride_runtime_div.log"
for mode in default skip; do
  link_case "stride_runtime_div.$mode"
  run_case "stride_runtime_div.$mode" $'7\n'
done
cmp "$TMP/stride_runtime_div.default.out" \
  "$TMP/stride_runtime_div.skip.out"
test "$(tr -d '\r' <"$TMP/stride_runtime_div.default.out")" = \
  $'191\n161'

# 循环不变除数快路径由运行时生成的除数描述符决定。用默认/关闭优化
# 两套汇编对照正负除数、幂次和整数边界，确认直接路径触发且其它值回退。
SYC_TRACE_COREIR_DIVISOR_SPECIALIZATION=1 \
  "$COMPILER" -S -O1 -o "$TMP/runtime_divisor.default.s" \
  "$ROOT/tests/riscv/runtime_invariant_divisor.sy" \
  2>"$TMP/runtime_divisor.log"
env SYC_SKIP_COREIR_DIVISOR_SPECIALIZATION=1 \
  "$COMPILER" -S -O1 -o "$TMP/runtime_divisor.skip.s" \
  "$ROOT/tests/riscv/runtime_invariant_divisor.sy"
grep -m 1 -F 'reason=runtime-direct-proof' "$TMP/runtime_divisor.log"
grep -m 1 -F 'reason=contains-subloop' "$TMP/runtime_divisor.log"
grep -m 1 -F 'direct-invariant-div.mulhi' "$TMP/runtime_divisor.default.s"
if cmp -s "$TMP/runtime_divisor.default.s" "$TMP/runtime_divisor.skip.s"; then
  echo "runtime invariant divisor specialization did not change assembly" >&2
  exit 1
fi
for mode in default skip; do
  link_case "runtime_divisor.$mode"
done
divisor_values=(
  -2147483648 -31 -17 -16 -9 -8 -7 -5 -3 -2 -1
  1 2 3 4 5 7 8 9 15 16 17 31 64 2147483647
)
dividend_values='20 0 1 -1 2 -2 3 -3 7 -7 31 -31 255 -255 1024 -1024 65535 -65535 2147483647 -2147483647 123456789'
divisor_index=0
for divisor in "${divisor_values[@]}"; do
  printf -v divisor_input '%s\n%s\n' "$divisor" "$dividend_values"
  run_case_as "runtime_divisor.default" \
    "runtime_divisor.default.$divisor_index" "$divisor_input"
  run_case_as "runtime_divisor.skip" \
    "runtime_divisor.skip.$divisor_index" "$divisor_input"
  cmp "$TMP/runtime_divisor.default.$divisor_index.out" \
      "$TMP/runtime_divisor.skip.$divisor_index.out"
  divisor_index=$((divisor_index + 1))
done

if [[ ! -f "$PERFORMANCE_ROOT/crc1.sy" ||
      ! -f "$PERFORMANCE_ROOT/conv2d-1.sy" ||
      ! -f "$PERFORMANCE_ROOT/sl1.sy" ]]; then
  echo "riscv compliance risks: PASS (semantic fixtures; set PERFORMANCE_ROOT for performance trace checks)"
  exit 0
fi

BIT_PATTERN_TRACE=1 "$COMPILER" -S -O1 -o "$TMP/crc1.s" \
  "$PERFORMANCE_ROOT/crc1.sy" 2>"$TMP/crc1.log"
grep -F '[bit-pattern] accept-helper _and and' "$TMP/crc1.log"
grep -F '[bit-pattern] accept-helper _xor xor' "$TMP/crc1.log"

SYC_ENABLE_COREIR_STAGE_FUSION=1 SYC_ENABLE_COREIR_STENCIL_INTERIOR=1 \
  SYC_TRACE_STAGE_FUSION=1 SYC_TRACE_COREIR_STENCIL=1 \
  SYC_TRACE_TRANSACTIONAL_PASSES=1 \
  "$COMPILER" -S -O1 -o "$TMP/conv2d-1.s" \
  "$PERFORMANCE_ROOT/conv2d-1.sy" 2>"$TMP/conv2d-1.log"
grep -F 'pass=stage-fusion reason=candidate-transform-rejected' \
  "$TMP/conv2d-1.log"
grep -F 'pass=stencil-interior reason=candidate-transform-rejected' \
  "$TMP/conv2d-1.log"
grep -F 'reason=load-coeff' "$TMP/conv2d-1.log"

SYC_TRACE_HIR_LOCAL_STORE_FORWARD=1 \
  "$COMPILER" -S -O1 -o "$TMP/sl1.s" \
  "$PERFORMANCE_ROOT/sl1.sy" 2>"$TMP/sl1.log"
grep -m 1 -F '[local-store-load-forward] func=main' "$TMP/sl1.log"
grep -m 1 -F ' forwarded' "$TMP/sl1.log"

echo "riscv compliance risks: PASS"
