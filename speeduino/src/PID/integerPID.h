#pragma once

#include <stdint.h>
#include "pid_common.h"

class integerPID
{
public:
  
  //commonly used functions **************************************************************************
    integerPID(long* Input, long* Output, long* Setpoint,
        int16_t Kp, int16_t Ki, int16_t Kd);     //   Setpoint.  Initial tuning parameters are also set here


    /** @brief Activates the PID controller. Must be called before Compute() will have any effect. */
    void activate(void);

    bool Compute(unsigned long now, long FeedForwardTerm = 0);   // * performs the PID calculation at provided time.
    bool Compute(long FeedForwardTerm = 0);                      // * legacy wrapper that calls millis().
    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application



  //available but not commonly used functions ********************************************************
    void SetTunings(int16_t Kp, int16_t Ki, int16_t Kd);       	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(PidDirection direction);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.
  void SetSampleTime(uint16_t NewSampleTime);              // * sets the frequency, in Milliseconds, with which
                                          //   the PID calculation is performed.  default is 100



  void Initialize(void);
  void ResetIntegeral(void);

  private:


  int16_t dispKp;
  int16_t dispKi;
  int16_t dispKd;
	int16_t  kp;                  // * (P)roportional Tuning Parameter
  int16_t  ki;                  // * (I)ntegral Tuning Parameter
  int16_t  kd;                  // * (D)erivative Tuning Parameter

	PidDirection _direction = PidDirection::Direct;

    long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    long *myOutput;             //   This creates a hard link between the variables and the
    long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.

	unsigned long lastTime;
	long outputSum, lastInput, lastMinusOneInput;
  
	uint16_t SampleTime;
	long outMin, outMax;
	bool _isActive = false;
};
