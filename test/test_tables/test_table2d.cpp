#include <unity.h>
#include <stdio.h>
#include "table2d.h"
#include "../test_utils.h"


static uint8_t table2d_data_u8[] = {
    251, 211, 199, 167, 127, 101, 59, 23, 5
};
static uint16_t table2d_data_u16[] = {
    32029, 26357, 21323, 16363, 11329, 7537, 5531, 2539, 1237
};
static int16_t table2d_data_i16[] = {
    32029, 21323, 11329, 5531, 1237, -5531, -11329, -21323, -32029
};

static uint8_t table2d_axis_u8[] {
    5, 23, 59, 101, 127, 167, 199, 211, 251,
};
static int8_t table2d_axis_i8[] {
    -125, -101, -59, -5, 5, 23, 59, 101, 125
};
static uint16_t table2d_axis_u16[] = {
    123, 2539, 5531, 7537, 11329, 16363, 21323, 26357, 32029,
};

static table2D<uint8_t, uint8_t, _countof(table2d_data_u8)> table2d_u8_u8(&table2d_axis_u8, &table2d_data_u8);
static table2D<uint16_t, uint8_t, _countof(table2d_data_u8)> table2d_u8_u16(&table2d_axis_u16, &table2d_data_u8);
static table2D<uint8_t, uint16_t, _countof(table2d_data_u16)> table2d_u16_u8(&table2d_axis_u8, &table2d_data_u16);
static table2D<uint16_t, uint16_t, _countof(table2d_data_u16)> table2d_u16_u16(&table2d_axis_u16, &table2d_data_u16);
static table2D<int8_t, uint8_t, _countof(table2d_data_u8)> table2d_u8_i8(&table2d_axis_i8, &table2d_data_u8);
static table2D<uint8_t, int16_t, _countof(table2d_data_i16)> table2d_i16_u8(&table2d_axis_u8, &table2d_data_i16);

namespace internal
{
  static const unsigned int FRONT_SIZE = sizeof("static const char* internal::GetTypeNameHelper<T>::GetTypeName() [with T = ") - 1u;
  static const unsigned int BACK_SIZE = sizeof("]") - 1u;
 
  template <typename T>
  struct GetTypeNameHelper
  {
    static const char* GetTypeName(void)
    {
      static const size_t size = sizeof(__PRETTY_FUNCTION__) - FRONT_SIZE - BACK_SIZE;
      static char typeName[size] = {};
      memcpy(typeName, __PRETTY_FUNCTION__ + FRONT_SIZE, size - 1u);
 
      return typeName;
    }
  };
}
 
 
template <typename T>
const char* GetTypeName(void)
{
  return internal::GetTypeNameHelper<T>::GetTypeName();
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_table2dLookup(const table2D<axis_t, value_t, sizeT> &table, uint8_t binFrac)
{
    for (uint8_t i=0; i<sizeT-1U; ++i) {
        axis_t lookupValue = intermediate(table.axis[i], table.axis[i+1], binFrac);
        value_t expected = map(lookupValue, table.axis[i], table.axis[i+1], table.values[i], table.values[i+1]);
        value_t result = table2D_getValue(&table, lookupValue);
        char szMsg[512];
        sprintf(szMsg, "Loop: %d, VT %s, AT %s, lookup: %d, data[i]: %d, data[i+1]: %d, binFrac %d", (int)i, GetTypeName<value_t>(), GetTypeName<axis_t>(), (int)lookupValue, (int)table.values[i], (int)table.values[i+1], (int)binFrac);
        TEST_ASSERT_INT_WITHIN_MESSAGE(1U, expected, result, szMsg);
        TEST_ASSERT_EQUAL(i+1, table.cache.lastBinUpperIndex);
    }
}

static void test_table2dLookup_bin_midpoints(void)
{
    test_table2dLookup(table2d_u8_u8, 50U);
    test_table2dLookup(table2d_u8_u16, 50U);
    test_table2dLookup(table2d_u16_u8, 50U);
    test_table2dLookup(table2d_u16_u16, 50U);
    test_table2dLookup(table2d_u8_i8, 50U);
    test_table2dLookup(table2d_i16_u8, 50U);    
}

static void test_table2dLookup_bin_33(void)
{
    test_table2dLookup(table2d_u8_u8, 33U);
    test_table2dLookup(table2d_u8_u16, 33U);
    test_table2dLookup(table2d_u16_u8, 33U);
    test_table2dLookup(table2d_u16_u16, 33U);
    test_table2dLookup(table2d_u8_i8, 33U);
    test_table2dLookup(table2d_i16_u8, 33U);    
}

static void test_table2dLookup_bin_66(void)
{
    test_table2dLookup(table2d_u8_u8, 66U);
    test_table2dLookup(table2d_u8_u16, 66U);
    test_table2dLookup(table2d_u16_u8, 66U);
    test_table2dLookup(table2d_u16_u16, 66U);
    test_table2dLookup(table2d_u8_i8, 66U);
    test_table2dLookup(table2d_i16_u8, 66U);    
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_table2dLookup_bin_edges(const table2D<axis_t, value_t, sizeT> &table) {
    for (uint8_t i=0; i<sizeT; ++i) {
        value_t result = (value_t)table2D_getValue(&table, table.axis[i]);
        // char szMsg[64];
        // sprintf(szMsg, "%d, %d lookup %d: %d", table.values.typeIndicator, table.axis.typeIndicator, i, axis[i]);
        TEST_ASSERT_EQUAL(table.values[i], result);
        // sprintf(szMsg, "%d, %d lookup %d", table.values.typeIndicator, table.axis.typeIndicator, i);
        TEST_ASSERT_EQUAL(i==0  ? 1 : i, table.cache.lastBinUpperIndex);
    }
}

static void test_table2dLookup_bin_edges(void)
{
    test_table2dLookup_bin_edges(table2d_u8_u8);
    test_table2dLookup_bin_edges(table2d_u8_u16);
    test_table2dLookup_bin_edges(table2d_u16_u8);
    test_table2dLookup_bin_edges(table2d_u16_u16);
    test_table2dLookup_bin_edges(table2d_u8_i8);
    test_table2dLookup_bin_edges(table2d_i16_u8);   
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_table2dLookup_overMax(const table2D<axis_t, value_t, sizeT> &table) {
    axis_t lookupValue =(axis_t)(table.axis[sizeT-1]+1);
    value_t result = table2D_getValue(&table, lookupValue);
    char szMsg[256];
    sprintf(szMsg, "VT %s, AT %s lookup: %d, data[sizeT-1]: %d", GetTypeName<value_t>(), GetTypeName<value_t>(), (int)lookupValue, (int)table.values[sizeT-1]);
    TEST_ASSERT_EQUAL_MESSAGE(table.values[sizeT-1], result, szMsg);
}

static void test_table2dLookup_overMax(void)
{
    test_table2dLookup_overMax(table2d_u8_u8);
    test_table2dLookup_overMax(table2d_u8_u16);
    test_table2dLookup_overMax(table2d_u16_u8);
    test_table2dLookup_overMax(table2d_u16_u16);
    test_table2dLookup_overMax(table2d_u8_i8);
    test_table2dLookup_overMax(table2d_i16_u8); 
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_table2dLookup_underMin(const table2D<axis_t, value_t, sizeT> &table) {
    value_t result = (value_t)table2D_getValue(&table, (axis_t)(table.axis[0]-1));
    TEST_ASSERT_EQUAL(table.values[0], result);
    TEST_ASSERT_EQUAL(1, table.cache.lastBinUpperIndex);
}

static void test_withinBin(void) {
  //Tests the Bin::withinBin function
  //This is used to check that the value is within the bin range
  _table2d_detail::Bin<uint16_t> testBin(0, 10, 5);
  TEST_ASSERT_FALSE(testBin.withinBin(4));
  TEST_ASSERT_FALSE(testBin.withinBin(5));
  TEST_ASSERT_TRUE(testBin.withinBin(6));
  TEST_ASSERT_TRUE(testBin.withinBin(10));
  TEST_ASSERT_FALSE(testBin.withinBin(11));
}

static void test_table2dLookup_underMin(void)
{
    test_table2dLookup_underMin(table2d_u8_u8);
    test_table2dLookup_underMin(table2d_u8_u16);
    test_table2dLookup_underMin(table2d_u16_u8);
    test_table2dLookup_underMin(table2d_u16_u16);
    test_table2dLookup_underMin(table2d_u8_i8);
    test_table2dLookup_underMin(table2d_i16_u8);      
}


static void test_table2d_all_decrementing(void)
{
    uint8_t u8_u8_result_last = UINT8_MAX;
    for (uint8_t loop=table2d_axis_u8[0]; loop<=table2d_axis_u8[_countof(table2d_axis_u8)-1]; ++loop)
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
    uint8_t end_index = _countof(table2d_axis_u8)-1U;
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
    RUN_TEST(test_withinBin);
  }
}