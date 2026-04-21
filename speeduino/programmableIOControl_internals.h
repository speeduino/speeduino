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

// It's much easier to work with a struct that represents the rule in a more direct way, so we convert the config13 struct into this for processing
// Ideally the tune would also be able to use this struct directly, but that would require some refactoring of how the config pages are stored and accessed, 
// so for now we keep using config13 for the tune and convert to programmableOutputRule for processing
struct programmableOutputRule {
  bool outputInverted; ///< Invert (on/off) value before writing to output pin (for all programmable I/O:s).
  bool kindOfLimiting; ///< Select which kind of output limiting are active (0 - minimum | 1 - maximum)
  uint8_t outputPin;   ///< Disable(0) or enable (set to valid pin number) Programmable Pin (output/target pin to set)
  uint8_t outputDelay; ///< Output write delay for each programmable I/O (Unit: 0.1S)
  uint8_t firstDataIn; ///< Set of first I/O vars to compare
  uint8_t secondDataIn;///< Set of second I/O vars to compare
  uint8_t outputTimeLimit; ///< Output delay for each programmable I/O, kindOfLimiting bit dependent(Unit: 0.1S)
  int16_t firstTarget; ///< first  target value to compare with numeric comp
  int16_t secondTarget;///< second target value to compare with bitwise op
  cmpOperation operation; ///< I/O variable comparison operations

  programmableOutputRule(const config13& page13, uint8_t index) 
   : outputInverted(BIT_CHECK(page13.outputInverted, index)),
     kindOfLimiting(BIT_CHECK(page13.kindOfLimiting, index)),
     outputPin(page13.outputPin[index]),
     outputDelay(page13.outputDelay[index]),
     firstDataIn(page13.firstDataIn[index]),
     secondDataIn(page13.secondDataIn[index]),
     outputTimeLimit(page13.outputTimeLimit[index]),
     firstTarget(page13.firstTarget[index]),
     secondTarget(page13.secondTarget[index]),
     operation(page13.operation[index])
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