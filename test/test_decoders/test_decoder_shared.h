
#pragma once

#include "src/decoders/shared/decoder_state.h"

static inline uint32_t calcNextTimePassPrimaryFilter(const decoders::shared::state_t &decoderState)
{
  return decoderState.toothLastToothTime + decoderState.triggerFilterTime + 1;
}

static inline uint32_t calcNextTimePassSecondaryFilter(const decoders::shared::state_t &decoderState)
{
  return decoderState.toothLastSecToothTime + decoderState.triggerSecFilterTime + 1;
}
