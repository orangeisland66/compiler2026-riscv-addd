#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
COMPILER="${COMPILER:-$ROOT/build/compiler}"
CASE="${CASE:-$ROOT/tests/riscv/hierarchical_loop_symbol_address_positive.sy}"
REPETITIONS="${REPETITIONS:-6}"
TMP="$(mktemp -d "${TMPDIR:-/tmp}/syc-codegen-determinism.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

if [[ ! -x "$COMPILER" || ! -f "$CASE" ]]; then
  echo "set COMPILER and CASE to an existing compiler and SysY source" >&2
  exit 2
fi
if [[ ! "$REPETITIONS" =~ ^[1-9][0-9]*$ ]]; then
  echo "REPETITIONS must be a positive integer" >&2
  exit 2
fi

reference=""
for ((iteration = 1; iteration <= REPETITIONS; ++iteration)); do
  output="$TMP/run-$iteration.s"
  "$COMPILER" -S -O1 -o "$output" "$CASE"
  digest="$(sha256sum "$output" | awk '{print $1}')"
  if [[ -z "$reference" ]]; then
    reference="$digest"
  elif [[ "$digest" != "$reference" ]]; then
    echo "nondeterministic assembly at run $iteration: $digest != $reference" >&2
    exit 1
  fi
done

echo "rv64 codegen determinism: PASS ($REPETITIONS/$REPETITIONS $reference)"
