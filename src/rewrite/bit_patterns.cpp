#include "research_switch.h"
// 文件职责：识别位运算等价惯用法，把逐位模拟替换为原生位运算。本文件负责实现具体逻辑。
// 这里实现 InstCombine / DAGCombine 风格的位运算等价化。AST/HIR 侧可
// 以提供 helper 摘要，但真正触发替换时仍要求 IR 级 use-chain、循环和 CFG
// 形态可证等价，避免按普通用户函数名或单一源码布局特判。
#include "bit_patterns.h"
#include "../analysis/analysis_manager.h"
#include "../analysis/cfg_utils.h"
#include "../analysis/dominators.h"
#include "../analysis/loop_info.h"
#include "../core_ir/passes/passes_utils.h"
#include "../frontend/ast.h"
#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace rewrite {

namespace {

bool bitPatternTraceEnabled() {
  return sycEnvironmentValue("BIT_PATTERN_TRACE") != nullptr;
}

void traceBitPatternDecision(core_ir::Function* func, const char* stage,
                             const char* reason) {
  if (!bitPatternTraceEnabled() || !func)
    return;
  std::cerr << "[bit-pattern] " << stage << " " << func->name() << " "
            << reason << "\n";
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

std::unique_ptr<BinaryExpr> makeBinaryExpr(const std::string& op, ExprPtr lhs,
                                           ExprPtr rhs, int line) {
  auto expr = std::make_unique<BinaryExpr>();
  expr->op = op;
  expr->lhs = std::move(lhs);
  expr->rhs = std::move(rhs);
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

std::unique_ptr<ReturnStmt> makeReturnStmt(ExprPtr val, int line) {
  auto stmt = std::make_unique<ReturnStmt>();
  stmt->val = std::move(val);
  stmt->line = line;
  return stmt;
}

bool isScalarIntParam(const FuncParam* param) {
  return param && param->type.base == BaseType::Int && !param->type.isArray();
}

bool isVarExpr(Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<LValExpr*>(expr);
  return lval && lval->indices.empty() && lval->name == name;
}

bool isIntLiteral(Expr* expr, int32_t value) {
  auto* lit = dynamic_cast<IntLitExpr*>(expr);
  return lit && lit->val == value;
}

bool matchAssignBinaryConst(Stmt* stmt, const std::string& dst,
                            const std::string& op,
                            const std::string& lhsVar,
                            int32_t rhsConst) {
  auto* assign = dynamic_cast<AssignStmt*>(stmt);
  if (!assign || !isVarExpr(assign->lval.get(), dst))
    return false;
  auto* bin = dynamic_cast<BinaryExpr*>(assign->rhs.get());
  return bin && bin->op == op && isVarExpr(bin->lhs.get(), lhsVar) &&
         isIntLiteral(bin->rhs.get(), rhsConst);
}

bool matchEqOne(Expr* expr, const std::string& name) {
  auto* bin = dynamic_cast<BinaryExpr*>(expr);
  if (!bin || bin->op != "==")
    return false;
  return (isVarExpr(bin->lhs.get(), name) && isIntLiteral(bin->rhs.get(), 1)) ||
         (isVarExpr(bin->rhs.get(), name) && isIntLiteral(bin->lhs.get(), 1));
}

bool matchAccumulateAssign(Stmt* stmt, const std::string& resultVar,
                           const std::string& powerVar) {
  auto* assign = dynamic_cast<AssignStmt*>(stmt);
  if (!assign || !isVarExpr(assign->lval.get(), resultVar))
    return false;
  auto* bin = dynamic_cast<BinaryExpr*>(assign->rhs.get());
  if (!bin || bin->op != "+")
    return false;
  return (isVarExpr(bin->lhs.get(), resultVar) &&
          isVarExpr(bin->rhs.get(), powerVar)) ||
         (isVarExpr(bin->rhs.get(), resultVar) &&
          isVarExpr(bin->lhs.get(), powerVar));
}

enum class AstBitOp {
  None,
  And,
  Or,
  Xor,
};

struct BitLoopSummary {
  std::string bitA;
  std::string bitB;
  std::string powerVar;
  bool hasBitA = false;
  bool hasBitB = false;
  bool shrinksFirstParam = false;
  bool shrinksSecondParam = false;
  bool doublesPower = false;
  bool decrementsLen = false;
  bool accumulatesResult = false;
  AstBitOp opKind = AstBitOp::None;
};

AstBitOp matchBitCondition(Expr* expr, const std::string& bitA,
                           const std::string& bitB) {
  auto* bin = dynamic_cast<BinaryExpr*>(expr);
  if (!bin)
    return AstBitOp::None;
  if (bin->op == "&&" && matchEqOne(bin->lhs.get(), bitA) &&
      matchEqOne(bin->rhs.get(), bitB))
    return AstBitOp::And;
  if (bin->op == "||" && matchEqOne(bin->lhs.get(), bitA) &&
      matchEqOne(bin->rhs.get(), bitB))
    return AstBitOp::Or;
  if (bin->op == "!=" &&
      ((isVarExpr(bin->lhs.get(), bitA) && isVarExpr(bin->rhs.get(), bitB)) ||
       (isVarExpr(bin->lhs.get(), bitB) && isVarExpr(bin->rhs.get(), bitA))))
    return AstBitOp::Xor;
  return AstBitOp::None;
}

bool matchAccumulatingIf(Stmt* stmt, const std::string& resultVar,
                         const std::string& powerVar,
                         const std::string& bitA,
                         const std::string& bitB,
                         AstBitOp& opKind) {
  auto* ifStmt = dynamic_cast<IfStmt*>(stmt);
  if (!ifStmt || ifStmt->elseBranch)
    return false;

  opKind = matchBitCondition(ifStmt->cond.get(), bitA, bitB);
  if (opKind == AstBitOp::None)
    return false;

  if (auto* block = dynamic_cast<BlockStmt*>(ifStmt->thenBranch.get())) {
    bool foundAccumulate = false;
    for (const auto& item : block->items) {
      if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt)
        return false;
      if (auto* exprStmt = dynamic_cast<ExprStmt*>(item.stmt.get())) {
        if (!exprStmt->expr)
          continue;
      }
      if (!matchAccumulateAssign(item.stmt.get(), resultVar, powerVar))
        return false;
      if (foundAccumulate)
        return false;
      foundAccumulate = true;
    }
    return foundAccumulate;
  }

  return matchAccumulateAssign(ifStmt->thenBranch.get(), resultVar, powerVar);
}

bool matchBitExtractAssign(Stmt* stmt,
                           const std::string& sourceParam,
                           std::string& bitVar) {
  auto* assign = dynamic_cast<AssignStmt*>(stmt);
  if (!assign)
    return false;
  auto* lhs = dynamic_cast<LValExpr*>(assign->lval.get());
  auto* rhs = dynamic_cast<BinaryExpr*>(assign->rhs.get());
  if (!lhs || !rhs || !lhs->indices.empty() || rhs->op != "%" ||
      !isVarExpr(rhs->lhs.get(), sourceParam) ||
      !isIntLiteral(rhs->rhs.get(), 2)) {
    return false;
  }
  bitVar = lhs->name;
  return true;
}

bool matchPowerDoubleAssign(Stmt* stmt, std::string& powerVar) {
  auto* assign = dynamic_cast<AssignStmt*>(stmt);
  if (!assign)
    return false;
  auto* lhs = dynamic_cast<LValExpr*>(assign->lval.get());
  if (!lhs || !lhs->indices.empty())
    return false;
  if (!matchAssignBinaryConst(stmt, lhs->name, "*", lhs->name, 2))
    return false;
  powerVar = lhs->name;
  return true;
}

bool collectBitLoopSummary(BlockStmt* loopBody,
                           const std::string& firstParam,
                           const std::string& secondParam,
                           const std::string& lenVar,
                           const std::string& resultVar,
                           BitLoopSummary& summary) {
  if (!loopBody)
    return false;

  std::vector<Stmt*> pendingIfs;
  bool sawUnknown = false;
  for (const auto& item : loopBody->items) {
    if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt) {
      sawUnknown = true;
      continue;
    }
    Stmt* stmt = item.stmt.get();

    std::string bitVar;
    if (!summary.hasBitA &&
        matchBitExtractAssign(stmt, firstParam, bitVar)) {
      summary.bitA = bitVar;
      summary.hasBitA = true;
      continue;
    }
    if (!summary.hasBitB &&
        matchBitExtractAssign(stmt, secondParam, bitVar)) {
      summary.bitB = bitVar;
      summary.hasBitB = true;
      continue;
    }
    if (!summary.shrinksFirstParam &&
        matchAssignBinaryConst(stmt, firstParam, "/", firstParam, 2)) {
      summary.shrinksFirstParam = true;
      continue;
    }
    if (!summary.shrinksSecondParam &&
        matchAssignBinaryConst(stmt, secondParam, "/", secondParam, 2)) {
      summary.shrinksSecondParam = true;
      continue;
    }

    std::string powerVar;
    if (!summary.doublesPower && matchPowerDoubleAssign(stmt, powerVar)) {
      summary.powerVar = powerVar;
      summary.doublesPower = true;
      continue;
    }
    if (!summary.decrementsLen &&
        matchAssignBinaryConst(stmt, lenVar, "-", lenVar, 1)) {
      summary.decrementsLen = true;
      continue;
    }
    if (dynamic_cast<IfStmt*>(stmt)) {
      pendingIfs.push_back(stmt);
      continue;
    }
    sawUnknown = true;
  }

  if (sawUnknown || !summary.hasBitA || !summary.hasBitB ||
      !summary.shrinksFirstParam || !summary.shrinksSecondParam ||
      !summary.doublesPower || !summary.decrementsLen) {
    return false;
  }

  for (Stmt* stmt : pendingIfs) {
    AstBitOp opKind = AstBitOp::None;
    if (matchAccumulatingIf(stmt, resultVar, summary.powerVar,
                            summary.bitA, summary.bitB, opKind)) {
      if (summary.accumulatesResult)
        return false;
      summary.opKind = opKind;
      summary.accumulatesResult = true;
    }
  }

  return pendingIfs.size() == 1 && summary.accumulatesResult &&
         summary.opKind != AstBitOp::None;
}

bool matchBitLoopFunction(FuncDef* func, AstBitOp& opKind) {
  if (!func || !func->body || func->retType.base != BaseType::Int ||
      func->retType.isArray() || func->params.size() != 2 ||
      !isScalarIntParam(func->params[0].get()) ||
      !isScalarIntParam(func->params[1].get()))
    return false;

  auto* body = func->body.get();
  WhileStmt* loop = nullptr;
  ReturnStmt* ret = nullptr;
  for (const auto& item : body->items) {
    if (item.kind == BlockStmt::Item::Kind::Decl)
      continue;
    if (dynamic_cast<WhileStmt*>(item.stmt.get())) {
      if (loop || ret)
        return false;
      loop = dynamic_cast<WhileStmt*>(item.stmt.get());
      continue;
    }
    if (dynamic_cast<ReturnStmt*>(item.stmt.get())) {
      if (ret)
        return false;
      ret = dynamic_cast<ReturnStmt*>(item.stmt.get());
      continue;
    }
    if (auto* exprStmt = dynamic_cast<ExprStmt*>(item.stmt.get())) {
      if (!exprStmt->expr)
        continue;
    }
    return false;
  }

  if (!loop || !ret || !ret->val)
    return false;

  auto* resultExpr = dynamic_cast<LValExpr*>(ret->val.get());
  if (!resultExpr || !resultExpr->indices.empty())
    return false;
  const std::string resultVar = resultExpr->name;

  std::string lenVar;
  if (auto* condVar = dynamic_cast<LValExpr*>(loop->cond.get())) {
    if (!condVar->indices.empty())
      return false;
    lenVar = condVar->name;
  } else {
    auto* condBin = dynamic_cast<BinaryExpr*>(loop->cond.get());
    if (!condBin || condBin->op != "!=")
      return false;
    auto* lhsVar = dynamic_cast<LValExpr*>(condBin->lhs.get());
    if (!lhsVar || !lhsVar->indices.empty() ||
        !isIntLiteral(condBin->rhs.get(), 0))
      return false;
    lenVar = lhsVar->name;
  }

  auto* loopBody = dynamic_cast<BlockStmt*>(loop->body.get());
  BitLoopSummary summary;
  if (!collectBitLoopSummary(loopBody,
                             func->params[0]->name,
                             func->params[1]->name,
                             lenVar,
                             resultVar,
                             summary)) {
    return false;
  }

  opKind = summary.opKind;
  return true;
}

void rewriteBitLoopFunction(FuncDef* func, AstBitOp opKind) {
  auto body = std::make_unique<BlockStmt>();
  body->line = func->body ? func->body->line : func->line;

  std::string op;
  if (opKind == AstBitOp::And)
    op = "&";
  else if (opKind == AstBitOp::Or)
    op = "|";
  else
    op = "^";

  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Stmt;
  item.stmt = makeReturnStmt(
      makeBinaryExpr(op,
                     makeVarExpr(func->params[0]->name, func->line),
                     makeVarExpr(func->params[1]->name, func->line),
                     func->line),
      func->line);
  body->items.push_back(std::move(item));
  func->body = std::move(body);
}

struct ExprHelperSummary {
  FuncDef* func = nullptr;
  ExprPtr exprTemplate;
  std::vector<std::string> params;
};

struct BitwiseEmulationSummary {
  ExprHelperSummary helper;
};

struct PowerOfTwoDigitHelperSummary {
  FuncDef* func = nullptr;
  std::string valueParam;
  std::string posParam;
  int radix = 0;
  int shift = 0;
  int mask = 0;
};

struct PowerOfTwoDigitLoopSummary {
  // 这里记录“循环语义角色”，而不是记录源码语句序号。
  // 只有能证明为 value 连续除以同一个 2 的幂 radix、IV 从 0 递增到
  // pos、最后返回 value % radix 时，才允许改写成移位取位段。
  std::string ivName;
  WhileStmt* loop = nullptr;
  ReturnStmt* ret = nullptr;
  int32_t radix = 0;
  bool valueDivides = false;
  bool ivIncrements = false;
};

// 深拷贝表达式树：按节点类型分派逐层克隆，遇到不支持的节点返回 nullptr，
// 供调用方放弃改写。
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
    copy->semaType = lval->semaType;
    copy->symbol = lval->symbol;
    for (const auto& idx : lval->indices)
      copy->indices.push_back(cloneExpr(idx.get()));
    return copy;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    auto copy = std::make_unique<CallExpr>();
    copy->name = call->name;
    copy->line = call->line;
    copy->semaType = call->semaType;
    copy->callee = call->callee;
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

const ReturnStmt* singleReturnStmt(FuncDef* func) {
  if (!func || !func->body)
    return nullptr;
  const ReturnStmt* ret = nullptr;
  for (const auto& item : func->body->items) {
    if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt)
      return nullptr;
    if (auto* exprStmt = dynamic_cast<ExprStmt*>(item.stmt.get())) {
      if (!exprStmt->expr)
        continue;
    }
    auto* candidate = dynamic_cast<ReturnStmt*>(item.stmt.get());
    if (!candidate || !candidate->val || ret)
      return nullptr;
    ret = candidate;
  }
  return ret;
}

bool isScalarIntParamList(const FuncDef* func) {
  if (!func)
    return false;
  for (const auto& param : func->params) {
    if (!param || param->type.isArray() || param->type.base != BaseType::Int)
      return false;
  }
  return true;
}

bool isIntConstExpr(const Expr* expr, int32_t& value) {
  auto* lit = dynamic_cast<const IntLitExpr*>(expr);
  if (!lit)
    return false;
  value = lit->val;
  return true;
}

bool isPowerOfTwoInt(int32_t value) {
  return value > 0 && (value & (value - 1)) == 0;
}

int log2PowerOfTwo(int32_t value) {
  int shift = 0;
  while (value > 1) {
    value >>= 1;
    ++shift;
  }
  return shift;
}

bool matchScalarDeclInit(const BlockStmt::Item& item,
                         std::string& name,
                         Expr*& initOut) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->defs.size() != 1) {
    return false;
  }
  const auto& def = item.decl->defs.front();
  if (!def || def->type.isArray() || !def->initVal)
    return false;
  auto* init = dynamic_cast<ScalarInit*>(def->initVal.get());
  if (!init || !init->expr)
    return false;
  name = def->name;
  initOut = init->expr.get();
  return true;
}

bool matchScalarAssign(Stmt* stmt,
                       const std::string& name,
                       Expr*& rhsOut) {
  auto* assign = dynamic_cast<AssignStmt*>(stmt);
  auto* lval = assign ? dynamic_cast<LValExpr*>(assign->lval.get()) : nullptr;
  if (!assign || !lval || !lval->indices.empty() || lval->name != name)
    return false;
  rhsOut = assign->rhs.get();
  return rhsOut != nullptr;
}

// 递归折叠整型常量表达式：字面量直接取值，标量变量查 constBindings，
// 一元仅支持 +/-，二元算术在除数为 0 时拒绝求值。
bool resolveIntConstExpr(const Expr* expr,
                         const std::unordered_map<std::string, int32_t>& constBindings,
                         int32_t& value) {
  if (!expr)
    return false;
  if (auto* lit = dynamic_cast<const IntLitExpr*>(expr)) {
    value = lit->val;
    return true;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (!lval->indices.empty())
      return false;
    auto it = constBindings.find(lval->name);
    if (it == constBindings.end())
      return false;
    value = it->second;
    return true;
  }
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    int32_t inner = 0;
    if (!resolveIntConstExpr(unary->operand.get(), constBindings, inner))
      return false;
    if (unary->op == '+') {
      value = inner;
      return true;
    }
    if (unary->op == '-') {
      value = -inner;
      return true;
    }
    return false;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    int32_t lhs = 0;
    int32_t rhs = 0;
    if (!resolveIntConstExpr(binary->lhs.get(), constBindings, lhs) ||
        !resolveIntConstExpr(binary->rhs.get(), constBindings, rhs)) {
      return false;
    }
    if (binary->op == "+") value = lhs + rhs;
    else if (binary->op == "-") value = lhs - rhs;
    else if (binary->op == "*") value = lhs * rhs;
    else if (binary->op == "/" && rhs != 0) value = lhs / rhs;
    else if (binary->op == "%" && rhs != 0) value = lhs % rhs;
    else return false;
    return true;
  }
  return false;
}

bool collectHarmlessScalarDecls(
    const DeclStmt* decl,
    std::unordered_map<std::string, int32_t>& constBindings) {
  if (!decl)
    return false;
  for (const auto& def : decl->defs) {
    if (!def || def->type.isArray())
      return false;

    // 本 pass 会重写整个 helper 函数体，因此局部声明必须没有
    // 可观察初始化副作用。无初始化的标量槽位只作为局部存储，删除安全；
    // 有初始化时只接受可在编译期求值的常量表达式。
    if (!def->initVal)
      continue;
    auto* init = dynamic_cast<ScalarInit*>(def->initVal.get());
    int32_t value = 0;
    if (!init || !resolveIntConstExpr(init->expr.get(), constBindings, value))
      return false;
    constBindings[def->name] = value;
  }
  return true;
}

bool matchDigitLoopCondition(Expr* expr,
                             const std::string& posParam,
                             std::string& ivName) {
  auto* cond = dynamic_cast<BinaryExpr*>(expr);
  if (!cond)
    return false;

  auto* lhs = dynamic_cast<LValExpr*>(cond->lhs.get());
  auto* rhs = dynamic_cast<LValExpr*>(cond->rhs.get());
  if (!lhs || !rhs || !lhs->indices.empty() || !rhs->indices.empty())
    return false;

  if (cond->op == "<" && rhs->name == posParam) {
    ivName = lhs->name;
    return true;
  }
  if (cond->op == ">" && lhs->name == posParam) {
    ivName = rhs->name;
    return true;
  }
  return false;
}

bool matchIncrementByOne(Expr* expr, const std::string& ivName) {
  auto* bin = dynamic_cast<BinaryExpr*>(expr);
  if (!bin || bin->op != "+")
    return false;
  return (isVarExpr(bin->lhs.get(), ivName) && isIntLiteral(bin->rhs.get(), 1)) ||
         (isIntLiteral(bin->lhs.get(), 1) && isVarExpr(bin->rhs.get(), ivName));
}

bool matchValueDivideByRadix(
    Expr* expr,
    const std::string& valueParam,
    const std::unordered_map<std::string, int32_t>& constBindings,
    int32_t& radixOut) {
  auto* bin = dynamic_cast<BinaryExpr*>(expr);
  return bin && bin->op == "/" && isVarExpr(bin->lhs.get(), valueParam) &&
         resolveIntConstExpr(bin->rhs.get(), constBindings, radixOut);
}

bool collectDigitLoopBody(
    BlockStmt* loopBody,
    const std::string& valueParam,
    const std::string& ivName,
    std::unordered_map<std::string, int32_t> constBindings,
    PowerOfTwoDigitLoopSummary& loopSummary) {
  if (!loopBody)
    return false;

  for (const auto& item : loopBody->items) {
    if (item.kind == BlockStmt::Item::Kind::Decl) {
      if (!collectHarmlessScalarDecls(item.decl.get(), constBindings))
        return false;
      continue;
    }

    if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt)
      return false;
    if (auto* exprStmt = dynamic_cast<ExprStmt*>(item.stmt.get())) {
      if (!exprStmt->expr)
        continue;
    }

    Expr* rhs = nullptr;
    if (matchScalarAssign(item.stmt.get(), valueParam, rhs)) {
      int32_t radix = 0;
      if (loopSummary.valueDivides ||
          !matchValueDivideByRadix(rhs, valueParam, constBindings, radix)) {
        return false;
      }
      loopSummary.valueDivides = true;
      loopSummary.radix = radix;
      continue;
    }

    if (matchScalarAssign(item.stmt.get(), ivName, rhs)) {
      if (loopSummary.ivIncrements || !matchIncrementByOne(rhs, ivName))
        return false;
      loopSummary.ivIncrements = true;
      continue;
    }

    return false;
  }

  return loopSummary.valueDivides && loopSummary.ivIncrements;
}

bool matchDigitHelperLoop(FuncDef* func,
                          const std::unordered_map<std::string, int32_t>& constBindings,
                          PowerOfTwoDigitHelperSummary& summary) {
  if (!func || !func->body || func->retType.isArray() ||
      func->retType.base != BaseType::Int || func->params.size() != 2 ||
      !isScalarIntParam(func->params[0].get()) ||
      !isScalarIntParam(func->params[1].get())) {
    return false;
  }

  BlockStmt* body = func->body.get();
  auto localConstBindings = constBindings;
  PowerOfTwoDigitLoopSummary loopSummary;

  for (const auto& item : body->items) {
    if (item.kind == BlockStmt::Item::Kind::Decl) {
      if (!collectHarmlessScalarDecls(item.decl.get(), localConstBindings))
        return false;
      continue;
    }

    if (item.kind != BlockStmt::Item::Kind::Stmt || !item.stmt)
      return false;

    if (auto* exprStmt = dynamic_cast<ExprStmt*>(item.stmt.get())) {
      if (!exprStmt->expr)
        continue;
    }

    if (auto* whileStmt = dynamic_cast<WhileStmt*>(item.stmt.get())) {
      if (loopSummary.loop || loopSummary.ret)
        return false;
      loopSummary.loop = whileStmt;
      if (!matchDigitLoopCondition(whileStmt->cond.get(),
                                   func->params[1]->name,
                                   loopSummary.ivName)) {
        return false;
      }
      continue;
    }

    if (auto* retStmt = dynamic_cast<ReturnStmt*>(item.stmt.get())) {
      if (loopSummary.ret || !retStmt->val)
        return false;
      loopSummary.ret = retStmt;
      continue;
    }

    return false;
  }

  if (!loopSummary.loop || !loopSummary.ret || loopSummary.ivName.empty() ||
      loopSummary.ivName == func->params[0]->name ||
      loopSummary.ivName == func->params[1]->name) {
    return false;
  }
  auto ivInitIt = localConstBindings.find(loopSummary.ivName);
  if (ivInitIt == localConstBindings.end() || ivInitIt->second != 0)
    return false;

  auto* loopBody = dynamic_cast<BlockStmt*>(loopSummary.loop->body.get());
  if (!collectDigitLoopBody(loopBody,
                            func->params[0]->name,
                            loopSummary.ivName,
                            localConstBindings,
                            loopSummary)) {
    return false;
  }

  // radix 至少为 4 是通用收益阈值。radix=2 的奇偶化应由
  // parity/InstCombine 规则处理，避免为过小收益生成较大的表达式。
  if (!isPowerOfTwoInt(loopSummary.radix) || loopSummary.radix < 4)
    return false;

  auto* retBin = dynamic_cast<BinaryExpr*>(loopSummary.ret->val.get());
  int32_t retRadix = 0;
  if (!retBin || retBin->op != "%" ||
      !isVarExpr(retBin->lhs.get(), func->params[0]->name) ||
      !resolveIntConstExpr(retBin->rhs.get(), localConstBindings, retRadix) ||
      retRadix != loopSummary.radix) {
    return false;
  }

  summary.func = func;
  summary.valueParam = func->params[0]->name;
  summary.posParam = func->params[1]->name;
  summary.radix = loopSummary.radix;
  summary.shift = log2PowerOfTwo(loopSummary.radix);
  summary.mask = loopSummary.radix - 1;
  return true;
}

void rewritePowerOfTwoDigitHelper(const PowerOfTwoDigitHelperSummary& summary) {
  if (!summary.func || summary.shift <= 0)
    return;

  int line = summary.func->line;
  auto body = std::make_unique<BlockStmt>();
  body->line = line;
  const int maxSafePos = 31 / summary.shift;
  auto valid = makeBinaryExpr("<",
                              makeVarExpr(summary.posParam, line),
                              makeIntLit(maxSafePos + 1, line),
                              line);
  auto validMask = makeUnaryExpr('-', cloneExpr(valid.get()), line);
  auto boundedPos = makeBinaryExpr(
      "&",
      makeVarExpr(summary.posParam, line),
      cloneExpr(validMask.get()),
      line);

  // 移位量为 pos*shift；pos 已被 validMask 截断，越界位段的移位量归 0。
  auto shiftAmount = makeBinaryExpr(
      "*",
      std::move(boundedPos),
      makeIntLit(summary.shift, line),
      line);
  // 符号修正：借 value 符号位构造借位项，使负数算术右移等价于 C 整除/取余的向零截断。
  auto signMask = makeBinaryExpr(
      "&",
      makeBinaryExpr(">>", makeVarExpr(summary.valueParam, line), makeIntLit(31, line), line),
      makeBinaryExpr(
          "-",
          makeBinaryExpr("<<", makeIntLit(1, line), cloneExpr(shiftAmount.get()), line),
          makeIntLit(1, line),
          line),
      line);
  auto shifted = makeBinaryExpr(
      ">>",
      makeBinaryExpr(
          "+",
          makeVarExpr(summary.valueParam, line),
          std::move(signMask),
          line),
      cloneExpr(shiftAmount.get()),
      line);
  // 余数符号修正：负值先加 mask 再取位段、最后减回调整项，等价于向零截断的取余语义。
  auto lowAdjust = makeBinaryExpr(
      "&",
      makeBinaryExpr(">>", cloneExpr(shifted.get()), makeIntLit(31, line), line),
      makeIntLit(summary.mask, line),
      line);
  auto lowAdjustClone = cloneExpr(lowAdjust.get());
  auto digit = makeBinaryExpr(
      "-",
      makeBinaryExpr(
          "&",
          makeBinaryExpr("+", std::move(shifted), std::move(lowAdjustClone), line),
          makeIntLit(summary.mask, line),
          line),
      std::move(lowAdjust),
      line);
  // 越界的 pos 已在 boundedPos 处被置 0，这里再整体套 validMask，使无效位段结果恒为 0。
  auto maskedDigit = makeBinaryExpr(
      "&",
      std::move(digit),
      std::move(validMask),
      line);

  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Stmt;
  item.stmt = makeReturnStmt(std::move(maskedDigit), line);
  body->items.push_back(std::move(item));
  summary.func->body = std::move(body);
}

bool exprLooksBitOrRotateLike(const Expr* expr) {
  if (!expr)
    return false;
  if (dynamic_cast<const IntLitExpr*>(expr) || dynamic_cast<const LValExpr*>(expr))
    return true;
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return unary->op == '-' || unary->op == '!' ?
        exprLooksBitOrRotateLike(unary->operand.get()) : false;
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprLooksBitOrRotateLike(cast->operand.get());
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    if (!exprLooksBitOrRotateLike(binary->lhs.get()) ||
        !exprLooksBitOrRotateLike(binary->rhs.get())) {
      return false;
    }
    if (binary->op == "&" || binary->op == "|" || binary->op == "^" ||
        binary->op == "&&" || binary->op == "||" ||
        binary->op == "<<" || binary->op == ">>") {
      return true;
    }
    if (binary->op == "+" || binary->op == "-" || binary->op == "*" ||
        binary->op == "/" || binary->op == "%") {
      int32_t rhs = 0;
      int32_t lhs = 0;
      if (isIntConstExpr(binary->rhs.get(), rhs) && isPowerOfTwoInt(std::abs(rhs)))
        return true;
      if (isIntConstExpr(binary->lhs.get(), lhs) && isPowerOfTwoInt(std::abs(lhs)))
        return true;
      if (binary->op == "+" || binary->op == "-")
        return true;
      return false;
    }
    if (binary->op == "==" || binary->op == "!=" || binary->op == "<" ||
        binary->op == "<=" || binary->op == ">" || binary->op == ">=") {
      return true;
    }
    return false;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    // 调用表达式本身只表示“可能是小 helper”。这里不看普通用户函数名，
    // 由 exprIsSummarizable 递归检查 callee 是否为纯位运算/强度削减摘要，
    // 并检查实参是否在同一闭包内。
    return true;
  }
  return false;
}

bool exprIsSummarizable(const Expr* expr,
                        const std::unordered_set<std::string>& paramNames,
                        const std::unordered_map<std::string, FuncDef*>& funcByName,
                        std::unordered_map<std::string, int>& memo,
                        std::unordered_set<std::string>& visiting);

bool functionIsSummarizable(FuncDef* func,
                            const std::unordered_map<std::string, FuncDef*>& funcByName,
                            std::unordered_map<std::string, int>& memo,
                            std::unordered_set<std::string>& visiting) {
  // 表达式 helper 摘要只处理“有实参参与计算”的小纯函数。
  // 这比按入口函数名排除更通用，也避免把无参入口/常量函数误当作可替换 helper。
  if (!func || func->params.empty())
    return false;

  auto memoIt = memo.find(func->name);
  if (memoIt != memo.end())
    return memoIt->second == 1;
  if (!visiting.insert(func->name).second)
    return false;

  bool ok = false;
  do {
    if (func->retType.isArray() || func->retType.base != BaseType::Int)
      break;
    if (!isScalarIntParamList(func))
      break;
    const ReturnStmt* ret = singleReturnStmt(func);
    if (!ret)
      break;
    if (!exprLooksBitOrRotateLike(ret->val.get()))
      break;

    std::unordered_set<std::string> paramNames;
    for (const auto& param : func->params)
      paramNames.insert(param->name);
    ok = exprIsSummarizable(ret->val.get(), paramNames, funcByName, memo, visiting);
  } while (false);

  visiting.erase(func->name);
  memo[func->name] = ok ? 1 : -1;
  return ok;
}

bool exprIsSummarizable(const Expr* expr,
                        const std::unordered_set<std::string>& paramNames,
                        const std::unordered_map<std::string, FuncDef*>& funcByName,
                        std::unordered_map<std::string, int>& memo,
                        std::unordered_set<std::string>& visiting) {
  if (!expr)
    return false;
  if (dynamic_cast<const IntLitExpr*>(expr))
    return true;
  if (auto* lval = dynamic_cast<const LValExpr*>(expr))
    return lval->indices.empty() && paramNames.count(lval->name) != 0;
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr))
    return exprIsSummarizable(unary->operand.get(), paramNames, funcByName, memo, visiting);
  if (auto* cast = dynamic_cast<const CastExpr*>(expr))
    return exprIsSummarizable(cast->operand.get(), paramNames, funcByName, memo, visiting);
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr))
    return exprIsSummarizable(binary->lhs.get(), paramNames, funcByName, memo, visiting) &&
           exprIsSummarizable(binary->rhs.get(), paramNames, funcByName, memo, visiting);
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    auto funcIt = funcByName.find(call->name);
    if (funcIt == funcByName.end())
      return false;
    FuncDef* callee = funcIt->second;
    if (!functionIsSummarizable(callee, funcByName, memo, visiting) ||
        call->args.size() != callee->params.size()) {
      return false;
    }
    for (const auto& arg : call->args) {
      if (!exprIsSummarizable(arg.get(), paramNames, funcByName, memo, visiting))
        return false;
    }
    return true;
  }
  return false;
}

std::unordered_map<std::string, BitwiseEmulationSummary>
collectBitwiseEmulationSummaries(
    CompUnit& cu,
    const std::unordered_set<std::string>& excludedFunctions) {
  std::unordered_map<std::string, FuncDef*> funcByName;
  for (auto& item : cu.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func)
      funcByName[item.func->name] = item.func.get();
  }

  std::unordered_map<std::string, int> memo;
  std::unordered_set<std::string> visiting;
  std::unordered_map<std::string, BitwiseEmulationSummary> summaries;
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    FuncDef* func = item.func.get();
    if (excludedFunctions.count(func->name) != 0)
      continue;
    if (!functionIsSummarizable(func, funcByName, memo, visiting))
      continue;

    const ReturnStmt* ret = singleReturnStmt(func);
    if (!ret || !ret->val)
      continue;
    BitwiseEmulationSummary summary;
    summary.helper.func = func;
    summary.helper.exprTemplate = cloneExpr(ret->val.get());
    for (const auto& param : func->params)
      summary.helper.params.push_back(param->name);
    summaries[func->name] = std::move(summary);
  }
  return summaries;
}

bool applyPowerOfTwoDigitHelperRewrite(
    CompUnit& cu,
    std::unordered_set<std::string>* rewrittenFunctions) {
  std::unordered_map<std::string, int32_t> constBindings;
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::GlobalDecl || !item.decl ||
        !item.decl->isConst) {
      continue;
    }
    for (const auto& def : item.decl->defs) {
      if (!def || def->type.isArray() || !def->initVal)
        continue;
      auto* init = dynamic_cast<ScalarInit*>(def->initVal.get());
      int32_t value = 0;
      if (init && resolveIntConstExpr(init->expr.get(), constBindings, value))
        constBindings[def->name] = value;
    }
  }

  bool changed = false;
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    PowerOfTwoDigitHelperSummary summary;
    if (!matchDigitHelperLoop(item.func.get(), constBindings, summary))
      continue;
    rewritePowerOfTwoDigitHelper(summary);
    if (rewrittenFunctions)
      rewrittenFunctions->insert(item.func->name);
    changed = true;
  }
  return changed;
}

ExprPtr rewriteExprWithSummaries(
    const Expr* expr,
    const std::unordered_map<std::string, BitwiseEmulationSummary>& summaries,
    const std::unordered_map<std::string, const Expr*>* substMap,
    int depth) {
  if (!expr)
    return nullptr;
  // 二次幂进制辅助函数会展开为较深但有限的表达式树。保留递归深度上限，
  // 同时留出足够空间替换调用点中的参数叶节点。
  if (depth > 128)
    return cloneExpr(expr);

  // 叶节点与普通节点：标量 LVal 先经 substMap 做形参->实参替换（辅助模板内联），
  // 其余按类型逐层递归重建副本；真正的辅助函数展开集中在下方 CallExpr 分支。
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    if (substMap && lval->indices.empty()) {
      auto it = substMap->find(lval->name);
      if (it != substMap->end())
        return rewriteExprWithSummaries(it->second, summaries, nullptr, depth + 1);
    }
    auto copy = std::make_unique<LValExpr>();
    copy->name = lval->name;
    copy->line = lval->line;
    copy->semaType = lval->semaType;
    copy->symbol = lval->symbol;
    for (const auto& idx : lval->indices)
      copy->indices.push_back(rewriteExprWithSummaries(idx.get(), summaries, substMap, depth + 1));
    return copy;
  }
  if (auto* lit = dynamic_cast<const IntLitExpr*>(expr))
    return cloneExpr(lit);
  if (auto* lit = dynamic_cast<const FloatLitExpr*>(expr))
    return cloneExpr(lit);
  if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    auto copy = std::make_unique<UnaryExpr>();
    copy->op = unary->op;
    copy->line = unary->line;
    copy->semaType = unary->semaType;
    copy->operand = rewriteExprWithSummaries(unary->operand.get(), summaries, substMap, depth + 1);
    return copy;
  }
  if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    auto copy = std::make_unique<CastExpr>();
    copy->targetType = cast->targetType;
    copy->line = cast->line;
    copy->semaType = cast->semaType;
    copy->operand = rewriteExprWithSummaries(cast->operand.get(), summaries, substMap, depth + 1);
    return copy;
  }
  if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    auto copy = std::make_unique<BinaryExpr>();
    copy->op = binary->op;
    copy->line = binary->line;
    copy->semaType = binary->semaType;
    copy->lhs = rewriteExprWithSummaries(binary->lhs.get(), summaries, substMap, depth + 1);
    copy->rhs = rewriteExprWithSummaries(binary->rhs.get(), summaries, substMap, depth + 1);
    return copy;
  }
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    std::vector<ExprPtr> rewrittenArgs;
    rewrittenArgs.reserve(call->args.size());
    for (const auto& arg : call->args)
      rewrittenArgs.push_back(rewriteExprWithSummaries(arg.get(), summaries, substMap, depth + 1));

    auto summaryIt = summaries.find(call->name);
    if (summaryIt != summaries.end() &&
        summaryIt->second.helper.params.size() == rewrittenArgs.size()) {
      std::unordered_map<std::string, const Expr*> callSubst;
      for (size_t i = 0; i < rewrittenArgs.size(); ++i)
        callSubst[summaryIt->second.helper.params[i]] = rewrittenArgs[i].get();
      return rewriteExprWithSummaries(summaryIt->second.helper.exprTemplate.get(), summaries,
                                      &callSubst, depth + 1);
    }

    auto copy = std::make_unique<CallExpr>();
    copy->name = call->name;
    copy->line = call->line;
    copy->semaType = call->semaType;
    copy->callee = call->callee;
    for (auto& arg : rewrittenArgs)
      copy->args.push_back(std::move(arg));
    return copy;
  }
  return cloneExpr(expr);
}

// 表达式级改写入口：基于 summaries 重建整棵表达式树，并原位替换原指针。
void rewriteExprPtr(ExprPtr& expr,
                    const std::unordered_map<std::string, BitwiseEmulationSummary>& summaries) {
  if (!expr)
    return;
  expr = rewriteExprWithSummaries(expr.get(), summaries, nullptr, 0);
}

void rewriteInitVal(InitVal* init,
                    const std::unordered_map<std::string, BitwiseEmulationSummary>& summaries);

// 语句级改写入口：按语句类型分派，递归改写其中包含的全部表达式；
// 块内声明走初值改写路径，if/while 递归处理条件与各分支。
void rewriteStmt(Stmt* stmt,
                 const std::unordered_map<std::string, BitwiseEmulationSummary>& summaries) {
  if (!stmt)
    return;
  if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) {
    rewriteExprPtr(assign->lval, summaries);
    rewriteExprPtr(assign->rhs, summaries);
    return;
  }
  if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
    rewriteExprPtr(exprStmt->expr, summaries);
    return;
  }
  if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
    for (auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Decl && item.decl) {
        for (auto& def : item.decl->defs) {
          if (def && def->initVal)
            rewriteInitVal(def->initVal.get(), summaries);
        }
      } else if (item.stmt) {
        rewriteStmt(item.stmt.get(), summaries);
      }
    }
    return;
  }
  if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
    rewriteExprPtr(ifStmt->cond, summaries);
    rewriteStmt(ifStmt->thenBranch.get(), summaries);
    rewriteStmt(ifStmt->elseBranch.get(), summaries);
    return;
  }
  if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
    rewriteExprPtr(whileStmt->cond, summaries);
    rewriteStmt(whileStmt->body.get(), summaries);
    return;
  }
  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt)) {
    rewriteExprPtr(ret->val, summaries);
    return;
  }
}

void rewriteInitVal(InitVal* init,
                    const std::unordered_map<std::string, BitwiseEmulationSummary>& summaries) {
  if (!init)
    return;
  if (auto* scalar = dynamic_cast<ScalarInit*>(init)) {
    rewriteExprPtr(scalar->expr, summaries);
    return;
  }
  if (auto* aggregate = dynamic_cast<AggregateInit*>(init)) {
    for (auto& item : aggregate->items)
      rewriteInitVal(item.get(), summaries);
  }
}

bool applyExprHelperSynthesis(CompUnit& cu,
                              const std::unordered_set<std::string>& excludedFunctions) {
  auto summaries = collectBitwiseEmulationSummaries(cu, excludedFunctions);
  if (summaries.empty())
    return false;

  bool changed = false;
  for (auto& item : cu.items) {
    if (item.kind == CompUnit::Item::Kind::GlobalDecl && item.decl) {
      for (auto& def : item.decl->defs) {
        if (def && def->initVal)
          rewriteInitVal(def->initVal.get(), summaries);
      }
      continue;
    }
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func || !item.func->body)
      continue;
    rewriteStmt(item.func->body.get(), summaries);
    changed = true;
  }
  return changed;
}

bool matchesAddPair(core_ir::Value* value, core_ir::Value* lhs,
                    core_ir::Value* rhs) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::Add || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == lhs && inst->operand(1) == rhs) ||
         (inst->operand(0) == rhs && inst->operand(1) == lhs);
}

bool matchesXorPair(core_ir::Value* value, core_ir::Value* lhs,
                    core_ir::Value* rhs) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::Xor || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == lhs && inst->operand(1) == rhs) ||
         (inst->operand(0) == rhs && inst->operand(1) == lhs);
}

// 检查指令是否提取单个位：(val >> i) & 1；返回源值和位位置，
// 无法识别时返回 {nullptr, -1}。
struct BitExtract {
  core_ir::Value* src = nullptr;
  int bitPos = -1;
  core_ir::Value* bitPosVal = nullptr;
};

BitExtract detectBitExtract(core_ir::Instruction* inst) {
  BitExtract result;
  if (inst->opcode() != core_ir::Opcode::And) return result;
  core_ir::Value* maskOp = nullptr;
  core_ir::Value* shiftOp = nullptr;
  if (auto* ci = dynamic_cast<core_ir::ConstInt*>(inst->operand(0))) {
    if (ci->value() == 1) {
      maskOp = inst->operand(0);
      shiftOp = inst->operand(1);
    }
  }
  if (!maskOp && dynamic_cast<core_ir::ConstInt*>(inst->operand(1))) {
    auto* ci = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
    if (ci->value() == 1) {
      maskOp = inst->operand(1);
      shiftOp = inst->operand(0);
    }
  }
  if (!maskOp || !shiftOp) return result;

  if (auto* shr = dynamic_cast<core_ir::Instruction*>(shiftOp)) {
    if (shr->opcode() == core_ir::Opcode::SDiv) {
      if (auto* ci = dynamic_cast<core_ir::ConstInt*>(shr->operand(1))) {
        int val = ci->value();
        if (val > 1 && (val & (val - 1)) == 0) {
          result.src = shr->operand(0);
          return result;
        }
      }
    }
  }
  return result;
}

bool isConstInt(core_ir::Value* value, int32_t expected) {
  auto* ci = dynamic_cast<core_ir::ConstInt*>(value);
  return ci && ci->value() == expected;
}

bool matchesSignBit(core_ir::Value* value, core_ir::Value* phi) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::And || inst->numOperands() != 2)
    return false;
  core_ir::Value* shift = nullptr;
  if (isConstInt(inst->operand(0), 1))
    shift = inst->operand(1);
  else if (isConstInt(inst->operand(1), 1))
    shift = inst->operand(0);
  else
    return false;
  auto* shr = dynamic_cast<core_ir::Instruction*>(shift);
  return shr && shr->opcode() == core_ir::Opcode::Shr &&
         shr->numOperands() == 2 && shr->operand(0) == phi &&
         isConstInt(shr->operand(1), 31);
}

bool matchesAdjustedArg(core_ir::Value* value, core_ir::Value* phi) {
  if (value == phi)
    return true;
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::Add || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == phi && matchesSignBit(inst->operand(1), phi)) ||
         (inst->operand(1) == phi && matchesSignBit(inst->operand(0), phi));
}

bool matchesLowBitValue(core_ir::Value* value, core_ir::Value* phi) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == core_ir::Opcode::And && inst->numOperands() == 2) {
    if (isConstInt(inst->operand(0), 1) && matchesAdjustedArg(inst->operand(1), phi))
      return true;
    if (isConstInt(inst->operand(1), 1) && matchesAdjustedArg(inst->operand(0), phi))
      return true;
  }
  if (inst->opcode() == core_ir::Opcode::Sub && inst->numOperands() == 2) {
    return matchesLowBitValue(inst->operand(0), phi) &&
           matchesSignBit(inst->operand(1), phi);
  }
  return false;
}

std::string bitCompareCondition(const core_ir::Instruction* inst) {
  if (!inst || inst->opcode() != core_ir::Opcode::ICmp)
    return "";
  const std::string& name = inst->name();
  const size_t separator = name.find('_');
  return separator == std::string::npos ? name : name.substr(0, separator);
}

bool matchesSignedLowBitValue(core_ir::Value* value, core_ir::Value* phi) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == core_ir::Opcode::SRem && inst->numOperands() == 2 &&
      inst->operand(0) == phi && isConstInt(inst->operand(1), 2)) {
    return true;
  }
  return inst->opcode() == core_ir::Opcode::Sub && inst->numOperands() == 2 &&
         matchesLowBitValue(inst->operand(0), phi) &&
         matchesSignBit(inst->operand(1), phi);
}

bool matchesNonNegativePredicate(core_ir::Value* value,
                                 core_ir::Value* phi) {
  auto* cmp = dynamic_cast<core_ir::Instruction*>(value);
  if (!cmp || cmp->opcode() != core_ir::Opcode::ICmp ||
      cmp->numOperands() != 2)
    return false;
  const std::string cond = bitCompareCondition(cmp);
  return (cond == "ge" && cmp->operand(0) == phi &&
          isConstInt(cmp->operand(1), 0)) ||
         (cond == "le" && isConstInt(cmp->operand(0), 0) &&
          cmp->operand(1) == phi);
}

bool matchesOddPredicate(core_ir::Value* value, core_ir::Value* phi) {
  auto* cmp = dynamic_cast<core_ir::Instruction*>(value);
  if (!cmp || cmp->opcode() != core_ir::Opcode::ICmp ||
      cmp->numOperands() != 2)
    return false;
  const std::string cond = bitCompareCondition(cmp);
  const auto signedOne = [&](core_ir::Value* remainder,
                             core_ir::Value* constant) {
    return cond == "eq" && matchesSignedLowBitValue(remainder, phi) &&
           isConstInt(constant, 1);
  };
  const auto nativeOdd = [&](core_ir::Value* bit, core_ir::Value* constant) {
    return cond == "ne" && matchesLowBitValue(bit, phi) &&
           isConstInt(constant, 0);
  };
  return signedOne(cmp->operand(0), cmp->operand(1)) ||
         signedOne(cmp->operand(1), cmp->operand(0)) ||
         nativeOdd(cmp->operand(0), cmp->operand(1)) ||
         nativeOdd(cmp->operand(1), cmp->operand(0));
}

bool matchesSignedOnePredicate(core_ir::Value* value, core_ir::Value* phi) {
  if (auto* truth = dynamic_cast<core_ir::Instruction*>(value);
      truth && truth->opcode() == core_ir::Opcode::ICmp &&
      truth->numOperands() == 2 && bitCompareCondition(truth) == "ne") {
    if (isConstInt(truth->operand(1), 0))
      return matchesSignedOnePredicate(truth->operand(0), phi);
    if (isConstInt(truth->operand(0), 0))
      return matchesSignedOnePredicate(truth->operand(1), phi);
  }
  if (matchesOddPredicate(value, phi)) {
    auto* cmp = dynamic_cast<core_ir::Instruction*>(value);
    if (cmp && ((matchesSignedLowBitValue(cmp->operand(0), phi) &&
                 isConstInt(cmp->operand(1), 1)) ||
                (matchesSignedLowBitValue(cmp->operand(1), phi) &&
                 isConstInt(cmp->operand(0), 1)))) {
      return true;
    }
  }
  auto* both = dynamic_cast<core_ir::Instruction*>(value);
  return both && both->opcode() == core_ir::Opcode::And &&
         both->numOperands() == 2 &&
         ((matchesNonNegativePredicate(both->operand(0), phi) &&
           matchesOddPredicate(both->operand(1), phi)) ||
          (matchesNonNegativePredicate(both->operand(1), phi) &&
           matchesOddPredicate(both->operand(0), phi)));
}

bool matchesBitAccumulatorUpdate(core_ir::Value* value,
                                 core_ir::Instruction* resultPhi,
                                 core_ir::Instruction* powerPhi,
                                 const char** failureReason) {
  auto fail = [&](const char* reason) {
    if (failureReason)
      *failureReason = reason;
    return false;
  };
  if (!resultPhi || !powerPhi)
    return fail("missing-input");
  auto* finalXor = dynamic_cast<core_ir::Instruction*>(value);
  if (finalXor && finalXor->opcode() == core_ir::Opcode::Phi) {
    bool sawCarry = false;
    bool sawIncrement = false;
    for (int i = 0; i + 1 < finalXor->numOperands(); i += 2) {
      core_ir::Value* incoming = finalXor->operand(i);
      if (incoming == resultPhi) {
        sawCarry = true;
        continue;
      }
      if (matchesAddPair(incoming, resultPhi, powerPhi)) {
        sawIncrement = true;
        continue;
      }
      return fail("phi-input");
    }
    if (sawCarry && sawIncrement) {
      if (failureReason)
        *failureReason = nullptr;
      return true;
    }
    return fail("phi-select");
  }
  if (!finalXor || finalXor->opcode() != core_ir::Opcode::Xor ||
      finalXor->numOperands() != 2) {
    return fail("final-xor");
  }

  core_ir::Value* maskedDelta = nullptr;
  if (finalXor->operand(0) == resultPhi)
    maskedDelta = finalXor->operand(1);
  else if (finalXor->operand(1) == resultPhi)
    maskedDelta = finalXor->operand(0);
  else
    return fail("final-xor-missing-result");

  auto* maskAnd = dynamic_cast<core_ir::Instruction*>(maskedDelta);
  if (!maskAnd || maskAnd->opcode() != core_ir::Opcode::And ||
      maskAnd->numOperands() != 2) {
    return fail("mask-and");
  }

  core_ir::Value* toggle = nullptr;
  core_ir::Value* negMask = nullptr;
  if (auto* sub = dynamic_cast<core_ir::Instruction*>(maskAnd->operand(0));
      sub && sub->opcode() == core_ir::Opcode::Sub && sub->numOperands() == 2 &&
      isConstInt(sub->operand(0), 0)) {
    negMask = sub;
    toggle = maskAnd->operand(1);
  } else if (auto* sub = dynamic_cast<core_ir::Instruction*>(maskAnd->operand(1));
             sub && sub->opcode() == core_ir::Opcode::Sub &&
             sub->numOperands() == 2 && isConstInt(sub->operand(0), 0)) {
    negMask = sub;
    toggle = maskAnd->operand(0);
  } else {
    return fail("neg-mask");
  }

  core_ir::Instruction* toggleXor =
      dynamic_cast<core_ir::Instruction*>(toggle);
  if (!toggleXor || toggleXor->opcode() != core_ir::Opcode::Xor ||
      toggleXor->numOperands() != 2) {
    return fail("toggle-xor");
  }
  core_ir::Value* added = nullptr;
  if (toggleXor->operand(0) == resultPhi)
    added = toggleXor->operand(1);
  else if (toggleXor->operand(1) == resultPhi)
    added = toggleXor->operand(0);
  else
    return fail("toggle-missing-result");

  if (!matchesAddPair(added, resultPhi, powerPhi))
    return fail("result-plus-power");

  auto* negMaskInst = dynamic_cast<core_ir::Instruction*>(negMask);
  auto* cond = negMaskInst
                   ? dynamic_cast<core_ir::Instruction*>(negMaskInst->operand(1))
                   : nullptr;
  if (!cond || cond->opcode() != core_ir::Opcode::ICmp)
    return fail("cond-icmp");
  if (failureReason)
    *failureReason = nullptr;
  return true;
}

bool matchesBitAccumulatorUpdate(core_ir::Value* value,
                                 core_ir::Instruction* resultPhi,
                                 core_ir::Instruction* powerPhi) {
  return matchesBitAccumulatorUpdate(value, resultPhi, powerPhi, nullptr);
}

bool bodyContainsBitAccumulatorUpdate(core_ir::BasicBlock* body,
                                      core_ir::Instruction* resultPhi,
                                      core_ir::Instruction* powerPhi,
                                      const char** failureReason = nullptr) {
  if (!body || !resultPhi || !powerPhi) {
    if (failureReason)
      *failureReason = "missing-body-or-phi";
    return false;
  }
  const char* lastFailure = "no-body-match";
  for (auto& instPtr : body->instructions()) {
    const char* localFailure = nullptr;
    if (matchesBitAccumulatorUpdate(instPtr.get(), resultPhi, powerPhi,
                                    &localFailure)) {
      if (failureReason)
        *failureReason = nullptr;
      return true;
    }
    if (localFailure)
      lastFailure = localFailure;
  }
  if (failureReason)
    *failureReason = lastFailure;
  return false;
}

bool loopContainsBitAccumulatorUpdate(const analysis::NaturalLoop& loop,
                                      core_ir::Instruction* resultPhi,
                                      core_ir::Instruction* powerPhi,
                                      const char** failureReason = nullptr) {
  const char* lastFailure = "no-loop-match";
  for (auto* bb : loop.blocks) {
    if (!bb || bb == loop.header)
      continue;
    const char* localFailure = nullptr;
    if (bodyContainsBitAccumulatorUpdate(bb, resultPhi, powerPhi,
                                         &localFailure)) {
      if (failureReason)
        *failureReason = nullptr;
      return true;
    }
    if (localFailure)
      lastFailure = localFailure;
  }
  if (failureReason)
    *failureReason = lastFailure;
  return false;
}

core_ir::BasicBlock* singleLoopSuccessor(const analysis::NaturalLoop& loop,
                                         core_ir::BasicBlock* block,
                                         bool wantInsideLoop) {
  if (!block)
    return nullptr;
  core_ir::BasicBlock* selected = nullptr;
  for (auto* succ : block->succs()) {
    if (!succ)
      continue;
    const bool inside = loop.blocks.count(succ) != 0;
    if (inside != wantInsideLoop)
      continue;
    if (selected && selected != succ)
      return nullptr;
    selected = succ;
  }
  return selected;
}

core_ir::Value* stripLoopPhiProxy(core_ir::Value* value,
                                  const analysis::NaturalLoop& loop) {
  std::unordered_set<int> visited;
  core_ir::Value* current = value;
  while (auto* phi = dynamic_cast<core_ir::Instruction*>(current)) {
    if (phi->opcode() != core_ir::Opcode::Phi || !phi->parent() ||
        !loop.blocks.count(phi->parent()) || !visited.insert(phi->id()).second) {
      break;
    }
    core_ir::Value* insideValue = nullptr;
    bool ambiguous = false;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(phi->operand(i + 1));
      if (!pred || !loop.blocks.count(pred))
        continue;
      if (!insideValue) {
        insideValue = phi->operand(i);
        continue;
      }
      if (insideValue != phi->operand(i)) {
        ambiguous = true;
        break;
      }
    }
    if (ambiguous || !insideValue || insideValue == current)
      break;
    current = insideValue;
  }
  return current;
}

void clearBlockInstructions(core_ir::Function* func, core_ir::BasicBlock* bb) {
  if (!func || !bb)
    return;
  for (auto& instPtr : bb->instructions()) {
    if (!instPtr)
      continue;
    if (instPtr->opcode() == core_ir::Opcode::Alloca)
      func->removeAlloca(instPtr.get());
    instPtr->dropAllOperands();
  }
  bb->instructions().clear();
}

bool isConstI32(core_ir::Value* value, int32_t expected);
bool isSignBitOf(core_ir::Value* value, core_ir::Value* arg);
std::string icmpCondName(core_ir::Instruction* inst);

bool isConstPow2I32(core_ir::Value* value, int32_t& pow2Out, int& shiftOut) {
  auto* ci = dynamic_cast<core_ir::ConstInt*>(value);
  if (!ci)
    return false;
  int32_t v = ci->value();
  if (v <= 1 || (v & (v - 1)) != 0)
    return false;
  pow2Out = v;
  shiftOut = log2PowerOfTwo(v);
  return true;
}

bool matchSignMaskAndConst(core_ir::Value* value,
                           core_ir::Value* input,
                           int32_t expectedMask) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::And || inst->numOperands() != 2)
    return false;
  if (isSignBitOf(inst->operand(0), input) && isConstI32(inst->operand(1), expectedMask))
    return true;
  if (isSignBitOf(inst->operand(1), input) && isConstI32(inst->operand(0), expectedMask))
    return true;
  return false;
}

bool matchAddWithPow2Bias(core_ir::Value* value,
                          core_ir::Value* input,
                          int32_t expectedMask) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::Add || inst->numOperands() != 2)
    return false;
  if (inst->operand(0) == input &&
      matchSignMaskAndConst(inst->operand(1), input, expectedMask)) {
    return true;
  }
  if (inst->operand(1) == input &&
      matchSignMaskAndConst(inst->operand(0), input, expectedMask)) {
    return true;
  }
  return false;
}

bool matchSignedPow2Div(core_ir::Value* value,
                        core_ir::Value* input,
                        int32_t& radixOut,
                        int& shiftOut) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == core_ir::Opcode::SDiv && inst->operand(0) == input) {
    if (!isConstPow2I32(inst->operand(1), radixOut, shiftOut))
      return false;
    return true;
  }
  if (inst->opcode() != core_ir::Opcode::Shr)
    return false;
  auto* shiftConst = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
  if (!shiftConst || shiftConst->value() <= 0 || shiftConst->value() >= 31)
    return false;
  const int shift = shiftConst->value();
  const int32_t radix = static_cast<int32_t>(1u << shift);
  if (!matchAddWithPow2Bias(inst->operand(0), input, radix - 1))
    return false;
  radixOut = radix;
  shiftOut = shift;
  return true;
}

bool matchSignedPow2Rem(core_ir::Value* value,
                        core_ir::Value* input,
                        int32_t& radixOut,
                        int& shiftOut) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->numOperands() != 2)
    return false;
  if (inst->opcode() == core_ir::Opcode::SRem && inst->operand(0) == input) {
    if (!isConstPow2I32(inst->operand(1), radixOut, shiftOut))
      return false;
    return true;
  }
  if (inst->opcode() != core_ir::Opcode::Sub)
    return false;

  auto* masked = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
  if (!masked || masked->opcode() != core_ir::Opcode::And ||
      masked->numOperands() != 2) {
    return false;
  }
  auto* add = dynamic_cast<core_ir::Instruction*>(masked->operand(0));
  auto* mask = dynamic_cast<core_ir::ConstInt*>(masked->operand(1));
  if ((!add || add->opcode() != core_ir::Opcode::Add || add->numOperands() != 2 ||
       !mask) &&
      (!add || add->opcode() != core_ir::Opcode::Add || add->numOperands() != 2 ||
       !(mask = dynamic_cast<core_ir::ConstInt*>(masked->operand(0))))) {
    return false;
  }
  if (mask == dynamic_cast<core_ir::ConstInt*>(masked->operand(0)))
    add = dynamic_cast<core_ir::Instruction*>(masked->operand(1));
  if (!add || !mask)
    return false;

  int32_t radix = mask->value() + 1;
  if (radix <= 1 || (radix & (radix - 1)) != 0)
    return false;
  const int shift = log2PowerOfTwo(radix);
  if (!matchAddWithPow2Bias(add, input, radix - 1))
    return false;
  if (!matchSignMaskAndConst(inst->operand(1), input, radix - 1))
    return false;
  radixOut = radix;
  shiftOut = shift;
  return true;
}

bool isIncrementByOne(core_ir::Value* value, core_ir::Instruction* phi) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::Add || inst->numOperands() != 2)
    return false;
  return (inst->operand(0) == phi && isConstI32(inst->operand(1), 1)) ||
         (inst->operand(1) == phi && isConstI32(inst->operand(0), 1));
}

core_ir::Value* buildSignedPow2RemConst(core_ir::IRBuilder& builder,
                                        core_ir::Value* input,
                                        int32_t radix,
                                        const std::string& prefix) {
  auto* sign = builder.createShr(input, builder.ci(31), prefix + ".sign");
  auto* bias = builder.createAnd(sign, builder.ci(radix - 1), prefix + ".bias");
  auto* adjusted = builder.createAdd(input, bias, prefix + ".adj");
  auto* masked = builder.createAnd(adjusted, builder.ci(radix - 1), prefix + ".masked");
  return builder.createSub(masked, bias, prefix + ".rem");
}

bool rewriteFunctionToPow2DigitHelper(core_ir::Function* func,
                                      int32_t radix,
                                      int shift) {
  if (!func)
    return false;
  core_ir::BasicBlock* entry = func->entryBlock();
  if (!entry || func->args().size() != 2 || shift <= 0 || radix <= 1)
    return false;

  std::vector<core_ir::BasicBlock*> toRemove;
  for (auto& bb : func->blocks()) {
    if (bb.get() == entry) {
      clearBlockInstructions(func, bb.get());
      continue;
    }
    clearBlockInstructions(func, bb.get());
    toRemove.push_back(bb.get());
  }
  for (auto* bb : toRemove)
    func->removeBlock(bb);

  core_ir::IRBuilder builder(*func->parent(), func);
  core_ir::Value* valueArg = func->args()[0].val;
  core_ir::Value* posArg = func->args()[1].val;
  const int maxSafePos = 31 / shift;

  core_ir::BasicBlock* negBB = func->newBlock("digit.neg");
  core_ir::BasicBlock* nonNegBB = func->newBlock("digit.nonneg");
  core_ir::BasicBlock* zeroBB = func->newBlock("digit.zero");
  core_ir::BasicBlock* fastBB = func->newBlock("digit.fast");
  core_ir::BasicBlock* fastPositiveBB = func->newBlock("digit.fast.pos");
  core_ir::BasicBlock* fastSignedBB = func->newBlock("digit.fast.signed");
  core_ir::BasicBlock* retBB = func->newBlock("digit.ret");

  builder.setInsertPoint(entry);
  auto* isNeg = builder.createICmp("lt", posArg, builder.ci(0), "digit.pos.neg");
  builder.createCondBr(isNeg, negBB, nonNegBB);

  builder.setInsertPoint(negBB);
  core_ir::Value* negDigit =
      buildSignedPow2RemConst(builder, valueArg, radix, "digit.neg");
  builder.createBr(retBB);

  builder.setInsertPoint(nonNegBB);
  auto* tooLarge = builder.createICmp("gt", posArg, builder.ci(maxSafePos),
                                      "digit.pos.large");
  builder.createCondBr(tooLarge, zeroBB, fastBB);

  builder.setInsertPoint(zeroBB);
  core_ir::Value* zeroDigit = builder.ci(0);
  builder.createBr(retBB);

  builder.setInsertPoint(fastBB);
  auto* shiftScale = builder.createMul(posArg, builder.ci(shift), "digit.shift");
  auto* valueNonNegative =
      builder.createICmp("ge", valueArg, builder.ci(0), "digit.value.nonneg");
  builder.createCondBr(valueNonNegative, fastPositiveBB, fastSignedBB);

  builder.setInsertPoint(fastPositiveBB);
  auto* positiveShifted =
      builder.createShr(valueArg, shiftScale, "digit.pos.quot");
  core_ir::Value* positiveDigit =
      builder.createAnd(positiveShifted, builder.ci(radix - 1), "digit.pos.masked");
  builder.createBr(retBB);

  builder.setInsertPoint(fastSignedBB);
  auto* oneShifted = builder.createShl(builder.ci(1), shiftScale, "digit.one");
  auto* divMask = builder.createSub(oneShifted, builder.ci(1), "digit.divmask");
  auto* sign = builder.createShr(valueArg, builder.ci(31), "digit.sign");
  auto* divBias = builder.createAnd(sign, divMask, "digit.divbias");
  auto* adjusted = builder.createAdd(valueArg, divBias, "digit.adjusted");
  auto* shifted = builder.createShr(adjusted, shiftScale, "digit.quot");
  core_ir::Value* digit = buildSignedPow2RemConst(builder, shifted, radix, "digit.fast");
  builder.createBr(retBB);

  builder.setInsertPoint(retBB);
  auto* retValue = builder.createPhi(
      core_ir::Type::i32(),
      {{negDigit, negBB},
       {zeroDigit, zeroBB},
       {positiveDigit, fastPositiveBB},
       {digit, fastSignedBB}},
      "digit.retval");
  builder.createRet(retValue);

  func->recomputeCFG();
  return true;
}

bool rewriteFunctionToBitOp(core_ir::Function* func, core_ir::Opcode bitOp) {
  if (!func)
    return false;
  core_ir::BasicBlock* entry = func->entryBlock();
  if (!entry || func->args().size() != 2)
    return false;

  std::vector<core_ir::BasicBlock*> toRemove;
  for (auto& bb : func->blocks()) {
    if (bb.get() == entry) {
      clearBlockInstructions(func, bb.get());
      continue;
    }
    clearBlockInstructions(func, bb.get());
    toRemove.push_back(bb.get());
  }
  for (auto* bb : toRemove)
    func->removeBlock(bb);

  core_ir::IRBuilder builder(*func->parent(), func);
  builder.setInsertPoint(entry);
  core_ir::Value* lhs = func->args()[0].val;
  core_ir::Value* rhs = func->args()[1].val;
  auto* lhsSign = builder.createShr(lhs, builder.ci(31), "bit.lhs.sign");
  auto* rhsSign = builder.createShr(rhs, builder.ci(31), "bit.rhs.sign");

  // 源循环使用有符号除法和余数逐位处理。负数的余数为 0 或 -1，
  // 因而不能直接套用二进制补码位运算；下面按原循环的判定逐位合并为
  // 无分支的符号掩码表达式，避免在位运算热循环中引入控制流和调用开销。
  core_ir::Value* result = nullptr;
  if (bitOp == core_ir::Opcode::And) {
    auto* negativeMask =
        builder.createOr(lhsSign, rhsSign, "bit.and.negative-mask");
    auto* allowedMask =
        builder.createXor(negativeMask, builder.ci(-1), "bit.and.allowed-mask");
    auto* nativeAnd = builder.createAnd(lhs, rhs, "bit.and.native");
    result = builder.createAnd(nativeAnd, allowedMask, "bit.and.signed");
  } else if (bitOp == core_ir::Opcode::Or) {
    auto* lhsMask =
        builder.createXor(lhsSign, builder.ci(-1), "bit.or.lhs-mask");
    auto* rhsMask =
        builder.createXor(rhsSign, builder.ci(-1), "bit.or.rhs-mask");
    auto* positiveLhs = builder.createAnd(lhs, lhsMask, "bit.or.lhs");
    auto* positiveRhs = builder.createAnd(rhs, rhsMask, "bit.or.rhs");
    result = builder.createOr(positiveLhs, positiveRhs, "bit.or.signed");
  } else {
    auto* lhsMagnitudeBits =
        builder.createXor(lhs, lhsSign, "bit.xor.lhs.flipped");
    auto* rhsMagnitudeBits =
        builder.createXor(rhs, rhsSign, "bit.xor.rhs.flipped");
    auto* lhsMagnitude =
        builder.createSub(lhsMagnitudeBits, lhsSign, "bit.xor.lhs.magnitude");
    auto* rhsMagnitude =
        builder.createSub(rhsMagnitudeBits, rhsSign, "bit.xor.rhs.magnitude");
    auto* differentSign =
        builder.createXor(lhsSign, rhsSign, "bit.xor.sign-difference");
    auto* differentBits =
        builder.createXor(lhsMagnitude, rhsMagnitude, "bit.xor.magnitudes");
    auto* sharedBits =
        builder.createAnd(lhsMagnitude, rhsMagnitude, "bit.xor.shared-bits");
    auto* mixedSignBits = builder.createAnd(
        sharedBits, differentSign, "bit.xor.mixed-sign-bits");
    result =
        builder.createOr(differentBits, mixedSignBits, "bit.xor.signed");
  }
  builder.createRet(result);
  func->recomputeCFG();
  return true;
}

bool detectAndReplacePow2DigitHelper(core_ir::Function* func,
                                     analysis::AnalysisManager& analysisManager) {
  if (!func || func->retTy() != core_ir::Type::i32() || !func->allocas().empty())
    return false;
  if (func->args().size() != 2 || !func->args()[0].type.isI32() ||
      !func->args()[1].type.isI32()) {
    return false;
  }

  func->recomputeCFG();
  analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.instructionCount() > 96)
    return false;
  auto loops = cache.loops();
  if (loops.size() != 1)
    return false;

  const auto& loop = loops[0];
  if (!loop.header || loop.latches.size() != 1)
    return false;
  core_ir::BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  core_ir::BasicBlock* exit = singleLoopSuccessor(loop, loop.header, false);
  if (!preheader || !exit)
    return false;

  auto* headerTerm = loop.header->terminator();
  auto* exitTerm = exit->terminator();
  if (!headerTerm || headerTerm->opcode() != core_ir::Opcode::CondBr ||
      headerTerm->numOperands() != 3 || !exitTerm ||
      exitTerm->opcode() != core_ir::Opcode::Ret || exitTerm->numOperands() != 1) {
    return false;
  }

  auto* loopCmp = dynamic_cast<core_ir::Instruction*>(headerTerm->operand(0));
  if (!loopCmp || loopCmp->opcode() != core_ir::Opcode::ICmp ||
      loopCmp->numOperands() != 2) {
    return false;
  }

  core_ir::Instruction* valuePhi = nullptr;
  core_ir::Instruction* posPhi = nullptr;
  int32_t radix = 0;
  int shift = 0;

  for (auto& instPtr : loop.header->instructions()) {
    auto* phi = instPtr.get();
    if (phi->opcode() != core_ir::Opcode::Phi)
      break;
    core_ir::Value* initValue = nullptr;
    core_ir::Value* backValue = nullptr;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(phi->operand(i + 1));
      if (!pred)
        continue;
      if (loop.blocks.count(pred))
        backValue = stripLoopPhiProxy(phi->operand(i), loop);
      else
        initValue = phi->operand(i);
    }
    if (!initValue || !backValue)
      continue;

    int32_t thisRadix = 0;
    int thisShift = 0;
    if (initValue == func->args()[0].val &&
        matchSignedPow2Div(backValue, phi, thisRadix, thisShift)) {
      if (valuePhi || thisShift <= 0)
        return false;
      valuePhi = phi;
      radix = thisRadix;
      shift = thisShift;
      continue;
    }
    if (isConstI32(initValue, 0) && isIncrementByOne(backValue, phi)) {
      if (posPhi)
        return false;
      posPhi = phi;
    }
  }

  if (!valuePhi || !posPhi || radix <= 1 || shift <= 0)
    return false;

  const std::string cond = icmpCondName(loopCmp);
  const bool cmpMatches =
      ((cond == "lt" || cond == "le") &&
       loopCmp->operand(0) == posPhi &&
       loopCmp->operand(1) == func->args()[1].val) ||
      ((cond == "gt" || cond == "ge") &&
       loopCmp->operand(0) == func->args()[1].val &&
       loopCmp->operand(1) == posPhi);
  if (!cmpMatches)
    return false;

  int32_t retRadix = 0;
  int retShift = 0;
  if (!matchSignedPow2Rem(exitTerm->operand(0), valuePhi, retRadix, retShift) ||
      retRadix != radix || retShift != shift) {
    return false;
  }

  traceBitPatternDecision(func, "accept-digit-helper", "pow2-digit");
  return rewriteFunctionToPow2DigitHelper(func, radix, shift);
}

AstBitOp detectBitAccumulatorHelperOp(const analysis::NaturalLoop& loop,
                                      core_ir::Value* arg0Phi,
                                      core_ir::Value* arg1Phi) {
  if (!loop.header || !arg0Phi || !arg1Phi)
    return AstBitOp::None;
  for (auto* bb : loop.blocks) {
    if (!bb || bb == loop.header)
      continue;
    for (auto& instPtr : bb->instructions()) {
      auto* inst = instPtr.get();
      if (!inst)
        continue;
      if ((inst->opcode() == core_ir::Opcode::And ||
           inst->opcode() == core_ir::Opcode::Or) &&
          inst->numOperands() == 2) {
        if ((matchesSignedOnePredicate(inst->operand(0), arg0Phi) &&
             matchesSignedOnePredicate(inst->operand(1), arg1Phi)) ||
            (matchesSignedOnePredicate(inst->operand(0), arg1Phi) &&
             matchesSignedOnePredicate(inst->operand(1), arg0Phi))) {
          return inst->opcode() == core_ir::Opcode::And ? AstBitOp::And
                                                        : AstBitOp::Or;
        }
      }
      if (inst->opcode() == core_ir::Opcode::ICmp && inst->numOperands() == 2 &&
          bitCompareCondition(inst) == "ne" &&
          ((matchesSignedLowBitValue(inst->operand(0), arg0Phi) &&
            matchesSignedLowBitValue(inst->operand(1), arg1Phi)) ||
           (matchesSignedLowBitValue(inst->operand(0), arg1Phi) &&
            matchesSignedLowBitValue(inst->operand(1), arg0Phi)))) {
        return AstBitOp::Xor;
      }
    }
  }
  return AstBitOp::None;
}

bool detectAndReplaceBitAccumulatorHelper(core_ir::Function* func,
                                          analysis::AnalysisManager& analysisManager) {
  if (!func || func->retTy() != core_ir::Type::i32())
    return false;
  if (func->args().size() != 2 || !func->args()[0].type.isI32() ||
      !func->args()[1].type.isI32()) {
    traceBitPatternDecision(func, "skip-helper", "non-i32-two-arg");
    return false;
  }
  // 替换整个函数时，外部可见结果只来自返回值。下面的结构匹配器会忽略无关
  // 纯计算，因此先排除内存和副作用指令，不让它们藏在累加使用链之外。
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      const auto op = instPtr->opcode();
      if ((!instPtr->isTerminator() && instPtr->hasSideEffects()) ||
          op == core_ir::Opcode::Load ||
          op == core_ir::Opcode::Store || op == core_ir::Opcode::Call ||
          op == core_ir::Opcode::Alloca) {
        traceBitPatternDecision(func, "skip-helper", "observable-effect");
        return false;
      }
    }
  }
  func->recomputeCFG();
  analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  analysis::FunctionAnalysisCache& cache = analysisManager.function(func);
  if (cache.instructionCount() > 160) {
    traceBitPatternDecision(func, "skip-helper", "too-large");
    return false;
  }
  // 结构前提：函数内必须恰有一个自然循环且 latch 唯一，才可能整体替换为位运算表达式。
  auto loops = cache.loops();
  if (loops.size() != 1) {
    traceBitPatternDecision(func, "skip-helper", "loop-count");
    return false;
  }
  const auto& loop = loops[0];
  if (!loop.header || loop.latches.size() != 1) {
    traceBitPatternDecision(func, "skip-helper", "loop-shape");
    return false;
  }

  // 规范结构校验：循环须具备 preheader、唯一 body/exit，latch 以无条件回边收尾，
  // exit 块直接返回；任一缺失即放弃替换。
  core_ir::BasicBlock* preheader = analysis::dedicatedPreheader(loop);
  core_ir::BasicBlock* body = singleLoopSuccessor(loop, loop.header, true);
  core_ir::BasicBlock* exit = singleLoopSuccessor(loop, loop.header, false);
  core_ir::BasicBlock* latch = loop.latches.front();
  if (!preheader || !body || !exit || !latch) {
    traceBitPatternDecision(func, "skip-helper", "missing-canonical-bbs");
    return false;
  }
  if (!loop.blocks.count(body) || !loop.blocks.count(latch)) {
    traceBitPatternDecision(func, "skip-helper", "bb-not-in-loop");
    return false;
  }
  auto* latchTerm = latch->terminator();
  if (!latchTerm || latchTerm->opcode() != core_ir::Opcode::Br ||
      latchTerm->numOperands() != 1 || latchTerm->operand(0) != loop.header) {
    traceBitPatternDecision(func, "skip-helper", "latch-term");
    return false;
  }
  auto* exitTerm = exit->terminator();
  if (!exitTerm || exitTerm->opcode() != core_ir::Opcode::Ret ||
      exitTerm->numOperands() != 1) {
    traceBitPatternDecision(func, "skip-helper", "exit-term");
    return false;
  }
  core_ir::Instruction* arg0Phi = nullptr;
  core_ir::Instruction* arg1Phi = nullptr;
  core_ir::Instruction* powerPhi = nullptr;
  core_ir::Instruction* resultPhi = nullptr;
  core_ir::Instruction* lenPhi = nullptr;
  std::vector<core_ir::Instruction*> zeroInitPhis;
  std::vector<core_ir::Instruction*> shiftedArgPhis;
  // 按“初值 + 回边更新”给 header 前置 phi 归类：Shr 右移 1 位的是参数，
  // 从 1 起 Shl 倍增的是位权，0 初值的是累加器候选，32 递减的是位数计数器。
  for (auto& instPtr : loop.header->instructions()) {
    auto* phi = instPtr.get();
    if (phi->opcode() != core_ir::Opcode::Phi)
      break;
    core_ir::Value* initValue = nullptr;
    core_ir::Value* backValue = nullptr;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(phi->operand(i + 1));
      if (!pred)
        continue;
      if (loop.blocks.count(pred))
        backValue = stripLoopPhiProxy(phi->operand(i), loop);
      else
        initValue = phi->operand(i);
    }
    auto* backInst = dynamic_cast<core_ir::Instruction*>(backValue);
    if (!backInst)
      continue;
    if (backInst->opcode() == core_ir::Opcode::Shr &&
        backInst->numOperands() == 2 && isConstInt(backInst->operand(1), 1) &&
        matchesAdjustedArg(backInst->operand(0), phi) &&
        !isConstInt(initValue, 0) && !isConstInt(initValue, 1) &&
        !isConstInt(initValue, 32)) {
      shiftedArgPhis.push_back(phi);
      continue;
    }
    if (isConstInt(initValue, 1) && backInst->opcode() == core_ir::Opcode::Shl &&
        backInst->numOperands() == 2 && backInst->operand(0) == phi &&
        isConstInt(backInst->operand(1), 1)) {
      powerPhi = phi;
      continue;
    }
    if (isConstInt(initValue, 0))
      zeroInitPhis.push_back(phi);
    if (isConstInt(initValue, 32) && backInst->opcode() == core_ir::Opcode::Sub &&
        backInst->numOperands() == 2 && backInst->operand(0) == phi &&
        isConstInt(backInst->operand(1), 1)) {
      lenPhi = phi;
      continue;
    }
  }

  if (shiftedArgPhis.size() == 2) {
    arg0Phi = shiftedArgPhis[0];
    arg1Phi = shiftedArgPhis[1];
  }

  if (powerPhi) {
    for (auto* phi : zeroInitPhis) {
      if (loopContainsBitAccumulatorUpdate(loop, phi, powerPhi)) {
        resultPhi = phi;
        break;
      }
    }
  }

  if (!arg0Phi || !arg1Phi || !powerPhi || !resultPhi || !lenPhi) {
    if (bitPatternTraceEnabled() && func) {
      std::cerr << "[bit-pattern] skip-helper " << func->name()
                << " missing-phi-role"
                << " arg0=" << (arg0Phi ? 1 : 0)
                << " arg1=" << (arg1Phi ? 1 : 0)
                << " power=" << (powerPhi ? 1 : 0)
                << " result=" << (resultPhi ? 1 : 0)
                << " len=" << (lenPhi ? 1 : 0) << "\n";
    }
    return false;
  }
  if (exitTerm->operand(0) != resultPhi) {
    traceBitPatternDecision(func, "skip-helper", "ret-not-result");
    return false;
  }

  auto phiInit = [&](core_ir::Instruction* phi) -> core_ir::Value* {
    if (!phi || phi->opcode() != core_ir::Opcode::Phi)
      return nullptr;
    core_ir::Value* init = nullptr;
    for (int i = 0; i + 1 < phi->numOperands(); i += 2) {
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(phi->operand(i + 1));
      if (!pred || loop.blocks.count(pred))
        continue;
      if (init)
        return nullptr;
      init = phi->operand(i);
    }
    return init;
  };
  core_ir::Value* arg0Init = phiInit(arg0Phi);
  core_ir::Value* arg1Init = phiInit(arg1Phi);
  core_ir::Value* input0 = func->args()[0].val;
  core_ir::Value* input1 = func->args()[1].val;
  if (!((arg0Init == input0 && arg1Init == input1) ||
        (arg0Init == input1 && arg1Init == input0)) ||
      !isConstInt(phiInit(powerPhi), 1) ||
      !isConstInt(phiInit(resultPhi), 0) ||
      !isConstInt(phiInit(lenPhi), 32)) {
    traceBitPatternDecision(func, "skip-helper", "phi-init");
    return false;
  }

  auto* preheaderTerm = preheader->terminator();
  if (!preheaderTerm || preheaderTerm->opcode() != core_ir::Opcode::Br ||
      preheaderTerm->numOperands() != 1 ||
      preheaderTerm->operand(0) != loop.header) {
    traceBitPatternDecision(func, "skip-helper", "preheader-edge");
    return false;
  }
  auto* headerTerm = loop.header->terminator();
  auto* loopCond = headerTerm && headerTerm->numOperands() == 3
                       ? dynamic_cast<core_ir::Instruction*>(headerTerm->operand(0))
                       : nullptr;
  if (!headerTerm || headerTerm->opcode() != core_ir::Opcode::CondBr ||
      !loopCond || loopCond->opcode() != core_ir::Opcode::ICmp ||
      loopCond->numOperands() != 2 ||
      !((loopCond->operand(0) == lenPhi && isConstInt(loopCond->operand(1), 0)) ||
        (loopCond->operand(1) == lenPhi && isConstInt(loopCond->operand(0), 0)))) {
    traceBitPatternDecision(func, "skip-helper", "trip-condition");
    return false;
  }
  auto* trueTarget = dynamic_cast<core_ir::BasicBlock*>(headerTerm->operand(1));
  auto* falseTarget = dynamic_cast<core_ir::BasicBlock*>(headerTerm->operand(2));
  const std::string& rawCondName = loopCond->name();
  const size_t condSep = rawCondName.find('_');
  const std::string condName = condSep == std::string::npos
                                   ? rawCondName
                                   : rawCondName.substr(0, condSep);
  if (!((condName == "ne" && trueTarget == body && falseTarget == exit) ||
        (condName == "eq" && trueTarget == exit && falseTarget == body))) {
    traceBitPatternDecision(func, "skip-helper", "trip-edge");
    return false;
  }

  size_t retCount = 0;
  for (const auto& bbPtr : func->blocks()) {
    if (bbPtr->terminator() &&
        bbPtr->terminator()->opcode() == core_ir::Opcode::Ret)
      ++retCount;
    if (bbPtr.get() != preheader && bbPtr.get() != exit &&
        !loop.blocks.count(bbPtr.get())) {
      traceBitPatternDecision(func, "skip-helper", "outside-region");
      return false;
    }
  }
  if (retCount != 1) {
    traceBitPatternDecision(func, "skip-helper", "return-count");
    return false;
  }

  AstBitOp opKind = detectBitAccumulatorHelperOp(loop, arg0Phi, arg1Phi);
  if (opKind == AstBitOp::None) {
    traceBitPatternDecision(func, "skip-helper", "op-detect");
    return false;
  }

  core_ir::Opcode bitOp = core_ir::Opcode::And;
  if (opKind == AstBitOp::Or)
    bitOp = core_ir::Opcode::Or;
  else if (opKind == AstBitOp::Xor)
    bitOp = core_ir::Opcode::Xor;
  traceBitPatternDecision(func, "accept-helper",
                          bitOp == core_ir::Opcode::And
                              ? "and"
                              : (bitOp == core_ir::Opcode::Or ? "or" : "xor"));
  return rewriteFunctionToBitOp(func, bitOp);
}

std::string icmpCondName(core_ir::Instruction* inst) {
  if (!inst || inst->opcode() != core_ir::Opcode::ICmp)
    return "";
  const std::string& name = inst->name();
  size_t pos = name.find('_');
  return pos == std::string::npos ? name : name.substr(0, pos);
}

bool isSameValuePair(core_ir::Value* lhs,
                     core_ir::Value* rhs,
                     core_ir::Value* a,
                     core_ir::Value* b) {
  return (lhs == a && rhs == b) || (lhs == b && rhs == a);
}

bool isConstI32(core_ir::Value* value, int32_t expected) {
  auto* ci = dynamic_cast<core_ir::ConstInt*>(value);
  return ci && ci->value() == expected;
}

bool isSignBitOf(core_ir::Value* value, core_ir::Value* arg) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  return inst && inst->opcode() == core_ir::Opcode::Shr &&
         inst->numOperands() == 2 && inst->operand(0) == arg &&
         isConstI32(inst->operand(1), 31);
}

bool isSignedHalfOf(core_ir::Value* value, core_ir::Value* arg) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == core_ir::Opcode::SDiv && inst->numOperands() == 2 &&
      inst->operand(0) == arg && isConstI32(inst->operand(1), 2)) {
    return true;
  }
  if (inst->opcode() != core_ir::Opcode::Shr || inst->numOperands() != 2 ||
      !isConstI32(inst->operand(1), 1)) {
    return false;
  }
  auto* adjusted = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
  if (!adjusted || adjusted->opcode() != core_ir::Opcode::Add ||
      adjusted->numOperands() != 2) {
    return false;
  }
  auto* bias = dynamic_cast<core_ir::Instruction*>(
      adjusted->operand(0) == arg ? adjusted->operand(1)
                                  : adjusted->operand(0));
  if (!bias || bias->opcode() != core_ir::Opcode::And ||
      bias->numOperands() != 2) {
    return false;
  }
  return (isSignBitOf(bias->operand(0), arg) && isConstI32(bias->operand(1), 1)) ||
         (isSignBitOf(bias->operand(1), arg) && isConstI32(bias->operand(0), 1));
}

bool sremByConst(core_ir::Value* value, int32_t& modOut) {
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst || inst->opcode() != core_ir::Opcode::SRem ||
      inst->numOperands() != 2) {
    return false;
  }
  auto* ci = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
  if (!ci || ci->value() <= 1)
    return false;
  modOut = ci->value();
  return true;
}

bool hasEqCompareWithConst(core_ir::Function* func,
                           core_ir::Value* value,
                           int32_t expected) {
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || inst->opcode() != core_ir::Opcode::ICmp ||
          icmpCondName(inst) != "eq" || inst->numOperands() != 2) {
        continue;
      }
      if ((inst->operand(0) == value && isConstI32(inst->operand(1), expected)) ||
          (inst->operand(1) == value && isConstI32(inst->operand(0), expected))) {
        return true;
      }
    }
  }
  return false;
}

bool functionAlreadyHasModMulFastPath(core_ir::Function* func) {
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (inst && inst->opcode() == core_ir::Opcode::Call &&
          inst->name() == "__syc_modmul_i32") {
        return true;
      }
    }
  }
  return false;
}

bool isOutlinedModMulFallback(const core_ir::Function* func) {
  static const std::string suffix = ".modmul.fallback";
  return func && func->name().size() >= suffix.size() &&
         func->name().compare(func->name().size() - suffix.size(),
                              suffix.size(), suffix) == 0;
}

bool matchRecursiveModMulHelperLegacy(core_ir::Function* func, int32_t& modOut) {
  if (!func || func->retTy() != core_ir::Type::i32() ||
      func->args().size() != 2 || !func->args()[0].type.isI32() ||
      !func->args()[1].type.isI32() || isOutlinedModMulFallback(func) ||
      functionAlreadyHasModMulFastPath(func)) {
    return false;
  }
  if (!func->allocas().empty())
    return false;

  // 递归基前提：必须同时存在 rhs==0 与 rhs==1 的常量等值比较，否则无法定位终止分支。
  core_ir::Value* lhsArg = func->args()[0].val;
  core_ir::Value* rhsArg = func->args()[1].val;
  if (!hasEqCompareWithConst(func, rhsArg, 0) ||
      !hasEqCompareWithConst(func, rhsArg, 1)) {
    return false;
  }

  core_ir::Instruction* selfCall = nullptr;
  int32_t mod = 0;
  bool sawBaseRem = false;
  bool sawDoubleRem = false;
  bool sawOddRem = false;

  // 全函数扫描：锁定唯一自递归调用（lhs、rhs 折半），要求所有 SRem 同模数，
  // 且覆盖直接取余、递归结果倍增、奇数分支（可写作 rem+lhs，需跨块查找）三种惯用法。
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() == core_ir::Opcode::Call &&
          inst->name() == func->name() && inst->numOperands() == 2 &&
          inst->operand(0) == lhsArg && isSignedHalfOf(inst->operand(1), rhsArg)) {
        if (selfCall)
          return false;
        selfCall = inst;
      }
      if (inst->opcode() != core_ir::Opcode::SRem || inst->numOperands() != 2)
        continue;
      int32_t thisMod = 0;
      if (!sremByConst(inst, thisMod))
        continue;
      if (mod == 0)
        mod = thisMod;
      if (mod != thisMod)
        return false;
      if (inst->operand(0) == lhsArg) {
        sawBaseRem = true;
        continue;
      }
      auto* add = dynamic_cast<core_ir::Instruction*>(inst->operand(0));
      if (!add || add->opcode() != core_ir::Opcode::Add ||
          add->numOperands() != 2) {
        continue;
      }
      if (selfCall && add->operand(0) == selfCall && add->operand(1) == selfCall) {
        sawDoubleRem = true;
        continue;
      }
      for (auto& otherBBPtr : func->blocks()) {
        for (auto& otherInstPtr : otherBBPtr->instructions()) {
          auto* rem = otherInstPtr.get();
          int32_t remMod = 0;
          if (!sremByConst(rem, remMod) || remMod != mod)
            continue;
          if (isSameValuePair(add->operand(0), add->operand(1), rem, lhsArg)) {
            sawOddRem = true;
          }
        }
      }
    }
  }

  if (!selfCall || mod <= 1 || !sawBaseRem || !sawDoubleRem || !sawOddRem)
    return false;
  modOut = mod;
  return true;
}

// 处理递归模乘惯用形式的闭式路径。旧识别器先建立数据依赖，本包装器再检查
// 这些依赖是否覆盖整个函数，而不是按固定块数或指令数匹配：
//   * 只包含纯运算和一次自递归调用，不含 Load/Store/Alloca 或其他副作用；
//   * 操作码来自算术、比较、分支、PHI 和返回指令集合；
//   * 只有一次自调用，符合折半递推；
//   * 单个返回值来自归约 PHI，函数结果不会绕过递推链。
// 这样可以处理不同块布局、额外纯算术和不同折半写法，输入范围条件仍由
// addModMulFastPath 负责。
bool matchRecursiveModMulHelper(core_ir::Function* func, int32_t& modOut) {
  int32_t mod = 0;
  if (!matchRecursiveModMulHelperLegacy(func, mod) || !func ||
      !func->allocas().empty())
    return false;

  // 只处理已纳入模式的操作码。算术规范化可能用 SDiv 或 Shr/Add/And 表示
  // 有符号减半，因此两种形式都保留，同时由原有依赖匹配器确认其含义是 rhs/2。
  static const std::unordered_set<core_ir::Opcode> allowed = {
      core_ir::Opcode::Add,   core_ir::Opcode::And,
      core_ir::Opcode::Shr,   core_ir::Opcode::SDiv,
      core_ir::Opcode::SRem,  core_ir::Opcode::ICmp,
      core_ir::Opcode::Call,  core_ir::Opcode::Phi,
      core_ir::Opcode::Br,    core_ir::Opcode::CondBr,
      core_ir::Opcode::Ret};
  core_ir::Instruction* returnPhi = nullptr;
  core_ir::Instruction* returnInst = nullptr;
  int selfCalls = 0;
  for (const auto& bbPtr : func->blocks()) {
    if (!bbPtr || !bbPtr->terminator())
      return false;
    for (const auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (!inst || ((!inst->isTerminator() && inst->hasSideEffects()) &&
                    inst->opcode() != core_ir::Opcode::Call) ||
          inst->opcode() == core_ir::Opcode::Load ||
          inst->opcode() == core_ir::Opcode::Store ||
          inst->opcode() == core_ir::Opcode::Alloca)
        return false;
      if (!allowed.count(inst->opcode()))
        return false;
      if (inst->opcode() == core_ir::Opcode::Call) {
        if (inst->name() != func->name())
          return false;
        ++selfCalls;
      } else if (inst->opcode() == core_ir::Opcode::Phi) {
        if (returnPhi)
          return false;
        returnPhi = inst;
      } else if (inst->opcode() == core_ir::Opcode::Ret) {
        if (returnInst)
          return false;
        returnInst = inst;
      }
    }
  }

  // 结构包含一次减半递归调用、一次返回，且返回值来自合流 phi。
  // 匹配过程不依赖基本块或指令数量相等。
  if (selfCalls != 1 || !returnPhi || !returnInst ||
      returnInst->numOperands() != 1 ||
      returnInst->operand(0) != returnPhi)
    return false;

  core_ir::Value* lhsArg = func->args()[0].val;
  core_ir::Value* rhsArg = func->args()[1].val;
  core_ir::Instruction* selfCall = nullptr;
  core_ir::Instruction* baseRem = nullptr;
  core_ir::Instruction* doubled = nullptr;
  core_ir::Instruction* doubleRem = nullptr;
  core_ir::Instruction* oddAdd = nullptr;
  core_ir::Instruction* oddRem = nullptr;
  core_ir::Instruction* parity = nullptr;
  // 分多遍扫描定位关键指令：先找自调用、基础取余 baseRem 与奇偶位 parity，
  // 再依次沿依赖链追踪 doubled -> doubleRem -> oddAdd -> oddRem，缺一环即判定不匹配。
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (inst->opcode() == core_ir::Opcode::Call &&
          inst->name() == func->name() && inst->numOperands() == 2 &&
          inst->operand(0) == lhsArg &&
          isSignedHalfOf(inst->operand(1), rhsArg))
        selfCall = inst;
      int32_t thisMod = 0;
      if (sremByConst(inst, thisMod) && thisMod == mod &&
          inst->operand(0) == lhsArg)
        baseRem = inst;
      if (inst->opcode() == core_ir::Opcode::And && inst->numOperands() == 2 &&
          ((inst->operand(0) == rhsArg && isConstI32(inst->operand(1), 1)) ||
           (inst->operand(1) == rhsArg && isConstI32(inst->operand(0), 1))))
        parity = inst;
    }
  }
  if (!selfCall || !baseRem || !parity)
    return false;
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (inst->opcode() == core_ir::Opcode::Add && inst->numOperands() == 2 &&
          inst->operand(0) == selfCall && inst->operand(1) == selfCall)
        doubled = inst;
    }
  }
  if (!doubled)
    return false;
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      int32_t thisMod = 0;
      if (sremByConst(inst, thisMod) && thisMod == mod &&
          inst->operand(0) == doubled)
        doubleRem = inst;
    }
  }
  if (!doubleRem)
    return false;
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      if (inst->opcode() == core_ir::Opcode::Add && inst->numOperands() == 2 &&
          isSameValuePair(inst->operand(0), inst->operand(1), doubleRem, lhsArg))
        oddAdd = inst;
    }
  }
  if (!oddAdd)
    return false;
  for (const auto& bbPtr : func->blocks()) {
    for (const auto& instPtr : bbPtr->instructions()) {
      auto* inst = instPtr.get();
      int32_t thisMod = 0;
      if (sremByConst(inst, thisMod) && thisMod == mod &&
          inst->operand(0) == oddAdd)
        oddRem = inst;
    }
  }
  if (!oddRem)
    return false;

  auto incomingPred = [&](core_ir::Value* value) -> core_ir::BasicBlock* {
    core_ir::BasicBlock* found = nullptr;
    for (int i = 0; i + 1 < returnPhi->numOperands(); i += 2) {
      if (returnPhi->operand(i) != value)
        continue;
      auto* pred = dynamic_cast<core_ir::BasicBlock*>(returnPhi->operand(i + 1));
      if (!pred || found)
        return nullptr;
      found = pred;
    }
    return found;
  };
  core_ir::Value* zero = func->parent()->getConstInt(0);
  core_ir::BasicBlock* zeroPred = incomingPred(zero);
  core_ir::BasicBlock* basePred = incomingPred(baseRem);
  core_ir::BasicBlock* evenPred = incomingPred(doubleRem);
  core_ir::BasicBlock* oddPred = incomingPred(oddRem);
  if (!zeroPred || !basePred || !evenPred || !oddPred)
    return false;
  for (int i = 0; i + 1 < returnPhi->numOperands(); i += 2) {
    core_ir::Value* value = returnPhi->operand(i);
    if (value != zero && value != baseRem && value != doubleRem && value != oddRem)
      return false;
  }

  auto condTarget = [](core_ir::Instruction* term,
                       int operand) -> core_ir::BasicBlock* {
    return term && term->opcode() == core_ir::Opcode::CondBr &&
                   term->numOperands() == 3
               ? dynamic_cast<core_ir::BasicBlock*>(term->operand(operand))
               : nullptr;
  };
  auto matchesEq = [](core_ir::Value* value, core_ir::Value* input,
                      int32_t constant) {
    auto* cmp = dynamic_cast<core_ir::Instruction*>(value);
    return cmp && cmp->opcode() == core_ir::Opcode::ICmp &&
           icmpCondName(cmp) == "eq" && cmp->numOperands() == 2 &&
           ((cmp->operand(0) == input && isConstI32(cmp->operand(1), constant)) ||
            (cmp->operand(1) == input && isConstI32(cmp->operand(0), constant)));
  };
  auto* entryTerm = func->entryBlock()->terminator();
  if (!entryTerm || !matchesEq(entryTerm->operand(0), rhsArg, 0) ||
      condTarget(entryTerm, 1) != zeroPred)
    return false;
  core_ir::BasicBlock* checkOne = condTarget(entryTerm, 2);
  auto* checkOneTerm = checkOne ? checkOne->terminator() : nullptr;
  if (!checkOneTerm || !matchesEq(checkOneTerm->operand(0), rhsArg, 1) ||
      condTarget(checkOneTerm, 1) != basePred)
    return false;
  core_ir::BasicBlock* recurrence = condTarget(checkOneTerm, 2);
  auto* recurrenceTerm = recurrence ? recurrence->terminator() : nullptr;
  auto* parityCmp = recurrenceTerm && recurrenceTerm->numOperands() == 3
                        ? dynamic_cast<core_ir::Instruction*>(recurrenceTerm->operand(0))
                        : nullptr;
  if (!parityCmp || parityCmp->opcode() != core_ir::Opcode::ICmp ||
      parityCmp->numOperands() != 2)
    return false;
  bool trueIsOdd = false;
  if (matchesSignedOnePredicate(parityCmp, rhsArg)) {
    trueIsOdd = true;
  } else if ((parityCmp->operand(0) == parity &&
              isConstI32(parityCmp->operand(1), 0)) ||
             (parityCmp->operand(1) == parity &&
              isConstI32(parityCmp->operand(0), 0))) {
    trueIsOdd = icmpCondName(parityCmp) == "ne";
    if (!trueIsOdd && icmpCondName(parityCmp) != "eq")
      return false;
  } else {
    return false;
  }
  if (condTarget(recurrenceTerm, trueIsOdd ? 1 : 2) != oddPred ||
      condTarget(recurrenceTerm, trueIsOdd ? 2 : 1) != evenPred)
    return false;

  modOut = mod;
  return true;
}

core_ir::Instruction* cloneInstructionShell(core_ir::Instruction* source) {
  auto* clone = new core_ir::Instruction(source->opcode(), source->type(),
                                         source->name());
  clone->setMemoryInfo(source->memoryElemTy(), source->memorySizeBytes(),
                       source->isArrayObject(), source->dims());
  return clone;
}

core_ir::Function* outlineModMulFallback(core_ir::Function* source) {
  if (!source || !source->parent())
    return nullptr;

  core_ir::Module& mod = *source->parent();
  const std::string fallbackName = source->name() + ".modmul.fallback";
  if (mod.getFunction(fallbackName))
    return nullptr;

  core_ir::Function* fallback = mod.newFunction(fallbackName, source->retTy());
  std::unordered_map<core_ir::Value*, core_ir::Value*> valueMap;
  for (const auto& arg : source->args()) {
    core_ir::Value* clonedArg = fallback->addArg(arg.name, arg.type);
    valueMap[arg.val] = clonedArg;
  }

  std::unordered_map<core_ir::BasicBlock*, core_ir::BasicBlock*> blockMap;
  for (const auto& blockPtr : source->blocks()) {
    core_ir::BasicBlock* clonedBlock = fallback->newBlock("body");
    blockMap[blockPtr.get()] = clonedBlock;
    if (blockPtr.get() == source->entryBlock())
      fallback->setEntryBlock(clonedBlock);
  }

  for (const auto& blockPtr : source->blocks()) {
    core_ir::BasicBlock* clonedBlock = blockMap.at(blockPtr.get());
    for (const auto& instPtr : blockPtr->instructions()) {
      core_ir::Instruction* clonedInst = cloneInstructionShell(instPtr.get());
      clonedBlock->pushBack(clonedInst);
      valueMap[instPtr.get()] = clonedInst;
      if (clonedInst->opcode() == core_ir::Opcode::Alloca)
        fallback->addAlloca(clonedInst);
    }
  }

  for (const auto& blockPtr : source->blocks()) {
    core_ir::BasicBlock* clonedBlock = blockMap.at(blockPtr.get());
    size_t instructionIndex = 0;
    for (const auto& instPtr : blockPtr->instructions()) {
      core_ir::Instruction* sourceInst = instPtr.get();
      core_ir::Instruction* clonedInst =
          clonedBlock->instructions()[instructionIndex++].get();
      for (int operandIndex = 0; operandIndex < sourceInst->numOperands();
           ++operandIndex) {
        core_ir::Value* operand = sourceInst->operand(operandIndex);
        auto mappedValue = valueMap.find(operand);
        if (mappedValue != valueMap.end()) {
          clonedInst->addOperand(mappedValue->second);
          continue;
        }
        auto* operandBlock = dynamic_cast<core_ir::BasicBlock*>(operand);
        auto mappedBlock = blockMap.find(operandBlock);
        if (mappedBlock != blockMap.end()) {
          clonedInst->addOperand(mappedBlock->second);
          continue;
        }
        clonedInst->addOperand(operand);
      }
      if (clonedInst->opcode() == core_ir::Opcode::Call &&
          sourceInst->name() == source->name()) {
        clonedInst->setName(fallbackName);
      }
    }
  }

  fallback->recomputeCFG();
  return fallback;
}

bool addModMulFastPath(core_ir::Function* func, int32_t modValue) {
  if (!func || !func->parent() || modValue <= 1 ||
      modValue > std::numeric_limits<int32_t>::max() / 2)
    return false;
  core_ir::BasicBlock* oldEntry = func->entryBlock();
  if (!oldEntry || !oldEntry->preds().empty())
    return false;

  core_ir::Function* fallback = outlineModMulFallback(func);
  if (!fallback)
    return false;

  std::vector<core_ir::BasicBlock*> oldBlocks;
  for (const auto& blockPtr : func->blocks())
    oldBlocks.push_back(blockPtr.get());
  for (core_ir::BasicBlock* block : oldBlocks)
    clearBlockInstructions(func, block);
  for (core_ir::BasicBlock* block : oldBlocks)
    func->removeBlock(block);

  core_ir::BasicBlock* entry = func->newBlock("modmul.entry");
  core_ir::BasicBlock* fast = func->newBlock("modmul.fast");
  core_ir::BasicBlock* fallbackCall = func->newBlock("modmul.slow");
  core_ir::BasicBlock* exit = func->newBlock("modmul.exit");
  func->setEntryBlock(entry);

  core_ir::IRBuilder builder(*func->parent(), func);
  core_ir::Value* lhs = func->args()[0].val;
  core_ir::Value* rhs = func->args()[1].val;
  core_ir::ConstInt* zero = func->parent()->getConstInt(0);
  core_ir::ConstInt* mod = func->parent()->getConstInt(modValue);

  builder.setInsertPoint(entry);
  auto* lhsNonNeg = builder.createICmp("ge", lhs, zero, "modmul.lhs.nonneg");
  auto* rhsNonNeg = builder.createICmp("ge", rhs, zero, "modmul.rhs.nonneg");
  auto* lhsBelow = builder.createICmp("lt", lhs, mod, "modmul.lhs.inrange");
  auto* lhsOk = builder.createAnd(lhsNonNeg, lhsBelow, "modmul.lhs.ok");
  auto* guard = builder.createAnd(lhsOk, rhsNonNeg, "modmul.guard");
  builder.createCondBr(guard, fast, fallbackCall);

  builder.setInsertPoint(fast);
  auto* product = builder.createCall("__syc_modmul_i32", core_ir::Type::i32(),
                                     {lhs, rhs, mod}, "modmul.fast");
  builder.createBr(exit);

  builder.setInsertPoint(fallbackCall);
  auto* fallbackResult = builder.createCall(
      fallback->name(), core_ir::Type::i32(), {lhs, rhs}, "modmul.slow");
  builder.createBr(exit);

  builder.setInsertPoint(exit);
  auto* result = builder.createPhi(
      core_ir::Type::i32(), {{product, fast}, {fallbackResult, fallbackCall}},
      "modmul.result");
  builder.createRet(result);

  func->recomputeCFG();
  return true;
}

bool applyRecursiveModMulFastPath(core_ir::Function* func,
                                  analysis::AnalysisManager& analysisManager) {
  // `__syc_modmul_i32` 只表示编译器物化的内部运算。若源程序定义了同名
  // 函数，则名称属于用户符号，不能据此生成内部快路径。
  if (!func || !func->parent() ||
      func->parent()->getFunction("__syc_modmul_i32"))
    return false;
  int32_t mod = 0;
  if (!matchRecursiveModMulHelper(func, mod))
    return false;
  if (!addModMulFastPath(func, mod))
    return false;
  traceBitPatternDecision(func, "accept-modmul-fastpath", "recursive-halving");
  analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  return true;
}

enum class SignedBitClosureKind { None, And, Or, Xor };

SignedBitClosureKind signedBitClosureKind(const core_ir::Instruction* inst) {
  if (!inst)
    return SignedBitClosureKind::None;
  if (inst->name().find("bit.and.signed") != std::string::npos)
    return SignedBitClosureKind::And;
  if (inst->name().find("bit.or.signed") != std::string::npos)
    return SignedBitClosureKind::Or;
  if (inst->name().find("bit.xor.signed") != std::string::npos)
    return SignedBitClosureKind::Xor;
  return SignedBitClosureKind::None;
}

bool signedBitClosureInputs(core_ir::Instruction* root,
                            SignedBitClosureKind kind, core_ir::Value*& lhs,
                            core_ir::Value*& rhs) {
  lhs = nullptr;
  rhs = nullptr;
  if (!root || root->numOperands() != 2)
    return false;
  if (kind == SignedBitClosureKind::And) {
    auto* native = dynamic_cast<core_ir::Instruction*>(root->operand(0));
    if (!native || native->numOperands() != 2)
      return false;
    lhs = native->operand(0);
    rhs = native->operand(1);
    return true;
  }
  if (kind == SignedBitClosureKind::Or) {
    auto* left = dynamic_cast<core_ir::Instruction*>(root->operand(0));
    auto* right = dynamic_cast<core_ir::Instruction*>(root->operand(1));
    if (!left || !right || left->numOperands() != 2 ||
        right->numOperands() != 2 ||
        left->name().find("bit.or.lhs") == std::string::npos ||
        right->name().find("bit.or.rhs") == std::string::npos)
      return false;
    lhs = left->operand(0);
    rhs = right->operand(0);
    return true;
  }
  if (kind == SignedBitClosureKind::Xor) {
    auto* different = dynamic_cast<core_ir::Instruction*>(root->operand(0));
    if (!different || different->numOperands() != 2 ||
        different->name().find("bit.xor.magnitudes") == std::string::npos)
      return false;
    auto* lhsMagnitude =
        dynamic_cast<core_ir::Instruction*>(different->operand(0));
    auto* rhsMagnitude =
        dynamic_cast<core_ir::Instruction*>(different->operand(1));
    if (!lhsMagnitude || !rhsMagnitude || lhsMagnitude->numOperands() != 2 ||
        rhsMagnitude->numOperands() != 2)
      return false;
    auto* left =
        dynamic_cast<core_ir::Instruction*>(lhsMagnitude->operand(0));
    auto* right =
        dynamic_cast<core_ir::Instruction*>(rhsMagnitude->operand(0));
    if (!left || !right || left->numOperands() != 2 ||
        right->numOperands() != 2 ||
        left->name().find("bit.xor.lhs.flipped") == std::string::npos ||
        right->name().find("bit.xor.rhs.flipped") == std::string::npos)
      return false;
    lhs = left->operand(0);
    rhs = right->operand(0);
    return true;
  }
  return false;
}

bool valueKnownNonNegative(
    core_ir::Value* value, std::unordered_set<int>& visited,
    std::unordered_set<core_ir::GlobalVar*>& globalsBeingProved);

bool scalarGlobalKnownNonNegative(
    core_ir::GlobalVar* global, std::unordered_set<int>& visited,
    std::unordered_set<core_ir::GlobalVar*>& globalsBeingProved) {
  if (!global || !global->dims().empty())
    return false;
  if (globalsBeingProved.count(global))
    return true;
  globalsBeingProved.insert(global);
  for (int32_t initial : global->initData) {
    if (initial < 0) {
      globalsBeingProved.erase(global);
      return false;
    }
  }
  for (core_ir::Instruction* user : global->uses()) {
    if (!user || !core_ir::Value::isLiveValue(user))
      continue;
    if (user->opcode() == core_ir::Opcode::Load && user->numOperands() == 1 &&
        user->operand(0) == global) {
      continue;
    }
    if (user->opcode() == core_ir::Opcode::Store && user->numOperands() == 2 &&
        user->operand(1) == global &&
        valueKnownNonNegative(user->operand(0), visited,
                              globalsBeingProved)) {
      continue;
    }
    globalsBeingProved.erase(global);
    return false;
  }
  globalsBeingProved.erase(global);
  return true;
}

bool valueKnownNonNegative(
    core_ir::Value* value, std::unordered_set<int>& visited,
    std::unordered_set<core_ir::GlobalVar*>& globalsBeingProved) {
  if (auto* constant = dynamic_cast<core_ir::ConstInt*>(value))
    return constant->value() >= 0;
  core_ir::KnownIntRange knownRange = core_ir::knownIntRange(value);
  if (knownRange.valid && knownRange.minValue >= 0)
    return true;
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst)
    return false;
  if (inst->opcode() == core_ir::Opcode::Phi) {
    if (!visited.insert(inst->id()).second)
      return true;
    for (int index = 0; index + 1 < inst->numOperands(); index += 2) {
      if (!valueKnownNonNegative(inst->operand(index), visited,
                                 globalsBeingProved))
        return false;
    }
    return true;
  }
  if (!visited.insert(inst->id()).second)
    return false;
  int64_t constantValue = 0;
  if (core_ir::evaluateConstIntExpression(inst, constantValue))
    return constantValue >= 0 &&
           constantValue <= std::numeric_limits<int32_t>::max();
  const SignedBitClosureKind closure = signedBitClosureKind(inst);
  if (closure == SignedBitClosureKind::And ||
      closure == SignedBitClosureKind::Or)
    return true;
  if (inst->name().find("bit.closure.nonnegative-result") !=
      std::string::npos)
    return true;
  if (inst->opcode() == core_ir::Opcode::ICmp)
    return true;
  if (inst->opcode() == core_ir::Opcode::Load && inst->numOperands() == 1) {
    if (auto* global =
            dynamic_cast<core_ir::GlobalVar*>(inst->operand(0))) {
      return scalarGlobalKnownNonNegative(global, visited,
                                          globalsBeingProved);
    }
  }
  if (inst->opcode() == core_ir::Opcode::And && inst->numOperands() == 2) {
    if (auto* constant = dynamic_cast<core_ir::ConstInt*>(inst->operand(0));
        constant && constant->value() >= 0)
      return true;
    if (auto* constant = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
        constant && constant->value() >= 0)
      return true;
  }
  if (inst->opcode() == core_ir::Opcode::Shr && inst->numOperands() == 2) {
    core_ir::Value* source = inst->operand(0);
    if (auto* adjusted = dynamic_cast<core_ir::Instruction*>(source);
        adjusted && adjusted->opcode() == core_ir::Opcode::Add &&
        adjusted->numOperands() == 2) {
      for (int sourceIndex = 0; sourceIndex < 2; ++sourceIndex) {
        core_ir::Value* candidate = adjusted->operand(sourceIndex);
        auto* bias = dynamic_cast<core_ir::Instruction*>(
            adjusted->operand(1 - sourceIndex));
        if (!bias || bias->opcode() != core_ir::Opcode::And ||
            bias->numOperands() != 2)
          continue;
        core_ir::Instruction* sign = nullptr;
        if (auto* first =
                dynamic_cast<core_ir::Instruction*>(bias->operand(0));
            first && first->opcode() == core_ir::Opcode::Shr)
          sign = first;
        if (!sign) {
          if (auto* second =
                  dynamic_cast<core_ir::Instruction*>(bias->operand(1));
              second && second->opcode() == core_ir::Opcode::Shr)
            sign = second;
        }
        if (sign && sign->numOperands() == 2 && sign->operand(0) == candidate &&
            isConstInt(sign->operand(1), 31)) {
          source = candidate;
          break;
        }
      }
    }
    return valueKnownNonNegative(source, visited, globalsBeingProved);
  }
  if (inst->opcode() == core_ir::Opcode::SDiv && inst->numOperands() == 2) {
    auto* divisor = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
    return divisor && divisor->value() > 0 &&
           valueKnownNonNegative(inst->operand(0), visited,
                                 globalsBeingProved);
  }
  if (inst->opcode() == core_ir::Opcode::Select && inst->numOperands() == 3)
    return valueKnownNonNegative(inst->operand(1), visited,
                                 globalsBeingProved) &&
           valueKnownNonNegative(inst->operand(2), visited,
                                 globalsBeingProved);
  if ((inst->opcode() == core_ir::Opcode::Add ||
       inst->opcode() == core_ir::Opcode::Sub) &&
      inst->numOperands() == 2) {
    auto* lhs = dynamic_cast<core_ir::ConstInt*>(inst->operand(0));
    auto* rhs = dynamic_cast<core_ir::ConstInt*>(inst->operand(1));
    if (lhs && rhs) {
      const int64_t result = inst->opcode() == core_ir::Opcode::Add
                                 ? static_cast<int64_t>(lhs->value()) +
                                       static_cast<int64_t>(rhs->value())
                                 : static_cast<int64_t>(lhs->value()) -
                                       static_cast<int64_t>(rhs->value());
      return result >= 0 && result <= std::numeric_limits<int32_t>::max();
    }
  }
  return false;
}

core_ir::Value* cloneSignedBitFormula(
    core_ir::Value* value, core_ir::Instruction* root,
    core_ir::BasicBlock* destination,
    std::unordered_map<core_ir::Value*, core_ir::Value*>& memo) {
  auto memoIt = memo.find(value);
  if (memoIt != memo.end())
    return memoIt->second;
  auto* inst = dynamic_cast<core_ir::Instruction*>(value);
  if (!inst)
    return value;
  const bool generatedBitNode =
      inst == root ||
      (inst->name().find("bit.") != std::string::npos &&
       signedBitClosureKind(inst) == SignedBitClosureKind::None);
  if (!generatedBitNode)
    return value;
  auto* clone =
      new core_ir::Instruction(inst->opcode(), inst->type(), inst->name());
  memo[value] = clone;
  for (int index = 0; index < inst->numOperands(); ++index) {
    clone->addOperand(cloneSignedBitFormula(inst->operand(index), root,
                                           destination, memo));
  }
  destination->pushBack(clone);
  return clone;
}

void replaceSuccessorPhiPredecessor(core_ir::BasicBlock* successor,
                                    core_ir::BasicBlock* oldPredecessor,
                                    core_ir::BasicBlock* newPredecessor) {
  if (!successor)
    return;
  for (const auto& instPtr : successor->instructions()) {
    core_ir::Instruction* phi = instPtr.get();
    if (phi->opcode() != core_ir::Opcode::Phi)
      break;
    for (int index = 1; index < phi->numOperands(); index += 2) {
      if (phi->operand(index) == oldPredecessor)
        phi->setOperand(index, newPredecessor);
    }
  }
}

bool lowerSignedBitClosure(core_ir::Instruction* root) {
  if (!root || !root->parent() || !root->parent()->parent())
    return false;
  SignedBitClosureKind kind = signedBitClosureKind(root);
  core_ir::Value* lhs = nullptr;
  core_ir::Value* rhs = nullptr;
  if (kind == SignedBitClosureKind::None ||
      !signedBitClosureInputs(root, kind, lhs, rhs))
    return false;

  std::unordered_set<int> lhsVisited;
  std::unordered_set<int> rhsVisited;
  std::unordered_set<core_ir::GlobalVar*> lhsGlobals;
  std::unordered_set<core_ir::GlobalVar*> rhsGlobals;
  const bool lhsNonNegative =
      valueKnownNonNegative(lhs, lhsVisited, lhsGlobals);
  const bool rhsNonNegative =
      valueKnownNonNegative(rhs, rhsVisited, rhsGlobals);

  if (sycEnvironmentValue("SYC_TRACE_SIGNED_BIT_FASTPATH")) {
    std::cerr << "[signed-bit-fastpath] function="
              << root->parent()->parent()->name() << " root=%" << root->id()
              << " lhs=%" << lhs->id() << " rhs=%" << rhs->id()
              << " known=" << lhsNonNegative << rhsNonNegative << "\n";
  }

  // AND 只有一侧可能为负时，原闭式只需一个符号掩码。保留这条无分支
  // 表达式比拆分菱形更短，也能继续共享已经存在的符号位计算。
  if (kind == SignedBitClosureKind::And &&
      lhsNonNegative != rhsNonNegative)
    return false;

  core_ir::BasicBlock* original = root->parent();
  core_ir::Function* function = original->parent();
  core_ir::Module* module = function->parent();
  if (lhsNonNegative && rhsNonNegative) {
    core_ir::Opcode nativeOpcode = core_ir::Opcode::Xor;
    const char* nativeName = "bit.closure.proven.xor";
    if (kind == SignedBitClosureKind::And) {
      nativeOpcode = core_ir::Opcode::And;
      nativeName = "bit.closure.proven.and";
    } else if (kind == SignedBitClosureKind::Or) {
      nativeOpcode = core_ir::Opcode::Or;
      nativeName = "bit.closure.proven.or";
    }
    auto* native =
        new core_ir::Instruction(nativeOpcode, core_ir::Type::i32(), nativeName);
    native->addOperand(lhs);
    native->addOperand(rhs);
    original->insertBefore(root, native);
    root->replaceAllUsesWith(native);
    original->remove(root);
    function->recomputeCFG();
    return true;
  }

  std::vector<core_ir::BasicBlock*> oldSuccessors = original->succs();
  core_ir::BasicBlock* slow = function->newBlock("bit.closure.slow");
  core_ir::BasicBlock* merge = function->newBlock("bit.closure.merge");
  core_ir::IRBuilder builder(*module, function);

  auto& instructions = original->instructions();
  size_t rootIndex = instructions.size();
  for (size_t index = 0; index < instructions.size(); ++index) {
    if (instructions[index].get() == root) {
      rootIndex = index;
      break;
    }
  }
  if (rootIndex == instructions.size())
    return false;
  std::vector<std::unique_ptr<core_ir::Instruction>> tail;
  for (size_t index = rootIndex + 1; index < instructions.size(); ++index)
    tail.push_back(std::move(instructions[index]));
  instructions.erase(instructions.begin() + rootIndex + 1,
                     instructions.end());

  // 快路径结果在分支前只计算一次；慢路径复用 AND 的原生结果，避免同一
  // 表达式分散到互斥块后给后端造成不必要的值合并压力。
  builder.setInsertPoint(original);
  core_ir::Value* fastResult = nullptr;
  if (kind == SignedBitClosureKind::And)
    fastResult = builder.createAnd(lhs, rhs, "bit.closure.fast.and");
  else if (kind == SignedBitClosureKind::Or)
    fastResult = builder.createOr(lhs, rhs, "bit.closure.fast.or");
  else
    fastResult = builder.createXor(lhs, rhs, "bit.closure.fast.xor");

  std::unordered_map<core_ir::Value*, core_ir::Value*> clonedValues;
  if (kind == SignedBitClosureKind::And)
    clonedValues[root->operand(0)] = fastResult;
  core_ir::Value* slowResult =
      cloneSignedBitFormula(root, root, slow, clonedValues);
  builder.setInsertPoint(slow);
  builder.createBr(merge);

  builder.setInsertPoint(merge);
  const char* resultName =
      kind == SignedBitClosureKind::Xor
          ? "bit.closure.result"
          : "bit.closure.nonnegative-result";
  auto* result = builder.createPhi(
      core_ir::Type::i32(), {{fastResult, original}, {slowResult, slow}},
      resultName);
  for (auto& instruction : tail)
    merge->pushBack(instruction.release());
  root->replaceAllUsesWith(result);
  original->remove(root);

  builder.setInsertPoint(original);
  core_ir::Value* guard = nullptr;
  if (!lhsNonNegative && !rhsNonNegative) {
    core_ir::Value* combinedSign =
        kind == SignedBitClosureKind::Or
            ? fastResult
            : builder.createOr(lhs, rhs, "bit.closure.combined-sign");
    guard = builder.createICmp("ge", combinedSign, builder.ci(0),
                               "bit.closure.inputs.nonnegative");
  } else if (!lhsNonNegative) {
    guard = builder.createICmp("ge", lhs, builder.ci(0),
                               "bit.closure.lhs.nonnegative");
  } else if (!rhsNonNegative) {
    guard = builder.createICmp("ge", rhs, builder.ci(0),
                               "bit.closure.rhs.nonnegative");
  }
  if (!guard)
    guard = builder.ci(1);
  builder.createCondBr(guard, merge, slow);

  for (core_ir::BasicBlock* successor : oldSuccessors)
    replaceSuccessorPhiPredecessor(successor, original, merge);
  function->recomputeCFG();
  return true;
}

} // namespace

bool applyBitAccumulatorRecognition(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    if (!detectAndReplaceBitAccumulatorHelper(funcPtr.get(), analysisManager))
      continue;
    changed = true;
    analysisManager.invalidateFunction(
        funcPtr.get(), analysis::AnalysisInvalidation::Effects);
  }
  return changed;
}

bool applyPow2DigitHelperRecognition(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    core_ir::Function* func = funcPtr.get();
    if (!detectAndReplacePow2DigitHelper(func, analysisManager))
      continue;
    changed = true;
    analysisManager.invalidateFunction(
        func, analysis::AnalysisInvalidation::Effects);
  }
  return changed;
}

bool applyRecursiveModMulRecognition(
    core_ir::Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  std::vector<core_ir::Function*> functions;
  for (const auto& funcPtr : mod.functions())
    functions.push_back(funcPtr.get());
  for (core_ir::Function* func : functions) {
    if (!applyRecursiveModMulFastPath(func, analysisManager))
      continue;
    changed = true;
    analysisManager.invalidateFunction(
        func, analysis::AnalysisInvalidation::Effects);
  }
  return changed;
}

bool lowerSignedBitClosureFastPaths(core_ir::Module& mod) {
  std::vector<core_ir::Instruction*> roots;
  for (const auto& function : mod.functions()) {
    for (const auto& block : function->blocks()) {
      for (const auto& instruction : block->instructions()) {
        if (signedBitClosureKind(instruction.get()) !=
            SignedBitClosureKind::None) {
          roots.push_back(instruction.get());
        }
      }
    }
  }
  bool changed = false;
  for (core_ir::Instruction* root : roots)
    changed |= lowerSignedBitClosure(root);
  if (changed && sycEnvironmentValue("SYC_TRACE_SIGNED_BIT_FASTPATH"))
    std::cerr << "[signed-bit-fastpath] count=" << roots.size() << "\n";
  return changed;
}

bool applyBitPatternRecognition(core_ir::Module& mod) {
  analysis::AnalysisManager analysisManager(mod);
  return applyBitPatternRecognition(mod, analysisManager);
}

bool applyBitPatternRecognition(core_ir::Module& mod,
                                analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  std::vector<core_ir::Function*> functions;
  for (const auto& funcPtr : mod.functions())
    functions.push_back(funcPtr.get());
  for (core_ir::Function* func : functions) {
    bool funcChanged = false;
    if (!sycEnvironmentValue("SYC_SKIP_COREIR_BIT_ACCUMULATOR"))
      funcChanged |= detectAndReplacePow2DigitHelper(func, analysisManager);
    if (!sycEnvironmentValue("SYC_SKIP_COREIR_BIT_ACCUMULATOR"))
      funcChanged |= detectAndReplaceBitAccumulatorHelper(func, analysisManager);
    if (!sycEnvironmentValue("SYC_SKIP_COREIR_RECURSIVE_MODMUL"))
      funcChanged |= applyRecursiveModMulFastPath(func, analysisManager);
    if (funcChanged) {
      changed = true;
      // 位运算循环替换可能删除 helper 调用、重建循环体并改变
      // 该函数的 effect 摘要，因此要同时失效函数结构分析和模块级
      // effect/recursive 摘要。
      analysisManager.invalidateFunction(
          func, analysis::AnalysisInvalidation::Effects);
    }
  }
  return changed;
}

bool applyBitPatternRecognition(CompUnit& cu) {
  bool changed = false;
  changed |= applyPowerOfTwoDigitHelperRewrite(cu, nullptr);
  for (auto& item : cu.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func)
      continue;
    AstBitOp opKind = AstBitOp::None;
    if (!matchBitLoopFunction(item.func.get(), opKind))
      continue;
    rewriteBitLoopFunction(item.func.get(), opKind);
    changed = true;
  }
  // 辅助函数体规范化为纯表达式后，也把这种形式传播到调用点。
  // 排除已经改写的辅助函数，避免最热的调用者继续承担完整调用开销。
  changed |= applyExprHelperSynthesis(cu, {});
  return changed;
}

} // namespace rewrite
