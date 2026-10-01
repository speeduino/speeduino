#include "rev_time_calcs.h"
#include "atomic.h"
#include "elapsed_time.h"
#include "crankMaths.h"

namespace decoders {

namespace detail {

// If tooth angle calculations are based on cam teeth (not crank teeth), then results must be
// divided by 2 (shifted by 1) 
static inline uint8_t calcToothCalcShift(bool isCamTeeth)
{
  return isCamTeeth ? 1U : 0U;
}

tooth_speed_sample_t atomicToothSpeedSample(const statuses &current, const state_t &decoderState)
{
  tooth_speed_sample_t sample = {};
  ATOMIC()
  {
    sample.lastToothTime = decoderState.toothLastToothTime;
    sample.prevToothTime = decoderState.toothLastMinusOneToothTime;
    sample.lastToothOneTime = decoderState.toothOneTime;
    sample.prevToothOneTime = decoderState.toothOneMinusOneTime;
    sample.startRevolutions = current.startRevolutions;
    sample.toothAngle = decoderState.triggerToothAngle;
    sample.syncStatus = decoderState.decoderStatus.syncStatus;
  }
  return sample;
}

uint32_t publishedRevolutionTime(const statuses &current)
{
  return current.revolutionTime;
}

// The tooth times are micros() values, which wrap around: timeElapsed() handles that, so the times are only checked for being set & distinct
static __attribute__((noinline)) bool toothInterval(uint32_t lastTime, uint32_t prevTime, uint32_t &intervalUs)
{
  if ((prevTime==0U) || (lastTime==0U) || (lastTime==prevTime)) { return false; }
  intervalUs = timeElapsed(lastTime, prevTime);
  return true;
}

bool lastToothInterval(const tooth_speed_sample_t &sample, uint32_t &intervalUs)
{
  return toothInterval(sample.lastToothTime, sample.prevToothTime, intervalUs);
}

bool toothOneInterval(const tooth_speed_sample_t &sample, uint32_t &intervalUs)
{
  return toothInterval(sample.lastToothOneTime, sample.prevToothOneTime, intervalUs);
}

/**
 * Crank period implied by one tooth interval that covers toothAngleDeg degrees.
 * Split so interval * 360 cannot wrap a uint32_t: (interval / angle) * 360 + (interval % angle) * 360 / angle.
 */
static inline uint32_t revolutionTimeFromInterval(uint32_t intervalUs, uint16_t toothAngleDeg)
{
  uint32_t whole = intervalUs / toothAngleDeg;
  uint32_t remainder = intervalUs % toothAngleDeg;
  return (whole * 360UL) + ((remainder * 360UL) / toothAngleDeg);
}

/** @return false when the angle or the interval cannot be used. Caller keeps the published period. */
bool revolutionTimeFromLastTooth(const tooth_speed_sample_t &sample, uint32_t &revolutionTime)
{
  uint32_t interval = 0U;
  if ((sample.toothAngle==0U) || (lastToothInterval(sample, interval)==false)) { return false; }
  revolutionTime = revolutionTimeFromInterval(interval, sample.toothAngle);
  return true;
}

__attribute__((noinline)) uint32_t stdGetRevolutionTime(const statuses &current, const state_t &decoderState, bool isCamTeeth)
{
  tooth_speed_sample_t sample = atomicToothSpeedSample(current, decoderState);
  bool cranking = (current.RPM < current.crankRPM) && (sample.startRevolutions==0U);
  uint32_t interval = 0U;
  if ((sample.syncStatus!=SyncStatus::None) && (cranking==false) && (toothOneInterval(sample, interval)==true))
  {
    //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)
    return interval >> calcToothCalcShift(isCamTeeth);
  }

  return publishedRevolutionTime(current);
}

 __attribute__((noinline)) uint32_t crankingGetRevolutionTime(const statuses &current, const state_t &decoderState, const config4 &page4, byte totalTeeth, bool isCamTeeth)
{
  tooth_speed_sample_t sample = atomicToothSpeedSample(current, decoderState);
  uint32_t interval = 0U;
  if ( (sample.startRevolutions >= page4.StgCycles)
    && (sample.syncStatus!=SyncStatus::None)
    && (lastToothInterval(sample, interval)==true) )
  {
    return (interval * totalTeeth) >> calcToothCalcShift(isCamTeeth);
  }

  return publishedRevolutionTime(current);
}

}
}