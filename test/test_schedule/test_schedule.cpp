#include <Arduino.h>
#include <unity.h>

#include "globals.h"
#include "init.h"

#include "test_initial_status_off.h"
#include "test_tansit_status_pending.h"

void do_tests()
{
    UNITY_BEGIN(); // start unit testing
    test_initial_status_off();
    test_transit_status_pending();
    UNITY_END(); // stop unit testing
}

void setup() {
    initialiseAll();
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    do_tests();
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}