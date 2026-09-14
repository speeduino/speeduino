#include <stdint.h>
#include "../pins/trackedOutputPin.h"

template <class TPin>
struct PwmOutputChannel {
    uint16_t maxDuty = 0;        ///< Maximum duty based on motor frequency
    uint16_t targetDuty = 0;     ///< Requested duty cycle (0-100% or 0-255)
    trackedOutputPin_t<TPin> pin; ///< The pin

    /** @brief Default construction - not usable yet */
    PwmOutputChannel() = default;

    /**
     * @brief Construct a new Pwm Output Channel object
     * 
     * @param pinNum The pin controlling the PWM motor
     * @param fullDuty The frequency in Hz at 100% duty
     */
    PwmOutputChannel(uint8_t pinNum, uint16_t fullDuty)
    : maxDuty(pwmFreqToTicks(fullDuty))
    {
        pin.setPin(pinNum, OUTPUT);
    }

    /**
     * @brief Set the target PWM duty
     * 
     * @param duty Target duty in %*2. I.e. 0 to 200
     */
    void setTargetDuty(uint8_t duty) noexcept
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

    
    bool isNoDuty(void) const
    {
        return targetDuty==0U;
    }

    bool isPartialDuty(void) const
    {
        return !isNoDuty() && !isFullDuty();
    }

    bool isFullDuty(void) const
    {
        return targetDuty==maxDuty;
    }
};
