#pragma once

#include "../../pins/boardOutputPin.h"

namespace fuelPumpController {

namespace detsil {

struct pump_state_t
{
    boardOutputPin_t pump_pin;
    uint8_t fpPrimeTime = 0; ///< The time (in seconds, based on @ref statuses.secl) that the fuel pump started priming
    bool isPumpOn : 1;
    bool isPrimingComplete : 1;

    pump_state_t(void)
    : isPumpOn(false)
    , isPrimingComplete(false)
    {
    }
};

} // detail

} // fuelPumpController