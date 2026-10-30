#!/usr/bin/env python3
"""Shared helpers for RISC-V performance and correctness runners."""

from __future__ import annotations

import re
import subprocess
import time
from pathlib import Path


TIMER_RE = re.compile(rb"Timer@[^:\r\n]*:\s*([0-9]+)(ns|us|ms)")


def find_cases(root: Path) -> list[Path]:
    return sorted(root.rglob("*.sy"))


def case_key(case: Path) -> str:
    if len(case.parts) >= 2:
        return "/".join(case.parts[-2:])
    return case.name


def case_family(case: Path) -> str:
    stem = case.stem
    if "-" in stem:
        head, tail = stem.rsplit("-", 1)
        if tail.isdigit():
            return head
    match = re.match(r"^(.*?)(\d+)$", stem)
    return match.group(1) if match and match.group(1) else stem


def run_cmd(
    command: list[str], *, timeout: int, stdin_data: bytes | None = None
) -> tuple[subprocess.CompletedProcess[bytes] | None, float]:
    started = time.perf_counter()
    try:
        result = subprocess.run(
            command,
            input=stdin_data,
            capture_output=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return None, (time.perf_counter() - started) * 1000.0
    return result, (time.perf_counter() - started) * 1000.0


def normalized(stdout: bytes, returncode: int) -> str:
    text = stdout.decode(errors="replace")
    if text and not text.endswith("\n"):
        text += "\n"
    return text + f"{returncode}\n"


def same_output(actual: str, expected_file: Path) -> bool:
    expected = expected_file.read_text(errors="replace")
    actual_lines = [line.rstrip() for line in actual.strip().splitlines()]
    expected_lines = [line.rstrip() for line in expected.strip().splitlines()]
    return actual_lines == expected_lines


def internal_time_us(stderr: bytes) -> float | None:
    total = 0.0
    found = False
    for raw_value, unit in TIMER_RE.findall(stderr):
        found = True
        value = int(raw_value)
        if unit == b"ns":
            total += value / 1000.0
        elif unit == b"ms":
            total += value * 1000.0
        else:
            total += value
    return total if found else None
