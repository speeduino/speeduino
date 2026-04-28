/** @file
 * Custom Programmable I/O.
 * The config related to Programmable I/O is found on page13 (of type @ref config13).
 */
#include "programmableIOControl.h"
#include "programmableIOControl_internals.h"
#include "logger.h"
#include "units.h"
#include "unit_testing.h"

using namespace programmableIOControl_details;

TESTABLE_STATIC state_t state; // The current state of the programmable I/O system, including the status of each channel and its timers

void __attribute__((optimize("Os"))) initialiseProgrammableIO(statuses& current, const config13& page13)
{
  for (uint8_t y = 0; y < _countof(state_t::channels); y++)
  {
    rule_t rule(page13, y);
    state.channels[y].initialize(rule);
    if (state.channels[y].isPinValid && rule.isPhysicalPin()) 
    {
      pinMode(rule.outputPin, OUTPUT);
      digitalWrite(rule.outputPin, rule.isOutputInverted);
    }
  }
  current.outputsStatus = state.compressedOutputStatus();
}

TESTABLE_INLINE_STATIC int16_t getComparisonData(uint8_t request, int16_t (*getData)(uint16_t index))
{
  int16_t data = 0;
  if ( request >= REUSE_RULES )
  {
    request -= REUSE_RULES;
    if ( request <= _countof(state.channels) ) 
    { 
      data = state.channels[request].isRuleActive; 
    }
  }
  else 
  { 
    data = getData(request); 
  }

  return data;
}

TESTABLE_INLINE_STATIC bool evaluateComparisonOp(uint8_t compType, int16_t lhs, int16_t rhs)
{
  switch (compType) {
    case COMPARATOR_EQUAL: return lhs == rhs;
    case COMPARATOR_NOT_EQUAL: return lhs != rhs;
    case COMPARATOR_GREATER: return lhs > rhs;
    case COMPARATOR_GREATER_EQUAL: return lhs >= rhs;
    case COMPARATOR_LESS: return lhs < rhs;
    case COMPARATOR_LESS_EQUAL: return lhs <= rhs;
    case COMPARATOR_AND: return (lhs & rhs) != 0;
    case COMPARATOR_XOR: return (lhs ^ rhs) != 0;
    default: return false; // Invalid comparator type
  }
}

static inline bool evaluateComparisonOp(const compOperation_t& operation, int16_t (*getData)(uint16_t index))
{
  return evaluateComparisonOp(operation.opType, getComparisonData(operation.dataIndex, getData), operation.target);
}

TESTABLE_INLINE_STATIC bool evaluateBitwiseOp(uint8_t compType, bool lhs, bool rhs)
{
  switch (compType) {
    case BITWISE_AND: return (lhs & rhs) != 0;
    case BITWISE_OR: return (lhs | rhs) != 0;
    case BITWISE_XOR: return (lhs ^ rhs) != 0;
    default: return false; // Invalid bitwise operator type
  }
}

static inline bool outputDelayExpired(const rule_t& rule, const channel_t& channel)
{
  return (channel.outputDelayCount > rule.outputTimeLimit);
}

TESTABLE_INLINE_STATIC bool applyOutputTimeLimit(const rule_t& rule, const channel_t& channel, bool ruleActive)
{
  return ruleActive && !(rule.hasMaxLimit() && outputDelayExpired(rule, channel));
}

TESTABLE_INLINE_STATIC bool isRuleActive(const rule_t& rule, const channel_t &channel, int16_t (*getData)(uint16_t index)) noexcept
{
  bool firstCheck = evaluateComparisonOp(rule.firstOp, getData);

  if ((rule.combineOpType != BITWISE_DISABLED) && (rule.secondOp.dataIndex <= (REUSE_RULES + _countof(state.channels))) ) //Failsafe check
  {
    bool secondCheck = evaluateComparisonOp(rule.secondOp, getData);
    firstCheck = evaluateBitwiseOp(rule.combineOpType, firstCheck, secondCheck);
  }

  return applyOutputTimeLimit(rule, channel, firstCheck);
}

static inline void updateChannelStatus(channel_t& channel, const rule_t& rule, bool ruleActive) noexcept
{
  channel.isOutputActive = rule.isOutputInverted ^ ruleActive;
  if (rule.isPhysicalPin()) { 
    digitalWrite(rule.outputPin, channel.isOutputActive); 
  } else {
    channel.isRuleActive = channel.isOutputActive;
  }
}

TESTABLE_INLINE_STATIC uint8_t nextOutDelay(const channel_t& channel, const rule_t& rule)
{
  if (rule.limitType==LimitingType::Max)
  {
    //Released before Maximum time, set delay to maximum to flip the output next
    if (channel.isOutputActive)
    {
      return rule.outputTimeLimit + 1; 
    }
  
    return 1; //Reset the counter for next time
  }
  return channel.outputDelayCount + 1;
}

/** Check all (8) programmable I/O:s and carry out action on output pin as needed.
 * Compare 2 (16 bit) vars in a way configured by @ref cmpOperation (see also @ref config13.operation).
 * Use ProgrammableIOGetData() to get 2 vars to compare.
 * Skip all programmable I/O:s where output pin is set 0 (meaning: not programmed).
 */
TESTABLE_INLINE_STATIC void checkProgrammableIO(const config13& page13, int16_t (*getData)(uint16_t index))
{
  for (uint8_t y = 0; y < _countof(state_t::channels); y++)
  {
    auto& channel = state.channels[y];
    if ( channel.isPinValid )
    {
      rule_t rule(page13, y);
      if (isRuleActive(rule, channel, getData))
      {
        ++channel.activationDelayCount;
        if (channel.activationDelayCount > rule.activationDelay)
        {
          if (channel.isOutputActive && !outputDelayExpired(rule, channel)) { ++channel.outputDelayCount; }
          updateChannelStatus(channel, rule, true);
        }
      }
      else
      {
        channel.outputDelayCount = nextOutDelay(channel, rule);
        if (outputDelayExpired(rule, channel))
        {
          if(rule.limitType==LimitingType::Min) { channel.outputDelayCount = 0; }
          updateChannelStatus(channel, rule, false);
        }

        channel.activationDelayCount = 0;
      }
    }
  }
}

// LCOV_EXCL_START
uint8_t checkProgrammableIO(const config13& page13)
{
  checkProgrammableIO(page13, ProgrammableIOGetData);
  return state.compressedOutputStatus();
}
// LCOV_EXCL_STOP

/** Get single I/O data var (from current) for comparison.
 * @param index - Field index/number (?)
 * @return 16 bit (int) result
 */
TESTABLE_INLINE_STATIC int16_t ProgrammableIOGetData(uint16_t index, byte (*pGetLogEntry)(uint16_t byteNum))
{
  int16_t result;
  if ( index < LOG_ENTRY_SIZE )
  {
    if(is2ByteEntry(index)) { result = word(pGetLogEntry(index+1), pGetLogEntry(index)); }
    else { result = pGetLogEntry(index); }
    
    //Special cases for temperatures
    if( (index == 6) || (index == 7) ) { result = temperatureRemoveOffset(result); }
  }
  else if ( index == 239U ) { result = (int16_t)max((uint32_t)runSecsX10, (uint32_t)32768); } //STM32 used std lib
  else { result = -1; } //Index is bigger than fullStatus array
  return result;
}

// LCOV_EXCL_START
int16_t ProgrammableIOGetData(uint16_t index)
{
  return ProgrammableIOGetData(index, getTSLogEntry);
}
// LCOV_EXCL_STOP