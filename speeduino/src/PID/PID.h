#pragma once

#include <stdint.h>

class PID
{
  public:

  //Constants used in some of the functions below
  #define DIRECT  0
  #define REVERSE  1

  //commonly used functions **************************************************************************
    PID(long* Input, long* Output, long* Setpoint,
        uint8_t Kp, uint8_t Ki, uint8_t Kd);     //   Setpoint.  Initial tuning parameters are also set here

    /** @brief Activates the PID controller. Must be called before Compute() will have any effect. */
    void activate(void);

    bool Compute(void);                   //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(long Min, long Max); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application



  //available but not commonly used functions ********************************************************
    void SetTunings(uint8_t Kp, uint8_t Ki, uint8_t Kd);         	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(uint8_t Direction);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.
	void Initialize();
private:
  long kp;                  // * (P)roportional Tuning Parameter
  long ki;                  // * (I)ntegral Tuning Parameter
  long kd;                  // * (D)erivative Tuning Parameter

	int controllerDirection = DIRECT;

  long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
  long *myOutput;             //   This creates a hard link between the variables and the
  long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                //   what these values are.  with pointers we'll just know.

  long ITerm, lastInput;

  long outMin, outMax;
  bool _isActive = false;
};

