// #include <string.h> // memcpy
#include <unity.h>
#include <stdio.h>
#include "table3d.h"
#include "../test_utils.h"

TEST_DATA_P table3d_value_t values[] = {
  //0     1      2    3     4     5     6     7  
    8,		11,		14,		17,		20,		23,		26,		29, // 0
    32,		35,		38,		41,		44,		47,		50,		53, // 1
    56,		59,		62,		65,		68,		71,		74,		77, // 2
    80,		83,		86,		89,		92,		95,		98,		101,// 3
    104,	107,	110,	113,	116,	119,	122,	125,// 4
    128,	131,	134,	137,	140,	143,	146,	149,// 5
    152,	155,	158,	161,	164,	167,	170,	173,// 6
    176,	179,	182,	185,	188,	191,	194,	197,// 7
  };

static constexpr table3d_axis_t XAXIS_FACTOR = 100U;
TEST_DATA_P table3d_axis_t tempXAxis[] = { 900U/XAXIS_FACTOR, 1600U/XAXIS_FACTOR, 2500U/XAXIS_FACTOR, 3500U/XAXIS_FACTOR, 4700U/XAXIS_FACTOR, 5900U/XAXIS_FACTOR, 6750U/XAXIS_FACTOR, 6990U/XAXIS_FACTOR};
static constexpr uint16_t xMin = tempXAxis[0]*XAXIS_FACTOR;
static constexpr uint16_t xMax = tempXAxis[_countof(tempXAxis)-1]*XAXIS_FACTOR;

static constexpr table3d_axis_t YAXIS_FACTOR = 2U;
TEST_DATA_P table3d_axis_t tempYAxis[] = { 16U/YAXIS_FACTOR, 30U/YAXIS_FACTOR, 40U/YAXIS_FACTOR, 50U/YAXIS_FACTOR, 60U/YAXIS_FACTOR, 70U/YAXIS_FACTOR, 86U/YAXIS_FACTOR, 96U/YAXIS_FACTOR};
static constexpr uint16_t yMin = tempYAxis[0]*YAXIS_FACTOR;
static constexpr uint16_t yMax = tempYAxis[_countof(tempYAxis)-1]*YAXIS_FACTOR;

static void test_tableLookup_50pct(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);
  uint16_t tempVE = get3DTableValue(&testTable, 53, 2250); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(92U, tempVE);
}

static void test_tableLookup_exact1Axis(void)
{
  //Tests a lookup that exactly matches on the X axis and 50% of the way between cells on the Y axis
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, 48, testTable.axisX.axis[6]*XAXIS_FACTOR); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(78U, tempVE);
}

static void test_tableLookup_exact2Axis(void)
{
  //Tests a lookup that exactly matches on both the X and Y axis
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, testTable.axisY.axis[5]*YAXIS_FACTOR, testTable.axisX.axis[7]*XAXIS_FACTOR); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(56U, tempVE);
}

static void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the RPM exceeds the highest value in the table. The Y value is a 50% match
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, 73, xMax+100); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(153U, tempVE);
}

static void test_tableLookup_overMaxY(void)
{
  //Tests a lookup where the load value exceeds the highest value in the table. The X value is a 50% match
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, yMax+10, 600); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(176U, tempVE);
}

static void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the RPM value is below the lowest value in the table. The Y value is a 50% match
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, 38, xMin-100); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(51U, tempVE);
}

static void test_tableLookup_underMinY(void)
{
  //Tests a lookup where the load value is below the lowest value in the table. The X value is a 50% match
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, yMin-5, 600); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(8U, tempVE);
}

static void test_tableLookup_roundUp(void)
{
  // Tests a lookup where the inputs result in a value that is outside the table range
  // due to fixed point rounding
  // Issue #726
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = get3DTableValue(&testTable, 17, 600);
  TEST_ASSERT_EQUAL(9U, tempVE);
}

static void test_all_incrementing(void)
{
#if defined(NATIVE_BOARD)
  //Test the when going up both the load and RPM axis that the returned value is always equal or higher to the previous one
  //Tests all combinations of load/rpm from between 0-200 load and 0-9000 rpm
  //WARNING: This can take a LONG time to run. It is disabled by default for this reason
  table3d8RpmLoad testTable;
  populate_table_P(testTable, tempXAxis, tempYAxis, values);

  uint16_t tempVE = 0;
  
  for(uint16_t rpm = 0; rpm<xMax+1000; rpm+=100)
  {
    tempVE = 0;
    for(uint8_t load = 0; load<yMax+10; load++)
    {
      uint16_t newVE = get3DTableValue(&testTable, load, rpm);
      // char buffer[256];
      // sprintf(buffer, "%d, %d"
      //                 ", %d, %d, %d, %d"
      //                 ", %d, %d, %d, %d"
      //                 ", %d", 
      //                 rpm, load, 
      //                 testTable.get_value_cache.lastXMin, testTable.get_value_cache.lastBinMax.x,
      //                 tempXAxis[testTable.get_value_cache.lastXMin], tempXAxis[testTable.get_value_cache.lastBinMax.x],

      //                 testTable.get_value_cache.lastYMin, testTable.get_value_cache.lastBinMax.y,
      //                 tempYAxis[testTable.get_value_cache.lastYMin], tempYAxis[testTable.get_value_cache.lastBinMax.y],

      //                 newVE);
      // TEST_MESSAGE(buffer);
      TEST_ASSERT_GREATER_OR_EQUAL(tempVE, newVE);
      tempVE = newVE;
    }
  }
#else
  TEST_IGNORE_MESSAGE("Test takes too long on device");
#endif
}
extern table3d_dim_t linear_bin_search(const table3d_axis_t *array, 
                            const table3d_dim_t length,
                            const table3d_axis_t value);

static void test_linear_bin_search(void) {
  // Test the linear search function used in the table lookup
  // This is a simple test to ensure that the linear search returns the correct index
  constexpr table3d_axis_t axis[] = { 50, 40, 30, 20, 10 };
  // Below axis min value
  TEST_ASSERT_EQUAL(_countof(axis)-2U, linear_bin_search(axis, _countof(axis), 5));
  // Middle of bins & the bin edges
  TEST_ASSERT_EQUAL(_countof(axis)-2U, linear_bin_search(axis, _countof(axis), 10));
  TEST_ASSERT_EQUAL(_countof(axis)-2U, linear_bin_search(axis, _countof(axis), 15));
  TEST_ASSERT_EQUAL(_countof(axis)-2U, linear_bin_search(axis, _countof(axis), 20));
  TEST_ASSERT_EQUAL(2U, linear_bin_search(axis, _countof(axis), 25));
  TEST_ASSERT_EQUAL(2U, linear_bin_search(axis, _countof(axis), 30));
  TEST_ASSERT_EQUAL(1U, linear_bin_search(axis, _countof(axis), 35));
  TEST_ASSERT_EQUAL(1U, linear_bin_search(axis, _countof(axis), 40));
  TEST_ASSERT_EQUAL(0U, linear_bin_search(axis, _countof(axis), 45));
  TEST_ASSERT_EQUAL(0U, linear_bin_search(axis, _countof(axis), 50));
  // Above axis max value
  TEST_ASSERT_EQUAL(0U, linear_bin_search(axis, _countof(axis), 55));
}

extern uint16_t mulQU1X8(uint16_t a, uint16_t b);
extern uint16_t QU1X8_ONE;
const uint16_t QU1X8_HALF = QU1X8_ONE/2U;
const uint16_t QU1X8_QTR = QU1X8_ONE/4U;

static void test_mulQU1X8(void) {

  TEST_ASSERT_EQUAL(0U, mulQU1X8(0U, 0U));
  TEST_ASSERT_EQUAL(QU1X8_ONE, mulQU1X8(QU1X8_ONE, QU1X8_ONE));
  TEST_ASSERT_EQUAL(QU1X8_QTR, mulQU1X8(QU1X8_HALF, QU1X8_HALF));
  TEST_ASSERT_EQUAL(QU1X8_ONE/8U, mulQU1X8(QU1X8_HALF, QU1X8_QTR));
  TEST_ASSERT_EQUAL(QU1X8_ONE/8U, mulQU1X8(QU1X8_QTR, QU1X8_HALF));
  TEST_ASSERT_EQUAL(144U, mulQU1X8(QU1X8_QTR*3U, QU1X8_QTR*3U));
}

extern uint16_t compute_bin_position(const uint16_t &value, const table3d_dim_t &upperBinIndex, const table3d_axis_t *pAxis, const uint16_t &multiplier);

static void assert_compute_bin_position(table3d_axis_t *axis, uint16_t multiplier, uint8_t percent) {
  char msg[64];
  sprintf(msg, "Mul: %u, Pct: %u", multiplier, percent);  
  TEST_ASSERT_INT_WITHIN_MESSAGE(1U, percentage(percent, QU1X8_ONE), compute_bin_position(intermediate(axis[1U]*multiplier, axis[0U]*multiplier, percent), 0U, axis, multiplier), msg);
}

static void assert_compute_bin_position_mult(table3d_axis_t *axis, uint16_t multiplier) {
  char msg[64];
  sprintf(msg, "Mul: %u", multiplier);
  // Below/at min
  TEST_ASSERT_EQUAL_MESSAGE(0U, compute_bin_position(axis[1U]*multiplier, 0U, axis, multiplier), msg);
  TEST_ASSERT_EQUAL_MESSAGE(0U, compute_bin_position((axis[1U]-5U)*multiplier, 0U, axis, multiplier), msg);
  // Above/at max
  TEST_ASSERT_EQUAL_MESSAGE(QU1X8_ONE, compute_bin_position(axis[0U]*multiplier, 0U, axis, multiplier), msg);
  TEST_ASSERT_EQUAL_MESSAGE(QU1X8_ONE, compute_bin_position((axis[0U]+5U)*multiplier, 0U, axis, multiplier), msg);
  // Intermediate
  assert_compute_bin_position(axis, multiplier, 50U);
  assert_compute_bin_position(axis, multiplier, 25U);
  assert_compute_bin_position(axis, multiplier, 75U);
  // We need bigger ranges for this level of fidelity
  if (((axis[0U]*multiplier)-(axis[1U]*multiplier))>75U) {
    assert_compute_bin_position(axis, multiplier, 33U);
    assert_compute_bin_position(axis, multiplier, 66U);
  }
}

static void test_compute_bin_position(void) {
  table3d_axis_t otherAxis[] = { 248, 128 };
  assert_compute_bin_position_mult(otherAxis, 1U);
  table3d_axis_t loadAxis[] = { 100, 86 };
  assert_compute_bin_position_mult(loadAxis, 2U);
  table3d_axis_t rpmAxis[] = { 25, 20 };
  assert_compute_bin_position_mult(rpmAxis, 100U);
}

extern table3d_value_t bilinear_interpolation( const table3d_value_t &tl,
                                                      const table3d_value_t &tr,
                                                      const table3d_value_t &bl,
                                                      const table3d_value_t &br,
                                                      const uint16_t &dx,
                                                      const uint16_t &dy);

static void test_bilinear_interpolation(void) {
  // All same
  TEST_ASSERT_EQUAL(1U, bilinear_interpolation(1U, 1U, 1U, 1U, 0U, 0U));
  // Corners
  TEST_ASSERT_EQUAL(30U, bilinear_interpolation(10U, 20U, 30U, 40U, 0U, 0U));
  TEST_ASSERT_EQUAL(10U, bilinear_interpolation(10U, 20U, 30U, 40U, 0U, QU1X8_ONE));
  TEST_ASSERT_EQUAL(40U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_ONE, 0U));
  TEST_ASSERT_EQUAL(20U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_ONE, QU1X8_ONE));
  // Edges
  TEST_ASSERT_EQUAL(35U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_HALF, 0U));
  TEST_ASSERT_EQUAL(20U, bilinear_interpolation(10U, 20U, 30U, 40U, 0U, QU1X8_HALF));
  TEST_ASSERT_EQUAL(30U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_ONE, QU1X8_HALF));
  TEST_ASSERT_EQUAL(15U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_HALF, QU1X8_ONE));
  // In-between
  TEST_ASSERT_EQUAL(25U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_HALF, QU1X8_HALF));
  TEST_ASSERT_EQUAL(27U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_QTR, QU1X8_QTR));
  TEST_ASSERT_EQUAL(32U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_QTR*3U, QU1X8_QTR));
  TEST_ASSERT_EQUAL(17U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_QTR, QU1X8_QTR*3U));
  TEST_ASSERT_EQUAL(22U, bilinear_interpolation(10U, 20U, 30U, 40U, QU1X8_QTR*3U, QU1X8_QTR*3U));
}

void testTables()
{
  SET_UNITY_FILENAME() {

  RUN_TEST(test_tableLookup_50pct);
  RUN_TEST(test_tableLookup_exact1Axis);
  RUN_TEST(test_tableLookup_exact2Axis);
  RUN_TEST(test_tableLookup_overMaxX);
  RUN_TEST(test_tableLookup_overMaxY);
  RUN_TEST(test_tableLookup_underMinX);
  RUN_TEST(test_tableLookup_underMinY);
  RUN_TEST(test_tableLookup_roundUp);
  RUN_TEST(test_linear_bin_search);
  RUN_TEST(test_mulQU1X8);
  RUN_TEST(test_compute_bin_position);
  RUN_TEST(test_bilinear_interpolation);
  RUN_TEST(test_all_incrementing);
  }  
}