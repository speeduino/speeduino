#include <Arduino.h>
#include <unity.h>
#include "tests_crankmaths.h"

extern void testPercent(void);
extern void testDivision(void);
extern void testBitShift(void);
extern void test_LOW_PASS_FILTER(void);

#define UNITY_EXCLUDE_DETAILS

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!

    testCrankMaths();
    testPercent();
    testDivision();
    testBitShift();
    test_LOW_PASS_FILTER();

    UNITY_END(); // stop unit testing
}

void loop()
{
    // Blink to indicate end of test
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}