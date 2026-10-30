import importlib.util
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "check_long_performance_regression",
    ROOT / "tools/check_long_performance_regression.py",
)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC.loader
SPEC.loader.exec_module(MODULE)


class LongPerformanceRegressionTests(unittest.TestCase):
    def test_only_long_cases_are_checked(self):
        baseline = {"short": 9.9, "long": 100.0}
        candidate = {"short": 1000.0, "long": 104.9}
        self.assertEqual(
            MODULE.check_regressions(baseline, candidate, 10.0, 5.0), []
        )

    def test_regression_and_missing_candidate_fail(self):
        baseline = {"slow": 20.0, "missing": 30.0}
        candidate = {"slow": 21.1}
        failures = MODULE.check_regressions(baseline, candidate, 10.0, 5.0)
        self.assertEqual([item[0] for item in failures], ["missing", "slow"])
        self.assertGreater(failures[1][3], 5.0)

    def test_csv_requires_successful_numeric_rows(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "report.csv"
            path.write_text("case,status,run_ms\na,PASS,12\n", encoding="utf-8")
            self.assertEqual(MODULE.read_report(path, "run_ms"), {"a": 12.0})

    def test_microsecond_metric_uses_microsecond_threshold(self):
        self.assertEqual(
            MODULE.threshold_in_metric_units("internal_us", None, 10000.0),
            (10000.0, "us"),
        )

    def test_default_and_legacy_thresholds_convert_to_metric(self):
        self.assertEqual(
            MODULE.threshold_in_metric_units("internal_us", None, None),
            (10000.0, "us"),
        )
        self.assertEqual(
            MODULE.threshold_in_metric_units("run_ms", 10.0, None),
            (10.0, "ms"),
        )


if __name__ == "__main__":
    unittest.main()
