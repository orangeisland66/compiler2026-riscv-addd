// 文件职责：提供研究/实验优化开关的统一接口。
//
// 默认构建不打开研究开关；只有在构建时定义 SYC_ENABLE_RESEARCH_PASSES，
// 且运行环境设置对应 SYC_ENABLE_* 变量时，实验性 pass 才会响应。
// 影响代码生成的环境开关只在 research 构建且显式设置
// SYC_RESEARCH=1 时生效。strict 构建只响应 TRACE/DUMP/VERIFY 等
// 可观测开关，保证相同命令行在不同运行环境下生成相同代码。
//
#pragma once
#ifndef SYC_RESEARCH_SWITCH_H
#define SYC_RESEARCH_SWITCH_H

#include <cstdlib>
#include <string_view>

inline const char* sycRawEnvironmentValue(const char* name) {
  return std::getenv(name);
}

enum class SyCEnvironmentClass {
  Observability,
  Rollback,
  SemanticEnable,
  UnknownSemantic,
};

inline bool sycStartsWith(std::string_view text, std::string_view prefix) {
  return text.size() >= prefix.size() &&
         text.substr(0, prefix.size()) == prefix;
}

inline bool sycEnvironmentValueIsObservabilityOnly(std::string_view name) {
  return name.find("TRACE") != std::string_view::npos ||
         name.find("DUMP") != std::string_view::npos ||
         name.find("VERIFY") != std::string_view::npos ||
         name.find("PROFILE") != std::string_view::npos ||
         name.find("REPORT") != std::string_view::npos ||
         name.find("VERBOSE") != std::string_view::npos ||
         name.find("LOG") != std::string_view::npos;
}

inline SyCEnvironmentClass sycEnvironmentVariableClass(
    std::string_view name) {
  if (sycEnvironmentValueIsObservabilityOnly(name))
    return SyCEnvironmentClass::Observability;
  if (sycStartsWith(name, "SYC_ENABLE_"))
    return SyCEnvironmentClass::SemanticEnable;
  if (sycStartsWith(name, "SYC_SKIP_") ||
      sycStartsWith(name, "MIR_NO_") ||
      name.find("DISABLE") != std::string_view::npos ||
      name == "MIR_SIMPLE_RA")
    return SyCEnvironmentClass::Rollback;
  return SyCEnvironmentClass::UnknownSemantic;
}

inline bool sycEnvironmentFlagValue(const char* value) {
  if (!value)
    return false;
  const std::string_view text(value);
  return text == "1" || text == "true" || text == "TRUE" ||
         text == "on" || text == "ON";
}

inline constexpr bool sycResearchBuildEnabled() {
#ifdef SYC_ENABLE_RESEARCH_PASSES
  return true;
#else
  return false;
#endif
}

inline bool sycResearchSwitchEnabled(const char* name) {
  return sycEnvironmentVariableClass(name) ==
             SyCEnvironmentClass::SemanticEnable &&
         sycResearchBuildEnabled() &&
         sycEnvironmentFlagValue(sycRawEnvironmentValue("SYC_RESEARCH")) &&
         sycEnvironmentFlagValue(sycRawEnvironmentValue(name));
}

inline const char* sycEnvironmentValue(const char* name) {
  const char* value = sycRawEnvironmentValue(name);
  if (!value || sycEnvironmentVariableClass(name) ==
                    SyCEnvironmentClass::Observability)
    return value;
  return sycResearchBuildEnabled() &&
                 sycEnvironmentFlagValue(
                     sycRawEnvironmentValue("SYC_RESEARCH"))
             ? value
             : nullptr;
}

#endif  // SYC_RESEARCH_SWITCH_H
