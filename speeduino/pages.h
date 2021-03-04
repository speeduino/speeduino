#pragma once
#include <Arduino.h>
#include "table.h"
#include "globals.h"

//These are the page numbers that the Tuner Studio serial protocol uses to transverse the different map and config pages.
#define veMapPage    2
#define veSetPage    1 //Note that this and the veMapPage were swapped in Feb 2019 as the 'algorithm' field must be declared in the ini before it's used in the fuel table
#define ignMapPage   3
#define ignSetPage   4//Config Page 2
#define afrMapPage   5
#define afrSetPage   6//Config Page 3
#define boostvvtPage 7
#define seqFuelPage  8
#define canbusPage   9//Config Page 9
#define warmupPage   10 //Config Page 10
#define fuelMap2Page 11
#define wmiMapPage   12
#define progOutsPage 13
#define ignMap2Page  14

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

byte getPageValue(byte pageNum, uint16_t offset);

void setPageValue(byte pageNum, uint16_t offset, byte value);

uint32_t calculateCRC32(byte pageNum);