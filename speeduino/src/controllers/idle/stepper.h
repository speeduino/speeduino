#pragma once

#include <stdint.h>

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

}
}