#include "../test_utils.h"
#include "src/controllers/aircon/airconController_details.h"

static void test_AfterEngineStartDelay(void)
{
    auto subject = airConController::details::state_t();
    config15 page15;
    page15.airConAfterStartDelay = 5;

    TEST_ASSERT_EQUAL(0, subject.afterEngineStartDelay);
    TEST_ASSERT_FALSE(subject.afterEngineStartDelayExpired(page15));

    for (uint8_t index=subject.afterEngineStartDelay; index<page15.airConAfterStartDelay-1; ++index)
    {
        TEST_ASSERT_FALSE(subject.nextAfterEngineStartDelay(page15));
        TEST_ASSERT_EQUAL(index+1, subject.afterEngineStartDelay);
        TEST_ASSERT_FALSE(subject.afterEngineStartDelayExpired(page15));
    }

    TEST_ASSERT_TRUE(subject.nextAfterEngineStartDelay(page15));
    TEST_ASSERT_TRUE(subject.afterEngineStartDelayExpired(page15));

    subject.resetAfterEngineStartDelay();
    TEST_ASSERT_EQUAL(0, subject.afterEngineStartDelay);
}

static void test_StartDelay(void)
{
    auto subject = airConController::details::state_t();
    config15 page15;
    page15.airConCompOnDelay = 5;

    for (uint8_t index=0; index<page15.airConCompOnDelay; ++index)
    {
        TEST_ASSERT_EQUAL(index, subject.startDelay);
        TEST_ASSERT_FALSE(subject.nextStartDelay(page15));
        TEST_ASSERT_EQUAL(index+1, subject.startDelay);
    }
    for (uint8_t index=0; index<5; ++index)
    {
        TEST_ASSERT_TRUE(subject.nextStartDelay(page15));
        TEST_ASSERT_EQUAL(page15.airConCompOnDelay+1, subject.startDelay);
    }

    subject.resetStartDelay();
    TEST_ASSERT_EQUAL(0, subject.startDelay);
}

static void test_TpsLockOutDelay(void)
{
    auto subject = airConController::details::state_t();
    config15 page15;
    page15.airConTPSCutTime = 5;

    for (uint8_t index=0; index<page15.airConTPSCutTime; ++index)
    {
        TEST_ASSERT_EQUAL(index, subject.tpsLockoutDelay);
        TEST_ASSERT_FALSE(subject.nextTpsLockoutDelay(page15));
        TEST_ASSERT_EQUAL(index+1, subject.tpsLockoutDelay);
    }
    for (uint8_t index=0; index<5; ++index)
    {
        TEST_ASSERT_TRUE(subject.nextTpsLockoutDelay(page15));
        TEST_ASSERT_EQUAL(page15.airConTPSCutTime+1, subject.tpsLockoutDelay);
    }

    subject.resetTpsLockoutDelay();
    TEST_ASSERT_EQUAL(0, subject.tpsLockoutDelay);
}

static void test_RpmLockOutDelay(void)
{
    auto subject = airConController::details::state_t();
    config15 page15;
    page15.airConRPMCutTime = 5;

    for (uint8_t index=0; index<page15.airConRPMCutTime; ++index)
    {
        TEST_ASSERT_EQUAL(index, subject.rpmLockoutDelay);
        TEST_ASSERT_FALSE(subject.nextRpmLockoutDelay(page15));
        TEST_ASSERT_EQUAL(index+1, subject.rpmLockoutDelay);
    }
    for (uint8_t index=0; index<5; ++index)
    {
        TEST_ASSERT_TRUE(subject.nextRpmLockoutDelay(page15));
        TEST_ASSERT_EQUAL(page15.airConRPMCutTime+1, subject.rpmLockoutDelay);
    }

    subject.resetRpmLockoutDelay();
    TEST_ASSERT_EQUAL(0, subject.rpmLockoutDelay);
}

void testAcControlDetails(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_AfterEngineStartDelay);
    RUN_TEST_P(test_StartDelay);
    RUN_TEST_P(test_TpsLockOutDelay);
    RUN_TEST_P(test_RpmLockOutDelay);
  }
}