
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

void test_status_pending_to_running_ign(void)
{
    initialiseSchedulers();
    ignitionSchedule[0].StartCallback = emptyCallback;
    ignitionSchedule[0].EndCallback = emptyCallback;
    setIgnitionSchedule(&ignitionSchedule[0], TIMEOUT, DURATION);
    while(ignitionSchedule[0].Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_EQUAL(RUNNING, ignitionSchedule[0].Status);
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
    
    RUN_TEST(test_status_pending_to_running_ign);
}
