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

uint32_t spark_random = 100;

void tickTimersX86(uint32_t time) {
    Timer1.tick(time);
    Timer2.tick(time);
}

void triggerTimerIsr() {
    if (trigger_isr != NULL) {
        trigger_isr();
        Timer2.setCompare(millis() + spark_random--);
        if (spark_random < 15) {
            spark_random = 100;
        }
    }
}

void tick1msTimer() {
    oneMSInterval();
    Timer1.setCompare(millis() + 1);
}

void initBoard(uint32_t baudRate) {
    printf("native-x86: board init, %d baud\n", baudRate);
    Serial.begin(baudRate);

    Timer1.attachInterrupt(1, tick1msTimer);
    Timer1.setCompare(millis());

    Timer2.attachInterrupt(1, triggerTimerIsr);
    Timer2.setCompare(millis());

}

uint16_t freeRam() {
    printf("native-x86: freeRam\n");
    return 0;
}

void boardInitPins() {
    printf("native-x86: boardInitPins\n");
}

void doSystemReset() {
    printf("native-x86: doSystemReset\n");
}
void jumpToBootloader() {
    printf("native-x86: jumpToBootloader\n");
}

void * memcpy_P(void * arg, const void * arg2, size_t size) {
    return memcpy(arg, arg2, size);
}

uint8_t getSystemTemp() {
    printf("native-x86: getSystemTemp()");
    return 0;
}

#endif