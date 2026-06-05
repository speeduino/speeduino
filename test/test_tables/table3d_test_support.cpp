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

static constexpr table3d_axis_t YAXIS_FACTOR = 2U;
TEST_DATA_P table3d_axis_t tempYAxis[] = { 16U/YAXIS_FACTOR, 30U/YAXIS_FACTOR, 40U/YAXIS_FACTOR, 50U/YAXIS_FACTOR, 60U/YAXIS_FACTOR, 70U/YAXIS_FACTOR, 86U/YAXIS_FACTOR, 96U/YAXIS_FACTOR};

table3d8RpmLoad getDummyTable(void)
{
    table3d8RpmLoad testTable;
    populate_table_P(testTable, tempXAxis, tempYAxis, values);
    return testTable;
}