#!/usr/bin/env python3
"""Compare project -O1 output with semantics-compatible GCC -O3 RV64GC code.

The tool is intentionally compile-only.  It converts the two SysY constructs
that are not directly accepted as equivalent C11 (timer macros and file-scope
const array bounds), assembles both outputs, and reports real object-code
instruction/section metrics.  Source names are output labels only and never
affect compilation decisions.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
import subprocess
import time
from collections import Counter
from pathlib import Path

from riscv_historical_audit import gcc_compatible_source


ARCH = ["-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany"]
HEADER = """#pragma once
int getint(void); int getch(void); int getarray(int a[]);
void putint(int); void putch(int); void putarray(int, int[]);
float getfloat(void); int getfarray(float[]);
void putfloat(float); void putfarray(int, float[]);
void _sysy_starttime(int); void _sysy_stoptime(int);
#define starttime() _sysy_starttime(__LINE__)
#define stoptime() _sysy_stoptime(__LINE__)
"""
DISASM_RE = re.compile(
    r"^\s*[0-9a-f]+:\s+(?:[0-9a-f]{2,8}\s+)+([a-z][a-z0-9_.]*)\s*(.*)$"
)
FUNCTION_RE = re.compile(r"^[0-9a-f]+ <([^>]+)>:$")


def run(command: list[str], timeout: int = 180) -> tuple[subprocess.CompletedProcess[str], float]:
    started = time.perf_counter()
    result = subprocess.run(command, text=True, capture_output=True, timeout=timeout, check=False)
    return result, (time.perf_counter() - started) * 1000.0


def classify(opcode: str, operands: str) -> str:
    base = opcode.removeprefix("c.")
    if base in {"lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "flw", "fld"}:
        return "loads"
    if base in {"sb", "sh", "sw", "sd", "fsw", "fsd"}:
        return "stores"
    if base.startswith("b") or base in {"beqz", "bnez"}:
        return "cond_branches"
    if base in {"jal", "jalr", "j", "jr", "ret"}:
        return "jumps"
    if base.startswith(("mul", "div", "rem")):
        return "mul_div_rem"
    if base.startswith("f"):
        return "floating"
    return "other"


def object_metrics(objdump: str, size: str, obj: Path, assembly: Path) -> dict[str, object]:
    disasm = subprocess.run(
        [objdump, "-d", "-M", "no-aliases", str(obj)],
        text=True,
        capture_output=True,
        check=True,
    ).stdout
    sections = subprocess.run(
        [size, "-A", str(obj)], text=True, capture_output=True, check=True
    ).stdout
    counts: Counter[str] = Counter()
    opcodes: Counter[str] = Counter()
    functions = re.findall(
        r"^\s*\.type\s+([^,\s]+),\s*@function\s*$",
        assembly.read_text(encoding="utf-8", errors="replace"),
        re.MULTILINE,
    )
    for raw in disasm.splitlines():
        line = raw.strip()
        if FUNCTION_RE.match(line):
            continue
        match = DISASM_RE.match(raw)
        if not match:
            continue
        opcode, operands = match.groups()
        opcodes[opcode] += 1
        counts["instructions"] += 1
        counts[classify(opcode, operands)] += 1
        if "(sp)" in operands:
            counts["stack_accesses"] += 1
        if opcode.startswith("c."):
            counts["compressed"] += 1
        if opcode in {"lui", "auipc"}:
            counts["upper_immediates"] += 1
        base = opcode.removeprefix("c.")
        if base.endswith("w") and base not in {"sw", "flw"}:
            counts["word_ops"] += 1
    text_bytes = 0
    for raw in sections.splitlines():
        fields = raw.split()
        if len(fields) >= 2 and fields[0].startswith(".text") and fields[1].isdigit():
            text_bytes += int(fields[1])
    result: dict[str, object] = dict(counts)
    result.update(
        {
            "text_bytes": text_bytes,
            "functions": len(functions),
            "specialized_functions": sum(
                any(
                    tag in name
                    for tag in (".constprop.", ".isra.", ".part.", ".spec.")
                )
                for name in functions
            ),
            "top_opcodes": ";".join(
                f"{opcode}:{count}" for opcode, count in opcodes.most_common(12)
            ),
        }
    )
    for key in (
        "instructions", "loads", "stores", "cond_branches", "jumps",
        "mul_div_rem", "floating", "stack_accesses", "compressed",
        "upper_immediates",
        "word_ops",
    ):
        result.setdefault(key, 0)
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cases", type=Path, required=True)
    parser.add_argument("--compiler", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    parser.add_argument("--gcc", default="riscv64-linux-gnu-gcc")
    parser.add_argument("--objdump", default="riscv64-linux-gnu-objdump")
    parser.add_argument("--size", default="riscv64-linux-gnu-size")
    args = parser.parse_args()

    cases = args.cases.resolve()
    compiler = args.compiler.resolve()
    out = args.out_dir.resolve()
    out.mkdir(parents=True, exist_ok=True)
    header = out / "sysy_gcc_compat.h"
    header.write_text(HEADER, encoding="utf-8")
    sources = sorted(cases.rglob("*.sy"))
    rows: list[dict[str, object]] = []
    failures: list[dict[str, str]] = []
    totals = {side: Counter() for side in ("ours", "gcc")}

    for index, source in enumerate(sources, 1):
        key = f"{source.stem}-{hashlib.sha1(str(source).encode()).hexdigest()[:10]}"
        work = out / "work" / key
        work.mkdir(parents=True, exist_ok=True)
        compat = work / "source.c"
        gcc_compatible_source(source, compat)
        commands = {
            "ours": [str(compiler), "-S", "-O1", "-o", str(work / "ours.s"), str(source)],
            "gcc": [
                args.gcc, *ARCH, "-O3", "-fwrapv", "-fno-strict-aliasing",
                "-fno-pie", "-fno-stack-protector",
                "-std=gnu11", "-w", "-include", str(header), "-S",
                "-o", str(work / "gcc.s"), str(compat),
            ],
        }
        metrics: dict[str, dict[str, object]] = {}
        compile_ms: dict[str, float] = {}
        failed = False
        for side, command in commands.items():
            result, elapsed = run(command)
            compile_ms[side] = elapsed
            if result.returncode:
                failures.append({"case": source.stem, "side": side, "message": result.stderr[-500:]})
                failed = True
                break
            assembly = work / f"{side}.s"
            assembled, _ = run([args.gcc, *ARCH, "-c", str(assembly), "-o", str(work / f"{side}.o")])
            if assembled.returncode:
                failures.append({"case": source.stem, "side": side + "-assemble", "message": assembled.stderr[-500:]})
                failed = True
                break
            metrics[side] = object_metrics(
                args.objdump, args.size, work / f"{side}.o", assembly
            )
            for name, value in metrics[side].items():
                if isinstance(value, int):
                    totals[side][name] += value
        if failed:
            continue
        row: dict[str, object] = {
            "case": source.stem,
            "ours_compile_ms": round(compile_ms["ours"], 3),
            "gcc_compile_ms": round(compile_ms["gcc"], 3),
        }
        for side in ("ours", "gcc"):
            row.update({f"{side}_{name}": value for name, value in metrics[side].items()})
        gcc_instructions = int(metrics["gcc"]["instructions"])
        gcc_text = int(metrics["gcc"]["text_bytes"])
        row["instruction_ratio"] = round(int(metrics["ours"]["instructions"]) / gcc_instructions, 4) if gcc_instructions else ""
        row["text_ratio"] = round(int(metrics["ours"]["text_bytes"]) / gcc_text, 4) if gcc_text else ""
        rows.append(row)
        if index == 1 or index % 10 == 0:
            print(f"PROGRESS {index}/{len(sources)} {source.stem}", flush=True)

    fields = list(rows[0]) if rows else ["case"]
    with (out / "per_case.csv").open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    summary = {
        "schema": 1,
        "cases_discovered": len(sources),
        "cases_compared": len(rows),
        "failures": failures,
        "commands": {
            "ours": "compiler -S -O1 -o output.s input.sy",
            "gcc": "riscv64-linux-gnu-gcc -march=rv64gc -mabi=lp64d -mcmodel=medany -O3 -fwrapv -fno-strict-aliasing -fno-pie -fno-stack-protector -std=gnu11 -include sysy_gcc_compat.h -S",
        },
        "totals": {side: dict(counter) for side, counter in totals.items()},
        "ratios": {
            name: (totals["ours"][name] / totals["gcc"][name] if totals["gcc"][name] else None)
            for name in ("instructions", "text_bytes", "loads", "stores", "cond_branches", "jumps", "stack_accesses", "mul_div_rem", "word_ops")
        },
        "cases_ours_more_instructions": sum(float(row["instruction_ratio"]) > 1.0 for row in rows),
        "cases_ours_fewer_instructions": sum(float(row["instruction_ratio"]) < 1.0 for row in rows),
        "cases_equal_instructions": sum(float(row["instruction_ratio"]) == 1.0 for row in rows),
    }
    (out / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False), flush=True)
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
