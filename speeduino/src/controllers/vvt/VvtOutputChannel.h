#include <stdint.h>
#include "../../pins/boardOutputPin.h"
#include "../../pins/trackedOutputPin.h"

struct VvtOutputChannel {
    uint16_t maxDuty = 0;        ///< Maximum duty based on motor frequency
    uint16_t targetDuty = 0;     ///< Requested duty cycle (0-100% or 0-255)
    uint16_t compareTicks = 0;   ///< Active compare threshold for ISR match
    bool     periodTicks = false;    // Total clock ticks per PWM period (frequency ceiling)
    trackedOutputPin_t<boardOutputPin_t> pin; ///< The pin

    VvtOutputChannel() = default;
    VvtOutputChannel(uint8_t pinNum, uint16_t motorFrequency);

    void setTargetDutyFromDuty(uint8_t duty) noexcept;

    bool isOff(void) const
    {
        return targetDuty==0U;
    }

    bool isOnPartial(void) const
    {
        return !isOff() && !isOnFull();
    }

    bool isOnFull(void) const
    {
        return targetDuty==maxDuty;
    }
};

enum class NextInterruptEvent : uint8_t
{
    VVT1,
    VVT2,
    Both
};