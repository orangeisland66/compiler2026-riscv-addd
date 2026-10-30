# RV64 通用优化能力实施计划

日期：2026-08-13  
目标分支：`master-new`  
性能参照：`master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`  
合规依据：仓库根目录 `关于编译优化合理性及相关违规行为认定的说明.pdf`

## 1. 计划目标

本计划不以逐个恢复公开性能样例为实现主线，而是在当前 CoreIR、分析框架、候选事务和 RV64 MIR 后端上建设通用优化能力。公开样例只用于发现缺口、评估影响面和验证性能，不参与 pass 触发、合法性证明或成本参数选择。

最终目标：

1. 让通用 SSA、内存、循环、跨过程、递归和 RV64 机器优化形成稳定固定点；
2. 将已有但过窄的 matcher 改成由 use-def、CFG、alias/effect、MemorySSA、依赖、终止和成本事实驱动的通用计划；
3. 所有扩大 CFG、重排内存或改变复杂度的候选统一经过 `prove → clone → transform → verify → machine admission → commit`；
4. 默认 `-O1` 只接纳对所有满足前提的程序正确、且 RV64 机器产物有收益的候选；
5. 在恢复已知约 3.5 秒性能缺口的同时，让优化自然覆盖未公开程序，而不是增加 family 特化。

## 2. 现有能力与实施分类

当前仓库不是空白实现。计划按三类管理，避免重复造轮子。

### 2.1 已存在，需要补强

| 能力 | 现有落点 | 主要缺口 |
|---|---|---|
| SCCP/GVN/PRE/CSE/instcombine/DSE | `src/core_ir/passes/` | 固定点编排、trap/effect 合同、MemorySSA 协作、精确失效 |
| LICM/unswitch/循环规范化 | `loop_basic.cpp`、`loop_unswitch.cpp`、HIR 循环 pass | 多出口、zero-trip、运行时界、机器收益和代码增长控制 |
| MemorySSA-lite/alias/pointer range | `src/analysis/` | 动态范围、call mod/ref、分区精度、版本化查询 |
| 仿射 access/dependence/loop band | `src/analysis/`、`src/hir/` | 统一 legality API、多候选 schedule、收益比较 |
| SROA、局部 store-load forwarding | `src/rewrite/`、`src/hir/` | 逃逸、部分覆盖、动态 GEP、MemorySSA 统一 |
| inline/specialization | `inline_passes.cpp`、`function_specialization.cpp` | 调用频度符号估计、RA/frame/code-size admission |
| MIR schedule/RA/frame/peephole | `src/backend_mir/` | 动态循环权重、spill 决策、地址复用、目标 profile |
| planning/admission | `src/core_ir/planning/`、`rv64_admission.*` | 模块 clone 成本、区域映射、动态成本、统一 pass 接入 |

### 2.2 已有研究实现，需要通用化

- `row_locality`：改为一般仿射 schedule/局部性 candidate；
- `stage_fusion`、`producer_map`：改为一般 producer-consumer region fusion；
- `transpose_view_forwarding`、`demand_driven_copy_transform`：改为一般 view forwarding 和 observer-driven region slicing；
- `cross_call_array_summary`：改为一般函数 mod/ref/区域转换摘要；
- `recursive_accumulator_normalize`、`recursion_memoization`：改为一般递归 SCC 计划；
- `pure_call_last_value`、`loop_state_fixpoint_break`：改为一般不可观察迭代与完整状态不动点；
- `radix_sort_lowering`：拆成通用递归优化、分区/桶语义分析和可选算法 lowering，不能直接恢复旧骨架 matcher。

### 2.3 需要新增或抽取

- 函数/区域级 planning clone 和局部 commit；
- 统一 `LegalityFacts`、`ProfitabilityFacts` 与稳定拒绝原因；
- 分区 MemorySSA 和跨调用 memory version；
- 通用 observer closure、region liveness 与 demand slice；
- 符号动态工作成本：循环、递归、访存复用和调用；
- pass remark/统计输出，用于解释“发现、拒绝、机器拒绝、提交”；
- 面向合法 IR 形态的生成式差分测试。

## 3. 总体实施原则

### 3.1 通用性定义

一个优化只有满足以下条件才称为通用：

- 触发完全由 IR opcode、类型、常量表达式、SSA/use-def、CFG、循环、alias/effect、依赖、终止、ABI 和 RV64 成本事实决定；
- 普通函数名、变量名、文件路径、参数个数指纹、输入数据和运行环境改变时，语义相同的程序得到相同决策；
- 合法性不使用性能样例输入；runtime guard 只能验证 alias、范围、对齐、trip count 等程序语义条件，并有原路径 fallback；
- 未知事实统一拒绝，不能用“现有测试都通过”代替证明；
- 测试至少包含两个互不相关的正例、重命名/扰动变体和形状相近的 near-miss。

### 3.2 统一事务

复杂候选统一使用：

```text
discover read-only facts
  → build CandidatePlan
  → prove legality
  → clone function/region
  → apply one candidate
  → verify CoreIR + proof-specific checker
  → lower baseline/candidate through identical RV64 MIR pipeline
  → compare dynamic-aware machine cost
  → commit candidate or preserve baseline
```

局部恒等式可直接使用共享 rewrite 框架，但仍需位宽、trap 和 effect 合同。循环重排、跨区域消除、递归变换、内联/克隆和明显扩大代码的优化必须使用事务。

### 3.3 编排规则

- 规范化与分析先于高层候选；
- 高层候选成功后运行有限清理固定点；
- 清理固定点不得无限循环，按 pass change bit 和 IR fingerprint 停止；
- 会互相抵消的 pass 不在同一轮反复执行；
- analysis mutation 后按函数/模块 epoch 精确失效；
- 机器 admission 比较经过相同 cleanup、schedule、RA、frame 和 emitter 的两侧。

## 4. 工作包 G0：证据、remark 和影响面工具

目标：所有优化都能被解释和快速验证。

实施步骤：

1. 在 `CandidateEvaluation` 增加 `discovered/proved/transformed/verified/admitted/committed` 阶段状态，不用一个布尔值混合失败原因。
2. 统一 reason code 命名：`legality-*`、`proof-*`、`transform-*`、`verify-*`、`machine-*`、`profit-*`。
3. 新增按 pass 汇总：候选数、证明拒绝数、机器拒绝数、提交数、编译时间和 IR 增长。
4. dump 只输出 Value ID、opcode 和结构事实；普通名称只允许作为日志定位字段，不参与决策。
5. 扩展汇编比较，记录 normalized hash、指令分类、函数 frame、spill/reload、call、branch、load/store、mul/div/rem。
6. 日常性能遵守 `AGENTS.md`：60 份汇编影响面扫描，动态只串行运行 changed cases，加一个 byte-identical control，默认一轮 B→C。

修改位置：

- `src/core_ir/planning/candidate_plan.*`
- `src/core_ir/passes/pass_manager.*`
- `src/backend_mir/mir_backend.*`
- `tests/run_performance.py` 及汇编比较工具

退出标准：任一 candidate 都能说明在哪个阶段、因哪个通用事实接受或拒绝；开启 trace 不改变汇编。

## 5. 工作包 G1：分析与局部事务基础

目标：提高分析精度，降低复杂候选的编译时间。

### G1.1 函数/区域 clone

1. 从当前完整 Module clone 抽取 `FunctionPlanningClone`。
2. clone 当前函数及其引用的声明、常量和全局 metadata；跨函数候选显式声明闭包。
3. 建立 source↔clone 的 function/block/value/instruction 双向映射。
4. commit 只替换目标函数或区域，失败销毁 clone 并恢复 Value ID checkpoint。
5. 对区域 clone 验证入口、出口、live-in、live-out、Phi 和 memory version 边界。

### G1.2 分析失效

1. 将失效粒度细分为 Values、CFG、Memory、Calls、ModuleGlobals。
2. 每个 pass 静态声明读取和失效集合；debug 构建可重算分析比较摘要。
3. CFG mutation 后禁止复用旧 dominance/loop/liveness；memory mutation 后禁止复用旧 MemorySSA/effect。

### G1.3 MemorySSA 与范围

1. 从单一保守 memory state 扩展为按不相交 object/region 分区。
2. 支持常量 GEP、仿射动态 GEP 的区间；无法证明范围时回退 object 级 may-alias。
3. 为 call 使用版本化 function mod/ref summary；unknown call clobber 所有可能逃逸区域。
4. 提供 `clobberBefore`、`sameMemoryVersion`、`mustOverwrite`、`hasObserverBetween` 公共查询，禁止每个 pass 自建 walker。

退出标准：analysis-only 汇编 60/60 不变；clone 拒绝不改变原模块序列化 hash；每种失效均有 stale-cache 反例。

## 6. 工作包 G2：通用标量和内存优化固定点

目标：让已有 pass 更完整地消除高层变换留下的冗余，并直接改善普通代码。

### G2.1 标量固定点

推荐一轮：

```text
SCCP → simplify-cfg → instcombine → GVN/PRE → CVP → DCE
```

实施步骤：

1. SCCP 支持可执行边、Phi、整数比较和 select；未知/overdefined 正确合流。
2. instcombine 建立位宽明确的 canonical form，避免两个 pass 来回改写。
3. GVN 只处理无副作用表达式；load value numbering 消费 MemorySSA version。
4. PRE 不得把可能 trap 的 div/rem/load/call 提前到原本不执行的路径。
5. CVP 使用 dominance edge facts，不把一条分支的范围泄漏到其他路径。
6. DCE/ADCE 保留 call、store、volatile、可能 trap、终止和控制依赖。
7. 用 IR fingerprint 限制固定点，默认最多 3 轮；达到上限输出 remark 而非继续。

### G2.2 内存优化

1. must-alias store-to-load forwarding，检查宽度、offset、覆盖和 memory version。
2. same-version load CSE；未知 call、may-alias store 和路径合流阻断。
3. DSE 支持完整覆盖、同块覆盖和跨块 must-overwrite；部分覆盖先不删除。
4. store 合并仅在端序、宽度、对齐、alias 和可编码性均证明后生成候选。
5. SROA 支持不逃逸 local aggregate 的常量字段和有限索引集合；动态未知索引拒绝。
6. redundant memset/zero-init 消除必须证明随后全覆盖且中间无观察者。

计划接入：局部 forwarding/CSE 可直接运行；跨块 DSE、store 合并、SROA 扩展通过 candidate transaction 和机器 admission。

退出标准：两个无关标量正例、两个数组/结构正例；trap、may-alias、partial overwrite、escape、unknown call 反例均正确拒绝；功能 140/140。

## 7. 工作包 G3：循环规范化和通用循环优化

目标：把循环优化建立在统一 Loop IR 合同上，而不是每个 pass 匹配自己的形状。

### G3.1 Loop canonical form

1. 单独形成 preheader、latch 和必要的 dedicated exits。
2. 规范 header Phi、canonical IV、step、continue predicate、zero-trip edge。
3. 建立 LCSSA 类 live-out 描述，使所有出口值显式可验证。
4. 支持多出口但不强制合并语义不同的出口；无法规范化只拒绝后续变换。
5. loop band 统一表示嵌套关系、domain、access 和 side effects。

### G3.2 LICM、peeling、unswitch

1. LICM 处理纯算术、无 clobber load 和循环不变地址；可能 trap 指令需要 guaranteed-executed 证明。
2. store hoist/sink 需要唯一写、无循环内观察者和 exit coverage。
3. peeling 用于对齐、首迭代特殊状态和简化循环；次数来自语义/目标成本，不来自样例输入。
4. unswitch 只克隆 invariant condition；比较 candidate 的 code size、pressure、frame 和动态 branch 成本。
5. 所有 side exit、break/continue 和 exit Phi 都进入 proof checker。

### G3.3 展开、分布、融合

1. 展开枚举 `{1, 2, 4, 8}` 中目标允许的候选，而不是固定选择；因子上限由寄存器预算、body cost 和 code growth 给出。
2. 支持 runtime trip 的 main loop + remainder，保留 zero-trip。
3. loop distribution 根据 SCC dependence graph 拆分独立语句，改善向量外的标量 cache/pressure。
4. fusion 需要相同/可映射 domain、合法依赖方向、effect 顺序和所有 live-out。
5. fusion 与 distribution 都通过 clone/admission，禁止形成来回振荡。

### G3.4 仿射 schedule 和局部性

1. access map 表示 base object、系数、常量和元素宽度。
2. dependence 分析输出方向/距离；未知依赖阻断 permutation。
3. 枚举合法 interchange、strip-mine/tiling、loop order candidate。
4. locality 成本比较 stride、cache-line 利用、reuse distance、工作集和额外循环控制。
5. tile factor 来自 RV64 target profile、元素宽度和寄存器预算；不使用公开矩阵维度。

退出标准：一般一层循环、嵌套矩阵、stencil、reduction 和带 side exit 循环都有正/负例；已知 `many_mat_cal/matmul/conv2d` 仅作为里程碑验收。

## 8. 工作包 G4：跨过程通用优化

目标：让 call 成本、内存 effect 和参数事实可安全传播。

### G4.1 函数语义摘要

摘要包括：

- 参数/全局 read、write、capture、returned；
- pure/readonly/readnone、may-trap、must/may-terminate；
- 标量返回依赖和区域写转换；
- 调用图 SCC 与摘要版本。

unknown external call 保守；递归 SCC 用固定点求摘要，未收敛返回 unknown。

### G4.2 通用内联

1. legality 处理递归、ABI、alloca、多个 return、Phi 和调用副作用。
2. profitability 使用符号调用频度、call overhead、常量传播机会、code growth 和 machine pressure。
3. 热循环内小 helper、single-caller、wrapper 是成本特征，不是名称特征。
4. baseline/candidate 经过真实 RA/frame；出现 spill/frame/code growth 时拒绝。

### G4.3 参数与返回优化

1. dead argument/return elimination 只处理内部、地址不逃逸的函数。
2. readonly/noalias/constant argument facts 传播到 callee clone；不能改变外部 ABI。
3. 跨调用 load/store elimination 使用 mod/ref 和 memory version。
4. function specialization 只由常量/范围/alias facts驱动，并限制 clone 数与总 code growth。

退出标准：互递归、函数指针不可见/未知调用、多返回、栈参数和可变全局 near-miss 完整；普通函数改名不改变决策。

## 9. 工作包 G5：区域观察者和多阶段优化

目标：把旧 transpose、copy、producer/checksum matcher 转换为一般 region dataflow。

### G5.1 Region model

1. region 由控制域、读写区域、入口 memory version、出口 state 和 observers 组成。
2. observer 包括 load、call capture、返回/输出、别名访问、后续控制和 trap/终止。
3. `RegionStateEquivalence` 比较 baseline/candidate 的可观察标量和内存状态。
4. 不可证明的数组元素不能从 observer 集合中删除。

### G5.2 通用 region DSE/demand slicing

1. 从最终 observers 逆向计算 demand slice。
2. 仅删除不在 slice 中、且不会影响 control/trap/termination 的计算和写。
3. partial demand 只删除无需求部分；不能因部分消费者可转发就删除整个 producer。
4. 后续完整覆盖可消除先前写，但必须覆盖每条可达路径和完整字节范围。

### G5.3 View/producer-consumer forwarding

1. 从 affine relation 推导 source→view→consumer 的索引组合。
2. 精确证明范围、overflow、alias、dominance、clobber 和 zero-trip。
3. 可仅转发部分 load；所有 view observers 消失后才删除 materialization。
4. producer-consumer fusion 保持每个动态读对应的最后写和其他 effect 顺序。
5. runtime guard 只用于语义 alias/range 条件，失败执行原区域。

退出标准：copy、transpose、map/reduce、stencil 只是四类正例；额外 reader、escape、部分覆盖、反向依赖、未知 call 和不同边界均有拒绝例。

## 10. 工作包 G6：递归通用优化

目标：优化所有满足关系的递归，而不是单独识别背包或排序。

### G6.1 基础递归优化

1. 构造 call graph SCC，识别直接和互递归边。
2. 通用 tail-call/tail-recursion conversion，保持实参求值、effect、ABI 和返回。
3. 递归不变量外提：只读且每层相同的表达式提升到 wrapper。
4. accumulator normalization 从 SSA recurrence 推导，覆盖所有 base/step/return。
5. 对参数单调变化、终止 measure 和 overflow 建立显式证明；未知拒绝。

### G6.2 Selective memoization

1. 仅支持已证明 pure/deterministic 的直接递归作为第一版。
2. key 包含影响返回、终止和 trap 的全部参数/状态；hash 命中逐字段比较。
3. 只在原调用成功返回后填表；miss 完整执行原递归。
4. cache generation 隔离顶层调用，处理重入、wrap 和满表 fallback。
5. 用递归参数转移证明重复状态可能存在，并比较递归树上界与 unique-state 上界。
6. lookup/fill、内存、frame 和 code size 进入 RV64 admission。
7. pointer/global/互递归逐项扩展，未支持情况不得默认为安全。

### G6.3 不动点和不可观察递归/迭代

1. 完整状态相等且 transition 确定、无额外 effect 时才提前终止。
2. last-observable contraction 证明被跳过执行不影响输出、内存、trap 和终止。
3. 不以固定迭代次数或输入阈值激活。

退出标准：生成式小域穷举、随机差分、非终止/trap/hidden-state/hash-collision near-miss；`knapsack_naive/h-1/huffman` 只作为性能验收。

## 11. 工作包 G7：RV64 通用机器优化

目标：让中端收益在 RV64GC 上真正落地，并防止静态变小、动态变慢。

### G7.1 动态感知成本

1. 每个 block 记录 loop depth、符号执行权重和 call context。
2. 循环成本表示 setup + trip×body + remainder，不代入官方输入。
3. 递归成本记录 SCC、分支因子、深度 measure 和 unique-state 上界。
4. memory 成本记录 stride、cache-line 利用、reuse 和中间数组流量。
5. frame、spill/reload、callee-save、div/rem、call 和 branch 独立计费。
6. 目标 profile 明确版本；generic 未标定常数只做保守排序，不伪装为具体 CPU 周期。

### G7.2 RA/frame

1. 改进 live interval holes、loop weight、call-crossing 和 split point。
2. rematerialization 只用于可安全重建的常量/地址，不跨 clobber 使用旧值。
3. copy coalescing 不能破坏 interference 或寄存器类别。
4. spill slot 按不重叠生命周期复用，保持宽度、对齐和跨迭代 stack home。
5. shrink wrapping 覆盖全部 framed paths 和多 return。

### G7.3 schedule/layout/peephole

1. 调度 DAG 包含寄存器、memory、call、terminator 和 ABI 依赖。
2. loop-carried latency、load-use、mul/div 和 spill 纳入优先级。
3. block layout 使用循环/CFG 结构，分支反转同步 CFG/fallthrough。
4. 地址 materialization 复用和 base+offset folding 遵守 signed 12-bit 编码范围。
5. peephole 仅做局部机器等价式，保持 W/XLEN、符号扩展和 flags-free RV64 语义。

退出标准：机器 verifier 全过；frame/spill/pressure/code size 无新增确定性回退；`03_sort/01_mm` 的汇编问题作为验收，不写入规则。

## 12. Pipeline 落地顺序

建议默认 `-O1` 最终编排：

```text
lower to CoreIR
  → verify
  → mem2reg + loop canonicalization
  → scalar/memory cleanup fixed point
  → function summary + conservative inline/specialization
  → cleanup fixed point
  → loop candidate planning
  → region/observer candidate planning
  → recursive candidate planning
  → late scalar/memory cleanup
  → verify
  → RV64 MIR lowering
  → machine cleanup + pre-RA schedule
  → RA/spill/frame planning
  → branch/layout + post-RA schedule + peephole
  → machine verify + emit
```

接入顺序按依赖而非短期样例收益：

1. G0 remark/证据；
2. G1 clone、失效、MemorySSA；
3. G2 标量/内存固定点；
4. G7 成本、RA 和 admission 基础；
5. G3 循环；
6. G4 跨过程；
7. G5 区域观察者；
8. G6 递归和复杂度优化；
9. 全流水线次序与固定点联合调优。

## 13. 分阶段交付与退出门槛

| 里程碑 | 交付内容 | 必须验证 | 性能条件 |
|---|---|---|---|
| M0 | G0 + 固定 baseline/remark | 静态门禁、trace 不变性 | 不做全量动态 |
| M1 | G1 分析与局部 clone | analysis/clone 单测，功能 140/140 | 60/60 汇编应不变 |
| M2 | G2 标量/内存固定点 | trap/alias/effect 反例，功能 140/140 | changed-only B→C |
| M3 | G7 动态成本、RA/frame | MIR verifier、ABI、frame、determinism | 完整 60 点里程碑一次 |
| M4 | G3 通用循环 | loop/dep/remainder/side-exit 证据 | changed-only；目标家族不得回退 |
| M5 | G4 跨过程 + G5 区域 | mod/ref/observer/escape 证据 | 完整 60 点里程碑一次 |
| M6 | G6 递归基础 | termination/overflow/所有出口 | changed-only B→C |
| M7 | selective memoization strict 候选 | 完整 key、碰撞、重入、非终止证明 | 完整 60 点 + changed cases 13 对 |
| M8 | 最大恢复发布候选 | 全部证明台账和 PDF 扫描 | 完整 60 点 + 稳定统计 |

## 14. 测试策略

### 14.1 每个优化的最小证据

- 两个不同算法/代码类别的正例；
- 全量普通符号重命名变体；
- 参数顺序、无关块、无关指令扰动变体；
- alias/effect/live-out/overflow/zero-trip/trap/termination near-miss；
- IR verifier 与 proof-specific checker；
- 小域穷举或生成式差分作为补充；
- baseline/candidate 机器 metrics 和稳定 reason code。

### 14.2 正确性

1. 局部修改先跑定向正负例。
2. 改 CFG、MemorySSA、effect、planning、MIR、RA、ABI 或 emitter 后跑 O0/O1 功能 140/140。
3. 里程碑跑性能输出正确性 60/60。
4. assembler/linker/verifier error、WRONG、timeout 或 crash 均阻断性能结论。

### 14.3 性能

严格执行当前 `AGENTS.md`：

1. 记录分支、HEAD、基线完整提交、编译器 hash、官方 RISC-V 用例路径和工具链；
2. 优化编译使用公开 `-S -O1 -o`；
3. 先扫描 60 份 normalized assembly；
4. 日常只串行运行 changed cases，加 byte-identical control，默认一轮 B→C、`--jobs 1`；
5. `run_ms`、`compile_ms`、`link_ms`、`wall_s` 和内部 Timer 分开报告；
6. 单轮不称 median；只有 M7/M8 或明确需要稳定结论时对 changed cases 做预定 13 对交错统计；
7. 汇编相同的运行差异作为噪声证据，不作为优化收益。

## 15. 合规检查

每个默认 pass 合入前检查：

- 源码和配置不包含公开 case/family/path 触发；
- 不按普通函数/变量名、参数个数指纹或输入值选择优化；
- 不读取 `.in`，不编译期执行待编译程序，不硬编码结果；
- strict 汇编不受非诊断环境变量、宿主 CPU 或 QEMU 环境影响；
- 不利用越界、未初始化、错误 alias、宿主 C++ overflow 或其他 UB；
- runtime guard 只检查通用语义条件，并保留正确 fallback；
- 对所有满足 matcher 前提的程序都有静态证明，而不只是测试覆盖；
- 名称重写和环境不变性测试通过。

## 16. 禁止的实施方式

- 按回退家族新增黑名单、白名单或专用 gate；
- 直接恢复旧 pass 后只补公开样例测试；
- 为了减少静态指令数接受动态递归、访存或 spill 增长；
- 将 ARM A53 指令延迟、NEON、AAPCS64 或 ARM 用例规模复制到 RV64；
- 同时启用多个高风险 pass 后再猜测收益来源；
- 用 research 环境开关改变 strict 提交二进制；
- 未运行测试却写“通过”，或把部分/中止运行写成完整结果；
- 将计划中的性能目标、家族时间或输入规模写入编译器阈值。

## 17. 完成定义

计划完成不是“已知 10 个家族恢复”这一单一条件，而是：

1. 通用 SSA/MemorySSA/循环/跨过程/区域/递归/MIR 能力均有独立合同和证据；
2. 复杂变换全部通过局部事务、proof checker 和真实 RV64 admission；
3. 默认 pipeline 固定点可终止、分析失效正确、跨进程汇编确定；
4. O0/O1 功能和性能输出全部正确；
5. PDF 禁止特征扫描、重命名不变性和 strict 环境不变性通过；
6. M8 对固定基线无新增稳定回退，并尽可能接近或优于约 6.7 秒历史基线；
7. 优化能在非公开、改名和结构扰动程序上触发，证明收益来自共享能力而非用例识别。

本计划与 `docs/compliance/pdf-only-performance-recovery-plan-20260813.md` 的关系是：后者保存回退汇编、消融归因和恢复上限；本文是实际开发主计划，以通用能力、源码落点、依赖次序和验收门槛组织工作。
