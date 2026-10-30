// 无用返回值消除：若某函数的全部调用点都忽略返回值，则该返回值对
// 程序行为不可观测，可把函数所有 Ret 的返回值替换为类型零常量；
// 原返回值计算链随后由 cleanup 的 worklist DCE 清除。
#include "dead_return_elim.h"
#include "pass_registry.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace core_ir {

bool deadReturnValueElimination(Module& mod, analysis::AnalysisManager&) {
    // 收集每个非内建、非 main 被调函数的全部非 void 调用点。
    std::unordered_map<Function*, std::vector<Instruction*>> callSites;
    for (auto& fp : mod.functions()) {
        for (auto& bp : fp->blocks()) {
            for (auto& ip : bp->instructions()) {
                Instruction* inst = ip.get();
                if (!inst || inst->opcode() != Opcode::Call)
                    continue;
                if (inst->type().isVoid())
                    continue;
                Function* callee = mod.getFunction(inst->name());
                if (!callee || callee->name() == "main" ||
                    mod.isBuiltin(callee->name()))
                    continue;
                callSites[callee].push_back(inst);
            }
        }
    }

    bool changed = false;
    for (auto& entry : callSites) {
        Function* callee = entry.first;
        // 任一调用点使用返回值即不可变换；递归调用点也在上面的
        // 全模块扫描中覆盖，因此该检查对递归函数同样安全。
        bool allUnused = true;
        for (Instruction* site : entry.second) {
            if (!site->uses().empty()) {
                allUnused = false;
                break;
            }
        }
        if (!allUnused)
            continue;
        // 只处理标量返回类型，保持 verifyModule 对 Ret 操作数类型的检查。
        Type retTy = callee->retTy();
        if (!retTy.isI32() && !retTy.isF32())
            continue;
        Value* zero = retTy.isI32() ? static_cast<Value*>(mod.getConstInt(0))
                                    : static_cast<Value*>(mod.getConstFloat(0.0f));
        for (auto& bp : callee->blocks()) {
            Instruction* term = bp->terminator();
            if (!term || term->opcode() != Opcode::Ret ||
                term->numOperands() != 1)
                continue;
            if (term->operand(0) == zero)
                continue;
            term->setOperand(0, zero);
            changed = true;
        }
    }
    return changed;
}

class DeadReturnValueEliminationPass : public Pass {
public:
    std::string name() const override { return "DeadReturnValueElimination"; }
    bool run(Module& mod, analysis::AnalysisManager& am) override {
        return deadReturnValueElimination(mod, am);
    }
};
REGISTER_PASS(DeadReturnValueEliminationPass);

} // namespace core_ir
