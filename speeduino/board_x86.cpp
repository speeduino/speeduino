//
// Created by Ognjen Galić on 23. 11. 2025..
//

#include <cstring>
#include "board_x86.h"

COMPARE_TYPE dummy_register;

void initBoard(uint32_t baudRate) {
    printf("native-x86: board init, %d baud\n", baudRate);
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
