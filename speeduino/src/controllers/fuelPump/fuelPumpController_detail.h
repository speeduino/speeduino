#pragma once

#include "../../pins/outputPin.h"

namespace fuelPumpController {

namespace detail {

struct pump_state_t
{
    outputPin_t pump_pin;
    uint8_t fpPrimeTime = 0; ///< The time (in seconds, based on @ref statuses.secl) that the fuel pump started priming
    uint8_t offDelay = 0;
    bool isPrimingComplete : 1;

    pump_state_t(void)
    : isPrimingComplete(false)
    {
    }
};

} // detail

} // fuelPumpController