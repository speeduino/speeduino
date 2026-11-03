#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testInit(void);
    extern void testOneMsInterval(void);
    extern void testTestMode(void);
    extern void testFlex(void);

    testInit();
    testOneMsInterval();
    testFlex();
}

TEST_HARNESS(runAllTests)
