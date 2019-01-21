
#include "globals.h"

#if defined(CORE_AVR)

void initBoard()
{
    
}

uint16_t freeRam()
{
    extern int __heap_start, *__brkval;
    uint16_t v;

    return (uint16_t) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#endif