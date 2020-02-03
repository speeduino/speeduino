#include <globals.h>
#include <corrections.h>
#include <unity.h>
#include "tests_corrections.h"


void testCorrections()
{
  test_corrections_WUE();
  /*
  RUN_TEST(test_corrections_cranking); //Not written yet
  RUN_TEST(test_corrections_ASE); //Not written yet
  RUN_TEST(test_corrections_floodclear); //Not written yet
  RUN_TEST(test_corrections_closedloop); //Not written yet
  RUN_TEST(test_corrections_flex); //Not written yet
  RUN_TEST(test_corrections_bat); //Not written yet
  RUN_TEST(test_corrections_iatdensity); //Not written yet
  RUN_TEST(test_corrections_baro); //Not written yet
  RUN_TEST(test_corrections_launch); //Not written yet
  RUN_TEST(test_corrections_dfco); //Not written yet
  */
}

void test_corrections_WUE_active(void)
{
  //Check for WUE being active
  currentStatus.coolant = 0;
  ((uint8_t*)WUETable.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET; //Set a WUE end value of 120
  correctionWUE();
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_WARMUP, currentStatus.engine);
}

void test_corrections_WUE_inactive(void)
{
  //Check for WUE being inactive due to the temp being too high
  currentStatus.coolant = 200;
  ((uint8_t*)WUETable.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET; //Set a WUE end value of 120
  correctionWUE();
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_WARMUP, currentStatus.engine);
}

void test_corrections_WUE_inactive_value(void)
{
  //Check for WUE being set to the final row of the WUE curve if the coolant is above the max WUE temp
  currentStatus.coolant = 200;
  ((uint8_t*)WUETable.axisX)[9] = 100;
  ((uint8_t*)WUETable.values)[9] = 123; //Use a value other than 100 here to ensure we are using the non-default value

  //Force invalidate the cache
  WUETable.cacheTime = currentStatus.secl - 1;
  
  TEST_ASSERT_EQUAL(123, correctionWUE() );
}

void test_corrections_WUE_active_value(void)
{
  //Check for WUE being made active and returning a correct interpolated value
  currentStatus.coolant = 80;
  //Set some fake values in the table axis. Target value will fall between points 6 and 7
  ((uint8_t*)WUETable.axisX)[0] = 0;
  ((uint8_t*)WUETable.axisX)[1] = 0;
  ((uint8_t*)WUETable.axisX)[2] = 0;
  ((uint8_t*)WUETable.axisX)[3] = 0;
  ((uint8_t*)WUETable.axisX)[4] = 0;
  ((uint8_t*)WUETable.axisX)[5] = 0;
  ((uint8_t*)WUETable.axisX)[6] = 70 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[7] = 90 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[8] = 100 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)WUETable.values)[6] = 120;
  ((uint8_t*)WUETable.values)[7] = 130;

  //Force invalidate the cache
  WUETable.cacheTime = currentStatus.secl - 1;
  
  //Value should be midway between 120 and 130 = 125
  TEST_ASSERT_EQUAL(125, correctionWUE() );
}

void test_corrections_WUE(void)
{
  RUN_TEST(test_corrections_WUE_active);
  RUN_TEST(test_corrections_WUE_inactive);
  RUN_TEST(test_corrections_WUE_active_value);
  RUN_TEST(test_corrections_WUE_inactive_value);
}
void test_corrections_cranking(void)
{

}
void test_corrections_ASE(void)
{

}
void test_corrections_floodclear(void)
{

}
void test_corrections_closedloop(void)
{

}
void test_corrections_flex(void)
{

}
void test_corrections_bat(void)
{

}
void test_corrections_iatdensity(void)
{

}
void test_corrections_baro(void)
{

}
void test_corrections_launch(void)
{

}
void test_corrections_dfco(void)
{

}