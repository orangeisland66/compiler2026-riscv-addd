#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RESEARCH_COMPILER="${RESEARCH_COMPILER:-$ROOT/build-research/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
TMP="${TMPDIR:-/tmp}/syc-research-if-convert"
mkdir -p "$TMP"

compile_case() {
  local compiler="$1" name="$2" source="$3"
  shift 3
  env "$@" "$compiler" -S -O1 -o "$TMP/$name.s" "$source" \
    2>"$TMP/$name.log"
}

link_case() {
  local name="$1"
  riscv64-linux-gnu-gcc -march=rv64gc -static -o "$TMP/$name.elf" \
    "$TMP/$name.s" "$RUNTIME" -lm
}

run_case() {
  local name="$1"
  timeout 60 qemu-riscv64 "$TMP/$name.elf" >"$TMP/$name.out"
}

positive="$ROOT/tests/riscv/if_convert_accumulation_positive.sy"
negative="$ROOT/tests/riscv/if_convert_accumulation_side_effect_negative.sy"
strict_safe="$ROOT/tests/riscv/if_convert_accumulation_strict_safe_positive.sy"

# direct global 的每个提前 load 都能由三层规范循环的常量范围
# 证明为 in-bounds，因此 strict 默认可转换；research enable 不得再
# 放宽到参数指针、动态上界或可能 i32 回绕的地址。
compile_case "$RESEARCH_COMPILER" positive.default "$positive"
compile_case "$RESEARCH_COMPILER" positive.enable "$positive" \
  SYC_ENABLE_IF_CONVERT_ACCUMULATION=1 SYC_TRACE_IF_CONVERT=1
grep -F "converted+merged" "$TMP/positive.enable.log"
cmp "$TMP/positive.default.s" "$TMP/positive.enable.s"
link_case positive.default
link_case positive.enable
run_case positive.default
run_case positive.enable
diff -u "$TMP/positive.default.out" "$TMP/positive.enable.out"

# 条件分支中的状态写入必须保守拒绝；根据清理后的 IR 形态，拒绝原因可能先
# 命中外部使用闭包，也可能直接命中副作用检查。
compile_case "$RESEARCH_COMPILER" negative.default "$negative"
compile_case "$RESEARCH_COMPILER" negative.enable "$negative" \
  SYC_ENABLE_IF_CONVERT_ACCUMULATION=1 SYC_TRACE_IF_CONVERT=1
if grep -Fq "converted+merged" "$TMP/negative.enable.log"; then
  echo "if-convert accepted a side-effecting arm" >&2
  exit 1
fi
grep -Eq "then block has (external uses|side effects)" \
  "$TMP/negative.enable.log"
cmp "$TMP/negative.default.s" "$TMP/negative.enable.s"
link_case negative.default
link_case negative.enable
run_case negative.default
run_case negative.enable
diff -u "$TMP/negative.default.out" "$TMP/negative.enable.out"

# strict 构建必须忽略 research enable；trace 只输出证明过程。
compile_case "$COMPILER" strict.default "$positive"
compile_case "$COMPILER" strict.enable "$positive" \
  SYC_ENABLE_IF_CONVERT_ACCUMULATION=1 SYC_TRACE_IF_CONVERT=1
cmp "$TMP/strict.default.s" "$TMP/strict.enable.s"

# 同一带 load 正例在 research rollback 中必须可回到原分支，且两路动态等价。
compile_case "$RESEARCH_COMPILER" positive.skip "$positive" \
  SYC_SKIP_IF_CONVERT_ACCUMULATION=1
if cmp -s "$TMP/positive.default.s" "$TMP/positive.skip.s"; then
  echo "bounded-load if-convert positive did not trigger" >&2
  exit 1
fi
link_case positive.skip
run_case positive.skip
diff -u "$TMP/positive.default.out" "$TMP/positive.skip.out"

# strict 只启用不含 load 的可推测子集。invariant-add 夹具必须默认触发；
# strict hostile skip 保持环境无关，research rollback 才可关闭该变换。
compile_case "$COMPILER" strict.safe.default "$strict_safe" \
  SYC_TRACE_IF_CONVERT=1
compile_case "$COMPILER" strict.safe.hostile "$strict_safe" \
  SYC_SKIP_IF_CONVERT_ACCUMULATION=1
grep -F "converted+merged" "$TMP/strict.safe.default.log"
cmp "$TMP/strict.safe.default.s" "$TMP/strict.safe.hostile.s"
compile_case "$RESEARCH_COMPILER" research.safe.default "$strict_safe"
compile_case "$RESEARCH_COMPILER" research.safe.skip "$strict_safe" \
  SYC_SKIP_IF_CONVERT_ACCUMULATION=1
cmp "$TMP/strict.safe.default.s" "$TMP/research.safe.default.s"
if cmp -s "$TMP/research.safe.default.s" "$TMP/research.safe.skip.s"; then
  echo "research rollback did not isolate strict-safe if-convert" >&2
  exit 1
fi
link_case research.safe.default
link_case research.safe.skip
run_case research.safe.default
run_case research.safe.skip
diff -u "$TMP/research.safe.default.out" "$TMP/research.safe.skip.out"

echo "research if-convert: PASS"
