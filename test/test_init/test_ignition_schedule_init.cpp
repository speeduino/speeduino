#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "schedule_calcs.h"

static void assert_ignition_schedules(uint16_t crankAngle, uint16_t expectedOutputs, uint16_t angle[])
{
  char szMsg[32];

  strcpy_P(szMsg, PSTR("CRANK_ANGLE_MAX_IGN"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_IGN, szMsg);
  strcpy_P(szMsg, PSTR("maxIgnOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedOutputs, maxIgnOutputs, szMsg);

  strcpy_P(szMsg, PSTR("channel1IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[0], channel1IgnDegrees, szMsg);
  strcpy_P(szMsg, PSTR("channel2IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[1], channel2IgnDegrees, szMsg);
  strcpy_P(szMsg, PSTR("channel3IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[2], channel3IgnDegrees, szMsg);
  strcpy_P(szMsg, PSTR("channel4IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel4IgnDegrees, szMsg);
#if IGN_CHANNELS>=5
  TEST_ASSERT_EQUAL_MESSAGE(angle[4], channel5IgnDegrees, "channel5IgnDegrees");
#endif
#if IGN_CHANNELS>=6
  TEST_ASSERT_EQUAL_MESSAGE(angle[5], channel6IgnDegrees, "channel6IgnDegrees");
#endif
#if IGN_CHANNELS>=7
  TEST_ASSERT_EQUAL_MESSAGE(angle[6], channel7IgnDegrees, "channel7IgnDegrees");
#endif
#if IGN_CHANNELS>=8
  TEST_ASSERT_EQUAL_MESSAGE(angle[7], channel8IgnDegrees, "channel8IgnDegrees");
#endif 
}

static void test_ignition_schedule_1_cylinder(void)
{
  configPage2.nCylinders = 1;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.divider = 1;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_ignition_schedules(720U, 1U, angle);
  }
}

void testIgnitionScheduleInit()
{
    RUN_TEST(test_ignition_schedule_1_cylinder);
}