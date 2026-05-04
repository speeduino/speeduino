#pragma once

#include <stdint.h>

class integerPID_ideal
{
public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0
  #define DIRECT  0
  #define REVERSE  1

  //commonly used functions **************************************************************************
    integerPID_ideal(long*, uint16_t*, uint16_t*, uint16_t*, uint8_t*,        // * constructor.  links the PID to the Input, Output, and
        uint8_t, uint8_t, uint8_t, uint8_t);     //   Setpoint.  Initial tuning parameters are also set here

    bool Compute(unsigned long now, uint16_t);  // * performs the PID calculation with injected time.
    bool Compute(uint16_t);                      // * legacy wrapper that calls millis().
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long, long); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application



  //available but not commonly used functions ********************************************************
    void SetTunings(uint8_t, uint8_t,       // * While most users will set the tunings once in the
                    uint8_t);         	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(uint8_t);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.



	void Initialize();

  private:


	uint8_t dispKp;				// * we'll hold on to the tuning parameters in user-entered
	uint8_t dispKi;				//   format for display purposes
	uint8_t dispKd;				//

	uint16_t  kp;                  // * (P)roportional Tuning Parameter
  uint16_t  ki;                  // * (I)ntegral Tuning Parameter
  uint16_t  kd;                  // * (D)erivative Tuning Parameter

	int controllerDirection;

    long *myInput;              //
    uint16_t *myOutput;         //   This is a percentage figure multiplied by 100 (To give 2 points of precision)
    uint16_t *mySetpoint;       //
    uint16_t *mySensitivity;
    uint8_t *mySampleTime;


	unsigned long lastTime;
  long lastError;
	long ITerm, lastInput;

	long outMin, outMax;
};
