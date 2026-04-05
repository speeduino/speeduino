#pragma once

#include <stdint.h>

enum class PidDirection : uint8_t
{
    Direct = 0,  ///< The output will increase when error is positive. 
    Reverse = 1, ///< The output will decrease when error is positive. 
};

/** @brief Structure to hold PID tuning parameters */
struct PidTuningParameters
{
    /**
     * @brief Construct a new PidTuningParameters object
     * 
     * @note The parameters are uint8_t for memory efficiency, but are stored as int16_t to allow 
     * for a wide range of tuning values. They can be scaled as needed using the provided operators 
     * or by adjusting the tunings directly.
     * 
     * @param kp Proportional tuning parameter (default 1)
     * @param ki Integral tuning parameter (default 1)
     * @param kd Derivative tuning parameter (default 1)
     */
    constexpr PidTuningParameters(uint8_t kp = 1U, uint8_t ki = 1U, uint8_t kd = 1U)
        : Kp(kp), Ki(ki), Kd(kd) {}

    int16_t Kp; ///> Proportional Tuning Parameter
    int16_t Ki; ///> Integral Tuning Parameter
    int16_t Kd; ///> Derivative Tuning Parameter
};

/** @brief Scalar multiplication operator for easy tuning adjustments */
///@{
static inline constexpr PidTuningParameters operator*(const PidTuningParameters& params, int8_t scalar) {
    PidTuningParameters scaledParams;
    scaledParams.Kp = params.Kp * scalar;
    scaledParams.Ki = params.Ki * scalar;
    scaledParams.Kd = params.Kd * scalar;
    return scaledParams;
}
static inline constexpr PidTuningParameters operator*(int8_t scalar, const PidTuningParameters& params) {
    return params * scalar;
}
///@}

/** @brief Scalar division operator for easy tuning adjustments */
///@{
static inline constexpr PidTuningParameters operator/(const PidTuningParameters& params, int8_t scalar) {
    PidTuningParameters scaledParams;
    scaledParams.Kp = params.Kp / scalar;
    scaledParams.Ki = params.Ki / scalar;
    scaledParams.Kd = params.Kd / scalar;
    return scaledParams;
}
static inline constexpr PidTuningParameters operator/(int8_t scalar, const PidTuningParameters& params) {
    return params / scalar;
}
///@}