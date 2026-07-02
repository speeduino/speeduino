#pragma once

#include "globals.h"
#include "crankMaths.h"

void setEngineSpeed(uint16_t rpm, int16_t max_crank)
{
  extern void SetRevolutionTime(uint32_t revTime);

  SetRevolutionTime(UDIV_ROUND_CLOSEST(60UL*1000000UL, rpm, uint32_t));
  CRANK_ANGLE_MAX_IGN = max_crank;
  CRANK_ANGLE_MAX_INJ = max_crank;
//   dwellAngle = timeToAngleDegPerMicroSec(DWELL_TIME_MS*1000UL);
}
