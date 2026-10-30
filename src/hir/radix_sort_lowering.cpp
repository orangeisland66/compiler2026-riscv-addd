#include "research_switch.h"
// 文件职责：识别递归基数排序结构并将其降低为后续中间表示可处理的形式。
#include "radix_sort_lowering.h"

#include "../frontend/ast.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hir {
namespace {

constexpr int kTargetRadix = 256;
constexpr const char* kScratch = "__syc_radix_scratch";
constexpr const char* kMaximum = "__syc_radix_maximum";
constexpr const char* kIndex = "__syc_radix_index";
constexpr const char* kPassCount = "__syc_radix_pass_count";
constexpr const char* kSourceBits = "__syc_radix_source_bits";
constexpr const char* kPass = "__syc_radix_pass";
constexpr const char* kBucket = "__syc_radix_bucket";
constexpr const char* kMask = "__syc_radix_mask";
constexpr const char* kRemainingBits = "__syc_radix_remaining_bits";
constexpr const char* kValue = "__syc_radix_value";
constexpr const char* kCount = "__syc_radix_count";
constexpr const char* kOffset = "__syc_radix_offset";

struct RadixGeneratedNames {
  std::string scratch;
  std::string maximum;
  std::string index;
  std::string passCount;
  std::string sourceBits;
  std::string pass;
  std::string bucket;
  std::string mask;
  std::string remainingBits;
  std::string value;
  std::string count;
  std::string offset;
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

RadixGeneratedNames chooseGeneratedNames(const CompUnit& compUnit,
                                         const FuncDef& func) {
  std::unordered_set<std::string> topLevelNames;
  for (const auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::GlobalDecl && item.decl) {
      for (const auto& def : item.decl->defs) {
        if (def)
          topLevelNames.insert(def->name);
      }
    } else if (item.kind == CompUnit::Item::Kind::FuncDef && item.func) {
      topLevelNames.insert(item.func->name);
    }
  }
  // scratch 在函数体内按名称引用，因此也不能与形参重名。
  for (const auto& param : func.params) {
    if (param)
      topLevelNames.insert(param->name);
  }

  RadixGeneratedNames names;
  names.scratch = takeFreshName(kScratch, topLevelNames);

  std::unordered_set<std::string> localNames;
  for (const auto& param : func.params) {
    if (param)
      localNames.insert(param->name);
  }
  localNames.insert(names.scratch);
  names.maximum = takeFreshName(kMaximum, localNames);
  names.index = takeFreshName(kIndex, localNames);
  names.passCount = takeFreshName(kPassCount, localNames);
  names.sourceBits = takeFreshName(kSourceBits, localNames);
  names.pass = takeFreshName(kPass, localNames);
  names.bucket = takeFreshName(kBucket, localNames);
  names.mask = takeFreshName(kMask, localNames);
  names.remainingBits = takeFreshName(kRemainingBits, localNames);
  names.value = takeFreshName(kValue, localNames);
  names.count = takeFreshName(kCount, localNames);
  names.offset = takeFreshName(kOffset, localNames);
  return names;
}

ExprPtr makeVar(const std::string& name, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->name = name;
  expr->line = line;
  return expr;
}

ExprPtr makeArray(const std::string& name, ExprPtr index, int line) {
  auto expr = std::make_unique<LValExpr>();
  expr->name = name;
  expr->line = line;
  expr->indices.push_back(std::move(index));
  return expr;
}

ExprPtr makeInt(int32_t value, int line) {
  auto expr = std::make_unique<IntLitExpr>();
  expr->val = value;
  expr->line = line;
  return expr;
}

ExprPtr makeBinary(const std::string& op, ExprPtr lhs, ExprPtr rhs, int line) {
  auto expr = std::make_unique<BinaryExpr>();
  expr->op = op;
  expr->lhs = std::move(lhs);
  expr->rhs = std::move(rhs);
  expr->line = line;
  return expr;
}

StmtPtr makeAssign(ExprPtr lhs, ExprPtr rhs, int line) {
  auto stmt = std::make_unique<AssignStmt>();
  stmt->lval = std::move(lhs);
  stmt->rhs = std::move(rhs);
  stmt->line = line;
  return stmt;
}

StmtPtr makeAssignScalar(const std::string& name, ExprPtr rhs, int line) {
  return makeAssign(makeVar(name, line), std::move(rhs), line);
}

StmtPtr makeBlock(std::vector<BlockStmt::Item> items, int line) {
  auto block = std::make_unique<BlockStmt>();
  block->items = std::move(items);
  block->line = line;
  return block;
}

StmtPtr makeWhile(ExprPtr cond, StmtPtr body, int line) {
  auto stmt = std::make_unique<WhileStmt>();
  stmt->cond = std::move(cond);
  stmt->body = std::move(body);
  stmt->line = line;
  return stmt;
}

StmtPtr makeIf(ExprPtr cond, StmtPtr thenBranch, int line) {
  auto stmt = std::make_unique<IfStmt>();
  stmt->cond = std::move(cond);
  stmt->thenBranch = std::move(thenBranch);
  stmt->line = line;
  return stmt;
}

StmtPtr makeReturn(int line) {
  auto stmt = std::make_unique<ReturnStmt>();
  stmt->line = line;
  return stmt;
}

BlockStmt::Item stmtItem(StmtPtr stmt) {
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Stmt;
  item.stmt = std::move(stmt);
  return item;
}

BlockStmt::Item scalarDeclItem(const std::string& name, ExprPtr init,
                               int line) {
  auto decl = std::make_unique<DeclStmt>();
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  decl->line = line;
  auto def = std::make_unique<VarDef>();
  def->name = name;
  def->isConst = false;
  def->type.base = BaseType::Int;
  def->line = line;
  if (init) {
    auto scalar = std::make_unique<ScalarInit>();
    scalar->expr = std::move(init);
    scalar->line = line;
    def->initVal = std::move(scalar);
  }
  decl->defs.push_back(std::move(def));
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Decl;
  item.decl = std::move(decl);
  return item;
}

BlockStmt::Item localArraysDeclItem(const RadixGeneratedNames& names,
                                    int line) {
  auto decl = std::make_unique<DeclStmt>();
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  decl->line = line;
  for (const std::string* name : {&names.count, &names.offset}) {
    auto def = std::make_unique<VarDef>();
    def->name = *name;
    def->isConst = false;
    def->type.base = BaseType::Int;
    def->type.dims.push_back(kTargetRadix);
    def->line = line;
    decl->defs.push_back(std::move(def));
  }
  BlockStmt::Item item;
  item.kind = BlockStmt::Item::Kind::Decl;
  item.decl = std::move(decl);
  return item;
}

CompUnit::Item scratchDeclItem(const std::string& scratchName,
                               int capacity,
                               int line) {
  auto decl = std::make_unique<DeclStmt>();
  decl->isConst = false;
  decl->baseType = BaseType::Int;
  decl->line = line;
  auto def = std::make_unique<VarDef>();
  def->name = scratchName;
  def->isConst = false;
  def->type.base = BaseType::Int;
  def->type.dims.push_back(capacity);
  def->line = line;
  decl->defs.push_back(std::move(def));
  CompUnit::Item item;
  item.kind = CompUnit::Item::Kind::GlobalDecl;
  item.decl = std::move(decl);
  return item;
}

bool isScalar(const Expr* expr, const std::string& name) {
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.empty();
}

bool isScalarInt(const FuncParam* param) {
  return param && param->type.base == BaseType::Int && !param->type.isArray();
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
  // 旧函数体会被递归或辅助调用替换，因此不能接受求值时会产生可观察效果的参数。
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
  if (const auto* loop = dynamic_cast<const WhileStmt*>(stmt))
    return expressionMentionsName(loop->cond.get(), name) ||
           statementMentionsName(loop->body.get(), name);
  if (const auto* ret = dynamic_cast<const ReturnStmt*>(stmt))
    return expressionMentionsName(ret->val.get(), name);
  return false;
}

bool functionMentionsParam(const FuncDef& func, const std::string& name) {
  return func.body && statementMentionsName(func.body.get(), name);
}

bool isNoOpItem(const BlockStmt::Item& item) {
  if (item.kind == BlockStmt::Item::Kind::Decl) {
    if (!item.decl)
      return false;
    for (const auto& def : item.decl->defs) {
      if (!def || def->type.isArray() || def->initVal)
        return false;
    }
    return true;
  }
  const auto* expr = dynamic_cast<const ExprStmt*>(item.stmt.get());
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

int powerOfTwoShift(int value) {
  if (value < 2 || value > kTargetRadix || (value & (value - 1)) != 0)
    return 0;
  int shift = 0;
  while (value > 1) {
    value >>= 1;
    ++shift;
  }
  return shift;
}

bool looksLikeRecursiveRadixSkeleton(const FuncDef& func, int& radix) {
  if (!func.body || !func.retType.isVoid() || func.params.size() < 4 ||
      !isScalarInt(func.params[0].get()) || !func.params[1] ||
      func.params[1]->type.base != BaseType::Int ||
      func.params[1]->type.dims.size() != 1 ||
      !isScalarInt(func.params[2].get()) ||
      !isScalarInt(func.params[3].get())) {
    return false;
  }
  for (size_t index = 4; index < func.params.size(); ++index) {
    const FuncParam* param = func.params[index].get();
    if (!isScalarInt(param) || functionMentionsParam(func, param->name))
      return false;
  }
  int radixArrays = 0;
  radix = 0;
  for (const auto& item : func.body->items) {
    if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl)
      continue;
    for (const auto& def : item.decl->defs) {
      if (def && def->type.base == BaseType::Int &&
          def->type.dims.size() == 1 &&
          powerOfTwoShift(def->type.dims[0]) != 0) {
        if (radix == 0)
          radix = def->type.dims[0];
        if (def->type.dims[0] != radix)
          return false;
        ++radixArrays;
      }
    }
  }
  return radixArrays == 3;
}

bool resolveInt(const Expr* expr,
                const std::unordered_map<std::string, int>& constants,
                int& value) {
  if (auto* literal = dynamic_cast<const IntLitExpr*>(expr)) {
    value = literal->val;
    return true;
  }
  if (const auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    int operand = 0;
    if ((unary->op == '+' || unary->op == '-') &&
        resolveInt(unary->operand.get(), constants, operand)) {
      value = unary->op == '-' ? -operand : operand;
      return true;
    }
    return false;
  }
  auto* lval = dynamic_cast<const LValExpr*>(expr);
  if (!lval || !lval->indices.empty())
    return false;
  auto it = constants.find(lval->name);
  if (it == constants.end())
    return false;
  value = it->second;
  return true;
}

bool scalarInitValue(const BlockStmt::Item& item, std::string& name,
                     int& value,
                     const std::unordered_map<std::string, int>& constants) {
  if (item.kind != BlockStmt::Item::Kind::Decl || !item.decl ||
      item.decl->defs.size() != 1 || !item.decl->defs[0] ||
      item.decl->defs[0]->type.isArray()) {
    return false;
  }
  auto* init = item.decl->defs[0]->initVal
                   ? dynamic_cast<const ScalarInit*>(
                         item.decl->defs[0]->initVal.get())
                   : nullptr;
  if (!init || !resolveInt(init->expr.get(), constants, value))
    return false;
  name = item.decl->defs[0]->name;
  return true;
}

bool matchAssignBinary(const Stmt* stmt, const std::string& destination,
                       const std::string& op, const std::string& lhsName,
                       const Expr*& rhsOperand) {
  auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  auto* lhs = assign ? dynamic_cast<const LValExpr*>(assign->lval.get())
                     : nullptr;
  auto* binary = assign ? dynamic_cast<const BinaryExpr*>(assign->rhs.get())
                        : nullptr;
  if (!lhs || !lhs->indices.empty() || lhs->name != destination || !binary ||
      binary->op != op || !isScalar(binary->lhs.get(), lhsName)) {
    return false;
  }
  rhsOperand = binary->rhs.get();
  return true;
}

bool isIntegerValue(const Expr* expr, int expected,
                    const std::unordered_map<std::string, int>& constants) {
  int value = 0;
  return resolveInt(expr, constants, value) && value == expected;
}

bool matchBinaryExpr(const Expr* expr, const std::string& op,
                     const std::function<bool(const Expr*)>& lhs,
                     const std::function<bool(const Expr*)>& rhs) {
  const auto* binary = dynamic_cast<const BinaryExpr*>(expr);
  return binary && binary->op == op && lhs(binary->lhs.get()) &&
         rhs(binary->rhs.get());
}

bool matchArrayExpr(const Expr* expr, const std::string& name,
                    const std::function<bool(const Expr*)>& index) {
  const auto* lval = dynamic_cast<const LValExpr*>(expr);
  return lval && lval->name == name && lval->indices.size() == 1 &&
         index(lval->indices[0].get());
}

bool matchScalarAssign(const Stmt* stmt, const std::string& name,
                       const std::function<bool(const Expr*)>& rhs) {
  const auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  return assign && isScalar(assign->lval.get(), name) && rhs(assign->rhs.get());
}

bool matchArrayAssign(const Stmt* stmt, const std::string& name,
                      const std::function<bool(const Expr*)>& index,
                      const std::function<bool(const Expr*)>& rhs) {
  const auto* assign = dynamic_cast<const AssignStmt*>(stmt);
  return assign && matchArrayExpr(assign->lval.get(), name, index) &&
         rhs(assign->rhs.get());
}

bool matchIncrement(const Stmt* stmt, const std::string& name,
                    const std::unordered_map<std::string, int>& constants) {
  return matchScalarAssign(stmt, name, [&](const Expr* expr) {
    return matchBinaryExpr(
        expr, "+", [&](const Expr* lhs) { return isScalar(lhs, name); },
        [&](const Expr* rhs) { return isIntegerValue(rhs, 1, constants); });
  });
}

bool matchDigitCall(const Expr* expr, const std::string& helper,
                    const std::function<bool(const Expr*)>& value,
                    const std::string& position) {
  const auto* call = dynamic_cast<const CallExpr*>(expr);
  if (!call || call->name != helper || call->args.size() < 2 ||
      !value(call->args[0].get()) ||
      !isScalar(call->args[1].get(), position)) {
    return false;
  }
  for (size_t index = 2; index < call->args.size(); ++index) {
    if (!isSideEffectFreeExpr(call->args[index].get()))
      return false;
  }
  return true;
}

bool matchReturnOnly(const Stmt* stmt) {
  if (const auto* ret = dynamic_cast<const ReturnStmt*>(stmt))
    return !ret->val;
  const auto* block = dynamic_cast<const BlockStmt*>(stmt);
  const auto items = meaningfulItems(block);
  return items.size() == 1 && items[0]->kind == BlockStmt::Item::Kind::Stmt &&
         matchReturnOnly(items[0]->stmt.get());
}

void flattenRadixItems(const BlockStmt* block,
                       std::vector<const BlockStmt::Item*>& result) {
  for (const BlockStmt::Item* item : meaningfulItems(block)) {
    const auto* nested =
        item->kind == BlockStmt::Item::Kind::Stmt
            ? dynamic_cast<const BlockStmt*>(item->stmt.get())
            : nullptr;
    if (nested)
      flattenRadixItems(nested, result);
    else
      result.push_back(item);
  }
}

bool matchZeroInitializer(const InitVal* init,
                          const std::unordered_map<std::string, int>& constants) {
  const auto* aggregate = dynamic_cast<const AggregateInit*>(init);
  if (!aggregate)
    return false;
  for (const auto& item : aggregate->items) {
    if (const auto* scalar = dynamic_cast<const ScalarInit*>(item.get())) {
      if (!isIntegerValue(scalar->expr.get(), 0, constants))
        return false;
    } else if (!matchZeroInitializer(item.get(), constants)) {
      return false;
    }
  }
  return true;
}

bool matchSingleScalarDeclaration(
    const BlockStmt::Item* item, std::string& name,
    const std::function<bool(const Expr*)>& init) {
  if (!item || item->kind != BlockStmt::Item::Kind::Decl || !item->decl ||
      item->decl->defs.size() != 1 || !item->decl->defs[0] ||
      item->decl->defs[0]->type.isArray()) {
    return false;
  }
  const auto* scalar = item->decl->defs[0]->initVal
                           ? dynamic_cast<const ScalarInit*>(
                                 item->decl->defs[0]->initVal.get())
                           : nullptr;
  if (!scalar || !init(scalar->expr.get()))
    return false;
  name = item->decl->defs[0]->name;
  return true;
}

// 完整识别原始递归基数排序的五个阶段。整个函数随后会被替换，因此这里
// 不只统计数组和调用数量，还逐项确认边界判断、计数、分桶置换、递归区间
// 以及所有数组写回；函数体中出现任何额外语句都会使匹配失败。
bool matchCanonicalRadixBody(
    const FuncDef& func, const std::string& helper, int radix,
    const std::unordered_map<std::string, int>& constants) {
  std::vector<const BlockStmt::Item*> items;
  flattenRadixItems(func.body.get(), items);
  size_t cursor = 0;
  std::vector<std::string> arrays;
  while (cursor < items.size() &&
         items[cursor]->kind == BlockStmt::Item::Kind::Decl) {
    const DeclStmt* decl = items[cursor]->decl.get();
    bool consumedArray = false;
    if (decl) {
      for (const auto& def : decl->defs) {
        if (!def || def->type.base != BaseType::Int ||
            def->type.dims.size() != 1 || def->type.dims[0] != radix ||
            !matchZeroInitializer(def->initVal.get(), constants)) {
          consumedArray = false;
          break;
        }
        arrays.push_back(def->name);
        consumedArray = true;
      }
    }
    if (!consumedArray)
      break;
    ++cursor;
  }
  if (arrays.size() != 3 || cursor >= items.size())
    return false;

  const std::string& position = func.params[0]->name;
  const std::string& source = func.params[1]->name;
  const std::string& begin = func.params[2]->name;
  const std::string& end = func.params[3]->name;
  const auto integer = [&](int value) {
    return [&, value](const Expr* expr) {
      return isIntegerValue(expr, value, constants);
    };
  };

  const auto* baseIf = items[cursor]->kind == BlockStmt::Item::Kind::Stmt
                           ? dynamic_cast<const IfStmt*>(items[cursor]->stmt.get())
                           : nullptr;
  const bool baseCondition = baseIf && !baseIf->elseBranch &&
      matchBinaryExpr(
          baseIf->cond.get(), "||",
          [&](const Expr* lhs) {
            return matchBinaryExpr(
                lhs, "==", [&](const Expr* value) { return isScalar(value, position); },
                integer(-1));
          },
          [&](const Expr* rhs) {
            return matchBinaryExpr(
                rhs, ">=",
                [&](const Expr* lhs) {
                  return matchBinaryExpr(
                      lhs, "+", [&](const Expr* value) { return isScalar(value, begin); },
                      integer(1));
                },
                [&](const Expr* value) { return isScalar(value, end); });
          }) &&
      matchReturnOnly(baseIf->thenBranch.get());
  if (!baseCondition)
    return false;
  ++cursor;

  std::string indexName;
  if (cursor >= items.size() ||
      !matchSingleScalarDeclaration(
          items[cursor], indexName,
          [&](const Expr* expr) { return isScalar(expr, begin); })) {
    return false;
  }
  ++cursor;

  for (size_t headIndex = 0; headIndex < arrays.size(); ++headIndex) {
    for (size_t tailIndex = 0; tailIndex < arrays.size(); ++tailIndex) {
      for (size_t countIndex = 0; countIndex < arrays.size(); ++countIndex) {
        if (headIndex == tailIndex || headIndex == countIndex ||
            tailIndex == countIndex)
          continue;
        const std::string& head = arrays[headIndex];
        const std::string& tail = arrays[tailIndex];
        const std::string& count = arrays[countIndex];
        size_t at = cursor;
        const auto sameIndex = [&](const Expr* expr) {
          return isScalar(expr, indexName);
        };
        const auto digitAtSource = [&](const Expr* expr) {
          return matchDigitCall(
              expr, helper,
              [&](const Expr* value) {
                return matchArrayExpr(value, source, sameIndex);
              },
              position);
        };

        const auto* countLoop =
            at < items.size() && items[at]->kind == BlockStmt::Item::Kind::Stmt
                ? dynamic_cast<const WhileStmt*>(items[at]->stmt.get())
                : nullptr;
        const auto countItems = meaningfulItems(
            countLoop ? dynamic_cast<const BlockStmt*>(countLoop->body.get())
                      : nullptr);
        if (!countLoop ||
            !matchBinaryExpr(
                countLoop->cond.get(), "<", sameIndex,
                [&](const Expr* expr) { return isScalar(expr, end); }) ||
            countItems.size() != 2 ||
            !matchArrayAssign(
                countItems[0]->stmt.get(), count, digitAtSource,
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+",
                      [&](const Expr* lhs) {
                        return matchArrayExpr(lhs, count, digitAtSource);
                      },
                      integer(1));
                }) ||
            !matchIncrement(countItems[1]->stmt.get(), indexName, constants)) {
          continue;
        }
        ++at;
        if (at + 4 >= items.size() ||
            !matchArrayAssign(items[at++]->stmt.get(), head, integer(0),
                              [&](const Expr* rhs) { return isScalar(rhs, begin); }) ||
            !matchArrayAssign(
                items[at++]->stmt.get(), tail, integer(0),
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+", [&](const Expr* lhs) { return isScalar(lhs, begin); },
                      [&](const Expr* value) {
                        return matchArrayExpr(value, count, integer(0));
                      });
                }) ||
            !matchScalarAssign(items[at++]->stmt.get(), indexName, integer(1))) {
          continue;
        }

        const auto* prefixLoop =
            dynamic_cast<const WhileStmt*>(items[at++]->stmt.get());
        const auto prefixItems = meaningfulItems(
            prefixLoop ? dynamic_cast<const BlockStmt*>(prefixLoop->body.get())
                       : nullptr);
        const auto previousIndex = [&](const Expr* expr) {
          return matchBinaryExpr(expr, "-", sameIndex, integer(1));
        };
        if (!prefixLoop ||
            !matchBinaryExpr(prefixLoop->cond.get(), "<", sameIndex,
                             integer(radix)) ||
            prefixItems.size() != 3 ||
            !matchArrayAssign(
                prefixItems[0]->stmt.get(), head, sameIndex,
                [&](const Expr* rhs) {
                  return matchArrayExpr(rhs, tail, previousIndex);
                }) ||
            !matchArrayAssign(
                prefixItems[1]->stmt.get(), tail, sameIndex,
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+",
                      [&](const Expr* lhs) {
                        return matchArrayExpr(lhs, head, sameIndex);
                      },
                      [&](const Expr* value) {
                        return matchArrayExpr(value, count, sameIndex);
                      });
                }) ||
            !matchIncrement(prefixItems[2]->stmt.get(), indexName, constants) ||
            at >= items.size() ||
            !matchScalarAssign(items[at++]->stmt.get(), indexName, integer(0))) {
          continue;
        }

        const auto* bucketLoop =
            at < items.size() ? dynamic_cast<const WhileStmt*>(items[at++]->stmt.get())
                              : nullptr;
        const auto bucketItems = meaningfulItems(
            bucketLoop ? dynamic_cast<const BlockStmt*>(bucketLoop->body.get())
                       : nullptr);
        if (!bucketLoop ||
            !matchBinaryExpr(bucketLoop->cond.get(), "<", sameIndex,
                             integer(radix)) ||
            bucketItems.size() != 2 ||
            !matchIncrement(bucketItems[1]->stmt.get(), indexName, constants)) {
          continue;
        }
        const auto* moveLoop =
            dynamic_cast<const WhileStmt*>(bucketItems[0]->stmt.get());
        const auto moveItems = meaningfulItems(
            moveLoop ? dynamic_cast<const BlockStmt*>(moveLoop->body.get())
                     : nullptr);
        if (!moveLoop ||
            !matchBinaryExpr(
                moveLoop->cond.get(), "<",
                [&](const Expr* lhs) { return matchArrayExpr(lhs, head, sameIndex); },
                [&](const Expr* rhs) { return matchArrayExpr(rhs, tail, sameIndex); }) ||
            moveItems.size() != 4) {
          continue;
        }
        std::string valueName;
        if (!matchSingleScalarDeclaration(
                moveItems[0], valueName,
                [&](const Expr* expr) {
                  return matchArrayExpr(
                      expr, source,
                      [&](const Expr* inner) {
                        return matchArrayExpr(inner, head, sameIndex);
                      });
                })) {
          continue;
        }
        const auto* cycleLoop =
            dynamic_cast<const WhileStmt*>(moveItems[1]->stmt.get());
        const auto cycleItems = meaningfulItems(
            cycleLoop ? dynamic_cast<const BlockStmt*>(cycleLoop->body.get())
                      : nullptr);
        const auto digitOf = [&](const std::string& name) {
          return [&](const Expr* expr) {
            return matchDigitCall(
                expr, helper,
                [&](const Expr* value) { return isScalar(value, name); }, position);
          };
        };
        if (!cycleLoop ||
            !matchBinaryExpr(
                cycleLoop->cond.get(), "!=", digitOf(valueName), sameIndex) ||
            cycleItems.size() != 4) {
          continue;
        }
        std::string temporaryName;
        if (!matchSingleScalarDeclaration(
                cycleItems[0], temporaryName,
                [&](const Expr* expr) { return isScalar(expr, valueName); }) ||
            !matchScalarAssign(
                cycleItems[1]->stmt.get(), valueName,
                [&](const Expr* rhs) {
                  return matchArrayExpr(
                      rhs, source,
                      [&](const Expr* inner) {
                        return matchArrayExpr(inner, head, digitOf(temporaryName));
                      });
                }) ||
            !matchArrayAssign(
                cycleItems[2]->stmt.get(), source,
                [&](const Expr* lhsIndex) {
                  return matchArrayExpr(lhsIndex, head, digitOf(temporaryName));
                },
                [&](const Expr* rhs) { return isScalar(rhs, temporaryName); }) ||
            !matchArrayAssign(
                cycleItems[3]->stmt.get(), head, digitOf(temporaryName),
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+",
                      [&](const Expr* lhs) {
                        return matchArrayExpr(lhs, head, digitOf(temporaryName));
                      },
                      integer(1));
                }) ||
            !matchArrayAssign(
                moveItems[2]->stmt.get(), source,
                [&](const Expr* lhsIndex) {
                  return matchArrayExpr(lhsIndex, head, sameIndex);
                },
                [&](const Expr* rhs) { return isScalar(rhs, valueName); }) ||
            !matchArrayAssign(
                moveItems[3]->stmt.get(), head, sameIndex,
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+",
                      [&](const Expr* lhs) {
                        return matchArrayExpr(lhs, head, sameIndex);
                      },
                      integer(1));
                })) {
          continue;
        }

        if (at < items.size() && items[at]->kind == BlockStmt::Item::Kind::Decl) {
          std::string repeatedIndex;
          if (!matchSingleScalarDeclaration(
                  items[at], repeatedIndex,
                  [&](const Expr* expr) { return isScalar(expr, begin); }) ||
              repeatedIndex != indexName) {
            continue;
          }
          ++at;
        }
        bool resetBeforeBounds = false;
        if (at < items.size() && items[at]->kind == BlockStmt::Item::Kind::Stmt &&
            matchScalarAssign(items[at]->stmt.get(), indexName, integer(0))) {
          resetBeforeBounds = true;
          ++at;
        }
        if (at + 2 >= items.size() ||
            !matchArrayAssign(items[at++]->stmt.get(), head, integer(0),
                              [&](const Expr* rhs) { return isScalar(rhs, begin); }) ||
            !matchArrayAssign(
                items[at++]->stmt.get(), tail, integer(0),
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+", [&](const Expr* lhs) { return isScalar(lhs, begin); },
                      [&](const Expr* value) {
                        return matchArrayExpr(value, count, integer(0));
                      });
                }) ||
            (!resetBeforeBounds &&
             (at >= items.size() ||
              !matchScalarAssign(items[at++]->stmt.get(), indexName,
                                 integer(0))))) {
          continue;
        }
        const auto* recursionLoop =
            dynamic_cast<const WhileStmt*>(items[at++]->stmt.get());
        const auto recursionItems = meaningfulItems(
            recursionLoop
                ? dynamic_cast<const BlockStmt*>(recursionLoop->body.get())
                : nullptr);
        if (!recursionLoop ||
            !matchBinaryExpr(recursionLoop->cond.get(), "<", sameIndex,
                             integer(radix)) ||
            recursionItems.size() != 3) {
          continue;
        }
        const auto* adjust =
            dynamic_cast<const IfStmt*>(recursionItems[0]->stmt.get());
        const auto adjustItems = meaningfulItems(
            adjust ? dynamic_cast<const BlockStmt*>(adjust->thenBranch.get())
                   : nullptr);
        if (!adjust || adjust->elseBranch ||
            !matchBinaryExpr(adjust->cond.get(), ">", sameIndex, integer(0)) ||
            adjustItems.size() != 2 ||
            !matchArrayAssign(
                adjustItems[0]->stmt.get(), head, sameIndex,
                [&](const Expr* rhs) {
                  return matchArrayExpr(rhs, tail, previousIndex);
                }) ||
            !matchArrayAssign(
                adjustItems[1]->stmt.get(), tail, sameIndex,
                [&](const Expr* rhs) {
                  return matchBinaryExpr(
                      rhs, "+",
                      [&](const Expr* lhs) {
                        return matchArrayExpr(lhs, head, sameIndex);
                      },
                      [&](const Expr* value) {
                        return matchArrayExpr(value, count, sameIndex);
                      });
                })) {
          continue;
        }
        const auto* recursiveStmt =
            dynamic_cast<const ExprStmt*>(recursionItems[1]->stmt.get());
        const auto* recursiveCall =
            recursiveStmt
                ? dynamic_cast<const CallExpr*>(recursiveStmt->expr.get())
                : nullptr;
        if (!recursiveCall || recursiveCall->name != func.name ||
            recursiveCall->args.size() != func.params.size() ||
            !matchBinaryExpr(
                recursiveCall->args[0].get(), "-",
                [&](const Expr* lhs) { return isScalar(lhs, position); }, integer(1)) ||
            !isScalar(recursiveCall->args[1].get(), source) ||
            !matchArrayExpr(recursiveCall->args[2].get(), head, sameIndex) ||
            !matchArrayExpr(recursiveCall->args[3].get(), tail, sameIndex) ||
            !matchIncrement(recursionItems[2]->stmt.get(), indexName, constants)) {
          continue;
        }
        bool extraArgumentsSafe = true;
        for (size_t argument = 4; argument < recursiveCall->args.size(); ++argument)
          extraArgumentsSafe &= isSideEffectFreeExpr(recursiveCall->args[argument].get());
        if (!extraArgumentsSafe)
          continue;
        if (at < items.size() && items[at]->kind == BlockStmt::Item::Kind::Stmt &&
            matchReturnOnly(items[at]->stmt.get()))
          ++at;
        if (at == items.size())
          return true;
      }
    }
  }
  return false;
}

// 匹配“取数位”辅助函数的形状：函数体为归纳变量声明 + 单个 while 循环 +
// return；循环体恰为“按基数整除”与“归纳变量加一”两条语句，
// 返回值是对基数取余。多余参数必须完全未被使用。
bool matchDigitHelper(const FuncDef& helper, int expectedRadix,
                      const std::unordered_map<std::string, int>& constants) {
  if (!helper.body || helper.params.size() < 2 ||
      !isScalarInt(helper.params[0].get()) ||
      !isScalarInt(helper.params[1].get()) || !helper.retType.isInt() ||
      helper.retType.isArray()) {
    return false;
  }
  for (size_t index = 2; index < helper.params.size(); ++index) {
    const FuncParam* param = helper.params[index].get();
    if (!isScalarInt(param) || functionMentionsParam(helper, param->name))
      return false;
  }
  std::string induction;
  const WhileStmt* loop = nullptr;
  const ReturnStmt* ret = nullptr;
  for (const BlockStmt::Item* item : meaningfulItems(helper.body.get())) {
    if (item->kind == BlockStmt::Item::Kind::Decl) {
      int initial = 0;
      std::string candidate;
      if (!scalarInitValue(*item, candidate, initial, constants)) {
        return false;
      }
      if (initial == 0 && induction.empty())
        induction = candidate;
      continue;
    }
    if (item->kind != BlockStmt::Item::Kind::Stmt || !item->stmt)
      return false;
    if (auto* candidate = dynamic_cast<const WhileStmt*>(item->stmt.get())) {
      if (loop)
        return false;
      loop = candidate;
    } else if (auto* candidate =
                   dynamic_cast<const ReturnStmt*>(item->stmt.get())) {
      if (ret)
        return false;
      ret = candidate;
    } else {
      return false;
    }
  }
  auto* condition = loop ? dynamic_cast<const BinaryExpr*>(loop->cond.get())
                         : nullptr;
  auto* body = loop ? dynamic_cast<const BlockStmt*>(loop->body.get()) : nullptr;
  const std::vector<const BlockStmt::Item*> loopItems =
      meaningfulItems(body);
  if (induction.empty() || !condition || condition->op != "<" ||
      !isScalar(condition->lhs.get(), induction) ||
      !isScalar(condition->rhs.get(), helper.params[1]->name) || !body ||
      loopItems.size() != 2 || !ret || !ret->val) {
    return false;
  }
  bool dividesValue = false;
  bool incrementsInduction = false;
  for (const BlockStmt::Item* item : loopItems) {
    if (item->kind != BlockStmt::Item::Kind::Stmt || !item->stmt)
      return false;
    const Expr* rhs = nullptr;
    int constant = 0;
    if (matchAssignBinary(item->stmt.get(), helper.params[0]->name, "/",
                          helper.params[0]->name, rhs) &&
        resolveInt(rhs, constants, constant) && constant == expectedRadix) {
      if (dividesValue)
        return false;
      dividesValue = true;
      continue;
    }
    if (matchAssignBinary(item->stmt.get(), induction, "+", induction, rhs) &&
        resolveInt(rhs, constants, constant) && constant == 1) {
      if (incrementsInduction)
        return false;
      incrementsInduction = true;
      continue;
    }
    return false;
  }
  auto* remainder = dynamic_cast<const BinaryExpr*>(ret->val.get());
  int returnRadix = 0;
  return dividesValue && incrementsInduction && remainder &&
         remainder->op == "%" &&
         isScalar(remainder->lhs.get(), helper.params[0]->name) &&
         resolveInt(remainder->rhs.get(), constants, returnRadix) &&
         returnRadix == expectedRadix;
}

void collectCallsExpr(const Expr* expr, std::vector<const CallExpr*>& calls) {
  if (!expr)
    return;
  if (auto* call = dynamic_cast<const CallExpr*>(expr)) {
    calls.push_back(call);
    for (const auto& arg : call->args)
      collectCallsExpr(arg.get(), calls);
    return;
  }
  if (auto* lval = dynamic_cast<const LValExpr*>(expr)) {
    for (const auto& index : lval->indices)
      collectCallsExpr(index.get(), calls);
  } else if (auto* binary = dynamic_cast<const BinaryExpr*>(expr)) {
    collectCallsExpr(binary->lhs.get(), calls);
    collectCallsExpr(binary->rhs.get(), calls);
  } else if (auto* unary = dynamic_cast<const UnaryExpr*>(expr)) {
    collectCallsExpr(unary->operand.get(), calls);
  } else if (auto* cast = dynamic_cast<const CastExpr*>(expr)) {
    collectCallsExpr(cast->operand.get(), calls);
  }
}

void collectCallsStmt(const Stmt* stmt, std::vector<const CallExpr*>& calls) {
  if (!stmt)
    return;
  if (auto* block = dynamic_cast<const BlockStmt*>(stmt)) {
    for (const auto& item : block->items) {
      if (item.kind == BlockStmt::Item::Kind::Stmt)
        collectCallsStmt(item.stmt.get(), calls);
      else if (item.decl) {
        for (const auto& def : item.decl->defs) {
          auto* scalar = def && def->initVal
                             ? dynamic_cast<const ScalarInit*>(def->initVal.get())
                             : nullptr;
          if (scalar)
            collectCallsExpr(scalar->expr.get(), calls);
        }
      }
    }
  } else if (auto* assign = dynamic_cast<const AssignStmt*>(stmt)) {
    collectCallsExpr(assign->lval.get(), calls);
    collectCallsExpr(assign->rhs.get(), calls);
  } else if (auto* exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
    collectCallsExpr(exprStmt->expr.get(), calls);
  } else if (auto* ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
    collectCallsExpr(ifStmt->cond.get(), calls);
    collectCallsStmt(ifStmt->thenBranch.get(), calls);
    collectCallsStmt(ifStmt->elseBranch.get(), calls);
  } else if (auto* loop = dynamic_cast<const WhileStmt*>(stmt)) {
    collectCallsExpr(loop->cond.get(), calls);
    collectCallsStmt(loop->body.get(), calls);
  } else if (auto* ret = dynamic_cast<const ReturnStmt*>(stmt)) {
    collectCallsExpr(ret->val.get(), calls);
  }
}

bool matchRecursiveRadixFunction(
    const FuncDef& func,
    const std::unordered_map<std::string, FuncDef*>& functions,
    const std::unordered_map<std::string, int>& constants,
    int& sourceShift) {
  int sourceRadix = 0;
  if (!looksLikeRecursiveRadixSkeleton(func, sourceRadix)) {
    return false;
  }
  sourceShift = powerOfTwoShift(sourceRadix);

  std::vector<const CallExpr*> calls;
  collectCallsStmt(func.body.get(), calls);
  int recursiveCalls = 0;
  std::unordered_map<std::string, int> digitCalls;
  for (const CallExpr* call : calls) {
    if (call->name == func.name) {
      if (call->args.size() != func.params.size())
        return false;
      for (const auto& arg : call->args) {
        if (!isSideEffectFreeExpr(arg.get()))
          return false;
      }
      ++recursiveCalls;
      continue;
    }
    auto helperIt = functions.find(call->name);
    if (helperIt == functions.end() || !helperIt->second ||
        call->args.size() < 2 ||
        call->args.size() != helperIt->second->params.size() ||
        !isScalar(call->args[1].get(), func.params[0]->name)) {
      return false;
    }
    for (const auto& arg : call->args) {
      if (!isSideEffectFreeExpr(arg.get()))
        return false;
    }
    ++digitCalls[call->name];
  }
  if (recursiveCalls != 1)
    return false;
  for (const auto& [name, count] : digitCalls) {
    const FuncDef* helper = functions.at(name);
    if (count >= 2 && helper &&
        matchDigitHelper(*helper, sourceRadix, constants) &&
        matchCanonicalRadixBody(func, name, sourceRadix, constants)) {
      return true;
    }
  }
  return false;
}

int globalArrayCapacity(const CompUnit& compUnit, const std::string& name) {
  for (const auto& item : compUnit.items) {
    if (item.kind != CompUnit::Item::Kind::GlobalDecl || !item.decl)
      continue;
    for (const auto& def : item.decl->defs) {
      if (def && def->name == name && def->type.base == BaseType::Int &&
          def->type.dims.size() == 1 && def->type.dims[0] > 0) {
        return def->type.dims[0];
      }
    }
  }
  return 0;
}

bool findExternalCallCapacity(const CompUnit& compUnit, const FuncDef& radix,
                              int& capacity) {
  int externalCalls = 0;
  capacity = 0;
  for (const auto& item : compUnit.items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func ||
        item.func.get() == &radix) {
      continue;
    }
    std::vector<const CallExpr*> calls;
    collectCallsStmt(item.func->body.get(), calls);
    for (const CallExpr* call : calls) {
      if (call->name != radix.name)
        continue;
      ++externalCalls;
      if (call->args.size() != radix.params.size())
        return false;
      auto* array = dynamic_cast<const LValExpr*>(call->args[1].get());
      if (!array || !array->indices.empty())
        return false;
      int callCapacity = globalArrayCapacity(compUnit, array->name);
      if (callCapacity <= 0)
        return false;
      capacity = std::max(capacity, callCapacity);
    }
  }
  return externalCalls > 0 && capacity > 0;
}

StmtPtr increment(const std::string& name, int amount, int line) {
  return makeAssignScalar(
      name, makeBinary("+", makeVar(name, line), makeInt(amount, line), line),
      line);
}

std::unique_ptr<BlockStmt> buildIterativeBody(const FuncDef& func,
                                              int sourceShift,
                                              const RadixGeneratedNames& names) {
  const int line = func.line;
  const std::string& bitround = func.params[0]->name;
  const std::string& source = func.params[1]->name;
  const std::string& left = func.params[2]->name;
  const std::string& right = func.params[3]->name;
  // 使用局部别名复用构造代码；这些名称均经过形参和全局作用域去重。
  const std::string& kScratch = names.scratch;
  const std::string& kMaximum = names.maximum;
  const std::string& kIndex = names.index;
  const std::string& kPassCount = names.passCount;
  const std::string& kSourceBits = names.sourceBits;
  const std::string& kPass = names.pass;
  const std::string& kBucket = names.bucket;
  const std::string& kMask = names.mask;
  const std::string& kRemainingBits = names.remainingBits;
  const std::string& kValue = names.value;
  const std::string& kCount = names.count;
  const std::string& kOffset = names.offset;
  std::vector<BlockStmt::Item> body;

  body.push_back(stmtItem(makeIf(
      makeBinary(
          "||", makeBinary("<", makeVar(bitround, line), makeInt(0, line), line),
          makeBinary(">=",
                     makeBinary("+", makeVar(left, line), makeInt(1, line), line),
                     makeVar(right, line), line),
          line),
      makeReturn(line), line)));
  body.push_back(scalarDeclItem(kMaximum, makeInt(0, line), line));
  body.push_back(scalarDeclItem(kIndex, makeVar(left, line), line));

  std::vector<BlockStmt::Item> scan;
  scan.push_back(stmtItem(makeIf(
      makeBinary(">", makeArray(source, makeVar(kIndex, line), line),
                 makeVar(kMaximum, line), line),
      makeAssignScalar(kMaximum,
                       makeArray(source, makeVar(kIndex, line), line), line),
      line)));
  scan.push_back(stmtItem(increment(kIndex, 1, line)));
  body.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kIndex, line), makeVar(right, line), line),
      makeBlock(std::move(scan), line), line)));

  body.push_back(scalarDeclItem(kPassCount, makeInt(1, line), line));
  std::vector<BlockStmt::Item> countPasses;
  countPasses.push_back(stmtItem(makeAssignScalar(
      kMaximum,
      makeBinary("/", makeVar(kMaximum, line), makeInt(kTargetRadix, line), line),
      line)));
  countPasses.push_back(stmtItem(increment(kPassCount, 1, line)));
  body.push_back(stmtItem(makeWhile(
      makeBinary(">=", makeVar(kMaximum, line), makeInt(kTargetRadix, line), line),
      makeBlock(std::move(countPasses), line), line)));

  body.push_back(scalarDeclItem(
      kSourceBits,
      makeBinary("*",
                 makeBinary("+", makeVar(bitround, line), makeInt(1, line), line),
                 makeInt(sourceShift, line), line),
      line));
  body.push_back(stmtItem(makeIf(
      makeBinary(">", makeVar(kSourceBits, line), makeInt(32, line), line),
      makeAssignScalar(kSourceBits, makeInt(32, line), line), line)));
  body.push_back(stmtItem(makeIf(
      makeBinary(">",
                 makeBinary("*", makeVar(kPassCount, line), makeInt(8, line), line),
                 makeVar(kSourceBits, line), line),
      makeAssignScalar(
          kPassCount,
          makeBinary("/",
                     makeBinary("+", makeVar(kSourceBits, line), makeInt(7, line), line),
                     makeInt(8, line), line),
          line),
      line)));

  body.push_back(localArraysDeclItem(names, line));
  body.push_back(scalarDeclItem(kPass, makeInt(0, line), line));
  body.push_back(scalarDeclItem(kBucket, makeInt(0, line), line));
  body.push_back(scalarDeclItem(kMask, makeInt(255, line), line));
  body.push_back(scalarDeclItem(kRemainingBits, makeInt(8, line), line));
  body.push_back(scalarDeclItem(kValue, makeInt(0, line), line));

  std::vector<BlockStmt::Item> outer;
  outer.push_back(stmtItem(makeAssignScalar(kBucket, makeInt(0, line), line)));
  std::vector<BlockStmt::Item> clear;
  clear.push_back(stmtItem(makeAssign(
      makeArray(kCount, makeVar(kBucket, line), line), makeInt(0, line), line)));
  clear.push_back(stmtItem(increment(kBucket, 1, line)));
  outer.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kBucket, line), makeInt(kTargetRadix, line), line),
      makeBlock(std::move(clear), line), line)));

  outer.push_back(stmtItem(makeAssignScalar(kMask, makeInt(255, line), line)));
  std::vector<BlockStmt::Item> partialMask;
  partialMask.push_back(stmtItem(makeAssignScalar(
      kRemainingBits,
      makeBinary("-", makeVar(kSourceBits, line),
                 makeBinary("*", makeVar(kPass, line), makeInt(8, line), line),
                 line),
      line)));
  partialMask.push_back(stmtItem(makeAssignScalar(
      kMask,
      makeBinary("-",
                 makeBinary("<<", makeInt(1, line),
                            makeVar(kRemainingBits, line), line),
                 makeInt(1, line), line),
      line)));
  outer.push_back(stmtItem(makeIf(
      makeBinary("<", makeVar(kSourceBits, line),
                 makeBinary("*",
                            makeBinary("+", makeVar(kPass, line), makeInt(1, line), line),
                            makeInt(8, line), line),
                 line),
      makeBlock(std::move(partialMask), line), line)));

  outer.push_back(stmtItem(makeAssignScalar(kIndex, makeVar(left, line), line)));
  std::vector<BlockStmt::Item> histogram;
  histogram.push_back(stmtItem(makeAssignScalar(
      kBucket,
      makeBinary(
          "&",
          makeBinary(">>", makeArray(source, makeVar(kIndex, line), line),
                     makeBinary("*", makeVar(kPass, line), makeInt(8, line), line), line),
          makeVar(kMask, line), line),
      line)));
  histogram.push_back(stmtItem(makeAssign(
      makeArray(kCount, makeVar(kBucket, line), line),
      makeBinary("+", makeArray(kCount, makeVar(kBucket, line), line),
                 makeInt(1, line), line),
      line)));
  histogram.push_back(stmtItem(increment(kIndex, 1, line)));
  outer.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kIndex, line), makeVar(right, line), line),
      makeBlock(std::move(histogram), line), line)));

  outer.push_back(stmtItem(makeAssign(
      makeArray(kOffset, makeInt(0, line), line), makeVar(left, line), line)));
  outer.push_back(stmtItem(makeAssignScalar(kBucket, makeInt(1, line), line)));
  std::vector<BlockStmt::Item> prefix;
  prefix.push_back(stmtItem(makeAssign(
      makeArray(kOffset, makeVar(kBucket, line), line),
      makeBinary(
          "+",
          makeArray(kOffset,
                    makeBinary("-", makeVar(kBucket, line), makeInt(1, line), line),
                    line),
          makeArray(kCount,
                    makeBinary("-", makeVar(kBucket, line), makeInt(1, line), line),
                    line),
          line),
      line)));
  prefix.push_back(stmtItem(increment(kBucket, 1, line)));
  outer.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kBucket, line), makeInt(kTargetRadix, line), line),
      makeBlock(std::move(prefix), line), line)));

  outer.push_back(stmtItem(makeAssignScalar(kIndex, makeVar(left, line), line)));
  std::vector<BlockStmt::Item> scatter;
  scatter.push_back(stmtItem(makeAssignScalar(
      kValue, makeArray(source, makeVar(kIndex, line), line), line)));
  scatter.push_back(stmtItem(makeAssignScalar(
      kBucket,
      makeBinary(
          "&",
          makeBinary(">>", makeVar(kValue, line),
                     makeBinary("*", makeVar(kPass, line), makeInt(8, line), line), line),
          makeVar(kMask, line), line),
      line)));
  scatter.push_back(stmtItem(makeAssign(
      makeArray(kScratch, makeArray(kOffset, makeVar(kBucket, line), line), line),
      makeVar(kValue, line), line)));
  scatter.push_back(stmtItem(makeAssign(
      makeArray(kOffset, makeVar(kBucket, line), line),
      makeBinary("+", makeArray(kOffset, makeVar(kBucket, line), line),
                 makeInt(1, line), line),
      line)));
  scatter.push_back(stmtItem(increment(kIndex, 1, line)));
  outer.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kIndex, line), makeVar(right, line), line),
      makeBlock(std::move(scatter), line), line)));

  outer.push_back(stmtItem(makeAssignScalar(kIndex, makeVar(left, line), line)));
  std::vector<BlockStmt::Item> copy;
  copy.push_back(stmtItem(makeAssign(
      makeArray(source, makeVar(kIndex, line), line),
      makeArray(kScratch, makeVar(kIndex, line), line), line)));
  copy.push_back(stmtItem(increment(kIndex, 1, line)));
  outer.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kIndex, line), makeVar(right, line), line),
      makeBlock(std::move(copy), line), line)));
  outer.push_back(stmtItem(increment(kPass, 1, line)));
  body.push_back(stmtItem(makeWhile(
      makeBinary("<", makeVar(kPass, line), makeVar(kPassCount, line), line),
      makeBlock(std::move(outer), line), line)));
  body.push_back(stmtItem(makeReturn(line)));

  auto result = std::make_unique<BlockStmt>();
  result->line = line;
  result->items = std::move(body);
  return result;
}

} // namespace

bool lowerRecursiveRadixSort(CompUnit& compUnit) {
  if (sycEnvironmentValue("SYC_SKIP_AST_RADIX_SORT_LOWERING")) {
    return false;
  }

  bool hasCandidateSkeleton = false;
  for (const auto& item : compUnit.items) {
    int ignoredRadix = 0;
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func &&
        looksLikeRecursiveRadixSkeleton(*item.func, ignoredRadix)) {
      hasCandidateSkeleton = true;
      break;
    }
  }
  if (!hasCandidateSkeleton)
    return false;

  std::unordered_map<std::string, FuncDef*> functions;
  std::unordered_map<std::string, int> constants;
  for (auto& item : compUnit.items) {
    if (item.kind == CompUnit::Item::Kind::FuncDef && item.func)
      functions[item.func->name] = item.func.get();
    if (item.kind != CompUnit::Item::Kind::GlobalDecl || !item.decl ||
        !item.decl->isConst)
      continue;
    for (const auto& def : item.decl->defs) {
      auto* init = def && def->initVal
                       ? dynamic_cast<const ScalarInit*>(def->initVal.get())
                       : nullptr;
      int value = 0;
      if (def && init && !def->type.isArray() &&
          resolveInt(init->expr.get(), constants, value)) {
        constants[def->name] = value;
      }
    }
  }

  for (size_t index = 0; index < compUnit.items.size(); ++index) {
    auto& item = compUnit.items[index];
    int sourceShift = 0;
    if (item.kind != CompUnit::Item::Kind::FuncDef || !item.func ||
        !matchRecursiveRadixFunction(*item.func, functions, constants,
                                     sourceShift)) {
      continue;
    }
    int capacity = 0;
    if (!findExternalCallCapacity(compUnit, *item.func, capacity))
      continue;
    const std::string functionName = item.func->name;
    const size_t argumentCount = item.func->params.size();
    const RadixGeneratedNames names =
        chooseGeneratedNames(compUnit, *item.func);
    item.func->body = buildIterativeBody(*item.func, sourceShift, names);
    compUnit.items.insert(compUnit.items.begin() + index,
                          scratchDeclItem(names.scratch, capacity,
                                          item.func->line));
    if (sycEnvironmentValue("SYC_TRACE_AST_RADIX_SORT_LOWERING")) {
      std::cerr << "[ast-radix] function=" << functionName
                << " capacity=" << capacity
                << " args=" << argumentCount << "\n";
    }
    return true;
  }
  return false;
}

} // namespace hir
