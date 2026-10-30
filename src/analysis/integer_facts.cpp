// 文件职责：实现不依赖源码名称的 i32 range/known-bit 事实求值。
#include "integer_facts.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <unordered_set>

namespace analysis {

namespace {

using namespace core_ir;

IntegerFacts exactFacts(int32_t value, const char* reason) {
  const uint32_t bits = static_cast<uint32_t>(value);
  IntegerFacts facts;
  facts.proof = ProofResult::success(reason);
  facts.rangeKnown = true;
  facts.minimum = value;
  facts.maximum = value;
  facts.knownZero = ~bits;
  facts.knownOne = bits;
  facts.nonNegative = value >= 0;
  facts.nonZero = value != 0;
  return facts;
}

IntegerFacts rangeFacts(int32_t minimum, int32_t maximum,
                        const char* reason) {
  IntegerFacts facts;
  facts.proof = ProofResult::success(reason);
  facts.rangeKnown = true;
  facts.minimum = minimum;
  facts.maximum = maximum;
  facts.nonNegative = minimum >= 0;
  facts.nonZero = maximum < 0 || minimum > 0;
  return facts;
}

bool signedRangeFits(int64_t minimum, int64_t maximum) {
  return minimum >= std::numeric_limits<int32_t>::min() &&
         maximum <= std::numeric_limits<int32_t>::max();
}

class Evaluator {
public:
  IntegerFacts evaluate(Value* value) {
    if (!value || !value->type().isI32())
      return {ProofResult::unknown("integer-facts-non-i32")};
    if (auto* constant = dynamic_cast<ConstInt*>(value))
      return exactFacts(constant->value(), "integer-constant");
    auto found = memo_.find(value);
    if (found != memo_.end()) return found->second;
    if (!active_.insert(value).second)
      return {ProofResult::unknown("integer-facts-cycle")};
    IntegerFacts result = evaluateInstruction(
        dynamic_cast<Instruction*>(value));
    active_.erase(value);
    memo_[value] = result;
    return result;
  }

private:
  IntegerFacts evaluateInstruction(Instruction* instruction) {
    if (!instruction)
      return {ProofResult::unknown("integer-facts-unsupported-value")};
    auto operand = [&](int index) {
      return index < instruction->numOperands()
                 ? evaluate(instruction->operand(index))
                 : IntegerFacts{
                       ProofResult::unknown("integer-facts-missing-operand")};
    };
    switch (instruction->opcode()) {
    case Opcode::Mov:
    case Opcode::BitCast:
      return instruction->numOperands() == 1
                 ? operand(0)
                 : IntegerFacts{ProofResult::unknown(
                       "integer-facts-malformed-copy")};
    case Opcode::ICmp:
      return rangeFacts(0, 1, "integer-compare-range");
    case Opcode::Phi: {
      bool sawIncoming = false;
      int32_t minimum = std::numeric_limits<int32_t>::max();
      int32_t maximum = std::numeric_limits<int32_t>::min();
      uint32_t knownZero = ~uint32_t{0};
      uint32_t knownOne = ~uint32_t{0};
      for (int index = 0; index + 1 < instruction->numOperands(); index += 2) {
        IntegerFacts incoming = operand(index);
        if (!incoming.proof.proved() || !incoming.rangeKnown)
          return {ProofResult::unknown("integer-phi-incoming-unknown")};
        sawIncoming = true;
        minimum = std::min(minimum, incoming.minimum);
        maximum = std::max(maximum, incoming.maximum);
        knownZero &= incoming.knownZero;
        knownOne &= incoming.knownOne;
      }
      if (!sawIncoming)
        return {ProofResult::unknown("integer-phi-empty")};
      auto result = rangeFacts(minimum, maximum, "integer-phi-range");
      result.knownZero = knownZero;
      result.knownOne = knownOne;
      return result;
    }
    case Opcode::And: {
      if (instruction->numOperands() != 2)
        return {ProofResult::unknown("integer-and-malformed")};
      ConstInt* mask = dynamic_cast<ConstInt*>(instruction->operand(0));
      if (!mask || mask->value() < 0)
        mask = dynamic_cast<ConstInt*>(instruction->operand(1));
      if (!mask || mask->value() < 0)
        return {ProofResult::unknown("integer-and-range-unknown")};
      auto result = rangeFacts(0, mask->value(), "integer-mask-range");
      result.knownZero = ~static_cast<uint32_t>(mask->value());
      return result;
    }
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul: {
      if (instruction->numOperands() != 2)
        return {ProofResult::unknown("integer-arithmetic-malformed")};
      IntegerFacts lhs = operand(0);
      IntegerFacts rhs = operand(1);
      if (!lhs.proof.proved() || !rhs.proof.proved() ||
          !lhs.rangeKnown || !rhs.rangeKnown)
        return {ProofResult::unknown("integer-arithmetic-range-unknown")};
      int64_t minimum = 0;
      int64_t maximum = 0;
      if (instruction->opcode() == Opcode::Add) {
        minimum = static_cast<int64_t>(lhs.minimum) + rhs.minimum;
        maximum = static_cast<int64_t>(lhs.maximum) + rhs.maximum;
      } else if (instruction->opcode() == Opcode::Sub) {
        minimum = static_cast<int64_t>(lhs.minimum) - rhs.maximum;
        maximum = static_cast<int64_t>(lhs.maximum) - rhs.minimum;
      } else {
        const int64_t products[] = {
            static_cast<int64_t>(lhs.minimum) * rhs.minimum,
            static_cast<int64_t>(lhs.minimum) * rhs.maximum,
            static_cast<int64_t>(lhs.maximum) * rhs.minimum,
            static_cast<int64_t>(lhs.maximum) * rhs.maximum};
        minimum = *std::min_element(std::begin(products), std::end(products));
        maximum = *std::max_element(std::begin(products), std::end(products));
      }
      if (!signedRangeFits(minimum, maximum))
        return {ProofResult::unknown("integer-arithmetic-may-wrap")};
      return rangeFacts(static_cast<int32_t>(minimum),
                        static_cast<int32_t>(maximum),
                        "integer-arithmetic-range");
    }
    case Opcode::Shl:
    case Opcode::Shr: {
      if (instruction->numOperands() != 2)
        return {ProofResult::unknown("integer-shift-malformed")};
      IntegerFacts source = operand(0);
      auto* amount = dynamic_cast<ConstInt*>(instruction->operand(1));
      if (!source.proof.proved() || !source.rangeKnown ||
          source.minimum != source.maximum || !amount || amount->value() < 0 ||
          amount->value() >= 32)
        return {ProofResult::unknown("integer-shift-range-unknown")};
      uint32_t bits = static_cast<uint32_t>(source.minimum);
      int32_t value = 0;
      if (instruction->opcode() == Opcode::Shl) {
        bits <<= static_cast<unsigned>(amount->value());
        std::memcpy(&value, &bits, sizeof(value));
      } else {
        const unsigned shift = static_cast<unsigned>(amount->value());
        uint32_t shifted = bits >> shift;
        if (source.minimum < 0 && shift != 0)
          shifted |= ~uint32_t{0} << (32 - shift);
        std::memcpy(&value, &shifted, sizeof(value));
      }
      return exactFacts(value, "integer-exact-shift");
    }
    case Opcode::SRem: {
      if (instruction->numOperands() != 2)
        return {ProofResult::unknown("integer-rem-malformed")};
      IntegerFacts dividend = operand(0);
      auto* divisor = dynamic_cast<ConstInt*>(instruction->operand(1));
      if (!dividend.proof.proved() || !dividend.rangeKnown || !divisor ||
          divisor->value() == 0 ||
          (divisor->value() == -1 &&
           dividend.minimum == std::numeric_limits<int32_t>::min()))
        return {ProofResult::unknown("integer-rem-range-unknown")};
      const int64_t magnitude = std::abs(static_cast<int64_t>(divisor->value()));
      const int32_t limit = static_cast<int32_t>(magnitude - 1);
      return rangeFacts(dividend.minimum < 0 ? -limit : 0,
                        dividend.maximum > 0 ? limit : 0,
                        "integer-rem-range");
    }
    default:
      break;
    }
    return {ProofResult::unknown("integer-opcode-unsupported")};
  }

  std::unordered_map<Value*, IntegerFacts> memo_;
  std::unordered_set<Value*> active_;
};

} // namespace

IntegerFacts evaluateIntegerFacts(core_ir::Value* value) {
  Evaluator evaluator;
  return evaluator.evaluate(value);
}

} // namespace analysis
