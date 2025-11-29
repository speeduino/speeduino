#include <globals.h>
#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"

void test_Staging_setCommon(config2 &page2, config10 &page10, statuses &current)
{
  // initialiseAll();
  
  page2.nCylinders = 4;
  current.RPM = 3000;
  current.fuelLoad = 50;

  /*
      These values are a percentage of the req_fuel value that would be required for each injector channel to deliver that much fuel.
      Eg:
      Pri injectors are 250cc
      Sec injectors are 500cc
      Total injector capacity = 750cc

      staged_req_fuel_mult_pri = 300% (The primary injectors would have to run 3x the overall PW in order to be the equivalent of the full 750cc capacity
      staged_req_fuel_mult_sec = 150% (The secondary injectors would have to run 1.5x the overall PW in order to be the equivalent of the full 750cc capacity
  */
  page10.stagedInjSizePri = 250;
  page10.stagedInjSizeSec = 500;
  uint32_t totalInjector = page10.stagedInjSizePri + page10.stagedInjSizeSec;

  staged_req_fuel_mult_pri = (100 * totalInjector) / page10.stagedInjSizePri;
  staged_req_fuel_mult_sec = (100 * totalInjector) / page10.stagedInjSizeSec;
}

void test_Staging_Off(void)
{
  config2 page2;
  config10 page10;
  statuses current;
  test_Staging_setCommon(page2, page10, current);

  current.stagingActive = true;
  page10.stagingEnabled = false;

  calculateStaging(1000*4U, 9000, 1000, page2, page10, current);
  TEST_ASSERT_FALSE(current.stagingActive);
}

void test_Staging_4cyl_Auto_Inactive(void)
{
  config2 page2;
  config10 page10;
  statuses current;
  test_Staging_setCommon(page2, page10, current);

  current.stagingActive = true;
  page2.injLayout = INJ_PAIRED;
  page10.stagingEnabled = true;
  page10.stagingMode = STAGING_MODE_AUTO;

  calculateStaging(3000, 9000, 1000U, page2, page10, current);
  //PW 1 and 2 should be normal, 3 and 4 should be 0 as that testPW is below the pwLimit
  //PW1/2 should be ((PW - openTime) * staged_req_fuel_mult_pri) + openTime = ((3000 - 1000) * 3.0) + 1000 = 7000
  TEST_ASSERT_EQUAL(7000, current.PW1);
  TEST_ASSERT_EQUAL(7000, current.PW2);
  TEST_ASSERT_EQUAL(0, current.PW3);
  TEST_ASSERT_EQUAL(0, current.PW4);
  TEST_ASSERT_FALSE(current.stagingActive);
}

void test_Staging_4cyl_Table_Inactive(void)
{
  config2 page2;
  config10 page10;
  statuses current;
  test_Staging_setCommon(page2, page10, current);

  current.stagingActive = true;
  page2.injLayout = INJ_PAIRED;
  page10.stagingEnabled = true;
  page10.stagingMode = STAGING_MODE_TABLE;

  //Load the staging table with all 0
  //For this test it doesn't matter what the X and Y axis are, as the table is all 0 values
  for(byte x=0; x<64; x++) { stagingTable.values.values[x] = 0; }

  calculateStaging(3000, 9000, 1000, page2, page10, current);
  //PW 1 and 2 should be normal, 3 and 4 should be 0 as that testPW is below the pwLimit
  //PW1/2 should be (PW - openTime) * staged_req_fuel_mult_pri = (3000 - 1000) * 3.0 = 6000
  TEST_ASSERT_EQUAL(7000, current.PW1);
  TEST_ASSERT_EQUAL(7000, current.PW2);
  TEST_ASSERT_EQUAL(0, current.PW3);
  TEST_ASSERT_EQUAL(0, current.PW4);
  TEST_ASSERT_FALSE(current.stagingActive);
}

void test_Staging_4cyl_Auto_50pct(void)
{
  config2 page2;
  config10 page10;
  statuses current;
  test_Staging_setCommon(page2, page10, current);

  current.stagingActive = false;
  page2.injLayout = INJ_PAIRED;
  page10.stagingEnabled = true;
  page10.stagingMode = STAGING_MODE_AUTO;

  uint16_t pwLimit = 9000; //90% duty cycle at 6000rpm
  calculateStaging(9000, 9000U, 1000U, page2, page10, current);
  //PW 1 and 2 should be maxed out at the pwLimit, 3 and 4 should be based on their relative size
  TEST_ASSERT_EQUAL(pwLimit, current.PW1); //PW1/2 run at maximum available limit
  TEST_ASSERT_EQUAL(pwLimit, current.PW2);
  TEST_ASSERT_EQUAL(9000, current.PW3);
  TEST_ASSERT_EQUAL(9000, current.PW4);
  // TEST_ASSERT_TRUE(current.stagingActive);
}

void test_Staging_4cyl_Auto_33pct(void)
{
  config2 page2;
  config10 page10;
  statuses current;
  test_Staging_setCommon(page2, page10, current);

  current.stagingActive = false;
  page2.injLayout = INJ_PAIRED;
  page10.stagingEnabled = true;
  page10.stagingMode = STAGING_MODE_AUTO;

  uint32_t pwLimit = 9000; //90% duty cycle at 6000rpm
  calculateStaging(7000, pwLimit, 0, page2, page10, current);
  //PW 1 and 2 should be maxed out at the pwLimit, 3 and 4 should be based on their relative size
  TEST_ASSERT_EQUAL(pwLimit, current.PW1); //PW1/2 run at maximum available limit
  TEST_ASSERT_EQUAL(pwLimit, current.PW2);
  TEST_ASSERT_EQUAL(6000, current.PW3);
  TEST_ASSERT_EQUAL(6000, current.PW4);
  TEST_ASSERT_TRUE(current.stagingActive);
}

void test_Staging_4cyl_Table_50pct(void)
{
  config2 page2;
  config10 page10;
  statuses current;
  test_Staging_setCommon(page2, page10, current);

  current.stagingActive = false;
  page2.injLayout = INJ_PAIRED;
  page10.stagingEnabled = true;
  page10.stagingMode = STAGING_MODE_TABLE;

  //Load the staging table with all 0
  //For this test it doesn't matter what the X and Y axis are, as the table is all 50 values
  for(byte x=0; x<64; x++) { stagingTable.values.values[x] = 50; }


  //Need to change the lookup values so we don't get a cached value
  current.RPM += 1;
  current.fuelLoad += 1;

  calculateStaging(3000, 9000, 1000, page2, page10, current);

  TEST_ASSERT_EQUAL(4000, current.PW1);
  TEST_ASSERT_EQUAL(4000, current.PW2);
  TEST_ASSERT_EQUAL(2500, current.PW3);
  TEST_ASSERT_EQUAL(2500, current.PW4);
  TEST_ASSERT_TRUE(current.stagingActive);
}

void testStaging(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST(test_Staging_Off);
    RUN_TEST(test_Staging_4cyl_Auto_Inactive);
    RUN_TEST(test_Staging_4cyl_Table_Inactive);
    RUN_TEST(test_Staging_4cyl_Auto_50pct);
    RUN_TEST(test_Staging_4cyl_Auto_33pct);
    RUN_TEST(test_Staging_4cyl_Table_50pct);
  }
}

