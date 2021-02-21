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


typedef enum { Raw, Table, None } page_subtype_t;

typedef struct
{
  typedef struct table_t {
    table3D *pTable;
    uint16_t offset;
  } table_t;
  typedef struct raw_t {
    void *pData;
    uint16_t offset;
  } raw_t;
  union {
    table_t table;
    raw_t raw;    
  };
  page_subtype_t type;
} entity_address;

// For some purposes a TS page is treated as a contiguous block of memory.
// However, in Speeduino it's sometimes made up of multiple distinct and
// non-contiguous chunks of data. This maps from the page address (number + offset)
// to the type & position of the corresponding memory block.
entity_address map_page_offset_to_memory(uint8_t pageNumber, uint16_t offset);