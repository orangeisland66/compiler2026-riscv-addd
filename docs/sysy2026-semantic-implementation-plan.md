# SysY2026 语义实现计划

## 1. 目标与边界

本计划以《SysY Language Definition (2022 Edition)》与《SysY Runtime
Library (2022 Edition)》为基础合同，在不移除或收紧任何 SysY2022 合法程序的
前提下，增加《2026 编译技术实验文法定义及相关说明》中的语言能力：

```text
SysY2026 源程序
    -> Lexer / Parser
    -> 类型与作用域分析
    -> 语义无错误的 Typed AST
    -> O0 直接后端 或 O1 CoreIR/MIR 后端
    -> RV64GC 汇编
```

本阶段只处理语言正确性、类型、控制流、对象布局、运行时接口和诊断，不新增
针对特定测试样例的优化。现有 CoreIR 优化、MIR、寄存器分配和栈帧能力作为
后续性能阶段复用，但不得承担本应由语义分析完成的类型判定。

SysY2022 的 `int/float`、多维数组、隐式数值转换、任意 int 型 `main` 返回表达式
和完整运行时接口都是长期基线，不是临时兼容能力。SysY2026 在此基础上增加
`char`、字符串、显式 int/char 转换、`switch`、局部 `static` 和 `printf`。
新增规则不得反向拒绝合法的 SysY2022 程序。

## 2. 当前差距

原始 `src/frontend/` 只定义 `Int/Float/Void`，没有 `char`、字符/字符串字面量、
`static`、`switch/case/default`、显式类型转换和 `printf` 的语法节点。
Parser 采用异常即停策略，不能收集文法要求的多条错误；`src/main.cpp` 也会
在首个异常后直接退出。

项目继续沿用 SysY2022 的无下划线运行时命名，并为字符串补充同风格接口。
文法材料中的下划线名称在本分支适配为 `getint/getch/getstr/putint/putch/putstr`；
因此必须同步更新类型、语义、降低和运行时 ABI，不能把两套名称混用。

## 3. 目标语义模型

### 3.1 源类型

统一的类型描述必须覆盖原有类型和新增类型：

```text
Scalar: int, float, char, void
Array:  int/float 的多维数组以及 char 数组
Function: 返回类型、参数类型和数组参数标记
```

`char` 的源语义为无符号 8 位，取值范围 0..255，运算结果按模 256 回绕。
`int` 保持 32 位有符号语义，`float` 保持 IEEE 754 单精度语义；SysY2022 的
int/float 隐式转换继续有效。未经显式转换时，char 不与 int/float 混合运算、
赋值、传参和返回。

建议在 CoreIR 中优先增加明确的 i8/byte load-store 与 `zext/trunc` 能力；如果
现有 IR 扩展成本过高，可以先采用“源类型 char、IR 中 canonical i32、访问
内存时显式 byte load/store”的过渡方案，但必须将截断和零扩展集中在 lowering
层，不能散落在后端。

### 3.2 作用域与符号

建立独立 `Sema`/`SymbolTable`：

- 文件、函数、块三级作用域
- 变量、常量、函数和 typedef-like 保留空间（为后续扩展预留）
- 当前作用域重复定义检查
- 未定义名字检查
- 函数签名先收集、函数体后检查，支持前后函数调用
- `const` 对象不可写
- 数组长度必须是合法 int 常量表达式

Typed AST 只在语义检查完成后交给任一后端；存在语义错误时禁止进入 HIR、
CoreIR 或汇编生成。

## 4. 分阶段实施

### 阶段 0：冻结合同和夹具

建立 `tests/sysy2026/`，将文法文件中的正确程序和错误程序按能力拆分：

- `c_level/`：int、函数、if、while、基础表达式
- `b_level/`：SysY2022 多维 int/float 数组，以及新增 char、switch
- `a_level/`：`!`、`&&`、`||` 与短路求值，以及整数按位运算
  `&`、`|`、`^`、`~`
- `runtime/`：输入输出、字符串、printf
- `errors/`：a-n 错误类别

每个能力至少准备一个最小正例和一个边界负例，并记录预期汇编是否生成。
先实现测试 harness，再改编译器，避免功能完成后才发现诊断格式不兼容。

### 阶段 1：词法和 AST

扩展 `src/frontend/lexer.h/.cpp`：

- 关键字：`char`、`static`、`switch`、`case`、`default`
- 标记：`:`、字符常量、字符串常量
- 处理转义字符、非法字符、未闭合字符/字符串和行号
- 保留注释和位置记录能力

扩展 AST：

- `CharLiteralExpr`、`StringLiteralExpr`
- `CastExpr`，记录目标类型和源表达式
- `SwitchStmt`、`CaseClause`
- 局部静态声明、char 数组初始化器，并保持原有多维数组初始化器
- `PrintfStmt` 或经过验证的 printf 调用节点

Parser 只负责结构，不在解析阶段偷偷完成 int/char 转换。

### 阶段 2：类型检查和错误恢复

新增 `src/frontend/sema.*` 与诊断收集器，完成：

- 类型等价和显式转换检查
- 运算符两侧类型一致性；按位运算只接受整数标量，并按照 C 的
  `|`、`^`、`&`、相等和关系运算优先级解析
- 赋值、返回值、函数参数和 switch case 类型检查
- printf 格式字符与参数数量、类型检查
- break/continue 所处循环和 switch 上下文检查
- main 必须为 `int main()`；其 int 型返回表达式沿用 SysY2022 语义
- 缺少返回值、重复 case/default、修改 const 等检查

Parser 和 Sema 不再在第一个错误处抛出并终止。采用同步点恢复（分号、右括号、
右花括号、下一个声明关键字），继续收集同一文件中的错误，最后按行号和错误
类别排序输出。语义错误文件不生成任何中间代码。

### 阶段 3：运行时和字符串

统一建立 SysY2026 runtime registry，供 O0 与 O1 共享：

```text
getint()          -> int
getch()           -> int
getfloat()        -> float
getarray(int[])   -> int
getfarray(float[]) -> int
getstr(char[], int) -> void
putint(int)       -> void
putch(int)        -> void
putfloat(float)   -> void
putarray(int, int[]) -> void
putfarray(int, float[]) -> void
putstr(char[])    -> void
printf(format, ...) -> void
```

字符串常量放入只读数据区，完成转义解析、长度计算和 char 数组初始化。`printf`
不作为未知函数直接发射，而是在语义检查后降低为受控的
`putint/putch/putstr` 调用序列；
不支持的格式字符必须在 Sema 阶段报告错误。

运行时符号的具体实现要与当前 RV64GC/medany 链接方式对齐。若课程运行时并非
RV64 版本，必须先确认目标平台，再决定增加兼容 runtime 还是更换后端 ABI。
名称解析始终以用户定义函数为先；只有不存在同名用户函数时，才使用运行时
内建签名或特殊 lowering，避免新增接口改变既有 SysY2022 程序的含义。

### 阶段 4：控制流、static 和数组降低

`switch` 先降低为显式 CFG：每个 case 建立入口块，按源顺序保留 fallthrough，
break 跳到 switch exit；后续再由 MIR 成本模型决定比较链或跳转表，不能在语义
降低阶段假设某种机器实现。

静态局部变量降低为具有内部唯一名称的模块级对象：

- 未初始化时进入 `.bss` 并清零
- 常量初值进入 `.data`
- 函数每次调用复用同一对象
- 名称只用于符号冲突避免，不参与优化判断

原有 int/float 多维数组继续使用行主序布局；新增 char 数组必须保持 0..255
语义。数组参数、聚合初始化和运行时调用不能因新增 char 类型而缩窄原有能力。

### 阶段 5：接入两条后端

Typed AST 是 O0 和 O1 的共同输入：

- O0：Typed AST -> `src/backend/`，先完成基础正确性
- O1：Typed AST -> HIR/CoreIR -> 现有 MIR 后端

先以 O0 作为语义基准，再接入 O1。两条路径对同一输入必须通过统一 reference
output；禁止在 O1 中用优化 pass 补救前端类型或控制流错误。

需要重点验证：char 的零扩展/截断、switch fallthrough、break 嵌套、静态变量
跨调用保持、字符串地址、printf 参数布局、函数返回值和 16 字节栈对齐。

### 阶段 6：验证与发布门槛

每个语义能力完成后运行：

```text
git diff --check
cmake Release 构建
语义 registry / strict audit
O0 定向正负例
O1 定向正负例
RV64GC 静态汇编与链接
QEMU reference output 对比
```

发布前必须完成：

1. C/B/A 三个等级的正确程序全部通过
2. a-n 错误样例输出行号和类别正确
3. 所有错误程序均不产生 CoreIR/MIR/汇编
4. O0 与 O1 对同一输入输出一致
5. 运行时、GCC、QEMU、输入和超时固定且可复核
6. 现有 SysY2022 回归若继续保留，必须通过独立 profile 运行，不与 SysY2026
   语义混用

## 5. 推荐里程碑

| 里程碑 | 完成内容 | 验收标准 |
|---|---|---|
| M0 | 合同、目录、诊断格式和测试 harness | 能运行最小正负例 |
| M1 | Lexer、Parser、Typed AST | C 级正确程序可建树 |
| M2 | Sema、作用域、错误恢复 | a-n 诊断样例通过 |
| M3 | char、cast、数组、字符串 | O0 输出与 reference 一致 |
| M4 | static、switch、printf、runtime | B 级和运行时样例通过 |
| M5 | O1 CoreIR/MIR 接入 | A 级短路与 ABI 样例通过 |
| M6 | 全量回归与性能基线 | 语义、链接、QEMU 门禁全部通过 |

## 6. 实施原则

- 先做类型和诊断，再做优化适配
- 语义规则集中在 Sema，不复制到两个后端
- 先保证 O0 正确，再接入 O1
- 所有 CFG 变换先完成合法性证明，再一次性修改
- runtime 名称只作为 ABI 合同，不作为优化触发条件
- 每个新增语义都配正例、近似负例和端到端输出证据

完成本计划后，现有优化基础设施仍可继续使用，但编译器的语言边界、错误
行为和运行时接口将以 SysY2026 为唯一明确合同。

## 7. 当前实施状态

已完成并通过定向 O0/O1、RV64GC 静态链接和 QEMU 验证：

- `char` 标量、字符常量、显式 `(int)/(char)` 转换和模 256 结果归一化
- `switch/case/default`、fallthrough、嵌套 break 目标
- 局部 `static` 标量和数组对象的持久化存储
- 字符串初始化、`printf` 的 `%d/%c/%s` 降低
- `getstr` 的逐字符读取、换行终止和 `max_len <= 0` 行为
- O0/O1 共用 Sema、正例/负例夹具和错误码门禁

仍需继续完成：

- Parser 其余语法分支的完整同步恢复；`a、i、j、k` 已能统一收集并阻止代码生成
- 更完整的常量表达式求值和所有初始化边界
- char 数组的独立 byte IR/内存模型，当前过渡实现使用规范化 word 布局
- 完整 RV64 ABI 边界、函数参数数组和外部 runtime 的正式链接测试
- 全量课程功能集与错误程序回归
