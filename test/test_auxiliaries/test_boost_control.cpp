#include <globals.h>
#include <auxiliaries.h>
#include <unity.h>
#include "test_boost_control.h"
#include <secondaryTables.h>
 

void testBoostControl()
{
  //RUN_TESTS
  RUN_TEST(test_boost_control_open_loop);
  RUN_TEST(test_boost_control_closed_loop);
  RUN_TEST(test_boost_control_open_loop_IAT_disable);
  RUN_TEST(test_boost_control_open_loop_CLT_disable);
  RUN_TEST(test_boost_control_closed_loop_IAT_disable);
  RUN_TEST(test_boost_control_closed_loop_CLT_disable);
  RUN_TEST(test_boost_control_flex_open_loop);
  RUN_TEST(test_boost_control_flex_closed_loop);
}

void test_boost_control_open_loop()
{

}

void test_boost_control_closed_loop()
{

}

void test_boost_control_open_loop_IAT_disable()
{

}

void test_boost_control_open_loop_CLT_disable()
{

}

void test_boost_control_closed_loop_IAT_disable()
{

}

void test_boost_control_closed_loop_CLT_disable()
{

}

void test_boost_control_flex_open_loop()
{

}

void test_boost_control_flex_closed_loop()
{

}