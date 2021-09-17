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


// For I/O purposes, table axes are transferred as 8-bit values. So they need scaled
// up & down from 16-bit. The scale factor depends on the axis data domain, as
// defined by this enum.
enum axis_domain { axis_domain_Rpm, axis_domain_Load, axis_domain_Tps };

#define TABLE_RPM_MULTIPLIER 100
#define TABLE_LOAD_MULTIPLIER 2

constexpr inline uint8_t getTableAxisFactor(axis_domain domain)
{
    return domain==axis_domain_Rpm ? TABLE_RPM_MULTIPLIER :
                domain==axis_domain_Load ? TABLE_LOAD_MULTIPLIER :
                    1;
}

// With no inheritance or virtual functions, we need to pass around void*
// In order to cast that back to a concrete type, we need to somehow identify
// the type. This packed int stores just enough information to do that.

typedef uint16_t table_type_t;
constexpr inline table_type_t table_type_key(uint8_t size, axis_domain x, axis_domain y)
{
  return size | (x<<8) | (y<<12);
}

constexpr inline uint8_t key_to_axissize(table_type_t key)
{
    return (uint8_t)key;
}
constexpr inline axis_domain key_to_xdomain(table_type_t key)
{
    return (axis_domain)((key>>8) & 0x0F);
}
constexpr inline axis_domain key_to_ydomain(table_type_t key)
{
    return (axis_domain)(key>>12);
}

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

// Each 3d table is given a distinct type based on size & axis domains
// This encapsulates the generation of the type name
#define DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) table3d ## size ## xDom ## yDom

// Generate the 3D table types
#define GEN_DECLARE_3DTABLE_TYPE(size, xDom, yDom) \
    struct DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) \
    { \
        /* This will take up zero space unless we take the address somewhere */ \
        static constexpr const table_type_t type_key = table_type_key(size, axis_domain_ ## xDom, axis_domain_ ## yDom); \
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
                              size, \
                              pTable->values, \
                              pTable->axisX, \
                              pTable->axisY, \
                              y, x); \
    } 
TABLE_GENERATOR(GEN_GET3D_TABLE_VALUE)

// ================================== Iterator support ========================

#define GEN_ROWS_BEGIN(size, xDom, yDom) \
    inline table_row_iterator_t rows_begin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return rows_begin(pTable->values, size);\
    }
TABLE_GENERATOR(GEN_ROWS_BEGIN)

#define GEN_Y_BEGIN(size, xDom, yDom) \
    inline table_axis_iterator_t y_begin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return y_begin(pTable->axisY, size, getTableAxisFactor(axis_domain_ ## yDom));\
    }
TABLE_GENERATOR(GEN_Y_BEGIN)

#define GEN_Y_RBEGIN(size, xDom, yDom) \
    inline table_axis_iterator_t y_rbegin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return y_rbegin(pTable->axisY, size, getTableAxisFactor(axis_domain_ ## yDom));\
    }
TABLE_GENERATOR(GEN_Y_RBEGIN)

#define GEN_X_BEGIN(size, xDom, yDom) \
    inline table_axis_iterator_t x_begin(const DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) *pTable) \
    { \
        return x_begin(pTable->axisX, size, getTableAxisFactor(axis_domain_ ## xDom)); \
    }
TABLE_GENERATOR(GEN_X_BEGIN)

// =============================== Table function calls =========================

// With no templates or inheritance we need some way to call functions
// for the various distinct table types. CONCRETE_TABLE_ACTION dispatches
// to a caller defined function overloaded by the type of the table. 
#define CONCRETE_TABLE_ACTION_INNER(size, xDomain, yDomain, action, ...) \
  case DECLARE_3DTABLE_TYPENAME(size, xDomain, yDomain)::type_key: action(size, xDomain, yDomain, ##__VA_ARGS__);
#define CONCRETE_TABLE_ACTION(testKey, action, ...) \
  switch (testKey) { \
  TABLE_GENERATOR(CONCRETE_TABLE_ACTION_INNER, action, ##__VA_ARGS__ ) \
  default: abort(); }

// =============================== Table function calls =========================

table_row_iterator_t rows_begin(const void *pTable, table_type_t key);

table_axis_iterator_t x_begin(const void *pTable, table_type_t key);

table_axis_iterator_t y_begin(const void *pTable, table_type_t key);

table_axis_iterator_t y_rbegin(const void *pTable, table_type_t key);