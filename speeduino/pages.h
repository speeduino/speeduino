#pragma once
#include <Arduino.h>
#include "table.h"
#include "globals.h"

inline int8_t getTableYAxisFactor(const table3D *pTable)
{
  if (pTable==&boostTable || pTable==&vvtTable)
  {
    return 1;  
  } 
  else
  {
    return TABLE_LOAD_MULTIPLIER;
  } 
}

inline int8_t getTableXAxisFactor(const table3D */*pTable*/ )
{
  return TABLE_RPM_MULTIPLIER;
}