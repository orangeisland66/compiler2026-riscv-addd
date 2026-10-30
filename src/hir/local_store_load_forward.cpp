#include "research_switch.h"
// 文件职责：在局部范围内转发已知的存储值，减少重复加载。
#include "local_store_load_forward.h"
#include "local_store_load_forward_detail.h"

#include "../analysis/analysis_manager.h"
#include "../analysis/loop_info.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>

namespace hir {
namespace {

using core_ir::BasicBlock;
using core_ir::ConstInt;
using core_ir::Function;
using core_ir::Instruction;
using core_ir::Module;
using core_ir::Opcode;
using core_ir::Value;

struct AddressKey {
  Value* base = nullptr;
  int64_t offset = 0;
  int64_t width = 0;

  bool operator==(const AddressKey& other) const {
    return base == other.base && offset == other.offset &&
           width == other.width;
  }
};

struct AddressKeyHash {
  size_t operator()(const AddressKey& key) const {
    return std::hash<Value*>{}(key.base) ^
           (std::hash<int64_t>{}(key.offset) << 1) ^
           (std::hash<int64_t>{}(key.width) << 2);
  }
};

bool traceEnabled() {
  return sycEnvironmentValue("SYC_TRACE_HIR_LOCAL_STORE_FORWARD") != nullptr;
}

void trace(Function* function, BasicBlock* block, const char* event) {
  if (!traceEnabled())
    return;
  std::cerr << "[local-store-load-forward] func="
            << (function ? function->name() : std::string("?"))
            << " block=" << (block ? block->name() : std::string("?"))
            << " " << event << "\n";
}

Value* incomingFor(Instruction* phi, BasicBlock* predecessor) {
  if (!phi || phi->opcode() != Opcode::Phi || !predecessor)
    return nullptr;
  for (int index = 0; index + 1 < phi->numOperands(); index += 2) {
    if (phi->operand(index + 1) == predecessor)
      return phi->operand(index);
  }
  return nullptr;
}

bool constGepOffset(Value* value, Value*& base, int64_t& offset) {
  offset = 0;
  Value* current = value;
  while (auto* inst = dynamic_cast<Instruction*>(current)) {
    if (inst->opcode() != Opcode::GEP || inst->numOperands() != 2)
      break;
    auto* index = dynamic_cast<ConstInt*>(inst->operand(1));
    if (!index)
      break;
    const int64_t delta = index->value();
    if ((delta > 0 && offset > std::numeric_limits<int64_t>::max() - delta) ||
        (delta < 0 && offset < std::numeric_limits<int64_t>::min() - delta)) {
      return false;
    }
    offset += delta;
    current = inst->operand(0);
  }
  base = current;
  return base != nullptr;
}

bool offsetFromPhi(Value* value, Instruction* phi, int64_t& offset) {
  Value* base = nullptr;
  if (!constGepOffset(value, base, offset))
    return false;
  return base == phi;
}

using PointerRelations = std::unordered_map<Instruction*, int64_t>;

bool buildPointerRelations(const analysis::NaturalLoop& natural,
                           Instruction* anchor,
                           PointerRelations& relations) {
  BasicBlock* header = natural.header;
  if (!header || !anchor || anchor->opcode() != Opcode::Phi ||
      !anchor->type().isPtr()) {
    return false;
  }

  std::vector<BasicBlock*> outsidePredecessors;
  for (auto* predecessor : header->preds()) {
    if (predecessor && !natural.blocks.count(predecessor))
      outsidePredecessors.push_back(predecessor);
  }
  if (outsidePredecessors.size() != 1)
    return false;
  BasicBlock* entryPredecessor = outsidePredecessors.front();

  Value* anchorInit = incomingFor(anchor, entryPredecessor);
  Value* anchorBase = nullptr;
  int64_t anchorOffset = 0;
  if (!anchorInit ||
      !constGepOffset(anchorInit, anchorBase, anchorOffset)) {
    return false;
  }

  relations.emplace(anchor, 0);
  for (auto& instructionPtr : header->instructions()) {
    Instruction* phi = instructionPtr.get();
    if (!phi || phi->opcode() != Opcode::Phi || !phi->type().isPtr())
      continue;
    Value* initial = incomingFor(phi, entryPredecessor);
    Value* initialBase = nullptr;
    int64_t initialOffset = 0;
    if (!initial ||
        !constGepOffset(initial, initialBase, initialOffset) ||
        initialBase != anchorBase) {
      continue;
    }

    const int64_t relativeOffset = initialOffset - anchorOffset;
    bool preservesRelation = true;
    for (auto* predecessor : header->preds()) {
      if (!predecessor || !natural.blocks.count(predecessor))
        continue;
      Value* anchorIncoming = incomingFor(anchor, predecessor);
      Value* phiIncoming = incomingFor(phi, predecessor);
      int64_t anchorStep = 0;
      int64_t phiStep = 0;
      if (!anchorIncoming || !phiIncoming ||
          !offsetFromPhi(anchorIncoming, anchor, anchorStep) ||
          !offsetFromPhi(phiIncoming, phi, phiStep) ||
          anchorStep != phiStep) {
        preservesRelation = false;
        break;
      }
    }
    if (preservesRelation)
      relations.emplace(phi, relativeOffset);
  }
  return relations.size() >= 2;
}

int64_t scalarMemoryWidth(const core_ir::Type& type) {
  return type.isPtr() ? 8 : 4;
}

bool normalizeAddress(Value* value, int64_t width,
                      const PointerRelations& relations, AddressKey& key) {
  if (width <= 0)
    return false;
  int64_t offset = 0;
  Value* base = nullptr;
  if (!constGepOffset(value, base, offset))
    return false;
  auto* phi = dynamic_cast<Instruction*>(base);
  if (!phi)
    return false;
  auto relation = relations.find(phi);
  if (relation == relations.end())
    return false;
  if ((offset > 0 &&
       relation->second > std::numeric_limits<int64_t>::max() - offset) ||
      (offset < 0 &&
       relation->second < std::numeric_limits<int64_t>::min() - offset)) {
    return false;
  }
  key.base = relations.begin()->first;
  key.offset = relation->second + offset;
  key.width = width;
  return true;
}

bool addressRangesOverlap(const AddressKey& lhs, const AddressKey& rhs) {
  return lhs.base == rhs.base && detail::memoryByteRangesOverlap(
                                      {lhs.offset, lhs.width},
                                      {rhs.offset, rhs.width});
}

bool forgetPossiblyAliasingStores(
    std::unordered_map<AddressKey, Value*, AddressKeyHash>& knownStores,
    const AddressKey& storedAddress) {
  bool killedPartialOverlap = false;
  for (auto it = knownStores.begin(); it != knownStores.end();) {
    if (addressRangesOverlap(it->first, storedAddress)) {
      killedPartialOverlap |= !(it->first == storedAddress);
      it = knownStores.erase(it);
    } else {
      ++it;
    }
  }
  return killedPartialOverlap;
}

// 块内存储转发：线性扫描指令，用规范化地址记录最近 Store 的存储值，
// 后续命中同一地址且类型匹配的 Load 直接用存储值替换并删除。
bool forwardInBlock(Function* function, BasicBlock* block,
                    const PointerRelations& relations) {
  if (!block)
    return false;

  std::unordered_map<AddressKey, Value*, AddressKeyHash> knownStores;
  std::vector<Instruction*> instructions;
  instructions.reserve(block->instructions().size());
  for (auto& instructionPtr : block->instructions())
    instructions.push_back(instructionPtr.get());

  bool changed = false;
  for (Instruction* instruction : instructions) {
    if (!instruction || !instruction->parent())
      continue;
    // Call/VStore 可能有未知内存副作用，保守清空已记录的存储值。
    if (instruction->opcode() == Opcode::Call ||
        instruction->opcode() == Opcode::VStore) {
      knownStores.clear();
      continue;
    }
    if (instruction->opcode() == Opcode::Store) {
      if (instruction->numOperands() != 2) {
        knownStores.clear();
        continue;
      }
      AddressKey address;
      const int64_t width = scalarMemoryWidth(instruction->operand(0)->type());
      if (!normalizeAddress(instruction->operand(1), width, relations,
                            address)) {
        knownStores.clear();
        continue;
      }
      if (forgetPossiblyAliasingStores(knownStores, address))
        trace(function, block, "kill-overlap");
      knownStores[address] = instruction->operand(0);
      continue;
    }
    if (instruction->opcode() != Opcode::Load ||
        instruction->numOperands() != 1) {
      continue;
    }
    AddressKey address;
    if (!normalizeAddress(instruction->operand(0),
                          scalarMemoryWidth(instruction->type()), relations,
                          address))
      continue;
    auto stored = knownStores.find(address);
    if (stored == knownStores.end() || !stored->second ||
        stored->second->type().kind != instruction->type().kind) {
      continue;
    }
    instruction->replaceAllUsesWith(stored->second);
    block->remove(instruction);
    changed = true;
    trace(function, block, "forwarded");
  }
  return changed;
}

bool applyToLoop(Function* function, const analysis::NaturalLoop& natural) {
  if (!function || !natural.header) {
    return false;
  }
  trace(function, natural.header, "inspect");
  if (!analysis::dedicatedPreheader(natural)) {
    trace(function, natural.header, "reject-no-preheader");
    return false;
  }

  std::vector<Instruction*> pointerPhis;
  for (auto& instructionPtr : natural.header->instructions()) {
    Instruction* instruction = instructionPtr.get();
    if (instruction && instruction->opcode() == Opcode::Phi &&
        instruction->type().isPtr()) {
      pointerPhis.push_back(instruction);
    }
  }

  bool changed = false;
  for (Instruction* anchor : pointerPhis) {
    PointerRelations relations;
    if (!buildPointerRelations(natural, anchor, relations)) {
      trace(function, natural.header, "reject-relations");
      continue;
    }
    for (auto* block : natural.blocks) {
      if (block && block != natural.header)
        changed |= forwardInBlock(function, block, relations);
    }
  }
  return changed;
}

} // namespace

bool applyLocalStoreLoadForward(Module& mod,
                                analysis::AnalysisManager& analysisManager) {
  bool changed = false;
  for (auto& functionPtr : mod.functions()) {
    Function* function = functionPtr.get();
    if (!function)
      continue;
    function->recomputeCFG();
    analysis::FunctionAnalysisCache& cache = analysisManager.function(function);
    auto loops = cache.loops();
    for (const auto& loop : loops) {
      if (applyToLoop(function, loop)) {
        changed = true;
        analysisManager.invalidateFunction(
            function, analysis::AnalysisInvalidation::CFG);
        break;
      }
    }
  }
  return changed;
}

} // namespace hir
