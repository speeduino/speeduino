#include "programmableIOControl_details.h"
#include "../../../bit_manip.h"
#include "../../../globals.h"

namespace programmableIOControl_details {

void channel_state_t::initialize(const config13& page13, uint8_t index) 
{
    _index = index;
    isRuleActive = false;
    activationDelayCount = 0;
    outputDelayCount = 0;
    processing_channel_t pChannel(page13, *this);
    isOutputActive = pChannel.isPinValid && pChannel.isOutputInverted;
          
    if (pChannel.isPinValid && pChannel.isPhysicalPin()) 
    {
      pinMode(pChannel.outputPin, OUTPUT);
      digitalWrite(pChannel.outputPin, pChannel.isOutputInverted);
    }
}

static inline bool isValidOutputPin(uint8_t pin)
{
  return (pin>0U)
      && ((pin>128U) || !pinIsUsed(pin))
  ;
}

processing_channel_t::processing_channel_t(const config13 &page13, channel_state_t& channel_state)
: _channel_state(channel_state)
, outputPin(page13.outputPin[channel_state._index])
, isPinValid(isValidOutputPin(page13.outputPin[channel_state._index]))
, isOutputInverted(BIT_CHECK(page13.outputInverted, channel_state._index))
, limitType(BIT_CHECK(page13.kindOfLimiting, channel_state._index) ? LimitingType::Max : LimitingType::Min)
, outputTimeLimit(page13.outputTimeLimit[channel_state._index])
, activationDelay(page13.outputDelay[channel_state._index])
{
}


uint8_t state_t::compressedOutputStatus(void) const
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

  if ((combineOpType != COMBINE_DISABLED) && (secondOp.dataIndex <= (REUSE_RULES + _countof(state.channels))) ) //Failsafe check
  {
    bool secondCheck = secondOp.evaluate(state, pGetData);
    firstCheck = evaluateCombineOp(firstCheck, secondCheck);
  }

  return firstCheck;

}


} // namespace programmableIOControl_details