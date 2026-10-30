#!/usr/bin/env python3
"""Run the fixed RISC-V 140+60 suite against one compiler build.

This runner deliberately never performs git checkout.  It is used for
candidate validation inside an isolated optimization worktree and reuses the
same per-case compiler/link/QEMU and output checks as the commit scanner.
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

from riscv_commit_regression import cases, run_case


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--suite-root", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument(
        "--runtime",
        type=Path,
        required=True,
        help="external SysY runtime archive used for local linking",
    )
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    parser.add_argument("--functional-runs", type=int, default=1)
    parser.add_argument("--performance-runs", type=int, default=1)
    parser.add_argument("--category", choices=["all", "functional", "performance"], default="all")
    parser.add_argument(
        "--filter",
        default="",
        help="comma-separated case-name substrings",
    )
    args = parser.parse_args()

    args.suite_root = args.suite_root.resolve()
    args.build = args.build.resolve()
    if args.runtime is not None:
        args.runtime = args.runtime.resolve()
    args.out_dir = args.out_dir.resolve()
    args.out_dir.mkdir(parents=True, exist_ok=True)

    functional = cases(args.suite_root / "2026初赛RISCV赛道功能用例" / "functional")
    functional += cases(args.suite_root / "2026初赛RISCV赛道功能用例" / "h_functional")
    performance = cases(args.suite_root / "2026初赛RISCV赛道性能用例" / "performance")
    if args.category == "performance":
        functional = []
    elif args.category == "functional":
        performance = []
    if args.filter:
        filters = [item for item in args.filter.split(",") if item]
        functional = [
            case for case in functional if any(item in case.stem for item in filters)
        ]
        performance = [
            case for case in performance if any(item in case.stem for item in filters)
        ]

    compiler = args.build / "compiler"
    runtime = args.runtime
    if not compiler.exists() or not runtime.exists():
        raise SystemExit(f"missing compiler {compiler} or runtime {runtime}")

    rows: list[dict[str, object]] = []
    summary: dict[str, object] = {
        "functional_total": len(functional),
        "performance_total": len(performance),
        "functional_correct": 0,
        "performance_correct": 0,
        "performance_total_ms": 0.0,
        "performance_total_us": 0.0,
    }
    asm_dir = args.out_dir / "asm"
    asm_dir.mkdir(parents=True, exist_ok=True)
    for category, suite, runs, timeout in (
        ("functional", functional, args.functional_runs, 120),
        ("performance", performance, args.performance_runs, 300),
    ):
        for source in suite:
            result = run_case(
                compiler,
                runtime,
                args.qemu,
                args.gcc,
                source,
                asm_dir,
                category == "performance",
                timeout,
                runs,
            )
            ok = result.get("status") == "OK"
            if category == "functional":
                summary["functional_correct"] = int(summary["functional_correct"]) + int(ok)
            else:
                summary["performance_correct"] = int(summary["performance_correct"]) + int(ok)
                if isinstance(result.get("run_ms"), (int, float)):
                    summary["performance_total_ms"] = float(summary["performance_total_ms"]) + float(result["run_ms"])
                if isinstance(result.get("internal_us"), (int, float)):
                    summary["performance_total_us"] = float(summary["performance_total_us"]) + float(result["internal_us"])
            rows.append(
                {
                    "category": category,
                    "case": source.stem,
                    "status": result.get("status", ""),
                    "run_ms": result.get("run_ms", ""),
                    "run_samples": result.get("run_samples", ""),
                    "internal_us": result.get("internal_us", ""),
                    "internal_raw_average_us": result.get(
                        "internal_raw_average_us", ""
                    ),
                    "internal_median_us": result.get("internal_median_us", ""),
                    "internal_outliers_us": result.get(
                        "internal_outliers_us", ""
                    ),
                    "internal_samples_us": result.get("internal_samples_us", ""),
                    "asm_bytes": result.get("asm_bytes", ""),
                    "asm_insns": result.get("asm_insns", ""),
                    "message": result.get("message", ""),
                }
            )

    summary["performance_total_ms"] = round(float(summary["performance_total_ms"]), 3)
    summary["performance_total_us"] = round(float(summary["performance_total_us"]), 3)
    (args.out_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    with (args.out_dir / "cases.csv").open("w", encoding="utf-8", newline="") as stream:
        fields = [
            "category",
            "case",
            "status",
            "run_ms",
            "run_samples",
            "internal_us",
            "internal_raw_average_us",
            "internal_median_us",
            "internal_outliers_us",
            "internal_samples_us",
            "asm_bytes",
            "asm_insns",
            "message",
        ]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)

    print(
        "FUNCTIONAL "
        f"{summary['functional_correct']}/{summary['functional_total']} "
        "PERFORMANCE "
        f"{summary['performance_correct']}/{summary['performance_total']} "
        f"TOTAL_US {summary['performance_total_us']}",
        flush=True,
    )
    return 0 if (
        summary["functional_correct"] == summary["functional_total"]
        and summary["performance_correct"] == summary["performance_total"]
    ) else 1


if __name__ == "__main__":
    raise SystemExit(main())
