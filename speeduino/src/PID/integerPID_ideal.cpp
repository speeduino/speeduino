/**********************************************************************************************
 * Arduino PID Library - Version 1.0.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/
#include "integerPID_ideal.h"
#include "../../maths.h"


/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
integerPID_ideal::integerPID_ideal(void)
{
   integerPID_ideal::SetOutputLimits(20, 80);				//default output limits
}

#include <Arduino.h>
/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool integerPID_ideal::Compute(unsigned long now, long input, uint16_t FeedForwardTerm, uint16_t* pOutput)
{
   unsigned long timeChange = (now - lastTime);
   if(timeChange >= _sampleTime)
   {
      /*Compute all the working error variables*/
      uint16_t sensitivity = 10001 - (_sensitivity * 2);
      long unitless_setpoint = (_setpoint * 10000L) / sensitivity;
      long unitless_input = (input * 10000L) / sensitivity;
      long error = unitless_setpoint - unitless_input;
      // Bias is % in whole numbers. Multiply it by 10 to get it with 2 places.
      uint32_t scaledFeedForward = FeedForwardTerm*10UL;

      ITerm += error;

      /*Compute PID Output*/
      long output = scaledFeedForward + (_pidParams.Kp * error) + (_pidParams.Ki * ITerm) + (_pidParams.Kd * (lastInput - input));
      
      if(output > outMax)
      {
         output = outMax;
         ITerm -= error;
      }
      else if(output < outMin)
      {
         output = outMin;
         ITerm -= error;
      }

	   //output is % multiplied by 1000. To get % with 2 decimal places, divide it by 10. 
      *pOutput = output/10;

      /*Remember some variables for next time*/
      lastTime = now;
      lastInput = input;

      return true;
   }
   else return false;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void integerPID_ideal::SetTunings(const PidTuningParameters& pidParams, PidDirection direction)
{
   if(direction == PidDirection::Reverse)
   {
      _pidParams = _pidParams * -1;
   }
   else
   {
      _pidParams = pidParams;
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
     constexpr uint16_t LIMIT_FACTOR = 1000; //How much outMin and OutMax must be multiplied by to get them in the same scale as the output

     outMin = Min * LIMIT_FACTOR;
     outMax = Max * LIMIT_FACTOR;
   }
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void integerPID_ideal::Initialize(long input)
{
   ITerm = 0;
   lastInput = input;
}

void integerPID_ideal::setSampleTime(unsigned long nowMs, uint16_t sampleTimeMs) 
{ 
   _sampleTime = sampleTimeMs; 
   lastTime = nowMs-sampleTimeMs;
}
