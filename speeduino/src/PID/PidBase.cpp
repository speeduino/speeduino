#include "PidBase.h"

int32_t PidBase::compute(int32_t feedForwardTerm, int32_t error, int32_t derivative)
{
   _integralTerm += error;

   /*Compute PID Output*/
   int32_t output = feedForwardTerm + _pidParams.compute(error, _integralTerm, derivative);

   /* Clamp output and back-calculate integral if necessary */
   if (output > _max)
   {
       output = _max;
       _integralTerm -= error;
   }
   else if (output < _min)
   {
       output = _min;
       _integralTerm -= error;
   }   

   return output;
}