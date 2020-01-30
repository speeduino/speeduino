
#include <Arduino.h>
#include <unity.h>

#include "test_schedule.h"

#include "globals.h"
#include "scheduler.h"

#define TIMEOUT 1000
#define DURATION 1000
#define DELTA 20

static uint32_t start_time, end_time;
static void startCallback(void) { start_time = micros(); }
static void endCallback(void) { end_time = micros(); }

void test_accuracy_duration_inj1(void)
{
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj2(void)
{
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj3(void)
{
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj4(void)
{
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_inj5(void)
{
#if INJ_CHANNELS >= 5
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
#endif
}

void test_accuracy_duration_inj6(void)
{
#if INJ_CHANNELS >= 6
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
#endif
}

void test_accuracy_duration_inj7(void)
{
#if INJ_CHANNELS >= 7
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
#endif
}

void test_accuracy_duration_inj8(void)
{
#if INJ_CHANNELS >= 8
    fuelSchedule1.Status = OFF;
    setFuelSchedule1(TIMEOUT, DURATION);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
    start_time = micros();
    while(fuelSchedule1.Status == RUNNING) /*Wait*/ ;
    end_time = micros();
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
#endif
}



void test_accuracy_duration_ign1(void)
{
    ignitionSchedule1.Status = OFF;
    setIgnitionSchedule1(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule1.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, DURATION, end_time - start_time);
}

void test_accuracy_duration_ign2(void)
{
    ignitionSchedule2.Status = OFF;
    setIgnitionSchedule2(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule2.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_duration_ign3(void)
{
    ignitionSchedule3.Status = OFF;
    setIgnitionSchedule3(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule3.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_duration_ign4(void)
{
    ignitionSchedule4.Status = OFF;
        setIgnitionSchedule4(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule4.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
}

void test_accuracy_duration_ign5(void)
{
#if IGN_CHANNELS >= 5
    ignitionSchedule5.Status = OFF;
        setIgnitionSchedule5(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule5.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
#endif
}

void test_accuracy_duration_ign6(void)
{
#if INJ_CHANNELS >= 6
    ignitionSchedule6.Status = OFF;
        setIgnitionSchedule6(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule6.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
#endif
}

void test_accuracy_duration_ign7(void)
{
#if INJ_CHANNELS >= 7
    ignitionSchedule7.Status = OFF;
    setIgnitionSchedule7(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule7.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
#endif
}

void test_accuracy_duration_ign8(void)
{
#if INJ_CHANNELS >= 8
    ignitionSchedule8.Status = OFF;
        setIgnitionSchedule8(startCallback, TIMEOUT, DURATION, endCallback);
    while(ignitionSchedule8.Status != OFF) /*Wait*/ ;
    TEST_ASSERT_UINT32_WITHIN(DELTA, TIMEOUT, end_time - start_time);
#endif
}

void test_accuracy_duration(void)
{
    RUN_TEST(test_accuracy_duration_inj1);
    RUN_TEST(test_accuracy_duration_inj2);
    RUN_TEST(test_accuracy_duration_inj3);
    RUN_TEST(test_accuracy_duration_inj4);
    RUN_TEST(test_accuracy_duration_inj5);
    RUN_TEST(test_accuracy_duration_inj6);
    RUN_TEST(test_accuracy_duration_inj7);
    RUN_TEST(test_accuracy_duration_inj8);

    RUN_TEST(test_accuracy_duration_ign1);
    RUN_TEST(test_accuracy_duration_ign2);
    RUN_TEST(test_accuracy_duration_ign3);
    RUN_TEST(test_accuracy_duration_ign4);
    RUN_TEST(test_accuracy_duration_ign5);
    RUN_TEST(test_accuracy_duration_ign6);
    RUN_TEST(test_accuracy_duration_ign7);
    RUN_TEST(test_accuracy_duration_ign8);
}
