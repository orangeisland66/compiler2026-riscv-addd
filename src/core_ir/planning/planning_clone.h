// 文件职责：完整克隆 CoreIR Module，供隔离候选验证和 shadow backend 使用。
#pragma once

#include "../ir/core_ir.h"

#include <memory>
#include <unordered_map>

namespace core_ir::planning {

class PlanningClone {
public:
  ~PlanningClone();
  Module& module() { return *module_; }
  const Module& module() const { return *module_; }
  Function* mapped(const Function* source) const;
  BasicBlock* mapped(const BasicBlock* source) const;
  Value* mapped(const Value* source) const;
  std::unique_ptr<Module> release() { return std::move(module_); }

private:
  friend std::unique_ptr<PlanningClone> cloneModuleForPlanning(
      const Module& source);
  std::unique_ptr<Module> module_;
  int valueIdCheckpoint_ = 0;
  std::unordered_map<const Function*, Function*> functions_;
  std::unordered_map<const BasicBlock*, BasicBlock*> blocks_;
  std::unordered_map<const Value*, Value*> values_;
};

std::unique_ptr<PlanningClone> cloneModuleForPlanning(const Module& source);

} // namespace core_ir::planning
