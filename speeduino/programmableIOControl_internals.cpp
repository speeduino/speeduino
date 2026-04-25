#include "programmableIOControl_internals.h"
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

}