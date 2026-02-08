#if defined(NATIVE_BOARD)
#include "board_native.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#include "timers.h"

#define IGNITION_INTERRUPT_NAME(index) CONCAT(CONCAT(ignitionSchedule, index), Interrupt)
#define FUEL_INTERRUPT_NAME(index) CONCAT(CONCAT(fuelSchedule, index), Interrupt)

#define FUEL_INTERRUPT(index) void FUEL_INTERRUPT_NAME(index)(void) {moveToNextState(fuelSchedule ## index);}
#define IGNITION_INTERRUPT(index) void IGNITION_INTERRUPT_NAME(index)(void) {moveToNextState(ignitionSchedule ## index);}

FUEL_INTERRUPT(1)
FUEL_INTERRUPT(2)
FUEL_INTERRUPT(3)
FUEL_INTERRUPT(4)
FUEL_INTERRUPT(5)
FUEL_INTERRUPT(6)
FUEL_INTERRUPT(7)
FUEL_INTERRUPT(8)

IGNITION_INTERRUPT(1)
IGNITION_INTERRUPT(2)
IGNITION_INTERRUPT(3)
IGNITION_INTERRUPT(4)
IGNITION_INTERRUPT(5)
IGNITION_INTERRUPT(6)
IGNITION_INTERRUPT(7)
IGNITION_INTERRUPT(8)

std::array<software_timer_t, INJ_CHANNELS> fuelTimers;
std::array<software_timer_t, IGN_CHANNELS> ignitionTimers;
software_timer_t boostTimer;
software_timer_t vvtTimer;
software_timer_t fanTimer;
software_timer_t idleTimer;
software_timer_t oneMSTimer;

void initBoard(uint32_t /*baudRate*/) {
    idleTimer.setCallback(idleInterrupt);
    fanTimer.setCallback(fanInterrupt);
    boostTimer.setCallback(boostInterrupt);
    vvtTimer.setCallback(vvtInterrupt);
    oneMSTimer.setCallback(oneMSInterval);

    fuelTimers[0].setCallback(FUEL_INTERRUPT_NAME(1));
    fuelTimers[1].setCallback(FUEL_INTERRUPT_NAME(2));
    fuelTimers[2].setCallback(FUEL_INTERRUPT_NAME(3));
    fuelTimers[3].setCallback(FUEL_INTERRUPT_NAME(4));
    fuelTimers[4].setCallback(FUEL_INTERRUPT_NAME(5));
    fuelTimers[5].setCallback(FUEL_INTERRUPT_NAME(6));
    fuelTimers[6].setCallback(FUEL_INTERRUPT_NAME(7));
    fuelTimers[7].setCallback(FUEL_INTERRUPT_NAME(8));
    
    ignitionTimers[0].setCallback(IGNITION_INTERRUPT_NAME(1));
    ignitionTimers[1].setCallback(IGNITION_INTERRUPT_NAME(2));
    ignitionTimers[2].setCallback(IGNITION_INTERRUPT_NAME(3));
    ignitionTimers[3].setCallback(IGNITION_INTERRUPT_NAME(4));
    ignitionTimers[4].setCallback(IGNITION_INTERRUPT_NAME(5));
    ignitionTimers[5].setCallback(IGNITION_INTERRUPT_NAME(6));
    ignitionTimers[6].setCallback(IGNITION_INTERRUPT_NAME(7));
    ignitionTimers[7].setCallback(IGNITION_INTERRUPT_NAME(8));
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