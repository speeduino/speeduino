/**
 * @file
 * @brief Not used outside of programmableIOControl.cpp, but needs to be shared with unit tests
 */

#include <stdint.h>
#include "../../../config_pages.h"
#include "../../../preprocessor.h"

namespace programmableIOControl_details
{

// Forward declares
struct state_t;

// Defined here for unit testing, but only used in programmableIOControl.cpp
constexpr uint8_t COMPARATOR_EQUAL = 0;
constexpr uint8_t COMPARATOR_NOT_EQUAL = 1;
constexpr uint8_t COMPARATOR_GREATER = 2;
constexpr uint8_t COMPARATOR_GREATER_EQUAL = 3;
constexpr uint8_t COMPARATOR_LESS = 4;
constexpr uint8_t COMPARATOR_LESS_EQUAL = 5;
constexpr uint8_t COMPARATOR_AND = 6;
constexpr uint8_t COMPARATOR_XOR = 7;

constexpr uint8_t COMBINE_DISABLED = 0;
constexpr uint8_t COMBINE_AND = 1;
constexpr uint8_t COMBINE_OR = 2;
constexpr uint8_t COMBINE_XOR = 3;

constexpr uint8_t REUSE_RULES = 240;

using getDataFn = int16_t (*)(uint16_t index);

struct compOperation_t {
  uint8_t opType;
  uint8_t dataIndex;
  int16_t target;

  bool isVirtualData() const {
    return dataIndex >= REUSE_RULES;
  }
  bool evaluate(int16_t lhs, int16_t rhs) const;
  int16_t getComparisonData(const state_t& state, getDataFn pGetData) const;
  bool evaluate(const state_t& state, getDataFn pGetData) const;
};

enum class LimitingType : uint8_t {
  Min = 0,
  Max = 1,
};

// It's much easier to work with a struct that represents the rule in a more direct way, so we convert the config13 struct into this for processing
// Ideally the tune would also be able to use this struct directly, but that would require some refactoring of how the config pages are stored and accessed, 
// so for now we keep using config13 for the tune and convert to programmableOutputRule for processing
struct rule_t {
  uint8_t combineOpType;
  compOperation_t firstOp; ///< First operand for comparison
  compOperation_t secondOp; ///< Second operand for comparison, used if bitwise operation is enabled

  rule_t() = default;
  rule_t(const config13& page13, uint8_t index) 
   : combineOpType(page13.operation[index].bitwise),
     firstOp({page13.operation[index].firstCompType, page13.firstDataIn[index], page13.firstTarget[index]}),
     secondOp({page13.operation[index].secondCompType, page13.secondDataIn[index], page13.secondTarget[index]})
  {
  }

  bool evaluateCombineOp(bool lhs, bool rhs) const;
  bool evaluate(const state_t& state, getDataFn pGetData) const;
};

// The struct representing the state of each programmable I/O channel, used for processing the 
// rules and keeping track of delays and active status.
struct channel_state_t
{
  bool isRuleActive : 1;
  bool isOutputActive : 1;
  uint8_t _index : 3;
  uint8_t activationDelayCount = 0;
  uint8_t outputDelayCount = 0;

  channel_state_t()
  : isRuleActive(false), isOutputActive(false), _index(0U)
  {}

  void initialize(const config13& page13, uint8_t index);
};

/**
 * @brief A struct that captures enough information to process a channel.
 * Instances of this class are ephemeral and created on the fly as needed.
 * 
 * This only exists to lower memory pressure: without that constraint, this
 * would be merged with stateful_channel_t (or access the @ref config13 arrays
 * directly). 
 */
struct processing_channel_t
{
  channel_state_t& _channel_state;
  uint8_t outputPin = 0;   ///< Disable(0) or enable (set to valid pin number) Programmable Pin (output/target pin to set)
  bool isPinValid : 1;
  bool isOutputInverted : 1; ///< Invert (on/off) value before writing to output pin (for all programmable I/O:s).
  LimitingType limitType; ///< Select which kind of output limiting are active (0 - minimum | 1 - maximum)
  uint8_t outputTimeLimit = 0; ///< Output delay for each programmable I/O, kindOfLimiting bit dependent(Unit: 0.1S)
  uint8_t activationDelay = 0; ///< Output write delay for each programmable I/O (Unit: 0.1S)

  processing_channel_t(const config13 &page13, channel_state_t& channel_state);

  bool isPhysicalPin(void) const {
    return outputPin < 128U;
  }

  bool hasMaxLimit(void) const {
    return (limitType==LimitingType::Max) && (outputTimeLimit != 0);
  }

  bool outputDelayExpired(void) const {
    return _channel_state.outputDelayCount > outputTimeLimit;
  }

  bool activationDelayExpired(void) const {
    return _channel_state.activationDelayCount > activationDelay;
  }

  void incrementOutputDelay(void);

  void incrementActivationDelay(void) {
    ++_channel_state.activationDelayCount;
  }
};

// The struct representing the current state of the programmable I/O system
struct state_t
{
  channel_state_t channels[_countof(config13::outputPin)];

  uint8_t compressedOutputStatus(void) const;
};

}