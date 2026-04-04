#pragma once

#include <stdint.h>
#include "pid_common.h"

class integerPID
{
public:
  
  //commonly used functions **************************************************************************
    integerPID(long* Input, long* Output, long* Setpoint);     //   Setpoint.  Initial tuning parameters are also set here


    /** @brief Activates the PID controller. Must be called before Compute() will have any effect. */
    void activate(void);

    bool Compute(unsigned long now, long FeedForwardTerm = 0);   // * performs the PID calculation at provided time.
    bool Compute(long FeedForwardTerm = 0);                      // * legacy wrapper that calls millis().
    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application



  //available but not commonly used functions ********************************************************
    void SetTunings(const PidTuningParameters &pidParams, PidDirection direction, uint16_t sampleTime);       	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control

  void Initialize(void);
  void ResetIntegeral(void);

  private:

  PidTuningParameters _pidParams = PID_TUNING_UNIT;

    long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    long *myOutput;             //   This creates a hard link between the variables and the
    long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.

	unsigned long lastTime;
	long outputSum, lastInput;
  
	uint16_t _sampleTime = 250; //default sample time is 250ms
	long outMin, outMax;
	bool _isActive = false;
};
