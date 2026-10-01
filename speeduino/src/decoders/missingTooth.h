#pragma once

#include "decoder_state.h"
#include "config_pages.h"
#include "statuses.h"

namespace decoders {

namespace missing_tooth {

    detail::state_t intialise(const config4 &page4);

    void triggerPrimary(uint32_t curTime, statuses &current, detail::state_t &decoderState, const config2 &page2, const config4 &page4);

    void triggerSecondary(uint32_t curTime, statuses &current, detail::state_t &decoderState, const config4 &page4, const config6 &page6, const config10 &page10);

    void triggerTertiary(uint32_t curTime, statuses &current, detail::state_t &decoderState, const config4 &page4, const config6 &page6);

    uint32_t getRevolutionTime(const statuses &current, detail::state_t &decoderState, const config4 &page4);

    void setEndTeeth(detail::state_t &decoderState, const config4 &page4);
}

}