#!/usr/bin/env python3
import argparse
import shutil
import subprocess
import sys
import time
from pathlib import Path


def find_cases(root: Path):
    if (root / "functional").is_dir() or (root / "h_functional").is_dir():
        files = []
        for sub in ("functional", "h_functional"):
            d = root / sub
            if d.is_dir():
                files.extend(sorted(d.glob("*.sy")))
        return files
    return sorted(root.rglob("*.sy"))


def run_cmd(cmd, *, timeout, stdin_data=None):
    try:
        return subprocess.run(
            cmd,
            input=stdin_data,
            capture_output=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return None


def normalized(stdout: bytes, retcode: int) -> str:
    text = stdout.decode(errors="replace")
    if text and not text.endswith("\n"):
        text += "\n"
    text += f"{retcode}\n"
    return text


def same_output(actual: str, expected_file: Path) -> bool:
    expected = expected_file.read_text(errors="replace")
    a_lines = [line.rstrip() for line in actual.strip().splitlines()]
    e_lines = [line.rstrip() for line in expected.strip().splitlines()]
    return a_lines == e_lines


def run_one(args, case: Path, work_root: Path):
    rel_key = "_".join(case.with_suffix("").parts[-2:])
    work_dir = work_root / rel_key
    work_dir.mkdir(parents=True, exist_ok=True)
    asm = work_dir / f"{case.stem}.s"
    elf = work_dir / f"{case.stem}.elf"
    actual_file = work_dir / f"{case.stem}.actual"
    expected = case.with_suffix(".out")
    input_file = case.with_suffix(".in")

    compile_cmd = [str(args.compiler), "-S", "-o", str(asm), str(case)]
    if args.opt:
        compile_cmd.append("-O1")
    r = run_cmd(compile_cmd, timeout=args.compile_timeout)
    if r is None:
        return False, "COMPILE_TIMEOUT"
    if r.returncode != 0:
        return False, "COMPILE_ERR: " + r.stderr.decode(errors="replace")[:500]

    link_cmd = [
        args.gcc,
        "-march=rv64gc",
        "-mcmodel=medany",
        str(asm),
        str(args.runtime),
        "-o",
        str(elf),
        "-static",
    ]
    r = run_cmd(link_cmd, timeout=args.link_timeout)
    if r is None:
        return False, "LINK_TIMEOUT"
    if r.returncode != 0:
        return False, "LINK_ERR: " + r.stderr.decode(errors="replace")[:500]

    stdin_data = input_file.read_bytes() if input_file.exists() else None
    r = run_cmd([args.qemu, str(elf)], timeout=args.run_timeout, stdin_data=stdin_data)
    if r is None:
        return False, "RUN_TIMEOUT"
    actual = normalized(r.stdout, r.returncode)
    actual_file.write_text(actual, encoding="utf-8")

    if not expected.exists():
        return False, "missing .out"
    if same_output(actual, expected):
        return True, ""

    if args.verbose:
        exp = expected.read_text(errors="replace").strip()
        act = actual.strip()
        return False, f"WRONG\nexpected:\n{exp[:500]}\nactual:\n{act[:500]}"
    return False, "WRONG"


def main():
    parser = argparse.ArgumentParser(description="Run 2026 SysY functional tests.")
    parser.add_argument("--compiler", type=Path, default=Path("./build/compiler"))
    parser.add_argument("--cases", type=Path, required=True)
    parser.add_argument(
        "--runtime",
        type=Path,
        required=True,
        help="external SysY runtime archive used only for local linking",
    )
    parser.add_argument("--work-dir", type=Path, default=Path("./build/test-work"))
    parser.add_argument("--filter", default="")
    parser.add_argument("--opt", action="store_true", help="pass -O1 to compiler")
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--qemu", default="qemu-riscv64")
    parser.add_argument("--compile-timeout", type=int, default=180)
    parser.add_argument("--link-timeout", type=int, default=30)
    parser.add_argument("--run-timeout", type=int, default=60)
    args = parser.parse_args()

    if not args.compiler.exists():
        print(f"compiler not found: {args.compiler}", file=sys.stderr)
        return 2
    if not args.runtime.exists():
        print(f"runtime not found: {args.runtime}", file=sys.stderr)
        return 2
    if not shutil.which(args.gcc):
        print(f"gcc not found: {args.gcc}", file=sys.stderr)
        return 2
    if not shutil.which(args.qemu):
        print(f"qemu not found: {args.qemu}", file=sys.stderr)
        return 2

    cases = find_cases(args.cases)
    if args.filter:
        cases = [c for c in cases if args.filter in c.name or args.filter in str(c.parent)]
    if not cases:
        print(f"no .sy cases found under {args.cases}", file=sys.stderr)
        return 2

    args.work_dir.mkdir(parents=True, exist_ok=True)
    start = time.time()
    passed = 0
    failures = []
    for case in cases:
        ok, msg = run_one(args, case, args.work_dir)
        rel = case.relative_to(args.cases) if case.is_relative_to(args.cases) else case
        if ok:
            passed += 1
            print(f"PASS {rel}", flush=True)
        else:
            failures.append((rel, msg))
            print(f"FAIL {rel} {msg}", flush=True)

    total = len(cases)
    print(f"\nResult: {passed}/{total} passed in {time.time() - start:.1f}s", flush=True)
    if failures:
        print("Failures:", flush=True)
        for rel, msg in failures:
            print(f"  - {rel}: {msg.splitlines()[0]}", flush=True)
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
