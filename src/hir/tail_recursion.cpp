#include "research_switch.h"
// 文件职责：识别并消除安全的尾递归，把尾调用改写为循环。本文件负责实现具体逻辑。
// 这里同时保留 CoreIR 主线实现和已退出主流水线的 AST helper。
// 自递归名字比较用于识别调用关系，`main` 排除用于保护入口 ABI。
#include "tail_recursion.h"
#include "../analysis/cfg_utils.h"
#include "../frontend/ast.h"
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {

namespace {

bool shouldBlockRecursiveRewrite(
    const std::string& funcName,
    const std::unordered_set<std::string>* blockedFunctions) {
  return blockedFunctions && blockedFunctions->count(funcName);
}

bool hasMemoScaffolding(const core_ir::Module& mod, const core_ir::Function* func) {
  if (!func)
    return false;
  const std::string prefix = "__memo." + func->name() + ".";
  for (const auto& gvPtr : mod.globals()) {
    if (gvPtr && gvPtr->name().rfind(prefix, 0) == 0)
      return true;
  }
  return false;
}

std::unique_ptr<IntLitExpr> makeIntLit(int32_t value, int line) {
  auto expr = std::make_unique<IntLitExpr>();
  expr->val = value;
  expr->line = line;
  return expr;
}

std::unique_ptr<LValExpr> makeVarExpr(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->name = name;
  expr->line = line;
  return expr;
}

std::unique_ptr<UnaryExpr> makeUnaryExpr(char op, ExprPtr operand, int line) {
  auto expr = std::make_unique<UnaryExpr>();
  expr->op = op;
  expr->operand = std::move(operand);
  expr->line = line;
  return expr;
}

bool isScalarParam(const FuncParam* param) {
  return param && !param->type.isArray() &&
         (param->type.base == BaseType::Int || param->type.base == BaseType::Float);
}

// 按具体 AST 类型深拷贝表达式：叶子节点复制字段，
// 复合节点对子节点递归克隆。
ExprPtr cloneExpr(const Expr* expr) {
  if (!expr)
    return nullptr;
  if (auto* lit = dynamic_cast<const IntLitExpr*>(expr)) {
    auto copy = std::make_unique<IntLitExpr>();
    copy->val = lit->val;
    copy->line = lit->line;
    copy->semaType = lit->semaType;
    return copy;
  }
  if (auto* lit = dynamic_cast<const FloatLitExpr*>(expr)) {
    auto copy = std::make_unique<FloatLitExpr>();
    copy->val = lit->val;
    copy->line = lit->line;
    copy->semaType = lit->semaType;
    return copy;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    auto copy = std::make_unique<LValExpr>();
    copy->name = lval->name;
    copy->line = lval->line;
    copy->symbol = lval->symbol;
    copy->semaType = lval->semaType;
    for (const auto& idx : lval->indices)
      copy->indices.push_back(cloneExpr(idx.get()));
    return copy;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    auto copy = std::make_unique<CallExpr>();
    copy->name = call->name;
    copy->line = call->line;
    copy->callee = call->callee;
    copy->semaType = call->semaType;
    for (const auto& arg : call->args)
      copy->args.push_back(cloneExpr(arg.get()));
    return copy;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    auto copy = std::make_unique<CastExpr>();
    copy->targetType = cast->targetType;
    copy->line = cast->line;
    copy->semaType = cast->semaType;
    copy->operand = cloneExpr(cast->operand.get());
    return copy;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    auto copy = std::make_unique<UnaryExpr>();
    copy->op = unary->op;
    copy->line = unary->line;
    copy->semaType = unary->semaType;
    copy->operand = cloneExpr(unary->operand.get());
    return copy;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    auto copy = std::make_unique<BinaryExpr>();
    copy->op = binary->op;
    copy->line = binary->line;
    copy->semaType = binary->semaType;
    copy->lhs = cloneExpr(binary->lhs.get());
    copy->rhs = cloneExpr(binary->rhs.get());
    return copy;
  }
  return nullptr;
}

// 按具体 AST 类型深拷贝语句；块语句里的变量声明
// 也要逐个重建定义与初值表达式。
StmtPtr cloneStmt(const Stmt* stmt) {
  if (!stmt)
    return nullptr;
  if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    auto copy = std::make_unique<AssignStmt>();
    copy->line = assign->line;
    copy->lval = cloneExpr(assign->lval.get());
    copy->rhs = cloneExpr(assign->rhs.get());
    return copy;
  }
  if (auto* exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
    auto copy = std::make_unique<ExprStmt>();
    copy->line = exprStmt->line;
    copy->expr = cloneExpr(exprStmt->expr.get());
    return copy;
  }
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    auto copy = std::make_unique<BlockStmt>();
    copy->line = block->line;
    for (const auto& item : block->items) {
      BlockStmt::Item cloned;
      cloned.kind = item.kind;
      if (item.kind == BlockStmt::Item::Kind::Stmt) {
        cloned.stmt = cloneStmt(item.stmt.get());
      } else if (item.decl) {
        auto decl = std::make_unique<DeclStmt>();
        decl->line = item.decl->line;
        decl->isConst = item.decl->isConst;
        decl->baseType = item.decl->baseType;
        for (const auto& def : item.decl->defs) {
          auto newDef = std::make_unique<VarDef>();
          newDef->line = def->line;
          newDef->name = def->name;
          newDef->isConst = def->isConst;
          newDef->type = def->type;
          if (auto* scalar = dynamic_cast<ScalarInit*>(def->initVal.get())) {
            auto init = std::make_unique<ScalarInit>();
            init->line = scalar->line;
            init->expr = cloneExpr(scalar->expr.get());
            newDef->initVal = std::move(init);
          }
          decl->defs.push_back(std::move(newDef));
        }
        cloned.decl = std::move(decl);
      }
      copy->items.push_back(std::move(cloned));
    }
    return copy;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    auto copy = std::make_unique<IfStmt>();
    copy->line = ifStmt->line;
    copy->cond = cloneExpr(ifStmt->cond.get());
    copy->thenBranch = cloneStmt(ifStmt->thenBranch.get());
    copy->elseBranch = cloneStmt(ifStmt->elseBranch.get());
    return copy;
  }
  if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
    auto copy = std::make_unique<WhileStmt>();
    copy->line = whileStmt->line;
    copy->cond = cloneExpr(whileStmt->cond.get());
    copy->body = cloneStmt(whileStmt->body.get());
    return copy;
  }
  if (dynamic_cast<const BreakStmt*>(stmt)) {
    auto copy = std::make_unique<BreakStmt>();
    copy->line = stmt->line;
    return copy;
  }
  if (dynamic_cast<const ContinueStmt*>(stmt)) {
    auto copy = std::make_unique<ContinueStmt>();
    copy->line = stmt->line;
    return copy;
  }
  if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt)) {
    auto copy = std::make_unique<ReturnStmt>();
    copy->line = ret->line;
    copy->val = cloneExpr(ret->val.get());
    return copy;
  }
  return nullptr;
}

bool exprHasSelfCall(Expr* expr, const std::string& funcName) {
  if (!expr)
    return false;
  if (auto* call = dynamic_cast<CallExpr*>(expr)) {
    if (call->name == funcName)
      return true;
    for (auto& arg : call->args) {
      if (exprHasSelfCall(arg.get(), funcName))
        return true;
    }
    return false;
  }
  if (auto* lval = dynamic_cast<LValExpr*>(expr)) {
    for (auto& idx : lval->indices) {
      if (exprHasSelfCall(idx.get(), funcName))
        return true;
    }
    return false;
  }
  if (auto* bin = dynamic_cast<BinaryExpr*>(expr))
    return exprHasSelfCall(bin->lhs.get(), funcName) ||
           exprHasSelfCall(bin->rhs.get(), funcName);
  if (auto* unary = dynamic_cast<UnaryExpr*>(expr))
    return exprHasSelfCall(unary->operand.get(), funcName);
  if (auto* cast = dynamic_cast<CastExpr*>(expr))
    return exprHasSelfCall(cast->operand.get(), funcName);
  return false;
}

bool stmtContainsLoop(Stmt* stmt) {
  if (!stmt)
    return false;
  if (dynamic_cast<WhileStmt*>(stmt))
    return true;
  if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt &&
          stmtContainsLoop(item.stmt.get()))
        return true;
    }
    return false;
  }
  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt))
    return stmtContainsLoop(ifStmt->thenBranch.get()) ||
           stmtContainsLoop(ifStmt->elseBranch.get());
  return false;
}

bool exprHasAnyCall(const Expr* expr) {
  if (!expr)
    return false;
  if (dynamic_cast<const CallExpr*>(expr))
    return true;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& idx : lval->indices) {
      if (exprHasAnyCall(idx.get()))
        return true;
    }
    return false;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprHasAnyCall(cast->operand.get());
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return exprHasAnyCall(unary->operand.get());
  if (auto* bin = dynamic_cast<const BinaryExpr*>(expr))
    return exprHasAnyCall(bin->lhs.get()) ||
           exprHasAnyCall(bin->rhs.get());
  return false;
}

bool isImmediateReturnStmt(const Stmt* stmt) {
  if (!stmt)
    return false;
  if (dynamic_cast<const ReturnStmt*>(stmt))
    return true;
  auto* block = dynamic_cast<const BlockStmt*>(stmt);
  if (!block)
    return false;
  bool sawReturn = false;
  for (const auto& item : block->items) {
    if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt)
      return false;
    if (auto* exprStmt = dynamic_cast<const ExprStmt*>(item.stmt.get())) {
      if (!exprStmt->expr)
        continue;
    }
    if (!dynamic_cast<const ReturnStmt*>(item.stmt.get()) || sawReturn)
      return false;
    sawReturn = true;
  }
  return sawReturn;
}

bool exprReferencesOnlyParams(const Expr* expr,
                              const std::unordered_set<std::string>& params) {
  if (!expr)
    return true;
  if (dynamic_cast<const IntLitExpr*>(expr) ||
      dynamic_cast<const FloatLitExpr*>(expr))
    return true;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->indices.empty())
      return params.count(lval->name) != 0;
    if (params.count(lval->name) == 0)
      return false;
    for (const auto& idx : lval->indices) {
      if (!exprReferencesOnlyParams(idx.get(), params))
        return false;
    }
    return true;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprReferencesOnlyParams(cast->operand.get(), params);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return exprReferencesOnlyParams(unary->operand.get(), params);
  if (auto* bin = dynamic_cast<const BinaryExpr*>(expr))
    return exprReferencesOnlyParams(bin->lhs.get(), params) &&
           exprReferencesOnlyParams(bin->rhs.get(), params);
  return false;
}

// 克隆表达式并顺带做替换：命中替代表的标量左值直接用
// 替换表达式的克隆结果，其余节点照常递归。
ExprPtr cloneExprWithSubstitution(
    const Expr* expr,
    const std::unordered_map<std::string, const Expr*>& subst) {
  if (!expr)
    return nullptr;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->indices.empty()) {
      auto it = subst.find(lval->name);
      if (it != subst.end())
        return cloneExpr(it->second);
    }
    auto copy = std::make_unique<LValExpr>();
    copy->name = lval->name;
    copy->line = lval->line;
    copy->symbol = lval->symbol;
    copy->semaType = lval->semaType;
    for (const auto& idx : lval->indices)
      copy->indices.push_back(cloneExprWithSubstitution(idx.get(), subst));
    return copy;
  }
  if (auto* lit = dynamic_cast<const IntLitExpr*>(expr))
    return cloneExpr(lit);
  if (auto* lit = dynamic_cast<const FloatLitExpr*>(expr))
    return cloneExpr(lit);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    auto copy = std::make_unique<CastExpr>();
    copy->targetType = cast->targetType;
    copy->line = cast->line;
    copy->semaType = cast->semaType;
    copy->operand = cloneExprWithSubstitution(cast->operand.get(), subst);
    return copy;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    auto copy = std::make_unique<UnaryExpr>();
    copy->op = unary->op;
    copy->line = unary->line;
    copy->semaType = unary->semaType;
    copy->operand = cloneExprWithSubstitution(unary->operand.get(), subst);
    return copy;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    auto copy = std::make_unique<BinaryExpr>();
    copy->op = binary->op;
    copy->line = binary->line;
    copy->semaType = binary->semaType;
    copy->lhs = cloneExprWithSubstitution(binary->lhs.get(), subst);
    copy->rhs = cloneExprWithSubstitution(binary->rhs.get(), subst);
    return copy;
  }
  return nullptr;
}

const Expr* findLeadingBaseCaseCond(const FuncDef* func) {
  if (!func || !func->body)
    return nullptr;
  for (const auto& item : func->body->items) {
    if (item.kind == BlockStmt::Item::Kind::Decl)
      continue;
    auto* ifStmt = dynamic_cast<const IfStmt*>(item.stmt.get());
    if (!ifStmt || ifStmt->elseBranch || !isImmediateReturnStmt(ifStmt->thenBranch.get()))
      return nullptr;
    return ifStmt->cond.get();
  }
  return nullptr;
}

bool rewritePrunableSelfCalls(StmtPtr& stmt,
                              const FuncDef* func,
                              const Expr* baseCond) {
  if (!stmt || !func || !baseCond)
    return false;
  if (auto* block = dynamic_cast<BlockStmt*>(stmt.get())) {
    bool changed = false;
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt)
        changed |= rewritePrunableSelfCalls(item.stmt, func, baseCond);
    }
    return changed;
  }
  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
    bool changed = false;
    changed |= rewritePrunableSelfCalls(ifStmt->thenBranch, func, baseCond);
    changed |= rewritePrunableSelfCalls(ifStmt->elseBranch, func, baseCond);
    return changed;
  }
  if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt.get()))
    return rewritePrunableSelfCalls(whileStmt->body, func, baseCond);

  auto* exprStmt = dynamic_cast<ExprStmt*>(stmt.get());
  auto* call = exprStmt && exprStmt->expr
                   ? dynamic_cast<CallExpr*>(exprStmt->expr.get())
                   : nullptr;
  if (!call || call->name != func->name || call->args.size() != func->params.size())
    return false;
  for (const auto& arg : call->args) {
    if (exprHasAnyCall(arg.get()))
      return false;
  }

  std::unordered_map<std::string, const Expr*> subst;
  for (size_t i = 0; i < call->args.size(); ++i)
    subst[func->params[i]->name] = call->args[i].get();

  ExprPtr skipCond = cloneExprWithSubstitution(baseCond, subst);
  if (!skipCond)
    return false;

  auto guard = std::make_unique<IfStmt>();
  guard->line = call->line;
  guard->cond = makeUnaryExpr('!', std::move(skipCond), call->line);
  guard->thenBranch = std::move(stmt);
  stmt = std::move(guard);
  return true;
}

bool containsAggregateInit(const InitVal* init) {
  if (!init)
    return false;
  if (dynamic_cast<const AggregateInit*>(init))
    return true;
  return false;
}

bool substituteExpr(Expr* expr,
                    const std::unordered_map<std::string, const Expr*>& subst);

// 对语句树就地做参数替换；遇到聚合初值等不支持形式返回 false，
// 让调用方放弃本次尾递归改写。
bool substituteStmt(Stmt* stmt,
                    const std::unordered_map<std::string, const Expr*>& subst) {
  if (!stmt)
    return true;
  if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) {
    return substituteExpr(assign->lval.get(), subst) &&
           substituteExpr(assign->rhs.get(), subst);
  }
  if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt))
    return substituteExpr(exprStmt->expr.get(), subst);
  if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt) {
        if (!substituteStmt(item.stmt.get(), subst))
          return false;
      } else if (item.decl) {
        for (auto& def : item.decl->defs) {
          if (!def || containsAggregateInit(def->initVal.get()))
            return false;
          if (auto* scalar = dynamic_cast<ScalarInit*>(def->initVal.get())) {
            if (!substituteExpr(scalar->expr.get(), subst))
              return false;
          }
        }
      }
    }
    return true;
  }
  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
    return substituteExpr(ifStmt->cond.get(), subst) &&
           substituteStmt(ifStmt->thenBranch.get(), subst) &&
           substituteStmt(ifStmt->elseBranch.get(), subst);
  }
  if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
    return substituteExpr(whileStmt->cond.get(), subst) &&
           substituteStmt(whileStmt->body.get(), subst);
  }
  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt))
    return substituteExpr(ret->val.get(), subst);
  return true;
}

bool substituteExpr(Expr* expr,
                    const std::unordered_map<std::string, const Expr*>& subst) {
  if (!expr)
    return true;
  if (auto* lval = dynamic_cast<LValExpr*>(expr)) {
    for (auto& idx : lval->indices) {
      if (!substituteExpr(idx.get(), subst))
        return false;
    }
    if (!lval->indices.empty())
      return true;
    auto it = subst.find(lval->name);
    if (it == subst.end())
      return true;
    ExprPtr replacement = cloneExpr(it->second);
    if (!replacement)
      return false;
    if (auto* replLVal = dynamic_cast<LValExpr*>(replacement.get()))
      *lval = std::move(*replLVal);
    else if (auto* replInt = dynamic_cast<IntLitExpr*>(replacement.get()))
      return false;
    else
      return false;
    return true;
  }
  if (auto* call = dynamic_cast<CallExpr*>(expr)) {
    for (auto& arg : call->args) {
      if (!substituteExpr(arg.get(), subst))
        return false;
    }
    return true;
  }
  if (auto* cast = dynamic_cast<CastExpr*>(expr))
    return substituteExpr(cast->operand.get(), subst);
  if (auto* unary = dynamic_cast<UnaryExpr*>(expr))
    return substituteExpr(unary->operand.get(), subst);
  if (auto* bin = dynamic_cast<BinaryExpr*>(expr))
    return substituteExpr(bin->lhs.get(), subst) &&
           substituteExpr(bin->rhs.get(), subst);
  return true;
}

struct TailScan {
  bool hasTailCall = false;
  bool hasBadSelfCall = false;
  int tailCallCount = 0;
};

void scanStmt(Stmt* stmt, const std::string& funcName, TailScan& scan) {
  if (!stmt || scan.hasBadSelfCall)
    return;

  if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt)
        scanStmt(item.stmt.get(), funcName, scan);
      else if (item.decl) {
        for (const auto& def : item.decl->defs) {
          if (!def || !def->initVal)
            continue;
          auto* init = dynamic_cast<ScalarInit*>(def->initVal.get());
          if (init && exprHasSelfCall(init->expr.get(), funcName)) {
            scan.hasBadSelfCall = true;
            return;
          }
        }
      }
    }
    return;
  }

  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
    if (exprHasSelfCall(ifStmt->cond.get(), funcName)) {
      scan.hasBadSelfCall = true;
      return;
    }
    scanStmt(ifStmt->thenBranch.get(), funcName, scan);
    scanStmt(ifStmt->elseBranch.get(), funcName, scan);
    return;
  }

  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt)) {
    if (!ret->val)
      return;
    if (auto* call = dynamic_cast<CallExpr*>(ret->val.get())) {
      if (call->name == funcName) {
        scan.hasTailCall = true;
        ++scan.tailCallCount;
        for (auto& arg : call->args) {
          if (exprHasSelfCall(arg.get(), funcName)) {
            scan.hasBadSelfCall = true;
            return;
          }
        }
        return;
      }
    }
    if (exprHasSelfCall(ret->val.get(), funcName))
      scan.hasBadSelfCall = true;
    return;
  }

  if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) {
    if (exprHasSelfCall(assign->lval.get(), funcName) ||
        exprHasSelfCall(assign->rhs.get(), funcName))
      scan.hasBadSelfCall = true;
    return;
  }

  if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
    if (exprHasSelfCall(exprStmt->expr.get(), funcName))
      scan.hasBadSelfCall = true;
    return;
  }
}

void collectCallsInExpr(const Expr* expr,
                        std::unordered_set<std::string>& calls) {
  if (!expr)
    return;
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    calls.insert(call->name);
    for (const auto& arg : call->args)
      collectCallsInExpr(arg.get(), calls);
    return;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    collectCallsInExpr(binary->lhs.get(), calls);
    collectCallsInExpr(binary->rhs.get(), calls);
    return;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    collectCallsInExpr(unary->operand.get(), calls);
    return;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    collectCallsInExpr(cast->operand.get(), calls);
    return;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& index : lval->indices)
      collectCallsInExpr(index.get(), calls);
  }
}

// 递归收集循环体内出现的函数调用名：包括循环条件、循环内声明初值
// 与循环内分支条件，用于判断改写后是否会改变循环中的调用行为。
void collectCallsInsideLoops(const Stmt* stmt, bool insideLoop,
                             std::unordered_set<std::string>& calls) {
  if (!stmt)
    return;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt) {
        collectCallsInsideLoops(item.stmt.get(), insideLoop, calls);
      } else if (insideLoop && item.decl) {
        for (const auto& def : item.decl->defs) {
          auto* init = def && def->initVal
                           ? dynamic_cast<const ScalarInit*>(
                                 def->initVal.get())
                           : nullptr;
          if (init)
            collectCallsInExpr(init->expr.get(), calls);
        }
      }
    }
    return;
  }
  if (auto* loop = dynamic_cast<const WhileStmt*>(stmt)) {
    collectCallsInExpr(loop->cond.get(), calls);
    collectCallsInsideLoops(loop->body.get(), true, calls);
    return;
  }
  if (auto* branch = dynamic_cast<const IfStmt*>(stmt)) {
    if (insideLoop)
      collectCallsInExpr(branch->cond.get(), calls);
    collectCallsInsideLoops(branch->thenBranch.get(), insideLoop, calls);
    collectCallsInsideLoops(branch->elseBranch.get(), insideLoop, calls);
    return;
  }
  if (!insideLoop)
    return;
  if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    collectCallsInExpr(assign->lval.get(), calls);
    collectCallsInExpr(assign->rhs.get(), calls);
  } else if (auto* expression = dynamic_cast<const ExprStmt*>(stmt)) {
    collectCallsInExpr(expression->expr.get(), calls);
  } else if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt)) {
    collectCallsInExpr(ret->val.get(), calls);
  }
}

std::unique_ptr<BlockStmt> makeTailLoopBlock(FuncDef* func, CallExpr* call,
                                             int& tempCounter) {
  auto block = std::make_unique<BlockStmt>();
  block->line = call->line;

  std::vector<std::string> tempNames;
  tempNames.reserve(call->args.size());

  for (size_t i = 0; i < call->args.size(); ++i) {
    const auto& param = func->params[i];
    std::string tempName = "__tailrec_" + func->name + "_" +
                           std::to_string(tempCounter++) + "_" +
                           std::to_string(i);
    tempNames.push_back(tempName);

    auto decl = std::make_unique<DeclStmt>();
    decl->isConst = false;
    decl->baseType = param->type.base;

    auto def = std::make_unique<VarDef>();
    def->name = tempName;
    def->isConst = false;
    def->type.base = param->type.base;
    auto init = std::make_unique<ScalarInit>();
    init->expr = std::move(call->args[i]);
    def->initVal = std::move(init);
    decl->defs.push_back(std::move(def));

    BlockStmt::Item item;
    item.kind = BlockStmt::Item::Kind::Decl;
    item.decl = std::move(decl);
    block->items.push_back(std::move(item));
  }

  for (size_t i = 0; i < func->params.size(); ++i) {
    auto assign = std::make_unique<AssignStmt>();
    assign->line = call->line;
    assign->lval = makeVarExpr(func->params[i]->name, call->line);
    assign->rhs = makeVarExpr(tempNames[i], call->line);

    BlockStmt::Item item;
    item.kind = BlockStmt::Item::Kind::Stmt;
    item.stmt = std::move(assign);
    block->items.push_back(std::move(item));
  }

  auto cont = std::make_unique<ContinueStmt>();
  cont->line = call->line;
  BlockStmt::Item contItem;
  contItem.kind = BlockStmt::Item::Kind::Stmt;
  contItem.stmt = std::move(cont);
  block->items.push_back(std::move(contItem));
  return block;
}

bool rewriteTailReturns(StmtPtr& stmt, FuncDef* func, int& tempCounter) {
  if (!stmt)
    return false;

  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt.get())) {
    auto* call = ret->val ? dynamic_cast<CallExpr*>(ret->val.get()) : nullptr;
    if (!call || call->name != func->name ||
        call->args.size() != func->params.size())
      return false;
    stmt = makeTailLoopBlock(func, call, tempCounter);
    return true;
  }

  if (auto* block = dynamic_cast<BlockStmt*>(stmt.get())) {
    bool changed = false;
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt)
        changed |= rewriteTailReturns(item.stmt, func, tempCounter);
    }
    return changed;
  }

  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
    bool changed = false;
    changed |= rewriteTailReturns(ifStmt->thenBranch, func, tempCounter);
    changed |= rewriteTailReturns(ifStmt->elseBranch, func, tempCounter);
    return changed;
  }

  return false;
}

struct CoreTailCallSite {
  core_ir::BasicBlock* block = nullptr;
  core_ir::Instruction* call = nullptr;
  core_ir::Instruction* terminator = nullptr;
  core_ir::Instruction* returnPhi = nullptr;
};

bool isSelfTailCall(const core_ir::Instruction* call,
                    const core_ir::Function* func) {
  return call && func &&
         call->opcode() == core_ir::Opcode::Call &&
         call->name() == func->name() &&
         call->numOperands() == static_cast<int>(func->args().size());
}

bool blockIsPlainReturnBlock(const core_ir::BasicBlock* bb) {
  if (!bb)
    return false;
  bool sawRet = false;
  for (const auto& instPtr : bb->instructions()) {
    if (!instPtr)
      return false;
    if (instPtr->opcode() == core_ir::Opcode::Phi)
      continue;
    if (instPtr->opcode() == core_ir::Opcode::Ret) {
      sawRet = true;
      continue;
    }
    return false;
  }
  return sawRet;
}

bool phiHasIncoming(const core_ir::Instruction* phi,
                    const core_ir::BasicBlock* pred,
                    const core_ir::Value* value) {
  if (!phi || phi->opcode() != core_ir::Opcode::Phi)
    return false;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i) == value && phi->operand(i + 1) == pred)
      return true;
  }
  return false;
}

bool removePhiIncomingFrom(core_ir::Instruction* phi,
                           const core_ir::BasicBlock* pred) {
  if (!phi || phi->opcode() != core_ir::Opcode::Phi)
    return false;

  std::vector<core_ir::Value*> kept;
  kept.reserve(phi->numOperands());
  bool removed = false;
  for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
    if (phi->operand(i + 1) == pred) {
      removed = true;
      continue;
    }
    kept.push_back(phi->operand(i));
    kept.push_back(phi->operand(i + 1));
  }
  if (!removed)
    return false;

  phi->dropAllOperands();
  for (auto* operand : kept)
    phi->addOperand(operand);
  return true;
}

bool returnConsumesTailCall(core_ir::Instruction* ret,
                            core_ir::BasicBlock* pred,
                            core_ir::Instruction* call,
                            core_ir::Instruction*& returnPhi) {
  returnPhi = nullptr;
  if (!ret || ret->opcode() != core_ir::Opcode::Ret || !call)
    return false;

  if (ret->numOperands() == 0)
    return call->type().isVoid();
  if (ret->numOperands() != 1)
    return false;
  if (ret->operand(0) == call)
    return true;

  auto* phi = dynamic_cast<core_ir::Instruction*>(ret->operand(0));
  if (!phi || phi->opcode() != core_ir::Opcode::Phi)
    return false;
  if (!phiHasIncoming(phi, pred, call))
    return false;
  returnPhi = phi;
  return true;
}

std::vector<CoreTailCallSite> collectCoreTailCallSites(
    core_ir::Function* func) {
  std::vector<CoreTailCallSite> sites;
  if (!func)
    return sites;

  for (auto& bbPtr : func->blocks()) {
    core_ir::BasicBlock* bb = bbPtr.get();
    auto& insts = bb->instructions();
    const int n = static_cast<int>(insts.size());
    if (n < 2)
      continue;

    core_ir::Instruction* term = insts[n - 1].get();
    core_ir::Instruction* call = insts[n - 2].get();
    if (!isSelfTailCall(call, func))
      continue;

    if (term->opcode() == core_ir::Opcode::Ret) {
      core_ir::Instruction* returnPhi = nullptr;
      if (!returnConsumesTailCall(term, bb, call, returnPhi))
        continue;
      sites.push_back({bb, call, term, returnPhi});
      continue;
    }

    if (term->opcode() != core_ir::Opcode::Br || term->numOperands() != 1)
      continue;
    auto* retBlock = dynamic_cast<core_ir::BasicBlock*>(term->operand(0));
    if (!blockIsPlainReturnBlock(retBlock))
      continue;
    core_ir::Instruction* ret = retBlock->terminator();
    core_ir::Instruction* returnPhi = nullptr;
    if (!returnConsumesTailCall(ret, bb, call, returnPhi))
      continue;

    sites.push_back({bb, call, term, returnPhi});
  }

  return sites;
}

// 尾调用消除要求函数中的递归边都属于尾调用边。混合递归函数可能同时包含
// 尾调用边和产生值的递归边（例如一条分支直接返回 f(...)，另一条分支再把
// f(...) 与加载结果组合）。只改写尾调用边会改变递归 CFG，也会影响后续递归
// 记忆化阶段识别辅助结构；这种情况保留整个函数。
bool hasNonTailSelfCall(
    const core_ir::Function* func,
    const std::vector<CoreTailCallSite>& tailSites) {
  if (!func)
    return true;

  std::unordered_set<const core_ir::Instruction*> tailCalls;
  for (const auto& site : tailSites) {
    if (site.call)
      tailCalls.insert(site.call);
  }

  for (const auto& bbPtr : func->blocks()) {
    const core_ir::BasicBlock* bb = bbPtr.get();
    if (!bb)
      continue;
    for (const auto& instPtr : bb->instructions()) {
      const core_ir::Instruction* inst = instPtr.get();
      if (!isSelfTailCall(inst, func))
        continue;
      if (!tailCalls.count(inst))
        return true;
    }
  }
  return false;
}

void replaceValueUsesExcept(core_ir::Value* oldValue,
                            core_ir::Value* newValue,
                            const std::unordered_set<core_ir::Instruction*>&
                                excludedUsers) {
  if (!oldValue || !newValue || oldValue == newValue)
    return;

  auto users = oldValue->uses();
  for (auto* user : users) {
    if (!user || excludedUsers.count(user))
      continue;
    for (int i = 0; i < user->numOperands(); ++i) {
      if (user->operand(i) == oldValue)
        user->setOperand(i, newValue);
    }
  }
}

} // namespace

bool eliminateTailRecursion(core_ir::Module& mod) {
  return eliminateTailRecursion(mod, nullptr);
}

bool eliminateTailRecursion(
    core_ir::Module& mod,
    const std::unordered_set<std::string>* blockedFunctions) {
  if (sycEnvironmentValue("SYC_SKIP_COREIR_TAIL_RECURSION"))
    return false;
  bool changed = false;

  for (auto& funcPtr : mod.functions()) {
    core_ir::Function* func = funcPtr.get();
    // main 是程序入口，调用约定和可观察入口语义不同；这里不是性能特判。
    if (func->name() == "main") continue;
    if (shouldBlockRecursiveRewrite(func->name(), blockedFunctions))
      continue;
    // 已经插入 memo 全局的函数必须保留其递归缓存结构，
    // 不允许再被晚期尾递归消成普通自循环。
    if (hasMemoScaffolding(mod, func))
      continue;
    if (analysis::instructionCount(func) > 2000) continue;

    core_ir::BasicBlock* oldEntry = func->entryBlock();
    if (!oldEntry)
      continue;

    auto sites = collectCoreTailCallSites(func);
    if (sites.empty())
      continue;
    if (hasNonTailSelfCall(func, sites))
      continue;

    // CoreIR 的函数实参是 SSA Value，不能像栈槽那样直接 store。
    // 标准做法是在原入口前插入一个真实 preheader，让原入口参数变成 phi：
    //   arg.loop = phi [arg.abi, preheader], [recursive.arg, tail.block]
    // 然后尾调用块跳回原入口。prependBlock 保证 MIR/汇编布局仍把新入口当首块。
    auto* preheader = func->prependBlock("tailrec.preheader");
    auto* entryBr = new core_ir::Instruction(core_ir::Opcode::Br,
                                             core_ir::Type::voidTy());
    entryBr->addOperand(oldEntry);
    preheader->pushBack(entryBr);
    func->setEntryBlock(preheader);

    const auto& args = func->args();
    std::vector<core_ir::Instruction*> argPhis(args.size(), nullptr);
    for (int i = static_cast<int>(args.size()) - 1; i >= 0; --i) {
      auto* phi = new core_ir::Instruction(
          core_ir::Opcode::Phi,
          args[i].type,
          "__tailrec.arg." + args[i].name);
      oldEntry->pushFront(phi);
      argPhis[i] = phi;
    }

    std::unordered_set<core_ir::Instruction*> excludedUsers;
    for (auto* phi : argPhis)
      excludedUsers.insert(phi);
    for (size_t i = 0; i < args.size(); ++i)
      replaceValueUsesExcept(args[i].val, argPhis[i], excludedUsers);

    for (size_t i = 0; i < args.size(); ++i) {
      argPhis[i]->addOperand(args[i].val);
      argPhis[i]->addOperand(preheader);
    }

    for (const auto& site : sites) {
      for (size_t i = 0; i < args.size(); ++i) {
        argPhis[i]->addOperand(site.call->operand(static_cast<int>(i)));
        argPhis[i]->addOperand(site.block);
      }

      if (site.returnPhi)
        removePhiIncomingFrom(site.returnPhi, site.block);

      site.block->remove(site.terminator);
      site.block->remove(site.call);
      auto* backedge = new core_ir::Instruction(core_ir::Opcode::Br,
                                                core_ir::Type::voidTy());
      backedge->addOperand(oldEntry);
      site.block->pushBack(backedge);
    }

    func->recomputeCFG();
    changed = true;
    if (sycEnvironmentValue("SYC_TRACE_COREIR_TAIL_RECURSION"))
      std::fprintf(stderr, "[coreir-tail-recursion] function=%s sites=%zu\n",
                   func->name().c_str(), sites.size());
  }

  return changed;
}

bool eliminateTailRecursion(CompUnit& cu) {
  return eliminateTailRecursion(cu, nullptr);
}

bool eliminateTailRecursion(
    CompUnit& cu,
    const std::unordered_set<std::string>* blockedFunctions) {
  bool changed = false;
  std::unordered_set<std::string> callsInsideLoops;
  for (const auto& item : cu.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
        item.func->body)
      collectCallsInsideLoops(item.func->body.get(), false, callsInsideLoops);
  }

  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func ||
        // main 是程序入口，不能像普通 helper 一样改写成自循环 wrapper。
        !item.func->body || item.func->name == "main")
      continue;
    if (shouldBlockRecursiveRewrite(item.func->name, blockedFunctions))
      continue;
    if (item.func->params.empty())
      continue;

    bool allScalar = true;
    for (const auto& param : item.func->params) {
      if (!isScalarParam(param.get())) {
        allScalar = false;
        break;
      }
    }
    if (!allScalar)
      continue;

    if (stmtContainsLoop(item.func->body.get()))
      continue;

    TailScan scan;
    scanStmt(item.func->body.get(), item.func->name, scan);
    if (!scan.hasTailCall || scan.hasBadSelfCall)
      continue;
    // 多条递归转移通常描述一个小型状态机。外层循环重复调用时，保留递归可以
    // 让后续持久化记忆化阶段跨迭代复用状态；改成紧凑循环虽能减小代码体积，
    // 却会重复每次状态遍历。
    if (scan.tailCallCount > 1 &&
        callsInsideLoops.count(item.func->name))
      continue;

    int tempCounter = 0;
    auto oldBody = std::move(item.func->body);
    StmtPtr bodyStmt = std::move(oldBody);
    if (!rewriteTailReturns(bodyStmt, item.func.get(), tempCounter)) {
      item.func->body.reset(dynamic_cast<BlockStmt*>(bodyStmt.release()));
      continue;
    }

    auto* newBodyBlock = dynamic_cast<BlockStmt*>(bodyStmt.release());
    if (!newBodyBlock) {
      item.func->body = std::move(oldBody);
      continue;
    }

    auto loop = std::make_unique<WhileStmt>();
    loop->line = item.func->line;
    loop->cond = makeIntLit(1, item.func->line);
    loop->body.reset(newBodyBlock);

    auto wrapper = std::make_unique<BlockStmt>();
    wrapper->line = item.func->line;
    BlockStmt::Item loopItem;
    loopItem.kind = BlockStmt::Item::Kind::Stmt;
    loopItem.stmt = std::move(loop);
    wrapper->items.push_back(std::move(loopItem));
    item.func->body = std::move(wrapper);
    changed = true;
    if (sycEnvironmentValue("SYC_TRACE_AST_TAIL_RECURSION"))
      std::fprintf(stderr, "[ast-tail-recursion] function=%s params=%zu calls=%d\n",
                   item.func->name.c_str(), item.func->params.size(),
                   scan.tailCallCount);
  }

  return changed;
}

bool pruneTrivialSelfRecursion(CompUnit& cu) {
  return pruneTrivialSelfRecursion(cu, nullptr);
}

bool pruneTrivialSelfRecursion(
    CompUnit& cu,
    const std::unordered_set<std::string>* blockedFunctions) {
  bool changed = false;

  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func ||
        // main 是程序入口，递归裁剪只处理普通函数体。
        !item.func->body || item.func->name == "main")
      continue;
    if (shouldBlockRecursiveRewrite(item.func->name, blockedFunctions))
      continue;

    const Expr* baseCond = findLeadingBaseCaseCond(item.func.get());
    if (!baseCond || exprHasAnyCall(baseCond))
      continue;

    std::unordered_set<std::string> paramNames;
    for (const auto& param : item.func->params) {
      if (!param)
        continue;
      paramNames.insert(param->name);
    }
    if (!exprReferencesOnlyParams(baseCond, paramNames))
      continue;

    ExprPtr baseCondCopy = cloneExpr(baseCond);
    if (!baseCondCopy)
      continue;

    StmtPtr bodyStmt = std::move(item.func->body);
    bool funcChanged =
        rewritePrunableSelfCalls(bodyStmt, item.func.get(), baseCondCopy.get());
    auto* newBody = dynamic_cast<BlockStmt*>(bodyStmt.release());
    if (!newBody) {
      item.func->body = std::make_unique<BlockStmt>();
      continue;
    }
    item.func->body.reset(newBody);
    changed |= funcChanged;
  }

  return changed;
}

} // namespace hir
