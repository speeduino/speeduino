#include <stdint.h>
#include "../../pins/boardOutputPin.h"
#include "../../pins/trackedOutputPin.h"

struct VvtOutputChannel {
    uint16_t targetDuty = 0;     // Requested duty cycle (0-100% or 0-255)
    uint16_t compareTicks = 0;   // Active compare threshold for ISR match
    bool     periodTicks = false;    // Total clock ticks per PWM period (frequency ceiling)
    trackedOutputPin_t<boardOutputPin_t> pin;
};