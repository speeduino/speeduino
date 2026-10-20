#pragma once

#include "src/pins/outputPin.h"
#include "src/pins/invertableOutputPin.h"
#include "src/pins/pinNumbers_t.h"
#include "config_pages.h"

namespace idleController {

namespace detail {

    /** @brief Stepper motor status */
    enum class StepperStatus : uint8_t 
    {
        OFF,      ///< The motor is not stepping or cooling
        STEPPING, ///< The motor is moving
        COOLING   ///< The motor is in it's cooling phase
    };

    struct stepperPosition_t 
    {
        uint16_t curStep = 0;                       ///< The current location of the motor
        uint16_t completedHomeSteps = 0;            ///< Number of homing steps completed
        StepperStatus status = StepperStatus::OFF;  ///< What phase the motor is in
    };

    /** @brief Captures the state of the stepper motor */
    struct stepperState_t : public stepperPosition_t
    {
        uint16_t targetStep = 0;                    ///< The target step we want to move *to*
        uint32_t startTime = 0;                     ///< The time in µS that we started the current phase

        outputPin_t pinStepperEnable;
        outputPin_t pinStepperStep;
        invertableOutputPinAdaper_t<outputPin_t> pinStepperDir;

        stepperState_t() = default;
        stepperState_t(const pinNumbers_t &pins, const config9 &page9);
    };
}
}