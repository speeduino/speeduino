#pragma once

#include "src/PID/integerPID.h"
#include "src/pins/fastOutputPin.h"

namespace idleController {

namespace detail {

enum class StepperStatus : uint8_t 
{
    SOFF, 
    STEPPING, ///< a high pulse is currently being sent and will need to be turned off at some point.
    COOLING
}; 

struct StepperIdle
{
  int curIdleStep; //Tracks the current location of the stepper
  int targetIdleStep; //What the targeted step is
  volatile StepperStatus stepperStatus;
  volatile unsigned long stepStartTime;
};

struct state_t
{
    uint8_t idleUpOutputHIGH = HIGH; // Used to invert the idle Up Output 
    uint8_t idleUpOutputLOW = LOW;   // Used to invert the idle Up Output 
    uint8_t idleCounter; //Used for tracking the number of calls to the idle control function
    uint8_t idleTaper;

    struct StepperIdle idleStepper;
    bool idleOn; //Simply tracks whether idle was on last time around
    uint8_t idleInitComplete = 99; //Tracks which idle method was initialised. 99 is a method that will never exist
    unsigned int iacStepTime_uS;
    unsigned int iacCoolTime_uS;
    unsigned int completedHomeSteps;

    volatile bool idle_pwm_state;
    bool lastDFCOValue;
    uint16_t idle_pwm_max_count; //Used for variable PWM frequency
    volatile unsigned int idle_pwm_cur_value;
    int32_t idle_pid_target_value;
    int32_t FeedForwardTerm;
    uint32_t idle_pwm_target_value;
    int32_t idle_cl_target_rpm;

    fastOutputPin_t idle_pin;
    fastOutputPin_t idle2_pin;

    integerPID idlePID; //This is the PID object if that algorithm is used. Needs to be global as it maintains state outside of each function call
};

} // detail

} // fuelPumpController