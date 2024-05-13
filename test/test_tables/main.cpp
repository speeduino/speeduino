#include "../device_test_harness.h"

void runAllTableTests(void)
{
    extern void testTables3d(void);
    extern void testTable2d(void);
    extern void testAlgorithms3D(void);

    testTables3d();
    testTable2d();
    testAlgorithms3D();
}

DEVICE_TEST(runAllTableTests)
