#pragma once

#include "globals.h"

// Gather all inputs to the calculation into one place
struct ComputePulseWidthsContext {
  config2 page2 = {};
  config6 page6 = {};
  config10 page10 = {};
  statuses current = {};
};

static inline ComputePulseWidthsContext getBasicPwContext(void) {
  ComputePulseWidthsContext context = {};
  context.current.nitrous_status = NITROUS_OFF;
  context.page10.stagingEnabled = false;
  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_OFF;
  context.page2.battVCorMode = BATTV_COR_MODE_WHOLE;
  context.page2.includeAFR = false;
  context.page2.incorporateAFR = false; 
  context.page2.dutyLim = 100;
  context.current.revolutionTime = UINT16_MAX;
  context.current.nSquirts = 1;
  return context;
}

static inline void setup_nitrous_stage1(config10 &page10, statuses &current) {
  page10.n2o_stage1_minRPM = 20; // RPM/100
  page10.n2o_stage1_maxRPM = 30; // RPM/100
  page10.n2o_stage1_adderMin = 11; // milliseconds
  page10.n2o_stage1_adderMax = 3; // milliseconds

  current.nitrous_status = NITROUS_STAGE1;
}

static inline void setup_nitrous_stage2(config10 &page10, statuses &current) {
  page10.n2o_stage2_minRPM = 25; // RPM/100
  page10.n2o_stage2_maxRPM = 30; // RPM/100
  page10.n2o_stage2_adderMin = 7; // milliseconds
  page10.n2o_stage2_adderMax = 1; // milliseconds

  current.nitrous_status = NITROUS_STAGE2;
}