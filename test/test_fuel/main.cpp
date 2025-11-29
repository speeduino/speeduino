#include "../device_test_harness.h"

void runAllFuelTests(void)
{
    extern void testCorrections(void);
    extern void testPW(void);
    extern void testStaging(void );
    extern void testPwApplyNitrous(void);
    testCorrections();
    testPW();
    testStaging();
    testPwApplyNitrous();
}

DEVICE_TEST(runAllFuelTests)
