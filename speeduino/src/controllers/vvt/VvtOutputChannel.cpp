#include "VvtOutputChannel.h"
#include "../../../maths.h"
#include "../../../units.h"

VvtOutputChannel::VvtOutputChannel(uint8_t pinNum)
{
    pin.setPin(pinNum, OUTPUT);
}

void VvtOutputChannel::setTargetDutyFromDuty(uint8_t duty, uint16_t maxPwmDuty)
{
    if(duty == 0)
    {
        //Make sure solenoid is off (0% duty)
        targetDuty = 0;
        pin.setPinLow();
        periodTicks = false;
    }
    else if(duty >= 200 )
    {
        //Make sure solenoid is on (100% duty)
        targetDuty = maxPwmDuty;
        pin.setPinHigh();
        periodTicks = true;
    }
    else
    {
        targetDuty = halfPercentage(duty, maxPwmDuty);
        periodTicks = false;
    }
}