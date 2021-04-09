#pragma once
#include "table3d.h"
#include "table_iterator.h"
#include "globals.h"

struct table3D {

  //All tables must be the same size for simplicity

  byte xSize;
  byte ySize;

  byte **values;
  int16_t *axisX;
  int16_t *axisY;

  table3DGetValueCache getValueCache;
};

void table3D_setSize(struct table3D *targetTable, byte);


inline int get3DTableValue(struct table3D *fromTable, int y, int x)
{
    return get3DTableValue(&fromTable->getValueCache,
                    fromTable->xSize,
                    fromTable->values,
                    fromTable->axisX,
                    fromTable->axisY,
                    y, x);
}

// ================================================================

int8_t getTableYAxisFactor(const table3D *pTable);

int8_t getTableXAxisFactor(const table3D *);

// ========================= ITERATION ========================= 

inline table_axis_iterator_t y_begin(const table3D *pTable)
{
    return y_begin(pTable->axisY, pTable->ySize, getTableYAxisFactor(pTable));
}

inline table_axis_iterator_t y_rbegin(const table3D *pTable)
{
    return y_rbegin(pTable->axisY, pTable->ySize, getTableYAxisFactor(pTable));
}

inline table_axis_iterator_t x_begin(const table3D *pTable)
{
    return x_begin(pTable->axisX, pTable->xSize, getTableXAxisFactor(pTable));
} 

inline table_row_iterator_t rows_begin(const table3D *pTable)
{
    return rows_begin(pTable->values, pTable->xSize);
};