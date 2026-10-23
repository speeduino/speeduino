#include "gm7x.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"
#include "unit_testing.h"

namespace decoders {

namespace gm7x {

shared::state_t __attribute__((optimize("Os"))) initialise(void)
{
    shared::state_t state;

    state.decoderFeatures.supportsPerToothIgnition = true;
    
    state.triggerToothAngle = 360 / 6; //The number of degrees that passes from tooth to tooth
    state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

    return state;
}


void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
{
    decoderState.lastGap = decoderState.curGap;
    decoderState.curGap = curTime - decoderState.toothLastToothTime;
    decoderState.toothCurrentCount++; //Increment the tooth counter
    decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

    if( (decoderState.toothLastToothTime > 0) && (decoderState.toothLastMinusOneToothTime > 0) )
    {
      if( decoderState.toothCurrentCount > 7 )
      {
        decoderState.toothCurrentCount = 1;
        decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
        decoderState.toothOneTime = curTime;

        decoderState.decoderStatus.toothAngleIsCorrect = true;
      }
      else
      {
        decoderState.targetGap = (decoderState.lastGap) >> 1; //The target gap is set at half the last tooth gap
        if ( decoderState.curGap < decoderState.targetGap ) //If the gap between this tooth and the last one is less than half of the previous gap, then we are very likely at the magical 3rd tooth
        {
          decoderState.toothCurrentCount = 3;
          decoderState.decoderStatus.syncStatus = SyncStatus::Full;
          decoderState.decoderStatus.toothAngleIsCorrect = false; //The tooth angle is double at this point
          current.startRevolutions++; //Counter
        }
        else
        {
          decoderState.decoderStatus.toothAngleIsCorrect = true;
        }
      }
    }

    //New ignition mode!
    if(page2.perToothIgn == true)
    {
      if(decoderState.toothCurrentCount != 3) //Never do the check on the extra tooth. It's not needed anyway
      {
        //page4.triggerAngle must currently be below 48 and above -81
        int16_t crankAngle;
        if( decoderState.toothCurrentCount < 3 )
        {
          crankAngle = ((decoderState.toothCurrentCount - 1) * decoderState.triggerToothAngle) + 42 + page4.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
        }
        else
        {
          crankAngle = ((decoderState.toothCurrentCount - 2) * decoderState.triggerToothAngle) + 42 + page4.triggerAngle; //Number of teeth that have passed since tooth 1, multiplied by the angle each tooth represents, plus the angle that tooth 1 is ATDC. This gives accuracy only to the nearest tooth.
        }
        checkPerToothTiming(current , decoderState, page4, crankAngle, decoderState.toothCurrentCount);
      } 
    }

    decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
    decoderState.toothLastToothTime = curTime;
}

int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
{
  auto data = atomic_copy(decoderState.toothLastToothTime, decoderState.revolutionOne, decoderState.toothCurrentCount, decoderState.triggerToothAngle);

  //Check if the last tooth seen was the reference tooth (Number 3). All others can be calculated, but tooth 3 has a unique angle
  int16_t crankAngle = 0;
  if( std::get<2>(data) == 3 )
  {
    decoders::shared::simple_crank_angle_calculator_t calculator;
    std::tie(calculator._toothLastToothTime, calculator._revZeroOrOne, std::ignore, std::ignore) = data;
    crankAngle = calculator.calculateCrankAngle(112, currMicros, page4);
  }
  else
  {
    decoders::shared::trigger_angle_crank_angle_calculator_t calculator(data);
    if (calculator._toothCurrentCount > 3 )
    {
      --calculator._toothCurrentCount;
    }
    crankAngle = calculator.calculateCrankAngle(currMicros, page4)+42;
  }

  return decoders::shared::clampCrankAngle(crankAngle);
}

}
}