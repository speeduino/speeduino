#include "../test_utils.h"
#include "programmableIOControl_internals.h"

using namespace programmableIOControl_details;

static int16_t mockGetData(uint16_t index)
{
    return (int16_t)(index * 10 + 1);
}

static void test_combine_and(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_AND;

    TEST_ASSERT_FALSE(rule.evaluateCombineOp(false, true));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(true, false));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(false, false));
    TEST_ASSERT_TRUE(rule.evaluateCombineOp(true, true));
}

static void test_combine_or(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_OR;

    TEST_ASSERT_TRUE(rule.evaluateCombineOp(false, true));
    TEST_ASSERT_TRUE(rule.evaluateCombineOp(true, false));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(false, false));
    TEST_ASSERT_TRUE(rule.evaluateCombineOp(true, true));
}

static void test_combine_xor(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_XOR;

    TEST_ASSERT_TRUE(rule.evaluateCombineOp(false, true));
    TEST_ASSERT_TRUE(rule.evaluateCombineOp(true, false));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(false, false));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(true, true));
}
 
static void test_combine_disabled(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_DISABLED; // Invalid combine type should return false

    TEST_ASSERT_FALSE(rule.evaluateCombineOp(false, true));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(true, false));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(false, false));
    TEST_ASSERT_FALSE(rule.evaluateCombineOp(true, true));
}

static void test_evaluate_first_op_only_true(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_DISABLED;
    rule.firstOp = { COMPARATOR_EQUAL, 0, 1 };

    TEST_ASSERT_TRUE(rule.evaluate(state_t{}, mockGetData));
}

static void test_evaluate_first_op_only_false(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_DISABLED;
    rule.firstOp = { COMPARATOR_LESS, 0, 1 };

    TEST_ASSERT_FALSE(rule.evaluate(state_t{}, mockGetData));
}

static void test_evaluate_combine_and(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_AND;
    rule.firstOp = { COMPARATOR_EQUAL, 0, 1 };
    rule.secondOp = { COMPARATOR_GREATER, 1, 5 };

    TEST_ASSERT_TRUE(rule.evaluate(state_t{}, mockGetData));
}

static void test_evaluate_combine_or(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_OR;
    rule.firstOp = { COMPARATOR_LESS, 0, 1 };
    rule.secondOp = { COMPARATOR_EQUAL, 1, 11 };

    TEST_ASSERT_TRUE(rule.evaluate(state_t{}, mockGetData));
}

static void test_evaluate_combine_xor(void)
{
    rule_t rule{};
    rule.combineOpType = COMBINE_XOR;
    rule.firstOp = { COMPARATOR_EQUAL, 0, 1 };
    rule.secondOp = { COMPARATOR_EQUAL, 1, 11 };

    TEST_ASSERT_FALSE(rule.evaluate(state_t{}, mockGetData));
}

void testRule(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_combine_and);
        RUN_TEST_P(test_combine_or);
        RUN_TEST_P(test_combine_xor);
        RUN_TEST_P(test_combine_disabled);
        RUN_TEST_P(test_evaluate_first_op_only_true);
        RUN_TEST_P(test_evaluate_first_op_only_false);
        RUN_TEST_P(test_evaluate_combine_and);
        RUN_TEST_P(test_evaluate_combine_or);
        RUN_TEST_P(test_evaluate_combine_xor);
    }
}