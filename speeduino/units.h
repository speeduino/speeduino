#pragma once

#include <stdint.h>
#include "maths.h"

/**
 * @file
 * @brief Integer conversions using scale & translate 
 * 
 * Rationale: It encapsulates all conversion factors and operations in one place
 *  * Removes magic numbers from many calculations.
 *  * Puts the necessary MISRA compliance code (E.g. casts) in one place
 *  * Allows independent unit testing of the conversion code
 * 
 * Background: For memory footprint reasons, many tune parameters are stored using a smaller type
 * than the full numeric range requires. E.g. 
 *   * RPM requires a uint16_t
 *   * We can store some RPM boundaries as a uint8_t containing RPM/100.
 * The trade off is a loss of precision. 
 *
 * See section 4.4.7, Scale and Translate, of the TunerStudio ECU definition manual.
 * 
 * Terminology:
 *   * A "raw" value is what is stored in the tune. E.g. Time in tenths of a millisecond
 *   * A "working" value is our internal type. E.g. Time in microseconds
 */

/** @brief Stores the scale and translate values to convert between user & raw page values. 
 * 
 * Typically these will be the same scale and translate values as specified in the TunerStudio
 * configuration file (ini)
*/
struct conversionFactor {
    uint16_t scale; // Cannot be zero!
    int8_t translate;
};

/**
 * @brief Convert a value from raw page value to working value.
 * 
 * It's the callers responsibility to make sure the converted value fits
 * into the destination type
 * 
 * @param factor The conversion factor
 * @param raw The raw value
 * @return uint16_t The working value
 */
static constexpr inline uint16_t toWorkingU16(const conversionFactor &factor, uint8_t raw) {
    return ((uint16_t)raw + (uint16_t)factor.translate) * factor.scale;
}

/** @copydoc toWorkingU16 */
static constexpr inline int16_t toWorkingS16(const conversionFactor &factor, uint8_t raw) {
    return ((int16_t)raw + (int16_t)factor.translate) * (int16_t)factor.scale;
}

/** @copydoc toWorkingU16 */
static constexpr inline uint32_t toWorkingU32(const conversionFactor &factor, uint8_t raw) {
    return ((uint32_t)raw + (uint32_t)factor.translate) * (uint32_t)factor.scale;
}

/**
 * @brief Convert a value from a working value to raw page value.
 * 
 * It's the callers responsibility to make sure the converted value fits
 * into the destination type
 * 
 * @param factor The conversion factor
 * @param working The working value
 * @return uint16_t The raw page value
 */
static constexpr inline uint8_t toRawU8(const conversionFactor &factor, uint16_t working) {
    return (uint8_t)((working / factor.scale) - (uint16_t)factor.translate);
}

/**
 * @defgroup conversions Conversion factors
 * 
 * @brief Since these are all constexpr, they should be zero overhead in time & space
 * (unless we take the address of one of them) * 
 * @{
 */

/** @brief RPM stored as RPM/100. E.g. 2700 -> 27 */
static constexpr conversionFactor RPM_COARSE = { .scale=100U, .translate=0U };

/** @brief RPM stored as RPM/10. E.g. 2700 -> 270
 * 
 * This limits the maximum value to 2550 RPM, but gives more precision than RPM_COARSE
 */
static constexpr conversionFactor RPM_MEDIUM = { .scale=10U, .translate=0U };

/** @brief RPM stored as RPM/5. E.g. 1300->260
 * 
 * This limits the maximum value to 1275 RPM, but gives more precision than RPM_MEDIUM
 */
static constexpr conversionFactor RPM_FINE = { .scale=5U, .translate=0U };

/** @brief Time values stored in 1/10th milliseconds I.e ms/10 
 * 
 * We convert to/from µS
*/
static constexpr conversionFactor TIME_TENTH_MILLIS = { .scale=10000U, .translate=0U };

/** @brief Time values stored in 20 milliseconds 
 * 
 * We convert to/from secs/0.1
*/
static constexpr conversionFactor TIME_TWENTY_MILLIS = { .scale=5U, .translate=0U };

/** @brief Time values stored in 10 milliseconds 
 * 
 * We convert to/from µS
*/
static constexpr conversionFactor TIME_TEN_MILLIS = { .scale=100U, .translate=0U };

/** @brief MAP values: kpa */
static constexpr conversionFactor MAP = { .scale=2U, .translate=0U };

/** @brief MAP rate of change: kpa/s  */
static constexpr conversionFactor MAP_DOT = { .scale=10U, .translate=0U };

/** @brief TPS rate of change: %/s */
static constexpr conversionFactor TPS_DOT = { .scale=10U, .translate=0U };

/** @brief Cranking enrichment values range from 0% to 1275% */
static constexpr conversionFactor CRANKING_ENRICHMENT = { .scale=5U, .translate=0U };

/** 
 * @brief Ignition values from the main spark table are offset 40 degrees downwards to allow for negative spark timing
 * This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 40
 */
static constexpr conversionFactor IGNITION_ADVANCE_LARGE = { .scale=1U, .translate=-40 };

/** @brief Ignition advance adjustments can use a smaller offset */
static constexpr conversionFactor IGNITION_ADVANCE_SMALL = { .scale=1U, .translate=-15 };


/** 
 * @brief All temperature measurements are stored offset by 40 degrees.
 * This is so we can use an unsigned byte (0-255) to represent temperature ranges from -40 to 215
 * 
 * @see temperatureAddOffset
 * @see temperatureRemoveOffset
 */
static constexpr conversionFactor TEMPERATURE = { .scale=1U, .translate=-40 };

/**
 * @brief Convert from a working temperature (-40, 215) to storage (0, 255)
 * 
 * This is done often enough to warrant wrapping toRawU8
 * 
 * @param temp Working temperature (-40, 215)
 */
static inline constexpr uint8_t temperatureAddOffset(int16_t temp) {
    return toRawU8(TEMPERATURE, temp);
}

/**
 * @brief Convert from a storage temperature (0, 255) to working (-40, 215)
 * 
 * This is done often enough to warrant wrapping toRawU8
 * 
 * @param temp Storage temperature (0, 255)
 */
static inline constexpr int16_t temperatureRemoveOffset(uint8_t temp) {
    return toWorkingS16(TEMPERATURE, temp);
}
///@}

