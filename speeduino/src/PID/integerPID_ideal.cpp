#include "integerPID_ideal.h"
#include "../../maths.h"

integerPID_ideal::integerPID_ideal(void)
{
   setOutputLimits(20, 80);				//default output limits
}

static inline int32_t calculateUnitlessError(uint16_t input, uint16_t setpoint, uint16_t sensitivity)
{
   // sensitivity range is [0, 5000]
   uint16_t scaledSensitivity = 10001U - (sensitivity * 2U);
   int32_t unitless_setpoint = (setpoint * 10000L) / scaledSensitivity;
   int32_t unitless_input = (input * 10000L) / scaledSensitivity;
   return unitless_setpoint - unitless_input;
}

bool integerPID_ideal::compute(uint32_t now, uint16_t input, uint16_t* pOutput)
{
   uint32_t timeChange = (now - _lastTime);
   if (timeChange < _sampleTime) return false;

   int32_t error = calculateUnitlessError(input, _setpoint, _sensitivity);
      
   // Bias is % in whole numbers. Multiply it by 10 to get it with 2 places.
   uint32_t scaledFeedForward = _feedForwardTerm*10UL;

   _integralTerm += error;

   /*Compute PID Output*/
   int32_t deltaInput = (int32_t)_lastInput - input;
   int32_t output = scaledFeedForward + _pidParams.compute(error, _integralTerm, deltaInput);
      
   if(output > (int32_t)_outMax)
   {
      output = _outMax;
      _integralTerm -= error;
   }
   else if(output < (int32_t)_outMin)
   {
      output = _outMin;
      _integralTerm -= error;
   }

   //output is % multiplied by 1000. To get % with 2 decimal places, divide it by 10. 
   *pOutput = output/10;

   /*Remember some variables for next time*/
   _lastTime = now;
   _lastInput = input;

   return true;
}

void integerPID_ideal::setOutputLimits(uint8_t min, uint8_t max)
{
   if(min < max)
   {
     constexpr uint32_t LIMIT_FACTOR = 1000UL; //How much outMin and OutMax must be multiplied by to get them in the same scale as the output

     _outMin = min * LIMIT_FACTOR;
     _outMax = max * LIMIT_FACTOR;
   }
}

void integerPID_ideal::initialize(uint16_t input)
{
   _integralTerm = 0;
   _lastInput = input;
}
