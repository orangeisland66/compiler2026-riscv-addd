#!/usr/bin/env python3
"""Maintain strict comparison and per-case ledgers from raw batch results."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


LEDGER_FIELDS = [
    "case",
    "status",
    "baseline",
    "group_id",
    "representative_commit",
    "representative_short_commit",
    "reference_asm_source_commit",
    "member_commits",
    "asm_relation",
    "batch1_baseline_wins",
    "batch1_reference_wins",
    "batch1_ties",
    "batch1_baseline_average_us",
    "batch1_baseline_median_us",
    "batch1_reference_average_us",
    "batch1_reference_median_us",
    "batch1_discarded_pairs",
    "batch1_classification",
    "batch1_evidence",
    "batch2_baseline_wins",
    "batch2_reference_wins",
    "batch2_ties",
    "batch2_baseline_average_us",
    "batch2_baseline_median_us",
    "batch2_reference_average_us",
    "batch2_reference_median_us",
    "batch2_discarded_pairs",
    "batch2_classification",
    "batch2_evidence",
    "final_baseline_wins",
    "final_reference_wins",
    "final_ties",
    "final_classification",
    "notes",
]

CASE_FIELDS = [
    "case",
    "status",
    "baseline",
    "distinct_legal_asm",
    "tested_asm",
    "worst_baseline_wins",
    "worst_reference_wins",
    "worst_ties",
    "classification",
    "strongest_reference",
    "evidence",
    "notes",
]

BATCH_RESULT_MAP = {
    "baseline_wins": "baseline_wins",
    "reference_wins": "reference_wins",
    "ties": "ties",
    "baseline_average_us": "baseline_average_us",
    "baseline_median_us": "baseline_median_us",
    "reference_average_us": "reference_average_us",
    "reference_median_us": "reference_median_us",
    "discarded_pairs": "discarded_pairs",
    "classification": "classification",
}


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream))


def write_csv(
    path: Path, rows: list[dict[str, str]], fields: list[str]
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    temporary.replace(path)


def parse_bool(value: str) -> bool:
    lowered = str(value).strip().lower()
    if lowered in {"true", "1", "yes"}:
        return True
    if lowered in {"false", "0", "no"}:
        return False
    raise ValueError(f"invalid boolean value: {value!r}")


def initialize_ledger(groups_csv: Path) -> list[dict[str, str]]:
    groups = read_csv(groups_csv)
    if not groups:
        raise ValueError(f"empty group inventory: {groups_csv}")
    rows: list[dict[str, str]] = []
    for group in groups:
        identical = group["relation_to_baseline"] == "identical"
        row = {field: "" for field in LEDGER_FIELDS}
        row.update(
            {
                "case": group["case"],
                "status": "complete" if identical else "pending",
                "baseline": "",
                "group_id": group["group_id"],
                "representative_commit": group["representative_commit"],
                "representative_short_commit": group[
                    "representative_short_commit"
                ],
                "reference_asm_source_commit": group[
                    "representative_asm_source_commit"
                ],
                "member_commits": group["member_commits"],
                "asm_relation": group["relation_to_baseline"],
                "final_classification": "tied_best" if identical else "",
                "notes": (
                    "规范化真实指令及操作数序列与当前基线相同"
                    if identical
                    else ""
                ),
            }
        )
        rows.append(row)
    return rows


def rebase_ledger(
    old_ledger: list[dict[str, str]],
    old_cases: list[dict[str, str]],
    new_groups_csv: Path,
    new_cases: list[dict[str, str]],
    baseline: str,
) -> tuple[list[dict[str, str]], set[str]]:
    """Carry results only for cases whose normalized baseline is unchanged."""

    old_digests = {
        row["case"]: row["baseline_normalized_sha256"] for row in old_cases
    }
    new_digests = {
        row["case"]: row["baseline_normalized_sha256"] for row in new_cases
    }
    if set(old_digests) != set(new_digests):
        raise ValueError("old and new baseline case inventories differ")

    changed_cases = {
        case
        for case in old_digests
        if old_digests[case] != new_digests[case]
    }
    old_by_group = {
        (
            row["case"],
            row["representative_commit"],
            row["member_commits"],
        ): row
        for row in old_ledger
    }
    rebased = initialize_ledger(new_groups_csv)
    carried_fields = [
        "status",
        *(
            field
            for field in LEDGER_FIELDS
            if field.startswith("batch") or field.startswith("final_")
        ),
        "notes",
    ]
    for row in rebased:
        row["baseline"] = baseline
        if row["case"] in changed_cases:
            continue
        key = (
            row["case"],
            row["representative_commit"],
            row["member_commits"],
        )
        old = old_by_group.get(key)
        if old is None:
            raise ValueError(f"missing old ledger group while rebasing: {key}")
        if old["asm_relation"] != row["asm_relation"]:
            raise ValueError(f"baseline relation changed without digest change: {key}")
        for field in carried_fields:
            row[field] = old[field]
    return rebased, changed_cases


def apply_batch(
    ledger: list[dict[str, str]],
    results: list[dict[str, str]],
    evidence: str,
    batch: int,
) -> None:
    if batch not in (1, 2):
        raise ValueError("batch must be 1 or 2")
    prefix = f"batch{batch}_"
    for result in results:
        matches = [
            row
            for row in ledger
            if row["case"] == result["case"]
            and row["representative_short_commit"]
            == result["reference_label"]
        ]
        if len(matches) != 1:
            raise ValueError(
                f"expected one ledger row for {result['case']}/"
                f"{result['reference_label']}, got {len(matches)}"
            )
        row = matches[0]
        if row["asm_relation"] != "different":
            raise ValueError(f"cannot ingest timing for tied row {row['group_id']}")
        if row["baseline"] and row["baseline"] != result["baseline_label"]:
            raise ValueError(
                f"baseline mismatch for {row['group_id']}: "
                f"{row['baseline']} vs {result['baseline_label']}"
            )
        row["baseline"] = result["baseline_label"]
        if batch == 1:
            for field in LEDGER_FIELDS:
                if field.startswith("batch2_") or field.startswith("final_"):
                    row[field] = ""
        for result_field, suffix in BATCH_RESULT_MAP.items():
            row[prefix + suffix] = result[result_field]
        row[prefix + "evidence"] = evidence

        classification = result["classification"]
        batch_valid = parse_bool(result["batch_valid"])
        if not batch_valid or classification in {"invalid", "invalid_batch"}:
            row["status"] = "in_progress"
            row["final_baseline_wins"] = ""
            row["final_reference_wins"] = ""
            row["final_ties"] = ""
            row["final_classification"] = ""
            continue
        if classification == "not_best":
            row["status"] = "not_best"
            row["final_classification"] = "not_best"
        elif batch == 1 and classification == "strict_best":
            row["status"] = "needs_second_batch"
            row["final_classification"] = ""
        elif batch == 1:
            row["status"] = "complete"
            row["final_classification"] = classification
        else:
            first_classification = row["batch1_classification"]
            first_wins = int(row["batch1_baseline_wins"] or 0)
            second_wins = int(result["baseline_wins"] or 0)
            both_pass = (
                first_classification
                in {"strict_best", "strong_dominance", "stable_dominance"}
                and classification
                in {"strict_best", "strong_dominance", "stable_dominance"}
                and first_wins >= 9
                and second_wins >= 9
                and first_wins + second_wins >= 19
            )
            row["status"] = "complete" if both_pass else "not_best"
            row["final_classification"] = (
                "strict_best" if both_pass else "not_best"
            )

        batches = (1, 2) if row["batch2_classification"] else (1,)
        if row["status"] in {"complete", "not_best"}:
            row["final_baseline_wins"] = str(
                sum(int(row[f"batch{index}_baseline_wins"]) for index in batches)
            )
            row["final_reference_wins"] = str(
                sum(
                    int(row[f"batch{index}_reference_wins"])
                    for index in batches
                )
            )
            row["final_ties"] = str(
                sum(int(row[f"batch{index}_ties"]) for index in batches)
            )


def weakest_complete(rows: list[dict[str, str]]) -> dict[str, str]:
    different = [row for row in rows if row["asm_relation"] == "different"]
    if not different:
        return rows[0]
    return min(
        different,
        key=lambda row: (
            int(row["final_baseline_wins"] or -1),
            -int(row["final_reference_wins"] or 0),
        ),
    )


def summarize_cases(
    ledger: list[dict[str, str]], baseline: str
) -> list[dict[str, str]]:
    by_case: dict[str, list[dict[str, str]]] = {}
    for row in ledger:
        by_case.setdefault(row["case"], []).append(row)

    classification_rank = {
        "not_best": 0,
        "strict_best": 1,
        "strong_dominance": 2,
        "stable_dominance": 3,
        "tied_best": 4,
    }
    summaries: list[dict[str, str]] = []
    for case, rows in sorted(by_case.items()):
        statuses = {row["status"] for row in rows}
        if "not_best" in statuses:
            status = "not_best"
        elif "in_progress" in statuses or "needs_second_batch" in statuses:
            status = "in_progress"
        elif "pending" in statuses:
            status = "pending"
        else:
            status = "complete"

        finished = [
            row
            for row in rows
            if row["status"] in {"complete", "not_best"}
        ]
        finished_different = [
            row for row in finished if row["asm_relation"] == "different"
        ]
        if finished_different:
            worst = weakest_complete(finished_different)
        elif status == "complete" and finished:
            worst = finished[0]
        else:
            worst = None
        classifications = [
            row["final_classification"]
            for row in finished
            if row["final_classification"]
        ]
        classification = (
            min(classifications, key=classification_rank.get)
            if status in {"complete", "not_best"} and classifications
            else ""
        )
        summaries.append(
            {
                "case": case,
                "status": status,
                "baseline": baseline,
                "distinct_legal_asm": str(len(rows)),
                "tested_asm": str(len(finished)),
                "worst_baseline_wins": (
                    worst["final_baseline_wins"] if worst else ""
                ),
                "worst_reference_wins": (
                    worst["final_reference_wins"] if worst else ""
                ),
                "worst_ties": worst["final_ties"] if worst else "",
                "classification": classification,
                "strongest_reference": (
                    worst["representative_commit"] if worst else ""
                ),
                "evidence": (
                    worst["batch2_evidence"]
                    or worst["batch1_evidence"]
                    if worst
                    else ""
                ),
                "notes": (
                    f"{len(finished)}/{len(rows)}个合法历史汇编组已完成"
                ),
            }
        )
    return summaries


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    initialize = subparsers.add_parser("init")
    initialize.add_argument("--groups-csv", type=Path, required=True)
    initialize.add_argument("--ledger", type=Path, required=True)
    initialize.add_argument("--case-status", type=Path, required=True)
    initialize.add_argument("--baseline", required=True)

    rebase = subparsers.add_parser("rebase")
    rebase.add_argument("--ledger", type=Path, required=True)
    rebase.add_argument("--old-cases", type=Path, required=True)
    rebase.add_argument("--new-groups", type=Path, required=True)
    rebase.add_argument("--new-cases", type=Path, required=True)
    rebase.add_argument("--case-status", type=Path, required=True)
    rebase.add_argument("--baseline", required=True)

    ingest = subparsers.add_parser("ingest")
    ingest.add_argument("--ledger", type=Path, required=True)
    ingest.add_argument("--results", type=Path, required=True)
    ingest.add_argument("--case-status", type=Path, required=True)
    ingest.add_argument("--evidence", required=True)
    ingest.add_argument("--batch", type=int, choices=(1, 2), required=True)

    args = parser.parse_args()
    ledger_path = args.ledger.resolve()
    case_status_path = args.case_status.resolve()
    changed_cases: set[str] = set()
    if args.command == "init":
        ledger = initialize_ledger(args.groups_csv.resolve())
        baseline = args.baseline
        for row in ledger:
            row["baseline"] = baseline
    elif args.command == "rebase":
        ledger, changed_cases = rebase_ledger(
            old_ledger=read_csv(args.ledger.resolve()),
            old_cases=read_csv(args.old_cases.resolve()),
            new_groups_csv=args.new_groups.resolve(),
            new_cases=read_csv(args.new_cases.resolve()),
            baseline=args.baseline,
        )
        baseline = args.baseline
    else:
        ledger = read_csv(ledger_path)
        results = read_csv(args.results.resolve())
        if not ledger or not results:
            raise SystemExit("ledger and results must both be nonempty")
        apply_batch(ledger, results, args.evidence, args.batch)
        baseline = next(
            (
                row["baseline"]
                for row in ledger
                if row["baseline"]
            ),
            results[0]["baseline_label"],
        )

    write_csv(ledger_path, ledger, LEDGER_FIELDS)
    summaries = summarize_cases(ledger, baseline)
    write_csv(case_status_path, summaries, CASE_FIELDS)
    status_counts = {
        status: sum(row["status"] == status for row in summaries)
        for status in sorted({row["status"] for row in summaries})
    }
    print(
        f"groups={len(ledger)} cases={len(summaries)} "
        f"status={status_counts}"
        + (
            f" changed_cases={','.join(sorted(changed_cases))}"
            if changed_cases
            else ""
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
