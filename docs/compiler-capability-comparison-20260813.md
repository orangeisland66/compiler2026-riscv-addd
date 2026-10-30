# 两个编译器仓库能力对比报告

评估日期：2026-08-13  
对比对象：

- 当前仓库：`/home/orangeisland66/code/compiler/master`，提交 `c7eba48`，RISC-V RV64GC。
- 对比仓库：`/home/orangeisland66/code/compiler/compiler2026-arm-addd`，提交 `7eb3770`，AArch64 Armv8-A。

## 一、结论先行

如果问题是“哪个仓库的整体编译器能力更强”，结论是：**`compiler2026-arm-addd` 更强，领先比较明显。**

但这是一个“分目标架构”的结论：

- 面向 RISC-V 比赛提交，当前仓库才是直接目标，不能用 ARM 后端能力替代 RISC-V 验证。
- 面向 AArch64 性能优化、向量化、成本模型和证明驱动工程，`compiler2026-arm-addd` 明显更成熟。
- 面向简单、清晰、聚焦的 RISC-V 编译链，当前仓库结构更直接，学习和维护入口更容易理解。

综合排序：

| 维度 | `master` RISC-V | `compiler2026-arm-addd` AArch64 | 胜者 |
|---|---|---|---|
| SysY 基础语言能力 | 强 | 强 | 持平 |
| 编译链完整性 | 完整 | 更完整 | ARM |
| 常规 SSA/CFG 优化 | 强 | 强 | ARM 略优 |
| 高级循环/数组优化 | 强 | 很强 | ARM |
| 向量化 | 基本没有成熟默认向量后端 | NEON/VPlan/tuple/interleave 已形成 | ARM |
| 目标成本模型 | RV64 级别成本处理 | Cortex-A53 多资源成本模型 | ARM |
| 后端工程能力 | RV64GC、MIR、linear scan | AArch64、MIR、A53 调度、向量 emitter/admission | ARM |
| 测试与证明材料 | 有 124 项 pass 台账和大量证据 | 有 634 个测试文件、169 份 ARM 证据和形式化证明归档 | ARM |
| 默认路径合规收敛 | 仍有 7 项 Block 风险 | strict/research 边界已形成审计闭环 | ARM |
| RISC-V 目标适配 | 直接可用 | 不支持 RISC-V | master |
| AArch64 目标适配 | 不支持 | 直接可用 | ARM |

## 二、实测对比

两个仓库均使用各自目标架构的交叉编译器、运行时库和 QEMU 完成相同规模的 SysY 测试。

### `master` RISC-V

- strict 构建成功。
- Python 工具测试：18 项全部通过。
- 功能用例 O0：`140/140`。
- 功能用例 O1：`140/140`。
- 性能用例 O1：`60/60`，完成编译、链接和 `qemu-riscv64` 执行。
- 仓库内 108 个 `.sy` 语义夹具均可在 O0/O1 成功生成汇编。
- 重点语义脚本：default recognizers、remaining pass semantics、compliance risks、loop semantic optimizations 均已通过；循环脚本使用实际性能用例路径后通过。

### `compiler2026-arm-addd` AArch64

- strict `build-audit` 构建成功。
- 仓库测试 Python 单测：24 项全部通过。
- 功能用例 O1：`140/140`。
- 性能用例 O1：`60/60`。
- 现有 ARM 证据还记录 optimized/no-opt functional `140/140`、五种 compliance 模式 `76/76`，以及完整性能计时、汇编变化、固定 ELF 和回归报告。
- 最新严格隔离材料记录：`SYCC_ENABLE_RESEARCH_PASSES=OFF` 的 strict 构建成功；research binary 未设置 `SYCC_RESEARCH=1` 时与 strict 在 60 个性能输入上汇编 `0/60 changed`。

两个架构的 QEMU 运行时间不能直接横向比较：RV64GC 与 Armv8-A 的指令集、模拟器、运行时和目标微架构不同。本报告只比较能力、覆盖和证据，不把跨 ISA 的毫秒数当作性能排名。

## 三、为什么 ARM 仓库整体更强

### 1. 优化实现规模更大、层次更丰富

静态规模统计：

| 指标 | `master` | `compiler2026-arm-addd` |
|---|---:|---:|
| C++ 源文件 | 100 | 138 |
| 头文件 | 103 | 123 |
| `src` 总 C++ 行数 | 约 75,702 | 约 101,582 |
| 优化实现 `.cpp` 数量 | 约 69 | 约 87 |
| 后端实现行数 | 约 11,378 | 约 16,303 |

ARM 仓库额外具备：

- AArch64/NEON 向量化和 VPlan-lite 候选规划；
- `ld2/ld3/ld4`、`st2/st3/st4` 等交错向量内存形式；
- Vector128 tuple 分配和向量 epilogue；
- Cortex-A53 延迟、吞吐、地址、代码尺寸、寄存器压力成本模型；
- pressure-aware reduction、trace layout、lifetime-hole reuse；
- affine cache tiling、domain split、exact-window linearization、unroll-and-jam；
- machine planning、shadow RA、emitter admission 和事务式候选提交。

当前 RISC-V 仓库的 CoreIR/HIR/MIR 能力已经很强，但主要集中在标量、循环、数组和通用机器级优化，尚未形成与 ARM 仓库同等成熟的向量后端和目标特化规划层。

### 2. ARM 仓库的“计划—验证—提交”闭环更成熟

ARM 优化普遍采用：

```text
候选分析 → planning clone → verifier → shadow RA/emitter admission
→ 成本模型比较 → 事务式提交，否则回退 baseline
```

这比当前 `master` 中部分 pass 直接在默认路径物化新算法阶段更稳健。ARM 仓库还将变换前提、负例、回退和固定 ELF 结果归档到 `governance/evidence/arm/`，形成了持续性能回归和证明追踪链。

### 3. ARM 仓库的 strict/research 边界更严密

ARM 的 `src/opt/research_mode.h` 要求 semantic-changing 研究开关同时满足：

1. 编译时 `SYCC_ENABLE_RESEARCH_PASSES=ON`；
2. 运行时显式 `SYCC_RESEARCH=1`；
3. 单个 pass 的 flag 不能绕过前两个门槛。

其审计记录还明确说明 strict/research binary 在未打开 research opt-in 时对 60 个性能输入汇编保持 `0/60 changed`。

当前 `master` 虽然也有 `SYC_ENABLE_RESEARCH_PASSES` 和 CMake 源文件隔离，但仍有多个严格默认路径中的算法级变换，以及 `SYC_SKIP_*`、`MIR_NO_SCHEDULE`、`MIR_SIMPLE_RA` 等环境旁路问题。根据此前的合规报告，当前仓库仍有 7 项 Block finding。

## 四、当前 RISC-V 仓库的优势

ARM 仓库并非在所有方面都胜出。

### 1. RISC-V 目标链路是当前仓库的直接交付能力

`master` 的 `src/backend_mir/` 直接生成 RV64GC 汇编，处理 `medany`、RV64 ABI、RISC-V spill/reload 和目标机器清理；ARM 仓库不能直接替代它完成 RISC-V 提交。

### 2. 当前仓库的结构更容易快速理解

当前主链路由 `src/main.cpp`、`src/core_ir/passes/pipeline_builder.cpp`、`src/core_ir/passes/passes.cpp` 和 `src/backend_mir/mir_backend.cpp` 串接，目录层次相对扁平。ARM 仓库采用 `src/opt/`、`src/mir/`、多个 planner/admission 模块，能力更强但理解和维护成本也更高。

### 3. RISC-V 仓库的专项语义夹具更集中

当前仓库的 `tests/riscv/` 和 `default_pass_semantics.tsv` 直接围绕 RISC-V 默认流水线组织，适合作为单一目标架构的合规回归基线。ARM 仓库测试和证据更丰富，但分布在 `tests/`、`testcases/`、`governance/evidence/arm/`、`build-local/` 多个层次。

## 五、合规与风险对比

### `master` 的主要风险

- 递归 radix lowering、demand-driven copy、cross-call array summary、stage fusion、producer-map、recursion memoization 仍在 strict 默认路径。
- 部分变换改变算法阶段或复杂度，现有正负夹具不能替代通用证明。
- strict 路径存在非 research 环境旁路。
- 默认 `-O1` 内部使用较激进的性能预算，公开等级和内部策略不完全一致。

### `compiler2026-arm-addd` 的主要风险

- 优化器更复杂，约 101k 行 `src` 代码和大量 planner/admission 模块增加维护难度。
- 当前工作树存在未提交文档/证据修改，不能简单把工作树状态等同于一个干净发布版本。
- ARM 仓库也使用大量 `COREIR_DISABLE_*`/`MIR_DISABLE_*` 环境开关；其设计已通过 research 总开关收口，但发布前仍应保持 strict 环境不变性测试。
- ARM 的部分性能证据依赖固定 Cortex-A53 成本模型或 QEMU，不能直接外推到所有 AArch64 处理器。

在“默认路径合规成熟度”这一项，ARM 当前明显优于 `master`；在“优化数量和目标性能潜力”上，ARM 也优于 `master`，但复杂度更高、审计维护成本更大。

## 六、最终判定

### 如果按“整体编译器工程能力”比较

**`compiler2026-arm-addd` 更强。**

理由是它不仅有完整前端、中端和后端，还具备当前仓库没有的向量化、AArch64 目标成本模型、复杂机器 admission、持续性能报告和更严密的 strict/research 证明闭环。

### 如果按“RISC-V 比赛提交能力”比较

**`master` 更适合作为 RISC-V 目标基础，但当前还不能称为更强的整体实现。**

它的 RISC-V 后端和端到端验证已通过当前 140/60 用例，但应先完成合规报告中的 P0 整改；否则 ARM 仓库在“可审计、可复现、证明闭环”方面仍占优势。

### 如果按“研究优化上限”比较

**ARM 仓库明显更强。**

尤其在矩阵、卷积、排序、`sl`、Huffman、NEON 向量化、tiling、unroll-and-jam、向量 reduction 和机器压力建模方面，ARM 仓库已经形成可持续迭代的优化平台；当前 RISC-V 仓库更像一个优化覆盖面较广但仍需收敛默认边界的版本。

## 七、建议

1. 如果目标是 RISC-V 参赛：以 `master` 为基线，优先移植 ARM 仓库的 planner/admission、证明门禁和测试组织方式，而不是直接移植 AArch64 指令变换。
2. 如果目标是继续做编译器优化研究：以 `compiler2026-arm-addd` 为主线，其分析设施、向量化和成本模型更值得继续投入。
3. 如果目标是形成统一编译器：抽取两仓库共有的 AST/CoreIR/analysis/证明框架，将 `master` 的 RV64 backend 和 ARM 的 AArch64 backend 作为不同 target 后端接入；这是长期最优方向，但工作量显著高于简单合并。

