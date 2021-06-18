#pragma once
#include <Arduino.h>
#include "table.h"
#include "globals.h"

inline int8_t getTableYAxisFactor(const table3D *pTable)
{
    return pTable==&boostTable || pTable==&vvtTable ? 1 : TABLE_LOAD_MULTIPLIER;
}

inline int8_t getTableXAxisFactor(const table3D *)
{
    return TABLE_RPM_MULTIPLIER;
}

// ========================= AXIS ITERATION =========================  
typedef struct table_axis_iterator_t
{
    int16_t *_pAxis;
    int16_t *_pAxisEnd;
    int16_t _axisFactor;
    int8_t _stride;
} table_axis_iterator_t;

inline table_axis_iterator_t& advance_axis(table_axis_iterator_t &it)
{ 
    it._pAxis += it._stride;
    return it;
}

inline bool at_end(const table_axis_iterator_t &it)
{
    return it._pAxis == it._pAxisEnd;
}

inline byte get_value(const table_axis_iterator_t &it)
{
    return *it._pAxis / it._axisFactor;
}

inline void set_value(table_axis_iterator_t &it, byte value)
{
    *it._pAxis = value * it._axisFactor;
}

inline table_axis_iterator_t y_begin(const table3D *pTable)
{
    return { pTable->axisY+(pTable->ySize-1), 
            (pTable->axisY)-1, getTableYAxisFactor(pTable),
            -1 };
}

inline table_axis_iterator_t x_begin(const table3D *pTable)
{
    return { pTable->axisX, pTable->axisX+pTable->xSize, getTableXAxisFactor(pTable), 1 };
} 

// ========================= INTRA-ROW ITERATION ========================= 

// A table row is directly iterable & addressable.
typedef struct table_row_t {
    byte *pValue;
    byte *pEnd;
} table_row_t;

inline bool at_end(const table_row_t &it)
{
    return it.pValue == it.pEnd;
}

// ========================= INTER-ROW ITERATION ========================= 
typedef struct table_row_iterator_t
{
    byte **pRowsStart;
    byte **pRowsEnd;
    uint8_t rowWidth;
} table_row_iterator_t;

inline table_row_iterator_t rows_begin(const table3D *pTable)
{
    return {    pTable->values + (pTable->ySize-1),
                pTable->values - 1,
                pTable->xSize
    };
};

inline bool at_end(const table_row_iterator_t &it)
{
    return it.pRowsStart == it.pRowsEnd;
}

inline table_row_t get_row(const table_row_iterator_t &it)
{
    return { *it.pRowsStart, (*it.pRowsStart) + it.rowWidth };
}

inline table_row_iterator_t& advance_row(table_row_iterator_t &it)
{
    --it.pRowsStart;
    return it;
}