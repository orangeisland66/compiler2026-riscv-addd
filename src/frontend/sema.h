// 文件职责：执行 SysY2026 的作用域、类型和控制流语义检查。
#pragma once

#include "ast.h"
#include <string>
#include <unordered_map>
#include <vector>

struct SemaDiagnostic {
    int line = 0;
    char code = 'e';
};

class Sema {
public:
    std::vector<SemaDiagnostic> analyze(CompUnit* unit);

private:
    struct SymbolInfo {
        SysType type;
        bool isConst = false;
    };
    struct FunctionInfo {
        SysType retType;
        std::vector<SysType> params;
    };

    std::vector<SemaDiagnostic> diagnostics_;
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
    std::unordered_map<std::string, FunctionInfo> functions_;
    std::unordered_map<std::string, SymbolInfo> globals_;
    SysType currentReturn_{};
    int loopDepth_ = 0;
    int switchDepth_ = 0;
    bool inCondition_ = false;

    void error(int line, char code);
    void pushScope();
    void popScope();
    const SymbolInfo* lookup(const std::string& name) const;
    bool declare(const std::string& name, SymbolInfo info, int line,
                 bool global);
    const FunctionInfo* lookupFunction(const std::string& name) const;

    void collectTopLevel(CompUnit* unit);
    void checkGlobalDecl(DeclStmt* decl);
    void checkFunction(FuncDef* func);
    void checkBlock(BlockStmt* block);
    void checkDecl(DeclStmt* decl);
    void checkStmt(Stmt* stmt);
    SysType checkExpr(Expr* expr, bool condition = false);
    SysType checkLVal(LValExpr* lv, bool write);
    void checkInit(InitVal* init, const SysType& target, bool global,
                   bool isConst);
    bool isScalar(const SysType& type) const;
    bool sameType(const SysType& lhs, const SysType& rhs) const;
    bool compatibleValue(const SysType& actual, const SysType& expected) const;
    bool compatibleParam(const SysType& actual, const SysType& expected) const;
    bool isConstExpr(Expr* expr, BaseType* result = nullptr);
};
