
#include <Arduino.h>
#include <unity.h>

#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000

static void emptyCallback(void) {  }

void test_status_pending_to_running_inj1(void)
{
    initialiseSchedulers();
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule1.Status);
}

void test_status_pending_to_running_inj2(void)
{
    initialiseSchedulers();
    setFuelSchedule2(TIMEOUT, DURATION);
    while(fuelSchedule2.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule2.Status);
}

void test_status_pending_to_running_inj3(void)
{
    initialiseSchedulers();
    setFuelSchedule3(TIMEOUT, DURATION);
    while(fuelSchedule3.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule3.Status);
}

void test_status_pending_to_running_inj4(void)
{
    initialiseSchedulers();
    setFuelSchedule4(TIMEOUT, DURATION);
    while(fuelSchedule4.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule4.Status);
}

void test_status_pending_to_running_inj5(void)
{
#if INJ_CHANNELS >= 5
    initialiseSchedulers();
    setFuelSchedule5(TIMEOUT, DURATION);
    while(fuelSchedule5.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule5.Status);
#endif
}

void test_status_pending_to_running_inj6(void)
{
#if INJ_CHANNELS >= 6
    initialiseSchedulers();
    setFuelSchedule6(TIMEOUT, DURATION);
    while(fuelSchedule6.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule6.Status);
#endif
}

void test_status_pending_to_running_inj7(void)
{
#if INJ_CHANNELS >= 7
    initialiseSchedulers();
    setFuelSchedule7(TIMEOUT, DURATION);
    while(fuelSchedule7.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule7.Status);
#endif
}

void test_status_pending_to_running_inj8(void)
{
#if INJ_CHANNELS >= 8
    initialiseSchedulers();
    setFuelSchedule8(TIMEOUT, DURATION);
    while(fuelSchedule8.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, fuelSchedule8.Status);
#endif
}


void test_status_pending_to_running_ign1(void)
{
    initialiseSchedulers();
    ignitionSchedule1.pStartCallback = emptyCallback;
    ignitionSchedule1.pEndCallback = emptyCallback;
    setIgnitionSchedule1(TIMEOUT, DURATION);
    while(ignitionSchedule1.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule1.Status);
}

void test_status_pending_to_running_ign2(void)
{
    initialiseSchedulers();
    ignitionSchedule2.pStartCallback = emptyCallback;
    ignitionSchedule2.pEndCallback = emptyCallback;
    setIgnitionSchedule2(TIMEOUT, DURATION);
    while(ignitionSchedule2.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule2.Status);
}

void test_status_pending_to_running_ign3(void)
{
    initialiseSchedulers();
    ignitionSchedule3.pStartCallback = emptyCallback;
    ignitionSchedule3.pEndCallback = emptyCallback;
    setIgnitionSchedule3(TIMEOUT, DURATION);
    while(ignitionSchedule3.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule3.Status);
}

void test_status_pending_to_running_ign4(void)
{
    initialiseSchedulers();
    ignitionSchedule4.pStartCallback = emptyCallback;
    ignitionSchedule4.pEndCallback = emptyCallback;
    setIgnitionSchedule4(TIMEOUT, DURATION);
    while(ignitionSchedule4.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule4.Status);
}

void test_status_pending_to_running_ign5(void)
{
#if IGN_CHANNELS >= 5
    initialiseSchedulers();
    ignitionSchedule5.pStartCallback = emptyCallback;
    ignitionSchedule5.pEndCallback = emptyCallback;
    setIgnitionSchedule5(TIMEOUT, DURATION);
    while(ignitionSchedule5.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule5.Status);
#endif
}

void test_status_pending_to_running_ign6(void)
{
#if INJ_CHANNELS >= 6
    initialiseSchedulers();
    ignitionSchedule6.pStartCallback = emptyCallback;
    ignitionSchedule6.pEndCallback = emptyCallback;
    setIgnitionSchedule6(TIMEOUT, DURATION);
    while(ignitionSchedule6.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule6.Status);
#endif
}

void test_status_pending_to_running_ign7(void)
{
#if INJ_CHANNELS >= 7
    initialiseSchedulers();
    ignitionSchedule7.pStartCallback = emptyCallback;
    ignitionSchedule7.pEndCallback = emptyCallback;
    setIgnitionSchedule7(TIMEOUT, DURATION);
    while(ignitionSchedule7.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule7.Status);
#endif
}

void test_status_pending_to_running_ign8(void)
{
#if INJ_CHANNELS >= 8
    initialiseSchedulers();
    ignitionSchedule8.pStartCallback = emptyCallback;
    ignitionSchedule8.pEndCallback = emptyCallback;
    setIgnitionSchedule8(TIMEOUT, DURATION);
    while(ignitionSchedule8.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule8.Status);
#endif
}

void test_status_pending_to_running(void)
{
    RUN_TEST(test_status_pending_to_running_inj1);
    RUN_TEST(test_status_pending_to_running_inj2);
    RUN_TEST(test_status_pending_to_running_inj3);
    RUN_TEST(test_status_pending_to_running_inj4);
    RUN_TEST(test_status_pending_to_running_inj5);
    RUN_TEST(test_status_pending_to_running_inj6);
    RUN_TEST(test_status_pending_to_running_inj7);
    RUN_TEST(test_status_pending_to_running_inj8);

    RUN_TEST(test_status_pending_to_running_ign1);
    RUN_TEST(test_status_pending_to_running_ign2);
    RUN_TEST(test_status_pending_to_running_ign3);
    RUN_TEST(test_status_pending_to_running_ign4);
    RUN_TEST(test_status_pending_to_running_ign5);
    RUN_TEST(test_status_pending_to_running_ign6);
    RUN_TEST(test_status_pending_to_running_ign7);
    RUN_TEST(test_status_pending_to_running_ign8);
}
