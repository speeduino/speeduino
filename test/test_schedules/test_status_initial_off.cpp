
#include <Arduino.h>
#include <unity.h>

#include "scheduler.h"

void test_status_initial_off_inj1(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule1.Status);
}

void test_status_initial_off_inj2(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule2.Status);
}

void test_status_initial_off_inj3(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule3.Status);
}

void test_status_initial_off_inj4(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule4.Status);
}

void test_status_initial_off_inj5(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule5.Status);
}

void test_status_initial_off_inj6(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule6.Status);
}

void test_status_initial_off_inj7(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule7.Status);
}

void test_status_initial_off_inj8(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, fuelSchedule8.Status);
}


void test_status_initial_off_ign1(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[0].Status);
}

void test_status_initial_off_ign2(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[1].Status);
}

void test_status_initial_off_ign3(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[2].Status);
}

void test_status_initial_off_ign4(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[3].Status);
}

void test_status_initial_off_ign5(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[4].Status);
}

void test_status_initial_off_ign6(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[5].Status);
}

void test_status_initial_off_ign7(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[6].Status);
}

void test_status_initial_off_ign8(void)
{
    initialiseSchedulers();
    TEST_ASSERT_EQUAL(OFF, ignitionSchedule[7].Status);
}

void test_status_initial_off(void)
{
    RUN_TEST(test_status_initial_off_inj1);
    RUN_TEST(test_status_initial_off_inj2);
    RUN_TEST(test_status_initial_off_inj3);
    RUN_TEST(test_status_initial_off_inj4);
    RUN_TEST(test_status_initial_off_inj5);
    RUN_TEST(test_status_initial_off_inj6);
    RUN_TEST(test_status_initial_off_inj7);
    RUN_TEST(test_status_initial_off_inj8);

    RUN_TEST(test_status_initial_off_ign1);
    RUN_TEST(test_status_initial_off_ign2);
    RUN_TEST(test_status_initial_off_ign3);
    RUN_TEST(test_status_initial_off_ign4);
    RUN_TEST(test_status_initial_off_ign5);
    RUN_TEST(test_status_initial_off_ign6);
    RUN_TEST(test_status_initial_off_ign7);
    RUN_TEST(test_status_initial_off_ign8);
}