#pragma once

#include "src/decoders/k6a.h"

constexpr uint32_t BASE_1ST_TOOTH_TIME = 1000000UL;
constexpr uint32_t BASE_2ND_TOOTH_TIME = 2000000UL;

struct context_k6a_t
{
  statuses current = {};
  config2 page2 = {};
  config4 page4 = {};
  decoders::shared::state_t decoderState;

  void init(void)
  {
    decoderState = decoders::k6a::initialise(1000, current, page4);
  }

  void triggerPrimary(uint32_t curTime)
  {
    decoders::k6a::triggerPrimary(curTime, current, decoderState, page2, page4);
  }
  
  void setEndTeeth(void)
  {
    decoders::k6a::setEndTeeth(decoderState, page4);
  }

  uint32_t getRevolutionTime()
  {
    return decoders::k6a::getRevolutionTime(current, decoderState);
  }

  auto getCrankAngle(uint32_t currMicros)
  {
    return decoders::k6a::getCrankAngle(currMicros, decoderState, page4);
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
