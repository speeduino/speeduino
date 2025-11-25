//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifdef PLATFORM_X86

#include <cstring>
#include <SoftwareTimer.h>
#include <timers.h>
#include "board_x86.h"

#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"

COMPARE_TYPE dummy_register;

SoftwareTimer Timer1;
SoftwareTimer Timer2;

SoftwareTimer Fuel1;
SoftwareTimer Fuel2;
SoftwareTimer Fuel3;
SoftwareTimer Fuel4;
SoftwareTimer Fuel5;
SoftwareTimer Fuel6;
SoftwareTimer Fuel7;
SoftwareTimer Fuel8;

SoftwareTimer Ignition1;
SoftwareTimer Ignition2;
SoftwareTimer Ignition3;
SoftwareTimer Ignition4;
SoftwareTimer Ignition5;
SoftwareTimer Ignition6;
SoftwareTimer Ignition7;
SoftwareTimer Ignition8;

SoftwareTimer Boost1;
SoftwareTimer Idle1;
SoftwareTimer VVT1;

void tickTimersX86(uint64_t time) {
    Timer1.tick(time);
    Timer2.tick(time);

    if (Fuel1.enabled) {
        Fuel1.tick(time);
    }
    if (Fuel2.enabled) {
        Fuel2.tick(time);
    }
    if (Fuel3.enabled) {
        Fuel3.tick(time);
    }
    if (Fuel4.enabled) {
        Fuel4.tick(time);
    }
    if (Fuel5.enabled) {
        Fuel5.tick(time);
    }
    if (Fuel6.enabled) {
        Fuel6.tick(time);
    }
    if (Fuel7.enabled) {
        Fuel7.tick(time);
    }
    if (Fuel8.enabled) {
        Fuel8.tick(time);
    }

    if (Ignition1.enabled) {
        Ignition1.tick(time);
    }
    if (Ignition2.enabled) {
        Ignition2.tick(time);
    }
    if (Ignition3.enabled) {
        Ignition3.tick(time);
    }
    if (Ignition4.enabled) {
        Ignition4.tick(time);
    }
    if (Ignition5.enabled) {
        Ignition5.tick(time);
    }
    if (Ignition6.enabled) {
        Ignition6.tick(time);
    }
    if (Ignition7.enabled) {
        Ignition7.tick(time);
    }
    if (Ignition8.enabled) {
        Ignition8.tick(time);
    }
    if (Boost1.enabled) {
        Boost1.tick(time);
    }
    if (VVT1.enabled) {
        VVT1.tick(time);
    }
    if (Idle1.enabled) {
        Idle1.tick(time);
    }
}

void triggerTimerIsr() {
    if (trigger_isr != NULL) {
        trigger_isr();
        Timer2.compare = micros() + 32000;
    }
}

void tick1msTimer() {
    oneMSInterval();
    Timer1.compare = micros() + 1000;
}

void initBoard(uint32_t baudRate) {
    log(X86BRD, "native-x86: board init, %d baud\n", baudRate);
    Serial.begin(baudRate);

    Timer1.attachInterrupt(1, tick1msTimer);
    Timer1.compare = micros();

    Timer2.attachInterrupt(1, triggerTimerIsr);
    Timer2.compare = micros();

    Fuel1.attachInterrupt(0, fuelSchedule1Interrupt);
    Fuel2.attachInterrupt(0, fuelSchedule2Interrupt);
    Fuel3.attachInterrupt(0, fuelSchedule3Interrupt);
    Fuel4.attachInterrupt(0, fuelSchedule4Interrupt);
    Fuel5.attachInterrupt(0, fuelSchedule5Interrupt);
    Fuel6.attachInterrupt(0, fuelSchedule6Interrupt);
    Fuel7.attachInterrupt(0, fuelSchedule7Interrupt);
    Fuel8.attachInterrupt(0, fuelSchedule8Interrupt);

    Ignition1.attachInterrupt(0, ignitionSchedule1Interrupt);
    Ignition2.attachInterrupt(0, ignitionSchedule2Interrupt);
    Ignition3.attachInterrupt(0, ignitionSchedule3Interrupt);
    Ignition4.attachInterrupt(0, ignitionSchedule4Interrupt);
    Ignition5.attachInterrupt(0, ignitionSchedule5Interrupt);
    Ignition6.attachInterrupt(0, ignitionSchedule6Interrupt);
    Ignition7.attachInterrupt(0, ignitionSchedule7Interrupt);
    Ignition8.attachInterrupt(0, ignitionSchedule8Interrupt);

    Boost1.attachInterrupt(0, boostInterrupt);
    VVT1.attachInterrupt(0, vvtInterrupt);
    Idle1.attachInterrupt(0, idleInterrupt);
}

uint16_t freeRam() {
    log(X86BRD, "native-x86: freeRam\n");
    return 0;
}

void boardInitPins() {
    log(X86BRD, "native-x86: boardInitPins\n");
}

void doSystemReset() {
    log(X86BRD, "native-x86: doSystemReset\n");
}
void jumpToBootloader() {
    log(X86BRD, "native-x86: jumpToBootloader\n");
}

void * memcpy_P(void * arg, const void * arg2, size_t size) {
    return memcpy(arg, arg2, size);
}

uint8_t getSystemTemp() {
    log(X86BRD, "native-x86: getSystemTemp()");
    return 0;
}

#endif