//
// Created by Ognjen Galić on 24. 11. 2025..
//

#ifndef FIRMWARE_SOFTWARETIMER_H
#define FIRMWARE_SOFTWARETIMER_H

#include <stdint.h>

class SoftwareTimer {

    uint32_t counter;
    uint32_t compare;
    void (*isr)(void) = nullptr;

public:
    void tick(uint32_t time);
    void setCompare(uint32_t compare);
    void attachInterrupt(uint32_t pin, void (*isr)(void));

};


#endif //FIRMWARE_SOFTWARETIMER_H