#include "../device_test_harness.h"

void runAllFuelTests(void)
{
    extern void testCorrections(void);
    extern void testPW(void);
    extern void testStaging(void );
    extern void testPwApplyNitrous(void);
    extern void testCalculateRequiredFuel(void);

    testCorrections();
    testPW();
    testStaging();
    testPwApplyNitrous();
    testCalculateRequiredFuel();
}

DEVICE_TEST(runAllFuelTests)
