#include "../test_utils.h"
#include "programmableIOControl_internals.h"

using namespace programmableIOControl_details;

static int16_t mockGetData(uint16_t index)
{
    return (int16_t)(index * 10 + 1);
}

static void test_equality(void)
{
    compOperation_t opEqual{ COMPARATOR_EQUAL, 1, 2  };
    TEST_ASSERT_TRUE(opEqual.evaluate(5, 5));
    TEST_ASSERT_FALSE(opEqual.evaluate(5, 6));
    TEST_ASSERT_FALSE(opEqual.evaluate(5, 4));
}

static void test_inequality(void)
{
    compOperation_t opNotEqual{ COMPARATOR_NOT_EQUAL, 1, 2  };
    TEST_ASSERT_FALSE(opNotEqual.evaluate(5, 5));
    TEST_ASSERT_TRUE(opNotEqual.evaluate(5, 6));
    TEST_ASSERT_TRUE(opNotEqual.evaluate(5, 4));
}

static void test_greater_than(void)
{
    compOperation_t opGreater{ COMPARATOR_GREATER, 1, 2 };
    TEST_ASSERT_TRUE(opGreater.evaluate(6, 5));
    TEST_ASSERT_FALSE(opGreater.evaluate(5, 6));
    TEST_ASSERT_FALSE(opGreater.evaluate(5, 5));
}

static void test_greater_than_or_equal(void)
{
    compOperation_t opGreaterEqual{ COMPARATOR_GREATER_EQUAL, 1, 2 };
    TEST_ASSERT_TRUE(opGreaterEqual.evaluate(6, 5));
    TEST_ASSERT_TRUE(opGreaterEqual.evaluate(5, 5));
    TEST_ASSERT_FALSE(opGreaterEqual.evaluate(4, 5));
}

static void test_less_than(void)
{
    compOperation_t opLess{ COMPARATOR_LESS, 1, 2 };
    TEST_ASSERT_TRUE(opLess.evaluate(4, 5));
    TEST_ASSERT_FALSE(opLess.evaluate(5, 4));
    TEST_ASSERT_FALSE(opLess.evaluate(5, 5));
}

static void test_less_than_or_equal(void)
{
    compOperation_t opLessEqual{ COMPARATOR_LESS_EQUAL, 1, 2 };
    TEST_ASSERT_FALSE(opLessEqual.evaluate(6, 5));
    TEST_ASSERT_TRUE(opLessEqual.evaluate(5, 5));
    TEST_ASSERT_TRUE(opLessEqual.evaluate(4, 5));
}

static void test_and(void)
{
    compOperation_t opAnd{ COMPARATOR_AND, 1, 2 };
    TEST_ASSERT_TRUE(opAnd.evaluate(0b0101, 0b0001));
    TEST_ASSERT_FALSE(opAnd.evaluate(0b0101, 0b0010));
}

static void test_xor(void)
{
    compOperation_t opXor{ COMPARATOR_XOR, 1, 2 };
    TEST_ASSERT_TRUE(opXor.evaluate(0b0101, 0b0001));
    TEST_ASSERT_FALSE(opXor.evaluate(0b0101, 0b0101));
}

static void test_invalid_comparator(void)
{
    compOperation_t opInvalid{ 211, 1, 2 }; // Invalid comparator type
    TEST_ASSERT_FALSE(opInvalid.evaluate(5, 3));
}

static void test_getComparisonData_direct_data_lookup(void)
{
    compOperation_t op{ COMPARATOR_EQUAL, 5, 0 };
    state_t state{};

    TEST_ASSERT_EQUAL_INT16(51, op.getComparisonData(state, mockGetData));
}

static void test_getComparisonData_virtual_state_channel(void)
{
    compOperation_t op{ COMPARATOR_EQUAL, static_cast<uint8_t>(REUSE_RULES + 2), 0 };
    state_t state{};
    state.channels[2].isRuleActive = true;

    TEST_ASSERT_EQUAL_INT16(1, op.getComparisonData(state, mockGetData));
}

static void test_getComparisonData_virtual_state_channel_out_of_range(void)
{
    state_t state{};
    const uint8_t invalidIndex = static_cast<uint8_t>(REUSE_RULES + _countof(state.channels));
    compOperation_t op{ COMPARATOR_EQUAL, invalidIndex, 0 };

    TEST_ASSERT_EQUAL_INT16(0, op.getComparisonData(state, mockGetData));
}

void testCompOperation(void) 
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_equality);
        RUN_TEST_P(test_inequality);
        RUN_TEST_P(test_greater_than);
        RUN_TEST_P(test_greater_than_or_equal);
        RUN_TEST_P(test_less_than);
        RUN_TEST_P(test_less_than_or_equal);
        RUN_TEST_P(test_and);
        RUN_TEST_P(test_xor);
        RUN_TEST_P(test_invalid_comparator);
        RUN_TEST_P(test_getComparisonData_direct_data_lookup);
        RUN_TEST_P(test_getComparisonData_virtual_state_channel);
        RUN_TEST_P(test_getComparisonData_virtual_state_channel_out_of_range);
    }
}