# AGENTS.md

本文件是本仓库自动化代理的长期工作规范。内容由 ARM 分支的稳定规范迁移而来，
但已按当前 RISC-V 工程的真实目录、公开接口、后端、ABI、测试工具和合规要求
重新适配。ARM 分支的阶段历史、性能数字、AArch64/NEON 专属能力和路径不得机械
复制到本仓库。

本文只保留长期有效的架构合同、正确性不变量、合规红线和验证门槛。当前能力、
审计结论和证据以源码、`docs/`、`tests/riscv/default_pass_semantics.tsv` 与
`tests/riscv/evidence/` 为准。

## 1. 项目与公开接口

- 目标：把 SysY2022 源码编译为可在 RV64GC Linux 环境使用的 RISC-V 汇编。
- 实现：C++17；评测可执行文件只能由仓库中的 `src/main.cpp` 提供一个编译器
  `main`，临时单元测试不得向提交源码增加第二个入口。
- 公开 CLI 只有：

```bash
compiler -S -o testcase.s testcase.sy
compiler -S -o testcase.s testcase.sy -O1
```

- `-O2`、`-O3`、`--legacy-backend`、`--mir-backend` 等不是公开接口，当前应明确
  拒绝，不能静默映射为其他模式。
- 不带 `-O1` 的功能通道：Lexer/Parser/AST -> `src/backend/` 直接 RISC-V
  代码生成器。
- 带 `-O1` 的性能通道：AST/HIR 规范化 -> CoreIR -> CoreIR 优化 -> MIR ->
  RV64GC 汇编。
- 性能通道统一由 `src/backend_mir/` 闭环；不得为了绕过 MIR、寄存器分配或
  frame 问题，恢复隐蔽的整函数/样例汇编发射器。
- strict 构建是默认提交形态。research pass 只有在 CMake 显式设置
  `-DSYC_ENABLE_RESEARCH_PASSES=ON`，并且运行时同时设置 `SYC_RESEARCH=1`
  和对应 `SYC_ENABLE_*` 时才允许改变代码生成。
- strict 构建中的环境变量只能用于 `TRACE`、`DUMP`、`VERIFY`、`PROFILE`、
  `REPORT`、`LOG` 等可观测行为；相同 CLI 和输入不得因普通环境差异生成不同
  汇编。

## 2. 关键目录与职责

```text
src/
  main.cpp                         CLI 与两条编译通道的顶层编排
  research_switch.h                strict/research 环境开关合同
  frontend/                        Lexer、Parser、AST
  analysis/                        CFG、dominance、loop、effect、affine 等分析
  hir/                             AST/HIR 规范化与高层变换
  core_ir/
    ir/                            CoreIR 类型、Value、Instruction、CFG、Phi
    lowering/                      AST -> CoreIR
    passes/                        Pass、pipeline、CoreIR 优化与 verifier
  rewrite/                         局部代数、位模式与等价重写
  backend/                         无 `-O1` 的 AST 直接 RISC-V 后端
  backend_mir/                     MIR lowering、cleanup、schedule、RA、frame、汇编
tests/
  run_functional.py                外部用例集 + runtime 的功能回归入口
  riscv/                           pass 语义台账、正例、近似负例和证据
  tools/                           合规、语义、strict/research 门禁脚本
tools/                             全量回归、汇编比较、历史与严格性能工具
docs/
  compiler-design.md               当前架构说明
  compliance/                      默认优化泛化性标准与审计报告
```

- 根 `CMakeLists.txt` 使用 `src/*.cpp` 递归收集源文件。新增 research-only 实现
  不能只靠“未挂入 pipeline”隔离，必须同时从 strict source list 排除并使用统一
  research 总开关。
- 每个 pass 保持独立入口；pipeline 只负责编排，不直接访问 pass 私有 helper，
  也不得通过 `#include` 其他 pass 的 `.cpp` 复用实现。
- SysY runtime 和正式功能/性能用例集是外部输入，不属于提交源码。文档和命令
  不得假设仓库存在 `lib/` 或 `testcases/`。

## 3. 修改前必须核对的资料

涉及默认 `-O1`、研究隔离、算法级变换或合规审查时，至少阅读：

- `docs/compliance/generality-audit-standard.md`
- `tests/riscv/default_pass_semantics.tsv`
- `tests/riscv/README.md`

如果工作区存在官方《关于编译优化合理性及相关违规行为认定的说明》PDF，PDF
是合规裁决的唯一规范性条件；仓库自定义标准、台账和历史审计只作为工程证明、
反例与排查材料，不能额外禁止 PDF 未禁止且能够保证通用正确性的优化。日期化/
外部材料可能不属于 Git 跟踪文件，不能在长期规范中假设固定文件名。

涉及架构或流水线时，同时阅读 `docs/compiler-design.md`、`src/main.cpp`、
`src/core_ir/passes/pipeline_builder.cpp` 和 `src/backend_mir/mir_backend.cpp`。
审计报告中的未关闭 finding 不能因“测试已通过”或“性能更好”而忽略。

## 4. 正确性不变量

### 4.1 CoreIR、CFG 与内存

- CFG 修改前必须完成 legality proof。预检失败时不得留下部分 edge、block、Phi
  或 ownership 改动；优先采用先证明、后一次性提交 mutation 的事务式结构。
- Phi incoming 必须与 predecessor 一一对应。修改 header、latch、preheader、
  exit、side exit 或分支目标后，立即修复 Phi/use-def，并使 dominance、loop、
  liveness、effect 等相关 analysis 失效。
- 自然循环、支配、归纳变量、live-out、side exit、trip count 和终止性必须来自
  IR 事实，不能由 block 名称、源码布局、固定节点数量或 benchmark 形状猜测。
- 可能发生 i32 有符号回绕时，不得把数学整数上的 range/trip count 当作
  machine-exact；整数变换必须保持当前 i32 模 `2^32` 行为。
- 禁止依赖 C/C++ UB、SysY 越界、未初始化值、未证明对齐或未证明的值域完成
  优化。除零、移位范围、`INT_MIN / -1` 和窄化/扩展语义必须显式处理。
- alias 以底层 object identity、offset/range 和 escape 事实证明。不同 direct
  global 或本函数 nonescaping local object 可在证明充分时判为不相交；pointer
  argument、未知 producer 和不完整 metadata 一律保守视为 may-alias。
- 任一 write 与 may-alias read/write 冲突时保守拒绝，除非具备合法 runtime
  guard、完整 fallback 和 guard 本身的可执行性证明。
- 未知 call 保守视为可能读写内存、改变全局状态或不终止；只有版本化且仍有效的
  effect/termination summary 才能放宽。
- 删除、合并或重排循环/调用前，必须证明 side effect 顺序、中间状态可观察性、
  trap/终止行为和所有 live-out 保持不变。
- 浮点优化默认不得重关联、改变求值顺序、吞掉 NaN/Inf/有符号零差异或假设
  fast-math。需要放宽时必须有明确语言合同和独立验证。

### 4.2 MIR、寄存器分配与 RV64GC

- 目标是 RV64GC 标量 ISA 与 RISC-V psABI；不能沿用 AArch64 的 NZCV、
  `CCMP`、条件执行、post-index、shifted register addressing、NEON/Vector128
  或 AAPCS64 假设。RV64GC 不包含 RISC-V V 扩展。
- 指针和地址计算使用 64 位语义；SysY `int` 的核心算术与比较保持 i32 语义。
  `ADDW/SUBW/MULW/DIVW/REMW` 等 W 指令会把低 32 位结果符号扩展到 XLEN，
  用 64 位指令替换时必须证明高位与后续比较/存储语义等价。
- `x0` 永远为零且不可分配；`sp`、`ra` 及其他保留寄存器不得被普通 vreg
  分配覆盖。caller-saved/callee-saved GPR/FPR、返回寄存器和参数寄存器必须按
  RV64 psABI 处理。
- ABI 参数布局必须正确处理 `a0..a7`、`fa0..fa7`、浮点回退到整数寄存器、
  overflow stack arguments、返回值宽度以及 call 前后的 live interval。
- SP 在所有 ABI 可观察边界保持 16 字节对齐。frame 必须覆盖 outgoing args、
  alloca、spill、保存的 `ra/s*` 与需要保存的浮点寄存器；序言和所有返回路径
  必须对称恢复。
- PHI edge copy 必须实现 parallel copy。所有寄存器源读取完成前不能覆盖其值；
  copy cycle 必须使用安全临时寄存器或稳定 stack home。
- 同一基本块的所有 Phi definition 共享 block-entry 位置，不能按 Phi 遍历顺序
  制造虚假的不重叠 live range。
- 循环中的 victim spill 必须有跨迭代稳定 stack home；不能把一次性
  physical-to-stack transition 当作每轮重新产生的定义。
- ABI call 后必须使 caller-clobbered 缓存和相关 rematerialization 假设失效；
  callee-saved/global-address cache 也要随实际寄存器失效同步清理。
- RISC-V 普通 load/store 与 `addi` 的立即数通常只有 signed 12-bit
  `[-2048, 2047]`。大 frame、大对象和大偏移必须通过安全地址 materialization
  降低，且临时寄存器不能与数据源、目标或 ABI 活跃值冲突。
- 生成的外部符号与大地址空间代码必须兼容工程使用的 `-mcmodel=medany` 链接
  方式；不得依赖宿主机地址、链接偶然布局或 QEMU 特性。
- 调度、block layout 和 peephole 必须保留真实数据依赖、内存顺序、call/return、
  ABI 实参准备和 fallthrough 语义。删除跳转或反转分支后必须同步 CFG。
- 后端暴露能力缺口时优先修 MIR、RA、frame lowering 或 RISC-V emitter，不能
  破坏已经证明正确的 CoreIR CFG/lowering 来迎合后端。

## 5. 合规红线

### 5.1 明确禁止

- 按 benchmark case、源文件、路径、目录、family、输出文件名、普通函数名、
  变量名、参数名或全局名触发优化。
- 匹配运行时输入、输入规模、值分布、随机种子、稀疏率、I/O 次数、计时调用
  邻域、行号、注释、源码字符串或源码 hash。
- 对特定用例硬编码输出、查找表、预计算答案、固定算法常量或跳过真实语义计算。
- 在缺少通用 legality proof 时识别公开 benchmark/算法骨架并替换整函数、
  整个调用链或多阶段计算；改变算法复杂度的 memoization、radix、summary、
  producer/checksum bypass 等可以作为通用优化，但进入 strict 前必须证明对全部
  满足触发前提的程序正确，并证明触发不依赖特定用例、输入或环境。
- 在无 IR-level range、alias、effect、termination、live-out 或依赖证明时假设
  输入值域、内存互异、循环次数、调用纯度或阶段不可观察。
- 利用 UB、越界、未初始化值、ABI 偶然性、栈布局、链接布局、宿主架构或
  QEMU/TCG 环境获取收益。
- 用普通环境变量在 strict build 中隐藏样例特判、关闭默认 pass、切换 RA/后端
  或恢复 research-only 变换。
- 在汇编元数据中硬编码 `program.sy` 等固定源名；需要 `.file` 时必须使用真实且
  不参与优化判定的输入信息。

名称只允许用于：

- 语言规定入口 `main`；
- 正式 SysY runtime builtin 和 ABI 协议；
- 编译器内部保留 helper 的冲突避免；
- 不影响代码生成的调试日志展示。

普通用户名称不得参与 transform 的启用、形态选择或成本判断。

### 5.2 允许使用的通用事实

- Opcode、类型、常量表达式和 use-def。
- CFG、SSA、dominance、Phi、liveness、自然循环与 induction facts。
- object identity、alias、escape、memory/call effect 和 termination facts。
- 可证明的 affine address、alignment、trip count、依赖关系与 runtime guard。
- RV64GC ISA、RISC-V psABI、可编码立即数/寻址、延迟/吞吐、code size、
  register pressure、spill/frame 和 working-set 成本。
- 通用代数恒等式以及明确保持位宽、溢出、浮点和内存语义的局部 rewrite。

block/inst/vreg 数量可以作为编译成本或 profitability 的输入，但不能冒充语义
正确性证明，也不能使用为某个公开用例量身定制的魔数阈值。

### 5.3 通用修复优先

- 优化失配、miscompile 或性能回退时，优先修共享 analysis、IR contract、
  canonicalization、planner、operand constraint、RA、frame 或 target cost model。
- 不得把按 family 排除、函数名黑名单、恢复旧输出、关闭通用优化或缩窄到公开
  样例形状作为共享能力缺口的首选修复。
- 只有缺少 correctness proof，或目标成本模型证明整类候选不盈利时才允许拒绝；
  trace/reject reason 应写明缺失的 dominance、alias、range、effect、ABI 或成本
  事实，而不是样例名称。

## 6. 工作方式

- 修改前确认目标 diff、公开通道、strict/research 归属、rollback 方式和影响层级。
- 尊重 dirty/staged worktree；不得 reset、checkout、覆盖、取消暂存或回滚用户及
  其他代理的修改。发现无法解释的新改动时先停止并核对。
- 不 commit、不 push、不建分支，除非用户明确要求。
- 修改保持 C++17、现有中文文件职责注释、命名空间和 `unique_ptr` 所有权风格。
- 每个 pass 暴露统一入口，避免跨层依赖私有 helper；新增共享能力放入明确的
  analysis/IR/MIR 公共合同，而不是复制实现。
- 新增影响代码生成的环境开关必须通过 `sycResearchSwitchEnabled` 或受
  `sycEnvironmentValue` 的 research 合同保护，命名语义化且不能含样例简称。
- 新增默认优化必须更新 `tests/riscv/default_pass_semantics.tsv`，提供至少一个
  真正触发的正例、一个形状相近但因 alias/effect/live-out/边界等拒绝的负例，
  并接入相应 `tests/tools/` 门禁。
- 不顺手修无关问题；既有失败明确标为 pre-existing，不得误报为本轮回归或全绿。
- 正式设计或合规结论更新到 `docs/`；可复核汇编、trace、QEMU 与汇总证据放在
  `tests/riscv/evidence/`，不要把临时 `/tmp` 产物提交进仓库。

## 7. 验证流程

### 7.0 性能测试口径（强制）

- 性能对比开始前必须显式记录 `git branch --show-current`、当前
  `git rev-parse HEAD`、基线 ref 及其完整提交号。不得把“当前分支的 clean
  HEAD”自行解释为性能基线；当仓库同时存在 `master`、`master-new` 或迁移
  锚点时，必须按任务/计划指定的 ref 独立导出和构建。
- RISC-V 性能结论只能使用正式 RISC-V 用例和对应 `.in/.out`。不得因为文件名
  或 `.sy` 内容相同而用 ARM 目录代替；输入规模可能不同，并会造成数量级错误。
  外部套件路径必须在报告中写明，必要时核对关键 `.sy/.in/.out` 的摘要。
- 性能编译必须使用公开优化通道 `-S -O1 -o`。`-o` 只指定输出文件，不能视为
  启用优化；漏传 `-O1` 的结果必须作废。
- 性能报告必须分别给出 SysY runtime 的内部计时 `internal_us`、外部纯运行墙钟
  `run_ms`、编译时间 `compile_ms`、链接时间 `link_ms` 和整轮墙钟 `wall_s`。
  性能胜负与总性能的唯一主口径是各用例有效内部计时及其总和
  `total_internal_us`；`run_ms` 只作为 QEMU 启动、宿主环境噪声、超时和异常值的
  辅助诊断，不得用于替代内部计时作性能结论。缺少有效内部 timer 的用例只能报告
  correctness 与辅助 `run_ms`，不能计入性能改善/回退结论。内部计时不得与外部
  `run_ms`、链接或整轮墙钟混加。
- 功能 runner 只用于判断输出正确性，不提供性能结论。性能测试使用
  `tests/run_performance.py --jobs 1`；动态对比不得并行，避免共享 CPU/内存资源
  污染主指标 `internal_us` 和辅助 `run_ms`。基线和候选必须使用相同 runtime、
  GCC、QEMU、用例、运行次数与超时设置。
- 有历史性能证据时先复现锚点，再测试候选。若基线 `total_internal_us` 与历史值
  相差明显，
  必须先排查分支、用例输入、runtime、优化参数和计时口径，不能继续输出回退
  结论。

### 7.1 构建与静态检查

```bash
git diff --check
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Release
cmake --build build-local -j"$(nproc)"
```

- CMake Release 是要求的编译器构建路径，不维护绕过 CMake 的 all-`.cpp` flat
  link 合同。
- 只有需要验证 research 隔离或实验 pass 时才额外构建：

```bash
cmake -S . -B build-research -DCMAKE_BUILD_TYPE=Release \
  -DSYC_ENABLE_RESEARCH_PASSES=ON
cmake --build build-research -j"$(nproc)"
```

- 修改 shared opcode、lowering、RA、frame、emitter 或 source-list 后，先检查生成
  汇编能由 `riscv64-linux-gnu-gcc` 以 RV64GC/medany 静态链接。
- 静态汇编检查优先于性能运行：关注热函数 instruction/branch/call、frame、
  spill/reload、`lw/sw/ld/sd/flw/fsw`、地址 materialization 和 W/XLEN 语义。

### 7.2 语义与合规门禁

```bash
python3 tests/tools/test_riscv_pass_semantic_registry.py
python3 tests/tools/test_riscv_strict_audit.py
```

涉及对应优化族时，再运行相关脚本：

```bash
RUNTIME=/absolute/path/libsysy_runtime.a \
COMPILER=./build-local/compiler \
bash tests/tools/test_riscv_compliance_risks.sh
```

- `tests/tools/` 中部分脚本需要外部 runtime、RISC-V 交叉 GCC 和 QEMU；缺少依赖
  时必须报告“未运行”，不能把静态检查当作动态 PASS。
- strict/research 隔离改动必须同时验证：strict 注入 enable 环境后汇编不变；
  research build 只有在 `SYC_RESEARCH=1` 和具体 enable 同时存在时才改变行为。

### 7.3 Correctness

仓库不内置正式功能用例和 runtime，路径必须显式提供：

```bash
python3 tests/run_functional.py \
  --compiler ./build-local/compiler \
  --cases /absolute/path/to/functional-suite \
  --runtime /absolute/path/libsysy_runtime.a \
  --work-dir /tmp/syc-riscv-functional

python3 tests/run_functional.py \
  --compiler ./build-local/compiler \
  --cases /absolute/path/to/functional-suite \
  --runtime /absolute/path/libsysy_runtime.a \
  --work-dir /tmp/syc-riscv-functional-opt \
  --opt
```

- 只改性能通道的局部 pass 时先跑定向正/负例和 `--opt`；修改前端、语言语义、
  runtime ABI 或共享逻辑时必须同时跑无优化与 `--opt`。
- 修改 CFG、Phi、effect、verifier、MIR、RA、ABI、frame 或默认 emitter 时，最终
  应跑完整功能集；任何 WRONG、timeout、崩溃、assembler/linker/verifier error
  都阻断性能结论。
- rollback/A-B 必须使用同一个 compiler binary。strict build 中不能靠代码生成
  环境开关做 rollback；需要 rollback 的实验必须使用 research build 与统一总开关。

### 7.4 性能验证（迁移 ARM 的影响面驱动规则，强制）

1. 先生成 baseline/candidate 的 60 份汇编，规范化后列出 changed cases。
2. 汇编完全一致时不跑动态性能；结论只写“代码生成未变化”，不得从计时噪声
   宣称收益或回退。
3. 无论 changed cases 数量或共享影响范围，日常验证只串行测试 changed cases，
   另加至少一个 byte-identical control 判断环境噪声，不重复计时未变用例。
4. 默认只运行一轮 `B -> C`。research rollback 可使用同一 compiler binary 和
   统一 research 总开关；strict baseline/candidate 使用独立二进制时，必须保持
   构建类型、runtime、GCC、QEMU、输入、超时和运行顺序一致。性能命令不得并行。
   除非用户明确要求，或实施计划把某个里程碑明确列为全量验收，不做多轮反序
   A/B 或完整 `60/60` 动态计时。
5. 汇报每个 changed case 的 base/candidate 原始 `internal_us`、改善百分比、family
   geomean、异常值和 control；同时列出辅助 `run_ms` 供噪声诊断。单轮结果不得
   表述为多轮 median。需要稳定性结论时，才升级为预先约定的多轮/交错统计，且
   不得事后挑选有利样本。

性能命令使用 `tests/run_performance.py --opt --jobs 1`；可用 `--filter` 逐个选择
changed case/family。`--jobs 1` 是动态对比的强制值，不因机器核数增加并发。

### 7.5 全量里程碑验证

固定 140+60 外部套件可使用：

```bash
python3 tools/riscv_suite_runner.py \
  --suite-root /absolute/path/to/suite-root \
  --build ./build-local \
  --runtime /absolute/path/libsysy_runtime.a \
  --out-dir /tmp/syc-riscv-suite \
  --category all
```

- 完整 `60/60` 动态性能只在用户明确要求或计划规定的发布/大阶段里程碑运行；
  日常 pass 迭代遵守 7.4 的 changed-only 规则。
- 里程碑动态比较仍必须串行、使用同一 runtime/toolchain/QEMU、相同输入和可复核
  的样本规则。严格多轮比较沿用 `tools/riscv_strict_*` 及其单元测试定义，不临时
  发明更有利的异常值或胜负标准。
- 报告原始内部 `starttime/stoptime` 样本、辅助 QEMU 墙钟、异常值、汇编大小/
  指令数和 control。内部 timer 是唯一性能判定指标；明确减少热路径调用、分支、
  spill 或地址计算可以作为正向静态证据，但不能伪造动态收益。
- 用户中止、依赖缺失或部分完成的运行必须明确标成 partial，不能写成完整 PASS。

## 8. 审查输出规范

审查优先级：合规与语义正确性 > ABI/内存/寄存器安全 > 构建与 correctness >
通用性能收益 > 可维护性。

Finding 分级：

- **P0**：样例/名称/输入触发、硬编码结果、无证明算法/整函数替换、UB 利用或
  已知合法输入误编译。禁止合入。
- **P1**：可能 miscompile、ABI/alias/Phi/call-clobber/位宽/栈对齐错误，或
  verifier、assembler、functional 失败。修复后才能合入。
- **P2**：确定性性能回退、缺成本模型、缺 strict/research 隔离、缺 rollback、
  trace、覆盖或构建路径不一致。
- **P3**：不影响正确性、合规性和默认性能的维护性问题。

每条 finding 使用：

`[级别] 文件:行号 - 触发条件；实际影响；判定依据；修复建议或缺失测试`

结论先列 findings，再列 open questions、验证证据与残余风险。没有 finding 时明确
写“未发现阻断问题”。缺少 legality proof、完整 correctness、汇编影响面或所需
性能证据时，只能写“证据不足/需要修改”，不能批准合入。

## 9. 常用诊断原则

- `COREIR_DUMP=1` 可观察优化后的 CoreIR；其他 trace 名称以源码中的
  `sycEnvironmentValue` / `sycResearchSwitchEnabled` 调用为准，不复制 ARM
  分支的 `AArch64`、`A53`、`NEON` 或 `MIR_DISABLE_*` 旧开关。
- trace 必须输出通用 accept/reject 事实，如 dominance、alias、range、effect、
  pressure 和 cost；不得输出或依赖 benchmark 名称来控制优化。
- 调试时优先保存同一输入的 CoreIR、MIR/汇编 diff、链接命令、QEMU 输出和
  compiler stderr。只看到性能变化但没有汇编或 IR 差异，不足以归因。
