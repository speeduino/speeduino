#include "../test_utils.h"
#include "src/pwm/PwmOutputChannel.h"

static void test_ctor(void)
{
    PwmOutputChannel subject(19, 1000);

    TEST_ASSERT_TRUE(subject.pin.isValid());
    TEST_ASSERT_NOT_EQUAL(0, subject.maxDuty);
}

static void test_setTargetDuty(void)
{
    PwmOutputChannel subject(19, 1000);

    // Test 0% duty
    subject.setTargetDuty(0);
    TEST_ASSERT_EQUAL(0, subject.targetDuty);
    TEST_ASSERT_TRUE(subject.pin.isPinLow());

    // Test 100% duty
    subject.setTargetDuty(200);
    TEST_ASSERT_EQUAL(subject.maxDuty, subject.targetDuty);
    TEST_ASSERT_TRUE(subject.pin.isPinHigh());

    // Test 50% duty
    auto isPinHigh = subject.pin.isPinHigh();
    subject.setTargetDuty(100);
    TEST_ASSERT_EQUAL(subject.maxDuty / 2, subject.targetDuty);
    TEST_ASSERT_EQUAL(isPinHigh, subject.pin.isPinHigh());
}

void testPwmOutputChannel(void)
{
  SET_UNITY_FILENAME()
  {
    RUN_TEST_P(test_ctor);
    RUN_TEST_P(test_setTargetDuty);
  }
}