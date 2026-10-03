#pragma once

#include "src/pins/inputPin.h"
#include "src/pins/outputPin.h"

namespace nitrous
{
namespace detail
{
    struct state_t
    {
        inputPin_t armingPin;
        outputPin_t stage1Pin;
        outputPin_t stage2Pin;
    };
}
}