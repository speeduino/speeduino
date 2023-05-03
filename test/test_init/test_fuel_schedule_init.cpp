#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "schedule_calcs.h"

extern uint16_t req_fuel_uS;

static constexpr uint16_t reqFuel = 86; // ms * 10

static void assert_fuel_schedules(uint16_t crankAngle, uint16_t reqFuel, const bool enabled[], const uint16_t angle[])
{
  char msg[32];

  strcpy_P(msg, PSTR("CRANK_ANGLE_MAX_INJ"));
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_INJ, msg);
  strcpy_P(msg, PSTR("req_fuel_uS"));
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(reqFuel, req_fuel_uS, msg);

  strcpy_P(msg, PSTR("channel1InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[0], BIT_CHECK(channelInjEnabled, INJ1_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel1InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[0], channel1InjDegrees, msg);
  strcpy_P(msg, PSTR("channel2InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[1], BIT_CHECK(channelInjEnabled, INJ2_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel2InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[1], channel2InjDegrees, msg);
  strcpy_P(msg, PSTR("channel3InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[2], BIT_CHECK(channelInjEnabled, INJ3_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel3InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[2], channel3InjDegrees, msg);
  strcpy_P(msg, PSTR("channel4InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[3], BIT_CHECK(channelInjEnabled, INJ4_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel4InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel4InjDegrees, msg);
#if INJ_CHANNELS>=5
  strcpy_P(msg, PSTR("channel5InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[4], BIT_CHECK(channelInjEnabled, INJ5_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel5InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[4], channel5InjDegrees, msg);
#endif
#if INJ_CHANNELS>=6
  strcpy_P(msg, PSTR("channel6InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[5], BIT_CHECK(channelInjEnabled, INJ6_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel6InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[5], channel6InjDegrees, msg);
#endif
#if INJ_CHANNELS>=7
  strcpy_P(msg, PSTR("channel7InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[6], BIT_CHECK(channelInjEnabled, INJ7_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel7InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[6], channel7InjDegrees, msg);
#endif
#if INJ_CHANNELS>=8
  strcpy_P(msg, PSTR("channel8InjDegrees.isEnabled"));
  TEST_ASSERT_EQUAL_MESSAGE(enabled[7], BIT_CHECK(channelInjEnabled, INJ8_CMD_BIT), msg);
  strcpy_P(msg, PSTR("channel8InjDegrees"));
  TEST_ASSERT_EQUAL_MESSAGE(angle[7], channel8InjDegrees, msg);
#endif 
}

void test_fuel_schedule_1_cylinder_4stroke(void)
{
  configPage2.nCylinders = 1;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, false, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, false, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }
}

void test_fuel_schedule_1_cylinder_2stroke(void)
{
  configPage2.nCylinders = 1;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel; 
  configPage2.divider = 1;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, false, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, false, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }
}


void test_fuel_schedule_2_cylinder_4stroke(void)
{
  configPage2.nCylinders = 2;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, enabled, angle);
  }
}

void test_fuel_schedule_2_cylinder_2stroke(void)
{
  configPage2.nCylinders = 2;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }
}

void test_fuel_schedule_3_cylinder_4stroke(void)
{
  configPage2.nCylinders = 3;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,240,480,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,80,160,0,0,0,0,0};
    assert_fuel_schedules(240U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,240,480,0,240,480,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,240,480,0,0,0,0,0};
#endif
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,80,160,0,80,160,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,80,160,0,0,0,0,0};
#endif
    assert_fuel_schedules(240U, reqFuel * 50U, enabled, angle);
  }
}


void test_fuel_schedule_3_cylinder_2stroke(void)
{
  configPage2.nCylinders = 3;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 1;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,80,160,0,0,0,0,0};
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,80,160,0,0,0,0,0};
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,80,160,0,80,160,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,80,160,0,0,0,0,0};
#endif
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,80,160,0,80,160,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,80,160,0,0,0,0,0};
#endif
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }
}


void test_fuel_schedule_4_cylinder_4stroke(void)
{
  configPage2.nCylinders = 4;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 2;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=8
    const bool enabled[] = {true, true, true, true, true, true, true, true};
    const uint16_t angle[] = {0,180,360,540,0,180,360,540};
#elif INJ_CHANNELS >= 5
    const bool enabled[] = {true, true, true, true, true, false, false, false};
    const uint16_t angle[] = {0,180,360,540,0,0,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,360,540,0,0,0,0};
#endif
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, enabled, angle);
  }
}

void test_fuel_schedule_4_cylinder_2stroke(void)
{
  configPage2.nCylinders = 4;
  configPage2.strokes = TWO_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 2;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, false, false, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=8
    const bool enabled[] = {true, true, true, true, true, true, true, true};
    const uint16_t angle[] = {0,180,0,0,0,180,0,0};
#elif INJ_CHANNELS >= 5
    const bool enabled[] = {true, true, true, true, true, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,0,0,0,0,0};
#endif
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }
}


void test_fuel_schedule_5_cylinder_4stroke(void)
{
  configPage2.nCylinders = 5;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 5;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 5
    const bool enabled[] = {true, true, true, true, true, false, false, false};
    const uint16_t angle[] = {0,144,288,432,576,0,0,0};
    uint16_t expectedFuel = reqFuel * 100U;
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    uint16_t expectedFuel = reqFuel * 50U;
#endif
    assert_fuel_schedules(720U, expectedFuel, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,72,144,216,288,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 6
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,144,288,432,576,0,0,0};
    uint16_t expectedFuel = reqFuel * 100U;
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    uint16_t expectedFuel = reqFuel * 50U;
#endif
    assert_fuel_schedules(720U, expectedFuel, enabled, angle);
  }

  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 5
    const bool enabled[] = {true, true, true, true, true, false, false, false};
    const uint16_t angle[] = {0,72,144,216,288,0,0,0};
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,72,144,216,288,0,0,0};
#endif
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }
}

void test_fuel_schedule_6_cylinder_4stroke(void)
{
  configPage2.nCylinders = 6;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 6;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 6
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
#else
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
#endif
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,120,240,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 8
    const bool enabled[] = {true, true, true, true, true, true, false, false};
    const uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
#else
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
#endif
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 8
    const bool enabled[] = {true, true, true, true, true, true, true, true};
    const uint16_t angle[] = {0,120,240,0,0,120,240,0};
#else
    const bool enabled[] = {true, true, true, false, false, false, false, false};
    const uint16_t angle[] = {0,120,240,0,0,0,0,0};
#endif
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }
}


void test_fuel_schedule_8_cylinder_4stroke(void)
{
  configPage2.nCylinders = 8;
  configPage2.strokes = FOUR_STROKE;
  configPage2.engineType = EVEN_FIRE;
  configPage2.injTiming = true;
  configPage2.reqFuel = reqFuel;
  configPage2.divider = 8;

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 8
    const bool enabled[] = {true, true, true, true, true, true, true, true};
    const uint16_t angle[] = {0,90,180,270,360,450,540,630};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
#else
    const bool enabled[] = {true, true, true, true, false, false, false, false};
    const uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
#endif
  }

  // Staging not supported on 8 cylinders
}


void testFuelScheduleInit()
{
    RUN_TEST(test_fuel_schedule_1_cylinder_4stroke);
    RUN_TEST(test_fuel_schedule_1_cylinder_2stroke);
    RUN_TEST(test_fuel_schedule_2_cylinder_4stroke);
    RUN_TEST(test_fuel_schedule_2_cylinder_2stroke);
    RUN_TEST(test_fuel_schedule_3_cylinder_4stroke);
    RUN_TEST(test_fuel_schedule_3_cylinder_2stroke);
    RUN_TEST(test_fuel_schedule_4_cylinder_4stroke);
    RUN_TEST(test_fuel_schedule_4_cylinder_2stroke);
    RUN_TEST(test_fuel_schedule_5_cylinder_4stroke);
    RUN_TEST(test_fuel_schedule_6_cylinder_4stroke);
    RUN_TEST(test_fuel_schedule_8_cylinder_4stroke);
}