// 文件职责：定义 CoreIR 类型、值、指令、基本块、函数和模块。本文件负责实现具体逻辑。
#include "core_ir.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace core_ir {

// ============================================================
// Value 实现。
// ============================================================
int Value::nextId_ = 0;
std::unordered_set<const Value*> Value::liveValues_;

Value::Value(Type ty, const std::string& name) : type_(ty), name_(name), id_(nextId_++) {
  liveValues_.insert(this);
}

Value::~Value() {
  liveValues_.erase(this);
}

bool Value::isLiveValue(const Value* value) {
  return value && liveValues_.count(value) != 0;
}

void Value::addUse(Instruction* user) {
  if (std::find(uses_.begin(), uses_.end(), user) == uses_.end())
    uses_.push_back(user);
}

void Value::removeUse(Instruction* user) {
  auto it = std::find(uses_.begin(), uses_.end(), user);
  if (it != uses_.end()) uses_.erase(it);
}

void Value::replaceAllUsesWith(Value* other) {
  // RAUW 不能把一个值替换成自身。若继续清空 uses_，后续删
  // 指令时会留下未登记的悬空 operand，递归/effect 分析再遍历就可能崩溃。
  if (other == this)
    return;
  auto users = uses_;
  for (auto* u : users) {
    for (int i = 0; i < u->numOperands(); ++i)
      if (u->operand(i) == this) u->setOperand(i, other);
  }
  uses_.clear();
}

// ============================================================
// 全局变量实现。
// ============================================================
GlobalVar::GlobalVar(const std::string& name, Type elemTy, std::vector<int> dims,
                     int sizeBytes, bool isConst, bool hasInit)
    : Value(Type::ptr(), name), elemTy_(elemTy), dims_(std::move(dims)),
      sizeBytes_(sizeBytes), isConst_(isConst), hasInit_(hasInit) {}

// ============================================================
// 指令实现。
// ============================================================
// Opcode 到字符串名称的映射，供 IR 打印与调试诊断使用。
const char* opcodeName(Opcode op) {
  switch (op) {
  case Opcode::Br:      return "br";
  case Opcode::CondBr:  return "cond_br";
  case Opcode::Ret:     return "ret";
  case Opcode::Alloca:  return "alloca";
  case Opcode::Load:    return "load";
  case Opcode::Store:   return "store";
  case Opcode::Add:     return "add";
  case Opcode::Sub:     return "sub";
  case Opcode::Mul:     return "mul";
  case Opcode::Mulh:    return "mulh";
  case Opcode::SDiv:    return "sdiv";
  case Opcode::SRem:    return "srem";
  case Opcode::SDivDirectPrepare:return "sdiv.direct.prepare";
  case Opcode::SDivDirectMagic:  return "sdiv.direct.magic";
  case Opcode::SDivDirectShift:  return "sdiv.direct.shift";
  case Opcode::SDivDirectFlag:   return "sdiv.direct.flag";
  case Opcode::SDivInvariantDirect:return "sdiv.invariant.direct";
  case Opcode::And:     return "and";
  case Opcode::Or:      return "or";
  case Opcode::Xor:     return "xor";
  case Opcode::Shl:     return "shl";
  case Opcode::Shr:     return "shr";
  case Opcode::FAdd:    return "fadd";
  case Opcode::FSub:    return "fsub";
  case Opcode::FMul:    return "fmul";
  case Opcode::FDiv:    return "fdiv";
  case Opcode::ICmp:    return "icmp";
  case Opcode::FCmp:    return "fcmp";
  case Opcode::SIToFP:  return "sitofp";
  case Opcode::FPToSI:  return "fptosi";
  case Opcode::Call:    return "call";
  case Opcode::GEP:     return "gep";
  case Opcode::Phi:     return "phi";
  case Opcode::Neg:     return "neg";
  case Opcode::FNeg:    return "fneg";
  case Opcode::BitCast: return "bitcast";
  case Opcode::Select:  return "select";
  case Opcode::Mov:     return "mov";
  case Opcode::VAdd:    return "vadd";
  case Opcode::VSub:    return "vsub";
  case Opcode::VMul:    return "vmul";
  case Opcode::VDiv:    return "vdiv";
  case Opcode::VFMA:    return "vfma";
  case Opcode::VLoad:   return "vload";
  case Opcode::VStore:  return "vstore";
  case Opcode::VSplat:  return "vsplat";
  case Opcode::VExtract:return "vextract";
  }
  return "???";
}

Instruction::Instruction(Opcode op, Type ty, const std::string& name)
    : Value(ty, name), op_(op) {}

void Instruction::addOperand(Value* v) {
  operands_.push_back(v);
  v->addUse(this);
}

void Instruction::setOperand(int i, Value* v) {
  Value* old = operands_[i];
  if (old == v)
    return;
  operands_[i] = v;
  // use-list 只按“用户指令”去重，不记录同一用户里的
  // operand 次数。因此替换一个 operand 后，只有确认该指令不再通过
  // 其它 operand 使用旧值时，才能从旧值的 uses_ 中删除本指令。
  bool stillUsesOld = false;
  for (Value* op : operands_) {
    if (op == old) {
      stillUsesOld = true;
      break;
    }
  }
  if (!stillUsesOld)
    old->removeUse(this);
  v->addUse(this);
}

void Instruction::dropAllOperands() {
  std::vector<Value*> uniqueOperands;
  for (auto* op : operands_) {
    if (std::find(uniqueOperands.begin(), uniqueOperands.end(), op) ==
        uniqueOperands.end())
      uniqueOperands.push_back(op);
  }
  operands_.clear();
  // 先清空 operand，再逐个删除 use；这样 removeUse 看到的是
  // “本指令已经完全不再使用该值”的状态，不会被重复 operand 误导。
  for (auto* op : uniqueOperands)
    op->removeUse(this);
}

bool Instruction::hasSideEffects() const {
  switch (op_) {
  case Opcode::Store:
  case Opcode::VStore:
  case Opcode::Br:
  case Opcode::CondBr:
  case Opcode::Ret:
  case Opcode::Call:
    return true;
  default:
    return false;
  }
}

bool Instruction::isTerminator() const {
  return op_ == Opcode::Br || op_ == Opcode::CondBr || op_ == Opcode::Ret;
}

void Instruction::setMemoryInfo(Type elemTy, int sizeBytes, bool arrayObject,
                                std::vector<int> dims) {
  memoryElemTy_ = elemTy;
  memorySizeBytes_ = sizeBytes;
  arrayObject_ = arrayObject;
  dims_ = std::move(dims);
}

// ============================================================
// 基本块实现。
// ============================================================
BasicBlock::BasicBlock(const std::string& name, Function* func)
    : Value(Type::label(), name), parent_(func) {}

void BasicBlock::pushBack(Instruction* inst) {
  inst->setParent(this);
  insts_.push_back(std::unique_ptr<Instruction>(inst));
}

void BasicBlock::pushFront(Instruction* inst) {
  inst->setParent(this);
  insts_.insert(insts_.begin(), std::unique_ptr<Instruction>(inst));
}

void BasicBlock::insertBefore(Instruction* pos, Instruction* inst) {
  inst->setParent(this);
  for (auto it = insts_.begin(); it != insts_.end(); ++it) {
    if (it->get() == pos) {
      insts_.insert(it, std::unique_ptr<Instruction>(inst));
      return;
    }
  }
}

void BasicBlock::remove(Instruction* inst) {
  auto it = std::find_if(insts_.begin(), insts_.end(),
                          [inst](const auto& p) { return p.get() == inst; });
  if (it != insts_.end()) {
    if ((*it)->opcode() == Opcode::Alloca && parent_)
      parent_->removeAlloca(it->get());
    (*it)->dropAllOperands();
    insts_.erase(it);
  }
}

Instruction* BasicBlock::terminator() const {
  if (insts_.empty()) return nullptr;
  auto* last = insts_.back().get();
  return last->isTerminator() ? last : nullptr;
}

void BasicBlock::replaceSuccessor(BasicBlock* oldBB, BasicBlock* newBB) {
  for (auto& s : succs_)
    if (s == oldBB) s = newBB;
}

void BasicBlock::replacePredecessor(BasicBlock* oldBB, BasicBlock* newBB) {
  for (auto& p : preds_)
    if (p == oldBB) p = newBB;
}

// ============================================================
// 函数。
// ============================================================
Function::Function(const std::string& name, Type retTy, Module* mod)
    : name_(name), retTy_(retTy), parent_(mod) {}

Value* Function::addArg(const std::string& name, Type ty) {
  auto* v = new Argument(ty, name);
  argValues_.push_back(std::unique_ptr<Argument>(v));
  args_.push_back({name, ty, v});
  return v;
}

BasicBlock* Function::newBlock(const std::string& prefix) {
  auto name = name_ + "." + (prefix.empty() ? "bb" : prefix) + std::to_string(blockCnt_);
  auto* bb = new BasicBlock(name, this);
  blocks_.push_back(std::unique_ptr<BasicBlock>(bb));
  ++blockCnt_;
  return bb;
}

BasicBlock* Function::prependBlock(const std::string& prefix) {
  auto name = name_ + "." + (prefix.empty() ? "bb" : prefix) + std::to_string(blockCnt_);
  auto* bb = new BasicBlock(name, this);
  blocks_.insert(blocks_.begin(), std::unique_ptr<BasicBlock>(bb));
  ++blockCnt_;
  return bb;
}

void Function::addAlloca(Instruction* a) {
  if (!a)
    return;
  if (std::find(allocas_.begin(), allocas_.end(), a) == allocas_.end())
    allocas_.push_back(a);
}

void Function::removeAlloca(Instruction* a) {
  auto it = std::remove(allocas_.begin(), allocas_.end(), a);
  if (it != allocas_.end())
    allocas_.erase(it, allocas_.end());
}

void Function::removeBlock(BasicBlock* bb) {
  if (bb) {
    for (auto& inst : bb->instructions()) {
      if (inst && inst->opcode() == Opcode::Alloca)
        removeAlloca(inst.get());
    }
  }
  auto it = std::find_if(blocks_.begin(), blocks_.end(),
                          [bb](const auto& p) { return p.get() == bb; });
  if (it != blocks_.end()) blocks_.erase(it);
}

void Function::recomputeCFG() {
  for (auto& bb : blocks_) {
    bb->clearPreds();
    bb->clearSuccs();
  }
  for (auto& bb : blocks_) {
    auto* term = bb->terminator();
    if (!term) continue;
    if (term->opcode() == Opcode::Br) {
      auto* target = dynamic_cast<BasicBlock*>(term->operand(0));
      if (target) { bb->addSuccessor(target); target->addPredecessor(bb.get()); }
    } else if (term->opcode() == Opcode::CondBr) {
      auto* tBB = dynamic_cast<BasicBlock*>(term->operand(1));
      auto* fBB = dynamic_cast<BasicBlock*>(term->operand(2));
      if (tBB) { bb->addSuccessor(tBB); tBB->addPredecessor(bb.get()); }
      if (fBB) { bb->addSuccessor(fBB); fBB->addPredecessor(bb.get()); }
    }
  }
}

// ============================================================
// 模块实现。
// ============================================================
Module::Module() {}

void Module::replaceWith(Module&& other) {
  *this = std::move(other);
  for (auto& function : funcs_)
    function->setParent(this);
}

Function* Module::newFunction(const std::string& name, Type retTy) {
  auto* f = new Function(name, retTy, this);
  funcs_.push_back(std::unique_ptr<Function>(f));
  return f;
}

Function* Module::getFunction(const std::string& name) const {
  for (auto& f : funcs_)
    if (f->name() == name) return f.get();
  return nullptr;
}

void Module::removeFunction(Function* func) {
  auto it = std::find_if(funcs_.begin(), funcs_.end(),
                         [func](const auto& candidate) {
                           return candidate.get() == func;
                         });
  if (it == funcs_.end())
    return;
  for (auto& bb : (*it)->blocks()) {
    for (auto& inst : bb->instructions())
      inst->dropAllOperands();
  }
  funcs_.erase(it);
}

GlobalVar* Module::newGlobal(const std::string& name, Type elemTy, std::vector<int> dims,
                             int sizeBytes, bool isConst, bool hasInit) {
  auto* g = new GlobalVar(name, elemTy, std::move(dims), sizeBytes, isConst, hasInit);
  globals_.push_back(std::unique_ptr<GlobalVar>(g));
  return g;
}

ConstInt* Module::getConstInt(int32_t val) {
  auto it = constInts_.find(val);
  if (it != constInts_.end()) return it->second;
  auto* ci = new ConstInt(val);
  ownedConstInts_.push_back(std::unique_ptr<ConstInt>(ci));
  constInts_[val] = ci;
  return ci;
}

ConstFloat* Module::getConstFloat(float val) {
  int32_t bits = 0;
  std::memcpy(&bits, &val, sizeof(bits));
  auto it = constFloats_.find(bits);
  if (it != constFloats_.end()) return it->second;
  auto* cf = new ConstFloat(val);
  ownedConstFloats_.push_back(std::unique_ptr<ConstFloat>(cf));
  constFloats_[bits] = cf;
  return cf;
}

void Module::declareBuiltin(const std::string& name, Type retTy, const std::vector<Type>& paramTys) {
  builtins_[name] = {retTy, paramTys};
}

bool Module::isBuiltin(const std::string& name) const {
  return builtins_.count(name) > 0;
}

std::vector<std::string> Module::builtinNames() const {
  std::vector<std::string> names;
  names.reserve(builtins_.size());
  for (const auto& entry : builtins_) names.push_back(entry.first);
  std::sort(names.begin(), names.end());
  return names;
}

Type Module::builtinRetTy(const std::string& name) const {
  auto it = builtins_.find(name);
  return it != builtins_.end() ? it->second.retTy : Type::voidTy();
}

const std::vector<Type>& Module::builtinParamTys(const std::string& name) const {
  static std::vector<Type> empty;
  auto it = builtins_.find(name);
  return it != builtins_.end() ? it->second.paramTys : empty;
}

// ============================================================
// IRBuilder 实现。
// ============================================================
IRBuilder::IRBuilder(Module& mod, Function* func) : mod_(mod), func_(func) {}

void IRBuilder::setInsertPoint(BasicBlock* bb) { bb_ = bb; }

ConstInt* IRBuilder::ci(int32_t v) { return mod_.getConstInt(v); }
ConstFloat* IRBuilder::cf(float v) { return mod_.getConstFloat(v); }

Instruction* IRBuilder::insert(Instruction* inst) {
  bb_->pushBack(inst);
  return inst;
}

Instruction* IRBuilder::createBr(BasicBlock* target) {
  auto* i = new Instruction(Opcode::Br, Type::voidTy());
  i->addOperand(target);
  return insert(i);
}

Instruction* IRBuilder::createCondBr(Value* cond, BasicBlock* trueBB, BasicBlock* falseBB) {
  auto* i = new Instruction(Opcode::CondBr, Type::voidTy());
  i->addOperand(cond);
  i->addOperand(trueBB);
  i->addOperand(falseBB);
  return insert(i);
}

Instruction* IRBuilder::createRet(Value* val) {
  Type ty = val ? val->type() : Type::voidTy();
  auto* i = new Instruction(Opcode::Ret, ty);
  if (val) i->addOperand(val);
  return insert(i);
}

Instruction* IRBuilder::createAlloca(Type ty, const std::string& name) {
  return createAlloca(ty, ty.isPtr() ? 8 : 4, false, {}, name);
}

Instruction* IRBuilder::createAlloca(Type elemTy, int sizeBytes, bool arrayObject,
                                     const std::vector<int>& dims,
                                     const std::string& name) {
  auto* i = new Instruction(Opcode::Alloca, Type::ptr(), name);
  i->setMemoryInfo(elemTy, sizeBytes, arrayObject, dims);
  func_->addAlloca(i);
  return insert(i);
}

Instruction* IRBuilder::createLoad(Value* addr, Type ty, const std::string& name) {
  auto* i = new Instruction(Opcode::Load, ty, name);
  i->addOperand(addr);
  return insert(i);
}

Instruction* IRBuilder::createLoad(Value* addr, const std::string& name) {
  Type ty = Type::i32();
  if (auto* inst = dynamic_cast<Instruction*>(addr)) {
    if (inst->opcode() == Opcode::Alloca)
      ty = inst->memoryElemTy();
  } else if (auto* gv = dynamic_cast<GlobalVar*>(addr)) {
    ty = gv->elemTy();
  }
  return createLoad(addr, ty, name);
}

Instruction* IRBuilder::createStore(Value* val, Value* addr) {
  auto* i = new Instruction(Opcode::Store, Type::voidTy());
  i->addOperand(val);
  i->addOperand(addr);
  return insert(i);
}

Instruction* IRBuilder::createAdd(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Add, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createSub(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Sub, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createMul(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Mul, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createSDiv(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::SDiv, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createSRem(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::SRem, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createAnd(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::And, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createOr(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Or, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createXor(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Xor, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createShl(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Shl, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createShr(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::Shr, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createFAdd(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::FAdd, Type::f32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createFSub(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::FSub, Type::f32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createFMul(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::FMul, Type::f32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createFDiv(Value* lhs, Value* rhs, const std::string& name) {
  auto* i = new Instruction(Opcode::FDiv, Type::f32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

static int cmpCode(const std::string& cond) {
  if (cond == "eq") return 0;
  if (cond == "ne") return 1;
  if (cond == "lt") return 2;
  if (cond == "le") return 3;
  if (cond == "gt") return 4;
  if (cond == "ge") return 5;
  return 0;
}

static std::string cmpName(const std::string& cond) {
  static const char* names[] = {"eq","ne","lt","le","gt","ge"};
  int c = cmpCode(cond);
  return names[c];
}

Instruction* IRBuilder::createICmp(const std::string& cond, Value* lhs, Value* rhs,
                                   const std::string& name) {
  auto* i = new Instruction(Opcode::ICmp, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  i->setName(cmpName(cond) + "_" + (name.empty() ? std::to_string(i->id()) : ""));
  return insert(i);
}

Instruction* IRBuilder::createFCmp(const std::string& cond, Value* lhs, Value* rhs,
                                   const std::string& name) {
  auto* i = new Instruction(Opcode::FCmp, Type::i32(), name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  i->setName(cmpName(cond) + "_" + (name.empty() ? std::to_string(i->id()) : ""));
  return insert(i);
}

Instruction* IRBuilder::createSIToFP(Value* val) {
  auto* i = new Instruction(Opcode::SIToFP, Type::f32());
  i->addOperand(val);
  return insert(i);
}

Instruction* IRBuilder::createFPToSI(Value* val) {
  auto* i = new Instruction(Opcode::FPToSI, Type::i32());
  i->addOperand(val);
  return insert(i);
}

Instruction* IRBuilder::createNeg(Value* val) {
  auto* i = new Instruction(Opcode::Neg, Type::i32());
  i->addOperand(val);
  return insert(i);
}

Instruction* IRBuilder::createFNeg(Value* val) {
  auto* i = new Instruction(Opcode::FNeg, Type::f32());
  i->addOperand(val);
  return insert(i);
}

Instruction* IRBuilder::createCall(const std::string& callee, Type retTy,
                                   const std::vector<Value*>& args, const std::string& name) {
  auto* i = new Instruction(Opcode::Call, retTy, name);
  i->setName(callee);
  for (auto* a : args) i->addOperand(a);
  return insert(i);
}

Instruction* IRBuilder::createGEP(Value* base, const std::vector<Value*>& indices,
                                  const std::string& name) {
  auto* i = new Instruction(Opcode::GEP, Type::ptr(), name);
  i->addOperand(base);
  for (auto* idx : indices) i->addOperand(idx);
  return insert(i);
}

Instruction* IRBuilder::createPhi(Type ty,
                                  const std::vector<std::pair<Value*, BasicBlock*>>& incoming,
                                  const std::string& name) {
  auto* i = new Instruction(Opcode::Phi, ty, name);
  for (auto& [val, bb] : incoming) {
    i->addOperand(val);
    i->addOperand(bb);
  }
  return insert(i);
}

Instruction* IRBuilder::createBitCast(Value* val, Type targetTy, const std::string& name) {
  auto* i = new Instruction(Opcode::BitCast, targetTy, name);
  i->addOperand(val);
  return insert(i);
}

Instruction* IRBuilder::createSelect(Value* cond, Value* trueVal, Value* falseVal,
                                     const std::string& name) {
  auto* i = new Instruction(Opcode::Select, trueVal->type(), name);
  i->addOperand(cond);
  i->addOperand(trueVal);
  i->addOperand(falseVal);
  return insert(i);
}

Instruction* IRBuilder::createVAdd(Value* lhs, Value* rhs, const std::string& name) {
  Type ty = lhs->type().isV4F32() ? Type::v4f32() : Type::v4i32();
  auto* i = new Instruction(Opcode::VAdd, ty, name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createVSub(Value* lhs, Value* rhs, const std::string& name) {
  Type ty = lhs->type().isV4F32() ? Type::v4f32() : Type::v4i32();
  auto* i = new Instruction(Opcode::VSub, ty, name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createVMul(Value* lhs, Value* rhs, const std::string& name) {
  Type ty = lhs->type().isV4F32() ? Type::v4f32() : Type::v4i32();
  auto* i = new Instruction(Opcode::VMul, ty, name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createVDiv(Value* lhs, Value* rhs, const std::string& name) {
  Type ty = lhs->type().isV4F32() ? Type::v4f32() : Type::v4i32();
  auto* i = new Instruction(Opcode::VDiv, ty, name);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createVFMA(Value* acc, Value* lhs, Value* rhs, const std::string& name) {
  Type ty = acc->type().isV4F32() ? Type::v4f32() : Type::v4i32();
  auto* i = new Instruction(Opcode::VFMA, ty, name);
  i->addOperand(acc);
  i->addOperand(lhs);
  i->addOperand(rhs);
  return insert(i);
}

Instruction* IRBuilder::createVLoad(Value* addr, Type vecTy, const std::string& name) {
  auto* i = new Instruction(Opcode::VLoad, vecTy, name);
  i->addOperand(addr);
  return insert(i);
}

Instruction* IRBuilder::createVStore(Value* val, Value* addr) {
  auto* i = new Instruction(Opcode::VStore, Type::voidTy());
  i->addOperand(val);
  i->addOperand(addr);
  return insert(i);
}

Instruction* IRBuilder::createVSplat(Value* scalar, Type vecTy, const std::string& name) {
  auto* i = new Instruction(Opcode::VSplat, vecTy, name);
  i->addOperand(scalar);
  return insert(i);
}

Instruction* IRBuilder::createVExtract(Value* vec, int lane, const std::string& name) {
  Type scalarTy = vec->type().isV4F32() ? Type::f32() : Type::i32();
  auto* i = new Instruction(Opcode::VExtract, scalarTy, name);
  i->addOperand(vec);
  i->addOperand(mod_.getConstInt(lane));
  return insert(i);
}

// ============================================================
// IR 打印器实现。
// ============================================================
// 以文本 IR 形式打印整个模块：依次输出全局变量、函数签名
// （返回类型与参数）及各基本块内的指令与操作数。
void printModule(std::ostream& out, const Module& mod) {
  for (auto& g : mod.globals()) {
    out << "@" << g->name() << " = global " << g->sizeBytes() << " bytes\n";
  }
  for (auto& f : mod.functions()) {
    out << "\ndefine ";
    auto rt = f->retTy();
    if (rt.isI32()) out << "i32";
    else if (rt.isF32()) out << "f32";
    else if (rt.isV4I32()) out << "v4i32";
    else if (rt.isV4F32()) out << "v4f32";
    else out << "void";
    out << " @" << f->name() << "(";
    for (size_t i = 0; i < f->args().size(); ++i) {
      if (i) out << ", ";
      auto& a = f->args()[i];
      if (a.type.isI32()) out << "i32";
      else if (a.type.isF32()) out << "f32";
      else out << "ptr";
      out << " %" << a.name;
    }
    out << ") {\n";
    for (auto& bb : f->blocks()) {
      out << bb->name() << ":\n";
      for (auto& inst : bb->instructions()) {
        out << "  ";
        if (!inst->type().isVoid()) {
          out << "%" << inst->id() << " = ";
        }
        out << opcodeName(inst->opcode());
        for (int i = 0; i < inst->numOperands(); ++i) {
          out << " ";
          auto* op = inst->operand(i);
          if (auto* ci = dynamic_cast<ConstInt*>(op))
            out << ci->value();
          else if (auto* cf = dynamic_cast<ConstFloat*>(op))
            out << cf->value();
          else if (auto* bbv = dynamic_cast<BasicBlock*>(op))
            out << "%" << bbv->name();
          else if (dynamic_cast<GlobalVar*>(op))
            out << "@" << op->name();
          else
            out << "%" << op->id();
        }
        out << "\n";
      }
    }
    out << "}\n";
  }
}

} // namespace core_ir
