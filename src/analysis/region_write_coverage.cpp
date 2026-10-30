// 文件职责：合并不重叠/相邻写区间，未知对象大小时只报告 partial/unknown。
#include "region_write_coverage.h"

#include <algorithm>
#include <unordered_map>

namespace analysis {

namespace {

int64_t objectSize(core_ir::Value* object) {
  if (auto* global = dynamic_cast<core_ir::GlobalVar*>(object))
    return global->sizeBytes();
  if (auto* instruction = dynamic_cast<core_ir::Instruction*>(object))
    if (instruction->opcode() == core_ir::Opcode::Alloca)
      return instruction->memorySizeBytes();
  return 0;
}

} // namespace

RegionWriteCoverage analyzeRegionWriteCoverage(
    const OrderedRegionEffectSummary& effects) {
  RegionWriteCoverage result;
  result.hasUnknownEffect = effects.hasUnknownEffect;
  std::unordered_map<core_ir::Value*, std::vector<std::pair<int64_t, int64_t>>>
      writes;
  std::unordered_map<core_ir::Value*, bool> reads;
  std::unordered_map<core_ir::Value*, bool> readBeforeWrite;
  for (const auto& event : effects.events) {
    if (!event.object) continue;
    if (event.kind == OrderedEffectKind::Read) {
      reads[event.object] = true;
      // Without path-sensitive ordering proof, any read in the same region
      // may observe the state before the complete overwrite.
      readBeforeWrite[event.object] = true;
    } else if (event.kind == OrderedEffectKind::Write) {
      if (event.range.proof.proved())
        writes[event.object].push_back(
            {event.range.beginByte, event.range.endByte});
    }
  }
  std::unordered_map<core_ir::Value*, bool> objects;
  for (const auto& item : reads) objects[item.first] = true;
  for (const auto& item : writes) objects[item.first] = true;
  for (const auto& item : objects) {
    RegionObjectCoverage coverage;
    coverage.object = item.first;
    coverage.objectBytes = objectSize(item.first);
    coverage.read = reads[item.first];
    coverage.written = !writes[item.first].empty();
    coverage.readBeforeCompleteWrite = readBeforeWrite[item.first];
    auto ranges = writes[item.first];
    std::sort(ranges.begin(), ranges.end());
    int64_t begin = 0;
    int64_t end = 0;
    bool have = false;
    int64_t contiguousEnd = 0;
    for (const auto& range : ranges) {
      if (!have || range.first > end) {
        if (have) coverage.coveredBytes += end - begin;
        begin = range.first;
        end = range.second;
        have = true;
      } else {
        end = std::max(end, range.second);
      }
      if (range.first >= 0 && range.first <= contiguousEnd)
        contiguousEnd = std::max(contiguousEnd, range.second);
    }
    if (have) coverage.coveredBytes += end - begin;
    if (coverage.objectBytes <= 0) {
      coverage.kind = RegionCoverageKind::Unknown;
      coverage.proof = ProofResult::unknown("object-size-unknown");
    } else if (contiguousEnd >= coverage.objectBytes &&
               !coverage.readBeforeCompleteWrite) {
      coverage.kind = RegionCoverageKind::Complete;
      coverage.proof = ProofResult::success("complete-object-write");
    } else {
      coverage.kind = RegionCoverageKind::Partial;
      coverage.proof = ProofResult::disproved("partial-object-write");
    }
    result.objects.push_back(std::move(coverage));
  }
  std::sort(result.objects.begin(), result.objects.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.object && rhs.object
                         ? lhs.object->id() < rhs.object->id()
                         : lhs.object < rhs.object;
            });
  if (result.hasUnknownEffect)
    result.proof = ProofResult::unknown("coverage-has-unknown-effect");
  else
    result.proof = ProofResult::success("coverage-inventory-complete");
  return result;
}

const char* regionCoverageKindName(RegionCoverageKind kind) {
  switch (kind) {
  case RegionCoverageKind::Complete: return "complete";
  case RegionCoverageKind::Partial: return "partial";
  case RegionCoverageKind::Unknown: return "unknown";
  }
  return "unknown";
}

} // namespace analysis
