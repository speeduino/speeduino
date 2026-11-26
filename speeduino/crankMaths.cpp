#include <avr-fast-shift.h>
#include "crankMaths.h"
#include "preprocessor.h"

int16_t CRANK_ANGLE_MAX_IGN = 360;
int16_t CRANK_ANGLE_MAX_INJ = 360; 

typedef uint32_t UQ24X8_t;
static constexpr uint8_t UQ24X8_Shift = 8U;
static constexpr uint8_t microsPerDegree_Shift = UQ24X8_Shift;

static constexpr uint8_t UQ1X15_Shift = 15U;
static constexpr uint8_t degreesPerMicro_Shift = UQ1X15_Shift;

/** @brief The factors used by the angle<->time conversion functions */
static angle_converter_factors_t angleConverterFactors;

angle_converter_factors_t calculateAngleConverterFactors(uint32_t revolutionTime) noexcept {
  angle_converter_factors_t factors = { 0U, 0U };
  if (revolutionTime!=0U)
  {
    factors.microsPerDegree = div360(lshift<microsPerDegree_Shift>(revolutionTime));
    constexpr uint32_t UQ1X15_360 = UINT32_C(360) << degreesPerMicro_Shift;
    factors.degreesPerMicro = (uint16_t)fast_div_closest(UQ1X15_360, revolutionTime);
  }
  return factors;
}

void applyAngleConverterFactors(const angle_converter_factors_t &factors) noexcept {
  angleConverterFactors = factors;
}

void setAngleConverterRevolutionTime(uint32_t revolutionTime) noexcept {
  applyAngleConverterFactors(calculateAngleConverterFactors(revolutionTime));
}

BEGIN_LTO_ALWAYS_INLINE(uint32_t) angleToTime(uint16_t angle) noexcept {
  UQ24X8_t micros = (uint32_t)angle * (uint32_t)angleConverterFactors.microsPerDegree;
  return rshift_round<microsPerDegree_Shift>(micros);
}
END_LTO_INLINE()

BEGIN_LTO_ALWAYS_INLINE(COMPARE_TYPE) angleToTimerTicks(uint16_t angle) noexcept {
    uint32_t micros = angleToTime(angle);
    return uS_TO_TIMER_COMPARE(micros);
}
END_LTO_INLINE()

BEGIN_LTO_ALWAYS_INLINE(uint16_t) timeToAngle(uint32_t time) noexcept {
    uint32_t degFixed = time * (uint32_t)angleConverterFactors.degreesPerMicro;
    return rshift_round<degreesPerMicro_Shift>(degFixed);
}
END_LTO_INLINE()