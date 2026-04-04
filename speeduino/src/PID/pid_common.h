#pragma once

#include <stdint.h>

enum class PidDirection : uint8_t
{
    Direct = 0,  ///< The output will increase when error is positive. 
    Reverse = 1, ///< The output will decrease when error is positive. 
};
