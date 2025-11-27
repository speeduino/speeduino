#include "../device_test_harness.h"

void runAllIgnitionTests(void)
{
    extern void testIgnCorrections(void);
    testIgnCorrections();
}

DEVICE_TEST(runAllIgnitionTests)