// 文件职责：从 region 产生的值沿 use-def 收集所有外部可观察者并证明闭包。
#pragma once

#include "function_effects.h"
#include "ordered_region_effect.h"
#include "proof_result.h"

#include <unordered_set>
#include <vector>

namespace analysis {

struct ObserverChainPlan {
  ProofResult proof;
  std::vector<core_ir::Instruction*> observers;
  std::unordered_set<core_ir::Instruction*> valueSlice;
  bool escapesFunction = false;
  bool reachesUnknownCall = false;
  bool reachesMemoryWrite = false;
};

struct RegionDemandSlice {
  ProofResult proof;
  std::unordered_set<core_ir::Instruction*> instructions;
  std::unordered_set<core_ir::Value*> liveIns;
  std::unordered_set<core_ir::Instruction*> effectInstructions;
  std::unordered_set<core_ir::Value*> memoryObjects;
  bool hasUnknownMemoryDemand = false;
};

struct RegionValueDeletionProof {
  ProofResult proof;
  std::vector<core_ir::Instruction*> instructions;
};

ObserverChainPlan buildObserverChainPlan(
    const std::vector<core_ir::Value*>& roots,
    const std::unordered_set<core_ir::BasicBlock*>& region,
    const FunctionEffectMap* effects = nullptr);

// 从已证明的外部 observer 逆向收集 region 内的值依赖。该查询
// 只给出值 demand，不单独授权删除未命中的 effect/trap/control。
RegionDemandSlice buildRegionDemandSlice(
    const std::vector<core_ir::Instruction*>& observers,
    const std::unordered_set<core_ir::BasicBlock*>& region,
    const OrderedRegionEffectSummary* effects = nullptr);

// 证明 value demand 之外的纯标量子图可以删除。内存、调用、控制流和
// 未证明不陷阱的指令一律不进入结果；结果中的每个 use 也必须位于该结果内。
RegionValueDeletionProof proveRegionValueDeletion(
    const RegionDemandSlice& demand,
    const std::unordered_set<core_ir::BasicBlock*>& region);

} // namespace analysis
