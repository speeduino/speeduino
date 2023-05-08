#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "schedule_calcs.h"
#include "scheduledIO.h"

static void assert_ignition_channel(uint16_t angle, uint8_t channel, int channelInjDegrees, void (*startFunction)(void), void (*endFunction)(void))
{
  char msg[32];

  sprintf_P(msg, PSTR("channe%" PRIu8 "1InjDegrees"), channel+1);
  TEST_ASSERT_EQUAL_MESSAGE(angle, channelInjDegrees, msg);
  sprintf_P(msg, PSTR("ign%" PRIu8 "StartFunction"), channel+1);
  TEST_ASSERT_TRUE_MESSAGE(channel>=maxIgnOutputs || (startFunction!=nullCallback), msg);
  sprintf_P(msg, PSTR("ign%" PRIu8 "EndFunction"), channel+1);
  TEST_ASSERT_TRUE_MESSAGE(channel>=maxIgnOutputs || (endFunction!=nullCallback), msg);
}

static void assert_ignition_schedules(uint16_t crankAngle, uint16_t expectedOutputs, const uint16_t angle[])
{
  char msg[48];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_IGN"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_IGN, msg);
  strcpy_P(msg, PSTR("maxIgnOutputs"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(expectedOutputs, maxIgnOutputs, msg);

  assert_ignition_channel(angle[0], 0, channel1IgnDegrees, ign1StartFunction, ign1EndFunction);
  assert_ignition_channel(angle[1], 1, channel2IgnDegrees, ign2StartFunction, ign2EndFunction);
  assert_ignition_channel(angle[2], 2, channel3IgnDegrees, ign3StartFunction, ign3EndFunction);
  assert_ignition_channel(angle[3], 3, channel4IgnDegrees, ign4StartFunction, ign4EndFunction);
#if IGN_CHANNELS>=5
  assert_ignition_channel(angle[4], 4, channel5IgnDegrees, ign5StartFunction, ign5EndFunction);
#endif
#if IGN_CHANNELS>=6
  assert_ignition_channel(angle[5], 5, channel6IgnDegrees, ign6StartFunction, ign6EndFunction);
#endif
#if IGN_CHANNELS>=7
  assert_ignition_channel(angle[6], 6, channel7IgnDegrees, ign7StartFunction, ign7EndFunction);
#endif
#if IGN_CHANNELS>=8
  assert_ignition_channel(angle[7], 7, channel8IgnDegrees, ign8StartFunction, ign8EndFunction);
#endif 
}

static void test_ignition_schedule_1_cylinder(void)
{
  configPage2.nCylinders = 1;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_ignition_schedules(720U, 1U, angle);
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_ignition_schedules(360U, 1U, angle);
  }  
}

static void test_ignition_schedule_2_cylinder(void)
{
  configPage2.nCylinders = 2;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_ignition_schedules(720U, 2U, angle);
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_ignition_schedules(360U, 2U, angle);
  }  
}

static void test_ignition_schedule_3_cylinder(void)
{
  configPage2.nCylinders = 3;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,240,480,0,0,0,0,0};
    assert_ignition_schedules(720U, 3U, angle);
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,120,240,0,0,0,0,0};
    assert_ignition_schedules(360U, 3U, angle);
  }  
}

static void test_ignition_schedule_4_cylinder(void)
{
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_ignition_schedules(720U, 4U, angle);
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_ignition_schedules(360U, 2U, angle);
  }  
}

static void test_ignition_schedule_5_cylinder(void)
{
  configPage2.nCylinders = 5;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,144,288,432,576,0,0,0};
    assert_ignition_schedules(720U, 5U, angle);
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,72,144,216,288,0,0,0};
    assert_ignition_schedules(360U, 5U, angle);
  }  
}

static void test_ignition_schedule_6_cylinder(void)
{
  configPage2.nCylinders = 6;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
#if IGN_CHANNELS >= 6
    const uint16_t angle[] = {0,120,240,360,480,540,0,0};
    assert_ignition_schedules(720U, 6U, angle);
#else
    const uint16_t angle[] = {0,120,240,0,0,0,0,0};
    assert_ignition_schedules(360U, 3U, angle);
#endif
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,120,240,0,0,0,0,0};
    assert_ignition_schedules(360U, 3U, angle);
  }  
}

static void test_ignition_schedule_8_cylinder(void)
{
  configPage2.nCylinders = 8;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;

  configPage4.sparkMode = IGN_MODE_SEQUENTIAL;
  initialiseAll(); //Run the main initialise function
  {
#if IGN_CHANNELS >= 8
    const uint16_t angle[] = {0,90,180,270,360,450,540,630};
    assert_ignition_schedules(720U, 8U, angle);
#else
    const uint16_t angle[] = {0,90,180,270,0,0,0,0};
    assert_ignition_schedules(360U, 4U, angle);
#endif
  }

  configPage4.sparkMode = IGN_MODE_WASTED;
  initialiseAll(); //Run the main initialise function
  {
    const uint16_t angle[] = {0,90,180,270,0,0,0,0};
    assert_ignition_schedules(360U, 4U, angle);
  }  
}

void testIgnitionScheduleInit()
{
    RUN_TEST(test_ignition_schedule_1_cylinder);
    RUN_TEST(test_ignition_schedule_2_cylinder);
    RUN_TEST(test_ignition_schedule_3_cylinder);
    RUN_TEST(test_ignition_schedule_4_cylinder);
    RUN_TEST(test_ignition_schedule_5_cylinder);
    RUN_TEST(test_ignition_schedule_6_cylinder);
    RUN_TEST(test_ignition_schedule_8_cylinder);
}