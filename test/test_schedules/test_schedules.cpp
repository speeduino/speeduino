#include <Arduino.h>
#include <unity.h>
#include <init.h>
#include <avr/sleep.h>

#include "test_schedules.h"

extern void testScheduleStateMachine(void);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
#if !defined(SIMULATOR)
    delay(2000);
#endif

  UNITY_BEGIN(); // start unit testing

  initialiseAll(); //Run the main initialise function
  test_status_initial_off();
  //test_status_off_to_pending();
  //test_status_pending_to_running();
  //test_status_running_to_pending();
  //test_status_running_to_off();
  test_accuracy_timeout();
  test_accuracy_duration();
  testScheduleStateMachine();

  
  UNITY_END(); // stop unit testing

#if defined(SIMULATOR)       // Tell SimAVR we are done
    cli();
    sleep_enable();
    sleep_cpu();
#endif   
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
}