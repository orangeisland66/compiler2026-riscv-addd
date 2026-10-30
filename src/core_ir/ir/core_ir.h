// 文件职责：定义 CoreIR 类型、值、指令、基本块、函数和模块。本文件负责声明接口和数据结构。
#pragma once
#ifndef CORE_IR_H_
#define CORE_IR_H_
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

// ============================================================
// 类型系统。
// ============================================================
enum class TypeKind { I32, F32, Void, Ptr, Label, V4I32, V4F32 };

struct Type {
  TypeKind kind;
  int ptrDepth = 0; // 0 表示标量，>0 表示指针层数，用于数组
  static Type i32() { return {TypeKind::I32, 0}; }
  static Type f32() { return {TypeKind::F32, 0}; }
  static Type voidTy() { return {TypeKind::Void, 0}; }
  static Type ptr() { return {TypeKind::Ptr, 0}; }
  static Type label() { return {TypeKind::Label, 0}; }
  static Type v4i32() { return {TypeKind::V4I32, 0}; }
  static Type v4f32() { return {TypeKind::V4F32, 0}; }
  bool isI32() const { return kind == TypeKind::I32; }
  bool isF32() const { return kind == TypeKind::F32; }
  bool isVoid() const { return kind == TypeKind::Void; }
  bool isPtr() const { return kind == TypeKind::Ptr; }
  bool isLabel() const { return kind == TypeKind::Label; }
  bool isV4I32() const { return kind == TypeKind::V4I32; }
  bool isV4F32() const { return kind == TypeKind::V4F32; }
  bool isVector() const { return kind == TypeKind::V4I32 || kind == TypeKind::V4F32; }
  bool isIntOrPtr() const { return kind == TypeKind::I32 || kind == TypeKind::Ptr; }
  bool operator==(const Type& o) const { return kind == o.kind && ptrDepth == o.ptrDepth; }
  bool operator!=(const Type& o) const { return !(*this == o); }
};

// ============================================================
// 前置声明。
// ============================================================
class Value;
class Instruction;
class BasicBlock;
class Function;
class Module;
class IRBuilder;

// ============================================================
// Value：IR 中所有值的基类。
// ============================================================
class Value {
public:
  Value(Type ty, const std::string& name = "");
  virtual ~Value();

  Type type() const { return type_; }
  const std::string& name() const { return name_; }
  void setName(const std::string& n) { name_ = n; }
  int id() const { return id_; }
  // Planning clone 失败时恢复诊断编号，避免一次被拒绝的 shadow 评估改变
  // 后续正式 IR 的 tie-break/打印。只允许 planning 所有权层调用。
  static int idCheckpoint() { return nextId_; }
  static void restoreIdCheckpoint(int checkpoint) { nextId_ = checkpoint; }

  // 使用关系跟踪：记录哪些指令使用该值。
  void addUse(Instruction* user);
  void removeUse(Instruction* user);
  const std::vector<Instruction*>& uses() const { return uses_; }
  void replaceAllUsesWith(Value* other);

  // 判断是否为常量。
  virtual bool isConstant() const { return false; }
  static bool isLiveValue(const Value* value);

private:
  Type type_;
  std::string name_;
  int id_;
  std::vector<Instruction*> uses_;
  static int nextId_;
  static std::unordered_set<const Value*> liveValues_;
};

// ============================================================
// 常量。
// ============================================================
class ConstInt : public Value {
public:
  ConstInt(int32_t val) : Value(Type::i32(), "ci"), val_(val) {}
  int32_t value() const { return val_; }
  bool isConstant() const override { return true; }

private:
  int32_t val_;
};

class ConstFloat : public Value {
public:
  ConstFloat(float val) : Value(Type::f32(), "cf"), val_(val) {}
  float value() const { return val_; }
  bool isConstant() const override { return true; }

private:
  float val_;
};

class Argument : public Value {
public:
  Argument(Type ty, const std::string& name) : Value(ty, name) {}
};

// ============================================================
// 全局变量。
// ============================================================
class GlobalVar : public Value {
public:
  GlobalVar(const std::string& name, Type elemTy, std::vector<int> dims,
            int sizeBytes, bool isConst, bool hasInit);
  Type elemTy() const { return elemTy_; }
  const std::vector<int>& dims() const { return dims_; }
  int sizeBytes() const { return sizeBytes_; }
  bool isConst() const { return isConst_; }
  bool hasInit() const { return hasInit_; }
  // 初始化数据，用于非零初始化。
  std::vector<int32_t> initData;

private:
  Type elemTy_;
  std::vector<int> dims_;
  int sizeBytes_;
  bool isConst_;
  bool hasInit_;
};

// ============================================================
// 指令操作码。
// ============================================================
enum class Opcode {
  // 终结指令。
  Br, CondBr, Ret,
  // 内存操作。
  Alloca, Load, Store,
  // 整数算术。
  Add, Sub, Mul, Mulh, SDiv, SRem,
  // 循环不变除数先在循环外验证直接魔数公式，再完成快速有符号除法。
  SDivDirectPrepare, SDivDirectMagic, SDivDirectShift, SDivDirectFlag,
  SDivInvariantDirect,
  // 整数位运算。
  And, Or, Xor, Shl, Shr,
  // 浮点算术。
  FAdd, FSub, FMul, FDiv,
  // 比较。
  ICmp, FCmp,
  // 类型转换。
  SIToFP, FPToSI,
  // 其他操作。
  Call, GEP, Phi, Neg, FNeg, BitCast, Select,
  // 移动操作，供寄存器分配使用。
  Mov,
  // 预留的固定宽度向量 IR；当前 RISC-V 流水线不生成这些操作。
  VAdd, VSub, VMul, VDiv, VFMA, VLoad, VStore, VSplat, VExtract,
};

const char* opcodeName(Opcode op);

// ============================================================
// Instruction：使用其他 Value 并产生结果的 IR 指令。
// ============================================================
class Instruction : public Value {
public:
  Instruction(Opcode op, Type ty, const std::string& name = "");

  Opcode opcode() const { return op_; }
  BasicBlock* parent() const { return parent_; }
  void setParent(BasicBlock* bb) { parent_ = bb; }

  // 操作数管理。
  int numOperands() const { return static_cast<int>(operands_.size()); }
  Value* operand(int i) const { return operands_[i]; }
  void addOperand(Value* v);
  void setOperand(int i, Value* v);
  void dropAllOperands();
  const std::vector<Value*>& operands() const { return operands_; }

  // 是否有副作用；call、store、br、ret 都会影响程序状态。
  bool hasSideEffects() const;

  bool isTerminator() const;

  void setMemoryInfo(Type elemTy, int sizeBytes, bool arrayObject,
                     std::vector<int> dims = {});
  Type memoryElemTy() const { return memoryElemTy_; }
  int memorySizeBytes() const { return memorySizeBytes_; }
  bool isArrayObject() const { return arrayObject_; }
  const std::vector<int>& dims() const { return dims_; }

  // 便捷访问接口。
  Value* getOperandAs(int i) const { return operands_[i]; }

private:
  Opcode op_;
  BasicBlock* parent_ = nullptr;
  std::vector<Value*> operands_;
  Type memoryElemTy_ = Type::i32();
  int memorySizeBytes_ = 0;
  bool arrayObject_ = false;
  std::vector<int> dims_;
};

// ============================================================
// BasicBlock 也是一种 Value（类型为 label），可以作为 br、cond_br、phi 的操作数。
// ============================================================
class BasicBlock : public Value {
public:
  BasicBlock(const std::string& name, Function* func);

  const std::string& name() const { return Value::name(); }
  Function* parent() const { return parent_; }

  // 指令列表。
  void pushBack(Instruction* inst);
  void pushFront(Instruction* inst);
  void insertBefore(Instruction* pos, Instruction* inst);
  void remove(Instruction* inst);
  const std::vector<std::unique_ptr<Instruction>>& instructions() const { return insts_; }
  std::vector<std::unique_ptr<Instruction>>& instructions() { return insts_; }
  bool empty() const { return insts_.empty(); }
  Instruction* terminator() const;

  // 前驱和后继。
  void addPredecessor(BasicBlock* bb) { preds_.push_back(bb); }
  void addSuccessor(BasicBlock* bb) { succs_.push_back(bb); }
  const std::vector<BasicBlock*>& preds() const { return preds_; }
  const std::vector<BasicBlock*>& succs() const { return succs_; }
  void clearPreds() { preds_.clear(); }
  void clearSuccs() { succs_.clear(); }
  void replaceSuccessor(BasicBlock* oldBB, BasicBlock* newBB);
  void replacePredecessor(BasicBlock* oldBB, BasicBlock* newBB);

  // 遍历辅助函数。
  using iterator = std::vector<std::unique_ptr<Instruction>>::iterator;
  iterator begin() { return insts_.begin(); }
  iterator end() { return insts_.end(); }

private:
  Function* parent_;
  std::vector<std::unique_ptr<Instruction>> insts_;
  std::vector<BasicBlock*> preds_;
  std::vector<BasicBlock*> succs_;
};

// ============================================================
// 函数参数。
// ============================================================
struct Arg {
  std::string name;
  Type type;
  Value* val = nullptr; // 该参数在 IR 中对应的 Value
};

// ============================================================
// 函数。
// ============================================================
class Function {
public:
  Function(const std::string& name, Type retTy, Module* mod);

  const std::string& name() const { return name_; }
  Type retTy() const { return retTy_; }
  Module* parent() const { return parent_; }
  void setParent(Module* mod) { parent_ = mod; }

  // 函数参数。
  Value* addArg(const std::string& name, Type ty);
  const std::vector<Arg>& args() const { return args_; }

  // 基本块。
  BasicBlock* entryBlock() const { return entry_; }
  void setEntryBlock(BasicBlock* bb) { entry_ = bb; }
  BasicBlock* newBlock(const std::string& prefix = "");
  BasicBlock* prependBlock(const std::string& prefix = "");
  const std::vector<std::unique_ptr<BasicBlock>>& blocks() const { return blocks_; }
  void removeBlock(BasicBlock* bb);
  void recomputeCFG();

  // Alloca 跟踪信息，供 Mem2Reg 使用。
  const std::vector<Instruction*>& allocas() const { return allocas_; }
  void addAlloca(Instruction* a);
  void removeAlloca(Instruction* a);
  bool hasLocalizedGlobal(const std::string& name) const {
    return localizedGlobals_.count(name) != 0;
  }
  void markLocalizedGlobal(const std::string& name) {
    localizedGlobals_.insert(name);
  }
  const std::unordered_set<std::string>& localizedGlobals() const {
    return localizedGlobals_;
  }

private:
  std::string name_;
  Type retTy_;
  Module* parent_;
  std::vector<Arg> args_;
  std::vector<std::unique_ptr<Argument>> argValues_;
  std::vector<std::unique_ptr<BasicBlock>> blocks_;
  BasicBlock* entry_ = nullptr;
  int blockCnt_ = 0;
  std::vector<Instruction*> allocas_;
  std::unordered_set<std::string> localizedGlobals_;
};

// ============================================================
// 模块。
// ============================================================
class Module {
public:
  Module();
  Module(const Module&) = delete;
  Module& operator=(const Module&) = delete;
  Module(Module&&) noexcept = default;
  Module& operator=(Module&&) noexcept = default;

  // Transactional planning commit: takes ownership of a fully verified module
  // and repairs Function::parent pointers after the move.
  void replaceWith(Module&& other);

  // 函数。
  Function* newFunction(const std::string& name, Type retTy);
  const std::vector<std::unique_ptr<Function>>& functions() const { return funcs_; }
  Function* getFunction(const std::string& name) const;
  void removeFunction(Function* func);

  // 全局变量。
  GlobalVar* newGlobal(const std::string& name, Type elemTy, std::vector<int> dims,
                       int sizeBytes, bool isConst, bool hasInit);
  const std::vector<std::unique_ptr<GlobalVar>>& globals() const { return globals_; }

  // 常量，已去重。
  ConstInt* getConstInt(int32_t val);
  ConstFloat* getConstFloat(float val);

  // 内建函数声明，例如 getint、putint 等。
  void declareBuiltin(const std::string& name, Type retTy, const std::vector<Type>& paramTys);
  bool isBuiltin(const std::string& name) const;
  std::vector<std::string> builtinNames() const;
  Type builtinRetTy(const std::string& name) const;
  const std::vector<Type>& builtinParamTys(const std::string& name) const;

private:
  std::vector<std::unique_ptr<Function>> funcs_;
  std::vector<std::unique_ptr<GlobalVar>> globals_;
  std::unordered_map<int32_t, ConstInt*> constInts_;
  std::unordered_map<int32_t, ConstFloat*> constFloats_; // 按浮点位模式索引
  std::vector<std::unique_ptr<ConstInt>> ownedConstInts_;
  std::vector<std::unique_ptr<ConstFloat>> ownedConstFloats_;
  struct BuiltinInfo {
    Type retTy;
    std::vector<Type> paramTys;
  };
  std::unordered_map<std::string, BuiltinInfo> builtins_;
};

// ============================================================
// IRBuilder：构造 IR 的辅助类。
// ============================================================
class IRBuilder {
public:
  IRBuilder(Module& mod, Function* func);

  void setInsertPoint(BasicBlock* bb);
  BasicBlock* insertPoint() const { return bb_; }
  Function* function() const { return func_; }
  Module& module() { return mod_; }

  // 常量。
  ConstInt* ci(int32_t v);
  ConstFloat* cf(float v);

  // 终结指令。
  Instruction* createBr(BasicBlock* target);
  Instruction* createCondBr(Value* cond, BasicBlock* trueBB, BasicBlock* falseBB);
  Instruction* createRet(Value* val = nullptr);

  // 内存操作。
  Instruction* createAlloca(Type elemTy, int sizeBytes, bool arrayObject,
                            const std::vector<int>& dims,
                            const std::string& name = "");
  Instruction* createAlloca(Type ty, const std::string& name = "");
  Instruction* createLoad(Value* addr, Type ty, const std::string& name = "");
  Instruction* createLoad(Value* addr, const std::string& name = "");
  Instruction* createStore(Value* val, Value* addr);

  // 整数算术。
  Instruction* createAdd(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createSub(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createMul(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createSDiv(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createSRem(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createAnd(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createOr(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createXor(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createShl(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createShr(Value* lhs, Value* rhs, const std::string& name = "");

  // 浮点算术。
  Instruction* createFAdd(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createFSub(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createFMul(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createFDiv(Value* lhs, Value* rhs, const std::string& name = "");

  // 比较。
  Instruction* createICmp(const std::string& cond, Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createFCmp(const std::string& cond, Value* lhs, Value* rhs, const std::string& name = "");

  // 类型转换。
  Instruction* createSIToFP(Value* val);
  Instruction* createFPToSI(Value* val);

  // 其他操作。
  Instruction* createNeg(Value* val);
  Instruction* createFNeg(Value* val);
  Instruction* createCall(const std::string& callee, Type retTy, const std::vector<Value*>& args, const std::string& name = "");
  Instruction* createGEP(Value* base, const std::vector<Value*>& indices, const std::string& name = "");
  Instruction* createPhi(Type ty, const std::vector<std::pair<Value*, BasicBlock*>>& incoming, const std::string& name = "");
  Instruction* createBitCast(Value* val, Type targetTy, const std::string& name = "");
  // Select：cond ? trueVal : falseVal（3 个操作数依次为条件、真值、假值）。
  Instruction* createSelect(Value* cond, Value* trueVal, Value* falseVal,
                            const std::string& name = "");

  // 预留的固定宽度向量 IR 构造接口，不由当前 RISC-V 流水线调用。
  Instruction* createVAdd(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createVSub(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createVMul(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createVDiv(Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createVFMA(Value* acc, Value* lhs, Value* rhs, const std::string& name = "");
  Instruction* createVLoad(Value* addr, Type vecTy, const std::string& name = "");
  Instruction* createVStore(Value* val, Value* addr);
  Instruction* createVSplat(Value* scalar, Type vecTy, const std::string& name = "");
  Instruction* createVExtract(Value* vec, int lane, const std::string& name = "");

private:
  Instruction* insert(Instruction* inst);

  Module& mod_;
  Function* func_;
  BasicBlock* bb_ = nullptr;
};

// ============================================================
// IR 打印器，用于调试。
// ============================================================
void printModule(std::ostream& out, const Module& mod);

} // namespace core_ir
#endif // CORE_IR_H_
