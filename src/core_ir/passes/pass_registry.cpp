// 文件职责：注册和查找 CoreIR pass 的名称与构造函数。
#include "pass_registry.h"

namespace core_ir {

PassRegistry& PassRegistry::instance() {
    static PassRegistry reg;
    return reg;
}

void PassRegistry::registerPass(const std::string& name, PassCreator creator) {
    creators_[name] = std::move(creator);
}

std::unique_ptr<Pass> PassRegistry::create(const std::string& name) const {
    auto it = creators_.find(name);
    if (it != creators_.end()) return it->second();
    return nullptr;
}

} // namespace core_ir
