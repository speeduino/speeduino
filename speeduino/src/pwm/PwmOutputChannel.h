#include <stdint.h>
#include "../pins/boardOutputPin.h"
#include "../pins/trackedOutputPin.h"

struct PwmOutputChannel {
    uint16_t maxDuty = 0;        ///< Maximum duty based on motor frequency
    uint16_t targetDuty = 0;     ///< Requested duty cycle (0-100% or 0-255)
    trackedOutputPin_t<boardOutputPin_t> pin; ///< The pin

    /** @brief Default construction - not usable yet */
    PwmOutputChannel() = default;

    /**
     * @brief Construct a new Pwm Output Channel object
     * 
     * @param pinNum The pin controlling the PWM motor
     * @param fullDuty The frequency in Hz at 100% duty
     */
    PwmOutputChannel(uint8_t pinNum, uint16_t fullDuty);

    /**
     * @brief Set the target PWM duty
     * 
     * @param duty Target duty in %*2. I.e. 0 to 200
     */
    void setTargetDuty(uint8_t duty) noexcept;
    
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
