# ARM 分析/证明/规划体系接入 RV64 后端实施计划

## 提交合规整改（2026-08-14）

按照根目录 PDF 的第三条第（5）项，证明档案未与当前实现闭环的整函数/整区域
算法替换不得进入提交 strict。`radix_sort_lowering`、
`demand_driven_copy_transform` 和 `cross_call_array_summary` 已重新加入 CMake
research source list；strict 二进制不含实现符号，research 还需
`SYC_RESEARCH=1` 与具体 `SYC_ENABLE_*` 同时设置。该整改使正式性能集相对整改前
有 `9/60 changed`：`01_mm1..3`、`03_sort1..3`、`transpose0..2`，后续性能计划
不得把其历史收益计入提交配置。

同时修复 simple RA 紧邻完整宽度 spill/reload 的 ABI 参数栈往返；稳定 home store
保留，reload 仅在同一 FrameIdx 或 frame lowering 后同一 `imm(sp)` 且物理源仍
可用时改成 move，真实 overflow stack arguments 保留。提交证据见
`tests/riscv/evidence/20260814/submission-compliance/README.md`。

## matmul 收口与下一步（2026-08-14）

matmul 阻断已通过通用 symmetric producer-rowmin 融合关闭。候选在 reduction
split 前完成循环、对象、仿射访问、transpose 关系、effect、live-out 与 CFG/Phi
闭包证明，再经 clone/verifier 和真实 RV64 admission 提交；未证明的非对称路径
仍保持 research-only。相对同二进制 rollback 仅 `matmul1/2/3` 汇编改变。

唯一性能指标的单轮正序为 master/current `76000 -> 70000 us`（改善 `7.89%`），
反序为 current/master `70000 -> 78000 us`（current 改善 `10.26%`）。辅助
`run_ms` 仅用于诊断。正式 strict `-O1` 功能集为 `140/140 PASS`；原始数据与
静态指标见 `tests/riscv/evidence/20260814/matmul-recovery/README.md`。

下一步 RISC-V 优化按以下顺序执行：

1. **P0，重建内部计时账本。** 对本阶段所有 changed cases 做一次固定顺序
   `master -> current`，另加 byte-identical control；只用 `internal_us` 排序新的
   top regressions，`run_ms` 仅排查环境异常。退出条件是逐项汇编首差、内部计时和
   family 汇总均可复核。
2. **P1，关闭已知小回退。** 优先复核 `many_mat_cal` 的 `108000 -> 113000 us`；
   若汇编差异仍在，按 CoreIR/MIR/RA 首差定位 if-conversion 的 load、branch、spill
   和 frame 成本。只修改共享 proof 或 RV64 成本合同，不按 family 排除。
3. **P2，扩展低风险标量/内存收益。** 从 must-alias MemorySSA、地址递推、W 指令
   选择、立即数 materialization、冗余扩展和局部 branch/select 中选择有真实热路径
   减少的候选。每项提供触发正例、alias/effect/位宽近似负例和 changed-only 动态
   证据。
4. **P3，标定 RV64 admission。** 用正式样本校准热循环分支、load-use、乘除、
   GPR/FPR 压力、spill 和 frame 的权重；容量只作为 legality/风险边界，收益仍由
   post-RA 机器指标决定，不从单个 benchmark 反推魔数。
5. **P4，发布里程碑。** P1/P2 收口后才执行预先约定的完整 `60/60` 串行内部计时
   和完整 correctness/compliance。高风险算法融合、memoization 与 RVV 在证明和
   目标能力不完整时继续留在 research，不进入该里程碑。

当前 matmul 子目标完成，但 M7 仍需上述全量内部计时和 compliance risk 阻断关闭
后才能改为发布候选。

## C0-C6 收口记录（2026-08-14）

本轮后续已按请求完成 C0 性能账本和首差归因。C0 是定位里程碑，不等于 M7；
当前工作树仍存在已定位的性能回退，不能表述为已通过性能发布门槛。

- C0：独立导出并构建 `master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`
  基线；正式 60 点为 `0 identical / 60 changed`，其中 48 点首差在 CoreIR、12 点
  首差在 MIR/frame。唯一性能指标 `total_internal_us` 为
  `701000 -> 1102000`（回退 `57.20%`）；辅助 `run_ms` 回退 `5.32%`，
  byte-identical control 的辅助墙钟为 `9.657 -> 9.778 ms`。逐项收益与原始数据见
  `tests/riscv/evidence/20260814/c0-attribution/C0_ATTRIBUTION.md`。
- C1/C2：在已有分区 MemorySSA、调用点 mod/ref、observer demand 与事务式 dead
  local materialization 上，补齐常量范围 partial-observer field store slicing；动态
  GEP、部分重叠、escape 和未知调用继续拒绝。
- C3：remaining、loop semantic 与 compliance risk 全量门禁通过。低风险
  recurrence/unroll/address/loop 优化继续按正负例触发；缺 streaming recurrence
  证明的 4x memory-loop fallback 已撤销。row locality 的不合法 PHI 候选、stage
  fusion 和 stencil 的不完整 region 候选在事务 verifier/transform 阶段回退。
- C4：增加版本化 `boom-rv64gc-uncalibrated` profile；在具体 BOOM 配置未知时不
  填造 latency。机器指标新增热循环资源、load-use、mul/div、地址物化与保守压缩
  机会。新增 12 整数参数、10 浮点参数、PHI 轮换、深调用与 5KB frame 的 O0/O1
  动态压力门禁，RV64GC/medany 汇编、链接、QEMU 均通过。
- C5：stage fusion、stencil/producer 与 memoization 保持 research-only，均经过
  clone/verify/admit/rollback；缺 proof 或出现 pressure/code-growth 时不提交。
- C6：strict/research Release、`git diff --check`、31 项 Python、9 项 registry、
  12 项 strict audit、planning/determinism/ABI/remaining/loop/compliance 门禁通过。
  109 个仓库语义用例的 O0/O1 逐项汇编在 strict 注入研究开关、research 未完整
  opt-in 两种情形下均为 `0 changed`。
  能力矩阵见 `docs/rv64-scalar-capability-matrix-20260814.md`。正式功能 O0/O1 的
  本轮结果为 `140/140`（13.5s）/`140/140`（51.5s）；C0 的 60 点动态正确性也为
  `60/60`，但性能回退使 M7 发布结论继续保留。

## 实施记录（2026-08-13）

本轮已经完成首轮可交付基础设施，而不是把尚未证明的 ARM 变换直接并入
strict：

- WP0：新增可重复基线工具，核对 `master`/`master-new` 锚点并记录逐用例
  hash、代码尺寸、指令分类、编译时间和工具链；
- WP1：落地三态 proof、pointer range、alias、loop access/dependence、
  MemorySSA-lite、保守 i32 integer facts、machine-exact/runtime-bounded
  trip recipe，以及 `AnalysisManager` epoch/失效协议；
- WP2：落地完整 Module planning clone、增强 IR verifier 和
  prove→clone→transform→verify→admit→commit 事务；
- WP3：正式与 planning 共用 RV64 machine pipeline，增加 target/cost model、
  machine verifier、压力/spill/frame/实际汇编尺寸指标及 fail-closed admission；
- A2/A3：补齐函数语义摘要、ordered effect、write coverage、statement
  dependence、region state equivalence 与 observer closure，并接入缓存失效和 dump；
- Phase 4 pilot：常量强度削减已提供 research-only 的完整事务接线，实际产生
  `accepted`、`candidate-transform-rejected`、`no-machine-benefit` 三类结果；
- Phase 4 扩展：局部等价组、stage fusion、stencil、loop fusion/interchange、
  loop accumulator、jump threading、zero coefficient 和 recursive memoization 的
  research 入口接入候选事务；row locality 等 strict 变换继续沿既有独立证明合同，
  不因 planning 基础设施自动获得新的授权；
- strict/research：代码生成环境开关统一由编译期选项、`SYC_RESEARCH=1` 和
  具体开关三重门控，环境变量有显式分类注册表，strict 对注入的语义开关保持
  当前 108 个仓库语义夹具 `0/108 changed`，research 未 opt-in 与 strict 也是
  `0/108 changed`；
- 当前重跑：迁移夹具 O0/O1 均为 108/108，analysis dump 为 `0/108 changed`；
  随后在正式 RISC-V 功能 140 点与性能 60 点目录完成动态复核和静态影响面扫描。
  详细路径、正确性与 changed-only 性能结果见
  `tests/riscv/evidence/20260813/PLAN_IMPLEMENTATION.md`。

额外修复了两类通用正确性问题：拒绝 planning clone 后恢复全局 Value ID，及
MIR 循环地址归纳按稳定块序选择代表。后者在 `conv2d-1.sy` 上由跨进程不同汇编
收敛为 6/6 相同 SHA-256。

Phase 4–6 中尚未逐项完成 proof archive 的复杂 ARM 变换继续保持
research-only；RVV 因目标能力明确为 `V=false` 未进入 strict。也就是说，
本记录把“计划基础设施已完成”和“未来每个优化均已迁移”严格区分，避免用
未验证移植冒充计划完成。

2026-08-14 的后续实施补齐了 candidate 六阶段状态、按 pass 拒绝/提交/耗时/IR
增长统计，并修复 reduction split 将相邻 i32 结果错误降低为裸指针 `+1 byte`
的 P1。官方功能集重跑 O0/O1 均为 140/140。与此同时，按指定 master 锚点扫描
官方 60 点得到 21 个 changed cases；changed-only 单轮总运行时间回退 85.9%，
所以 Phase 7/M7 仍明确未完成，不能用 correctness 通过替代性能退出条件。

同日 G1 继续补齐按底层对象登记的 MemorySSA 分区、四个统一内存版本/观察者
查询和 Values/Memory/CFG/Calls/ModuleGlobals 失效类别；未知 alias/call/path
事实保持 fail-closed。该批次只建立分析合同，尚未把未证明的 region/cross-call
算法替换恢复到 strict。

G2 首批已将标量 load CSE/store forwarding/同值 store 清理和同块 DSE
改为消费 MemorySSA-lite 公共查询。授权仅来自 SSA、等宽 must-alias、完整
覆盖、无 clobber 和无 observer 事实；未知 call、may-alias、partial overwrite 仍
拒绝。正式 O1 功能集 `140/140`；仅反向本批 diff 的 source A/B 为
`57/60` 汇编相同，`transpose0..2` changed-only 单轮合计改善 `15.1%`，
byte-identical control 基本不变。该结果不改变 Phase 7/M7 对 master 基线仍未达标的结论。

G4/G5 基础已新增按指针形参索引的 read/write 摘要、统一 call-site
mod/ref 查询和从 observers 逆向构造的 region value demand slice。unknown external、
未支持 opcode 和普通指针形参别名均 fail-closed；demand slice 本身不授权删除
effect/trap/control。正式 O1 功能 `140/140`，本批静态影响为
`57/60 identical`，`huffman-01..03` changed-only 单轮合计改善 `10.1%`，
control 基本不变。基础 capture/returned 形参索引也已进入摘要；跨块
memory version 已覆盖无分支、无合流、无回边的唯一 CFG 链，更一般的
dominance/post-dominance 合流证明仍未完成。region value demand 之外的纯标量
子图已有独立删除证明和事务式 clone/verify/admission 接口；首个合同候选因 MIR
清理已得到相同机器产物而以 `no-machine-benefit` 拒绝，带部分 observer、跨对象
或 producer-consumer 状态的通用 region deletion 仍未完成。当前 clone commit 仍是整 Module
替换，因此已强制使用 Module 级缓存失效，避免细粒度声明遗留悬空分析键。

后续已完成 dead local materialization 的首个 strict 闭环：仅对 sized、
nonescaping alloca 的完整 use closure 证明所有访问均为对象内常量范围 store，
再经 clone/verifier/RV64 admission 删除对象与写入。load、动态 GEP、call、
return/capture 和未知 use 均拒绝。配套收紧 DCE，使运行时未知除数/移位范围、
未证明可解引用的 dead load，以及可能不终止/陷阱的 pure call 不会因 store 删除
而被连带吞掉。正式 O1 功能 `140/140`；正式性能集 `33/60 identical`，27 个
changed cases 输出全正确。changed-only 单轮运行合计
`3857.574 -> 3769.548 ms`，但 control 同轮波动
`12.431 -> 8.889 ms`，因此不宣称稳定收益；更一般的跨对象、路径敏感 observer
和 producer-consumer materialization slicing 仍未完成。

partial-observer 首批也已闭环：同一 nonescaping alloca 的所有 load/store 都有
对象内常量 byte range 时，可以仅删除与全部读取区间均不相交的 store。例如两字段
对象只读取字段 0 时删除字段 1 写入；同字段读写、部分重叠、动态 GEP 或 escape
均拒绝。该候选与整对象删除合并到一次事务和一次 RV64 admission，避免每个字段
重复运行 shadow backend。正式 O1 功能仍为 `140/140`；相对上一批正式性能 60 点
汇编 `60/60 identical`，因此本批不运行动态性能，也不宣称收益。

本轮还关闭了两个证据缺口：宽模式 if-conversion 在提前执行 load 的研究正例上
曾把输出从 `4400` 错编为 `0`，现已 fail-closed 到 strict 的无 load 子集；
`pure_call_last_value` 与 `loop_state_fixpoint` 自基线提交 `0f18dc6e` 起没有流水线
入口，台账已改为 `DORMANT`，不再把保留实现和旧夹具计作已接线能力。

M7 性能回退定位（2026-08-14）新增两项通用修复：过程内联不再把“单 caller、两个
call site”的循环过程当作单调用者候选；部分展开 fallback 也不再对缺少 recurrence
证明的短访存循环默认 4x 展开。`01_mm`、`matmul`、`transpose` changed-only
复核均保持正确，但相对 master 的单轮运行回退仍存在，因此 M7 继续保持未完成。

### 当前完成度矩阵

| 阶段 | 状态 | 已有证据 | 尚缺出口条件 |
|---|---|---|---|
| Phase 0 | 完成 | strict/research Release；108 点 O0/O1 可重建基线；正式功能 O0/O1 140/140；正式性能 60/60 及分项计时；60 点静态影响面与 changed-only 对比基线 | 无 |
| Phase 1 | 核心完成 | A1–A3、integer facts/trip、缓存失效、dump `0/108 changed` | 更复杂 induction range/runtime guard 仍只返回 Unknown |
| Phase 2 | 核心完成 | 完整 clone、IR verifier、坏候选回退、Value ID 恢复 | 尚非 SMT/refinement 语义验证器 |
| Phase 3 | 核心完成 | 真实 RV64 pipeline、machine verifier、metrics、fail-closed admission、可执行 RV64GC 能力/成本合同 | latency/收益模型仍未绑定具体 RV64 微架构和正式性能样本 |
| Phase 4 | 本轮完成（高风险项隔离） | 低风险优化正负例通过；复杂 region/state 候选均为 research-only 且事务失败回退 | dormant/research 项不得在缺 proof archive 时恢复 strict |
| Phase 5 | 本轮完成 | source-list 分离、环境注册表、109 点 strict/research O0/O1 汇编等价 | 后续仅作不改变合同的渐进式文件拆分 |
| Phase 6 | C0 归因与正确性验收完成 | 31 项 Python、planning/remaining/loop/compliance/ABI/determinism 门禁、正式功能 O0/O1 140/140、60 点性能正确性、109 点隔离等价 | M7 回退关闭与性能发布结论 |
| Phase 7 / M7 | 未达到发布候选 | 当前 60 点均 changed；唯一性能指标内部 timer 回退 57.20%；辅助 `run_ms` 回退 5.32%，control +1.2% | 修复 blocking/unroll、inline 与 divisor-versioning 成本回退并重新验收 |

## 0. 当前分支与合并边界

- 当前工作分支：`master-new`。
- 2026-08-13 已将本地 `master-new` 以 `--ff-only` 快进到最新远端提交
  `0f18dc6e22872e1277dd42780a45de8a4015f123`；本地 `master` 与
  `origin/master` 均保持在 `c7eba4837484dab6b80e19dabf610ebfa78a02fc`。
- 后续实施只允许在 `master-new` 进行，不再切换、修改、合并或提交
  `master`。每个里程碑均用 `git rev-parse master master-new` 检查这一不变量。
- 当前工作区已有的审计报告、能力评估报告和对比报告继续保留在 `master-new`；这些文件不回写 `master`。
- `build-audit/` 是本地构建产物，不作为迁移交付物；不得为了清理它而影响现有
  工作区，也不得误纳入源码提交。
- 本计划不采用“把 ARM 仓库整体合并进来”的方式。两个仓库目标架构不同，应合入 ARM 的**目标无关分析、证明、规划和验证基础设施**，再由 RV64 后端实现独立 target adapter。

### 0.1 计划依据与信息边界

- 已按请求检索会话 `019ffae0-e8af-7f72-a9b7-f95b5f6c2f99`。本机当前
  session index、history 投影和 SQLite thread history 中已无该会话正文，仅日志
  WAL 保留了 ID 引用，无法可靠复原逐条对话；本计划不臆造缺失内容。
- ARM 侧事实以相邻仓库 `../compiler2026-arm-addd` 的 `AGENTS.md`、
  `docs/compliance/semantic_proofs.md`、`src/analysis/`、
  `src/opt/core_ir/`、`src/opt/aarch64/a53_cost_model.*`、
  `src/mir/emit_admission.*` 和 `src/mir/pipeline.*` 为准。
- RV64 侧事实以当前 `master-new` 的 CoreIR、`AnalysisManager`、
  `PipelineBuilder`、`backend_mir` 和 strict/research 构建边界为准。
- 迁移原则是“证明合同优先、目标无关实现适配、机器部分重写”；不得用文件级
  copy 代替接口审查，也不得把 ARM 的历史性能结论当作 RV64 验收证据。

## 1. 目标与非目标

### 1.1 最终目标

在当前 RISC-V 编译器中建立如下默认流水线：

```text
SysY AST
  → CoreIR SSA
  → 目标无关分析与语义证明
  → 事务式候选规划/克隆
  → RV64 成本模型与机器 admission
  → RV64 MIR
  → linear-scan RA / spill / schedule / layout
  → verifier
  → RV64GC 汇编
```

最终要求：

1. 高级变换必须先完成 legality、effect/alias、终止、live-out 和未定义行为边界证明。
2. 规划阶段必须比较 baseline 与 candidate 的真实 RV64 MIR、寄存器压力、spill、栈帧、指令数和代码尺寸。
3. 候选未通过 verifier 或机器 admission 时，自动回退原始 IR，不留下半完成改写。
4. strict 构建与运行环境无关；semantic-changing 研究能力只能由构建宏加统一 research opt-in 同时打开。
5. 任何默认优化都不得依赖文件名、函数名、变量名、公开 benchmark 骨架或固定输入。

### 1.2 非目标

- 不直接移植 ARM NEON 指令、AArch64 ABI、A53 指令延迟常量或 AArch64 汇编 emitter。
- 不在第一阶段引入 RVV；当前目标是 RV64GC，不应假设存在 `V` 扩展。
- 不把 ARM 的性能数字直接套用到 RV64；所有成本阈值必须由 RV64 target model 重新标定。
- 不在证明闭环完成前把 memoization、算法级融合、公开算法骨架替换等高风险变换重新放入 strict 默认路径。
- 不为了接入 ARM 体系重写已经稳定的 SysY 前端和 RV64 基础 lowering。

## 2. 总体架构设计

### 2.1 分层原则

将能力拆成三层：

| 层次 | 内容 | 是否目标相关 |
|---|---|---|
| Analysis | CFG、支配、循环、effect、alias、MemorySSA-lite、仿射区域、依赖、observer、状态等 | 目标无关 |
| Proof/Planning | legality、refinement、候选 clone、成本候选、回退、变换提交 | 主要目标无关，少量 target hook |
| Target backend | RV64 指令成本、寄存器/栈容量、spill、调度、汇编 admission | RV64 专属 |

目标无关层只输出候选及证明事实，不生成 RV64 指令；RV64 层只回答“该候选在真实目标机器上是否值得且可发射”。

### 2.2 与当前代码的接入点

- AST/入口：`src/main.cpp`。
- CoreIR 流水线：`src/core_ir/passes/pipeline_builder.cpp`、`src/core_ir/passes/passes.cpp`。
- 分析基础：`src/analysis/`。
- CoreIR IR 与 lowering：`src/core_ir/ir/`、`src/core_ir/lowering/`。
- RV64 MIR：`src/backend_mir/`。
- strict/research 构建：`CMakeLists.txt`、`src/research_switch.h`。
- 语义台账和夹具：`tests/riscv/default_pass_semantics.tsv`、`tests/riscv/`、`tests/tools/`。

建议新增或逐步抽取的模块：

```text
src/analysis/
  loop_access.*
  loop_dependence.*
  pointer_range.*
  alias_analysis.*
  function_semantic_summary.*
  memory_ssa_lite.*
  affine_tile.*
  region_write_coverage.*
  region_state_equivalence.*
  ordered_region_effect.*
  statement_dependence_relation.*
  observer_chain_plan.*

src/core_ir/planning/
  planning_clone.*
  candidate_plan.*
  legality_proof.*
  refinement_check.*
  machine_admission.*

src/backend_mir/
  rv64_cost_model.*
  rv64_machine_planning.*
  rv64_admission.*
```

实际命名可以按当前目录风格调整，但接口职责必须保持分离。

### 2.3 ARM 到 RV64 的迁移矩阵

| ARM 来源 | RV64 落点 | 处理方式 | 第一验收证据 |
|---|---|---|---|
| `src/analysis/loop_access.*`、`loop_dependence.*`、`pointer_range.*`、`alias_analysis.*` | `src/analysis/`，接入现有 `FunctionAnalysisCache` | 适配 CoreIR Value/GEP/CFG，不照搬类型系统 | analysis-only 汇编 hash 不变；alias/dependence 正负例 |
| `memory_ssa_lite.*`、`function_semantic_summary.*`、`ordered_region_effect.*` | `src/analysis/`，由 `AnalysisManager` 管理 epoch | 移植算法，重写 effect/call 映射 | CFG/effect 失效测试；unknown 保守回退 |
| `affine_region.*`、`affine_tile.*`、`statement_dependence_relation.*` | 扩展当前 `affine_region`、`loop_band` | 合并而非建立第二套仿射 IR | permutation/tile legality 近似负例 |
| `region_write_coverage.*`、`region_state_equivalence.*`、`observer_chain_plan.*` | `src/analysis/` | 证明框架移植，observer 绑定 use-def/effect | partial overwrite、escape、call observer 拒绝 |
| `function_planning_clone.*`、`loop_region_clone.*` | `src/core_ir/planning/` | 按当前所有权模型重写 clone/commit | clone 失败原模块序列化 hash 不变 |
| `verify_module.*`、`verify_helpers.*`、semantic proof contracts | 扩展 `src/core_ir/passes/verify.*`，新增 proof result/reason | 保留三态合同，补当前 IR invariants | 坏 PHI、支配、live-out、trap 负例 |
| `machine_planning.*`、`src/mir/emit_admission.*` | `src/backend_mir/rv64_machine_planning.*`、`rv64_admission.*` | 复用流程思想，机器实现全部 RV64 化 | baseline/candidate 两次 shadow pipeline 可比较 |
| `a53_cost_model.*` | `src/backend_mir/rv64_cost_model.*` | 只复用接口形状，成本常量重新标定 | 每项成本有来源、版本和保守 fallback |
| ARM optimization passes | `src/core_ir/passes/` 或 research sources | 逐 pass 迁移，先证明后 mutation | 每 pass 独立 positive/near-miss/rollback |
| NEON/AAPCS64/emitter | 无 | 禁止移植 | 静态扫描不存在 AArch64 opcode/ABI 假设 |

### 2.4 当前 RV64 后端必须先补的接口

当前 `src/backend_mir/mir_backend.cpp` 的 `runMIRBackend()` 把 lowering、cleanup、
pre-RA schedule、linear scan、layout、peephole、frame lowering 和文本输出串成一个
`void` 流程。它不能直接用于候选 admission，因为调用方拿不到中间 MIR、失败原因
或机器指标。因此 Phase 3 不能只增加一个成本表，必须先把现有流程拆成：

```text
lowerForPlanning(CoreIR) -> MachineModule
runMachinePipeline(MachineModule, PipelineMode) -> MachinePipelineResult
collectMachineMetrics(MachineModule) -> MachineMetrics
emitChecked(MachineModule) -> EmitResult
runMIRBackend(CoreIR, ostream) -> 沿用上述正式路径
```

`MachinePipelineResult` 至少携带 verifier 状态、未分配 vreg、GPR/FPR pressure、
spill/reload、stack slot/frame bytes、指令分类、基本块/分支和最终汇编字节数。
planning 和最终发射必须调用同一实现，禁止维护一套“估算后端”和另一套“正式后端”。

## 3. 分阶段实施路线

### Phase 0：分支基线与可重复构建

状态：已完成。分支锚点、strict 可重复构建、108 个仓库夹具 compile-only 基线、
正式功能 O0/O1 140/140、60 点汇编影响面和 changed-only 单轮运行时间基线均已有
记录；性能回退属于 Phase 7 阻断项，不再作为 Phase 0 的“路径缺失”。

任务：

1. 固定 `master-new@0f18dc6e` 为迁移基线，记录 `master@c7eba483` 和工作区文件清单。
2. 使用 `SYC_ENABLE_RESEARCH_PASSES=OFF` 完成 strict configure/build。
3. 建立 RV64 runtime、交叉 GCC、QEMU 的统一路径配置，禁止脚本依赖隐含工作目录。
4. 记录 baseline：O0/O1 汇编 hash、汇编字节数、指令数、frame、spill、compile time、60 点性能运行时间。
5. 运行现有静态测试、功能测试和语义脚本，保存 `master-new` 初始证据。

出口条件：

- strict 构建成功；
- 提供正式外部套件后，O0/O1 功能 runner 枚举用例全部通过；历史参考规模为
  `140`，但必须报告本轮实际 `passed/discovered`，不得沿用历史结果；
- 提供正式外部套件后，性能集正确性全部通过；历史参考规模为 `60`，但必须
  报告本轮实际 `passed/discovered`，不得沿用历史结果；
- baseline 报告可由单条命令重建；
- 不修改 `master`。

### Phase 1：抽取目标无关分析层

状态：A1–A3 核心接口和 integer facts/trip recipe 已完成；分析只通过查询/dump
暴露，不参与新增变换授权。更复杂 runtime range/guard 仍保守返回 Unknown。

目标：先移植 ARM 的分析能力，不改变任何生成汇编。

优先顺序：

1. `loop_access`、`loop_dependence`、`pointer_range`、`alias_analysis`。
2. `function_semantic_summary`、`memory_ssa_lite`、ordered effect。
3. `affine_region`、`statement_dependence_relation`、`region_write_coverage`。
4. `region_state_equivalence`、`repeated_region_summary`、`observer_chain_plan`。
5. `integer_facts`、trip-count、induction-range 和终止分析增强。

实施要求：

- 分析结果必须使用 CoreIR Value/Instruction/BasicBlock、SSA、CFG 和 use-def，不读取源文件路径或普通符号名称。
- 所有分析提供 `unknown`/`disproved`/`proved` 三态结果，未知一律保守回退。
- 分析缓存必须接入 `AnalysisManager` 的失效协议。
- 先提供 dump/trace 和单元测试，暂不修改 IR。

出口条件：

- 分析-only 模式下 O0/O1 汇编逐字节不变；
- CFG、Phi、MemorySSA-lite、effect 和 alias 单元测试通过；
- 为每个分析新增至少一个正例、一个近似负例和一个未知回退例。

实现批次固定为 `A1 -> A2 -> A3`，每批单独合入，禁止一次性导入 ARM 的全部
`src/analysis`：

- `A1`：CFG/loop/access/pointer/alias，建立 AnalysisManager epoch 和 dump 格式；
- `A2`：MemorySSA-lite/effect/semantic summary，闭合 call 与内存观察；
- `A3`：affine/dependence/region state/observer，只产出 proof facts。

### Phase 2：建立证明与事务式候选框架

状态：核心框架已完成。它验证结构合法性和机器可承载性，但不是完整语义
refinement 求解器；每个 pass 的 legality facts 仍必须独立证明。

目标：把 ARM 的 planning clone、verifier、shadow admission 思路接入 CoreIR。

#### 2.1 Candidate Plan 接口

每个候选至少包含：

- 输入函数、循环、region 和观察者集合；
- 变换前提及证明事实；
- baseline snapshot；
- candidate clone；
- 预期变换、失效分析集合和回退原因；
- 目标成本摘要。

#### 2.2 Planning clone

- 在临时 Module/Function 中克隆候选 region。
- 所有 PHI、CFG 边、内存对象和调用摘要必须保持可验证映射。
- clone 失败、dominance 失败、类型不一致或 effect 不闭合时只销毁 clone，不触碰原模块。

#### 2.3 Proof verifier

逐项验证：

1. 输入输出类型和位宽；
2. CFG 可达性、支配、回边和退出边；
3. SSA/use-def/PHI 完整性；
4. alias/mod-ref、memory order 和 observer closure；
5. 终止、零迭代、单迭代、边界 trip count；
6. 除零、非法移位、signed overflow/未定义行为边界；
7. 所有 live-in/live-out、全局状态和调用副作用；
8. baseline 与 candidate 的 refinement/等价条件。

#### 2.4 事务式提交

只有 `proof=proved` 且 `machine_admission=accepted` 才提交 candidate；否则回退 baseline 并记录稳定 reason code。

提交协议固定为：`discover -> prove -> clone -> transform clone -> verify clone ->
machine admit clone -> commit/replace -> invalidate analyses`。不得先修改原函数再尝试
修复；不得把异常或 verifier crash 当作普通拒绝继续生成汇编。

出口条件：

- verifier 对坏 PHI、别名、live-out、调用和零迭代负例均能拒绝；
- candidate 失败时原始模块 hash 不变；
- 每个候选均能生成 default/skip 对照汇编。

### Phase 3：构建 RV64 target model 与机器 admission

状态：通用 RV64GC profile、真实机器 pipeline、verifier、metrics 和 admission
已完成；planning contract 现验证 I/M/A/F/D/C、V=false、16 字节栈对齐及 DIVW
相对 ADDW 的保守成本排序。具体微架构 latency 标定与正式性能收益验收尚未完成。

目标：将 ARM A53 cost model 替换为 RV64GC 可解释模型。

#### 3.1 RV64 能力描述

显式记录：

- `RV64I/M/A/F/D/C` 能力；
- 当前是否启用 `V` 扩展（默认否）；
- i32/i64、整数乘除、浮点和 load/store 指令代价；
- caller/callee-saved GPR/FPR 数量；
- outgoing argument、spill slot、栈对齐和 frame 限制；
- branch、call、load-use、mul/div/rem 的近似延迟；
- medany 地址物化和大偏移成本。

#### 3.2 RV64 cost model

候选评分至少包含：

- 静态指令数和基本块数；
- load/store/call/branch 数；
- 整数和浮点寄存器压力峰值；
- spill/reload 数、spill bytes、frame bytes；
- loop-carried dependency 和关键路径；
- code growth、启动成本和 remainder 成本；
- 目标特性不支持时的明确回退。

成本模型必须只依赖 target/IR facts，不使用公开测试用例名称或固定输入规模。

#### 3.3 真实机器 admission

在 planning clone 上运行当前 RV64：

```text
CoreIR candidate
  → RV64 MIR lowering
  → shadow linear-scan RA
  → spill/reload insertion
  → schedule/layout/peephole
  → verifier + metrics
```

若 candidate 使 frame、spill、寄存器压力或代码尺寸超过预算，则拒绝 candidate；不允许“先改 IR，后发现后端无法承载”。

出口条件：

- candidate/default/rollback 的 RV64 汇编可重现；
- admission 能识别 spill 增长、非法 ABI、未分配 vreg、frame 超限和汇编 lowering 失败；
- 现有 RV64 代表性程序的 baseline 汇编保持不变。

#### 3.4 实施顺序

1. `B1`：从 `runMIRBackend()` 抽出可复用 machine pipeline，正式输出逐字节不变。
2. `B2`：增加 machine verifier 和结构化 `MachineMetrics`，仅观测不决策。
3. `B3`：同一 CoreIR 分别跑 baseline/candidate shadow pipeline，稳定输出 reason code。
4. `B4`：加入 RV64 cost/admission policy；未知目标参数只允许保守拒绝。

`B1/B2` 在 analysis-only 配置下必须保持全量汇编 `0 changed`，否则不得进入 B3。

### Phase 4：按风险从低到高移植优化

状态：仅完成低风险 pilot 与若干 research-only 事务接线。复杂 region/state
变换没有因“可事务回退”而视为已证明；宽 if-conversion 已因动态误编重新隔离。

#### 4.1 第一批：低风险、局部等价

优先接入：

1. signed power-of-two quotient/remainder；
2. local/affine store-load forwarding；
3. zero-store bundle 和 dead-write region slicing；
4. scalar memory recurrence promotion；
5. pointer/address recurrence；
6. same-value store elimination；
7. basic-block CSE、局部 load CSE、常量聚合传播。

每个 pass 必须经过 Phase 2 的 proof verifier 和 Phase 3 的 RV64 admission。

#### 4.2 第二批：循环和仿射变换

在第一批稳定后接入：

- exact partial unroll/peeling；
- loop rotation、loop canonicalization；
- affine domain split；
- conservative cache tiling/blocking；
- unroll-and-jam；
- linear/repeated/modular recurrence；
- transpose/locality forwarding。

默认只接受 proof 完整且机器成本为正的候选；不满足条件则保留原循环。

#### 4.3 第三批：复杂 region 与状态变换

仅在独立 proof archive 和广泛负例完成后评估：

- pairwise affine fusion；
- stencil low-rank/producer-consumer；
- wavefront schedule；
- selective recursive memoization；
- affine accumulator memoization；
- last-observable/overwritten iteration contraction。

任何可能改变算法复杂度、跨调用状态或中间存储观察行为的变换，默认先放入 research-only。

#### 4.4 向量化路线

- 本计划周期固定目标为 RV64GC，strict 与 research 均不实现、不启用 RVV。
- `RV64TargetInfo::hasV` 保持 `false`，正式链接参数保持 `-march=rv64gc`；不得新增
  RVV opcode、vector register class、vector emitter 或自动向量化 pass。
- ARM 的 NEON/VPlan/tuple/interleave 能力只作为未来独立项目的参考，不计入本轮
  ARM 能力对齐率，也不得以标量伪向量化或未声明 ISA 指令绕过该边界。
- 只有官方 ISA、FPGA bitstream、工具链、OS vector context 和实机探针都确认支持
  RVV 后，才能另立计划；不得在本计划中预埋会改变代码生成的 RVV 开关。

### Phase 5：流水线与配置收口

状态：strict/research source-list 和环境网关已完成，当前 108 点隔离为零差异；
大型 pipeline 文件的职责拆分及 dormant 入口的重新证明仍未完成。

修改范围：

- `src/core_ir/passes/pipeline_builder.cpp`：由能力摘要和 target profile 构建流水线。
- `src/core_ir/passes/passes.cpp`：将候选 pass 接入统一 planning/admission，不允许各 pass 自行绕过 verifier。
- `src/backend_mir/mir_backend.cpp`：暴露 RV64 machine metrics 和 admission 入口。
- `CMakeLists.txt`：拆分 `STRICT_SOURCES` 与 `RESEARCH_SOURCES`。
- `src/research_switch.h`：引入构建宏 + `SYC_RESEARCH=1` + pass flag 的双重门控。

配置原则：

1. strict 构建不得被单个环境变量改变 semantic-changing 默认路径。
2. trace、dump 和 disable 变量可以保留，但必须区分 observability、rollback 和 semantic-changing enable。
3. 所有 target profile 必须显式记录，不能通过宿主机检测推断 RV64 优化。
4. `-O1` 的内部预算必须文档化；若采用激进策略，应在流水线配置中明确命名和测试。

环境变量分为三类并写入注册表：`TRACE/DUMP` 只观测，`DISABLE/SKIP` 只用于
rollback，`ENABLE` 可能改变语义。strict 二进制不得包含 research-only `ENABLE`
路径；源码中的环境读取已统一经过 `research_switch.h` 网关，新增读取也必须遵守
同一分类合同。

### Phase 6：测试、证明和性能证据

状态：仓库内静态、planning、隔离和本地 runtime 语义门禁已完成；正式功能
O0/O1 140/140，正式性能 60/60，并完成汇编影响面扫描；changed-only 对比覆盖
7 个 changed family 和一个 byte-identical control。remaining/loop semantic 全量
门禁仍待运行，当前单轮性能回退阻断发布候选。

#### 6.1 单元与 IR 测试

- Analysis：CFG、dominance、loop、alias、effect、range、affine、MemorySSA-lite。
- Proof：类型、PHI、live-out、observer、termination、UB/trap、memory order。
- Planning：clone/rollback、candidate determinism、reason code、cache invalidation。
- RV64 backend：ABI、call copy、spill/reload、frame、large offset、float、branch、scheduler。

#### 6.2 语义夹具

每个新 pass 必须在 `tests/riscv/` 添加：

- 改名正例；
- 近似形状负例；
- alias/escape 负例；
- side-effect/call 负例；
- zero-trip/one-trip/boundary 负例；
- live-out/observer 负例；
- overflow/div-zero/shift-range 负例。

同步更新 `tests/riscv/default_pass_semantics.tsv` 和门禁脚本。

#### 6.3 端到端门槛

每个阶段结束都必须通过：

1. strict configure/build；
2. Python 静态/单元测试；
3. O0/O1 功能 runner 枚举用例全部通过，并报告 `passed/discovered`；
4. O1 性能 runner 枚举用例正确性全部通过，并报告 `passed/discovered`；
5. default/rollback 汇编比较；
6. 变更点固定 ELF 多轮运行；
7. 编译时间、汇编 bytes/insts、branch/load/store、spill/frame 和内部 timer 报告。

#### 6.4 合规门槛

- Block 命中数必须为 0。
- Proof-required pass 必须有证明文档、正负例、回退证据和可执行审计。
- 正式 60 点性能集在 strict/research 未启用 research 时应保持 `0/60 changed`；
  当前仓库 108 个语义夹具已验证 `0/108 changed`。
- 新增优化不得依赖源文件名、函数名、变量名、固定输入、IO/计时邻域或 benchmark 骨架。

### Phase 7：发布候选与正式性能验收

状态：未完成。只有 Phase 4–6 的剩余 proof archive、正式外部正确性、changed-only
静态影响面、串行多轮性能和最终合规报告全部闭合后，才允许进入 M7。

发布候选必须同时记录 branch/HEAD/baseline ref、正式套件摘要、runtime/toolchain、
`compile_ms`/`link_ms`/`run_ms`/`wall_s`、原始样本和 byte-identical control；不得用
当前仓库夹具 compile-only 结果替代正式性能结论。

## 4. 里程碑与验收门槛

| 里程碑 | 交付物 | 必须通过 |
|---|---|---|
| M0 | `master-new@0f18dc6e` 基线和可重建脚本 | strict build、枚举用例全过、`master@c7eba483` 未修改 |
| M1 | 目标无关分析层 | analysis-only 汇编 `0` 变化，单元测试通过 |
| M2 | planning clone + proof verifier | 所有坏候选回退，原模块不污染 |
| M3 | RV64 cost model + machine admission | spill/ABI/frame/lowering 失败可拒绝 |
| M4 | 第一批低风险优化 | 每 pass 正负例、default/rollback、O0/O1 正确 |
| M5 | 循环/仿射优化 | dependence、remainder、live-out、成本证明闭合 |
| M6 | 复杂 region/状态优化 | research-only 起步，证明归档后再评估 strict |
| M7 | 发布候选 | strict/research 隔离、0 Block、全量性能和合规报告 |

## 5. 风险与应对

| 风险 | 影响 | 应对 |
|---|---|---|
| ARM 分析接口与当前 CoreIR 数据模型不一致 | 高 | 先做 adapter，禁止大面积复制；以当前 CoreIR Value/CFG 为唯一事实源。 |
| ARM 成本模型误用于 RV64 | 高 | 所有 AArch64 常量替换为 RV64 target hook，未标定时拒绝变换。 |
| clone/verifier 失效导致半改写 IR | 高 | 事务式 clone，提交前统一 verifier，失败销毁 candidate。 |
| RV64 无 V 扩展却误启用向量逻辑 | 高 | 默认 target capability 明确无 `V`；本计划 strict/research 均禁止 RVV，未来只能另立项目。 |
| 高级变换引入合规问题 | 高 | 复杂算法/缓存/融合先 research-only，proof archive 后再评估。 |
| 编译时间因规划 clone 激增 | 中 | planning cache、候选预算、增量 machine metrics 和超时回退。 |
| 两条流水线结果不一致 | 中 | O0 direct backend、O1 CoreIR/MIR、default/skip 三方差分。 |
| ARM 代码量导致维护困难 | 中 | 每个 pass 独立目录、统一 reason code、台账和证据，不允许巨型 pipeline 文件继续增长。 |

## 6. 推荐执行顺序

1. 先完成 M0，冻结 `master-new` 基线并确认 `master` 不再修改。
2. 只移植 ARM 的分析接口和 proof contract，完成 M1/M2；此阶段不追求性能。
3. 接入 RV64 target model、shadow RA 和 machine admission，完成 M3。
4. 以 affine copy、signed pow2、address recurrence、局部内存优化作为第一批收益来源。
5. 再接入循环/仿射变换；每个 pass 单独测量收益与回退原因。
6. 将 stage fusion、producer-map、memoization 等复杂变换留在 research-only，直到证明和合规门槛满足。
7. 本计划不讨论或实现 RVV；NEON/RVV 能力从 RV64GC 标量交付范围中明确排除。

### 6.1 首轮可直接开工的四个工作包

| 工作包 | 修改范围 | 明确不做 | 完成标志 |
|---|---|---|---|
| WP0 基线 | `tools/`、`tests/tools/`、证据文档 | 不改优化逻辑 | 一条命令产出 branch/hash/build/test/asm metrics |
| WP1 分析骨架 | `AnalysisManager`、A1 分析、单元测试 | 不修改 IR | O0/O1 汇编逐字节不变 |
| WP2 事务 clone | `src/core_ir/planning/`、verifier 测试 | 不接复杂 pass | 正/坏 candidate 均可确定 commit/reject |
| WP3 RV64 admission | `backend_mir` pipeline/result/metrics | 不移植 A53 常量 | 正式输出不变且 shadow metrics 可重现 |

WP0 与 WP1 可先后推进；WP2 依赖 WP1 的事实和失效协议；WP3 可在 WP1 后独立
实现，但只有 WP2+WP3 同时完成后才允许迁移第一个会改变 IR 的 ARM pass。

## 7. 完成定义

该计划只有在以下条件全部满足时才算完成：

- `master-new` strict 构建可重复，`master` 分支没有任何新提交或修改；
- ARM 目标无关分析、proof、planning 和 admission 能在当前 CoreIR/RV64 链路运行；
- 所有默认变换均由通用 IR/目标事实触发；
- 所有 candidate 都经过 proof verifier 和 RV64 machine admission；
- O0/O1 功能与性能 runner 枚举用例全部通过，并记录发现数量；
- 新增 pass 的证明、正负夹具、回退和性能证据齐全；
- strict/research 隔离满足 0 Block、research 未启用时汇编不变；
- 发布报告同时给出正确性、合规性、编译时间、代码尺寸和目标性能数据。

## 8. 向 ARM 标量能力靠齐的后续计划（暂不实现 RVV）

### 8.1 对齐口径

本阶段的“向 ARM 能力靠齐”不是复制 AArch64 后端，也不是追求两个仓库源码文件
一一对应，而是在 RV64GC/BOOM 约束下补齐 ARM 已形成的四类工程能力：

1. 目标无关的分析、证明、候选规划和事务式回退；
2. 不依赖 NEON/RVV 的标量、内存、循环、仿射和过程间优化；
3. 面向 BOOM 的 RV64GC 成本模型、寄存器分配、调度、布局和机器准入；
4. strict/research 隔离、逐 pass 证据、正式套件 correctness 与性能回归闭环。

以下 ARM 专属能力不纳入本轮差距统计：NEON/VPlan、Vector128、tuple/interleave、
`ld2/ld3/ld4`、`st2/st3/st4`、AAPCS64、NZCV/`CCMP`、A53 专属成本常量和 AArch64
emitter。当前目标能力固定为 `RV64IMAFDC`、`V=false`、RISC-V psABI、16 字节栈
对齐和 `medany`。

### 8.2 当前差距与优先级

| 能力域 | 当前状态 | 与 ARM 的主要差距 | 优先级 |
|---|---|---|---|
| Proof/planning | 三态 proof、Module clone、verifier、RV64 admission 已有 | 各复杂 pass 尚未逐项闭合 proof archive | P0 |
| Memory/effect | MemorySSA-lite、call-site mod/ref、observer demand slice 已有 | 跨块 memory version、region deletion、完整 capture/escape 闭环不足 | P0 |
| 标量优化 | 常规 SSA、CSE/GVN/PRE、内联、部分 recurrence 已有 | ARM 的局部 aggregate、精确 recurrence、dead-write region 等通用能力未齐 | P1 |
| 循环/仿射 | 基础 loop、unroll、blocking、locality 及部分 research 候选已有 | domain split、exact window、unroll-and-jam、融合/交换证明与盈利判断不足 | P1 |
| 过程间优化 | 函数语义摘要和参数级 mod/ref 基础已有 | 摘要 SCC 收敛、返回/捕获传播、调用点特化与失效精度不足 | P1 |
| RV64 后端 | MIR、linear scan、schedule/layout、frame、medany 已有 | BOOM 标定、lifetime-hole、spill 成本、调度资源模型和 ABI 压力测试不足 | P0 |
| 复杂算法变换 | 多项实现或 research 入口存在 | 通用证明、observer/termination 闭包和 target profitability 尚未闭合 | P2，research-only |
| 向量化 | `hasV=false` | 本阶段明确不对齐 ARM NEON | 不实施 |

当前第一阻断项仍是 M7：相对计划基线 `master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`
的最终 60 个 changed cases 唯一性能指标内部 timer 回退 `57.20%`；辅助
`run_ms` 回退 `5.32%`，而 byte-identical control 的辅助墙钟仅 `+1.2%`。未关闭
该回退前，不新增默认复杂
变换，不用更多优化掩盖现有回退，也不把 correctness 全过表述为性能已达标。

### 8.3 实施顺序与里程碑

#### C0：冻结目标与重建 M7 性能账本

状态：完成。60 个 changed case 均记录首个 CoreIR/MIR 差异层级、逐项汇编 hash、
机器指标、family 触发能力和一轮 `B -> C` 动态结果；正式功能集中的
`00_main.sy` 作为 byte-identical control。证据见
`tests/riscv/evidence/20260814/c0-attribution/`。归因结果把主要阻断定位到
blocking/unroll、过度 inline 和 divisor versioning，而通用 frame/prologue 收缩
总体减少栈访问。C0 已完成不表示这些回退已修复。

目标：获得可重复的逐 case、逐 pass、逐机器阶段归因，先回答回退发生在 CoreIR、
MIR lowering、RA、schedule/layout 还是 frame/emitter。

任务：

1. 记录当前 branch/HEAD、基线 ref/完整提交号、两侧 compiler、runtime、GCC、QEMU
   和正式套件摘要；基线必须独立导出构建，不得用当前工作树 clean HEAD 替代。
2. 对 60 个性能用例生成 baseline/candidate `-S -O1` 汇编，规范化后重新列出
   changed cases；保存 bytes、inst、branch、call、load/store、frame 和 spill。
3. 对 changed cases 生成 CoreIR/MIR/pass statistics，建立 `pass -> changed case ->
   machine delta -> run_ms` 映射；名称只用于报告，不参与任何优化判定。
4. 按共享根因聚类，优先检查已知重灾区对应的内联、展开、地址递推、load/store
   消除、RA spill、调度和 block layout；每次只改一个通用合同。
5. 日常只串行运行 changed cases 和至少一个 byte-identical control，固定 `B -> C`
   一轮；只有 M7 最终验收才运行完整 60 点和预先约定的稳定性采样。

出口条件：每个 changed case 都有首个 IR/MIR 差异点和机器指标归因；无法归因的
case 不得进入“已修复”统计；所有修复保持正式功能 O1 `140/140`。

#### C1：关闭共享内存与过程间分析缺口

状态：首批完成。新增 analysis-only 的全无环路径同版本查询，diamond 合流只有在
每条路径均无相关写入时返回 proved，任一分支 clobber、回边或未知调用仍返回
Unknown；现有 CSE/DSE 继续使用原唯一链查询，因此默认优化行为不变。函数语义
摘要改为模块级迭代传播，并携带参数 read/write/capture/return、written globals、
termination 与跨调用 trap 事实。region demand 同时登记 value、effect 和 memory
demand，未知内存效果 fail-closed。正式性能 60 点相对实施前汇编 `0/60 changed`，
开启 analysis dump 也是 `0/60 changed`；正式 O1 功能 `140/140`。更一般的循环
MemoryPhi、可执行 region deletion 和消费新查询的默认优化仍属于后续 C1/C2 工作。

目标：达到 ARM 分析层的保守表达能力，为后续优化提供统一事实源，但不直接授权
算法级替换。

任务：

1. 将 MemorySSA-lite 从同块精确查询扩展到 dominance/path 一致时的跨块版本；
   merge、loop backedge、unknown call 和 may-alias 路径保持 `Unknown`。
2. 完成 function summary 的 SCC/fixpoint 传播，覆盖参数级 read/write、capture、
   returned pointer、termination、trap 和 globals；版本变化必须使调用者缓存失效。
3. 闭合 region write coverage、observer closure、value demand 与 memory demand；
   “未被值 slice 使用”本身不得授权删除 effect/trap/control。
4. 为 pointer range、object identity、offset/byte range、partial overwrite 和 escape
   建立统一查询，禁止各 pass 重复实现弱化版 alias 判断。

出口条件：analysis-only 对正式 60 点汇编 `0 changed`；每个查询都有 proved、
disproved、unknown 三类夹具；CFG/Memory/Calls/Globals 失效测试全部通过。

#### C2：补齐低风险标量与内存优化

目标：优先对齐 ARM 中不改变算法复杂度、可局部证明且适合 RV64GC 的能力。

固定顺序：

1. signed power-of-two quotient/remainder 与 integer-facts simplify；
2. local/affine load forwarding、readonly local array forwarding；
3. constant aggregate propagation、local aggregate promotion；
4. same-value store、zero-store bundle、dead-write region slicing；
5. scalar/conditional memory recurrence promotion；
6. address/pointer recurrence 与精确 i32/i64 strength reduction。

每个 pass 必须使用统一 proof facts，经 clone、verifier 和 RV64 admission 后提交；
默认启用时同步更新语义台账、真触发正例、近似负例和门禁。若机器收益仅来自
未标定成本模型，则保持 research-only。

出口条件：逐 pass 正/负例通过；正式功能 O1 全过；changed-only 无确定性回退；
提交候选不得增加 spill，除非实测收益覆盖且 policy 中有可解释的通用阈值。

#### C3：补齐循环、仿射与 recurrence 能力

目标：对齐 ARM 的标量循环优化深度，不引入任何向量 IR 或 RVV 指令。

固定顺序：

1. canonical loop、rotation、exact partial unroll/peeling；
2. affine domain split、exact-window linearization；
3. dependence-proved interchange、fusion 和 conservative tiling；
4. pressure-aware unroll-and-jam；
5. linear/modular/bounded/conditional recurrence；
6. transpose/locality、moving-memory 和 in-place row recurrence。

合法性必须覆盖 zero/one trip、i32 wrap、side exit、multiple latch、live-out、alias、
call effect、termination 和 partial tile/remainder。盈利判断使用真实 RV64 MIR 的
branch/load/store、关键路径、压力、spill、frame 和代码尺寸，不采用 A53 阈值。

出口条件：每项至少一个真触发正例和一个 dependence/alias/live-out near-miss；
候选失败后原模块 hash 不变；正式 changed-only 性能不出现未解释回退。

#### C4：补齐 BOOM 标量后端能力

目标：把 ARM 的成熟机器规划思想改写为 BOOM/RV64GC 实现，提高标量代码质量。

任务：

1. 建立版本化 BOOM target profile；在获得官方微架构参数或实机计数器前，未知
   latency/throughput 保守建模，不从 Cortex-A53 常量推导。
2. 扩展 machine metrics：按热循环估算资源占用、load-use、mul/div、branch、call、
   address materialization、spill/reload 和 compressed-instruction 机会。
3. 改进 linear scan：同位置 Phi 定义、lifetime holes、call-clobber、跨迭代稳定
   spill home、callee-saved 收益与 rematerialization 失效必须正确。
4. 调度器显式维护数据、内存、call/ABI 和 fallthrough 依赖；用 BOOM 发射宽度和
   load-use 特性做通用评分，但所有不确定依赖均禁止跨越。
5. 改进 block layout、branch rotation、large-offset/large-frame lowering、medany
   地址物化和压缩指令友好形态；不得依赖链接布局或 QEMU 偶然行为。
6. 增加 GPR/FPR 同时耗尽、8 个以上参数、parallel-copy cycle、深调用、大 frame、
   大全局地址和 caller/callee-saved 的专项动态测试。

出口条件：machine verifier 和 assembler/linker 全过；专项 ABI/RA/frame 用例动态
正确；对 M7 changed cases 的 spill、frame、热路径指令或分支至少一项有可解释改善，
且 byte-identical control 不显示环境异常。

#### C5：复杂 region/状态能力仅在 research 中孵化

目标：评估 ARM 的高级标量能力，但不让证明不足的算法变换进入 strict。

候选包括 pairwise affine fusion、stencil producer/consumer、wavefront、selective
memoization、last-observable/overwritten iteration contraction。每项必须独立具备：

- 与 benchmark 名称、输入和固定算法常量无关的候选发现；
- alias/effect/termination/live-out/observer/overflow 完整证明；
- clone/verify/admit/rollback 证据；
- 正例、形状相近负例、正式 correctness 和 changed-only 性能报告。

缺任一项即保持 dormant 或 research-only；“ARM 已默认启用”不构成 RV64 strict
启用理由。

#### C6：能力对齐发布验收

只有 C0–C4 完成、C5 中拟进入 strict 的项目逐项闭环后，才执行一次完整里程碑
验收：

1. strict Release 构建、`git diff --check`、语义台账与 strict audit；
2. 正式功能目录 O0/O1 全量，报告实际 `passed/discovered`；
3. 正式性能目录 60 点 `-O1 --jobs 1` 正确性与完整动态比较；
4. 分别报告 `compile_ms`、`link_ms`、`run_ms`、`wall_s`、内部 timer、汇编尺寸、
   指令分类、spill/frame 和 control；
5. strict 注入 research enable 后汇编不变，research 未完整 opt-in 时与 strict
   汇编不变；
6. 输出 ARM/RV64 能力矩阵：`equivalent`、`RV64-adapted`、`research-only`、
   `deferred-RVV`、`not-applicable-AArch64`，不得用文件数量冒充能力完成度。

发布出口条件：功能无回归、0 个阻断合规 finding、M7 性能回退关闭、默认新增能力
均有证明与证据；单轮噪声不能表述为稳定收益，需要稳定性结论时采用预先约定的
多轮交错样本。

### 8.4 工作包依赖与并行边界

```text
C0 性能归因 ───────────────┐
                           ├─> C2 低风险标量 ─> C3 循环/仿射 ─┐
C1 分析闭环 ───────────────┘                                 ├─> C6 发布
C0 + 现有 MIR metrics ─────────> C4 BOOM 后端 ───────────────┘
C1 + C2 + C3 + C4 ────────────> C5 research 复杂变换
```

- C0 与 C1 可以并行推进，但 C2/C3 的默认变换必须等待所需 proof facts 完成。
- C4 可先处理纯后端 correctness 与观测指标；涉及 profitability policy 时必须消费
  C0 的真实 RV64 数据。
- C5 不在关键发布路径上，不能阻塞先完成安全的标量能力与 M7 回退修复。

### 8.5 固定验证输入与命令口径

- 功能套件：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道功能用例`；
- 性能套件：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道性能用例`；
- 性能编译固定使用公开入口 `compiler -S -O1 -o`；
- 动态性能固定使用 `tests/run_performance.py --opt --jobs 1`；
- runtime、交叉 GCC、QEMU 路径和版本随每份证据显式记录；
- ARM 套件、ARM runtime 和 AArch64/QEMU 时间不得替代 RISC-V 证据，也不得直接
  与 RV64 毫秒数横向排名。

### 8.6 RVV 冻结条件

本计划完成前，以下任一改动都视为越界：

- 将 target ISA 从 `rv64gc` 改为 `rv64gcv`；
- 将 `RV64TargetInfo::hasV` 改为 `true`；
- 新增或发射 `vsetvl*`、vector load/store/arithmetic 指令；
- 新增 vector register class、VLEN/ELEN 假设、RVV ABI 或自动向量化流水线；
- 把 ARM NEON/VPlan 变换包装成 RV64 默认优化。

未来若官方明确提供 RVV，必须另立独立计划，从硬件/OS/工具链能力探测、RVV 版本、
VLEN/ELEN、tail/mask policy、寄存器分配、context save、ABI、emitter 和实机验证重新
设计；不得把本轮标量能力对齐默认解释为已经为 RVV 做好实现承诺。
# 2026-08-14：RV64 调用实参 parallel-copy 通用修复

- 删除 `lowerCall` 对入口函数和调用 CFG 位置的差异化 lowering；普通函数与入口
  函数都按 ABI location、RegClass 和真实 stack slot 数量生成调用实参。
- `a0..a7/fa0..fa7` 实参通过 linear-scan 后的 parallel-copy/cycle resolver 搬运；
  超过寄存器容量的参数直接写最终调用方栈参数区，不再为全部参数建立暂存区。
- 60 例相邻 `sd → ld aN` 从 237 降为 0，静态机器指令 `19441 → 18607`，
  `.text` `61518 → 59760 bytes`。
- 新增普通寄存器实参正例和十整数参数近似负例；定向动态、140/140 功能、
  60/60 性能输出正确性、strict audit、determinism 和 planning contracts 均通过。
- 正向 `B → C` 与反向 `C → B` 各一轮串行动态计时中，候选 `total_run_ms` 分别
  `+4.3608%`、`+0.4409%`；内部计时和 byte-identical control 均随顺序反转，
  且 12/20 family 方向不一致。因此当前证据不宣称稳定收益，也不足以判定稳定
  回退；完整原始数据固化在
  `tests/riscv/evidence/20260814/call-argument-parallel-copy/`。
