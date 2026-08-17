#include "decoder_tooth_tracker.h"
#include "crankMaths.h"
#include "elapsed_time.h"

static int16_t refineCrankAngle(int16_t crankAngle, uint32_t currMicros, const tooth_tracker_t &tracker)
{
  // Estimate the number of degrees travelled since the last tooth
   return crankAngle + timeToAngle(timeElapsed(currMicros, tracker.toothLastToothTime));  
}
    
static int16_t applyTriggerAngle(int16_t crankAngle, const config4 &page4)
{
  // Offset the angle by the user defined offset from TDC
  return crankAngle + page4.triggerAngle;
}

int16_t tooth_tracker_t::calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const
{
  if (toothCurrentCount==0U)
  {
    return refineCrankAngle(page4.triggerAngle, currMicros, *this);
  }

  return refineCrankAngle(
          applyTriggerAngle(
            // Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus
            // the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
            (toothCurrentCount - 1) * triggerToothAngle, 
            page4),
          currMicros,
          *this);
}

int16_t tooth_tracker_t::calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const
{
  if ((toothCurrentCount==0U) || (toothAngles==nullptr))
  {
    return refineCrankAngle(page4.triggerAngle, currMicros, *this);
  }

  return refineCrankAngle(
          applyTriggerAngle(
            // Perform a lookup of the fixed toothAngles array to find what the angle of the last tooth passed was.
            toothAngles[toothCurrentCount - 1],
            page4),
          currMicros,
          *this);
}


static uint16_t getSecondRevolutionOffset(const seq_tooth_tracker_t &tracker, const config4 &page4)
{
  //Sequential check (simply sets whether we're on the first or 2nd revolution of the cycle)
  if ( (tracker.revZeroOrOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
  { 
    return 360; 
  }
  return 0;
}

int16_t seq_tooth_tracker_t::calculateCrankAngle(uint32_t currMicros, uint16_t triggerToothAngle, const config4 &page4) const
{
    return tooth_tracker_t::calculateCrankAngle(currMicros, triggerToothAngle, page4) + getSecondRevolutionOffset(*this, page4);
}
    
int16_t seq_tooth_tracker_t::calculateCrankAngle(uint32_t currMicros, const int16_t toothAngles[], const config4 &page4) const
{
    return tooth_tracker_t::calculateCrankAngle(currMicros, toothAngles, page4) + getSecondRevolutionOffset(*this, page4);
}
