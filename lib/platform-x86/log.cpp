//
// Created by Ognjen Galić on 24. 11. 2025..
//
#include "log.h"

#include <stdio.h>
#include <stdarg.h>

uint8_t loglevel =
  //  ARDUINO_CORE |
    HARDWARE_SERIAL |
    PORT |
    ATOM |
    SPIDBG |
    TIMER |
    X86BRD;

void log(uint8_t tag, const char * __restrict format, ...) {
    if (!(loglevel & tag)) {
        return;
    }
    va_list arglist;
    va_start(arglist, format);
    vprintf(format, arglist);
    va_end(arglist);
}
