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

    state.triggerToothAngle = 0; //The number of degrees that passes from tooth to tooth (primary)
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

}
}