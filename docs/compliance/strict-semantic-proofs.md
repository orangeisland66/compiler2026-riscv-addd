# Strict 优化语义证明与隔离合同

本文档定义 `SYC_ENABLE_RESEARCH_PASSES=OFF` 的可接受变换集。证明不使用源文件名、普通符号名、输入值、IO 邻域或公开测例骨架。实现和反例是证明合同的一部分；实现弱化任一前提时，变换必须重新进入 research 隔离。

## 编译期隔离

`radix_sort_lowering`、`demand_driven_copy_transform`、
`cross_call_array_summary` 和 `producer_map` 仍属于 research-only；
`transpose_view_forwarding` 的实现仅在 research source list 保留，但当前流水线未
注册，状态为 DORMANT。strict CMake 对象列表不包含这些实现。
`stage_fusion.cpp` 是混合职责 TU：map/reduce/checksum mutation 由
`SYC_ENABLE_RESEARCH_PASSES` 编译边界排除，strict 只保留下述已闭环的
row-broadcast producer-rowmin 变换。`row_locality`、
`recursive_accumulator_normalize` 和 `recursion_memoization` 也按各自证明保留。
research 构建仍需同时设置 `SYC_RESEARCH=1` 和对应的 `SYC_ENABLE_*` 开关才能
执行已注册的实验变换。

这个选择保留了源码和原有反例，但不把“反例暂未命中”当作默认合法性证明。

## Row-broadcast producer-rowmin

`applyRowBroadcastTransposeChecksumFusion` 只从自然循环、规范 IV、SSA use-def、
direct-global object identity、仿射下标和 transpose-copy 事实建立候选。基础路径
要求逐行最小值、完整广播、完整转置取负和未加权 checksum 的 observer/effect/
live-out 闭包；对称 producer 路径还要求同一 source 的 `row,k`、`col,k`、
`k,row`、`k,col` 四种访问角色，其中缺失角色只能由已证明的完整 transpose copy
关系组合得到。未知 alias、额外 store/call、部分覆盖、加权 checksum 或中间矩阵
观察者均拒绝。

物化在 planning clone 内完成，被旁路自然循环、旧 outer latch 和所有相关 Phi
predecessor 作为同一 CFG mutation 删除；CoreIR verifier 和真实 RV64
post-RA admission 全部通过后才提交。正例、weighted near-miss、strict 环境不变性
及正式 matmul 对称 producer 证据见
`tests/tools/test_riscv_remaining_pass_semantics.sh` 和
`tests/riscv/evidence/20260814/matmul-recovery/README.md`。

## Row locality 与 K-block

`applyInPlaceRowLocality` 只接受三层自然循环及专用 preheader/latch/exit。地址必须归一化为同一二维根对象的 `row * rowBytes + col * 4` 仿射式，源矩阵只读，目标矩阵是唯一写入根；完整 loop region 禁止 call、alloca、未知指针和向量内存，所有 SSA 值必须闭合在循环内。匹配器还检查行跨度、scratch 上限（16 KiB）、边界/后缀行和所有 live-out。物化后的 pre/self/post 三段按原 `k` 顺序计算，边界行走原路径，故每个元素贡献一次且规约顺序不变。K-block 只在同一计划已通过这些事实后按固定寄存器预算分块，remainder 使用半开区间；预算影响成本，不影响合法性。

`tests/tools/test_riscv_loop_semantic_optimizations.sh` 覆盖正例和变量改名，`tests/tools/test_riscv_remaining_pass_semantics.sh` 覆盖外层 IV 依赖、非仿射地址和 verifier/副作用拒绝；两套测试均比较 default/skip 输出。

## Recursive accumulator

`normalizeRecursiveAccumulators` 仅识别直接尾递归的固定字面量增量。累加器必须在递归前只通过 `acc + delta`（或等价交换形式）更新，不能被读取、写入地址、传入调用或参与含副作用表达式；终止分支必须提供非负固定 sentinel，递归参数步进方向和比较关系必须一致。生成 helper 使用模块 fresh-name 分配，保留全部原参数（包括数组/上下文），并把 sentinel、零迭代和溢出行为留在原 i32 运算语义中。任何非字面量增量、额外 live-out、多个递归边或副作用都 fail-closed。

`tests/tools/test_riscv_compliance_risks.sh` 的 `recursive_accumulator_extra_params_positive`、name-collision、zero/sentinel 输入和 `recursive_accumulator_negative` 构成与名称、上下文和输入无关的正负证据。

## Recursive memoization

缓存只对递归函数纯度或完整稳定状态摘要成立时启用：递归调用唯一、参数和返回值为标量，记忆化切片无 store/call/IO/trap，持久状态仅来自分析确认的稳定标量全局。dense 布局必须有每个参数的闭合 `[lower, lower+extent)` 范围守卫；否则使用 hash fallback，并逐参数比较存储 key，不能把 hash 当作等价证明。持久状态同时保存 hash 和每个稳定全局的值，命中必须逐项相等。每个调用轮次由 epoch/depth/stamp 隔离；epoch 回绕清空 stamp 表，避免旧值复用，缺失描述符或状态事实时保留原调用。

可执行反例覆盖可变数组、状态 hash 碰撞 `(0,1)`/`(1,64)`、多次外部调用和 epoch 回绕；`tests/tools/test_riscv_compliance_risks.sh` 对 default/skip 汇编及运行输出逐项比较。

## 环境无关性

`sycEnvironmentValue` 是唯一的运行环境读取入口。TRACE/DUMP/VERIFY/PROFILE/REPORT/LOG/VERBOSE 变量只能增加观测；其他变量在 strict 构建中恒等于未设置。research 构建中的代码生成开关需要进程级 `SYC_RESEARCH=1` 总开关。因此任意 `SYC_SKIP_*`、`MIR_NO_*` 或 `MIR_SIMPLE_RA` 不能改变 strict 产物。

## 通用 strip-mining / loop blocking

变换输入是自然循环、规范归纳变量、LoopBand/affine-region 依赖事实和目标成本提示。`loopCompatibleForStripMine` 要求单回边、专用 preheader/latch/exit、单位正向步长和闭合的 header PHI；不可表示的循环不变换。

对原迭代区间 `[init,bound)`，外层产生 `t=init,init+F,...`，内层边界是 `min(t+F,bound)`。每个原索引恰好属于一个半开 tile，tile 的连接顺序与原索引顺序相同，所以不重排任意两个动态迭代。原循环体、内部分支和访存指令不被改写，因而副作用顺序、整数溢出和 live-out PHI 与原程序一致。`min` 路径覆盖 remainder，零迭代直接走原 exit。tile factor 只来自成本/工作集计划，不参与正确性判定。

可执行证据：`tests/tools/test_riscv_loop_semantic_optimizations.sh` 的正例、remainder、多出口、非规则下标、依赖和副作用 near-miss。

## 标量递推

### 模平移递推

只接受正模数、单一闭合 i32 状态和 `s' = srem(s+c,m)`。第一次迭代单独按原式求值，从而保留 C/SysY 有符号余数的代表元；剩余次数用 i64 形成不溢出的平移量再取模。非正 trip count 、越界或守卫失败走未改写的原 CFG。额外 live-out、call/store 副作用、负模数或非平移更新均拒绝。

### 重复规约

只接受完整 def-use 切片证明的 `F(acc)=acc+delta` 平移。对执行次数 `n`，归纳得 `F^n(acc)=acc+n*delta (mod 2^32)`。乘法、地址、分支或其他状态依赖无法满足平移检查；任意额外 effect/live-out 使计划失败。

### 小型线性递推

状态更新必须是 i32 环 `Z/(2^32 Z)` 上的闭合线性映射 `s'=A s`。特征多项式/矩阵幂计算仍在同一环上，所以包括有符号溢出在内的每个 i32 位模式一致。零迭代使用单位矩阵。非线性项、未闭合 PHI、记忆体、call、trap 可能性或额外 live-out 拒绝。

可执行证据分别在 `tests/tools/test_riscv_remaining_pass_semantics.sh`、`tests/tools/test_riscv_loop_semantic_optimizations.sh` 和 `tests/tools/test_riscv_default_recognizers.sh`。

## 位模式与内部 modmul helper

局部位变换只接受完整 SSA 表达式和已证明的 i32 位宽/移位范围；未知移位、有符号除法舍入不同、effect 或额外 use 拒绝。这是 A7 局部恒等式，不删除一个动态规模的计算阶段。

`__syc_modmul_i32` 的调用只能由 `addModMulFastPath` 在递归减半、唯一自调用、统一正模数、闭合纯度和有符号范围守卫全部成立后物化。模块中存在同名用户定义时，`applyRecursiveModMulFastPath` 在 mutation 前拒绝；`internal_helper_name_collision_negative.sy` 是永久反例。fallback 名称只标记编译器在同一个已接受的 transaction 中生成的 clone，不会使任何未证明的用户函数获得快路径。

可执行证据：`tests/tools/test_riscv_default_recognizers.sh` 的改名正例、倒数边界、副作用负例和同名冲突负例。
