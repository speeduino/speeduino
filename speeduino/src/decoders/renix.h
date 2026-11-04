/**
 * @file 
 * @brief Renix 44-2-2  and 66-2-2-2 decoder.
 * 
 * Renix trigger wheel doesn't decode into 360 degrees nicely (360/44 = 8.18 degrees or 360/66 = 5.454545). Speeduino can't handle any teeth that have a decimal point.
 * Solution is to count teeth, every 11 teeth = a proper angle. For 66 tooth decoder its 60 degrees per 11 teeth, for 44 tooth decoder its 90 degrees per 11 teeth.
 * This means the system sees 4 teeth on the 44 tooth wheel and 6 teeth on the 66 tooth wheel.
 * Double missing tooth in the pattern is actually a large tooth and a large gap. If the trigger is set to rising you'll see the start of the large tooth
 * then the gap. If its not set to rising the code won't work due to seeing two gaps
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace renix {

  shared::state_t initialise(const config2 &page2, config4 &page4);

}
}