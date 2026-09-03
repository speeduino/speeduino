#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllTests(void)
{
    extern void testIgnitionController(void);
    extern void testIgnitionScheduleInit(void);
    extern void testControllerCalcs(void);
    extern void testOverDwell(void);

    testIgnitionController();
    testIgnitionScheduleInit();
    testControllerCalcs();
    testOverDwell();
}

TEST_HARNESS(runAllTests)
