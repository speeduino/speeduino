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

   /*Compute PID Output*/
   *pOutput = PidBase::compute(_feedForwardTerm, _setpoint - input, _lastInput - input) >> PID_SHIFTS;

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
    scaledParams.Kd = ((scaledParams.Kd) * inverseSampleTimeInSec)>>2;
    return scaledParams;
}

void integerPID::setTunings(const PidTuningParameters &pidParams, uint32_t nowMs, uint16_t minComputeInterval)
{
   _sampleTime = minComputeInterval;
   _lastTime = nowMs - minComputeInterval;
   PidBase::setTunings(scaleTuningParameters(pidParams, minComputeInterval));
}

void integerPID::setOutputLimits(int32_t min, int32_t max)
{
   PidBase::setOutputLimits(min<< PID_SHIFTS, max<< PID_SHIFTS);
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
   PidBase::resetIntegral();
}

void integerPID::setFeedForwardTerm(int32_t feedForwardTerm) 
{ 
   _feedForwardTerm = feedForwardTerm << PID_SHIFTS; 
}