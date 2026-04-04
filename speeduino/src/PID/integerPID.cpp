#include "integerPID.h"
#include <Arduino.h>

constexpr uint8_t PID_SHIFTS = 10; //Increased resolution

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
/**
 * @brief A standard integer PID controller. 
 *
 * @param Input Pointer to the variable holding the current value that is to be controlled. Eg In an idle control this would point to RPM
 * @param Output The address in the page that should be returned. This is as per the page definition in the ini
 * 
 * @return uint8_t The current target advance value in degrees
 */
integerPID::integerPID(long* Input, long* Output, long* Setpoint)
{

   myOutput = Output;
   myInput = Input;
   mySetpoint = Setpoint;

	integerPID::SetOutputLimits(0, 255);   //default output limit corresponds to the arduino pwm limits

   _sampleTime = 250;							//default Controller Sample Time is 0.25 seconds. This is the 4Hz control time for Idle and VVT

   lastTime = millis()-_sampleTime;
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool integerPID::Compute(unsigned long now, long FeedForwardTerm)
{
   if(!_isActive) return false;
   unsigned long timeChange = (now - lastTime);
   if(timeChange >= _sampleTime)
   {
      /*Compute all the working error variables*/
	   long input = *myInput;
      long error = *mySetpoint - input;
      long dInput = (input - lastInput);
      FeedForwardTerm <<= PID_SHIFTS;

      if (_pidParams.Ki!= 0)
      {
         outputSum += (_pidParams.Ki * error); //integral += error × dt
         if(outputSum > outMax-FeedForwardTerm) { outputSum = outMax-FeedForwardTerm; }
         else if(outputSum < outMin-FeedForwardTerm) { outputSum = outMin-FeedForwardTerm; }
      }

      /*Compute PID Output*/
      long output;
      
      output = (_pidParams.Kp * error);
      if (_pidParams.Ki != 0) { output += outputSum; }
      if (_pidParams.Kd != 0) { output -= (_pidParams.Kd * dInput)>>2; }
      output += FeedForwardTerm;

      if(output > outMax) output = outMax;
      else if(output < outMin) output = outMin;

      *myOutput = output >> PID_SHIFTS;

      /*Remember some variables for next time*/
      lastInput = input;
      lastTime = now;

      return true;
   }
   return false;
}

// LCOV_EXCL_START
bool integerPID::Compute(long FeedForwardTerm)
{
    return Compute(millis(), FeedForwardTerm);
}
// LCOV_EXCL_STOP

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void integerPID::SetTunings(const PidTuningParameters &pidParams, PidDirection direction, uint16_t sampleTime)
{
   _sampleTime = sampleTime;

   /*
   double SampleTimeInSec = ((double)_sampleTime)/1000;
   _pidParams.Kp = Kp;
   _pidParams.Ki = Ki * SampleTimeInSec;
   _pidParams.Kd = Kd / SampleTimeInSec;
   */
   long InverseSampleTimeInSec = 1000 / _sampleTime;
   //New resolution, 32x to improve _pidParams.Ki here | _pidParams.Kp 3.125% | _pidParams.Ki 3.125% | _pidParams.Kd 0.781%
   _pidParams = pidParams * 32;
   _pidParams.Ki = _pidParams.Ki / InverseSampleTimeInSec;
   _pidParams.Kd = _pidParams.Kd * InverseSampleTimeInSec;

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
void integerPID::SetOutputLimits(long Min, long Max)
{
   if(Min >= Max) return;
   outMin = Min << PID_SHIFTS;
   outMax = Max << PID_SHIFTS;

   if(_isActive)
   {
      if(*myOutput > Max) *myOutput = Max;
	   else if(*myOutput < Min) *myOutput = Min;

	   if(outputSum > outMax) { outputSum = outMax; }
	   else if(outputSum < outMin) { outputSum = outMin; }
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void integerPID::activate(void)
{
   if (!_isActive)
   {  /*we just went from manual to auto*/
      integerPID::Initialize();
   }
   _isActive = true;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void integerPID::Initialize()
{
   outputSum = *myOutput<<PID_SHIFTS;
   lastInput = *myInput;
   if(outputSum > outMax) { outputSum = outMax; }
   else if(outputSum < outMin) { outputSum = outMin; }
}

void integerPID::ResetIntegeral() { outputSum=0;}
