#include "dualWheel.h"

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

}
}