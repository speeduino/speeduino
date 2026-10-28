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

}
}