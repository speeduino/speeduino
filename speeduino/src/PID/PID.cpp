#include "PID.h"
#include "../../maths.h"

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(void)
{
	PID::SetOutputLimits(0, 255);				//default output limit corresponds to
												//the arduino pwm limits
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool PID::Compute(long input, long* pOutput)
{
   if(!_isActive) return false;
   /*Compute all the working error variables*/
   long error = _setpoint - input;
   ITerm = clamp(ITerm + (_pidParams.Ki * error), outMin, outMax);

   long dInput = (input - lastInput);

   /*Compute PID Output*/
   long output = (_pidParams.Kp * error) + ITerm- (_pidParams.Kd * dInput);
   output = clamp(output, outMin, outMax);

   *pOutput = output/1000;

   /*Remember some variables for next time*/
   lastInput = input;
   return true;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void PID::SetTunings(const PidTuningParameters &pidParams, PidDirection direction)
{
   /*
   double SampleTimeInSec = ((double)SampleTime)/1000;
   _pidParams.Kp = Kp;
   _pidParams.Ki = Ki * SampleTimeInSec;
   _pidParams.Kd = Kd / SampleTimeInSec;
   */
  //long InverseSampleTimeInSec = 100;
  _pidParams = pidParams;
  _pidParams.Kd = _pidParams.Kd * 10;

  if(direction == PidDirection::Reverse)
   {
      _pidParams = _pidParams * -1;
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
void PID::SetOutputLimits(long Min, long Max)
{
   if(Min >= Max) return;
   outMin = Min*1000;
   outMax = Max*1000;
}

void PID::activate(long input)
{
   if(!_isActive)
   {  
   // We just went from manual to auto
      PID::Initialize(input);
   }
   _isActive = true;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize(long input)
{
   ITerm = 0;
   lastInput = input;
}