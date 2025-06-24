// #include <string.h> // memcpy
#include <unity.h>
#include <stdio.h>
#include "tests_tables.h"
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
  RUN_TEST(test_linear_search);
  //RUN_TEST(test_all_incrementing);

  }  
}