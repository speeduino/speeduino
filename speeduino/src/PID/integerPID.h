#pragma once

#include <stdint.h>

class integerPID
{
public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0
  #define DIRECT  0
  #define REVERSE  1
  
  //commonly used functions **************************************************************************
    integerPID(long*, long*, long*,        // * constructor.  links the PID to the Input, Output, and
        int16_t, int16_t, int16_t, uint8_t);     //   Setpoint.  Initial tuning parameters are also set here


    void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute(unsigned long now, long FeedForwardTerm = 0);   // * performs the PID calculation at provided time.
    bool Compute(long FeedForwardTerm = 0);                      // * legacy wrapper that calls millis().
    void SetOutputLimits(long, long); //clamps the output to a specific range. 0-255 by default, but
										  //it's likely the user will want to change this depending on
										  //the application



  //available but not commonly used functions ********************************************************
    void SetTunings(int16_t, int16_t,       // * While most users will set the tunings once in the
                    int16_t);       	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
	void SetControllerDirection(uint8_t);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.
  void SetSampleTime(uint16_t);              // * sets the frequency, in Milliseconds, with which
                                          //   the PID calculation is performed.  default is 100



  void Initialize();
  void ResetIntegeral();

  private:


  int16_t dispKp;
  int16_t dispKi;
  int16_t dispKd;
	int16_t  kp;                  // * (P)roportional Tuning Parameter
  int16_t  ki;                  // * (I)ntegral Tuning Parameter
  int16_t  kd;                  // * (D)erivative Tuning Parameter

	int controllerDirection;

    long *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    long *myOutput;             //   This creates a hard link between the variables and the
    long *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.

	unsigned long lastTime;
	long outputSum, lastInput, lastMinusOneInput;
  
	uint16_t SampleTime;
	long outMin, outMax;
	bool inAuto;
};
