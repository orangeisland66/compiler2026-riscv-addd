#!/usr/bin/env python3
"""Run RISC-V performance cases and report compile/link/runtime separately."""

from __future__ import annotations

import argparse
import concurrent.futures
import csv
import json
import shutil
import statistics
import time
from pathlib import Path

from perf_common import (
    case_family,
    case_key,
    find_cases,
    internal_time_us,
    normalized,
    run_cmd,
    same_output,
)


ARCH = ["-march=rv64gc", "-mcmodel=medany"]


def run_one(args: argparse.Namespace, case: Path) -> dict[str, object]:
    rel_key = "_".join(case.with_suffix("").parts[-2:])
    work_dir = args.work_dir / rel_key
    work_dir.mkdir(parents=True, exist_ok=True)
    asm = work_dir / f"{case.stem}.s"
    elf = work_dir / f"{case.stem}.elf"
    expected = case.with_suffix(".out")
    input_file = case.with_suffix(".in")

    compile_command = [
        str(args.compiler),
        "-S",
        "-O1",
        "-o",
        str(asm),
        str(case),
    ]
    compile_result, compile_ms = run_cmd(
        compile_command, timeout=args.compile_timeout
    )
    common: dict[str, object] = {
        "case": case_key(case),
        "family": case_family(case),
        "compile_ms": compile_ms,
        "link_ms": 0.0,
        "run_ms": 0.0,
        "internal_us": "",
        "message": "",
    }
    if compile_result is None:
        return {**common, "status": "COMPILE_TIMEOUT"}
    if compile_result.returncode:
        return {
            **common,
            "status": "COMPILE_ERR",
            "message": compile_result.stderr.decode(errors="replace")[-500:],
        }

    link_result, link_ms = run_cmd(
        [
            args.gcc,
            *ARCH,
            "-static",
            "-o",
            str(elf),
            str(asm),
            str(args.runtime),
            "-lm",
        ],
        timeout=args.link_timeout,
    )
    common["link_ms"] = link_ms
    if link_result is None:
        return {**common, "status": "LINK_TIMEOUT"}
    if link_result.returncode:
        return {
            **common,
            "status": "LINK_ERR",
            "message": link_result.stderr.decode(errors="replace")[-500:],
        }

    stdin_data = input_file.read_bytes() if input_file.exists() else None
    run_samples: list[float] = []
    internal_samples: list[float] = []
    for _ in range(args.runs):
        run_result, run_ms = run_cmd(
            [args.qemu, str(elf)],
            timeout=args.run_timeout,
            stdin_data=stdin_data,
        )
        if run_result is None:
            common["run_ms"] = statistics.median(run_samples) if run_samples else run_ms
            return {**common, "status": "RUN_TIMEOUT"}
        run_samples.append(run_ms)
        actual = normalized(run_result.stdout, run_result.returncode)
        if not expected.exists():
            return {**common, "run_ms": run_ms, "status": "MISSING_OUT"}
        if not same_output(actual, expected):
            return {
                **common,
                "run_ms": run_ms,
                "status": "WRONG",
                "message": actual[:500],
            }
        measured = internal_time_us(run_result.stderr)
        if measured is not None:
            internal_samples.append(measured)

    common["run_ms"] = statistics.median(run_samples)
    common["run_samples_ms"] = ";".join(f"{sample:.3f}" for sample in run_samples)
    if len(internal_samples) == len(run_samples):
        common["internal_us"] = statistics.median(internal_samples)
    return {**common, "status": "PASS"}


def write_reports(
    rows: list[dict[str, object]], report: Path, summary: dict[str, object]
) -> tuple[Path, Path, Path]:
    report.parent.mkdir(parents=True, exist_ok=True)
    csv_path = report.with_suffix(".csv")
    json_path = report.with_suffix(".json")
    markdown_path = report.with_suffix(".md")
    fields = [
        "case",
        "family",
        "status",
        "compile_ms",
        "link_ms",
        "run_ms",
        "run_samples_ms",
        "internal_us",
        "message",
    ]
    with csv_path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)
    json_path.write_text(
        json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    with markdown_path.open("w", encoding="utf-8") as stream:
        stream.write(f"- passed: {summary['passed']}/{summary['total']}\n")
        stream.write(f"- wall_s: {summary['wall_s']:.3f}\n")
        stream.write(f"- total_compile_ms: {summary['total_compile_ms']:.3f}\n")
        stream.write(f"- total_link_ms: {summary['total_link_ms']:.3f}\n")
        stream.write(f"- total_run_ms: {summary['total_run_ms']:.3f}\n")
        stream.write(f"- total_internal_us: {summary['total_internal_us']:.3f}\n")
        stream.write(f"- runs: {summary['runs']}\n")
        stream.write(f"- jobs: {summary['jobs']}\n\n")
        stream.write("| case | status | compile_ms | link_ms | run_ms | internal_us |\n")
        stream.write("|---|---:|---:|---:|---:|---:|\n")
        for row in rows:
            stream.write(
                f"| `{row['case']}` | {row['status']} | "
                f"{float(row['compile_ms']):.3f} | {float(row['link_ms']):.3f} | "
                f"{float(row['run_ms']):.3f} | {row['internal_us']} |\n"
            )
    return csv_path, json_path, markdown_path


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run RISC-V performance correctness and timing tests."
    )
    parser.add_argument("--compiler", type=Path, default=Path("./build/compiler"))
    parser.add_argument("--cases", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--work-dir", type=Path, default=Path("./build/perf-work"))
    parser.add_argument(
        "--report", type=Path, default=Path("./build/perf-work/performance_report")
    )
    parser.add_argument("--filter", default="")
    parser.add_argument(
        "--opt",
        action="store_true",
        required=True,
        help="required acknowledgement that performance cases use compiler -O1",
    )
    parser.add_argument("--runs", type=int, default=1)
    parser.add_argument("--jobs", type=int, default=1)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64")
    parser.add_argument("--compile-timeout", type=int, default=180)
    parser.add_argument("--link-timeout", type=int, default=60)
    parser.add_argument("--run-timeout", type=int, default=300)
    args = parser.parse_args()

    for path, label in ((args.compiler, "compiler"), (args.runtime, "runtime")):
        if not path.exists():
            raise SystemExit(f"{label} not found: {path}")
    for executable in (args.gcc, args.qemu):
        if not shutil.which(executable):
            raise SystemExit(f"executable not found: {executable}")
    if args.runs <= 0 or args.jobs <= 0:
        raise SystemExit("--runs and --jobs must be positive")

    selected = find_cases(args.cases)
    if args.filter:
        selected = [case for case in selected if args.filter in str(case)]
    if not selected:
        raise SystemExit(f"no .sy cases found under {args.cases}")
    args.work_dir.mkdir(parents=True, exist_ok=True)

    started = time.perf_counter()
    if args.jobs == 1:
        rows = [run_one(args, case) for case in selected]
    else:
        rows_by_index: list[dict[str, object] | None] = [None] * len(selected)
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
            futures = {
                pool.submit(run_one, args, case): index
                for index, case in enumerate(selected)
            }
            for future in concurrent.futures.as_completed(futures):
                rows_by_index[futures[future]] = future.result()
        rows = [row for row in rows_by_index if row is not None]

    wall_s = time.perf_counter() - started
    summary: dict[str, object] = {
        "compiler": str(args.compiler.resolve()),
        "cases": str(args.cases.resolve()),
        "runtime": str(args.runtime.resolve()),
        "gcc": args.gcc,
        "qemu": args.qemu,
        "passed": sum(row["status"] == "PASS" for row in rows),
        "total": len(rows),
        "wall_s": wall_s,
        "total_compile_ms": sum(float(row["compile_ms"]) for row in rows),
        "total_link_ms": sum(float(row["link_ms"]) for row in rows),
        "total_run_ms": sum(float(row["run_ms"]) for row in rows),
        "total_internal_us": sum(
            float(row["internal_us"]) for row in rows if row["internal_us"] != ""
        ),
        "runs": args.runs,
        "jobs": args.jobs,
    }
    reports = write_reports(rows, args.report, summary)
    for row in rows:
        print(
            f"{row['status']:>16} {row['case']} "
            f"compile={float(row['compile_ms']):.1f}ms "
            f"link={float(row['link_ms']):.1f}ms "
            f"run={float(row['run_ms']):.1f}ms",
            flush=True,
        )
    print(f"\nResult: {summary['passed']}/{summary['total']} passed")
    print(f"Wall_s: {summary['wall_s']:.3f}")
    print(f"Total compile_ms: {summary['total_compile_ms']:.3f}")
    print(f"Total link_ms: {summary['total_link_ms']:.3f}")
    print(f"Total run_ms: {summary['total_run_ms']:.3f}")
    print(f"Total internal_us: {summary['total_internal_us']:.3f}")
    print("Reports: " + " ".join(str(path) for path in reports))
    return 0 if summary["passed"] == summary["total"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
