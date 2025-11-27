#include "../device_test_harness.h"

void runAllFuelTests(void)
{
    extern void testCorrections(void);
    extern void testPW(void);
    extern void testStaging(void );

    testCorrections();
    testPW();
    testStaging();
}

DEVICE_TEST(runAllFuelTests)
