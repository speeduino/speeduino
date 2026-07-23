#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runBoostControlTests(void)
{
    extern void testBoostDisable(void);
    extern void testBoostInit(void);
    extern void testBoostControl(void);
    extern void testBoostInterrupt(void);

    testBoostDisable();
    testBoostInit();
    testBoostControl();
    testBoostInterrupt();
}

TEST_HARNESS(runBoostControlTests)
