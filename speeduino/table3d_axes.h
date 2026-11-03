/** 
 * @addtogroup table_3d 
 *  @{
 */

/** \file
 * @brief 3D table axis types and iterators
 */

#pragma once

#include <stdint.h>

/**\enum AxisDomain
 * @brief Encodes the real world measurement that a table axis captures
 * */
enum class AxisDomain : uint8_t {
    /** RPM (engine speed) */
    Rpm,
    /** Load */
    Load,
};

static constexpr uint16_t getConversionFactor(AxisDomain domain)
{
    return domain==AxisDomain::Rpm ? 100U : 2U;
}

/** @} */