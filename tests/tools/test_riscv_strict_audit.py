from __future__ import annotations

import csv
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.riscv_strict_common import (
    classify_pairs,
    outlier_pair_indices,
    prune_candidate_batch,
)
from tools.riscv_strict_inventory import build_inventory, candidate_rank
from tools.riscv_strict_ledger import (
    apply_batch,
    initialize_ledger,
    rebase_ledger,
    summarize_cases,
)


class StrictRulesTest(unittest.TestCase):
    def test_outlier_rejects_whole_pair_when_either_side_deviates(self) -> None:
        pairs = [(100.0, 110.0)] * 12 + [(150.0, 109.0)]
        self.assertEqual(outlier_pair_indices(pairs, 0.10), {12})

    def test_one_discard_requests_one_replacement_pair(self) -> None:
        pairs = [(100.0, 110.0)] * 12 + [(150.0, 109.0)]
        result = prune_candidate_batch(pairs, 13, 0.10, 0, 3)
        self.assertEqual(result.discarded_indices, (12,))
        self.assertEqual(result.refill_pairs, 1)
        self.assertFalse(result.batch_unstable)

    def test_four_discards_invalidate_the_batch(self) -> None:
        pairs = (
            [(100.0, 110.0)] * 9
            + [(150.0, 110.0)] * 2
            + [(100.0, 170.0)] * 2
        )
        result = prune_candidate_batch(pairs, 13, 0.10, 0, 3)
        self.assertEqual(len(result.discarded_indices), 4)
        self.assertTrue(result.batch_unstable)

    def test_stable_dominance_requires_aggregate_advantage(self) -> None:
        verdict = classify_pairs([(100.0, 110.0)] * 13)
        self.assertEqual(verdict.classification, "stable_dominance")
        self.assertFalse(verdict.needs_second_batch)

    def test_strict_best_requires_second_batch(self) -> None:
        pairs = [(100.0, 110.0)] * 10 + [(111.0, 110.0)] * 3
        verdict = classify_pairs(pairs)
        self.assertEqual(verdict.classification, "strict_best")
        self.assertTrue(verdict.needs_second_batch)

    def test_eight_wins_is_not_best(self) -> None:
        pairs = [(100.0, 110.0)] * 8 + [(111.0, 110.0)] * 5
        self.assertEqual(classify_pairs(pairs).classification, "not_best")

    def test_median_failure_is_not_best_even_with_nine_wins(self) -> None:
        pairs = [(100.0, 101.0)] * 9 + [(1000.0, 101.0)] * 4
        self.assertEqual(classify_pairs(pairs).classification, "not_best")


class InventoryTest(unittest.TestCase):
    @staticmethod
    def write_asm(path: Path, instruction: str) -> None:
        path.write_text(
            f".text\nmain:\n  {instruction}\n  ret\n", encoding="utf-8"
        )

    def test_groups_identical_history_and_emits_only_different_tasks(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            baseline = root / "baseline"
            history = root / "history"
            (history / "asm" / "aaaaaaa11111").mkdir(parents=True)
            (history / "asm" / "bbbbbbb22222").mkdir(parents=True)
            (history / "asm" / "ccccccc33333").mkdir(parents=True)
            baseline.mkdir()
            self.write_asm(baseline / "case_hash.s", "li a0, 1")
            self.write_asm(
                history / "asm" / "aaaaaaa11111" / "case_hash.s", "li a0, 1"
            )
            self.write_asm(
                history / "asm" / "bbbbbbb22222" / "case_hash.s", "li a0, 2"
            )
            self.write_asm(
                history / "asm" / "ccccccc33333" / "case_hash.s", "li a0, 2"
            )
            scope = root / "scope.csv"
            with scope.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.DictWriter(
                    stream,
                    fieldnames=[
                        "order",
                        "short_commit",
                        "commit",
                        "status",
                    ],
                )
                writer.writeheader()
                writer.writerows(
                    [
                        {
                            "order": 1,
                            "short_commit": "aaaaaaa",
                            "commit": "aaaaaaa111111111",
                            "status": "passed",
                        },
                        {
                            "order": 2,
                            "short_commit": "bbbbbbb",
                            "commit": "bbbbbbb222222222",
                            "status": "passed",
                        },
                        {
                            "order": 3,
                            "short_commit": "ccccccc",
                            "commit": "ccccccc333333333",
                            "status": "passed",
                        },
                        {
                            "order": 4,
                            "short_commit": "ddddddd",
                            "commit": "ddddddd444444444",
                            "status": "invalid_reference",
                        },
                    ]
                )

            inventory = build_inventory(
                scope_csv=scope,
                baseline_asm_dir=baseline,
                history_roots=[history],
                baseline_label="base",
            )
            self.assertEqual(len(inventory["legal_commits"]), 3)
            self.assertEqual(len(inventory["groups"]), 2)
            self.assertEqual(len(inventory["comparisons"]), 1)
            self.assertEqual(
                inventory["cases"][0]["different_asm_groups"], 1
            )
            self.assertEqual(
                inventory["groups"][1]["member_count"], 2
            )

    def test_candidate_outputs_do_not_impersonate_historical_reference(
        self,
    ) -> None:
        path = Path("round-025-78d4823/candidate-full/asm")
        self.assertIsNone(candidate_rank(path, "78d4823", "78d4823abcdef"))


class LedgerTest(unittest.TestCase):
    def test_rebase_keeps_only_unchanged_case_results(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            groups = Path(temporary) / "groups.csv"
            fields = [
                "case",
                "group_id",
                "relation_to_baseline",
                "representative_commit",
                "representative_short_commit",
                "representative_asm_source_commit",
                "member_commits",
            ]
            with groups.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                for case in ("same", "changed"):
                    writer.writerow(
                        {
                            "case": case,
                            "group_id": f"{case}:G001",
                            "relation_to_baseline": "different",
                            "representative_commit": f"{case}-full",
                            "representative_short_commit": case,
                            "representative_asm_source_commit": f"{case}-full",
                            "member_commits": f"{case}-full",
                        }
                    )

            old = initialize_ledger(groups)
            for row in old:
                row["status"] = "complete"
                row["batch1_baseline_wins"] = "13"
                row["final_baseline_wins"] = "13"
                row["final_classification"] = "stable_dominance"
            old_cases = [
                {"case": "same", "baseline_normalized_sha256": "same-hash"},
                {"case": "changed", "baseline_normalized_sha256": "old-hash"},
            ]
            new_cases = [
                {"case": "same", "baseline_normalized_sha256": "same-hash"},
                {"case": "changed", "baseline_normalized_sha256": "new-hash"},
            ]

            rebased, changed = rebase_ledger(
                old, old_cases, groups, new_cases, "new-base"
            )
            by_case = {row["case"]: row for row in rebased}
            self.assertEqual(changed, {"changed"})
            self.assertEqual(by_case["same"]["status"], "complete")
            self.assertEqual(by_case["same"]["final_baseline_wins"], "13")
            self.assertEqual(by_case["changed"]["status"], "pending")
            self.assertEqual(by_case["changed"]["final_baseline_wins"], "")
            self.assertEqual(by_case["same"]["baseline"], "new-base")

    def test_stable_batch_completes_different_group_and_case(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            groups = Path(temporary) / "groups.csv"
            fields = [
                "case",
                "group_id",
                "relation_to_baseline",
                "representative_commit",
                "representative_short_commit",
                "representative_asm_source_commit",
                "member_commits",
            ]
            with groups.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                writer.writerows(
                    [
                        {
                            "case": "case",
                            "group_id": "case:G001",
                            "relation_to_baseline": "identical",
                            "representative_commit": "same-full",
                            "representative_short_commit": "same",
                            "representative_asm_source_commit": "same-full",
                            "member_commits": "same-full",
                        },
                        {
                            "case": "case",
                            "group_id": "case:G002",
                            "relation_to_baseline": "different",
                            "representative_commit": "other-full",
                            "representative_short_commit": "other",
                            "representative_asm_source_commit": "other-full",
                            "member_commits": "other-full",
                        },
                    ]
                )
            ledger = initialize_ledger(groups)
            result = {
                "case": "case",
                "reference_label": "other",
                "baseline_label": "base",
                "baseline_wins": "13",
                "reference_wins": "0",
                "ties": "0",
                "baseline_average_us": "100",
                "baseline_median_us": "100",
                "reference_average_us": "110",
                "reference_median_us": "110",
                "discarded_pairs": "0",
                "classification": "stable_dominance",
                "batch_valid": "True",
            }
            apply_batch(ledger, [result], "evidence", 1)
            summaries = summarize_cases(ledger, "base")
            self.assertEqual(ledger[1]["status"], "complete")
            self.assertEqual(summaries[0]["status"], "complete")
            self.assertEqual(
                summaries[0]["classification"], "stable_dominance"
            )

    def test_strict_best_requires_second_batch_and_aggregate_19_wins(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            groups = Path(temporary) / "groups.csv"
            fields = [
                "case",
                "group_id",
                "relation_to_baseline",
                "representative_commit",
                "representative_short_commit",
                "representative_asm_source_commit",
                "member_commits",
            ]
            with groups.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=fields)
                writer.writeheader()
                writer.writerow(
                    {
                        "case": "case",
                        "group_id": "case:G001",
                        "relation_to_baseline": "different",
                        "representative_commit": "other-full",
                        "representative_short_commit": "other",
                        "representative_asm_source_commit": "other-full",
                        "member_commits": "other-full",
                    }
                )
            ledger = initialize_ledger(groups)
            base_result = {
                "case": "case",
                "reference_label": "other",
                "baseline_label": "base",
                "baseline_wins": "9",
                "reference_wins": "4",
                "ties": "0",
                "baseline_average_us": "100",
                "baseline_median_us": "100",
                "reference_average_us": "101",
                "reference_median_us": "101",
                "discarded_pairs": "0",
                "classification": "strict_best",
                "batch_valid": "True",
            }
            apply_batch(ledger, [base_result], "batch1", 1)
            self.assertEqual(ledger[0]["status"], "needs_second_batch")
            second = dict(base_result, baseline_wins="10", reference_wins="3")
            apply_batch(ledger, [second], "batch2", 2)
            self.assertEqual(ledger[0]["status"], "complete")
            self.assertEqual(ledger[0]["final_baseline_wins"], "19")


if __name__ == "__main__":
    unittest.main()
