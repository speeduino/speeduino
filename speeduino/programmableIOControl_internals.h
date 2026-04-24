/**
 * @file
 * @brief Not used outside of programmableIOControl.cpp, but needs to be shared with unit tests
 */

#include <stdint.h>
#include "config_pages.h"
#include "preprocessor.h"

namespace programmableIOControl_details
{

// Defined here for unit testing, but only used in programmableIOControl.cpp
constexpr uint8_t COMPARATOR_EQUAL = 0;
constexpr uint8_t COMPARATOR_NOT_EQUAL = 1;
constexpr uint8_t COMPARATOR_GREATER = 2;
constexpr uint8_t COMPARATOR_GREATER_EQUAL = 3;
constexpr uint8_t COMPARATOR_LESS = 4;
constexpr uint8_t COMPARATOR_LESS_EQUAL = 5;
constexpr uint8_t COMPARATOR_AND = 6;
constexpr uint8_t COMPARATOR_XOR = 7;

constexpr uint8_t BITWISE_DISABLED = 0;
constexpr uint8_t BITWISE_AND = 1;
constexpr uint8_t BITWISE_OR = 2;
constexpr uint8_t BITWISE_XOR = 3;

constexpr uint8_t REUSE_RULES = 240;

struct compOperation_t {
  uint8_t opType;
  uint8_t dataIndex;
  int16_t target;

  bool isVirtualData() const {
    return dataIndex >= REUSE_RULES;
  }
  uint8_t getDataIndex() const {
    if (!isVirtualData()) {
      return dataIndex;
    }
    return dataIndex - REUSE_RULES;
  }
};

enum class LimitingType : uint8_t {
  Min = 0,
  Max = 1,
};

// It's much easier to work with a struct that represents the rule in a more direct way, so we convert the config13 struct into this for processing
// Ideally the tune would also be able to use this struct directly, but that would require some refactoring of how the config pages are stored and accessed, 
// so for now we keep using config13 for the tune and convert to programmableOutputRule for processing
struct rule_t {
  bool isOutputInverted; ///< Invert (on/off) value before writing to output pin (for all programmable I/O:s).
  LimitingType limitType; ///< Select which kind of output limiting are active (0 - minimum | 1 - maximum)
  uint8_t outputPin;   ///< Disable(0) or enable (set to valid pin number) Programmable Pin (output/target pin to set)
  uint8_t activationDelay; ///< Output write delay for each programmable I/O (Unit: 0.1S)
  uint8_t outputTimeLimit; ///< Output delay for each programmable I/O, kindOfLimiting bit dependent(Unit: 0.1S)
  uint8_t combineOpType;
  compOperation_t firstOp; ///< First operand for comparison
  compOperation_t secondOp; ///< Second operand for comparison, used if bitwise operation is enabled
  uint8_t _index; // Index of the rule

  rule_t() = default;
  rule_t(const config13& page13, uint8_t index) 
   : isOutputInverted(BIT_CHECK(page13.outputInverted, index)),
     limitType(BIT_CHECK(page13.kindOfLimiting, index) ? LimitingType::Max : LimitingType::Min),
     outputPin(page13.outputPin[index]),
     activationDelay(page13.outputDelay[index]),
     outputTimeLimit(page13.outputTimeLimit[index]),
     combineOpType(page13.operation[index].bitwise),
     firstOp({page13.operation[index].firstCompType, page13.firstDataIn[index], page13.firstTarget[index]}),
     secondOp({page13.operation[index].secondCompType, page13.secondDataIn[index], page13.secondTarget[index]}),
     _index(index)
  {
  }

  bool isCascadeRule(void) const {
    return outputPin >= 128U;
  }
  bool isPhysicalPin(void) const {
    return !isCascadeRule();
  }
};

// The struct representing the state of each programmable I/O channel, used for processing the 
// rules and keeping track of delays and active status.
struct channel_t
{
  bool isPinValid : 1;
  bool isRuleActive : 1;
  uint8_t activationDelayCount = 0;
  uint8_t outputDelayCount = 0;

  channel_t()
  : isPinValid(false), isRuleActive(false), activationDelayCount(0), outputDelayCount(0)
  {}

  void initialize(const rule_t& rule);
};

// The struct representing the current state of the programmable I/O system
struct state_t
{
  channel_t channels[_countof(config13::outputPin)];
};

}