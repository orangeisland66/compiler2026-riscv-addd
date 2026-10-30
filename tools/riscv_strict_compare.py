#!/usr/bin/env python3
"""Run strict paired RISC-V performance comparisons with anomaly refill."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import re
import statistics
import subprocess
from dataclasses import asdict, dataclass
from pathlib import Path

try:
    from .riscv_strict_common import classify_pairs, prune_candidate_batch
except ImportError:
    from riscv_strict_common import classify_pairs, prune_candidate_batch


ARCH = ["-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany"]
TIMER_RE = re.compile(rb"Timer@[^:\r\n]*:\s*([0-9]+)(ns|us|ms)")


@dataclass
class Attempt:
    attempt: int
    order: str
    baseline_us: float | None
    reference_us: float | None
    baseline_correct: bool
    reference_correct: bool
    accepted: bool = False
    discard_reason: str = ""


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
    candidates = sorted(suite_root.rglob("performance"))
    for candidate in candidates:
        if candidate.is_dir() and any(candidate.glob("*.sy")):
            return candidate
    raise ValueError(f"cannot locate performance suite below {suite_root}")


def assembly_for(directory: Path, case: str) -> Path:
    matches = sorted(directory.glob(f"{case}_*.s"))
    if len(matches) != 1:
        raise ValueError(
            f"expected one assembly for {case} in {directory}, got {len(matches)}"
        )
    return matches[0]


def file_digest(path: Path) -> bytes:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.digest()


def cached_elf(
    asm: Path,
    runtime: Path,
    cache_dir: Path,
    gcc: str,
    timeout: int,
    runtime_digest: bytes,
) -> Path:
    digest = hashlib.sha256()
    digest.update(file_digest(asm))
    digest.update(runtime_digest)
    digest.update(gcc.encode())
    digest.update("\0".join(ARCH).encode())
    key = digest.hexdigest()
    elf = cache_dir / f"{key}.elf"
    if elf.is_file():
        return elf
    cache_dir.mkdir(parents=True, exist_ok=True)
    temporary = cache_dir / f"{key}.{os.getpid()}.tmp.elf"
    proc = command(
        [
            gcc,
            *ARCH,
            "-static",
            "-o",
            str(temporary),
            str(asm),
            str(runtime),
            "-lm",
        ],
        timeout,
    )
    if proc is None or proc.returncode:
        temporary.unlink(missing_ok=True)
        message = "timeout" if proc is None else proc.stderr.decode(
            errors="replace"
        )[-1000:]
        raise RuntimeError(f"link failed for {asm}: {message}")
    temporary.replace(elf)
    return elf


def save_process_output(
    directory: Path,
    attempt: int,
    version: str,
    proc: subprocess.CompletedProcess[bytes] | None,
) -> None:
    prefix = directory / f"attempt-{attempt:03d}-{version}"
    if proc is None:
        prefix.with_suffix(".timeout").write_text("timeout\n", encoding="utf-8")
        return
    prefix.with_suffix(".stdout").write_bytes(proc.stdout)
    prefix.with_suffix(".stderr").write_bytes(proc.stderr)
    prefix.with_suffix(".returncode").write_text(
        f"{proc.returncode}\n", encoding="utf-8"
    )


def run_one(
    qemu: str,
    elf: Path,
    stdin: bytes | None,
    expected: Path,
    timeout: int,
) -> tuple[subprocess.CompletedProcess[bytes] | None, bool, float | None]:
    proc = command([qemu, str(elf)], timeout, stdin)
    correct = matches(proc, expected)
    measured = internal_time_us(proc.stderr) if correct and proc else None
    return proc, correct and measured is not None, measured


def run_case(
    case: str,
    baseline_elf: Path,
    reference_elf: Path,
    suite: Path,
    out_dir: Path,
    qemu: str,
    valid_pairs: int,
    outlier_threshold: float,
    max_discarded: int,
    warmups: int,
    timeout: int,
) -> tuple[dict[str, object], list[Attempt]]:
    input_path = suite / f"{case}.in"
    expected = suite / f"{case}.out"
    if not expected.is_file():
        raise ValueError(f"missing expected output: {expected}")
    stdin = input_path.read_bytes() if input_path.is_file() else None
    raw_dir = out_dir / "raw" / case
    raw_dir.mkdir(parents=True, exist_ok=True)

    for warmup in range(warmups):
        for version, elf in (
            ("baseline", baseline_elf),
            ("reference", reference_elf),
        ):
            proc = command([qemu, str(elf)], timeout, stdin)
            save_process_output(raw_dir, -(warmup + 1), f"warmup-{version}", proc)

    attempts: list[Attempt] = []
    active: list[int] = []
    discarded = 0
    correctness_failed = False
    batch_unstable = False

    while len(active) < valid_pairs:
        attempt_number = len(attempts) + 1
        order = (
            ("baseline", "reference")
            if attempt_number % 2 == 1
            else ("reference", "baseline")
        )
        results: dict[str, tuple[bool, float | None]] = {}
        for version in order:
            elf = baseline_elf if version == "baseline" else reference_elf
            proc, correct, measured = run_one(
                qemu, elf, stdin, expected, timeout
            )
            save_process_output(raw_dir, attempt_number, version, proc)
            results[version] = (correct, measured)

        attempt = Attempt(
            attempt=attempt_number,
            order="->".join(order),
            baseline_us=results["baseline"][1],
            reference_us=results["reference"][1],
            baseline_correct=results["baseline"][0],
            reference_correct=results["reference"][0],
        )
        attempts.append(attempt)
        if not attempt.baseline_correct or not attempt.reference_correct:
            attempt.discard_reason = "correctness_or_timer_failure"
            correctness_failed = True
            break
        active.append(len(attempts) - 1)

        if len(active) < valid_pairs:
            continue
        active_pairs = [
            (
                float(attempts[index].baseline_us),
                float(attempts[index].reference_us),
            )
            for index in active
        ]
        prune = prune_candidate_batch(
            pairs=active_pairs,
            required_pairs=valid_pairs,
            threshold=outlier_threshold,
            discarded_so_far=discarded,
            max_discarded=max_discarded,
        )
        if prune.discarded_indices:
            rejected = [
                active[index] for index in prune.discarded_indices
            ]
            for index in rejected:
                attempts[index].discard_reason = (
                    f"deviation_gt_{outlier_threshold:.3f}"
                )
            active = [
                attempt_index
                for local_index, attempt_index in enumerate(active)
                if local_index in prune.accepted_indices
            ]
            discarded += len(rejected)
            if prune.batch_unstable:
                batch_unstable = True
                break

    if correctness_failed:
        classification = "invalid"
    elif batch_unstable:
        classification = "invalid_batch"
    else:
        for index in active:
            attempts[index].accepted = True
        pairs = [
            (
                float(attempts[index].baseline_us),
                float(attempts[index].reference_us),
            )
            for index in active
        ]
        verdict = classify_pairs(pairs, valid_pairs)
        result = {
            "case": case,
            **asdict(verdict),
            "valid_pairs": len(active),
            "discarded_pairs": discarded,
            "attempted_pairs": len(attempts),
            "outlier_threshold": outlier_threshold,
            "batch_valid": True,
        }
        return result, attempts

    return {
        "case": case,
        "baseline_wins": "",
        "reference_wins": "",
        "ties": "",
        "baseline_median_us": "",
        "reference_median_us": "",
        "baseline_average_us": "",
        "reference_average_us": "",
        "classification": classification,
        "needs_second_batch": False,
        "valid_pairs": len(active),
        "discarded_pairs": discarded,
        "attempted_pairs": len(attempts),
        "outlier_threshold": outlier_threshold,
        "batch_valid": False,
    }, attempts


def write_attempts(path: Path, attempts: list[Attempt]) -> None:
    rows = [asdict(attempt) for attempt in attempts]
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def parse_cases(value: str) -> list[str]:
    cases = sorted({item.strip() for item in value.split(",") if item.strip()})
    if not cases:
        raise ValueError("no cases selected")
    return cases


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--suite-root", type=Path, required=True)
    parser.add_argument("--baseline-asm-dir", type=Path, required=True)
    parser.add_argument("--reference-asm-dir", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--elf-cache-dir", type=Path)
    parser.add_argument("--cases", required=True)
    parser.add_argument("--baseline-label", default="baseline")
    parser.add_argument("--reference-label", default="reference")
    parser.add_argument("--valid-pairs", type=int, default=13)
    parser.add_argument("--outlier-threshold", type=float, default=0.10)
    parser.add_argument("--max-discarded", type=int, default=3)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64-static")
    parser.add_argument("--link-timeout", type=int, default=60)
    parser.add_argument("--run-timeout", type=int, default=300)
    args = parser.parse_args()

    suite_root = args.suite_root.resolve()
    baseline_asm_dir = args.baseline_asm_dir.resolve()
    reference_asm_dir = args.reference_asm_dir.resolve()
    runtime = args.runtime.resolve()
    out_dir = args.out_dir.resolve()
    elf_cache_dir = (
        args.elf_cache_dir.resolve()
        if args.elf_cache_dir
        else out_dir / "elf-cache"
    )
    for path in (suite_root, baseline_asm_dir, reference_asm_dir, runtime):
        if not path.exists():
            raise SystemExit(f"missing input: {path}")
    if args.valid_pairs != 13:
        raise SystemExit("--valid-pairs must be exactly 13 for a strict audit")
    if not 0 < args.outlier_threshold < 1:
        raise SystemExit("--outlier-threshold must be between zero and one")
    if args.max_discarded < 0 or args.warmups < 0:
        raise SystemExit("--max-discarded and --warmups must be nonnegative")

    cases = parse_cases(args.cases)
    suite = locate_performance_dir(suite_root)
    out_dir.mkdir(parents=True, exist_ok=True)
    runtime_digest = file_digest(runtime)
    results: list[dict[str, object]] = []
    all_attempts: list[dict[str, object]] = []

    for case in cases:
        baseline_elf = cached_elf(
            assembly_for(baseline_asm_dir, case),
            runtime,
            elf_cache_dir,
            args.gcc,
            args.link_timeout,
            runtime_digest,
        )
        reference_elf = cached_elf(
            assembly_for(reference_asm_dir, case),
            runtime,
            elf_cache_dir,
            args.gcc,
            args.link_timeout,
            runtime_digest,
        )
        case_dir = out_dir / "cases" / case
        case_dir.mkdir(parents=True, exist_ok=True)
        result, attempts = run_case(
            case=case,
            baseline_elf=baseline_elf,
            reference_elf=reference_elf,
            suite=suite,
            out_dir=case_dir,
            qemu=args.qemu,
            valid_pairs=args.valid_pairs,
            outlier_threshold=args.outlier_threshold,
            max_discarded=args.max_discarded,
            warmups=args.warmups,
            timeout=args.run_timeout,
        )
        result["baseline_label"] = args.baseline_label
        result["reference_label"] = args.reference_label
        results.append(result)
        write_attempts(case_dir / "attempts.csv", attempts)
        for attempt in attempts:
            all_attempts.append(
                {
                    "case": case,
                    "baseline_label": args.baseline_label,
                    "reference_label": args.reference_label,
                    **asdict(attempt),
                }
            )
        print(json.dumps(result, ensure_ascii=False), flush=True)

    with (out_dir / "results.csv").open(
        "w", encoding="utf-8", newline=""
    ) as stream:
        writer = csv.DictWriter(stream, fieldnames=list(results[0]))
        writer.writeheader()
        writer.writerows(results)
    with (out_dir / "attempts.csv").open(
        "w", encoding="utf-8", newline=""
    ) as stream:
        writer = csv.DictWriter(stream, fieldnames=list(all_attempts[0]))
        writer.writeheader()
        writer.writerows(all_attempts)

    summary = {
        "baseline": args.baseline_label,
        "reference": args.reference_label,
        "cases": len(cases),
        "valid_pairs": args.valid_pairs,
        "outlier_threshold": args.outlier_threshold,
        "max_discarded": args.max_discarded,
        "classifications": {
            classification: sum(
                result["classification"] == classification for result in results
            )
            for classification in sorted(
                {str(result["classification"]) for result in results}
            )
        },
    }
    (out_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print(json.dumps(summary, ensure_ascii=False), flush=True)
    return 0 if all(result["batch_valid"] for result in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
