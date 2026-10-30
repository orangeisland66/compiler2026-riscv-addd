"""校验 RISC-V 优化语义台账的完整性、中文说明、证据路径和隔离状态。"""

from __future__ import annotations

import csv
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
REGISTRY = ROOT / "tests/riscv/default_pass_semantics.tsv"
ENV_REGISTRY = ROOT / "tests/riscv/environment_switch_classes.tsv"

EXPECTED_IDS = set(
    """
ast_simplify ast_radix_sort ast_demand_copy ast_cross_call_summary
ast_tail_recursion ast_recursive_accumulator ast_matmul_interchange
ast_reflective_max ast_linear_recurrence_fusion array_sroa
const_global_load_forward local_load_forward single_block_mem2reg
global_scalar_mem2reg dce const_fold ir_algebraic constant_strength_reduction
parity_canonicalize basic_block_cse dominator_cse local_gep_rebase
merge_linear_blocks simplify_cfg dead_store_elim pow2_digit_recognition
dead_local_materialization
direct_global_scalar_promotion loop_canonicalize gvn pre gcm sccp loop_lvn
licm invariant_load_hoist modular_scalar_recurrence repeated_reduction
small_linear_recurrence row_locality row_locality_k_block bit_accumulator
bit_pattern recursive_modmul function_specialization dead_return_elim
inst_combine pure_call_last_value loop_state_fixpoint tiny_pure_inline
single_caller_loop_inline overwrite_repeat_loop loop_unswitch
divisor_specialization runtime_bound_localize stage_fusion producer_map_bypass
row_broadcast_fusion loop_band_canonicalize transpose_view_forwarding
if_convert_accumulation reduction_split stencil_interior loop_unroll
single_trip_unroll loop_blocking affine_loop_tightening affine_scalar_rewrite
array_address_recurrence local_store_load_forward dead_global_array_store
recursive_memoization tail_recursion_coreir late_inline early_return_call_guard
scalar_global_forward small_global_array_forward unobserved_global_store_removal
invariant_store_hoist correlated_value_propagation bounded_signed_rem
small_diamond_ifconvert signed_bit_closure branch_compare_split
phi_edge_normalize redundant_global_reload_store dead_function_elimination
loop_fusion loop_interchange loop_scalar_accumulator loop_memory_accumulator
loop_continue_bound jump_threading zero_coeff_specialization
mir_lower mir_power2_divrem machine_invariant_constant_hoist
machine_symbol_hoist machine_singleton_la_hoist machine_extended_la_hoist
machine_loop_address_reduction machine_loop_address_stride_combine
machine_even_path_halving machine_signed_pow2_bias_fold
machine_boolean_normalization machine_producer_move_fold
machine_redundant_constant_elim machine_spill_reload_forward
machine_adjacent_copy_fold machine_separated_copy_fold machine_dead_def_elim
machine_parity_masked_accumulation early_return_shrinkwrap pre_ra_fp_schedule
linear_scan_ra simple_ra_debug loop_branch_rotate list_schedule block_layout
peephole cmp_branch_fusion post_ra_store_cleanup post_ra_load_hoist frame_lower
asm_print
""".split()
)

ALLOWED_STATUS = {
    "PASS_独立正负",
    "PASS_标准语义",
    "PASS_组合回归",
    "DEFAULT_UNOBSERVED",
    "RESEARCH_ONLY",
    "DORMANT",
    "DEBUG_ONLY",
}

# 这些变换会删除循环、调用或访存阶段，必须有独立的正负夹具，或明确隔离。
PROOF_REQUIRED = {
    "ast_radix_sort",
    "ast_demand_copy",
    "ast_cross_call_summary",
    "ast_tail_recursion",
    "ast_recursive_accumulator",
    "pure_call_last_value",
    "loop_state_fixpoint",
    "stage_fusion",
    "producer_map_bypass",
    "transpose_view_forwarding",
    "if_convert_accumulation",
    "reduction_split",
    "repeated_reduction",
    "modular_scalar_recurrence",
    "row_locality",
    "function_specialization",
    "overwrite_repeat_loop",
    "divisor_specialization",
    "row_broadcast_fusion",
    "stencil_interior",
    "array_address_recurrence",
    "dead_global_array_store",
    "dead_local_materialization",
    "recursive_memoization",
    "tail_recursion_coreir",
    "recursive_modmul",
    "machine_loop_address_reduction",
}

STRICT_ISOLATED = {
    "ast_radix_sort": "src/hir/radix_sort_lowering.cpp",
    "ast_demand_copy": "src/hir/demand_driven_copy_transform.cpp",
    "ast_cross_call_summary": "src/hir/cross_call_array_summary.cpp",
    "producer_map_bypass": "src/core_ir/passes/producer_map.cpp",
    "stencil_interior": "src/core_ir/passes/producer_map.cpp",
    "transpose_view_forwarding": "src/hir/transpose_view_forwarding.cpp",
}

# stage_fusion.cpp also owns the default row-broadcast pass. Its research-only
# map/reduce mutation therefore uses a compile boundary inside the mixed TU;
# strict builds contain the shared proof helpers but no mutation or public
# applyMapRowReduceChecksumFusion definition.
COMPILE_GUARDED_RESEARCH = {
    "stage_fusion": "src/core_ir/passes/stage_fusion.cpp",
}


def contains_chinese(text: str) -> bool:
    return re.search(r"[\u4e00-\u9fff]", text) is not None


def read_registry() -> list[dict[str, str]]:
    with REGISTRY.open(encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


class PassSemanticRegistryTest(unittest.TestCase):
    def test_registry_has_exactly_the_audited_passes(self) -> None:
        rows = read_registry()
        ids = [row["id"] for row in rows]
        self.assertEqual(len(ids), len(set(ids)), "台账中存在重复 pass id")
        self.assertEqual(set(ids), EXPECTED_IDS)

    def test_every_row_has_chinese_semantics_and_examples(self) -> None:
        required = {
            "id",
            "阶段",
            "状态",
            "实现",
            "入口",
            "正确语义",
            "正例",
            "负例",
            "证据",
        }
        for row in read_registry():
            self.assertEqual(set(row), required)
            for key in required:
                self.assertTrue(row[key].strip(), f"{row['id']} 缺少 {key}")
            self.assertIn(row["状态"], ALLOWED_STATUS)
            for key in ("阶段", "正确语义", "正例", "负例"):
                self.assertTrue(
                    contains_chinese(row[key]), f"{row['id']} 的 {key} 不是中文说明"
                )

    def test_implementation_and_evidence_paths_exist(self) -> None:
        for row in read_registry():
            for key in ("实现", "证据"):
                for value in row[key].split(";"):
                    path = ROOT / value.strip()
                    self.assertTrue(path.exists(), f"{row['id']} 引用不存在：{value}")

    def test_registered_entry_exists_in_implementation(self) -> None:
        for row in read_registry():
            implementation = "\n".join(
                (ROOT / value.strip()).read_text(encoding="utf-8")
                for value in row["实现"].split(";")
            )
            for entry in row["入口"].split(";"):
                self.assertIn(entry.strip(), implementation, row["id"])

    def test_proof_required_passes_have_independent_evidence_or_are_isolated(
        self,
    ) -> None:
        by_id = {row["id"]: row for row in read_registry()}
        acceptable = {"PASS_独立正负", "RESEARCH_ONLY", "DORMANT"}
        for pass_id in PROOF_REQUIRED:
            self.assertIn(by_id[pass_id]["状态"], acceptable, pass_id)

    def test_source_has_no_benchmark_identifier_trigger(self) -> None:
        # 只检查实现源码；测试夹具和普通术语不属于触发条件。
        banned = re.compile(
            r"\b(?:crc[123]|conv2d-[123]|huffman-0[123]|matmul[123]|"
            r"transpose[012]|sl[123]|many_mat_cal-[123])\b",
            re.IGNORECASE,
        )
        findings: list[str] = []
        for path in (ROOT / "src").rglob("*"):
            if path.suffix not in {".cpp", ".h"}:
                continue
            text = path.read_text(encoding="utf-8")
            if banned.search(text):
                findings.append(str(path.relative_to(ROOT)))
        self.assertEqual(findings, [])

    def test_algorithm_level_passes_are_removed_from_strict_objects(self) -> None:
        by_id = {row["id"]: row for row in read_registry()}
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("set(RESEARCH_SOURCES", cmake)
        self.assertIn("list(REMOVE_ITEM STRICT_SOURCES ${RESEARCH_SOURCES})", cmake)
        self.assertIn("set(COMPILER_SOURCES ${STRICT_SOURCES})", cmake)
        for pass_id, implementation in STRICT_ISOLATED.items():
            self.assertIn(
                by_id[pass_id]["状态"], {"RESEARCH_ONLY", "DORMANT"}, pass_id
            )
            self.assertIn(implementation, cmake, pass_id)
        for pass_id, implementation in COMPILE_GUARDED_RESEARCH.items():
            self.assertEqual(by_id[pass_id]["状态"], "RESEARCH_ONLY", pass_id)
            source = (ROOT / implementation).read_text(encoding="utf-8")
            self.assertRegex(
                source,
                re.compile(
                    r"#ifdef SYC_ENABLE_RESEARCH_PASSES\s+"
                    r"bool materializeMapRowReduceChecksumFusion\(.*?"
                    r"bool applyMapRowReduceChecksumFusion\(Module&.*?"
                    r"#endif",
                    re.DOTALL,
                ),
                pass_id,
            )

    def test_environment_reads_use_the_strict_research_gateway(self) -> None:
        direct_reads: list[str] = []
        for path in (ROOT / "src").rglob("*"):
            if path.suffix not in {".cpp", ".h"} or path.name == "research_switch.h":
                continue
            if re.search(r"(?:std::)?getenv\(", path.read_text(encoding="utf-8")):
                direct_reads.append(str(path.relative_to(ROOT)))
        self.assertEqual(direct_reads, [])

        gateway = (ROOT / "src/research_switch.h").read_text(encoding="utf-8")
        self.assertIn('sycRawEnvironmentValue("SYC_RESEARCH")', gateway)
        self.assertIn("sycResearchBuildEnabled()", gateway)
        self.assertIn("sycEnvironmentValueIsObservabilityOnly", gateway)
        self.assertIn("SyCEnvironmentClass::Rollback", gateway)
        self.assertIn("SyCEnvironmentClass::SemanticEnable", gateway)

        with ENV_REGISTRY.open(encoding="utf-8", newline="") as stream:
            rows = list(csv.DictReader(stream, delimiter="\t"))
        self.assertEqual(
            {row["class"] for row in rows},
            {"OBSERVABILITY", "ROLLBACK", "SEMANTIC_ENABLE", "UNKNOWN_SEMANTIC"},
        )
        for row in rows:
            if row["class"] != "OBSERVABILITY":
                self.assertEqual(row["strict_behavior"], "ignored")

    def test_plan_uses_current_official_suite_status(self) -> None:
        plan = (ROOT / "plan.md").read_text(encoding="utf-8")
        evidence = (
            ROOT / "tests/riscv/evidence/20260813/PLAN_IMPLEMENTATION.md"
        ).read_text(encoding="utf-8")
        self.assertNotIn("M8", plan)
        self.assertNotIn("M8", evidence)
        self.assertNotIn("外部正式功能/性能套件与本轮 `run_ms` 基线未提供", plan)
        self.assertIn("正式功能 O0/O1 140/140", plan)
        self.assertIn("正式性能 60/60", plan)
        self.assertIn("2026初赛RISCV赛道功能用例", evidence)
        self.assertIn("2026初赛RISCV赛道性能用例", evidence)


if __name__ == "__main__":
    unittest.main()
