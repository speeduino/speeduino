#include "integerPID.h"
#include "../../maths.h"

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
integerPID::integerPID(void)
{
   SetOutputLimits(0, 255);   //default output limit corresponds to the arduino pwm limits
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool integerPID::Compute(unsigned long now, long input, long FeedForwardTerm, long* pOutput)
{
   if(!_isActive) return false;
   unsigned long timeChange = (now - _lastTime);
   if(timeChange >= _sampleTime)
   {
      /*Compute all the working error variables*/
      long error = _setpoint - input;
      long dInput = (input - lastInput);
      FeedForwardTerm <<= PID_SHIFTS;

      if (_pidParams.Ki != 0)
      {
         outputSum += (_pidParams.Ki * error); //integral += error × dt
         outputSum = clamp(outputSum, outMin - FeedForwardTerm, outMax - FeedForwardTerm);
      }

      /*Compute PID Output*/
      long output;
      
      output = (_pidParams.Kp * error);
      if (_pidParams.Ki != 0) { output += outputSum; }
      if (_pidParams.Kd != 0) { output -= (_pidParams.Kd * dInput)>>2; }
      output = clamp(output + FeedForwardTerm, outMin, outMax);

      *pOutput = output >> PID_SHIFTS;

      /*Remember some variables for next time*/
      lastInput = input;
      _lastTime = now;

      return true;
   }
   return false;
}


static PidTuningParameters scaleTuningParameters(const PidTuningParameters& params, uint8_t sampleTime)
{
    PidTuningParameters scaledParams = params * 32;
    uint16_t inverseSampleTimeInSec = MILLIS_PER_SEC / sampleTime;
    scaledParams.Ki = (scaledParams.Ki) / inverseSampleTimeInSec;
    scaledParams.Kd = (scaledParams.Kd) * inverseSampleTimeInSec;
    return scaledParams;
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void integerPID::SetTunings(const PidTuningParameters &pidParams, PidDirection direction, uint32_t nowMs, uint16_t sampleTime)
{
   _sampleTime = sampleTime;
   _lastTime = nowMs - sampleTime;

   if(direction == PidDirection::Reverse)
   {
      _pidParams = scaleTuningParameters(pidParams, sampleTime) * -1;
   }
   else
   {
      _pidParams = scaleTuningParameters(pidParams, sampleTime);
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
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void integerPID::activate(long input)
{
   if (!_isActive)
   {  /*we just went from manual to auto*/
      integerPID::Initialize(input);
   }
   _isActive = true;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void integerPID::Initialize(long input)
{
   lastInput = input;
   ResetIntegeral();
}

void integerPID::ResetIntegeral() { outputSum=0;}
