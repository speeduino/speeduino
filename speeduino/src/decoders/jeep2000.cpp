#include "jeep2000.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace jeep2000 {

shared::state_t __attribute__((optimize("Os"), noinline)) initialise(uint32_t curMicros, const statuses &current)
{
    shared::state_t state;

    state.toothAngles[0] = 174;
    state.toothAngles[1] = 194;
    state.toothAngles[2] = 214;
    state.toothAngles[3] = 234;
    state.toothAngles[4] = 294;
    state.toothAngles[5] = 314;
    state.toothAngles[6] = 334;
    state.toothAngles[7] = 354;
    state.toothAngles[8] = 414;
    state.toothAngles[9] = 434;
    state.toothAngles[10] = 454;
    state.toothAngles[11] = 474;

    state.MAX_STALL_TIME = (MICROS_PER_DEG_1_RPM/50U) * 60U; //Minimum 50rpm. (3333uS is the time per degree at 50rpm). Largest gap between teeth is 60 degrees.
    if(current.initialisationComplete == false) { state.toothCurrentCount = 13; state.toothLastToothTime = curMicros; } //Set a startup value here to avoid filter errors when starting. This MUST have the initial check to prevent the fuel pump just staying on all the time
    state.decoderStatus.toothAngleIsCorrect = true;

    return state;
}


void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
{
  if(decoderState.toothCurrentCount == 13) { decoderState.decoderStatus.syncStatus = SyncStatus::None; } //Indicates sync has not been achieved (Still waiting for 1 revolution of the crank to take place)
  else
  {
    decoderState.curGap = curTime - decoderState.toothLastToothTime;
    if ( decoderState.curGap >= decoderState.triggerFilterTime )
    {
      if(decoderState.toothCurrentCount == 0)
      {
         decoderState.toothCurrentCount = 1; //Reset the counter
         decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
         decoderState.toothOneTime = curTime;
         decoderState.decoderStatus.syncStatus = SyncStatus::Full;
         current.startRevolutions++; //Counter
         decoderState.triggerToothAngle = 60; //There are groups of 4 pulses (Each 20 degrees apart), with each group being 60 degrees apart. Hence #1 is always 60
      }
      else
      {
        decoderState.toothCurrentCount++; //Increment the tooth counter
        decoderState.triggerToothAngle = decoderState.toothAngles[(decoderState.toothCurrentCount-1)] - decoderState.toothAngles[(decoderState.toothCurrentCount-2)]; //Calculate the last tooth gap in degrees
      }

      decoderState.setFilter(decoderState.curGap, page4); //Recalc the new filter value

      decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

      decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
      decoderState.toothLastToothTime = curTime;
    } //Trigger filter
  } //Sync check
}

void triggerSecondary(shared::state_t &decoderState)
{
  decoderState.toothCurrentCount = 0; //All we need to do is reset the tooth count back to zero, indicating that we're at the beginning of a new revolution
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount);

  int16_t crankAngle = 0;
  // This is the special case to handle when the 'last tooth' seen was the cam tooth. Since
  // the tooth timings were taken on the previous crank tooth, the previous crank tooth angle is used here, not cam angle.
  if (std::get<2>(data) == 0)
  {
    decoders::shared::simple_crank_angle_calculator_t calculator;
    std::tie(calculator._toothLastToothTime, calculator._revZeroOrOne, std::ignore) = data;
    crankAngle = calculator.calculateCrankAngle(114, currMicros, page4);
  } 
  else
  { 
    crankAngle = decoders::shared::lookup_crank_angle_calculator_t(data).calculateCrankAngle(currMicros, decoderState.toothAngles, page4);
  }

  //Estimate the number of degrees travelled since the last tooth}
  return decoders::shared::clampCrankAngle(crankAngle);
}

}
}