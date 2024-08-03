#include <unity.h>
#include <stdio.h>
#include "table2d.h"
#include "../test_utils.h"


static constexpr uint8_t TEST_TABLE2D_SIZE = 9;
static uint8_t table2d_data_u8[TEST_TABLE2D_SIZE] = {
    251, 211, 199, 167, 127, 101, 59, 23, 5
};
static uint16_t table2d_data_u16[TEST_TABLE2D_SIZE] = {
    32029, 26357, 21323, 16363, 11329, 7537, 5531, 2539, 1237
};
static int16_t table2d_data_i16[TEST_TABLE2D_SIZE] = {
    32029, 21323, 11329, 5531, 1237, -5531, -11329, -21323, -32029
};

static uint8_t table2d_axis_u8[TEST_TABLE2D_SIZE] {
    5, 23, 59, 101, 127, 167, 199, 211, 251,
};
static int8_t table2d_axis_i8[TEST_TABLE2D_SIZE] {
    -127, -101, -59, -5, 5, 23, 59, 101, 127
};
static uint16_t table2d_axis_u16[TEST_TABLE2D_SIZE] = {
    123, 2539, 5531, 7537, 11329, 16363, 21323, 26357, 32029,
};

static table2D table2d_u8_u8;
static table2D table2d_u8_u16;
static table2D table2d_u16_u8;
static table2D table2d_u16_u16;
static table2D table2d_u8_i8;
static table2D table2d_i16_u8;

static void setup_test_subjects(void)
{
    construct2dTable(table2d_u8_u8, table2d_data_u8, table2d_axis_u8);
    construct2dTable(table2d_u8_u16, table2d_data_u8, table2d_axis_u16);
    construct2dTable(table2d_u16_u8, table2d_data_u16, table2d_axis_u8);
    construct2dTable(table2d_u16_u16, table2d_data_u16, table2d_axis_u16);
    construct2dTable(table2d_u8_i8, table2d_data_u8, table2d_axis_i8);
    construct2dTable(table2d_i16_u8, table2d_data_i16, table2d_axis_u8);
}

template <typename TValue, typename TAxis>
static void test_table2dLookup(table2D &table, TValue *data, TAxis *axis, uint8_t binFrac)
{
    for (uint8_t i=0; i<TEST_TABLE2D_SIZE-1U; ++i) {
        TAxis lookupValue = intermediate(axis[i], axis[i+1], binFrac);
        TValue expected = map(lookupValue, axis[i], axis[i+1], data[i], data[i+1]);
        TValue result = (TValue)table2D_getValue(&table, lookupValue);
        char szMsg[128];
        sprintf(szMsg, "Loop: %d, VT %d, AT %d lookup: %d, data[i]: %d, data[i+1]: %d, binFrac %d", i, table.valueType, table.axisType, lookupValue, data[i], data[i+1], binFrac);
        TEST_ASSERT_INT_WITHIN_MESSAGE(1U, expected, result, szMsg);
        TEST_ASSERT_EQUAL(i+1, table.cache.lastBinUpperIndex);
    }
}

static void test_table2dLookup_bin_midpoints(void)
{
    setup_test_subjects();

    test_table2dLookup(table2d_u8_u8, table2d_data_u8, table2d_axis_u8, 50U);
    test_table2dLookup(table2d_u8_u16, table2d_data_u8, table2d_axis_u16, 50U);
    test_table2dLookup(table2d_u16_u8, table2d_data_u16, table2d_axis_u8, 50U);
    test_table2dLookup(table2d_u16_u16, table2d_data_u16, table2d_axis_u16, 50U);
    test_table2dLookup(table2d_u8_i8, table2d_data_u8, table2d_axis_i8, 50U);
    test_table2dLookup(table2d_i16_u8, table2d_data_i16, table2d_axis_u8, 50U);    
}

static void test_table2dLookup_bin_33(void)
{
    setup_test_subjects();

    test_table2dLookup(table2d_u8_u8, table2d_data_u8, table2d_axis_u8, 33U);
    test_table2dLookup(table2d_u8_u16, table2d_data_u8, table2d_axis_u16, 33U);
    test_table2dLookup(table2d_u16_u8, table2d_data_u16, table2d_axis_u8, 33U);
    test_table2dLookup(table2d_u16_u16, table2d_data_u16, table2d_axis_u16, 33U);
    test_table2dLookup(table2d_u8_i8, table2d_data_u8, table2d_axis_i8, 33U);
    test_table2dLookup(table2d_i16_u8, table2d_data_i16, table2d_axis_u8, 33U);    
}

static void test_table2dLookup_bin_66(void)
{
    setup_test_subjects();

    test_table2dLookup(table2d_u8_u8, table2d_data_u8, table2d_axis_u8, 66U);
    test_table2dLookup(table2d_u8_u16, table2d_data_u8, table2d_axis_u16, 66U);
    test_table2dLookup(table2d_u16_u8, table2d_data_u16, table2d_axis_u8, 66U);
    test_table2dLookup(table2d_u16_u16, table2d_data_u16, table2d_axis_u16, 66U);
    test_table2dLookup(table2d_u8_i8, table2d_data_u8, table2d_axis_i8, 66U);
    test_table2dLookup(table2d_i16_u8, table2d_data_i16, table2d_axis_u8, 66U);    
}

template <typename TValue, typename TAxis>
static void test_table2dLookup_bin_edges(table2D &table, TValue *data, TAxis *axis) {
    for (uint8_t i=0; i<TEST_TABLE2D_SIZE; ++i) {
        TValue result = (TValue)table2D_getValue(&table, axis[i]);
        char szMsg[64];
        sprintf(szMsg, "%d, %d lookup %d: %d", table.valueType, table.axisType, i, axis[i]);
        TEST_ASSERT_EQUAL_MESSAGE(data[i], result, szMsg);
        // sprintf(szMsg, "Index %d, lastBinUpperIndex: %d", i, table.lastBinUpperIndex);
        // TEST_MESSAGE(szMsg);
        // TEST_ASSERT_EQUAL_MESSAGE((i==0 || i==1) ? 1 : i+1, table.lastBinUpperIndex, szMsg);
    }
}

static void test_table2dLookup_bin_edges(void)
{
    setup_test_subjects();

    test_table2dLookup_bin_edges(table2d_u8_u8, table2d_data_u8, table2d_axis_u8);
    test_table2dLookup_bin_edges(table2d_u8_u16, table2d_data_u8, table2d_axis_u16);
    test_table2dLookup_bin_edges(table2d_u16_u8, table2d_data_u16, table2d_axis_u8);
    test_table2dLookup_bin_edges(table2d_u16_u16, table2d_data_u16, table2d_axis_u16);
    test_table2dLookup_bin_edges(table2d_u8_i8, table2d_data_u8, table2d_axis_i8);
    test_table2dLookup_bin_edges(table2d_i16_u8, table2d_data_i16, table2d_axis_u8);   
}

template <typename TValue, typename TAxis>
static void test_table2dLookup_overMax(table2D &table, TValue *data, TAxis *axis) {
    TValue result = (TValue)table2D_getValue(&table, axis[TEST_TABLE2D_SIZE-1]+1);
    TEST_ASSERT_EQUAL(data[TEST_TABLE2D_SIZE-1], result);
}

static void test_table2dLookup_overMax(void)
{
    setup_test_subjects();

    test_table2dLookup_overMax(table2d_u8_u8, table2d_data_u8, table2d_axis_u8);
    test_table2dLookup_overMax(table2d_u8_u16, table2d_data_u8, table2d_axis_u16);
    test_table2dLookup_overMax(table2d_u16_u8, table2d_data_u16, table2d_axis_u8);
    test_table2dLookup_overMax(table2d_u16_u16, table2d_data_u16, table2d_axis_u16);
    test_table2dLookup_overMax(table2d_u8_i8, table2d_data_u8, table2d_axis_i8);
    test_table2dLookup_overMax(table2d_i16_u8, table2d_data_i16, table2d_axis_u8); 
}

template <typename TValue, typename TAxis>
static void test_table2dLookup_underMin(table2D &table, TValue *data, TAxis *axis) {
    TValue result = (TValue)table2D_getValue(&table, axis[0]-1);
    TEST_ASSERT_EQUAL(data[0], result);
}

static void test_table2dLookup_underMin(void)
{
    setup_test_subjects();

    test_table2dLookup_underMin(table2d_u8_u8, table2d_data_u8, table2d_axis_u8);
    test_table2dLookup_underMin(table2d_u8_u16, table2d_data_u8, table2d_axis_u16);
    test_table2dLookup_underMin(table2d_u16_u8, table2d_data_u16, table2d_axis_u8);
    test_table2dLookup_underMin(table2d_u16_u16, table2d_data_u16, table2d_axis_u16);
    test_table2dLookup_underMin(table2d_u8_i8, table2d_data_u8, table2d_axis_i8);
    test_table2dLookup_underMin(table2d_i16_u8, table2d_data_i16, table2d_axis_u8);      
}


static void test_table2d_all_decrementing(void)
{
    setup_test_subjects();

    uint8_t u8_u8_result_last = UINT8_MAX;
    for (uint8_t loop=table2d_axis_u8[0]; loop<=table2d_axis_u8[TEST_TABLE2D_SIZE-1]; ++loop)
    {
        uint8_t u8_u8_result = table2D_getValue(&table2d_u8_u8, loop);
        TEST_ASSERT_LESS_OR_EQUAL(u8_u8_result_last, u8_u8_result);
        u8_u8_result_last = u8_u8_result;
    }
}


#include "../timer.hpp"


static void test_lookup_perf(void) {
    uint16_t iters = 1000;
    uint8_t start_index = 0;
    uint8_t end_index = TEST_TABLE2D_SIZE-1U;
    uint8_t step = 1;

    timer timerA;
    uint32_t paramA = 0;
    auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
        checkSum += table2D_getValue(&table2d_u8_u8, (uint8_t)(table2d_axis_u8[index]+1));
    };
    measure_executiontime<uint8_t, uint32_t&>(iters, start_index, end_index, step, timerA, paramA, nativeTest);

    // The checksums will be different due to rounding. This is only
    // here to force the compiler to run the loops above
    TEST_ASSERT_INT32_WITHIN(UINT32_MAX/2, UINT32_MAX/2, paramA);

    char buffer[128];
    sprintf(buffer, "Timing: %" PRIu32, timerA.duration_micros());
    TEST_MESSAGE(buffer);

}

void testTable2d()
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_table2dLookup_overMax);
    RUN_TEST(test_table2dLookup_underMin);
    RUN_TEST(test_table2d_all_decrementing); 
    RUN_TEST(test_table2dLookup_bin_midpoints);
    RUN_TEST(test_table2dLookup_bin_33);
    RUN_TEST(test_table2dLookup_bin_66);
    RUN_TEST(test_table2dLookup_bin_edges);
    RUN_TEST(test_lookup_perf);
  }
}