#pragma once

#include <stdint.h>
#ifdef USE_LIBDIVIDE
#include <libdivide.h>
#endif
#include "config_pages.h"

namespace decoders {

namespace detail {

    // Stores the internal decoder state(s)
    struct state_t {
        volatile unsigned long curGap = 0U;
        volatile unsigned long curGap2 = 0U;
        volatile unsigned long curGap3 = 0U;
        volatile unsigned long lastGap = 0U;
        volatile unsigned long targetGap = 0U;
        unsigned long MAX_STALL_TIME = 0U; // //The maximum time (in uS) that the system will continue to function before the engine is considered stalled/stopped. This is unique to each decoder, depending on the number of teeth etc. 500000 (half a second) is used as the default value, most decoders will be much less.
        uint16_t toothCurrentCount = 0; //The current number of teeth. Once sync has been achieved, this can never actually be 0.
        volatile byte toothSystemCount = 0; //Used for decoders such as Audi 135 where not every tooth is used for calculating crank angle. This variable stores the actual number of teeth, not the number being used to calculate crank angle    };
        volatile unsigned long toothSystemLastToothTime = 0; //As below, but used for decoders where not every tooth count is used for calculation
        volatile uint32_t toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
        volatile unsigned long toothLastSecToothTime = 0; //The time (micros()) that the last tooth was registered on the secondary input
        volatile unsigned long toothLastThirdToothTime = 0; //The time (micros()) that the last tooth was registered on the second cam input
        volatile uint32_t toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered
        volatile unsigned long toothLastMinusOneSecToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered on secondary input
        volatile unsigned long toothLastToothRisingTime = 0; //The time (micros()) that the last tooth rose (used by special decoders to determine missing teeth polarity)
        volatile unsigned long toothLastSecToothRisingTime = 0; //The time (micros()) that the last tooth rose on the secondary input (used by special decoders to determine missing teeth polarity)
        volatile unsigned long targetGap2 = 0;
        volatile unsigned long targetGap3 = 0;
        volatile unsigned long toothOneTime = 0; //The time (micros()) that tooth 1 last triggered
        volatile unsigned long toothOneMinusOneTime = 0; //The 2nd to last time (micros()) that tooth 1 last triggered
        volatile unsigned long lastSyncRevolution = 0; // the revolution value of last valid sync
        volatile bool revolutionOne = 0; // For sequential operation, this tracks whether the current revolution is 1 or 2 (not 1)
        uint16_t triggerActualTeeth = 0;
        volatile uint16_t secondaryToothCount = 0; //Used for identifying the current secondary (Usually cam) tooth for patterns with multiple secondary teeth
        volatile unsigned long triggerFilterTime = 0; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
        volatile unsigned long triggerSecFilterTime = 0; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the secondary input
        volatile unsigned long triggerThirdFilterTime = 0; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the Third input
        volatile uint16_t triggerToothAngle = 0; //The number of crank degrees that elapse per tooth
        uint8_t checkSyncToothCount = 0; //How many teeth must've been seen on this revolution before we try to confirm sync (Useful for missing tooth type decoders)        
        unsigned long lastVVTtime = 0; //The time between the vvt reference pulse and the last crank pulse        
        uint16_t ignitionEndTeeth[IGN_CHANNELS]{};
        int16_t toothAngles[24]{}; //An array for storing fixed tooth angles. Currently sized at 24 for the GM 24X decoder, but may grow later if there are other decoders that use this style        
        decoder_status_t decoderStatus{};
        decoder_features_t decoderFeatures{};
#ifdef USE_LIBDIVIDE
        libdivide::libdivide_s16_t divTriggerToothAngle{};
#endif

        /**
         * Sets the new filter time based on the current settings.
         * This ONLY works for even spaced decoders.
         */
        void setFilter(unsigned long curGap, const config4 &page4);
        
        /** @brief Reset tooth statues & times */
        void reset(void);
    };
}
}