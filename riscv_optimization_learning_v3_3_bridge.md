# RISC-V 优化学习 v3.2 —— 从“知道优化名字”到“看懂我们为什么这样实现”

> 这份文档不是再加一批优化名词，而是专门补 v3.1 暴露出来的缺口：
>
> - 缩写/名词对应不牢；
> - 不知道一个优化发生在哪一层；
> - IR / operand / CFG 仍然模糊；
> - mem2reg 讲得太快；
> - “不是越多越好”到底怎么判断；
> - alias / effect 到底是什么；
> - 不知道源码该怎么看；
> - 不知道经典优化和我们项目的真实关系；
> - 不知道比赛差异到底在哪里；
> - 不知道老师最可能问什么。
>
> 当前项目事实基线：
>
> `master-new @ 462a17835dfddb9afd0d452c0480ef38a96f4ea9`
>
> strict Release build，research passes OFF，200/200 correctness。

---

# 0. 先处理一个词：OVMLR

目前在我们使用的 `v2` 和 `v3.1` 学习文档中，没有出现 `OVMLR`。

它也不是一个我能可靠确认的常见编译器标准缩写。

所以现在**不要猜着背**。

以后如果再次看到它，请把“它出现的完整一行”发出来，再确认它是：

- 某个项目自定义缩写；
- 某个变量/类名；
- 还是看错/打错了其他词。

学习编译器时一个很重要的习惯就是：

> **缩写不知道就先定位上下文，不要靠字母猜含义。**

---

# 1. 第一张必须真正记住的图：优化到底发生在哪一层

一个常见误区是：

> “Constant Folding、CSE、Unroll 都叫优化，所以它们应该都在同一个地方。”

不是。

优化可以发生在不同层。

我们当前项目可以先粗略记成：

```text
SysY
 ↓
Lexer / Parser
 ↓
AST
 ↓
AST / HIR 变换
 ↓
CoreIR
 ↓
CoreIR 中端优化
 ↓
MIR
 ↓
机器级优化
 ↓
RISC-V Assembly
```

## 1.1 我们项目中的典型对应

| 优化/工作 | 主要阶段 | 当前项目典型位置 |
|---|---|---|
| AST 简化、部分循环规范化 | AST/HIR | `src/hir/` |
| Loop Unroll | HIR | `src/hir/loop_unroll.cpp` |
| Constant Folding | CoreIR | `src/core_ir/passes/const_fold.cpp` |
| DCE | CoreIR cleanup | `src/core_ir/passes/cleanup.cpp` |
| mem2reg | CoreIR | `src/core_ir/passes/mem2reg.cpp` |
| CFG Simplification | CoreIR | `src/core_ir/passes/simplify_cfg.cpp` |
| CSE | CoreIR | `src/core_ir/passes/cse.cpp` |
| LICM | CoreIR / Loop | `src/core_ir/passes/loop_basic.cpp` |
| Inlining | CoreIR / 过程间 | `src/core_ir/passes/inline_passes.cpp` |
| DSE | CoreIR / Memory | `src/core_ir/passes/dead_store_elim.cpp` |
| CoreIR → MIR | Lowering | `src/backend_mir/lower.cpp` |
| Liveness | MIR | `src/backend_mir/mir_liveness.cpp` |
| Linear Scan RA | MIR | `src/backend_mir/linear_scan.cpp` |
| Scheduling | MIR / Machine | `src/backend_mir/schedule.cpp` |
| Peephole | MIR / Machine | `src/backend_mir/peephole.cpp` |
| Frame Lowering | MIR / Machine | `src/backend_mir/prologue.cpp` |

注意：

> **一个“优化思想”并不天然绑定某一层。**

例如常量折叠在很多编译器里可能 AST 做一次、IR 又做一次。

这里说的是：

> **我们这个项目当前主要在哪实现。**

---

# 2. IR、Instruction、Operand、Value、CFG 到底是什么

这几个词如果混在一起，后面所有优化都会变得很难理解。

先看一条假的教学 IR：

```text
%3 = add %1, 5
```

把它拆开。

## 2.1 IR

IR = Intermediate Representation。

中文：

> 中间表示。

它不是某一条指令。

它是：

> **编译器内部用来表示程序的一整套语言/数据结构。**

例如：

```text
%1 = load ...
%2 = mul ...
%3 = add ...
br ...
ret ...
```

这些整体构成 IR。

---

## 2.2 Instruction

Instruction = 指令。

例如：

```text
%3 = add %1, 5
```

这一整条是一个 instruction。

它通常包含：

```text
opcode
operands
result/type
parent basic block
uses
```

---

## 2.3 Opcode

Opcode = Operation Code，操作码。

上面的：

```text
add
```

就是 opcode。

它告诉编译器：

> “这条指令做加法。”

---

## 2.4 Operand

Operand = 操作数。

```text
%3 = add %1, 5
```

这里：

```text
%1
5
```

是两个 operands。

一句话：

> **operand 是一条指令吃进去的输入。**

---

## 2.5 Value

Value = 值。

`%1`、`%3` 都可以是 Value。

可以把 Value 理解成：

> “编译器能被其他指令引用的一个结果。”

例如：

```text
%3 = add %1, 5
%4 = mul %3, 2
```

`%3` 是第一条 instruction 的结果，同时又成为下一条 instruction 的 operand。

---

# 3. Basic Block 和 CFG

Basic Block = 基本块。

粗略理解：

> **一段从入口开始顺着执行，中间不发生分叉，直到末尾跳转/返回的指令序列。**

例如：

```text
entry:
  %1 = ...
  %2 = ...
  cond_br %2 then else
```

这是一个 basic block。

---

## 3.1 CFG

CFG = Control-Flow Graph。

中文：

> 控制流图。

把：

```text
Basic Block
```

当成点。

把：

```text
jump / branch
```

当成边。

例如：

```text
        entry
       /     \
    then     else
       \     /
        merge
```

这就是一个小 CFG。

为什么优化器需要 CFG？

因为它必须知道：

```text
哪些路可能执行
哪些块永远到不了
一个值在哪些路径上已经定义
两个分支最后在哪里汇合
循环到底有哪些块
```

---

# 4. mem2reg：这次完整讲一遍

mem2reg 是 v3.1 最应该补深的一块。

全称：

> Memory to Register Promotion

中文常叫：

> 内存到寄存器提升。

但这个中文名容易误导。

这里的“register”主要不是指：

```text
RISC-V 的 a0、t0、s1
```

而是指：

> **把临时的内存变量表示提升成 SSA Value。**

---

# 5. 为什么一开始要把变量放内存

源码：

```c
int x = 1;
x = x + 2;
return x;
```

前端最容易实现的方法是：

```text
先给 x 一块地址
```

于是：

```text
%x.addr = alloca
store 1, %x.addr
%t0 = load %x.addr
%t1 = add %t0, 2
store %t1, %x.addr
%t2 = load %x.addr
ret %t2
```

为什么这么麻烦？

因为源码里的：

```c
x = ...
```

天然有“可修改变量”的感觉。

用内存表示：

```text
变量名 → 地址
赋值 → store
读取 → load
```

前端非常容易写对。

---

# 6. mem2reg 第一步：判断这块内存能不能提升

不是所有 `alloca` 都能删。

编译器先检查：

```text
这个地址是不是一个简单局部标量？
这个地址有没有逃逸？
是不是只通过合法的 load/store 使用？
有没有奇怪的指针操作？
```

例如：

```c
int x;
foo(&x);
```

这里：

```text
&x
```

传给了函数。

函数可能保存这个地址、读取、写入。

这种就比：

```c
int x = 1;
return x;
```

复杂得多。

所以：

> mem2reg 不是“看到 alloca 就删”。

---

# 7. mem2reg 第二步：没有分支时很简单

原来：

```text
store 1
load
add
store
load
```

可以变成：

```text
%x0 = 1
%x1 = add %x0, 2
ret %x1
```

这里：

```text
%x0
%x1
```

就是 x 的两个 SSA 版本。

你可以把 SSA 理解成：

> 每次“重新赋值”，不覆盖旧变量，而是产生一个新版本。

源码：

```text
x = 1
x = x + 2
```

SSA 思路：

```text
x0 = 1
x1 = x0 + 2
```

---

# 8. mem2reg 第三步：有分支就必须 PHI

看：

```c
int y;

if (cond)
    y = 3;
else
    y = 4;

return y;
```

then：

```text
y1 = 3
```

else：

```text
y2 = 4
```

到了 merge：

```text
return ???
```

不能写：

```text
return y1
```

因为可能走 else。

也不能写：

```text
return y2
```

因为可能走 then。

所以：

```text
y3 = phi [y1, then], [y2, else]
ret y3
```

---

# 9. PHI 不是“执行一个选择函数”

这是很容易误解的一点。

PHI：

```text
y3 = phi [3, then], [4, else]
```

更准确的意思是：

> **程序控制流实际从哪个前驱块进入，我就采用那个前驱对应的值。**

它是 SSA 层表达控制流合流的方式。

最后 RISC-V 没有 `phi` 指令。

后端会把它变成：

```text
then 里准备 3
else 里准备 4
merge 时使用同一个目标位置
```

我们 final compiler 的真实教学例中，CoreIR 仍然有：

```text
%26 = phi 3 %main.then2 4 %main.else3
```

而最终 assembly 已经没有文字 PHI。

---

# 10. mem2reg 为什么需要 dominance

Dominance = 支配。

先用非常粗糙的直觉：

> **如果想到 B，不管走哪条路都必须先经过 A，那么 A 支配 B。**

为什么 mem2reg 要知道它？

因为如果你准备让：

```text
某个 SSA 定义
```

取代一堆 load：

必须保证：

> 每条可能到达这个 load 的执行路径上，这个定义都已经出现。

否则可能出现：

```text
使用了一个根本没执行过的值
```

---

# 11. dominance frontier 为什么会和 PHI 有关

不用背算法，只理解图。

假设：

```text
       entry
       /   \
      A     B
       \   /
        C
```

A 定义：

```text
x1
```

B 定义：

```text
x2
```

C 是两条定义第一次真正“碰头”的地方。

于是：

```text
C
```

就是典型需要考虑 PHI 的位置。

Dominance Frontier 可以先理解成：

> **“不同定义的控制流开始汇合、单一路径支配关系不再够用的位置。”**

现在不用学它怎么计算。

---

# 12. mem2reg 做完到底有什么收益

不只是：

```text
少几个 load/store
```

更重要的是：

它把：

```text
内存里的模糊变量
```

变成：

```text
清楚的 SSA Value
```

然后：

```text
Constant Propagation
CSE
DCE
LICM
...
```

都更容易看懂值的关系。

所以 mem2reg 更像：

> **给后续优化铺路的基础设施。**

---

# 13. “优化不是越多越好”到底怎么判断

这里必须把两个词分开。

## 13.1 Legality

Legality = 合法性。

问题是：

> **能不能做？**

例如：

```text
x / x → 1
```

如果 x 可能为 0：

不能无条件做。

这是 correctness 问题。

---

## 13.2 Profitability

Profitability = 收益性。

问题是：

> **虽然能做，但值不值得做？**

这是 performance / code-size 问题。

例如一个函数：

```c
foo()
```

完全可以 inline。

语义上没问题。

但 inline 以后：

```text
代码大 5 倍
寄存器压力增加
instruction cache 更差
最后反而更慢
```

那么：

```text
legal = YES
profitable = NO
```

这是两个完全不同的问题。

---

# 14. 实际怎么判断 profitability

没有一个万能公式。

真实编译器通常综合：

```text
静态启发式
+
目标机器 cost model
+
代码结构
+
寄存器压力
+
代码体积
+
循环次数估计
+
benchmark / profile 验证
```

---

# 15. Inline 为什么不是越多越好

收益：

```text
少 call/ret
少参数搬运
暴露常量
暴露 CSE/DCE
```

代价：

```text
代码体积增长
寄存器压力增长
instruction cache 压力
编译时间增长
```

所以常见判断：

```text
函数够不够小？
调用频率高不高？
调用者是不是 hot？
内联后能否产生明显简化？
代码膨胀预算还有多少？
```

---

# 16. Loop Unroll 为什么不是越多越好

收益：

```text
少 branch
少 i++
更多并行机会
更多 constant/CSE
```

代价：

```text
body 复制
代码变大
live values 变多
寄存器压力变大
可能产生 spill
```

所以：

```text
unroll 2x
```

可能变快。

但：

```text
unroll 32x
```

可能更慢。

---

# 17. LICM 也不是“能搬就全搬”

把：

```text
a*b
```

从循环里搬出去可能很好。

但外提以后这个结果的 live range 变长：

```text
原来只在循环局部活跃
→
现在从 preheader 一直活到循环使用
```

这可能增加：

```text
register pressure
```

甚至造成 spill。

所以不同优化之间会互相影响。

---

# 18. 我们 final 代码怎么处理“值不值得做”

这一点正是 `462a178` 比之前版本值得讲的地方之一。

Final 代码新增/加强了：

```text
analysis / proof
candidate planning
clone / rollback
RV64 cost model
RV64 admission
machine verification
```

可以粗略理解为：

```text
发现一个候选
↓
先证明它能做
↓
在候选/clone 上尝试
↓
估计目标机器代价
↓
收益不合适则拒绝/回滚
↓
通过才进入最终结果
```

注意：

> Cost model 也不是神谕。

所以最后仍然必须在真实目标环境测。

---

# 19. Alias 是什么

Alias = 内存别名。

一句话：

> **两个看起来不同的地址，可能其实指向同一块内存。**

例如：

```c
int *p;
int *q;
```

你看到：

```text
p
q
```

名字不同。

但运行时完全可能：

```text
p == q
```

---

# 20. Alias 为什么直接影响优化

看：

```text
x = load p
store 7, q
y = load p
```

你想做：

```text
y = x
```

可以吗？

如果：

```text
p 和 q 不 alias
```

可以考虑。

如果：

```text
p 和 q 指同一块内存
```

第二次 load 应该读到 7。

那么复用 x 就错了。

所以：

```text
CSE
Load Forwarding
DSE
LICM
```

都非常依赖 alias 信息。

---

# 21. Effect 是什么

Effect = 副作用/效果摘要。

尤其针对：

```text
call foo()
```

优化器需要知道：

```text
foo 会不会读内存？
会不会写内存？
会不会改 global？
会不会 I/O？
会不会影响外部可观察行为？
```

这就是 effect analysis 的作用。

例如：

```text
x = load p
foo()
y = load p
```

如果不知道 foo 做什么：

就不能放心：

```text
y = x
```

如果能证明：

```text
foo 不写 p 可能指向的内存
```

才可能复用。

---

# 22. Alias 和 Effect 的区别

Alias 回答：

> **“两个地址是不是可能同一块？”**

Effect 回答：

> **“这个操作/函数到底会读写什么？”**

它们经常一起使用。

例如：

```text
call foo(q)
```

effect 告诉你：

```text
foo 可能写 q
```

alias 告诉你：

```text
q 是否可能和 p 指向同一块
```

两者合起来决定：

```text
之前的 load p 还有效吗？
```

---

# 23. 90 分钟看源码到底“怎么看”

之前写：

```text
看 main.cpp 10 分钟
```

确实太抽象。

真正正确的第一次源码阅读不是：

> 打开文件，从第一行读到最后一行。

而是：

> **带着一个问题，用搜索和跳转追一条链。**

---

# 24. 第一次源码阅读：只追 DCE

这是最推荐的第一次。

你的问题只有一个：

> **“项目里的 DCE 到底怎么删一条指令？”**

---

## Step 1：在 passes.cpp 里搜 `dce(`

不要读整个 `passes.cpp`。

只找：

```text
dce(mod, analysisManager)
```

观察：

```text
它前面是谁？
后面是谁？
为什么 cleanup 会多次调用它？
```

你只需要得到：

```text
DCE 是 pipeline 中一个 cleanup 环节。
```

---

## Step 2：Ctrl+点击 `dce`

进入：

```text
src/core_ir/passes/cleanup.cpp
```

找：

```text
runWorklistDCE
```

---

## Step 3：只找“死”的判断条件

看到类似：

```text
uses().empty()
!hasSideEffects()
```

停。

不要继续读 300 行。

先把它翻译成人话：

```text
没人用
+
没有副作用
=
dead candidate
```

---

## Step 4：找删除动作

看：

```text
remove(...)
```

或者类似 block remove instruction 的逻辑。

问题：

> “删掉这条以后，为什么还要重新检查它的 operands？”

答案：

因为 producer 原来有一个消费者。

消费者被删后：

```text
producer 也可能变 dead
```

于是形成 worklist 连锁删除。

---

## Step 5：回到 passes.cpp

现在再看：

```text
DCE 前后为什么还有 const fold / CSE / CFG simplify
```

你会真正看到：

```text
优化之间互相产生机会
```

---

# 25. 第一次读完 DCE 后，你只需要写出这 5 句话

如果能写出来，就算看懂了：

1. DCE 在 CoreIR cleanup 阶段运行。
2. 它找 `uses` 为空的 instruction。
3. 还必须确认没有 side effect。
4. 删除后会重新检查 operand producers。
5. 它会和 constant folding / CFG / CSE 等反复协作。

不要求你能默写 C++。

---

# 26. 第二次源码阅读才看 mem2reg

问题：

> “为什么普通变量最后变成 SSA + PHI？”

阅读顺序：

```text
ast_to_ir.cpp
↓
观察 alloca/load/store
↓
mem2reg.cpp
↓
找 promotable alloca
↓
找 PHI 创建
↓
找 rename/replace load
```

这次不要试图理解：

```text
完整 dominator 算法
```

只确认：

```text
前端先放内存
mem2reg 找可提升槽
分支合流放 PHI
load/store 最终被 SSA value 替代
```

---

# 27. 经典优化大家都知道，比赛为什么还能拉开差距

这是非常关键的问题。

答案是：

> **“知道优化名字”和“实现一个高性能优化编译器”之间差得非常远。**

大家都知道：

```text
DCE
CSE
Inlining
LICM
Unroll
```

就像大家都知道：

```text
排序
哈希表
动态规划
```

并不意味着所有实现性能一样。

---

# 28. 第一层差距：谁能证明更多程序可以安全优化

两个编译器都做 LICM。

A：

```text
遇到 load 就不敢动
```

B：

通过 alias/effect 分析证明：

```text
这个 load 不会被循环内写破坏
```

于是 B 能外提更多。

两者名字都叫：

```text
LICM
```

能力完全不同。

---

# 29. 第二层差距：谁知道什么时候“不该优化”

两个编译器都能 inline。

A：

```text
小函数全 inline
```

B：

根据：

```text
代码大小
调用位置
后续优化机会
目标成本
```

选择性 inline。

B 可能更快。

---

# 30. 第三层差距：Pass 顺序

假设：

```text
Inline
→ Constant Propagation
→ CFG Simplification
→ DCE
```

可能很好。

如果顺序不同：

```text
DCE
→ Inline
→ 不再 cleanup
```

就可能留下很多垃圾。

这叫：

> phase ordering，优化阶段排序。

经典 pass 相同，顺序也能产生巨大差异。

---

# 31. 第四层差距：IR 设计和 Analysis

如果 IR 的：

```text
use-def
CFG
dominance
alias
effect
loop information
```

做得好：

优化 pass 能更精确。

如果这些基础设施弱：

即使“实现了 LICM”，也可能非常保守。

---

# 32. 第五层差距：后端

前端和中端生成同样语义的 IR。

最后 RISC-V 仍然可能差很多：

```text
instruction selection
RA
spill
calling convention
schedule
block layout
peephole
frame
```

最终 CPU 跑的是：

```text
机器指令
```

不是 pass 名字。

---

# 33. 第六层差距：针对目标硬件

一个优化在 x86 好，不代表在 RISC-V/BOOM 上同样好。

不同 CPU：

```text
latency
throughput
branch
cache
register resources
instruction cost
```

都不同。

所以比赛强调：

> 面向目标平台优化。

---

# 34. 我们的项目和“教科书优化”到底是什么关系

可以分成两层看。

## 第一层：我们当然也使用经典优化

例如：

```text
Constant Folding
DCE
mem2reg
CFG Simplification
CSE
LICM
Inlining
Loop Unroll
```

这些不是我们的原创概念。

也没必要假装是。

---

## 第二层：我们的实现组合和工程闭环是自己的

Final `462a178` 当前比较值得讲的不是：

> “我们发明了 DCE。”

而是：

```text
共享 analysis/proof 层
↓
candidate planning
↓
clone / rollback
↓
legality
↓
RV64 cost/admission
↓
cleanup
↓
IR verifier / machine verifier
```

也就是说：

> **我们把经典优化放进了一套更明确的分析、证明、收益判断和验证框架里。**

这是比“我们有 50 个 pass”更好的答辩叙事。

---

# 35. 我们 final 代码有哪些比较有辨识度的地方

目前最适合讲的几个点：

## A. 两级 IR

```text
AST/HIR
→ CoreIR
→ MIR
```

CoreIR 做机器无关优化。

MIR 处理：

```text
virtual/physical registers
ABI
frame
machine instruction
```

---

## B. Final analysis/proof 层

Final commit 加强了：

```text
alias
range / integer facts
function effect / modref
loop dependence
memory SSA-like information
semantic summary
```

这些的意义不是“多了几个文件”。

而是：

> 多个优化能共享同一批事实，不需要各自瞎猜。

---

## C. Candidate planning / rollback

复杂优化不是直接把原 IR 改坏。

思路是：

```text
形成 candidate
在 clone/计划上尝试
检查
收益不合适就拒绝
```

这是 final 版本很值得讲的一点。

---

## D. RV64 cost/admission

不是：

```text
IR 上看起来能优化
→ 必须接受
```

而是开始加入：

```text
目标机器上的成本判断
```

它直接对应你问的：

> “不是越多越好，怎么判断？”

---

## E. MIR backend

真实做：

```text
liveness
linear scan
spill
schedule
block layout
peephole
frame lowering
machine verification
```

这也是为什么我们有真实：

```text
160B frame
s1-s11
8 spill
8 reload
```

这样的案例。

---

# 36. 为什么我们会得到不同的运行性能

先区分两件事。

## 程序输出

应该一样。

Final：

```text
200/200
```

证明公开恢复测试集上：

```text
语义输出一致
```

---

## 运行时间

可以差很多。

两个编译器都输出正确结果，但：

Compiler A：

```text
1000 条动态指令
很多 load/store
很多 spill
```

Compiler B：

```text
700 条动态指令
更少 spill
更好 schedule
```

当然可能不同。

---

# 37. 同一个优化版本为什么有时更快、有时反而更慢

因为一个大版本可能：

```text
某些 case 减少指令
某些 case 增加寄存器压力
某些 case 改 block layout
某些 case 产生不同 spill
某些 case code size 变大
```

Final `462a178` 对旧 clean baseline：

```text
145/200 cases 汇编发生变化
60/60 performance cases 全变化
```

但这个 commit 同时改了太多层。

因此当前没有逐 pass ablation 时：

> 不能把某个 case 的速度变化强行归因给某一个 pass。

这也是严谨答辩必须说清楚的。

---

# 38. 比赛到底是给模板还是从头做

先区分：

> “比赛给语言规范、运行库、评测环境、仓库/提交平台”

和：

> “比赛给一个已经实现好的编译器模板，只让你加优化”

不是一回事。

目前我们能确认的官方要求是：

> 参赛队要设计并实现综合性的编译系统，并且可以自行决定编译器系统架构、前端、后端和优化细节。

因此不要把这个比赛理解成：

```text
官方给 LLVM-like 完整编译器
↓
大家只写 5 个优化 pass
```

对于我们这个项目，仓库本身包含：

```text
frontend
AST
CoreIR
MIR
direct backend
optimized backend
RA
frame
assembly emission
```

显然不是一个“只补几个 pass”的薄层工程。

目前没有足够证据让我说：

> “2026 官方绝对没有提供任何 starter skeleton。”

所以最安全的理解是：

> **赛事规定接口/语言/运行和评测要求；完整编译系统的架构与大量实现工作由参赛队负责。**

---

# 39. 老师最可能问什么

按我们现在的项目和 PPT，优先级可以这样排。

## 第一档：几乎一定要能答

### 1. 你们编译器整体流程是什么？

要能画：

```text
SysY
→ AST/HIR
→ CoreIR
→ MIR
→ RA
→ RISC-V
```

---

### 2. 为什么需要 CoreIR 和 MIR 两层？

CoreIR：

```text
机器无关 SSA/CFG 优化
```

MIR：

```text
机器寄存器、ABI、指令、frame
```

---

### 3. 举一个你真正懂的优化

最推荐：

```text
Constant Folding
mem2reg + PHI
DCE
```

老师可能继续问：

```text
什么时候能做？
什么时候不能做？
怎么验证？
```

---

### 4. mem2reg 为什么需要 PHI？

这是很自然的追问。

---

### 5. 什么是 CFG / SSA / dominance？

不一定要求你讲算法。

但必须有准确直觉。

---

### 6. 寄存器不够怎么办？

你要能说：

```text
liveness
live interval
linear scan
spill
stack frame
```

---

### 7. caller-saved / callee-saved 是什么？

RA 案例后非常可能接这个。

---

# 40. 第二档：优化方向高概率

### 8. 你们怎么判断一个优化是否合法？

回答：

```text
本项目通过“保守分析 + 结构验证 + 测试证据”判断优化是否合法：每个 pass 先根据 CFG、SSA、支配关系、alias、effect、终止性、值域和 live-out 等通用事实证明变换前提，无法证明时直接拒绝；复杂变换先在克隆 IR 上事务式执行，通过 CoreIR 的CFG/Phi/use-def 验证、MIR 的寄存器与栈帧验证后才提交，并在修改后失效旧分析缓存；最后用真正触发的正例、相近但应拒绝的负例和功能回归验证语义。需要注意，verifier 只能保证 IR 结构正确，优化前后的语义等价仍必须由 pass 的前提证明保证。
```

---

### 9. 为什么不是优化越多越好？

回答：

```text
legality
+
profitability
```

举：

```text
inline / unroll
```

---

### 10. 你们 final 优化体系和普通 pass 堆叠有什么不同？

可以讲：

```text
我们的 final 优化体系不是把多个 pass 按顺序简单堆叠，而是以 shared analysis 统一提供 CFG、支配、alias、effect、值域和循环依赖等事实；每个优化先通过 candidate planning 形成带合法性证明的候选，在克隆 IR 上执行，失败时通过 rollback保证原程序不受污染；候选还需经过 RV64 admission，根据真实 MIR 的寄存器压力、spill、栈帧和指令成本判断是否值得采用，最后由 verifier 检查 CoreIR 的 CFG/SSA/Phi 以及 MIR 的寄存器、控制流和 ABI 不变量，全部通过后才提交，因此同时保证语义合法、结构正确和目标机器上有收益。
```

---

### 11. LICM 为什么难？

关键词：

```text
LICM 的难点不只是识别 loop invariant，还要证明把指令移出循环不会改变语义：目标循环必须存在合法的 preheader，指令的操作数定义必须满足 dominance，所有执行路径都能安全使用提升后的值；对于 load，还必须通过 alias 分析证明循环内没有可能覆盖同一地址的 store，并通过 effect 分析确认循环中的调用不会修改相关内存。任何一项无法证明都必须放弃提升，否则可能改变内存结果、异常行为或程序执行路径。
```

---

# 41. 第三档：比赛/工程高概率

### 12. 你们怎么保证正确性？

回答：

```text
strict build
research OFF
200 cases
compile
link
QEMU
output compare
```

不要说：

> “所以证明了所有程序都正确。”

---

### 13. 为什么 QEMU 和 BOOM 结果可能不同？

因为目标微结构不同。

QEMU：

```text
功能验证 + 本地趋势
```

BOOM：

```text
最终目标平台性能
```

---

### 14. 为什么 Final 版本某些 case 反而可能更慢？

这正可以回答：

```text
cost model 不是完美预测
pass interaction
register pressure
spill
code size
schedule
target behavior
```

---

### 15. research-only 是什么？

说：

> 实现保留用于研究/实验，但 strict final build 默认不进入生产代码生成路径。

---

# 42. 第四档：有可能直接问“你们有什么特别的”

不要回答：

> “我们优化很多。”

可以回答：

> “经典 pass 本身并不新，我们 final 版本更强调共享分析、合法性证明、候选规划与回滚，以及 RV64 机器成本准入；中端和 MIR 后端之间有完整验证链。”

然后拿三个案例：

```text
ret 20
PHI
真实 spill
```

证明不是空话。

---

# 43. 官方决赛为什么还可能问“怎么适应变化”

赛事技术方案明确强调：

> 决赛可能根据现场发布的目标语言或目标平台变化要求调整编译系统。

所以老师可能不只问：

```text
你现在有什么
```

还可能问：

```text
如果目标 ISA / 语言要求变化，你们架构怎么改？
```

你们分层设计在这里很好回答：

```text
语言变化
→ frontend / AST / lowering

中端通用优化
→ CoreIR 大部分保留

目标机器变化
→ MIR lowering / cost model / backend
```

---

# 44. v3.1 自测题答案补全

## 1

Constant Folding：

> 直接计算常量表达式。

Constant Propagation：

> 把已知常量沿 use-def 传给使用点。

---

## 2

`uses().empty()` 不够。

还要：

```text
无副作用
```

---

## 3

`putint(3)` 有 I/O 副作用。

不能删。

---

## 4

不同控制流路径可能给同一源码变量产生不同定义。

合流时需要 PHI。

---

## 5

PHI 不是 RISC-V 指令。

最终通过 predecessor copy / register assignment 消解。

---

## 6

删除 CFG block 会改变 predecessor/successor。

PHI incoming 必须同步，否则“值来自哪条边”的语义会坏。

---

## 7

CSE 跨块复用时，要保证 producer 在所有到 use 的路径上先执行。

这需要 dominance。

---

## 8

`a+b` 的输入是 SSA values。

`load p` 还受中间 store/call/alias 影响。

---

## 9

Alias：

> 两个不同地址表达式可能指向同一内存。

---

## 10

Effect：

> 指令/函数可能读写内存、I/O 或产生其他可观察行为的摘要。

---

## 11

LICM 要把指令移到循环外。

Preheader 提供统一、安全的循环外插入点。

---

## 12

`x/x → 1`：

x 可能是 0。

所以不能无条件改。

---

## 13

Inline 后：

```text
原来的函数参数
```

可能变成：

```text
调用处真实常量/值
```

于是暴露 constant fold、DCE、CSE 等机会。

---

## 14

Unroll 复制 body。

同时 live 的临时值可能增加。

寄存器压力变大后可能 spill。

---

## 15

CoreIR 不知道最终：

```text
物理寄存器
ABI
真实指令
frame
latency
```

所以 MIR 后端仍要优化。

---

## 16

物理寄存器不足，并且某个 live value 不能保存在现有寄存器时，需要 spill。

---

## 17

函数 call 可能破坏 caller-saved registers。

跨 call live value 需要更谨慎的寄存器选择或 spill。

---

## 18

一个 pass 会制造另一个 pass 的新机会。

所以反复执行到不再变化。

---

## 19

窄 matcher 只代表：

> 适用范围窄。

是否合理要看：

```text
是否描述一般语义条件
是否有正确性证明
是否依赖 testcase identity
```

---

## 20

三个 case 收益集中只能说明：

```text
这个 matcher 值得重点审查
```

不能单凭性能分布证明它依赖 testcase 身份。

---

# 45. 你接下来怎么学

现在不要再“快速看一整篇”。

下一步建议分三轮。

## Round A：名词地基

必须能口头解释：

```text
IR
Instruction
Operand
Value
Basic Block
CFG
SSA
PHI
Dominance
Alias
Effect
```

---

## Round B：真正吃透三个 pass

只学：

```text
Constant Folding
DCE
mem2reg
```

要求每个都能回答：

```text
输入是什么？
发现什么？
合法条件？
怎么改？
改完帮助谁？
项目代码在哪？
```

---

## Round C：第一次看源码

只追：

```text
DCE
```

不要 90 分钟同时啃 7 个文件。

等你第一次能把 DCE 从：

```text
passes.cpp
→ cleanup.cpp
→ uses / side effect
→ remove / worklist
```

完整讲出来，再进入 mem2reg。

---

# 46. 当前学习目标不是“成为优化器专家”

答辩前真正需要达到的是：

你看到：

```text
Constant Folding
DCE
mem2reg
CSE
LICM
RA
```

不会只是认出英文。

而是能立即回答：

```text
它解决什么问题？
在哪一层？
安全条件是什么？
收益条件是什么？
我们代码在哪？
为什么它可能改变最终汇编？
```

做到这一层，你就已经从：

> “背 PPT 的主答辩人”

变成：

> “能解释自己编译器设计的主答辩人”。

