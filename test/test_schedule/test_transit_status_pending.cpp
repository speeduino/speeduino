
#include <Arduino.h>
#include <unity.h>

#include "globals.h"
#include "scheduler.h"

static void emptyCallback(void) {  }

void test_transit_status_pending_inj1(void)
{
    setFuelSchedule1(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule1.Status);
    while(fuelSchedule1.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_inj2(void)
{
    setFuelSchedule2(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule2.Status);
    while(fuelSchedule2.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_inj3(void)
{
    setFuelSchedule3(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule3.Status);
    while(fuelSchedule3.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_inj4(void)
{
    setFuelSchedule4(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule4.Status);
    while(fuelSchedule4.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_inj5(void)
{
#if INJ_CHANNELS >= 5
    setFuelSchedule5(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule5.Status);
    while(fuelSchedule5.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending_inj6(void)
{
#if INJ_CHANNELS >= 6
    setFuelSchedule6(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule6.Status);
    while(fuelSchedule6.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending_inj7(void)
{
#if INJ_CHANNELS >= 7
    setFuelSchedule7(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule7.Status);
    while(fuelSchedule7.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending_inj8(void)
{
#if INJ_CHANNELS >= 8
    setFuelSchedule8(100, 100);
    TEST_ASSERT_EQUAL(PENDING, fuelSchedule8.Status);
    while(fuelSchedule8.Status == PENDING) /*Wait*/ ;
#endif
}


void test_transit_status_pending_ign1(void)
{
    setIgnitionSchedule1(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule1.Status);
    while(ignitionSchedule1.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_ign2(void)
{
    setIgnitionSchedule2(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule2.Status);
    while(ignitionSchedule2.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_ign3(void)
{
    setIgnitionSchedule3(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule3.Status);
    while(ignitionSchedule3.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_ign4(void)
{
    setIgnitionSchedule4(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule4.Status);
    while(ignitionSchedule4.Status == PENDING) /*Wait*/ ;
}

void test_transit_status_pending_ign5(void)
{
#if IGN_CHANNELS >= 5
    setIgnitionSchedule5(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule5.Status);
    while(ignitionSchedule5.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending_ign6(void)
{
#if INJ_CHANNELS >= 6
    setIgnitionSchedule6(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule6.Status);
    while(ignitionSchedule6.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending_ign7(void)
{
#if INJ_CHANNELS >= 7
    setIgnitionSchedule7(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule7.Status);
    while(ignitionSchedule7.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending_ign8(void)
{
#if INJ_CHANNELS >= 8
    setIgnitionSchedule8(emptyCallback, 100, 100, emptyCallback);
    TEST_ASSERT_EQUAL(PENDING, ignitionSchedule8.Status);
    while(ignitionSchedule8.Status == PENDING) /*Wait*/ ;
#endif
}

void test_transit_status_pending(void)
{
    RUN_TEST(test_transit_status_pending_inj1);
    RUN_TEST(test_transit_status_pending_inj2);
    RUN_TEST(test_transit_status_pending_inj3);
    RUN_TEST(test_transit_status_pending_inj4);
    RUN_TEST(test_transit_status_pending_inj5);
    RUN_TEST(test_transit_status_pending_inj6);
    RUN_TEST(test_transit_status_pending_inj7);
    RUN_TEST(test_transit_status_pending_inj8);

    RUN_TEST(test_transit_status_pending_ign1);
    RUN_TEST(test_transit_status_pending_ign2);
    RUN_TEST(test_transit_status_pending_ign3);
    RUN_TEST(test_transit_status_pending_ign4);
    RUN_TEST(test_transit_status_pending_ign5);
    RUN_TEST(test_transit_status_pending_ign6);
    RUN_TEST(test_transit_status_pending_ign7);
    RUN_TEST(test_transit_status_pending_ign8);
}
