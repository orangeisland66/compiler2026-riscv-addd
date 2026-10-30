// 文件职责：把 SysY AST 降低为 CoreIR，并建立控制流和内存模型。本文件负责声明接口和数据结构。
#pragma once
#include "../ir/core_ir.h"
#include "../../frontend/ast.h"
#include <memory>
#include <optional>

namespace core_ir {

// AST -> CoreIR 降低。
// 将 SysY AST 转换为 CoreIR Module。
class ASTToIR {
public:
  ASTToIR();
  std::unique_ptr<Module> lower(CompUnit* cu);

private:
  struct VarInfo {
    Value* addr = nullptr;
    Type elemTy = Type::i32();
    std::vector<int> dims;
    bool isArray = false;
    bool isArrayParam = false;
    bool isChar = false;
    bool isConst = false;
    std::optional<int32_t> constWord;
    std::vector<int32_t> constWords;
  };

  std::unique_ptr<Module> mod_;
  IRBuilder* builder_ = nullptr;
  Function* curFunc_ = nullptr;

  // 循环 break/continue 目标。
  BasicBlock* breakTarget_ = nullptr;
  BasicBlock* continueTarget_ = nullptr;

  // 返回值存储位置和返回块。
  Value* retValSlot_ = nullptr;
  BasicBlock* retBlock_ = nullptr;
  BaseType curRetBase_ = BaseType::Int;

  // 作用域：变量名 -> 存储位置和类型信息。
  std::vector<std::unordered_map<std::string, VarInfo>> scopes_;
  std::unordered_map<std::string, VarInfo> globals_;

  // 函数签名信息。
  std::unordered_map<std::string, SysType> funcRetTypes_;
  std::unordered_map<std::string, std::vector<std::pair<BaseType, bool>>> funcParamTypes_;

  void pushScope();
  void popScope();
  VarInfo* lookupVar(const std::string& name);
  void addVar(const std::string& name, VarInfo info);

  void lowerFunc(FuncDef* func);
  void lowerGlobalDecl(DeclStmt* decl);

  // 语句。
  void lowerStmt(Stmt* stmt);
  void lowerBlock(BlockStmt* block);
  void lowerDecl(DeclStmt* decl);

  // 表达式 -> Value* 结果。
  Value* lowerExpr(Expr* expr);
  Value* lowerLValAddr(LValExpr* lv);
  Value* lowerLValLoad(LValExpr* lv);
  bool isSpeculativelySafeBoolExpr(Expr* expr);
  Value* lowerBoolExprEager(Expr* expr);

  // 辅助函数。
  Type toIRType(BaseType bt, bool isArray = false) const;
  Value* ensureType(Value* val, Type target);
  Value* normalizeCond(Value* val);
  int typeSize(Type ty) const;
  int totalElems(const std::vector<int>& dims) const;
  int flattenConstIndex(const VarInfo& info, LValExpr* lv);
  bool constExprIsFloat(Expr* e);
  void flattenInit(InitVal* init, BaseType base, const std::vector<int>& dims,
                   std::vector<int32_t>& out, int& idx, int total, int depth);
  int32_t evalConstInt(Expr* e);
  float evalConstFloat(Expr* e);
};

} // namespace core_ir
