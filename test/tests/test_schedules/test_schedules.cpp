#include <Arduino.h>
#include <unity.h>

#include "test_schedules.h"
#include "globals.h"
/*
void two_second_blink()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(667);
    digitalWrite(LED_BUILTIN, LOW);
    delay(667);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(667);
    digitalWrite(LED_BUILTIN, LOW);

void test_schedules()
{
    test_status_initial_off();
    test_status_off_to_pending();
    test_status_pending_to_running();
    test_status_running_to_pending();
    test_status_running_to_off();

    test_accuracy_timeout();
    test_accuracy_duration();
}

void setup()
{
    initBoard(); // Not using initialiseAll() as it will cause memory overflow

    two_second_blink(); // Very important line

    UNITY_BEGIN(); // start unit testing

    do_tests();

    UNITY_END(); // stop unit testing

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}
*/
void testSchedules()
{
    
    test_status_initial_off();
    test_status_off_to_pending();
    test_status_pending_to_running();
    test_status_running_to_pending();
    
    TEST_ASSERT_GREATER_THAN(freeRam(), 100);
    test_status_running_to_off();

    test_accuracy_timeout();
    test_accuracy_duration();
}