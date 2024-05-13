#include <unity.h>
#include "table3d.h"
#include "../test_utils.h"

static uint16_t sumAxis(table_axis_iterator it) {
    uint16_t sum = 0;
    auto sumOperation = [](table_axis_iterator &it, void* pSum) { *(uint16_t*)pSum += *it; };
    for_each(it, sumOperation, &sum);
    return sum;
}

template <typename T>
static void test_axis_for_each(T axis) {
    // Set
    auto setOperation = [](table_axis_iterator &it, void*) { *it = 5; };
    for_each(axis.begin(), setOperation, nullptr);
    TEST_ASSERT_EQUAL(axis.length*5, sumAxis(axis.begin()));

    // Mutate
    auto mutateOperation = [](table_axis_iterator &it, void*) { *it = *it - 3; };
    for_each(axis.begin(), mutateOperation, nullptr);
    TEST_ASSERT_EQUAL(axis.length*(5-3), sumAxis(axis.begin()));
}

static void test_yaxis_for_each(void) {
    table3d4RpmLoad subject;
    test_axis_for_each(subject.axisY);
}

static void test_xaxis_for_each(void) {
    table3d16RpmLoad subject;
    test_axis_for_each(subject.axisX);
}

static uint16_t sumTable(table_value_iterator it) {
    uint16_t sum = 0;
    auto sumOperation = [](table_row_iterator &it, void* pSum) { *(uint16_t*)pSum += *it; };
    for_each(it, sumOperation, &sum);
    return sum;
}

static void test_values_for_each(void) {
    table3d16RpmLoad subject;

    // Set
    auto setOperation = [](table_row_iterator &it, void*) { *it = 5; };
    for_each(subject.values.begin(), setOperation, nullptr);
    TEST_ASSERT_EQUAL(subject.axisX.length*subject.axisY.length*5, sumTable(subject.values.begin()));

    // Mutate
    auto mutateOperation = [](table_row_iterator &it, void*) { *it = *it - 3; };
    for_each(subject.values.begin(), mutateOperation, nullptr);
    TEST_ASSERT_EQUAL(subject.axisX.length*subject.axisY.length*(5-3), sumTable(subject.values.begin()));
}


void testAlgorithms3D(void) {
    SET_UNITY_FILENAME() {
        RUN_TEST(test_yaxis_for_each);
        RUN_TEST(test_xaxis_for_each);
        RUN_TEST(test_values_for_each);
    }
}