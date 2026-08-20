#include "crank_angle_calculator.h"
#include "src/utils/tuple.h"
#include "crankMaths.h"
#include "elapsed_time.h"

// ================================ Calculator Mixins =====================================

last_tooth_rev_calculator_t::last_tooth_rev_calculator_t(uint32_t toothLastToothTime)
: _toothLastToothTime(toothLastToothTime)
{
}
    
int16_t last_tooth_rev_calculator_t::calculate(uint32_t currMicros) const
{
  // Estimate the number of degrees travelled since the last tooth
  return timeToAngle(timeElapsed(currMicros, _toothLastToothTime));
}

tooth_interval_calculator_t::tooth_interval_calculator_t(uint32_t toothLastToothTime, uint32_t toothLastMinusOneToothTime, uint16_t toothAngle, bool toothAngleCorrect)
: _toothLastToothTime(toothLastToothTime)
, _toothLastMinusOneToothTime(toothLastMinusOneToothTime)
, _toothAngle(toothAngle)
, _toothAngleCorrect(toothAngleCorrect)
{
}
  
int16_t tooth_interval_calculator_t::calculate(uint32_t currMicros) const
{
  int16_t result = 0;
  if (_toothAngleCorrect)
  {
    result = (timeElapsed(currMicros, _toothLastToothTime) * _toothAngle) / timeElapsed(_toothLastToothTime, _toothLastMinusOneToothTime);
  }
  else
  {
    result = timeToAngle(timeElapsed(currMicros, _toothLastToothTime));
  }
  return result;
}

lookup_initial_calculator_t::lookup_initial_calculator_t(uint16_t toothCurrentCount)
: _toothCurrentCount(toothCurrentCount)
{
}

int16_t lookup_initial_calculator_t::calculate(const int16_t toothAngles[]) const
{
  int16_t initial = 0;
  if ((_toothCurrentCount!=0U) && (toothAngles!=nullptr))
  {
    // Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was.
    initial = toothAngles[_toothCurrentCount - 1];
  }
  return initial;
}


compute_initial_calculator_t::compute_initial_calculator_t(uint16_t toothCurrentCount, uint16_t toothAngle)
: _toothCurrentCount(toothCurrentCount)
, _toothAngle(toothAngle)
{
}
  
int16_t compute_initial_calculator_t::calculate(void) const
{
  int16_t initial = 0;
  if (_toothCurrentCount!=0U)
  {
    // Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents,
    // This gives accuracy only to the nearest tooth.
    initial = (_toothCurrentCount - 1) * _toothAngle;
  }
  return initial;
}

sequential_correction_calculator_t::sequential_correction_calculator_t(bool revZeroOrOne)
: _revZeroOrOne(revZeroOrOne)
{
}

int16_t sequential_correction_calculator_t::calculate(const config4 &page4) const
{
  int16_t offset = 0;
  //Sequential check (simply sets whether we're on the first or 2nd revolution of the cycle)
  if ( (_revZeroOrOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
  { 
    offset = 360; 
  }
  return offset;
}

// ================================ Calculators =====================================

simple_crank_angle_calculator_t::simple_crank_angle_calculator_t(const std::tuple<uint32_t, bool> &data)
: last_tooth_rev_calculator_t(std::get<0>(data))
, sequential_correction_calculator_t(std::get<1>(data))
{
}

int16_t simple_crank_angle_calculator_t::calculate(int16_t initialCrankAngle, uint32_t currMicros, const config4 &page4) const
{
  return  initialCrankAngle 
        + page4.triggerAngle 
        + last_tooth_rev_calculator_t::calculate(currMicros)
        + sequential_correction_calculator_t::calculate(page4);
}

lookup_crank_angle_calculator_t::lookup_crank_angle_calculator_t(const std::tuple<uint32_t, bool, uint16_t> &data)
: last_tooth_rev_calculator_t(std::get<0>(data))
, sequential_correction_calculator_t(std::get<1>(data))
, lookup_initial_calculator_t(std::get<2>(data))
{
}

int16_t lookup_crank_angle_calculator_t::calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const
{
  return lookup_initial_calculator_t::calculate(toothAngles)
        + page4.triggerAngle 
        + last_tooth_rev_calculator_t::calculate(currMicros)
        + sequential_correction_calculator_t::calculate(page4);
}

trigger_angle_crank_angle_calculator_t::trigger_angle_crank_angle_calculator_t(const data_type &data)
: last_tooth_rev_calculator_t(std::get<0>(data))
, sequential_correction_calculator_t(std::get<1>(data))
, compute_initial_calculator_t(std::get<2>(data), std::get<3>(data))
{
}

int16_t trigger_angle_crank_angle_calculator_t::calculate(uint32_t currMicros, const config4 &page4) const
{
  return compute_initial_calculator_t::calculate()
        + page4.triggerAngle 
        + last_tooth_rev_calculator_t::calculate(currMicros)
        + sequential_correction_calculator_t::calculate(page4);
}

lookup_crank_angle_calculator_tooth_interval_t::lookup_crank_angle_calculator_tooth_interval_t(const std::tuple<uint32_t, uint32_t, bool, uint16_t, uint16_t, bool> &data)
: tooth_interval_calculator_t(std::get<0>(data), std::get<1>(data), std::get<4>(data), std::get<5>(data))
, sequential_correction_calculator_t(std::get<2>(data))
, lookup_initial_calculator_t(std::get<3>(data))
{
}

int16_t lookup_crank_angle_calculator_tooth_interval_t::calculate(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const
{
  return lookup_initial_calculator_t::calculate(toothAngles)
        + page4.triggerAngle 
        + tooth_interval_calculator_t::calculate(currMicros)
        + sequential_correction_calculator_t::calculate(page4);
}


compute_crank_angle_calculator_tooth_interval_t::compute_crank_angle_calculator_tooth_interval_t(const std::tuple<uint32_t, uint32_t, bool, uint16_t, uint16_t, bool> &data)
: tooth_interval_calculator_t(std::get<0>(data), std::get<1>(data), std::get<4>(data), std::get<5>(data))
, sequential_correction_calculator_t(std::get<2>(data))
, compute_initial_calculator_t(std::get<3>(data), std::get<4>(data))
{
}

int16_t compute_crank_angle_calculator_tooth_interval_t::calculate(uint32_t currMicros, const config4 &page4) const
{
  return compute_initial_calculator_t::calculate()
        + page4.triggerAngle 
        + tooth_interval_calculator_t::calculate(currMicros)
        + sequential_correction_calculator_t::calculate(page4);
}
