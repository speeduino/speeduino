#if defined(NATIVE_BOARD)
#include "board_native.h"

native_fake::timer fuelTimers[8];
native_fake::timer ignitionTimers[8];
native_fake::timer boostTimer;
native_fake::timer vvtTimer;
native_fake::timer fanTimer;
native_fake::timer idleTimer;

void initBoard(uint32_t /*baudRate*/) {
    // Nothing to do here for the native platform
}
uint16_t freeRam() {
    return UINT16_MAX; 
}
void doSystemReset() { 
    // Not implemented on this platform yet 
}
void jumpToBootloader() {
    // Not implemented on this platform yet 
}
uint8_t getSystemTemp() { 
    return 0; 
}

uint16_t makeWord(uint16_t w) {
    return w;
}
uint16_t makeWord(uint8_t h, uint8_t l) {
    return (h << 8) | l;
}

void boardInitRTC(void)
{
  // Do nothing
}

void boardInitPins(void)
{
  // Do nothing
}

#endif