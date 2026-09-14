#include "PwmOutputChannel.h"
#include "../../maths.h"
#include "../../units.h"

PwmOutputChannel::PwmOutputChannel(uint8_t pinNum, uint16_t fullDuty)
: maxDuty(pwmFreqToTicks(fullDuty))
{
    pin.setPin(pinNum, OUTPUT);
}

void PwmOutputChannel::setTargetDuty(uint8_t duty) noexcept
{
    if(duty == 0)
    {
        //Make sure solenoid is off (0% duty)
        targetDuty = 0;
        pin.setPinLow();
    }
    else if(duty >= 200 )
    {
        //Make sure solenoid is on (100% duty)
        targetDuty = maxDuty;
        pin.setPinHigh();
    }
    else
    {
        targetDuty = halfPercentage(duty, maxDuty);
    }
}
