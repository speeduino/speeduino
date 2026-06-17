#include "programmableIOControl_details.h"
#include "../../../bit_manip.h"
#include "../../../globals.h"

namespace programmableIOControl_details {

void channel_t::initialize(const config13& page13, uint8_t index) 
{
    rule_t rule(page13, index);
    isPinValid = rule.outputPin>0 && (rule.isCascadeRule() || !pinIsUsed(rule.outputPin));
    isRuleActive = false;
    activationDelayCount = 0;
    outputDelayCount = 0;
    isOutputActive = isPinValid && rule.isOutputInverted;
    _index = index;

    if (isPinValid && rule.isPhysicalPin()) 
    {
      pinMode(rule.outputPin, OUTPUT);
      digitalWrite(rule.outputPin, rule.isOutputInverted);
    }
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
    if ( (dataIndex - REUSE_RULES) < _countof(state_t::channels) ) 
    { 
      data = state.channels[dataIndex - REUSE_RULES].isRuleActive; 
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