#pragma once

#if defined(NATIVE_BOARD)
#include <stdint.h>
#include <array>
#include "../lib/ArduinoFake/SoftwareTimer.h"

/*
***********************************************************************************************************
* General
*/
using COMPARE_TYPE = software_timer_t::counter_t;
#define FPU_MAX_SIZE 32 //Size of the FPU buffer. 0 means no FPU.
constexpr uint16_t BLOCKING_FACTOR = 121;
constexpr uint16_t TABLE_BLOCKING_FACTOR = 64;
#define IGN_CHANNELS 8
#define INJ_CHANNELS 8
#define TS_SERIAL_BUFFER_SIZE 517 //Size of the serial buffer used by new comms protocol. For SD transfers this must be at least 512 + 1 (flag) + 4 (sector)

#define BOARD_MAX_DIGITAL_PINS NUM_DIGITAL_PINS
#define BOARD_MAX_IO_PINS NUM_DIGITAL_PINS

static inline bool pinIsReserved(uint8_t pin) { return pin==0U; } //Forbidden pins like USB on other boards

#if false
static constexpr uint8_t A0 = 0U;
static constexpr uint8_t A1 = 0U;
static constexpr uint8_t A2 = 0U;
static constexpr uint8_t A3 = 0U;
static constexpr uint8_t A5 = 0U;
static constexpr uint8_t A6 = 0U;
static constexpr uint8_t A7 = 0U;
#endif
static constexpr uint8_t A8 = 0U;
static constexpr uint8_t A9 = 0U;
static constexpr uint8_t A10 = 0U;
static constexpr uint8_t A11 = 0U;
static constexpr uint8_t A12 = 0U;
static constexpr uint8_t A13 = 0U;
static constexpr uint8_t A14 = 0U;
static constexpr uint8_t A15 = 0U;

#define PWM_FAN_AVAILABLE

/*
***********************************************************************************************************
* Schedules
*/
#define MAX_TIMER_PERIOD (UINT32_MAX/1000UL) //The longest period of time (in uS) that the timer can permit
#define uS_TO_TIMER_COMPARE(uS1) ((uS1) / 1000U) //Converts a given number of uS into the required number of timer ticks until that time has passed

#define DEFINE_TIMER_VARS(Prefix, index, array) \
    static inline void Prefix##index##_TIMER_ENABLE(void) { array[index-1].enableTimer(); } \
    static inline void Prefix##index##_TIMER_DISABLE(void) { array[index-1].disableTimer(); } \
    static std::atomic<software_timer_t::counter_t>& Prefix##index##_COUNTER = array[index-1].counter; \
    static std::atomic<software_timer_t::counter_t>& Prefix##index##_COMPARE = array[index-1].compare;

extern std::array<software_timer_t, INJ_CHANNELS> fuelTimers;

DEFINE_TIMER_VARS(FUEL, 1, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 2, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 3, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 4, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 5, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 6, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 7, fuelTimers)
DEFINE_TIMER_VARS(FUEL, 8, fuelTimers)

extern std::array<software_timer_t, IGN_CHANNELS> ignitionTimers;

DEFINE_TIMER_VARS(IGN, 1, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 2, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 3, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 4, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 5, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 6, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 7, ignitionTimers)
DEFINE_TIMER_VARS(IGN, 8, ignitionTimers)

/*
***********************************************************************************************************
* Auxiliaries
*/

extern software_timer_t boostTimer;
#define ENABLE_BOOST_TIMER()  boostTimer.enableTimer()
#define DISABLE_BOOST_TIMER() boostTimer.disableTimer()
#define BOOST_TIMER_COMPARE   boostTimer.compare
#define BOOST_TIMER_COUNTER   boostTimer.counter

extern software_timer_t vvtTimer;
#define ENABLE_VVT_TIMER()    vvtTimer.enableTimer()
#define DISABLE_VVT_TIMER()   vvtTimer.disableTimer()
#define VVT_TIMER_COMPARE     vvtTimer.compare
#define VVT_TIMER_COUNTER     vvtTimer.counter

extern software_timer_t fanTimer;
#define ENABLE_FAN_TIMER()  fanTimer.enableTimer()
#define DISABLE_FAN_TIMER() fanTimer.disableTimer()
#define FAN_TIMER_COMPARE     fanTimer.compare
#define FAN_TIMER_COUNTER     fanTimer.counter

/*
***********************************************************************************************************
* Idle
*/
extern software_timer_t idleTimer;
#define IDLE_TIMER_ENABLE()  idleTimer.enableTimer()
#define IDLE_TIMER_DISABLE() idleTimer.disableTimer()
#define IDLE_COUNTER   idleTimer.counter
#define IDLE_COMPARE   idleTimer.compare

#define ATOMIC() // No atomic operations needed for this platform

#if !defined(max)
template<typename _Tp>
constexpr const _Tp& max(const _Tp& __a, const _Tp& __b) {
    if (__b > __a) {
        return __b;
    }
    return __a;
}
#endif
#if !defined(min)
template<typename _Tp>
constexpr const _Tp& min(const _Tp& __a, const _Tp& __b) {
    if (__b < __a) {
        return __b;
    }
    return __a;
}
#endif

/*
***********************************************************************************************************
* CAN / Second serial
*/
#define SECONDARY_SERIAL_T decltype(Serial)

#define RTC_LIB_H <time.h>
#define EEPROM_LIB_H <EEPROM.h>
using eeprom_address_t = uint16_t;
class EEPROMClass;
using EEPROM_t = EEPROMClass;

#endif // NATIVE_BOARD