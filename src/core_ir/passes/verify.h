// 文件职责：声明 CoreIR 结构检查接口。
#pragma once
#include "../ir/core_ir.h"

#include <string>

namespace core_ir {

struct IRVerificationResult {
  bool ok = true;
  std::string reasonCode = "accepted";
  std::string detail;
};

void verifyModule(const Module& mod);
IRVerificationResult verifyModuleChecked(const Module& mod) noexcept;

} // namespace core_ir
