#!/usr/bin/env python3
"""Fail when long-running performance cases regress beyond the project limit."""

from __future__ import annotations

import argparse
import csv
import math
from pathlib import Path


def read_report(path: Path, metric: str) -> dict[str, float]:
    with path.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise ValueError(f"empty performance report: {path}")
    values: dict[str, float] = {}
    for row in rows:
        case = (row.get("case") or row.get("Case") or "").strip()
        status = (row.get("status") or row.get("结果") or "").strip().upper()
        raw = row.get(metric, "")
        if not case:
            raise ValueError(f"missing case in {path}")
        if case in values:
            raise ValueError(f"duplicate case {case!r} in {path}")
        if status and status not in {"PASS", "PASSED", "AC", "OK"}:
            raise ValueError(f"case {case} is not successful in {path}: {status}")
        try:
            value = float(raw)
        except (TypeError, ValueError) as exc:
            raise ValueError(f"missing/non-numeric {metric} for {case} in {path}") from exc
        if not math.isfinite(value) or value < 0:
            raise ValueError(f"invalid {metric} for {case} in {path}: {raw!r}")
        values[case] = value
    return values


def check_regressions(
    baseline: dict[str, float],
    candidate: dict[str, float],
    threshold: float,
    limit: float,
) -> list[tuple[str, float, float, float]]:
    failures = []
    for case, base in sorted(baseline.items()):
        if base <= threshold:
            continue
        if case not in candidate:
            failures.append((case, base, math.nan, math.inf))
            continue
        current = candidate[case]
        delta = (current / base - 1.0) * 100.0 if base else 0.0
        if delta > limit:
            failures.append((case, base, current, delta))
    return failures


def threshold_in_metric_units(
    metric: str, longer_than_ms: float | None, longer_than_us: float | None
) -> tuple[float, str]:
    metric_unit = "us" if metric.endswith("_us") else "ms" if metric.endswith("_ms") else "units"
    if longer_than_us is not None:
        threshold_us = longer_than_us
    elif longer_than_ms is not None:
        threshold_us = longer_than_ms * 1000.0
    else:
        threshold_us = 10000.0
    if metric_unit == "us":
        return threshold_us, metric_unit
    if metric_unit == "ms":
        return threshold_us / 1000.0, metric_unit
    if longer_than_us is not None or longer_than_ms is not None:
        raise ValueError(
            f"cannot convert a time threshold for unitless metric {metric!r}"
        )
    return 10.0, metric_unit


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--metric", default="run_ms")
    threshold_group = parser.add_mutually_exclusive_group()
    threshold_group.add_argument("--longer-than-ms", type=float)
    threshold_group.add_argument("--longer-than-us", type=float)
    parser.add_argument("--max-regression-pct", type=float, default=5.0)
    args = parser.parse_args()
    supplied_thresholds = (args.longer_than_ms, args.longer_than_us)
    if any(value is not None and value < 0 for value in supplied_thresholds) or args.max_regression_pct < 0:
        parser.error("thresholds must be non-negative")
    try:
        threshold, metric_unit = threshold_in_metric_units(
            args.metric, args.longer_than_ms, args.longer_than_us
        )
        baseline = read_report(args.baseline, args.metric)
        candidate = read_report(args.candidate, args.metric)
        failures = check_regressions(
            baseline, candidate, threshold, args.max_regression_pct
        )
    except ValueError as exc:
        parser.error(str(exc))
    long_cases = sum(value > threshold for value in baseline.values())
    print(
        f"checked={long_cases} metric={args.metric} baseline>{threshold:g}{metric_unit} "
        f"limit={args.max_regression_pct:g}%"
    )
    for case, base, current, delta in failures:
        if math.isnan(current):
            print(f"FAIL {case}: missing candidate (baseline={base:.3f}{metric_unit})")
        else:
            print(
                f"FAIL {case}: baseline={base:.3f}{metric_unit} "
                f"candidate={current:.3f}{metric_unit} regression={delta:.2f}%"
            )
    if failures:
        return 1
    print("PASS long-running performance regression gate")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
