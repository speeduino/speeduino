#pragma once

#include <stdint.h>
#include "config_pages.h"
#include "../../pins/inputPin.h"
#include "../../pins/outputPin.h"

namespace airConController {

namespace details {

    struct state_t
    {
        inputPin_t reqPin;
        outputPin_t compPin;
        outputPin_t fanPin;
        bool isEnabled = false;
        bool standAloneFanIsEnabled = false;
        bool waitedAfterCranking = false; // This starts false and prevents the A/C from running until a few seconds after cranking
        uint8_t startDelay = 0;
        uint8_t tpsLockoutDelay = 0;
        uint8_t rpmLockoutDelay = 0;
        uint8_t afterEngineStartDelay = 0;

        void nextAfterEngineStartDelay(const config15 &page15);
        void resetAfterEngineStartDelay(void);

        bool nextStartDelay(const config15 &page15);
        void resetStartDelay(void);

        bool nextTpsLockoutDelay(const config15 &page15);
        void resetTpsLockoutDelay(void);

        bool nextRpmLockoutDelay(const config15 &page15);
        void resetRpmLockoutDelay(void);     
    };

} // namespace details

} // namespace airConController