#include "../test_utils.h"
#include "programmableIOControl_internals.h"

using namespace programmableIOControl_details;

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
    }
}