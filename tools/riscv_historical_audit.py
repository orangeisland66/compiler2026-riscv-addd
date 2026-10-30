#!/usr/bin/env python3
"""RISC-V historical 2020-2025 correctness and GCC/Clang -O2 audit.

The input is the canonical NSCSCC catalog.  A case is identified by source,
input, and expected-output hashes, so repeated yearly copies are not measured
twice.  The compiler under test is always invoked with -O1; GCC and Clang are
only reference baselines and never provide the oracle output.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import re
import statistics
import subprocess
import sys
import time
from pathlib import Path


ARCH_FLAGS = ["-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany"]
INSN_RE = re.compile(r"^\s+[A-Za-z][A-Za-z0-9.]*\s")
CONST_INT_RE = re.compile(r"^(\s*)const\s+int\s+([^;]+);(.*)$")
IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
TIMER_RE = re.compile(rb"Timer@[^:\r\n]*:\s*([0-9]+)(ns|us|ms)")


def command(cmd: list[str], timeout: int, stdin: bytes | None = None):
    started = time.perf_counter()
    try:
        proc = subprocess.run(
            cmd,
            input=stdin,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return None, (time.perf_counter() - started) * 1000.0
    return proc, (time.perf_counter() - started) * 1000.0


def norm_lines(data: bytes) -> list[str]:
    return [line.rstrip() for line in data.decode(errors="replace").strip().splitlines()]


def output_ok(proc, expected: Path) -> bool:
    if proc is None:
        return False
    # NSCSCC .out files contain program stdout followed by the process return
    # value; the compiler itself does not print that return value.
    text = proc.stdout.decode(errors="replace")
    if text and not text.endswith("\n"):
        text += "\n"
    text += f"{proc.returncode}\n"
    actual = [line.rstrip() for line in text.strip().splitlines()]
    return actual == norm_lines(expected.read_bytes())


def internal_time_us(stderr: bytes) -> float | None:
    total_us = 0.0
    found = False
    for raw_value, raw_unit in TIMER_RE.findall(stderr):
        found = True
        value = int(raw_value)
        if raw_unit == b"ns":
            total_us += value / 1000.0
        elif raw_unit == b"ms":
            total_us += value * 1000.0
        else:
            total_us += value
    return total_us if found else None


def filtered_average(
    samples: list[float], relative_threshold: float = 0.10
) -> tuple[float | None, float | None, float | None, list[float]]:
    if not samples:
        return None, None, None, []
    median = statistics.median(samples)
    outliers = (
        [
            sample
            for sample in samples
            if abs(sample - median) / median > relative_threshold
        ]
        if median
        else []
    )
    accepted = [sample for sample in samples if sample not in outliers]
    if not accepted:
        accepted = samples
    return (
        statistics.fmean(accepted),
        statistics.fmean(samples),
        median,
        outliers,
    )


def split_const_items(text: str) -> list[tuple[str, str]] | None:
    items: list[tuple[str, str]] = []
    for part in text.split(","):
        if "=" not in part:
            return None
        name, expr = (piece.strip() for piece in part.split("=", 1))
        if not IDENT_RE.fullmatch(name) or not expr:
            return None
        items.append((name, expr))
    return items


def gcc_compatible_source(source: Path, destination: Path) -> None:
    lines: list[str] = []
    for line in source.read_text(encoding="utf-8", errors="replace").splitlines():
        match = CONST_INT_RE.match(line)
        if not match:
            lines.append(line)
            continue
        indent, declarations, suffix = match.groups()
        items = split_const_items(declarations)
        if not items:
            lines.append(line)
            continue
        body = ", ".join(f"{name} = {expr}" for name, expr in items)
        lines.append(f"{indent}enum {{ {body} }};{suffix}")
    destination.write_text("\n".join(lines) + "\n", encoding="utf-8")


def asm_stats(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    instructions = 0
    for line in data.decode(errors="replace").splitlines():
        if INSN_RE.match(line) and not line.lstrip().startswith((".", "#")):
            instructions += 1
    return len(data), instructions


def load_cases(workspace: Path, category: str, year: str) -> list[dict[str, str]]:
    catalog = workspace / "data/corpus/nscscc/historical/2020-2025/catalog/all-cases.csv"
    with catalog.open(encoding="utf-8-sig", newline="") as stream:
        rows = list(csv.DictReader(stream))
    rows = [row for row in rows if row["category"] in ("functional", "performance")]
    if category != "all":
        rows = [row for row in rows if row["category"] == category]
    if year:
        rows = [row for row in rows if year in row["years_seen"].split(";")]
    return rows


def paths(workspace: Path, row: dict[str, str]) -> tuple[Path, Path | None, Path]:
    source = workspace / row["source_path"]
    input_path = workspace / row["input_path"] if row["input_path"] else None
    expected = workspace / row["expected_path"]
    return source, input_path, expected


def build_ours(args, source: Path, work: Path, runtime: Path) -> dict[str, object]:
    asm = work / "ours.s"
    elf = work / "ours.elf"
    proc, compile_ms = command([str(args.compiler), "-S", "-O1", "-o", str(asm), str(source)], args.compile_timeout)
    result: dict[str, object] = {"status": "COMPILE_TIMEOUT" if proc is None else "COMPILE_ERR" if proc.returncode else "OK", "compile_ms": compile_ms}
    if proc is None or proc.returncode:
        result["message"] = (proc.stderr.decode(errors="replace")[-300:] if proc else "timeout").replace("\n", " ")
        return result
    link, link_ms = command([args.gcc, *ARCH_FLAGS, "-static", "-o", str(elf), str(asm), str(runtime), "-lm"], args.link_timeout)
    result["link_ms"] = link_ms
    if link is None or link.returncode:
        result["status"] = "LINK_TIMEOUT" if link is None else "LINK_ERR"
        result["message"] = (link.stderr.decode(errors="replace")[-300:] if link else "timeout").replace("\n", " ")
        return result
    result["asm_bytes"], result["asm_insns"] = asm_stats(asm)
    return result


def build_reference(args, compiler: str, source: Path, work: Path, runtime: Path, header: Path) -> dict[str, object]:
    compat = work / "source.c"
    gcc_compatible_source(source, compat)
    asm = work / f"{compiler}.s"
    elf = work / f"{compiler}.elf"
    if compiler == "gcc":
        compile_cmd = [args.gcc, *ARCH_FLAGS, "-O2", "-std=gnu11", "-w", "-include", str(header), "-S", "-o", str(asm), str(compat)]
    else:
        compile_cmd = [args.clang, "--target=riscv64-linux-gnu", *ARCH_FLAGS, "-O2", "-fno-addrsig", "-std=gnu11", "-w", "-include", str(header), "-S", "-o", str(asm), str(compat)]
    proc, compile_ms = command(compile_cmd, args.reference_timeout)
    result: dict[str, object] = {"status": "COMPILE_TIMEOUT" if proc is None else "COMPILE_ERR" if proc.returncode else "OK", "compile_ms": compile_ms}
    if proc is None or proc.returncode:
        result["message"] = (proc.stderr.decode(errors="replace")[-300:] if proc else "timeout").replace("\n", " ")
        return result
    link, link_ms = command([args.gcc, *ARCH_FLAGS, "-static", "-o", str(elf), str(asm), str(runtime), "-lm"], args.link_timeout)
    result["link_ms"] = link_ms
    if link is None or link.returncode:
        result["status"] = "LINK_TIMEOUT" if link is None else "LINK_ERR"
        result["message"] = (link.stderr.decode(errors="replace")[-300:] if link else "timeout").replace("\n", " ")
        return result
    result["asm_bytes"], result["asm_insns"] = asm_stats(asm)
    return result


def run_elf(args, elf: Path, source: Path, expected: Path | None, count: int) -> dict[str, object]:
    input_path = source.with_suffix(".in")
    stdin = input_path.read_bytes() if input_path.is_file() else None
    samples: list[float] = []
    internal_samples_us: list[float] = []
    last = None
    for _ in range(count):
        last, elapsed = command([args.qemu, str(elf)], args.run_timeout, stdin)
        samples.append(elapsed)
        if last is None:
            return {"status": "RUN_TIMEOUT", "median_ms": "", "message": "timeout"}
        if expected is not None and not output_ok(last, expected):
            return {"status": "WRONG_OUTPUT", "median_ms": statistics.median(samples), "message": "stdout mismatch"}
        internal_us = internal_time_us(last.stderr)
        if internal_us is not None:
            internal_samples_us.append(internal_us)
    internal_average_us = internal_raw_average_us = internal_median_us = None
    internal_outliers_us: list[float] = []
    if len(internal_samples_us) == len(samples) and internal_samples_us:
        (
            internal_average_us,
            internal_raw_average_us,
            internal_median_us,
            internal_outliers_us,
        ) = filtered_average(internal_samples_us)
    return {
        "status": "OK",
        "median_ms": statistics.median(samples),
        "samples_ms": samples,
        "average_internal_us": (
            internal_average_us if internal_average_us is not None else ""
        ),
        "raw_average_internal_us": (
            internal_raw_average_us
            if internal_raw_average_us is not None
            else ""
        ),
        "median_internal_us": (
            internal_median_us if internal_median_us is not None else ""
        ),
        "outliers_internal_us": internal_outliers_us,
        "samples_internal_us": internal_samples_us,
        "message": "",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--workspace", type=Path, required=True)
    parser.add_argument("--compiler", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--category", choices=["all", "functional", "performance"], default="all")
    parser.add_argument("--year", default="")
    parser.add_argument("--filter", default="")
    parser.add_argument("--functional-runs", type=int, default=1)
    parser.add_argument("--performance-runs", type=int, default=3)
    parser.add_argument("--max-cases", type=int, default=0)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--clang", default="clang")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    parser.add_argument("--compile-timeout", type=int, default=120)
    parser.add_argument("--reference-timeout", type=int, default=120)
    parser.add_argument("--link-timeout", type=int, default=60)
    parser.add_argument("--run-timeout", type=int, default=300)
    args = parser.parse_args()
    args.workspace = args.workspace.resolve()
    args.compiler = args.compiler.resolve()
    args.runtime = args.runtime.resolve()
    args.out_dir = args.out_dir.resolve()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    header = args.out_dir / "sysy_gcc_compat.h"
    header.write_text("""#pragma once\nint getint(void); int getch(void); int getarray(int a[]);\nvoid putint(int); void putch(int); void putarray(int, int[]);\nfloat getfloat(void); int getfarray(float[]); void putfloat(float); void putfarray(int, float[]);\nvoid _sysy_starttime(int); void _sysy_stoptime(int);\n#define starttime() _sysy_starttime(__LINE__)\n#define stoptime() _sysy_stoptime(__LINE__)\n""", encoding="utf-8")
    rows = load_cases(args.workspace, args.category, args.year)
    if args.filter:
        rows = [row for row in rows if args.filter in row["case_id"] or args.filter in row["representative_label"] or args.filter in row["source_path"]]
    if args.max_cases:
        rows = rows[:args.max_cases]
    fields = ["idx", "case_id", "category", "years", "label", "source_path", "ours_status", "gcc_status", "clang_status", "ours_correct", "gcc_correct", "clang_correct", "ours_internal_us", "gcc_internal_us", "clang_internal_us", "ours_raw_average_us", "gcc_raw_average_us", "clang_raw_average_us", "ours_outliers_us", "gcc_outliers_us", "clang_outliers_us", "ours_median_ms", "gcc_median_ms", "clang_median_ms", "ours_vs_gcc", "ours_vs_clang", "ours_asm_bytes", "gcc_asm_bytes", "clang_asm_bytes", "ours_asm_insns", "gcc_asm_insns", "clang_asm_insns", "message"]
    csv_path = args.out_dir / "results.csv"
    summary = {"total": len(rows), "ours_correct": 0, "gcc_correct": 0, "clang_correct": 0, "both_baselines_build": 0, "performance": {"count": 0, "ours_vs_gcc_geomean": None, "ours_vs_clang_geomean": None, "ours_vs_gcc_ratios": [], "ours_vs_clang_ratios": []}}
    with csv_path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for idx, row in enumerate(rows, 1):
            source, input_path, expected = paths(args.workspace, row)
            work = args.out_dir / "work" / row["case_id"]
            work.mkdir(parents=True, exist_ok=True)
            ours = build_ours(args, source, work, args.runtime)
            # build_* uses source-side .in; use the canonical path for aliases with
            # a different representative filename by placing a temporary input link.
            if input_path and input_path != source.with_suffix(".in"):
                target = source.with_suffix(".in")
                if not target.exists():
                    target.symlink_to(input_path)
            count = args.performance_runs if row["category"] == "performance" else args.functional_runs
            ours["runs"] = run_elf(args, work / "ours.elf", source, expected, count) if ours["status"] == "OK" else {}
            if row["category"] == "performance":
                gcc = build_reference(args, "gcc", source, work, args.runtime, header)
                gcc["runs"] = run_elf(args, work / "gcc.elf", source, expected, count) if gcc["status"] == "OK" else {}
                clang = build_reference(args, "clang", source, work, args.runtime, header)
                clang["runs"] = run_elf(args, work / "clang.elf", source, expected, count) if clang["status"] == "OK" else {}
            else:
                gcc = {"status": "SKIPPED", "runs": {"status": "SKIPPED"}}
                clang = {"status": "SKIPPED", "runs": {"status": "SKIPPED"}}
            ours_ok = ours.get("runs", {}).get("status") == "OK"
            gcc_ok = gcc.get("runs", {}).get("status") == "OK"
            clang_ok = clang.get("runs", {}).get("status") == "OK"
            summary["ours_correct"] += int(ours_ok)
            summary["gcc_correct"] += int(gcc_ok)
            summary["clang_correct"] += int(clang_ok)
            ours_ms, gcc_ms, clang_ms = (ours.get("runs", {}).get("median_ms"), gcc.get("runs", {}).get("median_ms"), clang.get("runs", {}).get("median_ms"))
            ours_us, gcc_us, clang_us = (ours.get("runs", {}).get("average_internal_us"), gcc.get("runs", {}).get("average_internal_us"), clang.get("runs", {}).get("average_internal_us"))
            ratio_gcc = ours_us / gcc_us if isinstance(ours_us, (int, float)) and isinstance(gcc_us, (int, float)) and gcc_us else ""
            ratio_clang = ours_us / clang_us if isinstance(ours_us, (int, float)) and isinstance(clang_us, (int, float)) and clang_us else ""
            if row["category"] == "performance" and ours_ok and gcc_ok and clang_ok and ratio_gcc and ratio_clang:
                summary["performance"]["count"] += 1
                summary["performance"]["ours_vs_gcc_ratios"].append(ratio_gcc)
                summary["performance"]["ours_vs_clang_ratios"].append(ratio_clang)
            writer.writerow({"idx": idx, "case_id": row["case_id"], "category": row["category"], "years": row["years_seen"], "label": row["representative_label"], "source_path": row["source_path"], "ours_status": ours.get("status", "") + "/" + ours.get("runs", {}).get("status", ""), "gcc_status": gcc.get("status", "") + "/" + gcc.get("runs", {}).get("status", ""), "clang_status": clang.get("status", "") + "/" + clang.get("runs", {}).get("status", ""), "ours_correct": int(ours_ok), "gcc_correct": int(gcc_ok), "clang_correct": int(clang_ok), "ours_internal_us": ours_us or "", "gcc_internal_us": gcc_us or "", "clang_internal_us": clang_us or "", "ours_raw_average_us": ours.get("runs", {}).get("raw_average_internal_us", ""), "gcc_raw_average_us": gcc.get("runs", {}).get("raw_average_internal_us", ""), "clang_raw_average_us": clang.get("runs", {}).get("raw_average_internal_us", ""), "ours_outliers_us": ";".join(str(value) for value in ours.get("runs", {}).get("outliers_internal_us", [])), "gcc_outliers_us": ";".join(str(value) for value in gcc.get("runs", {}).get("outliers_internal_us", [])), "clang_outliers_us": ";".join(str(value) for value in clang.get("runs", {}).get("outliers_internal_us", [])), "ours_median_ms": ours_ms or "", "gcc_median_ms": gcc_ms or "", "clang_median_ms": clang_ms or "", "ours_vs_gcc": ratio_gcc, "ours_vs_clang": ratio_clang, "ours_asm_bytes": ours.get("asm_bytes", ""), "gcc_asm_bytes": gcc.get("asm_bytes", ""), "clang_asm_bytes": clang.get("asm_bytes", ""), "ours_asm_insns": ours.get("asm_insns", ""), "gcc_asm_insns": gcc.get("asm_insns", ""), "clang_asm_insns": clang.get("asm_insns", ""), "message": "; ".join(str(x.get("message", "")) for x in (ours, gcc, clang) if x.get("message"))[:500]})
            if idx == 1 or idx % 25 == 0 or not (ours_ok and gcc_ok and clang_ok):
                print(f"PROGRESS {idx}/{len(rows)} ours={summary['ours_correct']} gcc={summary['gcc_correct']} clang={summary['clang_correct']} case={row['representative_label']}", flush=True)
    performance_summary = summary["performance"]
    for key in ("ours_vs_gcc_ratios", "ours_vs_clang_ratios"):
        ratios = performance_summary[key]
        output_key = key.replace("_ratios", "_geomean")
        performance_summary[output_key] = math.exp(sum(math.log(value) for value in ratios) / len(ratios)) if ratios else None
        del performance_summary[key]
    (args.out_dir / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"CSV {csv_path}")
    print(f"SUMMARY {args.out_dir / 'summary.json'}")
    return 0 if summary["ours_correct"] == len(rows) else 1


if __name__ == "__main__":
    raise SystemExit(main())
