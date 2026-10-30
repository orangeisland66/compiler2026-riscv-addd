// 文件职责：在 AST 层整理常量、表达式和无效语句等简单结构。
#include "ast_simplify.h"

#include "../frontend/ast.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {
namespace {

using NameSet = std::unordered_set<std::string>;

struct FunctionFacts {
  NameSet intScalarNames;
};

bool isScalarIntType(const SysType& type) {
  return type.base == BaseType::Int && !type.isArray();
}

bool isPlainLValOf(const Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.empty();
}

std::unique_ptr<LValExpr> makePlainLVal(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->name = name;
  expr->line = line;
  return expr;
}

std::unique_ptr<IntLitExpr> makeIntLit(int32_t value, int line) {
  auto expr = std::make_unique<IntLitExpr>();
  expr->val = value;
  expr->line = line;
  return expr;
}

std::unique_ptr<BinaryExpr> makeAddExpr(const std::string& name,
                                        int32_t delta,
                                        int line) {
  auto expr = std::make_unique<BinaryExpr>();
  expr->op = delta >= 0 ? "+" : "-";
  expr->lhs = makePlainLVal(name, line);
  expr->rhs = makeIntLit(delta >= 0 ? delta : -delta, line);
  expr->line = line;
  return expr;
}

void collectExprUses(const Expr* expr, NameSet& uses);
void collectInitUses(const InitVal* init, NameSet& uses);
void collectStmtUses(const Stmt* stmt, NameSet& uses);
bool simplifyStmt(StmtPtr& stmt, const FunctionFacts& facts);
bool simplifyBlock(BlockStmt& block, const FunctionFacts& facts);

bool exprHasCall(const Expr* expr) {
  if (!expr)
    return false;
  if (dynamic_cast<const CallExpr*>(expr))
    return true;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& idx : lval->indices)
      if (exprHasCall(idx.get()))
        return true;
    return false;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprHasCall(cast->operand.get());
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return exprHasCall(unary->operand.get());
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return exprHasCall(binary->lhs.get()) || exprHasCall(binary->rhs.get());
  return false;
}

bool initHasCall(const InitVal* init) {
  if (!init)
    return false;
  if (auto* scalar = dynamic_cast<const ScalarInit*>(init))
    return exprHasCall(scalar->expr.get());
  if (auto* aggregate = dynamic_cast<const AggregateInit*>(init)) {
    for (const auto& item : aggregate->items)
      if (initHasCall(item.get()))
        return true;
  }
  return false;
}

void collectExprUses(const Expr* expr, NameSet& uses) {
  if (!expr)
    return;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    uses.insert(lval->name);
    for (const auto& idx : lval->indices)
      collectExprUses(idx.get(), uses);
    return;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    for (const auto& arg : call->args)
      collectExprUses(arg.get(), uses);
    return;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    collectExprUses(cast->operand.get(), uses);
    return;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    collectExprUses(unary->operand.get(), uses);
    return;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    collectExprUses(binary->lhs.get(), uses);
    collectExprUses(binary->rhs.get(), uses);
    return;
  }
}

void collectInitUses(const InitVal* init, NameSet& uses) {
  if (!init)
    return;
  if (auto* scalar = dynamic_cast<const ScalarInit*>(init)) {
    collectExprUses(scalar->expr.get(), uses);
    return;
  }
  if (auto* aggregate = dynamic_cast<const AggregateInit*>(init)) {
    for (const auto& item : aggregate->items)
      collectInitUses(item.get(), uses);
  }
}

void collectStmtUses(const Stmt* stmt, NameSet& uses) {
  if (!stmt)
    return;
  if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    collectExprUses(assign->lval.get(), uses);
    collectExprUses(assign->rhs.get(), uses);
    return;
  }
  if (auto* exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
    collectExprUses(exprStmt->expr.get(), uses);
    return;
  }
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl && item.decl) {
        for (const auto& def : item.decl->defs)
          collectInitUses(def->initVal.get(), uses);
      } else if (item.kind == BlockStmt::Item::Kind::Stmt) {
        collectStmtUses(item.stmt.get(), uses);
      }
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    collectExprUses(ifStmt->cond.get(), uses);
    collectStmtUses(ifStmt->thenBranch.get(), uses);
    collectStmtUses(ifStmt->elseBranch.get(), uses);
    return;
  }
  if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
    collectExprUses(whileStmt->cond.get(), uses);
    collectStmtUses(whileStmt->body.get(), uses);
    return;
  }
  if (auto* doStmt = dynamic_cast<const DoWhileStmt*>(stmt)) {
    collectStmtUses(doStmt->body.get(), uses);
    collectExprUses(doStmt->cond.get(), uses);
    return;
  }
  if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt)) {
    collectExprUses(ret->val.get(), uses);
    return;
  }
}

bool intConstValue(const Expr* expr, int32_t& value) {
  if (auto* lit = dynamic_cast<const IntLitExpr*>(expr)) {
    value = lit->val;
    return true;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    int32_t inner = 0;
    if (!intConstValue(unary->operand.get(), inner))
      return false;
    if (unary->op == '+') {
      value = inner;
      return true;
    }
    if (unary->op == '-' && inner != std::numeric_limits<int32_t>::min()) {
      value = -inner;
      return true;
    }
    if (unary->op == '!') {
      value = !inner;
      return true;
    }
  }
  return false;
}

bool stmtAlwaysExits(const Stmt* stmt) {
  if (!stmt)
    return false;
  if (dynamic_cast<const ReturnStmt*>(stmt) ||
      dynamic_cast<const BreakStmt*>(stmt) ||
      dynamic_cast<const ContinueStmt*>(stmt))
    return true;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (auto it = block->items.rbegin(); it != block->items.rend(); ++it) {
      if (it->kind == BlockStmt::Item::Kind::Stmt && it->stmt)
        return stmtAlwaysExits(it->stmt.get());
      if (it->kind == BlockStmt::Item::Kind::Decl)
        continue;
    }
    return false;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    return ifStmt->thenBranch && ifStmt->elseBranch &&
           stmtAlwaysExits(ifStmt->thenBranch.get()) &&
           stmtAlwaysExits(ifStmt->elseBranch.get());
  }
  return false;
}

bool pruneUnreachableTail(BlockStmt& block) {
  bool changed = false;
  std::vector<BlockStmt::Item> kept;
  kept.reserve(block.items.size());
  bool terminated = false;
  for (auto& item : block.items) {
    if (terminated) {
      changed = true;
      continue;
    }
    if (item.kind == BlockStmt::Item::Kind::Stmt && item.stmt &&
        stmtAlwaysExits(item.stmt.get()))
      terminated = true;
    kept.push_back(std::move(item));
  }
  block.items = std::move(kept);
  return changed;
}

bool pruneDeadLocalDecls(BlockStmt& block) {
  bool changed = false;
  NameSet needed;

  for (auto itemIt = block.items.rbegin(); itemIt != block.items.rend();
       ++itemIt) {
    auto& item = *itemIt;
    if (item.kind == BlockStmt::Item::Kind::Stmt) {
      collectStmtUses(item.stmt.get(), needed);
      continue;
    }
    if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl)
      continue;

    auto& decl = item.decl;
    std::vector<std::unique_ptr<VarDef>> keptDefs;
    keptDefs.reserve(decl->defs.size());
    for (auto defIt = decl->defs.rbegin(); defIt != decl->defs.rend();
         ++defIt) {
      std::unique_ptr<VarDef>& def = *defIt;
      const bool scalar = def && !def->type.isArray();
      const bool usedLater = def && needed.count(def->name) != 0;
      const bool hasSideEffects = def && initHasCall(def->initVal.get());
      const bool canDrop = scalar && !usedLater && !hasSideEffects;

      if (def) {
        needed.erase(def->name);
        if (!canDrop)
          collectInitUses(def->initVal.get(), needed);
      }

      if (canDrop) {
        changed = true;
      } else {
        keptDefs.push_back(std::move(def));
      }
    }

    if (keptDefs.empty()) {
      decl->defs.clear();
      changed = true;
      continue;
    }

    std::reverse(keptDefs.begin(), keptDefs.end());
    decl->defs = std::move(keptDefs);
  }

  if (changed) {
    std::vector<BlockStmt::Item> keptItems;
    keptItems.reserve(block.items.size());
    for (auto& item : block.items) {
      if (item.kind == BlockStmt::Item::Kind::Decl && item.decl &&
          item.decl->defs.empty())
        continue;
      keptItems.push_back(std::move(item));
    }
    block.items = std::move(keptItems);
  }

  return changed;
}

bool matchAddAssignDelta(const Stmt* stmt,
                         const FunctionFacts& facts,
                         std::string& name,
                         int64_t& delta) {
  auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  if (!assign)
    return false;
  auto* lhs = dynamic_cast<const LValExpr*>(assign->lval.get());
  if (!lhs || !lhs->indices.empty())
    return false;
  if (!facts.intScalarNames.count(lhs->name))
    return false;
  auto* rhs = dynamic_cast<const BinaryExpr*>(assign->rhs.get());
  if (!rhs || (rhs->op != "+" && rhs->op != "-"))
    return false;

  int32_t literal = 0;
  if (isPlainLValOf(rhs->lhs.get(), lhs->name) &&
      intConstValue(rhs->rhs.get(), literal)) {
    name = lhs->name;
    delta = rhs->op == "+" ? literal : -static_cast<int64_t>(literal);
    return true;
  }
  if (rhs->op == "+" && isPlainLValOf(rhs->rhs.get(), lhs->name) &&
      intConstValue(rhs->lhs.get(), literal)) {
    name = lhs->name;
    delta = literal;
    return true;
  }
  return false;
}

bool combineAdjacentAddAssigns(BlockStmt& block, const FunctionFacts& facts) {
  bool changed = false;
  std::vector<BlockStmt::Item> out;
  out.reserve(block.items.size());

  for (size_t i = 0; i < block.items.size();) {
    BlockStmt::Item& item = block.items[i];
    if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt) {
      out.push_back(std::move(item));
      ++i;
      continue;
    }

    std::string name;
    int64_t total = 0;
    if (!matchAddAssignDelta(item.stmt.get(), facts, name, total)) {
      out.push_back(std::move(item));
      ++i;
      continue;
    }

    size_t j = i + 1;
    for (; j < block.items.size(); ++j) {
      BlockStmt::Item& next = block.items[j];
      if (next.kind != BlockStmt::Item::Kind::Stmt || !next.stmt)
        break;
      std::string nextName;
      int64_t nextDelta = 0;
      if (!matchAddAssignDelta(next.stmt.get(), facts, nextName, nextDelta) ||
          nextName != name)
        break;
      const int64_t candidate = total + nextDelta;
      if (candidate > std::numeric_limits<int32_t>::max() ||
          candidate <= std::numeric_limits<int32_t>::min())
        break;
      total = candidate;
    }

    if (j > i + 1 && total != std::numeric_limits<int32_t>::min()) {
      auto* assign = dynamic_cast<AssignStmt*>(item.stmt.get());
      assign->rhs = makeAddExpr(name, static_cast<int32_t>(total), assign->line);
      changed = true;
    }
    out.push_back(std::move(item));
    i = j;
  }

  block.items = std::move(out);
  return changed;
}

bool simplifyStmt(StmtPtr& stmt, const FunctionFacts& facts) {
  if (!stmt)
    return false;

  bool changed = false;
  if (auto* block = dynamic_cast<BlockStmt*>(stmt.get())) {
    changed |= simplifyBlock(*block, facts);
    return changed;
  }

  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
    changed |= simplifyStmt(ifStmt->thenBranch, facts);
    changed |= simplifyStmt(ifStmt->elseBranch, facts);
    return changed;
  }

  if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
    changed |= simplifyStmt(whileStmt->body, facts);
    return changed;
  }

  if (auto* doStmt = dynamic_cast<DoWhileStmt*>(stmt.get())) {
    changed |= simplifyStmt(doStmt->body, facts);
    return changed;
  }

  return changed;
}

bool simplifyBlock(BlockStmt& block, const FunctionFacts& facts) {
  bool changed = false;
  for (auto& item : block.items) {
    if (item.kind == BlockStmt::Item::Kind::Stmt)
      changed |= simplifyStmt(item.stmt, facts);
  }
  changed |= pruneUnreachableTail(block);
  changed |= combineAdjacentAddAssigns(block, facts);
  changed |= pruneDeadLocalDecls(block);
  return changed;
}

void collectDeclFacts(const DeclStmt* decl,
                      std::unordered_map<std::string, int>& counts,
                      std::unordered_map<std::string, bool>& intScalar) {
  if (!decl)
    return;
  for (const auto& def : decl->defs) {
    if (!def)
      continue;
    counts[def->name] += 1;
    const bool isIntScalar =
        decl->baseType == BaseType::Int && isScalarIntType(def->type);
    auto inserted = intScalar.emplace(def->name, isIntScalar);
    if (!inserted.second)
      inserted.first->second = inserted.first->second && isIntScalar;
  }
}

void collectStmtDeclFacts(const Stmt* stmt,
                          std::unordered_map<std::string, int>& counts,
                          std::unordered_map<std::string, bool>& intScalar) {
  if (!stmt)
    return;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl)
        collectDeclFacts(item.decl.get(), counts, intScalar);
      else
        collectStmtDeclFacts(item.stmt.get(), counts, intScalar);
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    collectStmtDeclFacts(ifStmt->thenBranch.get(), counts, intScalar);
    collectStmtDeclFacts(ifStmt->elseBranch.get(), counts, intScalar);
    return;
  }
  if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
    collectStmtDeclFacts(whileStmt->body.get(), counts, intScalar);
    return;
  }
  if (auto* doStmt = dynamic_cast<const DoWhileStmt*>(stmt))
    collectStmtDeclFacts(doStmt->body.get(), counts, intScalar);
}

FunctionFacts buildFunctionFacts(const FuncDef& func) {
  std::unordered_map<std::string, int> counts;
  std::unordered_map<std::string, bool> intScalar;

  for (const auto& param : func.params) {
    if (!param)
      continue;
    counts[param->name] += 1;
    const bool isIntScalar = isScalarIntType(param->type);
    auto inserted = intScalar.emplace(param->name, isIntScalar);
    if (!inserted.second)
      inserted.first->second = inserted.first->second && isIntScalar;
  }
  collectStmtDeclFacts(func.body.get(), counts, intScalar);

  FunctionFacts facts;
  for (const auto& entry : counts) {
    const std::string& name = entry.first;
    int count = entry.second;
    auto typeIt = intScalar.find(name);
    if (count == 1 && typeIt != intScalar.end() && typeIt->second)
      facts.intScalarNames.insert(name);
  }
  return facts;
}

} // namespace

bool simplifyAST(CompUnit& compUnit) {
  bool changed = false;
  for (auto& item : compUnit.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func ||
        !item.func->body)
      continue;
    FunctionFacts facts = buildFunctionFacts(*item.func);
    changed |= simplifyBlock(*item.func->body, facts);
  }
  return changed;
}

} // namespace hir
