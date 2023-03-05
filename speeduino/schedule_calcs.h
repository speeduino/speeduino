#pragma once

#include <stdint.h>
#include "scheduler.h"

extern int channel1InjDegrees; /**< The number of crank degrees until cylinder 1 is at TDC (This is obviously 0 for virtually ALL engines, but there's some weird ones) */
extern int channel2InjDegrees; /**< The number of crank degrees until cylinder 2 (and 5/6/7/8) is at TDC */
extern int channel3InjDegrees; /**< The number of crank degrees until cylinder 3 (and 5/6/7/8) is at TDC */
extern int channel4InjDegrees; /**< The number of crank degrees until cylinder 4 (and 5/6/7/8) is at TDC */
#if (INJ_CHANNELS >= 5)
extern int channel5InjDegrees; /**< The number of crank degrees until cylinder 5 is at TDC */
#endif
#if (INJ_CHANNELS >= 6)
extern int channel6InjDegrees; /**< The number of crank degrees until cylinder 6 is at TDC */
#endif
#if (INJ_CHANNELS >= 7)
extern int channel7InjDegrees; /**< The number of crank degrees until cylinder 7 is at TDC */
#endif
#if (INJ_CHANNELS >= 8)
extern int channel8InjDegrees; /**< The number of crank degrees until cylinder 8 is at TDC */
#endif

static inline uint16_t __attribute__((always_inline)) calculateInjectorStartAngle(uint16_t PWdivTimerPerDegree, int16_t injChannelDegrees, uint16_t injAngle);

static inline uint32_t __attribute__((always_inline)) calculateInjectorTimeout(const FuelSchedule &schedule, int injectorStartAngle, int crankAngle);

static inline __attribute__((always_inline)) void calculateIgnitionAngle(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance);

// Ignition for rotary.
static inline __attribute__((always_inline)) void calculateIgnitionTrailingRotary(IgnitionSchedule &leading, uint16_t dwellAngle, int rotarySplitDegrees, IgnitionSchedule &trailing);

static inline __attribute__((always_inline)) uint32_t calculateIgnitionTimeout(const IgnitionSchedule &schedule, int crankAngle);

#include "schedule_calcs.hpp"
