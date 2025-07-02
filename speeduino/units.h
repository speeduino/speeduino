#pragma once

#include <stdint.h>

/**
 * @file
 * @brief Integer conversions using scale & translate 
 */

/** @brief Scale and translate signed/unsigned values to convert between "raw" & "user" values. 
 * 
 * Terminology:
 *   * A "raw" value is what is stored in the tune. E.g. Time in tenths of a millisecond
 *   * A "user" value is our internal type. E.g. Time in microseconds
 * 
 * The scaling and translation values are used as follows
 *   * rawValue  = userValue / scale – translate 
 *   * userValue = (rawValue + translate) * scale 
 * 
 * Typically these will be the same scale and translate values as specified in the TunerStudio
 * configuration file (ini)
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
 * @tparam TUser The type of a user value. E.g. int8_t
 * @tparam TRaw The type of a raw value. E.g. uint8_t
 */
template <typename TUser,
          typename TRaw>
struct conversionFactor {
    uint16_t scale;    ///< Scale factor, must be >0
    int16_t translate; ///< Translation - can be negative

    /**
     * @brief Convert a value from raw page value to user value.
     * 
     * It's the callers responsibility to make sure the converted value fits
     * into the destination type after translation & scaling
     */
    constexpr TUser toUser(TRaw raw) const {
        return ((TUser)raw + (TUser)translate) * (TUser)scale;
    }

    /**
     * @brief Convert a value from a user value to raw page value.
     * 
     * It's the callers responsibility to make sure the converted value fits
     * into the destination type
     */
    constexpr TRaw toRaw(TUser user) const {
        // Since all conversionFactor instances are constexpr, compiler will heavily
        // optimize this. E.g. removing division or replacing with shift & multiply.
        return scaleUser(user) - (TRaw)translate;
    }
private:
    constexpr TRaw scaleUser(TUser user) const {
        // Since all conversionFactor instances are constexpr, compiler will heavily
        // optimize this. E.g. removing division or replacing with shift & multiply.
        return user / (TUser)scale;
    }
};

/**
 * @name  Conversion factors
 * 
 * @note Since these are all constexpr, they should be zero overhead in time & space
 * (unless we take the address of one of them)
 * 
 * @{
 */

/** @brief RPM stored as RPM/100. E.g. 2700 -> 27 -> 2700 */
static constexpr conversionFactor<uint16_t, uint8_t> RPM_COARSE = { .scale=100U, .translate=0U };

/** @brief RPM stored as RPM/10. E.g. 2700 -> 270 -> 2700
 * 
 * This limits the maximum value to 2550 RPM, but gives more precision than RPM_COARSE
 */
static constexpr conversionFactor<uint16_t, uint8_t> RPM_MEDIUM = { .scale=10U, .translate=0U };

/** @brief RPM stored as RPM/5. E.g. 1300->260->1300
 * 
 * This limits the maximum value to 1275 RPM, but gives more precision than RPM_MEDIUM
 */
static constexpr conversionFactor<uint16_t, uint8_t> RPM_FINE = { .scale=5U, .translate=0U };

/** @brief Time values stored in 1/10th milliseconds I.e ms/10 
 * 
 * We convert to/from µS
 */
static constexpr conversionFactor<uint32_t, uint8_t> TIME_TENTH_MILLIS = { .scale=10000U, .translate=0U };

/** @brief Time values stored in 20 milliseconds 
 * 
 * We convert to/from secs/0.1
 */
static constexpr conversionFactor<uint16_t, uint8_t> TIME_TWENTY_MILLIS = { .scale=5U, .translate=0U };

/** @brief Time values stored in 10 milliseconds 
 * 
 * We convert to/from µS
 */
static constexpr conversionFactor<uint16_t, uint8_t> TIME_TEN_MILLIS = { .scale=100U, .translate=0U };

/** @brief MAP values: kpa */
static constexpr conversionFactor<uint16_t, uint8_t> MAP = { .scale=2U, .translate=0U };

/** @brief MAP rate of change: kpa/s  */
static constexpr conversionFactor<int16_t, uint8_t> MAP_DOT = { .scale=10U, .translate=0U };

/** @brief TPS rate of change: %/s */
static constexpr conversionFactor<int16_t, uint8_t> TPS_DOT = { .scale=10U, .translate=0U };

/** @brief Cranking enrichment values range from 0% to 1275% */
static constexpr conversionFactor<uint16_t, uint8_t> CRANKING_ENRICHMENT = { .scale=5U, .translate=0U };

/** @brief Ignition values from the main spark table are offset 40 degrees downwards to allow for negative spark timing */
static constexpr conversionFactor<int8_t, uint8_t> IGNITION_ADVANCE_LARGE = { .scale=1U, .translate=-40 };

/** @brief Ignition advance adjustments can use a smaller offset */
static constexpr conversionFactor<int8_t, uint8_t> IGNITION_ADVANCE_SMALL = { .scale=1U, .translate=-15 };

/** @brief All temperature measurements are stored offset by 40 degrees, to represent temperature ranges from -40 to 215 */
static constexpr conversionFactor<int16_t, uint8_t> TEMPERATURE = { .scale=1U, .translate=-40 };

///@}

/**
 * @brief Convert from a user temperature (-40, 215) to storage (0, 255)
 * 
 * This is done often enough to warrant wrapping toRaw
 * 
 * @param temp Working temperature (-40, 215)
 */
static inline constexpr uint8_t temperatureAddOffset(int16_t temp) {
    // TODO: remove this function, replace with TEMPERATURE.toRaw()
    return TEMPERATURE.toRaw(temp);
}

/**
 * @brief Convert from a storage temperature (0, 255) to user (-40, 215)
 * 
 * This is done often enough to warrant wrapping toUser
 * 
 * @param temp Storage temperature (0, 255)
 */
static inline constexpr int16_t temperatureRemoveOffset(uint8_t temp) {
    // TODO: remove this function, replace with TEMPERATURE.toUser()
    return TEMPERATURE.toUser(temp);
}

