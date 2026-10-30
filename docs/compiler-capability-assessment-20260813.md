# 编译器整体能力评估报告

评估日期：2026-08-13  
评估对象：当前 `compiler/master`，提交 `c7eba48`
评估目标：从语言覆盖、编译架构、优化能力、RISC-V 后端、工程质量和可交付性六个方面判断当前编译器的整体成熟度。

## 一、总体结论

当前项目已经不是“能生成简单汇编”的原型，而是一条较完整的 SysY2022 → RV64GC 编译链：具备独立功能通道、CoreIR SSA 中间表示、CFG/循环/effect/alias/仿射分析、多阶段优化、MIR 降低、线性扫描寄存器分配、调度、栈帧布局和汇编输出。

综合判断：

| 维度 | 评价 | 判断 |
|---|---|---|
| SysY 语言与前端 | 强 | 已覆盖整数、浮点、数组、函数、递归、控制流和运行时调用；140/140 功能用例通过。 |
| 编译器架构完整性 | 强 | 前端、HIR、CoreIR、Rewrite、MIR、ABI 和两条编译通道均已成形。 |
| 常规优化能力 | 强 | 具备 SSA、CSE/GVN、SCCP、LICM、循环、内存和目标相关优化。 |
| 算法级性能优化 | 很强但风险高 | 覆盖递推、融合、数组摘要、stencil、memoization 等高级变换，但部分默认 pass 存在此前合规审计指出的 Block/Proof-required 风险。 |
| RISC-V 后端能力 | 中上 | RV64GC、lp64d ABI、medany、spill/reload、调度和栈帧均已实现；边界压力测试仍需扩大。 |
| 测试与工程成熟度 | 中上 | 语义台账和脚本体系较完善；端到端功能验证充分，但性能基准、工具链依赖和 warning 清理仍影响交付稳健性。 |

综合等级：**工程能力中上，优化能力强，比赛/研究原型成熟度较高；若按“可长期维护、严格合规、可直接提交”的标准，当前仍需处理默认算法级变换和环境旁路问题。**

## 二、架构能力

### 1. 编译链完整

项目源码约 100 个 `.cpp`、103 个头文件，总规模约 7.6 万行。主链路为：

```text
Lexer → Parser/AST → HIR/AST 变换 → ASTToIR → CoreIR/SSA
      → Analysis/Passes/Rewrite → MIR lowering
      → register allocation → scheduling/layout/peephole
      → frame lowering → RV64GC assembly
```

这条链的价值在于：

- 功能通道不带 `-O1` 时走 `src/backend/codegen.cpp` 的独立 AST 直接后端，便于把优化错误与基础代码生成错误区分开。
- 性能通道在 `src/main.cpp` 中统一进入 AST/HIR、CoreIR 和 MIR，具备足够的中间表示承载复杂优化。
- `AnalysisManager` 维护 CFG、支配、自然循环、effect、alias/escape、终止、仿射访问和 liveness 等分析，并在 IR 变化后进行失效处理。
- CoreIR 有显式 BasicBlock、Phi、use-def 和 CFG，已经达到实现中等复杂优化的结构基础。

### 2. 前端能力

从 `src/frontend/lexer.cpp`、`parser.cpp` 和 `ast.h` 看，前端支持：

- `int`、`float`、`void`；
- 十进制、八进制、十六进制整数及浮点/指数形式；
- 全局/局部变量和常量、多维数组、聚合初始化；
- 函数定义、参数、调用、返回；
- `if/else`、`while`、`break`、`continue`、短路逻辑；
- 注释、作用域、常量维度和基础编译期常量求值；
- SysY 运行时函数，包括整数/浮点及数组输入输出。

对比赛型 SysY 程序而言，语言覆盖已经足够完整。功能回归中的排序、图算法、动态规划、矩阵、浮点、复杂作用域和长代码用例均通过，说明前端和基础语义实现不是当前主要瓶颈。

## 三、优化能力

### 1. 常规优化基础扎实

`tests/riscv/default_pass_semantics.tsv` 登记了 124 个逻辑入口，其中 80 个标为标准语义、31 个标为独立正负证据、10 个为 research-only、1 个 debug-only、2 个 default-unobserved。已实现的常规能力包括：

- 常量折叠、代数化简、强度削弱；
- Mem2Reg、SROA、DCE、死存储消除、dead-return；
- basic-block CSE、支配树 CSE、GVN、PRE、SCCP、GCM、instcombine；
- LICM、load hoist、循环规范化、循环展开、循环不变量和地址递推；
- effect/alias 驱动的 load/store forwarding；
- 函数内联、受限常量实参特化和尾递归消除；
- MIR 层的 peephole、block layout、列表调度、分支旋转、寄存器分配后清理。

这些能力属于比较标准的现代编译器中端/后端功能，整体技术路线正确。

### 2. 高级优化覆盖面很大

当前还实现了：

- modular scalar recurrence、small linear recurrence、repeated reduction；
- row locality、transpose view forwarding、loop blocking；
- producer-map/stencil interior、stage fusion、row broadcast fusion；
- demand-driven copy、cross-call array summary；
- recursive radix lowering、recursive accumulator normalization；
- recursion memoization、recursive modular multiplication；
- 运行时不变除数版本化、MIR 地址递推和 pre/post-RA 调度。

从研究和竞赛优化能力看，这个覆盖面已经明显高于普通课程项目，能够针对矩阵、递推、数组、卷积、排序和复杂循环产生较大代码形态变化。

### 3. 优化能力的主要风险

高级优化的“能力强”不等于“默认路径成熟”。此前合规审计已确认：

- `radix_sort_lowering.cpp`、`demand_driven_copy_transform.cpp`、`cross_call_array_summary.cpp`、`stage_fusion.cpp`、`producer_map.cpp` 和 `recursion_memoization.cpp` 在 strict 默认路径中仍执行算法骨架识别、阶段删除、旁路或缓存化。
- 这些变换可能改变算法复杂度或跨阶段可观察行为，不能仅用局部 IR 事实说明正确；其中一些应移到 research-only，另一些至少需要完整通用证明。
- `SYC_SKIP_*`、`MIR_NO_SCHEDULE`、`MIR_SIMPLE_RA` 等环境变量仍能改变 strict 默认代码生成路径，影响可复现性和提交配置边界。

因此，当前优化器应评价为：**性能研究能力强，通用编译器默认优化的边界管理尚未完全成熟。**

## 四、后端与目标平台能力

### 1. RISC-V 支持范围

后端明确面向 RV64GC，并按 `lp64d` 风格处理整数/浮点 ABI。已覆盖：

- CoreIR → MIR 指令选择和类型降低；
- 整数、浮点、比较、分支、调用和内存操作；
- `i32` word 运算及符号扩展；
- 整数/浮点参数和返回值寄存器；
- 栈参数、spill slot、caller/callee-saved 寄存器；
- `medany` 大地址模型和大偏移地址物化；
- 线性扫描寄存器分配及 spill/reload；
- 分配前/后列表调度、循环分支旋转、块布局和 peephole；
- 栈帧建立、保存恢复和最终汇编打印。

### 2. 后端实测表现

- 108 个仓库内 SysY 夹具在 O0 和 O1 下均成功生成汇编。
- 代表性的位模式、尾递归、递归模乘和递归 memoization 程序均可完成 RISC-V 链接和 QEMU 运行；O0/O1 输出一致。
- 语义脚本 `test_riscv_default_recognizers.sh`、`test_riscv_remaining_pass_semantics.sh` 和 `test_riscv_compliance_risks.sh` 在使用重建的 RISC-V runtime 后通过。
- MIR 地址递推、转置视图、循环展开、row locality、stencil interior、除数特化和 if-conversion 等 trace 均显示实际路径可达，说明后端不是“只登记未落地”。

### 3. 后端成熟度限制

当前尚缺少以下强证据：

- 大规模随机程序、随机寄存器压力和随机调用图下的长时间稳定性；
- 浮点 NaN、边界舍入、异常路径和混合 ABI 的系统性测试；
- 栈参数很多、FPR/GPR 同时耗尽、深层嵌套调用和大帧对象的专门压力测试；
- 多种 RISC-V 核心的真实性能数据，当前主要是目标成本模型和 QEMU 证据；
- 完整公开性能集的 O0/O1 等价和性能回归报告归档。

因此后端可称为“功能可用、结构完整、仍需扩大压力验证”，而不是已经达到 GCC/LLVM 级别的工业稳健性。

## 五、测试与验证能力

### 1. 已验证项目

- strict 构建：`cmake --build /tmp/compiler-cap-build -j2` 成功。
- Python 单元/静态测试：`python3 -m unittest discover -s tests/tools -p 'test_*.py'`，18 项全部通过。
- 功能回归 O0：140/140 通过。
- 功能回归 O1：140/140 通过。
- 性能用例 O1 端到端：60/60 通过，使用 `testcases/performance` 和重建的 RISC-V runtime，在编译、链接和 QEMU 执行层面完成验证。
- 108 个仓库内语义夹具：O0/O1 均可成功编译。

### 2. 测试体系优点

- 正例/近似负例并存，关注 alias、side effect、live-out、边界、终止和零迭代。
- pass 台账将入口、语义、正例、负例和证据路径关联起来，便于审计和回归。
- 多数专项脚本会比较 default/skip 汇编，并进一步链接和运行；这比只检查“编译成功”可靠得多。
- strict/research 隔离和静态风险扫描已有自动化门禁。

### 3. 测试体系缺口

- 部分脚本依赖外部性能用例路径和 runtime 环境，参数不统一时容易出现“脚本本身失败”而非编译器失败。
- O1 性能结果主要验证功能和变换触发，缺少统一的运行时间、代码尺寸、指令数、访存数和回归阈值报告。
- 仍有 `DEFAULT_UNOBSERVED` pass，表示部分入口还没有独立可观察证据。
- 编译时存在较多 warning，包括未使用函数/参数、未处理枚举项和误导性缩进；目前不阻塞构建，但降低长期维护信心。
- 端到端测试集中在已有夹具，随机差分、模糊测试和 sanitizer/UBSan/ASan 长时运行证据不足。

## 六、工程成熟度和可维护性

### 优点

- 模块边界清楚：frontend、analysis、HIR、CoreIR、rewrite、MIR 和 backend 分层明确。
- `PipelineBuilder` 把性能配置从 `main.cpp` 集中管理，优化入口可追踪。
- IR 改写后普遍有 CFG/effect 分析失效和重新计算逻辑。
- 研究 pass 有 CMake 级隔离，不只是运行时关闭。
- 文档、测试台账、审计标准和工具脚本已经形成较完整的工程闭环。

### 问题

- HIR/CoreIR 中部分文件规模较大，算法级变换与常规优化交织，增加了审查和回归成本。
- 一些 `README` 仍保留早期 TODO，与当前实现成熟度不完全同步。
- 默认 `-O1` 内部使用“O3 级预算”配置（见 `src/core_ir/passes/pipeline_builder.cpp:15`），公开优化等级语义与内部策略不完全一致，后续应明确命名或文档化。
- 环境变量可以旁路默认 pass，使同一命令在不同环境下生成不同汇编；提交构建需要更严格的配置封装。
- 当前构建依赖 CMake glob 和大量条件编译，新增/删除 pass 时需要额外检查是否真的进入 strict 二进制。

## 七、能力边界

### 已具备

- 面向 SysY2022 竞赛语言的完整编译能力；
- RV64GC 静态链接汇编输出；
- 中等复杂程序、浮点、数组、递归、图算法和矩阵程序；
- 现代 SSA 中端和基本机器后端；
- 面向循环和数组密集程序的较强性能优化潜力；
- 可执行的 pass 语义回归和端到端 RISC-V 验证。

### 尚不应宣称

- 不应宣称达到 LLVM/GCC 的通用语言、目标架构和诊断能力；
- 不应宣称所有高级 pass 已完成严格通用正确性证明；
- 不应宣称已有稳定、可复现的跨平台性能优势；
- 不应把 60 个性能用例通过等同于所有输入和所有别名/副作用条件都安全；
- 不应把当前默认优化路径直接视为已完成比赛合规审查。

## 八、建议路线图

### P0：提交/发布前必须完成

1. 将算法级默认 pass（radix、demand-copy、cross-call summary、stage fusion、producer-map、recursive memoization）逐一改为 research-only，或补齐可审计的通用等价证明。
2. 将 `SYC_SKIP_*`、`MIR_NO_*` 等影响代码生成的开关统一纳入明确的 research/debug 配置，确保 strict 二进制和环境无关。
3. 统一所有测试脚本的 runtime、性能用例和工具链参数，避免路径错误掩盖真实结果。

### P1：提升工程稳健性

1. 清理编译 warning，至少消除未处理枚举、误导性缩进、未使用函数和未使用参数。
2. 补充随机差分测试：O0 vs O1、default vs skip、直接后端 vs CoreIR/MIR 后端。
3. 为每个高级 pass 建立代码尺寸、运行时间、指令数和失败回退统计。
4. 增加寄存器压力、深调用、浮点边界、超大数组和大偏移栈帧测试。

### P2：长期能力建设

1. 把 CoreIR verifier、effect/alias 证明和 pass 失效协议进一步形式化。
2. 将性能配置从“公开 `-O1`、内部 O3 预算”拆分为清晰的成本模型与可解释等级。
3. 建立跨提交性能基线，区分真实通用收益与单个 benchmark 的收益。

## 九、最终评价

当前编译器的**基础编译能力已经达到可用且较完整的竞赛级水平**；从架构和优化数量看，明显高于普通 SysY 课程项目。尤其是 140/140 功能用例、60/60 性能用例 O1 端到端通过，说明当前版本具备较强的实际交付能力。

但它仍更像“优化研究能力很强的竞赛编译器”，而不是已经完成所有边界证明和配置收敛的工业编译器。短期最大风险不是前端或 RV64 后端缺功能，而是默认路径中高级算法变换的合规/通用性证明，以及环境旁路导致的可复现性问题。若先完成 P0 项，整体能力可从“强但有风险”提升到“可审计、可提交、可持续维护”。
