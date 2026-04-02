#pragma once

#include <stdint.h>

class PID
{
  public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0
  #define DIRECT  0
  #define REVERSE  1

  //commonly used functions **************************************************************************
    PID(long*, long*, long*,        // * constructor.  links the PID to the Input, Output, and
        uint8_t, uint8_t, uint8_t, uint8_t);     //   Setpoint.  Initial tuning parameters are also set here

    void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute(void);                   //   called every time loop() cycles. ON/OFF and
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
  long kp;                  // * (P)roportional Tuning Parameter
  long ki;                  // * (I)ntegral Tuning Parameter
  long kd;                  // * (D)erivative Tuning Parameter

	int controllerDirection;

  long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
  long *myOutput;             //   This creates a hard link between the variables and the
  long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                //   what these values are.  with pointers we'll just know.

  long ITerm, lastInput;

  long outMin, outMax;
  bool inAuto;
};

