#include "programmableIOControl_internals.h"
#include "bit_manip.h"
#include "globals.h"

namespace programmableIOControl_details {

void channel_t::initialize(const rule_t& rule) 
{
    isPinValid = rule.outputPin>0 && (rule.isCascadeRule() || !pinIsUsed(rule.outputPin));
    isRuleActive = false;
    activationDelayCount = 0;
    outputDelayCount = 0;
    isOutputActive = isPinValid && rule.isOutputInverted;
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

}