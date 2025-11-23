//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <cstring>
#include "board_x86.h"

COMPARE_TYPE dummy_register;

void initBoard(uint32_t baudRate) {
    printf("native-x86: board init, %d baud", baudRate);
}

uint16_t freeRam() {
    return 0;
}

void boardInitPins() {

}

void doSystemReset() { return; }
void jumpToBootloader() { return; }

void * memcpy_P(void * arg, const void * arg2, size_t size) {
    return memcpy(arg, arg2, size);
}

uint8_t getSystemTemp() {
    return 0;
}
