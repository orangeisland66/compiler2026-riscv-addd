// 文件职责：定义 SysY 抽象语法树节点、类型和语义信息承载结构。本文件负责声明接口和数据结构。
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <cstdint>
#include <optional>

// 前向声明（Sema 模块定义 / 后续定义）
struct Symbol;
struct FuncDef;

// ============================================================
// SysY2022 类型系统
// ============================================================
enum class BaseType { Int, Float, Char, Void };

// 完整类型：基础类型 + 数组维度列表
// 若 dims 为空，则是标量；否则是数组（dims[0]==0 表示该维可省略，如函数参数首维）
struct SysType {
    BaseType base;
    std::vector<int> dims; // 各维大小，-1 表示参数的第一维（省略）

    bool isArray()  const { return !dims.empty(); }
    bool isInt()    const { return base == BaseType::Int; }
    bool isFloat()  const { return base == BaseType::Float; }
    bool isChar()   const { return base == BaseType::Char; }
    bool isVoid()   const { return base == BaseType::Void; }

    // 数组元素总数
    int totalElems() const {
        int n = 1;
        for (int d : dims) if (d > 0) n *= d;
        return n;
    }
    // 去掉第一维（取子数组类型，用于数组传参）
    SysType deref() const {
        SysType t; t.base = base;
        t.dims = std::vector<int>(dims.begin()+1, dims.end());
        return t;
    }
};

// ============================================================
// AST 节点基类（多态）
// ============================================================
struct Node {
    int line = 0; // 源码行号，用于错误提示
    virtual ~Node() = default;
};

using NodePtr = std::unique_ptr<Node>;

// ============================================================
// 表达式节点
// ============================================================
struct Expr : Node {
    std::optional<SysType> semaType; // Sema 阶段填充的类型标注
};
using ExprPtr = std::unique_ptr<Expr>;

// 整型字面量
struct IntLitExpr : Expr {
    int32_t val;
};

// 浮点字面量（2025年新增）
struct FloatLitExpr : Expr {
    float val;
};

struct CharLitExpr : Expr {
    uint8_t val;
};

// 字符串字面量表达式。字符串可由相邻字面量拼接而成；当前 SysY
// 扩展主要使用其常量下标形式，例如 "A"[0]。
struct StringLitExpr : Expr {
    std::string value;
    std::vector<ExprPtr> indices;
};

// 左值：变量名 + 可选下标列表
// 例：a[1][2] → name="a", indices={1, 2}
struct LValExpr : Expr {
    std::string name;
    std::vector<ExprPtr> indices;
    Symbol* symbol = nullptr; // Sema 阶段绑定的符号
};

// 函数调用
struct CallExpr : Expr {
    std::string name;
    std::vector<ExprPtr> args;
    FuncDef* callee = nullptr; // Sema 阶段绑定的函数定义
};

// 隐式类型转换（Sema 插入）
// 例：int + float → CastExpr(Int→Float, int-expr) + float-expr
struct CastExpr : Expr {
    SysType targetType;
    ExprPtr operand;
};

// 一元表达式：+, -, !, ~
struct UnaryExpr : Expr {
    char op;        // '+', '-', '!', '~'
    ExprPtr operand;
};

// 二元表达式（含算术、关系、按位与逻辑运算）
// op 用字符串表示："+", "-", "*", "/", "%", "<", "<=", ">", ">=",
// "==", "!=", "&", "^", "|", "&&", "||"
struct BinaryExpr : Expr {
    std::string op;
    ExprPtr lhs, rhs;
};

// ============================================================
// 初始化值节点（用于变量声明）
// ============================================================
struct InitVal : Node {};
using InitValPtr = std::unique_ptr<InitVal>;

// 标量初始化：单个表达式
struct ScalarInit : InitVal {
    ExprPtr expr;
};

// 聚合初始化：{v1, v2, {v3, v4}, ...}
struct AggregateInit : InitVal {
    std::vector<InitValPtr> items;
};

struct StringInit : InitVal {
    std::string value;
};

// ============================================================
// 声明节点
// ============================================================

// 单个变量/常量定义（ConstDef 或 VarDef）
struct VarDef : Node {
    std::string name;
    bool isConst;
    SysType type;         // 含数组维度信息（基础类型在外层 Decl 中）
    InitValPtr initVal;   // 可为 nullptr（对 const 必须有）
};

// 变量声明语句（一行可声明多个，如 int a, b[3];）
struct DeclStmt : Node {
    bool isConst;
    bool isStatic = false;
    BaseType baseType;
    std::vector<std::unique_ptr<VarDef>> defs;
};

// ============================================================
// 语句节点
// ============================================================
struct Stmt : Node {};
using StmtPtr = std::unique_ptr<Stmt>;

// 赋值语句：lval = expr;
struct AssignStmt : Stmt {
    ExprPtr lval;    // LValExpr
    ExprPtr rhs;
};

// 表达式语句（可为空）
struct ExprStmt : Stmt {
    ExprPtr expr; // 可为 nullptr（空语句 ;）
};

// 语句块 { ... }
struct BlockStmt : Stmt {
    // 块中的元素：声明或语句
    struct Item {
        enum class Kind { Decl, Stmt } kind;
        std::unique_ptr<DeclStmt> decl;
        StmtPtr stmt;
    };
    std::vector<Item> items;
};

// if-else 语句
struct IfStmt : Stmt {
    ExprPtr cond;
    StmtPtr thenBranch;
    StmtPtr elseBranch; // 可为 nullptr
};

// while 循环
struct WhileStmt : Stmt {
    ExprPtr cond;
    StmtPtr body;
};

// do body while (cond); 循环体至少执行一次，continue 转向条件判断。
struct DoWhileStmt : Stmt {
    StmtPtr body;
    ExprPtr cond;
};

struct SwitchStmt : Stmt {
    struct Clause {
        int line = 0;
        bool isDefault = false;
        ExprPtr value;
        std::vector<StmtPtr> statements;
    };
    ExprPtr value;
    std::vector<Clause> clauses;
};

struct PrintfStmt : Stmt {
    std::string format;
    std::vector<ExprPtr> args;
};

// 跳转语句节点：break、continue 与 return [expr]
struct BreakStmt    : Stmt {};
struct ContinueStmt : Stmt {};
struct ReturnStmt   : Stmt {
    ExprPtr val; // void 函数可为 nullptr
};

// ============================================================
// 函数形参
// ============================================================
struct FuncParam : Node {
    std::string name;
    SysType type;        // 含数组维度（第一维为 -1 表示省略）
};

// ============================================================
// 顶层：函数定义 和 全局声明
// ============================================================
struct FuncDef : Node {
    SysType retType;
    std::string name;
    std::vector<std::unique_ptr<FuncParam>> params;
    std::unique_ptr<BlockStmt> body;
};

// 编译单元（程序根节点）
struct CompUnit : Node {
    struct Item {
        enum class Kind { GlobalDecl, FuncDef } kind;
        std::unique_ptr<DeclStmt> decl;
        std::unique_ptr<FuncDef>  func;
    };
    std::vector<Item> items;
};
