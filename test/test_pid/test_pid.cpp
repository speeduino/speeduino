#include <unity.h>
#include "src/PID/PID.h"
#include "../test_utils.h"

static void test_pid_mode_transitions_and_controller_direction(void)
{
    long input = 50;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct);
    pid.SetOutputLimits(-200, 200);
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(12, output); // 255*50/1000 integer scaling yields 12

    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Reverse);
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_LESS_THAN(0, output); // sign changed
}

static void test_pid_output_limits_in_auto_scope(void)
{
    long input = 0;
    long output = 100;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct);
    pid.setTargetValue(500);
    pid.SetOutputLimits(0, 100); // inAuto path updates output and ITerm
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_LESS_OR_EQUAL(100, output);
}

static void test_pid_manual_mode_compute_false(void)
{
    long input = 0;
    long output = 42;

    PID pid(&input, &output);
    pid.setTargetValue(100);
    pid.SetTunings(PidTuningParameters(100, 0, 0), PidDirection::Direct);

    TEST_ASSERT_FALSE(pid.Compute());
    TEST_ASSERT_EQUAL(42, output);
}

static void test_pid_auto_mode_proportional(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct);
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(25, output);  // 255 * 100 / 1000 = 25

    input = 100;
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(0, output);
}

static void test_pid_output_limits_clamp(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct);
    pid.setTargetValue(1000); // results in max output for Kp=255
    pid.SetOutputLimits(0, 255);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(255, output);
}

static void test_pid_reverse_direction(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Reverse);
    pid.setTargetValue(1000);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(-255, output);
}

static void test_pid_set_output_limits_long_range(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(255, 0, 0), PidDirection::Direct);
    pid.setTargetValue(1000);
    pid.activate();

    pid.SetOutputLimits(0, 10);
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(10, output);

    pid.SetOutputLimits(-20, 20);
    input = 1000;
    pid.setTargetValue(0);
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(-20, output);

    pid.SetOutputLimits(100, 200);
    input = 0;
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(100, output);

    pid.SetOutputLimits(150, 150); // invalid min==max should be ignored
    input = 0;
    pid.Initialize();
    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(100, output); // remains from previous in-range bounds
}

static void test_pid_integral_accumulation(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(0, 10, 0), PidDirection::Direct);
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(1, output);

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_EQUAL(2, output);
}

static void test_pid_set_tunings_runtime_changes(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct);
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    long originalOutput = output;

    pid.SetTunings(PidTuningParameters(20, 0, 0), PidDirection::Direct); // Change Kp
    TEST_ASSERT_TRUE(pid.Compute());
    // Output should change due to new Kp
    TEST_ASSERT_NOT_EQUAL(originalOutput, output);
}

static void test_pid_initialize_resets_state(void)
{
    long input = 0;
    long output = 50;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(0, 10, 0), PidDirection::Direct);
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute()); // Accumulate some integral

    pid.Initialize(); // Reset state
    TEST_ASSERT_TRUE(pid.Compute());
    // After reset, output should be based on new state (integral reset)
    TEST_ASSERT_NOT_EQUAL(50, output); // Should not be the initial output
}

static void test_pid_set_output_limits_invalid_ignored(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Direct);
    pid.SetOutputLimits(50, 20); // Invalid: Min >= Max
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    // Output should still be computed normally (limits ignored, default 0-255)
    TEST_ASSERT_NOT_EQUAL(0, output);
}

static void test_pid_set_controller_direction_runtime_manual(void)
{
    long input = 0;
    long output = 0;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(10, 0, 0), PidDirection::Reverse); // Should not affect in manual mode
    pid.SetOutputLimits(-25, 25);
    pid.setTargetValue(100);
    pid.activate();

    TEST_ASSERT_TRUE(pid.Compute());
    TEST_ASSERT_LESS_THAN(0, output); // Reverse direction normally produces negative output
}

static String createIterationMsg(int16_t iteration, long input, long output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)input, (int32_t)output);
    return szMsg;
}

// Run the PID for maxIterations and confirm it hits the setpoint
static void assert_pid_complete(PID &pid, long *pInput, long *pOutput, long setpoint, uint16_t maxIterations)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "Start: %" PRId32 ", SetPoint: %" PRId32, (int32_t)*pInput, (int32_t)setpoint);
    UnityPrint(szMsg); UNITY_PRINT_EOL();

    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, *pInput, setpoint).c_str()); UNITY_PRINT_EOL();

    for (uint16_t iteration=0; iteration<maxIterations; ++iteration)
    {
        TEST_ASSERT_TRUE(pid.Compute());
        *pInput += *pOutput;

        UnityPrint(createIterationMsg(iteration, *pInput, *pOutput).c_str()); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(abs(DIV_ROUND_CLOSEST(setpoint, 100, int32_t)), setpoint, *pInput);
}

static void test_end_to_end_positive_positive_up(void) 
{
    long output = 0;
    long input = 900;
    constexpr long SET_POINT = 1500;

    PID pid(&input, &output);
    pid.setTargetValue(SET_POINT);
    pid.SetTunings(PidTuningParameters(100, 30, 50), PidDirection::Direct);
    pid.SetOutputLimits(-25, 25);
    pid.activate();
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, SET_POINT, 50);
}

static void test_end_to_end_positive_positive_down(void) 
{
    long output = 0;
    long input = 1250;
    constexpr long SET_POINT = 900;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(100, 5, 5), PidDirection::Direct);
    pid.SetOutputLimits(-75, 75);
    pid.setTargetValue(SET_POINT);
    pid.activate();
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, SET_POINT, 100);
}

static void test_end_to_end_negative_negative_up(void) 
{
    long output = 0;
    long input = -1500;
    constexpr long SET_POINT = -900;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(100, 25, 2), PidDirection::Direct);
    pid.SetOutputLimits(-255, 255);
    pid.setTargetValue(SET_POINT);
    pid.activate();

    assert_pid_complete(pid, &input, &output, SET_POINT, 50);
}

static void test_end_to_end_negative_negative_down(void) 
{
    long output = 0;
    long input = -900;
    constexpr long SET_POINT = -1500;

    PID pid(&input, &output);
    pid.setTargetValue(SET_POINT);
    pid.SetTunings(PidTuningParameters(100, 30, 50), PidDirection::Direct);
    pid.SetOutputLimits(-25, 25);
    pid.activate();
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, SET_POINT, 50);
}

static void test_end_to_end_negative_positive(void) 
{
    long output = 0;
    long input = -199;
    constexpr long SET_POINT = 199;

    PID pid(&input, &output);
    pid.setTargetValue(SET_POINT);
    pid.SetTunings(PidTuningParameters(50, 1, 80), PidDirection::Direct);
    pid.SetOutputLimits(-255, 255);
    pid.activate();

    assert_pid_complete(pid, &input, &output, SET_POINT, 50);
}

static void test_end_to_end_positive_to_negative(void) 
{
    long output = 0;
    long input = 900;
    constexpr long SET_POINT = -1500;

    PID pid(&input, &output);
    pid.SetTunings(PidTuningParameters(100, 30, 20), PidDirection::Direct);
    pid.SetOutputLimits(-255, 255);
    pid.setTargetValue(SET_POINT);
    pid.activate();
    pid.Initialize();

    assert_pid_complete(pid, &input, &output, SET_POINT, 50);
}

void testPID(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_pid_manual_mode_compute_false);
        RUN_TEST_P(test_pid_auto_mode_proportional);
        RUN_TEST_P(test_pid_output_limits_clamp);
        RUN_TEST_P(test_pid_reverse_direction);
        RUN_TEST_P(test_pid_integral_accumulation);
        RUN_TEST_P(test_pid_output_limits_in_auto_scope);
        RUN_TEST_P(test_pid_mode_transitions_and_controller_direction);
        RUN_TEST_P(test_pid_set_output_limits_long_range);
        RUN_TEST_P(test_pid_set_tunings_runtime_changes);
        RUN_TEST_P(test_pid_initialize_resets_state);
        RUN_TEST_P(test_pid_set_output_limits_invalid_ignored);
        RUN_TEST_P(test_pid_set_controller_direction_runtime_manual);
        RUN_TEST_P(test_end_to_end_positive_positive_up);
        RUN_TEST_P(test_end_to_end_positive_positive_down);
        RUN_TEST_P(test_end_to_end_negative_negative_up);
        RUN_TEST_P(test_end_to_end_negative_negative_down);
        RUN_TEST_P(test_end_to_end_negative_positive);
        RUN_TEST_P(test_end_to_end_positive_to_negative);
    }
}