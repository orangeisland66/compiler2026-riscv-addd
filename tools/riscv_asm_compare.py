#!/usr/bin/env python3
"""Compare normalized instruction sequences in two RISC-V asm directories."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


INSN_RE = re.compile(r"^\s+[A-Za-z][A-Za-z0-9.]*\s")


def assemblies(directory: Path) -> dict[str, Path]:
    result: dict[str, Path] = {}
    for path in sorted(directory.glob("*.s")):
        marker = path.stem.rfind("_")
        if marker <= 0:
            continue
        case = path.stem[:marker]
        if case in result:
            raise SystemExit(f"duplicate case {case} in {directory}")
        result[case] = path
    return result


def normalized(path: Path) -> list[str]:
    result: list[str] = []
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line or line.startswith(".") or line.endswith(":"):
            continue
        result.append(re.sub(r"\s+", " ", line))
    return result


def counted_instructions(path: Path) -> int:
    return sum(
        1
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines()
        if INSN_RE.match(line) and not line.lstrip().startswith((".", "#"))
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline-asm-dir", type=Path, required=True)
    parser.add_argument("--reference-asm-dir", type=Path, required=True)
    parser.add_argument("--baseline-label", default="baseline")
    parser.add_argument("--reference-label", default="reference")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    baseline_dir = args.baseline_asm_dir.resolve()
    reference_dir = args.reference_asm_dir.resolve()
    output = args.out.resolve()
    for directory in (baseline_dir, reference_dir):
        if not directory.is_dir():
            raise SystemExit(f"missing asm directory: {directory}")

    baseline = assemblies(baseline_dir)
    reference = assemblies(reference_dir)
    common = sorted(set(baseline) & set(reference))
    missing_baseline = sorted(set(reference) - set(baseline))
    missing_reference = sorted(set(baseline) - set(reference))
    same: list[str] = []
    changed: list[dict[str, object]] = []
    for case in common:
        baseline_seq = normalized(baseline[case])
        reference_seq = normalized(reference[case])
        if baseline_seq == reference_seq:
            same.append(case)
            continue
        changed.append(
            {
                "case": case,
                "baseline_insns": counted_instructions(baseline[case]),
                "reference_insns": counted_instructions(reference[case]),
                "baseline_normalized": len(baseline_seq),
                "reference_normalized": len(reference_seq),
            }
        )

    result = {
        "baseline": args.baseline_label,
        "reference": args.reference_label,
        "total": len(common),
        "same_count": len(same),
        "changed_count": len(changed),
        "missing_baseline": missing_baseline,
        "missing_reference": missing_reference,
        "same": same,
        "changed": changed,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print(json.dumps(result, ensure_ascii=False))
    return 0 if not missing_baseline and not missing_reference else 1


if __name__ == "__main__":
    raise SystemExit(main())
