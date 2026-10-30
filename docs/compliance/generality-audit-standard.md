# RISC-V 默认优化泛化性审计标准

> **来源**：工作区通用审计规范 `governance/architecture/generality-audit-prompt.md`
> **作用域**：本仓库 `src/`、`CMakeLists.txt`、默认 `-O1` 流水线及其测试证据
> **用途**：审 `-O1` 默认管道与所有 strict build 编译进来的 pass 是否符合"通用编译器原则"——优化必须由**与具体输入程序无关**的 IR/类型/SSA/use-def/CFG/支配/循环/别名/effect/ABI/成本模型等通用事实证明，而非由文件名、函数名、变量名、固定输入值、固定 IO/计时邻域、公开 benchmark 骨架或硬编码常量所触发。
> **证据位置**：pass 语义登记在 `tests/riscv/default_pass_semantics.tsv`，正负夹具位于 `tests/riscv/`，可复核产物位于 `tests/riscv/evidence/`。

---

## 0. 审计员身份

你是一名**通用编译器架构审计员**，正在审查某 SysY2022 编译器（C++ 实现）的 `-O1` 默认优化管道。审计准则只有一条：

> 优化的触发与形态必须由通用编译器事实集合（A 列）驱动；任何使用具体输入特征决定 transform 是否触发或如何改写的代码，都是违规。

你不读作者自述的"这是泛化的"注释——只看实际控制流是否真的被通用事实驱动。一处即报一处。性能改善不构成豁免。已 audit 的结论不是免审理由。

---

## 1. 判定框架（四个维度，互不混淆）

### A. 合法触发（白名单）

仅由下列**与具体输入无关**的通用事实驱动触发判定，**默认 -O1 允许且无需特别证明**：

| 代码 | 含义 |
|---|---|
| A1 | AST/类型信息（类型、形参数量、数组形状由类型而非名字决定） |
| A2 | SSA/use-def、支配关系、phi 网 |
| A3 | CFG 结构（block、edge、回边、不可达、支配树） |
| A4 | 自然循环结构、归纳变量、循环不变量、循 trip count 上界 |
| A5 | effect/alias/escape/modref 分析 |
| A6 | 目标 ABI（调用约定、寄存器类、栈槽）与目标成本模型（指令代价、寄存器压力、字宽） |
| A7 | 通用代数恒等式（强度削减、常量折叠、CSE、dead-code） |
| A8 | 名称仅用于语言/runtime ABI 协议：`main`、SysY 运行时库函数、`memset`、编译器内部生成（以 `__syc_` 前缀声明的）的 helper 名——但**不能**根据这些名字的存在与否来决定优化是否触发，只能用它们做正确的 ABI 调用 |

> A 类的判据是"事实驱动"，不是"语义驱动"。一个变换即便语义等价，只要它的**触发条件**用了输入特征就是违规。

### B. 非法触发（黑名单）—— 默认 -O1 出现即 Block，必须从 strict build 移走

| 代码 | 触发因子 | grep 关键字示例 |
|---|---|---|
| **B1** | 源文件名 / 路径 / `.sy` 名 / 目录 | `__FILE__`、`source.filename()`、`endsWith(".sy")`、`contains("performance")` |
| **B2** | 普通用户函数名 / 符号名 / 全局名 | `getName() == "..."`、`startsWith("__syc_modmul")`、`strncmp(name,"fib",3)`、`__syc_modmul_i32`、`"program.sy"` |
| **B3** | 普通变量名 / 形参名 / 局部名 | `ParmVarDecl->getName()=="n"`、`var.getName()=="arr"`、`argName=="modulus"` |
| **B4** | 固定 AST 节点计数 / block 数 / 语句数 | `"exactly 3"`、`== 8 blocks`、`four return`、`three decisions`、`requiresExactly N` |
| **B5** | 固定魔数常量比对 | `== 1000003`、`== 998244353`、`== 1000000007`、`== 1024`、`modulus==1000003`、`kModulus=1000003` |
| **B6** | 固定输入值 / 输入规模触发 | `getint() >= 128`、`n >= 2300 fast`、`guard.*(0..N)` 范围字面常量 |
| **B7** | IO 邻域 / 计时邻域触发 | `countPutint`、`hasStarttime`、`"TOTAL: "` 字面量、对 `putint`/`getint` 调用次数做模式匹配 |
| **B8** | 公开 benchmark 骨架识别 | 已 audit 的禁用 pass 名 `stage_fusion`、`producer_map`、`row_locality`、`linear_recurrence_fusion`、`reflective_max_simplify`、`recursive_accumulator_normalize`、`recursion_memoization`、`transpose_view_forwarding`、`modmul` 等；以及多级 map/reduce/checksum/broadcast/transpose 骨架匹配 |
| **B9** | 改算法复杂度的识别类 | `replaceFunction`、`materializeHelper`、`bypassCompute`、`skipOriginalCompute`、`hardcodedReturnValue`、`memoize`、`fibonacciTable` |
| **B10** | 硬编码汇编 metadata | `.file "program.sy"`、`".file "` 后跟字符串常量而非真实输入路径 |
| **B11** | 运行时 env 悄切非默认优化 | `getenv(`、`std::getenv`、`SYC_ENABLE_*`（除 RESEARCH mode 总开关）出现在 strict 默认路径 |
| **B12** | `-o` 输出路径 / 文件名 prefix 字符串触发 | `outputPath.contains("performance")`、`outFile.endsWith(".sy")` 决定优化 |
| **B13** | 计时函数特殊化（仅对 starttime/stoptime 打 inline 关键字等） | `isStarttime`、`isStoptime`、`always_inline.*Starttime`、`_sysy_timer_start/stop` |
| **B14** | 源码注释 / 字面量字符串触发 | 识别 `//FFT`、`// matrix mult`、`// benchmark` 后改行为；源码内嵌源注释字面量被解析 |
| **B15** | 行号 / 列号 / `SourceLocation` 触发 | `loc.getLine() >= N`、`column == N` 决定优化 |
| **B16** | 函数体行数 / 循环体行数硬阈值（魔数而非成本模型） | `func.lines() > 50`、`loop.bodySize >= 30` 用整数常量、而不是 `CostModel::shouldUnroll` |
| **B17** | 特定数组维度硬编码触发 | `== 1024 && dim == 1024`、`arraysize == 2300` 字面量 |
| **B18** | AST/源 hash / IR dump cache 按源内容命中 | 用源码字符串 hash 作 pass 输入（不是 IR 表征 hash）；`sourceHash(...)` 决定是否跳过 |
| **B19** | 检测审计 / 调试运行而切换路径 | `if (audit_mode)`、`getenv("COMPLIANCE_AUDIT")` 切回合规路径；`#ifdef AUDIT_BUILD` |
| **B20** | 下游产物名 / 输出文件 basename 触发 | `outBasename.contains("fft")`、`outputName.startsWith("01_mm")` |
| **B21** | 异常 / UB 兜底硬编码正确结果 | catch 失败后 `return hardcodedOk`、`return 0` 兜底 |
| **B22** | 宿主环境检测做 RISC-V/ARM 优化 | `#ifdef __x86_64__` 给 RISC-V 优化开挂、识别 QEMU/TCG 特征字符串改 lowering（与合法的 ABI 选择区分） |
| **B23** | 依赖未定义/未指明行为触发 | 依赖具体 alignment padding、`int` 字节序硬编码做折叠、依赖 local 存活优化而无声改写 |
| **B24** | 时序 / 并发副作用 hack | 生成代码中插 `fence`/barrier 仅为"压 timing 基线"而非语义需要 |
| **B25** | 优化等级倒挂 | `-O1` 才出现、`-O0` 反而没有的"针对骨架识别"——`#if OPT_LEVEL==1` enable modmul matcher |

### C. 结构性违规（不靠触发判定，而是行为本身违规）

| 代码 | 描述 |
|---|---|
| **C1** | 改算法复杂度而非局部等价改写（即便由通用 fact 触发也不允许 by default，例如把 O(n²) 改成 O(n log n) 必须有 amplitude proof） |
| **C2** | 缺少通用证明就改默认 -O1（包括缺负例，缺等价性、缺支配/别名/effect 跟随分析） |
| **C3** | pass 文件被 strict build 编译但不挂默认管道——只要在 src/ 里就 report（属于"quarantine"违规的扩展） |
| **C4** | runtime env 开关未走 RESEARCH mode 总开关、单独 env 悄切 |

### D. 历史 audit 追溯附录（仅背景，不作判定依据）

D 仅列**已确认曾被 audit 的历史 pass** 供审计员优先核对是否回归。D 类命中本身不算新违规，B/C 维度才是判定依据：

- `stage_fusion.cpp`、`producer_map.cpp`、`row_locality.cpp`、`reflective_max_simplify.cpp`、`linear_recurrence_fusion.cpp`、`recursive_accumulator_normalize.cpp`、`recursion_memoization.cpp`、`transpose_view_forwarding.cpp`
- `bit_patterns.cpp` 内 modmul 识别、`scalar_recurrence.cpp` 多个 mode、`__syc_modmul_i32` builtin-like 处理、`.file "program.sy"` 硬编码

---

## 2. RISC-V 扫描范围

| 扫描对象 | 路径 |
|---|---|
| 编译器实现 | `src/**/*.{cpp,h}` |
| 构建与研究隔离 | `CMakeLists.txt`、`src/research_switch.h` |
| 默认入口与证据 | `src/main.cpp`、`src/core_ir/passes/`、`tests/riscv/`、`tests/tools/` |

CMake / 脚本里也要确认：strict build 是否真的把 D 列历史 pass 摘掉；`SYC_ENABLE_RESEARCH_PASSES=OFF` 默认下哪些 pass 仍编译/挂入默认 -O1。

---

## 3. 具体 grep 命令模板

```bash
# 在本仓库根目录执行
SRC=src

# B1 文件名/路径/.sy 名
rg -n '__FILE__|source\.filename|\.filename\(\)|endsWith\("\.sy"\)|contains\("(performance|functional)"\)' $SRC
rg -n '"01_mm|"h-[0-9]-[0-9]+|"crypto|"fft|"huffman|"knapsack|"matmul|"transpose|"shuffle|"conv2d|"crc|"sl[0-9]' $SRC

# B2 函数名/符号名/硬编码 metadata 字面量
rg -n 'getName\(\)\s*==\s*"|strncmp\(.+,\s*"|startsWith\("__syc|__syc_modmul|"program\.sy"|matchHelperName|isHelperFn|TreatAsBuiltin' $SRC

# B3 变量名/形参名比对
rg -n 'ParmVarDecl.*getName|VarDecl.*getName.*==|param->getName\(\)\s*==\s*"|argName\s*==\s*"' $SRC

# B4 AST count / block count 硬配
rg -n -i 'exactly\s+[0-9]+|n == [0-9]+\s*blocks?|[0-9]+\s*blocks?\s*&&|four return|three (decision|return)|requiresExactly' $SRC

# B5 魔数常量
rg -n '==\s?1000003|==\s?998244353|==\s?1000000007|modulus.*==\s?[0-9]{5,}|kModulus.*=\s?[0-9]{5,}' $SRC

# B6 输入值/规模
rg -n 'getint\(\)\s*[<>]=|n\s*[<>]=\s*[0-9]{2,}.*fast|guard.*[<>]=\s*[0-9]{3,}' $SRC

# B7 IO/计时邻域
rg -n 'countPutint|hasStarttime|hasStoptime|"TOTAL:\s"|"TOTAL: "' $SRC

# B8 历史 audit pass 名 + 骨架识别
rg -n 'stage_fusion|producer_map|row_locality|linear_recurrence_fusion|reflective_max_simplify|recursive_accumulator_normalize|recursion_memoization|transpose_view_forwarding' $SRC

# B9 改算法复杂度
rg -n 'replaceFunction|materializeHelper|bypassCompute|skipOriginalCompute|hardcodedReturnValue|memoize|fibonacciTable' $SRC

# B10 硬编码 .file metadata
rg -n '\.file\s+"program\.sy"|"\.file"|\.file\s+".+\.sy"' $SRC

# B11 运行时 env 悄切
rg -n 'getenv\(|std::getenv|SYC_ENABLE_' $SRC
rg -n 'getenv\(.+\).+SYC_(?!RESEARCH)' $SRC

# B12 输出路径触发
rg -n 'outputPath.*contains|outFile.*contains\("(performance|tmp|functional)' $SRC

# B13 计时函数特殊化
rg -n 'isStarttime|isStoptime|_sysy_timer_start|_sysy_timer_stop' $SRC
rg -n 'always_inline|AlwaysInline' $SRC | rg -i 'start|stop|timer'

# B14 源注释/字面量识别
rg -n '//\s*(FFT|matrix|benchmark|fib|stencil|modmul)|/\*\s*(FAST PATH|BENCHMARK)' $SRC

# B15 行号/列号触发
rg -n 'getLine\(\)\s*[<>=]|getColumn\(\)\s*[<>=]|SourceLocation.*==\s*\d' $SRC

# B16 函数体行数/循环体大小硬阈值（魔数）
rg -n 'functionSize|funcLines|loopBodySize|bodySize\s*[<>=]+\s*[0-9]+' $SRC

# B17 数组维度硬编码
rg -n 'dim\s*==\s*1024|arraySize\s*==\s*[0-9]{3,}|N\s*==\s*1024' $SRC

# B18 源 hash cache
rg -n 'sourceHash|srcHashCache|hashAST|computeSourceFingerprint' $SRC

# B19 审计/调试切换
rg -n 'AUDIT_MODE|COMPLIANCE_AUDIT|audit_mode|debugBypass|#ifdef\s+AUDIT' $SRC

# B20 下游产物名触发
rg -n 'outBasename|outputName.*contains|startsWith\("01_mm|startsWith\("crypto' $SRC

# B21 异常/UB 兜底
rg -n 'catch.*return\s+(0|42|42 \| 0)|return.*"OK".*//.*fallback|fallbackReturn' $SRC

# B22 宿主环境检测做目标架构优化
rg -n '#ifdef\s+__x86_64__|#ifdef\s+_WIN32|qemu|tcg.*lower' $SRC

# B23 依赖未指明行为
rg -n 'assumeAligned\([0-9]+\)|__builtin_assume_aligned|pragma\s+pack' $SRC

# B24 时序 hack
rg -n 'fence|__sync_synchronize|asm.*volatile.*mfence' $SRC | rg -iv 'memory|atomic|order' | head

# B25 优化等级倒挂（O1-only 的特殊识别）
rg -n '#if\s+OPT_LEVEL|optLevel\s*==\s*1|O1.*enable|enable.*O1' $SRC

# C1 算法复杂度替换
rg -n 'replacesAlgorithm|swapAlgorithm|hardcodedCompute|closedFormReplace' $SRC

# C3 strict build 仍编译 D 类 pass
rg -n 'stage_fusion|producer_map|row_locality' CMakeLists.txt $SRC
```

---

## 4. 输出格式

每条命中按这个格式输出（**只登记，不直接出整改方案**）：

```
[B<P/C>-<severity>] file:line
类型: B<P/C><n>  触发因子: <一句话>
匹配片段:
  <最多 3 行代码>
为何违规: <引用 A.x/B.n 规则>
建议处置简写: Block-quarantine / Proof-required / Allowed-with-justification
```

`<severity>`：
- `Block` 默认 -O1 出现即违规
- `Proof-required` 默认 -O1 不允许除非补通用证明 + 负例
- `Allowed` 命中规则但只是字面合法通用代码

## 5. 汇总表与最终展开

扫描完成后输出这张汇总表：

| 严重度 | 命中数 | 主要文件（前 5） |
|---|---:|---|
| Block | ? | |
| Proof-required | ? | |
| Allowed | ? | |

随后对**最严重的 5 处逐条展开**：
- 违规位置 (`file:line`)
- 违规机制描述（用了哪个 B/C 的触发因子）
- 可恢复的等价通用化思路（用 A 类中何种通用 fact 代替命名/常量/骨架识别）

## 6. 归档位置

完成审计后：
- 更新 `tests/riscv/default_pass_semantics.tsv`，不要删除已有 pass 的历史证据入口。
- 正负夹具放入 `tests/riscv/`，门禁脚本放入 `tests/tools/`。
- 汇编、QEMU、哈希和汇总结果归档到 `tests/riscv/evidence/YYYYMMDD/`；不提交 build、ELF 和运行时库。

## 7. 验收门槛

- Block 类命中数 = 0
- Proof-required 类全部有 `tests/riscv/default_pass_semantics.tsv` 对应条目和负例脚本
- 所有新增默认 -O1 优化提交前过这份 prompt
- 任一不满足 → 不允许进入默认 `-O1`、不允许进 strict build

## 8. 审计准则约束

1. **不读作者自述注释**判断合规性，只看实际控制流。
2. **一次即报**，即便仅在 RESEARCH mode 启用——`src/` 里存在该 pass 也要记录并标明隔离状态。
3. **性能再好也不豁免**——按骨架触发就违规。
4. **不预信已 audit 结论**——独立审。
5. **必须给精确 `file:line`**，不要泛泛"某处"。
6. **目标事实必须可解释**——架构相关条件只允许来自 RISC-V ABI、指令语义和明确的成本模型（A6）。
