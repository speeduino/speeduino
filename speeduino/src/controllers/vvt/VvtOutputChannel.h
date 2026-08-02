#include <stdint.h>
#include "../../pins/boardOutputPin.h"
#include "../../pins/trackedOutputPin.h"

struct VvtOutputChannel {
    uint16_t maxDuty = 0;        ///< Maximum duty based on motor frequency
    uint16_t targetDuty = 0;     ///< Requested duty cycle (0-100% or 0-255)
    trackedOutputPin_t<boardOutputPin_t> pin; ///< The pin

    VvtOutputChannel() = default;
    VvtOutputChannel(uint8_t pinNum, uint16_t motorFrequency);

    void setTargetDutyFromDuty(uint8_t duty) noexcept;
    
    void toggleOn(void) noexcept;
    void toggleOff(void) noexcept;

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

enum class NextInterruptEvent : uint8_t
{
    VVT1,
    VVT2,
    Both
};