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

   SampleTime = 250;							//default Controller Sample Time is 0.25 seconds. This is the 4Hz control time for Idle and VVT

   integerPID::SetTunings(1, 1, 1);

   lastTime = millis()-SampleTime;
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
   if(timeChange >= SampleTime)
   {
      /*Compute all the working error variables*/
	   long input = *myInput;
      long error = *mySetpoint - input;
      long dInput = (input - lastInput);
      FeedForwardTerm <<= PID_SHIFTS;

      if (ki != 0)
      {
         outputSum += (ki * error); //integral += error × dt
         if(outputSum > outMax-FeedForwardTerm) { outputSum = outMax-FeedForwardTerm; }
         else if(outputSum < outMin-FeedForwardTerm) { outputSum = outMin-FeedForwardTerm; }
      }

      /*Compute PID Output*/
      long output;
      
      output = (kp * error);
      if (ki != 0) { output += outputSum; }
      if (kd != 0) { output -= (kd * dInput)>>2; }
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
void integerPID::SetTunings(int16_t Kp, int16_t Ki, int16_t Kd)
{
   /*
   double SampleTimeInSec = ((double)SampleTime)/1000;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;
   */
   long InverseSampleTimeInSec = 1000 / SampleTime;
   //New resolution, 32x to improve ki here | kp 3.125% | ki 3.125% | kd 0.781%
   kp = Kp * 32;
   ki = (long)(Ki * 32) / InverseSampleTimeInSec;
   kd = (long)(Kd * 32) * InverseSampleTimeInSec;

   if(_direction == PidDirection::Reverse)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void integerPID::SetSampleTime(uint16_t NewSampleTime)
{
   if (SampleTime == (unsigned long)NewSampleTime) return; //If new value = old value, no action required.

   int16_t oldInverseSampleTimeInSec = 1000 / SampleTime;
   int16_t reverseKp = kp / 32;
   int16_t reverseKi = (long)(ki / 32) * oldInverseSampleTimeInSec;
   int16_t reverseKd = (long)(kd / 32) / oldInverseSampleTimeInSec;
   if(_direction == PidDirection::Reverse)
   {
      reverseKp *= -1;
      reverseKi *= -1;
      reverseKd *= -1;
   }
   
   SampleTime = NewSampleTime;
   //This resets the tuning values with the appropriate new scaling
   SetTunings(reverseKp, reverseKi, reverseKd);
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
   lastMinusOneInput = *myInput;
   if(outputSum > outMax) { outputSum = outMax; }
   else if(outputSum < outMin) { outputSum = outMin; }
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void integerPID::SetControllerDirection(PidDirection direction)
{
   if(_direction != direction)
   {
       kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   _direction = direction;
}

void integerPID::ResetIntegeral() { outputSum=0;}
