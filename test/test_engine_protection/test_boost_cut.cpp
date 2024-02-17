#include <globals.h>
#include <engineProtection.h>
#include <unity.h>
#include "test_boost_cut.h"
 

void testBoostCut()
{
  //RUN_TESTS
  RUN_TEST(test_boost_cut_basic);
  test_boost_cut_CLT();
  test_flex_boost_cut_basic();
  test_flex_CLT_boost_cut();

}

void test_boost_cut_basic()
{
  configPage6.boostCutEnabled = 1;
  configPage15.CLTBoostCutEnabled = 0;
  configPage6.boostLimit = 200/2; //set MAP limit to 200 kPa
  configPage2.flexEnabled = 0;

  currentStatus.MAP = 180;
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.MAP = 201;
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

//~~~~~~~~~~ CLT Boost Cut ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void test_boost_cut_CLT_warming()
{
  currentStatus.coolant = 100; 
  currentStatus.MAP = 139; //just under boost limit while warming up
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 159;
  currentStatus.MAP = 141; //exceeds boost limit while warming up
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void test_boost_cut_CLT_operating_temp()
{
  currentStatus.coolant = 185;
  currentStatus.MAP = 199; //just under boost limit at normal operating temp
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 185;
  currentStatus.MAP = 201; //exceeds boost limit at normal operating temp
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut

}

void test_boost_cut_CLT_overheating()
{
  currentStatus.coolant = 215; 
  currentStatus.MAP = 99; //just under boost limit while overheating
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 207;
  currentStatus.MAP = 141; //exceeds boost limit while overheating
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut

}

void test_boost_cut_CLT_extreme_overheating()
{
  currentStatus.coolant = 236; 
  currentStatus.MAP = 99; //just under boost limit while extreme overheating
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 236;
  currentStatus.MAP = 101; //exceeds boost limit while extreme overheating
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void set_CLT_boost_limit_table()
{
  ((uint8_t*)CLTBoostLimitTable.axisX)[0] = 160 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)CLTBoostLimitTable.axisX)[1] = 162 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)CLTBoostLimitTable.axisX)[2] = 205 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)CLTBoostLimitTable.axisX)[3] = 207 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)CLTBoostLimitTable.axisX)[4] = 214 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)CLTBoostLimitTable.axisX)[5] = 230 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)CLTBoostLimitTable.values)[0] = 140; //Don't want too much boost while engine is warming up
  ((uint8_t*)CLTBoostLimitTable.values)[1] = 200; //Proper operating temp range
  ((uint8_t*)CLTBoostLimitTable.values)[2] = 200;
  ((uint8_t*)CLTBoostLimitTable.values)[3] = 140; //Don't want too much boost when engine is beginning to overheat
  ((uint8_t*)CLTBoostLimitTable.values)[4] = 100;
  ((uint8_t*)CLTBoostLimitTable.values)[5] = 100;
}

void test_boost_cut_CLT()
{
  configPage6.boostCutEnabled = 1;
  configPage15.CLTBoostCutEnabled = 1;
  configPage2.flexEnabled = 0;
  set_CLT_boost_limit_table();

  RUN_TEST(test_boost_cut_CLT_warming);
  RUN_TEST(test_boost_cut_CLT_operating_temp);
  RUN_TEST(test_boost_cut_CLT_overheating);
  RUN_TEST(test_boost_cut_CLT_extreme_overheating);
}
//~~~~~~~~~~ END CLT Boost Cut ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~ Flex Boost Cut ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void test_flex_boost_cut_basic_no_eth()
{
  currentStatus.ethanolPct = 0;
  currentStatus.MAP = 199; //just under base boost limit
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.ethanolPct = 0;
  currentStatus.MAP = 201; //just over base boost limit
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut

}

void test_flex_boost_cut_basic_low_eth()
{
  currentStatus.ethanolPct = 20;
  currentStatus.MAP = 199; //just under base boost limit + 0 at low ethanol pct
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.ethanolPct = 20;
  currentStatus.MAP = 201; //just over base boost limit + 0 at low ethanol pct
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut 
}

void test_flex_boost_cut_basic_mid_eth()
{
  currentStatus.ethanolPct = 60;
  currentStatus.MAP = 229; //just under base boost limit + 30 at medium ethanol pct
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.ethanolPct = 60;
  currentStatus.MAP = 231; //just over base boost limit + 30 at medium ethanol pct
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void test_flex_boost_cut_basic_high_eth()
{
  currentStatus.ethanolPct = 85;
  currentStatus.MAP = 299; //just under base boost limit + 100 at high ethanol pct
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.ethanolPct = 85;
  currentStatus.MAP = 301; //just over base boost limit + 100 at high ethanol pct
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void set_flex_boost_limit_add_table()
{
  ((uint8_t*)flexBoostLimitAddTable.axisX)[0] = 10;
  ((uint8_t*)flexBoostLimitAddTable.axisX)[1] = 20;
  ((uint8_t*)flexBoostLimitAddTable.axisX)[2] = 40;
  ((uint8_t*)flexBoostLimitAddTable.axisX)[3] = 60;
  ((uint8_t*)flexBoostLimitAddTable.axisX)[4] = 80;
  ((uint8_t*)flexBoostLimitAddTable.axisX)[5] = 85;

  ((uint8_t*)flexBoostLimitAddTable.values)[0] = 0;
  ((uint8_t*)flexBoostLimitAddTable.values)[1] = 0;
  ((uint8_t*)flexBoostLimitAddTable.values)[2] = 20; //20 kPa extra, roughly 3 PSI
  ((uint8_t*)flexBoostLimitAddTable.values)[3] = 30; 
  ((uint8_t*)flexBoostLimitAddTable.values)[4] = 80; // little bit less than 12 extra PSI
  ((uint8_t*)flexBoostLimitAddTable.values)[5] = 100; // about 14 extra PSI
}

void test_flex_boost_cut_basic()
{
  configPage6.boostCutEnabled = 1;
  configPage15.CLTBoostCutEnabled = 0;
  configPage6.boostLimit = 200/2; //set base MAP limit to 200 kPa
  configPage2.flexEnabled = 1;

  RUN_TEST(test_flex_boost_cut_basic_no_eth);
  RUN_TEST(test_flex_boost_cut_basic_low_eth);
  RUN_TEST(test_flex_boost_cut_basic_mid_eth);
  RUN_TEST(test_flex_boost_cut_basic_high_eth);
}
//~~~~~~~~~~ End Flex Boost Cut ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~ CLT Flex Boost Cut ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void test_flex_CLT_boost_cut_warming_no_eth()
{
  currentStatus.coolant = 100; 
  currentStatus.MAP = 139; //just under boost limit while warming up + 0
  currentStatus.ethanolPct = 0;
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 159;
  currentStatus.MAP = 141; //exceeds boost limit while warming up + 0
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void test_flex_CLT_boost_cut_operating_temp_high_eth()
{
  currentStatus.coolant = 185; 
  currentStatus.MAP = 299; //just under boost limit while at operating temp + 100
  currentStatus.ethanolPct = 85;
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 185;
  currentStatus.MAP = 301; //exceeds boost limit while at operating temp + 100
  currentStatus.ethanolPct = 85;
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void test_flex_CLT_boost_cut_overheating_high_eth()
{
  currentStatus.coolant = 207; 
  currentStatus.MAP = 239; //just under boost limit while at overheating + 100
  currentStatus.ethanolPct = 85;
  checkBoostLimit();
  TEST_ASSERT_BIT_LOW(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should not trigger boost cut

  currentStatus.coolant = 207;
  currentStatus.MAP = 241; //exceeds boost limit while overheating + 100
  currentStatus.ethanolPct = 85;
  checkBoostLimit();
  TEST_ASSERT_BIT_HIGH(ENGINE_PROTECT_BIT_MAP, currentStatus.engineProtectStatus); //should trigger boost cut
}

void test_flex_CLT_boost_cut()
{
  configPage6.boostCutEnabled = 1;
  configPage15.CLTBoostCutEnabled = 1;
  configPage6.boostLimit = 200/2; //set base MAP limit to 200 kPa
  configPage2.flexEnabled = 1;

  set_CLT_boost_limit_table();
  set_flex_boost_limit_add_table();

  RUN_TEST(test_flex_CLT_boost_cut_warming_no_eth);
  RUN_TEST(test_flex_CLT_boost_cut_operating_temp_high_eth);
  RUN_TEST(test_flex_CLT_boost_cut_overheating_high_eth);
}
//~~~~~~~~~~ End CLT Flex Boost Cut ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~