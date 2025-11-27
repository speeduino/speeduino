#include "../device_test_harness.h"

void runAllDecoderTests(void)
{
    extern void testMissingTooth(void);
    extern void testDualWheel(void);
    extern void testRenix(void);
    extern void testNissan360(void);
    extern void testFordST170(void);
    extern void testNGC(void);
    extern void testSuzukiK6A_setEndTeeth(void);
    extern void testSuzukiK6A_getCrankAngle(void);
    extern void testDecoder_General(void);

    testMissingTooth();
    testDualWheel();
    testRenix();
    testNissan360();
    testFordST170();
    testNGC();
    testSuzukiK6A_setEndTeeth();
    testSuzukiK6A_getCrankAngle();
    testDecoder_General();
}

DEVICE_TEST(runAllDecoderTests)
