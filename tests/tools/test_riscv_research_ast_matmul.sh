#!/usr/bin/env bash
set -euo pipefail
export SYC_RESEARCH=1

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
RESEARCH_COMPILER="${RESEARCH_COMPILER:-$ROOT/build-research/compiler}"
RUNTIME="${RUNTIME:?set RUNTIME to an external SysY runtime archive}"
TMP="${TMPDIR:-/tmp}/syc-research-ast-matmul"
mkdir -p "$TMP"

source="$ROOT/tests/riscv/ast_matmul_interchange_research_positive.sy"

compile_case() {
  local compiler="$1" name="$2"
  shift 2
  env "$@" "$compiler" -S -O1 -o "$TMP/$name.s" "$source" \
    2>"$TMP/$name.log"
}

link_run() {
  local name="$1"
  riscv64-linux-gnu-gcc -march=rv64gc -static -o "$TMP/$name.elf" \
    "$TMP/$name.s" "$RUNTIME" -lm
  timeout 60 qemu-riscv64 "$TMP/$name.elf" >"$TMP/$name.out"
}

compile_case "$RESEARCH_COMPILER" research.default
compile_case "$RESEARCH_COMPILER" research.enable \
  SYC_ENABLE_AST_MATMUL_INTERCHANGE=1 SYC_TRACE_AST_MATMUL_INTERCHANGE=1
grep -F "[ast-matmul] applied" "$TMP/research.enable.log"
if cmp -s "$TMP/research.default.s" "$TMP/research.enable.s"; then
  echo "research AST matmul interchange did not reach its positive fixture" >&2
  exit 1
fi
link_run research.default
link_run research.enable
diff -u "$TMP/research.default.out" "$TMP/research.enable.out"

compile_case "$COMPILER" strict.default
compile_case "$COMPILER" strict.enable \
  SYC_ENABLE_AST_MATMUL_INTERCHANGE=1 SYC_TRACE_AST_MATMUL_INTERCHANGE=1
cmp "$TMP/strict.default.s" "$TMP/strict.enable.s"

echo "research AST matmul interchange: PASS"
