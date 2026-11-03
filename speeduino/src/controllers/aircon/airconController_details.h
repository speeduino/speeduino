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
        uint8_t startDelay = 0;
        uint8_t tpsLockoutDelay = 0;
        uint8_t rpmLockoutDelay = 0;
        uint8_t afterEngineStartDelay = 0;

        bool nextAfterEngineStartDelay(const config15 &page15);
        void resetAfterEngineStartDelay(void);
        bool afterEngineStartDelayExpired(const config15 &page15) const;

        bool nextStartDelay(const config15 &page15);
        void resetStartDelay(void);

        bool nextTpsLockoutDelay(const config15 &page15);
        void resetTpsLockoutDelay(void);

        bool nextRpmLockoutDelay(const config15 &page15);
        void resetRpmLockoutDelay(void);     
    };

} // namespace details

} // namespace airConController