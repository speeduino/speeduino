/**
 * @file 
 * @brief Nissan 360 tooth on cam (Optical trigger disc inside distributor housing).
 * 
 * @see http://wiki.r31skylineclub.com/index.php/Crank_Angle_Sensor.
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace nissan360 {

    shared::state_t initialise(void);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

}
}