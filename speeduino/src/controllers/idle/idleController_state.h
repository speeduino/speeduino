#pragma once

#include "src/PID/integerPID.h"
#include "stepper.h"
#include "src/pins/fastOutputPin.h"
#include "src/pins/outputPin.h"
#include "src/pins/invertableOutputPin.h"
#include "src/pins/inputPin.h"
#include "src/pins/pinNumbers_t.h"
#include "config_pages.h"

namespace idleController {

namespace detail {

using fnCurMicros_t = decltype(micros)*;

static inline uint32_t pwmTicksToPidCount(uint32_t pwmTicks)
{
    return pwmTicks << 2U;
}

static inline uint32_t pidCountToPwmTicks(uint32_t pidCount)
{
    return pidCount >> 2U;
}

struct state_t
{
    uint8_t idleCounter = 0; //Used for tracking the number of calls to the idle control function
    uint8_t idleTaper = 0;

    stepperState_t idleStepper;
    bool idleOn = false; //Simply tracks whether idle was on last time around
    uint8_t idleInitComplete = 99; //Tracks which idle method was initialised. 99 is a method that will never exist
    unsigned int iacStepTime_uS = 0;
    unsigned int iacCoolTime_uS = 0;

    volatile bool idle_pwm_state = false;
    bool lastDFCOValue = false;
    uint16_t idle_pwm_max_count = 0; //Used for variable PWM frequency
    volatile unsigned int idle_pwm_cur_value = 0;
    int32_t idle_pid_target_value = 0;
    int32_t FeedForwardTerm = 0;
    uint32_t idle_pwm_target_value = 0;
    int32_t idle_cl_target_rpm = 0;

    invertableOutputPinAdaper_t<fastOutputPin_t> idle_pin;
    invertableOutputPinAdaper_t<fastOutputPin_t> idle2_pin;
    invertableOutputPinAdaper_t<outputPin_t> pinIdleUpOutput;
    inputPin_t pinIdleUpInput;

    integerPID idlePID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call

    state_t() = default;
    state_t(const pinNumbers_t &pins, const config2 &page2, const config6 &page6, const config9 &page9);

    void setPinHigh(void)
    {
        idle_pin.setPinHigh();
        idle2_pin.setPinLow(); //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }

    void setPinLow(void)
    {
        idle_pin.setPinLow();
        idle2_pin.setPinHigh(); //If 2 idle channels are in use, flip idle2 to be the opposite of idle1
    }

    /** Convert a duty percentage (0-100) to PWM timer ticks */
    uint16_t loadToPwmTicks(uint8_t load) const
    {
        return percentage(load, idle_pwm_max_count);
    }

    /** Convert a PWM frequency (in Hz) to duty percentage (0-100)*/
    inline uint8_t pwmTicksToLoad(uint16_t pwmFreq)
    {
        return (uint8_t)std::min((uint16_t)ONE_HUNDRED_PCT, fast_div32_16((uint32_t)(pwmFreq * UINT32_C(100)), idle_pwm_max_count));
    }    

    uint32_t rpmToPidCount(uint16_t rpm) const
    {
        return percentage(rpm, getPidMaxCount());
    }

    uint32_t getPidMaxCount(void) const
    {
        return pwmTicksToPidCount(idle_pwm_max_count);
    }    
};

} // detail

} // fuelPumpController