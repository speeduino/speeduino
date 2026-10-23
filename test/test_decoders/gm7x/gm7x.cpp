#include "../test_utils.h"

void testGM7X(void)
{
    extern void testGM7XCrankAngle(void);
    extern void testGM7XTriggers(void);
    extern void testGM7XPerToothIgn(void);

    testGM7XCrankAngle();
    testGM7XTriggers();
    testGM7XPerToothIgn();
}