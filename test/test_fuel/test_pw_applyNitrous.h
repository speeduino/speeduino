#pragma once

#include "globals.h"

static inline void setup_nitrous_stage1(config10 &page10, statuses &current) {
  page10.n2o_stage1_minRPM = 20; // RPM/100
  page10.n2o_stage1_maxRPM = 30; // RPM/100
  page10.n2o_stage1_adderMin = 11; // milliseconds
  page10.n2o_stage1_adderMax = 3; // milliseconds

  current.nitrous_status = NITROUS_STAGE1;
  current.RPM = (page10.n2o_stage1_minRPM+4)*100;
}
static constexpr uint16_t NITROUS_STAGE1_ADDPW = 800;//780;

static inline void setup_nitrous_stage2(config10 &page10, statuses &current) {
  page10.n2o_stage2_minRPM = 25; // RPM/100
  page10.n2o_stage2_maxRPM = 30; // RPM/100
  page10.n2o_stage2_adderMin = 7; // milliseconds
  page10.n2o_stage2_adderMax = 1; // milliseconds

  current.nitrous_status = NITROUS_STAGE2;
  current.RPM = (page10.n2o_stage2_maxRPM-3U)*100;  
}
static constexpr uint16_t NITROUS_STAGE2_ADDPW = 500;//460;
