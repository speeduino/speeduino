#pragma once

#include "table3d_typedefs.h"

// ========================= AXIS ITERATION =========================  

typedef uint8_t byte;

// Represents a 16-bit value as a byte
//
// Modelled after the Arduino EERef class
struct int16_ref
{
    int16_t *_pValue;
    uint8_t _factor;

    // Getters
    inline byte operator*() const { return (byte)(*_pValue /_factor); }
    inline operator byte() const { return **this; }
    // Setter
    inline int16_ref &operator=( byte in )  { return (*_pValue = (int16_t)in * (int16_t)_factor), *this;  }
};


typedef struct table_axis_iterator_t
{
    table3d_axis_t *_pAxis;
    table3d_axis_t *_pAxisEnd;
    uint8_t _axisFactor;
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

inline int16_ref get_value(const table_axis_iterator_t &it)
{
    return int16_ref { ._pValue = it._pAxis, ._factor = it._axisFactor };
}


inline table_axis_iterator_t y_begin(const table3d_axis_t *pAxis, table3d_dim_t size, uint8_t factor)
{
    return { const_cast<table3d_axis_t*>(pAxis)+(size-1), 
             const_cast<table3d_axis_t*>(pAxis)-1, 
             factor,
             -1 };
}

inline table_axis_iterator_t y_rbegin(const table3d_axis_t *pAxis, table3d_dim_t size, uint8_t factor)
{
    return { const_cast<table3d_axis_t*>(pAxis), const_cast<table3d_axis_t*>(pAxis)+size, factor, 1 };
}

inline table_axis_iterator_t x_begin(const table3d_axis_t *pAxis, table3d_dim_t size, uint8_t factor)
{
    return { const_cast<table3d_axis_t*>(pAxis), const_cast<table3d_axis_t*>(pAxis)+size, factor, 1 };
}

// ========================= INTRA-ROW ITERATION ========================= 

// A table row is directly iterable & addressable.
typedef struct table_row_t {
    table3d_value_t *pValue;
    table3d_value_t *pEnd;
} table_row_t;

inline bool at_end(const table_row_t &it)
{
    return it.pValue == it.pEnd;
}

inline table3d_value_t& get_value(const table_row_t &it)
{
    return *it.pValue;
}

// ========================= INTER-ROW ITERATION ========================= 
typedef struct table_row_iterator_t
{
    table3d_value_t *pRowsStart;
    table3d_value_t *pRowsEnd;
    uint8_t rowWidth;
} table_row_iterator_t;

inline table_row_iterator_t rows_begin(const table3d_value_t *pValues, table3d_dim_t axisSize)
{
    return {    const_cast<table3d_value_t*>(pValues) + (axisSize*(axisSize-1)),
                const_cast<table3d_value_t*>(pValues) - axisSize,
                axisSize
    };
}

inline bool at_end(const table_row_iterator_t &it)
{
    return it.pRowsStart == it.pRowsEnd;
}

inline table_row_t get_row(const table_row_iterator_t &it)
{
    return { it.pRowsStart, it.pRowsStart + it.rowWidth };
}

inline table_row_iterator_t& advance_row(table_row_iterator_t &it)
{
    it.pRowsStart -= it.rowWidth;
    return it;
}