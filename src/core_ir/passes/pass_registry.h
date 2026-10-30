// 文件职责：声明 Pass 基类、全局 pass 注册表与 REGISTER_PASS 注册宏。
#pragma once
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

// 在 global namespace 中前向声明，避免在 core_ir 内部污染 analysis 命名空间
namespace core_ir { class Module; }
namespace analysis { class AnalysisManager; }

namespace core_ir {

// ============================================================
// Pass 基类
// ============================================================

class Pass {
public:
    virtual ~Pass() = default;
    virtual std::string name() const = 0;
    virtual bool run(Module& mod, analysis::AnalysisManager& am) = 0;
};

// ============================================================
// 注册表 + REGISTER_PASS 宏
// ============================================================

using PassCreator = std::function<std::unique_ptr<Pass>()>;

struct PassRegistry {
    static PassRegistry& instance();
    void registerPass(const std::string& name, PassCreator creator);
    std::unique_ptr<Pass> create(const std::string& name) const;
    const auto& creators() const { return creators_; }
private:
    std::unordered_map<std::string, PassCreator> creators_;
};

#define REGISTER_PASS(PassClass) \
    static bool _reg_##PassClass = [] { \
        PassRegistry::instance().registerPass( \
            #PassClass, []{ return std::make_unique<PassClass>(); }); \
        return true; \
    }()

} // namespace core_ir
