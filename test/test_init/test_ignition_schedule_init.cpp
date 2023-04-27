#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "schedule_calcs.h"

static void assert_ignition_schedules(uint16_t crankAngle, uint16_t expectedOutputs, uint16_t angle[])
{
  char msg[32];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_IGN"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_IGN, msg);
  strcpy_P(msg, PSTR("maxIgnOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedOutputs, maxIgnOutputs, msg);

  strcpy_P(msg, PSTR("channel1IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[0], channel1IgnDegrees, msg);
  strcpy_P(msg, PSTR("channel2IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[1], channel2IgnDegrees, msg);
  strcpy_P(msg, PSTR("channel3IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[2], channel3IgnDegrees, msg);
  strcpy_P(msg, PSTR("channel4IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel4IgnDegrees, msg);
#if IGN_CHANNELS>=5
  strcpy_P(msg, PSTR("channel5IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel5IgnDegrees, msg);
#endif
#if IGN_CHANNELS>=6
  strcpy_P(msg, PSTR("channel6IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel6IgnDegrees, msg);
#endif
#if IGN_CHANNELS>=7
  strcpy_P(msg, PSTR("channel7IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel7IgnDegrees, msg);
#endif
#if IGN_CHANNELS>=8
  strcpy_P(msg, PSTR("channel8IgnDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel8IgnDegrees, msg);
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