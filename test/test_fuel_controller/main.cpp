#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testFuelController(void);
    extern void testFuelScheduleInit(void);
    extern void testInjCalcs(void);

    testFuelController();
    testFuelScheduleInit();
    testInjCalcs();
}

TEST_HARNESS(runAllTests)
