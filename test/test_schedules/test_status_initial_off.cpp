#include <Arduino.h>
#include <unity.h>
#include "../test_utils.h"
#include "scheduler.h"

static void test_status_initial_off_inj1(void)
{
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule1.Status);
}

static void test_status_initial_off_inj2(void)
{
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule2.Status);
}

static void test_status_initial_off_inj3(void)
{
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule3.Status);
}

static void test_status_initial_off_inj4(void)
{
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule4.Status);
}

static void test_status_initial_off_inj5(void)
{
#if INJ_CHANNELS >= 5
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule5.Status);
#endif 
}

static void test_status_initial_off_inj6(void)
{
#if INJ_CHANNELS >= 6
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule6.Status);
#endif 
}

static void test_status_initial_off_inj7(void)
{
#if INJ_CHANNELS >= 7
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule7.Status);
#endif 
}

static void test_status_initial_off_inj8(void)
{
#if INJ_CHANNELS >= 8
    initialiseFuelSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule8.Status);
#endif 
}


static void test_status_initial_off_ign1(void)
{
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule1.Status);
}

static void test_status_initial_off_ign2(void)
{
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule2.Status);
}

static void test_status_initial_off_ign3(void)
{
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule3.Status);
}

static void test_status_initial_off_ign4(void)
{
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule4.Status);
}

static void test_status_initial_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule5.Status);
#endif
}

static void test_status_initial_off_ign6(void)
{
#if IGN_CHANNELS >= 6
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule6.Status);
#endif
}

static void test_status_initial_off_ign7(void)
{
#if IGN_CHANNELS >= 7
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule7.Status);
#endif
}

static void test_status_initial_off_ign8(void)
{
#if IGN_CHANNELS >= 8
    initialiseIgnitionSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule8.Status);
#endif
}

void test_status_initial_off(void)
{
  SET_UNITY_FILENAME() {

    RUN_TEST_P(test_status_initial_off_inj1);
    RUN_TEST_P(test_status_initial_off_inj2);
    RUN_TEST_P(test_status_initial_off_inj3);
    RUN_TEST_P(test_status_initial_off_inj4);
    RUN_TEST_P(test_status_initial_off_inj5);
    RUN_TEST_P(test_status_initial_off_inj6);
    RUN_TEST_P(test_status_initial_off_inj7);
    RUN_TEST_P(test_status_initial_off_inj8);

    RUN_TEST_P(test_status_initial_off_ign1);
    RUN_TEST_P(test_status_initial_off_ign2);
    RUN_TEST_P(test_status_initial_off_ign3);
    RUN_TEST_P(test_status_initial_off_ign4);
    RUN_TEST_P(test_status_initial_off_ign5);
    RUN_TEST_P(test_status_initial_off_ign6);
    RUN_TEST_P(test_status_initial_off_ign7);
    RUN_TEST_P(test_status_initial_off_ign8);
  }
}