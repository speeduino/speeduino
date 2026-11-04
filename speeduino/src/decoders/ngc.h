/**
 * @file 
 * @brief A decoder for vehicles with 4, 6 and 8 cylinder Chrysler NGC pattern.
 * 
 *   * 4-cyl: 36+2-2 crank wheel and 7 tooth cam
 *   * 6-cyl: 36-2+2 crank wheel and 12 tooth cam in 6 groups
 *   * 8-cyl: 36-2+2 crank wheel and 15 tooth cam in 8 groups
 * 
 * The crank decoder uses the polarity of the missing teeth to determine position. 
 * The 4-cyl cam decoder uses the polarity of the missing teeth to determine position
 * The 6 and 8-cyl cam decoder uses the amount of teeth in the two previous groups of teeth to determine position
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace ngc {

    shared::state_t initialise(const config2 &page2, config4 &page4);

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary4(uint32_t curTime, const statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary68(uint32_t curTime, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

    uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4);

    void setEndTeeth(shared::state_t &decoderState, const config4 &page4);

}
}