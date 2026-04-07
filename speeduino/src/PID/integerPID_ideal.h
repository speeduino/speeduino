#pragma once

#include <stdint.h>
#include "PidTuningParameters.h"

class integerPID_ideal
{
public:
  //commonly used functions **************************************************************************
    integerPID_ideal(void);

    bool Compute(unsigned long now, long input, uint16_t* pOutput);  // * performs the PID calculation with injected time.
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application
  void setSampleTime(unsigned long now, uint16_t sampleTime);
  void SetTunings(const PidTuningParameters& params);
  void setTargetValue(uint16_t setpoint) { _setpoint = setpoint; } //Convenience function to set the target value without having to dereference the pointer
  void setSensitivity(uint16_t sensitivity) { _sensitivity = sensitivity; }
	void Initialize(long input);
  void setFeedForwardTerm(uint16_t feedForwardTerm) { _feedForwardTerm = feedForwardTerm; }

private:

  PidTuningParameters _pidParams;

  uint16_t _setpoint;       //
  uint16_t _sensitivity;
  uint16_t _sampleTime = 250; 
  uint16_t _feedForwardTerm = 0;

	unsigned long lastTime;
	long ITerm, lastInput;

	long outMin, outMax;
};
