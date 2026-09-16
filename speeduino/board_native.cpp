#include "board_definition.h"

#if defined(NATIVE_BOARD)
#include <EEPROM.h>
#include "board_native.h"
#include "src/controllers/vvt/vvtController.h"
#include "src/controllers/idle/idle.h"
#include "timers.h"
#include "board_eeprom_adapter.hpp"
#include "scheduler_ignition_controller.h"
#include "scheduler_fuel_controller.h"
#include "src/controllers/fan/fanController.h"
#include "src/controllers/boost/boostController.h"

template <uint8_t index>
void fuelScheduleIsr(void) {
    if (index<_countof(fuelSchedules)) 
    {
        moveToNextState(fuelSchedules[index]);
    }
}

template <uint8_t index>
void ignitionScheduleIsr(void) {
    if (index<_countof(ignitionSchedules)) 
    {
        moveToNextState(ignitionSchedules[index]);
    }
}

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

    fuelTimers[0].setCallback(fuelScheduleIsr<0>);
#if INJ_CHANNELS>=2
    fuelTimers[1].setCallback(fuelScheduleIsr<1>);
#endif
#if INJ_CHANNELS>=3
    fuelTimers[2].setCallback(fuelScheduleIsr<2>);
#endif
#if INJ_CHANNELS>=4
    fuelTimers[3].setCallback(fuelScheduleIsr<3>);
#endif
#if INJ_CHANNELS>=5
    fuelTimers[4].setCallback(fuelScheduleIsr<4>);
#endif
#if INJ_CHANNELS>=6
    fuelTimers[5].setCallback(fuelScheduleIsr<5>);
#endif
#if INJ_CHANNELS>=7
    fuelTimers[6].setCallback(fuelScheduleIsr<6>);
#endif
#if INJ_CHANNELS>=8
    fuelTimers[7].setCallback(fuelScheduleIsr<7>);
#endif
    
    ignitionTimers[0].setCallback(ignitionScheduleIsr<0>);
#if IGN_CHANNELS>=2
    ignitionTimers[1].setCallback(ignitionScheduleIsr<1>);
#endif
#if IGN_CHANNELS>=3
    ignitionTimers[2].setCallback(ignitionScheduleIsr<2>);
#endif
#if IGN_CHANNELS>=4
    ignitionTimers[3].setCallback(ignitionScheduleIsr<3>);
#endif
#if IGN_CHANNELS>=5
    ignitionTimers[4].setCallback(ignitionScheduleIsr<4>);
#endif
#if IGN_CHANNELS>=6
    ignitionTimers[5].setCallback(ignitionScheduleIsr<5>);
#endif
#if IGN_CHANNELS>=7
    ignitionTimers[6].setCallback(ignitionScheduleIsr<6>);
#endif
#if IGN_CHANNELS>=8
    ignitionTimers[7].setCallback(ignitionScheduleIsr<7>);
#endif
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

void boardInitPins(uint8_t, pinNumbers_t &)
{
  // Do nothing
}


static uint16_t getEepromWriteBlockSize(const statuses &)
{
  return 64U;
}

/** @brief Get the EEPROM storage API for the board */
storage_api_t getBoardStorageApi(void)
{
  return getEEPROMStorageApi(getEepromWriteBlockSize);
}

/** @brief Get the PWM timer resolution in uS */
uint8_t getPwmTimerResolution(void)
{
  return 2;
}

#endif