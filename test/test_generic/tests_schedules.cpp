#include <globals.h>
#include <init.h>
#include <unity.h>
#include "tests_schedules.h"
#include "scheduler.h"


/* ****************************************************************************
 * Static functions
 * ***************************************************************************/

static void test_schedule_fuel_time(void);
static void test_schedule_fuel_pending(void);
static void test_schedule_ignition_time(void);
static void test_schedule_ignition_pending(void);

static uint32_t beginTime, endTime;
static void beginCallback(void) { beginTime = micros(); }
static void endCallback(void) { endTime = micros(); }


/* ****************************************************************************
 * Global variables
 * ***************************************************************************/

static FuelSchedule* fuelSchedule[] = {
  &fuelSchedule1,
  &fuelSchedule2,
  &fuelSchedule3,
  &fuelSchedule4,
  &fuelSchedule5,
  &fuelSchedule6,
  &fuelSchedule7,
  &fuelSchedule8,
};

static Schedule* ignitionSchedule[] = {
  &ignitionSchedule1,
  &ignitionSchedule2,
  &ignitionSchedule3,
  &ignitionSchedule4,
  &ignitionSchedule5,
  &ignitionSchedule6,
  &ignitionSchedule7,
  &ignitionSchedule8,
};

void (*setFuelSchedule[])(unsigned long, unsigned long) = {
  setFuelSchedule1,
  setFuelSchedule2,
  setFuelSchedule3,
  setFuelSchedule4,
#if INJ_CHANNELS >= 5
  setFuelSchedule5,
#endif
#if INJ_CHANNELS >= 6
  setFuelSchedule6,
#endif
#if INJ_CHANNELS >= 7
  setFuelSchedule7,
#endif
#if INJ_CHANNELS >= 8
  setFuelSchedule8,
#endif
};

void (*setIgnitionSchedule[])(void (*)(), unsigned long, unsigned long, void (*)()) = {
  setIgnitionSchedule1,
  setIgnitionSchedule2,
  setIgnitionSchedule3,
  setIgnitionSchedule4,
#if IGN_CHANNELS >= 5
  setIgnitionSchedule5,
#endif
#if IGN_CHANNELS >= 6
  setIgnitionSchedule6,
#endif
#if IGN_CHANNELS >= 7
  setIgnitionSchedule7,
#endif
#if IGN_CHANNELS >= 8
  setIgnitionSchedule8,
#endif
};


/* ****************************************************************************
 * Test Main
 * ***************************************************************************/

void testSchedules()
{
  RUN_TEST(test_schedule_fuel_time);
  RUN_TEST(test_schedule_fuel_pending);
  RUN_TEST(test_schedule_ignition_time);
  RUN_TEST(test_schedule_ignition_pending);
}


/* ****************************************************************************
 * Test Functions
 * ***************************************************************************/

static void test_schedule_fuel_time(void)
{
  TEST_ASSERT_EQUAL(true, initialisationComplete);

  for(int i = 0; i < INJ_CHANNELS; i++)
  {
    TEST_ASSERT_EQUAL(OFF, fuelSchedule[i]->Status);

    uint32_t time1 = micros();
    setFuelSchedule[i](1000, 1000);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule[i]->Status);

    while(fuelSchedule[i]->Status == PENDING); // wait
    uint32_t time2 = micros();
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule[i]->Status);

    while(fuelSchedule[i]->Status == RUNNING); // wait
    uint32_t time3 = micros();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule[i]->Status);

    TEST_ASSERT_EQUAL(1016, time2 - time1);
    TEST_ASSERT_EQUAL(1008, time3 - time2);
  }
}

static void test_schedule_fuel_pending(void)
{
  TEST_ASSERT_EQUAL(true, initialisationComplete);

  for(int i = 0; i < INJ_CHANNELS; i++)
  {
    TEST_ASSERT_EQUAL(OFF, fuelSchedule[i]->Status);

    setFuelSchedule[i](1000, 1000);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule[i]->Status);

    while(fuelSchedule[i]->Status == PENDING); // wait
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule[i]->Status);
    setFuelSchedule[i](1500, 500); // Once running, set another schedule

    while(fuelSchedule[i]->Status == RUNNING); // wait
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule[i]->Status);

    while(fuelSchedule[i]->Status == PENDING); // wait
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule[i]->Status);

    while(fuelSchedule[i]->Status == RUNNING); // wait
    TEST_ASSERT_EQUAL(OFF, fuelSchedule[i]->Status);
  }
}

static void test_schedule_ignition_time(void)
{
  TEST_ASSERT_EQUAL(true, initialisationComplete);

  for(int i = 0; i < IGN_CHANNELS; i++)
  {
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[i]->Status);

    uint32_t time1 = micros();
    setIgnitionSchedule[i](beginCallback, 1000, 1000, endCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule[i]->Status);

    while(ignitionSchedule[i]->Status == PENDING); // wait
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule[i]->Status);

    while(ignitionSchedule[i]->Status == RUNNING); // wait
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[i]->Status);

    TEST_ASSERT_EQUAL(1012, beginTime - time1);
    TEST_ASSERT_EQUAL(1012, endTime - beginTime);
  }
}

static void test_schedule_ignition_pending(void)
{
  TEST_ASSERT_EQUAL(true, initialisationComplete);

  for(int i = 0; i < IGN_CHANNELS; i++)
  {
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[i]->Status);

    setIgnitionSchedule[i](beginCallback, 1000, 1000, endCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule[i]->Status);

    while(ignitionSchedule[i]->Status == PENDING); // wait
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule[i]->Status);
    setIgnitionSchedule[i](beginCallback, 1500, 500, endCallback); // Once running, set another schedule

    while(ignitionSchedule[i]->Status == RUNNING); // wait
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule[i]->Status);

    while(ignitionSchedule[i]->Status == PENDING); // wait
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule[i]->Status);

    while(ignitionSchedule[i]->Status == RUNNING); // wait
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[i]->Status);
  }
}