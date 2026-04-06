#include <unity.h>
#include "src/PID/integerPID_ideal.h"
#include "../test_utils.h"

constexpr uint32_t NOW = 10000UL;

static void test_p_only_clamped_to_min(void)
{
    long input = 100;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.SetTunings(PidTuningParameters(1, 0, 0), PidDirection::Direct); // P-only
    pid.setSampleTime(NOW, 0);
    pid.setTargetValue(200);
    pid.setSensitivity(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(2000u, output); // min clamp 20% *100
}

static void test_p_only_clamped_to_max(void)
{
    long input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.SetTunings(PidTuningParameters(100, 0, 0), PidDirection::Direct); // P-only
    pid.setSampleTime(NOW, 0);
    pid.setTargetValue(1000);
    pid.setSensitivity(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(8000u, output); // max clamp 80% *100
}

static void test_sample_time_gate(void)
{
    long input = 500;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setSampleTime(NOW, 250);
    pid.setTargetValue(500);
    pid.setSensitivity(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    // Running immediately again should be gated by sample time (likely false)
    TEST_ASSERT_FALSE(pid.Compute(NOW, input, &output));
}

static void test_ki_windup_limits(void)
{
    long input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.SetTunings(PidTuningParameters(10, 10, 0), PidDirection::Direct);
    pid.setSampleTime(NOW, 0);
    pid.SetOutputLimits(20, 80);
    pid.setTargetValue(1000);
    pid.setSensitivity(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    // output should stay within [2000, 8000] scale after clamping
    TEST_ASSERT_LESS_OR_EQUAL(8000u, output);
    TEST_ASSERT_GREATER_OR_EQUAL(2000u, output);
}

static void test_reverse_direction(void)
{
    long input = 500;
    uint16_t output = 0;

    integerPID_ideal pidDirect;
    pidDirect.SetTunings(PidTuningParameters(50, 0, 0), PidDirection::Direct);
    pidDirect.setSampleTime(NOW, 0);
    pidDirect.SetOutputLimits(0, 5000);
    pidDirect.setTargetValue(1000);
    pidDirect.setSensitivity(1);
    pidDirect.Initialize(input);

    TEST_ASSERT_TRUE(pidDirect.Compute(NOW, input, &output));
    uint16_t directOutput = output;

    integerPID_ideal pidReverse;
    pidReverse.SetTunings(PidTuningParameters(50, 0, 0), PidDirection::Reverse);
    pidReverse.setSampleTime(NOW, 0);
    pidReverse.SetOutputLimits(0, 5000);
    pidReverse.setTargetValue(1000);
    pidReverse.setSensitivity(1);
    pidReverse.Initialize(input);
    TEST_ASSERT_TRUE(pidReverse.Compute(NOW, input, &output));
    uint16_t reverseOutput = output;

    TEST_ASSERT_LESS_THAN(directOutput, reverseOutput);
}

static void test_feedforward_applied(void)
{
    long input = 500;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.setSampleTime(NOW, 0);
    pid.setTargetValue(500);
    pid.setSensitivity(50);
    pid.Initialize(input);

    pid.setFeedForwardTerm(5000);
    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(5000u, output); // no PID action, output equals feedforward (above min clamp)
}

static void test_set_output_limits_invalid_bounds_are_ignored(void)
{
    long input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.SetTunings(PidTuningParameters(100, 0, 0), PidDirection::Direct);
    pid.SetOutputLimits(80, 20);
    pid.setTargetValue(1000);
    pid.setSensitivity(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    TEST_ASSERT_EQUAL(8000u, output); // invalid limits ignored, default max clamp remains 80%
}

static void test_initialize_resets_integral_and_error(void)
{
    long input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.SetTunings(PidTuningParameters(1, 10, 0), PidDirection::Direct);
    pid.SetOutputLimits(0, 100);
    pid.setTargetValue(500);
    pid.setSampleTime(NOW, 1);
    pid.setSensitivity(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));

    input = 1000;
    pid.Initialize(input);
    TEST_ASSERT_TRUE(pid.Compute(NOW + 1U, input, &output));
    TEST_ASSERT_EQUAL(0u, output); // with zero error and reset integral, output should return to zero
}

static void test_derivative_term_changes_output_on_error_transition(void)
{
    long input = 0;
    uint16_t output = 0;

    integerPID_ideal pid;
    pid.SetTunings(PidTuningParameters(1, 0, 1), PidDirection::Direct);
    pid.SetOutputLimits(0, 100);
    pid.setSampleTime(NOW, 1);
    pid.setTargetValue(1000);
    pid.setSensitivity(50);
    pid.setTargetValue(50);
    pid.Initialize(input);

    TEST_ASSERT_TRUE(pid.Compute(NOW, input, &output));
    uint16_t firstOutput = output;

    input = 100;
    TEST_ASSERT_TRUE(pid.Compute(NOW + 1U, input, &output));
    TEST_ASSERT_NOT_EQUAL_UINT16(firstOutput, output);
}

static String createIterationMsg(int16_t iteration, uint16_t input, uint16_t output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRIu16 ", %" PRIu16, iteration, input, output);
    return szMsg;
}

// Run the PID for 50 and confirm it hits the setpoint
static void assert_pid_complete(integerPID_ideal &pid, long input, uint16_t setpoint, uint8_t sampleTime)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, input, setpoint).c_str()); UNITY_PRINT_EOL();

    uint16_t output;
    for (uint16_t iteration=0; iteration<50U; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute(NOW+(iteration*sampleTime), input, &output));
        UnityPrint(createIterationMsg(iteration, input, output).c_str()); UNITY_PRINT_EOL();
        input = output;
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(abs(DIV_ROUND_CLOSEST(setpoint, 100, int32_t)), setpoint, input);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr long START_POINT = 30;
    constexpr uint8_t SAMPLE_TIME = 25;
    constexpr uint16_t SET_POINT = 90;

    integerPID_ideal pid;
    pid.setTargetValue(SET_POINT);
    pid.setSampleTime(10000, SAMPLE_TIME);
    pid.SetTunings(PidTuningParameters(3, 2, 1), PidDirection::Direct);
    pid.SetOutputLimits(0, 255);
    pid.setSensitivity(0);
    pid.setFeedForwardTerm(7);
    pid.Initialize(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, SAMPLE_TIME);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr long START_POINT = 90;
    constexpr uint8_t SAMPLE_TIME = 25;
    constexpr uint16_t SET_POINT = 30;

    integerPID_ideal pid;
    pid.setTargetValue(SET_POINT);
    pid.setSampleTime(10000, SAMPLE_TIME);
    pid.SetTunings(PidTuningParameters(3, 2, 1), PidDirection::Direct);
    pid.SetOutputLimits(0, 255);
    pid.setSensitivity(0);
    pid.setFeedForwardTerm(11);
    pid.Initialize(START_POINT);

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