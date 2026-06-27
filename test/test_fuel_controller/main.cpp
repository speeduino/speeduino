#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testFuelController(void);
    extern void testFuelScheduleInit(void);

    testFuelController();
    testFuelScheduleInit();
}

TEST_HARNESS(runAllTests)
