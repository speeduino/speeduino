
#include <Arduino.h>
#include <unity.h>

#include "scheduler.h"
#include "scheduledIO.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 24

static uint32_t start_time, end_time;
static void startCallback(void) { end_time = micros(); }
static void endCallback(void) { /*Empty*/ }

void test_accuracy_timeout_inj(FuelSchedule &schedule)
{
    initialiseSchedulers();
    schedule.pStartFunction = startCallback;
    schedule.pEndFunction = endCallback;
    start_time = micros();
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_inj1(void)
{
    test_accuracy_timeout_inj(fuelSchedule1);
}

void test_accuracy_timeout_inj2(void)
{
    test_accuracy_timeout_inj(fuelSchedule2);
}

void test_accuracy_timeout_inj3(void)
{
    test_accuracy_timeout_inj(fuelSchedule3);
}

void test_accuracy_timeout_inj4(void)
{
    test_accuracy_timeout_inj(fuelSchedule4);
}

#if INJ_CHANNELS >= 5
void test_accuracy_timeout_inj5(void)
{
    test_accuracy_timeout_inj(fuelSchedule5);
}
#endif

#if INJ_CHANNELS >= 6
void test_accuracy_timeout_inj6(void)
{
    test_accuracy_timeout_inj(fuelSchedule6);
}
#endif

#if INJ_CHANNELS >= 7
void test_accuracy_timeout_inj7(void)
{
    test_accuracy_timeout_inj(fuelSchedule7);
}
#endif

#if INJ_CHANNELS >= 8
void test_accuracy_timeout_inj8(void)
{
    test_accuracy_timeout_inj(fuelSchedule8);
}
#endif


void test_accuracy_timeout_ign1(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule1.pStartCallback = startCallback;
    ignitionSchedule1.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule1, TIMEOUT, DURATION);
    while(ignitionSchedule1.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_ign2(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule2.pStartCallback = startCallback;
    ignitionSchedule2.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule2, TIMEOUT, DURATION);
    while(ignitionSchedule2.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_ign3(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule3.pStartCallback = startCallback;
    ignitionSchedule3.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule3, TIMEOUT, DURATION);
    while(ignitionSchedule3.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_timeout_ign4(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule4.pStartCallback = startCallback;
    ignitionSchedule4.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule4, TIMEOUT, DURATION);
    while(ignitionSchedule4.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

#if IGN_CHANNELS >= 5
void test_accuracy_timeout_ign5(void)
{
    initialiseSchedulers();
    ignitionSchedule5.pStartCallback = startCallback;
    ignitionSchedule5.pEndCallback = endCallback;
    start_time = micros();
    setIgnitionSchedule(ignitionSchedule5, TIMEOUT, DURATION);
    while(ignitionSchedule5.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

#if IGN_CHANNELS >= 6
void test_accuracy_timeout_ign6(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule6.pStartCallback = startCallback;
    ignitionSchedule6.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule6, TIMEOUT, DURATION);
    while(ignitionSchedule6.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

#if IGN_CHANNELS >= 7
void test_accuracy_timeout_ign7(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule7.pStartCallback = startCallback;
    ignitionSchedule7.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule7, TIMEOUT, DURATION);
    while(ignitionSchedule7.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

#if IGN_CHANNELS >= 8
void test_accuracy_timeout_ign8(void)
{
    initialiseSchedulers();
    start_time = micros();
    ignitionSchedule8.pStartCallback = startCallback;
    ignitionSchedule8.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule8, TIMEOUT, DURATION);
    while(ignitionSchedule8.Status == PENDING) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

void test_accuracy_timeout(void)
{
    RUN_TEST(test_accuracy_timeout_inj1);
    RUN_TEST(test_accuracy_timeout_inj2);
    RUN_TEST(test_accuracy_timeout_inj3);
    RUN_TEST(test_accuracy_timeout_inj4);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_accuracy_timeout_inj5);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_accuracy_timeout_inj6);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_accuracy_timeout_inj7);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_accuracy_timeout_inj8);
#endif

    RUN_TEST(test_accuracy_timeout_ign1);
    RUN_TEST(test_accuracy_timeout_ign2);
    RUN_TEST(test_accuracy_timeout_ign3);
    RUN_TEST(test_accuracy_timeout_ign4);
#if IGN_CHANNELS >= 5
    RUN_TEST(test_accuracy_timeout_ign5);
#endif
#if IGN_CHANNELS >= 6
    RUN_TEST(test_accuracy_timeout_ign6);
#endif
#if IGN_CHANNELS >= 7
    RUN_TEST(test_accuracy_timeout_ign7);
#endif
#if IGN_CHANNELS >= 8
    RUN_TEST(test_accuracy_timeout_ign8);
#endif
}
