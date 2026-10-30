#!/usr/bin/env python3
"""Compare two sets of prebuilt RISC-V assembly with one runtime.

The historical backtrack audit first filters cases by normalized assembly.
This tool handles the remaining cases: it links both assemblies with the same
runtime, checks output, warms each ELF once, runs the versions in alternating
order, and records internal microsecond timing for every round.
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import statistics
import subprocess
from pathlib import Path


ARCH = ["-march=rv64gc", "-mcmodel=medany"]
TIMER_RE = re.compile(rb"Timer@[^:\r\n]*:\s*([0-9]+)(ns|us|ms)")


def command(
    argv: list[str], timeout: int, stdin: bytes | None = None
) -> subprocess.CompletedProcess[bytes] | None:
    try:
        return subprocess.run(
            argv, input=stdin, capture_output=True, timeout=timeout
        )
    except subprocess.TimeoutExpired:
        return None


def expected_text(stdout: bytes, returncode: int) -> list[str]:
    text = stdout.decode(errors="replace")
    if text and not text.endswith("\n"):
        text += "\n"
    text += f"{returncode}\n"
    return [line.rstrip() for line in text.strip().splitlines()]


def matches(proc: subprocess.CompletedProcess[bytes] | None, expected: Path) -> bool:
    if proc is None:
        return False
    expected_lines = [
        line.rstrip()
        for line in expected.read_text(errors="replace").strip().splitlines()
    ]
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


def locate_performance_dir(suite_root: Path) -> Path:
    matches = sorted(suite_root.rglob("performance"))
    for candidate in matches:
        if candidate.is_dir() and any(candidate.glob("*.sy")):
            return candidate
    raise SystemExit(f"cannot locate performance suite below {suite_root}")


def assembly_for(directory: Path, case: str) -> Path:
    matches = sorted(directory.glob(f"{case}_*.s"))
    if len(matches) != 1:
        raise SystemExit(
            f"expected one assembly for {case} in {directory}, got {len(matches)}"
        )
    return matches[0]


def cases_from_args(args: argparse.Namespace) -> list[str]:
    selected: list[str] = []
    if args.compare_json:
        data = json.loads(args.compare_json.read_text(encoding="utf-8"))
        for item in data.get("changed", []):
            selected.append(item["case"] if isinstance(item, dict) else str(item))
    if args.cases:
        selected.extend(item for item in args.cases.split(",") if item)
    selected = sorted(set(selected))
    if not selected:
        raise SystemExit("no cases selected; use --compare-json or --cases")
    return selected


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--suite-root", type=Path, required=True)
    parser.add_argument("--baseline-asm-dir", type=Path, required=True)
    parser.add_argument("--reference-asm-dir", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--compare-json", type=Path)
    parser.add_argument("--cases", default="")
    parser.add_argument("--runs", type=int, default=2)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    args = parser.parse_args()

    args.suite_root = args.suite_root.resolve()
    args.baseline_asm_dir = args.baseline_asm_dir.resolve()
    args.reference_asm_dir = args.reference_asm_dir.resolve()
    args.runtime = args.runtime.resolve()
    args.out_dir = args.out_dir.resolve()
    if args.compare_json:
        args.compare_json = args.compare_json.resolve()
    args.out_dir.mkdir(parents=True, exist_ok=True)

    if args.runs <= 0:
        raise SystemExit("--runs must be positive")
    for required in (
        args.baseline_asm_dir,
        args.reference_asm_dir,
        args.runtime,
    ):
        if not required.exists():
            raise SystemExit(f"missing input: {required}")

    suite = locate_performance_dir(args.suite_root)
    selected = cases_from_args(args)
    versions = {
        "baseline": args.baseline_asm_dir,
        "reference": args.reference_asm_dir,
    }
    elfs: dict[tuple[str, str], Path] = {}
    link_dir = args.out_dir / "elf"
    link_dir.mkdir(exist_ok=True)

    for version, asm_dir in versions.items():
        version_dir = link_dir / version
        version_dir.mkdir(exist_ok=True)
        for case in selected:
            asm = assembly_for(asm_dir, case)
            elf = version_dir / f"{case}.elf"
            proc = command(
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
                60,
            )
            if proc is None or proc.returncode:
                message = "timeout" if proc is None else proc.stderr.decode(
                    errors="replace"
                )[-400:]
                raise SystemExit(f"link failed for {version}/{case}: {message}")
            elfs[(version, case)] = elf

    samples: dict[tuple[str, str], list[float]] = {
        (version, case): [] for version in versions for case in selected
    }
    correct = {
        (version, case): True for version in versions for case in selected
    }

    # Warm both binaries before collecting paired results.
    for case in selected:
        input_path = suite / f"{case}.in"
        stdin = input_path.read_bytes() if input_path.is_file() else None
        for version in versions:
            command([args.qemu, str(elfs[(version, case)])], 300, stdin)

    version_names = list(versions)
    for round_index in range(args.runs):
        order = (
            version_names
            if round_index % 2 == 0
            else list(reversed(version_names))
        )
        for case in selected:
            input_path = suite / f"{case}.in"
            expected = suite / f"{case}.out"
            stdin = input_path.read_bytes() if input_path.is_file() else None
            for version in order:
                proc = command(
                    [args.qemu, str(elfs[(version, case)])], 300, stdin
                )
                if not matches(proc, expected):
                    correct[(version, case)] = False
                    continue
                measured = internal_time_us(proc.stderr)
                if measured is None:
                    correct[(version, case)] = False
                    continue
                samples[(version, case)].append(measured)

    rows: list[dict[str, object]] = []
    verdict_counts: dict[str, int] = {}
    for case in selected:
        baseline = samples[("baseline", case)]
        reference = samples[("reference", case)]
        both_correct = (
            correct[("baseline", case)]
            and correct[("reference", case)]
            and len(baseline) == args.runs
            and len(reference) == args.runs
        )
        if not both_correct:
            verdict = "INVALID"
        elif all(lhs < rhs for lhs, rhs in zip(baseline, reference)):
            verdict = "BASELINE_FASTER"
        elif all(lhs > rhs for lhs, rhs in zip(baseline, reference)):
            verdict = "REFERENCE_FASTER"
        else:
            verdict = "TIE_OR_NOISE"
        verdict_counts[verdict] = verdict_counts.get(verdict, 0) + 1
        rows.append(
            {
                "case": case,
                "baseline_correct": correct[("baseline", case)],
                "reference_correct": correct[("reference", case)],
                "baseline_average_us": (
                    statistics.fmean(baseline) if baseline else ""
                ),
                "reference_average_us": (
                    statistics.fmean(reference) if reference else ""
                ),
                "baseline_samples_us": ";".join(
                    f"{sample:.3f}" for sample in baseline
                ),
                "reference_samples_us": ";".join(
                    f"{sample:.3f}" for sample in reference
                ),
                "verdict": verdict,
            }
        )

    with (args.out_dir / "results.csv").open(
        "w", encoding="utf-8", newline=""
    ) as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    summary = {
        "cases": len(selected),
        "runs": args.runs,
        "baseline_correct": sum(
            bool(row["baseline_correct"]) for row in rows
        ),
        "reference_correct": sum(
            bool(row["reference_correct"]) for row in rows
        ),
        "verdicts": verdict_counts,
    }
    (args.out_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print(json.dumps(summary, ensure_ascii=False))
    return 0 if verdict_counts.get("INVALID", 0) == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
