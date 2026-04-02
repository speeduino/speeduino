/**********************************************************************************************
 * Arduino PID Library - Version 1.0.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/
#include "integerPID_ideal.h"
#include <Arduino.h>


/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
integerPID_ideal::integerPID_ideal(long* Input, uint16_t* Output, uint16_t* Setpoint, uint16_t* Sensitivity, uint8_t* SampleTime,
                                   uint8_t Kp, uint8_t Ki, uint8_t Kd, uint8_t ControllerDirection)
{

    myOutput = Output;
    myInput = (long*)Input;
    mySetpoint = Setpoint;
    mySensitivity = Sensitivity;
    mySampleTime = SampleTime;

	  integerPID_ideal::SetOutputLimits(20, 80);				//default output limits

    integerPID_ideal::SetControllerDirection(ControllerDirection);
    integerPID_ideal::SetTunings(Kp, Ki, Kd);

    lastTime = millis()- *mySampleTime;
    lastError = 0;
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool integerPID_ideal::Compute(unsigned long now, uint16_t FeedForward)
{
   constexpr uint16_t limitMultiplier = 100; //How much outMin and OutMax must be multiplied by to get them in the same scale as the output

   unsigned long timeChange = (now - lastTime);
   if(timeChange >= *mySampleTime)
   {
      /*Compute all the working error variables*/
      uint16_t sensitivity = 10001 - (*mySensitivity * 2);
      long unitless_setpoint = (((long)*mySetpoint - 0) * 10000L) / (sensitivity - 0);
      long unitless_input = (((long)*myInput - 0) * 10000L) / (sensitivity - 0);
      long error = unitless_setpoint - unitless_input;

      ITerm += error;

      // uint16_t bias = 50; //Base target DC%
      long output = 0;

      if(ki != 0)
      {
        output = ((outMax * limitMultiplier * 100) - FeedForward) / (long)ki;
        if (output < 0) { output = 0; }
      }
      if (ITerm > output) { ITerm = output; }

      if(ki != 0)
      {
        output = (FeedForward - (-outMin * limitMultiplier * 100)) / (long)ki;
        if (output < 0) { output = 0; }
      }
      else { output = 0; }
      if (ITerm < -output) { ITerm = -output; }

      /*Compute PID Output*/
      output = (kp * error) + (ki * ITerm) + (kd * (error - lastError));
      output = FeedForward + (output / 10); //output is % multiplied by 1000. To get % with 2 decimal places, divide it by 10. Likewise, bias is % in whole numbers. Multiply it by 100 to get it with 2 places.

      if(output > (outMax * limitMultiplier))
      {
         output  = (outMax * limitMultiplier);
         ITerm -= error;
      }
      if(output < (outMin * limitMultiplier))
      {
         output  = (outMin * limitMultiplier);
         ITerm -= error;
      }

	    *myOutput = output;

      /*Remember some variables for next time*/
      lastTime = now;
      lastError = error;

      return true;
   }
   else return false;
}

// LCOV_EXCL_START
bool integerPID_ideal::Compute(uint16_t FeedForward)
{
    return Compute(millis(), FeedForward);
}
// LCOV_EXCL_STOP

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void integerPID_ideal::SetTunings(uint8_t Kp, uint8_t Ki, uint8_t Kd)
{
   if ( dispKp == Kp && dispKi == Ki && dispKd == Kd ) return; //Only do anything if one of the values has changed
   dispKp = Kp; dispKi = Ki; dispKd = Kd;

  kp = Kp;
  ki = Ki;
  kd = Kd;

  if(controllerDirection == REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void integerPID_ideal::SetOutputLimits(long Min, long Max)
{
   if(Min < Max)
   {
     outMin = Min;
     outMax = Max;
   }
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void integerPID_ideal::Initialize()
{
   ITerm = 0;
   lastInput = *myInput;
   lastError = 0;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void integerPID_ideal::SetControllerDirection(uint8_t Direction)
{
   if(Direction != controllerDirection)
   {
	    kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}