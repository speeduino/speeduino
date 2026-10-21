#include "dualWheel.h"
#include "shared/perToothIgnition.h"
#include "shared/rev_time_calcs.h"
#include "shared/crank_angle_calculator.h"
#include "scheduler_ignition_controller.h"

namespace decoders {

namespace dual_wheel {

    shared::state_t __attribute__((optimize("Os"))) initialise(const config4 &page4)
    {
        shared::state_t state = {};

        state.decoderFeatures.supportsSequential = true;
        state.decoderStatus.toothAngleIsCorrect = true; //This is always true for this pattern
        state.decoderFeatures.supportsPerToothIgnition = true;

        uint16_t degreeTracking = (page4.TrigSpeed == CAM_SPEED) ? 720 : 360;
        state.setTriggerToothAngle(degreeTracking / page4.triggerTeeth);
        state.toothCurrentCount = UINT8_MAX; //Default value
        state.triggerFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * page4.triggerTeeth)); //Trigger filter time is the shortest possible time (in uS) that there can be between crank teeth (ie at max RPM). Any pulses that occur faster than this time will be discarded as noise
        state.triggerSecFilterTime = (MICROS_PER_SEC / (MAX_RPM / 60U * 2U)) / 2U; //Same as above, but fixed at 2 teeth on the secondary input and divided by 2 (for cam speed)
        state.MAX_STALL_TIME = ((MICROS_PER_DEG_1_RPM/50U) * state.triggerToothAngle); //Minimum 50rpm. (3333uS is the time per degree at 50rpm)

        return state;
    }

    void triggerPrimary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config2 &page2, const config4 &page4)
    {
        decoderState.curGap = curTime - decoderState.toothLastToothTime;
        if ( decoderState.curGap >= decoderState.triggerFilterTime )
        {
            decoderState.toothCurrentCount++; //Increment the tooth counter
            decoderState.decoderStatus.validTrigger = true; //Flag this pulse as being a valid trigger (ie that it passed filters)

            decoderState.toothLastMinusOneToothTime = decoderState.toothLastToothTime;
            decoderState.toothLastToothTime = curTime;

            if ( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
            {
                if ( (decoderState.toothCurrentCount == 1) || (decoderState.toothCurrentCount > page4.triggerTeeth) )
                {
                    decoderState.toothCurrentCount = 1;
                    decoderState.revolutionOne = !decoderState.revolutionOne; //Flip sequential revolution tracker
                    decoderState.toothOneMinusOneTime = decoderState.toothOneTime;
                    decoderState.toothOneTime = curTime;
                    current.startRevolutions++; //Counter
                    if ( page4.TrigSpeed == CAM_SPEED ) { current.startRevolutions++; } //Add an extra revolution count if we're running at cam speed
                }

                decoderState.setFilter(decoderState.curGap, page4); //Recalc the new filter value
            }

            //NEW IGNITION MODE
            if( (page2.perToothIgn == true) && (current.rotationStatus!=EngineRotationStatus::Cranking) ) 
            {
                int16_t crankAngle = ( (decoderState.toothCurrentCount-1) * decoderState.triggerToothAngle ) + page4.triggerAngle;
                uint16_t currentTooth;
                if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (decoderState.revolutionOne == true) && (page4.TrigSpeed == CRANK_SPEED) )
                {
                crankAngle += 360;
                currentTooth = (page4.triggerTeeth + decoderState.toothCurrentCount); 
                }
                else{ currentTooth = decoderState.toothCurrentCount; }
                checkPerToothTiming(current, decoderState, page4, crankAngle, currentTooth);
            }
        } //Trigger filter
    }
}
}