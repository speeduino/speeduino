/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include "table2d.h"
#include "maths.h"
#if !defined(UNIT_TEST)
#include "globals.h"
#endif

uint8_t _table2d_detail::getCacheTime(void) 
{
#if !defined(UNIT_TEST)
  return currentStatus.secl;
#else
  return 0;
#endif
}

uint8_t _table2d_detail::interpolate(const uint8_t axisValue, const Bin<uint8_t> &axisBin, const Bin<uint8_t> &valueBin) 
{
  return ::fast_map(axisValue, axisBin.lowerValue(), axisBin.upperValue(), valueBin.lowerValue(), valueBin.upperValue());
}
