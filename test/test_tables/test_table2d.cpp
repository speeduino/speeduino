#include <unity.h>
#include <stdio.h>
#include "table2d.h"
#include "../test_utils.h"
#include "type_traits.h"

#define U8_X4_DATA 251, 211, 199, 167,
static uint8_t table2d_data_u8_4[4] = { U8_X4_DATA };
#define U8_X6_DATA U8_X4_DATA 127, 101,
static uint8_t table2d_data_u8_6[6] = { U8_X6_DATA };
#define U8_X8_DATA U8_X6_DATA 59, 23,
static uint8_t table2d_data_u8_8[8] = { U8_X8_DATA };
#define U8_X9_DATA U8_X8_DATA 20,
static uint8_t table2d_data_u8_9[9] = { U8_X9_DATA };
#define U8_X10_DATA U8_X9_DATA 10,
static uint8_t table2d_data_u8_10[10] = { U8_X10_DATA };
#define U16_X4_DATA 32029, 26357, 21323, 16363, 
static uint16_t table2d_data_u16_4[4] = { U16_X4_DATA };
#define S16_X6_DATA 32029, 21323, 11329, 5531, 1237, -5531, 
static int16_t table2d_data_s16_6[6] = { S16_X6_DATA };
#define U16_X32_DATA \
    65535, 63490, 61445, 59400, 57355, 55310, 53265, 51220, \
    49175, 47130, 45085, 43040, 40995, 38950, 36905, 34860, \
    32815, 30770, 28725, 26680, 24635, 22590, 20545, 18500, \
    16455, 14410, 12365, 10320, 8275,  6230,  4185,  2140, 
static uint16_t table2d_data_u16_32[32] = { U16_X32_DATA };
#define U8_X32_DATA \
    254, 247, 240, 233, 226, 219, 212, 205, \
    198, 191, 184, 177, 170, 163, 156, 149, \
    142, 135, 128, 121, 114, 107, 100, 93, \
    86, 79, 72, 65, 58, 51, 44, 37, 
static uint8_t table2d_data_u8_32[32] = { U8_X32_DATA };

#define U8_X4_AXIS 5, 23, 59, 101,
static uint8_t table2d_axis_u8_4[4] = { U8_X4_AXIS };
#define U8_X6_AXIS U8_X4_AXIS 127, 167,
static uint8_t table2d_axis_u8_6[6] = { U8_X6_AXIS };
#define U8_X8_AXIS U8_X6_AXIS 199, 211,
static uint8_t table2d_axis_u8_8[8] = { U8_X8_AXIS };
#define U8_X9_AXIS U8_X8_AXIS 221,
static uint8_t table2d_axis_u8_9[9] = { U8_X9_AXIS };
#define U8_X10_AXIS U8_X9_AXIS 231,
static uint8_t table2d_axis_u8_10[10] = { U8_X10_AXIS };
#define I8_X4_AXIS -125, -101, -59, -5
static int8_t table2d_axis_i8_4[4] = { I8_X4_AXIS };
#define U16_X32_AXIS \
    2140, 4185,  6230,  8275,  10320, 12365, 14410, 16455,  \
    18500, 20545, 22590, 24635, 26680, 28725, 30770, 32815, \
    34860, 36905, 38950, 40995, 43040, 45085, 47130, 49175, \
    51220, 53265, 55310, 57355, 59400, 61445, 63490, 64353, 
static uint16_t table2d_axis_u16_32[32] = { U16_X32_AXIS };

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

#define RUN_PARAMETRIZED_TEST(op, axis, data, tag, ...) \
    { \
        auto testFunc = [](void) { \
            op(axis, data, ##__VA_ARGS__); \
        }; \
        UnityDefaultTestRun(testFunc, #op tag, __LINE__); \
    }
#define APPLY_TEST_TO_ALL_TYPES(op, tag, ...) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_i8_4, table2d_data_u8_4, tag "_i8_u8_4", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_4, table2d_data_u8_4, tag "_u8_u8_4", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_6, table2d_data_u8_6, tag "_u8_u8_6", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_8, table2d_data_u8_8, tag "_u8_u8_8", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_9, table2d_data_u8_9, tag "_u8_u8_9", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_10, table2d_data_u8_10, tag "_u8_u8_10", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u16_32, table2d_data_u8_32, tag "_u16_u8_32", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u16_32, table2d_data_u16_32, tag "_u16_u16_32", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_4, table2d_data_u16_4, tag "_u8_u16_4", ##__VA_ARGS__) \
    RUN_PARAMETRIZED_TEST(op, table2d_axis_u8_6, table2d_data_s16_6, tag "_u8_s16_6", ##__VA_ARGS__);

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_getValue_withinBins(axis_t (&pAxisBin)[sizeT], value_t (&pCurve)[sizeT], uint8_t binFrac)
{
    table2D<axis_t, value_t, sizeT> testSubject(&pAxisBin, &pCurve);

    for (uint8_t i=0; i<sizeT-1U; ++i) {
        axis_t lookupValue = intermediate(testSubject.axis[i], testSubject.axis[i+1], binFrac);
        int32_t expected = (int32_t)map(lookupValue, testSubject.axis[i], testSubject.axis[i+1], testSubject.values[i], testSubject.values[i+1]);
        value_t result = table2D_getValue(&testSubject, lookupValue);
        char szMsg[512];
        sprintf(szMsg, "Loop: %" PRIu8 ", VT %s, AT %s, lookup: %" PRId32 ", data[i]: %" PRId32 ", data[i+1]: %" PRId32 ", binFrac %" PRIu8, 
                        i,
                        GetTypeName<value_t>(), GetTypeName<axis_t>(), 
                        (int32_t)lookupValue, (int32_t)expected, (int32_t)result, binFrac);
        TEST_ASSERT_INT32_WITHIN_MESSAGE(1, expected, result, szMsg);
        TEST_ASSERT_EQUAL(i+1, testSubject.cache.lastBinUpperIndex);
    }
}

static void test_getValue_bin_midpoints(void)
{
    // Need a *STATIC* local copy of the binFrac to pass to the lambda, as the lambda can't capture the parameter
    static uint8_t binFracLocal = 50; 
    APPLY_TEST_TO_ALL_TYPES(test_getValue_withinBins, "_midpoint", binFracLocal);
}

static void test_getValue_bin_33(void)
{
    static uint8_t binFracLocal = 33; 
    APPLY_TEST_TO_ALL_TYPES(test_getValue_withinBins, "_one_third", binFracLocal);
}

static void test_getValue_bin_66(void)
{
    static uint8_t binFracLocal = 66; 
    APPLY_TEST_TO_ALL_TYPES(test_getValue_withinBins, "_two_thirds", binFracLocal);
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_getValue_bin_edges(axis_t (&pAxisBin)[sizeT], value_t (&pCurve)[sizeT])
{
    table2D<axis_t, value_t, sizeT> testSubject(&pAxisBin, &pCurve);

    for (uint8_t i=0; i<sizeT; ++i) {
        value_t result = (value_t)table2D_getValue(&testSubject, testSubject.axis[i]);
        // char szMsg[64];
        // sprintf(szMsg, "%d, %d lookup %d: %d", testSubject.values.typeIndicator, testSubject.axis.typeIndicator, i, axis[i]);
        TEST_ASSERT_EQUAL(testSubject.values[i], result);
        // sprintf(szMsg, "%d, %d lookup %d", testSubject.values.typeIndicator, testSubject.axis.typeIndicator, i);
        TEST_ASSERT_EQUAL(i==0  ? 1 : i, testSubject.cache.lastBinUpperIndex);
    }
}

static void test_getValue_bin_edges(void)
{
    APPLY_TEST_TO_ALL_TYPES(test_getValue_bin_edges, "");
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_getValue_overMax(axis_t (&pAxisBin)[sizeT], value_t (&pCurve)[sizeT])
{
    table2D<axis_t, value_t, sizeT> testSubject(&pAxisBin, &pCurve);

    axis_t lookupValue =(axis_t)(testSubject.axis[sizeT-1]+1);
    value_t result = table2D_getValue(&testSubject, lookupValue);
    char szMsg[256];
    sprintf(szMsg, "VT %s, AT %s, sizeT: %" PRIu8 ", lookup: %" PRId32 ", data[sizeT-1]: %" PRId32, 
            GetTypeName<value_t>(), GetTypeName<axis_t>(), 
            sizeT, (int32_t)lookupValue, (int32_t)testSubject.values[sizeT-1]);
    TEST_ASSERT_EQUAL_MESSAGE(testSubject.values[sizeT-1], result, szMsg);
}

static void test_getValue_overMax(void)
{
    APPLY_TEST_TO_ALL_TYPES(test_getValue_overMax, "");
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_getValue_underMin(axis_t (&pAxisBin)[sizeT], value_t (&pCurve)[sizeT])
{
    table2D<axis_t, value_t, sizeT> testSubject(&pAxisBin, &pCurve);

    value_t result = (value_t)table2D_getValue(&testSubject, (axis_t)(testSubject.axis[0]-1));
    TEST_ASSERT_EQUAL(testSubject.values[0], result);
    TEST_ASSERT_EQUAL(1, testSubject.cache.lastBinUpperIndex);
}

static void test_getValue_underMin(void)
{
    APPLY_TEST_TO_ALL_TYPES(test_getValue_underMin, "");
}

template <typename TIntegral>
static void test_withinBin(const _table2d_detail::Bin<TIntegral> &testSubject)
{
    TEST_ASSERT_FALSE(testSubject.withinBin(testSubject.lowerValue()-1));
    TEST_ASSERT_FALSE(testSubject.withinBin(testSubject.lowerValue()));
    TEST_ASSERT_TRUE(testSubject.withinBin(testSubject.lowerValue()+1));
    TEST_ASSERT_TRUE(testSubject.withinBin(testSubject.upperValue()));
    TEST_ASSERT_FALSE(testSubject.withinBin(testSubject.upperValue()+1));
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_withinBin(axis_t (&pAxisBin)[sizeT], value_t (&)[sizeT])
{
    test_withinBin(_table2d_detail::Bin<axis_t>(1, pAxisBin[1], pAxisBin[0]));
    test_withinBin(_table2d_detail::Bin<axis_t>(pAxisBin, 1));
}

static void test_withinBin(void) {
    APPLY_TEST_TO_ALL_TYPES(test_withinBin, "");
}

template <typename axis_t, uint8_t sizeT>
static void assert_findBin(axis_t (&pAxisBin)[sizeT], axis_t lookUp, uint8_t expectedUpperIndex)
{
    _table2d_detail::Bin<axis_t> result = _table2d_detail::findBin<axis_t, sizeT>(pAxisBin, lookUp);
    TEST_ASSERT_EQUAL(expectedUpperIndex, result.upperIndex);
    TEST_ASSERT_EQUAL(pAxisBin[expectedUpperIndex-1], result.lowerValue());
    TEST_ASSERT_EQUAL(pAxisBin[expectedUpperIndex], result.upperValue());
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_findBin(axis_t (&pAxisBin)[sizeT], value_t (&)[sizeT])
{
    assert_findBin<axis_t, sizeT>(pAxisBin, pAxisBin[0]-1, 1U);
    assert_findBin<axis_t, sizeT>(pAxisBin, pAxisBin[0], 1U);
    assert_findBin<axis_t, sizeT>(pAxisBin, pAxisBin[0]+1, 1U);
    assert_findBin<axis_t, sizeT>(pAxisBin, pAxisBin[sizeT-1]-1, sizeT-1U);
    assert_findBin<axis_t, sizeT>(pAxisBin, pAxisBin[sizeT-1], sizeT-1U);
    assert_findBin<axis_t, sizeT>(pAxisBin, pAxisBin[sizeT-1]+1, sizeT-1U);
}

static void test_findBin(void) {
    APPLY_TEST_TO_ALL_TYPES(test_findBin, "");
}

template <typename axis_t, typename value_t, uint8_t sizeT>
static void test_table2d_all_decrementing(axis_t (&pAxisBin)[sizeT], value_t (&pCurve)[sizeT])
{
    table2D<axis_t, value_t, sizeT> testSubject(&pAxisBin, &pCurve);

    value_t result_last = (numeric_limits<value_t>::max)();
    for (axis_t loop=testSubject.axis[0]; loop<=testSubject.axis[sizeT-1]; ++loop)
    {
        value_t result = table2D_getValue(&testSubject, loop);
        TEST_ASSERT_LESS_OR_EQUAL(result_last, result);
        result_last = result;
    }
}

static void test_table2d_all_decrementing(void) {
    APPLY_TEST_TO_ALL_TYPES(test_table2d_all_decrementing, "");
}

#include "../timer.hpp"

static void test_lookup_perf(void) {
    uint16_t iters = 1000;
    uint8_t start_index = 0;
    uint8_t end_index = _countof(table2d_axis_u8_9)-1U;
    uint8_t step = 1;

    timer timerA;
    uint32_t paramA = 0;
    auto nativeTest = [] (uint8_t index, uint32_t &checkSum) { 
        table2D<uint8_t, uint8_t, 9> testSubject(&table2d_axis_u8_9, &table2d_data_u8_9);
        checkSum += table2D_getValue(&testSubject, (uint8_t)(table2d_axis_u8_9[index]+1));
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
    test_getValue_overMax();
    test_getValue_underMin();
    test_table2d_all_decrementing(); 
    test_getValue_bin_midpoints();
    test_getValue_bin_33();
    test_getValue_bin_66();
    test_getValue_bin_edges();
    test_withinBin();
    test_findBin();
    RUN_TEST(test_lookup_perf);
  }
}