#pragma once

#include <stdint.h>
#include "pid_common.h"

class integerPID_ideal
{
public:
  //commonly used functions **************************************************************************
    integerPID_ideal(long* Input, uint16_t* Output, uint16_t* Sensitivity);

    bool Compute(unsigned long now, uint16_t FeedForward);  // * performs the PID calculation with injected time.
    bool Compute(uint16_t FeedForward);                      // * legacy wrapper that calls millis().
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application
  void setSampleTime(unsigned long now, uint16_t sampleTime);
  void SetTunings(const PidTuningParameters& params, PidDirection direction);
  void setTargetValue(uint16_t setpoint) { _setpoint = setpoint; } //Convenience function to set the target value without having to dereference the pointer

	void Initialize(void);

private:

  PidTuningParameters _pidParams;
	PidDirection _direction = PidDirection::Direct;

    long *myInput;              //
    uint16_t *myOutput;         //   This is a percentage figure multiplied by 100 (To give 2 points of precision)
    uint16_t _setpoint;       //
    uint16_t *mySensitivity;
    uint16_t _sampleTime = 250; 


	unsigned long lastTime;
	long ITerm, lastInput;

	long outMin, outMax;
};
