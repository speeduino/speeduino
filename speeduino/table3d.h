#pragma once

#include "table3d_interpolate.h"
#include "table3d_iterator.h"

// Overview
//
// Logical: each 3D table is a continuous height map spread over
// 2 orthogonal and bounded axes. The X & Y axes are non-linear.
// E.g. for a 3x3 table
//      Y-Max    V6      V7      V8
//      Y-Int    V3      V4      V5
//      Y-Min    V0      V1      V2
//              X-Min   X-Int   X-Max
// Assumptions:
//      The axis values are increasing. I.e. x[n] >= x[n-1]
//
// Physical:
//      The X axis is conventional: [0] is the min
//      The Y-axis is inverted: y[0] is the max
//      The value locations match the axes. E.g.for a 3x3 table:
//          value[0][0] is the first value on the last row.
//          value[2][0] ia the first value on the first row.

// We have a fixed number of table types: they are defined by this macro
// GENERATOR is expected to be another macros that takes at least 3 arguments:
//    axis length, x-axis domain, y-axis domain
#define TABLE_GENERATOR(GENERATOR, ...) \
    GENERATOR(6, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(4, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(8, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(8, Rpm, Tps, ##__VA_ARGS__) \
    GENERATOR(16, Rpm, Load, ##__VA_ARGS__)

// ================================== Core 3D table ========================

// With no inheritance or virtual functions, we need to pass around void*
// In order to cast that back to a concrete type, we need to somehow identify
// the type. 
//
// We use a packed enum for size (8 bit) & performance (enum values are adjacent)
#define TO_TYPE_KEY(size, xDom, yDom) table3d ## size ## xDom ## yDom ## _key
typedef enum __attribute__ ((__packed__)) /* Packed is required to minimize to 8-bit */ {
    table_type_None,
    #define GEN_TYPE_KEY(size, xDom, yDom) TO_TYPE_KEY(size, xDom, yDom),
    TABLE_GENERATOR(GEN_TYPE_KEY)
} table_type_t;

// 3D table type metadata
struct table3d_metadata {
    table_type_t type_key;
    table3d_dim_t axis_length;
    axis_metadata x_axis_meta;
    axis_metadata y_axis_meta;
};

// Each 3d table is given a distinct type based on size & axis domains
// This encapsulates the generation of the type name
#define DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) table3d ## size ## xDom ## yDom

// Generate the 3D table types
#define GEN_DECLARE_3DTABLE_TYPE(size, xDom, yDom) \
    struct DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) \
    { \
        /* This will take up zero space unless we take the address somewhere */ \
        static constexpr table3d_metadata _metadata = { \
            .type_key = TO_TYPE_KEY(size, xDom, yDom), \
            .axis_length = size, \
            .x_axis_meta = CREATE_AXIS_METADATA(size, xDom), \
            .y_axis_meta = CREATE_AXIS_METADATA(size, yDom), \
        }; \
        \
        table3DGetValueCache get_value_cache; \
        table3d_value_t values[size*size]; \
        table3d_axis_t axisX[size]; \
        table3d_axis_t axisY[size]; \
    };
TABLE_GENERATOR(GEN_DECLARE_3DTABLE_TYPE)

// Generate get3DTableValue() functions
#define GEN_GET3D_TABLE_VALUE(size, xDom, yDom) \
    inline int get3DTableValue(DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable, table3d_axis_t y, table3d_axis_t x) \
    { \
      return get3DTableValue( &pTable->get_value_cache, \
                              pTable->_metadata.axis_length, \
                              pTable->values, \
                              pTable->axisX, \
                              pTable->axisY, \
                              y, x); \
    } 
TABLE_GENERATOR(GEN_GET3D_TABLE_VALUE)

// Generate single byte value access function.
//
// Since table values aren't laid out linearily, converting a linear 
// offset to the equivalent memory address requires a modulus operation.
//
// This is slow, since AVR hardware has no divider. We can gain performance
// in 2 ways:
//  1. Forcing uint8_t calculations. These are much faster than 16-bit calculations
//  2. Compiling this per table *type*. This encodes the axis length as a constant
//  thus allowing the optimizing compiler more opportunity. E.g. for axis lengths
//  that are a power of 2, the modulus can be optimised to add/multiple/shift - much
//  cheaper than calling a software division routine such as __udivmodqi4
//
// THIS IS WORTH 20% to 30% speed up
//
// This limits us to 16x16 tables. If we need bigger and move to 16-bit 
// operations, consider using libdivide.
#define GEN_TABLE_VALUE(size, xDom, yDom) \
    inline byte& get_table_value(DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable, uint8_t linear_index) \
    { \
        static_assert(pTable->_metadata.axis_length<17, "Table is too big"); \
        constexpr uint8_t first_index = pTable->_metadata.axis_length*(pTable->_metadata.axis_length-1); \
        const uint8_t index = first_index + (2*(linear_index % pTable->_metadata.axis_length)) - linear_index; \
        return pTable->values[index]; \
    }
TABLE_GENERATOR(GEN_TABLE_VALUE)


// ================================== Iterator support ========================

#define GEN_ROWS_BEGIN(size, xDom, yDom) \
    inline table_value_iterator rows_begin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return table_value_iterator(pTable->values, DECLARE_3DTABLE_TYPENAME(size, xDom, yDom)::_metadata.axis_length);\
    }
TABLE_GENERATOR(GEN_ROWS_BEGIN)

#define GEN_Y_BEGIN(size, xDom, yDom) \
    inline table_axis_iterator y_begin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return table_axis_iterator::y_begin(pTable->axisY, DECLARE_3DTABLE_TYPENAME(size, xDom, yDom)::_metadata.y_axis_meta);\
    }
TABLE_GENERATOR(GEN_Y_BEGIN)

#define GEN_X_BEGIN(size, xDom, yDom) \
    inline table_axis_iterator x_begin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return table_axis_iterator::x_begin(pTable->axisX, DECLARE_3DTABLE_TYPENAME(size, xDom, yDom)::_metadata.x_axis_meta); \
    }
TABLE_GENERATOR(GEN_X_BEGIN)

// =============================== Table function calls =========================

// With no templates or inheritance we need some way to call functions
// for the various distinct table types. CONCRETE_TABLE_ACTION dispatches
// to a caller defined function overloaded by the type of the table. 
#define CONCRETE_TABLE_ACTION_INNER(size, xDomain, yDomain, action, ...) \
  case TO_TYPE_KEY(size, xDomain, yDomain): action(size, xDomain, yDomain, ##__VA_ARGS__);
#define CONCRETE_TABLE_ACTION(testKey, action, ...) \
  switch ((table_type_t)testKey) { \
  TABLE_GENERATOR(CONCRETE_TABLE_ACTION_INNER, action, ##__VA_ARGS__ ) \
  default: abort(); }

// =============================== Table function calls =========================

table_value_iterator rows_begin(const void *pTable, table_type_t key);

table_axis_iterator x_begin(const void *pTable, table_type_t key);

table_axis_iterator y_begin(const void *pTable, table_type_t key);
