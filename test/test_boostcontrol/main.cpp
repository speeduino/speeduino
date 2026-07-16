#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runBoostControlTests(void)
{
    extern void testBoostDisable(void);
    extern void testBoostInit(void);
    extern void testBoostControl(void);

    testBoostDisable();
    testBoostInit();
    testBoostControl();
}

TEST_HARNESS(runBoostControlTests)
