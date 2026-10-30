// 文件职责：统一目标无关分析/证明的三态合同；Unknown 一律不得授权变换。
#pragma once

#include <string>

namespace analysis {

enum class ProofState {
  Disproved,
  Unknown,
  Proved,
};

struct ProofResult {
  ProofState state = ProofState::Unknown;
  std::string reasonCode = "unknown";
  std::string detail;

  bool proved() const { return state == ProofState::Proved; }
  static ProofResult success(std::string reason = "proved") {
    return {ProofState::Proved, std::move(reason), {}};
  }
  static ProofResult unknown(std::string reason, std::string detail = {}) {
    return {ProofState::Unknown, std::move(reason), std::move(detail)};
  }
  static ProofResult disproved(std::string reason, std::string detail = {}) {
    return {ProofState::Disproved, std::move(reason), std::move(detail)};
  }
};

} // namespace analysis
