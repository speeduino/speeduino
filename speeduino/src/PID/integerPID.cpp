#include "integerPID.h"
#include "../../maths.h"

constexpr uint8_t PID_SHIFTS = 10; //Increased resolution

integerPID::integerPID(void)
{
   setOutputLimits(0, 255); 
}

bool integerPID::compute(uint32_t now, int32_t input, int32_t* pOutput)
{
   uint32_t timeChange = (now - _lastTime);
   if ((!_isActive) || (timeChange < _sampleTime)) return false;

   /*Compute all the working error variables*/
   int32_t error = _setpoint - input;
   int32_t dInput = (input - _lastInput);

   if (_pidParams.Ki != 0)
   {
      _integralTerm += (_pidParams.Ki * error); //integral += error × dt
      _integralTerm = clamp(_integralTerm, _outMin - _feedForwardTerm, _outMax - _feedForwardTerm);
   }

   /*Compute PID Output*/
   int32_t output;
   
   output = (_pidParams.Kp * error);
   if (_pidParams.Ki != 0) { output += _integralTerm; }
   if (_pidParams.Kd != 0) { output -= (_pidParams.Kd * dInput)>>2; }
   output = clamp(output + _feedForwardTerm, _outMin, _outMax);

   *pOutput = output >> PID_SHIFTS;

   /*Remember some variables for next time*/
   _lastInput = input;
   _lastTime = now;

   return true;
}


static PidTuningParameters scaleTuningParameters(const PidTuningParameters& params, uint8_t minComputeInterval)
{
    PidTuningParameters scaledParams = params * 32;
    uint16_t inverseSampleTimeInSec = MILLI_PER_SEC / minComputeInterval;
    scaledParams.Ki = (scaledParams.Ki) / inverseSampleTimeInSec;
    scaledParams.Kd = (scaledParams.Kd) * inverseSampleTimeInSec;
    return scaledParams;
}

void integerPID::setTunings(const PidTuningParameters &pidParams, uint32_t nowMs, uint16_t minComputeInterval)
{
   _sampleTime = minComputeInterval;
   _lastTime = nowMs - minComputeInterval;
   _pidParams = scaleTuningParameters(pidParams, minComputeInterval);
}

void integerPID::setOutputLimits(int32_t min, int32_t max)
{
   if(min >= max) return;
   _outMin = min << PID_SHIFTS;
   _outMax = max << PID_SHIFTS;
}

void integerPID::activate(int32_t input)
{
   if (!_isActive)
   {  /*we just went from manual to auto*/
      integerPID::reset(input);
   }
   _isActive = true;
}

void integerPID::reset(int32_t input)
{
   _lastInput = input;
   resetIntegeral();
}

void integerPID::resetIntegeral(void) 
{ 
   _integralTerm=0;
}

void integerPID::setFeedForwardTerm(int32_t feedForwardTerm) 
{ 
   _feedForwardTerm = feedForwardTerm << PID_SHIFTS; 
}