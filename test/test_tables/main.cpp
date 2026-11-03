#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTableTests(void)
{
    extern void testTables(void);
    extern void testTable2d(void);
    extern void test3DTableUtils(void);

    testTables();
    testTable2d();
    test3DTableUtils();
}

TEST_HARNESS(runAllTableTests)
