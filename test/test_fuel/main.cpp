#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllFuelTests(void)
{
    extern void testCorrections(void);
    extern void testComputePulseWidths(void);
    extern void testPwApplyNitrous(void);
    extern void testCalculateRequiredFuel(void);
    extern void testApplyPwLimit(void);
    extern void testCalculateSecondaryPw(void);
    extern void testApplyPwToInjectorChannels(void);
    extern void testCalculateOpenTime(void);

    testCorrections();
    testComputePulseWidths();
    testPwApplyNitrous();
    testCalculateRequiredFuel();
    testApplyPwLimit();
    testCalculateSecondaryPw();
    testApplyPwToInjectorChannels();
    testCalculateOpenTime();
}

TEST_HARNESS(runAllFuelTests)
