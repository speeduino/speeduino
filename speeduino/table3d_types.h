#pragma once

#include "table3d.h"
#include "table_iterator.h"

// Each 3d table is given a distinct type based on size & axis domains
// This encapsulates the generation of the type name
#define DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) table3d ## size ## xDom ## yDom

// With no inheritance or virtual functions, we need to pass around void*
// In order to cast that back to a concrete type, we need to somehow identify
// the type. This packed int stores just enough information to do that.
//
// (if necessary, we could decompose the key back to it's individual elements)
constexpr inline uint16_t table_type_key(uint8_t size, axis_domain x, axis_domain y)
{
  return size | (x<<8) | (y<<12);
}

constexpr inline uint8_t table_key_to_axissize(uint16_t key)
{
    return (uint8_t)key;
}

constexpr inline axis_domain key_to_xdomain(uint16_t key)
{
    return (axis_domain)((key>>8) & 0x0F);
}
constexpr inline axis_domain key_to_ydomain(uint16_t key)
{
    return (axis_domain)(key>>12);
}

// We have a fixed number of table types: they are defined by this macro
#define TABLE_GENERATOR(GENERATOR, ...) \
    GENERATOR(6, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(4, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(8, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(8, Rpm, Tps, ##__VA_ARGS__) \
    GENERATOR(16, Rpm, Load, ##__VA_ARGS__)

// ================================== Core 3D table ========================

// Generate the 3D table types
#define GEN_DECLARE_3DTABLE_TYPE(size, xDom, yDom) \
    struct DECLARE_3DTABLE_TYPENAME(size, xDom, yDom) \
    { \
        /* This will take up zero space unless we take the address somewhere */ \
        static constexpr const uint16_t type_key = table_type_key(size, axis_domain_ ## xDom, axis_domain_ ## yDom); \
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