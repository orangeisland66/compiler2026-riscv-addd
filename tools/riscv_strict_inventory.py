#!/usr/bin/env python3
"""Build the per-case assembly-group inventory for the strict RISC-V audit."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import subprocess
from dataclasses import dataclass
from pathlib import Path

try:
    from .riscv_asm_compare import assemblies, counted_instructions, normalized
except ImportError:
    from riscv_asm_compare import assemblies, counted_instructions, normalized


@dataclass(frozen=True)
class CommitEntry:
    order: int
    short_commit: str
    commit: str
    asm_dir: Path
    asm_source_commit: str


def normalized_digest(path: Path) -> str:
    payload = "\n".join(normalized(path)).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def candidate_assemblies(path: Path) -> dict[str, Path]:
    """Read a possible asm directory without failing on unrelated layouts."""

    try:
        return assemblies(path)
    except SystemExit:
        return {}


def read_legal_commits(scope_csv: Path) -> list[tuple[int, str, str]]:
    with scope_csv.open(encoding="utf-8", newline="") as stream:
        rows = list(csv.DictReader(stream))
    required = {"order", "short_commit", "commit", "status"}
    if not rows or not required.issubset(rows[0]):
        raise ValueError(f"invalid scope CSV columns: {scope_csv}")
    legal: list[tuple[int, str, str]] = []
    for row in rows:
        if row["status"] != "passed":
            continue
        legal.append((int(row["order"]), row["short_commit"], row["commit"]))
    if not legal:
        raise ValueError(f"no passed commits in {scope_csv}")
    return sorted(legal)


def compiler_tree_signature(repo: Path, commit: str) -> tuple[str, ...]:
    """Identify commits that build the exact same compiler implementation."""

    objects = ("src", "lib", "CMakeLists.txt")
    proc = subprocess.run(
        [
            "git",
            "-C",
            str(repo),
            "rev-parse",
            *(f"{commit}:{path}" for path in objects),
        ],
        capture_output=True,
        text=True,
        timeout=30,
    )
    if proc.returncode:
        raise ValueError(
            f"cannot resolve compiler tree for {commit}: {proc.stderr.strip()}"
        )
    hashes = tuple(line for line in proc.stdout.splitlines() if line)
    if len(hashes) != len(objects):
        raise ValueError(f"incomplete compiler tree signature for {commit}")
    return hashes


def candidate_rank(path: Path, short_commit: str, commit: str) -> int | None:
    """Prefer commit-keyed/reference outputs and reject round candidates."""

    lowered_parts = [part.lower() for part in path.parts]
    if any(
        marker in part
        for part in lowered_parts
        for marker in ("candidate", "baseline", "current")
    ):
        return None
    if commit.startswith(path.name) or path.name.startswith(short_commit):
        return 0
    if short_commit.lower() not in str(path).lower():
        return None
    if any(part.startswith("reference") for part in lowered_parts):
        return 1
    if short_commit.lower() in path.parent.name.lower():
        return 2
    return 3


def locate_commit_dirs(
    legal: list[tuple[int, str, str]],
    history_roots: list[Path],
    expected_cases: set[str],
    repo: Path | None,
) -> list[CommitEntry]:
    candidates: list[Path] = []
    for root in history_roots:
        if not root.is_dir():
            raise ValueError(f"missing historical search root: {root}")
        asm_root = root / "asm"
        if asm_root.is_dir():
            candidates.extend(path for path in asm_root.iterdir() if path.is_dir())
        candidates.extend(path for path in root.rglob("asm") if path.is_dir())
    candidates = sorted(set(path.resolve() for path in candidates))
    candidate_files = {
        path: files
        for path in candidates
        if set(files := candidate_assemblies(path)) == expected_cases
    }

    direct: dict[str, Path] = {}
    for order, short_commit, commit in legal:
        ranked = [
            (rank, path)
            for path in candidate_files
            if (rank := candidate_rank(path, short_commit, commit)) is not None
        ]
        best_rank = min((rank for rank, _ in ranked), default=None)
        matches = [
            path for rank, path in ranked if rank == best_rank
        ]
        if not matches:
            continue
        signatures = {
            tuple(
                (case, normalized_digest(candidate_files[path][case]))
                for case in sorted(expected_cases)
            )
            for path in matches
        }
        if len(signatures) != 1:
            raise ValueError(
                f"conflicting complete asm directories for {commit}: "
                f"{[str(path) for path in matches]}"
            )
        direct[commit] = matches[0]

    tree_signatures: dict[str, tuple[str, ...]] = {}
    if len(direct) != len(legal):
        if repo is None:
            missing = [commit for _, _, commit in legal if commit not in direct]
            raise ValueError(
                f"no complete asm directory for commits {missing}; "
                "--repo is required to resolve source-identical aliases"
            )
        tree_signatures = {
            commit: compiler_tree_signature(repo, commit)
            for _, _, commit in legal
        }

    entries: list[CommitEntry] = []
    for order, short_commit, commit in legal:
        source_commit = commit
        resolved = direct.get(commit)
        if resolved is None:
            equivalent = [
                candidate_commit
                for _, _, candidate_commit in legal
                if candidate_commit in direct
                and tree_signatures[candidate_commit] == tree_signatures[commit]
            ]
            if not equivalent:
                raise ValueError(
                    f"no complete or source-identical asm source for {commit}"
                )
            source_commit = equivalent[0]
            resolved = direct[source_commit]
        entries.append(
            CommitEntry(
                order=order,
                short_commit=short_commit,
                commit=commit,
                asm_dir=resolved,
                asm_source_commit=source_commit,
            )
        )
    return entries


def build_inventory(
    scope_csv: Path,
    baseline_asm_dir: Path,
    history_roots: list[Path],
    baseline_label: str,
    repo: Path | None = None,
) -> dict[str, object]:
    legal = read_legal_commits(scope_csv)
    baseline_files = assemblies(baseline_asm_dir)
    if not baseline_files:
        raise ValueError(f"no baseline assemblies in {baseline_asm_dir}")
    commits = locate_commit_dirs(
        legal, history_roots, set(baseline_files), repo
    )

    historical_files: dict[str, dict[str, Path]] = {}
    for entry in commits:
        files = assemblies(entry.asm_dir)
        missing = sorted(set(baseline_files) - set(files))
        extra = sorted(set(files) - set(baseline_files))
        if missing or extra:
            raise ValueError(
                f"case mismatch for {entry.commit}: "
                f"missing={missing}, extra={extra}"
            )
        historical_files[entry.commit] = files

    case_records: list[dict[str, object]] = []
    group_records: list[dict[str, object]] = []
    comparison_records: list[dict[str, object]] = []
    for case, baseline_path in sorted(baseline_files.items()):
        baseline_hash = normalized_digest(baseline_path)
        groups: dict[str, list[CommitEntry]] = {}
        for entry in commits:
            digest = normalized_digest(historical_files[entry.commit][case])
            groups.setdefault(digest, []).append(entry)

        baseline_group = ""
        for group_index, (digest, members) in enumerate(groups.items(), start=1):
            group_id = f"{case}:G{group_index:03d}"
            representative = members[0]
            representative_asm = historical_files[representative.commit][case]
            relation = "identical" if digest == baseline_hash else "different"
            if relation == "identical":
                baseline_group = group_id
            member_commits = ";".join(member.commit for member in members)
            group_records.append(
                {
                    "case": case,
                    "group_id": group_id,
                    "normalized_sha256": digest,
                    "relation_to_baseline": relation,
                    "representative_commit": representative.commit,
                    "representative_short_commit": representative.short_commit,
                    "representative_asm_dir": str(representative.asm_dir),
                    "representative_asm_source_commit": (
                        representative.asm_source_commit
                    ),
                    "representative_asm": str(representative_asm),
                    "instruction_count": counted_instructions(representative_asm),
                    "member_count": len(members),
                    "member_commits": member_commits,
                }
            )
            if relation == "different":
                comparison_records.append(
                    {
                        "case": case,
                        "baseline": baseline_label,
                        "group_id": group_id,
                        "representative_commit": representative.commit,
                        "representative_short_commit": (
                            representative.short_commit
                        ),
                        "reference_asm_source_commit": (
                            representative.asm_source_commit
                        ),
                        "reference_asm_dir": str(representative.asm_dir),
                        "member_commits": member_commits,
                        "status": "pending",
                    }
                )

        case_records.append(
            {
                "case": case,
                "baseline": baseline_label,
                "baseline_asm": str(baseline_path),
                "baseline_normalized_sha256": baseline_hash,
                "legal_commits": len(commits),
                "distinct_legal_asm": len(groups),
                "identical_group": baseline_group,
                "different_asm_groups": sum(
                    digest != baseline_hash for digest in groups
                ),
            }
        )

    return {
        "baseline": baseline_label,
        "baseline_asm_dir": str(baseline_asm_dir),
        "scope_csv": str(scope_csv),
        "history_roots": [str(path) for path in history_roots],
        "legal_commits": [
            {
                "order": entry.order,
                "short_commit": entry.short_commit,
                "commit": entry.commit,
                "asm_dir": str(entry.asm_dir),
                "asm_source_commit": entry.asm_source_commit,
            }
            for entry in commits
        ],
        "cases": case_records,
        "groups": group_records,
        "comparisons": comparison_records,
    }


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    if not rows:
        raise ValueError(f"refusing to write empty CSV: {path}")
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--scope-csv", type=Path, required=True)
    parser.add_argument("--baseline-asm-dir", type=Path, required=True)
    parser.add_argument("--history-root", type=Path, action="append", required=True)
    parser.add_argument("--baseline-label", required=True)
    parser.add_argument("--repo", type=Path)
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()

    scope_csv = args.scope_csv.resolve()
    baseline_asm_dir = args.baseline_asm_dir.resolve()
    history_roots = [path.resolve() for path in args.history_root]
    out_dir = args.out_dir.resolve()
    repo = args.repo.resolve() if args.repo else None
    out_dir.mkdir(parents=True, exist_ok=True)

    inventory = build_inventory(
        scope_csv=scope_csv,
        baseline_asm_dir=baseline_asm_dir,
        history_roots=history_roots,
        baseline_label=args.baseline_label,
        repo=repo,
    )
    write_csv(out_dir / "cases.csv", inventory["cases"])
    write_csv(out_dir / "groups.csv", inventory["groups"])
    write_csv(out_dir / "comparisons.csv", inventory["comparisons"])
    (out_dir / "manifest.json").write_text(
        json.dumps(inventory, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    summary = {
        "baseline": args.baseline_label,
        "legal_commits": len(inventory["legal_commits"]),
        "cases": len(inventory["cases"]),
        "distinct_groups": len(inventory["groups"]),
        "direct_comparisons": len(inventory["comparisons"]),
    }
    print(json.dumps(summary, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
