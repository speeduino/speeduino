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


/// @cond
// Functions private to this file
namespace scale_translate_detail {
static constexpr inline uint16_t _toRawApplyTranslateU16(const conversionFactor &factor, uint16_t working) {
    return working - (uint16_t)factor.translate;
}

static constexpr inline int16_t _toRawApplyTranslateS16(const conversionFactor &factor, int16_t working) {
    return working - (int16_t)factor.translate;
}

static constexpr inline int16_t _toWorkingApplyTranslateS16(const conversionFactor &factor, uint8_t raw) {
    return (int16_t)raw + (int16_t)factor.translate;
}

static constexpr inline int16_t _toWorkingApplyTranslateS16(const conversionFactor &factor, int8_t raw) {
    return (int16_t)raw + (int16_t)factor.translate;
}

static constexpr inline int32_t _toWorkingApplyTranslateS32(const conversionFactor &factor, uint16_t raw) {
    return (int32_t)raw + (int32_t)factor.translate;
}

static constexpr inline uint16_t _optimisedDivideU16(const conversionFactor &factor, uint16_t working) {
            // No division required
    return  (factor.scale==1U) ? working :
                // uint16_t/2U
                (factor.scale==2U) ? working >> 1U :
                    // uint16_t/100
                    (factor.scale==100U) ? div100(working) :
                        // uint16_t/10
                        (factor.scale==10U) ? (working/10U) :
                            fast_div(working, factor.scale);
}

static constexpr inline int16_t _optimisedDivideS16(const conversionFactor &factor, int16_t working) {
            // No division required
    return  (factor.scale==1U) ? working :
                // Unsigned path
                (working>=0) ? _optimisedDivideU16(factor, (uint16_t)working) :
                    // int16_t/100
                    (factor.scale==100U) ? div100(working) :
                        // Faster than int16_t/10.
                        (factor.scale==10U) ? div100(working*10) :
                            // Slowest path
                            working = working / (int16_t)factor.scale;
}
}
/// @endcond

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
static constexpr inline uint16_t toWorkingU8U16(const conversionFactor &factor, uint8_t raw) {
    return (uint16_t)scale_translate_detail::_toWorkingApplyTranslateS16(factor, raw) * factor.scale;
}

/** @copydoc toWorkingU8U16 */
static constexpr inline int16_t toWorkingU8S16(const conversionFactor &factor, uint8_t raw) {
    return scale_translate_detail::_toWorkingApplyTranslateS16(factor, raw) * factor.scale;
}

/** @copydoc toWorkingU8U16 */
static constexpr inline int16_t toWorkingS8S16(const conversionFactor &factor, int8_t raw) {
    return scale_translate_detail::_toWorkingApplyTranslateS16(factor, raw) * (int16_t)factor.scale;
}


/** @copydoc toWorkingU8U16 */
static constexpr inline uint32_t toWorkingU32(const conversionFactor &factor, uint16_t raw) {
    return (uint32_t)scale_translate_detail::_toWorkingApplyTranslateS32(factor, raw) * (uint32_t)factor.scale;
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
static constexpr inline int8_t toRawS8(const conversionFactor &factor, int16_t working) {
    return scale_translate_detail::_toRawApplyTranslateS16(factor, scale_translate_detail::_optimisedDivideS16(factor, working));
}

/** @copydoc toRawS8 */
static constexpr inline uint8_t toRawU8(const conversionFactor &factor, uint16_t working) {
    return scale_translate_detail::_toRawApplyTranslateU16(factor, scale_translate_detail::_optimisedDivideU16(factor, working));
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

/** @brief Time values stored in 1/10th milliseconds I.e ms/10 
 * 
 * We convert to/from µS
*/
static constexpr conversionFactor TIME_TENTH_MILLIS = { .scale=10000U, .translate=0U };

/** @brief MAP values: kpa */
static constexpr conversionFactor MAP = { .scale=2U, .translate=0U };

/** @brief MAP rate of change: kpa/s  */
static constexpr conversionFactor MAP_DOT = { .scale=10U, .translate=0U };

/** @brief TPS rate of change: %/s */
static constexpr conversionFactor TPS_DOT = { .scale=10U, .translate=0U };

/** @brief Cranking enrichment values range from 0% to 1275% */
static constexpr conversionFactor CRANKING_ENRICHMENT = { .scale=5U, .translate=0U };

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
    return toWorkingU8S16(TEMPERATURE, temp);
}
///@}

