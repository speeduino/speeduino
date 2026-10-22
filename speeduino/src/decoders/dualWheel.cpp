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
                uint16_t currentTooth = 0;
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

    void triggerSecondary(uint32_t curTime, statuses &current, shared::state_t &decoderState, const config4 &page4)
    {
        decoderState.curGap2 = curTime - decoderState.toothLastSecToothTime;
        if ( decoderState.curGap2 >= decoderState.triggerSecFilterTime )
        {
            decoderState.toothLastSecToothTime = curTime;
            decoderState.triggerSecFilterTime = decoderState.curGap2 >> 2; //Set filter at 25% of the current speed

            if( (decoderState.decoderStatus.syncStatus!=SyncStatus::Full) || (current.startRevolutions <= page4.StgCycles) )
            {
                decoderState.toothLastToothTime = curTime;
                decoderState.toothLastMinusOneToothTime = curTime - ((MICROS_PER_MIN/10U) / page4.triggerTeeth); //Fixes RPM at 10rpm until a full revolution has taken place
                decoderState.toothCurrentCount = page4.triggerTeeth;
                decoderState.triggerFilterTime = 0; //Need to turn the filter off here otherwise the first primary tooth after achieving sync is ignored

                decoderState.decoderStatus.syncStatus = SyncStatus::Full;
            }
            else 
            {
                if ( (decoderState.toothCurrentCount != page4.triggerTeeth) && (current.startRevolutions > 2)) { current.syncLossCounter++; } //Indicates likely sync loss.
                if (page4.useResync == 1) { decoderState.toothCurrentCount = page4.triggerTeeth; }
            }

            decoderState.revolutionOne = true; //Sequential revolution reset
        }
        else 
        {
            decoderState.triggerSecFilterTime = current.revolutionTime >> 1; //Set filter at 25% of the current cam speed. This needs to be performed here to prevent a situation where the RPM and decoderState.triggerSecFilterTime get out of alignment and decoderState.curGap2 never exceeds the filter value
        } //Trigger filter
    }

    uint32_t getRevolutionTime(const statuses &current, const shared::state_t &decoderState, const config4 &page4)
    {
        if( decoderState.decoderStatus.syncStatus==SyncStatus::Full )
        {
            //Account for cam speed
            if( current.RPM < current.crankRPM )
            {
                return crankingGetRevolutionTime(current, decoderState, page4, page4.triggerTeeth, page4.TrigSpeed==CAM_SPEED);
            }
            else
            {
                return stdGetRevolutionTime(current, decoderState, page4.TrigSpeed==CAM_SPEED);
            }
        }
        return 0U; //Speed is unknown without sync
    }

    int16_t getCrankAngle(uint32_t currMicros, const shared::state_t &decoderState, const config4 &page4)
    {
        auto calculator = decoders::shared::atomic_make_angle_caa(decoderState);

        //Handle case where the secondary tooth was the last one seen
        if(calculator._toothCurrentCount == 0) { calculator._toothCurrentCount = page4.triggerTeeth; }

        return decoders::shared::clampCrankAngle(calculator.calculateCrankAngle(currMicros, page4));
    }

    static uint16_t __attribute__((noinline)) calcEndTooth(const IgnitionSchedule &schedule, uint8_t toothAdder, const shared::state_t &decoderState, const config4 &page4) {
      return decoders::shared::clampToToothCount(page4, decoderState.toothNumFromScheduleAngles(page4, schedule), toothAdder);
    }

    void setEndTeeth(shared::state_t &decoderState, const config4 &page4)
    {
        //The toothAdder variable is used for when a setup is running sequentially, but the primary wheel is running at crank speed. This way the count of teeth will go up to 2* the number of primary teeth to allow for a sequential count. 
        byte toothAdder = 0;
        if( (page4.sparkMode == IGN_MODE_SEQUENTIAL) && (page4.TrigSpeed == CRANK_SPEED) ) { toothAdder = page4.triggerTeeth; }

        decoderState.ignitionEndTeeth[0] = calcEndTooth(ignitionSchedule1, toothAdder, decoderState, page4);
        #if (IGN_CHANNELS >= 2)
        decoderState.ignitionEndTeeth[1] = calcEndTooth(ignitionSchedule2, toothAdder, decoderState, page4);
        #endif
        #if (IGN_CHANNELS >= 3)
        decoderState.ignitionEndTeeth[2] = calcEndTooth(ignitionSchedule3, toothAdder, decoderState, page4);
        #endif
        #if (IGN_CHANNELS >= 4)
        decoderState.ignitionEndTeeth[3] = calcEndTooth(ignitionSchedule4, toothAdder, decoderState, page4);
        #endif
        #if IGN_CHANNELS >= 5
        decoderState.ignitionEndTeeth[4] = calcEndTooth(ignitionSchedule5, toothAdder, decoderState, page4);
        #endif
        #if IGN_CHANNELS >= 6
        decoderState.ignitionEndTeeth[5] = calcEndTooth(ignitionSchedule6, toothAdder, decoderState, page4);
        #endif
        #if IGN_CHANNELS >= 7
        decoderState.ignitionEndTeeth[6] = calcEndTooth(ignitionSchedule7, toothAdder, decoderState, page4);
        #endif
        #if IGN_CHANNELS >= 8
        decoderState.ignitionEndTeeth[7] = calcEndTooth(ignitionSchedule8, toothAdder, decoderState, page4);
        #endif
    }

}
}