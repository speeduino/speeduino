#include "stepper.h"

namespace idleController {

namespace detail {

    __attribute__((optimize("Os"))) stepperState_t::stepperState_t(const pinNumbers_t &pins, const config9 &page9)
    {
        pinStepperDir.setPin(pins.pinStepperDir, OUTPUT);
        pinStepperDir.setInverted(page9.iacStepperInv);
        pinStepperStep.setPin(pins.pinStepperStep, OUTPUT);
        pinStepperEnable.setPin(pins.pinStepperEnable, OUTPUT);
    }

}
}