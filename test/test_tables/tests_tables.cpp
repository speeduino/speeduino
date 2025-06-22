// #include <string.h> // memcpy
#include <unity.h>
#include <stdio.h>
#include "tests_tables.h"
#include "table3d.h"
#include "../test_utils.h"

TEST_DATA_P table3d_value_t values[] = {
 //0    1    2   3     4    5    6    7    8    9   10   11   12   13    14   15
34,  34,  34,  34,  34,  34,  34,  34,  34,  35,  35,  35,  35,  35,  35,  35, 
34,  35,  36,  37,  39,  41,  42,  43,  43,  44,  44,  44,  44,  44,  44,  44, 
35,  36,  38,  41,  44,  46,  47,  48,  48,  49,  49,  49,  49,  49,  49,  49, 
36,  39,  42,  46,  50,  51,  52,  53,  53,  53,  53,  53,  53,  53,  53,  53, 
38,  43,  48,  52,  55,  56,  57,  58,  58,  58,  58,  58,  58,  58,  58,  58, 
42,  49,  54,  58,  61,  62,  62,  63,  63,  63,  63,  63,  63,  63,  63,  63, 
48,  56,  60,  64,  66,  66,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68, 
54,  62,  66,  69,  71,  71,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72, 
61,  69,  72,  74,  76,  76,  77,  77,  77,  77,  77,  77,  77,  77,  77,  77, 
68,  75,  78,  79,  81,  81,  81,  82,  82,  82,  82,  82,  82,  82,  82,  82, 
74,  80,  83,  84,  85,  86,  86,  86,  87,  87,  87,  87,  87,  87,  87,  87, 
81,  86,  88,  89,  90,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91, 
93,  96,  98,  99,  99,  100, 100, 101, 101, 101, 101, 101, 101, 101, 101, 101, 
98,  101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 
109, 111, 112, 113, 114, 114, 114, 115, 115, 115, 114, 114, 114, 114, 114, 114, 
  };

static constexpr table3d_axis_t XAXIS_FACTOR = 100U;
TEST_DATA_P table3d_axis_t tempXAxis[] = { 500U/XAXIS_FACTOR, 700U/XAXIS_FACTOR, 900U/XAXIS_FACTOR, 1200U/XAXIS_FACTOR, 1600U/XAXIS_FACTOR, 2000U/XAXIS_FACTOR, 2500U/XAXIS_FACTOR, 3100U/XAXIS_FACTOR, 3500U/XAXIS_FACTOR, 4100U/XAXIS_FACTOR, 4700U/XAXIS_FACTOR, 5300U/XAXIS_FACTOR, 5900U/XAXIS_FACTOR, 6500U/XAXIS_FACTOR, 6750U/XAXIS_FACTOR, 7000U/XAXIS_FACTOR};
static constexpr uint16_t xMin = tempXAxis[0]*XAXIS_FACTOR;
static constexpr uint16_t xMax = tempXAxis[_countof(tempXAxis)-1]*XAXIS_FACTOR;

static constexpr table3d_axis_t YAXIS_FACTOR = 2U;
// TEST_DATA_P table3d_axis_t tempYAxis[] = { 16U, 26U, 30U, 36U, 40U, 46U, 50U, 56U, 60U, 66U, 70U, 76U, 86U, 90U, 96U, 100U};
TEST_DATA_P table3d_axis_t tempYAxis[] = { 16U/YAXIS_FACTOR, 26U/YAXIS_FACTOR, 30U/YAXIS_FACTOR, 36U/YAXIS_FACTOR, 40U/YAXIS_FACTOR, 46U/YAXIS_FACTOR, 50U/YAXIS_FACTOR, 56U/YAXIS_FACTOR, 60U/YAXIS_FACTOR, 66U/YAXIS_FACTOR, 70U/YAXIS_FACTOR, 76U/YAXIS_FACTOR, 86U/YAXIS_FACTOR, 90U/YAXIS_FACTOR, 96U/YAXIS_FACTOR, 100U/YAXIS_FACTOR};
static constexpr uint16_t yMin = tempYAxis[0]*YAXIS_FACTOR;
static constexpr uint16_t yMax = tempYAxis[_countof(tempYAxis)-1]*YAXIS_FACTOR;

static table3d16RpmLoad testTable;

void setup_TestTable(void)
{
  //Setup the fuel table with some sane values for testing
  //Table is setup per the below
  /*
  100 |  109 |  111 |  112 |  113 |  114 |  114 |  114 |  115 |  115 |  115 |  114 |  114 |  113 |  112 |  111 |  111
  96  |  104 |  106 |  107 |  108 |  109 |  109 |  110 |  110 |  110 |  110 |  110 |  109 |  108 |  107 |  107 |  106
  90  |   98 |  101 |  103 |  103 |  104 |  105 |  105 |  105 |  105 |  105 |  105 |  104 |  104 |  103 |  102 |  102
  86  |   93 |   96 |   98 |   99 |   99 |  100 |  100 |  101 |  101 |  101 |  100 |  100 |   99 |   98 |   98 |   97
  76  |   81 |   86 |   88 |   89 |   90 |   91 |   91 |   91 |   91 |   91 |   91 |   90 |   90 |   89 |   89 |   88
  70  |   74 |   80 |   83 |   84 |   85 |   86 |   86 |   86 |   87 |   86 |   86 |   86 |   85 |   84 |   84 |   84
  65  |   68 |   75 |   78 |   79 |   81 |   81 |   81 |   82 |   82 |   82 |   82 |   81 |   81 |   80 |   79 |   79
  60  |   61 |   69 |   72 |   74 |   76 |   76 |   77 |   77 |   77 |   77 |   77 |   76 |   76 |   75 |   75 |   74
  56  |   54 |   62 |   66 |   69 |   71 |   71 |   72 |   72 |   72 |   72 |   72 |   72 |   71 |   71 |   70 |   70
  50  |   48 |   56 |   60 |   64 |   66 |   66 |   68 |   68 |   68 |   68 |   67 |   67 |   67 |   66 |   66 |   65
  46  |   42 |   49 |   54 |   58 |   61 |   62 |   62 |   63 |   63 |   63 |   63 |   62 |   62 |   61 |   61 |   61
  40  |   38 |   43 |   48 |   52 |   55 |   56 |   57 |   58 |   58 |   58 |   58 |   58 |   57 |   57 |   57 |   56
  36  |   36 |   39 |   42 |   46 |   50 |   51 |   52 |   53 |   53 |   53 |   53 |   53 |   53 |   52 |   52 |   52
  30  |   35 |   36 |   38 |   41 |   44 |   46 |   47 |   48 |   48 |   49 |   48 |   48 |   48 |   48 |   47 |   47
  26  |   34 |   35 |   36 |   37 |   39 |   41 |   42 |   43 |   43 |   44 |   44 |   44 |   43 |   43 |   43 |   43
  16  |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   34 |   35 |   34 |   34 |   34 |   34 |   34 |   34
      ----------------------------------------------------------------------------------------------------------------
         500 |  700 |  900 | 1200 | 1600 | 2000 | 2500 | 3100 | 3500 | 4100 | 4700 | 5300 | 5900 | 6500 | 6750 | 7000
  */
  populate_table_P(testTable, tempXAxis, tempYAxis, values);
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
  //RUN_TEST(test_all_incrementing);

  }  
}

void test_tableLookup_50pct(void)
{
  //Tests a lookup that is exactly 50% of the way between cells on both the X and Y axis
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, 53, 2250); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(69U, tempVE);
}

void test_tableLookup_exact1Axis(void)
{
  //Tests a lookup that exactly matches on the X axis and 50% of the way between cells on the Y axis
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, 48, testTable.axisX.axis[6]*XAXIS_FACTOR); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(65U, tempVE);
}

void test_tableLookup_exact2Axis(void)
{
  //Tests a lookup that exactly matches on both the X and Y axis
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, testTable.axisY.axis[5]*YAXIS_FACTOR, testTable.axisX.axis[9]*XAXIS_FACTOR); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(86U, tempVE);
}

void test_tableLookup_overMaxX(void)
{
  //Tests a lookup where the RPM exceeds the highest value in the table. The Y value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, 73, xMax+100); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(89U, tempVE);
}

void test_tableLookup_overMaxY(void)
{
  //Tests a lookup where the load value exceeds the highest value in the table. The X value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, yMax+10, 600); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(tempVE, 110);
}

void test_tableLookup_underMinX(void)
{
  //Tests a lookup where the RPM value is below the lowest value in the table. The Y value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, 38, xMin-100); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(37U, tempVE);
}

void test_tableLookup_underMinY(void)
{
  //Tests a lookup where the load value is below the lowest value in the table. The X value is a 50% match
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, yMin-5, 600); //Perform lookup into fuel map for RPM vs MAP value
  TEST_ASSERT_EQUAL(34U, tempVE);
}

void test_tableLookup_roundUp(void)
{
  // Tests a lookup where the inputs result in a value that is outside the table range
  // due to fixed point rounding
  // Issue #726
  setup_TestTable();

  uint16_t tempVE = get3DTableValue(&testTable, 17, 600);
  TEST_ASSERT_EQUAL(34U, tempVE);
}

void test_all_incrementing(void)
{
  //Test the when going up both the load and RPM axis that the returned value is always equal or higher to the previous one
  //Tests all combinations of load/rpm from between 0-200 load and 0-9000 rpm
  //WARNING: This can take a LONG time to run. It is disabled by default for this reason
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