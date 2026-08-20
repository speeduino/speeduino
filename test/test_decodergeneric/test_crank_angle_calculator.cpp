#include "../test_utils.h"
#include "crank_angle_calculator.h"
#include "crankMaths.h"

static void test_last_tooth_rev_calculator_t(void)
{
    setAngleConverterRevolutionTime(360);

    last_tooth_rev_calculator_t subject;
    // No time elapsed -> zero degrees
    subject._toothLastToothTime = 1000;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(1000));

    // Small positive elapsed -> equals elapsed when revolutionTime == 360
    TEST_ASSERT_EQUAL_INT16(25, subject.calculate(1025));

    // Wrap-around case: start near UINT32_MAX, now small -> elapsed should wrap correctly
    subject._toothLastToothTime = UINT32_C(0xFFFFFFF0); // 4294967280
    uint32_t now = 20;
    // elapsed = 20 - 4294967280 (mod 2^32) = 36
    TEST_ASSERT_EQUAL_INT16(36, subject.calculate(now));
}

static void test_tooth_interval_calculator_t(void)
{
    tooth_interval_calculator_t subject;

    // Use 360us per revolution so timeToAngle(delta) == delta
    setAngleConverterRevolutionTime(360);

    // Basic setup: previous tooth 900, last tooth 1000 -> prevInterval = 100
    subject._toothLastMinusOneToothTime = 900;
    subject._toothLastToothTime = 1000;

    // Case A: _toothAngleCorrect == false -> use timeToAngle(elapsed)
    subject._toothAngleCorrect = false;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(1000));
    TEST_ASSERT_EQUAL_INT16(25, subject.calculate(1025));

    // Case B: _toothAngleCorrect == true -> interpolate: (elapsed * toothAngle) / prevInterval
    subject._toothAngleCorrect = true;
    subject._toothAngle = 36; // previous tooth angle
    // elapsed = 25 -> (25*36)/100 == 9 (integer division)
    TEST_ASSERT_EQUAL_INT16(9, subject.calculate(1025));

    // elapsed == prevInterval -> expect full toothAngle
    TEST_ASSERT_EQUAL_INT16(36, subject.calculate(1100));

    // toothAngle == 0 -> result 0
    subject._toothAngle = 0;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(1100));

    // Wrap-around: set times near UINT32 max and ensure modular arithmetic works
    subject._toothLastMinusOneToothTime = UINT32_C(0xFFFFFF00);
    subject._toothLastToothTime = subject._toothLastMinusOneToothTime + 100; // wraps modulo 2^32 as needed
    subject._toothAngle = 50;
    subject._toothAngleCorrect = true;
    // elapsed = 25 -> (25*50)/100 == 12
    uint32_t curr = subject._toothLastToothTime + 25;
    TEST_ASSERT_EQUAL_INT16((25 * 50) / 100, subject.calculate(curr));
}

static void test_lookup_initial_calculator_t(void)
{
    lookup_initial_calculator_t subject;

    int16_t toothAngles[] = { -33, 0, 33 };

    // Default tooth count = 0 -> should return 0 even if array provided
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(toothAngles));

    // Valid lookups (1-based indices)
    subject._toothCurrentCount = 1;
    TEST_ASSERT_EQUAL_INT16(-33, subject.calculate(toothAngles));

    subject._toothCurrentCount = 2;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(toothAngles));

    subject._toothCurrentCount = 3;
    TEST_ASSERT_EQUAL_INT16(33, subject.calculate(toothAngles));

    // Non-nullptr handling: if toothAngles is nullptr, always returns 0
    subject._toothCurrentCount = 2;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(nullptr));
}

static void test_compute_initial_calculator_t(void)
{
    compute_initial_calculator_t subject;

    // Default count 0 -> calculate() == 0
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate());

    // toothCount = 1 -> (1-1) * toothAngle == 0
    subject._toothCurrentCount = 1;
    subject._toothAngle = 36;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate());

    // toothCount = 2 -> (2-1) * toothAngle == toothAngle
    subject._toothCurrentCount = 2;
    TEST_ASSERT_EQUAL_INT16(36, subject.calculate());

    // toothCount = 5 -> (5-1) * toothAngle == 4 * toothAngle
    subject._toothCurrentCount = 5;
    TEST_ASSERT_EQUAL_INT16(4 * 36, subject.calculate());

    // toothAngle == 0 -> always 0
    subject._toothAngle = 0;
    subject._toothCurrentCount = 10;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate());
}

static void test_sequential_correction_calculator_t(void)
{
    sequential_correction_calculator_t subject;

    config4 page4;

    // Default: not in second revolution -> always 0
    subject._revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(page4));

    page4.TrigSpeed = CRANK_SPEED;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(page4));

    // When in second revolution and using CRANK_SPEED, returns 360
    subject._revZeroOrOne = true;
    page4.TrigSpeed = CAM_SPEED;
    TEST_ASSERT_EQUAL_INT16(0, subject.calculate(page4));

    page4.TrigSpeed = CRANK_SPEED;
    TEST_ASSERT_EQUAL_INT16(360, subject.calculate(page4));
}

static void test_simple_crank_angle_calculator_t(void)
{
    setAngleConverterRevolutionTime(360);

    simple_crank_angle_calculator_t subject;
    config4 page4;
    page4.triggerAngle = 10;

    subject._toothLastToothTime = 1000;
    subject._revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;
    // initial + trigger + delta
    TEST_ASSERT_EQUAL_INT16(5 + 10 + 20, subject.calculate(5, subject._toothLastToothTime + 20, page4));

    // when sequential and CRANK_SPEED, adds 360
    subject._revZeroOrOne = true;
    page4.TrigSpeed = CRANK_SPEED;
    TEST_ASSERT_EQUAL_INT16(5 + 10 + 20 + 360, subject.calculate(5, subject._toothLastToothTime + 20, page4));
}

static void test_lookup_crank_angle_calculator_t(void)
{
    setAngleConverterRevolutionTime(360);

    lookup_crank_angle_calculator_t subject;
    config4 page4;
    page4.triggerAngle = 7;
    subject._toothLastToothTime = 1000;
    subject._toothCurrentCount = 2;
    subject._revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;
    int16_t toothAngles[] = { 1, 2, 3 };

    // lookup (toothAngles[1]=2) + trigger + delta
    TEST_ASSERT_EQUAL_INT16(2 + 7 + 15, subject.calculate(subject._toothLastToothTime + 15, toothAngles, page4));
}

static void test_trigger_angle_crank_angle_calculator_t(void)
{
    setAngleConverterRevolutionTime(360);

    trigger_angle_crank_angle_calculator_t subject;
    config4 page4;
    page4.triggerAngle = 9;
    subject._toothLastToothTime = 2000;
    subject._toothCurrentCount = 3;
    subject._toothAngle = 20; // compute initial = (3-1)*20 = 40
    subject._revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;

    TEST_ASSERT_EQUAL_INT16(40 + 9 + 30, subject.calculate(subject._toothLastToothTime + 30, page4));
}

static void test_lookup_crank_angle_calculator_tooth_interval_t(void)
{
    setAngleConverterRevolutionTime(360);

    lookup_crank_angle_calculator_tooth_interval_t subject;
    config4 page4;
    page4.triggerAngle = 4;
    subject._toothLastMinusOneToothTime = 900;
    subject._toothLastToothTime = 1000; // prev interval = 100
    subject._toothCurrentCount = 1;
    subject._toothAngle = 40;
    subject._toothAngleCorrect = true;
    subject._revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;

    // lookup tooth 1 -> toothAngles[0]
    int16_t toothAngles[] = { 5, 6 };
    // elapsed 20 -> (20*40)/100 = 8
    TEST_ASSERT_EQUAL_INT16(5 + 4 + 8, subject.calculate(subject._toothLastToothTime + 20, toothAngles, page4));
}

static void test_compute_crank_angle_calculator_tooth_interval_t(void)
{
    setAngleConverterRevolutionTime(360);

    compute_crank_angle_calculator_tooth_interval_t subject;
    config4 page4;
    page4.triggerAngle = 6;
    subject._toothLastMinusOneToothTime = 1000;
    subject._toothLastToothTime = 1100; // prev interval = 100
    subject._toothCurrentCount = 3;
    subject.compute_initial_calculator_t::_toothAngle = 30; // compute initial = (3-1)*30 = 60
    subject.tooth_interval_calculator_t::_toothAngle = subject.compute_initial_calculator_t::_toothAngle;
    subject._toothAngleCorrect = true;
    subject._revZeroOrOne = false;
    page4.TrigSpeed = CAM_SPEED;

    // elapsed 10 -> (10*30)/100 = 3
    TEST_ASSERT_EQUAL_INT16(60 + 6 + 3, subject.calculate(subject._toothLastToothTime + 10, page4));
}

void testCrankAngleCalculators(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_last_tooth_rev_calculator_t);
    RUN_TEST_P(test_tooth_interval_calculator_t);
    RUN_TEST_P(test_lookup_initial_calculator_t);
    RUN_TEST_P(test_compute_initial_calculator_t);
    RUN_TEST_P(test_sequential_correction_calculator_t);
    RUN_TEST_P(test_simple_crank_angle_calculator_t);
    RUN_TEST_P(test_lookup_crank_angle_calculator_t);
    RUN_TEST_P(test_trigger_angle_crank_angle_calculator_t);
    RUN_TEST_P(test_lookup_crank_angle_calculator_tooth_interval_t);
    RUN_TEST_P(test_compute_crank_angle_calculator_tooth_interval_t);
  }
}

