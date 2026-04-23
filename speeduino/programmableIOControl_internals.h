/**
 * @file
 * @brief Not used outside of programmableIOControl.cpp, but needs to be shared with unit tests
 */

#include <stdint.h>

namespace programmableIOControl_details
{

struct programmableio_channel_t
{
  bool pinIsValid = false;
  uint8_t ioDelay = 0;
  uint8_t ioOutDelay = 0;
  bool currentRuleStatus = false;
};

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

struct compOperation {
  uint8_t opType;
  uint8_t dataIndex;
  int16_t target;
};

// It's much easier to work with a struct that represents the rule in a more direct way, so we convert the config13 struct into this for processing
// Ideally the tune would also be able to use this struct directly, but that would require some refactoring of how the config pages are stored and accessed, 
// so for now we keep using config13 for the tune and convert to programmableOutputRule for processing
struct programmableOutputRule {
  bool outputInverted; ///< Invert (on/off) value before writing to output pin (for all programmable I/O:s).
  bool kindOfLimiting; ///< Select which kind of output limiting are active (0 - minimum | 1 - maximum)
  uint8_t outputPin;   ///< Disable(0) or enable (set to valid pin number) Programmable Pin (output/target pin to set)
  uint8_t outputDelay; ///< Output write delay for each programmable I/O (Unit: 0.1S)
  uint8_t outputTimeLimit; ///< Output delay for each programmable I/O, kindOfLimiting bit dependent(Unit: 0.1S)
  uint8_t opCompare;
  compOperation firstOp; ///< First operand for comparison
  compOperation secondOp; ///< Second operand for comparison, used if bitwise operation is enabled

  programmableOutputRule() = default;
  programmableOutputRule(const config13& page13, uint8_t index) 
   : outputInverted(BIT_CHECK(page13.outputInverted, index)),
     kindOfLimiting(BIT_CHECK(page13.kindOfLimiting, index)),
     outputPin(page13.outputPin[index]),
     outputDelay(page13.outputDelay[index]),
     outputTimeLimit(page13.outputTimeLimit[index]),
     opCompare(page13.operation[index].bitwise),
     firstOp({page13.operation[index].firstCompType, page13.firstDataIn[index], page13.firstTarget[index]}),
     secondOp({page13.operation[index].secondCompType, page13.secondDataIn[index], page13.secondTarget[index]})
  {
  }

  bool isCascadeRule(void) const {
    return outputPin >= 128U;
  }
  bool isPhysicalPin(void) const {
    return !isCascadeRule();
  }
};

}