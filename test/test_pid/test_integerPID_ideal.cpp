#include <unity.h>
#include "src/PID/integerPID_ideal.h"
#include "../test_utils.h"

constexpr uint32_t NOW = 10000UL;

static void test_p_only_clamped_to_min(void)
{
    uint16_t input = 100;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setTunings(PidTuningParameters(1, 0, 0)); // P-only
    pid.setSampleTime(NOW, 0);
    pid.setSetPoint(200);
    pid.setSensitivity(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(2000u, output); // min clamp 20% *100
}

static void test_p_only_clamped_to_max(void)
{
    uint16_t input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setTunings(PidTuningParameters(100, 0, 0)); // P-only
    pid.setSampleTime(NOW, 0);
    pid.setSetPoint(1000);
    pid.setSensitivity(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(8000u, output); // max clamp 80% *100
}

static void test_sample_time_gate(void)
{
    uint16_t input = 500;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setSampleTime(NOW, 250);
    pid.setSetPoint(500);
    pid.setSensitivity(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // Running immediately again should be gated by sample time (likely false)
    TEST_ASSERT_FALSE(pid.compute(NOW, input, &output));
}

static void test_ki_windup_limits(void)
{
    uint16_t input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setTunings(PidTuningParameters(10, 10, 0));
    pid.setSampleTime(NOW, 0);
    pid.setOutputLimits(20, 80);
    pid.setSetPoint(1000);
    pid.setSensitivity(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    // output should stay within [2000, 8000] scale after clamping
    TEST_ASSERT_LESS_OR_EQUAL(8000u, output);
    TEST_ASSERT_GREATER_OR_EQUAL(2000u, output);
}

static void test_reverse_direction(void)
{
    uint16_t input = 500;
    uint16_t output = 0;

    integerPID_ideal pidDirect;
    pidDirect.setTunings(PidTuningParameters(50, 0, 0));
    pidDirect.setSampleTime(NOW, 0);
    pidDirect.setOutputLimits(0, 255);
    pidDirect.setSetPoint(1000);
    pidDirect.setSensitivity(1);
    pidDirect.initialize(input);

    TEST_ASSERT_TRUE(pidDirect.compute(NOW, input, &output));
    uint16_t directOutput = output;

    integerPID_ideal pidReverse;
    pidReverse.setTunings(PidTuningParameters(50, 0, 0) * -1);
    pidReverse.setSampleTime(NOW, 0);
    pidReverse.setOutputLimits(0, 255);
    pidReverse.setSetPoint(1000);
    pidReverse.setSensitivity(1);
    pidReverse.initialize(input);
    TEST_ASSERT_TRUE(pidReverse.compute(NOW, input, &output));
    uint16_t reverseOutput = output;

    TEST_ASSERT_LESS_THAN(directOutput, reverseOutput);
}

static void test_feedforward_applied(void)
{
    uint16_t input = 500;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setSampleTime(NOW, 0);
    pid.setSetPoint(500);
    pid.setSensitivity(50);
    pid.setFeedForwardTerm(5000);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(5000u, output); // no PID action, output equals feedforward (above min clamp)
}

static void test_set_output_limits_invalid_bounds_are_ignored(void)
{
    uint16_t input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setTunings(PidTuningParameters(100, 0, 0));
    pid.setOutputLimits(80, 20);
    pid.setSetPoint(1000);
    pid.setSensitivity(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(8000u, output); // invalid limits ignored, default max clamp remains 80%
}

static void test_initialize_resets_integral_and_error(void)
{
    uint16_t input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setTunings(PidTuningParameters(1, 10, 0));
    pid.setOutputLimits(0, 100);
    pid.setSetPoint(500);
    pid.setSampleTime(NOW, 1);
    pid.setSensitivity(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));

    input = 1000;
    pid.initialize(input);
    TEST_ASSERT_TRUE(pid.compute(NOW + 1U, input, &output));
    TEST_ASSERT_EQUAL(0u, output); // with zero error and reset integral, output should return to zero
}

static void test_derivative_term_changes_output_on_error_transition(void)
{
    uint16_t input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setTunings(PidTuningParameters(1, 0, 1));
    pid.setOutputLimits(0, 100);
    pid.setSampleTime(NOW, 1);
    pid.setSetPoint(1000);
    pid.setSensitivity(50);
    pid.setSetPoint(50);
    pid.initialize(input);

    TEST_ASSERT_TRUE(pid.compute(NOW, input, &output));
    uint16_t firstOutput = output;

    input = 100;
    TEST_ASSERT_TRUE(pid.compute(NOW + 1U, input, &output));
    TEST_ASSERT_NOT_EQUAL_UINT16(firstOutput, output);
}

static String createIterationMsg(int16_t iteration, uint16_t input, uint16_t output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRIu16 ", %" PRIu16, iteration, input, output);
    return szMsg;
}

// Run the PID for 50 and confirm it hits the setpoint
static void assert_pid_complete(integerPID_ideal &pid, uint16_t input, uint16_t setpoint, uint8_t sampleTime)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, input, setpoint).c_str()); UNITY_PRINT_EOL();

    uint16_t output;
    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.compute(NOW+(iteration*sampleTime), input, &output));
        UnityPrint(createIterationMsg(iteration, input, output).c_str()); UNITY_PRINT_EOL();
        input = output;
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(abs(DIV_ROUND_CLOSEST(setpoint, 100, int32_t)), setpoint, input);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr uint16_t START_POINT = 30;
    constexpr uint8_t SAMPLE_TIME = 25;
    constexpr uint16_t SET_POINT = 90;

    integerPID_ideal pid;
    pid.setSetPoint(SET_POINT);
    pid.setSampleTime(NOW, SAMPLE_TIME);
    pid.setTunings(PidTuningParameters(3, 2, 1));
    pid.setOutputLimits(0, 255);
    pid.setSensitivity(0);
    pid.setFeedForwardTerm(7);
    pid.initialize(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr uint16_t START_POINT = 90;
    constexpr uint8_t SAMPLE_TIME = 25;
    constexpr uint16_t SET_POINT = 30;

    integerPID_ideal pid;
    pid.setSetPoint(SET_POINT);
    pid.setSampleTime(NOW, SAMPLE_TIME);
    pid.setTunings(PidTuningParameters(3, 2, 1));
    pid.setOutputLimits(0, 255);
    pid.setSensitivity(0);
    pid.setFeedForwardTerm(11);
    pid.initialize(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

void testIntegerPID_ideal(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_p_only_clamped_to_min);
        RUN_TEST_P(test_p_only_clamped_to_max);
        RUN_TEST_P(test_sample_time_gate);
        RUN_TEST_P(test_ki_windup_limits);
        RUN_TEST_P(test_reverse_direction);
        RUN_TEST_P(test_feedforward_applied);
        RUN_TEST_P(test_set_output_limits_invalid_bounds_are_ignored);
        RUN_TEST_P(test_initialize_resets_integral_and_error);
        RUN_TEST_P(test_derivative_term_changes_output_on_error_transition);
        RUN_TEST_P(test_end_to_end_positive_positive_up);
        RUN_TEST_P(test_end_to_end_positive_positive_down);
        // Following do not make sense for integerPID_ideal since input & output are uint16_t
        // RUN_TEST_P(test_end_to_end_negative_negative_up);
        // RUN_TEST_P(test_end_to_end_negative_negative_down);
        // RUN_TEST_P(test_end_to_end_negative_positive);
        // RUN_TEST_P(test_end_to_end_positive_to_negative);       
    }
}