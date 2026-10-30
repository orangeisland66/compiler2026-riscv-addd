#include "research_switch.h"
// 文件职责：简化 max(x, C-x) 链，移除已被第一次反射覆盖的后续阈值判断。
#include "reflective_max_simplify.h"

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

// 深拷贝表达式树：按节点类型分派逐层克隆，不支持的节点返回 nullptr，
// 供调用方放弃改写。
ExprPtr cloneExpr(const Expr* expr) {
  if (!expr)
    return nullptr;
  if (auto* node = dynamic_cast<const IntLitExpr*>(expr)) {
    auto out = std::make_unique<IntLitExpr>();
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

// 克隆并做标量替换：复制表达式树时把名为 name 的标量叶节点替换为
// replacement 的副本，用于将返回模板中的结果变量代入展开。
ExprPtr cloneReplacingScalar(const Expr* expr, const std::string& name,
                             const Expr* replacement) {
  if (!expr)
    return nullptr;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->indices.empty() && lval->name == name)
      return cloneExpr(replacement);
  }
  if (auto* node = dynamic_cast<const BinaryExpr*>(expr)) {
    auto out = std::make_unique<BinaryExpr>();
    out->line = node->line;
    out->op = node->op;
    out->lhs = cloneReplacingScalar(node->lhs.get(), name, replacement);
    out->rhs = cloneReplacingScalar(node->rhs.get(), name, replacement);
    return out;
  }
  if (auto* node = dynamic_cast<const UnaryExpr*>(expr)) {
    auto out = std::make_unique<UnaryExpr>();
    out->line = node->line;
    out->op = node->op;
    out->operand = cloneReplacingScalar(node->operand.get(), name, replacement);
    return out;
  }
  if (auto* node = dynamic_cast<const CastExpr*>(expr)) {
    auto out = std::make_unique<CastExpr>();
    out->line = node->line;
    out->targetType = node->targetType;
    out->operand = cloneReplacingScalar(node->operand.get(), name, replacement);
    return out;
  }
  if (auto* node = dynamic_cast<const CallExpr*>(expr)) {
    auto out = std::make_unique<CallExpr>();
    out->line = node->line;
    out->name = node->name;
    out->callee = node->callee;
    for (const auto& arg : node->args)
      out->args.push_back(
          cloneReplacingScalar(arg.get(), name, replacement));
    return out;
  }
  return cloneExpr(expr);
}

bool isScalar(const Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.empty();
}

bool isIntLit(const Expr* expr, int value) {
  auto* lit = dynamic_cast<const IntLitExpr*>(expr);
  return lit && lit->val == value;
}

bool matchReflectSub(const Expr* expr, const std::string& var, int& value) {
  auto* sub = dynamic_cast<const BinaryExpr*>(expr);
  auto* literal = sub ? dynamic_cast<const IntLitExpr*>(sub->lhs.get())
                      : nullptr;
  if (!sub || sub->op != "-" || !literal ||
      !isScalar(sub->rhs.get(), var)) {
    return false;
  }
  value = literal->val;
  return true;
}

bool returnScalar(const Stmt* stmt, const std::string& name) {
  if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt))
    return isScalar(ret->val.get(), name);
  auto* block = dynamic_cast<const BlockStmt*>(stmt);
  if (!block || block->items.size() != 1 ||
      block->items[0].kind != BlockStmt::Item::Kind::Stmt) {
    return false;
  }
  return returnScalar(block->items[0].stmt.get(), name);
}

bool isMaxHelper(const FuncDef& func) {
  if (!func.retType.isInt() || func.params.size() != 2 || !func.body ||
      (func.body->items.size() != 1 && func.body->items.size() != 2) ||
      func.body->items[0].kind != BlockStmt::Item::Kind::Stmt) {
    return false;
  }
  const FuncParam* lhs = func.params[0].get();
  const FuncParam* rhs = func.params[1].get();
  if (!lhs || !rhs || !lhs->type.isInt() || !rhs->type.isInt() ||
      lhs->type.isArray() || rhs->type.isArray()) {
    return false;
  }
  auto* ifStmt = dynamic_cast<const IfStmt*>(func.body->items[0].stmt.get());
  auto* cond = ifStmt ? dynamic_cast<const BinaryExpr*>(ifStmt->cond.get()) : nullptr;
  if (!ifStmt || !cond || cond->op != "<" ||
      !isScalar(cond->lhs.get(), lhs->name) ||
      !isScalar(cond->rhs.get(), rhs->name)) {
    return false;
  }
  if (!returnScalar(ifStmt->thenBranch.get(), rhs->name))
    return false;
  if (ifStmt->elseBranch)
    return func.body->items.size() == 1 &&
           returnScalar(ifStmt->elseBranch.get(), lhs->name);
  return func.body->items.size() == 2 &&
         func.body->items[1].kind == BlockStmt::Item::Kind::Stmt &&
         returnScalar(func.body->items[1].stmt.get(), lhs->name);
}

const Expr* scalarInit(const BlockStmt::Item& item, std::string& name) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->baseType != BaseType::Int || item.decl->defs.size() != 1) {
    return nullptr;
  }
  const VarDef* def = item.decl->defs[0].get();
  if (!def || def->type.isArray())
    return nullptr;
  auto* init = dynamic_cast<const ScalarInit*>(def->initVal.get());
  if (!init)
    return nullptr;
  name = def->name;
  return init->expr.get();
}

bool matchMaxReflectCall(const Expr* expr,
                         const std::unordered_set<std::string>& maxHelpers,
                         const std::string& var,
                         int& reflectedConstant) {
  auto* call = dynamic_cast<const CallExpr*>(expr);
  if (!call || !maxHelpers.count(call->name) || call->args.size() != 2)
    return false;
  return (isScalar(call->args[0].get(), var) &&
          matchReflectSub(call->args[1].get(), var, reflectedConstant)) ||
         (isScalar(call->args[1].get(), var) &&
          matchReflectSub(call->args[0].get(), var, reflectedConstant));
}

bool referencesScalar(const Expr* expr, const std::string& name) {
  if (!expr)
    return false;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (lval->indices.empty() && lval->name == name)
      return true;
    for (const auto& index : lval->indices) {
      if (referencesScalar(index.get(), name))
        return true;
    }
    return false;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    for (const auto& arg : call->args) {
      if (referencesScalar(arg.get(), name))
        return true;
    }
    return false;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return referencesScalar(binary->lhs.get(), name) ||
           referencesScalar(binary->rhs.get(), name);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return referencesScalar(unary->operand.get(), name);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return referencesScalar(cast->operand.get(), name);
  return false;
}

ExprPtr makeVar(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->line = line;
  expr->name = name;
  return expr;
}

ExprPtr makeInt(int value, int line) {
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

BlockStmt::Item declItem(std::unique_ptr<DeclStmt> decl) {
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Decl;
  item.decl = std::move(decl);
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
  return declItem(std::move(decl));
}

StmtPtr blockStmt(std::vector<BlockStmt::Item> items, int line) {
  auto block = std::make_unique<BlockStmt>();
  block->line = line;
  block->items = std::move(items);
  return block;
}

BlockStmt::Item returnItem(ExprPtr value, int line) {
  auto stmt = std::make_unique<ReturnStmt>();
  stmt->line = line;
  stmt->val = std::move(value);
  return stmtItem(std::move(stmt));
}

bool analyzeFunction(const FuncDef& func,
                     const std::unordered_set<std::string>& maxHelpers,
                     std::string& resultVar, ExprPtr& returnTemplate,
                     int& reflectionConstant, int& reflectionThreshold) {
  if (!func.body || func.params.size() != 1 || !func.retType.isInt() ||
      !func.params[0]->type.isInt() || func.params[0]->type.isArray() ||
      func.body->items.size() < 2) {
    return false;
  }

  const std::string x = func.params[0]->name;
  std::string previous = x;
  std::vector<std::string> stages;
  for (size_t index = 0; index + 1 < func.body->items.size(); ++index) {
    std::string stage;
    int constant = 0;
    const Expr* init = scalarInit(func.body->items[index], stage);
    if (!init || !matchMaxReflectCall(init, maxHelpers, previous, constant) ||
        constant <= 0 ||
        (!stages.empty() && reflectionConstant < constant)) {
      return false;
    }
    if (stages.empty())
      reflectionConstant = constant;
    stages.push_back(stage);
    previous = stage;
  }
  auto* ret = dynamic_cast<const ReturnStmt*>(
      func.body->items.back().kind == BlockStmt::Item::Kind::Stmt
          ? func.body->items.back().stmt.get()
          : nullptr);

  if (!ret || stages.empty() || referencesScalar(ret->val.get(), x))
    return false;
  for (size_t index = 0; index + 1 < stages.size(); ++index) {
    if (referencesScalar(ret->val.get(), stages[index]))
      return false;
  }

  resultVar = stages.back();
  returnTemplate = cloneExpr(ret->val.get());
  reflectionThreshold = reflectionConstant / 2 + reflectionConstant % 2;

  if (sycEnvironmentValue("SYC_TRACE_AST_REFLECTIVE_MAX_SIMPLIFY")) {
    std::cerr << "[ast-reflective-max-simplify] function=" << func.name
              << " param=" << x
              << " constant=" << reflectionConstant << "\n";
  }
  return true;
}

bool matchScalarAssign(const Stmt* stmt, const std::string& name,
                       const Expr*& rhs) {
  auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  auto* lval = assign ? dynamic_cast<const LValExpr*>(assign->lval.get())
                      : nullptr;
  if (!assign || !lval || !lval->indices.empty() || lval->name != name)
    return false;
  rhs = assign->rhs.get();
  return rhs != nullptr;
}

bool matchIncrement(const Expr* expr, const std::string& value,
                    const std::string& step) {
  auto* add = dynamic_cast<const BinaryExpr*>(expr);
  return add && add->op == "+" &&
         ((isScalar(add->lhs.get(), value) &&
           isScalar(add->rhs.get(), step)) ||
          (isScalar(add->rhs.get(), value) &&
           isScalar(add->lhs.get(), step)));
}

bool containsScalarCall(const Expr* expr, const std::string& callee,
                        const std::string& argument) {
  if (!expr)
    return false;
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    if (call->name == callee && call->args.size() == 1 &&
        isScalar(call->args[0].get(), argument)) {
      return true;
    }
    for (const auto& arg : call->args) {
      if (containsScalarCall(arg.get(), callee, argument))
        return true;
    }
    return false;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return containsScalarCall(binary->lhs.get(), callee, argument) ||
           containsScalarCall(binary->rhs.get(), callee, argument);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return containsScalarCall(unary->operand.get(), callee, argument);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return containsScalarCall(cast->operand.get(), callee, argument);
  return false;
}

// 克隆表达式树并内联反射恒等式：callee(argument) 调用被替换为返回模板中
// 结果变量代入 (reflectionConstant - argument) 后的表达式，其余节点原样递归克隆。
ExprPtr cloneWithFastReflectiveCall(const Expr* expr,
                                    const std::string& callee,
                                    const std::string& argument,
                                    const std::string& resultVar,
                                    const Expr* returnTemplate,
                                    int reflectionConstant) {
  if (!expr)
    return nullptr;
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    if (call->name == callee && call->args.size() == 1 &&
        isScalar(call->args[0].get(), argument)) {
      auto reflected = makeBinary(
          "-", makeInt(reflectionConstant, call->line),
          cloneExpr(call->args[0].get()), call->line);
      return cloneReplacingScalar(returnTemplate, resultVar, reflected.get());
    }
    auto out = std::make_unique<CallExpr>();
    out->line = call->line;
    out->name = call->name;
    out->callee = call->callee;
    for (const auto& arg : call->args) {
      out->args.push_back(cloneWithFastReflectiveCall(
          arg.get(), callee, argument, resultVar, returnTemplate,
          reflectionConstant));
    }
    return out;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    return makeBinary(
        binary->op,
        cloneWithFastReflectiveCall(binary->lhs.get(), callee, argument,
                                    resultVar, returnTemplate,
                                    reflectionConstant),
        cloneWithFastReflectiveCall(binary->rhs.get(), callee, argument,
                                    resultVar, returnTemplate,
                                    reflectionConstant),
        binary->line);
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    auto out = std::make_unique<UnaryExpr>();
    out->line = unary->line;
    out->op = unary->op;
    out->operand = cloneWithFastReflectiveCall(
        unary->operand.get(), callee, argument, resultVar, returnTemplate,
        reflectionConstant);
    return out;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    auto out = std::make_unique<CastExpr>();
    out->line = cast->line;
    out->targetType = cast->targetType;
    out->operand = cloneWithFastReflectiveCall(
        cast->operand.get(), callee, argument, resultVar, returnTemplate,
        reflectionConstant);
    return out;
  }
  return cloneExpr(expr);
}

// 识别“范围受限求和循环”形态：函数体恰为 sum=0、归纳变量=start 两个声明、
// while(induction<end) 循环与 return sum；循环体内须按 step 步进且累加项
// 引用 callee(induction)。任一形态不符即放弃改写。
bool rewriteRangeGuardedLoop(FuncDef& func, const std::string& callee,
                             const std::string& resultVar,
                             const Expr* returnTemplate,
                             int reflectionConstant,
                             int reflectionThreshold) {
  if (!func.body || !returnTemplate || !func.retType.isInt() ||
      func.params.size() != 3 || func.body->items.size() != 4) {
    return false;
  }
  for (const auto& param : func.params) {
    if (!param || !param->type.isInt() || param->type.isArray())
      return false;
  }
  const std::string start = func.params[0]->name;
  const std::string end = func.params[1]->name;
  const std::string step = func.params[2]->name;
  std::string sum;
  std::string induction;
  const Expr* sumInit = scalarInit(func.body->items[0], sum);
  const Expr* inductionInit = scalarInit(func.body->items[1], induction);
  auto* loop = dynamic_cast<const WhileStmt*>(
      func.body->items[2].kind == BlockStmt::Item::Kind::Stmt
          ? func.body->items[2].stmt.get()
          : nullptr);
  auto* ret = dynamic_cast<const ReturnStmt*>(
      func.body->items[3].kind == BlockStmt::Item::Kind::Stmt
          ? func.body->items[3].stmt.get()
          : nullptr);
  auto* condition = loop ? dynamic_cast<const BinaryExpr*>(loop->cond.get())
                         : nullptr;
  auto* loopBody = loop ? dynamic_cast<const BlockStmt*>(loop->body.get())
                        : nullptr;
  if (!isIntLit(sumInit, 0) || !isScalar(inductionInit, start) || !condition ||
      condition->op != "<" || !isScalar(condition->lhs.get(), induction) ||
      !isScalar(condition->rhs.get(), end) || !ret ||
      !isScalar(ret->val.get(), sum) || !loopBody ||
      loopBody->items.size() != 2 ||
      loopBody->items[0].kind != BlockStmt::Item::Kind::Stmt ||
      loopBody->items[1].kind != BlockStmt::Item::Kind::Stmt) {
    return false;
  }

  const Expr* sumRhs = nullptr;
  const Expr* inductionRhs = nullptr;
  if (!matchScalarAssign(loopBody->items[0].stmt.get(), sum, sumRhs) ||
      !containsScalarCall(sumRhs, callee, induction) ||
      !matchScalarAssign(loopBody->items[1].stmt.get(), induction,
                         inductionRhs) ||
      !matchIncrement(inductionRhs, induction, step)) {
    return false;
  }

  const int line = func.line;
  std::vector<BlockStmt::Item> fastLoopBody;
  fastLoopBody.push_back(stmtItem(makeAssign(
      sum,
      cloneWithFastReflectiveCall(sumRhs, callee, induction, resultVar,
                                  returnTemplate, reflectionConstant),
      line)));
  fastLoopBody.push_back(
      stmtItem(makeAssign(induction, cloneExpr(inductionRhs), line)));
  std::vector<BlockStmt::Item> secondIteration;
  secondIteration.push_back(stmtItem(makeAssign(
      sum,
      cloneWithFastReflectiveCall(sumRhs, callee, induction, resultVar,
                                  returnTemplate, reflectionConstant),
      line)));
  secondIteration.push_back(
      stmtItem(makeAssign(induction, cloneExpr(inductionRhs), line)));
  auto secondGuard = std::make_unique<IfStmt>();
  secondGuard->line = line;
  secondGuard->cond = cloneExpr(loop->cond.get());
  secondGuard->thenBranch = blockStmt(std::move(secondIteration), line);
  fastLoopBody.push_back(stmtItem(std::move(secondGuard)));
  auto fastLoop = std::make_unique<WhileStmt>();
  fastLoop->line = line;
  fastLoop->cond = cloneExpr(loop->cond.get());
  fastLoop->body = blockStmt(std::move(fastLoopBody), line);

  std::vector<BlockStmt::Item> fastPath;
  fastPath.push_back(intDeclItem(sum, makeInt(0, line), line));
  fastPath.push_back(intDeclItem(induction, makeVar(start, line), line));
  fastPath.push_back(stmtItem(std::move(fastLoop)));
  fastPath.push_back(returnItem(makeVar(sum, line), line));

  auto safeRange = makeBinary(
      "&&",
      makeBinary(
          "&&", makeBinary(">=", makeVar(start, line), makeInt(0, line), line),
          makeBinary(">", makeVar(step, line), makeInt(0, line), line), line),
      makeBinary(
          "&&",
          makeBinary("<=", makeVar(end, line),
                     makeInt(reflectionThreshold, line),
                     line),
          makeBinary(
              "<=", makeVar(step, line),
              makeBinary("-", makeInt(2147483647, line),
                         makeVar(end, line), line),
              line),
          line),
      line);
  auto guard = std::make_unique<IfStmt>();
  guard->line = line;
  guard->cond = std::move(safeRange);
  guard->thenBranch = blockStmt(std::move(fastPath), line);
  func.body->items.insert(func.body->items.begin(), stmtItem(std::move(guard)));
  return true;
}

} // namespace

bool simplifyReflectiveMaxLadders(CompUnit& cu) {
  if (sycEnvironmentValue("SYC_SKIP_AST_REFLECTIVE_MAX_SIMPLIFY"))
    return false;
  std::unordered_set<std::string> maxHelpers;
  for (const auto& item : cu.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
        isMaxHelper(*item.func)) {
      maxHelpers.insert(item.func->name);
    }
  }
  if (maxHelpers.empty())
    return false;
  bool changed = false;
  struct ReflectiveSummary {
    std::string function;
    std::string resultVar;
    ExprPtr returnTemplate;
    int reflectionConstant = 0;
    int reflectionThreshold = 0;
  };
  std::vector<ReflectiveSummary> summaries;
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    ReflectiveSummary summary;
    summary.function = item.func->name;
    if (analyzeFunction(*item.func, maxHelpers, summary.resultVar,
                        summary.returnTemplate, summary.reflectionConstant,
                        summary.reflectionThreshold)) {
      summaries.push_back(std::move(summary));
    }
  }
  for (const auto& summary : summaries) {
    for (auto& item : cu.items) {
      if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
          item.func->name != summary.function) {
        changed |= rewriteRangeGuardedLoop(
            *item.func, summary.function, summary.resultVar,
            summary.returnTemplate.get(), summary.reflectionConstant,
            summary.reflectionThreshold);
      }
    }
  }
  return changed;
}

} // namespace hir
