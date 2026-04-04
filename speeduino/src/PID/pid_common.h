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
    int16_t Kp; ///> Proportional Tuning Parameter
    int16_t Ki; ///> Integral Tuning Parameter
    int16_t Kd; ///> Derivative Tuning Parameter
};

static inline constexpr PidTuningParameters make_pid_tuning(int16_t Kp, int16_t Ki, int16_t Kd) {
    return PidTuningParameters{Kp, Ki, Kd};
}

// Scalar multiplication operator for easy tuning adjustments
static inline constexpr PidTuningParameters operator*(const PidTuningParameters& params, int8_t scalar) {
    return make_pid_tuning(
        static_cast<int16_t>(params.Kp * scalar),
        static_cast<int16_t>(params.Ki * scalar),
        static_cast<int16_t>(params.Kd * scalar)
    );
}
static inline constexpr PidTuningParameters operator*(int8_t scalar, const PidTuningParameters& params) {
    return params * scalar;
}

// Scalar division operator for easy tuning adjustments
static inline constexpr PidTuningParameters operator/(const PidTuningParameters& params, int8_t scalar) {
    return make_pid_tuning(
        static_cast<int16_t>(params.Kp / scalar),
        static_cast<int16_t>(params.Ki / scalar),
        static_cast<int16_t>(params.Kd / scalar)
    );
}
static inline constexpr PidTuningParameters operator/(int8_t scalar, const PidTuningParameters& params) {
    return params / scalar;
}

constexpr PidTuningParameters PID_TUNING_UNIT = make_pid_tuning(1, 1, 1); ///< Default tuning parameters for a unit PID controller (Kp=1, Ki=1, Kd=1)
