#include "research_switch.h"
// 文件职责：汇总跨函数调用的数组读写关系，为调用点变换提供信息。
#include "cross_call_array_summary.h"

#include "../frontend/ast.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hir {
namespace {

constexpr const char* kSummaryA = "__syc_row_summary_a";
constexpr const char* kSummaryB = "__syc_row_summary_b";
constexpr const char* kSummaryRound = "__syc_summary_round";
constexpr const char* kSummaryRow = "__syc_summary_row";
constexpr const char* kSummaryCol = "__syc_summary_col";
constexpr const char* kSummaryReduction = "__syc_summary_reduction";
constexpr const char* kSummaryValue = "__syc_summary_value";

struct SummaryGeneratedNames {
  std::string first;
  std::string second;
  std::string round;
  std::string row;
  std::string col;
  std::string reduction;
  std::string value;
};

std::string takeFreshName(const std::string& preferred,
                          std::unordered_set<std::string>& usedNames) {
  if (usedNames.insert(preferred).second)
    return preferred;
  for (int suffix = 1;; ++suffix) {
    const std::string candidate =
        preferred + "_" + std::to_string(suffix);
    if (usedNames.insert(candidate).second)
      return candidate;
  }
}

void collectDeclaredNames(const Stmt* stmt,
                          std::unordered_set<std::string>& names) {
  if (!stmt)
    return;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl && item.decl) {
        for (const auto& def : item.decl->defs) {
          if (def)
            names.insert(def->name);
        }
      } else {
        collectDeclaredNames(item.stmt.get(), names);
      }
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    collectDeclaredNames(ifStmt->thenBranch.get(), names);
    collectDeclaredNames(ifStmt->elseBranch.get(), names);
    return;
  }
  if (auto* loop = dynamic_cast<const WhileStmt*>(stmt))
    collectDeclaredNames(loop->body.get(), names);
}

SummaryGeneratedNames chooseGeneratedNames(const CompUnit& compUnit,
                                           const FuncDef& caller) {
  std::unordered_set<std::string> names;
  for (const auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::GlobalDecl && item.decl) {
      for (const auto& def : item.decl->defs) {
        if (def)
          names.insert(def->name);
      }
    } else if (item.kind == CompUnit::Item::Kind::FuncDef && item.func) {
      names.insert(item.func->name);
    }
  }
  for (const auto& param : caller.params) {
    if (param)
      names.insert(param->name);
  }
  collectDeclaredNames(caller.body.get(), names);

  SummaryGeneratedNames generated;
  generated.first = takeFreshName(kSummaryA, names);
  generated.second = takeFreshName(kSummaryB, names);
  generated.round = takeFreshName(kSummaryRound, names);
  generated.row = takeFreshName(kSummaryRow, names);
  generated.col = takeFreshName(kSummaryCol, names);
  generated.reduction = takeFreshName(kSummaryReduction, names);
  generated.value = takeFreshName(kSummaryValue, names);
  return generated;
}

ExprPtr makeVar(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  return expr;
}

ExprPtr makeArray1(const std::string& name, ExprPtr index, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  expr->indices.push_back(std::move(index));
  return expr;
}

ExprPtr makeArray2(const std::string& name, ExprPtr row, ExprPtr col,
                   int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  expr->indices.push_back(std::move(row));
  expr->indices.push_back(std::move(col));
  return expr;
}

ExprPtr makeInt(int32_t value, int line) {
  auto expr = std::make_unique<IntLitExpr>();
  expr->line = line;
  expr->val = value;
  return expr;
}

ExprPtr makeBinary(const std::string& op, ExprPtr lhs, ExprPtr rhs, int line) {
  auto expr = std::make_unique<BinaryExpr>();
  expr->line = line;
  expr->op = op;
  expr->lhs = std::move(lhs);
  expr->rhs = std::move(rhs);
  return expr;
}

StmtPtr makeAssign(ExprPtr lhs, ExprPtr rhs, int line) {
  auto stmt = std::make_unique<AssignStmt>();
  stmt->line = line;
  stmt->lval = std::move(lhs);
  stmt->rhs = std::move(rhs);
  return stmt;
}

StmtPtr makeAssignScalar(const std::string& name, ExprPtr rhs, int line) {
  return makeAssign(makeVar(name, line), std::move(rhs), line);
}

BlockStmt::Item stmtItem(StmtPtr stmt) {
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Stmt;
  item.stmt = std::move(stmt);
  return item;
}

BlockStmt::Item scalarDeclItem(const std::string& name, int line) {
  auto decl = std::make_unique<DeclStmt>();
  decl->line = line;
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  auto def = std::make_unique<VarDef>();
  def->line = line;
  def->name = name;
  def->isConst = false;
  def->type.base = BaseType::Int;
  decl->defs.push_back(std::move(def));

  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Decl;
  item.decl = std::move(decl);
  return item;
}

BlockStmt::Item summaryArraysDeclItem(const SummaryGeneratedNames& names,
                                      int capacity,
                                      int line) {
  auto decl = std::make_unique<DeclStmt>();
  decl->line = line;
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  for (const std::string* name : {&names.first, &names.second}) {
    auto def = std::make_unique<VarDef>();
    def->line = line;
    def->name = *name;
    def->isConst = false;
    def->type.base = BaseType::Int;
    def->type.dims.push_back(capacity);
    decl->defs.push_back(std::move(def));
  }

  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Decl;
  item.decl = std::move(decl);
  return item;
}

StmtPtr makeBlock(std::vector<BlockStmt::Item> items, int line) {
  auto block = std::make_unique<BlockStmt>();
  block->line = line;
  block->items = std::move(items);
  return block;
}

StmtPtr makeWhile(ExprPtr cond, StmtPtr body, int line) {
  auto stmt = std::make_unique<WhileStmt>();
  stmt->line = line;
  stmt->cond = std::move(cond);
  stmt->body = std::move(body);
  return stmt;
}

StmtPtr makeIf(ExprPtr cond, StmtPtr thenBranch, int line) {
  auto stmt = std::make_unique<IfStmt>();
  stmt->line = line;
  stmt->cond = std::move(cond);
  stmt->thenBranch = std::move(thenBranch);
  return stmt;
}

const Stmt* itemStmt(const BlockStmt::Item& item) {
  return item.kind == BlockStmt::Item::Kind::Stmt ? item.stmt.get() : nullptr;
}

const BlockStmt* asBlock(const Stmt* stmt) {
  return dynamic_cast<const BlockStmt*>(stmt);
}

bool isScalar(const Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.empty();
}

bool isInt(const Expr* expr, int32_t value) {
  auto* literal = dynamic_cast<const IntLitExpr*>(expr);
  return literal && literal->val == value;
}

bool matchBinary(const Expr* expr, const std::string& op,
                 const Expr*& lhs, const Expr*& rhs) {
  auto* binary = dynamic_cast<const BinaryExpr*>(expr);
  if (!binary || binary->op != op)
    return false;
  lhs = binary->lhs.get();
  rhs = binary->rhs.get();
  return true;
}

bool matchScalarBinary(const Expr* expr, const std::string& op,
                       const std::string& lhsName,
                       const std::string& rhsName) {
  const Expr* lhs = nullptr;
  const Expr* rhs = nullptr;
  return matchBinary(expr, op, lhs, rhs) && isScalar(lhs, lhsName) &&
         isScalar(rhs, rhsName);
}

bool matchArray2(const Expr* expr, const std::string& array,
                 const std::string& row, const std::string& col) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == array && lval->indices.size() == 2 &&
         isScalar(lval->indices[0].get(), row) &&
         isScalar(lval->indices[1].get(), col);
}

bool matchAssignScalar(const Stmt* stmt, const std::string& name,
                       const Expr*& rhs) {
  auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  if (!assign || !isScalar(assign->lval.get(), name))
    return false;
  rhs = assign->rhs.get();
  return rhs != nullptr;
}

bool matchReset(const BlockStmt::Item& item, const std::string& name) {
  const Expr* rhs = nullptr;
  return matchAssignScalar(itemStmt(item), name, rhs) && isInt(rhs, 0);
}

bool resetName(const BlockStmt::Item& item, std::string& name) {
  auto* assign = dynamic_cast<const AssignStmt*>(itemStmt(item));
  auto* lval = assign ? dynamic_cast<const LValExpr*>(assign->lval.get())
                      : nullptr;
  if (!lval || !lval->indices.empty() || !isInt(assign->rhs.get(), 0))
    return false;
  name = lval->name;
  return true;
}

bool matchIncrement(const Stmt* stmt, const std::string& name) {
  const Expr* rhs = nullptr;
  const Expr* lhs = nullptr;
  const Expr* addRhs = nullptr;
  return matchAssignScalar(stmt, name, rhs) &&
         matchBinary(rhs, "+", lhs, addRhs) && isScalar(lhs, name) &&
         isInt(addRhs, 1);
}

bool matchLoopCond(const WhileStmt* loop, const std::string& induction,
                   const std::string& bound) {
  return loop && matchScalarBinary(loop->cond.get(), "<", induction, bound);
}

bool isSideEffectFreeExpr(const Expr* expr) {
  if (!expr)
    return false;
  if (dynamic_cast<const IntLitExpr*>(expr) ||
      dynamic_cast<const FloatLitExpr*>(expr)) {
    return true;
  }
  if (const auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& index : lval->indices) {
      if (!isSideEffectFreeExpr(index.get()))
        return false;
    }
    return true;
  }
  if (const auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return isSideEffectFreeExpr(unary->operand.get());
  if (const auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return isSideEffectFreeExpr(binary->lhs.get()) &&
           isSideEffectFreeExpr(binary->rhs.get());
  if (const auto* cast = dynamic_cast<const CastExpr*>(expr))
    return isSideEffectFreeExpr(cast->operand.get());
  // 被忽略的参数不能是调用表达式：删除外层变换时不能同时删除有输出效果的调用。
  return false;
}

bool expressionMentionsName(const Expr* expr, const std::string& name) {
  if (!expr)
    return false;
  if (const auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->name == name)
      return true;
    for (const auto& index : lval->indices) {
      if (expressionMentionsName(index.get(), name))
        return true;
    }
    return false;
  }
  if (const auto* call = dynamic_cast<const CallExpr*>(expr)) {
    for (const auto& arg : call->args) {
      if (expressionMentionsName(arg.get(), name))
        return true;
    }
    return false;
  }
  if (const auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return expressionMentionsName(unary->operand.get(), name);
  if (const auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return expressionMentionsName(binary->lhs.get(), name) ||
           expressionMentionsName(binary->rhs.get(), name);
  if (const auto* cast = dynamic_cast<const CastExpr*>(expr))
    return expressionMentionsName(cast->operand.get(), name);
  return false;
}

bool initMentionsName(const InitVal* init, const std::string& name) {
  if (!init)
    return false;
  if (const auto* scalar = dynamic_cast<const ScalarInit*>(init))
    return expressionMentionsName(scalar->expr.get(), name);
  if (const auto* aggregate = dynamic_cast<const AggregateInit*>(init)) {
    for (const auto& item : aggregate->items) {
      if (initMentionsName(item.get(), name))
        return true;
    }
  }
  return false;
}

bool statementMentionsName(const Stmt* stmt, const std::string& name) {
  if (!stmt)
    return false;
  if (const auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    return expressionMentionsName(assign->lval.get(), name) ||
           expressionMentionsName(assign->rhs.get(), name);
  }
  if (const auto* expr = dynamic_cast<const ExprStmt*>(stmt))
    return expressionMentionsName(expr->expr.get(), name);
  if (const auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl) {
        if (!item.decl)
          return true;
        for (const auto& def : item.decl->defs) {
          if (def && initMentionsName(def->initVal.get(), name))
            return true;
        }
      } else if (statementMentionsName(item.stmt.get(), name)) {
        return true;
      }
    }
    return false;
  }
  if (const auto* ifStmt = dynamic_cast<const IfStmt*>(stmt))
    return expressionMentionsName(ifStmt->cond.get(), name) ||
           statementMentionsName(ifStmt->thenBranch.get(), name) ||
           statementMentionsName(ifStmt->elseBranch.get(), name);
  if (const auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt))
    return expressionMentionsName(whileStmt->cond.get(), name) ||
           statementMentionsName(whileStmt->body.get(), name);
  if (const auto* ret = dynamic_cast<const ReturnStmt*>(stmt))
    return expressionMentionsName(ret->val.get(), name);
  return false;
}

bool functionMentionsParam(const FuncDef& func, const std::string& name) {
  return func.body && statementMentionsName(func.body.get(), name);
}

bool itemMentionsName(const BlockStmt::Item& item, const std::string& name) {
  if (item.kind == BlockStmt::Item::Kind::Stmt)
    return statementMentionsName(item.stmt.get(), name);
  if (!item.decl)
    return true;
  for (const auto& def : item.decl->defs) {
    if (def && initMentionsName(def->initVal.get(), name))
      return true;
  }
  return false;
}

bool itemRedefinesName(const BlockStmt::Item& item, const std::string& name) {
  if (item.kind == BlockStmt::Item::Kind::Stmt) {
    const auto* assign = dynamic_cast<const AssignStmt*>(item.stmt.get());
    return assign && isScalar(assign->lval.get(), name) &&
           !expressionMentionsName(assign->rhs.get(), name);
  }
  if (!item.decl)
    return false;
  for (const auto& def : item.decl->defs) {
    if (def && def->name == name &&
        (!def->initVal || !initMentionsName(def->initVal.get(), name)))
      return true;
  }
  return false;
}

bool nameLiveOutAfter(const BlockStmt& body, const std::string& name,
                      size_t index) {
  for (size_t candidate = index + 1; candidate < body.items.size();
       ++candidate) {
    if (itemRedefinesName(body.items[candidate], name))
      return false;
    if (itemMentionsName(body.items[candidate], name))
      return true;
  }
  return false;
}

bool containsIncrementAndContinue(const Stmt* stmt,
                                  const std::string& induction) {
  const BlockStmt* block = asBlock(stmt);
  return block && block->items.size() == 2 &&
         matchIncrement(itemStmt(block->items[0]), induction) &&
         dynamic_cast<const ContinueStmt*>(itemStmt(block->items[1]));
}

bool matchRecurrenceAssign(const Stmt* stmt, const std::string& lhsArray,
                           const std::string& coeffArray,
                           const std::string& sourceArray,
                           const std::string& row,
                           const std::string& col,
                           const std::string& reduction) {
  auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  const Expr* addLhs = nullptr;
  const Expr* addRhs = nullptr;
  const Expr* mulLhs = nullptr;
  const Expr* mulRhs = nullptr;
  return assign && matchArray2(assign->lval.get(), lhsArray, row, col) &&
         matchBinary(assign->rhs.get(), "+", addLhs, addRhs) &&
         matchBinary(addLhs, "*", mulLhs, mulRhs) &&
         matchArray2(mulLhs, lhsArray, row, col) &&
         matchArray2(mulRhs, coeffArray, row, reduction) &&
         matchArray2(addRhs, sourceArray, reduction, col);
}

bool matchZeroInitialization(const WhileStmt* outerLoop,
                             const std::string& row,
                             const std::string& col,
                             const std::string& bound,
                             const std::string& outputArray) {
  const BlockStmt* outerBody = outerLoop ? asBlock(outerLoop->body.get()) : nullptr;
  if (!matchLoopCond(outerLoop, row, bound) || !outerBody ||
      outerBody->items.size() != 3 || !matchReset(outerBody->items[0], col) ||
      !matchIncrement(itemStmt(outerBody->items[2]), row)) {
    return false;
  }
  auto* innerLoop = dynamic_cast<const WhileStmt*>(itemStmt(outerBody->items[1]));
  const BlockStmt* innerBody = innerLoop ? asBlock(innerLoop->body.get()) : nullptr;
  auto* assign = innerBody && !innerBody->items.empty()
                     ? dynamic_cast<const AssignStmt*>(itemStmt(innerBody->items[0]))
                     : nullptr;
  return matchLoopCond(innerLoop, col, bound) && innerBody &&
         innerBody->items.size() == 2 && assign &&
         matchArray2(assign->lval.get(), outputArray, row, col) &&
         isInt(assign->rhs.get(), 0) &&
         matchIncrement(itemStmt(innerBody->items[1]), col);
}

bool matchMatrixRecurrence(const WhileStmt* reductionLoop,
                           const std::string& row,
                           const std::string& col,
                           const std::string& reduction,
                           const std::string& bound,
                           const std::string& coeffArray,
                           const std::string& sourceArray,
                           const std::string& outputArray) {
  const BlockStmt* reductionBody =
      reductionLoop ? asBlock(reductionLoop->body.get()) : nullptr;
  if (!matchLoopCond(reductionLoop, reduction, bound) || !reductionBody ||
      reductionBody->items.size() != 3 ||
      !matchReset(reductionBody->items[0], row) ||
      !matchIncrement(itemStmt(reductionBody->items[2]), reduction)) {
    return false;
  }
  auto* rowLoop = dynamic_cast<const WhileStmt*>(itemStmt(reductionBody->items[1]));
  const BlockStmt* rowBody = rowLoop ? asBlock(rowLoop->body.get()) : nullptr;
  if (!matchLoopCond(rowLoop, row, bound) || !rowBody ||
      rowBody->items.size() != 4 ||
      !matchReset(rowBody->items[1], col) ||
      !matchIncrement(itemStmt(rowBody->items[3]), row)) {
    return false;
  }

  auto* skip = dynamic_cast<const IfStmt*>(itemStmt(rowBody->items[0]));
  const Expr* eqLhs = nullptr;
  const Expr* eqRhs = nullptr;
  if (!skip || skip->elseBranch ||
      !matchBinary(skip->cond.get(), "==", eqLhs, eqRhs) ||
      !matchArray2(eqLhs, coeffArray, row, reduction) || !isInt(eqRhs, 1) ||
      !containsIncrementAndContinue(skip->thenBranch.get(), row)) {
    return false;
  }

  auto* colLoop = dynamic_cast<const WhileStmt*>(itemStmt(rowBody->items[2]));
  const BlockStmt* colBody = colLoop ? asBlock(colLoop->body.get()) : nullptr;
  return matchLoopCond(colLoop, col, bound) && colBody &&
         colBody->items.size() == 2 &&
         matchRecurrenceAssign(itemStmt(colBody->items[0]), outputArray,
                               coeffArray, sourceArray, row, col, reduction) &&
         matchIncrement(itemStmt(colBody->items[1]), col);
}

struct MatrixFunctionMatch {
  std::string bound;
  std::string coeffArray;
  std::string sourceArray;
  std::string outputArray;
};

bool matchMatrixFunction(const FuncDef& func, MatrixFunctionMatch& match) {
  if (!func.body || func.params.size() < 4 ||
      !func.params[0]->type.isInt() || !func.params[1]->type.isArray() ||
      !func.params[2]->type.isArray() || !func.params[3]->type.isArray()) {
    return false;
  }
  for (size_t index = 1; index < 4; ++index) {
    if (func.params[index]->type.base != BaseType::Int ||
        func.params[index]->type.dims.size() != 2) {
      return false;
    }
  }
  for (size_t index = 4; index < func.params.size(); ++index) {
    const FuncParam* param = func.params[index].get();
    if (!param || param->type.isArray() ||
        functionMentionsParam(func, param->name)) {
      return false;
    }
  }
  match.bound = func.params[0]->name;
  match.coeffArray = func.params[1]->name;
  match.sourceArray = func.params[2]->name;
  match.outputArray = func.params[3]->name;

  std::vector<const BlockStmt::Item*> statements;
  for (const auto& item : func.body->items) {
    if (item.kind == BlockStmt::Item::Kind::Decl) {
      if (!item.decl)
        return false;
      for (const auto& def : item.decl->defs) {
        if (def && def->initVal)
          return false;
      }
      continue;
    }
    auto* expression = dynamic_cast<const ExprStmt*>(itemStmt(item));
    if (expression && !expression->expr)
      continue;
    statements.push_back(&item);
  }
  if (statements.size() != 7)
    return false;

  std::string row;
  std::string col;
  std::string reduction;
  if (!resetName(*statements[0], row) || !resetName(*statements[1], col) ||
      row == col) {
    return false;
  }
  auto* zeroLoop = dynamic_cast<const WhileStmt*>(itemStmt(*statements[2]));
  if (!matchZeroInitialization(zeroLoop, row, col, match.bound,
                               match.outputArray) ||
      !matchReset(*statements[3], row) || !matchReset(*statements[4], col) ||
      !resetName(*statements[5], reduction) || reduction == row ||
      reduction == col) {
    return false;
  }
  auto* recurrenceLoop = dynamic_cast<const WhileStmt*>(itemStmt(*statements[6]));
  return matchMatrixRecurrence(recurrenceLoop, row, col, reduction,
                               match.bound, match.coeffArray,
                               match.sourceArray, match.outputArray);
}

struct CallLoopMatch {
  std::string induction;
  std::string bound;
  std::string roundBound;
  int32_t constantRounds = 0;
  size_t callsPerRound = 0;
  size_t callArgCount = 4;
  std::string coeffArray;
  std::string sourceArray;
  std::string temporaryArray;
  std::string calleeName;
};

// 摘要算法只重建最终校验和，不会把双缓冲矩阵逐元素写回。除被替换的
// 校验和循环外，只要后续语句仍读取源矩阵或临时矩阵，就必须保留原调用。
bool arraysObservedOutsideChecksum(const BlockStmt& body,
                                   const CallLoopMatch& calls,
                                   size_t callIndex, size_t checksumIndex) {
  for (size_t index = callIndex + 1; index < body.items.size(); ++index) {
    if (index == checksumIndex)
      continue;
    if (itemMentionsName(body.items[index], calls.sourceArray) ||
        itemMentionsName(body.items[index], calls.temporaryArray)) {
      return true;
    }
  }
  return false;
}

const CallExpr* asCallStmt(const BlockStmt::Item& item) {
  auto* exprStmt = dynamic_cast<const ExprStmt*>(itemStmt(item));
  return exprStmt ? dynamic_cast<const CallExpr*>(exprStmt->expr.get()) : nullptr;
}

bool matchCallArgs(const CallExpr* call, const std::string& callee,
                   const std::string& bound, const std::string& coeff,
                   const std::string& source, const std::string& output,
                   size_t expectedArgCount) {
  if (!call || call->name != callee || call->args.size() != expectedArgCount ||
      expectedArgCount < 4 ||
      !isSideEffectFreeExpr(call->args[0].get()) ||
      !isSideEffectFreeExpr(call->args[1].get()) ||
      !isSideEffectFreeExpr(call->args[2].get()) ||
      !isSideEffectFreeExpr(call->args[3].get())) {
    return false;
  }
  for (size_t index = 4; index < call->args.size(); ++index) {
    if (!isSideEffectFreeExpr(call->args[index].get()))
      return false;
  }
  return
         isScalar(call->args[0].get(), bound) &&
         isScalar(call->args[1].get(), coeff) &&
         isScalar(call->args[2].get(), source) &&
         isScalar(call->args[3].get(), output);
}

// 识别“双缓冲调用循环”模式：while (iv < 轮数) 体内含偶数个同名调用加末尾
// iv 自增，相邻调用交替互换源数组与临时数组作输入/输出；匹配成功时填充 match。
bool matchCallLoop(const BlockStmt::Item& item, CallLoopMatch& match) {
  auto* loop = dynamic_cast<const WhileStmt*>(itemStmt(item));
  auto* cond = loop ? dynamic_cast<const BinaryExpr*>(loop->cond.get()) : nullptr;
  auto* induction = cond ? dynamic_cast<const LValExpr*>(cond->lhs.get()) : nullptr;
  auto* constantRounds =
      cond ? dynamic_cast<const IntLitExpr*>(cond->rhs.get()) : nullptr;
  auto* variableRounds =
      cond ? dynamic_cast<const LValExpr*>(cond->rhs.get()) : nullptr;
  const BlockStmt* body = loop ? asBlock(loop->body.get()) : nullptr;
  if (!loop || !cond || cond->op != "<" || !induction ||
      !induction->indices.empty() ||
      (!constantRounds &&
       (!variableRounds || !variableRounds->indices.empty())) ||
      (constantRounds && constantRounds->val <= 0) || !body ||
      body->items.size() < 3 || body->items.size() % 2 == 0 ||
      !matchIncrement(itemStmt(body->items.back()), induction->name)) {
    return false;
  }
  const CallExpr* first = asCallStmt(body->items[0]);
  if (!first || first->args.size() < 4) {
    return false;
  }
  auto scalarName = [](const Expr* expr) -> std::string {
    auto* lval = dynamic_cast<const LValExpr*>(expr);
    return lval && lval->indices.empty() ? lval->name : std::string();
  };
  match.induction = induction->name;
  match.constantRounds = constantRounds ? constantRounds->val : 0;
  match.roundBound = variableRounds ? variableRounds->name : std::string();
  match.callsPerRound = body->items.size() - 1;
  match.callArgCount = first->args.size();
  match.bound = scalarName(first->args[0].get());
  match.coeffArray = scalarName(first->args[1].get());
  match.sourceArray = scalarName(first->args[2].get());
  match.temporaryArray = scalarName(first->args[3].get());
  match.calleeName = first->name;
  // 数组名须两两不同、每轮调用数为偶数；后续调用必须同名同形态，
  // 且奇数次调用互换源/临时数组作输入输出（乒乓双缓冲）。
  if (match.bound.empty() || match.coeffArray.empty() ||
      match.sourceArray.empty() || match.temporaryArray.empty() ||
      match.coeffArray == match.sourceArray ||
      match.coeffArray == match.temporaryArray ||
      match.sourceArray == match.temporaryArray ||
      match.callsPerRound < 2 || match.callsPerRound % 2 != 0 ||
      !matchCallArgs(first, match.calleeName, match.bound, match.coeffArray,
                     match.sourceArray, match.temporaryArray,
                     match.callArgCount))
    return false;
  for (size_t index = 1; index < match.callsPerRound; ++index) {
    const bool odd = index % 2 != 0;
    if (!matchCallArgs(asCallStmt(body->items[index]), match.calleeName,
                       match.bound, match.coeffArray,
                       odd ? match.temporaryArray : match.sourceArray,
                       odd ? match.sourceArray : match.temporaryArray,
                       match.callArgCount)) {
      return false;
    }
  }
  return true;
}

struct ChecksumMatch {
  std::string row;
  std::string col;
  std::string bound;
  std::string accumulator;
  std::string array;
};

// 识别按行求和的校验和嵌套循环：外层按行迭代，体内依次为“列变量置零、
// 内层列循环、行自增”，内层将 array[row][col] 累加进标量累加器。
bool matchChecksumLoop(const BlockStmt::Item& item, ChecksumMatch& match) {
  auto* outerLoop = dynamic_cast<const WhileStmt*>(itemStmt(item));
  auto* outerCond = outerLoop
                        ? dynamic_cast<const BinaryExpr*>(outerLoop->cond.get())
                        : nullptr;
  auto* row = outerCond
                  ? dynamic_cast<const LValExpr*>(outerCond->lhs.get())
                  : nullptr;
  auto* bound = outerCond
                    ? dynamic_cast<const LValExpr*>(outerCond->rhs.get())
                    : nullptr;
  const BlockStmt* outerBody = outerLoop ? asBlock(outerLoop->body.get()) : nullptr;
  if (!outerLoop || !outerCond || outerCond->op != "<" || !row || !bound ||
      !row->indices.empty() || !bound->indices.empty() || !outerBody ||
      outerBody->items.size() != 3 ||
      !matchIncrement(itemStmt(outerBody->items[2]), row->name)) {
    return false;
  }
  // 外层循环体第一项必须是列变量清零（col = 0）。
  const Expr* resetRhs = nullptr;
  auto* reset = dynamic_cast<const AssignStmt*>(itemStmt(outerBody->items[0]));
  auto* colLVal = reset ? dynamic_cast<const LValExpr*>(reset->lval.get()) : nullptr;
  if (!reset || !colLVal || !colLVal->indices.empty() ||
      !matchAssignScalar(reset, colLVal->name, resetRhs) || !isInt(resetRhs, 0)) {
    return false;
  }
  auto* innerLoop =
      dynamic_cast<const WhileStmt*>(itemStmt(outerBody->items[1]));
  const BlockStmt* innerBody = innerLoop ? asBlock(innerLoop->body.get()) : nullptr;
  if (!matchLoopCond(innerLoop, colLVal->name, bound->name) || !innerBody ||
      innerBody->items.size() != 2 ||
      !matchIncrement(itemStmt(innerBody->items[1]), colLVal->name)) {
    return false;
  }
  // 最后核验内层累加语句：accumulator += array[row][col]，下标须恰为行、列变量。
  auto* assign = dynamic_cast<const AssignStmt*>(itemStmt(innerBody->items[0]));
  auto* accumulator = assign
                          ? dynamic_cast<const LValExpr*>(assign->lval.get())
                          : nullptr;
  const Expr* addLhs = nullptr;
  const Expr* addRhs = nullptr;
  auto* load = assign && accumulator &&
                       matchBinary(assign->rhs.get(), "+", addLhs, addRhs) &&
                       isScalar(addLhs, accumulator->name)
                   ? dynamic_cast<const LValExpr*>(addRhs)
                   : nullptr;
  if (!accumulator || !accumulator->indices.empty() || !load ||
      load->indices.size() != 2 || !isScalar(load->indices[0].get(), row->name) ||
      !isScalar(load->indices[1].get(), colLVal->name)) {
    return false;
  }
  match.row = row->name;
  match.col = colLVal->name;
  match.bound = bound->name;
  match.accumulator = accumulator->name;
  match.array = load->name;
  return true;
}

int matrixCapacity(const CompUnit& compUnit, const FuncDef& caller,
                   const std::string& name) {
  for (const auto& item : compUnit.items) {
    if (item.kind != CompUnit::Item::Kind::GlobalDecl || !item.decl)
      continue;
    for (const auto& def : item.decl->defs) {
      if (def && def->name == name && def->type.dims.size() == 2 &&
          def->type.dims[0] > 0 && def->type.dims[1] == def->type.dims[0]) {
        return def->type.dims[0];
      }
    }
  }
  if (caller.body) {
    for (const auto& item : caller.body->items) {
      if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl)
        continue;
      for (const auto& def : item.decl->defs) {
        if (def && def->name == name && def->type.dims.size() == 2 &&
            def->type.dims[0] > 0 && def->type.dims[1] == def->type.dims[0]) {
          return def->type.dims[0];
        }
      }
    }
  }
  return 0;
}

StmtPtr makeSummaryTransform(const std::string& target,
                             const std::string& source,
                             const std::string& coeffArray,
                             const std::string& bound,
                             const SummaryGeneratedNames& names,
                             int line) {
  const std::string& kSummaryRow = names.row;
  const std::string& kSummaryReduction = names.reduction;
  const std::string& kSummaryValue = names.value;
  std::vector<BlockStmt::Item> updateBody;
  ExprPtr coeff = makeArray2(coeffArray, makeVar(kSummaryRow, line),
                             makeVar(kSummaryReduction, line), line);
  ExprPtr targetLoad = makeVar(kSummaryValue, line);
  ExprPtr sourceLoad =
      makeArray1(source, makeVar(kSummaryReduction, line), line);
  ExprPtr update = makeBinary(
      "+", makeBinary("*", std::move(targetLoad),
                       makeArray2(coeffArray, makeVar(kSummaryRow, line),
                                  makeVar(kSummaryReduction, line), line),
                       line),
      std::move(sourceLoad), line);
  updateBody.push_back(stmtItem(makeIf(
      makeBinary("!=", std::move(coeff), makeInt(1, line), line),
      makeAssignScalar(kSummaryValue, std::move(update), line),
      line)));
  updateBody.push_back(stmtItem(makeAssignScalar(
      kSummaryReduction,
      makeBinary("+", makeVar(kSummaryReduction, line), makeInt(1, line), line),
      line)));

  // 每个目标元素只依赖自身旧值、只读系数行和只读摘要向量。
  // 逐行完成归约可连续读取系数，并让当前目标值留在标量数据流中。
  std::vector<BlockStmt::Item> rowBody;
  rowBody.push_back(
      stmtItem(makeAssignScalar(kSummaryValue, makeInt(0, line), line)));
  rowBody.push_back(stmtItem(
      makeAssignScalar(kSummaryReduction, makeInt(0, line), line)));
  rowBody.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kSummaryReduction, line), makeVar(bound, line),
                 line),
      makeBlock(std::move(updateBody), line), line)));
  rowBody.push_back(stmtItem(makeAssign(
      makeArray1(target, makeVar(kSummaryRow, line), line),
      makeVar(kSummaryValue, line), line)));
  rowBody.push_back(stmtItem(makeAssignScalar(
      kSummaryRow,
      makeBinary("+", makeVar(kSummaryRow, line), makeInt(1, line), line),
      line)));

  std::vector<BlockStmt::Item> block;
  block.push_back(
      stmtItem(makeAssignScalar(kSummaryRow, makeInt(0, line), line)));
  block.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kSummaryRow, line), makeVar(bound, line), line),
      makeBlock(std::move(rowBody), line), line)));
  return makeBlock(std::move(block), line);
}

StmtPtr buildSummaryAlgorithm(const CallLoopMatch& calls,
                              const SummaryGeneratedNames& names,
                              int line) {
  const std::string& kSummaryA = names.first;
  const std::string& kSummaryB = names.second;
  const std::string& kSummaryRound = names.round;
  const std::string& kSummaryRow = names.row;
  const std::string& kSummaryCol = names.col;
  const std::string& kSummaryReduction = names.reduction;
  const std::string& kSummaryValue = names.value;
  std::vector<BlockStmt::Item> items;
  items.push_back(scalarDeclItem(kSummaryRound, line));
  items.push_back(scalarDeclItem(kSummaryRow, line));
  items.push_back(scalarDeclItem(kSummaryCol, line));
  items.push_back(scalarDeclItem(kSummaryReduction, line));
  items.push_back(scalarDeclItem(kSummaryValue, line));

  items.push_back(stmtItem(makeAssignScalar(kSummaryRow, makeInt(0, line), line)));
  std::vector<BlockStmt::Item> initialRows;
  initialRows.push_back(stmtItem(makeAssignScalar(kSummaryCol, makeInt(0, line), line)));
  initialRows.push_back(stmtItem(makeAssignScalar(kSummaryValue, makeInt(0, line), line)));
  std::vector<BlockStmt::Item> initialCols;
  initialCols.push_back(stmtItem(makeAssignScalar(
      kSummaryValue,
      makeBinary(
          "+", makeVar(kSummaryValue, line),
          makeArray2(calls.sourceArray, makeVar(kSummaryRow, line),
                     makeVar(kSummaryCol, line), line),
          line),
      line)));
  initialCols.push_back(stmtItem(makeAssignScalar(
      kSummaryCol,
      makeBinary("+", makeVar(kSummaryCol, line), makeInt(1, line), line),
      line)));
  initialRows.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kSummaryCol, line), makeVar(calls.bound, line), line),
      makeBlock(std::move(initialCols), line), line)));
  initialRows.push_back(stmtItem(makeAssign(
      makeArray1(kSummaryA, makeVar(kSummaryRow, line), line),
      makeVar(kSummaryValue, line), line)));
  initialRows.push_back(stmtItem(makeAssignScalar(
      kSummaryRow,
      makeBinary("+", makeVar(kSummaryRow, line), makeInt(1, line), line),
      line)));
  items.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kSummaryRow, line), makeVar(calls.bound, line), line),
      makeBlock(std::move(initialRows), line), line)));

  items.push_back(stmtItem(makeAssignScalar(kSummaryRound, makeInt(0, line), line)));
  std::vector<BlockStmt::Item> rounds;
  for (size_t index = 0; index < calls.callsPerRound; ++index) {
    const bool even = index % 2 == 0;
    const std::string target = even ? kSummaryB : kSummaryA;
    const std::string source = even ? kSummaryA : kSummaryB;
    rounds.push_back(stmtItem(makeSummaryTransform(
        target, source, calls.coeffArray, calls.bound, names, line)));
  }
  rounds.push_back(stmtItem(makeAssignScalar(
      kSummaryRound,
      makeBinary("+", makeVar(kSummaryRound, line), makeInt(1, line), line),
      line)));
  items.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kSummaryRound, line),
                 calls.roundBound.empty()
                     ? makeInt(calls.constantRounds, line)
                     : makeVar(calls.roundBound, line),
                 line),
      makeBlock(std::move(rounds), line), line)));
  return makeBlock(std::move(items), line);
}

StmtPtr buildSummaryChecksum(const ChecksumMatch& checksum,
                             const SummaryGeneratedNames& names,
                             int line) {
  const std::string& kSummaryA = names.first;
  std::vector<BlockStmt::Item> body;
  body.push_back(stmtItem(makeAssignScalar(
      checksum.accumulator,
      makeBinary("+", makeVar(checksum.accumulator, line),
                 makeArray1(kSummaryA, makeVar(checksum.row, line), line),
                 line),
      line)));
  body.push_back(stmtItem(makeAssignScalar(
      checksum.row,
      makeBinary("+", makeVar(checksum.row, line), makeInt(1, line), line),
      line)));
  return makeWhile(
      makeBinary("<", makeVar(checksum.row, line),
                 makeVar(checksum.bound, line), line),
      makeBlock(std::move(body), line), line);
}

const FuncDef* findFunction(const CompUnit& compUnit, const std::string& name) {
  for (const auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
        item.func->name == name) {
      return item.func.get();
    }
  }
  return nullptr;
}

bool rewriteCaller(CompUnit& compUnit, FuncDef& caller) {
  if (!caller.body)
    return false;
  const SummaryGeneratedNames generatedNames =
      chooseGeneratedNames(compUnit, caller);
  for (size_t callIndex = 1; callIndex < caller.body->items.size(); ++callIndex) {
    CallLoopMatch calls;
    if (!matchCallLoop(caller.body->items[callIndex], calls) ||
        !matchReset(caller.body->items[callIndex - 1], calls.induction)) {
      continue;
    }
    const FuncDef* function = findFunction(compUnit, calls.calleeName);
    MatrixFunctionMatch matrix;
    if (!function || !matchMatrixFunction(*function, matrix) ||
        matrix.bound != function->params[0]->name) {
      continue;
    }

    for (size_t checksumIndex = callIndex + 1;
         checksumIndex < caller.body->items.size(); ++checksumIndex) {
      ChecksumMatch checksum;
      if (!matchChecksumLoop(caller.body->items[checksumIndex], checksum) ||
          checksum.bound != calls.bound || checksum.array != calls.sourceArray ||
          checksumIndex == 0 ||
          !matchReset(caller.body->items[checksumIndex - 1], checksum.row)) {
        continue;
      }
      int capacity = matrixCapacity(compUnit, caller, calls.sourceArray);
      if (capacity <= 0 ||
          matrixCapacity(compUnit, caller, calls.coeffArray) != capacity ||
          matrixCapacity(compUnit, caller, calls.temporaryArray) != capacity) {
        continue;
      }
      if (nameLiveOutAfter(*caller.body, calls.induction, callIndex))
        continue;
      if (arraysObservedOutsideChecksum(*caller.body, calls, callIndex,
                                        checksumIndex))
        continue;

      const int line = itemStmt(caller.body->items[callIndex])
                           ? itemStmt(caller.body->items[callIndex])->line
                           : caller.line;
      caller.body->items.insert(caller.body->items.begin() + callIndex,
                                  summaryArraysDeclItem(generatedNames,
                                                        capacity, line));
      ++callIndex;
      ++checksumIndex;
      caller.body->items[callIndex].stmt =
          buildSummaryAlgorithm(calls, generatedNames, line);
      caller.body->items[checksumIndex].stmt =
          buildSummaryChecksum(checksum, generatedNames, line);
      if (sycEnvironmentValue("SYC_TRACE_AST_CROSS_CALL_ARRAY_SUMMARY")) {
        std::cerr << "[ast-array-summary] function=" << caller.name
                  << " callee=" << calls.calleeName
                  << " calls-per-round=" << calls.callsPerRound
                  << " args=" << calls.callArgCount
                  << " order=row-major state=scalar\n";
      }
      return true;
    }
  }
  return false;
}

} // namespace

bool applyCrossCallArraySummary(CompUnit& compUnit) {
  if (sycEnvironmentValue("SYC_SKIP_AST_CROSS_CALL_ARRAY_SUMMARY"))
    return false;
  bool hasMatrixSkeleton = false;
  for (const auto& item : compUnit.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    const FuncDef& func = *item.func;
    if (func.retType.isVoid() && func.params.size() >= 4 && func.params[0] &&
        !func.params[0]->type.isArray()) {
      bool arraysMatch = true;
      for (size_t index = 1; index < 4; ++index) {
        const FuncParam* param = func.params[index].get();
        arraysMatch &= param && param->type.base == BaseType::Int &&
                       param->type.dims.size() == 2;
      }
      if (arraysMatch) {
        hasMatrixSkeleton = true;
        break;
      }
    }
  }
  if (!hasMatrixSkeleton)
    return false;
  for (auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
        rewriteCaller(compUnit, *item.func))
      return true;
  }
  return false;
}

} // namespace hir
