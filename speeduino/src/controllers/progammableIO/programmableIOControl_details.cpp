#include "programmableIOControl_details.h"
#include "../../../bit_manip.h"
#include "../../../globals.h"

namespace programmableIOControl_details {

static uint8_t validatePin(uint8_t pin)
{
  if (pinIsUsed(pin))
  {
    return NOT_A_PIN;
  }
  return pin;
}

void __attribute__((optimize("Os"))) channel_state_t::initialize(config13& page13, uint8_t index) 
{
  _index = index;
  isRuleActive = false;
  activationDelayCount = 0;
  outputDelayCount = 0;
  // A physical output pin can only be driven by one function. If this rule's pin is
  // already claimed elsewhere, clear it in the tune (disabling the rule) rather than
  // letting the rule silently never run. Clearing the pin changes the page CRC, so
  // TunerStudio detects the conflict and surfaces it to the user. Virtual/cascade
  // pins (>=128) and already-disabled slots (0) are left untouched.
  page13.outputPin[index] = _pin = validatePin(page13.outputPin[index]);

  isOutputInverted = BIT_CHECK(page13.outputInverted, index);
  isOutputActive = isPinValid() && isOutputInverted;
        
  if (isPinValid() && isPhysicalPin()) 
  {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, isOutputInverted);
  }
}

void channel_state_t::updateStatus(bool ruleActive) noexcept
{
  if (isPinValid())
  {
    isOutputActive = isOutputInverted ? !ruleActive : ruleActive;
    if (isPhysicalPin()) { 
      digitalWrite(_pin, isOutputActive); 
    } else {
      isRuleActive = isOutputActive;
    }
  }
}


processing_channel_t::processing_channel_t(const config13 &page13, channel_state_t& channel_state)
: _channel_state(channel_state)
, limitType(BIT_CHECK(page13.kindOfLimiting, channel_state._index) ? LimitingType::Max : LimitingType::Min)
, outputTimeLimit(page13.outputTimeLimit[channel_state._index])
, activationDelay(page13.outputDelay[channel_state._index])
{
}

void processing_channel_t::incrementOutputDelay(void)
{
  if (limitType==LimitingType::Max)
  {
    //Released before Maximum time, set delay to maximum to flip the output next
    if (_channel_state.isOutputActive)
    {
      _channel_state.outputDelayCount = outputTimeLimit + 1; 
    }
    else
    {
      _channel_state.outputDelayCount = 1; //Reset the counter for next time
    }
  }
  else
  {
    ++_channel_state.outputDelayCount;
  }
}

uint8_t __attribute__((optimize("Os"))) state_t::compressedOutputStatus(void) const
{
  uint8_t status = 0;
  for (uint8_t i = 0; i < _countof(state_t::channels); i++)
  {
    BIT_WRITE(status, i, channels[i].isOutputActive);
  }
  return status;    
}

int16_t compOperation_t::getComparisonData(const state_t& state, getDataFn pGetData) const
{
  int16_t data = 0;
  if ( isVirtualData() )
  {
    uint8_t realIndex = dataIndex - REUSE_RULES; 
    if ( realIndex < _countof(state_t::channels) ) 
    { 
      data = state.channels[realIndex].isRuleActive; 
    }
  }
  else 
  { 
    data = pGetData(dataIndex); 
  }

  return data;
}

bool compOperation_t::evaluate(int16_t lhs, int16_t rhs) const
{
  switch (opType)
  {
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

bool compOperation_t::evaluate(const state_t& state, getDataFn pGetData) const
{
  int16_t lhs = getComparisonData(state, pGetData);
  return evaluate(lhs, target);
}

bool rule_t::evaluateCombineOp(bool lhs, bool rhs) const
{
  switch (combineOpType) {
    case COMBINE_AND: return lhs && rhs;
    case COMBINE_OR: return lhs || rhs;
    case COMBINE_XOR: return lhs != rhs;
    default: return false; // Invalid bitwise operator type
  }
}

bool rule_t::evaluate(const state_t& state, getDataFn pGetData) const
{
  bool firstCheck = firstOp.evaluate(state, pGetData);

  if ((combineOpType != COMBINE_DISABLED) && (secondOp.dataIndex < (REUSE_RULES + _countof(state.channels))) ) //Failsafe check
  {
    bool secondCheck = secondOp.evaluate(state, pGetData);
    firstCheck = evaluateCombineOp(firstCheck, secondCheck);
  }

  return firstCheck;

}


} // namespace programmableIOControl_details