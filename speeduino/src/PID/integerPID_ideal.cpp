#include "integerPID_ideal.h"

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

   // We are using "Derivative on Measurement" as described [here](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-derivative-kick/)
   *pOutput = _pidCore.compute(scaledFeedForward, error, (int32_t)_lastInput - input)/10L;

   /*Remember some variables for next time*/
   _lastTime = now;
   _lastInput = input;

   return true;
}

void integerPID_ideal::setOutputLimits(uint8_t min, uint8_t max)
{
   constexpr int32_t limitMultiplier = 1000; //How much outMin and OutMax must be multiplied by to get them in the same scale as the output
   _pidCore.setOutputLimits(min*limitMultiplier, max*limitMultiplier);
}

void integerPID_ideal::initialize(uint16_t input)
{
   _pidCore.resetIntegral();
   _lastInput = input;
}
