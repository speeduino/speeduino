#pragma once

#include <stdint.h>
#include "PidTuningParameters.h"

class PID
{
public:

  //commonly used functions **************************************************************************
    PID(void);     //   Setpoint.  Initial tuning parameters are also set here

    long Compute(long input);                   //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application

    void setTargetValue(int16_t setpoint) { _setpoint = setpoint; } //Convenience function to set the target value without having to dereference the pointer

  //available but not commonly used functions ********************************************************
    void SetTunings(const PidTuningParameters &pidParams);         	  //   constructor, this function gives the user the option
                                        //   of changing tunings during runtime for Adaptive control

  void resetIntegral(long input);

private:
  PidTuningParameters _pidParams;

  int16_t _setpoint = 0;           //   PID, freeing the user from having to constantly tell us
                                //   what these values are.  with pointers we'll just know.

  long ITerm, lastInput;

  long outMin, outMax;
};

