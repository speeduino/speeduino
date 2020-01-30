
#include <Arduino.h>
#include <unity.h>

#include "globals.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void emptyCallback(void) {  }

void test_status_running_to_off_inj1(void)
{
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while( (fuelSchedule1.Status == PENDING) || (fuelSchedule1.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule1.Status);
}

void test_status_running_to_off_inj2(void)
{
    fuelSchedule2.Status = OFF;
    setFuelSchedule2(TIMEOUT, DURATION);
    while( (fuelSchedule2.Status == PENDING) || (fuelSchedule2.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule2.Status);
}

void test_status_running_to_off_inj3(void)
{
    fuelSchedule3.Status = OFF;
    setFuelSchedule3(TIMEOUT, DURATION);
    while( (fuelSchedule3.Status == PENDING) || (fuelSchedule3.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule3.Status);
}

void test_status_running_to_off_inj4(void)
{
    fuelSchedule4.Status = OFF;
    setFuelSchedule4(TIMEOUT, DURATION);
    while( (fuelSchedule4.Status == PENDING) || (fuelSchedule4.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule4.Status);
}

void test_status_running_to_off_inj5(void)
{
#if INJ_CHANNELS >= 5
    fuelSchedule5.Status = OFF;
    setFuelSchedule5(TIMEOUT, DURATION);
    while( (fuelSchedule5.Status == PENDING) || (fuelSchedule5.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule5.Status);
#endif
}

void test_status_running_to_off_inj6(void)
{
#if INJ_CHANNELS >= 6
    fuelSchedule6.Status = OFF;
    setFuelSchedule6(TIMEOUT, DURATION);
    while( (fuelSchedule6.Status == PENDING) || (fuelSchedule6.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule6.Status);
#endif
}

void test_status_running_to_off_inj7(void)
{
#if INJ_CHANNELS >= 7
    fuelSchedule7.Status = OFF;
    setFuelSchedule7(TIMEOUT, DURATION);
    while( (fuelSchedule7.Status == PENDING) || (fuelSchedule7.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule7.Status);
#endif
}

void test_status_running_to_off_inj8(void)
{
#if INJ_CHANNELS >= 8
    fuelSchedule8.Status = OFF;
    setFuelSchedule8(TIMEOUT, DURATION);
    while( (fuelSchedule8.Status == PENDING) || (fuelSchedule8.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, fuelSchedule8.Status);
#endif
}


void test_status_running_to_off_ign1(void)
{
    ignitionSchedule1.Status = OFF;
    setIgnitionSchedule1(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule1.Status == PENDING) || (ignitionSchedule1.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule1.Status);
}

void test_status_running_to_off_ign2(void)
{
    ignitionSchedule2.Status = OFF;
    setIgnitionSchedule2(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule2.Status == PENDING) || (ignitionSchedule2.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule2.Status);
}

void test_status_running_to_off_ign3(void)
{
    ignitionSchedule3.Status = OFF;
    setIgnitionSchedule3(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule3.Status == PENDING) || (ignitionSchedule3.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule3.Status);
}

void test_status_running_to_off_ign4(void)
{
    ignitionSchedule4.Status = OFF;
    setIgnitionSchedule4(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule4.Status == PENDING) || (ignitionSchedule4.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule4.Status);
}

void test_status_running_to_off_ign5(void)
{
#if IGN_CHANNELS >= 5
    ignitionSchedule5.Status = OFF;
    setIgnitionSchedule5(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule5.Status == PENDING) || (ignitionSchedule5.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule5.Status);
#endif
}

void test_status_running_to_off_ign6(void)
{
#if INJ_CHANNELS >= 6
    ignitionSchedule6.Status = OFF;
    setIgnitionSchedule6(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule6.Status == PENDING) || (ignitionSchedule6.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule6.Status);
#endif
}

void test_status_running_to_off_ign7(void)
{
#if INJ_CHANNELS >= 7
    ignitionSchedule7.Status = OFF;
    setIgnitionSchedule7(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule7.Status == PENDING) || (ignitionSchedule7.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule7.Status);
#endif
}

void test_status_running_to_off_ign8(void)
{
#if INJ_CHANNELS >= 8
    ignitionSchedule8.Status = OFF;
    setIgnitionSchedule8(emptyCallback, TIMEOUT, DURATION, emptyCallback);
    while( (ignitionSchedule8.Status == PENDING) || (ignitionSchedule8.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule8.Status);
#endif
}

void test_status_running_to_off(void)
{
    RUN_TEST(test_status_running_to_off_inj1);
    RUN_TEST(test_status_running_to_off_inj2);
    RUN_TEST(test_status_running_to_off_inj3);
    RUN_TEST(test_status_running_to_off_inj4);
    RUN_TEST(test_status_running_to_off_inj5);
    RUN_TEST(test_status_running_to_off_inj6);
    RUN_TEST(test_status_running_to_off_inj7);
    RUN_TEST(test_status_running_to_off_inj8);

    RUN_TEST(test_status_running_to_off_ign1);
    RUN_TEST(test_status_running_to_off_ign2);
    RUN_TEST(test_status_running_to_off_ign3);
    RUN_TEST(test_status_running_to_off_ign4);
    RUN_TEST(test_status_running_to_off_ign5);
    RUN_TEST(test_status_running_to_off_ign6);
    RUN_TEST(test_status_running_to_off_ign7);
    RUN_TEST(test_status_running_to_off_ign8);
}
