#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testFuelController(void);
    extern void testFuelScheduleInit(void);
    extern void testInjCalcs(void);
    extern void testApplyPwToInjectorChannels(void);

    testFuelController();
    testFuelScheduleInit();
    testInjCalcs();
    testApplyPwToInjectorChannels();
}

TEST_HARNESS(runAllTests)
