#include <globals.h>
#include <auxiliaries.h>
#include <unity.h>
#include "test_boost_control.h"
#include <secondaryTables.h>
 

void testBoostControl()
{
  //RUN_TESTS
//   RUN_TEST(test_boost_table_value_lookup_basic);
//   RUN_TEST(test_boost_table_value_lookup_flex);
//   RUN_TEST(test_boost_control_IAT_disable);
//   RUN_TEST(test_boost_control_CLT_disable);
}

// void setBoostTables()
// {
//  return;
// }

void test_boost_table_value_lookup_basic()
{
  currentStatus.TPS = 100 / 2;
  currentStatus.RPM = 4500;
  //set boost tables
  //set flex boost tables
  //set ethanol pct
  //disable protections

}

void test_boost_table_value_lookup_flex()
{

}

void test_boost_control_IAT_disable()
{

}

void test_boost_control_CLT_disable()
{

}