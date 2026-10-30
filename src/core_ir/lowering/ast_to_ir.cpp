// 文件职责：把 SysY AST 降低为 CoreIR，并建立控制流和内存模型。本文件负责实现具体逻辑。
#include "ast_to_ir.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <unordered_set>

namespace core_ir {

static std::string cmpCondFromOp(const std::string& op) {
  if (op == "==") return "eq";
  if (op == "!=") return "ne";
  if (op == "<") return "lt";
  if (op == "<=") return "le";
  if (op == ">") return "gt";
  if (op == ">=") return "ge";
  return op;
}

static float floatFromWord(int32_t word) {
  float val = 0.0f;
  std::memcpy(&val, &word, sizeof(val));
  return val;
}

ASTToIR::ASTToIR() {}

// ============================================================
// 入口处理。
// ============================================================
std::unique_ptr<Module> ASTToIR::lower(CompUnit* cu) {
  mod_ = std::make_unique<Module>();
  funcRetTypes_.clear();
  funcParamTypes_.clear();

  std::unordered_set<std::string> userFunctions;
  for (auto& item : cu->items)
    if (item.kind == CompUnit::Item::Kind::FuncDef)
      userFunctions.insert(item.func->name);

  auto declareRuntime = [&](const std::string& name, Type retTy,
                            const std::vector<Type>& paramTys) {
    if (!userFunctions.count(name))
      mod_->declareBuiltin(name, retTy, paramTys);
  };

  // 用户定义优先；没有同名定义时才声明运行时函数。
  declareRuntime("getint", Type::i32(), {});
  declareRuntime("getch", Type::i32(), {});
  declareRuntime("getarray", Type::i32(), {Type::ptr()});
  declareRuntime("putint", Type::voidTy(), {Type::i32()});
  declareRuntime("putch", Type::voidTy(), {Type::i32()});
  declareRuntime("putarray", Type::voidTy(), {Type::i32(), Type::ptr()});
  declareRuntime("getfloat", Type::f32(), {});
  declareRuntime("getfarray", Type::i32(), {Type::ptr()});
  declareRuntime("putfloat", Type::voidTy(), {Type::f32()});
  declareRuntime("putfarray", Type::voidTy(), {Type::i32(), Type::ptr()});
  declareRuntime("getstr", Type::voidTy(), {Type::ptr(), Type::i32()});
  declareRuntime("putstr", Type::voidTy(), {Type::ptr()});
  // 内部 helper 只由前端优化 pass 在语义证明后生成。它不是
  // 公开 SysY 库函数，后端会直接降为 RV64 宽乘取模，避免保留运行时符号。
  mod_->declareBuiltin("__syc_modmul_i32", Type::i32(),
                       {Type::i32(), Type::i32(), Type::i32()});

  // 收集函数签名。
  for (auto& item : cu->items) {
    if (item.kind != CompUnit::Item::Kind::FuncDef) continue;
    funcRetTypes_[item.func->name] = item.func->retType;
    std::vector<std::pair<BaseType, bool>> params;
    for (auto& p : item.func->params)
      params.push_back({p->type.base, p->type.isArray()});
    funcParamTypes_[item.func->name] = std::move(params);
  }

  // 先处理全局声明。
  for (auto& item : cu->items)
    if (item.kind == CompUnit::Item::Kind::GlobalDecl)
      lowerGlobalDecl(item.decl.get());

  // 再处理函数定义。
  for (auto& item : cu->items)
    if (item.kind == CompUnit::Item::Kind::FuncDef)
      lowerFunc(item.func.get());

  return std::move(mod_);
}

// ============================================================
// 类型辅助函数。
// ============================================================
Type ASTToIR::toIRType(BaseType bt, bool isArray) const {
  if (isArray) return Type::ptr();
  switch (bt) {
  case BaseType::Int:   return Type::i32();
  case BaseType::Char:  return Type::i32();
  case BaseType::Float: return Type::f32();
  case BaseType::Void:  return Type::voidTy();
  }
  return Type::i32();
}

Value* ASTToIR::ensureType(Value* val, Type target) {
  if (val->type() == target) return val;
  if (val->type().isI32() && target.isF32())
    return builder_->createSIToFP(val);
  if (val->type().isF32() && target.isI32())
    return builder_->createFPToSI(val);
  return val;
}

Value* ASTToIR::normalizeCond(Value* val) {
  if (val->type().isI32())
    return builder_->createICmp("ne", val, builder_->ci(0), "tobool");
  if (val->type().isF32())
    return builder_->createFCmp("ne", val, builder_->cf(0.0f), "tobool");
  return val;
}

bool ASTToIR::isSpeculativelySafeBoolExpr(Expr* expr) {
  if (!expr)
    return false;
  if (dynamic_cast<IntLitExpr*>(expr) || dynamic_cast<FloatLitExpr*>(expr))
    return true;
  if (auto* lv = dynamic_cast<LValExpr*>(expr)) {
    VarInfo* info = lookupVar(lv->name);
    if (!info)
      return false;
    // 只有纯标量/数组读这种无副作用表达式才允许 eager bool
    // lowering。函数调用、短路里嵌套写内存等情况仍保持原始 short-circuit CFG。
    return true;
  }
  if (auto* cast = dynamic_cast<CastExpr*>(expr))
    return isSpeculativelySafeBoolExpr(cast->operand.get());
  if (auto* unary = dynamic_cast<UnaryExpr*>(expr))
    return isSpeculativelySafeBoolExpr(unary->operand.get());
  if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
    return isSpeculativelySafeBoolExpr(bin->lhs.get()) &&
           isSpeculativelySafeBoolExpr(bin->rhs.get());
  }
  return false;
}

Value* ASTToIR::lowerBoolExprEager(Expr* expr) {
  if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
    if (bin->op == "&&" || bin->op == "||") {
      Value* lhs = normalizeCond(lowerBoolExprEager(bin->lhs.get()));
      Value* rhs = normalizeCond(lowerBoolExprEager(bin->rhs.get()));
      if (bin->op == "&&")
        return builder_->createAnd(lhs, rhs);
      return builder_->createOr(lhs, rhs);
    }
  }
  return normalizeCond(lowerExpr(expr));
}

int ASTToIR::typeSize(Type ty) const {
  return ty.isPtr() ? 8 : 4;
}

int ASTToIR::totalElems(const std::vector<int>& dims) const {
  int n = 1;
  for (int d : dims)
    if (d > 0) n *= d;
  return n;
}

// ============================================================
// 作用域管理。
// ============================================================
void ASTToIR::pushScope() { scopes_.push_back({}); }
void ASTToIR::popScope() { scopes_.pop_back(); }

ASTToIR::VarInfo* ASTToIR::lookupVar(const std::string& name) {
  for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; --i) {
    auto it = scopes_[i].find(name);
    if (it != scopes_[i].end()) return &it->second;
  }
  auto it = globals_.find(name);
  if (it != globals_.end()) return &it->second;
  return nullptr;
}

void ASTToIR::addVar(const std::string& name, VarInfo info) {
  if (scopes_.empty()) return;
  scopes_.back()[name] = std::move(info);
}

// ============================================================
// 全局声明。
// ============================================================
void ASTToIR::flattenInit(InitVal* init, BaseType base, const std::vector<int>& dims,
                          std::vector<int32_t>& out, int& idx, int total, int depth) {
  if (!init || idx >= total) return;

  if (auto* str = dynamic_cast<StringInit*>(init)) {
    for (unsigned char c : str->value) {
      if (idx >= total) break;
      out[idx++] = c;
    }
    if (idx < total) out[idx++] = 0;
    return;
  }

  if (auto* sc = dynamic_cast<ScalarInit*>(init)) {
    int32_t word = 0;
    try {
      if (base == BaseType::Float) {
        float fv = evalConstFloat(sc->expr.get());
        std::memcpy(&word, &fv, sizeof(word));
      } else {
        word = evalConstInt(sc->expr.get());
        if (base == BaseType::Char) word &= 255;
      }
    } catch (...) {
      word = 0;
    }
    if (idx >= 0 && idx < total) out[idx] = word;
    ++idx;
    return;
  }

  auto* agg = dynamic_cast<AggregateInit*>(init);
  if (!agg) return;

  int subSize = 1;
  for (int d = depth + 1; d < static_cast<int>(dims.size()); ++d)
    if (dims[d] > 0) subSize *= dims[d];

  for (auto& item : agg->items) {
    if (idx >= total) break;
    if (auto* nested = dynamic_cast<AggregateInit*>(item.get())) {
      if (subSize > 1 && idx % subSize != 0)
        idx = (idx / subSize + 1) * subSize;
      int baseIdx = idx;
      flattenInit(nested, base, dims, out, idx, total, depth + 1);
      if (subSize > 1)
        idx = baseIdx / subSize * subSize + subSize;
    } else {
      flattenInit(item.get(), base, dims, out, idx, total, depth + 1);
    }
  }
}

void ASTToIR::lowerGlobalDecl(DeclStmt* decl) {
  for (auto& def : decl->defs) {
    int totalElems = def->type.isArray() ? def->type.totalElems() : 1;
    int bytes = totalElems * 4;
    bool emptyAggregateInit = false;
    if (auto* agg = dynamic_cast<AggregateInit*>(def->initVal.get()))
      emptyAggregateInit = agg->items.empty();
    bool hasInit = def->initVal != nullptr && !emptyAggregateInit;
    Type elemTy = toIRType(decl->baseType);
    std::vector<int> dims = def->type.dims;

    auto* gv = mod_->newGlobal(def->name, elemTy, dims, bytes, decl->isConst, hasInit);

    if (hasInit) {
      gv->initData.assign(totalElems, 0);
      int idx = 0;
      flattenInit(def->initVal.get(), decl->baseType, dims, gv->initData, idx, totalElems, 0);
    }

    VarInfo info;
    info.addr = gv;
    info.elemTy = elemTy;
    info.dims = dims;
    info.isArray = def->type.isArray();
    info.isArrayParam = false;
    info.isChar = decl->baseType == BaseType::Char;
    info.isConst = decl->isConst;
    if (decl->isConst && hasInit) {
      info.constWords = gv->initData;
      if (!info.constWords.empty())
        info.constWord = info.constWords[0];
    }
    globals_[def->name] = info;
  }
}

// ============================================================
// 函数降低。
// ============================================================
void ASTToIR::lowerFunc(FuncDef* func) {
  Type retTy = toIRType(func->retType.base);
  curRetBase_ = func->retType.base;
  curFunc_ = mod_->newFunction(func->name, retTy);
  builder_ = new IRBuilder(*mod_, curFunc_);

  scopes_.clear();
  pushScope();

  // 创建入口块，newBlock 会自动加入函数名称前缀。
  auto* entryBB = curFunc_->newBlock("entry");
  curFunc_->setEntryBlock(entryBB);
  builder_->setInsertPoint(entryBB);

  // 分配参数存储位置。数组参数本身已经是指针，在 SysY 中不会作为普通值再次赋值，
  // 因此直接保留为 SSA 值，不额外引入栈槽和 reload 链。
  for (auto& p : func->params) {
    Type elemTy = toIRType(p->type.base);
    Type paramTy = p->type.isArray() ? Type::ptr() : elemTy;
    Value* arg = curFunc_->addArg(p->name, paramTy);

    VarInfo info;
    if (p->type.isArray()) {
      info.addr = arg;
    } else {
      auto* alloca =
          builder_->createAlloca(paramTy, typeSize(paramTy), false, {},
                                 p->name + ".addr");
      builder_->createStore(arg, alloca);
      info.addr = alloca;
    }
    info.elemTy = elemTy;
    info.dims = p->type.dims;
    info.isArray = p->type.isArray();
    info.isArrayParam = p->type.isArray();
    info.isChar = p->type.base == BaseType::Char;
    if (info.isChar && !p->type.isArray())
      arg = builder_->createAnd(arg, builder_->ci(255), "char.param.wrap");
    addVar(p->name, info);
  }

  // 返回块，块名会自动添加函数名前缀。
  retBlock_ = curFunc_->newBlock("ret");
  retValSlot_ = nullptr;
  if (!retTy.isVoid()) {
    retValSlot_ = builder_->createAlloca(retTy, typeSize(retTy), false, {}, "retval");
    builder_->setInsertPoint(retBlock_);
    auto* rv = builder_->createLoad(retValSlot_, retTy, "retval.load");
    builder_->createRet(rv);
  } else {
    builder_->setInsertPoint(retBlock_);
    builder_->createRet();
  }

  // 降低函数体。
  builder_->setInsertPoint(entryBB);
  breakTarget_ = nullptr;
  continueTarget_ = nullptr;

  lowerBlock(func->body.get());

  // 如果入口块最后一条指令不是终结指令，则补充分支到返回块。
  // lowerBlock/lowerStmt 会通过分支指令维护控制流。
  // 必要时补充到返回块的显式分支。
  auto* lastBB = builder_->insertPoint();
  if (lastBB && !lastBB->terminator()) {
    builder_->createBr(retBlock_);
  }

  // 清理空基本块；这里保持简单处理。

  delete builder_;
  builder_ = nullptr;

  curFunc_->recomputeCFG();
}

// ============================================================
// 基本块降低。
// ============================================================
void ASTToIR::lowerBlock(BlockStmt* block) {
  pushScope();
  for (auto& item : block->items) {
    // 检查当前块是否已有终结指令；已有时说明后续代码不可达。
    if (builder_->insertPoint()->terminator()) break;

    if (item.kind == BlockStmt::Item::Kind::Decl) {
      if (item.decl)
        lowerDecl(item.decl.get());
    } else if (item.stmt) {
      lowerStmt(item.stmt.get());
    }
  }
  popScope();
}

// ============================================================
// 声明降低。
// ============================================================
void ASTToIR::lowerDecl(DeclStmt* decl) {
  for (auto& def : decl->defs) {
    if (!def)
      continue;
    Type elemTy = toIRType(decl->baseType);
    bool isArray = def->type.isArray();
    int bytes = isArray ? std::max(1, def->type.totalElems()) * typeSize(elemTy)
                        : typeSize(elemTy);

    if (decl->isStatic) {
      const std::string staticName = "__sysy_static_" + curFunc_->name() +
                                     "_" + def->name;
      const bool hasInit = def->initVal != nullptr;
      auto* gv = mod_->newGlobal(staticName, elemTy, def->type.dims, bytes,
                                  decl->isConst, hasInit);
      if (hasInit) {
        const int total = isArray ? def->type.totalElems() : 1;
        gv->initData.assign(total, 0);
        int index = 0;
        flattenInit(def->initVal.get(), decl->baseType, def->type.dims,
                    gv->initData, index, total, 0);
      }
      VarInfo info;
      info.addr = gv;
      info.elemTy = elemTy;
      info.dims = def->type.dims;
      info.isArray = isArray;
      info.isConst = decl->isConst;
      if (decl->isConst && hasInit) {
        info.constWords = gv->initData;
        if (!info.constWords.empty()) info.constWord = info.constWords[0];
      }
      addVar(def->name, info);
      continue;
    }
    auto* alloca = builder_->createAlloca(elemTy, bytes, isArray, def->type.dims,
                                          def->name + ".addr");

    VarInfo info;
    info.addr = alloca;
    info.elemTy = elemTy;
    info.dims = def->type.dims;
    info.isArray = isArray;
    info.isArrayParam = false;
    info.isChar = decl->baseType == BaseType::Char;
    info.isConst = decl->isConst;

    if (decl->isConst && def->initVal) {
      if (auto* sc = dynamic_cast<ScalarInit*>(def->initVal.get())) {
        try {
          int32_t word = 0;
          if (elemTy.isF32()) {
            float fv = evalConstFloat(sc->expr.get());
            std::memcpy(&word, &fv, sizeof(word));
          } else {
            word = evalConstInt(sc->expr.get());
          }
          info.constWord = word;
          info.constWords = {word};
        } catch (...) {
        }
      } else if (auto* agg = dynamic_cast<AggregateInit*>(def->initVal.get())) {
        int total = def->type.isArray() ? def->type.totalElems() : 1;
        info.constWords.assign(total, 0);
        int idx = 0;
        flattenInit(agg, decl->baseType, def->type.dims, info.constWords, idx, total, 0);
        if (!info.constWords.empty())
          info.constWord = info.constWords[0];
      }
    }

    addVar(def->name, info);

    // 初始化。
    if (def->initVal) {
      if (auto* sc = dynamic_cast<ScalarInit*>(def->initVal.get())) {
        Value* initVal = lowerExpr(sc->expr.get());
        initVal = ensureType(initVal, elemTy);
        if (info.isChar)
          initVal = builder_->createAnd(initVal, builder_->ci(255), "char.wrap");
        builder_->createStore(initVal, alloca);
      } else if (dynamic_cast<AggregateInit*>(def->initVal.get())) {
        // 聚合初始化：先填零，再设置各个元素。
        builder_->createCall("memset", Type::voidTy(),
                             {alloca, builder_->ci(0), builder_->ci(bytes)});

        // 根据初始化列表设置指定元素。
        int elemIdx = 0;
        auto& dims = def->type.dims;
        std::function<void(AggregateInit*, int)> initRec;
        initRec = [&](AggregateInit* a, int depth) {
          int subSize = 1;
          for (size_t d = depth + 1; d < dims.size(); ++d)
            if (dims[d] > 0) subSize *= dims[d];
          for (auto& item : a->items) {
            if (auto* nested = dynamic_cast<AggregateInit*>(item.get())) {
              if (subSize > 1 && elemIdx % subSize != 0)
                elemIdx = (elemIdx / subSize + 1) * subSize;
              int baseIdx = elemIdx;
              initRec(nested, depth + 1);
              if (subSize > 1)
                elemIdx = baseIdx / subSize * subSize + subSize;
            } else if (auto* sc2 = dynamic_cast<ScalarInit*>(item.get())) {
              Value* v = lowerExpr(sc2->expr.get());
              v = ensureType(v, toIRType(decl->baseType));
              if (decl->baseType == BaseType::Char)
                v = builder_->createAnd(v, builder_->ci(255), "char.wrap");
              auto* idx = builder_->ci(elemIdx * typeSize(elemTy));
              auto* gep = builder_->createGEP(alloca, {idx}, "gep");
              builder_->createStore(v, gep);
              ++elemIdx;
            }
          }
        };
        if (auto* a = dynamic_cast<AggregateInit*>(def->initVal.get()))
          initRec(a, 0);
      } else if (auto* str = dynamic_cast<StringInit*>(def->initVal.get())) {
        int index = 0;
        for (unsigned char c : str->value) {
          if (index >= def->type.totalElems()) break;
          auto* addr = builder_->createGEP(alloca,
                                           {builder_->ci(index * typeSize(elemTy))},
                                           "str.elem");
          builder_->createStore(builder_->ci(c), addr);
          ++index;
        }
      }
    }
  }
}

// ============================================================
// 语句降低。
// ============================================================
void ASTToIR::lowerStmt(Stmt* stmt) {
  if (auto* b = dynamic_cast<BlockStmt*>(stmt)) {
    lowerBlock(b);
    return;
  }

  if (auto* as = dynamic_cast<AssignStmt*>(stmt)) {
    Value* rhs = lowerExpr(as->rhs.get());
    auto* lv = dynamic_cast<LValExpr*>(as->lval.get());
    if (!lv) throw std::runtime_error("assign target must be lval");
    auto* info = lookupVar(lv->name);
    if (!info) throw std::runtime_error("undefined: " + lv->name);
    rhs = ensureType(rhs, info->elemTy);
    if (info->isChar)
      rhs = builder_->createAnd(rhs, builder_->ci(255), "char.assign.wrap");
    Value* addr = lowerLValAddr(lv);
    builder_->createStore(rhs, addr);
    return;
  }

  if (auto* es = dynamic_cast<ExprStmt*>(stmt)) {
    if (es->expr) lowerExpr(es->expr.get());
    return;
  }

  if (auto* ifs = dynamic_cast<IfStmt*>(stmt)) {
    Value* cond = lowerExpr(ifs->cond.get());
    cond = normalizeCond(cond);

    auto* thenBB = curFunc_->newBlock("then");
    auto* elseBB = ifs->elseBranch ? curFunc_->newBlock("else") : nullptr;
    auto* endBB = curFunc_->newBlock("endif");

    if (elseBB) {
      builder_->createCondBr(cond, thenBB, elseBB);
    } else {
      builder_->createCondBr(cond, thenBB, endBB);
    }

    // then 分支。
    builder_->setInsertPoint(thenBB);
    lowerStmt(ifs->thenBranch.get());
    if (!builder_->insertPoint()->terminator())
      builder_->createBr(endBB);

    // else 分支。
    if (elseBB) {
      builder_->setInsertPoint(elseBB);
      lowerStmt(ifs->elseBranch.get());
      if (!builder_->insertPoint()->terminator())
        builder_->createBr(endBB);
    }

    builder_->setInsertPoint(endBB);
    return;
  }

  if (auto* ws = dynamic_cast<WhileStmt*>(stmt)) {
    auto* condBB = curFunc_->newBlock("while.cond");
    auto* bodyBB = curFunc_->newBlock("while.body");
    auto* endBB = curFunc_->newBlock("while.end");

    auto oldBreak = breakTarget_;
    auto oldContinue = continueTarget_;
    breakTarget_ = endBB;
    continueTarget_ = condBB;

    builder_->createBr(condBB);

    // 条件块。
    builder_->setInsertPoint(condBB);
    Value* cond = lowerExpr(ws->cond.get());
    cond = normalizeCond(cond);
    builder_->createCondBr(cond, bodyBB, endBB);

    // 函数体块。
    builder_->setInsertPoint(bodyBB);
    lowerStmt(ws->body.get());
    if (!builder_->insertPoint()->terminator())
      builder_->createBr(condBB);

    builder_->setInsertPoint(endBB);
    breakTarget_ = oldBreak;
    continueTarget_ = oldContinue;
    return;
  }

  if (auto* ds = dynamic_cast<DoWhileStmt*>(stmt)) {
    auto* bodyBB = curFunc_->newBlock("do.body");
    auto* condBB = curFunc_->newBlock("do.cond");
    auto* endBB = curFunc_->newBlock("do.end");

    auto oldBreak = breakTarget_;
    auto oldContinue = continueTarget_;
    breakTarget_ = endBB;
    continueTarget_ = condBB;

    builder_->createBr(bodyBB);
    builder_->setInsertPoint(bodyBB);
    lowerStmt(ds->body.get());
    if (!builder_->insertPoint()->terminator())
      builder_->createBr(condBB);

    builder_->setInsertPoint(condBB);
    Value* cond = normalizeCond(lowerExpr(ds->cond.get()));
    builder_->createCondBr(cond, bodyBB, endBB);

    builder_->setInsertPoint(endBB);
    breakTarget_ = oldBreak;
    continueTarget_ = oldContinue;
    return;
  }

  if (auto* sw = dynamic_cast<SwitchStmt*>(stmt)) {
    Value* selector = lowerExpr(sw->value.get());
    auto* endBB = curFunc_->newBlock("switch.end");
    std::vector<BasicBlock*> clauses;
    clauses.reserve(sw->clauses.size());
    BasicBlock* defaultBB = endBB;
    for (const auto& clause : sw->clauses) {
      auto* block = curFunc_->newBlock(clause.isDefault ? "switch.default"
                                                        : "switch.case");
      clauses.push_back(block);
      if (clause.isDefault) defaultBB = block;
    }

    BasicBlock* dispatch = builder_->insertPoint();
    for (size_t i = 0; i < sw->clauses.size(); ++i) {
      const auto& clause = sw->clauses[i];
      if (clause.isDefault) continue;
      auto* next = curFunc_->newBlock("switch.next");
      builder_->setInsertPoint(dispatch);
      Value* caseValue = lowerExpr(clause.value.get());
      Value* equal = builder_->createICmp("eq", selector, caseValue,
                                          "switch.match");
      builder_->createCondBr(equal, clauses[i], next);
      dispatch = next;
    }
    builder_->setInsertPoint(dispatch);
    builder_->createBr(defaultBB);

    auto* oldBreak = breakTarget_;
    breakTarget_ = endBB;
    for (size_t i = 0; i < sw->clauses.size(); ++i) {
      builder_->setInsertPoint(clauses[i]);
      for (auto& body : sw->clauses[i].statements) {
        if (builder_->insertPoint()->terminator()) break;
        lowerStmt(body.get());
      }
      if (!builder_->insertPoint()->terminator())
        builder_->createBr(i + 1 < clauses.size() ? clauses[i + 1] : endBB);
    }
    breakTarget_ = oldBreak;
    builder_->setInsertPoint(endBB);
    return;
  }

  if (auto* ps = dynamic_cast<PrintfStmt*>(stmt)) {
    size_t arg = 0;
    for (size_t i = 0; i < ps->format.size(); ++i) {
      if (ps->format[i] == '%' && i + 1 < ps->format.size()) {
        const char spec = ps->format[++i];
        if (arg >= ps->args.size()) break;
        Value* value = lowerExpr(ps->args[arg++].get());
        if (spec != 's') {
          builder_->createCall(spec == 'd' ? "putint" : "putch",
                               Type::voidTy(), {value});
        } else {
          auto* ptrSlot = builder_->createAlloca(Type::ptr(), 8, false, {},
                                                 "printf.str.ptr");
          builder_->createStore(value, ptrSlot);
          auto* cond = curFunc_->newBlock("printf.str.cond");
          auto* body = curFunc_->newBlock("printf.str.body");
          auto* end = curFunc_->newBlock("printf.str.end");
          builder_->createBr(cond);
          builder_->setInsertPoint(cond);
          auto* ptr = builder_->createLoad(ptrSlot, Type::ptr(), "printf.str.load");
          auto* chAddr = builder_->createGEP(ptr, {builder_->ci(0)}, "printf.ch.addr");
          auto* ch = builder_->createLoad(chAddr, "printf.ch");
          auto* nonzero = builder_->createICmp("ne", ch, builder_->ci(0),
                                               "printf.ch.nonzero");
          builder_->createCondBr(nonzero, body, end);
          builder_->setInsertPoint(body);
          builder_->createCall("putch", Type::voidTy(), {ch});
          auto* next = builder_->createGEP(ptr, {builder_->ci(4)}, "printf.str.next");
          builder_->createStore(next, ptrSlot);
          builder_->createBr(cond);
          builder_->setInsertPoint(end);
        }
      } else {
        builder_->createCall("putch", Type::voidTy(),
                             {builder_->ci(static_cast<unsigned char>(
                                 ps->format[i]))});
      }
    }
    return;
  }

  if (dynamic_cast<BreakStmt*>(stmt)) {
    if (!breakTarget_) throw std::runtime_error("break outside loop");
    builder_->createBr(breakTarget_);
    return;
  }

  if (dynamic_cast<ContinueStmt*>(stmt)) {
    if (!continueTarget_) throw std::runtime_error("continue outside loop");
    builder_->createBr(continueTarget_);
    return;
  }

  if (auto* ret = dynamic_cast<ReturnStmt*>(stmt)) {
    if (ret->val && !curFunc_->retTy().isVoid()) {
      Value* rv = lowerExpr(ret->val.get());
      rv = ensureType(rv, curFunc_->retTy());
      if (curRetBase_ == BaseType::Char)
        rv = builder_->createAnd(rv, builder_->ci(255), "char.return.wrap");
      builder_->createStore(rv, retValSlot_);
    }
    builder_->createBr(retBlock_);
    return;
  }

  throw std::runtime_error("unknown statement type");
}

// ============================================================
// 表达式降低。
// ============================================================
Value* ASTToIR::lowerExpr(Expr* expr) {
  if (auto* x = dynamic_cast<IntLitExpr*>(expr))
    return builder_->ci(x->val);

  if (auto* x = dynamic_cast<CharLitExpr*>(expr))
    return builder_->ci(x->val);

  if (auto* x = dynamic_cast<StringLitExpr*>(expr)) {
    if (x->indices.size() != 1)
      throw std::runtime_error("string literal requires one constant index");
    int32_t index = 0;
    try { index = evalConstInt(x->indices.front().get()); }
    catch (...) { throw std::runtime_error("string literal index must be constant"); }
    if (index < 0 || index >= static_cast<int>(x->value.size()))
      throw std::runtime_error("string literal index must be a valid constant");
    return builder_->ci(static_cast<unsigned char>(x->value[index]));
  }

  if (auto* x = dynamic_cast<FloatLitExpr*>(expr))
    return builder_->cf(x->val);

  if (auto* x = dynamic_cast<LValExpr*>(expr))
    return lowerLValLoad(x);

  if (auto* x = dynamic_cast<CallExpr*>(expr)) {
    const bool userDefined = funcRetTypes_.count(x->name) != 0;
    // 处理 starttime/stoptime。
    if (!userDefined &&
        (x->name == "starttime" || x->name == "stoptime")) {
      std::string rtName = x->name == "starttime" ? "_sysy_starttime" : "_sysy_stoptime";
      return builder_->createCall(rtName, Type::voidTy(), {builder_->ci(x->line)});
    }

    if (!userDefined && x->name == "getstr" && x->args.size() == 2) {
      Value* buffer = lowerExpr(x->args[0].get());
      Value* maxLen = lowerExpr(x->args[1].get());
      auto* ptrSlot = builder_->createAlloca(Type::ptr(), 8, false, {},
                                             "getstr.ptr");
      auto* indexSlot = builder_->createAlloca(Type::i32(), 4, false, {},
                                               "getstr.index");
      builder_->createStore(buffer, ptrSlot);
      builder_->createStore(builder_->ci(0), indexSlot);
      auto* cond = curFunc_->newBlock("getstr.cond");
      auto* body = curFunc_->newBlock("getstr.body");
      auto* stop = curFunc_->newBlock("getstr.stop");
      auto* next = curFunc_->newBlock("getstr.next");
      auto* newlineBB = curFunc_->newBlock("getstr.newline");
      auto* done = curFunc_->newBlock("getstr.done");
      builder_->createBr(cond);
      builder_->setInsertPoint(cond);
      auto* hasCapacity = builder_->createICmp("gt", maxLen, builder_->ci(0),
                                               "getstr.capacity");
      auto* capacityBB = curFunc_->newBlock("getstr.capacity.body");
      builder_->createCondBr(hasCapacity, capacityBB, done);
      builder_->setInsertPoint(capacityBB);
      auto* index = builder_->createLoad(indexSlot, "getstr.index.load");
      auto* limit = builder_->createSub(maxLen, builder_->ci(1), "getstr.limit");
      auto* room = builder_->createICmp("lt", index, limit, "getstr.room");
      builder_->createCondBr(room, body, stop);
      builder_->setInsertPoint(body);
      auto* ch = builder_->createCall("getch", Type::i32(), {});
      auto* ptr = builder_->createLoad(ptrSlot, Type::ptr(), "getstr.ptr.load");
      auto* byteOffset = builder_->createMul(index, builder_->ci(4),
                                             "getstr.offset");
      auto* addr = builder_->createGEP(ptr, {byteOffset}, "getstr.addr");
      builder_->createStore(ch, addr);
      auto* isNewline = builder_->createICmp("eq", ch, builder_->ci(10),
                                             "getstr.newline");
      builder_->createCondBr(isNewline, newlineBB, next);
      builder_->setInsertPoint(next);
      auto* nextIndex = builder_->createAdd(index, builder_->ci(1),
                                            "getstr.next.index");
      builder_->createStore(nextIndex, indexSlot);
      builder_->createBr(cond);
      builder_->setInsertPoint(newlineBB);
      auto* newlineIndex = builder_->createAdd(index, builder_->ci(1),
                                               "getstr.newline.index");
      builder_->createStore(newlineIndex, indexSlot);
      builder_->createBr(stop);
      builder_->setInsertPoint(stop);
      auto* stopIndex = builder_->createLoad(indexSlot, "getstr.stop.index");
      auto* stopPtr = builder_->createLoad(ptrSlot, Type::ptr(), "getstr.stop.ptr");
      auto* stopOffset = builder_->createMul(stopIndex, builder_->ci(4),
                                             "getstr.stop.offset");
      auto* stopAddr = builder_->createGEP(stopPtr, {stopOffset},
                                           "getstr.stop.addr");
      builder_->createStore(builder_->ci(0), stopAddr);
      builder_->createBr(done);
      builder_->setInsertPoint(done);
      return builder_->createStore(buffer, ptrSlot);
    }

    if (!userDefined && x->name == "putstr" && x->args.size() == 1) {
      Value* buffer = lowerExpr(x->args[0].get());
      auto* ptrSlot = builder_->createAlloca(Type::ptr(), 8, false, {},
                                             "putstr.ptr");
      builder_->createStore(buffer, ptrSlot);
      auto* cond = curFunc_->newBlock("putstr.cond");
      auto* body = curFunc_->newBlock("putstr.body");
      auto* done = curFunc_->newBlock("putstr.done");
      builder_->createBr(cond);
      builder_->setInsertPoint(cond);
      auto* ptr = builder_->createLoad(ptrSlot, Type::ptr(), "putstr.ptr.load");
      auto* chAddr = builder_->createGEP(ptr, {builder_->ci(0)}, "putstr.addr");
      auto* ch = builder_->createLoad(chAddr, "putstr.ch");
      auto* nonzero = builder_->createICmp("ne", ch, builder_->ci(0),
                                           "putstr.nonzero");
      builder_->createCondBr(nonzero, body, done);
      builder_->setInsertPoint(body);
      builder_->createCall("putch", Type::voidTy(), {ch});
      auto* next = builder_->createGEP(ptr, {builder_->ci(4)}, "putstr.next");
      builder_->createStore(next, ptrSlot);
      builder_->createBr(cond);
      builder_->setInsertPoint(done);
      return builder_->createStore(buffer, ptrSlot);
    }

    // 检查是否为内建函数。
    Type retTy;
    if (userDefined) {
      auto it = funcRetTypes_.find(x->name);
      retTy = it != funcRetTypes_.end() ? toIRType(it->second.base) : Type::i32();
    } else if (mod_->isBuiltin(x->name))
      retTy = mod_->builtinRetTy(x->name);
    else
      retTy = Type::i32();

    std::vector<Value*> args;
    std::vector<Type> expectedTypes;
    if (userDefined) {
      auto paramIt = funcParamTypes_.find(x->name);
      if (paramIt != funcParamTypes_.end()) {
        for (auto& pi : paramIt->second)
          expectedTypes.push_back(toIRType(pi.first, pi.second));
      }
    } else if (mod_->isBuiltin(x->name))
      expectedTypes = mod_->builtinParamTys(x->name);

    for (size_t i = 0; i < x->args.size(); ++i) {
      Value* arg = lowerExpr(x->args[i].get());
      if (i < expectedTypes.size())
        arg = ensureType(arg, expectedTypes[i]);
      args.push_back(arg);
    }

    return builder_->createCall(x->name, retTy, args);
  }

  if (auto* x = dynamic_cast<UnaryExpr*>(expr)) {
    Value* op = lowerExpr(x->operand.get());
    if (x->op == '+') return op;
    if (x->op == '-') {
      if (op->type().isF32())
        return builder_->createFNeg(op);
      return builder_->createNeg(op);
    }
    if (x->op == '!') {
      // 与零比较相等即可得到规范化的逻辑非结果。
      Value* zero = op->type().isF32()
          ? (Value*)builder_->cf(0.0f)
          : (Value*)builder_->ci(0);
      Value* cond = op->type().isF32()
          ? builder_->createFCmp("eq", op, zero)
          : builder_->createICmp("eq", op, zero);
      return cond; // 与 0 比较相等等价于逻辑非
    }
    if (x->op == '~')
      return builder_->createXor(op, builder_->ci(-1), "bitnot");
    return op;
  }

  if (auto* x = dynamic_cast<BinaryExpr*>(expr)) {
    // 短路 && 和 ||；先使用 alloca 表示，之后由 Mem2Reg 清理。
    if (x->op == "&&" || x->op == "||") {
      if (isSpeculativelySafeBoolExpr(x->lhs.get()) &&
          isSpeculativelySafeBoolExpr(x->rhs.get())) {
        // 对可投机、无副作用的布尔链直接按 eager 布尔算术降低，
        // 避免人为制造一串 sc.rhs/sc.end 基本块，便于后续 CSE/展开/guard 分析。
        return lowerBoolExprEager(x);
      }
      auto* slot = builder_->createAlloca(Type::i32(), "sc_slot");
      auto* endBB = curFunc_->newBlock("sc.end");

      Value* lhs = lowerExpr(x->lhs.get());
      lhs = normalizeCond(lhs);
      builder_->createStore(lhs, slot);

      auto* rhsBB = curFunc_->newBlock("sc.rhs");

      if (x->op == "&&") {
        // lhs 为假（0）时跳过 rhs。
        auto* lhsVal = builder_->createLoad(slot, "lhs.val");
        Value* zero = builder_->ci(0);
        auto* isTrue = builder_->createICmp("ne", lhsVal, zero);
        builder_->createCondBr(isTrue, rhsBB, endBB);
      } else {
        // ||：lhs 为真（非 0）时跳过 rhs。
        auto* lhsVal = builder_->createLoad(slot, "lhs.val");
        Value* zero = builder_->ci(0);
        auto* isTrue = builder_->createICmp("ne", lhsVal, zero);
        builder_->createCondBr(isTrue, endBB, rhsBB);
      }

      // 右操作数块。
      builder_->setInsertPoint(rhsBB);
      Value* rhs = lowerExpr(x->rhs.get());
      rhs = normalizeCond(rhs);
      builder_->createStore(rhs, slot);
      builder_->createBr(endBB);

      // 结束块。
      builder_->setInsertPoint(endBB);
      return builder_->createLoad(slot, "sc.result");
    }

    // 普通二元运算。
    Value* lhs = lowerExpr(x->lhs.get());
    Value* rhs = lowerExpr(x->rhs.get());

    // 判断是否为浮点运算。
    bool isFloat = lhs->type().isF32() || rhs->type().isF32();
    if (isFloat) {
      lhs = ensureType(lhs, Type::f32());
      rhs = ensureType(rhs, Type::f32());

      if (x->op == "+") return builder_->createFAdd(lhs, rhs);
      if (x->op == "-") return builder_->createFSub(lhs, rhs);
      if (x->op == "*") return builder_->createFMul(lhs, rhs);
      if (x->op == "/") return builder_->createFDiv(lhs, rhs);
      // 浮点比较。
      return builder_->createFCmp(cmpCondFromOp(x->op), lhs, rhs);
    }

      // 整数运算。
    Value* arithmetic = nullptr;
    if (x->op == "+") arithmetic = builder_->createAdd(lhs, rhs);
    if (x->op == "-") arithmetic = builder_->createSub(lhs, rhs);
    if (x->op == "*") arithmetic = builder_->createMul(lhs, rhs);
    if (x->op == "/") arithmetic = builder_->createSDiv(lhs, rhs);
    if (x->op == "%") arithmetic = builder_->createSRem(lhs, rhs);
    if (arithmetic) {
      if (x->semaType && x->semaType->base == BaseType::Char)
        arithmetic = builder_->createAnd(arithmetic, builder_->ci(255),
                                         "char.wrap");
      return arithmetic;
    }
    if (x->op == "&") return builder_->createAnd(lhs, rhs);
    if (x->op == "|") return builder_->createOr(lhs, rhs);
    if (x->op == "^") return builder_->createXor(lhs, rhs);
    if (x->op == "<<") return builder_->createShl(lhs, rhs);
    if (x->op == ">>") return builder_->createShr(lhs, rhs);
      // 整数比较。
    return builder_->createICmp(cmpCondFromOp(x->op), lhs, rhs);
  }

  if (auto* x = dynamic_cast<CastExpr*>(expr)) {
    Value* op = lowerExpr(x->operand.get());
    Type target = toIRType(x->targetType.base);
    Value* converted = ensureType(op, target);
    if (x->targetType.base == BaseType::Char)
      converted = builder_->createAnd(converted, builder_->ci(255), "char.wrap");
    return converted;
  }

  throw std::runtime_error("unknown expression type");
}

// ============================================================
// LVal 地址和加载。
// ============================================================
Value* ASTToIR::lowerLValAddr(LValExpr* lv) {
  VarInfo* info = lookupVar(lv->name);
  if (!info) throw std::runtime_error("undefined: " + lv->name);

  Value* base = info->addr;

  if (lv->indices.empty()) return base;

  Value* byteOffset = builder_->ci(0);
  const auto& dims = info->dims;
  for (int d = 0; d < static_cast<int>(lv->indices.size()); ++d) {
    Value* idxVal = lowerExpr(lv->indices[d].get());
    int stride = typeSize(info->elemTy);
    for (int k = d + 1; k < static_cast<int>(dims.size()); ++k)
      if (dims[k] > 0) stride *= dims[k];
    if (stride != 1)
      idxVal = builder_->createMul(idxVal, builder_->ci(stride), "idx.scale");
    byteOffset = builder_->createAdd(byteOffset, idxVal, "idx.off");
  }
  return builder_->createGEP(base, {byteOffset}, "gep");
}

Value* ASTToIR::lowerLValLoad(LValExpr* lv) {
  VarInfo* info = lookupVar(lv->name);
  if (!info) throw std::runtime_error("undefined: " + lv->name);

  if (!info->isArray && !info->isArrayParam && lv->indices.empty() &&
      info->isConst && info->constWord.has_value()) {
    if (info->elemTy.isF32())
      return builder_->cf(floatFromWord(*info->constWord));
    int32_t word = *info->constWord;
    if (info->isChar) word &= 255;
    return builder_->ci(word);
  }

  Value* addr = lowerLValAddr(lv);

  if (info->isArray) {
    int ndims = static_cast<int>(info->dims.size());
    if (static_cast<int>(lv->indices.size()) < ndims)
      return addr;
  }

  Value* loaded = builder_->createLoad(addr, info->elemTy, "load." + lv->name);
  if (info->isChar)
    loaded = builder_->createAnd(loaded, builder_->ci(255), "char.load.wrap");
  return loaded;
}

// ============================================================
    // 常量求值，沿用原后端的求值规则。
// ============================================================
int ASTToIR::flattenConstIndex(const VarInfo& info, LValExpr* lv) {
  if (lv->indices.empty()) return 0;
  int flat = 0;
  for (int d = 0; d < static_cast<int>(lv->indices.size()); ++d) {
    int idx = evalConstInt(lv->indices[d].get());
    int stride = 1;
    for (int k = d + 1; k < static_cast<int>(info.dims.size()); ++k)
      if (info.dims[k] > 0) stride *= info.dims[k];
    flat += idx * stride;
  }
  return flat;
}

bool ASTToIR::constExprIsFloat(Expr* e) {
  if (dynamic_cast<FloatLitExpr*>(e)) return true;
  if (dynamic_cast<IntLitExpr*>(e)) return false;
  if (auto* x = dynamic_cast<CastExpr*>(e))
    return x->targetType.base == BaseType::Float;
  if (auto* x = dynamic_cast<LValExpr*>(e)) {
    if (auto* info = lookupVar(x->name))
      return info->elemTy.isF32();
    return false;
  }
  if (auto* x = dynamic_cast<UnaryExpr*>(e))
    return x->op != '!' && x->op != '~' &&
           constExprIsFloat(x->operand.get());
  if (auto* x = dynamic_cast<BinaryExpr*>(e))
    return constExprIsFloat(x->lhs.get()) || constExprIsFloat(x->rhs.get());
  return false;
}

// 编译期常量求值（int 结果）：递归计算字面量/强转/一元/二元表达式；
// 任一侧操作数为 float 时先按浮点计算再截断；const 变量（含常量数组
// 下标访问）直接取预存字；无法求值时抛异常。
int32_t ASTToIR::evalConstInt(Expr* e) {
  if (auto* x = dynamic_cast<IntLitExpr*>(e)) return x->val;
  if (auto* x = dynamic_cast<FloatLitExpr*>(e)) return static_cast<int32_t>(x->val);
  if (auto* x = dynamic_cast<CastExpr*>(e)) {
    if (x->targetType.base == BaseType::Float)
      return static_cast<int32_t>(evalConstFloat(x->operand.get()));
    return evalConstInt(x->operand.get());
  }
  if (auto* x = dynamic_cast<UnaryExpr*>(e)) {
    if (constExprIsFloat(x->operand.get())) {
      float v = evalConstFloat(x->operand.get());
      if (x->op == '-') return static_cast<int32_t>(-v);
      if (x->op == '!') return !(v != 0.0f);
      if (x->op == '~') return ~static_cast<int32_t>(v);
      return static_cast<int32_t>(v);
    }
    int32_t v = evalConstInt(x->operand.get());
    if (x->op == '-') return -v;
    if (x->op == '!') return !v;
    if (x->op == '~') return ~v;
    return v;
  }
  if (auto* x = dynamic_cast<BinaryExpr*>(e)) {
    if (constExprIsFloat(x->lhs.get()) || constExprIsFloat(x->rhs.get())) {
      float l = evalConstFloat(x->lhs.get());
      float r = evalConstFloat(x->rhs.get());
      if (x->op == "+") return static_cast<int32_t>(l + r);
      if (x->op == "-") return static_cast<int32_t>(l - r);
      if (x->op == "*") return static_cast<int32_t>(l * r);
      if (x->op == "/") return r != 0.0f ? static_cast<int32_t>(l / r) : 0;
      if (x->op == "<") return l < r;
      if (x->op == "<=") return l <= r;
      if (x->op == ">") return l > r;
      if (x->op == ">=") return l >= r;
      if (x->op == "==") return l == r;
      if (x->op == "!=") return l != r;
      if (x->op == "&&") return (l != 0.0f) && (r != 0.0f);
      if (x->op == "||") return (l != 0.0f) || (r != 0.0f);
    }
    int32_t l = evalConstInt(x->lhs.get());
    int32_t r = evalConstInt(x->rhs.get());
    if (x->op == "+") return l + r;
    if (x->op == "-") return l - r;
    if (x->op == "*") return l * r;
    if (x->op == "/") return r ? l / r : 0;
    if (x->op == "%") return r ? l % r : 0;
    if (x->op == "&") return l & r;
    if (x->op == "|") return l | r;
    if (x->op == "^") return l ^ r;
    if (x->op == "<<") return l << r;
    if (x->op == ">>") return l >> r;
    if (x->op == "<") return l < r;
    if (x->op == "<=") return l <= r;
    if (x->op == ">") return l > r;
    if (x->op == ">=") return l >= r;
    if (x->op == "==") return l == r;
    if (x->op == "!=") return l != r;
    if (x->op == "&&") return l && r;
    if (x->op == "||") return l || r;
  }
  if (auto* x = dynamic_cast<LValExpr*>(e)) {
    VarInfo* info = lookupVar(x->name);
    if (info && info->isConst) {
      if (x->indices.empty() && info->constWord.has_value()) {
        if (info->elemTy.isF32())
          return static_cast<int32_t>(floatFromWord(*info->constWord));
        return *info->constWord;
      }
      if (!x->indices.empty() && !info->constWords.empty()) {
        int idx = flattenConstIndex(*info, x);
        if (idx >= 0 && idx < static_cast<int>(info->constWords.size())) {
          if (info->elemTy.isF32())
            return static_cast<int32_t>(floatFromWord(info->constWords[idx]));
          return info->constWords[idx];
        }
      }
    }
  }
  throw std::runtime_error("not a compile-time int constant");
}

// 编译期常量求值（float 结果）：与 int 版结构对称；
// 二元运算中非四则运算符回落到 evalConstInt 后转换。
float ASTToIR::evalConstFloat(Expr* e) {
  if (auto* x = dynamic_cast<FloatLitExpr*>(e)) return x->val;
  if (auto* x = dynamic_cast<IntLitExpr*>(e)) return static_cast<float>(x->val);
  if (auto* x = dynamic_cast<CastExpr*>(e)) {
    if (x->targetType.base == BaseType::Int)
      return static_cast<float>(evalConstInt(x->operand.get()));
    return evalConstFloat(x->operand.get());
  }
  if (auto* x = dynamic_cast<UnaryExpr*>(e)) {
    float v = evalConstFloat(x->operand.get());
    if (x->op == '-') return -v;
    if (x->op == '!') return !(v != 0.0f) ? 1.0f : 0.0f;
    return v;
  }
  if (auto* x = dynamic_cast<BinaryExpr*>(e)) {
    float l = evalConstFloat(x->lhs.get());
    float r = evalConstFloat(x->rhs.get());
    if (x->op == "+") return l + r;
    if (x->op == "-") return l - r;
    if (x->op == "*") return l * r;
    if (x->op == "/") return l / r;
    return static_cast<float>(evalConstInt(e));
  }
  if (auto* x = dynamic_cast<LValExpr*>(e)) {
    VarInfo* info = lookupVar(x->name);
    if (info && info->isConst) {
      if (x->indices.empty() && info->constWord.has_value()) {
        if (info->elemTy.isF32())
          return floatFromWord(*info->constWord);
        return static_cast<float>(*info->constWord);
      }
      if (!x->indices.empty() && !info->constWords.empty()) {
        int idx = flattenConstIndex(*info, x);
        if (idx >= 0 && idx < static_cast<int>(info->constWords.size())) {
          if (info->elemTy.isF32())
            return floatFromWord(info->constWords[idx]);
          return static_cast<float>(info->constWords[idx]);
        }
      }
    }
  }
  return static_cast<float>(evalConstInt(e));
}

} // namespace core_ir
