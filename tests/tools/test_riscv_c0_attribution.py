#!/usr/bin/env python3

import importlib.util
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "riscv_c0_attribution", ROOT / "tools/riscv_c0_attribution.py"
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader is not None
SPEC.loader.exec_module(MODULE)


class C0AttributionTest(unittest.TestCase):
    def test_normalization_ignores_nonsemantic_file_metadata(self):
        left = '  .file "a.sy"\n  addi a0, a0, 1\n  .ident "one"\n'
        right = '  .file "b.sy"\n\taddi\ta0, a0, 1\n  .ident "two"\n'
        self.assertEqual(
            MODULE.normalized_assembly(left), MODULE.normalized_assembly(right)
        )

    def test_machine_metrics_classify_stack_and_control(self):
        metrics = MODULE.assembly_metrics(
            "\tlw a0, 8(sp)\n\tsw a1, 12(sp)\n\tbnez a0, .L1\n\tcall f\n\tli a2, 9\n"
        )
        self.assertEqual(metrics["instructions"], 5)
        self.assertEqual(metrics["loads"], 1)
        self.assertEqual(metrics["stores"], 1)
        self.assertEqual(metrics["stack_accesses"], 2)
        self.assertEqual(metrics["conditional_branches"], 1)
        self.assertEqual(metrics["calls"], 1)
        self.assertEqual(metrics["address_materializations"], 1)

    def test_first_difference_reports_both_sides(self):
        self.assertEqual(
            MODULE.first_difference("a\nb\n", "a\nc\n"),
            {"line": 2, "baseline": "b", "candidate": "c"},
        )


if __name__ == "__main__":
    unittest.main()
