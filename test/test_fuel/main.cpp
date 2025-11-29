#include "../device_test_harness.h"

void runAllFuelTests(void)
{
    extern void testCorrections(void);
    extern void testPW(void);
    extern void testPwApplyNitrous(void);
    extern void testCalculateRequiredFuel(void);
    extern void testApplyPwLimit(void);
    extern void testCalculateSecondaryPw(void);
    extern void testApplyPwToInjectorChannels(void);

    testCorrections();
    testPW();
    testPwApplyNitrous();
    testCalculateRequiredFuel();
    testApplyPwLimit();
    testCalculateSecondaryPw();
    testApplyPwToInjectorChannels();
}

DEVICE_TEST(runAllFuelTests)
