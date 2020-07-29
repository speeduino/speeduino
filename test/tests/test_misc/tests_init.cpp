#include <globals.h>
#include <init.h>
#include <unity.h>
#include "tests_init.h"


void testInitialisation()
{
  RUN_TEST(test_initialisation_complete);
  RUN_TEST(test_initialisation_ports);
}

void test_initialisation_complete(void)
{
  initialiseAll(); //Run the main initialise function
  TEST_ASSERT_EQUAL(true, initialisationComplete);
}

void test_initialisation_ports(void)
{
  //Test that all the port values have been set
  initialiseAll(); //Run the main initialise function
  TEST_ASSERT_NOT_EQUAL(0, inj1_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, inj2_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, inj3_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, inj4_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, ign1_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, ign2_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, ign3_pin_port);
  TEST_ASSERT_NOT_EQUAL(0, ign4_pin_port);
}