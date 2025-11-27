#include "../device_test_harness.h"

void runAllTableTests(void)
{
    extern void testTables(void);
    extern void testTable2d(void);

    extern void testTables(void);
    extern void testTable2d(void);

    testTables();
    testTable2d();
}

DEVICE_TEST(runAllTableTests)
