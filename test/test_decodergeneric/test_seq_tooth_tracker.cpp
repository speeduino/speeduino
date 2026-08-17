#include "../test_utils.h"
#include "decoder_tooth_tracker.h"
#include "crankMaths.h"


static void test_calculateCrankAngle_zeroToothCount(void)
{
    int16_t angles[2];
    setAngleConverterRevolutionTime(1);
    config4 page4;
    page4.triggerAngle = 33;

    seq_tooth_tracker_t subject;
    TEST_ASSERT_EQUAL(0, subject.toothCurrentCount);
    TEST_ASSERT_EQUAL(page4.triggerAngle, subject.calculateCrankAngle(1000, nullptr, page4));
    TEST_ASSERT_EQUAL(page4.triggerAngle, subject.calculateCrankAngle(1000, angles, page4));
    TEST_ASSERT_EQUAL(page4.triggerAngle, subject.calculateCrankAngle(1000, -55, page4));
}

static void assert_calculateCrankAngle_triggerangle(uint16_t toothCount, uint32_t timeDelta, int16_t triggerAngle, int16_t toothAngle)
{
    setAngleConverterRevolutionTime(360);

    config4 page4;
    seq_tooth_tracker_t subject;
    page4.triggerAngle = triggerAngle;

    subject.toothLastToothTime = 1111;
    subject.toothCurrentCount = toothCount;
    TEST_ASSERT_EQUAL_INT16((toothAngle * (toothCount-1)) + triggerAngle + timeDelta, subject.calculateCrankAngle(subject.toothLastToothTime+timeDelta, toothAngle, page4));
}

static void test_calculateCrankAngle_triggerangle(void)
{
    for (auto tooth: {1, 2, 3})
    {
        for (auto timeDelta: {-100, 0, 100})
        {
            for (auto trigAngle: {-55, 0, 55} )
            {
                for (auto toothAngle: {-33, 0, 33} )
                {
                    assert_calculateCrankAngle_triggerangle(tooth, timeDelta, trigAngle, toothAngle);
                }
            }
        }
    }
}

static void assert_calculateCrankAngle_lookup(uint16_t toothNum, int16_t toothAngles[], uint32_t timeDelta, int16_t triggerAngle)
{
    setAngleConverterRevolutionTime(360);

    config4 page4;
    seq_tooth_tracker_t subject;
    page4.triggerAngle = triggerAngle;

    subject.toothLastToothTime = 1111;
    subject.toothCurrentCount = toothNum;
    TEST_ASSERT_EQUAL_INT16(toothAngles[toothNum-1] + triggerAngle + timeDelta, subject.calculateCrankAngle(subject.toothLastToothTime+timeDelta, toothAngles, page4));
}

static void assert_calculateCrankAngle_lookup(uint32_t timeDelta, int16_t triggerAngle)
{
    int16_t toothAngles[] = { -33, 0, 33 };
    assert_calculateCrankAngle_lookup(1, toothAngles, timeDelta, triggerAngle);
    assert_calculateCrankAngle_lookup(2, toothAngles, timeDelta, triggerAngle);
    assert_calculateCrankAngle_lookup(3, toothAngles, timeDelta, triggerAngle);
}

static void test_calculateCrankAngle_lookup(void)
{
    for (auto timeDelta: {-100, 0, 100})
    {
        for (auto trigAngle: {-55, 0, 55} )
        {
            assert_calculateCrankAngle_lookup(timeDelta, trigAngle);
        }
    }
}

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

void testSequentialToothTracker(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calculateCrankAngle_zeroToothCount);
    RUN_TEST_P(test_calculateCrankAngle_triggerangle);
    RUN_TEST_P(test_calculateCrankAngle_lookup);
    RUN_TEST_P(test_calculateCrankAngle_whichRev);
  }
}