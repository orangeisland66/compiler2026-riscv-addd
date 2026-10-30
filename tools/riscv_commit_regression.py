#!/usr/bin/env python3
"""Run the identical RISC-V 140+60 suite at every commit since a date.

This is an experiment driver, not part of the compiler pipeline.  It checks
out only the supplied worktree, builds with the same CMake/GCC environment,
and records per-case output correctness, QEMU wall time, and assembly size.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import re
import statistics
import subprocess
import time
from pathlib import Path


ARCH = ["-march=rv64gc", "-mcmodel=medany"]
INSN_RE = re.compile(r"^\s+[A-Za-z][A-Za-z0-9.]*\s")
TIMER_RE = re.compile(rb"Timer@[^:\r\n]*:\s*([0-9]+)(ns|us|ms)")


def run(cmd: list[str], timeout: int, stdin: bytes | None = None):
    started = time.perf_counter()
    try:
        proc = subprocess.run(cmd, input=stdin, capture_output=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return None, (time.perf_counter() - started) * 1000.0
    return proc, (time.perf_counter() - started) * 1000.0


def expected_text(stdout: bytes, returncode: int) -> list[str]:
    text = stdout.decode(errors="replace")
    if text and not text.endswith("\n"):
        text += "\n"
    text += f"{returncode}\n"
    return [line.rstrip() for line in text.strip().splitlines()]


def matches(proc, expected: Path) -> bool:
    if proc is None:
        return False
    expected_lines = [line.rstrip() for line in expected.read_text(errors="replace").strip().splitlines()]
    return expected_text(proc.stdout, proc.returncode) == expected_lines


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


def asm_stats(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    insns = sum(1 for line in data.decode(errors="replace").splitlines() if INSN_RE.match(line) and not line.lstrip().startswith((".", "#")))
    return len(data), insns


def cases(root: Path) -> list[Path]:
    return sorted(root.rglob("*.sy"))


def run_case(compiler: Path, runtime: Path, qemu: str, gcc: str, source: Path, out_dir: Path, save_asm: bool, timeout: int, runs: int) -> dict[str, object]:
    rel = source.relative_to(source.parents[2]).as_posix().replace("/", "_")
    stem = source.stem + "_" + hashlib.sha1(rel.encode()).hexdigest()[:12]
    asm = out_dir / (stem + ".s")
    elf = out_dir / (stem + ".elf")
    proc, compile_ms = run([str(compiler), "-S", "-O1", "-o", str(asm), str(source)], 120)
    result: dict[str, object] = {"status": "COMPILE_TIMEOUT" if proc is None else "COMPILE_ERR" if proc.returncode else "OK", "compile_ms": compile_ms}
    if proc is None or proc.returncode:
        result["message"] = (proc.stderr.decode(errors="replace")[-240:] if proc else "timeout").replace("\n", " ")
        return result
    link, link_ms = run([gcc, *ARCH, "-static", "-o", str(elf), str(asm), str(runtime), "-lm"], 60)
    result["link_ms"] = link_ms
    if link is None or link.returncode:
        result["status"] = "LINK_TIMEOUT" if link is None else "LINK_ERR"
        result["message"] = (link.stderr.decode(errors="replace")[-240:] if link else "timeout").replace("\n", " ")
        return result
    result["asm_bytes"], result["asm_insns"] = asm_stats(asm)
    if not save_asm:
        asm.unlink(missing_ok=True)
    input_path = source.with_suffix(".in")
    expected = source.with_suffix(".out")
    stdin = input_path.read_bytes() if input_path.is_file() else None
    samples: list[float] = []
    internal_samples_us: list[float] = []
    for _ in range(max(1, runs)):
        proc, elapsed_ms = run([qemu, str(elf)], timeout, stdin)
        if proc is None:
            result["status"] = "RUN_TIMEOUT"
            break
        samples.append(elapsed_ms)
        if not matches(proc, expected):
            result["status"] = "WRONG_OUTPUT"
            break
        internal_us = internal_time_us(proc.stderr)
        if internal_us is not None:
            internal_samples_us.append(internal_us)
    else:
        result["status"] = "OK"
    result["run_ms"] = statistics.median(samples) if samples else ""
    result["run_samples"] = ";".join(f"{sample:.3f}" for sample in samples)
    internal_average_us = internal_raw_average_us = internal_median_us = None
    internal_outliers_us: list[float] = []
    if len(internal_samples_us) == len(samples) and internal_samples_us:
        (
            internal_average_us,
            internal_raw_average_us,
            internal_median_us,
            internal_outliers_us,
        ) = filtered_average(internal_samples_us)
    result["internal_us"] = internal_average_us if internal_average_us is not None else ""
    result["internal_raw_average_us"] = (
        internal_raw_average_us if internal_raw_average_us is not None else ""
    )
    result["internal_median_us"] = (
        internal_median_us if internal_median_us is not None else ""
    )
    result["internal_outliers_us"] = ";".join(
        f"{sample:.3f}" for sample in internal_outliers_us
    )
    result["internal_samples_us"] = ";".join(
        f"{sample:.3f}" for sample in internal_samples_us
    )
    result["message"] = ""
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--worktree", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument(
        "--runtime",
        type=Path,
        required=True,
        help="external SysY runtime archive used for every local link",
    )
    parser.add_argument("--since", default="2026-07-25T00:00:00+08:00")
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    parser.add_argument("--functional-runs", type=int, default=1)
    parser.add_argument("--performance-runs", type=int, default=1)
    parser.add_argument("--max-commits", type=int, default=0)
    parser.add_argument("--start-index", type=int, default=1)
    parser.add_argument("--end-index", type=int, default=0)
    args = parser.parse_args()
    args.worktree = args.worktree.resolve()
    args.runtime = args.runtime.resolve()
    if not args.runtime.exists():
        raise SystemExit(f"runtime not found: {args.runtime}")
    args.out_dir = args.out_dir.resolve()
    args.out_dir.mkdir(parents=True, exist_ok=True)
    functional = cases(args.worktree / "2026初赛RISCV赛道功能用例" / "functional") + cases(args.worktree / "2026初赛RISCV赛道功能用例" / "h_functional")
    performance = cases(args.worktree / "2026初赛RISCV赛道性能用例" / "performance")
    log = run(["git", "-C", str(args.worktree), "log", "--since=" + args.since, "--reverse", "--format=%H", "master"], 30)[0]
    if log is None or log.returncode:
        raise SystemExit("cannot enumerate commits")
    commits = [line for line in log.stdout.decode().splitlines() if line]
    if args.max_commits > 0:
        commits = commits[:args.max_commits]
    indexed_commits = list(enumerate(commits, 1))[max(0, args.start_index - 1):]
    if args.end_index > 0:
        indexed_commits = [item for item in indexed_commits if item[0] <= args.end_index]
    rows: list[dict[str, object]] = []
    commit_rows: list[dict[str, object]] = []
    for index, commit in indexed_commits:
        short = commit[:12]
        checkout, _ = run(["git", "-C", str(args.worktree), "checkout", "--detach", commit], 60)
        if checkout is None or checkout.returncode:
            commit_rows.append({"index": index, "commit": commit, "status": "CHECKOUT_ERR", "functional": "", "performance": ""})
            continue
        build = args.out_dir / "build" / short
        configure, _ = run(["cmake", "-S", str(args.worktree), "-B", str(build), "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release"], 120)
        build_result = run(["cmake", "--build", str(build), "-j4"], 300)[0] if configure and configure.returncode == 0 else None
        if configure is None or configure.returncode or build_result is None or build_result.returncode:
            commit_rows.append({"index": index, "commit": commit, "status": "BUILD_ERR", "functional": "", "performance": ""})
            continue
        compiler = build / "compiler"
        asm_dir = args.out_dir / "asm" / short
        asm_dir.mkdir(parents=True, exist_ok=True)
        functional_pass = 0
        performance_pass = 0
        total_ms = 0.0
        total_us = 0.0
        for category, suite, count in (("functional", functional, args.functional_runs), ("performance", performance, args.performance_runs)):
            for source in suite:
                result = run_case(compiler, args.runtime, args.qemu, args.gcc, source, asm_dir, category == "performance", 300 if category == "performance" else 120, count)
                ok = result.get("status") == "OK"
                if category == "functional":
                    functional_pass += int(ok)
                else:
                    performance_pass += int(ok)
                    if isinstance(result.get("run_ms"), (int, float)):
                        total_ms += float(result["run_ms"])
                    if isinstance(result.get("internal_us"), (int, float)):
                        total_us += float(result["internal_us"])
                rows.append({"index": index, "commit": commit, "short": short, "category": category, "case": source.stem, "status": result.get("status", ""), "run_ms": result.get("run_ms", ""), "run_samples": result.get("run_samples", ""), "internal_us": result.get("internal_us", ""), "internal_raw_average_us": result.get("internal_raw_average_us", ""), "internal_median_us": result.get("internal_median_us", ""), "internal_outliers_us": result.get("internal_outliers_us", ""), "internal_samples_us": result.get("internal_samples_us", ""), "asm_bytes": result.get("asm_bytes", ""), "asm_insns": result.get("asm_insns", ""), "message": result.get("message", "")})
        commit_rows.append({"index": index, "commit": commit, "status": "OK", "functional": f"{functional_pass}/{len(functional)}", "performance": f"{performance_pass}/{len(performance)}", "performance_total_us": f"{total_us:.3f}", "performance_total_ms": f"{total_ms:.3f}"})
        print(f"COMMIT {index}/{len(commits)} {short} functional={functional_pass}/{len(functional)} performance={performance_pass}/{len(performance)} total_us={total_us:.3f} wall_ms={total_ms:.3f}", flush=True)
    with (args.out_dir / "cases.csv").open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=["index", "commit", "short", "category", "case", "status", "run_ms", "run_samples", "internal_us", "internal_raw_average_us", "internal_median_us", "internal_outliers_us", "internal_samples_us", "asm_bytes", "asm_insns", "message"])
        writer.writeheader()
        writer.writerows(rows)
    with (args.out_dir / "commits.csv").open("w", encoding="utf-8", newline="") as stream:
        fields = ["index", "commit", "status", "functional", "performance", "performance_total_us", "performance_total_ms"]
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(commit_rows)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
