#pragma once

#include <stdint.h>
#include "../../../statuses.h"
#include "../../../config_pages.h"

void initialiseTachoControl(uint8_t tachoPin, const config2 &page2, const config6 &page6, const statuses &current);

void tachoControl(const statuses &current);
void tachoOutputOn(void);
void tachoOutputOff(void);

// Exposed here for unit testing
namespace tachoControl_detail
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
    };
}
