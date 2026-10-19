#include "idleController_state.h"
#include "units.h"
#include "globals.h"

namespace idleController {

namespace detail {

    __attribute__((optimize("Os"))) state_t::state_t(const pinNumbers_t &pins, const config2 &page2, const config6 &page6)
    : idleStepper()
    , idle_pwm_max_count(pwmFreqToTicks(FREQUENCY.toUser(page6.idleFreq)))
    {
        idle_pin.setPin(pins.pinIdle1, OUTPUT);
        idle_pin.setInverted(page6.iacPWMdir && isPwmIac(page6));
        if (page6.iacChannels>0)
        {
            idle2_pin.setPin(pins.pinIdle2, OUTPUT);
            idle2_pin.setInverted(page6.iacPWMdir && isPwmIac(page6));
        }
  
        if( (page2.idleUpEnabled) && (!pinIsOutput(pins.pinIdleUp)) )
        {
            pinIdleUpInput.setPin(pins.pinIdleUp, page2.idleUpPolarity ? INPUT : INPUT_PULLUP); //Normal setting
            
            if (page2.idleUpOutputEnabled)
            {
                pinIdleUpOutput.setPin(pins.pinIdleUpOutput, OUTPUT);
                pinIdleUpOutput.setInverted(page2.idleUpOutputInv);
                pinIdleUpOutput.setPinLow(); //Initialise program with the idle up output in the off state if it is enabled. 
            }
        }
    }
}
}
