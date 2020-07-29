#include <Arduino.h>
#include <unity.h>

#include "test_misc/tests_corrections.h"
#include "test_misc/tests_init.h"
#include "test_misc/tests_tables.h"
#include "test_misc/tests_PW.h"
#include "test_schedules/test_schedules.h"
#include "test_decoders/test_decoders.h"


void doTests()
{
    testInitialisation();
    testCorrections();
    testPW();
    //testSchedules(); //This is currently causing issues
    testDecoders();
    testTables();
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