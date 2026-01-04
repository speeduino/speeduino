#include "../device_test_harness.h"

void runAllIgnitionTests(void)
{
    extern void testIgnCorrections(void);
    extern void testDwell(void);

    testIgnCorrections();
    testDwell();
}

DEVICE_TEST(runAllIgnitionTests)