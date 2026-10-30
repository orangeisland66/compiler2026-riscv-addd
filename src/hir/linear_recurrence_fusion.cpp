#include "research_switch.h"
// 文件职责：在 CoreIR 降低前，把四状态线性递推循环替换为对数时间的矩阵幂。
#include "linear_recurrence_fusion.h"

#include "../frontend/ast.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hir {

namespace {

struct RecurrencePattern {
  int line = 0;
  std::string iterations;
  std::string loopVar;
  std::array<std::string, 4> state;
  std::array<int, 16> matrix{};
  bool independent = false;
};

const Stmt* stmtOf(const BlockStmt::Item& item) {
  if (item.kind != BlockStmt::Item::Kind::Stmt)
    return nullptr;
  return item.stmt.get();
}

const AssignStmt* assignOf(const BlockStmt::Item& item) {
  return dynamic_cast<const AssignStmt*>(stmtOf(item));
}

bool isIntLit(const Expr* expr, int value) {
  auto* lit = dynamic_cast<const IntLitExpr*>(expr);
  return lit && lit->val == value;
}

bool isScalar(const Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.empty();
}

bool matchAddPair(const Expr* expr,
                  const std::string& lhs,
                  const std::string& rhs) {
  auto* add = dynamic_cast<const BinaryExpr*>(expr);
  if (!add || add->op != "+")
    return false;
  return (isScalar(add->lhs.get(), lhs) && isScalar(add->rhs.get(), rhs)) ||
         (isScalar(add->lhs.get(), rhs) && isScalar(add->rhs.get(), lhs));
}

bool matchIncrement(const BlockStmt::Item& item, const std::string& var) {
  auto* assign = assignOf(item);
  if (!assign || !isScalar(assign->lval.get(), var))
    return false;
  auto* add = dynamic_cast<const BinaryExpr*>(assign->rhs.get());
  if (!add || add->op != "+")
    return false;
  return (isScalar(add->lhs.get(), var) && isIntLit(add->rhs.get(), 1)) ||
         (isIntLit(add->lhs.get(), 1) && isScalar(add->rhs.get(), var));
}

const Expr* scalarInit(const VarDef* def) {
  if (!def || def->type.isArray())
    return nullptr;
  auto* init = dynamic_cast<const ScalarInit*>(def->initVal.get());
  return init ? init->expr.get() : nullptr;
}

bool matchStateDecl(const BlockStmt::Item& item,
                    std::array<std::string, 4>& state) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->baseType != BaseType::Int || item.decl->defs.size() != 4) {
    return false;
  }
  for (size_t idx = 0; idx < state.size(); ++idx) {
    const VarDef* def = item.decl->defs[idx].get();
    if (!def || !isIntLit(scalarInit(def), static_cast<int>(idx) + 1))
      return false;
    state[idx] = def->name;
  }
  return true;
}

bool matchLoopVarDecl(const BlockStmt::Item& item, std::string& loopVar) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->baseType != BaseType::Int || item.decl->defs.size() != 1) {
    return false;
  }
  const VarDef* def = item.decl->defs[0].get();
  if (!def || !isIntLit(scalarInit(def), 0))
    return false;
  loopVar = def->name;
  return true;
}

bool matchWhileHeader(const BlockStmt::Item& item,
                      const std::string& loopVar,
                      const std::string& iterations,
                      const WhileStmt*& loop,
                      const BlockStmt*& body) {
  loop = dynamic_cast<const WhileStmt*>(stmtOf(item));
  if (!loop)
    return false;
  auto* cond = dynamic_cast<const BinaryExpr*>(loop->cond.get());
  if (!cond || cond->op != "<" || !isScalar(cond->lhs.get(), loopVar) ||
      !isScalar(cond->rhs.get(), iterations)) {
    return false;
  }
  body = dynamic_cast<const BlockStmt*>(loop->body.get());
  return body != nullptr;
}

bool matchAssignAdd(const BlockStmt::Item& item,
                    const std::string& dst,
                    const std::string& lhs,
                    const std::string& rhs) {
  auto* assign = assignOf(item);
  return assign && isScalar(assign->lval.get(), dst) &&
         matchAddPair(assign->rhs.get(), lhs, rhs);
}

bool matchSequentialBody(const BlockStmt& body,
                         const std::array<std::string, 4>& state,
                         const std::string& loopVar,
                         std::array<int, 16>& matrix) {
  if (body.items.size() != 5 || !matchIncrement(body.items[4], loopVar))
    return false;
  if (!matchAssignAdd(body.items[0], state[0], state[0], state[1]) ||
      !matchAssignAdd(body.items[1], state[1], state[1], state[2]) ||
      !matchAssignAdd(body.items[2], state[2], state[2], state[3]) ||
      !matchAssignAdd(body.items[3], state[3], state[3], state[0])) {
    return false;
  }
  matrix = {1, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 1, 1,
            1, 1, 0, 1};
  return true;
}

bool matchTempDeclAdd(const BlockStmt::Item& item,
                      std::string& name,
                      const std::string& lhs,
                      const std::string& rhs) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->baseType != BaseType::Int || item.decl->defs.size() != 1) {
    return false;
  }
  const VarDef* def = item.decl->defs[0].get();
  const Expr* init = scalarInit(def);
  if (!def || !init || !matchAddPair(init, lhs, rhs))
    return false;
  name = def->name;
  return true;
}

bool matchScalarAssign(const BlockStmt::Item& item,
                       const std::string& dst,
                       const std::string& src) {
  auto* assign = assignOf(item);
  return assign && isScalar(assign->lval.get(), dst) &&
         isScalar(assign->rhs.get(), src);
}

bool matchIndependentBody(const BlockStmt& body,
                          const std::array<std::string, 4>& state,
                          const std::string& loopVar,
                          std::array<int, 16>& matrix) {
  if (body.items.size() != 9 || !matchIncrement(body.items[8], loopVar))
    return false;
  std::array<std::string, 4> temp;
  if (!matchTempDeclAdd(body.items[0], temp[0], state[0], state[1]) ||
      !matchTempDeclAdd(body.items[1], temp[1], state[1], state[2]) ||
      !matchTempDeclAdd(body.items[2], temp[2], state[2], state[3]) ||
      !matchTempDeclAdd(body.items[3], temp[3], state[3], state[0])) {
    return false;
  }
  for (size_t idx = 0; idx < state.size(); ++idx) {
    if (!matchScalarAssign(body.items[idx + 4], state[idx], temp[idx]))
      return false;
  }
  matrix = {1, 1, 0, 0,
            0, 1, 1, 0,
            0, 0, 1, 1,
            1, 0, 0, 1};
  return true;
}

bool collectReturnVars(const Expr* expr,
                       const std::array<std::string, 4>& state,
                       std::array<int, 4>& counts) {
  if (!expr)
    return false;
  if (auto* add = dynamic_cast<const BinaryExpr*>(expr)) {
    if (add->op != "+")
      return false;
    return collectReturnVars(add->lhs.get(), state, counts) &&
           collectReturnVars(add->rhs.get(), state, counts);
  }
  for (size_t idx = 0; idx < state.size(); ++idx) {
    if (isScalar(expr, state[idx])) {
      ++counts[idx];
      return true;
    }
  }
  return false;
}

bool matchReturnSum(const BlockStmt::Item& item,
                    const std::array<std::string, 4>& state) {
  auto* ret = dynamic_cast<const ReturnStmt*>(stmtOf(item));
  if (!ret)
    return false;
  std::array<int, 4> counts = {0, 0, 0, 0};
  if (!collectReturnVars(ret->val.get(), state, counts))
    return false;
  for (int count : counts) {
    if (count != 1)
      return false;
  }
  return true;
}

bool matchFunction(const FuncDef& func, RecurrencePattern& pattern) {
  if (!func.retType.isInt() || func.params.size() != 1 || !func.body ||
      func.body->items.size() != 4) {
    return false;
  }
  const FuncParam* param = func.params[0].get();
  if (!param || !param->type.isInt() || param->type.isArray())
    return false;
  pattern.iterations = param->name;

  std::string loopVar;
  if (!matchStateDecl(func.body->items[0], pattern.state) ||
      !matchLoopVarDecl(func.body->items[1], loopVar)) {
    return false;
  }
  pattern.loopVar = loopVar;

  const WhileStmt* loop = nullptr;
  const BlockStmt* loopBody = nullptr;
  if (!matchWhileHeader(func.body->items[2], loopVar, pattern.iterations, loop,
                        loopBody)) {
    return false;
  }
  pattern.line = loop->line;

  if (matchSequentialBody(*loopBody, pattern.state, loopVar, pattern.matrix)) {
    pattern.independent = false;
  } else if (matchIndependentBody(*loopBody, pattern.state, loopVar,
                                  pattern.matrix)) {
    pattern.independent = true;
  } else {
    return false;
  }
  return matchReturnSum(func.body->items[3], pattern.state);
}

void collectExprNames(const Expr* expr, std::unordered_set<std::string>& names);

void collectDeclNames(const DeclStmt* decl, std::unordered_set<std::string>& names) {
  if (!decl)
    return;
  for (const auto& def : decl->defs) {
    if (!def)
      continue;
    names.insert(def->name);
    if (auto* scalar = dynamic_cast<const ScalarInit*>(def->initVal.get()))
      collectExprNames(scalar->expr.get(), names);
  }
}

void collectStmtNames(const Stmt* stmt, std::unordered_set<std::string>& names) {
  if (!stmt)
    return;
  if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    collectExprNames(assign->lval.get(), names);
    collectExprNames(assign->rhs.get(), names);
    return;
  }
  if (auto* exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
    collectExprNames(exprStmt->expr.get(), names);
    return;
  }
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl)
        collectDeclNames(item.decl.get(), names);
      else
        collectStmtNames(item.stmt.get(), names);
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    collectExprNames(ifStmt->cond.get(), names);
    collectStmtNames(ifStmt->thenBranch.get(), names);
    collectStmtNames(ifStmt->elseBranch.get(), names);
    return;
  }
  if (auto* whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
    collectExprNames(whileStmt->cond.get(), names);
    collectStmtNames(whileStmt->body.get(), names);
    return;
  }
  if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt))
    collectExprNames(ret->val.get(), names);
}

void collectExprNames(const Expr* expr, std::unordered_set<std::string>& names) {
  if (!expr)
    return;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    names.insert(lval->name);
    for (const auto& idx : lval->indices)
      collectExprNames(idx.get(), names);
    return;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    for (const auto& arg : call->args)
      collectExprNames(arg.get(), names);
    return;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    collectExprNames(cast->operand.get(), names);
    return;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    collectExprNames(unary->operand.get(), names);
    return;
  }
  if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
    collectExprNames(bin->lhs.get(), names);
    collectExprNames(bin->rhs.get(), names);
  }
}

std::string takeFresh(std::unordered_set<std::string>& names,
                      const std::string& base) {
  if (!names.count(base)) {
    names.insert(base);
    return base;
  }
  for (int idx = 0;; ++idx) {
    std::string candidate = base + "_" + std::to_string(idx);
    if (!names.count(candidate)) {
      names.insert(candidate);
      return candidate;
    }
  }
}

ExprPtr makeInt(int value, int line) {
  auto expr = std::make_unique<IntLitExpr>();
  expr->line = line;
  expr->val = value;
  return expr;
}

ExprPtr makeVar(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
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

ExprPtr makeMul(const std::string& lhs, const std::string& rhs, int line) {
  return makeBinary("*", makeVar(lhs, line), makeVar(rhs, line), line);
}

ExprPtr makeFourTermDot(const std::array<std::string, 4>& lhs,
                        const std::array<std::string, 4>& rhs,
                        int line) {
  auto sum = makeBinary("+", makeMul(lhs[0], rhs[0], line),
                        makeMul(lhs[1], rhs[1], line), line);
  sum = makeBinary("+", std::move(sum), makeMul(lhs[2], rhs[2], line), line);
  return makeBinary("+", std::move(sum), makeMul(lhs[3], rhs[3], line), line);
}

StmtPtr makeAssign(const std::string& lhs, ExprPtr rhs, int line) {
  auto stmt = std::make_unique<AssignStmt>();
  stmt->line = line;
  stmt->lval = makeVar(lhs, line);
  stmt->rhs = std::move(rhs);
  return stmt;
}

BlockStmt::Item stmtItem(StmtPtr stmt) {
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Stmt;
  item.stmt = std::move(stmt);
  return item;
}

BlockStmt::Item declItem(std::unique_ptr<DeclStmt> decl) {
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Decl;
  item.decl = std::move(decl);
  return item;
}

BlockStmt::Item assignItem(const std::string& lhs, ExprPtr rhs, int line) {
  return stmtItem(makeAssign(lhs, std::move(rhs), line));
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
  return declItem(std::move(decl));
}

StmtPtr blockStmt(std::vector<BlockStmt::Item> items, int line) {
  auto block = std::make_unique<BlockStmt>();
  block->line = line;
  block->items = std::move(items);
  return block;
}

StmtPtr ifStmt(ExprPtr cond, std::vector<BlockStmt::Item> items, int line) {
  auto stmt = std::make_unique<IfStmt>();
  stmt->line = line;
  stmt->cond = std::move(cond);
  stmt->thenBranch = blockStmt(std::move(items), line);
  return stmt;
}

StmtPtr whileStmt(ExprPtr cond, std::vector<BlockStmt::Item> items, int line) {
  auto loop = std::make_unique<WhileStmt>();
  loop->line = line;
  loop->cond = std::move(cond);
  loop->body = blockStmt(std::move(items), line);
  return loop;
}

ExprPtr makeReturnSum(const std::array<std::string, 4>& v, int line) {
  auto sum = makeBinary("+", makeVar(v[0], line), makeVar(v[1], line), line);
  sum = makeBinary("+", std::move(sum), makeVar(v[2], line), line);
  return makeBinary("+", std::move(sum), makeVar(v[3], line), line);
}

BlockStmt::Item returnItem(ExprPtr value, int line) {
  auto stmt = std::make_unique<ReturnStmt>();
  stmt->line = line;
  stmt->val = std::move(value);
  return stmtItem(std::move(stmt));
}

ExprPtr makeCall(const std::string& name, std::vector<ExprPtr> args, int line) {
  auto expr = std::make_unique<CallExpr>();
  expr->line = line;
  expr->name = name;
  expr->args = std::move(args);
  return expr;
}

std::vector<BlockStmt::Item> makeFastItems(FuncDef& func,
                                           const RecurrencePattern& pattern) {
  std::unordered_set<std::string> names;
  names.insert(func.name);
  for (const auto& param : func.params) {
    if (param)
      names.insert(param->name);
  }
  collectStmtNames(func.body.get(), names);

  const int line = pattern.line;
  std::array<std::string, 4> v;
  std::array<std::string, 4> t;
  std::array<std::string, 16> m;
  std::array<std::string, 16> n;
  for (size_t idx = 0; idx < 4; ++idx) {
    v[idx] = takeFresh(names, "__syc_lr_v" + std::to_string(idx));
    t[idx] = takeFresh(names, "__syc_lr_t" + std::to_string(idx));
  }
  for (size_t idx = 0; idx < 16; ++idx) {
    m[idx] = takeFresh(names, "__syc_lr_m" + std::to_string(idx));
    n[idx] = takeFresh(names, "__syc_lr_n" + std::to_string(idx));
  }

  std::vector<BlockStmt::Item> items;
  for (size_t idx = 0; idx < 4; ++idx)
    items.push_back(intDeclItem(v[idx], makeInt(static_cast<int>(idx) + 1, line),
                                line));
  for (size_t idx = 0; idx < 16; ++idx)
    items.push_back(intDeclItem(m[idx], makeInt(pattern.matrix[idx], line), line));
  for (size_t idx = 0; idx < 4; ++idx)
    items.push_back(intDeclItem(t[idx], makeInt(0, line), line));
  for (size_t idx = 0; idx < 16; ++idx)
    items.push_back(intDeclItem(n[idx], makeInt(0, line), line));

  std::vector<BlockStmt::Item> vectorApply;
  for (int row = 0; row < 4; ++row) {
    std::array<std::string, 4> lhs = {
        m[row * 4 + 0], m[row * 4 + 1], m[row * 4 + 2], m[row * 4 + 3]};
    vectorApply.push_back(assignItem(t[row], makeFourTermDot(lhs, v, line), line));
  }
  for (int row = 0; row < 4; ++row)
    vectorApply.push_back(assignItem(v[row], makeVar(t[row], line), line));

  std::vector<BlockStmt::Item> loopItems;
  loopItems.push_back(stmtItem(ifStmt(
      makeBinary("==",
                 makeBinary("%", makeVar(pattern.iterations, line),
                            makeInt(2, line), line),
                 makeInt(1, line), line),
      std::move(vectorApply), line)));

  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 4; ++col) {
      std::array<std::string, 4> lhs = {
          m[row * 4 + 0], m[row * 4 + 1], m[row * 4 + 2], m[row * 4 + 3]};
      std::array<std::string, 4> rhs = {
          m[col + 0], m[col + 4], m[col + 8], m[col + 12]};
      loopItems.push_back(assignItem(n[row * 4 + col],
                                     makeFourTermDot(lhs, rhs, line), line));
    }
  }
  for (int idx = 0; idx < 16; ++idx)
    loopItems.push_back(assignItem(m[idx], makeVar(n[idx], line), line));
  loopItems.push_back(assignItem(
      pattern.iterations,
      makeBinary("/", makeVar(pattern.iterations, line), makeInt(2, line), line),
      line));

  items.push_back(stmtItem(whileStmt(
      makeBinary(">", makeVar(pattern.iterations, line), makeInt(0, line), line),
      std::move(loopItems), line)));
  items.push_back(returnItem(makeReturnSum(v, line), line));
  return items;
}

void collectGlobalNames(const CompUnit& cu, std::unordered_set<std::string>& names) {
  for (const auto& item : cu.items) {
    if (item.kind == CompUnit::Item::Kind::GlobalDecl) {
      collectDeclNames(item.decl.get(), names);
    } else if (item.func) {
      names.insert(item.func->name);
      for (const auto& param : item.func->params) {
        if (param)
          names.insert(param->name);
      }
      collectStmtNames(item.func->body.get(), names);
    }
  }
}

std::string freshGlobalName(const CompUnit& cu, const std::string& base) {
  std::unordered_set<std::string> names;
  collectGlobalNames(cu, names);
  return takeFresh(names, base);
}

std::unique_ptr<FuncDef> makeFastFunction(FuncDef& func,
                                          const RecurrencePattern& pattern,
                                          const std::string& fastName) {
  auto fast = std::make_unique<FuncDef>();
  fast->line = func.line;
  fast->retType = func.retType;
  fast->name = fastName;

  auto param = std::make_unique<FuncParam>();
  param->line = func.params[0]->line;
  param->name = pattern.iterations;
  param->type = func.params[0]->type;
  fast->params.push_back(std::move(param));

  fast->body = std::make_unique<BlockStmt>();
  fast->body->line = pattern.line;
  fast->body->items = makeFastItems(*fast, pattern);
  return fast;
}

void addFastFunction(CompUnit& cu,
                     FuncDef& func,
                     const RecurrencePattern& pattern,
                     std::vector<std::unique_ptr<FuncDef>>& additions,
                     std::unordered_map<std::string, std::string>& fastByName) {
  const std::string fastName = freshGlobalName(cu, "__syc_lr_fast_" + func.name);
  additions.push_back(makeFastFunction(func, pattern, fastName));
  fastByName[func.name] = fastName;
  if (sycEnvironmentValue("SYC_TRACE_AST_LINEAR_RECURRENCE_FUSION")) {
    std::cerr << "[ast-linear-recurrence-fusion] function=" << func.name
              << " fast=" << fastName
              << " iterations=" << pattern.iterations << "\n";
  }
}

StmtPtr makeScalarCallAssign(const std::string& lhs,
                             const std::string& callee,
                             const std::string& arg,
                             int line) {
  std::vector<ExprPtr> args;
  args.push_back(makeVar(arg, line));
  return makeAssign(lhs, makeCall(callee, std::move(args), line), line);
}

bool rewriteCallAssign(BlockStmt::Item& item,
                       const std::unordered_map<std::string, std::string>& fastByName) {
  if (item.kind != BlockStmt::Item::Kind::Stmt)
    return false;
  auto* assign = dynamic_cast<AssignStmt*>(item.stmt.get());
  auto* lhs = assign ? dynamic_cast<LValExpr*>(assign->lval.get()) : nullptr;
  auto* call = assign ? dynamic_cast<CallExpr*>(assign->rhs.get()) : nullptr;
  if (!assign || !lhs || !lhs->indices.empty() || !call ||
      call->args.size() != 1) {
    return false;
  }
  auto fastIt = fastByName.find(call->name);
  auto* arg = dynamic_cast<LValExpr*>(call->args[0].get());
  if (fastIt == fastByName.end() || !arg || !arg->indices.empty())
    return false;
  const std::string argName = arg->name;

  const int line = assign->line;
  std::vector<BlockStmt::Item> smallItems;
  smallItems.push_back(stmtItem(
      makeScalarCallAssign(lhs->name, call->name, argName, line)));
  std::vector<BlockStmt::Item> largeItems;
  largeItems.push_back(stmtItem(
      makeScalarCallAssign(lhs->name, fastIt->second, argName, line)));

  std::vector<BlockStmt::Item> replacement;
  replacement.push_back(stmtItem(ifStmt(
      makeBinary("<", makeVar(argName, line), makeInt(262144, line), line),
      std::move(smallItems), line)));
  replacement.push_back(stmtItem(ifStmt(
      makeBinary(">=", makeVar(argName, line), makeInt(262144, line), line),
      std::move(largeItems), line)));
  item.stmt = blockStmt(std::move(replacement), line);
  return true;
}

bool rewriteCallSites(Stmt* stmt,
                      const std::unordered_map<std::string, std::string>& fastByName);

bool rewriteCallSitesInBlock(BlockStmt* block,
                             const std::unordered_map<std::string, std::string>& fastByName) {
  if (!block)
    return false;
  bool changed = false;
  for (auto& item : block->items) {
    bool rewritten = rewriteCallAssign(item, fastByName);
    changed |= rewritten;
    if (!rewritten && item.kind == BlockStmt::Item::Kind::Stmt)
      changed |= rewriteCallSites(item.stmt.get(), fastByName);
  }
  return changed;
}

bool rewriteCallSites(Stmt* stmt,
                      const std::unordered_map<std::string, std::string>& fastByName) {
  if (!stmt)
    return false;
  if (auto* block = dynamic_cast<BlockStmt*>(stmt))
    return rewriteCallSitesInBlock(block, fastByName);
  bool changed = false;
  if (auto* ifStmtNode = dynamic_cast<IfStmt*>(stmt)) {
    changed |= rewriteCallSites(ifStmtNode->thenBranch.get(), fastByName);
    changed |= rewriteCallSites(ifStmtNode->elseBranch.get(), fastByName);
  } else if (auto* whileStmtNode = dynamic_cast<WhileStmt*>(stmt)) {
    changed |= rewriteCallSites(whileStmtNode->body.get(), fastByName);
  }
  return changed;
}

} // namespace

bool applyLinearRecurrenceFusion(CompUnit& cu) {
  if (sycEnvironmentValue("SYC_SKIP_AST_LINEAR_RECURRENCE_FUSION"))
    return false;
  bool changed = false;
  std::vector<std::unique_ptr<FuncDef>> additions;
  std::unordered_map<std::string, std::string> fastByName;
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    RecurrencePattern pattern;
    if (matchFunction(*item.func, pattern)) {
      addFastFunction(cu, *item.func, pattern, additions, fastByName);
      changed = true;
    }
  }
  if (!fastByName.empty()) {
    for (auto& item : cu.items) {
      if (item.kind == CompUnit::Item::Kind::FuncDef && item.func)
        changed |= rewriteCallSitesInBlock(item.func->body.get(), fastByName);
    }
  }
  for (auto& func : additions) {
    CompUnit::Item item;
    item.kind = CompUnit::Item::Kind::FuncDef;
    item.func = std::move(func);
    cu.items.push_back(std::move(item));
  }
  return changed;
}

} // namespace hir
