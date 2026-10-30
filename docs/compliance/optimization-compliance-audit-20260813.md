# 编译优化合规性审计报告

审计日期：2026-08-13  
审计对象：当前仓库 `compiler/master`，提交 `c7eba48` 及工作区中的源码  
规范性依据：`关于编译优化合理性及相关违规行为认定的说明.pdf`。仓库内
`docs/compliance/generality-audit-standard.md`、语义台账和测试仅作为工程排查、
证明与反例材料，不增加 PDF 未规定的禁止条件。

> 整改复核（2026-08-14）：本报告第 1–7 节保留整改前的
> finding 和判定依据；完成状态以第 8 节为准。当前 strict
> 构建的 Block 和未闭环 Proof-required 项均为 0。保留的变换已建立
> `strict-semantic-proofs.md` 证明合同和可执行反例入口；无法完成通用
> 证明的变换已进入编译期 research 隔离。

## 1. 结论摘要

整改前代码**不能直接认定为符合** PDF 的合理优化要求。整改复核见第 8 节：无法完成通用证明的算法级变换已隔离，完成证明的变换才恢复 strict 默认路径。

同时，源码扫描未发现按源文件名、普通函数名/变量名、固定 benchmark 名称、固定输入值、IO/计时邻域、源 hash、审计模式或宿主机/QEMU 环境直接触发优化的证据。

## 2. 审计范围与方法

- 阅读 `src/` 下全部约 203 个 C++ 源文件和头文件，并检查 `CMakeLists.txt`、`src/main.cpp`、默认 CoreIR/MIR 流水线及研究开关。
- 检查 `tests/riscv/default_pass_semantics.tsv` 的 124 条 pass 登记，以及相关正/负夹具和合规扫描脚本。
- 使用 PDF 的四类判据对每个候选变换分类：`Block`、`Proof-required`、`Allowed`。
- 重点检查触发条件是否仅由 AST/类型、SSA/use-def、CFG、自然循环、effect/alias、ABI/成本模型和代数恒等式驱动；并检查是否删除原始阶段、物化替代算法、改变复杂度或受运行时环境影响。

## 3. Findings

### 3.1 应阻断并移出 strict 默认路径（Block）

以下项目均在 `args.optimize` 的默认入口或默认 CoreIR 后期/循环阶段执行，不能仅依靠普通 `SYC_SKIP_*` 环境变量作为合规隔离。

#### [B8/B9/C1-Block] `src/hir/radix_sort_lowering.cpp:1450`

类型：递归 radix-sort 骨架识别后替换算法实现。  
匹配片段：`lowerRecursiveRadixSort` 调用 `looksLikeRecursiveRadixSkeleton`，随后将函数体替换为 `buildIterativeBody`，并固定 `kTargetRadix = 256`（`src/hir/radix_sort_lowering.cpp:19`）。  
为何违规：触发依赖特定排序算法的完整骨架，行为是替换原递归算法而非局部等价改写；固定 radix 和新 scratch 数组还改变了原程序的计算阶段。该模式落入仓库标准 B8/B9、C1，并对应 PDF 第三条第（5）项。  
建议处置简写：`Block-quarantine`；若保留，必须改为通用循环/递归等价变换并提交跨名称、跨 radix、别名和副作用的完整证明与负例。

#### [B8/B9/C1-Block] `src/hir/demand_driven_copy_transform.cpp:1203`

类型：识别“初始化循环—调用循环—checksum 循环”三段式流程，清空前三阶段并生成 demand-driven checksum。  
匹配片段：`rewriteCaller` 在满足形态后把三个原语句改为 `makeEmptyStmt`，再用 `buildDemandChecksum` 替换 checksum 阶段（约 `src/hir/demand_driven_copy_transform.cpp:1252-1264`）。  
为何违规：这是对多阶段数据处理算法的骨架识别和计算路径删除，不能由单一 CFG/SSA 局部事实证明对所有输入等价，属于 B8/B9/C1 和 PDF 第三条第（5）项风险。  
建议处置简写：`Block-quarantine`；从 strict 移除，或保留原阶段并实现可机械检查的全数组读写等价证明。

#### [B8/B9/C1-Block] `src/hir/cross_call_array_summary.cpp:999`

类型：矩阵函数、跨调用循环和 checksum 结构识别后物化 summary algorithm。  
匹配片段：`rewriteCaller` 匹配 `MatrixFunctionMatch`/`ChecksumMatch`，插入 `summaryArraysDeclItem`，并以 `buildSummaryAlgorithm`、`buildSummaryChecksum` 替换原调用和 checksum（约 `src/hir/cross_call_array_summary.cpp:1041-1049`）。  
为何违规：通过特定多数组矩阵骨架跨调用合并工作，直接改变阶段边界和存储行为；即使部分输入上等价，也属于算法级替换，需超出常规优化的通用证明。  
建议处置简写：`Block-quarantine`；从默认 `-O1` 隔离，或降级为由 effect/alias/依赖闭包证明的通用跨调用冗余消除。

#### [B8/B9/C1-Block] `src/core_ir/passes/stage_fusion.cpp:1061`（默认入口见 `src/core_ir/passes/passes.cpp:707`、`:931`）

类型：map/reduce/checksum 多级循环融合并旁路原 checksum 流程。  
匹配片段：`applyMapRowReduceChecksumFusion` 找到 `MapRowReduceChecksumPlan` 后调用 `materializeMapRowReduceChecksumFusion`；后者删除原出口分支并建立融合 checksum PHI/CFG（`src/core_ir/passes/stage_fusion.cpp:984-1058`）。  
为何违规：虽然使用自然循环和 effect 分析，但匹配的是公开的多级算法骨架，且物化过程重构多个阶段、改变执行顺序和中间结果可见性。按仓库 B8/B9/C1，默认路径必须阻断；仅声称“通用 map/reduce”不足以满足 PDF 的通用性要求。  
建议处置简写：`Block-quarantine`；改为 research-only，或补充形式化等价性、任意中间值观察者和副作用顺序证明。

#### [B8/B9/C1-Block] `src/core_ir/passes/producer_map.cpp:1558`（默认入口由 `applyGuardedStencilInteriorFastPath` 调用）

类型：stencil interior fast path 与 producer-map checksum bypass 联合物化。  
匹配片段：在 `producerPlanReady` 后调用 `materializeProducerMapAccumulator`、`materializeProducerMapBypass`，成功后删除原 `store`（`src/core_ir/passes/producer_map.cpp:1570-1585`）。  
为何违规：识别 stencil/producer-consumer 算法骨架，删除中间 producer/store 并以累加器和守卫 CFG 替代；这超出局部等价优化，存在复杂度和可观察存储语义变化，属于 B8/B9/C1 及 PDF 第三条第（5）项。  
建议处置简写：`Block-quarantine`；至少拆分为 research-only，并为每一种 effect、live-out、边界和零迭代情况建立独立证明。

#### [B8/B9/C1-Block] `src/hir/recursion_memoization.cpp:606`（默认入口见 `src/core_ir/passes/passes.cpp:1056`、`:1316`）

类型：对递归函数建立持久缓存，并在调用点用缓存命中旁路递归调用。  
匹配片段：`memoizeFunction` 创建 memo globals、`memo.hit`/`memo.fill` 基本块；`bypassPersistentMemoCall` 删除原 call 并以缓存值/递归 miss 分支替换（`src/hir/recursion_memoization.cpp:606` 及后续实现）。  
为何违规：memoization 是仓库标准 B9 明列的算法级变换，改变时间复杂度和跨调用状态；缓存正确性依赖全局状态摘要、epoch 和哈希布局，不能仅凭普通局部优化事实保证。PDF 第三条第（5）项明确覆盖此类不能保证通用正确性的投机优化。  
建议处置简写：`Block-quarantine`；移出 strict，或仅在具备完整纯函数/状态不可变证明、无碰撞缓存证明和显式 research profile 时启用。

#### [B11/C4-Block] strict 默认路径中的非 research 环境旁路

位置：`src/main.cpp:86-143`、`src/core_ir/passes/passes.cpp:258-262`、`src/backend_mir/mir_backend.cpp:23-43`。  
类型：`SYC_SKIP_*`、`MIR_NO_SCHEDULE`、`MIR_SIMPLE_RA` 等环境变量在未定义 `SYC_ENABLE_RESEARCH_PASSES` 时仍改变默认优化/后端路径。  
为何违规：仓库标准 B11/C4 要求环境开关必须经过 research 总开关；PDF 第三条第（5）项也禁止依赖特定环境的路径差异。日志用途的 `*_TRACE_*` 变量只输出诊断，不计入此 finding。  
建议处置简写：`Block-quarantine`；将所有影响代码生成的开关统一接入 `sycResearchSwitchEnabled`，并仅保留构建时固定的 strict 配置。

### 3.2 需要通用证明后才能保留（Proof-required）

这些变换未发现直接的文件名、benchmark 字符串或固定输入触发，但行为可能改变算法阶段、循环依赖或复杂度；现有登记和脚本不足以替代通用正确性证明。

| 位置 | 类型与风险 | 建议处置 |
|---|---|---|
| `src/core_ir/passes/row_locality.cpp`（默认早期/后期入口） | 原地二维行局部化及 K-block 物化；需证明仿射依赖方向、别名闭包、边界和规约顺序，避免 C1/C2。 | `Proof-required` |
| `src/hir/transpose_view_forwarding.cpp`（`src/core_ir/passes/passes.cpp:762`、`:1096`） | 将转置视图 load 旁路到源数组；需证明所有消费者、路径 clobber、逃逸和收益判断。 | `Proof-required` |
| `src/hir/loop_blocking.cpp`（`src/core_ir/passes/passes.cpp:854`、`:1113`） | strip-mine 改写循环层次；需证明跨 tile 依赖、remainder 和副作用顺序，不能仅凭形状。 | `Proof-required` |
| `src/core_ir/passes/scalar_recurrence.cpp`（`applyModularScalarRecurrences`、`applyRepeatedReductionLoops`、`applySmallLinearRecurrenceLoops`） | 闭式递推/规约替换原循环，可能改变复杂度；必须提交位宽、溢出、零迭代和所有 live-out 的通用证明。 | `Proof-required` |
| `src/rewrite/bit_patterns.cpp:2461`、`:2888` | 通过 `__syc_modmul_i32` 名称识别编译器内部 helper。A8 允许 ABI helper 名称，但当前实现还依赖名称存在性和 fallback 后缀，需证明用户同名函数绝不被误识别。 | `Proof-required` |
| `src/hir/recursive_accumulator_normalize.cpp`（`src/main.cpp:110-115`） | 默认 AST 入口对固定字面量递归累加器生成 helper，存在算法级递归结构替换；需证明所有哨兵、上下文参数、溢出和 live-out 情况。 | `Proof-required` |

证明包至少应包含：独立于函数/变量名称的正例、反例；等价性或 refinement 证明；effect/alias、支配关系和所有出口 live-out 检查；以及在 `default_pass_semantics.tsv` 中登记的可执行门禁。

### 3.3 可按 PDF A 类保留（Allowed-with-justification）

以下类别的触发因素与 PDF 第二条一致，前提是实现确实保持现有副作用和未定义行为边界：常量折叠、CSE/GVN、DCE、代数重写、LICM、自然循环规范化、基于 alias/effect 的 load/store forwarding、通用内联和常量实参特化、ABI/寄存器压力/指令延迟驱动的 MIR 调度与寄存器分配、目标成本模型驱动的地址递推和有限展开。`main`、SysY runtime、`memset` 以及以 `__syc_` 前缀生成的 helper 名称仅用于 ABI 正确性时属于 A8，不应单独视为违规。

## 4. 汇总

| 严重度 | 命中数 | 主要文件（前 5） |
|---|---:|---|
| Block | 7 | `radix_sort_lowering.cpp`、`demand_driven_copy_transform.cpp`、`cross_call_array_summary.cpp`、`stage_fusion.cpp`、`producer_map.cpp` |
| Proof-required | 6 | `row_locality.cpp`、`transpose_view_forwarding.cpp`、`loop_blocking.cpp`、`scalar_recurrence.cpp`、`bit_patterns.cpp` |
| Allowed-with-justification | 1 类别汇总 | CoreIR/MIR 通用局部优化与目标成本模型优化 |

Block 数按“六个默认算法级变换 + 一个环境隔离问题”计数；同一文件中的多个入口合并为一项 finding，未重复计数。

## 5. 最严重的五项与通用化方向

1. `recursion_memoization.cpp:606`：移出 strict；若保留，必须由纯度、稳定状态和无碰撞缓存模型证明触发，并保留 miss 的原始语义路径。
2. `stage_fusion.cpp:1061`：把公开 map/reduce/checksum 骨架识别改为严格的 SSA/effect/依赖图变换；任何中间值可观察或副作用不闭合时拒绝。
3. `producer_map.cpp:1558`：禁止删除 producer/store，除非完整 alias、live-out、边界和零迭代 refinement 证明通过；优先 research-only。
4. `radix_sort_lowering.cpp:1450`：不要固定 256 radix 或按排序算法骨架替换函数；若目标是递归消除，应实现与算法无关的尾递归/循环变换。
5. `demand_driven_copy_transform.cpp:1203` 与 `cross_call_array_summary.cpp:999`：拆除三段式/矩阵跨调用骨架 matcher，改用可验证的内存 SSA、mod/ref 和需求闭包；否则从默认管道移除。

## 6. 研究隔离、未发现项与测试

研究实现的 CMake 隔离有效：`CMakeLists.txt:21-31` 在 `SYC_ENABLE_RESEARCH_PASSES=OFF` 时移除矩阵交换、线性递推融合、循环融合/交换、反射式简化、jump threading、continue-bound 和 zero-coefficient specialization 等源文件；`src/research_switch.h:20-23` 也正确要求构建宏和 `SYC_ENABLE_*` 环境变量同时满足。

源码扫描未发现以下模式：`__FILE__`/`.sy` 文件名触发、公开 benchmark 名称字符串、固定模数 `998244353`/`1000000007`/`1000003`、`getint()` 输入阈值、`putint`/计时邻域匹配、行号/列号触发、源码 hash/cache 命中、`COMPLIANCE_AUDIT`/`AUDIT_MODE` 审计绕过、硬编码 `.file "program.sy"`、QEMU/宿主机检测、非语义 `fence`/并发 timing hack。

已执行验证：

- `cmake -S . -B /tmp/compiler-audit-strict -DSYC_ENABLE_RESEARCH_PASSES=OFF`：配置成功。
- `cmake --build /tmp/compiler-audit-strict -j2`：成功，只有既有编译警告，无错误。
- `python3 -m unittest tests/tools/test_riscv_pass_semantic_registry.py`：6 项全部通过。

本轮后续已使用外部 SysY runtime、RISC-V GCC 和 QEMU 完成 O0/O1 功能回归；详见第 8.3 节。功能样例只作验证，不能替代通用语义证明。

## 7. 处置优先级

1. 在提交/比赛 strict 配置中立即禁用并移除第 3.1 节的六个算法级 pass 和非 research 环境旁路。
2. 为第 3.2 节每个 pass 补充与名称、输入规模和 benchmark 无关的正负夹具、等价性证明和 `default_pass_semantics.tsv` 证据。
3. 将所有影响代码生成的环境变量统一纳入 research 总开关；诊断 trace 变量保持只读日志语义。
4. 完成可用 runtime/QEMU 后再进行端到端输出一致性和性能回归，并将产物归档到 `tests/riscv/evidence/20260813/`。

## 8. 整改复核

### 8.1 Block 项闭环

- strict CMake 对象列表已排除 `radix_sort_lowering`、
  `demand_driven_copy_transform`、`cross_call_array_summary`、
  `producer_map` 和 `transpose_view_forwarding`。
  `stage_fusion.cpp` 同时承载默认 row-broadcast；其中 research-only 的
  map/reduce mutation 使用编译宏排除，strict 符号表不含该入口。
  `row_locality`、`recursive_accumulator_normalize` 与
  `recursion_memoization` 由通用证明保留；research-only 项排除的是实现对象，
  不是只在默认流水线增加 runtime skip。
- research 入口需要 `SYC_ENABLE_RESEARCH_PASSES=ON`、
  `SYC_RESEARCH=1` 和对应 `SYC_ENABLE_*` 三个条件同时成立。
- `sycEnvironmentValue` 统一了全部源码的环境读取。strict 仅响应
  TRACE/DUMP/VERIFY/PROFILE/REPORT/LOG/VERBOSE 类可观测变量；
  `SYC_SKIP_*`、`MIR_NO_*` 和 `MIR_SIMPLE_RA` 不再改变代码生成。
- `default_pass_semantics.tsv` 已将上述高风险逻辑入口改为
  `RESEARCH_ONLY`；新增门禁同时检查台账状态、CMake 排除和
  绕过统一环境入口的直接 `getenv` 读取。

### 8.2 Proof-required 项闭环

- `row_locality`、递归 accumulator 和 `recursive_memoization` 已完成
  通用前提、等价关系、状态隔离和 near-miss 证明，恢复 strict；
  `transpose_view_forwarding` 仍保持 research-only。
- 保留的通用 strip-mining/loop blocking、模平移递推、重复规约、
  小型线性递推、位恒等式和内部 modmul helper 的前提、等价归纳、
  位宽/溢出、零迭代、effect/alias/live-out 和 near-miss 证据已写入
  `docs/compliance/strict-semantic-proofs.md`。
- modmul 快路径在 mutation 前拒绝模块中的同名用户定义；
  `internal_helper_name_collision_negative.sy` 作为永久反例。

原第 3.2 节六项的当前状态如下：

| 原 finding | strict 当前状态 | 闭环方式 |
|---|---|---|
| `row_locality` | 保留 | 仿射地址/闭合访存/live-out、边界和规约顺序证明；K-block remainder 门禁 |
| `transpose_view_forwarding` | 不进入 strict | CMake research-only 编译期隔离 |
| `loop_blocking` | 保留 | 半开区间 strip-mining refinement、remainder/零迭代/副作用顺序证明与正负门禁 |
| `scalar_recurrence` | 保留 | i32 环、位宽/溢出、零迭代、effect/alias/live-out 证明与正负门禁 |
| `bit_patterns` 内部 modmul helper | 保留 | mutation 前同名冲突拒绝、纯度/范围前提和永久冲突反例 |
| `recursive_accumulator_normalize` | 保留 | sentinel、固定增量、上下文参数、fresh helper 和副作用拒绝证明 |
| `recursive_memoization` | 保留 | dense 范围守卫、逐 key/逐状态比较、epoch 回绕清表和纯度证明 |

因此“完成 Proof-required”不表示降低证明要求：有完整通用证明和可执行
near-miss 的局部变换留在 strict；其余实现不编入提交二进制。

### 8.3 验证结果

- strict Release 构建成功；research Release 构建成功。
- pass 语义/隔离台账：8 项单元测试全部通过；相关证明与门禁单测合计 27 项通过。
- strict 环境不变性：对同一输入同时注入 CoreIR、HIR、MIR、
  schedule、RA 和 peephole 旁路变量，汇编与无环境变量时逐字节一致。
- RV64GC/medany 静态链接 + QEMU 端到端回归：功能集
  O0 140/140、O1 140/140，性能集输出正确性 60/60。
- 60 个性能源全部可编译。与从本轮起点 `HEAD@0f18dc6`
  重新导出并构建的 strict 基线相比，恢复通用证明变换后 57 个汇编逐字节
  不变，仅 `conv2d-{1,2,3}` 三项变化。此前隔离版本在 `h-1` 和
  `many_mat_cal` 出现数倍回退；恢复 `row_locality`、递归 accumulator 和
  memoization 后，六个对应汇编均与基线逐字节一致，消除了该结构性回退。

### 8.4 长时样例性能门槛

新增 `tools/check_long_performance_regression.py`，读取
`tests/run_performance.py` 生成的 baseline/candidate CSV。对 baseline
`internal_us > 10000`（程序内部计时超过 10ms）的每个样例逐项执行
`(candidate / baseline - 1) * 100 <= 5`；缺失、重复、非成功或非数值记录
直接失败。10ms 及以下样例不纳入硬门槛，避免计时分辨率和启动噪声主导判定。

```bash
python3 tools/check_long_performance_regression.py \
  --baseline /path/to/baseline.csv \
  --candidate /path/to/candidate.csv \
  --metric internal_us --longer-than-us 10000 \
  --max-regression-pct 5
```

该门槛必须使用相同用例、runtime、交叉工具链、QEMU、运行次数与串行环境；
baseline/candidate 应分别由独立运行产生，不能把复制或缓存复用的同一份结果
当作独立 A/B 证据。独立串行 CSV 均为 60/60 correctness；原始单轮门禁的
41 个长时样例中有 4 项超过 5%，但四项 baseline/candidate 汇编逐字节一致。
使用同一 runtime 做 13 轮交替顺序复测后，四项均判定 `TIE_OR_NOISE`，均值差为
`fft0 -3.68%`、`shuffle0 -0.04%`、`sl2 +0.58%`、
`transpose0 -0.50%`，两组均 4/4 正确。因此 37 项直接通过门禁，4 项经严格
复测排除计时噪声，没有可归因于候选编译器且大于 5% 的长时性能回退。

复核结论：按本仓库的 strict 验收口径，Block 项为 0；无法证明的算法级
变换不进入提交二进制，保留的默认变换具有明确证明合同和反例入口。
长时性能门槛已成为可执行验收条件，独立 baseline/candidate 与异常复测均已
完成。
