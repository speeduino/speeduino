#pragma once

#include <stdint.h>
#include "scheduler.h"

static inline __attribute__((always_inline)) uint16_t calculateInjectorStartAngle(uint16_t PWdivTimerPerDegree, int16_t injChannelDegrees, uint16_t injAngle);

static inline __attribute__((always_inline)) uint32_t calculateInjectorTimeout(const FuelSchedule &schedule, int16_t injectorStartAngle, int16_t crankAngle);

static inline __attribute__((always_inline)) void calculateIgnitionAngles(IgnitionSchedule &schedule, uint16_t dwellAngle, int8_t advance);

// Ignition for rotary.
static inline __attribute__((always_inline)) void calculateIgnitionTrailingRotary(IgnitionSchedule &leading, uint16_t dwellAngle, int16_t rotarySplitDegrees, IgnitionSchedule &trailing);

static inline __attribute__((always_inline)) uint32_t _calculateIgnitionTimeout(const IgnitionSchedule &schedule, int16_t crankAngle);

#include "schedule_calcs.hpp"
