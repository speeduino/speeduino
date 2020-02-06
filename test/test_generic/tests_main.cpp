#include <Arduino.h>
#include <unity.h>

#include "tests_corrections.h"
#include "tests_init.h"

void doTests()
{
    testInitialisation();
    testCorrections();
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();    // IMPORTANT LINE!

    doTests();

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