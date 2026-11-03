#include "airconController_details.h"

namespace airConController {

namespace details {

bool state_t::nextAfterEngineStartDelay(const config15 &page15)
{
    if(afterEngineStartDelay < page15.airConAfterStartDelay)
    {
        ++afterEngineStartDelay;
    }
    return afterEngineStartDelayExpired(page15);
}

void state_t::resetAfterEngineStartDelay(void)
{
    afterEngineStartDelay = 0;
}

bool state_t::afterEngineStartDelayExpired(const config15 &page15) const
{
    return afterEngineStartDelay >= page15.airConAfterStartDelay;
}


bool state_t::nextStartDelay(const config15 &page15)
{
    if(startDelay <= page15.airConCompOnDelay)
    {
        ++startDelay;
    }
    return startDelay>page15.airConCompOnDelay;
}

void state_t::resetStartDelay(void)
{
    startDelay = 0;
}

bool state_t::nextTpsLockoutDelay(const config15 &page15)
{
    if(tpsLockoutDelay <= page15.airConTPSCutTime)
    {
        ++tpsLockoutDelay;
    }
    return tpsLockoutDelay>page15.airConTPSCutTime;
}

void state_t::resetTpsLockoutDelay(void)
{
    tpsLockoutDelay = 0;
}

bool state_t::nextRpmLockoutDelay(const config15 &page15)
{
    if(rpmLockoutDelay <= page15.airConRPMCutTime)
    {
        ++rpmLockoutDelay;
    }
    return rpmLockoutDelay>page15.airConRPMCutTime;
}

void state_t::resetRpmLockoutDelay(void)
{
    rpmLockoutDelay = 0;
}

} // namespace details

} // namespace airConController