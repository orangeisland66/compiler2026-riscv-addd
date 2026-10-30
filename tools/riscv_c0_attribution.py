#!/usr/bin/env python3
"""Compare two SysY compilers and attribute RV64 assembly changes."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import time
from pathlib import Path


INSTRUCTION = re.compile(r"^\s+([a-z][a-z0-9.]*)\b(.*)$")
LOADS = {
    "lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "flw", "fld",
}
STORES = {"sb", "sh", "sw", "sd", "fsw", "fsd"}
CONDITIONAL_BRANCHES = {
    "beq", "bne", "blt", "bge", "bltu", "bgeu", "beqz", "bnez",
    "blez", "bgez", "bltz", "bgtz",
}
ADDRESS_MATERIALIZATION = {"la", "lla", "li", "lui", "auipc"}


def digest(data: str) -> str:
    return hashlib.sha256(data.encode()).hexdigest()


def family(path: Path) -> str:
    stem = path.stem
    if "-" in stem:
        head, tail = stem.rsplit("-", 1)
        if tail.isdigit():
            return head
    match = re.match(r"^(.*?)(\d+)$", stem)
    return match.group(1) if match and match.group(1) else stem


def normalized_assembly(text: str) -> str:
    lines = []
    for raw in text.splitlines():
        line = raw.strip()
        if line.startswith((".file", ".ident")):
            continue
        if line:
            lines.append(" ".join(line.split()))
    return "\n".join(lines) + "\n"


def assembly_metrics(text: str) -> dict[str, int]:
    metrics = {
        "bytes": len(text.encode()),
        "instructions": 0,
        "loads": 0,
        "stores": 0,
        "calls": 0,
        "branches": 0,
        "conditional_branches": 0,
        "stack_accesses": 0,
        "address_materializations": 0,
        "mul": 0,
        "div_rem": 0,
    }
    for raw in text.splitlines():
        match = INSTRUCTION.match(raw)
        if not match:
            continue
        opcode, operands = match.group(1), match.group(2)
        metrics["instructions"] += 1
        metrics["loads"] += opcode in LOADS
        metrics["stores"] += opcode in STORES
        metrics["calls"] += opcode in {"call", "jal", "jalr"}
        metrics["conditional_branches"] += opcode in CONDITIONAL_BRANCHES
        metrics["branches"] += (
            opcode in CONDITIONAL_BRANCHES or opcode in {"j", "jr", "ret"}
        )
        metrics["stack_accesses"] += (
            opcode in LOADS | STORES and re.search(r"\([^)]*sp\)", operands) is not None
        )
        metrics["address_materializations"] += opcode in ADDRESS_MATERIALIZATION
        metrics["mul"] += opcode in {"mul", "mulw", "mulh", "mulhu", "mulhsu"}
        metrics["div_rem"] += opcode.startswith(("div", "rem"))
    return metrics


def first_difference(left: str, right: str) -> dict[str, object]:
    lhs, rhs = left.splitlines(), right.splitlines()
    for index in range(max(len(lhs), len(rhs))):
        old = lhs[index] if index < len(lhs) else "<end>"
        new = rhs[index] if index < len(rhs) else "<end>"
        if old != new:
            return {"line": index + 1, "baseline": old[:240], "candidate": new[:240]}
    return {}


def compile_case(compiler: Path, source: Path, output: Path) -> dict[str, object]:
    env = os.environ.copy()
    env["COREIR_DUMP"] = "1"
    started = time.perf_counter()
    result = subprocess.run(
        [str(compiler), "-S", "-O1", "-o", str(output), str(source)],
        text=True,
        capture_output=True,
        env=env,
        timeout=300,
        check=False,
    )
    compile_ms = (time.perf_counter() - started) * 1000.0
    if result.returncode:
        return {
            "ok": False,
            "compile_ms": round(compile_ms, 3),
            "error": result.stderr[-1000:],
        }
    assembly = output.read_text(errors="replace")
    normalized = normalized_assembly(assembly)
    return {
        "ok": True,
        "compile_ms": round(compile_ms, 3),
        "assembly": normalized,
        "assembly_sha256": digest(normalized),
        "core_ir": result.stderr,
        "core_ir_sha256": digest(result.stderr),
        "metrics": assembly_metrics(assembly),
    }


def delta(candidate: dict[str, int], baseline: dict[str, int]) -> dict[str, int]:
    return {key: candidate[key] - baseline[key] for key in baseline}


def write_markdown(report: dict[str, object], path: Path) -> None:
    changed = report["changed_cases"]
    totals = report["totals"]
    lines = [
        "# C0 RV64 attribution",
        "",
        f"- baseline: `{report['baseline']}`",
        f"- candidate: `{report['candidate']}`",
        f"- cases: `{report['passed']}/{report['discovered']}`",
        f"- assembly: `{report['identical']} identical / {len(changed)} changed`",
        f"- CoreIR-first: `{report['core_ir_first']}`",
        f"- MIR/backend-first: `{report['backend_first']}`",
        "",
        "## Aggregate machine delta",
        "",
        "| metric | baseline | candidate | delta |",
        "|---|---:|---:|---:|",
    ]
    for metric, base in totals["baseline"].items():
        current = totals["candidate"][metric]
        lines.append(f"| {metric} | {base} | {current} | {current - base:+d} |")
    lines.extend([
        "",
        "## Changed cases",
        "",
        "| case | family | first layer | inst | load | store | branch | call | stack | bytes |",
        "|---|---|---|---:|---:|---:|---:|---:|---:|---:|",
    ])
    for row in changed:
        item = row["delta"]
        lines.append(
            f"| `{row['case']}` | {row['family']} | {row['first_changed_layer']} | "
            f"{item['instructions']:+d} | {item['loads']:+d} | {item['stores']:+d} | "
            f"{item['branches']:+d} | {item['calls']:+d} | "
            f"{item['stack_accesses']:+d} | {item['bytes']:+d} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--cases", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--baseline-ref", required=True)
    parser.add_argument("--candidate-ref", required=True)
    args = parser.parse_args()

    baseline = args.baseline.resolve()
    candidate = args.candidate.resolve()
    cases = sorted(args.cases.resolve().rglob("*.sy"))
    output = args.output_dir.resolve()
    asm_root = output / "assembly"
    asm_root.mkdir(parents=True, exist_ok=True)
    rows = []
    totals = {
        "baseline": {key: 0 for key in assembly_metrics("")},
        "candidate": {key: 0 for key in assembly_metrics("")},
    }
    for index, source in enumerate(cases):
        base = compile_case(baseline, source, asm_root / f"{index:02d}-{source.stem}.base.s")
        current = compile_case(candidate, source, asm_root / f"{index:02d}-{source.stem}.candidate.s")
        row: dict[str, object] = {
            "case": source.name,
            "family": family(source),
            "source": str(source),
            "baseline_ok": base["ok"],
            "candidate_ok": current["ok"],
            "baseline_compile_ms": base["compile_ms"],
            "candidate_compile_ms": current["compile_ms"],
        }
        if base["ok"] and current["ok"]:
            base_metrics = base["metrics"]
            current_metrics = current["metrics"]
            for key in base_metrics:
                totals["baseline"][key] += base_metrics[key]
                totals["candidate"][key] += current_metrics[key]
            changed = base["assembly_sha256"] != current["assembly_sha256"]
            core_changed = base["core_ir_sha256"] != current["core_ir_sha256"]
            row.update({
                "changed": changed,
                "first_changed_layer": "CoreIR" if core_changed else ("MIR/backend" if changed else "none"),
                "baseline_assembly_sha256": base["assembly_sha256"],
                "candidate_assembly_sha256": current["assembly_sha256"],
                "baseline_core_ir_sha256": base["core_ir_sha256"],
                "candidate_core_ir_sha256": current["core_ir_sha256"],
                "baseline_metrics": base_metrics,
                "candidate_metrics": current_metrics,
                "delta": delta(current_metrics, base_metrics),
                "first_core_ir_difference": first_difference(
                    base["core_ir"], current["core_ir"]
                ),
                "first_assembly_difference": first_difference(base["assembly"], current["assembly"]),
            })
        else:
            row["changed"] = False
            row["error"] = base.get("error", "") + current.get("error", "")
        rows.append(row)
        print(f"[{index + 1:02d}/{len(cases):02d}] {source.name}: "
              f"{row.get('first_changed_layer', 'compile-error')}", flush=True)

    changed = [row for row in rows if row.get("changed")]
    report: dict[str, object] = {
        "schema": 1,
        "baseline": args.baseline_ref,
        "candidate": args.candidate_ref,
        "baseline_compiler_sha256": hashlib.sha256(baseline.read_bytes()).hexdigest(),
        "candidate_compiler_sha256": hashlib.sha256(candidate.read_bytes()).hexdigest(),
        "cases": str(args.cases.resolve()),
        "discovered": len(rows),
        "passed": sum(row["baseline_ok"] and row["candidate_ok"] for row in rows),
        "identical": sum(not row.get("changed") for row in rows),
        "core_ir_first": sum(row.get("first_changed_layer") == "CoreIR" for row in changed),
        "backend_first": sum(row.get("first_changed_layer") == "MIR/backend" for row in changed),
        "totals": totals,
        "changed_cases": changed,
        "all_cases": rows,
    }
    output.mkdir(parents=True, exist_ok=True)
    (output / "attribution.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    write_markdown(report, output / "attribution.md")
    return 0 if report["passed"] == report["discovered"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
