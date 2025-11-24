//
// Created by Ognjen Galić on 24. 11. 2025..
//

#ifndef FIRMWARE_SOFTWARETIMER_H
#define FIRMWARE_SOFTWARETIMER_H

#include <stdint.h>

class SoftwareTimer {

    void (*isr)(void) = nullptr;

public:

    uint64_t counter;
    uint64_t compare;
    bool enabled = false;

    void tick(uint64_t time);
    void attachInterrupt(uint64_t pin, void (*isr)(void));

};


#endif //FIRMWARE_SOFTWARETIMER_H