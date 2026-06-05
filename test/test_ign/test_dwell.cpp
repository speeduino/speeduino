#include "dwell.h"
#include "../test_utils.h"

static void test_computeDwell_cranking(void) {
    statuses cur = {};
    config2 p2 = {};
    config4 p4 = {};
    table3d4RpmLoad table = {};

    cur.engineIsCranking = true;
    p4.dwellCrank = 43; // 4.3 ms stored as ms*10

    TEST_ASSERT_EQUAL_UINT16(4300U, computeDwell(cur, p2, p4, table));
}

static void test_computeDwell_map(void) {
    statuses cur = {};
    config2 p2 = {};
    config4 p4 = {};
    table3d4RpmLoad table = {};

    cur.engineIsCranking = false;
    p2.useDwellMap = true;

    // Fill the dwell table with a constant value of 55 (5.5 ms -> 5500 us)
    fill_table_values(table, (table3d_value_t)55);

    TEST_ASSERT_EQUAL_UINT16(5500U, computeDwell(cur, p2, p4, table));
}

static void test_computeDwell_running(void) {
    statuses cur = {};
    config2 p2 = {};
    config4 p4 = {};
    table3d4RpmLoad table = {};

    cur.engineIsCranking = false;
    p2.useDwellMap = false;
    p4.dwellRun = 60; // 6.0 ms -> 6000 us

    TEST_ASSERT_EQUAL_UINT16(6000U, computeDwell(cur, p2, p4, table));
}

void testDwell(void)
{
    SET_UNITY_FILENAME() {
        RUN_TEST_P(test_computeDwell_cranking);
        RUN_TEST_P(test_computeDwell_map);
        RUN_TEST_P(test_computeDwell_running);
    }
}
