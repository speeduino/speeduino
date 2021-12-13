/** 
 * @addtogroup table_3d 
 *  @{
 */


/** \file
 * @brief Typedefs for primitive 3D table elements
 * 
 * These used are for consistency across functions that work on 3D table data. 
 * For example:<br>
 * <c>table3d_value_t foo(table3d_axis_t input);</c><br>
 * instead of:<br>
 * <c>uint8_t foo(int16_t input);</c>
 */

#pragma once

#include <stdint.h>

/** @brief Encodes the \b length of the axes */
typedef uint8_t table3d_dim_t;

/** @brief The type of each table value */
typedef uint8_t table3d_value_t;

/** @brief The type of each axis value */
typedef int16_t table3d_axis_t;

/** @brief Core 3d table generation macro
 * 
 * We have a fixed number of table types: they are defined by this macro.
 * GENERATOR is expected to be another macros that takes at least 3 arguments:
 *    axis length, x-axis domain, y-axis domain
 */
#define TABLE3D_GENERATOR(GENERATOR, ...) \
    GENERATOR(6, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(4, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(8, Rpm, Load, ##__VA_ARGS__) \
    GENERATOR(8, Rpm, Tps, ##__VA_ARGS__) \
    GENERATOR(16, Rpm, Load, ##__VA_ARGS__)

// Each 3d table is given a distinct type based on size & axis domains
// This encapsulates the generation of the type name
#define TABLE3D_TYPENAME_BASE(size, xDom, yDom) table3d ## size ## xDom ## yDom

#define CAT_HELPER(a, b) a ## b
#define CONCAT(A, B) CAT_HELPER(A, B)

/** @} */