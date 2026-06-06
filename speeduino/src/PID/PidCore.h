#pragma once
#include "PidTuningParameters.h"

/** @brief A base class for %PID implementations
 * 
 * Provides a shared %PID algorithm. @see compute.
 */
class PidCore
{
public:
    PidCore(void) = default;

    /** @brief Set the output limits */
    void setOutputLimits(int32_t min, int32_t max) {
        if(min >= max) return;
        _min = min;
        _max = max;
    }

    /** @brief Set the PID parameters */
    void setTunings(const PidTuningParameters& params) { 
        _pidParams = params; 
    }

    /** @brief (Optional) Reset the integral term */
    void resetIntegral(void) { 
        _integralTerm = 0; 
    }

    /** @brief Compute the next correction. */
    int32_t compute(int32_t feedForwardTerm, int32_t error, int32_t derivative);

private:
    PidTuningParameters _pidParams;

    int32_t _integralTerm = 0;
    // TODO: shrink these to int16_t if possible
    int32_t _min = 0;
    int32_t _max = 0;
};