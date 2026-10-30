#!/usr/bin/env python3
"""Checks SysY2026 positive parsing/lowering and semantic diagnostics."""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
import tempfile


EXPECTED_ERRORS = {
    "bitwise_float.sy": ["3 e"],
    "bitwise_not_float.sy": ["3 e"],
    "const_assignment.sy": ["3 h"],
    "duplicate_case.sy": ["5 n"],
    "invalid_break.sy": ["2 m"],
    "missing_tokens.sy": ["2 k", "4 i", "4 j"],
    "printf_arity.sy": ["2 l"],
    "type_mismatch.sy": ["3 e"],
}


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, text=True, capture_output=True, check=False)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", required=True, type=pathlib.Path)
    args = parser.parse_args()

    root = pathlib.Path(__file__).resolve().parents[2]
    cases = root / "tests" / "sysy2026"
    failed = False

    with tempfile.TemporaryDirectory(prefix="sysy2026-sema-") as work:
        out_dir = pathlib.Path(work)
        for source in sorted((cases / "positive").glob("*.sy")):
            for suffix, opt in (("o0", []), ("o1", ["-O1"])):
                output = out_dir / f"{source.stem}-{suffix}.s"
                result = run([
                    str(args.compiler), "-S", "-o", str(output),
                    str(source), *opt,
                ])
                if result.returncode != 0 or not output.exists():
                    failed = True
                    print(f"FAIL {source.name} {suffix}: {result.stderr.strip()}")
                    continue

                if source.name == "runtime_name_override.sy":
                    assembly = output.read_text(encoding="utf-8")
                    required = []
                    if suffix == "o0":
                        required.extend([
                            ".globl putstr",
                            ".globl starttime",
                            "call putstr",
                            "call starttime",
                        ])
                    missing = [text for text in required if text not in assembly]
                    if missing or "_sysy_starttime" in assembly:
                        failed = True
                        print(
                            f"FAIL {source.name} {suffix}: "
                            f"missing assembly markers {missing!r}, "
                            f"runtime rewrite={'_sysy_starttime' in assembly}"
                        )

        for source in sorted((cases / "errors").glob("*.sy")):
            output = out_dir / f"{source.stem}.s"
            result = run([
                str(args.compiler), "-S", "-o", str(output), str(source),
            ])
            expected = EXPECTED_ERRORS[source.name]
            actual = result.stderr.strip()
            missing = [diagnostic for diagnostic in expected if diagnostic not in actual]
            if result.returncode == 0 or missing or output.exists():
                failed = True
                print(
                    f"FAIL {source.name}: missing {missing!r}, "
                    f"return={result.returncode}, stderr={actual!r}, "
                    f"assembly={output.exists()}"
                )

    if failed:
        return 1
    print("SysY2026 semantic checks: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
