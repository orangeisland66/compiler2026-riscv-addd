#!/usr/bin/env python3
"""Rebuild a strict compiler and record reproducible RV64 migration evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import time
from pathlib import Path


INSTRUCTION = re.compile(r"^\s+(?![.#])([a-z][a-z0-9.]*)\b")
IGNORED_CASE_DIRECTORIES = {"evidence", "assembly", "runtime", "build"}


def run(command: list[str], cwd: Path, timeout: int = 1800) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command, cwd=cwd, text=True, capture_output=True, timeout=timeout, check=False
    )


def checked(command: list[str], cwd: Path, timeout: int = 1800) -> str:
    result = run(command, cwd, timeout)
    if result.returncode:
        raise RuntimeError(
            f"command failed ({result.returncode}): {' '.join(command)}\n"
            f"{result.stdout[-2000:]}\n{result.stderr[-2000:]}"
        )
    return result.stdout.strip()


def assembly_metrics(path: Path) -> dict[str, object]:
    data = path.read_bytes()
    instructions: list[str] = []
    for line in data.decode(errors="replace").splitlines():
        match = INSTRUCTION.match(line)
        if match:
            instructions.append(match.group(1))
    return {
        "sha256": hashlib.sha256(data).hexdigest(),
        "bytes": len(data),
        "instructions": len(instructions),
        "loads": sum(op.startswith(("l", "fl")) and op not in {"li", "la"} for op in instructions),
        "stores": sum(op.startswith(("s", "fs")) and op not in {"sll", "slli", "slt", "slti"} for op in instructions),
        "branches": sum(op.startswith("b") or op in {"j", "jr", "ret", "call"} for op in instructions),
    }


def discover(root: Path | None) -> list[Path]:
    if not root or not root.is_dir():
        return []
    return sorted(
        path
        for path in root.rglob("*.sy")
        if not any(
            part.startswith(".") or part in IGNORED_CASE_DIRECTORIES
            for part in path.relative_to(root).parts[:-1]
        )
    )


def compile_suite(
    compiler: Path, cases: list[Path], output: Path, optimized: bool
) -> dict[str, object]:
    mode = "O1" if optimized else "O0"
    mode_dir = output / mode
    mode_dir.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, object]] = []
    started = time.perf_counter()
    for index, source in enumerate(cases):
        target = mode_dir / f"{index:04d}-{source.stem}.s"
        command = [str(compiler), "-S", "-o", str(target), str(source)]
        if optimized:
            command.append("-O1")
        case_started = time.perf_counter()
        result = run(command, compiler.parent.parent, timeout=300)
        elapsed_ms = (time.perf_counter() - case_started) * 1000
        row: dict[str, object] = {
            "source": str(source),
            "ok": result.returncode == 0,
            "compile_ms": round(elapsed_ms, 3),
        }
        if result.returncode == 0:
            row.update(assembly_metrics(target))
        else:
            row["stderr"] = result.stderr[-1000:]
        rows.append(row)
    return {
        "mode": mode,
        "passed": sum(bool(row["ok"]) for row in rows),
        "discovered": len(rows),
        "compile_ms": round((time.perf_counter() - started) * 1000, 3),
        "assembly_bytes": sum(int(row.get("bytes", 0)) for row in rows),
        "instructions": sum(int(row.get("instructions", 0)) for row in rows),
        "cases": rows,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--build-dir", type=Path, default=Path("build-migration-baseline"))
    parser.add_argument("--output", type=Path, default=Path("build-migration-baseline/baseline.json"))
    parser.add_argument("--cases", type=Path, default=Path("tests/riscv"))
    parser.add_argument("--functional-cases", type=Path)
    parser.add_argument("--performance-cases", type=Path)
    parser.add_argument("--runtime", type=Path)
    parser.add_argument("--jobs", type=int, default=1)
    parser.add_argument("--expected-master", default="c7eba4837484dab6b80e19dabf610ebfa78a02fc")
    parser.add_argument("--expected-head", default="0f18dc6e22872e1277dd42780a45de8a4015f123")
    args = parser.parse_args()

    repo = args.repo.resolve()
    build = args.build_dir if args.build_dir.is_absolute() else repo / args.build_dir
    output = args.output if args.output.is_absolute() else repo / args.output
    cases_root = args.cases if args.cases.is_absolute() else repo / args.cases
    branch = checked(["git", "branch", "--show-current"], repo)
    master = checked(["git", "rev-parse", "master"], repo)
    head = checked(["git", "rev-parse", "HEAD"], repo)
    if master != args.expected_master or head != args.expected_head or branch != "master-new":
        raise RuntimeError(
            f"branch invariant failed: branch={branch} master={master} head={head}"
        )

    configure_started = time.perf_counter()
    checked(
        ["cmake", "-S", str(repo), "-B", str(build), "-DCMAKE_BUILD_TYPE=Release",
         "-DSYC_ENABLE_RESEARCH_PASSES=OFF"],
        repo,
    )
    checked(["cmake", "--build", str(build), f"-j{max(1, args.jobs)}"], repo)
    build_ms = (time.perf_counter() - configure_started) * 1000
    compiler = build / "compiler"
    cases = discover(cases_root)
    evidence_dir = output.parent / "assembly"
    report: dict[str, object] = {
        "schema": 1,
        "branch": branch,
        "master": master,
        "head": head,
        "strict_research_passes": False,
        "build_ms": round(build_ms, 3),
        "compiler_sha256": hashlib.sha256(compiler.read_bytes()).hexdigest(),
        "toolchain": {
            "cmake": checked(["cmake", "--version"], repo).splitlines()[0],
            "cxx": checked([os.environ.get("CXX", "c++"), "--version"], repo).splitlines()[0],
        },
        "compile_only": [
            compile_suite(compiler, cases, evidence_dir, False),
            compile_suite(compiler, cases, evidence_dir, True),
        ],
    }
    runtime = args.runtime.resolve() if args.runtime else None
    if runtime and runtime.exists():
        end_to_end = []
        for label, root, optimized in (
            ("functional-O0", args.functional_cases, False),
            ("functional-O1", args.functional_cases, True),
            ("performance-O1", args.performance_cases, True),
        ):
            if not root:
                continue
            root = root.resolve()
            work = output.parent / "runtime" / label
            command = [
                "python3", str(repo / "tests/run_functional.py"),
                "--compiler", str(compiler), "--cases", str(root),
                "--runtime", str(runtime), "--work-dir", str(work),
            ]
            if optimized:
                command.append("--opt")
            result = run(command, repo, timeout=7200)
            end_to_end.append({
                "label": label,
                "ok": result.returncode == 0,
                "summary": next(
                    (line for line in reversed(result.stdout.splitlines()) if line.startswith("Result:")),
                    "missing-summary",
                ),
            })
        report["end_to_end"] = end_to_end

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n")
    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
