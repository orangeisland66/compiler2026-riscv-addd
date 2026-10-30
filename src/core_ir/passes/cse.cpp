#include "research_switch.h"
// 文件职责：识别并复用重复的 CoreIR 表达式和加载结果。
#include "cse.h"
#include "mem2reg.h"
#include "passes_utils.h"
#include "pass_registry.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core_ir {

using analysis::buildDomTreeChildren;
using analysis::computeIDom;

bool localLoadForwarding(Module& mod, analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  bool changed = false;
  for (auto& func : mod.functions()) {
    const analysis::MemorySSALite& memory =
        analysisManager.memorySSA(func.get());
    bool removedStore = false;
    for (auto& bb : func->blocks()) {
      struct KnownMemoryValue {
        Instruction* access = nullptr;
        Value* value = nullptr;
      };
      std::vector<Instruction*> stores;
      std::vector<Instruction*> loads;
      std::vector<KnownMemoryValue> knownValues;
      std::vector<Instruction*> deadStores;

      for (auto& instPtr : bb->instructions()) {
        auto* inst = instPtr.get();
        if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
          for (auto it = knownValues.rbegin(); it != knownValues.rend(); ++it) {
            if (!it->access || it->value != inst->operand(0))
              continue;
            if (analysis::memoryValueReachesStore(it->access, inst,
                                                  &effects).proved()) {
              deadStores.push_back(inst);
              break;
            }
          }
          stores.push_back(inst);
          knownValues.push_back({inst, inst->operand(0)});
          continue;
        }
        if (inst->opcode() != Opcode::Load || inst->numOperands() != 1)
          continue;

        Value* replacement = nullptr;
        for (auto it = stores.rbegin(); it != stores.rend(); ++it) {
          Instruction* store = *it;
          if (!store || store->numOperands() != 2 ||
              store->operand(0)->type() != inst->type())
            continue;
          if (analysis::storedValueReachesLoad(store, inst, &effects).proved()) {
            replacement = store->operand(0);
            break;
          }
        }
        if (!replacement) {
          for (auto it = loads.rbegin(); it != loads.rend(); ++it) {
            Instruction* load = *it;
            if (!load || load->type() != inst->type())
              continue;
            if (analysis::sameMemoryVersion(memory, load, inst,
                                            &effects).proved()) {
              replacement = load;
              break;
            }
          }
        }
        if (replacement) {
          inst->replaceAllUsesWith(replacement);
          changed = true;
        }
        loads.push_back(inst);
        knownValues.push_back({inst, replacement ? replacement : inst});
      }
      for (Instruction* store : deadStores) {
        if (store && store->parent() == bb.get()) {
          store->dropAllOperands();
          bb->remove(store);
          removedStore = true;
          changed = true;
        }
      }
    }
    if (removedStore)
      analysisManager.invalidateFunction(
          func.get(), analysis::AnalysisInvalidation::Memory);
  }
  return changed;
}

bool isCSECandidate(Instruction* inst) {
  if (inst->type().isVoid()) return false;
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
  case Opcode::Neg:
  case Opcode::FNeg:
  case Opcode::BitCast:
  case Opcode::GEP:
    return true;
  default:
    return false;
  }
}

bool isCommutativeCSE(Opcode op, const std::string& cond) {
  if (op == Opcode::Add || op == Opcode::Mul) return true;
  if (op == Opcode::ICmp && (cond == "eq" || cond == "ne")) return true;
  return false;
}

std::string valueKey(Value* v) {
  if (auto* ci = dynamic_cast<ConstInt*>(v))
    return "ci:" + std::to_string(ci->value());
  if (auto* cf = dynamic_cast<ConstFloat*>(v)) {
    int32_t bits = 0;
    float value = cf->value();
    std::memcpy(&bits, &value, sizeof(bits));
    return "cf:" + std::to_string(bits);
  }
  return "v:" + std::to_string(v->id());
}


// 沿 Add/Sub/Shl/Mul 链递归收集 GEP 偏移的线性项：常量折算进 constant，
// 非常量记为 (valueKey, 系数) 项；active 集防环，移位量超限或乘法两侧
// 均非常量即判定失败。
bool appendLinearOffsetTerms(Value* value,
                             int64_t coeff,
                             LinearOffsetKey& out,
                             std::unordered_set<int>& active) {
  if (!value || coeff == 0)
    return true;
  if (auto* ci = dynamic_cast<ConstInt*>(value)) {
    out.constant += coeff * static_cast<int64_t>(ci->value());
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst) {
    out.terms.push_back({valueKey(value), coeff});
    return true;
  }
  if (!active.insert(inst->id()).second)
    return false;
  bool ok = true;
  if (inst->opcode() == Opcode::Add && inst->numOperands() == 2) {
    ok = appendLinearOffsetTerms(inst->operand(0), coeff, out, active) &&
         appendLinearOffsetTerms(inst->operand(1), coeff, out, active);
  } else if (inst->opcode() == Opcode::Sub && inst->numOperands() == 2) {
    ok = appendLinearOffsetTerms(inst->operand(0), coeff, out, active) &&
         appendLinearOffsetTerms(inst->operand(1), -coeff, out, active);
  } else if (inst->opcode() == Opcode::Shl && inst->numOperands() == 2) {
    auto* shift = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!shift || shift->value() < 0 || shift->value() > 20) {
      ok = false;
    } else {
      int64_t scale = 1LL << shift->value();
      ok = appendLinearOffsetTerms(inst->operand(0), coeff * scale, out, active);
    }
  } else if (inst->opcode() == Opcode::Mul && inst->numOperands() == 2) {
    auto* lhsConst = dynamic_cast<ConstInt*>(inst->operand(0));
    auto* rhsConst = dynamic_cast<ConstInt*>(inst->operand(1));
    if (lhsConst) {
      ok = appendLinearOffsetTerms(inst->operand(1),
                                   coeff * static_cast<int64_t>(lhsConst->value()),
                                   out, active);
    } else if (rhsConst) {
      ok = appendLinearOffsetTerms(inst->operand(0),
                                   coeff * static_cast<int64_t>(rhsConst->value()),
                                   out, active);
    } else {
      ok = false;
    }
  } else {
    out.terms.push_back({valueKey(value), coeff});
  }
  active.erase(inst->id());
  return ok;
}

LinearOffsetKey buildGEPLinearOffsetKey(Instruction* gep) {
  LinearOffsetKey key;
  if (!gep || gep->opcode() != Opcode::GEP || gep->numOperands() < 2)
    return key;
  key.root = gep->operand(0);
  std::unordered_set<int> active;
  for (int i = 1; i < gep->numOperands(); ++i) {
    if (!appendLinearOffsetTerms(gep->operand(i), 1, key, active))
      return {};
  }
  std::sort(key.terms.begin(), key.terms.end());
  std::vector<std::pair<std::string, int64_t>> merged;
  for (const auto& term : key.terms) {
    if (!merged.empty() && merged.back().first == term.first) {
      merged.back().second += term.second;
      continue;
    }
    merged.push_back(term);
  }
  key.terms.clear();
  for (const auto& term : merged) {
    if (term.second != 0)
      key.terms.push_back(term);
  }
  key.valid = true;
  return key;
}

std::string linearOffsetShapeKey(const LinearOffsetKey& key) {
  if (!key.valid || !key.root)
    return {};
  std::ostringstream os;
  os << valueKey(key.root);
  for (const auto& [termKey, coeff] : key.terms)
    os << "|" << termKey << ":" << coeff;
  return os.str();
}

std::string linearOffsetExactKey(const LinearOffsetKey& key) {
  std::string shape = linearOffsetShapeKey(key);
  if (shape.empty())
    return {};
  return shape + "|c=" + std::to_string(key.constant);
}

std::string cseKey(Instruction* inst) {
  if (inst && inst->opcode() == Opcode::GEP) {
    LinearOffsetKey linearKey = buildGEPLinearOffsetKey(inst);
    std::string key = linearOffsetExactKey(linearKey);
    if (!key.empty()) {
      // GVN/InstCombine 风格的 GEP 规范化。展开和 blocking
      // 会生成很多语法不同但线性 offset 相同的地址；用 root+线性项+常量
      // 做 key，可以让现有 CSE 合并它们。
      return "gep-linear:" + typeName(inst->type()) + ":" + key;
    }
  }

  std::string key = std::string(opcodeName(inst->opcode())) + ":" +
                    typeName(inst->type()) + ":";
  std::string cond;
  if (inst->opcode() == Opcode::ICmp || inst->opcode() == Opcode::FCmp) {
    cond = cmpCond(inst);
    key += cond + ":";
  }

  std::vector<std::string> ops;
  ops.reserve(inst->operands().size());
  for (auto* op : inst->operands())
    ops.push_back(valueKey(op));

  if (ops.size() == 2 && isCommutativeCSE(inst->opcode(), cond))
    std::sort(ops.begin(), ops.end());

  for (auto& op : ops)
    key += op + ",";
  return key;
}

bool rebaseLocalGEPChains(Module& mod) {
  bool changed = false;
  constexpr int64_t kMaxCheapDelta = 2047;
  // 只重定位相邻的行或切片，不连接相距很远的区域。
  // 过大的偏移会拉长地址依赖链，并让多个派生基址跨越内层循环保持活跃，
  // 增加寄存器压力。
  constexpr int64_t kMaxScaledDelta = 64 << 10;
  for (auto& func : mod.functions()) {
    for (auto& bb : func->blocks()) {
      std::unordered_map<std::string, std::vector<std::pair<int64_t, Instruction*>>>
          seenByShape;
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::GEP)
          continue;

        LinearOffsetKey key = buildGEPLinearOffsetKey(inst);
        std::string shape = linearOffsetShapeKey(key);
        if (shape.empty())
          continue;
        bool hasWideDynamicScale = false;
        for (const auto& term : key.terms) {
          if (term.second > 16 || term.second < -16) {
            hasWideDynamicScale = true;
            break;
          }
        }
        const int64_t maxDelta =
            hasWideDynamicScale ? kMaxScaledDelta : kMaxCheapDelta;

        Instruction* bestBase = nullptr;
        int64_t bestBaseConstant = 0;
        int64_t bestAbsDelta = maxDelta + 1;
        auto& seen = seenByShape[shape];
        for (const auto& [prevConst, prevGep] : seen) {
          if (!prevGep || prevGep == inst)
            continue;
          int64_t delta = key.constant - prevConst;
          int64_t absDelta = delta < 0 ? -delta : delta;
          if (absDelta == 0 || absDelta > maxDelta ||
              absDelta >= bestAbsDelta)
            continue;
          bestBase = prevGep;
          bestBaseConstant = prevConst;
          bestAbsDelta = absDelta;
        }
        if (bestBase) {
          // InstCombine/LSR 风格的块内 GEP 重基址。unroll 后常见
          // 多个地址共享同一线性动态项、只差小常量偏移；把后续地址改成
          // `gep previous, delta`，让 lowering 使用 addi/基址寻址，而不重新
          // 物化整条 `shl + add + gep` 链。这里只使用同一块内已支配的前驱
          // GEP，并且要求线性项完全同形。
          int64_t delta = key.constant - bestBaseConstant;
          inst->dropAllOperands();
          inst->addOperand(bestBase);
          inst->addOperand(mod.getConstInt(static_cast<int32_t>(delta)));
          changed = true;
        }
        seen.push_back({key.constant, inst});
      }
    }
  }
  return changed;
}

void collectMemoryRoots(Value* value, std::unordered_set<int>& active,
                        std::unordered_set<Value*>& roots) {
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst) {
    if (value)
      roots.insert(value);
    return;
  }
  if (!active.insert(inst->id()).second)
    return;

  if ((inst->opcode() == Opcode::GEP || inst->opcode() == Opcode::BitCast) &&
      inst->numOperands() >= 1) {
    collectMemoryRoots(inst->operand(0), active, roots);
  } else if (inst->opcode() == Opcode::Phi && inst->type().isPtr()) {
    for (Value* operand : inst->operands()) {
      if (operand && operand->type().isPtr())
        collectMemoryRoots(operand, active, roots);
    }
  } else {
    roots.insert(value);
  }
  active.erase(inst->id());
}

Value* memoryRoot(Value* addr) {
  std::unordered_set<int> active;
  std::unordered_set<Value*> roots;
  collectMemoryRoots(addr, active, roots);
  return roots.size() == 1 ? *roots.begin() : addr;
}

bool runtimeBoundExprOpcodeAllowed(Opcode op) {
  switch (op) {
  case Opcode::Phi:
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
  case Opcode::ICmp:
  case Opcode::Mov:
  case Opcode::Neg:
  case Opcode::BitCast:
    return true;
  default:
    return false;
  }
}

bool valueDependsOnRuntimeBoundLoads(
    Value* value, const std::unordered_set<int>& loadIds,
    std::unordered_map<int, bool>& memo) {
  if (!value)
    return false;
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return false;
  if (loadIds.count(inst->id()))
    return true;
  auto it = memo.find(inst->id());
  if (it != memo.end())
    return it->second;
  memo[inst->id()] = false;
  for (Value* operand : inst->operands()) {
    if (valueDependsOnRuntimeBoundLoads(operand, loadIds, memo)) {
      memo[inst->id()] = true;
      break;
    }
  }
  return memo[inst->id()];
}

Value* cloneRuntimeBoundExpr(Value* value, BasicBlock* preheader,
                             Instruction* preheaderTerm,
                             const analysis::NaturalLoop& loop,
                             const std::unordered_set<int>& loadIds,
                             std::unordered_map<int, Value*>& cache) {
  if (!value)
    return nullptr;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value) || dynamic_cast<BasicBlock*>(value)) {
    return value;
  }

  auto it = cache.find(value->id());
  if (it != cache.end())
    return it->second;

  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst)
    return nullptr;
  if (loadIds.count(inst->id())) {
    auto* clone = new Instruction(
        inst->opcode(), inst->type(),
        inst->name().empty() ? "runtime.bound.load"
                             : inst->name() + ".runtime.bound");
    clone->addOperand(inst->operand(0));
    preheader->insertBefore(preheaderTerm, clone);
    cache[value->id()] = clone;
    return clone;
  }
  if (!inst->parent() || !loop.blocks.count(inst->parent())) {
    cache[value->id()] = value;
    return value;
  }
  if (inst->opcode() == Opcode::Phi ||
      !runtimeBoundExprOpcodeAllowed(inst->opcode())) {
    return nullptr;
  }

  auto* clone = new Instruction(
      inst->opcode(), inst->type(),
      inst->name().empty() ? "runtime.bound"
                           : inst->name() + ".runtime.bound");
  for (Value* operand : inst->operands()) {
    Value* clonedOperand =
        cloneRuntimeBoundExpr(operand, preheader, preheaderTerm, loop, loadIds,
                              cache);
    if (!clonedOperand)
      return nullptr;
    clone->addOperand(clonedOperand);
  }
  preheader->insertBefore(preheaderTerm, clone);
  cache[value->id()] = clone;
  return clone;
}

bool localizeRuntimeScalarGlobalBounds(Function* func, Module& mod,
                                       analysis::AnalysisManager& analysisManager) {
  if (!func || !func->entryBlock())
    return false;
  const auto& effects = analysisManager.functionEffects();
  auto& cache = analysisManager.function(func);
  const auto loops = cache.loops();
  bool changed = false;

  for (const auto& loop : loops) {
    BasicBlock* preheader = analysis::dedicatedPreheader(loop);
    if (!preheader || !preheader->terminator())
      continue;

    auto infos = analysis::collectRuntimeScalarGlobalBounds(
        func, loop, cache.inductionVars(loop), &effects);
    if (infos.empty())
      continue;

    Instruction* preheaderTerm = preheader->terminator();
    std::unordered_set<int> seenCompares;
    std::unordered_map<int, Value*> cloneCache;
    for (const auto& info : infos) {
      if (!info.compare || !info.localizable || !info.hasInLoopLoad)
        continue;
      if (!seenCompares.insert(info.compare->id()).second)
        continue;

      std::unordered_set<int> loadIds;
      for (auto* load : info.directLoads) {
        if (load)
          loadIds.insert(load->id());
      }
      if (loadIds.empty())
        continue;

      std::vector<Value*> rewrittenOperands;
      rewrittenOperands.reserve(info.compare->numOperands());
      std::unordered_map<int, bool> depMemo;
      bool compareChanged = false;
      bool compareValid = true;
      for (int i = 0; i < info.compare->numOperands(); ++i) {
        Value* operand = info.compare->operand(i);
        if (!valueDependsOnRuntimeBoundLoads(operand, loadIds, depMemo)) {
          rewrittenOperands.push_back(operand);
          continue;
        }
        Value* cloned = cloneRuntimeBoundExpr(operand, preheader, preheaderTerm,
                                              loop, loadIds, cloneCache);
        if (!cloned) {
          compareValid = false;
          break;
        }
        rewrittenOperands.push_back(cloned);
        compareChanged = true;
      }
      if (!compareValid || !compareChanged)
        continue;
      for (int i = 0; i < static_cast<int>(rewrittenOperands.size()); ++i)
        info.compare->setOperand(i, rewrittenOperands[static_cast<size_t>(i)]);
      changed = true;
    }
  }

  if (changed)
    analysisManager.invalidateFunction(func, analysis::AnalysisInvalidation::CFG);
  (void)mod;
  return changed;
}

bool localizeRuntimeScalarGlobalBounds(Module& mod,
                                       analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    changed |=
        localizeRuntimeScalarGlobalBounds(funcPtr.get(), mod, analysisManager);
  }
  return changed;
}


MemoryAliasInfo classifyMemory(Value* addr) {
  MemoryAliasInfo info;
  Value* root = memoryRoot(addr);
  if (!root)
    return info;

  info.valid = true;
  info.root = root;
  if (auto* gv = dynamic_cast<GlobalVar*>(root)) {
    info.precise = true;
    info.invariant = gv->isConst();
    info.rootKind = MemoryRootKind::Global;
    info.preciseKey = "g:" + std::to_string(gv->id());
    return info;
  }
  if (auto* arg = dynamic_cast<Argument*>(root)) {
    // 指针参数不应默认视为不同对象：两个参数可能互相别名，
    // 也可能指向可变全局对象。除非后续 noalias 分析给出更明确的信息，
    // 否则按未知内存处理。
    info.precise = false;
    info.rootKind = MemoryRootKind::Argument;
    (void)arg;
    return info;
  }
  if (auto* inst = dynamic_cast<Instruction*>(root)) {
    if (inst->opcode() == Opcode::Alloca) {
      info.precise = true;
      info.rootKind = MemoryRootKind::Alloca;
      info.preciseKey = "a:" + std::to_string(inst->id());
      return info;
    }
  }
  return info;
}

bool preciseModRefEnabled() {
  return sycEnvironmentValue("SYC_DISABLE_PRECISE_MODREF") == nullptr;
}

bool memoryMayAlias(const MemoryAliasInfo& lhs, const MemoryAliasInfo& rhs) {
  if (!lhs.valid || !rhs.valid)
    return true;
  if (!preciseModRefEnabled()) {
    return !lhs.precise || !rhs.precise ||
           lhs.preciseKey == rhs.preciseKey;
  }
  if (lhs.precise && rhs.precise)
    return lhs.preciseKey == rhs.preciseKey;

  // 指针参数可能指向全局对象或其他参数，但不会指向当前函数激活期间创建的
  // alloca。
  if ((lhs.rootKind == MemoryRootKind::Alloca &&
       (rhs.rootKind == MemoryRootKind::Argument ||
        rhs.rootKind == MemoryRootKind::Global)) ||
      (rhs.rootKind == MemoryRootKind::Alloca &&
       (lhs.rootKind == MemoryRootKind::Argument ||
        lhs.rootKind == MemoryRootKind::Global))) {
    return false;
  }
  return true;
}


const std::vector<int>& preciseMemoryVersion(
    const MemoryVersionState& state, const std::string& key) {
  static const std::vector<int> empty;
  auto it = state.preciseVersions.find(key);
  return it == state.preciseVersions.end() ? empty : it->second;
}

std::string memoryVersionKey(const std::vector<int>& versions) {
  if (versions.empty())
    return "entry";
  std::string key;
  for (int version : versions) {
    if (!key.empty()) key += ".";
    key += std::to_string(version);
  }
  return key;
}

std::string canonicalMemoryAddressKey(Value* addr) {
  if (!addr)
    return {};
  auto* inst = dynamic_cast<Instruction*>(addr);
  if (inst && inst->opcode() == Opcode::BitCast &&
      inst->numOperands() == 1) {
    return canonicalMemoryAddressKey(inst->operand(0));
  }
  if (inst && inst->opcode() == Opcode::GEP) {
    LinearOffsetKey linear = buildGEPLinearOffsetKey(inst);
    std::string linearKey = linearOffsetExactKey(linear);
    if (!linearKey.empty())
      return "gep:" + linearKey;
  }
  return valueKey(addr);
}

bool isTrackedScalarMemoryValue(Value* value) {
  if (!value)
    return false;
  Type ty = value->type();
  return ty.isI32() || ty.isF32() || ty.isPtr();
}

std::string exactMemoryAddressKey(Value* addr, Type valueTy) {
  MemoryAliasInfo info = classifyMemory(addr);
  if (!info.valid || !info.precise)
    return {};
  if (!valueTy.isI32() && !valueTy.isF32() && !valueTy.isPtr())
    return {};
  std::string addressKey = canonicalMemoryAddressKey(addr);
  if (addressKey.empty())
    return {};
  return typeName(valueTy) + ":" + addressKey;
}

void eraseExactStoredValuesForRoot(MemoryVersionState& state,
                                   const std::string& preciseRootKey) {
  for (auto it = state.exactStoredValues.begin();
       it != state.exactStoredValues.end();) {
    if (it->second.preciseRootKey == preciseRootKey)
      it = state.exactStoredValues.erase(it);
    else
      ++it;
  }
}

void recordExactStoredValue(MemoryVersionState& state,
                            Value* addr,
                            Value* storedValue) {
  if (!addr || !storedValue || !isTrackedScalarMemoryValue(storedValue))
    return;
  MemoryAliasInfo info = classifyMemory(addr);
  if (!info.valid || !info.precise)
    return;
  std::string key = exactMemoryAddressKey(addr, storedValue->type());
  if (key.empty())
    return;
  eraseExactStoredValuesForRoot(state, info.preciseKey);
  state.exactStoredValues[key] = {storedValue, info.preciseKey};
}

Value* forwardedExactStoredValue(Instruction* inst,
                                 const MemoryVersionState& state) {
  if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
    return nullptr;
  std::string key = exactMemoryAddressKey(inst->operand(0), inst->type());
  if (key.empty())
    return nullptr;
  auto it = state.exactStoredValues.find(key);
  if (it == state.exactStoredValues.end())
    return nullptr;
  return it->second.value && it->second.value->type() == inst->type()
             ? it->second.value
             : nullptr;
}

void eraseExternalExactStoredValues(MemoryVersionState& state);
void eraseExternalPreciseVersions(MemoryVersionState& state);

void recordMemoryStore(MemoryVersionState& state, const MemoryAliasInfo& info,
                       int definitionToken) {
  if (!preciseModRefEnabled()) {
    if (info.precise)
      eraseExactStoredValuesForRoot(state, info.preciseKey);
    else
      state.exactStoredValues.clear();
    state.allStoreVersions = {definitionToken};
    if (info.precise)
      state.preciseVersions[info.preciseKey] = {definitionToken};
    else {
      state.globalBarriers = {definitionToken};
      state.preciseVersions.clear();
    }
    return;
  }
  if (info.precise) {
    eraseExactStoredValuesForRoot(state, info.preciseKey);
  } else if (info.rootKind == MemoryRootKind::Argument) {
    eraseExternalExactStoredValues(state);
  } else {
    state.exactStoredValues.clear();
  }
  state.allStoreVersions = {definitionToken};
  if (info.rootKind != MemoryRootKind::Alloca)
    state.externalStoreVersions = {definitionToken};
  if (info.precise) {
    state.preciseVersions[info.preciseKey] = {definitionToken};
  } else if (info.rootKind == MemoryRootKind::Argument) {
    state.externalBarriers = {definitionToken};
    eraseExternalPreciseVersions(state);
  } else {
    state.globalBarriers = {definitionToken};
    state.externalBarriers = {definitionToken};
    state.preciseVersions.clear();
  }
}

void eraseExternalExactStoredValues(MemoryVersionState& state) {
  for (auto it = state.exactStoredValues.begin();
       it != state.exactStoredValues.end();) {
    if (it->second.preciseRootKey.rfind("g:", 0) == 0)
      it = state.exactStoredValues.erase(it);
    else
      ++it;
  }
}

void eraseExternalPreciseVersions(MemoryVersionState& state) {
  for (auto it = state.preciseVersions.begin();
       it != state.preciseVersions.end();) {
    if (it->first.rfind("g:", 0) == 0)
      it = state.preciseVersions.erase(it);
    else
      ++it;
  }
}

void recordMemoryCall(MemoryVersionState& state, int definitionToken) {
  state.exactStoredValues.clear();
  state.allStoreVersions = {definitionToken};
  state.externalStoreVersions = {definitionToken};
  state.globalBarriers = {definitionToken};
  state.externalBarriers = {definitionToken};
  state.preciseVersions.clear();
}

std::string loadCSEKey(Instruction* inst, const MemoryVersionState& state) {
  if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
    return {};
  MemoryAliasInfo info = classifyMemory(inst->operand(0));
  if (!info.valid)
    return {};

  std::string addressKey = canonicalMemoryAddressKey(inst->operand(0));
  if (addressKey.empty())
    return {};
  std::string key =
      "load:" + typeName(inst->type()) + ":" + addressKey + ":";
  if (info.invariant) {
    key += "invariant";
    return key;
  }
  if (info.precise) {
    key += memoryVersionKey(state.globalBarriers);
    key += ":";
    if (preciseModRefEnabled() && info.rootKind == MemoryRootKind::Global) {
      key += memoryVersionKey(state.externalBarriers);
      key += ":";
    }
    key += memoryVersionKey(
        preciseMemoryVersion(state, info.preciseKey));
  } else if (preciseModRefEnabled() &&
             info.rootKind == MemoryRootKind::Argument) {
    key += "external-wild:";
    key += memoryVersionKey(state.externalStoreVersions);
  } else {
    key += "wild:";
    key += memoryVersionKey(state.allStoreVersions);
  }
  return key;
}

std::string callCSEKey(Instruction* inst, const Module& mod,
                       const analysis::FunctionEffectMap& effects,
                       const MemoryVersionState& state) {
  if (!inst || inst->opcode() != Opcode::Call)
    return {};
  const auto* summary = analysis::lookupFunctionEffect(effects, mod, inst->name());
  if (!summary || (!summary->pure && !summary->readonly))
    return {};

  std::string key = "call:" + inst->name() + ":" + typeName(inst->type()) + ":";
  for (auto* op : inst->operands()) {
    key += valueKey(op);
    key += ",";
  }

  if (summary->pure || summary->stableGlobalRead)
    return key;

  key += "|mem:";
  if (summary->argMemOnly) {
    for (int i = 0; i < inst->numOperands(); ++i) {
      Value* op = inst->operand(i);
      if (!op || !op->type().isPtr())
        continue;
      MemoryAliasInfo info = classifyMemory(op);
      key += "p" + std::to_string(i) + "=";
      if (!info.valid) {
        key += "unknown:" + memoryVersionKey(state.allStoreVersions);
      } else if (info.invariant) {
        key += "invariant";
      } else if (info.precise) {
        key += memoryVersionKey(state.globalBarriers);
        key += ":";
        if (preciseModRefEnabled() &&
            info.rootKind == MemoryRootKind::Global) {
          key += memoryVersionKey(state.externalBarriers);
          key += ":";
        }
        key += memoryVersionKey(
            preciseMemoryVersion(state, info.preciseKey));
      } else if (preciseModRefEnabled() &&
                 info.rootKind == MemoryRootKind::Argument) {
        key += "external-wild:" +
               memoryVersionKey(state.externalStoreVersions);
      } else {
        key += "wild:" + memoryVersionKey(state.allStoreVersions);
      }
      key += ";";
    }
    return key;
  }

  key += "global:";
  key += memoryVersionKey(state.globalBarriers);
  key += ":";
  key += memoryVersionKey(state.allStoreVersions);
  return key;
}

void clearLoadCSEEntries(std::unordered_map<std::string, Value*>& exprs) {
  for (auto it = exprs.begin(); it != exprs.end();) {
    if (it->first.rfind("load:", 0) == 0)
      it = exprs.erase(it);
    else
      ++it;
  }
}

bool rootIsLocalAlloca(Value* addr) {
  auto* inst = dynamic_cast<Instruction*>(memoryRoot(addr));
  return inst && inst->opcode() == Opcode::Alloca;
}

bool isReadonlyHelperCall(Instruction* inst, const Module& mod,
                          const analysis::FunctionEffectMap& effects) {
  return analysis::isReadonlyCall(inst, mod, effects);
}

bool callPreservesTrackedMemory(Instruction* inst,
                                const Module& mod,
                                const analysis::FunctionEffectMap& effects) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  if (analysis::isReadonlyCall(inst, mod, effects))
    return true;
  if (preciseModRefEnabled() &&
      (inst->name() == "putarray" || inst->name() == "putfarray"))
    return true;
  if (mod.isBuiltin(inst->name())) {
    for (int i = 0; i < inst->numOperands(); ++i) {
      if (inst->operand(i) && inst->operand(i)->type().isPtr())
        return false;
    }
    return true;
  }
  // 计时运行时会写自己的隐藏状态，但不接触用户 IR 里的数组和全局标量，
  // 因此允许后续针对用户内存的 load/store forwarding 保留精确状态。
  return inst->name() == "_sysy_starttime" || inst->name() == "_sysy_stoptime";
}

bool collectKnownCallWrites(
    Instruction* inst, const Module& mod,
    const analysis::FunctionEffectMap& effects,
    std::vector<Value*>& writtenPointers) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  if (inst->name() == "getarray" || inst->name() == "getfarray" ||
      inst->name() == "memset") {
    if (inst->numOperands() < 1 || !inst->operand(0) ||
        !inst->operand(0)->type().isPtr())
      return false;
    writtenPointers.push_back(inst->operand(0));
    return true;
  }
  const auto* summary =
      analysis::lookupFunctionEffect(effects, mod, inst->name());
  if (!summary || summary->writesUnknownMemory)
    return false;
  // 只处理全局状态的辅助函数通常组成紧密的状态机。跨过这类调用保留无关的
  // 全局加载会延长活跃区间，却不一定减少热点内存流量，因此保留原有调用
  // 边界；精细路径主要用于调用者能区分内存对象的指针处理例程。
  if (!summary->writesArgMemory && !summary->writtenGlobals.empty())
    return false;
  if (summary->writesArgMemory) {
    for (Value* operand : inst->operands()) {
      if (operand && operand->type().isPtr())
        writtenPointers.push_back(operand);
    }
  }
  for (const GlobalVar* global : summary->writtenGlobals)
    writtenPointers.push_back(const_cast<GlobalVar*>(global));
  return true;
}

bool callMayWriteMemory(Instruction* inst, const MemoryAliasInfo& target,
                        const Module& mod,
                        const analysis::FunctionEffectMap& effects) {
  if (!inst || inst->opcode() != Opcode::Call)
    return false;
  if (callPreservesTrackedMemory(inst, mod, effects))
    return false;
  if (!preciseModRefEnabled())
    return true;
  std::vector<Value*> writtenPointers;
  if (!collectKnownCallWrites(inst, mod, effects, writtenPointers))
    return true;
  for (Value* pointer : writtenPointers) {
    if (memoryMayAlias(classifyMemory(pointer), target))
      return true;
  }
  return false;
}

void recordMemoryCallEffects(MemoryVersionState& state, Instruction* inst,
                             const Module& mod,
                             const analysis::FunctionEffectMap& effects,
                             int definitionToken) {
  if (callPreservesTrackedMemory(inst, mod, effects))
    return;
  if (!preciseModRefEnabled()) {
    recordMemoryCall(state, definitionToken);
    return;
  }
  std::vector<Value*> writtenPointers;
  if (!collectKnownCallWrites(inst, mod, effects, writtenPointers)) {
    recordMemoryCall(state, definitionToken);
    return;
  }
  for (Value* pointer : writtenPointers)
    recordMemoryStore(state, classifyMemory(pointer), definitionToken);
}

bool functionMayObserveScalarGlobal(Function* func, GlobalVar* gv,
                                    Module& mod,
                                    std::unordered_set<Function*>& visiting);

bool callMayObserveScalarGlobal(Instruction* call, GlobalVar* gv,
                                Module& mod,
                                std::unordered_set<Function*>& visiting) {
  if (!call || call->opcode() != Opcode::Call || !gv)
    return true;
  if (call->name() == "_sysy_starttime" ||
      call->name() == "_sysy_stoptime") {
    return false;
  }

  for (Value* operand : call->operands()) {
    if (!operand || !operand->type().isPtr())
      continue;
    Value* root = memoryRoot(operand);
    if (root == gv)
      return true;
    if (!dynamic_cast<GlobalVar*>(root)) {
      auto* rootInst = dynamic_cast<Instruction*>(root);
      if (!rootInst || rootInst->opcode() != Opcode::Alloca)
        return true;
    }
  }

  if (mod.isBuiltin(call->name()) || call->name() == "memset")
    return false;
  Function* callee = mod.getFunction(call->name());
  if (!callee || !callee->entryBlock())
    return true;
  return functionMayObserveScalarGlobal(callee, gv, mod, visiting);
}

bool functionMayObserveScalarGlobal(Function* func, GlobalVar* gv,
                                    Module& mod,
                                    std::unordered_set<Function*>& visiting) {
  if (!func || !gv)
    return true;
  if (!visiting.insert(func).second)
    return false;
  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() == Opcode::Call) {
        if (callMayObserveScalarGlobal(inst, gv, mod, visiting)) {
          visiting.erase(func);
          return true;
        }
        continue;
      }
      Value* addr = nullptr;
      if (inst->opcode() == Opcode::Load && inst->numOperands() == 1)
        addr = inst->operand(0);
      else if (inst->opcode() == Opcode::Store && inst->numOperands() == 2)
        addr = inst->operand(1);
      if (addr && memoryRoot(addr) == gv) {
        visiting.erase(func);
        return true;
      }
    }
  }
  visiting.erase(func);
  return false;
}

bool isSmallScalarGlobalRoot(Value* value) {
  auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(value));
  return gv && gv->sizeBytes() == 4 && gv->dims().empty();
}

bool forwardSingleAssignmentScalarGlobals(
    Function* func,
    Module& mod,
    const analysis::FunctionEffectMap& effects) {
  if (!func || !func->entryBlock())
    return false;

  BasicBlock* entry = func->entryBlock();
  std::unordered_map<GlobalVar*, Instruction*> uniqueStore;
  std::unordered_map<GlobalVar*, Value*> storedValue;
  std::unordered_set<GlobalVar*> rejected;

  for (auto& bbPtr : func->blocks()) {
    for (auto& instPtr : bbPtr->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (inst->opcode() == Opcode::Call &&
          !callPreservesTrackedMemory(inst, mod, effects)) {
        return false;
      }
      if (inst->opcode() != Opcode::Store || inst->numOperands() != 2)
        continue;
      auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(1)));
      if (!gv || !isSmallScalarGlobalRoot(gv))
        continue;
      if (rejected.count(gv))
        continue;
      auto it = uniqueStore.find(gv);
      if (it != uniqueStore.end()) {
        uniqueStore.erase(it);
        storedValue.erase(gv);
        rejected.insert(gv);
        continue;
      }
      uniqueStore[gv] = inst;
      storedValue[gv] = inst->operand(0);
    }
  }

  if (uniqueStore.empty())
    return false;

  std::unordered_map<Instruction*, size_t> entryOrder;
  for (size_t i = 0; i < entry->instructions().size(); ++i)
    entryOrder[entry->instructions()[i].get()] = i;

  std::unordered_map<GlobalVar*, Value*> candidates;
  for (const auto& [gv, store] : uniqueStore) {
    if (!gv || !store || store->parent() != entry)
      continue;
    auto orderIt = entryOrder.find(store);
    if (orderIt == entryOrder.end())
      continue;
    bool seenEarlierLoad = false;
    size_t storeIndex = orderIt->second;
    for (size_t i = 0; i < storeIndex; ++i) {
      Instruction* inst = entry->instructions()[i].get();
      if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
        continue;
      auto* loadGv = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(0)));
      if (loadGv == gv) {
        seenEarlierLoad = true;
        break;
      }
    }
    if (!seenEarlierLoad)
      candidates[gv] = storedValue.at(gv);
  }

  if (candidates.empty())
    return false;

  bool changed = false;
  for (auto& bbPtr : func->blocks()) {
    BasicBlock* bb = bbPtr.get();
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
        continue;
      auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(0)));
      auto it = gv ? candidates.find(gv) : candidates.end();
      if (it == candidates.end())
        continue;
      if (bb == entry) {
        auto loadOrderIt = entryOrder.find(inst);
        auto storeOrderIt = entryOrder.find(uniqueStore.at(gv));
        if (loadOrderIt == entryOrder.end() || storeOrderIt == entryOrder.end() ||
            loadOrderIt->second <= storeOrderIt->second) {
          continue;
        }
      }
      inst->replaceAllUsesWith(it->second);
      changed = true;
    }
  }
  return changed;
}

int64_t constantByteOffsetFromRoot(Value* addr, GlobalVar* expectedRoot) {
  if (!addr || !expectedRoot)
    return std::numeric_limits<int64_t>::min();
  if (addr == expectedRoot)
    return 0;
  auto* inst = dynamic_cast<Instruction*>(addr);
  if (!inst)
    return std::numeric_limits<int64_t>::min();
  if (inst->opcode() == Opcode::BitCast && inst->numOperands() == 1)
    return constantByteOffsetFromRoot(inst->operand(0), expectedRoot);
  if (inst->opcode() != Opcode::GEP || inst->numOperands() < 1)
    return std::numeric_limits<int64_t>::min();

  int64_t base = constantByteOffsetFromRoot(inst->operand(0), expectedRoot);
  if (base == std::numeric_limits<int64_t>::min())
    return base;
  for (int i = 1; i < inst->numOperands(); ++i) {
    int64_t operandOffset = 0;
    if (!evaluateConstIntExpression(inst->operand(i), operandOffset))
      return std::numeric_limits<int64_t>::min();
    // GlobalOpt/InstCombine 常量表转发需要识别 `gep @T,
    // shl(add const,const),2` 这类展开后仍未清理干净的常量 offset。
    // 这里递归求值只接受纯整数常量表达式。
    base += operandOffset;
  }
  return base;
}

bool forwardConstantSmallGlobalArrayLoads(
    Module& mod, const analysis::FunctionEffectMap& effects) {
  constexpr int kMaxForwardBytes = 256;

  enum class CellKind {
    Top,
    Unknown,
    Constant,
  };
  struct Cell {
    CellKind kind = CellKind::Top;
    int32_t value = 0;

    bool operator==(const Cell& other) const {
      return kind == other.kind &&
             (kind != CellKind::Constant || value == other.value);
    }
  };

  using State = std::unordered_map<GlobalVar*, std::vector<Cell>>;
  std::vector<GlobalVar*> candidates;
  for (auto& gvPtr : mod.globals()) {
    GlobalVar* gv = gvPtr.get();
    if (!gv || gv->isConst() || gv->dims().empty() ||
        !gv->elemTy().isI32() || gv->sizeBytes() <= 0 ||
        gv->sizeBytes() > kMaxForwardBytes) {
      continue;
    }
    candidates.push_back(gv);
  }
  if (candidates.empty())
    return false;

  // 数据流辅助 lambda：按 4 字节 cell 建格构造状态、相等比较与逐格 meet
  // （Top 吸收、相同常量保留、其余降为 Unknown），以及转移函数
  // （Call/VStore 保守清空全部，Store 仅在偏移精确时更新单格）。
  auto makeState = [&](CellKind kind) {
    State state;
    for (GlobalVar* gv : candidates) {
      state[gv] = std::vector<Cell>(
          static_cast<size_t>(gv->sizeBytes() / 4), Cell{kind, 0});
    }
    return state;
  };
  auto setUnknown = [](std::vector<Cell>& cells) {
    for (Cell& cell : cells)
      cell = Cell{CellKind::Unknown, 0};
  };
  auto clearAll = [&](State& state) {
    for (GlobalVar* gv : candidates)
      setUnknown(state[gv]);
  };
  auto isCandidate = [&](GlobalVar* gv) {
    return std::find(candidates.begin(), candidates.end(), gv) !=
           candidates.end();
  };
  auto statesEqual = [&](const State& lhs, const State& rhs) {
    for (GlobalVar* gv : candidates) {
      auto lhsIt = lhs.find(gv);
      auto rhsIt = rhs.find(gv);
      if (lhsIt == lhs.end() || rhsIt == rhs.end() ||
          lhsIt->second != rhsIt->second) {
        return false;
      }
    }
    return true;
  };
  auto meetCell = [](const Cell& lhs, const Cell& rhs) {
    if (lhs.kind == CellKind::Top)
      return rhs;
    if (rhs.kind == CellKind::Top)
      return lhs;
    if (lhs.kind == CellKind::Constant &&
        rhs.kind == CellKind::Constant && lhs.value == rhs.value) {
      return lhs;
    }
    return Cell{CellKind::Unknown, 0};
  };
  auto meetInto = [&](State& dst, const State& src) {
    for (GlobalVar* gv : candidates) {
      auto& dstCells = dst[gv];
      const auto& srcCells = src.at(gv);
      for (size_t i = 0; i < dstCells.size(); ++i)
        dstCells[i] = meetCell(dstCells[i], srcCells[i]);
    }
  };
  auto applyTransfer = [&](State& state, Instruction* inst) {
    if (!inst)
      return;
    if (inst->opcode() == Opcode::Call) {
      if (!callPreservesTrackedMemory(inst, mod, effects))
        clearAll(state);
      return;
    }
    if (inst->opcode() == Opcode::VStore) {
      clearAll(state);
      return;
    }
    if (inst->opcode() != Opcode::Store || inst->numOperands() != 2)
      return;

    Value* addr = inst->operand(1);
    Value* root = memoryRoot(addr);
    auto* gv = dynamic_cast<GlobalVar*>(root);
    if (gv && isCandidate(gv)) {
      int64_t offset = constantByteOffsetFromRoot(addr, gv);
      if (offset < 0 || offset % 4 != 0 ||
          offset + 4 > gv->sizeBytes()) {
        setUnknown(state[gv]);
        return;
      }
      size_t index = static_cast<size_t>(offset / 4);
      auto* ci = dynamic_cast<ConstInt*>(inst->operand(0));
      state[gv][index] = ci ? Cell{CellKind::Constant, ci->value()}
                            : Cell{CellKind::Unknown, 0};
      return;
    }

    auto* rootInst = dynamic_cast<Instruction*>(root);
    if (!gv && (!rootInst || rootInst->opcode() != Opcode::Alloca))
      clearAll(state);
  };

  bool changed = false;
  std::vector<Instruction*> deadLoads;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || !func->entryBlock())
      continue;
    func->recomputeCFG();

    std::unordered_map<BasicBlock*, State> inStates;
    std::unordered_map<BasicBlock*, State> outStates;
    for (auto& bbPtr : func->blocks()) {
      inStates.emplace(bbPtr.get(), makeState(CellKind::Top));
      outStates.emplace(bbPtr.get(), makeState(CellKind::Top));
    }

    const size_t maxIterations =
        std::max<size_t>(8, func->blocks().size() * 4);
    for (size_t iteration = 0; iteration < maxIterations; ++iteration) {
      bool iterationChanged = false;
      for (auto& bbPtr : func->blocks()) {
        BasicBlock* bb = bbPtr.get();
        State nextIn = makeState(CellKind::Top);
        if (bb == func->entryBlock()) {
          nextIn = makeState(CellKind::Unknown);
          if (func->name() == "main") {
            for (GlobalVar* gv : candidates) {
              auto& cells = nextIn[gv];
              for (size_t i = 0; i < cells.size(); ++i) {
                int32_t initial =
                    i < gv->initData.size() ? gv->initData[i] : 0;
                cells[i] = Cell{CellKind::Constant, initial};
              }
            }
          }
        } else if (bb->preds().empty()) {
          nextIn = makeState(CellKind::Unknown);
        } else {
          for (BasicBlock* pred : bb->preds())
            meetInto(nextIn, outStates.at(pred));
        }

        State nextOut = nextIn;
        for (auto& instPtr : bb->instructions())
          applyTransfer(nextOut, instPtr.get());
        if (!statesEqual(inStates.at(bb), nextIn)) {
          inStates[bb] = nextIn;
          iterationChanged = true;
        }
        if (!statesEqual(outStates.at(bb), nextOut)) {
          outStates[bb] = nextOut;
          iterationChanged = true;
        }
      }
      if (!iterationChanged)
        break;
    }

    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      State state = inStates.at(bb);
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (inst && inst->opcode() == Opcode::Load &&
            inst->numOperands() == 1) {
          Value* addr = inst->operand(0);
          auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(addr));
          if (gv && isCandidate(gv)) {
            int64_t offset = constantByteOffsetFromRoot(addr, gv);
            if (offset >= 0 && offset % 4 == 0 &&
                offset + 4 <= gv->sizeBytes()) {
              const Cell& cell =
                  state.at(gv)[static_cast<size_t>(offset / 4)];
              if (cell.kind == CellKind::Constant) {
                inst->replaceAllUsesWith(mod.getConstInt(cell.value));
                deadLoads.push_back(inst);
                changed = true;
              }
            }
          }
        }
        applyTransfer(state, inst);
      }
    }
  }

  for (Instruction* inst : deadLoads) {
    if (inst && inst->parent())
      inst->parent()->remove(inst);
  }
  return changed;
}

bool isReadOnlyInitializedScalarGlobal(GlobalVar* global) {
  if (!global || !global->hasInit() || !global->elemTy().isI32() ||
      global->sizeBytes() != 4 || global->initData.size() != 1)
    return false;
  for (Instruction* user : global->uses()) {
    if (!user || user->opcode() != Opcode::Load ||
        user->numOperands() != 1 || user->operand(0) != global) {
      return false;
    }
  }
  return true;
}

bool functionHasCFGCycle(Function* func) {
  if (!func)
    return false;
  std::unordered_map<BasicBlock*, unsigned> state;
  std::function<bool(BasicBlock*)> visit = [&](BasicBlock* bb) {
    if (!bb)
      return false;
    if (state[bb] == 1)
      return true;
    if (state[bb] == 2)
      return false;
    state[bb] = 1;
    for (BasicBlock* succ : bb->succs()) {
      if (visit(succ))
        return true;
    }
    state[bb] = 2;
    return false;
  };
  for (const auto& bbPtr : func->blocks()) {
    if (visit(bbPtr.get()))
      return true;
  }
  return false;
}

// 将只读、已初始化的 i32 标量全局的 load 直接折叠为 initializer 常量；
// CFG 无环时先统计被多个调用喂值的 load 并跳过相应全局，避免干扰跨调用
// 的值传递。
bool forwardConstGlobalLoads(Module& mod) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    const bool hasCFGCycle = functionHasCFGCycle(funcPtr.get());
    std::unordered_map<GlobalVar*, unsigned> callFedLoadCounts;
    if (!hasCFGCycle) {
      for (auto& bbPtr : funcPtr->blocks()) {
        for (auto& instPtr : bbPtr->instructions()) {
          Instruction* load = instPtr.get();
          if (!load || load->opcode() != Opcode::Load ||
              load->numOperands() != 1)
            continue;
          auto* global =
              dynamic_cast<GlobalVar*>(memoryRoot(load->operand(0)));
          if (!global || global->isConst())
            continue;
          for (Instruction* user : load->uses()) {
            if (user && user->opcode() == Opcode::Call)
              ++callFedLoadCounts[global];
          }
        }
      }
    }
    for (auto& bbPtr : funcPtr->blocks()) {
      std::vector<Instruction*> deadLoads;
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Load || inst->numOperands() != 1)
          continue;
        auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(0)));
        const bool sharedAcrossCalls =
            gv && callFedLoadCounts[gv] > 1;
        if (!gv || (!gv->isConst() &&
                    (sharedAcrossCalls ||
                     !isReadOnlyInitializedScalarGlobal(gv))) ||
            !gv->hasInit() || !gv->elemTy().isI32())
          continue;
        int64_t offset = constantByteOffsetFromRoot(inst->operand(0), gv);
        if (offset < 0 || offset % 4 != 0)
          continue;
        int64_t index = offset / 4;
        if (index < 0 || index >= static_cast<int64_t>(gv->initData.size()))
          continue;
        // 这是 SCCP/GlobalOpt 风格的 const global load folding。
        // 只有只读全局、常量偏移和已知 initializer 同时成立时才替换；
        // 它会把 loop bound、查表常量等交给后续 CSE/LoopUnroll 消费。
        inst->replaceAllUsesWith(mod.getConstInt(gv->initData[index]));
        deadLoads.push_back(inst);
      }
      for (Instruction* inst : deadLoads) {
        if (inst && inst->parent()) {
          inst->parent()->remove(inst);
          changed = true;
        }
      }
    }
  }
  return changed;
}

bool deadPureCallElimination(Module& mod,
                             analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  if (effects.empty())
    return false;

  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    for (auto& bbPtr : funcPtr->blocks()) {
      auto& insts = bbPtr->instructions();
      for (auto it = insts.begin(); it != insts.end();) {
        Instruction* inst = it->get();
        Function* callee = inst && inst->opcode() == Opcode::Call
                               ? mod.getFunction(inst->name())
                               : nullptr;
        const analysis::FunctionSemanticSummary* semantics =
            callee ? &analysisManager.functionSemantics(callee) : nullptr;
        if (!analysis::isPureCall(inst, mod, effects) ||
            !inst->uses().empty() || !semantics || !semantics->total ||
            !semantics->nonTrapping) {
          ++it;
          continue;
        }
        inst->dropAllOperands();
        it = insts.erase(it);
        changed = true;
      }
    }
  }
  return changed;
}

bool isObservableGlobalUse(Instruction* inst, int operandIndex) {
  if (!inst || operandIndex < 0 || operandIndex >= inst->numOperands())
    return false;
  switch (inst->opcode()) {
  case Opcode::Store:
    // store 的地址操作数本身不是读取；若这个全局对象没有其它
    // 可观察用途，写入它就是 Dead Store Elimination 可以删除的目标。
    return operandIndex != 1;
  case Opcode::GEP:
  case Opcode::BitCast:
  case Opcode::Phi:
    // 指针 PHI 只传递地址，加载、调用、返回或存储地址等实际用途由本轮扫描
    // 单独检查；把 PHI 本身当作可观察用途会使循环携带的无效存储继续保留。
    return false;
  default:
    // load、call 参数、return 或其它未知用途都会让地址逃逸或
    // 被读取，因此按 MemorySSA/DSE 的保守规则把整块全局对象标为可观察。
    return true;
  }
}

bool removeUnobservedScalarGlobalStores(Module& mod) {
  Function* mainFunc = mod.getFunction("main");
  if (!mainFunc)
    return false;

  std::unordered_map<std::string, Function*> nameToFunc;
  for (auto& funcPtr : mod.functions()) {
    if (funcPtr)
      nameToFunc[funcPtr->name()] = funcPtr.get();
  }

  std::unordered_set<Function*> reachableFuncs;
  std::vector<Function*> stack{mainFunc};
  while (!stack.empty()) {
    Function* func = stack.back();
    stack.pop_back();
    if (!func || !reachableFuncs.insert(func).second)
      continue;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Call || inst->name().empty())
          continue;
        auto it = nameToFunc.find(inst->name());
        if (it != nameToFunc.end())
          stack.push_back(it->second);
      }
    }
  }

  std::unordered_set<GlobalVar*> observedGlobals;
  for (Function* func : reachableFuncs) {
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst)
          continue;
        for (int i = 0; i < inst->numOperands(); ++i) {
          auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(i)));
          if (!gv || gv->isConst())
            continue;
          if (isObservableGlobalUse(inst, i))
            observedGlobals.insert(gv);
        }
      }
    }
  }

  bool changed = false;
  for (Function* func : reachableFuncs) {
    for (auto& bbPtr : func->blocks()) {
      BasicBlock* bb = bbPtr.get();
      std::vector<Instruction*> deadStores;
      for (auto& instPtr : bb->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst || inst->opcode() != Opcode::Store || inst->numOperands() != 2)
          continue;
        auto* gv = dynamic_cast<GlobalVar*>(memoryRoot(inst->operand(1)));
        if (!gv || gv->isConst())
          continue;
        if (observedGlobals.count(gv))
          continue;
        deadStores.push_back(inst);
      }
      for (Instruction* inst : deadStores) {
        if (inst->parent() == bb) {
          bb->remove(inst);
          changed = true;
        }
      }
    }
  }
  return changed;
}

namespace {

bool canReachBlock(BasicBlock* from, BasicBlock* target,
                   const std::unordered_set<BasicBlock*>& reachable) {
  if (!from || !target || !reachable.count(from) || !reachable.count(target))
    return false;
  std::queue<BasicBlock*> worklist;
  std::unordered_set<BasicBlock*> visited;
  worklist.push(from);
  visited.insert(from);
  while (!worklist.empty()) {
    BasicBlock* block = worklist.front();
    worklist.pop();
    if (block == target)
      return true;
    for (BasicBlock* successor : block->succs()) {
      if (successor && reachable.count(successor) &&
          visited.insert(successor).second)
        worklist.push(successor);
    }
  }
  return false;
}

bool hasScalarGlobalClobberBetween(
    Instruction* load, Instruction* store, GlobalVar* global,
    const analysis::DomSet& doms,
    const std::unordered_set<BasicBlock*>& reachable) {
  if (!load || !store || !global || !load->parent() || !store->parent() ||
      !analysis::dominates(doms, load->parent(), store->parent()))
    return true;

  if (load->parent() == store->parent()) {
    bool loadSeen = false;
    for (auto& instPtr : load->parent()->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst == load) {
        loadSeen = true;
        continue;
      }
      if (inst == store)
        break;
      if (!loadSeen)
        continue;
      if (inst->opcode() == Opcode::Call ||
          inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Store)
        return true;
    }
    return !loadSeen;
  }

  for (BasicBlock* block : reachable) {
    if (!analysis::dominates(doms, load->parent(), block) ||
        !canReachBlock(block, store->parent(), reachable))
      continue;

    bool afterLoad = block != load->parent();
    for (auto& instPtr : block->instructions()) {
      Instruction* inst = instPtr.get();
      if (!inst)
        continue;
      if (block == load->parent() && !afterLoad) {
        if (inst == load)
          afterLoad = true;
        continue;
      }
      if (block == store->parent() && inst == store)
        break;
      if (inst->opcode() == Opcode::Call ||
          inst->opcode() == Opcode::VStore ||
          inst->opcode() == Opcode::Store)
        return true;
    }
  }
  return false;
}

} // namespace

// 识别"Load 读出的值随即存回同一全局标量"的 reload 模式，并在 load 与
// store 之间无该全局的其他写入（经支配与可达性校验）时删除冗余 store。
bool removeRedundantScalarGlobalReloadStores(
    Module& mod, analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || !func->entryBlock())
      continue;
    auto& cache = analysisManager.function(func);
    const auto& reachable = cache.reachable();
    const auto& doms = cache.dominators();
    std::vector<Instruction*> redundant;
    for (BasicBlock* block : reachable) {
      if (!block)
        continue;
      for (auto& instPtr : block->instructions()) {
        Instruction* store = instPtr.get();
        if (!store || store->opcode() != Opcode::Store ||
            store->numOperands() != 2)
          continue;
        auto* loaded = dynamic_cast<Instruction*>(store->operand(0));
        if (!loaded || loaded->opcode() != Opcode::Load ||
            loaded->numOperands() != 1)
          continue;
        auto* loadGlobal =
            dynamic_cast<GlobalVar*>(memoryRoot(loaded->operand(0)));
        auto* storeGlobal =
            dynamic_cast<GlobalVar*>(memoryRoot(store->operand(1)));
        if (!loadGlobal || loadGlobal != storeGlobal ||
            !isSmallScalarGlobalRoot(loadGlobal) || loadGlobal->isConst())
          continue;
        if (hasScalarGlobalClobberBetween(loaded, store, loadGlobal, doms,
                                          reachable))
          continue;
        redundant.push_back(store);
      }
    }
    for (Instruction* store : redundant) {
      BasicBlock* block = store ? store->parent() : nullptr;
      if (!block)
        continue;
      store->dropAllOperands();
      block->remove(store);
      changed = true;
    }
  }
  return changed;
}

bool promoteDirectScalarGlobalsToEntryLocals(
    Module& mod,
    analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  const bool tracePromotion =
      sycEnvironmentValue("SYC_TRACE_COREIR_GLOBAL_SCALAR_PROMOTION") != nullptr;

  for (auto& funcPtr : mod.functions()) {
    Function* func = funcPtr.get();
    if (!func || !func->entryBlock())
      continue;

    struct CandidateInfo {
      GlobalVar* gv = nullptr;
      std::vector<Instruction*> loads;
      std::vector<Instruction*> stores;
      bool hasLoopLoad = false;
      bool hasLoopStore = false;
      bool rejected = false;
    };

    std::unordered_set<BasicBlock*> loopBlocks;
    for (const auto& loop : analysisManager.function(func).loops())
      loopBlocks.insert(loop.blocks.begin(), loop.blocks.end());

    std::unordered_set<std::string> existingShadowNames;
    for (Instruction* alloca : func->allocas()) {
      if (!alloca)
        continue;
      existingShadowNames.insert(alloca->name());
    }

    std::unordered_map<GlobalVar*, CandidateInfo> candidates;
    std::vector<Instruction*> calls;
    for (auto& bbPtr : func->blocks()) {
      for (auto& instPtr : bbPtr->instructions()) {
        Instruction* inst = instPtr.get();
        if (!inst)
          continue;

        if (inst->opcode() == Opcode::Call) {
          calls.push_back(inst);
          continue;
        }

        if ((inst->opcode() != Opcode::Load && inst->opcode() != Opcode::Store) ||
            inst->numOperands() < 1) {
          continue;
        }

        Value* addr = inst->opcode() == Opcode::Load ? inst->operand(0)
                                                     : inst->operand(1);
        Value* root = memoryRoot(addr);
        auto* gv = dynamic_cast<GlobalVar*>(root);
        if (!gv)
          continue;
        if (gv->sizeBytes() != 4 || !gv->dims().empty() ||
            !gv->elemTy().isI32() || gv->isConst()) {
          continue;
        }
        if (addr != gv) {
          candidates[gv].rejected = true;
          continue;
        }

        CandidateInfo& info = candidates[gv];
        info.gv = gv;
        if (inst->opcode() == Opcode::Load) {
          info.loads.push_back(inst);
          info.hasLoopLoad |= loopBlocks.count(inst->parent()) != 0;
        } else {
          info.stores.push_back(inst);
          info.hasLoopStore |= loopBlocks.count(inst->parent()) != 0;
        }
      }
    }
    // 把全局标量提升成本地 alloca 后，后续 mem2reg 会把它变成
    // 跨基本块的 SSA 值。含调用的函数仍须通过上面的 effect 证明，确认调用
    // 不会观察目标全局；跨调用 live-range 则由 MIR 的 call-aware 分配处理。
    if (candidates.empty())
      continue;
    if (!calls.empty() &&
        sycEnvironmentValue("SYC_SKIP_COREIR_CROSS_CALL_GLOBAL_PROMOTION")) {
      continue;
    }

    int definedCalleeCalls = 0;
    for (Instruction* call : calls) {
      Function* callee = call ? mod.getFunction(call->name()) : nullptr;
      if (callee && callee->entryBlock())
        ++definedCalleeCalls;
    }

    // 逐候选过滤：已本地化或 shadow 名已存在的跳过；存在多个已定义调用
    // 且访存次数不足 4 时收益过低跳过；任一调用链可能读写该全局则跳过。
    bool funcChanged = false;
    for (auto& [gv, info] : candidates) {
      if (!gv || info.rejected) {
        if (tracePromotion && gv)
          std::fprintf(stderr, "[global-scalar] reject func=%s global=%s reason=indirect\n",
                       func->name().c_str(), gv->name().c_str());
        continue;
      }
      if (func->hasLocalizedGlobal(gv->name())) {
        if (tracePromotion)
          std::fprintf(stderr,
                       "[global-scalar] reject func=%s global=%s reason=already-localized\n",
                       func->name().c_str(), gv->name().c_str());
        continue;
      }
      if (existingShadowNames.count(gv->name() + ".shadow"))
        continue;
      if (info.loads.empty() && info.stores.empty())
        continue;
      // 循环中的一组直接读写会在运行时重复执行，因此即使静态指令只有
      // load/store 各一条，也值得交给 mem2reg 形成循环携带的 SSA 值。
      const bool repeatedLoopAccess =
          info.hasLoopLoad && info.hasLoopStore &&
          !sycEnvironmentValue("SYC_SKIP_COREIR_LOOP_GLOBAL_SCALAR_PROMOTION");
      if (!calls.empty() &&
          (definedCalleeCalls > 1 ||
           (info.loads.size() + info.stores.size() < 4 &&
            !repeatedLoopAccess))) {
        continue;
      }
      bool observedByCall = false;
      for (Instruction* call : calls) {
        std::unordered_set<Function*> visiting;
        if (callMayObserveScalarGlobal(call, gv, mod, visiting)) {
          if (tracePromotion)
            std::fprintf(stderr,
                         "[global-scalar] reject func=%s global=%s reason=call callee=%s\n",
                         func->name().c_str(), gv->name().c_str(),
                         call->name().c_str());
          observedByCall = true;
          break;
        }
      }
      if (observedByCall)
        continue;
      if (tracePromotion)
        std::fprintf(stderr, "[global-scalar] apply func=%s global=%s\n",
                     func->name().c_str(), gv->name().c_str());
      func->markLocalizedGlobal(gv->name());

      // 这里只做最保守的标量全局本地化。
      // 要求访问必须是直接 @gv 的 load/store，函数中的调用也必须可证明
      // 不会读写这类用户内存，然后再交给现有 mem2reg 吃成 SSA。
      Instruction* slot = createScalarSlotAllocaInEntry(
          func, gv->elemTy(), gv->name() + ".shadow");
      func->addAlloca(slot);

      Instruction* firstNonAlloca = nullptr;
      for (auto& instPtr : func->entryBlock()->instructions()) {
        if (instPtr.get() == slot)
          continue;
        if (instPtr->opcode() != Opcode::Alloca) {
          firstNonAlloca = instPtr.get();
          break;
        }
      }

      auto* initLoad = new Instruction(Opcode::Load, gv->elemTy(),
                                       gv->name() + ".shadow.init");
      initLoad->addOperand(gv);
      if (firstNonAlloca)
        func->entryBlock()->insertBefore(firstNonAlloca, initLoad);
      else
        func->entryBlock()->pushBack(initLoad);

      auto* initStore = new Instruction(Opcode::Store, Type::voidTy());
      initStore->addOperand(initLoad);
      initStore->addOperand(slot);
      if (firstNonAlloca)
        func->entryBlock()->insertBefore(firstNonAlloca, initStore);
      else
        func->entryBlock()->pushBack(initStore);

      for (Instruction* load : info.loads) {
        if (!load || !load->parent())
          continue;
        auto* localLoad = new Instruction(Opcode::Load, load->type(),
                                          gv->name() + ".shadow.load");
        localLoad->addOperand(slot);
        load->parent()->insertBefore(load, localLoad);
        load->replaceAllUsesWith(localLoad);
        load->parent()->remove(load);
      }

      for (Instruction* store : info.stores) {
        if (!store || !store->parent())
          continue;
        auto* localStore = new Instruction(Opcode::Store, Type::voidTy());
        localStore->addOperand(store->operand(0));
        localStore->addOperand(slot);
        store->parent()->insertBefore(store, localStore);
        store->parent()->remove(store);
      }

      for (auto& bbPtr : func->blocks()) {
        BasicBlock* bb = bbPtr.get();
        Instruction* term = bb ? bb->terminator() : nullptr;
        if (!bb || !term || term->opcode() != Opcode::Ret)
          continue;
        auto* flushLoad = new Instruction(Opcode::Load, gv->elemTy(),
                                          gv->name() + ".shadow.flush");
        flushLoad->addOperand(slot);
        bb->insertBefore(term, flushLoad);
        auto* flushStore = new Instruction(Opcode::Store, Type::voidTy());
        flushStore->setName(gv->name() + ".shadow.flush.store");
        flushStore->addOperand(flushLoad);
        flushStore->addOperand(gv);
        bb->insertBefore(term, flushStore);
      }
      funcChanged = true;
    }

    if (funcChanged) {
      changed = true;
      analysisManager.invalidateFunction(
          func, analysis::AnalysisInvalidation::CFG);
    }
  }
  return changed;
}

// 基本块内 CSE：Load 先做精确存储转发再按内存版本键 CSE；Call 按副作用
// 键 CSE 或更新内存版本；Store 记录版本与精确存储值；其余指令按普通
// 表达式键去重。
bool basicBlockCSE(Module& mod, analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  bool changed = false;
  for (auto& func : mod.functions()) {
    for (auto& bb : func->blocks()) {
      std::unordered_map<std::string, Value*> exprs;
      MemoryVersionState memState;
      for (auto& instPtr : bb->instructions()) {
        auto* inst = instPtr.get();
        if (inst->opcode() == Opcode::Load) {
          if (Value* forwarded = forwardedExactStoredValue(inst, memState)) {
            inst->replaceAllUsesWith(forwarded);
            changed = true;
            continue;
          }
          std::string key = loadCSEKey(inst, memState);
          if (!key.empty()) {
            auto it = exprs.find(key);
            if (it != exprs.end()) {
              inst->replaceAllUsesWith(it->second);
              changed = true;
            } else {
              exprs[key] = inst;
            }
          }
          continue;
        }
        if (inst->opcode() == Opcode::Call) {
          std::string key = callCSEKey(inst, mod, effects, memState);
          if (!key.empty()) {
            auto it = exprs.find(key);
            if (it != exprs.end()) {
              inst->replaceAllUsesWith(it->second);
              changed = true;
            } else {
              exprs[key] = inst;
            }
            continue;
          }
          if (callPreservesTrackedMemory(inst, mod, effects))
            continue;
          recordMemoryCallEffects(memState, inst, mod, effects, inst->id());
          continue;
        }
        if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
          recordMemoryStore(memState, classifyMemory(inst->operand(1)),
                            inst->id());
          recordExactStoredValue(memState, inst->operand(1), inst->operand(0));
          continue;
        }
        if (!isCSECandidate(inst)) continue;

        std::string key = cseKey(inst);
        auto it = exprs.find(key);
        if (it != exprs.end()) {
          inst->replaceAllUsesWith(it->second);
          changed = true;
        } else {
          exprs[key] = inst;
        }
      }
    }
  }
  return changed;
}


bool dominates(const DomSet& doms, BasicBlock* a, BasicBlock* b) {
  auto it = doms.find(b);
  return it != doms.end() && it->second.count(a);
}

int instructionCount(Function* func) {
  if (!func) return 0;
  int count = 0;
  for (auto& bb : func->blocks())
    count += static_cast<int>(bb->instructions().size());
  return count;
}

std::vector<BasicBlock*> reachableCachedBlocks(
    analysis::FunctionAnalysisCache& cache) {
  std::vector<BasicBlock*> blocks;
  const auto& reachable = cache.reachable();
  for (auto* bb : cache.blocks()) {
    if (reachable.count(bb))
      blocks.push_back(bb);
  }
  return blocks;
}

void sortUniqueVersions(std::vector<int>& versions) {
  std::sort(versions.begin(), versions.end());
  versions.erase(std::unique(versions.begin(), versions.end()), versions.end());
}

void unionVersions(std::vector<int>& destination,
                   const std::vector<int>& source) {
  destination.insert(destination.end(), source.begin(), source.end());
  sortUniqueVersions(destination);
}

bool memoryStatesEqual(const MemoryVersionState& lhs,
                       const MemoryVersionState& rhs) {
  if (lhs.globalBarriers != rhs.globalBarriers ||
      lhs.allStoreVersions != rhs.allStoreVersions ||
      lhs.externalStoreVersions != rhs.externalStoreVersions ||
      lhs.externalBarriers != rhs.externalBarriers ||
      lhs.preciseVersions.size() != rhs.preciseVersions.size() ||
      lhs.exactStoredValues.size() != rhs.exactStoredValues.size()) {
    return false;
  }
  for (const auto& entry : lhs.preciseVersions) {
    auto it = rhs.preciseVersions.find(entry.first);
    if (it == rhs.preciseVersions.end() || it->second != entry.second)
      return false;
  }
  for (const auto& entry : lhs.exactStoredValues) {
    auto it = rhs.exactStoredValues.find(entry.first);
    if (it == rhs.exactStoredValues.end() ||
        it->second.value != entry.second.value ||
        it->second.preciseRootKey != entry.second.preciseRootKey) {
      return false;
    }
  }
  return true;
}

MemoryVersionState mergeMemoryStates(
    const std::vector<const MemoryVersionState*>& incoming) {
  MemoryVersionState merged;
  if (incoming.empty())
    return merged;

  for (const MemoryVersionState* state : incoming) {
    if (!state) continue;
    unionVersions(merged.globalBarriers, state->globalBarriers);
    unionVersions(merged.allStoreVersions, state->allStoreVersions);
    unionVersions(merged.externalStoreVersions, state->externalStoreVersions);
    unionVersions(merged.externalBarriers, state->externalBarriers);
    for (const auto& version : state->preciseVersions)
      unionVersions(merged.preciseVersions[version.first], version.second);
  }

  // 前递值要求所有前驱都为同一规范地址携带同一个 SSA 值，满足时才保留。
  merged.exactStoredValues = incoming.front()->exactStoredValues;
  for (size_t index = 1; index < incoming.size(); ++index) {
    const auto& values = incoming[index]->exactStoredValues;
    for (auto it = merged.exactStoredValues.begin();
         it != merged.exactStoredValues.end();) {
      auto other = values.find(it->first);
      if (other == values.end() || other->second.value != it->second.value ||
          other->second.preciseRootKey != it->second.preciseRootKey) {
        it = merged.exactStoredValues.erase(it);
      } else {
        ++it;
      }
    }
  }
  return merged;
}

MemoryVersionState transferMemoryState(
    BasicBlock* block, MemoryVersionState state, Module& mod,
    const analysis::FunctionEffectMap& effects) {
  if (!block)
    return state;
  for (auto& instPtr : block->instructions()) {
    Instruction* inst = instPtr.get();
    if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
      recordMemoryStore(state, classifyMemory(inst->operand(1)), inst->id());
      recordExactStoredValue(state, inst->operand(1), inst->operand(0));
    } else if (inst->opcode() == Opcode::Call &&
               !callPreservesTrackedMemory(inst, mod, effects)) {
      recordMemoryCallEffects(state, inst, mod, effects, inst->id());
    }
  }
  return state;
}

struct MemoryDataflowResult {
  std::unordered_map<BasicBlock*, MemoryVersionState> entry;
  std::unordered_map<BasicBlock*, MemoryVersionState> exit;
};

MemoryDataflowResult computeMemoryDataflow(
    Function* func, Module& mod,
    const analysis::FunctionEffectMap& effects,
    analysis::FunctionAnalysisCache& cache) {
  MemoryDataflowResult result;
  if (!func || !func->entryBlock())
    return result;

  const auto& reachable = cache.reachable();
  auto blocks = reachableCachedBlocks(cache);
  std::queue<BasicBlock*> worklist;
  std::unordered_set<BasicBlock*> queued;
  for (BasicBlock* block : blocks) {
    result.entry[block] = {};
    result.exit[block] =
        transferMemoryState(block, MemoryVersionState{}, mod, effects);
    worklist.push(block);
    queued.insert(block);
  }

  while (!worklist.empty()) {
    BasicBlock* block = worklist.front();
    worklist.pop();
    queued.erase(block);

    MemoryVersionState newEntry;
    if (block != func->entryBlock()) {
      std::vector<const MemoryVersionState*> incoming;
      for (BasicBlock* predecessor : block->preds()) {
        if (predecessor && reachable.count(predecessor))
          incoming.push_back(&result.exit[predecessor]);
      }
      newEntry = mergeMemoryStates(incoming);
    }
    MemoryVersionState newExit =
        transferMemoryState(block, newEntry, mod, effects);
    if (!memoryStatesEqual(result.entry[block], newEntry) ||
        !memoryStatesEqual(result.exit[block], newExit)) {
      result.entry[block] = std::move(newEntry);
      result.exit[block] = std::move(newExit);
      for (BasicBlock* successor : block->succs()) {
        if (successor && reachable.count(successor) &&
            queued.insert(successor).second) {
          worklist.push(successor);
        }
      }
    }
  }
  return result;
}

bool dominatorTreeCSE(Function* func,
                      Module& mod,
                      const analysis::FunctionEffectMap& effects,
                      analysis::FunctionAnalysisCache& cache) {
  if (!func || !func->entryBlock()) return false;
  if (cache.instructionCount() > 12000) return false;

  const auto& reachable = cache.reachable();
  auto blocks = reachableCachedBlocks(cache);
  if (blocks.empty()) return false;

  // 沿支配树前序遍历做 CSE：多前驱块先过滤从前驱继承的表达式表——循环
  // 头直接清空全部 Load CSE 项，其余按各前驱的 store/未知写 kill 可能别名
  // 的 Load 项；内存版本状态取数据流入口值。
  const DomSet& doms = cache.dominators();
  auto idom = computeIDom(func, blocks, doms);
  auto children = buildDomTreeChildren(blocks, idom);
  MemoryDataflowResult memoryDataflow =
      computeMemoryDataflow(func, mod, effects, cache);
  std::unordered_set<BasicBlock*> loopHeaders;
  for (const auto& loop : cache.loops()) {
    if (loop.header) loopHeaders.insert(loop.header);
  }
  bool changed = false;
  std::function<void(BasicBlock*, std::unordered_map<std::string, Value*>,
                     MemoryVersionState)> walk =
      [&](BasicBlock* bb, std::unordered_map<std::string, Value*> exprs,
          MemoryVersionState memState) {
    if (!bb || !reachable.count(bb)) return;
    if (bb != func->entryBlock() && bb->preds().size() != 1) {
      if (loopHeaders.count(bb))
        clearLoadCSEEntries(exprs);
      else {
        std::vector<MemoryAliasInfo> incomingStores;
        bool incomingUnknownWrite = false;
        for (BasicBlock* pred : bb->preds()) {
          if (!pred || !reachable.count(pred))
            continue;
          for (const auto& predInstPtr : pred->instructions()) {
            Instruction* predInst = predInstPtr.get();
            if (predInst->opcode() == Opcode::Store &&
                predInst->numOperands() == 2) {
              incomingStores.push_back(
                  classifyMemory(predInst->operand(1)));
            } else if (predInst->opcode() == Opcode::Call &&
                       !callPreservesTrackedMemory(predInst, mod, effects)) {
              incomingUnknownWrite = true;
            }
          }
        }
        for (auto it = exprs.begin(); it != exprs.end();) {
          auto* previousLoad = dynamic_cast<Instruction*>(it->second);
          if (!previousLoad || previousLoad->opcode() != Opcode::Load) {
            ++it;
            continue;
          }
          bool kill = incomingUnknownWrite;
          MemoryAliasInfo loadInfo =
              classifyMemory(previousLoad->operand(0));
          for (const MemoryAliasInfo& storeInfo : incomingStores)
            kill |= memoryMayAlias(loadInfo, storeInfo);
          if (kill)
            it = exprs.erase(it);
          else
            ++it;
        }
      }
      if (loopHeaders.count(bb)) {
        memState = {};
      } else {
        auto stateIt = memoryDataflow.entry.find(bb);
        memState = stateIt == memoryDataflow.entry.end()
                       ? MemoryVersionState{}
                       : stateIt->second;
      }
    }

    // 块内指令扫描：规则与 basicBlockCSE 一致（Load 精确转发、Call 副作用
    // CSE、Store 记版本与精确值、其余表达式去重）。
    for (auto& instPtr : bb->instructions()) {
      Instruction* inst = instPtr.get();
      if (inst->opcode() == Opcode::Load) {
        if (Value* forwarded = forwardedExactStoredValue(inst, memState)) {
          inst->replaceAllUsesWith(forwarded);
          changed = true;
          continue;
        }
        std::string key = loadCSEKey(inst, memState);
        if (!key.empty()) {
          auto it = exprs.find(key);
          if (it != exprs.end()) {
            inst->replaceAllUsesWith(it->second);
            changed = true;
          } else {
            exprs[key] = inst;
          }
        }
        continue;
      }
      if (inst->opcode() == Opcode::Call) {
        std::string key = callCSEKey(inst, mod, effects, memState);
        if (!key.empty()) {
          auto it = exprs.find(key);
          if (it != exprs.end()) {
            inst->replaceAllUsesWith(it->second);
            changed = true;
          } else {
            exprs[key] = inst;
          }
          continue;
        }
        if (callPreservesTrackedMemory(inst, mod, effects))
          continue;
        recordMemoryCallEffects(memState, inst, mod, effects, inst->id());
        continue;
      }
      if (inst->opcode() == Opcode::Store && inst->numOperands() == 2) {
        recordMemoryStore(memState, classifyMemory(inst->operand(1)),
                          inst->id());
        recordExactStoredValue(memState, inst->operand(1), inst->operand(0));
        continue;
      }
      if (!isCSECandidate(inst)) continue;

      std::string key = cseKey(inst);
      auto it = exprs.find(key);
      if (it != exprs.end()) {
        inst->replaceAllUsesWith(it->second);
        changed = true;
      } else {
        exprs[key] = inst;
      }
    }

    auto childIt = children.find(bb);
    if (childIt == children.end()) return;
    for (auto* child : childIt->second)
      walk(child, exprs, memState);
  };

  walk(func->entryBlock(), {}, {});
  return changed;
}

bool valueAvailableBeforeTerminator(Value* value, BasicBlock* block,
                                    const DomSet& doms) {
  if (!value || !block)
    return false;
  if (value->isConstant() || dynamic_cast<Argument*>(value) ||
      dynamic_cast<GlobalVar*>(value)) {
    return true;
  }
  auto* inst = dynamic_cast<Instruction*>(value);
  if (!inst || !inst->parent())
    return false;
  if (inst->parent() != block)
    return analysis::dominates(doms, inst->parent(), block);
  for (const auto& current : block->instructions()) {
    if (current.get() == inst)
      return true;
    if (current->isTerminator())
      break;
  }
  return false;
}

bool moveBeforeTerminator(Instruction* inst, BasicBlock* target) {
  if (!inst || !inst->parent() || !target || !target->terminator())
    return false;
  BasicBlock* source = inst->parent();
  auto& sourceInsts = source->instructions();
  auto sourceIt = std::find_if(
      sourceInsts.begin(), sourceInsts.end(),
      [inst](const auto& owned) { return owned.get() == inst; });
  if (sourceIt == sourceInsts.end())
    return false;

  auto& targetInsts = target->instructions();
  auto targetIt = std::find_if(
      targetInsts.begin(), targetInsts.end(),
      [target](const auto& owned) {
        return owned.get() == target->terminator();
      });
  if (targetIt == targetInsts.end())
    return false;

  std::unique_ptr<Instruction> owned = std::move(*sourceIt);
  sourceInsts.erase(sourceIt);
  owned->setParent(target);
  targetInsts.insert(targetIt, std::move(owned));
  return true;
}

bool hoistOneCommonBranchMultiply(
    Function* func, analysis::FunctionAnalysisCache& cache) {
  if (!func || !func->entryBlock() || cache.instructionCount() > 12000)
    return false;
  const DomSet& doms = cache.dominators();
  const auto& reachable = cache.reachable();

  for (const auto& branchPtr : func->blocks()) {
    BasicBlock* branch = branchPtr.get();
    Instruction* term = branch ? branch->terminator() : nullptr;
    if (!term || term->opcode() != Opcode::CondBr ||
        term->numOperands() != 3 || !reachable.count(branch)) {
      continue;
    }
    auto* trueBlock = dynamic_cast<BasicBlock*>(term->operand(1));
    auto* falseBlock = dynamic_cast<BasicBlock*>(term->operand(2));
    if (!trueBlock || !falseBlock || trueBlock == falseBlock ||
        !reachable.count(trueBlock) || !reachable.count(falseBlock) ||
        trueBlock->preds().size() != 1 ||
        falseBlock->preds().size() != 1 ||
        trueBlock->preds().front() != branch ||
        falseBlock->preds().front() != branch) {
      continue;
    }

    for (const auto& trueInstPtr : trueBlock->instructions()) {
      Instruction* trueInst = trueInstPtr.get();
      if (!trueInst || trueInst->opcode() != Opcode::Mul ||
          trueInst->numOperands() != 2 || trueInst->uses().empty()) {
        continue;
      }
      if (!valueAvailableBeforeTerminator(trueInst->operand(0), branch, doms) ||
          !valueAvailableBeforeTerminator(trueInst->operand(1), branch, doms)) {
        continue;
      }
      for (const auto& falseInstPtr : falseBlock->instructions()) {
        Instruction* falseInst = falseInstPtr.get();
        if (!falseInst || falseInst->opcode() != Opcode::Mul ||
            falseInst->uses().empty() ||
            !(falseInst->type() == trueInst->type()) ||
            falseInst->numOperands() != trueInst->numOperands() ||
            falseInst->operand(0) != trueInst->operand(0) ||
            falseInst->operand(1) != trueInst->operand(1)) {
          continue;
        }
        if (!moveBeforeTerminator(trueInst, branch))
          return false;
        falseInst->replaceAllUsesWith(trueInst);
        falseBlock->remove(falseInst);
        return true;
      }
    }
  }
  return false;
}

bool hoistCommonBranchExpressions(
    Function* func, analysis::FunctionAnalysisCache& cache) {
  bool changed = false;
  for (int iteration = 0; iteration < 32; ++iteration) {
    if (!hoistOneCommonBranchMultiply(func, cache))
      break;
    changed = true;
  }
  return changed;
}

bool dominatorTreeCSE(Module& mod, analysis::AnalysisManager& analysisManager) {
  const analysis::FunctionEffectMap& effects = analysisManager.functionEffects();
  bool changed = false;
  for (auto& func : mod.functions()) {
    analysis::FunctionAnalysisCache& cache =
        analysisManager.function(func.get());
    changed |= dominatorTreeCSE(func.get(), mod, effects, cache);
    changed |= hoistCommonBranchExpressions(func.get(), cache);
  }
  return changed;
}


class ForwardConstGlobalLoadsPass : public Pass {
public:
    std::string name() const override { return "ForwardConstGlobalLoads"; }
    bool run(Module& mod, analysis::AnalysisManager&) override {
        return forwardConstGlobalLoads(mod);
    }
};
REGISTER_PASS(ForwardConstGlobalLoadsPass);

class DominatorTreeCSEPass : public Pass {
public:
    std::string name() const override { return "DominatorTreeCSE"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return dominatorTreeCSE(mod, am);
    }
};
REGISTER_PASS(DominatorTreeCSEPass);

} // namespace core_ir
