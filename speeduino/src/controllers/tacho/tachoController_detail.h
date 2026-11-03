#pragma once

#include <stdint.h>
#include "../../../board_definition.h"
#include "../../pins/fastOutputPin.h"
#include "../../pins/outputPin.h"

namespace tachoController 
{

namespace detail
{
    enum class TachoOutputStatus : uint8_t {
        INACTIVE = 0, READY = 1, ACTIVE = 2 //The 3 statuses that the tacho output pulse can have. 
    };

    struct tacho_control_state
    {
        tacho_control_state(void);

        TachoOutputStatus tachoOutputFlag = TachoOutputStatus::INACTIVE;
        uint8_t tachoEndTime = 0; //The time (in ms) that the tacho pulse needs to end at
        uint8_t tachoDuration = 0;
        uint16_t controlCounter = 0;
        uint16_t tachoSweepIncr = 0;
        uint16_t tachoSweepAccum = 0;
        bool tachoSweepEnabled : 1;
        bool tachoAlt : 1;
        bool tachoHalf : 1;
        bool modeDwell : 1;
        boardOutputPin_t tach_pin;
    };
}

}