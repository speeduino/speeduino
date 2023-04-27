#include <Arduino.h>
#include <unity.h>
#include "globals.h"
#include "init.h"
#include "schedule_calcs.h"

extern uint16_t req_fuel_uS;

static constexpr uint16_t reqFuel = 86; // ms * 10

static void assert_fuel_schedules(uint16_t crankAngle, uint16_t reqFuel, bool enabled[], uint16_t angle[])
{
  // TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, currentStatus.nSquirts, "nSquirts");
  TEST_ASSERT_EQUAL_INT16_MESSAGE(crankAngle, CRANK_ANGLE_MAX_INJ, "CRANK_ANGLE_MAX_INJ");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(reqFuel, req_fuel_uS, "req_fuel_uS");

  TEST_ASSERT_EQUAL_MESSAGE(enabled[0], BIT_CHECK(channelInjEnabled, INJ1_CMD_BIT), "fuelSchedule1.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[0], channel1InjDegrees, "channel1InjDegrees");
  TEST_ASSERT_EQUAL_MESSAGE(enabled[1], BIT_CHECK(channelInjEnabled, INJ2_CMD_BIT), "fuelSchedule2.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[1], channel2InjDegrees, "channel2InjDegrees");
  TEST_ASSERT_EQUAL_MESSAGE(enabled[2], BIT_CHECK(channelInjEnabled, INJ3_CMD_BIT), "fuelSchedule3.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[2], channel3InjDegrees, "channel3InjDegrees");
  TEST_ASSERT_EQUAL_MESSAGE(enabled[3], BIT_CHECK(channelInjEnabled, INJ4_CMD_BIT), "fuelSchedule4.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[3], channel4InjDegrees, "channel4InjDegrees");
#if INJ_CHANNELS>=5
  TEST_ASSERT_EQUAL_MESSAGE(enabled[4], BIT_CHECK(channelInjEnabled, INJ5_CMD_BIT), "fuelSchedule5.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[4], channel5InjDegrees, "channel5InjDegrees");
#endif
#if INJ_CHANNELS>=6
  TEST_ASSERT_EQUAL_MESSAGE(enabled[5], BIT_CHECK(channelInjEnabled, INJ6_CMD_BIT), "fuelSchedule6.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[5], channel6InjDegrees, "channel6InjDegrees");
#endif
#if INJ_CHANNELS>=7
  TEST_ASSERT_EQUAL_MESSAGE(enabled[6], BIT_CHECK(channelInjEnabled, INJ7_CMD_BIT), "fuelSchedule7.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[6], channel7InjDegrees, "channel7InjDegrees");
#endif
#if INJ_CHANNELS>=8
  TEST_ASSERT_EQUAL_MESSAGE(enabled[7], BIT_CHECK(channelInjEnabled, INJ8_CMD_BIT), "fuelSchedule8.isEnabled");
  TEST_ASSERT_EQUAL_MESSAGE(angle[7], channel8InjDegrees, "channel8InjDegrees");
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
    bool enabled[] = {true, false, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, false, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
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
    bool enabled[] = {true, false, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, false, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
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
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,180,0,0,0,0};
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
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,180,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,180,0,0,0,0};
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
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,240,480,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,80,160,0,0,0,0,0};
    assert_fuel_schedules(240U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,240,480,0,240,480,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,240,480,0,0,0,0,0};
#endif
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,80,160,0,80,160,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,80,160,0,0,0,0,0};
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
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,80,160,0,0,0,0,0};
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,80,160,0,0,0,0,0};
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,80,160,0,80,160,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,80,160,0,0,0,0,0};
#endif
    assert_fuel_schedules(120U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=6
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,80,160,0,80,160,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,80,160,0,0,0,0,0};
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
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,360,540,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(360U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=8
    bool enabled[] = {true, true, true, true, true, true, true, true};
    uint16_t angle[] = {0,180,360,540,0,180,360,540};
#elif INJ_CHANNELS >= 5
    bool enabled[] = {true, true, true, true, true, false, false, false};
    uint16_t angle[] = {0,180,360,540,0,0,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,360,540,0,0,0,0};
#endif
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,180,0,0,0,0};
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
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, false, false, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS>=8
    bool enabled[] = {true, true, true, true, true, true, true, true};
    uint16_t angle[] = {0,180,0,0,0,180,0,0};
#elif INJ_CHANNELS >= 5
    bool enabled[] = {true, true, true, true, true, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,0,0,0,0,0};
#endif
    assert_fuel_schedules(180U, reqFuel * 100U, enabled, angle);
  }

  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,180,0,180,0,0,0,0};
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
    bool enabled[] = {true, true, true, true, true, false, false, false};
    uint16_t angle[] = {0,144,288,432,576,0,0,0};
    uint16_t expectedFuel = reqFuel * 100U;
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    uint16_t expectedFuel = reqFuel * 50U;
#endif
    assert_fuel_schedules(720U, expectedFuel, enabled, angle);
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,72,144,216,288,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 6
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,144,288,432,576,0,0,0};
    uint16_t expectedFuel = reqFuel * 100U;
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    uint16_t expectedFuel = reqFuel * 50U;
#endif
    assert_fuel_schedules(720U, expectedFuel, enabled, angle);
  }

  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 5
    bool enabled[] = {true, true, true, true, true, false, false, false};
    uint16_t angle[] = {0,72,144,216,288,0,0,0};
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,72,144,216,288,0,0,0};
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
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
#else
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
#endif
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = false;
  initialiseAll(); //Run the main initialise function
  {
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,120,240,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
  }

  configPage2.injLayout = INJ_SEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 8
    bool enabled[] = {true, true, true, true, true, true, false, false};
    uint16_t angle[] = {0,120,240,360,480,600,0,0};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
#else
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
    assert_fuel_schedules(720U, reqFuel * 50U, enabled, angle);
#endif
  }

  configPage2.injLayout = INJ_SEMISEQUENTIAL;
  configPage10.stagingEnabled = true;
  initialiseAll(); //Run the main initialise function
  {
#if INJ_CHANNELS >= 8
    bool enabled[] = {true, true, true, true, true, true, true, true};
    uint16_t angle[] = {0,120,240,0,0,120,240,0};
#else
    bool enabled[] = {true, true, true, false, false, false, false, false};
    uint16_t angle[] = {0,120,240,0,0,0,0,0};
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
    bool enabled[] = {true, true, true, true, true, true, true, true};
    uint16_t angle[] = {0,90,180,270,360,450,540,630};
    assert_fuel_schedules(720U, reqFuel * 100U, enabled, angle);
#else
    bool enabled[] = {true, true, true, true, false, false, false, false};
    uint16_t angle[] = {0,0,0,0,0,0,0,0};
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