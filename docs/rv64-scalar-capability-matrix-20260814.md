# RV64 标量能力对齐矩阵（2026-08-14）

本矩阵按可执行合同和当前流水线状态分类，不按文件数量评价能力。C0 性能账本
现已完成；M7 回退尚未关闭，因此本文不作发布性能通过结论。

| 能力 | 分类 | 当前合同 |
|---|---|---|
| 三态 proof、analysis epoch、planning clone/rollback | equivalent | Unknown 不授权；失败候选保持原 Module 与 Value ID |
| pointer range、object/range alias、MemorySSA-lite | RV64-adapted | owned object、byte range 与无环全路径可证明；回边/未知调用 fail-closed |
| 函数 effect/mod-ref/termination/capture/return 摘要 | equivalent | 模块迭代传播，调用点按参数和 global 查询 |
| 局部 CSE、load/store forwarding、dead local/field writes | RV64-adapted | 统一 alias/MemorySSA facts，候选经 verifier 与真实 RV64 admission |
| i32 strength reduction、recurrence、unroll、loop/address canonicalization | RV64-adapted | 保持 W/XLEN、wrap、zero-trip、live-out 与成本边界 |
| linear-scan lifetime holes、PHI edge copy、stable spill home | RV64-adapted | RV64 psABI、同位置 PHI、并行复制与跨迭代栈 home |
| BOOM target profile 与机器指标 | RV64-adapted | profile 已版本化；未获得具体核参数前 latency/throughput 不标定 |
| fusion、stencil producer/consumer、selective memoization | research-only | 三重 opt-in；proof/verify/admission 任一失败即回退 |
| pure-call last value、loop-state fixpoint、transpose view | research-only | 当前 dormant，不计作已接线能力 |
| RVV/自动向量化 | deferred-RVV | `hasV=false`，不发射 V 指令 |
| NEON tuple/interleave、NZCV/CCMP、AAPCS64 | not-applicable-AArch64 | 不迁移到 RV64GC |

## 验证摘要

- strict/research Release 构建、31 项 Python 工具测试、pass registry 与 strict
  audit 通过。
- planning、loop semantic、remaining pass、compliance risk 和 codegen
  determinism 门禁通过。
- 109 个仓库语义用例的 O0/O1 汇编在 strict 注入研究开关、research 未完整
  opt-in 时均与 strict 基准逐项相同。
- `rv64_abi_pressure.sy` 在 O0/O1 下经 RV64GC/medany 静态链接和 QEMU 动态通过。
- 正式功能 O0/O1 本轮均为 `140/140`。
- C0 已重建 60 点账本：48 点首差在 CoreIR、12 点在 MIR/frame；唯一性能指标
  `total_internal_us` 回退 57.20%（辅助 `run_ms` 回退 5.32%），故 M7 仍未通过。
