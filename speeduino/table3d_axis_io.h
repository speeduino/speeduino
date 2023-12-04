/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis I/O support 
 */

#pragma once

#include "table3d_axes.h"

/** @brief Byte type. This is not defined in any C or C++ standard header. */
typedef uint8_t byte;

/** @brief Models int16->byte conversion 
 * @see table3d_axis_io_converter 
 */
typedef byte(*pToByteConverter)(int16_t value);

/** @brief Models byte->int16 conversion
 * @see table3d_axis_io_converter 
 */
typedef int16_t(*pFromByteConverter)(byte in);

/** @brief Convert a 16-bit value to/from a byte. Useful for I/O. 
 *
 * Often we need to deal internally with values that fit in 16-bits but do
 * not require much accuracy. E.g. table axes in RPM. For these values we can 
 * save storage space (EEPROM) by scaling to/from 8-bits.
 */
struct table3d_axis_io_converter {
    pToByteConverter to_byte;
    pFromByteConverter from_byte;
};

/**
 * @brief Obtain a converter instance for a given axis domain.
 * 
 * Often we need to deal internally with values that fit in 16-bits but do
 * not require much accuracy. E.g. RPM. For these values we can 
 * save storage space (EEPROM) by scaling to/from 8-bits.
 * 
 * The converter is dependent on the domain. I.e all axes with the same domain use 
 * the same converter 
 * 
 * Conversion during I/O is orthogonal to other axis concerns, so is separated and
 * encapsulated here.
 */
table3d_axis_io_converter get_table3d_axis_converter(axis_domain domain);

/** @} */