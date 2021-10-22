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
typedef uint8_t table3d_value_t; // 

/** @brief The type of each axis value */
typedef int16_t table3d_axis_t;

/** @} */