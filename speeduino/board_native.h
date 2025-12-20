#pragma once

#if defined(NATIVE_BOARD)
#include <stdint.h>

/*
***********************************************************************************************************
* General
*/
#define COMPARE_TYPE uint16_t
#define FPU_MAX_SIZE 32 //Size of the FPU buffer. 0 means no FPU.
#define TIMER_RESOLUTION 4
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
#define MAX_TIMER_PERIOD 262140UL //The longest period of time (in uS) that the timer can permit (IN this case it is 65535 * 4, as each timer tick is 4uS)
#define uS_TO_TIMER_COMPARE(uS1) ((uS1) >> 2) //Converts a given number of uS into the required number of timer ticks until that time has passed

namespace native_fake{
    struct timer
    {
        bool enabled;
        uint32_t counter;
        uint32_t compare;
    };

    static inline void enableTimer(timer &s) { s.enabled = true; }
    static inline void disableTimer(timer &s) { s.enabled = false; }
}

extern native_fake::timer fuelTimers[8];

static inline void FUEL1_TIMER_ENABLE(void) { enableTimer(fuelTimers[0]); }
static inline void FUEL2_TIMER_ENABLE(void) { enableTimer(fuelTimers[1]); }
static inline void FUEL3_TIMER_ENABLE(void) { enableTimer(fuelTimers[2]); }
static inline void FUEL4_TIMER_ENABLE(void) { enableTimer(fuelTimers[3]); }
static inline void FUEL5_TIMER_ENABLE(void) { enableTimer(fuelTimers[4]); }
static inline void FUEL6_TIMER_ENABLE(void) { enableTimer(fuelTimers[5]); }
static inline void FUEL7_TIMER_ENABLE(void) { enableTimer(fuelTimers[6]); }
static inline void FUEL8_TIMER_ENABLE(void) { enableTimer(fuelTimers[7]); }

static inline void FUEL1_TIMER_DISABLE(void) { disableTimer(fuelTimers[0]); }
static inline void FUEL2_TIMER_DISABLE(void) { disableTimer(fuelTimers[1]); }
static inline void FUEL3_TIMER_DISABLE(void) { disableTimer(fuelTimers[2]); }
static inline void FUEL4_TIMER_DISABLE(void) { disableTimer(fuelTimers[3]); }
static inline void FUEL5_TIMER_DISABLE(void) { disableTimer(fuelTimers[4]); }
static inline void FUEL6_TIMER_DISABLE(void) { disableTimer(fuelTimers[5]); }
static inline void FUEL7_TIMER_DISABLE(void) { disableTimer(fuelTimers[6]); }
static inline void FUEL8_TIMER_DISABLE(void) { disableTimer(fuelTimers[7]); }

#define FUEL1_COUNTER fuelTimers[0].counter
#define FUEL2_COUNTER fuelTimers[1].counter
#define FUEL3_COUNTER fuelTimers[2].counter
#define FUEL4_COUNTER fuelTimers[3].counter
#define FUEL5_COUNTER fuelTimers[4].counter
#define FUEL6_COUNTER fuelTimers[5].counter
#define FUEL7_COUNTER fuelTimers[6].counter
#define FUEL8_COUNTER fuelTimers[7].counter

#define FUEL1_COMPARE fuelTimers[0].compare
#define FUEL2_COMPARE fuelTimers[1].compare
#define FUEL3_COMPARE fuelTimers[2].compare
#define FUEL4_COMPARE fuelTimers[3].compare
#define FUEL5_COMPARE fuelTimers[4].compare
#define FUEL6_COMPARE fuelTimers[5].compare
#define FUEL7_COMPARE fuelTimers[6].compare
#define FUEL8_COMPARE fuelTimers[7].compare

extern native_fake::timer ignitionTimers[8];
#define IGN1_COUNTER  ignitionTimers[0].counter
#define IGN2_COUNTER  ignitionTimers[1].counter
#define IGN3_COUNTER  ignitionTimers[2].counter
#define IGN4_COUNTER  ignitionTimers[3].counter
#define IGN5_COUNTER  ignitionTimers[4].counter
#define IGN6_COUNTER  ignitionTimers[5].counter
#define IGN7_COUNTER  ignitionTimers[6].counter
#define IGN8_COUNTER  ignitionTimers[7].counter

#define IGN1_COMPARE ignitionTimers[0].compare
#define IGN2_COMPARE ignitionTimers[1].compare
#define IGN3_COMPARE ignitionTimers[2].compare
#define IGN4_COMPARE ignitionTimers[3].compare
#define IGN5_COMPARE ignitionTimers[4].compare
#define IGN6_COMPARE ignitionTimers[5].compare
#define IGN7_COMPARE ignitionTimers[6].compare
#define IGN8_COMPARE ignitionTimers[7].compare

static inline void IGN1_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[0]); }
static inline void IGN2_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[1]); }
static inline void IGN3_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[2]); }
static inline void IGN4_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[3]); }
static inline void IGN5_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[4]); }
static inline void IGN6_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[5]); }
static inline void IGN7_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[6]); }
static inline void IGN8_TIMER_ENABLE(void)  { enableTimer(ignitionTimers[7]); }

static inline void IGN1_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[0]); }
static inline void IGN2_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[1]); }
static inline void IGN3_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[2]); }
static inline void IGN4_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[3]); }
static inline void IGN5_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[4]); }
static inline void IGN6_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[5]); }
static inline void IGN7_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[6]); }
static inline void IGN8_TIMER_DISABLE(void)  { disableTimer(ignitionTimers[7]); }

/*
***********************************************************************************************************
* Auxiliaries
*/

extern native_fake::timer boostTimer;
#define ENABLE_BOOST_TIMER()  enableTimer(boostTimer)
#define DISABLE_BOOST_TIMER() disableTimer(boostTimer)
#define BOOST_TIMER_COMPARE   boostTimer.compare
#define BOOST_TIMER_COUNTER   boostTimer.counter

extern native_fake::timer vvtTimer;
#define ENABLE_VVT_TIMER()    enableTimer(vvtTimer)
#define DISABLE_VVT_TIMER()   disableTimer(vvtTimer)
#define VVT_TIMER_COMPARE     vvtTimer.compare
#define VVT_TIMER_COUNTER     vvtTimer.counter

extern native_fake::timer fanTimer;
#define ENABLE_FAN_TIMER()  enableTimer(fanTimer)
#define DISABLE_FAN_TIMER() disableTimer(fanTimer)
#define FAN_TIMER_COMPARE     fanTimer.compare
#define FAN_TIMER_COUNTER     fanTimer.counter

/*
***********************************************************************************************************
* Idle
*/
extern native_fake::timer idleTimer;
#define IDLE_TIMER_ENABLE()  enableTimer(idleTimer)
#define IDLE_TIMER_DISABLE() disableTimer(idleTimer)
#define IDLE_COUNTER   idleTimer.counter
#define IDLE_COMPARE   idleTimer.compare

#define ATOMIC() // No atomic operations needed for this platform

#if !defined(max)
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#if !defined(min)
#define min(a,b) ((a)<(b)?(a):(b))  
#endif

/*
***********************************************************************************************************
* CAN / Second serial
*/
#define SECONDARY_SERIAL_T decltype(Serial)

#define RTC_LIB_H <time.h>
#define EEPROM_LIB_H <EEPROM.h>
using eeprom_address_t = uint16_t;


#endif // NATIVE_BOARD