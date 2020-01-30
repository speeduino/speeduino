#include <Arduino.h>
#include <unity.h>

#include "test_schedule.h"

#include "init.h"

void do_tests()
{
    test_status_initial_off();
    test_status_off_to_pending();
    test_status_pending_to_running();
    test_status_running_to_pending();
    test_status_running_to_off();

    test_accuracy_timeout();
    test_accuracy_duration();
}

void setup() {
    initialiseAll();
    pinMode(LED_BUILTIN, OUTPUT);

    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN(); // start unit testing

    do_tests();

    UNITY_END(); // stop unit testing
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}