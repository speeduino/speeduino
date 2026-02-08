#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTableTests(void)
{
    extern void testTables(void);
    extern void testTable2d(void);

    extern void testTables(void);
    extern void testTable2d(void);

    testTables();
    testTable2d();
}

TEST_HARNESS(runAllTableTests)
