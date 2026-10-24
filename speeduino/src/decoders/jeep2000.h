/**
 * @file 
 * @brief Jeep 2000 Decoder: 1991 to 2000 6 cylinder Jeep engines
 * 
 * 24 crank teeth over 720 degrees, in groups of 4.
 * Crank wheel is high for 360 crank degrees. Quite similar to the 24X setup.
 * As we only need timing within 360 degrees, only 12 tooth angles are defined.
 * Tooth number 1 represents the first tooth seen after the cam signal goes high.
 * 
 * @note Useful references: www.speeduino.com/forum/download/file.php?id=205
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace jeep2000 {

    shared::state_t initialise(uint32_t curMicros, const statuses &current);

}
}