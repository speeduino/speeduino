//
// Created by Ognjen Galić on 23. 11. 2025..
//

#ifdef PLATFORM_X86

#include <cstring>
#include <SoftwareTimer.h>
#include <timers.h>
#include "board_x86.h"

COMPARE_TYPE dummy_register;

SoftwareTimer Timer1;
SoftwareTimer Timer2;

uint64_t spark_random = 100000;

void tickTimersX86(uint64_t time) {
    Timer1.tick(time);
    Timer2.tick(time);
}

void triggerTimerIsr() {
    if (trigger_isr != NULL) {
        trigger_isr();
        Timer2.compare = micros() + (spark_random = spark_random - 1000);
        if (spark_random < 15000) {
            spark_random = 100000;
        }
    }
}

void tick1msTimer() {
    oneMSInterval();
    Timer1.compare = micros() + 1000000;
}

void initBoard(uint32_t baudRate) {
    log(X86BRD, "native-x86: board init, %d baud\n", baudRate);
    Serial.begin(baudRate);

    Timer1.attachInterrupt(1, tick1msTimer);
    Timer1.compare = micros();

    Timer2.attachInterrupt(1, triggerTimerIsr);
    Timer2.compare = micros();

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