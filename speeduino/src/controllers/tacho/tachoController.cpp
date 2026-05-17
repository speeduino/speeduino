#include "tachoController.h"
#include "board_definition.h"
#include "../../../board_definition.h"
#include "../../pins/fastOutputPin.h"
#include "../../pins/outputPin.h"
#include "../../../maths.h"
#include "../../../unit_testing.h"

namespace tachoControl_detail
{
    tacho_control_state::tacho_control_state(void)
    : tachoSweepEnabled(false)
    , tachoAlt(false)
    , tachoHalf(false)
    , modeDwell(false)
    {
    }

  TESTABLE_STATIC boardOutputPin_t tach_pin;
  TESTABLE_STATIC tacho_control_state state;
}

using namespace tachoControl_detail;

TESTABLE_CONSTEXPR uint16_t TACHO_SWEEP_TIME_MS = 1500;
TESTABLE_CONSTEXPR uint16_t TACHO_SWEEP_RAMP_MS = TACHO_SWEEP_TIME_MS * 2 / 3;

void __attribute__((optimize("Os"))) initialiseTachoControl(uint8_t tachoPin, const config2 &page2, const config6 &page6, const statuses &current)
{
    state = tacho_control_state();
    tach_pin.setPin(tachoPin, OUTPUT);
    //Set the tacho output default state
    tach_pin.setPinHigh();

    state = tachoControl_detail::tacho_control_state();
    state.tachoSweepEnabled = (page2.useTachoSweep != 0);
    /* SweepMax is stored as a byte, RPM/100. divide by 60 to convert min to sec (net 5/3).  Multiply by ignition pulses per rev.
        tachoSweepIncr is also the number of tach pulses per second */
    state.tachoSweepIncr = (page2.tachoSweepMaxRPM * current.maxIgnOutputs * 5) / 3;
    state.tachoHalf = page2.tachoDiv == 0;
    state.tachoDuration = page2.tachoDuration;
    state.modeDwell = page6.tachoMode;
}

static void tachoSweep(const statuses &current, tachoControl_detail::tacho_control_state &tachoState)
{
  // See if we're in power-on sweep mode
  tachoState.tachoSweepEnabled =   tachoState.tachoSweepEnabled 
                                && (current.rotationStatus==EngineRotationStatus::Stopped)
                                && (tachoState.controlCounter<TACHO_SWEEP_TIME_MS)
                                ;

  if( tachoState.tachoSweepEnabled )
  {
    // Ramp the needle smoothly to the max over the SWEEP_RAMP time
    tachoState.tachoSweepAccum += clamp((uint16_t)map(tachoState.controlCounter, 0, TACHO_SWEEP_RAMP_MS, 0, tachoState.tachoSweepIncr),
                                        (uint16_t)0U, 
                                        tachoState.tachoSweepIncr);
            
    // Each time it rolls over, it's time to pulse the Tach
    if( tachoState.tachoSweepAccum >= MILLI_PER_SEC ) 
    {  
        tachoState.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::READY;
        tachoState.tachoSweepAccum -= MILLI_PER_SEC;
    }
  }
}

static void tachoOutput(tachoControl_detail::tacho_control_state &tachoState)
{
  //Tacho output check. This code will not do anything if tacho pulse duration is fixed to coil dwell.
  if(tachoState.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::READY)
  {
    //Check for half speed tacho
    if( (tachoState.tachoHalf) || (tachoState.tachoAlt) ) 
    { 
      tach_pin.setPinLow();
      //controlCounter is cast down to a byte as the tacho duration can only be in the range of 1-6, so no extra resolution above that is required
      tachoState.tachoEndTime = (uint8_t)tachoState.controlCounter + tachoState.tachoDuration;
      tachoState.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::ACTIVE;
    }
    else
    {
      //Don't run on this pulse (Half speed tacho)
      tachoState.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::INACTIVE;
    }
    tachoState.tachoAlt = !tachoState.tachoAlt; //Flip the alternating value in case half speed tacho is in use. 
  }
  else if(tachoState.tachoOutputFlag == tachoControl_detail::TachoOutputStatus::ACTIVE)
  {
    //If the tacho output is already active, check whether it's reached it's end time
    if((uint8_t)tachoState.controlCounter == tachoState.tachoEndTime)
    {
      tach_pin.setPinHigh();
      tachoState.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::INACTIVE;
    }
  }
}

void tachoControl(const statuses &current)
{
    // Tacho controller operates every millisecond (approx)
    // if (BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_1KHZ))
    {
        ++state.controlCounter;
        tachoSweep(current, state);
        tachoOutput(state);
    }
}

void tachoOutputOn(void) { if(state.modeDwell) { tach_pin.setPinLow(); } else { state.tachoOutputFlag = tachoControl_detail::TachoOutputStatus::READY; } }
void tachoOutputOff(void) { if(state.modeDwell) { tach_pin.setPinHigh(); } }
