// 文件职责：串接 MIR lowering、寄存器分配、调度、栈帧和打印流程。本文件负责声明接口和数据结构。
#pragma once

#include "machine_ir.h"

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace core_ir { class Module; }

namespace mir {

enum class MachinePipelineMode {
  Production,
  Planning,
};

struct MachineMetrics {
  std::size_t functions = 0;
  std::size_t basicBlocks = 0;
  std::size_t instructions = 0;
  std::size_t loads = 0;
  std::size_t stores = 0;
  std::size_t calls = 0;
  std::size_t branches = 0;
  std::size_t conditionalBranches = 0;
  std::size_t integerMultiplies = 0;
  std::size_t integerDivRems = 0;
  std::size_t floatingDivSqrts = 0;
  std::size_t addressMaterializations = 0;
  std::size_t loadUsePairs = 0;
  std::size_t compressedInstructionOpportunities = 0;
  std::size_t hotLoopInstructions = 0;
  std::size_t hotLoopLoads = 0;
  std::size_t hotLoopStores = 0;
  std::size_t hotLoopBranches = 0;
  std::size_t spillInstructions = 0;
  std::size_t reloadInstructions = 0;
  std::size_t spillSlots = 0;
  std::size_t spillBytes = 0;
  std::size_t frameBytes = 0;
  std::size_t emittedAssemblyBytes = 0;
  std::size_t estimatedCriticalPath = 0;
  std::size_t cyclicCFGEdges = 0;
  int peakGPRPressure = 0;
  int peakFPRPressure = 0;
};

struct MachineVerificationResult {
  bool ok = true;
  std::string reasonCode = "accepted";
  std::string detail;
};

struct MachinePipelineResult {
  std::unique_ptr<MachineModule> module;
  MachineMetrics metrics;
  MachineVerificationResult verification;
};

struct EmitResult {
  bool ok = false;
  std::string assembly;
  std::string reasonCode;
  std::string detail;
};

// Planning and production use these exact same stages. Planning mode only
// suppresses environment-controlled diagnostic alternatives; it does not use
// a second estimator backend.
std::unique_ptr<MachineModule> lowerForPlanning(const core_ir::Module& mod);
MachinePipelineResult runMachinePipeline(
    std::unique_ptr<MachineModule> module,
    MachinePipelineMode mode = MachinePipelineMode::Planning);
MachineMetrics collectMachineMetrics(const MachineModule& module);
MachineVerificationResult verifyMachineModule(
    const MachineModule& module, bool requireAllocatedRegisters);
EmitResult emitChecked(const MachineModule& module,
                       const core_ir::Module& coreModule);

// 端到端 MIR 后端：将 CoreIR 模块转换为汇编文本。
// 流程：
//   1. lowerToMIR        （CoreIR -> 带虚拟寄存器和 PHI 伪指令的 MachineModule）
//   2. runLinearScanRA   （默认）或通过 MIR_SIMPLE_RA=1 使用 runSimpleRA
//   3. runPeephole       （MIR_NO_PEEPHOLE=1 时跳过）
//   4. runListSchedule   （MIR_NO_SCHEDULE=1 时跳过）
//   5. lowerFrame        （计算栈帧，插入序言/尾声，将 FrameIdx 转为 sp 偏移）
//   6. printAsm          （向 out 输出 .text/.data/.rodata）
//
// 这是公开 -O1 性能模式使用的后端路径。
void runMIRBackend(const core_ir::Module& mod, std::ostream& out);

} // namespace mir
