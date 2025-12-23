#include "../device_test_harness.h"

void runAllMathTests(void)
{
    extern void testCrankMaths(void);
    extern void testPercent(void);
    extern void testDivision(void);
    extern void test_LOW_PASS_FILTER(void);
    extern void test_fast_map(void);

    testCrankMaths();
    testPercent();
    testDivision();
    test_LOW_PASS_FILTER();
    test_fast_map();
}

DEVICE_TEST(runAllMathTests)
