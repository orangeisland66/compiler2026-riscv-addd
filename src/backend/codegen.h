// 文件职责：实现未优化路径的直接 RISC-V 汇编生成，作为功能性后备后端。本文件负责声明接口和数据结构。
#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include <functional>
#include <optional>

// ============================================================
// 符号信息：记录变量在栈帧中的位置和类型
// ============================================================
struct Symbol {
    std::string name;
    SysType     type;
    bool        isConst   = false;
    bool        isGlobal  = false;
    bool        isParam   = false;  // 是否是函数参数（数组参数传指针）

    // 局部变量：相对于帧指针 s0 的偏移（负值，如 -8 表示 s0-8）
    int fpOffset = 0;

    // const 标量的编译期值（已求值）
    std::optional<int32_t> constInt;
    std::optional<float>   constFloat;
    // const 数组的编译期值
    std::vector<int32_t>   constIntArr;
    std::vector<float>     constFloatArr;
};

// ============================================================
// RISC-V 代码生成器
// 设计原则：
//   - 所有变量/临时值存储在栈上（naive 分配，功能正确但性能较差）
//   - 整型表达式结果放入 a0（int）或 fa0（float）
//   - 二元运算用栈保存左操作数，然后与右操作数合并
//   - 使用帧指针 s0 访问局部变量
// ============================================================
class CodeGen {
public:
    CodeGen(std::ostream& out, bool optimize);

    // 入口：生成整个程序
    void generate(CompUnit* cu);

private:
    std::ostream& out_;       // 输出汇编到此流
    int labelCnt_ = 0;        // 全局标签计数器

    // ---- 符号表（作用域栈）----
    // 每个作用域是一个 name→Symbol 的映射
    std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    void pushScope();
    void popScope();
    Symbol* lookupSym(const std::string& name);
    void addSym(Symbol sym);

    // ---- 当前函数状态 ----
    int frameSize_    = 0;   // 当前帧大小（字节，含 ra/s0 保存区域）
    int nextOffset_   = 0;   // 下一个可用的负偏移（从 -16 开始，前16字节给 ra/s0）
    SysType curRetType_;     // 当前函数返回类型
    std::string curFuncName_;
    std::string curRetLabel_;   // 函数统一 epilogue 标签
    std::string breakLabel_;    // break 跳转目标
    std::string continueLabel_; // continue 跳转目标

    // ---- 函数返回类型表（用于确定用户函数返回值类型）----
    std::unordered_map<std::string, BaseType> funcRetTypes_;
    std::unordered_set<std::string> userFunctions_;
    // ---- 函数参数类型表（用于隐式类型转换）----
    std::unordered_map<std::string, std::vector<std::pair<BaseType, bool>>> funcParamTypes_;

    // ---- 全局变量 ----
    std::unordered_map<std::string, Symbol> globals_;

    // ---- 标签生成 ----
    std::string newLabel(const std::string& prefix = ".L");

    // ---- emit 辅助 ----
    void emit(const std::string& s);
    void emitLabel(const std::string& lbl);
    void emitLoadGlobalAddr(const std::string& reg, const std::string& sym);
    void normalizeCond(BaseType t);

    // ---- 窥孔优化：函数体指令缓冲 ----
    bool optimize_ = false;
    bool funcBufMode_ = false;
    std::vector<std::string> funcBuf_; // 原始指令（不带前导空格）或 "label:"
    void beginFuncBuf();
    void flushFuncBuf(); // 应用窥孔优化后输出

    // ---- 常量求值（编译期）----
    // 对 const 变量初始化使用
    int32_t evalConstInt(Expr* e);
    float   evalConstFloat(Expr* e);

    // ---- 全局变量生成 ----
    void genGlobal(DeclStmt* decl);
    void emitGlobalData(const Symbol& sym, InitVal* init, int& idx, int total, int depth = 0);

    // ---- 函数生成 ----
    void genFunc(FuncDef* func);

    // 预扫描函数体，计算所有局部变量需要的总栈空间
    int calcFrameSize(FuncDef* func);
    void scanDecls(BlockStmt* block, int& offset);
    void scanStmt(Stmt* stmt, int& offset);

    // 为变量分配栈槽，返回偏移
    int allocLocal(int bytes, int align = 4);

    // ---- 语句生成 ----
    void genBlock(BlockStmt* block);
    void genStmt(Stmt* s);
    void genDecl(DeclStmt* decl);

    // 初始化局部变量（将 initVal 写入已分配的栈槽）
    void genLocalInit(Symbol& sym, InitVal* init);
    // 对数组进行逐元素初始化
    void genArrayInit(Symbol& sym, AggregateInit* agg, int& elemIdx, int depth);

    // ---- 表达式生成 ----
    // 返回表达式的 BaseType（Int 或 Float）
    // 结果放在：Int→a0, Float→fa0
    BaseType genExpr(Expr* e);

    // 二元运算
    BaseType genBinary(BinaryExpr* e);

    // 短路运算（&&, ||）必须用条件跳转实现
    BaseType genLogicalAnd(BinaryExpr* e);
    BaseType genLogicalOr(BinaryExpr* e);

    // 一元运算
    BaseType genUnary(UnaryExpr* e);

    // 函数调用
    BaseType genCall(CallExpr* e);

    // 左值地址计算：将地址放入 a0
    // 返回元素类型（BaseType）
    BaseType genLValAddr(LValExpr* lv);

    // 左值读取（load）
    BaseType genLValLoad(LValExpr* lv);

    // ---- 类型转换 ----
    // 将 a0（Int）转换为 fa0（Float），或反之
    void intToFloat();  // a0 → fa0
    void floatToInt();  // fa0 → a0

    // ---- 整型运算指令发射 ----
    void emitIntBinop(const std::string& op); // a1 op a0 → a0
    void emitFloatBinop(const std::string& op); // fa1 op fa0 → fa0

    // ---- 比较指令 ----
    // 比较结果放入 a0（0 or 1）
    void emitIntCmp(const std::string& op);
    void emitFloatCmp(const std::string& op);

    // ---- 加载立即数 ----
    void loadImm(const std::string& reg, int64_t val);

    // ---- Float 常量加载 ----
    void loadFloatConst(float val);

    // ---- 大偏移量辅助（处理超出 12-bit 立即数范围的栈帧偏移）----
    // RISC-V 12-bit 立即数范围：-2048 ~ 2047
    static bool isSmallImm(int v) { return v >= -2048 && v <= 2047; }

    // dst = base + offset（偏移过大时使用 li+add，否则 addi）
    void emitAddrAdd(const std::string& dst,
                     const std::string& base, int offset);

    // mem[base+offset] 32-bit 有符号加载 → reg
    void emitLw (const std::string& reg, const std::string& base, int offset);
    void emitFlw(const std::string& reg, const std::string& base, int offset);
    // mem[base+offset] 64-bit 加载 → reg
    void emitLd (const std::string& reg, const std::string& base, int offset);

    // reg → mem[base+offset] 32-bit 存储
    void emitSw (const std::string& reg, const std::string& base, int offset);
    void emitFsw(const std::string& reg, const std::string& base, int offset);
    // reg → mem[base+offset] 64-bit 存储
    void emitSd (const std::string& reg, const std::string& base, int offset);
};
