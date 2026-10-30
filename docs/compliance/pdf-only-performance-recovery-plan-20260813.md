# RV64 回退样例汇编分析与最大性能恢复计划（PDF 唯一合规口径）

日期：2026-08-13  
适用分支：`master-new` 当前工作区  
性能基线：`master@c7eba4837484dab6b80e19dabf610ebfa78a02fc`  
唯一合规依据：仓库根目录 `关于编译优化合理性及相关违规行为认定的说明.pdf`

实际开发按 `docs/generic-optimization-implementation-plan-20260813.md` 的通用能力工作包执行；本文保留回退汇编、旧优化消融归因、恢复价值和 PDF-only 裁决。

## 1. 目的与边界

本计划研究当前 strict 编译器相对性能基线发生回退的用例及汇编差异，并给出尽可能恢复性能的实施顺序。实施单位是可复用的 analysis、IR contract、planner 和机器优化能力，而不是 benchmark family；回退样例只负责暴露能力缺口和验证收益。合规结论只依据上述 PDF；仓库中的 `generality-audit-standard.md`、既有 B8/B9/C1 分类和历史审计报告只作为工程排查材料，不再作为额外禁止条件。

本计划允许恢复改变算法复杂度、识别通用算法结构或跨越多个循环阶段的优化，但必须同时满足：

1. 不以函数名、变量名、源文件名、字符串、测试集名称、参数个数指纹、输入值或输入模式识别特定测例；
2. 不依赖宿主机、QEMU、环境变量、计时调用邻域或其他比赛环境特征激活提交路径；
3. 对任意满足显式前提的程序均保持可观察语义，而不只是通过现有样例；
4. 不利用源程序未定义行为获取性能；
5. 证明不完整、分析未知或机器级收益不成立时保留原程序。

这里的“参数个数指纹”与正常类型检查必须区分：为了证明一次变换覆盖了全部实参而读取函数签名是语义检查；仅凭“有 N 个参数”猜测它属于某个测例则是 PDF 明确禁止的特征识别。

## 2. 按 PDF 重新评估后的合规结论

### 2.1 PDF 实际允许和禁止的范围

PDF 第二条明确把基于基本循环结构的展开、交换、合并、不变量外提，以及基于处理器通用特性的调度和寄存器分配列为合理通用优化。它没有规定“算法识别”“跨阶段改写”“memoization”或“复杂度变化”天然违规。

PDF 第三条禁止的是：

- 用函数名或特定字符串推测测例；
- 硬编码特定测例结果；
- 根据特定输入模式激活路径；
- 利用未定义行为；
- 依赖特定输入或环境、且不能保证通用正确性的投机或猜测。

因此，本轮采用如下裁决规则：

| 等级 | PDF-only 判定 | 默认流水线处置 |
|---|---|---|
| F：明确禁止 | 命中第三条列举行为，或只对已知输入正确 | 删除或仅保留诊断源码，不得进入提交二进制 |
| P：可证明后允许 | 触发来自 IR 结构/语义，但当前通用正确性证明不完整 | research 隔离；补齐证明、反例和回退后可进入 strict |
| A：允许 | 通用结构触发，等价性前提完整，未知时拒绝 | 可进入 strict，并接受机器级收益门禁 |

### 2.2 对现有旧优化的重新分类

源码扫描未发现按公开测例名、`.sy` 文件名、输入常数、输出结果、计时函数或 QEMU/宿主环境直接触发这些优化。日志打印函数名不参与触发，不构成函数名识别；但必须用测试保证改名后产物决策不变。

此前因“算法级”被整体隔离的优化，在 PDF-only 口径下应改判为 **P：原则上允许，证明完成前隔离**，而不是永久 Block：

| 旧优化 | PDF-only 重新判定 | 进入 strict 前缺少的关键条件 |
|---|---|---|
| 递归 memoization | P | 纯度、完整 cache key、碰撞比较、重入/生命周期、异常终止保持和收益证明 |
| recursive accumulator normalize | P | 任意函数名/常数均成立的递归等价归纳、溢出和所有退出路径 |
| stage fusion / producer-map bypass | P | 完整 effect/alias/依赖/观察者闭包和副作用顺序证明 |
| row locality / loop interchange / blocking | P 或 A | 仿射依赖、边界、remainder、规约顺序和成本模型；证明齐全后属于 PDF 第二条典型合理优化 |
| transpose view forwarding | P | 所有消费者、逃逸、clobber、索引映射和未执行路径证明 |
| pure-call last value / loop-state fixpoint break | P | 纯度、终止、trap、完整状态相等和未来行为确定性 |
| cross-call array summary | P | 跨函数 mod/ref、完整区域状态、所有调用/观察者闭包；不得用参数个数作为测例指纹 |
| demand-driven copy | P | MemorySSA、写覆盖、需求切片、别名、边界和被删除阶段不可观察证明 |
| recursive radix lowering | P | 对所有匹配程序的排序语义、稳定性/符号范围、容量、递归终止和资源语义证明 |

固定 `kTargetRadix = 256` 不是“硬编码测试结果”，但当前实现把算法选择写死，既增加特化嫌疑也妨碍通用收益判断。恢复时应让 radix/桶大小来自已识别源语义或目标缓存/寄存器成本模型，并保留合法回退；不能从比赛输入规模推导。

## 3. 性能与汇编证据

### 3.1 测试口径

测试使用 RISC-V 官方功能与性能目录，而不是相邻 ARM 仓库的 `testcases`：

- 功能：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道功能用例`
- 性能：`/home/orangeisland66/下载/设计文档/2026初赛RISCV赛道性能用例`
- 编译选项：优化测试必须传 `-o`；每个用例运行 3 次并取中位数；以程序输出正确为前提。

本轮 60 个性能运行全部正确：

| 版本 | 60 次运行外部总时间 | 程序内部总时间 | 相对基线 |
|---|---:|---:|---:|
| `master@c7eba48` | 6715.822 ms | 734333.333 us | 基线 |
| 当前 strict 工作区 | 10218.241 ms | 4334500 us | +52.15% |

两版本规范化汇编有 30/60 个变化，集中在 10 个家族；其余 30 个汇编相同，用时变化主要视为 QEMU 测量噪声，不作为优化归因依据。当前工作区静态指令总量曾测得约 18048，基线为 19053；静态指令更少但动态时间更差，说明不能用总指令数替代动态路径和访存成本。

### 3.2 回退家族及最大恢复价值

下表为每个家族 3 个用例中位数之和。变化家族的回退合计约 3612.056 ms；其中 `knapsack_naive` 占当前与基线总差距的 81.64%，是最大恢复计划的第一关键路径。

| 家族 | 基线 ms | 当前 ms | 差值 ms | 旧开关消融归因 |
|---|---:|---:|---:|---|
| `knapsack_naive` | 30.843 | 2890.071 | +2859.228 | `COREIR_RECURSIVE_MEMOIZATION` |
| `many_mat_cal` | 172.484 | 465.035 | +292.551 | `COREIR_ROW_LOCALITY` |
| `transpose` | 29.291 | 209.879 | +180.587 | `AST_DEMAND_DRIVEN_COPY_TRANSFORM` |
| `01_mm` | 68.678 | 153.398 | +84.721 | `AST_CROSS_CALL_ARRAY_SUMMARY` |
| `matmul` | 190.820 | 272.027 | +81.206 | stage fusion / transpose forwarding / row broadcast 交互 |
| `h-1` | 47.282 | 116.169 | +68.887 | accumulator normalize 与 memoization 交互 |
| `03_sort` | 86.832 | 112.030 | +25.198 | `AST_RADIX_SORT_LOWERING` |
| `conv2d` | 39.620 | 53.754 | +14.135 | stage fusion 与 stencil interior 交互 |
| `huffman` | — | — | +6.233 | `HIR_LOOP_STATE_FIXPOINT_BREAK` |
| `crc` | 当前略快 | 当前略快 | -0.690 | `HIR_PURE_CALL_LAST_VALUE` 改变汇编但本轮未形成稳定回退 |

这些数字是恢复排序依据，不会写入编译器触发条件或成本阈值。

### 3.3 代表性汇编差异

| 家族 | 基线汇编特征 | 当前汇编特征 | 动态回退解释 |
|---|---|---|---|
| `knapsack_naive` | 约 136 条；存在 memo 全局、命中/填充路径 | 约 103 条；无 memo，全量递归 | 静态更小但动态递归调用呈组合爆炸；仅看 code size 会得出相反结论 |
| `h-1` | 约 201 条；`__norm_fun` helper 与 memo 状态 | 约 66 条；helper 消失 | 当前每个状态重复递归，静态缩小不能代表总工作量 |
| `many_mat_cal` | 约 424 条；行局部化后按复用方向访问 | 约 339 条；更多 store，局部性变差 | 动态 load/store 次数和 cache 工作集恶化 |
| `transpose` | 约 118 条、19 个分支 | 约 138 条、26 个分支、更多乘法 | 未做需求切片后保留物化/遍历阶段，循环控制工作增加 |
| `01_mm` | 约 220 条，25 load/16 store，frame 96 | 约 436 条，88 load/59 store，frame 96 | 跨调用摘要消失后重复数组工作，代码和动态内存操作同时膨胀 |
| `03_sort` | 约 508 条，frame 64，div/rem 1 | 约 390 条，frame 304，div/rem 5 | 静态缩小但递归/除法/栈流量增加；旧 radix lowering 消除了动态高成本路径 |
| `conv2d` | 约 729 条，49 store，frame 176 | 约 608 条，57 store，frame 224 | stage/producer 合并消失后中间存储和 frame 增长 |
| `crc` | 约 61 条，5 个分支 | 约 114 条，12 个分支 | last-value contraction 消失；本轮总时间较短，先以正确性和稳定测量决定是否恢复 |

## 4. 总体恢复架构

所有大改写统一走以下事务，不允许 pass 发现模式后直接修改原模块：

```text
discover（只读）
  -> prove（通用语义前提）
  -> clone baseline/candidate
  -> transform candidate
  -> CoreIR verifier + refinement checks
  -> baseline/candidate 使用同一 MIR/RA/emitter
  -> RV64 动态感知成本 admission
  -> accepted 才 commit，否则完整回退 baseline
```

当前 `src/core_ir/planning/` 和 `src/backend_mir/rv64_admission.*` 已具备 clone、验证、真实机器流水线比较和拒绝原因框架，应在其上扩展。ARM 目录只参考这种“先证明、再 clone、最后以真实机器产物 admission”的机制；A53 延迟常数、ARM 指令形态和 ARM 测例不得复制为 RV64 规则。

每个计划对象至少携带：

- 触发所使用的 IR 事实及来源；
- 必须成立的 alias/effect/control/dependence/overflow/termination 条件；
- 未知事实的统一拒绝原因；
- baseline 与 candidate 的函数/区域映射；
- 语义验证结果和机器 admission 结果；
- 不包含普通符号名、测试路径或输入采样值的稳定 reason code。

### 4.1 通用优化能力池

回退恢复不能只实现后文列出的旧 pass。每一阶段都应从下列通用能力池选择可由现有分析证明、能覆盖多种程序形态的优化；同一能力至少用两个无关算法正例验证后才算完成。

| 能力层 | 优先建设的通用优化 | 可复用前提 | 预期覆盖面 |
|---|---|---|---|
| SSA/标量 | SCCP、GVN/PRE、copy/constant propagation、instcombine、DCE、partial DCE、公共比较复用 | use-def、dominance、位宽、trap/effect | 所有函数，降低后端压力 |
| 内存 | MemorySSA load forwarding、store-to-load forwarding、DSE、write coalescing、SROA、局部数组 scalar replacement | alias、escape、range、memory version、observer | 矩阵、转置、卷积之外的普通数组代码 |
| 循环规范化 | preheader/latch/exit 规范化、rotation、IV simplify、exit-value simplify、LCSSA 类 live-out 合同 | 自然循环、Phi、dominance、zero-trip | 为全部循环优化提供统一入口 |
| 循环变换 | LICM、unswitch、peeling、unroll、interchange、fusion、distribution、strip-mine/tiling、reduction 优化 | dependence、effect、trip range、remainder、成本模型 | 基本循环、嵌套循环和规约 |
| 跨过程 | 纯度/终止/mod-ref 摘要、成本内联、只读参数传播、返回值/参数属性、跨调用冗余内存消除 | call graph、SCC、完整实参映射、escape | 小函数、helper、数组调用链 |
| 递归 | tail-call elimination、tail-recursion conversion、递归不变量外提、accumulator normalization、selective memoization | SCC、base/step relation、纯度、完整 key | 任意满足关系的递归，不限背包或排序 |
| 区域/观察者 | region DSE、demand slicing、view forwarding、producer-consumer forwarding、last-observable contraction | MemorySSA、write coverage、observer closure、state equivalence | 多阶段数组流水线和临时对象 |
| RV64 MIR | 地址递推/折叠、相邻 load/store 地址复用与调度、rematerialization、copy coalescing、branch layout、shrink wrapping、RA/spill 改进 | psABI、依赖 DAG、pressure、frame、编码范围 | 所有优化代码的最终质量 |

能力池的选择顺序遵循：先补共享分析，再做局部等价优化，再做循环/区域组合，最后做需要强终止或完整状态证明的复杂度优化。不得因为某个公开家族短期收益大就跳过它所依赖的共享合同。

## 5. 分阶段详细实施步骤

### 阶段 0：冻结证据与建立 PDF 门禁

目标：防止再次用错分支、测试集或 `-o` 参数，并让后续每个收益数字可复现。

1. 将 `c7eba48` 编译器和当前编译器分别复制到独立只读构建目录，记录提交、工作区 diff 摘要和二进制 SHA-256。
2. `tests/run_performance.py` 默认要求显式给出 RISC-V 性能目录、编译器、`--opt` 和运行次数；输出每个阶段的 compile/runtime 与总 runtime。
3. 为每个用例保存规范化汇编、输出 hash、静态 opcode 计数、frame 大小、call/branch/load/store/mul/div 数量。
4. 新增 PDF 禁止模式扫描：测试集/源文件名、普通函数名白名单匹配、输入常数或字符串、计时/IO 邻域、直接 `getenv` 代码生成旁路、硬编码输出。
5. 增加“重命名不变性”测试：随机重命名普通函数、参数、局部和全局变量，优化是否触发及生成 CFG 形状必须等价。
6. 增加“输入不可见性”测试：编译器不得读取 `.in` 文件，不得在编译期执行待编译程序，不得按运行环境改变 strict 汇编。

退出条件：功能 140/140、60/60 汇编和输出影响面可生成；对 changed cases 加 control 的一轮串行 B→C 可复现；所有后续提交均能生成同格式差异报告。完整 60 点多轮动态计时只用于计划明确列出的里程碑。

### 阶段 1：先修 RV64 admission，避免再次接受“静态更小、动态更慢”

目标：建立所有恢复优化共用的收益底座。当前静态加权和无法识别 knapsack/h-1 的递归爆炸、many_mat 的局部性损失和 03_sort 的栈/除法代价。

1. 扩展 `RV64CostMetrics`：
   - 每个自然循环的层级、符号 trip count/range、每迭代成本和退出概率类别；
   - 调用图 SCC、递归 call site 数、是否尾递归、是否存在重叠子问题证明；
   - 仿射访问的 stride、同一 cache line/row 的复用距离、中间数组写后读流量；
   - frame、spill/reload、callee-save、动态 call、div/rem 和间接分支成本；
   - code size 仍保留，但不得压过已证明的动态工作增长。
2. 将总成本表达为符号多项式/上界，而不是代入官方输入：比较 baseline 与 candidate 在共同合法参数域上的主项、系数和机器常数。
3. 若两侧复杂度不同，先比较渐近主项；若同阶，再比较循环权重、访存、压力和关键路径。无法比较时拒绝高风险 candidate，而不是猜测接受。
4. 对 memoization 单独记录“miss 与 baseline 等价、hit 节省递归子树”的成本；只有编译期能证明可能出现相同完整 key 时才生成缓存。
5. 对循环变换枚举少量目标驱动候选（例如不展开、不同合法展开/tiling 因子），全部经 clone 和真实 MIR/RA/emitter，选择成本最小者；候选集合来自 RV64 寄存器数、cache line 和指令延迟，不来自用例规模。
6. 增加永久回归：当前 `knapsack`/`h-1` 形态必须把“无 memo 的静态小代码”识别为动态工作更大；`03_sort` 必须计入 frame、div/rem 和递归调用；`many_mat` 必须计入 stride 和内存流量。

退出条件：现有 baseline/candidate 对的方向判断与实测方向一致；任一未知动态因素都有明确 fail-closed reason；成本模型源码中无测试名称或官方输入常数。

### 阶段 2：恢复低风险通用机器级与循环级收益

目标：先回收不需要算法替换的稳定收益，并降低后续候选的机器代码噪声。

1. 通用循环展开改为多候选 planning：以寄存器压力、分支摊销、remainder 和 code growth 选择因子；修复 `01_mm` 类 220 -> 436 的无收益膨胀。
2. 栈帧与 RA：让 frame growth、spill/reload 和 callee-save 成为硬 admission 项；针对 `03_sort` 的 64 -> 304 bytes 建立非测例化回归形态。
3. 通用尾调用/尾递归消除：只检查 CFG、调用位置、ABI 和 live-out，不识别算法；保持参数求值、副作用和全部语言可观察行为，资源/终止语义无法确认时拒绝。
4. 强度削弱和地址递推：把循环内重复乘法/除法变成已证明无溢出或位模式等价的递推；有符号除法必须保持向零舍入。
5. 分支布局和调度：使用自然循环 backedge、dominance 和 RV64 延迟/吞吐，不用函数名或测量输入。
6. 每项分别提交，要求全量正确且汇编变化能由通用成本理由解释；出现新家族显著回退即自动回退该 candidate。

除上述针对已知汇编问题的项目外，本阶段并行建设以下普适优化序列，但每项仍独立提交和验收：

1. **SSA 清理固定点**：SCCP → instcombine → GVN → DCE，确保循环/区域改写产生的冗余能被消除；所有 pass 共用 analysis invalidation 合同。
2. **通用 PRE**：只对无副作用且所有路径可用的表达式做部分冗余消除；可能 trap 的 div/rem/load 不得被提前执行。
3. **MemorySSA 局部优化**：同一 memory version 上的 load forwarding、must-alias store forwarding、被完整覆盖 store 的 DSE；未知调用或 may-alias 立即停止跨越。
4. **SROA 与局部数组标量化**：仅处理不逃逸、常量/可证明有限索引集合的对象，保留初始化、越界和生命周期语义。
5. **循环不变量与 unswitch**：LICM 不越过 trap/effect；unswitch 通过 clone/admission 控制 code growth，并正确合并所有 exit Phi。
6. **IV/exit-value 优化**：统一识别单位与非单位步长、递增/递减、运行时 bound；用位模式证明处理 i32 回绕，不把数学整数假设写进 correctness。
7. **通用 reduction lowering**：整数规约在同一模环保持顺序或证明可结合；浮点默认不重关联。小循环由成本模型拒绝额外 setup。
8. **MIR copy/地址优化**：copy coalescing、常量/全局地址 rematerialization、base+offset 复用、分支 fallthrough 和 shrink wrapping，共同以 spill/frame/critical-path admission 控制。

预期：本阶段不是 3.5 秒缺口的主要来源，但能阻止错误展开、frame 和 spill 吞噬随后恢复的收益。

### 阶段 3：恢复仿射局部性、视图转发和阶段融合

目标：优先恢复 `many_mat_cal`、`matmul`、`conv2d`、`transpose`，理论可回收约 568 ms 以上。

#### 3A. 通用 loop interchange / strip-mine / locality

1. 从自然循环构造 perfect/imperfect loop band，提取规范 IV、界、步长和仿射 access map。
2. 用 `loop_dependence` 计算 RAW/WAR/WAW 距离向量；只在交换/tiling 后仍保持依赖字典序时合法。
3. 用 alias、pointer range 和 ordered effects 排除重叠、volatile、call clobber、未知地址和可观察顺序变化。
4. 对规约显式证明结合/交换性质；SysY i32 位模式语义按模环证明，浮点默认不重排。
5. tile/展开因子只由 RV64 cache line、寄存器预算、元素宽度和符号工作集产生；所有 remainder、零迭代、负界走统一语义路径。
6. 为 baseline 和多个 candidate 完整 lower，按 stride、reuse、frame/spill 和动态循环成本选择。

#### 3B. transpose/view forwarding 与 demand slicing

1. 用 MemorySSA-lite 识别生产区域定义、转置/映射关系和消费 load，不以“transpose”名称作为触发。
2. 建立每个目标元素到源元素的可逆索引映射，并证明边界、整数溢出、支配和路径覆盖。
3. 计算 observer closure：目标数组没有逃逸、未知调用、别名 load、最终输出或后续 clobber 可观察时，才允许旁路/删除物化。
4. 若只有部分消费可证明，先仅转发这些 load，不删除生产循环；完整写覆盖和零外部观察者都证明后才能删除阶段。
5. demand-driven copy 改造成一般 region slicing：从最终观察者反向求需求集合，仅消除其补集中的无观察写；不得匹配“初始化—调用—checksum”固定段数作为充分条件。
6. 保留边界/别名守卫时，守卫只比较运行时语义条件；失败执行原始 baseline，不比较官方输入规模。

#### 3C. producer/consumer 和跨循环融合

1. 只基于 def-use、MemorySSA、循环域和访问函数建立 producer-consumer 图。
2. 证明融合后每个消费读到与原程序相同的最后写，并保持其他 store/call 的顺序。
3. 中间数组若可能被观察，只做合法循环融合而不删除 store；只有 observer closure 为空才做 scalar replacement/bypass。
4. stencil interior 可生成通用 interior + boundary 版本，但 interior 域由下标范围推导；边界始终走等价路径。
5. 使用事务式 admission 防止静态代码变大、压力增加或 cache 行为恶化。

退出条件：重命名正例仍触发；alias、逃逸、额外消费者、额外 store、call effect、非仿射下标、零迭代、remainder、溢出 near-miss 全部拒绝或走正确 guard；四个目标家族均不慢于阶段 2。

### 阶段 4：恢复纯调用 last-value 与循环不动点

目标：恢复 `huffman`、评估 `crc`，同时建立可复用的状态等价证明。

1. `FunctionSemanticSummary` 必须证明函数确定、无外部读写、无 IO、无 volatile、无 UB 前提泄漏；递归函数还需单独处理终止性。
2. last-value contraction 只有在被删除迭代的返回值和所有中间状态均不可观察、调用总是终止且不会 trap 时合法；否则保留每次调用。
3. fixpoint break 比较完整 loop-carried state 和循环可读内存摘要，不能只比较一个标量。
4. 证明从相同完整状态出发的下一步确定且没有额外副作用，才可推导后续状态不变并提前退出。
5. 比较操作本身、额外分支和状态快照需经过 RV64 admission；短循环不应因守卫开销回退。
6. 增加非纯调用、隐藏全局、别名写、不同 NaN/除零/trap、部分状态相等、非终止调用等反例。

退出条件：`huffman` 的早停由完整状态等价触发；`crc` 只有稳定收益为正才接受，不能为了恢复旧汇编强制启用。

### 阶段 5：恢复通用递归规范化与 selective memoization

目标：处理 `knapsack_naive` 和 `h-1`，这是总缺口约 83% 的核心阶段。按 PDF-only 口径，memoization 可以进入 strict，但证明要求必须覆盖所有输入，而非只覆盖性能数据。

#### 5A. 递归 CFG 与 accumulator 规范化

1. 在调用图 SCC 中识别直接/互递归边、base case、参数转换和所有返回合流，不检查普通函数名。
2. 对尾递归先做 ABI 等价循环化；非尾递归不强行改写。
3. accumulator 规范化必须从 SSA 等式推导单步关系，并以归纳证明 helper/迭代版本与原递归在每个深度返回相同位模式。
4. 覆盖所有哨兵返回、负数、零、极值、溢出、额外上下文参数、全局读写和异常退出；未知就拒绝。
5. helper 使用编译器保留前缀并做用户符号碰撞检查，名称只用于 ABI 链接，不参与匹配。

#### 5B. 通用 selective memoization 正确性合同

1. **纯度**：被缓存结果只能依赖完整参数 key 和已证明不变的只读内存版本；存在 IO、volatile、未知调用、可变全局或可逃逸指针即拒绝。
2. **完整 key**：纳入影响返回值、终止和 trap 的全部标量、指针身份/指向内容版本及隐含状态；不能只缓存“看起来变化的两个参数”。
3. **碰撞安全**：hash 只能定位槽位，命中必须逐字段比较完整 key；不得把 hash 相等当作语义相等。
4. **生命周期**：cache 必须按顶层调用 generation 隔离，处理嵌套调用、递归重入、generation wrap 和线程/信号不可观察性；不允许跨不相关调用泄漏旧状态。
5. **终止与 trap**：只在原调用成功返回后填 cache；miss 完整执行原路径。不得缓存“正在计算”值或把非终止/会 trap 的调用变成返回。
6. **资源语义**：缓存空间由编译器通用预算和 key 类型决定；满表时退化为 miss，绝不能覆盖尚需比较的 key 或改变返回值。
7. **适用性**：调用图和参数转移必须证明可能到达相同完整 key；这一证明来自递归关系/符号域，不读取官方输入。无法证明重叠时不生成缓存。
8. **收益**：比较 baseline 递归树上界与 unique-state 上界，再计入 lookup/fill、frame、访存和 code size；只有符号成本更优才 admission。
9. **实现顺序**：先支持纯标量 key、直接递归、单线程、无 pointer/global；再逐项扩展。第一版不支持的情况一律拒绝，不能用假设补洞。

#### 5C. 验证

1. 正例使用随机生成的纯递归关系、不同函数/参数名、不同参数顺序和不同 base case 常数。
2. near-miss 覆盖 hash 碰撞、可变全局、alias 指针、同 key 不同隐藏状态、嵌套顶层调用、generation wrap、非终止分支、trap 和 cache 满。
3. 对小输入穷举 baseline/candidate 输出与终止状态；对大输入做随机差分，但随机测试不能替代静态证明。
4. 对 `knapsack_naive` 只把它作为验收样例：预计恢复约 2.86 秒，但编译器源码、成本模型和测试 matcher 不得出现该名字或其输入规模。

退出条件：证明包通过独立代码复核；strict 正/负夹具和全量功能正确；memo candidate 对目标形态恢复明显且不会让无重叠递归回退。若合同任何一项未完成，memoization 保持 research，而其余阶段照常发布。

### 阶段 6：恢复 cross-call summary、demand copy 与 radix lowering 的剩余上限

目标：在前述通用分析成熟后回收 `01_mm`、`transpose` 和 `03_sort` 的剩余性能，而不是直接复活旧 matcher。

#### 6A. 跨调用区域摘要

1. 内联无关的 `FunctionSemanticSummary` 表示参数区域的 read/write/kill/reduction/return 关系。
2. 从调用点的实际 alias 与调用序列组合摘要；所有实参必须按语义映射，不能凭参数个数猜测函数角色。
3. 用 region state equivalence 证明替代摘要后的最终数组状态和所有中间观察一致。
4. summary 只表达一般仿射映射、规约或覆盖关系；若需要识别固定“三数组矩阵 + checksum”才能成立，说明通用证明仍不完整，应拒绝。
5. 先做跨调用冗余 load/store 消除和局部融合，再考虑删除完整阶段；每扩大一步都增加独立 observer/effect 证明。

#### 6B. 通用 radix/递归排序 lowering

1. 识别依据是递归分区语义、bucket 索引表达式和数组 effect，不是函数名、三数组数量或固定调用场景。
2. 从源程序证明排序键域、符号处理、稳定性要求、元素 payload、边界、容量、递归深度和终止。
3. 若替换为迭代 radix，证明每一 pass 的桶划分与原递归划分产生相同最终序列；若原语义不要求稳定，可用相应 refinement，但不能擅自改变可观察顺序。
4. radix 值优先继承源语义；新算法选择值由 RV64/cache 成本模型产生并对所有合法输入正确，不能取自测试输入特征。
5. scratch 申请、栈/全局生命周期、溢出和失败行为必须与语言/runtime 约束相容；资源不可保证时回退原程序。
6. 第一安全落点是通用尾递归消除、强度削弱和栈优化；完整算法 lowering 只有在端到端 refinement 证明完成后才进入 strict。

退出条件：`01_mm`、`transpose`、`03_sort` 恢复收益；改名/改数组名/改合法 radix/改界仍正确，额外观察者、别名、非稳定 payload、负键、容量不足等反例拒绝。

## 6. 优先级、预期回收与发布档位

| 优先级 | 工作包 | 主要目标 | 基于当前测量的可恢复量 | 合规/实现风险 |
|---:|---|---|---:|---|
| 0 | 测试与 PDF 门禁 | 防止错误基线和违规触发 | 间接 | 低 |
| 1 | 动态感知 admission | 防止静态小而动态慢 | 间接、所有后续共用 | 中 |
| 2 | SSA/MemorySSA/循环规范化/MIR 通用池 | 全部程序 | 未单独计入，支撑所有后续 | 低中 |
| 3 | selective memoization | 所有纯重叠子问题递归；现有回退中以 knapsack、h-1 验收 | 约 2.86 s 以上 | 高 |
| 4 | affine locality / stage fusion | 所有合法嵌套循环；现有回退中以 many_mat、matmul、conv2d 验收 | 约 0.39 s | 中 |
| 5 | demand/observer forwarding | 所有不可观察临时区域；现有回退中以 transpose 验收 | 约 0.18 s 以上 | 中高 |
| 6 | cross-call summary | 所有可组合区域摘要；现有回退中以 01_mm 验收 | 约 0.085 s | 高 |
| 7 | radix/递归分区 lowering | 所有满足证明的递归分区；现有回退中以 03_sort 验收 | 约 0.025 s | 高 |
| 8 | fixpoint/last-value | 所有完整状态不动点/不可观察迭代；以 huffman、crc 验收 | 约 0.006 s，需复测 | 中 |

建议两个发布档位：

- **稳健 strict**：阶段 0–4 和 5A 完成，未通过完整 memo 证明时不纳入 5B。目标总时间不高于约 7.5 s；这是测试目标，不是编译器阈值。
- **最大恢复 strict**：5B 与阶段 6 均通过 PDF 条件和独立证明复核。目标接近 6.7–6.9 s 基线区间，并力争不出现新的家族回退。

若 memoization 未获完整证明，不能为了追求 2.86 秒收益降低通用正确性要求；该情况下应接受稳健档位，而不是重新引入猜测路径。

表中的优先级表示性能价值；实际开发仍按阶段 0–5A 先完成共用分析、成本模型和递归语义底座，再实施 5B memoization，不能绕过其证明前置项。

## 7. 测试矩阵与验收标准

### 7.1 每个 pass 的语义证据包

每个候选 pass 必须新增：

1. 与原样例名称完全不同的至少两个结构正例；
2. 重命名、参数换序和无关 CFG 扰动后仍应触发的等价变体；
3. alias、effect、额外 live-out/observer、边界、零迭代、溢出、trap 和非终止 near-miss；
4. baseline/candidate 的随机差分和小域穷举；
5. 证明前提到代码检查点的一一映射表；
6. 触发、拒绝和机器 admission 的稳定 reason code；
7. strict 环境不变性与符号重命名不变性结果。

### 7.2 日常性能验证：迁移 ARM 的 changed-only 条件

每个独立 pass 提交候选按以下固定顺序验证：

1. strict Release 构建和对应证明单测；共享 IR/CFG/MIR 改动再跑官方功能集 O0 与优化模式 140/140；
2. baseline/candidate 对官方性能集全部 60 个源生成汇编，完成 assemble/link 与规范化影响面扫描；日常不运行汇编未变用例；
3. 规范化汇编后列出 changed cases；若 0/60 changed，不运行动态性能，只报告代码生成未变化；
4. 若存在变化，无论数量多少，日常只串行运行 changed cases，并加入至少一个 byte-identical control；不重复计时其他未变用例；
5. 默认只做一轮 `B -> C`，使用 `tests/run_performance.py --opt --runs 1 --jobs 1 --filter ...`；基线和候选的 runtime、GCC、QEMU、输入、超时、CPU 条件和运行顺序必须相同；
6. 报告每 case 原始 base/candidate `run_ms`、内部时间、改善率、family geomean、异常值、control，以及汇编 opcode/frame/spill/调用/循环差异；单轮不得写成 median；
7. byte-identical control 波动足以解释 changed case 差异时，只能写“动态证据不足”，不得宣称收益；
8. 动态确定性回退先修共享 analysis/cost/RA 能力，不得按 family 增加黑名单或缩窄 gate。

### 7.3 里程碑全量与稳定统计

以下节点由本计划明确授权进行完整 `60/60` 串行动静态验收：阶段 1 成本模型完成、阶段 3 通用循环/区域优化完成、阶段 5 memoization 决定进入 strict 前、阶段 6 最大恢复候选完成。其他日常迭代不得默认启动完整多轮性能。

1. 里程碑先跑官方功能 O0/O1 140/140 和性能输出 60/60；任何错误阻断计时。
2. 完整性能仍使用 `--jobs 1`。第一轮用于总量和影响面；需要发布“稳定改善/回退”结论的 changed cases 才升级为至少 13 对预先安排的交错 B/C 顺序。
3. 稳定统计报告 paired median、family geomean、bootstrap 置信区间、全部原始样本和 byte-identical control，不事后删除不利样本。
4. 当前分支必须对固定 baseline 比较，不允许拿错误 ARM 用例、错误输入或错误分支替代。

### 7.4 合并门槛

- correctness：功能和性能输出零失败；证明反例零误触发；
- PDF 合规：源码触发不包含第三条禁止特征，strict 不受代码生成环境变量影响；
- 通用性：改名、换合法常数/边界、不同输入均保持正确；
- 性能：总时间不回退，目标家族有稳定收益，非目标家族不得出现统计显著回退；
- 机器质量：frame/spill/code growth/关键路径/动态循环成本均在 admission 策略内；
- 可回退性：candidate 拒绝或内部验证失败时生成与 baseline 等价的原路径，不允许留下半改写 IR。

## 8. 推荐提交顺序

1. `perf-evidence-pdf-gates`：冻结基线、测试路径、`-o`、runtime 与禁用特征扫描；
2. `rv64-dynamic-cost-model`：循环/递归/内存局部性/栈成本；
3. `planning-function-scope`：把模块级 clone 缩小为函数/区域事务，降低编译时开销；
4. `generic-ssa-memory-fixedpoint`：SCCP/GVN/PRE/DCE、MemorySSA forwarding/DSE、SROA；
5. `generic-loop-canonicalization`：rotation、IV/exit simplify、LICM、unswitch；
6. `generic-unroll-ra-tailcall`：多候选展开、机器 admission、通用尾调用和 RA/frame；
7. `affine-locality-plan`：interchange/tiling/fusion/distribution/row locality；
8. `observer-view-forwarding-plan`：region DSE、view forwarding、demand slicing；
9. `producer-consumer-fusion-plan`：一般 producer-consumer 图与中间 store 消除；
10. `pure-fixpoint-plan`：last-value 与完整状态不动点；
11. `recursive-normalization-plan`：通用尾递归、递归不变量与 accumulator；
12. `selective-memoization-plan`：先纯标量直接递归，再扩展；
13. `interprocedural-region-summary-plan`：通用 mod/ref 与跨调用区域摘要；
14. `generic-radix-lowering-plan`：最后处理已证明的递归分区 lowering。

每个提交只引入一种证明合同和对应测试；性能证据单独归档。不要把多个高风险旧 pass 一次性恢复，否则无法确认正确性边界和回退来源。

## 9. 明确禁止的捷径

即使能恢复性能，以下实现按 PDF 唯一口径仍不得采用：

- 在编译器中出现 `knapsack`、`matmul`、`huffman` 等测例名或源路径作为触发；
- 仅凭函数名、字符串、参数个数、固定数组维度组合或调用次数猜测测例身份；
- 读取 `.in`、运行待编译程序采样输入、按输入大小/值选择特化；
- 硬编码输出、checksum 或已知中间结果；
- 根据 QEMU、宿主 CPU、比赛机器、计时函数邻域或非 research 环境变量改变 strict 代码生成；
- 以“现有 140+60 全过”替代对所有满足触发条件程序的正确性证明；
- 利用越界、未初始化值、有符号溢出的宿主 C++ 行为、错误别名假设或其他未定义行为；
- 直接 cherry-pick 旧 matcher 并仅增加几个反例，未建立 fail-closed 证明和事务回退；
- 把本计划中的基线时间、用例输入规模或恢复目标写成编译器内部 magic threshold。

## 10. 完成定义

最大恢复计划完成需同时满足：

1. 10 个汇编变化家族均有可复现归因，30 个未变化样例不被误报为代码回退；
2. 递归、局部性、观察者和跨调用成本进入 RV64 admission，静态指令数不再作为唯一收益依据；
3. 每个恢复 pass 只使用程序语义/结构和 RV64 通用硬件事实触发；
4. 每个 pass 的证明、正例、改名变体、near-miss 和全量回归齐全；
5. PDF 第三条禁止模式扫描为零命中，strict 环境不变性通过；
6. 官方功能输出全部正确；日常 changed-only B→C 与 control 无确定性回退，里程碑性能输出 60/60 正确且重点 13 对统计无新增稳定回退；
7. 若全部高收益证明通过，总时间接近基线；若 memoization 等高风险证明未闭环，则保持 research 隔离并发布稳健档位，绝不以性能替代正确性。

结论：按 PDF 作为唯一合规条件，最大性能恢复的空间比旧内部标准下更大。递归 memoization、阶段融合、跨调用摘要、需求驱动复制和 radix lowering 都不是因类别而天然违规；它们可以恢复，但必须从“旧样例骨架 matcher”升级为对所有满足语义前提程序成立的通用变换。实施上的最优顺序是先补动态成本与事务门禁，再建设 SSA、MemorySSA、循环规范化和 RV64 MIR 通用能力，随后在这些共享合同上完成高收益 selective memoization、仿射局部性、观察者驱动消除和跨调用/排序变换。
