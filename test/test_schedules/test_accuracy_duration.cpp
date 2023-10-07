
#include <Arduino.h>
#include <unity.h>

#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 20

static uint32_t start_time, end_time;
static void startCallback(void) { start_time = micros(); }
static void endCallback(void) { end_time = micros(); }

void test_accuracy_duration_inj(FuelSchedule &schedule)
{
    initialiseSchedulers();
    schedule.pStartFunction = startCallback;
    schedule.pEndFunction = endCallback;
    setFuelSchedule(schedule, TIMEOUT, DURATION);
    while(schedule.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj1(void)
{
    test_accuracy_duration_inj(fuelSchedule1);
}

void test_accuracy_duration_inj2(void)
{
    test_accuracy_duration_inj(fuelSchedule2);
}

void test_accuracy_duration_inj3(void)
{
    test_accuracy_duration_inj(fuelSchedule3);
}

void test_accuracy_duration_inj4(void)
{
    test_accuracy_duration_inj(fuelSchedule4);
}

#if INJ_CHANNELS >= 5
void test_accuracy_duration_inj5(void)
{
    test_accuracy_duration_inj(fuelSchedule5);
}
#endif

#if INJ_CHANNELS >= 6
void test_accuracy_duration_inj6(void)
{
    test_accuracy_duration_inj(fuelSchedule6);
}
#endif

#if INJ_CHANNELS >= 7
void test_accuracy_duration_inj7(void)
{
    test_accuracy_duration_inj(fuelSchedule7);
}
#endif

#if INJ_CHANNELS >= 8
void test_accuracy_duration_inj8(void)
{
    test_accuracy_duration_inj(fuelSchedule8);
}
#endif


void test_accuracy_duration_ign1(void)
{
    initialiseSchedulers();
    ignitionSchedule1.pStartCallback = startCallback;
    ignitionSchedule1.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule1, TIMEOUT, DURATION);
    while( (ignitionSchedule1.Status == PENDING) || (ignitionSchedule1.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_ign2(void)
{
    initialiseSchedulers();
    ignitionSchedule2.pStartCallback = startCallback;
    ignitionSchedule2.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule2, TIMEOUT, DURATION);
    while( (ignitionSchedule2.Status == PENDING) || (ignitionSchedule2.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_duration_ign3(void)
{
    initialiseSchedulers();
    ignitionSchedule3.pStartCallback = startCallback;
    ignitionSchedule3.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule3, TIMEOUT, DURATION);
    while( (ignitionSchedule3.Status == PENDING) || (ignitionSchedule3.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_duration_ign4(void)
{
    initialiseSchedulers();
    ignitionSchedule4.pStartCallback = startCallback;
    ignitionSchedule4.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule4, TIMEOUT, DURATION);
    while( (ignitionSchedule4.Status == PENDING) || (ignitionSchedule4.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_duration_ign5(void)
{
#if IGN_CHANNELS >= 5
    initialiseSchedulers();
    ignitionSchedule5.pStartCallback = startCallback;
    ignitionSchedule5.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule5, TIMEOUT, DURATION);
    while( (ignitionSchedule5.Status == PENDING) || (ignitionSchedule5.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
#endif
}

#if INJ_CHANNELS >= 6
void test_accuracy_duration_ign6(void)
{
    initialiseSchedulers();
    ignitionSchedule6.pStartCallback = startCallback;
    ignitionSchedule6.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule6, TIMEOUT, DURATION);
    while( (ignitionSchedule6.Status == PENDING) || (ignitionSchedule6.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

#if INJ_CHANNELS >= 7
void test_accuracy_duration_ign7(void)
{
    initialiseSchedulers();
    ignitionSchedule7.pStartCallback = startCallback;
    ignitionSchedule7.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule7, TIMEOUT, DURATION);
    while( (ignitionSchedule7.Status == PENDING) || (ignitionSchedule7.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

#if INJ_CHANNELS >= 8
void test_accuracy_duration_ign8(void)
{
    initialiseSchedulers();
    ignitionSchedule8.pStartCallback = startCallback;
    ignitionSchedule8.pEndCallback = endCallback;
    setIgnitionSchedule(ignitionSchedule8, TIMEOUT, DURATION);
    while( (ignitionSchedule8.Status == PENDING) || (ignitionSchedule8.Status == RUNNING) ) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}
#endif

void test_accuracy_duration(void)
{
    RUN_TEST(test_accuracy_duration_inj1);
    RUN_TEST(test_accuracy_duration_inj2);
    RUN_TEST(test_accuracy_duration_inj3);
    RUN_TEST(test_accuracy_duration_inj4);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_accuracy_duration_inj5);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_accuracy_duration_inj6);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_accuracy_duration_inj7);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_accuracy_duration_inj8);
#endif

    RUN_TEST(test_accuracy_duration_ign1);
    RUN_TEST(test_accuracy_duration_ign2);
    RUN_TEST(test_accuracy_duration_ign3);
    RUN_TEST(test_accuracy_duration_ign4);
#if INJ_CHANNELS >= 5
    RUN_TEST(test_accuracy_duration_ign5);
#endif
#if INJ_CHANNELS >= 6
    RUN_TEST(test_accuracy_duration_ign6);
#endif
#if INJ_CHANNELS >= 7
    RUN_TEST(test_accuracy_duration_ign7);
#endif
#if INJ_CHANNELS >= 8
    RUN_TEST(test_accuracy_duration_ign8);
#endif
}
