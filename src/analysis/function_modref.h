// 文件职责：把函数 effect 摘要映射到具体调用点与内存对象。
#pragma once

#include "function_effects.h"
#include "proof_result.h"

namespace analysis {

struct CallModRefResult {
  ProofResult proof;
  bool mayRead = true;
  bool mayWrite = true;
};

CallModRefResult queryCallModRef(
    const core_ir::Instruction* call,
    core_ir::Value* location,
    int locationBytes,
    const FunctionEffectMap& effects);

} // namespace analysis
