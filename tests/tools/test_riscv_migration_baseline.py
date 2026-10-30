import importlib.util
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "riscv_migration_baseline", ROOT / "tools/riscv_migration_baseline.py"
)
BASELINE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader
SPEC.loader.exec_module(BASELINE)


class MigrationBaselineTests(unittest.TestCase):
    def test_assembly_metrics_are_content_based(self):
        with tempfile.TemporaryDirectory() as directory:
            assembly = Path(directory) / "renamed.s"
            assembly.write_text(
                ".text\nrenamed:\n  lw a0, 0(a1)\n  sw a0, 4(a1)\n  bnez a0, renamed\n  ret\n"
            )
            metrics = BASELINE.assembly_metrics(assembly)
            self.assertEqual(metrics["instructions"], 4)
            self.assertEqual(metrics["loads"], 1)
            self.assertEqual(metrics["stores"], 1)
            self.assertEqual(metrics["branches"], 2)
            self.assertEqual(len(metrics["sha256"]), 64)

    def test_discovery_is_recursive_and_stable(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "nested").mkdir()
            (root / "evidence").mkdir()
            (root / ".hidden").mkdir()
            (root / "z.sy").write_text("int main(){return 0;}")
            (root / "nested" / "a.sy").write_text("int main(){return 0;}")
            (root / "evidence" / "historical.sy").write_text(
                "int main(){return 0;}"
            )
            (root / ".hidden" / "temporary.sy").write_text(
                "int main(){return 0;}"
            )
            self.assertEqual(
                BASELINE.discover(root),
                [root / "nested" / "a.sy", root / "z.sy"],
            )


if __name__ == "__main__":
    unittest.main()
