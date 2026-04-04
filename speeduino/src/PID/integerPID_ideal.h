#pragma once

#include <stdint.h>
#include "pid_common.h"

class integerPID_ideal
{
public:
  //commonly used functions **************************************************************************
    integerPID_ideal(long* Input, uint16_t* Output, uint16_t* Setpoint, uint16_t* Sensitivity, uint8_t* SampleTime);

    bool Compute(unsigned long now, uint16_t FeedForward);  // * performs the PID calculation with injected time.
    bool Compute(uint16_t FeedForward);                      // * legacy wrapper that calls millis().
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application



  //available but not commonly used functions ********************************************************
    void SetTunings(const PidTuningParameters& params);         	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(PidDirection direction);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.



	void Initialize(void);

private:

  PidTuningParameters _pidParams = PID_TUNING_UNIT;
	PidDirection _direction = PidDirection::Direct;

    long *myInput;              //
    uint16_t *myOutput;         //   This is a percentage figure multiplied by 100 (To give 2 points of precision)
    uint16_t *mySetpoint;       //
    uint16_t *mySensitivity;
    uint8_t *mySampleTime;


	unsigned long lastTime;
	long ITerm, lastInput;

	long outMin, outMax;
};
