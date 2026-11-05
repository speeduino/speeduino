#pragma once

#include "src/decoders/fordtfi.h"

constexpr uint32_t BASE_1ST_TOOTH_TIME = 1000000UL;
constexpr uint32_t BASE_2ND_TOOTH_TIME = 2000000UL;

struct context_ford_tfi_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(uint8_t nCylinders)
  {
    page2.nCylinders = nCylinders;
    decoderState = decoders::ford_tfi::initialise(page2);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::ford_tfi::triggerPrimary(curTime, current, decoderState, page2, page4);
  }

  void triggerSecondary(uint32_t curTime)
  {
    decoders::ford_tfi::triggerSecondary(curTime, current, decoderState, page4);
  }
  
  void setEndTeeth(void)
  {
    decoders::ford_tfi::setEndTeeth(decoderState, page2, page4);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::ford_tfi::getRevolutionTime(current, decoderState, page4);
  }

  auto getCrankAngle(uint32_t currMicros)
  {
    return decoders::ford_tfi::getCrankAngle(currMicros, decoderState, page4);
  }

  void setup_primaryfilter(uint32_t lastGap, uint32_t filter)
  {
    decoderState.toothLastToothTime = BASE_1ST_TOOTH_TIME;
    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime - lastGap;
    decoderState.toothOneTime = BASE_1ST_TOOTH_TIME;
    decoderState.toothOneMinusOneTime = decoderState.toothOneTime - lastGap;
    decoderState.triggerFilterTime = filter;
  }

  // void setup_secondaryfilter(uint32_t lastGap, uint32_t filter)
  // {
  //   decoderState.toothLastSecToothTime = BASE_2ND_TOOTH_TIME;
  //   decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime - lastGap;
  //   decoderState.triggerSecFilterTime = filter;
  // }

  // void setup_ign_inj(uint8_t ignMode, uint8_t injLayout)
  // {
  //   page4.sparkMode = ignMode;
  //   page2.injLayout = injLayout;
  // }
};
