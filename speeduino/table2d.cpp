/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include "table2d.h"
#if !defined(UNIT_TEST)
#include "globals.h"
#endif

uint8_t _table2d_detail::getCacheTime(void) {
#if !defined(UNIT_TEST)
  return currentStatus.secl;
#else
  return 0;
#endif
}


uint8_t _table2d_detail::interpolate(const uint8_t axisValue, const Bin<uint8_t> &axisBin, const Bin<uint8_t> &valueBin) {
  // Using uint16_t to avoid overflow when calculating the result
  uint16_t m = axisValue - axisBin.lowerValue();
  uint8_t xRange = axisBin.upperValue() - axisBin.lowerValue();

  uint8_t yMax = valueBin.upperValue();
  uint8_t yMin = valueBin.lowerValue();
  
  if (yMin>yMax) {
    uint8_t yRange = yMin-yMax;
    return yMin - ((m * yRange) / xRange);
  } else {
    uint8_t yRange = yMax-yMin;
    return ((m * yRange) / xRange) + yMin;
  }
}
