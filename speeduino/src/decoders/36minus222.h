/**
 * @file 
 * @brief 36-2-2-2 crank based trigger wheel.
 * 
 * A crank based trigger with a nominal 36 teeth, but 6 of these removed in 3 groups of 2.
 * 2 of these groups are located concurrently.
 * @note: This decoder supports both the H4 version (13-missing-16-missing-1-missing) and the H6 version of 36-2-2-2 (19-missing-10-missing-1-missing)
 * The decoder checks which pattern is selected in order to determine the tooth number
 * 
 *  @see www.thefactoryfiveforum.com/attachment.php?attachmentid=34279&d=1412431418
 */
#pragma once

#include "shared/decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace thirtysixminus222 {

  shared::state_t initialise(const config4 &page4);

  void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4);

  uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config2 &page2, const config4 &page4);

  void setEndTeeth(const statuses &current, shared::state_t &decoderState, const config2 &page2);
}
}