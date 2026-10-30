// 文件职责：为等价的 CoreIR 值建立编号并消除重复计算。
#include "gvn.h"
#include "pass_registry.h"
#include "passes_utils.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../../analysis/dominators.h"

namespace core_ir {

namespace {

struct ExprKey {
  Opcode op;
  Type type;
  // SSA 操作数之外的指令属性。比较指令把谓词保存在指令名称中，GEP 还带有
  // 布局信息。
  std::string semantic;
  std::vector<uint32_t> operandVNs;

  bool operator==(const ExprKey& other) const {
    return op == other.op && type == other.type &&
           semantic == other.semantic && operandVNs == other.operandVNs;
  }
};

struct ExprKeyHash {
  size_t operator()(const ExprKey& key) const {
    size_t hash = std::hash<int>()(static_cast<int>(key.op));
    auto mix = [&](size_t value) {
      hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    };
    mix(std::hash<int>()(static_cast<int>(key.type.kind)));
    mix(std::hash<int>()(key.type.ptrDepth));
    mix(std::hash<std::string>()(key.semantic));
    for (uint32_t vn : key.operandVNs)
      mix(std::hash<uint32_t>()(vn));
    return hash;
  }
};

bool isCommutative(Opcode op, const std::string& predicate) {
  switch (op) {
  case Opcode::Add:
  case Opcode::Mul:
  case Opcode::FAdd:
  case Opcode::FMul:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
    return true;
  case Opcode::ICmp:
  case Opcode::FCmp:
    return predicate == "eq" || predicate == "ne";
  default:
    return false;
  }
}

// 只给结果完全由 SSA 操作数和不可变指令属性决定的表达式编号。加载指令不在
// 候选范围内，因为没有 MemorySSA 时，中间的存储或调用可能改变内存内容。
bool isGVNCandidate(const Instruction* inst) {
  if (!inst || inst->type().isVoid() || inst->hasSideEffects())
    return false;
  switch (inst->opcode()) {
  case Opcode::Add:
  case Opcode::Sub:
  case Opcode::Mul:
  case Opcode::SDiv:
  case Opcode::SRem:
  case Opcode::And:
  case Opcode::Or:
  case Opcode::Xor:
  case Opcode::Shl:
  case Opcode::Shr:
  case Opcode::FAdd:
  case Opcode::FSub:
  case Opcode::FMul:
  case Opcode::FDiv:
  case Opcode::ICmp:
  case Opcode::FCmp:
  case Opcode::SIToFP:
  case Opcode::FPToSI:
  case Opcode::GEP:
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::Select:
  case Opcode::VAdd:
  case Opcode::VSub:
  case Opcode::VMul:
  case Opcode::VDiv:
  case Opcode::VFMA:
  case Opcode::VSplat:
  case Opcode::VExtract:
    return true;
  default:
    return false;
  }
}

std::string semanticKey(Instruction* inst) {
  if (inst->opcode() == Opcode::ICmp || inst->opcode() == Opcode::FCmp)
    return cmpCond(inst);
  if (inst->opcode() != Opcode::GEP)
    return {};

  std::string key = typeName(inst->memoryElemTy()) + ":" +
                    std::to_string(inst->memorySizeBytes()) + ":" +
                    (inst->isArrayObject() ? "array" : "scalar");
  for (int dimension : inst->dims())
    key += ":" + std::to_string(dimension);
  return key;
}

class GVNImpl {
public:
  GVNImpl(Module& module, analysis::AnalysisManager& analysisManager)
      : mod(module), am(analysisManager) {}

  bool run(Function* function) {
    resetFunctionState();
    const analysis::DomSet& dominators = am.function(function).dominators();

    std::vector<Instruction*> instructions;
    for (auto& block : function->blocks()) {
      if (!block)
        continue;
      for (auto& inst : block->instructions()) {
        if (inst)
          instructions.push_back(inst.get());
      }
    }

    // 按确定的程序顺序记录表达式代表。递归 getVN 调用可以为操作数编号，
    // 但只有这一轮安装表达式代表。
    for (Instruction* inst : instructions) {
      if (!isGVNCandidate(inst))
        continue;
      uint32_t vn = getVN(inst);
      vnToLeader.emplace(vn, inst);
    }

    for (Instruction* inst : instructions) {
      if (!isGVNCandidate(inst))
        continue;
      auto leaderIt = vnToLeader.find(getVN(inst));
      if (leaderIt == vnToLeader.end() || leaderIt->second == inst)
        continue;

      Instruction* leader = leaderIt->second;
      if (!leader || !leader->parent() || !inst->parent())
        continue;
      if (!analysis::dominates(dominators, leader->parent(), inst->parent()))
        continue;

      // 两个表达式在同一块时，仅凭块支配关系还不够，还要确认代表指令的
      // 位置在当前指令之前。
      if (leader->parent() == inst->parent() &&
          !appearsBefore(leader->parent(), leader, inst))
        continue;

      inst->replaceAllUsesWith(leader);
      changed = true;
    }
    return changed;
  }

private:
  Module& mod;
  analysis::AnalysisManager& am;
  uint32_t nextVN = 1;
  bool changed = false;
  std::unordered_map<Value*, uint32_t> valueToVN;
  std::unordered_map<uint32_t, Instruction*> vnToLeader;
  std::unordered_map<ExprKey, uint32_t, ExprKeyHash> exprToVN;
  std::unordered_set<Value*> visiting;
  std::unordered_map<int32_t, uint32_t> intToVN;
  std::unordered_map<uint32_t, uint32_t> floatToVN;

  void resetFunctionState() {
    nextVN = 1;
    valueToVN.clear();
    vnToLeader.clear();
    exprToVN.clear();
    visiting.clear();
    intToVN.clear();
    floatToVN.clear();
  }

  static bool appearsBefore(BasicBlock* block, Instruction* first,
                            Instruction* second) {
    for (const auto& inst : block->instructions()) {
      if (inst.get() == first)
        return true;
      if (inst.get() == second)
        return false;
    }
    return false;
  }

  uint32_t freshVN() { return nextVN++; }

  uint32_t getVN(Value* value) {
    auto known = valueToVN.find(value);
    if (known != valueToVN.end())
      return known->second;
    if (!visiting.insert(value).second)
      return freshVN();
    uint32_t vn = assignVN(value);
    visiting.erase(value);
    valueToVN[value] = vn;
    return vn;
  }

  uint32_t assignVN(Value* value) {
    if (auto* constant = dynamic_cast<ConstInt*>(value)) {
      auto [it, inserted] = intToVN.emplace(constant->value(), 0);
      if (inserted)
        it->second = freshVN();
      return it->second;
    }
    if (auto* constant = dynamic_cast<ConstFloat*>(value)) {
      uint32_t bits = 0;
      float raw = constant->value();
      std::memcpy(&bits, &raw, sizeof(bits));
      auto [it, inserted] = floatToVN.emplace(bits, 0);
      if (inserted)
        it->second = freshVN();
      return it->second;
    }
    if (dynamic_cast<Argument*>(value) || dynamic_cast<GlobalVar*>(value) ||
        dynamic_cast<BasicBlock*>(value))
      return freshVN();

    auto* inst = dynamic_cast<Instruction*>(value);
    if (!isGVNCandidate(inst))
      return freshVN();

    ExprKey key{inst->opcode(), inst->type(), semanticKey(inst), {}};
    key.operandVNs.reserve(inst->operands().size());
    for (Value* operand : inst->operands())
      key.operandVNs.push_back(getVN(operand));
    if (isCommutative(key.op, key.semantic) && key.operandVNs.size() == 2)
      std::sort(key.operandVNs.begin(), key.operandVNs.end());

    auto existing = exprToVN.find(key);
    if (existing != exprToVN.end())
      return existing->second;
    uint32_t vn = freshVN();
    exprToVN.emplace(std::move(key), vn);
    return vn;
  }
};

} // namespace

bool globalValueNumbering(Module& mod, analysis::AnalysisManager& am) {
  GVNImpl implementation(mod, am);
  bool changed = false;
  for (auto& function : mod.functions())
    changed |= implementation.run(function.get());
  return changed;
}

class GlobalValueNumberingPass : public Pass {
public:
  std::string name() const override { return "GlobalValueNumbering"; }
  bool run(Module& mod, analysis::AnalysisManager& am) override {
    return globalValueNumbering(mod, am);
  }
};

REGISTER_PASS(GlobalValueNumberingPass);

} // namespace core_ir
