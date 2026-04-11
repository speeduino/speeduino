#include <unity.h>
#include "src/PID/PID.h"
#include "../test_utils.h"

static void test_pid_mode_transitions_and_controller_direction(void)
{
    long input = 50;

    PID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0));
    pid.setOutputLimits(-200, 200);
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    TEST_ASSERT_EQUAL(12, pid.compute(input)); // 255*50/1000 integer scaling yields 12

    pid.setTunings(PidTuningParameters(255, 0, 0) * -1);
    TEST_ASSERT_LESS_THAN(0, pid.compute(input)); // sign changed
}

static void test_pid_output_limits_in_auto_scope(void)
{
    long input = 0;
    
    PID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0));
    pid.setSetPoint(500);
    pid.setOutputLimits(0, 100); // inAuto path updates output and ITerm
    pid.resetIntegral(input);

    TEST_ASSERT_LESS_OR_EQUAL(100, pid.compute(input));
}

static void test_pid_auto_mode_proportional(void)
{
    long input = 0;
    
    PID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0));
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    TEST_ASSERT_EQUAL(25, pid.compute(input));  // 255 * 100 / 1000 = 25

    input = 100;
    TEST_ASSERT_EQUAL(0, pid.compute(input));
}

static void test_pid_output_limits_clamp(void)
{
    long input = 0;
    
    PID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0));
    pid.setSetPoint(1000); // results in max output for Kp=255
    pid.setOutputLimits(0, 255);
    pid.resetIntegral(input);

    TEST_ASSERT_EQUAL(255, pid.compute(input));
}

static void test_pid_reverse_direction(void)
{
    long input = 0;
    
    PID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0) * -1);
    pid.setSetPoint(1000);
    pid.setOutputLimits(-255, 255);
    pid.resetIntegral(input);

    TEST_ASSERT_EQUAL(-255, pid.compute(input));
}

static void test_pid_set_output_limits_long_range(void)
{
    long input = 0;
    
    PID pid;
    pid.setTunings(PidTuningParameters(255, 0, 0));
    pid.setSetPoint(1000);
    pid.resetIntegral(0);

    pid.setOutputLimits(0, 10);
    TEST_ASSERT_EQUAL(10, pid.compute(input));

    pid.setOutputLimits(-20, 20);
    input = 1000;
    pid.setSetPoint(0);
    TEST_ASSERT_EQUAL(-20, pid.compute(input));

    pid.setOutputLimits(100, 200);
    input = 0;
    TEST_ASSERT_EQUAL(100, pid.compute(input));

    pid.setOutputLimits(150, 150); // invalid min==max should be ignored
    input = 0;
    TEST_ASSERT_EQUAL(100, pid.compute(input)); // remains from previous in-range bounds
}

static void test_pid_integral_accumulation(void)
{
    long input = 0;
    
    PID pid;
    pid.setTunings(PidTuningParameters(0, 10, 0));
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    TEST_ASSERT_EQUAL(1, pid.compute(input));

    TEST_ASSERT_EQUAL(2, pid.compute(input));
}

static void test_pid_set_tunings_runtime_changes(void)
{
    long input = 0;

    PID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0));
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    long originalOutput = pid.compute(input);

    pid.setTunings(PidTuningParameters(20, 0, 0)); // Change Kp
    // Output should change due to new Kp
    TEST_ASSERT_NOT_EQUAL(originalOutput, pid.compute(input));
}

static void test_pid_initialize_resets_state(void)
{
    long input = 0;

    PID pid;
    pid.setTunings(PidTuningParameters(0, 10, 0));
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    long initialCorrection = pid.compute(input);
    TEST_ASSERT_NOT_EQUAL_INT32(0, initialCorrection); // Accumulate some integral
    TEST_ASSERT_NOT_EQUAL_INT32(initialCorrection, pid.compute(input)); // Accumulate some integral

    pid.resetIntegral(input); // Reset state
    // After reset, output should be based on new state (integral reset)
    TEST_ASSERT_EQUAL_INT32(initialCorrection, pid.compute(input)); // Should be the initial output
}

static void test_pid_set_output_limits_invalid_ignored(void)
{
    long input = 0;

    PID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0));
    pid.setOutputLimits(50, 20); // Invalid: Min >= Max
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    // Output should still be computed normally (limits ignored, default 0-255)
    TEST_ASSERT_NOT_EQUAL(0, pid.compute(input));
}

static void test_pid_set_controller_direction_runtime_manual(void)
{
    long input = 0;

    PID pid;
    pid.setTunings(PidTuningParameters(10, 0, 0)*-1); // Should not affect in manual mode
    pid.setOutputLimits(-25, 25);
    pid.setSetPoint(100);
    pid.resetIntegral(input);

    TEST_ASSERT_LESS_THAN(0, pid.compute(input)); // Reverse direction normally produces negative output
}

static String createIterationMsg(int16_t iteration, long input, long output)
{
    char szMsg[64];
    snprintf(szMsg, _countof(szMsg)-1, "%" PRId16 ", %" PRId32 ", %" PRId32, iteration, (int32_t)input, (int32_t)output);
    return szMsg;
}

// Run the PID for maxIterations and confirm it hits the setpoint
static void assert_pid_complete(PID &pid, long input, long setpoint, uint16_t maxIterations)
{
    UnityPrint("Iter,Input,Output"); UNITY_PRINT_EOL();
    UnityPrint(createIterationMsg(-1, input, setpoint).c_str()); UNITY_PRINT_EOL();

    long output;
    for (uint16_t iteration=0; iteration<maxIterations; ++iteration)
    {
        input += pid.compute(input);

        UnityPrint(createIterationMsg(iteration, input, output).c_str()); UNITY_PRINT_EOL();
    }
    // Tolerance of 1%
    TEST_ASSERT_INT32_WITHIN(abs(DIV_ROUND_CLOSEST(setpoint, 100, int32_t)), setpoint, input);
}

static void test_end_to_end_positive_positive_up(void) 
{
    constexpr long START_POINT = 900;
    constexpr long SET_POINT = 1500;

    PID pid;
    pid.setSetPoint(SET_POINT);
    pid.setTunings(PidTuningParameters(100, 30, 50));
    pid.setOutputLimits(-25, 25);
    pid.resetIntegral(START_POINT);
    
    assert_pid_complete(pid, START_POINT, SET_POINT, 50);
}

static void test_end_to_end_positive_positive_down(void) 
{
    constexpr long START_POINT = 1250;
    constexpr long SET_POINT = 900;

    PID pid;
    pid.setSetPoint(SET_POINT);
    pid.setTunings(PidTuningParameters(100, 5, 5));
    pid.setOutputLimits(-75, 75);
    pid.resetIntegral(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, 100);
}

static void test_end_to_end_negative_negative_up(void) 
{
    constexpr long START_POINT = -1500;
    constexpr long SET_POINT = -900;

    PID pid;
    pid.setTunings(PidTuningParameters(100, 25, 2));
    pid.setOutputLimits(-255, 255);
    pid.setSetPoint(SET_POINT);
    pid.resetIntegral(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, 50);
}

static void test_end_to_end_negative_negative_down(void) 
{
    constexpr long START_POINT = -900;
    constexpr long SET_POINT = -1500;

    PID pid;
    pid.setSetPoint(SET_POINT);
    pid.setTunings(PidTuningParameters(100, 30, 50));
    pid.setOutputLimits(-25, 25);
    pid.resetIntegral(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, 50);
}

static void test_end_to_end_negative_positive(void) 
{
    constexpr long START_POINT = -199;
    constexpr long SET_POINT = 199;

    PID pid;
    pid.setSetPoint(SET_POINT);
    pid.setTunings(PidTuningParameters(50, 1, 80));
    pid.setOutputLimits(-255, 255);
    pid.resetIntegral(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, 50);
}

static void test_end_to_end_positive_to_negative(void) 
{
    constexpr long START_POINT = 900;
    constexpr long SET_POINT = -1500;

    PID pid;
    pid.setTunings(PidTuningParameters(100, 30, 20));
    pid.setOutputLimits(-255, 255);
    pid.setSetPoint(SET_POINT);
    pid.resetIntegral(START_POINT);

    assert_pid_complete(pid, START_POINT, SET_POINT, 51);
}

void testPID(void)
{
    SET_UNITY_FILENAME() {
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