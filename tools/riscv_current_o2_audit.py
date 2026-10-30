#!/usr/bin/env python3
"""Compare the current RISC-V performance suite with GCC/Clang O2.

Only samples for which all three programs are correct enter the performance
ranking.  The reference build is intentionally kept separate from the
project compiler and uses the same RISC-V runtime and QEMU.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import statistics
from pathlib import Path
from types import SimpleNamespace

from riscv_historical_audit import build_ours, build_reference, run_elf


HEADER = """#pragma once
int getint(void); int getch(void); int getarray(int a[]);
void putint(int); void putch(int); void putarray(int, int[]);
float getfloat(void); int getfarray(float[]); void putfloat(float); void putfarray(int, float[]);
void _sysy_starttime(int); void _sysy_stoptime(int);
#define starttime() _sysy_starttime(__LINE__)
#define stoptime() _sysy_stoptime(__LINE__)
"""


def geometric_mean(values: list[float]) -> float | None:
    return math.exp(sum(math.log(value) for value in values) / len(values)) if values else None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--suite-root", type=Path, required=True)
    parser.add_argument("--compiler", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--clang", default="clang")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    parser.add_argument("--ours-runs", type=int, default=3)
    parser.add_argument("--reference-runs", type=int, default=3)
    parser.add_argument("--filter", default="")
    args = parser.parse_args()

    args.suite_root = args.suite_root.resolve()
    args.compiler = args.compiler.resolve()
    args.runtime = args.runtime.resolve()
    args.out_dir = args.out_dir.resolve()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    header = args.out_dir / "sysy_gcc_compat.h"
    header.write_text(HEADER, encoding="utf-8")

    sources = sorted(
        (args.suite_root / "2026初赛RISCV赛道性能用例" / "performance").rglob("*.sy")
    )
    if args.filter:
        sources = [source for source in sources if args.filter in source.stem]

    build_args = SimpleNamespace(
        compiler=args.compiler,
        gcc=args.gcc,
        clang=args.clang,
        qemu=args.qemu,
        compile_timeout=120,
        reference_timeout=120,
        link_timeout=60,
        run_timeout=300,
    )
    fields = [
        "case",
        "ours_status",
        "gcc_status",
        "clang_status",
        "ours_us",
        "gcc_us",
        "clang_us",
        "ours_raw_average_us",
        "gcc_raw_average_us",
        "clang_raw_average_us",
        "ours_outliers_us",
        "gcc_outliers_us",
        "clang_outliers_us",
        "ours_ms",
        "gcc_ms",
        "clang_ms",
        "ours_vs_gcc",
        "ours_vs_clang",
        "ours_asm_bytes",
        "gcc_asm_bytes",
        "clang_asm_bytes",
        "ours_asm_insns",
        "gcc_asm_insns",
        "clang_asm_insns",
        "message",
    ]
    rows: list[dict[str, object]] = []
    ours_correct = gcc_correct = clang_correct = 0
    strict_gcc: list[float] = []
    strict_clang: list[float] = []
    asm_dir = args.out_dir / "asm"
    asm_dir.mkdir(parents=True, exist_ok=True)

    for index, source in enumerate(sources, 1):
        key = source.stem + "_" + hashlib.sha1(str(source).encode()).hexdigest()[:12]
        work = args.out_dir / "work" / key
        work.mkdir(parents=True, exist_ok=True)
        ours = build_ours(build_args, source, work, args.runtime)
        ours["runs"] = run_elf(build_args, work / "ours.elf", source, source.with_suffix(".out"), args.ours_runs) if ours["status"] == "OK" else {}
        gcc = build_reference(build_args, "gcc", source, work, args.runtime, header)
        gcc["runs"] = run_elf(build_args, work / "gcc.elf", source, source.with_suffix(".out"), args.reference_runs) if gcc["status"] == "OK" else {}
        clang = build_reference(build_args, "clang", source, work, args.runtime, header)
        clang["runs"] = run_elf(build_args, work / "clang.elf", source, source.with_suffix(".out"), args.reference_runs) if clang["status"] == "OK" else {}

        ours_ok = ours.get("runs", {}).get("status") == "OK"
        gcc_ok = gcc.get("runs", {}).get("status") == "OK"
        clang_ok = clang.get("runs", {}).get("status") == "OK"
        ours_correct += int(ours_ok)
        gcc_correct += int(gcc_ok)
        clang_correct += int(clang_ok)
        ours_ms = ours.get("runs", {}).get("median_ms")
        gcc_ms = gcc.get("runs", {}).get("median_ms")
        clang_ms = clang.get("runs", {}).get("median_ms")
        ours_us = ours.get("runs", {}).get("average_internal_us")
        gcc_us = gcc.get("runs", {}).get("average_internal_us")
        clang_us = clang.get("runs", {}).get("average_internal_us")
        ratio_gcc = ours_us / gcc_us if isinstance(ours_us, (int, float)) and isinstance(gcc_us, (int, float)) and gcc_us else ""
        ratio_clang = ours_us / clang_us if isinstance(ours_us, (int, float)) and isinstance(clang_us, (int, float)) and clang_us else ""
        if ours_ok and gcc_ok and clang_ok and ratio_gcc and ratio_clang:
            strict_gcc.append(float(ratio_gcc))
            strict_clang.append(float(ratio_clang))

        rows.append(
            {
                "case": source.stem,
                "ours_status": ours.get("status", "") + "/" + ours.get("runs", {}).get("status", ""),
                "gcc_status": gcc.get("status", "") + "/" + gcc.get("runs", {}).get("status", ""),
                "clang_status": clang.get("status", "") + "/" + clang.get("runs", {}).get("status", ""),
                "ours_us": ours_us or "",
                "gcc_us": gcc_us or "",
                "clang_us": clang_us or "",
                "ours_raw_average_us": ours.get("runs", {}).get(
                    "raw_average_internal_us", ""
                ),
                "gcc_raw_average_us": gcc.get("runs", {}).get(
                    "raw_average_internal_us", ""
                ),
                "clang_raw_average_us": clang.get("runs", {}).get(
                    "raw_average_internal_us", ""
                ),
                "ours_outliers_us": ";".join(
                    str(value)
                    for value in ours.get("runs", {}).get(
                        "outliers_internal_us", []
                    )
                ),
                "gcc_outliers_us": ";".join(
                    str(value)
                    for value in gcc.get("runs", {}).get(
                        "outliers_internal_us", []
                    )
                ),
                "clang_outliers_us": ";".join(
                    str(value)
                    for value in clang.get("runs", {}).get(
                        "outliers_internal_us", []
                    )
                ),
                "ours_ms": ours_ms or "",
                "gcc_ms": gcc_ms or "",
                "clang_ms": clang_ms or "",
                "ours_vs_gcc": ratio_gcc,
                "ours_vs_clang": ratio_clang,
                "ours_asm_bytes": ours.get("asm_bytes", ""),
                "gcc_asm_bytes": gcc.get("asm_bytes", ""),
                "clang_asm_bytes": clang.get("asm_bytes", ""),
                "ours_asm_insns": ours.get("asm_insns", ""),
                "gcc_asm_insns": gcc.get("asm_insns", ""),
                "clang_asm_insns": clang.get("asm_insns", ""),
                "message": "; ".join(str(item.get("message", "")) for item in (ours, gcc, clang) if item.get("message"))[:500],
            }
        )
        if index == 1 or index % 10 == 0 or not (ours_ok and gcc_ok and clang_ok):
            print(f"PROGRESS {index}/{len(sources)} ours={ours_correct} gcc={gcc_correct} clang={clang_correct} case={source.stem}", flush=True)

    summary = {
        "total": len(rows),
        "ours_correct": ours_correct,
        "gcc_correct": gcc_correct,
        "clang_correct": clang_correct,
        "strict_correct_intersection": len(strict_gcc),
        "ours_vs_gcc_geomean": geometric_mean(strict_gcc),
        "ours_vs_clang_geomean": geometric_mean(strict_clang),
    }
    with (args.out_dir / "results.csv").open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    (args.out_dir / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False), flush=True)
    return 0 if ours_correct == len(rows) else 1


if __name__ == "__main__":
    raise SystemExit(main())
