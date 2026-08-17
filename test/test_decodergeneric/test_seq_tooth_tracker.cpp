#include "../test_utils.h"
#include "decoder_tooth_tracker.h"
#include "crankMaths.h"

static void test_calculateCrankAngle_whichRev(void)
{
    setAngleConverterRevolutionTime(360);

    config4 page4;
    seq_tooth_tracker_t subject;
    page4.triggerAngle = 33;

    subject.toothLastToothTime = 1111;
    subject.toothCurrentCount = 3;
    subject.revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;
    int16_t initial = subject.calculateCrankAngle(subject.toothLastToothTime, 55, page4);
    TEST_ASSERT_LESS_THAN(360, initial);
    TEST_ASSERT_GREATER_THAN(0, initial);

    subject.revZeroOrOne = true;
    page4.TrigSpeed = CAM_SPEED;
    TEST_ASSERT_EQUAL(initial, subject.calculateCrankAngle(subject.toothLastToothTime, 55, page4));

    subject.revZeroOrOne = true;
    page4.TrigSpeed = CRANK_SPEED;
    TEST_ASSERT_EQUAL(initial+360, subject.calculateCrankAngle(subject.toothLastToothTime, 55, page4));
}

static void test_calculateCrankAngle_triggerangle_coverage(void)
{
    int16_t angles[] = { 33, 37, 41 };
    setAngleConverterRevolutionTime(360);

    config4 page4;
    seq_tooth_tracker_t subject;
    page4.triggerAngle = 33;

    subject.toothLastToothTime = 1111;
    subject.toothCurrentCount = 2;
    subject.revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;
    TEST_ASSERT_EQUAL(70, subject.calculateCrankAngle(subject.toothLastToothTime, angles, page4));
}

void testSequentialToothTracker(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateCrankAngle_whichRev);
    RUN_TEST_P(test_calculateCrankAngle_triggerangle_coverage);
  }
}