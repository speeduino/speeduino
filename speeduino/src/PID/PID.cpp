#include "PID.h"
#include "../../maths.h"

PID::PID(void)
{
	PID::setOutputLimits(0, 255);
}

int32_t PID::compute(int32_t input)
{
   /*Compute all the working error variables*/
   int32_t error = _setpoint - input;
   int32_t dInput = _lastInput- input;

   _integralTerm += error;

   /*Compute PID Output*/
   int32_t output = _pidParams.compute(error, _integralTerm, dInput);

   /* Clamp output and back-calculate integral if necessary */
   if (output > _outMax)
   {
       output = _outMax;
       _integralTerm -= error;
   }
   else if (output < _outMin)
   {
       output = _outMin;
       _integralTerm -= error;
   }   

   /*Remember some variables for next time*/
   _lastInput = input;

   return output/1000;
}

void PID::setTunings(const PidTuningParameters &pidParams)
{
  _pidParams = pidParams;
  _pidParams.Kd = _pidParams.Kd * 10;
}

void PID::setOutputLimits(int32_t Min, int32_t Max)
{
   if(Min >= Max) return;
   _outMin = Min*1000;
   _outMax = Max*1000;
}

void PID::resetIntegral(int32_t input)
{
   _integralTerm = 0;
   _lastInput = input;
}