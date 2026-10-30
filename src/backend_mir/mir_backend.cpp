// 文件职责：串接 MIR lowering、寄存器分配、调度、栈帧和打印流程。本文件负责实现具体逻辑。
#include "mir_backend.h"

#include "linear_scan.h"
#include "loop_branch_rotate.h"
#include "lower.h"
#include "machine_cleanup.h"
#include "machine_ir.h"
#include "mir_liveness.h"
#include "peephole.h"
#include "prologue.h"
#include "rv64_cost_model.h"
#include "schedule.h"
#include "simple_ra.h"

#include "core_ir.h"
#include "research_switch.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace mir {

namespace {

std::pair<int, int> collectPeakPressure(MachineModule& module) {
  int peakGPR = 0;
  int peakFPR = 0;
  for (auto& function : module.functions()) {
    auto live = analyzeLiveness(*function);
    std::vector<std::pair<int, int>> gprEvents;
    std::vector<std::pair<int, int>> fprEvents;
    for (const auto& interval : live.intervals) {
      auto& events = interval.cls == RegClass::GPR ? gprEvents : fprEvents;
      for (const auto& segment : interval.segments) {
        events.push_back({segment.start, 1});
        events.push_back({segment.end + 1, -1});
      }
    }
    auto peak = [](std::vector<std::pair<int, int>>& events) {
      std::sort(events.begin(), events.end(), [](const auto& lhs,
                                                const auto& rhs) {
        if (lhs.first != rhs.first) return lhs.first < rhs.first;
        return lhs.second < rhs.second;
      });
      int current = 0;
      int result = 0;
      for (const auto& event : events) {
        current += event.second;
        result = std::max(result, current);
      }
      return result;
    };
    peakGPR = std::max(peakGPR, peak(gprEvents));
    peakFPR = std::max(peakFPR, peak(fprEvents));
  }
  return {peakGPR, peakFPR};
}

bool reaches(const MachineBasicBlock* start, const MachineBasicBlock* target) {
  if (!start || !target) return false;
  std::unordered_set<const MachineBasicBlock*> seen{start};
  std::vector<const MachineBasicBlock*> work{start};
  for (std::size_t index = 0; index < work.size(); ++index) {
    if (work[index] == target) return true;
    for (const auto* successor : work[index]->succs())
      if (successor && seen.insert(successor).second)
        work.push_back(successor);
  }
  return false;
}

bool isIntegerMultiply(MIOp opcode) {
  return opcode == MIOp::MUL || opcode == MIOp::MULW ||
         opcode == MIOp::MULH;
}

bool isIntegerDivRem(MIOp opcode) {
  switch (opcode) {
  case MIOp::DIV:
  case MIOp::DIVW:
  case MIOp::DIVU:
  case MIOp::DIVUW:
  case MIOp::REM:
  case MIOp::REMW:
  case MIOp::REMU:
  case MIOp::REMUW:
    return true;
  default:
    return false;
  }
}

bool isFloatingDivSqrt(MIOp opcode) {
  return opcode == MIOp::FDIV_S || opcode == MIOp::FSQRT_S;
}

bool isSmallImmediate(const MachineInst& instruction) {
  for (const auto& operand : instruction.operands())
    if (operand.isImm()) return operand.imm >= -32 && operand.imm <= 31;
  return false;
}

bool isConservativelyCompressible(const MachineInst& instruction) {
  switch (instruction.opcode()) {
  case MIOp::MV:
  case MIOp::J:
  case MIOp::RET:
  case MIOp::NOP:
    return true;
  case MIOp::LI:
  case MIOp::ADDI:
    return isSmallImmediate(instruction);
  default:
    return false;
  }
}

bool usesAnyDefinition(const MachineInst& producer,
                       const MachineInst& consumer) {
  std::vector<int> definitions;
  std::vector<int> uses;
  producer.collectDefs(definitions);
  consumer.collectUses(uses);
  for (int definition : definitions)
    if (std::find(uses.begin(), uses.end(), definition) != uses.end())
      return true;
  return false;
}

} // namespace

std::unique_ptr<MachineModule> lowerForPlanning(const core_ir::Module& mod) {
  return lowerToMIR(mod);
}

MachineVerificationResult verifyMachineModule(
    const MachineModule& module, bool requireAllocatedRegisters) {
  MachineVerificationResult result;
  for (const auto& function : module.functions()) {
    if (!function->entryBlock())
      return {false, "missing-machine-entry", function->name()};
    std::unordered_set<const MachineBasicBlock*> blocks;
    for (const auto& block : function->blocks()) blocks.insert(block.get());
    for (std::size_t blockIndex = 0; blockIndex < function->blocks().size();
         ++blockIndex) {
      const auto& block = function->blocks()[blockIndex];
      // Block layout/peephole intentionally removes an unconditional jump to
      // the immediately following block.  That is an implicit fallthrough,
      // not a missing terminator.
      if (!block->terminator()) {
        const bool validFallthrough =
            block->succs().size() == 1 &&
            blockIndex + 1 < function->blocks().size() &&
            block->succs().front() == function->blocks()[blockIndex + 1].get();
        if (!validFallthrough)
          return {false, "missing-machine-terminator", block->label()};
      }
      for (const auto* successor : block->succs()) {
        if (!successor || !blocks.count(successor))
          return {false, "invalid-machine-cfg-edge", block->label()};
        if (std::find(successor->preds().begin(), successor->preds().end(),
                      block.get()) == successor->preds().end())
          return {false, "asymmetric-machine-cfg-edge", block->label()};
      }
      for (const auto* predecessor : block->preds()) {
        if (!predecessor || !blocks.count(predecessor))
          return {false, "invalid-machine-cfg-edge", block->label()};
        if (std::find(predecessor->succs().begin(), predecessor->succs().end(),
                      block.get()) == predecessor->succs().end())
          return {false, "asymmetric-machine-cfg-edge", block->label()};
      }
      for (const auto& instruction : block->insts()) {
        if (instruction->parent() != block.get())
          return {false, "machine-parent-mismatch", block->label()};
        for (const auto& operand : instruction->operands()) {
          if (operand.isMBB() && (!operand.mbb || !blocks.count(operand.mbb)))
            return {false, "invalid-machine-block-operand", block->label()};
          if (!operand.isReg()) continue;
          if (operand.reg < 0)
            return {false, "invalid-machine-register", block->label()};
          if (requireAllocatedRegisters && isVirtualReg(operand.reg))
            return {false, "unallocated-vreg", block->label()};
          if (isPhysReg(operand.reg) &&
              ((operand.regClass == RegClass::GPR && !isGPR(operand.reg)) ||
               (operand.regClass == RegClass::FPR && !isFPR(operand.reg))))
            return {false, "register-class-mismatch", block->label()};
        }
      }
    }
    if (function->frameSize < 0 || function->frameSize % 16 != 0)
      return {false, "invalid-frame-alignment", function->name()};
  }
  return result;
}

MachineMetrics collectMachineMetrics(const MachineModule& module) {
  MachineMetrics metrics;
  metrics.functions = module.functions().size();
  for (const auto& function : module.functions()) {
    metrics.basicBlocks += function->blocks().size();
    metrics.frameBytes += static_cast<std::size_t>(function->frameSize);
    for (const auto& object : function->frame()) {
      if (!object.isSpill) continue;
      ++metrics.spillSlots;
      metrics.spillBytes += static_cast<std::size_t>(std::max(0, object.size));
    }
    for (const auto& block : function->blocks()) {
      std::size_t blockCriticalPath = 0;
      bool inCycle = false;
      for (const auto* successor : block->succs())
        if (successor == block.get() || reaches(successor, block.get())) {
          ++metrics.cyclicCFGEdges;
          inCycle = true;
        }
      for (std::size_t index = 0; index < block->insts().size(); ++index) {
        const auto& instruction = block->insts()[index];
        ++metrics.instructions;
        const auto opcode = instruction->opcode();
        metrics.loads += mioMayLoad(opcode);
        metrics.stores += mioMayStore(opcode);
        metrics.calls += mioIsCall(opcode);
        metrics.branches += mioIsBranch(opcode);
        metrics.conditionalBranches += mioIsBranch(opcode);
        metrics.integerMultiplies += isIntegerMultiply(opcode);
        metrics.integerDivRems += isIntegerDivRem(opcode);
        metrics.floatingDivSqrts += isFloatingDivSqrt(opcode);
        metrics.addressMaterializations +=
            opcode == MIOp::LA || opcode == MIOp::LI;
        metrics.compressedInstructionOpportunities +=
            isConservativelyCompressible(*instruction);
        if (mioMayLoad(opcode) && index + 1 < block->insts().size() &&
            usesAnyDefinition(*instruction, *block->insts()[index + 1]))
          ++metrics.loadUsePairs;
        if (inCycle) {
          ++metrics.hotLoopInstructions;
          metrics.hotLoopLoads += mioMayLoad(opcode);
          metrics.hotLoopStores += mioMayStore(opcode);
          metrics.hotLoopBranches += mioIsBranch(opcode);
        }
        metrics.spillInstructions += opcode == MIOp::SPILL;
        metrics.reloadInstructions += opcode == MIOp::RELOAD;
        blockCriticalPath += static_cast<std::size_t>(
            std::max(1, rv64InstructionCost(opcode).latency));
      }
      metrics.estimatedCriticalPath =
          std::max(metrics.estimatedCriticalPath, blockCriticalPath);
    }
  }
  return metrics;
}

MachinePipelineResult runMachinePipeline(
    std::unique_ptr<MachineModule> mm, MachinePipelineMode mode) {
  MachinePipelineResult result;
  if (!mm) {
    result.verification = {false, "lowering-failed", "null MachineModule"};
    return result;
  }
  runMachineCleanup(*mm);
  const bool allowAlternatives = mode == MachinePipelineMode::Production;
  if (!(allowAlternatives &&
        sycEnvironmentValue("SYC_SKIP_MIR_EARLY_RETURN_SHRINKWRAP")))
    prepareEarlyReturnShrinkWrapping(*mm);
  // 寄存器分配前调度一次，让热展开块在物理寄存器反依赖引入前暴露更多 ILP。
  // pre-RA 阶段只处理浮点密集块，以控制寄存器压力并避免改变整数块布局。
  const bool noSchedule =
      allowAlternatives && sycEnvironmentValue("MIR_NO_SCHEDULE");
  if (!noSchedule) runListScheduleFPHot(*mm);
  auto pressure = collectPeakPressure(*mm);
  // 默认使用 linear-scan；设置 MIR_SIMPLE_RA=1 时退回 spill-everything
  // 分配器，仅用于调试。
  if (allowAlternatives && sycEnvironmentValue("MIR_SIMPLE_RA")) {
    runSimpleRA(*mm);
  } else {
    runLinearScanRA(*mm);
  }
  if (!(allowAlternatives &&
        sycEnvironmentValue("MIR_NO_LOOP_BRANCH_ROTATE")))
    runLoopBranchRotate(*mm);
  // 列表调度器：在基本块内重排指令以隐藏加载使用延迟，
  // 并适配 BOOM 的双发射前端。设置 MIR_NO_SCHEDULE 可关闭它以便对比。
  if (!noSchedule) runListSchedule(*mm);
  runBlockLayout(*mm);
  // 依赖布局的窥孔规则（分支反转、直落跳转删除、跳板串联）需要看到最终块顺序。
  if (!(allowAlternatives && sycEnvironmentValue("MIR_NO_PEEPHOLE")))
    runPeephole(*mm);
  runMachineCleanup(*mm);
  // Post-RA cleanup can expose an adjacent full-width spill/reload pair that
  // was separated by now-removed copies. Re-run the idempotent local rules
  // before frame indices become concrete offsets.
  if (!(allowAlternatives && sycEnvironmentValue("MIR_NO_PEEPHOLE")))
    runPeephole(*mm);
  // 删除寄存器分配后的存储指令可能留下无用的地址物化。
  // 重新执行物理寄存器清理，避免这些新暴露的无用定义进入最终汇编。
  if (removeRedundantPostRAStores(*mm))
    runMachineCleanup(*mm);
  lowerFrame(*mm);
  // Frame lowering may coalesce non-overlapping spill homes onto the same
  // concrete offset. The final peephole can then forward an immediately
  // reloaded value while retaining the store required by the stable home.
  if (!(allowAlternatives && sycEnvironmentValue("MIR_NO_PEEPHOLE")))
    runPeephole(*mm);
  // Several established MIR rewrites move unique_ptr-owned instructions
  // between blocks.  Ownership is authoritative; restore the cached parent
  // link once after the complete mutation pipeline before verification and
  // emission.
  for (auto& function : mm->functions())
    for (auto& block : function->blocks())
      for (auto& instruction : block->insts())
        instruction->setParent(block.get());
  result.metrics = collectMachineMetrics(*mm);
  result.metrics.peakGPRPressure = pressure.first;
  result.metrics.peakFPRPressure = pressure.second;
  result.verification = verifyMachineModule(*mm, true);
  result.module = std::move(mm);
  return result;
}

EmitResult emitChecked(const MachineModule& module,
                       const core_ir::Module& coreModule) {
  const auto verification = verifyMachineModule(module, true);
  if (!verification.ok)
    return {false, {}, verification.reasonCode, verification.detail};
  try {
    std::ostringstream assembly;
    printAsm(assembly, module, coreModule);
    return {true, assembly.str(), "accepted", {}};
  } catch (const std::exception& error) {
    return {false, {}, "assembly-emission-failed", error.what()};
  }
}

void runMIRBackend(const core_ir::Module& mod, std::ostream& out) {
  auto pipeline = runMachinePipeline(lowerForPlanning(mod),
                                     MachinePipelineMode::Production);
  if (!pipeline.verification.ok)
    throw std::runtime_error("MIR pipeline rejected: " +
                             pipeline.verification.reasonCode + ": " +
                             pipeline.verification.detail);
  auto emission = emitChecked(*pipeline.module, mod);
  if (!emission.ok)
    throw std::runtime_error("MIR emission rejected: " +
                             emission.reasonCode + ": " + emission.detail);
  pipeline.metrics.emittedAssemblyBytes = emission.assembly.size();
  out << emission.assembly;
}

} // namespace mir
