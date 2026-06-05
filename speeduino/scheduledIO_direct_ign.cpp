#include "scheduledIO_direct_ign.h"
#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "preprocessor.h"

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control
 
static fastOutputPin_t pins[IGN_CHANNELS];

static inline void coilLow(uint8_t channel)
{
    INTERNAL_TEST_ASSERT(channel>0 && channel<=_countof(pins));
    pins[channel-1U].setPinLow();
}

static inline void coilHigh(uint8_t channel)
{
    INTERNAL_TEST_ASSERT(channel>0 && channel<=_countof(pins));
    pins[channel-1U].setPinHigh();
}

using channelFunc = void(*)(uint8_t);
static channelFunc coilChargingFn = coilHigh;
static channelFunc coilDischargingFn = coilLow;

void initIgnDirectIO(const config4 &page4, const uint8_t (&pinNumbers)[_countof(pins)])
{
    for (uint8_t i = 0; i < _countof(pins); i++)
    {
        pins[i].setPin(pinNumbers[i], OUTPUT);
    }
    if (page4.IgInv == GOING_HIGH)
    {
        coilChargingFn = coilLow;
        coilDischargingFn = coilHigh;
    }
    else
    {
        coilChargingFn = coilHigh;
        coilDischargingFn = coilLow;
    }
}

void coilCharging_DIRECT(uint8_t channel) 
{ 
    coilChargingFn(channel);
}

void coilStopCharging_DIRECT(uint8_t channel) 
{
    coilDischargingFn(channel);
}

// LCOV_EXCL_STOP