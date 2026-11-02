#include "st170.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace st170 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(config4 &page4)
{
  shared::state_t state = {};

  //Set these as we are using the existing missing tooth primary decoder and these will never change.
  page4.triggerTeeth = 36;  
  page4.triggerMissingTeeth = 1;
  page4.TrigSpeed = CRANK_SPEED;
  
  state.setTriggerToothAngle(360 / page4.triggerTeeth);
  state.triggerActualTeeth = page4.triggerTeeth - page4.triggerMissingTeeth; //The number of physical teeth on the wheel. Doing this here saves us a calculation each time in the interrupt
  state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
  
  state.triggerSecFilterTime = MICROS_PER_MIN / MAX_RPM / 8U / 2U; //Cam pattern is 8-3, so 2 nearest teeth are 90 deg crank angle apart. Cam can be advanced by 60 deg, so going from fully retarded to fully advanced closes the gap to 30 deg. Zetec cam pulleys aren't keyed from factory, so I subtracted additional 10 deg to avoid filter to be too aggressive. And there you have it 720/20=36.
  
  state.decoderFeatures.supportsSequential = true;
  state.decoderFeatures.supportsPerToothIgnition = true;
  state.checkSyncToothCount = 36 >> 1; //50% of the total teeth.
  state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle * (1U + 1U)); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

  return state;
}

void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4, const config6 &page6, const config10 &page10)
{
  decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;

  //Safety check for initial startup
  if( (decoderState.toothLastSecToothTime == 0) )
  { 
    decoderState.curGap2 = 0; 
    decoderState.toothLastSecToothTime = curTime;
  }

  if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime )
  {
      decoderState.targetGap2 = (3 * (decoderState.toothLastSecToothTime - decoderState.toothLastMinusOneSecToothTime)) >> 1; //If the time between the current tooth and the last is greater than 1.5x the time between the last tooth and the tooth before that, we make the assertion that we must be at the first tooth after the gap
      decoderState.toothLastMinusOneSecToothTime = decoderState.toothLastSecToothTime;
      if ( (decoderState.curGap2 >= decoderState.targetGap2) || (decoderState.secondaryToothCount == 5) )
      {
        decoderState.secondaryToothCount = 1;
        decoderState.revolutionOne = 1; //Sequential revolution reset
        decoderState.triggerSecFilterTime = 0; //This is used to prevent a condition where serious intermittent signals (Eg someone furiously plugging the sensor wire in and out) can leave the filter in an unrecoverable state
      }
      else
      {
        decoderState.triggerSecFilterTime = decoderState.curGap2 >> 2; //Set filter at 25% of the current speed. Filter can only be recalculated for the regular teeth, not the missing one.
        decoderState.secondaryToothCount++;
      }

    decoderState.toothLastSecToothTime = curTime;

    //Record the VVT Angle
    //We use the first tooth after the long gap as our reference, this remains in the same engine
    //cycle even when the VVT is at either end of its full swing.
    if( (page6.vvtEnabled > 0) && (decoderState.revolutionOne == 1) && (decoderState.secondaryToothCount == 1) )
    {
      int16_t curAngle = normalize((int16_t)0, (int16_t)360, current.decoder.getCrankAngle());
      if( page6.vvtMode == VVT_MODE_CLOSED_LOOP )
      {
        curAngle = LOW_PASS_FILTER( (curAngle << 1), page4.ANGLEFILTER_VVT, curAngle);
        current.vvt1.angle = 360 - curAngle - page10.vvtCL0DutyAng;
      }
    }
  } //Trigger filter
}

uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4)
{
  uint32_t revolutionTime = 0;
  if( current.RPM < current.crankRPM )
  {
    if(decoderState.toothCurrentCount != 1)
    {
      revolutionTime = crankingGetRevolutionTime(current, decoderState, page4, 36, CRANK_SPEED);
    }
    else { revolutionTime = shared::publishedRevolutionTime(current); } //Can't do per tooth calculation if we're at tooth #1 as the missing tooth messes the calculation
  }
  else
  {
    revolutionTime = stdGetRevolutionTime(current, decoderState, CRANK_SPEED);
  }
  return revolutionTime;
}

static uint16_t __attribute__((noinline)) calcSetEndTeeth_FordST170(const IgnitionSchedule &schedule, uint8_t toothAdder, const shared::state_t &decoderState, const config4 &page4) {
  int16_t tempEndTooth = decoderState.toothNumFromScheduleAngles(page4, schedule);
  tempEndTooth = nudge((int16_t)1, (int16_t)(36U + toothAdder + 1U), (int16_t)(tempEndTooth - 1));
  return decoderState.clampToActualTeeth(page4, (uint16_t)tempEndTooth, toothAdder);
}

void setEndTeeth(shared::state_t &decoderState, const config4 &page4)
{
  byte toothAdder = 0;
   if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page4.TrigSpeed == CRANK_SPEED) ) { toothAdder = 36; }

  decoderState.ignitionEndTeeth[0] = calcSetEndTeeth_FordST170(ignitionSchedule1, toothAdder, decoderState, page4);
#if (IGN_CHANNELS >= 2)
  decoderState.ignitionEndTeeth[1] = calcSetEndTeeth_FordST170(ignitionSchedule2, toothAdder, decoderState, page4);
#endif
#if (IGN_CHANNELS >= 3)
  decoderState.ignitionEndTeeth[2] = calcSetEndTeeth_FordST170(ignitionSchedule3, toothAdder, decoderState, page4);
#endif
#if (IGN_CHANNELS >= 4)
  decoderState.ignitionEndTeeth[3] = calcSetEndTeeth_FordST170(ignitionSchedule4, toothAdder, decoderState, page4);
#endif

  // Removed ign channels >4 as an ST170 engine is a 4 cylinder
}

}
}