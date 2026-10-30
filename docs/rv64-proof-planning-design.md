# RV64 证明、事务规划与机器准入设计

日期：2026-08-13  
目标：落实 `plan.md` 的 A1–A3、WP0–WP3 基础设施，并为后续逐 pass 迁移提供统一合同。

## 1. 已落地的数据流

```text
CoreIR
  -> AnalysisManager(epoch)
       -> pointer range / alias
       -> i32 integer facts / loop trip recipe
       -> loop access / RAW-WAR-WAW dependence
       -> function effects / MemorySSA-lite
       -> semantic/termination/trap summary
       -> ordered region effects / write coverage
       -> statement dependence / state equivalence / observers
  -> CandidateTransaction
       -> legality == proved
       -> isolated full-module clone
       -> transform clone
       -> CFG/SSA/PHI/type/dominance verifier
       -> real RV64 MIR + linear scan + schedule/layout/frame
       -> machine verifier + metrics + admission
       -> commit by module replacement, or destroy clone
```

`Unknown` 不是弱证明，而是拒绝授权变换。候选必须明确取得 `Proved`，否则原模块
保持不变。机器准入也 fail-closed：baseline 无效、candidate verifier 失败、spill、
frame、pressure、代码增长或加权成本没有改善，均返回稳定 reason code。

## 2. 论文和规范如何影响实现

### Translation validation 与 refinement

- Pnueli 等提出的 translation validation 路线强调验证每次实际编译结果，而不是
  假设优化器实现整体正确。本实现对应地把候选放入隔离 clone，并在提交前验证
  该次候选；当前结构 verifier 不是完整语义求解器，因此 legality 仍必须由具体
  pass 给出，不得把“结构合法”误称为“语义等价”。参考
  [Validation of Optimizing Compilers](https://cs.nyu.edu/home/people/in_memoriam/pnueli/ZPL01.pdf)。
- Alive2 使用 refinement 和有界 translation validation，并以避免 false alarm
  为目标。这里采用相同的保守方向：资源或事实不足返回 `Unknown`/reject，不把
  timeout、异常或 verifier crash 当成接受。参考
  [Alive2: Bounded Translation Validation for LLVM](https://web.ist.utl.pt/nuno.lopes/pubs.php?id=alive2-pldi21)
  及其[内存模型工作](https://web.ist.utl.pt/nuno.lopes/pubs.php?id=alive2-mem-cav21)。
- CompCert 的语义保持陈述允许编译器拒绝生成代码，但一旦输出代码，就必须保持
  允许的可观察行为。当前 reason-code/fail-closed 协议采用这一工程边界；本项目
  并不宣称已获得 CompCert 式机器证明。参考
  [CompCert C manual: Semantic preservation](https://compcert.org/man/manual001.html)。

### MemorySSA、alias 与依赖

- LLVM MemorySSA 使用 `MemoryDef`、`MemoryUse` 和 `MemoryPhi` 形成内存 def-use
  图；LLVM 当前采用单内存分区。`MemorySSALite` 同样使用单分区，未知调用是 Def，
  readonly 摘要调用才可降为 Use。参考
  [LLVM MemorySSA documentation](https://www.llvm.org/docs/MemorySSA.html)。
- 当前 alias 只证明同一 SSA 指针 MustAlias、不同 global/local alloca NoAlias，
  以及同一对象内常量范围不相交；参数和动态 GEP 均保持 MayAlias。这个范围有意
  小于完整 Andersen/Steensgaard 或 MemorySSA walker，避免错误的 NoAlias。
- Pluto 的 polyhedral 路线要求 affine domain、access 和 dependence 后才能做
  schedule/tiling。本实现目前只建立 access/dependence inventory；尚未证明的
  permutation/tile 不得进入 strict mutation。参考
  [PLUTO publications and implementation](https://pluto-compiler.sourceforge.net/)。

### RV64 目标和成本

- target profile 显式声明 RV64 `I/M/A/F/D/C`，默认 `V=false`。`G` 的正式含义
  是 IMAFD 加标准系统扩展，C 可另行组合成 RV64GC；因此不从宿主机能力推断目标，
  也不把预留 CoreIR vector opcode 当作 RVV。参考
  [RISC-V RV32/64G listings](https://docs.riscv.org/reference/isa/unpriv/rv-32-64g.html)
  和 [RV64I specification](https://docs.riscv.org/reference/isa/unpriv/rv64.html)。
- `rv64_cost_model` 的延迟是保守初始值，不代表任一具体微架构。真实准入首先使用
  同一正式 pipeline 的 frame/spill/pressure/instruction facts；未来标定必须绑定
  明确 CPU profile。LLVM MCA 也明确指出预测质量受 scheduling model 质量限制，
  这正是未知目标参数必须保守处理的原因。参考
  [llvm-mca documentation](https://llvm.org/docs/CommandGuide/llvm-mca.html)。
- MLGO 显示成本决策可以由学习模型替代启发式，但需要语料、特征提取与持续评估。
  当前只有 60 个公开性能点，不足以训练可泛化模型，因此保留可解释确定性策略和
  结构化 metrics，不在 strict 默认路径引入 ML。参考
  [MLGO paper](https://research.google/pubs/mlgo-a-machine-learning-guided-compiler-optimizations-framework/)。

## 3. 关键接口与不变量

- `analysis::ProofResult`：唯一三态证明结果。
- `analysis::IntegerFacts`：只对 i32 常量、无回绕的局部算术、常量 mask、
  exact shift/rem 等给出 range/known-bit；无法排除模 `2^32` 回绕时返回 Unknown。
- `analysis::LoopTripRecipe`：区分 `ConstantMachineExact`、`RuntimeBounded` 和
  `Unknown`。常量 trip 必须证明最终 IV 不越过 i32 边界；runtime bounded 只提供
  可查询上限，不自动授权删除、展开或终止性变换。
- `AnalysisManager::{moduleEpoch,functionEpoch}`：任何 CFG/effect/module mutation
  都递增相应 epoch，并清除 MemorySSA-lite 和函数语义摘要；`dump()` 只观测，
  `SYC_DUMP_ANALYSIS=1` 不改变生成汇编。
- `verifyModuleChecked`：检查 terminator、双向 CFG、leading PHI、PHI predecessor、
  类型、live operand、use-list、同块先定义后使用及跨块 dominance。
- `PlanningClone`：完整克隆 builtin/global/function/argument/block/instruction/
  constant 和 memory metadata；拒绝候选销毁全部 clone Value 后恢复全局 Value ID
  检查点，避免失败规划改变后续确定性排序。
- `CandidateTransaction`：严格执行 prove → clone → transform → verify → shadow
  pipeline → admit → commit。
- `runMachinePipeline`：production/planning 共用 cleanup、shrink-wrap、pre-RA
  schedule、linear scan、branch rotation、post-RA schedule、layout、peephole、frame。
- `MachineMetrics`：函数/块/指令、load/store/call/branch、GPR/FPR 峰值压力、
  spill/reload、spill slot/bytes、frame bytes、估算关键路径、循环 CFG 边和汇编
  bytes；另提供热循环 load/store/branch、load-use、mul/div、地址物化和保守压缩
  机会指标，用于归因而不单独授权变换。
- `rv64TargetProfile`：显式区分 generic RV64GC 与未标定的 BOOM RV64GC profile。
  BOOM profile 只记录已知的多发射结构属性；在具体核配置与计数器证据缺失时，
  load-use latency 保持 0（unknown），且 `costsCalibratedForSpecificCPU=false`。
- `verifyMachineModule`：检查 entry、CFG target、parent、寄存器编号/类别、未分配
  vreg 和 16-byte frame alignment。

## 4. 当前保守边界

1. CoreIR verifier 是结构验证，不是 SMT refinement checker；具体 pass 仍需提供
   termination、overflow、trap、alias、observer 和 live-out proof。
2. MemorySSA-lite 是按底层对象分区的函数内轻量实现；它适合 clobber、无环路径
   合流和拒绝候选，循环 MemoryPhi 与复杂跨调用 memory equivalence 仍保持
   Unknown。
3. pointer range 只接受 owned object 和常量 GEP。integer facts 已能处理保守的
   i32 局部范围与简单 runtime-bound 上限，但动态 GEP、跨循环 induction range 和
   runtime guard 仍未闭环，必须保持 MayAlias/Unknown。
4. 当前 RV64 latency 表没有绑定真实 CPU，admission 中 spill/frame/pressure 和
   硬性 verifier 证据比估算 latency 优先。
5. RV64GC strict 不生成 RVV；未来 RVV 必须新增独立 TargetInfo、ABI、RA、tail/
   mask 和 emitter 合同。
6. `pure_call_last_value` 和 `loop_state_fixpoint` 在当前基线没有 pipeline 入口，
   状态为 `DORMANT`。保留实现不等于已接线能力，恢复前必须重新完成 proof archive。
7. if-convert accumulation 只启用不提前执行 load 的 strict 子集。宽 research 模式
   曾在矩阵累积夹具上把 `4400` 错编为 `0`；具体 enable 现在也不能绕过 strict
   speculation gate，相关夹具作为 fail-closed 隔离回归。

## 5. 稳定拒绝原因

IR：`bad-phi-arity`、`phi-predecessor-mismatch`、`use-before-definition`、
`definition-does-not-dominate-use`、`bad-store` 等。  
MIR：`missing-machine-terminator`、`invalid-machine-cfg-edge`、`unallocated-vreg`、
`register-class-mismatch`、`invalid-frame-alignment`。  
Admission：`candidate-verifier-failed`、`spill-growth`、`frame-growth`、
`frame-limit-exceeded`、`register-pressure-growth`、`code-growth`、
`critical-path-growth`、`cyclic-cfg-growth`、`assembly-size-growth`、
`no-machine-benefit`。其中汇编字节数来自 baseline/candidate 各自完成真实 RV64
机器流水线后的实际打印结果，因而还覆盖数据段、对齐和指令计数看不到的膨胀。

`SYC_ENABLE_PLANNED_STRENGTH_REDUCTION` 是首个端到端接线 pilot：仅在 research
构建、`SYC_RESEARCH=1` 和该具体开关同时满足时，常量强度削减会先在 planning
clone 上运行，再经过 verifier、真实 RV64 admission 后提交。默认 strict 仍走
已经审计的原路径；pilot 用于持续标定成本阈值和拒绝原因，而不扩大提交面。

这些字符串用于测试和 trace；新增判断应新增 reason，不应复用不相干原因。

## 6. Phase 4/5 当前接线

- `SYC_ENABLE_PLANNED_LOCAL_EQUIVALENCES` 将 local load forwarding、常量折叠、
  局部代数、常量强度削减、parity 和 basic-block CSE 作为一个 research 候选，
  统一经过 verifier 与 RV64 admission。
- stage fusion、stencil、loop fusion/interchange、loop accumulator、jump threading、
  zero coefficient specialization 和 recursive memoization 的 research 入口已接到
  事务包装；未产生候选、坏 PHI、frame/code growth 都只形成稳定拒绝，不污染
  baseline。row locality 等 strict 变换继续依赖自身既有 proof contract，不因通用
  transaction 基础设施自动获得新授权。
- strict/research 源集合在 CMake 中显式分离；语义环境变量按 observability、
  rollback、semantic enable、unknown semantic 分类。strict 忽略后三类，research
  还必须满足 `SYC_RESEARCH=1`。
- MIR 自然循环仍用哈希集合做成员查询，但所有会选择 rewrite representative 的
  遍历先按块标签排序。该契约消除了 ASLR 导致的循环地址归纳非确定性。

## 7. 当前验证边界

- 当前 108 个 `tests/riscv/*.sy`：strict O0/O1 均可编译；analysis dump、strict
  hostile semantic env、research 未 opt-in 三组比较均为 `0/108 changed`。
- planning 合同覆盖 exact integer range、may-wrap Unknown、四次 machine-exact
  trip、runtime masked bound 最大 15 次、`INT_MAX` 回绕/不终止拒绝，以及 clone/
  verifier/admission 回退。
- 本地 RV64 runtime 下 semantic compliance、default recognizer、loop-nest、
  research if-convert 隔离和 AST matmul 专项门禁通过。
- 正式外部功能/性能套件路径本轮不可用；因此本文不把历史 140+60 结果重新表述为
  本轮动态验收，也不提供新的性能收益结论。
