#include "PID.h"
#include "../../maths.h"

PID::PID(void)
{
	PID::SetOutputLimits(0, 255);
}

long PID::Compute(long input)
{
   /*Compute all the working error variables*/
   long error = _setpoint - input;
   ITerm = clamp(ITerm + (_pidParams.Ki * error), outMin, outMax);

   long dInput = (input - lastInput);
   /*Remember some variables for next time*/
   lastInput = input;

   /*Compute PID Output*/
   long output = (_pidParams.Kp * error) + ITerm- (_pidParams.Kd * dInput);
   return clamp(output, outMin, outMax)/1000;
}

void PID::SetTunings(const PidTuningParameters &pidParams)
{
  _pidParams = pidParams;
  _pidParams.Kd = _pidParams.Kd * 10;
}

void PID::SetOutputLimits(long Min, long Max)
{
   if(Min >= Max) return;
   outMin = Min*1000;
   outMax = Max*1000;
}

void PID::resetIntegral(long input)
{
   ITerm = 0;
   lastInput = input;
}