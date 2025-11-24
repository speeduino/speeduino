//
// Created by Ognjen Galić on 24. 11. 2025..
//

#ifndef FIRMWARE_LOG_H
#define FIRMWARE_LOG_H

#include <stdint.h>

#define ARDUINO_CORE        0b00000001
#define HARDWARE_SERIAL     0b00000010
#define PORT                0b00000100
#define ATOM                0b00001000
#define SPIDBG              0b00010000
#define TIMER               0b00100000
#define X86BRD              0b01000000

extern  uint8_t loglevel;
extern void log(uint8_t tag, const char * __restrict, ...);

#endif //FIRMWARE_LOG_H