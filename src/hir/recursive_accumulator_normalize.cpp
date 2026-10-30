#include "research_switch.h"
// 文件职责：把递归 accumulator 形态归一化，便于后续尾递归和 helper 优化。本文件负责实现具体逻辑。
#include "recursive_accumulator_normalize.h"

#include "../frontend/ast.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hir {

namespace {

ExprPtr cloneExpr(const Expr* expr);
StmtPtr cloneStmt(const Stmt* stmt);
std::unique_ptr<BlockStmt> cloneBlock(const BlockStmt* block);
InitValPtr cloneInitVal(const InitVal* init);
std::unique_ptr<DeclStmt> cloneDecl(const DeclStmt* decl);

// 从首选名称生成未被占用的标识符。无冲突时保留原名称，避免改变
// 已有程序的输出；发生冲突时追加递增后缀，并立即登记新名称。
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
  if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt))
    collectDeclaredNames(whileStmt->body.get(), names);
}

std::unordered_set<std::string> collectFunctionNames(const CompUnit& cu) {
  std::unordered_set<std::string> names;
  for (const auto& item : cu.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func)
      names.insert(item.func->name);
  }
  return names;
}

std::unordered_set<std::string> collectLocalNames(const FuncDef& func) {
  std::unordered_set<std::string> names;
  for (const auto& param : func.params) {
    if (param)
      names.insert(param->name);
  }
  collectDeclaredNames(func.body.get(), names);
  return names;
}

// 深拷贝表达式树：按节点类型分派逐层克隆，不支持的节点返回 nullptr，
// 供调用方放弃规范化。
ExprPtr cloneExpr(const Expr* expr) {
  if (!expr)
    return nullptr;
  if (auto* node = dynamic_cast<const IntLitExpr*>(expr)) {
    auto out = std::make_unique<IntLitExpr>();
    out->line = node->line;
    out->val = node->val;
    return out;
  }
  if (auto* node = dynamic_cast<const FloatLitExpr*>(expr)) {
    auto out = std::make_unique<FloatLitExpr>();
    out->line = node->line;
    out->val = node->val;
    return out;
  }
  if (auto* node = dynamic_cast<const LValExpr*>(expr)) {
    auto out = std::make_unique<LValExpr>();
    out->line = node->line;
    out->name = node->name;
    out->symbol = node->symbol;
    for (const auto& idx : node->indices)
      out->indices.push_back(cloneExpr(idx.get()));
    return out;
  }
  if (auto* node = dynamic_cast<const CallExpr*>(expr)) {
    auto out = std::make_unique<CallExpr>();
    out->line = node->line;
    out->name = node->name;
    out->callee = node->callee;
    for (const auto& arg : node->args)
      out->args.push_back(cloneExpr(arg.get()));
    return out;
  }
  if (auto* node = dynamic_cast<const CastExpr*>(expr)) {
    auto out = std::make_unique<CastExpr>();
    out->line = node->line;
    out->targetType = node->targetType;
    out->operand = cloneExpr(node->operand.get());
    return out;
  }
  if (auto* node = dynamic_cast<const UnaryExpr*>(expr)) {
    auto out = std::make_unique<UnaryExpr>();
    out->line = node->line;
    out->op = node->op;
    out->operand = cloneExpr(node->operand.get());
    return out;
  }
  if (auto* node = dynamic_cast<const BinaryExpr*>(expr)) {
    auto out = std::make_unique<BinaryExpr>();
    out->line = node->line;
    out->op = node->op;
    out->lhs = cloneExpr(node->lhs.get());
    out->rhs = cloneExpr(node->rhs.get());
    return out;
  }
  return nullptr;
}

InitValPtr cloneInitVal(const InitVal* init) {
  if (!init)
    return nullptr;
  if (auto* node = dynamic_cast<const ScalarInit*>(init)) {
    auto out = std::make_unique<ScalarInit>();
    out->line = node->line;
    out->expr = cloneExpr(node->expr.get());
    return out;
  }
  if (auto* node = dynamic_cast<const AggregateInit*>(init)) {
    auto out = std::make_unique<AggregateInit>();
    out->line = node->line;
    for (const auto& item : node->items)
      out->items.push_back(cloneInitVal(item.get()));
    return out;
  }
  return nullptr;
}

std::unique_ptr<DeclStmt> cloneDecl(const DeclStmt* decl) {
  if (!decl)
    return nullptr;
  auto out = std::make_unique<DeclStmt>();
  out->line = decl->line;
  out->isConst = decl->isConst;
  out->baseType = decl->baseType;
  for (const auto& def : decl->defs) {
    auto newDef = std::make_unique<VarDef>();
    newDef->line = def->line;
    newDef->name = def->name;
    newDef->isConst = def->isConst;
    newDef->type = def->type;
    newDef->initVal = cloneInitVal(def->initVal.get());
    out->defs.push_back(std::move(newDef));
  }
  return out;
}

// 深拷贝语句：按语句类型分派，递归克隆其中包含的表达式与子语句，
// BlockStmt 交给 cloneBlock 处理。
StmtPtr cloneStmt(const Stmt* stmt) {
  if (!stmt)
    return nullptr;
  if (auto* node = dynamic_cast<const AssignStmt*>(stmt)) {
    auto out = std::make_unique<AssignStmt>();
    out->line = node->line;
    out->lval = cloneExpr(node->lval.get());
    out->rhs = cloneExpr(node->rhs.get());
    return out;
  }
  if (auto* node = dynamic_cast<const ExprStmt*>(stmt)) {
    auto out = std::make_unique<ExprStmt>();
    out->line = node->line;
    out->expr = cloneExpr(node->expr.get());
    return out;
  }
  if (auto* node = dynamic_cast<const BlockStmt*>(stmt))
    return cloneBlock(node);
  if (auto* node = dynamic_cast<const IfStmt*>(stmt)) {
    auto out = std::make_unique<IfStmt>();
    out->line = node->line;
    out->cond = cloneExpr(node->cond.get());
    out->thenBranch = cloneStmt(node->thenBranch.get());
    out->elseBranch = cloneStmt(node->elseBranch.get());
    return out;
  }
  if (auto* node = dynamic_cast<const WhileStmt*>(stmt)) {
    auto out = std::make_unique<WhileStmt>();
    out->line = node->line;
    out->cond = cloneExpr(node->cond.get());
    out->body = cloneStmt(node->body.get());
    return out;
  }
  if (dynamic_cast<const BreakStmt*>(stmt)) {
    auto out = std::make_unique<BreakStmt>();
    out->line = stmt->line;
    return out;
  }
  if (dynamic_cast<const ContinueStmt*>(stmt)) {
    auto out = std::make_unique<ContinueStmt>();
    out->line = stmt->line;
    return out;
  }
  if (auto* node = dynamic_cast<const ReturnStmt*>(stmt)) {
    auto out = std::make_unique<ReturnStmt>();
    out->line = node->line;
    out->val = cloneExpr(node->val.get());
    return out;
  }
  return nullptr;
}

std::unique_ptr<BlockStmt> cloneBlock(const BlockStmt* block) {
  if (!block)
    return nullptr;
  auto out = std::make_unique<BlockStmt>();
  out->line = block->line;
  for (const auto& item : block->items) {
    BlockStmt::Item cloned;
    cloned.kind = item.kind;
    if (item.kind == BlockStmt::Item::Kind::Decl)
      cloned.decl = cloneDecl(item.decl.get());
    else
      cloned.stmt = cloneStmt(item.stmt.get());
    out->items.push_back(std::move(cloned));
  }
  return out;
}

std::unique_ptr<IntLitExpr> makeIntLit(int32_t value, int line) {
  auto expr = std::make_unique<IntLitExpr>();
  expr->line = line;
  expr->val = value;
  return expr;
}

std::unique_ptr<LValExpr> makeVarExpr(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  return expr;
}

std::unique_ptr<BinaryExpr> makeBinary(const std::string& op,
                                       ExprPtr lhs,
                                       ExprPtr rhs,
                                       int line) {
  auto expr = std::make_unique<BinaryExpr>();
  expr->line = line;
  expr->op = op;
  expr->lhs = std::move(lhs);
  expr->rhs = std::move(rhs);
  return expr;
}

std::unique_ptr<CallExpr> makeCall(const std::string& name,
                                   std::vector<ExprPtr> args,
                                   int line) {
  auto expr = std::make_unique<CallExpr>();
  expr->line = line;
  expr->name = name;
  expr->args = std::move(args);
  return expr;
}

bool sameScalarVar(const Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.empty();
}

bool exprMentionsVar(const Expr* expr, const std::string& name) {
  if (!expr)
    return false;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->name == name)
      return true;
    for (const auto& idx : lval->indices) {
      if (exprMentionsVar(idx.get(), name))
        return true;
    }
    return false;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    for (const auto& arg : call->args) {
      if (exprMentionsVar(arg.get(), name))
        return true;
    }
    return false;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprMentionsVar(cast->operand.get(), name);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return exprMentionsVar(unary->operand.get(), name);
  if (auto* bin = dynamic_cast<const BinaryExpr*>(expr))
    return exprMentionsVar(bin->lhs.get(), name) ||
           exprMentionsVar(bin->rhs.get(), name);
  return false;
}

bool exprHasSelfCall(const Expr* expr, const std::string& funcName) {
  if (!expr)
    return false;
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    if (call->name == funcName)
      return true;
    for (const auto& arg : call->args) {
      if (exprHasSelfCall(arg.get(), funcName))
        return true;
    }
    return false;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& idx : lval->indices) {
      if (exprHasSelfCall(idx.get(), funcName))
        return true;
    }
    return false;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprHasSelfCall(cast->operand.get(), funcName);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return exprHasSelfCall(unary->operand.get(), funcName);
  if (auto* bin = dynamic_cast<const BinaryExpr*>(expr))
    return exprHasSelfCall(bin->lhs.get(), funcName) ||
           exprHasSelfCall(bin->rhs.get(), funcName);
  return false;
}

bool stmtHasLoop(const Stmt* stmt) {
  if (!stmt)
    return false;
  if (dynamic_cast<const WhileStmt*>(stmt))
    return true;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt &&
          stmtHasLoop(item.stmt.get()))
        return true;
    }
    return false;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt))
    return stmtHasLoop(ifStmt->thenBranch.get()) ||
           stmtHasLoop(ifStmt->elseBranch.get());
  return false;
}

bool declHasSelfCall(const DeclStmt* decl, const std::string& funcName) {
  if (!decl)
    return false;
  for (const auto& def : decl->defs) {
    if (!def || !def->initVal)
      continue;
    if (auto* scalar = dynamic_cast<const ScalarInit*>(def->initVal.get())) {
      if (exprHasSelfCall(scalar->expr.get(), funcName))
        return true;
      continue;
    }
    if (dynamic_cast<const AggregateInit*>(def->initVal.get()))
      return true;
  }
  return false;
}

bool declMentionsVar(const DeclStmt* decl, const std::string& name) {
  if (!decl)
    return false;
  for (const auto& def : decl->defs) {
    if (!def || !def->initVal)
      continue;
    if (auto* scalar = dynamic_cast<const ScalarInit*>(def->initVal.get())) {
      if (exprMentionsVar(scalar->expr.get(), name))
        return true;
      continue;
    }
    if (dynamic_cast<const AggregateInit*>(def->initVal.get()))
      return true;
  }
  return false;
}

struct NormalizePattern {
  bool valid = false;
  int accumIndex = -1;
  int delta = -1;
  std::optional<int32_t> sentinel;
  bool sawAccumulatorBase = false;
  bool sawRecursiveTail = false;
};

struct NormalizedFunctionInfo {
  std::string wrapperName;
  std::string helperName;
  int accumIndex = -1;
  bool helperDirectCallPreservesValue = false;
};

bool needsTaggedDeltaEncoding(const NormalizePattern& pattern) {
  return pattern.sentinel.has_value() && *pattern.sentinel >= 0;
}

bool recursiveAccumulatorTraceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_AST_RECURSIVE_ACCUMULATOR_NORMALIZE") !=
         nullptr;
}

bool matchAccumulatorStep(const Expr* expr,
                          const std::string& accumName,
                          int& deltaOut) {
  auto* bin = dynamic_cast<const BinaryExpr*>(expr);
  if (!bin || bin->op != "+")
    return false;
  const IntLitExpr* lit = nullptr;
  if (sameScalarVar(bin->lhs.get(), accumName))
    lit = dynamic_cast<const IntLitExpr*>(bin->rhs.get());
  else if (sameScalarVar(bin->rhs.get(), accumName))
    lit = dynamic_cast<const IntLitExpr*>(bin->lhs.get());
  if (!lit || lit->val < 0)
    return false;
  deltaOut = lit->val;
  return true;
}

bool exprIsZero(const Expr* expr) {
  if (!expr)
    return false;
  if (auto* lit = dynamic_cast<const IntLitExpr*>(expr))
    return lit->val == 0;
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprIsZero(cast->operand.get());
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    if (unary->op == '+' || unary->op == '-')
      return exprIsZero(unary->operand.get());
  }
  return false;
}

// 分类 return 形态：直接返回累加器本身是基准情形；整型字面量是哨兵值
// （所有哨兵返回须一致）；自调用要求累加器实参按固定 delta 步进，
// 其余实参不得引用累加器。
bool analyzeReturn(const ReturnStmt* ret,
                   const FuncDef* func,
                   const std::string& accumName,
                   NormalizePattern& pattern) {
  if (!ret || !ret->val)
    return false;
  if (sameScalarVar(ret->val.get(), accumName)) {
    pattern.sawAccumulatorBase = true;
    return true;
  }
  if (auto* ci = dynamic_cast<const IntLitExpr*>(ret->val.get())) {
    if (!pattern.sentinel)
      pattern.sentinel = ci->val;
    return pattern.sentinel == ci->val;
  }
  auto* call = dynamic_cast<const CallExpr*>(ret->val.get());
  if (!call || call->name != func->name ||
      call->args.size() != func->params.size()) {
    return false;
  }
  int matchedDelta = -1;
  bool sawAccumArg = false;
  for (size_t i = 0; i < call->args.size(); ++i) {
    const Expr* arg = call->args[i].get();
    if (static_cast<int>(i) == pattern.accumIndex) {
      if (!matchAccumulatorStep(arg, accumName, matchedDelta))
        return false;
      sawAccumArg = true;
      continue;
    }
    if (exprMentionsVar(arg, accumName))
      return false;
  }
  if (!sawAccumArg)
    return false;
  if (pattern.delta < 0)
    pattern.delta = matchedDelta;
  if (pattern.delta != matchedDelta)
    return false;
  pattern.sawRecursiveTail = true;
  return true;
}

bool analyzeStmt(const Stmt* stmt,
                 const FuncDef* func,
                 const std::string& accumName,
                 NormalizePattern& pattern) {
  if (!stmt)
    return true;
  if (dynamic_cast<const WhileStmt*>(stmt))
    return false;
  if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt))
    return analyzeReturn(ret, func, accumName, pattern);
  if (auto* exprStmt = dynamic_cast<const ExprStmt*>(stmt))
    return !exprHasSelfCall(exprStmt->expr.get(), func->name) &&
           !exprMentionsVar(exprStmt->expr.get(), accumName);
  if (auto* assign = dynamic_cast<const AssignStmt*>(stmt))
    return !exprHasSelfCall(assign->rhs.get(), func->name) &&
           !exprMentionsVar(assign->lval.get(), accumName) &&
           !exprMentionsVar(assign->rhs.get(), accumName);
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt) {
        if (!analyzeStmt(item.stmt.get(), func, accumName, pattern))
          return false;
      } else if (declHasSelfCall(item.decl.get(), func->name) ||
                 declMentionsVar(item.decl.get(), accumName)) {
        return false;
      }
    }
    return true;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    if (exprHasSelfCall(ifStmt->cond.get(), func->name) ||
        exprMentionsVar(ifStmt->cond.get(), accumName))
      return false;
    return analyzeStmt(ifStmt->thenBranch.get(), func, accumName, pattern) &&
           analyzeStmt(ifStmt->elseBranch.get(), func, accumName, pattern);
  }
  return true;
}

bool collectPattern(const FuncDef* func, NormalizePattern& pattern) {
  if (!func || !func->body || func->retType.base != BaseType::Int)
    return false;
  if (stmtHasLoop(func->body.get()))
    return false;
  for (const auto& param : func->params) {
    if (!param)
      return false;
  }
  for (size_t i = 0; i < func->params.size(); ++i) {
    const auto& accumParam = func->params[i];
    if (!accumParam || accumParam->type.isArray() ||
        accumParam->type.base != BaseType::Int)
      continue;
    NormalizePattern trial;
    trial.accumIndex = static_cast<int>(i);
    const std::string& accumName = accumParam->name;
    if (!analyzeStmt(func->body.get(), func, accumName, trial))
      continue;
    if (trial.sawAccumulatorBase && trial.sawRecursiveTail &&
        trial.delta >= 0 && trial.sentinel.has_value()) {
      trial.valid = true;
      pattern = trial;
      return true;
    }
  }
  return false;
}

std::unique_ptr<BlockStmt> makeSentinelAwareHelperBlock(
    const std::string& helperName,
    std::vector<ExprPtr> helperArgs,
    int sentinel,
    int delta,
    bool taggedDeltaEncoding,
    int line,
    int& tempCounter,
    std::unordered_set<std::string>& usedLocalNames) {
  auto block = std::make_unique<BlockStmt>();
  block->line = line;

  const std::string tempName = takeFreshName(
      "__norm_tmp_" + std::to_string(tempCounter++), usedLocalNames);

  auto decl = std::make_unique<DeclStmt>();
  decl->line = line;
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  auto def = std::make_unique<VarDef>();
  def->line = line;
  def->name = tempName;
  def->isConst = false;
  def->type.base = BaseType::Int;
  auto init = std::make_unique<ScalarInit>();
  init->line = line;
  init->expr = makeCall(helperName, std::move(helperArgs), line);
  def->initVal = std::move(init);
  decl->defs.push_back(std::move(def));

  BlockStmt::Item declItem;
  declItem.kind = BlockStmt::Item::Kind::Decl;
  declItem.decl = std::move(decl);
  block->items.push_back(std::move(declItem));

  auto ifStmt = std::make_unique<IfStmt>();
  ifStmt->line = line;
  ifStmt->cond = makeBinary("==", makeVarExpr(tempName, line),
                            makeIntLit(sentinel, line), line);
  auto thenRet = std::make_unique<ReturnStmt>();
  thenRet->line = line;
  thenRet->val = makeVarExpr(tempName, line);
  ifStmt->thenBranch = std::move(thenRet);

  BlockStmt::Item ifItem;
  ifItem.kind = BlockStmt::Item::Kind::Stmt;
  ifItem.stmt = std::move(ifStmt);
  block->items.push_back(std::move(ifItem));

  auto finalRet = std::make_unique<ReturnStmt>();
  finalRet->line = line;
  if (taggedDeltaEncoding) {
    finalRet->val = makeBinary("-", makeVarExpr(tempName, line),
                               makeIntLit(delta, line), line);
  } else {
    finalRet->val = makeBinary("+", makeVarExpr(tempName, line),
                               makeIntLit(delta, line), line);
  }
  BlockStmt::Item retItem;
  retItem.kind = BlockStmt::Item::Kind::Stmt;
  retItem.stmt = std::move(finalRet);
  block->items.push_back(std::move(retItem));
  return block;
}

void rewriteHelperReturns(StmtPtr& stmt,
                          const FuncDef* wrapper,
                          const std::string& helperName,
                          const NormalizePattern& pattern,
                          int& tempCounter,
                          std::unordered_set<std::string>& usedLocalNames) {
  if (!stmt)
    return;
  const std::string& accumName = wrapper->params[pattern.accumIndex]->name;
  const bool taggedDeltaEncoding = needsTaggedDeltaEncoding(pattern);
  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt.get())) {
    if (sameScalarVar(ret->val.get(), accumName)) {
      ret->val = makeIntLit(taggedDeltaEncoding ? -1 : 0, ret->line);
      return;
    }
    if (dynamic_cast<IntLitExpr*>(ret->val.get()))
      return;
    auto* call = dynamic_cast<CallExpr*>(ret->val.get());
    if (!call || call->name != wrapper->name ||
        call->args.size() != wrapper->params.size()) {
      return;
    }
    std::vector<ExprPtr> helperArgs;
    helperArgs.reserve(call->args.size() - 1);
    for (size_t i = 0; i < call->args.size(); ++i) {
      if (static_cast<int>(i) == pattern.accumIndex)
        continue;
      helperArgs.push_back(cloneExpr(call->args[i].get()));
    }
    int recursiveDelta = pattern.delta;
    if (!matchAccumulatorStep(call->args[pattern.accumIndex].get(), accumName,
                              recursiveDelta)) {
      return;
    }
    stmt = makeSentinelAwareHelperBlock(helperName, std::move(helperArgs),
                                        *pattern.sentinel, recursiveDelta,
                                        taggedDeltaEncoding,
                                        ret->line, tempCounter,
                                        usedLocalNames);
    return;
  }
  if (auto* block = dynamic_cast<BlockStmt*>(stmt.get())) {
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt)
        rewriteHelperReturns(item.stmt, wrapper, helperName, pattern,
                             tempCounter, usedLocalNames);
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
    rewriteHelperReturns(ifStmt->thenBranch, wrapper, helperName, pattern,
                         tempCounter, usedLocalNames);
    rewriteHelperReturns(ifStmt->elseBranch, wrapper, helperName, pattern,
                         tempCounter, usedLocalNames);
  }
}

std::unique_ptr<BlockStmt> buildWrapperBody(const FuncDef* func,
                                            const std::string& helperName,
                                            const NormalizePattern& pattern,
                                            const std::string& resultName) {
  auto body = std::make_unique<BlockStmt>();
  body->line = func->line;

  auto decl = std::make_unique<DeclStmt>();
  decl->line = func->line;
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  auto def = std::make_unique<VarDef>();
  def->line = func->line;
  def->name = resultName;
  def->isConst = false;
  def->type.base = BaseType::Int;
  auto init = std::make_unique<ScalarInit>();
  init->line = func->line;
  std::vector<ExprPtr> args;
  for (size_t i = 0; i < func->params.size(); ++i) {
    if (static_cast<int>(i) == pattern.accumIndex)
      continue;
    args.push_back(makeVarExpr(func->params[i]->name, func->line));
  }
  init->expr = makeCall(helperName, std::move(args), func->line);
  def->initVal = std::move(init);
  decl->defs.push_back(std::move(def));

  BlockStmt::Item declItem;
  declItem.kind = BlockStmt::Item::Kind::Decl;
  declItem.decl = std::move(decl);
  body->items.push_back(std::move(declItem));

  auto ifStmt = std::make_unique<IfStmt>();
  ifStmt->line = func->line;
  ifStmt->cond = makeBinary(
      "==", makeVarExpr(resultName, func->line),
      makeIntLit(*pattern.sentinel, func->line), func->line);
  auto thenRet = std::make_unique<ReturnStmt>();
  thenRet->line = func->line;
  thenRet->val = makeVarExpr(resultName, func->line);
  ifStmt->thenBranch = std::move(thenRet);

  BlockStmt::Item ifItem;
  ifItem.kind = BlockStmt::Item::Kind::Stmt;
  ifItem.stmt = std::move(ifStmt);
  body->items.push_back(std::move(ifItem));

  auto finalRet = std::make_unique<ReturnStmt>();
  finalRet->line = func->line;
  if (needsTaggedDeltaEncoding(pattern)) {
    auto delta = makeBinary(
        "-", makeIntLit(0, func->line),
        makeVarExpr(resultName, func->line), func->line);
    delta = makeBinary("-", std::move(delta), makeIntLit(1, func->line),
                       func->line);
    finalRet->val = makeBinary(
        "+", std::move(delta),
        makeVarExpr(func->params[pattern.accumIndex]->name, func->line),
        func->line);
  } else {
    finalRet->val = makeBinary(
        "+", makeVarExpr(resultName, func->line),
        makeVarExpr(func->params[pattern.accumIndex]->name, func->line),
        func->line);
  }
  BlockStmt::Item retItem;
  retItem.kind = BlockStmt::Item::Kind::Stmt;
  retItem.stmt = std::move(finalRet);
  body->items.push_back(std::move(retItem));
  return body;
}

// 构造递归辅助函数：剔除累加器形参、整体克隆原函数体，供 wrapper
// 在累加器初值为 0 的调用点直接调用。辅助函数名由调用方提前消除冲突。
std::unique_ptr<FuncDef> buildHelper(const FuncDef* func,
                                     const NormalizePattern& pattern,
                                     const std::string& helperName) {
  auto helper = std::make_unique<FuncDef>();
  helper->line = func->line;
  helper->retType = func->retType;
  helper->name = helperName;
  for (size_t i = 0; i < func->params.size(); ++i) {
    if (static_cast<int>(i) == pattern.accumIndex)
      continue;
    auto param = std::make_unique<FuncParam>();
    param->line = func->params[i]->line;
    param->name = func->params[i]->name;
    param->type = func->params[i]->type;
    helper->params.push_back(std::move(param));
  }
  helper->body = cloneBlock(func->body.get());
  return helper;
}

void rewriteNormalizedCallsInExpr(ExprPtr& expr,
                                  const std::vector<NormalizedFunctionInfo>& infos);

// 语句级遍历：递归下降所有语句类型，把其中每个表达式位置
// （含声明的标量初值表达式）交给表达式级改写。
void rewriteNormalizedCallsInStmt(StmtPtr& stmt,
                                  const std::vector<NormalizedFunctionInfo>& infos) {
  if (!stmt)
    return;
  if (auto* assign = dynamic_cast<AssignStmt*>(stmt.get())) {
    rewriteNormalizedCallsInExpr(assign->lval, infos);
    rewriteNormalizedCallsInExpr(assign->rhs, infos);
    return;
  }
  if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt.get())) {
    rewriteNormalizedCallsInExpr(exprStmt->expr, infos);
    return;
  }
  if (auto* block = dynamic_cast<BlockStmt*>(stmt.get())) {
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt) {
        rewriteNormalizedCallsInStmt(item.stmt, infos);
      } else if (item.decl) {
        for (auto& def : item.decl->defs) {
          if (!def || !def->initVal)
            continue;
          if (auto* scalar = dynamic_cast<ScalarInit*>(def->initVal.get()))
            rewriteNormalizedCallsInExpr(scalar->expr, infos);
        }
      }
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
    rewriteNormalizedCallsInExpr(ifStmt->cond, infos);
    rewriteNormalizedCallsInStmt(ifStmt->thenBranch, infos);
    rewriteNormalizedCallsInStmt(ifStmt->elseBranch, infos);
    return;
  }
  if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
    rewriteNormalizedCallsInExpr(whileStmt->cond, infos);
    rewriteNormalizedCallsInStmt(whileStmt->body, infos);
    return;
  }
  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt.get()))
    rewriteNormalizedCallsInExpr(ret->val, infos);
}

// 表达式级改写：先递归处理子表达式；对累加器实参为 0 的 wrapper 调用，
// 直接改写成去掉累加器实参的 helper 调用，跳过 wrapper 的初始分支。
void rewriteNormalizedCallsInExpr(ExprPtr& expr,
                                  const std::vector<NormalizedFunctionInfo>& infos) {
  if (!expr)
    return;
  if (auto* lval = dynamic_cast<LValExpr*>(expr.get())) {
    for (auto& idx : lval->indices)
      rewriteNormalizedCallsInExpr(idx, infos);
    return;
  }
  if (auto* cast = dynamic_cast<CastExpr*>(expr.get())) {
    rewriteNormalizedCallsInExpr(cast->operand, infos);
    return;
  }
  if (auto* unary = dynamic_cast<UnaryExpr*>(expr.get())) {
    rewriteNormalizedCallsInExpr(unary->operand, infos);
    return;
  }
  if (auto* bin = dynamic_cast<BinaryExpr*>(expr.get())) {
    rewriteNormalizedCallsInExpr(bin->lhs, infos);
    rewriteNormalizedCallsInExpr(bin->rhs, infos);
    return;
  }
  auto* call = dynamic_cast<CallExpr*>(expr.get());
  if (!call)
    return;
  for (auto& arg : call->args)
    rewriteNormalizedCallsInExpr(arg, infos);
  for (const auto& info : infos) {
    if (call->name != info.wrapperName ||
        !info.helperDirectCallPreservesValue ||
        info.accumIndex < 0 ||
        static_cast<size_t>(info.accumIndex) >= call->args.size() ||
        !exprIsZero(call->args[info.accumIndex].get())) {
      continue;
    }
    std::vector<ExprPtr> helperArgs;
    helperArgs.reserve(call->args.size() - 1);
    for (size_t i = 0; i < call->args.size(); ++i) {
      if (static_cast<int>(i) == info.accumIndex)
        continue;
      helperArgs.push_back(cloneExpr(call->args[i].get()));
    }
    expr = makeCall(info.helperName, std::move(helperArgs), call->line);
    return;
  }
}

} // namespace

bool normalizeRecursiveAccumulators(
    CompUnit& cu,
    const std::unordered_set<std::string>* blockedFunctions) {
  bool changed = false;
  std::vector<CompUnit::Item> additions;
  std::vector<NormalizedFunctionInfo> normalizedInfos;
  auto usedFunctionNames = collectFunctionNames(cu);
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func ||
        // main 是入口语义边界；accumulator 归一化只处理普通可封装函数。
        !item.func->body || item.func->name == "main")
      continue;
    if (blockedFunctions && blockedFunctions->count(item.func->name))
      continue;

    NormalizePattern pattern;
    if (!collectPattern(item.func.get(), pattern))
      continue;

    const std::string helperName = takeFreshName(
        "__norm_" + item.func->name, usedFunctionNames);
    auto usedLocalNames = collectLocalNames(*item.func);
    const std::string resultName = takeFreshName(
        "__norm_ret_" + item.func->name, usedLocalNames);
    auto helper = buildHelper(item.func.get(), pattern, helperName);
    if (!helper || !helper->body)
      continue;

    StmtPtr helperBody = std::move(helper->body);
    int tempCounter = 0;
    rewriteHelperReturns(helperBody, item.func.get(), helper->name, pattern,
                         tempCounter, usedLocalNames);
    helper->body.reset(dynamic_cast<BlockStmt*>(helperBody.release()));
    if (!helper->body)
      continue;
    if (recursiveAccumulatorTraceEnabled()) {
      std::fprintf(stderr,
                   "[ast-recursive-accumulator] function=%s accum=%s "
                   "params=%zu delta=%d sentinel=%d\n",
                   item.func->name.c_str(),
                   item.func->params[pattern.accumIndex]->name.c_str(),
                   item.func->params.size(), pattern.delta,
                   pattern.sentinel.value());
    }
    item.func->body = buildWrapperBody(item.func.get(), helper->name, pattern,
                                       resultName);

    CompUnit::Item helperItem;
    helperItem.kind = CompUnit::Item::Kind::FuncDef;
    helperItem.func = std::move(helper);
    additions.push_back(std::move(helperItem));
    normalizedInfos.push_back({item.func->name, helperName, pattern.accumIndex,
                               !needsTaggedDeltaEncoding(pattern)});
    changed = true;
  }

  for (auto& item : additions)
    cu.items.push_back(std::move(item));
  if (!normalizedInfos.empty()) {
    for (auto& item : cu.items) {
      if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
          item.func->body) {
        StmtPtr body = std::move(item.func->body);
        rewriteNormalizedCallsInStmt(body, normalizedInfos);
        item.func->body.reset(dynamic_cast<BlockStmt*>(body.release()));
      }
    }
  }
  return changed;
}

bool normalizeRecursiveAccumulators(CompUnit& cu) {
  return normalizeRecursiveAccumulators(cu, nullptr);
}

} // namespace hir
