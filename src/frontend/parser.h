// 文件职责：解析 SysY 语法并构建带基础语义信息的 AST。本文件负责声明接口和数据结构。
#pragma once
#include "lexer.h"
#include "ast.h"
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

struct ParseDiagnostic {
    int line = 0;
    char code = 'i';
};

// ============================================================
// 递归下降语法分析器
// 严格按照 SysY2022 BNF 文法实现
// ============================================================
class Parser {
public:
    explicit Parser(Lexer& lex);

    // 解析整个编译单元
    std::unique_ptr<CompUnit> parseCompUnit();
    const std::vector<ParseDiagnostic>& diagnostics() const {
        return diagnostics_;
    }

private:
    Lexer& lex_;
    std::vector<ParseDiagnostic> diagnostics_;

    // ---- 辅助方法 ----
    const Token& peek()  { return lex_.peek(); }
    Token next()         { return lex_.next(); }
    bool check(TK t)     { return peek().type == t; }
    bool checkType()     { return check(TK::Int) || check(TK::Float) ||
                                  check(TK::Char) || check(TK::Void); }

    // 消费指定类型的 token，否则报错
    Token expect(TK t, const char* msg = nullptr);
    // 若下一个 token 类型匹配则消费并返回 true
    bool tryConsume(TK t);

    std::string expectId();

    void error(const std::string& msg);

    // ---- 编译期常量表（记录已解析的 const int 变量值，用于数组维度求值）----
    std::unordered_map<std::string, int32_t> constInts_;

    // 尝试对数组维度表达式求值，返回 -1 表示无法求值
    int32_t tryEvalDim(Expr* e);

    // ---- 类型解析 ----
    BaseType parseBaseType(); // int | float | char | void

    // ---- 顶层声明 / 函数定义 ----
    // 区分全局变量声明和函数定义（通过前瞻）
    CompUnit::Item parseTopLevel();

    // ---- 函数定义 ----
    std::unique_ptr<FuncDef> parseFuncDef(SysType retType, std::string name);
    std::unique_ptr<FuncParam> parseFuncParam();

    // ---- 声明语句 ----
    std::unique_ptr<DeclStmt> parseDecl(bool isConst, BaseType base,
                                        bool isStatic = false);
    std::unique_ptr<VarDef>   parseVarDef(bool isConst, BaseType base);
    InitValPtr parseInitVal();

    // ---- 语句 ----
    StmtPtr parseStmt();
    std::unique_ptr<BlockStmt> parseBlock();

    // ---- 表达式（按优先级从低到高）----
    ExprPtr parseExpr();        // 扩展 Exp → LOrExp
    ExprPtr parseCond();        // Cond → LOrExp
    ExprPtr parseLOrExpr();     // ||
    ExprPtr parseLAndExpr();    // &&
    ExprPtr parseBitOrExpr();   // |
    ExprPtr parseBitXorExpr();  // ^
    ExprPtr parseBitAndExpr();  // &
    ExprPtr parseEqExpr();      // == !=
    ExprPtr parseRelExpr();     // < <= > >=
    ExprPtr parseAddExpr();     // + -
    ExprPtr parseMulExpr();     // * / %
    ExprPtr parseUnaryExpr();   // 一元运算符
    ExprPtr parsePrimaryExpr(); // 基本表达式

    // 解析数组下标列表：('[' Exp ']')*
    std::vector<ExprPtr> parseIndices();

    // 解析常量表达式（语义上要求编译期可求值，语法上同 Exp）
    ExprPtr parseConstExpr();

};
