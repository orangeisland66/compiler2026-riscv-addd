#include "research_switch.h"
// 文件职责：根据后续使用情况整理循环中的按需复制和恢复操作。
#include "demand_driven_copy_transform.h"

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

constexpr const char* kDemandIndex = "__syc_demand_index";
constexpr const char* kOutputIndex = "__syc_demand_output";
constexpr const char* kCallIndex = "__syc_demand_call";
constexpr const char* kRowSize = "__syc_demand_rowsize";
constexpr const char* kColSize = "__syc_demand_colsize";
constexpr const char* kCutoffOuter = "__syc_demand_cutoff_outer";
constexpr const char* kCutoffInner = "__syc_demand_cutoff_inner";
constexpr const char* kActive = "__syc_demand_active";
constexpr const char* kDestOuter = "__syc_demand_dest_outer";
constexpr const char* kDestInner = "__syc_demand_dest_inner";
constexpr const char* kDemandValue = "__syc_demand_value";

struct DemandGeneratedNames {
  std::string demandIndex;
  std::string outputIndex;
  std::string callIndex;
  std::string rowSize;
  std::string colSize;
  std::string cutoffOuter;
  std::string cutoffInner;
  std::string active;
  std::string destOuter;
  std::string destInner;
  std::string demandValue;
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

DemandGeneratedNames chooseGeneratedNames(const CompUnit& compUnit,
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

  DemandGeneratedNames generated;
  generated.demandIndex = takeFreshName(kDemandIndex, names);
  generated.outputIndex = takeFreshName(kOutputIndex, names);
  generated.callIndex = takeFreshName(kCallIndex, names);
  generated.rowSize = takeFreshName(kRowSize, names);
  generated.colSize = takeFreshName(kColSize, names);
  generated.cutoffOuter = takeFreshName(kCutoffOuter, names);
  generated.cutoffInner = takeFreshName(kCutoffInner, names);
  generated.active = takeFreshName(kActive, names);
  generated.destOuter = takeFreshName(kDestOuter, names);
  generated.destInner = takeFreshName(kDestInner, names);
  generated.demandValue = takeFreshName(kDemandValue, names);
  return generated;
}

ExprPtr makeVar(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  return expr;
}

ExprPtr makeArray(const std::string& name, ExprPtr index, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  expr->indices.push_back(std::move(index));
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

// 克隆表达式并做两类替换：标量名命中时替换为替换表达式的克隆；
// 数组元素 arr[i] 的下标命中指定归纳变量时替换为对应替换表达式；
// 其余节点照常递归。
ExprPtr cloneExprReplacing(const Expr* expr, const std::string& scalarName,
                           const Expr* scalarReplacement,
                           const std::string* arrayName = nullptr,
                           const std::string* arrayIndex = nullptr,
                           const Expr* arrayReplacement = nullptr) {
  if (!expr)
    return nullptr;
  if (auto* literal = dynamic_cast<const IntLitExpr*>(expr)) {
    return makeInt(literal->val, literal->line);
  }
  if (auto* literal = dynamic_cast<const FloatLitExpr*>(expr)) {
    auto copy = std::make_unique<FloatLitExpr>();
    copy->val = literal->val;
    copy->line = literal->line;
    return copy;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->indices.empty() && lval->name == scalarName)
      return cloneExprReplacing(scalarReplacement, "", nullptr);
    auto* replacementIndex = lval->indices.size() == 1
                                 ? dynamic_cast<const LValExpr*>(
                                       lval->indices[0].get())
                                 : nullptr;
    if (arrayName && arrayIndex && arrayReplacement &&
        lval->name == *arrayName && replacementIndex &&
        replacementIndex->indices.empty() &&
        replacementIndex->name == *arrayIndex) {
      return cloneExprReplacing(arrayReplacement, "", nullptr);
    }
    auto copy = std::make_unique<LValExpr>();
    copy->name = lval->name;
    copy->line = lval->line;
    copy->symbol = lval->symbol;
    for (const auto& index : lval->indices) {
      copy->indices.push_back(cloneExprReplacing(
          index.get(), scalarName, scalarReplacement, arrayName, arrayIndex,
          arrayReplacement));
    }
    return copy;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    return makeBinary(
        binary->op,
        cloneExprReplacing(binary->lhs.get(), scalarName, scalarReplacement,
                           arrayName, arrayIndex, arrayReplacement),
        cloneExprReplacing(binary->rhs.get(), scalarName, scalarReplacement,
                           arrayName, arrayIndex, arrayReplacement),
        binary->line);
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    auto copy = std::make_unique<UnaryExpr>();
    copy->op = unary->op;
    copy->line = unary->line;
    copy->operand = cloneExprReplacing(
        unary->operand.get(), scalarName, scalarReplacement, arrayName,
        arrayIndex, arrayReplacement);
    return copy;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    auto copy = std::make_unique<CastExpr>();
    copy->targetType = cast->targetType;
    copy->line = cast->line;
    copy->operand = cloneExprReplacing(
        cast->operand.get(), scalarName, scalarReplacement, arrayName,
        arrayIndex, arrayReplacement);
    return copy;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    auto copy = std::make_unique<CallExpr>();
    copy->name = call->name;
    copy->callee = call->callee;
    copy->line = call->line;
    for (const auto& arg : call->args) {
      copy->args.push_back(cloneExprReplacing(
          arg.get(), scalarName, scalarReplacement, arrayName, arrayIndex,
          arrayReplacement));
    }
    return copy;
  }
  return nullptr;
}

bool isPureInductionExpression(const Expr* expr,
                               const std::string& induction) {
  if (!expr)
    return false;
  if (dynamic_cast<const IntLitExpr*>(expr) ||
      dynamic_cast<const FloatLitExpr*>(expr)) {
    return true;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr))
    return lval->indices.empty() && lval->name == induction;
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return isPureInductionExpression(binary->lhs.get(), induction) &&
           isPureInductionExpression(binary->rhs.get(), induction);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return isPureInductionExpression(unary->operand.get(), induction);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return isPureInductionExpression(cast->operand.get(), induction);
  return false;
}

StmtPtr makeAssign(const std::string& name, ExprPtr rhs, int line) {
  auto stmt = std::make_unique<AssignStmt>();
  stmt->line = line;
  stmt->lval = makeVar(name, line);
  stmt->rhs = std::move(rhs);
  return stmt;
}

BlockStmt::Item stmtItem(StmtPtr stmt) {
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Stmt;
  item.stmt = std::move(stmt);
  return item;
}

BlockStmt::Item intDeclItem(const std::string& name, ExprPtr init, int line) {
  auto decl = std::make_unique<DeclStmt>();
  decl->line = line;
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  auto def = std::make_unique<VarDef>();
  def->line = line;
  def->name = name;
  def->isConst = false;
  def->type.base = BaseType::Int;
  auto scalar = std::make_unique<ScalarInit>();
  scalar->line = line;
  scalar->expr = std::move(init);
  def->initVal = std::move(scalar);
  decl->defs.push_back(std::move(def));

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

StmtPtr makeIf(ExprPtr cond, StmtPtr thenBranch, StmtPtr elseBranch, int line) {
  auto stmt = std::make_unique<IfStmt>();
  stmt->line = line;
  stmt->cond = std::move(cond);
  stmt->thenBranch = std::move(thenBranch);
  stmt->elseBranch = std::move(elseBranch);
  return stmt;
}

StmtPtr makeWhile(ExprPtr cond, StmtPtr body, int line) {
  auto stmt = std::make_unique<WhileStmt>();
  stmt->line = line;
  stmt->cond = std::move(cond);
  stmt->body = std::move(body);
  return stmt;
}

StmtPtr makeEmptyStmt(int line) {
  auto stmt = std::make_unique<ExprStmt>();
  stmt->line = line;
  return stmt;
}

const BlockStmt* asBlock(const Stmt* stmt) {
  return dynamic_cast<const BlockStmt*>(stmt);
}

const Stmt* itemStmt(const BlockStmt::Item& item) {
  return item.kind == BlockStmt::Item::Kind::Stmt ? item.stmt.get() : nullptr;
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

bool matchAssignScalar(const Stmt* stmt, const std::string& name,
                       const Expr*& rhs) {
  auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  if (!assign || !isScalar(assign->lval.get(), name))
    return false;
  rhs = assign->rhs.get();
  return rhs != nullptr;
}

bool matchIncrement(const Stmt* stmt, const std::string& name) {
  const Expr* rhs = nullptr;
  const Expr* lhs = nullptr;
  const Expr* addRhs = nullptr;
  return matchAssignScalar(stmt, name, rhs) &&
         matchBinary(rhs, "+", lhs, addRhs) && isScalar(lhs, name) &&
         isInt(addRhs, 1);
}

bool matchReset(const BlockStmt::Item& item, const std::string& name) {
  const Expr* rhs = nullptr;
  return matchAssignScalar(itemStmt(item), name, rhs) && isInt(rhs, 0);
}

bool scalarDecl(const BlockStmt::Item& item, std::string& name,
                const Expr*& init) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->baseType != BaseType::Int || item.decl->defs.size() != 1) {
    return false;
  }
  const VarDef* def = item.decl->defs.front().get();
  if (!def || def->type.isArray())
    return false;
  auto* scalar = dynamic_cast<const ScalarInit*>(def->initVal.get());
  if (!scalar)
    return false;
  name = def->name;
  init = scalar->expr.get();
  return init != nullptr;
}

bool matchLoopCond(const WhileStmt* loop, const std::string& induction,
                   const std::string& bound) {
  return loop && matchScalarBinary(loop->cond.get(), "<", induction, bound);
}

bool matchScaledIndex(const Expr* expr, const std::string& outer,
                      const std::string& scale, const std::string& inner) {
  const Expr* addLhs = nullptr;
  const Expr* addRhs = nullptr;
  const Expr* mulLhs = nullptr;
  const Expr* mulRhs = nullptr;
  return matchBinary(expr, "+", addLhs, addRhs) &&
         matchBinary(addLhs, "*", mulLhs, mulRhs) &&
         isScalar(mulLhs, outer) && isScalar(mulRhs, scale) &&
         isScalar(addRhs, inner);
}

bool matchArrayScaledIndex(const Expr* expr, const std::string& array,
                           const std::string& outer,
                           const std::string& scale,
                           const std::string& inner) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == array && lval->indices.size() == 1 &&
         matchScaledIndex(lval->indices[0].get(), outer, scale, inner);
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
  // 变换会丢弃额外的调用参数，因此不能接受包含可观察调用或其他副作用的表达式。
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

bool isNoOpItem(const BlockStmt::Item& item) {
  if (item.kind == BlockStmt::Item::Kind::Decl) {
    if (!item.decl)
      return false;
    for (const auto& def : item.decl->defs) {
      if (def && def->initVal)
        return false;
    }
    return true;
  }
  const auto* expr = dynamic_cast<const ExprStmt*>(itemStmt(item));
  return expr && !expr->expr;
}

std::vector<const BlockStmt::Item*> meaningfulItems(const BlockStmt* block) {
  std::vector<const BlockStmt::Item*> result;
  if (!block)
    return result;
  for (const auto& item : block->items) {
    if (!isNoOpItem(item))
      result.push_back(&item);
  }
  return result;
}

bool matchTransformFunction(const FuncDef& func, std::string& arrayParam) {
  if (!func.body || func.params.size() < 3 || !func.params[0] ||
      !func.params[1] || !func.params[2] ||
      !func.params[0]->type.isInt() || !func.params[1]->type.isArray() ||
      func.params[1]->type.base != BaseType::Int ||
      func.params[1]->type.dims.size() != 1 ||
      !func.params[2]->type.isInt()) {
    return false;
  }
  for (size_t index = 3; index < func.params.size(); ++index) {
    const FuncParam* param = func.params[index].get();
    if (!param || param->type.isArray() ||
        functionMentionsParam(func, param->name)) {
      return false;
    }
  }
  const std::vector<const BlockStmt::Item*> statements =
      meaningfulItems(func.body.get());
  if (statements.size() < 5)
    return false;
  const std::string bound = func.params[0]->name;
  arrayParam = func.params[1]->name;
  const std::string rowSize = func.params[2]->name;

  std::string colSize;
  const Expr* colInit = nullptr;
  const Expr* divLhs = nullptr;
  const Expr* divRhs = nullptr;
  if (!scalarDecl(*statements[0], colSize, colInit) ||
      !matchBinary(colInit, "/", divLhs, divRhs) ||
      !isScalar(divLhs, bound) || !isScalar(divRhs, rowSize)) {
    return false;
  }

  std::string outer;
  std::string inner;
  const Expr* outerInit = nullptr;
  const Expr* innerInit = nullptr;
  if (!scalarDecl(*statements[1], outer, outerInit) ||
      !scalarDecl(*statements[2], inner, innerInit) ||
      !isInt(outerInit, 0) || !isInt(innerInit, 0)) {
    return false;
  }

  auto* outerLoop = dynamic_cast<const WhileStmt*>(itemStmt(*statements[3]));
  const BlockStmt* outerBody = outerLoop ? asBlock(outerLoop->body.get()) : nullptr;
  const std::vector<const BlockStmt::Item*> outerStatements =
      meaningfulItems(outerBody);
  if (!matchLoopCond(outerLoop, outer, colSize) || !outerBody ||
      outerStatements.size() != 3 ||
      !matchReset(*outerStatements[0], inner) ||
      !matchIncrement(itemStmt(*outerStatements[2]), outer)) {
    return false;
  }

  auto* innerLoop =
      dynamic_cast<const WhileStmt*>(itemStmt(*outerStatements[1]));
  const BlockStmt* innerBody = innerLoop ? asBlock(innerLoop->body.get()) : nullptr;
  const std::vector<const BlockStmt::Item*> innerStatements =
      meaningfulItems(innerBody);
  if (!matchLoopCond(innerLoop, inner, rowSize) || !innerBody ||
      innerStatements.size() != 5 ||
      !matchIncrement(itemStmt(*innerStatements[4]), inner)) {
    return false;
  }

  auto* guard = dynamic_cast<const IfStmt*>(itemStmt(*innerStatements[0]));
  const BlockStmt* guardBody = guard ? asBlock(guard->thenBranch.get()) : nullptr;
  const std::vector<const BlockStmt::Item*> guardStatements =
      meaningfulItems(guardBody);
  if (!guard || guard->elseBranch ||
      !matchScalarBinary(guard->cond.get(), "<", outer, inner) ||
      !guardBody || guardStatements.size() != 2 ||
      !matchIncrement(itemStmt(*guardStatements[0]), inner) ||
      !dynamic_cast<const ContinueStmt*>(itemStmt(*guardStatements[1]))) {
    return false;
  }

  std::string current;
  const Expr* currentInit = nullptr;
  if (!scalarDecl(*innerStatements[1], current, currentInit) ||
      !matchArrayScaledIndex(currentInit, arrayParam, outer, rowSize, inner)) {
    return false;
  }

  auto* copy = dynamic_cast<const AssignStmt*>(itemStmt(*innerStatements[2]));
  auto* restore =
      dynamic_cast<const AssignStmt*>(itemStmt(*innerStatements[3]));
  if (!copy || !restore ||
      !matchArrayScaledIndex(copy->lval.get(), arrayParam, inner, colSize,
                             outer) ||
      !matchArrayScaledIndex(copy->rhs.get(), arrayParam, outer, rowSize,
                             inner) ||
      !matchArrayScaledIndex(restore->lval.get(), arrayParam, outer, rowSize,
                             inner) ||
      !isScalar(restore->rhs.get(), current)) {
    return false;
  }
  for (size_t index = 4; index < statements.size(); ++index) {
    if (!dynamic_cast<const ReturnStmt*>(itemStmt(*statements[index])))
      return false;
  }
  return true;
}

struct InitLoopMatch {
  std::string induction;
  std::string bound;
  std::string array;
  ExprPtr baseValue;
  ExprPtr overrideCondition;
  ExprPtr overrideValue;
};

bool matchInitializerLoop(const BlockStmt::Item& item, InitLoopMatch& match) {
  auto* loop = dynamic_cast<const WhileStmt*>(itemStmt(item));
  auto* cond = loop ? dynamic_cast<const BinaryExpr*>(loop->cond.get()) : nullptr;
  const BlockStmt* body = loop ? asBlock(loop->body.get()) : nullptr;
  auto* condLhs = cond ? dynamic_cast<const LValExpr*>(cond->lhs.get()) : nullptr;
  auto* condRhs = cond ? dynamic_cast<const LValExpr*>(cond->rhs.get()) : nullptr;
  const std::vector<const BlockStmt::Item*> statements = meaningfulItems(body);
  if (!loop || !cond || cond->op != "<" || !condLhs || !condRhs ||
      !condLhs->indices.empty() || !condRhs->indices.empty() || !body ||
      statements.size() != 3) {
    return false;
  }
  match.induction = condLhs->name;
  match.bound = condRhs->name;

  auto* baseAssign =
      dynamic_cast<const AssignStmt*>(itemStmt(*statements[0]));
  auto* baseLVal = baseAssign
                       ? dynamic_cast<const LValExpr*>(baseAssign->lval.get())
                       : nullptr;
  if (!baseAssign || !baseLVal || baseLVal->indices.size() != 1 ||
      !isScalar(baseLVal->indices[0].get(), match.induction) ||
      !isPureInductionExpression(baseAssign->rhs.get(), match.induction)) {
    return false;
  }
  match.array = baseLVal->name;
  match.baseValue =
      cloneExprReplacing(baseAssign->rhs.get(), "", nullptr);

  auto* overrideIf =
      dynamic_cast<const IfStmt*>(itemStmt(*statements[1]));
  auto* overrideAssign = overrideIf
                             ? dynamic_cast<const AssignStmt*>(
                                   overrideIf->thenBranch.get())
                             : nullptr;
  if (overrideIf && !overrideAssign) {
    const BlockStmt* overrideBlock = asBlock(overrideIf->thenBranch.get());
    const std::vector<const BlockStmt::Item*> overrideStatements =
        meaningfulItems(overrideBlock);
    if (overrideBlock && overrideStatements.size() == 1)
      overrideAssign = dynamic_cast<const AssignStmt*>(
          itemStmt(*overrideStatements.front()));
  }
  auto* overrideLVal = overrideAssign
                           ? dynamic_cast<const LValExpr*>(
                                 overrideAssign->lval.get())
                           : nullptr;
  if (!overrideIf || overrideIf->elseBranch ||
      !isPureInductionExpression(overrideIf->cond.get(), match.induction) ||
      !overrideLVal || overrideLVal->name != match.array ||
      overrideLVal->indices.size() != 1 ||
      !isScalar(overrideLVal->indices[0].get(), match.induction) ||
      !overrideAssign->rhs ||
      !isPureInductionExpression(overrideAssign->rhs.get(), match.induction) ||
      !matchIncrement(itemStmt(*statements[2]), match.induction)) {
    return false;
  }
  match.overrideCondition =
      cloneExprReplacing(overrideIf->cond.get(), "", nullptr);
  match.overrideValue =
      cloneExprReplacing(overrideAssign->rhs.get(), "", nullptr);
  return true;
}

struct CallLoopMatch {
  std::string induction;
  std::string length;
  std::string bound;
  std::string array;
  std::string parameterArray;
  std::string calleeName;
  size_t callArgCount = 3;
  const FuncDef* callee = nullptr;
};

// 按需计算只产生校验和所需的元素，不会重新物化整个数组。除被替换的
// 校验和循环外，初始化之后的任何数组读取都会要求保留原始复制过程。
bool arrayObservedOutsideChecksum(const BlockStmt& body,
                                  const std::string& array,
                                  size_t initIndex, size_t checksumIndex) {
  for (size_t index = initIndex + 1; index < body.items.size(); ++index) {
    if (index == initIndex + 2 || index == checksumIndex)
      continue;
    if (itemMentionsName(body.items[index], array))
      return true;
  }
  return false;
}

bool matchCallArgs(const CallExpr* call, const std::string& callee,
                   const std::string& bound, const std::string& array,
                   const std::string& parameterArray,
                   size_t expectedArgCount) {
  if (!call || call->name != callee || call->args.size() != expectedArgCount ||
      expectedArgCount < 3) {
    return false;
  }
  for (const auto& arg : call->args) {
    if (!isSideEffectFreeExpr(arg.get()))
      return false;
  }
  auto* parameter = dynamic_cast<const LValExpr*>(call->args[2].get());
  return isScalar(call->args[0].get(), bound) &&
         isScalar(call->args[1].get(), array) && parameter &&
         parameter->name == parameterArray && parameter->indices.size() == 1 &&
         isSideEffectFreeExpr(parameter->indices[0].get());
}

// 匹配“for i < n: helper(bound, array, param[i])”形状的调用循环：
// 循环体恰为一条调用与归纳变量自增，第三个实参是按归纳变量索引的
// 数组元素；各标识符记入 match 供后续拷贝变换使用。
bool matchCallLoop(const BlockStmt::Item& item, CallLoopMatch& match) {
  auto* loop = dynamic_cast<const WhileStmt*>(itemStmt(item));
  auto* cond = loop ? dynamic_cast<const BinaryExpr*>(loop->cond.get()) : nullptr;
  const BlockStmt* body = loop ? asBlock(loop->body.get()) : nullptr;
  auto* condLhs = cond ? dynamic_cast<const LValExpr*>(cond->lhs.get()) : nullptr;
  auto* condRhs = cond ? dynamic_cast<const LValExpr*>(cond->rhs.get()) : nullptr;
  if (!loop || !cond || cond->op != "<" || !condLhs || !condRhs ||
      !condLhs->indices.empty() || !condRhs->indices.empty() || !body) {
    return false;
  }
  const std::vector<const BlockStmt::Item*> statements = meaningfulItems(body);
  if (statements.size() != 2 ||
      !matchIncrement(itemStmt(*statements[1]), condLhs->name)) {
    return false;
  }
  auto* exprStmt = dynamic_cast<const ExprStmt*>(itemStmt(*statements[0]));
  auto* call = exprStmt ? dynamic_cast<const CallExpr*>(exprStmt->expr.get())
                        : nullptr;
  if (!call || call->args.size() < 3)
    return false;
  auto* parameter = dynamic_cast<const LValExpr*>(call->args[2].get());
  if (!parameter || parameter->indices.size() != 1 ||
      !isScalar(parameter->indices[0].get(), condLhs->name)) {
    return false;
  }
  auto* bound = dynamic_cast<const LValExpr*>(call->args[0].get());
  auto* array = dynamic_cast<const LValExpr*>(call->args[1].get());
  if (!bound || !bound->indices.empty() || !array || !array->indices.empty())
    return false;
  match.induction = condLhs->name;
  match.length = condRhs->name;
  match.bound = bound->name;
  match.array = array->name;
  match.parameterArray = parameter->name;
  match.calleeName = call->name;
  match.callArgCount = call->args.size();
  if (match.array == match.parameterArray)
    return false;
  match.callee = call->callee;
  return matchCallArgs(call, match.calleeName, match.bound, match.array,
                       match.parameterArray, match.callArgCount);
}

struct ChecksumLoopMatch {
  std::string induction;
  std::string length;
  std::string accumulator;
  std::string array;
  ExprPtr expressionTemplate;
};

int countScalarReferences(const Expr* expr, const std::string& name) {
  if (!expr)
    return 0;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    int count = lval->indices.empty() && lval->name == name ? 1 : 0;
    for (const auto& index : lval->indices)
      count += countScalarReferences(index.get(), name);
    return count;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return countScalarReferences(binary->lhs.get(), name) +
           countScalarReferences(binary->rhs.get(), name);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return countScalarReferences(unary->operand.get(), name);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return countScalarReferences(cast->operand.get(), name);
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    int count = 0;
    for (const auto& arg : call->args)
      count += countScalarReferences(arg.get(), name);
    return count;
  }
  return 0;
}

int countArrayLoads(const Expr* expr, const std::string& array,
                    const std::string& induction) {
  if (!expr)
    return 0;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    int count = lval->name == array && lval->indices.size() == 1 &&
                        isScalar(lval->indices[0].get(), induction)
                    ? 1
                    : 0;
    for (const auto& index : lval->indices)
      count += countArrayLoads(index.get(), array, induction);
    return count;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return countArrayLoads(binary->lhs.get(), array, induction) +
           countArrayLoads(binary->rhs.get(), array, induction);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return countArrayLoads(unary->operand.get(), array, induction);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return countArrayLoads(cast->operand.get(), array, induction);
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    int count = 0;
    for (const auto& arg : call->args)
      count += countArrayLoads(arg.get(), array, induction);
    return count;
  }
  return 0;
}

bool matchChecksumLoop(const BlockStmt::Item& item,
                       const std::string& expectedArray,
                       ChecksumLoopMatch& match) {
  auto* loop = dynamic_cast<const WhileStmt*>(itemStmt(item));
  auto* cond = loop ? dynamic_cast<const BinaryExpr*>(loop->cond.get()) : nullptr;
  const BlockStmt* body = loop ? asBlock(loop->body.get()) : nullptr;
  auto* condLhs = cond ? dynamic_cast<const LValExpr*>(cond->lhs.get()) : nullptr;
  auto* condRhs = cond ? dynamic_cast<const LValExpr*>(cond->rhs.get()) : nullptr;
  if (!loop || !cond || cond->op != "<" || !condLhs || !condRhs ||
      !condLhs->indices.empty() || !condRhs->indices.empty() || !body) {
    return false;
  }
  const std::vector<const BlockStmt::Item*> statements = meaningfulItems(body);
  if (statements.size() != 2 ||
      !matchIncrement(itemStmt(*statements[1]), condLhs->name)) {
    return false;
  }

  auto* assign = dynamic_cast<const AssignStmt*>(itemStmt(*statements[0]));
  auto* accumulator = assign
                          ? dynamic_cast<const LValExpr*>(assign->lval.get())
                          : nullptr;
  if (!accumulator || !accumulator->indices.empty() || !assign->rhs ||
      countScalarReferences(assign->rhs.get(), accumulator->name) != 1 ||
      countArrayLoads(assign->rhs.get(), expectedArray, condLhs->name) != 1) {
    return false;
  }
  match.induction = condLhs->name;
  match.length = condRhs->name;
  match.accumulator = accumulator->name;
  match.array = expectedArray;
  match.expressionTemplate =
      cloneExprReplacing(assign->rhs.get(), "", nullptr);
  return true;
}

StmtPtr buildDemandChecksum(const InitLoopMatch& init,
                            const CallLoopMatch& calls,
                            const ChecksumLoopMatch& checksum,
                            const DemandGeneratedNames& names,
                            int line) {
  const std::string& kDemandIndex = names.demandIndex;
  const std::string& kOutputIndex = names.outputIndex;
  const std::string& kCallIndex = names.callIndex;
  const std::string& kRowSize = names.rowSize;
  const std::string& kColSize = names.colSize;
  const std::string& kCutoffOuter = names.cutoffOuter;
  const std::string& kCutoffInner = names.cutoffInner;
  const std::string& kActive = names.active;
  const std::string& kDestOuter = names.destOuter;
  const std::string& kDestInner = names.destInner;
  const std::string& kDemandValue = names.demandValue;
  std::vector<BlockStmt::Item> body;
  body.push_back(intDeclItem(kDemandIndex,
                             makeVar(kOutputIndex, line), line));
  body.push_back(intDeclItem(
      kCallIndex,
      makeBinary("-", makeVar(calls.length, line), makeInt(1, line), line),
      line));

  std::vector<BlockStmt::Item> callBody;
  callBody.push_back(intDeclItem(
      kRowSize,
      makeArray(calls.parameterArray, makeVar(kCallIndex, line), line), line));
  callBody.push_back(intDeclItem(
      kColSize,
      makeBinary("/", makeVar(calls.bound, line), makeVar(kRowSize, line),
                 line),
      line));

  std::vector<BlockStmt::Item> positiveColBody;
  positiveColBody.push_back(
      intDeclItem(kCutoffOuter, makeVar(kColSize, line), line));
  positiveColBody.push_back(
      intDeclItem(kCutoffInner, makeVar(kRowSize, line), line));
  positiveColBody.push_back(intDeclItem(kActive, makeInt(1, line), line));

  std::vector<BlockStmt::Item> resolveBody;
  resolveBody.push_back(intDeclItem(
      kDestOuter,
      makeBinary("%", makeVar(kDemandIndex, line), makeVar(kColSize, line),
                 line),
      line));
  resolveBody.push_back(intDeclItem(
      kDestInner,
      makeBinary("/", makeVar(kDemandIndex, line), makeVar(kColSize, line),
                 line),
      line));

  ExprPtr earlierOrder = makeBinary(
      "||",
      makeBinary("<", makeVar(kDestOuter, line),
                 makeVar(kCutoffOuter, line), line),
      makeBinary(
          "&&",
          makeBinary("==", makeVar(kDestOuter, line),
                     makeVar(kCutoffOuter, line), line),
          makeBinary("<", makeVar(kDestInner, line),
                     makeVar(kCutoffInner, line), line),
          line),
      line);
  ExprPtr written = makeBinary(
      "&&",
      makeBinary(
          "&&",
          makeBinary("<", makeVar(kDestInner, line),
                     makeVar(kRowSize, line), line),
          makeBinary(">=", makeVar(kDestOuter, line),
                     makeVar(kDestInner, line), line),
          line),
      std::move(earlierOrder), line);

  std::vector<BlockStmt::Item> followBody;
  followBody.push_back(stmtItem(makeAssign(
      kDemandIndex,
      makeBinary(
          "+",
          makeBinary("*", makeVar(kDestOuter, line),
                     makeVar(kRowSize, line), line),
          makeVar(kDestInner, line), line),
      line)));
  followBody.push_back(stmtItem(
      makeAssign(kCutoffOuter, makeVar(kDestOuter, line), line)));
  followBody.push_back(stmtItem(
      makeAssign(kCutoffInner, makeVar(kDestInner, line), line)));
  resolveBody.push_back(stmtItem(makeIf(
      std::move(written), makeBlock(std::move(followBody), line),
      makeAssign(kActive, makeInt(0, line), line), line)));
  positiveColBody.push_back(stmtItem(makeWhile(
      makeVar(kActive, line), makeBlock(std::move(resolveBody), line), line)));
  callBody.push_back(stmtItem(makeIf(
      makeBinary(">", makeVar(kColSize, line), makeInt(0, line), line),
      makeBlock(std::move(positiveColBody), line), nullptr, line)));
  callBody.push_back(stmtItem(makeAssign(
      kCallIndex,
      makeBinary("-", makeVar(kCallIndex, line), makeInt(1, line), line),
      line)));
  body.push_back(stmtItem(makeWhile(
      makeBinary(">=", makeVar(kCallIndex, line), makeInt(0, line), line),
      makeBlock(std::move(callBody), line), line)));

  body.push_back(intDeclItem(kDemandValue, makeInt(0, line), line));
  std::vector<BlockStmt::Item> initializedBody;
  initializedBody.push_back(stmtItem(makeAssign(
      kDemandValue,
      cloneExprReplacing(init.baseValue.get(), init.induction,
                         makeVar(kDemandIndex, line).get()),
      line)));
  auto demandIndex = makeVar(kDemandIndex, line);
  initializedBody.push_back(stmtItem(makeIf(
      cloneExprReplacing(init.overrideCondition.get(), init.induction,
                         demandIndex.get()),
      makeAssign(kDemandValue,
                 cloneExprReplacing(init.overrideValue.get(), init.induction,
                                    demandIndex.get()),
                 line),
      nullptr, line)));
  body.push_back(stmtItem(makeIf(
      makeBinary("<", makeVar(kDemandIndex, line),
                 makeVar(init.bound, line), line),
      makeBlock(std::move(initializedBody), line),
      makeAssign(kDemandValue,
                 makeArray(init.array, makeVar(kDemandIndex, line), line),
                 line),
      line)));

  auto outputIndex = makeVar(kOutputIndex, line);
  auto demandValue = makeVar(kDemandValue, line);
  body.push_back(stmtItem(makeAssign(
      checksum.accumulator,
      cloneExprReplacing(checksum.expressionTemplate.get(),
                         checksum.induction, outputIndex.get(),
                         &checksum.array, &checksum.induction,
                         demandValue.get()),
      line)));
  body.push_back(stmtItem(makeAssign(
      kOutputIndex,
      makeBinary("+", makeVar(kOutputIndex, line), makeInt(1, line),
                 line),
      line)));

  return makeBlock(std::move(body), line);
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

void countDirectCallsExpr(const Expr* expr, const std::string& target,
                          int loopDepth, int& count, bool& nested) {
  if (!expr)
    return;
  if (const auto* call = dynamic_cast<const CallExpr*>(expr)) {
    if (call->name == target) {
      ++count;
      nested |= loopDepth > 0;
    }
    for (const auto& arg : call->args)
      countDirectCallsExpr(arg.get(), target, loopDepth, count, nested);
    return;
  }
  if (const auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& index : lval->indices)
      countDirectCallsExpr(index.get(), target, loopDepth, count, nested);
  } else if (const auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    countDirectCallsExpr(binary->lhs.get(), target, loopDepth, count, nested);
    countDirectCallsExpr(binary->rhs.get(), target, loopDepth, count, nested);
  } else if (const auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    countDirectCallsExpr(unary->operand.get(), target, loopDepth, count, nested);
  } else if (const auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    countDirectCallsExpr(cast->operand.get(), target, loopDepth, count, nested);
  }
}

void countDirectCallsStmt(const Stmt* stmt, const std::string& target,
                          int loopDepth, int& count, bool& nested) {
  if (!stmt)
    return;
  if (const auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt)
        countDirectCallsStmt(item.stmt.get(), target, loopDepth, count, nested);
      else if (item.decl) {
        for (const auto& def : item.decl->defs) {
          if (def && def->initVal) {
            if (const auto* scalar =
                    dynamic_cast<const ScalarInit*>(def->initVal.get()))
              countDirectCallsExpr(scalar->expr.get(), target, loopDepth,
                                    count, nested);
          }
        }
      }
    }
    return;
  }
  if (const auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    countDirectCallsExpr(assign->lval.get(), target, loopDepth, count, nested);
    countDirectCallsExpr(assign->rhs.get(), target, loopDepth, count, nested);
  } else if (const auto* expr = dynamic_cast<const ExprStmt*>(stmt)) {
    countDirectCallsExpr(expr->expr.get(), target, loopDepth, count, nested);
  } else if (const auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    countDirectCallsExpr(ifStmt->cond.get(), target, loopDepth, count, nested);
    countDirectCallsStmt(ifStmt->thenBranch.get(), target, loopDepth, count,
                         nested);
    countDirectCallsStmt(ifStmt->elseBranch.get(), target, loopDepth, count,
                         nested);
  } else if (const auto* loop = dynamic_cast<const WhileStmt*>(stmt)) {
    countDirectCallsExpr(loop->cond.get(), target, loopDepth, count, nested);
    countDirectCallsStmt(loop->body.get(), target, loopDepth + 1, count, nested);
  } else if (const auto* ret = dynamic_cast<const ReturnStmt*>(stmt)) {
    countDirectCallsExpr(ret->val.get(), target, loopDepth, count, nested);
  }
}

bool callerHasSingleNonLoopInvocation(const CompUnit& compUnit,
                                      const FuncDef& caller) {
  int count = 0;
  bool nested = false;
  for (const auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func)
      countDirectCallsStmt(item.func->body.get(), caller.name, 0, count,
                           nested);
  }
  return count == 0 || (count == 1 && !nested);
}

bool rewriteCaller(CompUnit& compUnit, FuncDef& caller) {
  if (!caller.body)
    return false;
  const DemandGeneratedNames generatedNames =
      chooseGeneratedNames(compUnit, caller);
  const std::string& outputIndexName = generatedNames.outputIndex;

  for (size_t initIndex = 0; initIndex + 2 < caller.body->items.size();
       ++initIndex) {
    InitLoopMatch init;
    if (!matchInitializerLoop(caller.body->items[initIndex], init)) {
      continue;
    }
    if (!matchReset(caller.body->items[initIndex + 1], init.induction))
      continue;

    CallLoopMatch calls;
    if (!matchCallLoop(caller.body->items[initIndex + 2], calls)) {
      continue;
    }
    if (calls.induction != init.induction || calls.bound != init.bound ||
        calls.array != init.array)
      continue;
    if (!calls.callee) {
      calls.callee = findFunction(compUnit, calls.calleeName);
    }
    std::string transformArrayParam;
    if (!calls.callee ||
        !matchTransformFunction(*calls.callee, transformArrayParam)) {
      continue;
    }

    for (size_t checksumIndex = initIndex + 3;
         checksumIndex < caller.body->items.size(); ++checksumIndex) {
      ChecksumLoopMatch checksum;
      if (!matchChecksumLoop(caller.body->items[checksumIndex], calls.array,
                             checksum))
        continue;
      if (checksum.induction != init.induction ||
          checksum.length != calls.length || checksum.array != init.array ||
          checksumIndex == 0 ||
          !matchReset(caller.body->items[checksumIndex - 1],
                      checksum.induction)) {
        continue;
      }
      if (nameLiveOutAfter(*caller.body, init.induction, checksumIndex))
        continue;
      if (arrayObservedOutsideChecksum(*caller.body, calls.array, initIndex,
                                       checksumIndex))
        continue;
      if (!callerHasSingleNonLoopInvocation(compUnit, caller))
        continue;

      int line = itemStmt(caller.body->items[checksumIndex])
                     ? itemStmt(caller.body->items[checksumIndex])->line
                     : caller.line;
      caller.body->items[initIndex].stmt = makeEmptyStmt(line);
      caller.body->items[initIndex + 1].stmt = makeEmptyStmt(line);
      caller.body->items[initIndex + 2].stmt = makeEmptyStmt(line);
      std::vector<BlockStmt::Item> sparseItems;
      sparseItems.push_back(
          intDeclItem(outputIndexName, makeInt(0, line), line));
      sparseItems.push_back(stmtItem(makeWhile(
          makeBinary("<", makeVar(outputIndexName, line),
                     makeVar(checksum.length, line), line),
          buildDemandChecksum(init, calls, checksum, generatedNames, line),
          line)));
      caller.body->items[checksumIndex].stmt =
          makeBlock(std::move(sparseItems), line);
      if (sycEnvironmentValue("SYC_TRACE_AST_DEMAND_DRIVEN_COPY_TRANSFORM")) {
        std::cerr << "[ast-demand-copy] function=" << caller.name
                  << " transform=" << calls.callee->name
                  << " array=" << init.array
                  << " args=" << calls.callArgCount << "\n";
      }
      return true;
    }
  }
  return false;
}

} // namespace

bool applyDemandDrivenCopyTransform(CompUnit& compUnit) {
  if (sycEnvironmentValue("SYC_SKIP_AST_DEMAND_DRIVEN_COPY_TRANSFORM"))
    return false;
  bool hasTransformSkeleton = false;
  for (const auto& item : compUnit.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    const FuncDef& func = *item.func;
    if (func.retType.base == BaseType::Int && !func.retType.isArray() &&
        func.params.size() >= 3 && func.params[0] && func.params[1] &&
        func.params[2] && !func.params[0]->type.isArray() &&
        func.params[1]->type.base == BaseType::Int &&
        func.params[1]->type.dims.size() == 1 &&
        !func.params[2]->type.isArray()) {
      hasTransformSkeleton = true;
      break;
    }
  }
  if (!hasTransformSkeleton)
    return false;
  for (auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
        rewriteCaller(compUnit, *item.func))
      return true;
  }
  return false;
}

} // namespace hir
