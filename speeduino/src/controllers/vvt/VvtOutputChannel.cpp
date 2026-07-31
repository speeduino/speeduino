#include "VvtOutputChannel.h"
#include "../../../maths.h"
#include "../../../units.h"

VvtOutputChannel::VvtOutputChannel(uint8_t pinNum, uint16_t motorFrequency)
: maxDuty(pwmFreqToTicks(motorFrequency))
{
    pin.setPin(pinNum, OUTPUT);
}

void VvtOutputChannel::setTargetDutyFromDuty(uint8_t duty)
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
        targetDuty = maxDuty;
        pin.setPinHigh();
        periodTicks = true;
    }
    else
    {
        targetDuty = halfPercentage(duty, maxDuty);
        periodTicks = false;
    }
}