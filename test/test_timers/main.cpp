#include "../test_harness_device.h"
#include "../test_harness_native.h"


void runAllTests(void)
{
    extern void testInit(void);
    extern void testTacho(void);
    extern void testOneMsInterval(void);
    extern void testTestMode(void);
    extern void testFlex(void);

    testInit();
    testTacho();
    testOneMsInterval();
    testTestMode();
    testFlex();
}

TEST_HARNESS(runAllTests)
