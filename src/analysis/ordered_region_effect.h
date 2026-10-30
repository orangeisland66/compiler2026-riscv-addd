// 文件职责：按 IR 顺序记录 region 内的读写、调用和控制流边界。
#pragma once

#include "function_effects.h"
#include "pointer_range.h"

#include <unordered_set>
#include <vector>

namespace analysis {

enum class OrderedEffectKind { Read, Write, ReadWriteCall, ControlExit };

struct OrderedEffectEvent {
  OrderedEffectKind kind = OrderedEffectKind::Read;
  core_ir::Instruction* instruction = nullptr;
  core_ir::Value* object = nullptr;
  PointerRange range;
};

struct OrderedRegionEffectSummary {
  ProofResult proof;
  std::vector<OrderedEffectEvent> events;
  std::unordered_set<core_ir::Value*> readObjects;
  std::unordered_set<core_ir::Value*> writtenObjects;
  bool hasUnknownEffect = false;
  bool hasObservableCall = false;
  bool hasSideExit = false;
};

OrderedRegionEffectSummary analyzeOrderedRegionEffects(
    core_ir::Function* function,
    const std::unordered_set<core_ir::BasicBlock*>& region,
    const FunctionEffectMap* effects = nullptr);

} // namespace analysis
