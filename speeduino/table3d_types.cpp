#include "table3d_types.h"

#include "globals.h"
#define TABLE_RPM_MULTIPLIER  100
#define TABLE_LOAD_MULTIPLIER 2

int8_t getTableYAxisFactor(const table3D *pTable)
{
    return pTable==&boostTable || pTable==&vvtTable ? 1 : TABLE_LOAD_MULTIPLIER;
}

int8_t getTableXAxisFactor(const table3D *)
{
    return TABLE_RPM_MULTIPLIER;
}

//Define the total table memory sizes. Used for adding up the static heap size
#define TABLE3D_SIZE_16  (16 * 16 + 32 + 32 + (16 * sizeof(byte*))) //2 bytes for each value on the axis + allocation for array pointers
#define TABLE3D_SIZE_12  (12 * 12 + 24 + 24 + (12 * sizeof(byte*))) //2 bytes for each value on the axis + allocation for array pointers
#define TABLE3D_SIZE_8   (8 * 8 + 16 + 16 + (8 * sizeof(byte*))) //2 bytes for each value on the axis + allocation for array pointers
#define TABLE3D_SIZE_6   (6 * 6 + 12 + 12 + (6 * sizeof(byte*))) //2 bytes for each value on the axis + allocation for array pointers
#define TABLE3D_SIZE_4   (4 * 4 + 8 + 8 + (4 * sizeof(byte*))) //2 bytes for each value on the axis + allocation for array pointers

//Define the table sizes
#define TABLE_FUEL1_SIZE    16;
#define TABLE_FUEL2_SIZE    16;
#define TABLE_IGN1_SIZE     16;
#define TABLE_IGN2_SIZE     16;
#define TABLE_AFR_SIZE      16;
#define TABLE_STAGING_SIZE  8;
#define TABLE_BOOST_SIZE    8;
#define TABLE_VVT1_SIZE     8;
#define TABLE_VVT2_SIZE     8;
#define TABLE_WMI_SIZE      8;
#define TABLE_TRIM1_SIZE    6;
#define TABLE_TRIM2_SIZE    6;
#define TABLE_TRIM3_SIZE    6;
#define TABLE_TRIM4_SIZE    6;
#define TABLE_TRIM5_SIZE    6;
#define TABLE_TRIM6_SIZE    6;
#define TABLE_TRIM7_SIZE    6;
#define TABLE_TRIM8_SIZE    6;
#define TABLE_DWELL_SIZE    4;

/*
*********** WARNING! ***********
YOU MUST UPDATE THE TABLE COUNTS IN THE LINE BELOW WHENEVER A NEW TABLE IS ADDED!
*/
#define TABLE_HEAP_SIZE     ((5 * TABLE3D_SIZE_16) + (5 * TABLE3D_SIZE_8) + (8 * TABLE3D_SIZE_6) + (1 * TABLE3D_SIZE_4) + 1)

static uint8_t _3DTable_heap[TABLE_HEAP_SIZE];
static uint16_t _heap_pointer = 0;

void* heap_alloc(uint16_t size)
 {
     uint8_t* value = nullptr;
     if (size < (TABLE_HEAP_SIZE - _heap_pointer))
     {
         value = &_3DTable_heap[_heap_pointer];
         _heap_pointer += size;
     }
     return value;
 }


void table3D_setSize(struct table3D *targetTable, byte newSize)
{
  if(initialisationComplete == false)
  {
    /*
    targetTable->values = (byte **)malloc(newSize * sizeof(byte*));
    for(byte i = 0; i < newSize; i++) { targetTable->values[i] = (byte *)malloc(newSize * sizeof(byte)); }
    */
    targetTable->values = (byte **)heap_alloc(newSize * sizeof(byte*));
    for(byte i = 0; i < newSize; i++) { targetTable->values[i] = (byte *)heap_alloc(newSize * sizeof(byte)); }

    /*
    targetTable->axisX = (int16_t *)malloc(newSize * sizeof(int16_t));
    targetTable->axisY = (int16_t *)malloc(newSize * sizeof(int16_t));
    */
    targetTable->axisX = (int16_t *)heap_alloc(newSize * sizeof(int16_t));
    targetTable->axisY = (int16_t *)heap_alloc(newSize * sizeof(int16_t));
    targetTable->xSize = newSize;
    targetTable->ySize = newSize;
    targetTable->getValueCache.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
  } //initialisationComplete
}
