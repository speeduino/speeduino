#pragma once

#include <stdint.h>

namespace idleController {

namespace detail {

    /** @brief Stepper motor status */
    enum class StepperStatus : uint8_t 
    {
        OFF,      ///< The motor is not stepping or cooling
        STEPPING, ///< The motor is moving
        COOLING   ///< The motor is in it's cooling phase
    }; 

    /** @brief Captures the state of the stepper motor */
    struct stepperState_t
    {
        uint16_t curStep = 0;                       ///< The current location of the motor
        uint16_t targetStep = 0;                    ///< The target step we want to move *to*
        StepperStatus status = StepperStatus::OFF;  ///< What phase the motor is in
        uint32_t startTime = 0;                     ///< The time in µS that we started the current phase
    };

}
}