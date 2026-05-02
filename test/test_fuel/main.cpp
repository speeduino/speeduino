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
    extern void testCalculateOpenTime(void);
    extern void testCalculatePWLimit(void);
    extern void testCalcPrimaryPulseWidth(void);

    testCorrections();
    testComputePulseWidths();
    testPwApplyNitrous();
    testCalculateRequiredFuel();
    testApplyPwLimit();
    testCalculateSecondaryPw();
    testCalculateOpenTime();
    testCalculatePWLimit();
    testCalcPrimaryPulseWidth();
}

TEST_HARNESS(runAllFuelTests)
