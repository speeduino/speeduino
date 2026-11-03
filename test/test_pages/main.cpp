#include "../test_harness_device.h"
#include "../test_harness_native.h"

void runAllPageTests(void)
{
    extern void testPage(void);
    extern void testPageCrc(void);

    testPage();
    testPageCrc();
}

TEST_HARNESS(runAllPageTests)
