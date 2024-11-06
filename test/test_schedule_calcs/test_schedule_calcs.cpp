#include <Arduino.h>
#include <unity.h>
#include <init.h>
#include <avr/sleep.h>

extern void test_calc_ign_timeout();
extern void test_calc_inj_timeout();
extern void test_adjust_crank_angle();

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
#if !defined(SIMULATOR)
    delay(2000);
#endif

  UNITY_BEGIN(); // start unit testing

  test_calc_ign_timeout();
  test_calc_inj_timeout();
  test_adjust_crank_angle();
  
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