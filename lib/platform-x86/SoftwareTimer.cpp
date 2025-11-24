//
// Created by Ognjen Galić on 24. 11. 2025..
//

#include "SoftwareTimer.h"

void SoftwareTimer::tick(uint32_t time) {
    this->counter = time;
    if (this->counter != 0 && this->compare < this->counter) {
        this->counter--; // make sure timer does not fire *twice*
        if (this->isr != nullptr) {
            this->isr();
        }
    }
}

void SoftwareTimer::setCompare(uint32_t compare) {
    this->compare = compare;
}

void SoftwareTimer::attachInterrupt(uint32_t pin, void(*isr)()) {
    this->isr = isr;
}
