#include <globals.h>
#include <corrections.h>
#include <unity.h>
#include "test_corrections.h"
#include <secondaryTables.h>
#include "fuel_tables_setup.h"

void testCorrections()
{
  test_corrections_WUE();
  test_corrections_dfco();
  test_corrections_TAE(); //TPS based accel enrichment corrections
  test_corrections_cranking();
  test_corrections_ASE();
  RUN_TEST(test_corrections_flex);
  test_corrections_floodclear();
  test_corrections_bat();
  /*
  RUN_TEST(test_corrections_closedloop); //Not written yet
  
  RUN_TEST(test_corrections_iatdensity); //Not written yet
  RUN_TEST(test_corrections_baro); //Not written yet
  RUN_TEST(test_corrections_launch); //Not written yet
  RUN_TEST(test_corrections_dfco); //Not written yet
  */
  test_corrections_integration();
}
//**********************************************************************************************************************
void test_corrections_WUE_active(void)
{
  //Check for WUE being active
  currentStatus.coolant = 0;
  configPage2.flexEnabled = false;

  test_fuel_set_WUE_tables();
  correctionWUE();

  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_WARMUP, currentStatus.engine);
}
void test_corrections_WUE_inactive(void)
{
  //Check for WUE being inactive due to the temp being too high
  currentStatus.coolant = 200;
  configPage2.flexEnabled = false;

  test_fuel_set_WUE_tables();
  correctionWUE();

  TEST_ASSERT_BIT_LOW(BIT_ENGINE_WARMUP, currentStatus.engine);
}
void test_corrections_WUE_inactive_value(void)
{
  //Check for WUE being set to the final row of the WUE curve if the coolant is above the max WUE temp
  currentStatus.coolant = 200;
  configPage2.flexEnabled = false;

  test_fuel_set_WUE_tables();
  
  TEST_ASSERT_EQUAL(100, correctionWUE() );
}
void test_corrections_WUE_active_value(void)
{
  //Check for WUE being made active and returning a correct interpolated value
  currentStatus.coolant = 80;
  configPage2.flexEnabled = false;
  
  test_fuel_set_WUE_tables();
  
  //Value should be midway between 120 and 130 = 125
  TEST_ASSERT_EQUAL(125, correctionWUE() );
}
void test_corrections_WUE_active_flex_value(void)
{
  //Check for WUE being made active and returning a correct interpolated value
  currentStatus.coolant = 80;
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;

  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();

  //Value should be (1 - 0.7)125 + (0.7)220 = ~192
  TEST_ASSERT_EQUAL(192, correctionWUE() );
}
void test_corrections_WUE(void)
{
  RUN_TEST(test_corrections_WUE_active);
  RUN_TEST(test_corrections_WUE_inactive);
  RUN_TEST(test_corrections_WUE_active_value);
  RUN_TEST(test_corrections_WUE_inactive_value);
  RUN_TEST(test_corrections_WUE_active_flex_value);
}
//**********************************************************************************************************************
void test_corrections_cranking_active_value(void)
{
  //Check for cranking returning a correct interpolated value
  currentStatus.coolant = 90;
  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = false;

  test_fuel_set_cranking_tables();
  
  //Value should be midway between 190 and 140 = 165
  TEST_ASSERT_EQUAL(165, correctionCranking() );
}
void test_corrections_cranking_inactive_value(void)
{
  //Check for cranking to return 100 when not active
  currentStatus.coolant = 80;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = false;
  crankingEnrichTaper = 5;
  configPage10.crankingEnrichTaper = 3;

  test_fuel_set_cranking_tables();
  
  //Value should be 100 when crankingEnrich is inactive
  TEST_ASSERT_EQUAL(100, correctionCranking() );
}
void test_corrections_cranking_active_flex_value(void)
{
  //Check for cranking returning the proper flex correction
  currentStatus.coolant = 60;
  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 40;

  test_fuel_set_cranking_tables();
  test_fuel_set_flex_tables();
  
  //Value should be (1 - 0.47)190 + (0.47)400 = 
  TEST_ASSERT_EQUAL(289, correctionCranking() );
}
void test_corrections_cranking_inactive_flex_value(void)
{
  //Check for cranking to return 100 when inactive and flex is on
  currentStatus.coolant = 80;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;
  crankingEnrichTaper = 5;
  configPage10.crankingEnrichTaper = 3;

  test_fuel_set_cranking_tables();
  test_fuel_set_flex_tables();

  //Value should be 100 when crankingEnrich is inactive
  TEST_ASSERT_EQUAL(100, correctionCranking() );
}
void test_corrections_cranking(void)
{
  RUN_TEST(test_corrections_cranking_active_value);
  RUN_TEST(test_corrections_cranking_inactive_value);
  RUN_TEST(test_corrections_cranking_active_flex_value);
  RUN_TEST(test_corrections_cranking_inactive_flex_value);
}
//**********************************************************************************************************************
void test_corrections_ASE_inactive(void)
{
  //test condition: already running while warm
  currentStatus.coolant = 200;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  currentStatus.runSecs = 255;
  configPage2.flexEnabled = 0;
  configPage2.aseTaperTime = 1;
  aseTaper = 2;

  test_fuel_set_ASE_tables(); //included in each test so that each test can be run in isolation

  correctionASE();

  TEST_ASSERT_BIT_LOW(BIT_ENGINE_ASE, currentStatus.engine);
}
void test_corrections_ASE_inactive_value(void)
{
  //test condition: already running while warm
  currentStatus.coolant = 200;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  currentStatus.runSecs = 255;
  configPage2.flexEnabled = 0;
  configPage2.aseTaperTime = 1;
  aseTaper = 2;

  test_fuel_set_ASE_tables(); //included in each test so that each test can be run in isolation

  TEST_ASSERT_EQUAL(100, correctionASE());
}
void test_corrections_ASE_inactive_flex_value(void)
{
  //test condition: already running while warm
  currentStatus.coolant = 200;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  currentStatus.runSecs = 255;
  configPage2.flexEnabled = 1;
  configPage2.aseTaperTime = 1;
  aseTaper = 2;

  test_fuel_set_ASE_tables(); //included in each test so that each test can be run in isolation
  test_fuel_set_flex_tables();

  TEST_ASSERT_EQUAL(100, correctionASE());
}
void test_corrections_ASE_active(void)
{
  //test condition: cold start
  currentStatus.coolant = 50;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = 0;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  test_fuel_set_ASE_tables(); //included in each test so that each test can be run in isolation

  correctionASE();

  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ASE, currentStatus.engine);
}
void test_corrections_ASE_active_value(void)
{
  //test condition: cold start
  currentStatus.coolant = 0;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = 0;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  test_fuel_set_ASE_tables(); //included in each test so that each test can be run in isolation
  
  //Should be 100 + 100 = 200
  TEST_ASSERT_EQUAL(200, correctionASE());
}
void test_corrections_ASE_active_flex_value(void)
{
  //test condition: cold start
  currentStatus.coolant = 0;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = true;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;
  currentStatus.ethanolPct = 40;

  test_fuel_set_ASE_tables(); //included in each test so that each test can be run in isolation
  test_fuel_set_flex_tables();

  //Should be 100 + (1 - 0.47)100 + (0.47)1000 = 623
  TEST_ASSERT_EQUAL(623, correctionASE());
}
void test_corrections_ASE(void)
{
  RUN_TEST(test_corrections_ASE_active);
  RUN_TEST(test_corrections_ASE_inactive);
  RUN_TEST(test_corrections_ASE_active_value);
  RUN_TEST(test_corrections_ASE_inactive_value);
  RUN_TEST(test_corrections_ASE_active_flex_value);
  RUN_TEST(test_corrections_ASE_inactive_flex_value);
}
//**********************************************************************************************************************
void test_corrections_floodclear_inactive_cranking(void)
{
  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage4.floodClear = 90;
  currentStatus.TPS = 0;

  TEST_ASSERT_EQUAL(100, correctionFloodClear());
}
void test_corrections_floodclear_active_cranking(void)
{
  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage4.floodClear = 90;
  currentStatus.TPS = 100;

  TEST_ASSERT_EQUAL(0, correctionFloodClear());
}
void test_corrections_floodclear_inactive_running(void)
{
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(currentStatus.engine, BIT_ENGINE_RUN);
  configPage4.floodClear = 90;
  currentStatus.TPS = 100;

  TEST_ASSERT_EQUAL(100, correctionFloodClear());
}
void test_corrections_floodclear(void)
{
  RUN_TEST(test_corrections_floodclear_inactive_cranking);
  RUN_TEST(test_corrections_floodclear_active_cranking);
  RUN_TEST(test_corrections_floodclear_inactive_running);

}
//**********************************************************************************************************************
void test_corrections_closedloop(void)
{

}
//**********************************************************************************************************************
void test_corrections_flex(void)
{
  TEST_ASSERT_EQUAL(100, biasedAverage(0, 100, 200)); //0 bias, should return val1
  TEST_ASSERT_EQUAL(200, biasedAverage(100, 100, 200)); //100 bias, should return val2
  TEST_ASSERT_EQUAL(150, biasedAverage(50, 100, 200)); //should return 50% val1 + 50% val2
  TEST_ASSERT_EQUAL(150, biasedAverage(200, 50, 100));
  TEST_ASSERT_EQUAL(125, biasedAverage(150, 50, 100));
  TEST_ASSERT_EQUAL(255, biasedAverage(200, 100, 200)); //should return 255 for calculations that exceed 255

  TEST_ASSERT_EQUAL(100, biasedAverage_uint16(0, 100, 200)); //0 bias, should return val1
  TEST_ASSERT_EQUAL(200, biasedAverage_uint16(100, 100, 200)); //100 bias, should return val2
  TEST_ASSERT_EQUAL(150, biasedAverage_uint16(50, 100, 200)); //should return 50% val1 + 50% val2
  TEST_ASSERT_EQUAL(150, biasedAverage_uint16(200, 50, 100));
  TEST_ASSERT_EQUAL(125, biasedAverage_uint16(150, 50, 100));

  TEST_ASSERT_EQUAL(1000, biasedAverage_uint16(0, 1000, 2000)); //0 bias, should return val1
  TEST_ASSERT_EQUAL(2000, biasedAverage_uint16(100, 1000, 2000)); //100 bias, should return val2
  TEST_ASSERT_EQUAL(1500, biasedAverage_uint16(50, 1000, 2000)); //should return 50% val1 + 50% val2
  TEST_ASSERT_EQUAL(600, biasedAverage_uint16(200, 200, 400));
  TEST_ASSERT_EQUAL(600, biasedAverage_uint16(150, 300, 500));
}
//**********************************************************************************************************************
void test_corrections_bat_low_voltage(void)
{
  currentStatus.battery10 = 88;
  test_fuel_set_bat_correction_table();

  //should be halfway between 196 and 152 = 174
  TEST_ASSERT_EQUAL(174, correctionBatVoltage()); 
}
void test_corrections_bat_normal_voltage(void)
{
  currentStatus.battery10 = 138;
  test_fuel_set_bat_correction_table();

  //should be halfway between 100 and 90 = 95
  TEST_ASSERT_EQUAL(95, correctionBatVoltage()); 
}
void test_corrections_bat_high_voltage(void)
{
  currentStatus.battery10 = 154;
  test_fuel_set_bat_correction_table();

  //should be 90 - [(154 - 144) / (160 - 144) * (90 - 80)] = ~84
  TEST_ASSERT_EQUAL(84, correctionBatVoltage()); 
}
void test_corrections_bat(void)
{
  RUN_TEST(test_corrections_bat_low_voltage);
  RUN_TEST(test_corrections_bat_normal_voltage);
  RUN_TEST(test_corrections_bat_high_voltage);

}
//**********************************************************************************************************************
void test_corrections_iatdensity(void)
{

}
//**********************************************************************************************************************
void test_corrections_baro(void)
{

}
//**********************************************************************************************************************
void test_corrections_launch(void)
{

}
//**********************************************************************************************************************
void setup_DFCO_on()
{
  //Sets all the required conditions to have the DFCO be active
  configPage2.dfcoEnabled = 1; //Ensure DFCO option is turned on
  currentStatus.RPM = 4000; //Set the current simulated RPM to a level above the DFCO rpm threshold
  currentStatus.TPS = 0; //Set the simulated TPS to 0 
  currentStatus.coolant = 80;
  configPage4.dfcoRPM = 150; //DFCO enable RPM = 1500
  configPage4.dfcoTPSThresh = 1;
  configPage4.dfcoHyster = 50;
  configPage2.dfcoMinCLT = 40; //Actually 0 with offset
  configPage2.dfcoDelay = 10;

  dfcoTaper = 1;
  correctionDFCO();
  dfcoTaper = 20;
}
void test_corrections_dfco_on(void)
{
  //Test under ideal conditions that DFCO goes active
  setup_DFCO_on();

  TEST_ASSERT_TRUE(correctionDFCO());
}
void test_corrections_dfco_off_RPM()
{
  //Test that DFCO comes on and then goes off when the RPM drops below threshold
  setup_DFCO_on();

  TEST_ASSERT_TRUE(correctionDFCO()); //Make sure DFCO is on initially
  currentStatus.RPM = 1000; //Set the current simulated RPM below the threshold + hyster
  TEST_ASSERT_FALSE(correctionDFCO()); //Test DFCO is now off
}
void test_corrections_dfco_off_TPS()
{
  //Test that DFCO comes on and then goes off when the TPS goes above the required threshold (ie not off throttle)
  setup_DFCO_on();

  TEST_ASSERT_TRUE(correctionDFCO()); //Make sure DFCO is on initially
  currentStatus.TPS = 10; //Set the current simulated TPS to be above the threshold
  TEST_ASSERT_FALSE(correctionDFCO()); //Test DFCO is now off
}
void test_corrections_dfco_off_delay()
{
  //Test that DFCO comes will not activate if there has not been a long enough delay
  //The steup function below simulates a 2 second delay
  setup_DFCO_on();

  //Set the threshold to be 2.5 seconds, above the simulated delay of 2s
  configPage2.dfcoDelay = 250;

  TEST_ASSERT_FALSE(correctionDFCO()); //Make sure DFCO does not come on
}

void test_corrections_dfco()
{
  RUN_TEST(test_corrections_dfco_on);
  RUN_TEST(test_corrections_dfco_off_RPM);
  RUN_TEST(test_corrections_dfco_off_TPS);
  RUN_TEST(test_corrections_dfco_off_delay);
}
//**********************************************************************************************************************
void test_corrections_TAE_no_rpm_taper()
{
  test_corrections_TAE_setup(); //included in each test so that each test can be run in isolation

  //Disable the taper
  currentStatus.RPM = 2000;
  configPage2.aeTaperMin = 50; //5000
  configPage2.aeTaperMax = 60; //6000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+132), accelValue);
	TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC)); //Confirm AE is flagged on
}

void test_corrections_TAE_50pc_rpm_taper()
{
  test_corrections_TAE_setup(); //included in each test so that each test can be run in isolation

  //RPM is 50% of the way through the taper range
  currentStatus.RPM = 3000;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+66), accelValue);
	TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC)); //Confirm AE is flagged on
}

void test_corrections_TAE_110pc_rpm_taper()
{
  test_corrections_TAE_setup(); //included in each test so that each test can be run in isolation

  //RPM is 110% of the way through the taper range, which should result in no additional AE
  currentStatus.RPM = 5400;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC)); //Confirm AE is flagged on
}

void test_corrections_TAE_under_threshold()
{
  test_corrections_TAE_setup(); //included in each test so that each test can be run in isolation
  
  //RPM is 50% of the way through the taper range, but TPS value will be below threshold
  currentStatus.RPM = 3000;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 6; //3% actual value. TPSDot should be 90%/s
	configPage2.taeThresh = 100; //Above the reading of 90%/s

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(90, currentStatus.tpsDOT); //DOT is 90%/s (3% * 30)
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_FALSE(BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC)); //Confirm AE is flagged off
}

void test_corrections_TAE_50pc_warmup_taper()
{
  test_corrections_TAE_setup(); //included in each test so that each test can be run in isolation

  //Disable the RPM taper
  currentStatus.RPM = 2000;
  configPage2.aeTaperMin = 50; //5000
  configPage2.aeTaperMax = 60; //6000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value
	
	//Set a cold % of 50% increase
	configPage2.aeColdPct = 150;
	configPage2.aeColdTaperMax = 60 + CALIBRATION_TEMPERATURE_OFFSET;
	configPage2.aeColdTaperMin = 0 + CALIBRATION_TEMPERATURE_OFFSET;
	//Set the coolant to be 50% of the way through the warmup range
	currentStatus.coolant = 30;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+165), accelValue); //Total AE should be 132 + (50% * 50%) = 132 * 1.25 = 165
	TEST_ASSERT_TRUE(BIT_CHECK(currentStatus.engine, BIT_ENGINE_ACC)); //Confirm AE is flagged on
}

void test_corrections_TAE()
{
  RUN_TEST(test_corrections_TAE_no_rpm_taper);
  RUN_TEST(test_corrections_TAE_50pc_rpm_taper);
  RUN_TEST(test_corrections_TAE_110pc_rpm_taper);
  RUN_TEST(test_corrections_TAE_under_threshold);
  RUN_TEST(test_corrections_TAE_50pc_warmup_taper);
}

/***********************************************************************************************************************
 * INTEGRATION TESTS *
 ***********************************************************************************************************************/
void test_corrections_disable_AE(void)
{
  configPage2.aeMode = AE_MODE_TPS;
  currentStatus.TPS = 0;
  currentStatus.TPSlast = 0;
}
void test_corrections_disable_bat_correction(void)
{
  test_fuel_set_bat_correction_table();
  currentStatus.battery10 = 132; //100% correction, no change
}
void test_corrections_disable_IAT_correction(void)
{
  test_fuel_set_IAT_density_table();
  currentStatus.IAT = 75; //100% correction, no change
}
void test_corrections_disable_baro_correction(void)
{
  test_fuel_set_baro_table();
  currentStatus.baro = 100; //100% correction, no change
}
void test_corrections_disable_launch_correction(void)
{
  currentStatus.launchingHard = 0;
  currentStatus.launchingSoft = 0;
}
void test_corrections_disable_fuel_temp_correction(void)
{
  test_fuel_set_fuel_temp_correction_table();
  currentStatus.fuelTemp = 100;
}
void test_corrections_disable_closed_loop(void)
{
  configPage6.egoType = 0;
  configPage2.incorporateAFR = false;
}

void test_corrections_cranking_and_WUE_active_value_extreme_cold(void)
{
  //0 degrees cold start
  currentStatus.coolant = 0;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK); //ASE is disabled when cranking
  configPage2.flexEnabled = false;

  test_fuel_set_cranking_tables();
  test_fuel_set_WUE_tables();

  //Cranking value should be 225, WUE value should be 200, gammaE = 100 * 2.25 * 2 = 450
  TEST_ASSERT_EQUAL(450, correctionsFuel());
}

void test_corrections_WUE_and_ASE_active_value_extreme_cold(void)
{
  //0 degrees cold start
  currentStatus.coolant = 0;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = false;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  test_fuel_set_ASE_tables();
  test_fuel_set_WUE_tables();

  //ASE value should be 200, WUE value should be 200, gammaE = 100 * 2 * 2 = 400
  TEST_ASSERT_EQUAL(400, correctionsFuel());
}

void test_corrections_cranking_and_WUE_active_flex_value_extreme_cold(void)
{
  //0 degrees cold start
  currentStatus.coolant = 0;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;

  test_fuel_set_cranking_tables();
  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();

  /* 
  ** Cranking value should be (1 - 0.7)225 + (0.7)800 = ~628
  ** WUE value should be (1 - 0.7)200 + (0.7)400 = 340
  ** gammaE = 100 * 6.28 * 3.40 = 2135
  */
  TEST_ASSERT_EQUAL(2135, correctionsFuel());
}

void test_corrections_WUE_and_ASE_active_flex_value_extreme_cold(void)
{
  //0 degrees cold start
  currentStatus.coolant = 0;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;


  test_fuel_set_ASE_tables();
  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();
  
  /* 
  ** ASE value should be 100 + (1 - 0.7)100 + (0.7)1000 = 830
  ** WUE value should be (1 - 0.7)200 + (0.7)400 = 340
  ** gammaE = 100 * 8.30 * 3.40 = 2135
  */
  TEST_ASSERT_EQUAL(2822, correctionsFuel());
}

void test_corrections_cranking_and_WUE_active_value_mild_cold(void)
{
  //60 degrees cold start
  currentStatus.coolant = 60;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = false;

  test_fuel_set_cranking_tables();
  test_fuel_set_WUE_tables();

  //Cranking value should be 190, WUE value should be 140, gammaE = 100 * 1.90 * 1.40 = 266
  TEST_ASSERT_EQUAL(266, correctionsFuel());
}

void test_corrections_WUE_and_ASE_active_value_mild_cold(void)
{
  //60 degrees cold start
  currentStatus.coolant = 60;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = false;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;


  test_fuel_set_ASE_tables();
  test_fuel_set_WUE_tables();
  
  //ASE value should be 140, WUE value should be 140, gammaE = 100 * 1.4 * 1.4 = 196
  TEST_ASSERT_EQUAL(196, correctionsFuel());
}

void test_corrections_cranking_and_WUE_active_flex_value_mild_cold(void)
{
  //60 degrees cold start
  currentStatus.coolant = 60;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;

  test_fuel_set_cranking_tables();
  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();

  /* 
  ** Cranking value should be (1 - 0.7)190 + (0.7)400 = 337
  ** WUE value should be (1 - 0.7)140 + (0.7)275 = ~234
  ** gammaE = 100 * 3.37 * 2.34 = 792
  */

  uint16_t expected = 792;
  byte tolerance = expected / 50; //2% tolerance to account for rounding error
  TEST_ASSERT_UINT16_WITHIN(tolerance, expected, correctionsFuel());
}

void test_corrections_WUE_and_ASE_active_flex_value_mild_cold(void)
{
  //60 degrees cold start
  currentStatus.coolant = 60;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  test_fuel_set_ASE_tables();
  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();
  
  /* 
  ** ASE value should be 100 + (1 - 0.7)40 + (0.7)400 = 392
  ** WUE value should be (1 - 0.7)140 + (0.7)275 = 235
  ** gammaE = 100 * 3.92 * 2.35 = 921
  */

  uint16_t expected = 921;
  byte tolerance = expected / 50; //2% tolerance to account for rounding error
  TEST_ASSERT_UINT16_WITHIN(expected, tolerance, correctionsFuel());
}

void test_corrections_cranking_and_WUE_active_value_warm(void)
{
  //180 degrees start
  currentStatus.coolant = 180;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = false;

  test_fuel_set_cranking_tables();
  test_fuel_set_WUE_tables();

  //Cranking value should be 120, WUE value should be 100, gammaE = 100 * 1.2 * 1 = 120
  TEST_ASSERT_EQUAL(120, correctionsFuel());
}

void test_corrections_WUE_and_ASE_active_value_warm(void)
{
  //180 degrees start
  currentStatus.coolant = 180;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = false;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  test_fuel_set_ASE_tables();
  test_fuel_set_WUE_tables();

  //ASE value should be 105, WUE value should be 100, gammaE = 100 * 1.05 * 1 = 105
  TEST_ASSERT_EQUAL(105, correctionsFuel());
}

void test_corrections_cranking_and_WUE_active_flex_value_warm(void)
{
  //180 degrees start
  currentStatus.coolant = 180;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_SET(currentStatus.engine, BIT_ENGINE_CRANK);
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;

  test_fuel_set_cranking_tables();
  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();

  /* 
  ** Cranking value should be (1 - 0.7)120 + (0.7)130 = 127
  ** WUE value should be (1 - 0.7)100 + (0.7)100 = 100
  ** gammaE = 100 * 1.27 * 1 = 127
  */
  TEST_ASSERT_EQUAL(127, correctionsFuel());
}

void test_corrections_WUE_and_ASE_active_flex_value_warm(void)
{
  //180 degrees start
  currentStatus.coolant = 180;

  //set conditions to trigger only cranking and WUE
  test_corrections_disable_AE();
  configPage4.floodClear = 90; //should be disabled since TPS is set above
  test_corrections_disable_bat_correction();
  test_corrections_disable_IAT_correction();
  test_corrections_disable_baro_correction();
  configPage2.dfcoEnabled = 0;
  test_corrections_disable_launch_correction();
  test_corrections_disable_fuel_temp_correction();
  test_corrections_disable_closed_loop();

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 0;
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  test_fuel_set_ASE_tables();
  test_fuel_set_WUE_tables();
  test_fuel_set_flex_tables();
  
  /* 
  ** ASE value should be 100 + (1 - 0.7)5 + (0.7)20 = 116
  ** WUE value should be (1 - 0.7)100 + (0.7)100 = 100
  ** gammaE = 100 * 1.16 * 1 = 116
  */
  TEST_ASSERT_EQUAL(116, correctionsFuel());
}

void test_corrections_integration(void)
{
  RUN_TEST(test_corrections_cranking_and_WUE_active_value_extreme_cold);
  RUN_TEST(test_corrections_WUE_and_ASE_active_value_extreme_cold);
  RUN_TEST(test_corrections_cranking_and_WUE_active_flex_value_extreme_cold);
  RUN_TEST(test_corrections_WUE_and_ASE_active_flex_value_extreme_cold);

  RUN_TEST(test_corrections_cranking_and_WUE_active_value_mild_cold);
  RUN_TEST(test_corrections_WUE_and_ASE_active_value_mild_cold);
  RUN_TEST(test_corrections_cranking_and_WUE_active_flex_value_mild_cold);
  RUN_TEST(test_corrections_WUE_and_ASE_active_flex_value_mild_cold);

  RUN_TEST(test_corrections_cranking_and_WUE_active_value_warm);
  RUN_TEST(test_corrections_WUE_and_ASE_active_value_warm);
  RUN_TEST(test_corrections_cranking_and_WUE_active_flex_value_warm);
  RUN_TEST(test_corrections_WUE_and_ASE_active_flex_value_warm);
}