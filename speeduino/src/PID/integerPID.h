#pragma once

#include <stdint.h>
#include "PidTuningParameters.h"

class integerPID
{
public:
  
  //commonly used functions **************************************************************************
    integerPID(void);     //   Setpoint.  Initial tuning parameters are also set here


    /** @brief Activates the PID controller. Must be called before Compute() will have any effect. */
    void activate(long input);

    bool Compute(unsigned long now, long input, long* pOutput);   // * performs the PID calculation at provided time.
    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application
    void setTargetValue(long setpoint) { _setpoint = setpoint; } //Convenience function to set the target value without having to dereference the pointer
    void setFeedForwardTerm(long feedForwardTerm);

  //available but not commonly used functions ********************************************************
    void SetTunings(const PidTuningParameters &pidParams, uint32_t nowMs, uint16_t sampleTime);       	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control

  void Initialize(long input);
  void ResetIntegeral(void);

  private:

  PidTuningParameters _pidParams;

  long _setpoint = 0;
  long _feedForwardTerm = 0;

	uint32_t _lastTime = 0;
	long outputSum, lastInput;
  
	uint16_t _sampleTime = 250; //default sample time is 250ms
	long outMin, outMax;
	bool _isActive = false;
};
