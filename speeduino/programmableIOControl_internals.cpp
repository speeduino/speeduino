#include "programmableIOControl_internals.h"
#include "bit_manip.h"
#include "globals.h"

namespace programmableIOControl_details {

void channel_t::initialize(const rule_t& rule, uint8_t index) 
{
    isPinValid = rule.outputPin>0 && (rule.isCascadeRule() || !pinIsUsed(rule.outputPin));
    isRuleActive = false;
    activationDelayCount = 0;
    outputDelayCount = 0;
    isOutputActive = isPinValid && rule.isOutputInverted;
    _index = index;
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

} // namespace programmableIOControl_details