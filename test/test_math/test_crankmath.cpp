#include "crankMaths.h"
#include "../test_utils.h"

extern uint16_t injectorLimits(uint16_t angle);

static void test_ignitionLimits_within_range(void)
{
    CRANK_ANGLE_MAX_IGN = 360;

    TEST_ASSERT_EQUAL_INT16(0, ignitionLimits(0));
    TEST_ASSERT_EQUAL_INT16(179, ignitionLimits(179));
    TEST_ASSERT_EQUAL_INT16(359, ignitionLimits(-1));
    TEST_ASSERT_EQUAL_INT16(0, ignitionLimits(360));
}

static void test_injectorLimits_uint16_wrap(void)
{
    CRANK_ANGLE_MAX_INJ = 360;

    TEST_ASSERT_EQUAL_UINT16(0U, injectorLimits((uint16_t)360U));
    TEST_ASSERT_EQUAL_UINT16(1U, injectorLimits((uint16_t)361U));
    TEST_ASSERT_EQUAL_UINT16(0U, injectorLimits((uint16_t)720U));
    TEST_ASSERT_EQUAL_UINT16(1U, injectorLimits((uint16_t)1081U));
}

static void test_angleToTimerTicks_matches_uS_conversion(void)
{
    const uint32_t revolutionTime = 3000000UL;
    setAngleConverterRevolutionTime(revolutionTime);

    const uint16_t angle = 25U;
    const uint32_t micros = angleToTimeMicroSecPerDegree(angle);
    const COMPARE_TYPE expectedTicks = uS_TO_TIMER_COMPARE(micros);

    TEST_ASSERT_EQUAL(expectedTicks, angleToTimerTicks(angle));
}

static void test_timeToAngleDegPerMicroSec_inverse_roundtrip(void)
{
    setAngleConverterRevolutionTime(MICROS_PER_MIN/4000);

    const uint16_t angles[] = { 0, 1, 25, 123, 360, 720 };
    for (auto angle : angles)
    {
        const uint32_t time = angleToTimeMicroSecPerDegree(angle);
        const uint16_t recoveredAngle = timeToAngleDegPerMicroSec(time);
        TEST_ASSERT_UINT16_WITHIN(1U, angle, recoveredAngle);
    }
}

static void test_setAngleConverterRevolutionTime_revolution_values(void)
{
    const uint32_t revolutionTime = 1500000UL;
    setAngleConverterRevolutionTime(revolutionTime);

    TEST_ASSERT_UINT32_WITHIN(1U, revolutionTime, angleToTimeMicroSecPerDegree(360));
    TEST_ASSERT_UINT32_WITHIN(1U, revolutionTime * 2UL, angleToTimeMicroSecPerDegree(720));
}

void testCrankMath()
{
  SET_UNITY_FILENAME() {
      RUN_TEST_P(test_ignitionLimits_within_range);
      RUN_TEST_P(test_injectorLimits_uint16_wrap);
      RUN_TEST_P(test_angleToTimerTicks_matches_uS_conversion);
      RUN_TEST_P(test_timeToAngleDegPerMicroSec_inverse_roundtrip);
      RUN_TEST_P(test_setAngleConverterRevolutionTime_revolution_values);
  }
}
