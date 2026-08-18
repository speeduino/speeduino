#include "crank_angle_calculator.h"
#include "crankMaths.h"
#include "elapsed_time.h"

crank_angle_calculator_t::crank_angle_calculator_t(uint16_t _toothCurrentCount, uint32_t _toothLastToothTime, bool _revZeroOrOne)
: toothCurrentCount(_toothCurrentCount)
, toothLastToothTime(_toothLastToothTime)
, revZeroOrOne(_revZeroOrOne)
{
}

uint16_t crank_angle_calculator_t::getSecondRevolutionOffset(const config4 &page4) const
{
  uint16_t offset = 0;
  //Sequential check (simply sets whether we're on the first or 2nd revolution of the cycle)
  if ( (revZeroOrOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
  { 
    offset = 360; 
  }
  return offset;
}

int16_t crank_angle_calculator_t::calculateAdjustmentSinceLastTooth(uint32_t currMicros) const
{
  // Estimate the number of degrees travelled since the last tooth
  return timeToAngle(timeElapsed(currMicros, toothLastToothTime));  
}

int16_t crank_angle_calculator_t::calculateFromInitial(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const
{
  return  initialCrankAngle 
        + page4.triggerAngle 
        + calculateAdjustmentSinceLastTooth(currMicros)
        + getSecondRevolutionOffset(page4);
}

int16_t lookup_crank_angle_calculator_t::calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const
{
  return calculateFromInitial(calculateInitialAngle(toothAngles), currMicros, page4);
}

int16_t lookup_crank_angle_calculator_t::calculateInitialAngle(const int16_t toothAngles[]) const
{
  int16_t initial = 0;
  if ((toothCurrentCount!=0U) && (toothAngles!=nullptr))
  {
    // Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was.
    initial = toothAngles[toothCurrentCount - 1];
  }
  return initial;
}

int16_t trigger_angle_crank_angle_calculator_t::calculate(uint32_t currMicros, uint16_t toothAngle, const config4 &page4) const
{
  return calculateFromInitial(calculateInitialAngle(toothAngle), currMicros, page4);
}

int16_t trigger_angle_crank_angle_calculator_t::calculateInitialAngle(uint16_t toothAngle) const
{
  int16_t initial = 0;
  if (toothCurrentCount!=0U)
  {
    // Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus
    // the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
    initial = (toothCurrentCount - 1) * toothAngle;
  }
  return initial;
}