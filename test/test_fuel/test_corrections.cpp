#include <globals.h>
#include <corrections.h>
#include <unity.h>
#include "test_corrections.h"
#include <secondaryTables.h>


void testCorrections()
{
  test_corrections_WUE();
  test_corrections_dfco();
  test_corrections_TAE(); //TPS based accel enrichment corrections
  test_corrections_ASE();
  RUN_TEST(test_corrections_flex);
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
  configPage2.flexEnabled = false;
  ((uint8_t*)WUETable.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET; //Set a WUE end value of 120
  correctionWUE();
  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_WARMUP, currentStatus.engine);
}

void test_corrections_WUE_inactive(void)
{
  //Check for WUE being inactive due to the temp being too high
  currentStatus.coolant = 200;
  configPage2.flexEnabled = false;
  ((uint8_t*)WUETable.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET; //Set a WUE end value of 120
  correctionWUE();
  TEST_ASSERT_BIT_LOW(BIT_ENGINE_WARMUP, currentStatus.engine);
}

void test_corrections_WUE_inactive_value(void)
{
  //Check for WUE being set to the final row of the WUE curve if the coolant is above the max WUE temp
  currentStatus.coolant = 200;
  configPage2.flexEnabled = false;
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
  configPage2.flexEnabled = false;
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

void test_corrections_WUE_active_flex_value(void)
{
  //Check for WUE being made active and returning a correct interpolated value
  currentStatus.coolant = 80;
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 60;
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
  ((uint8_t*)WUETable.axisX)[0] = 0;
  
  ((uint8_t*)WUETable2.axisX)[1] = 0;
  ((uint8_t*)WUETable2.axisX)[2] = 0;
  ((uint8_t*)WUETable2.axisX)[3] = 0;
  ((uint8_t*)WUETable2.axisX)[4] = 0;
  ((uint8_t*)WUETable2.axisX)[5] = 0;
  ((uint8_t*)WUETable2.axisX)[6] = 70 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable2.axisX)[7] = 90 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable2.axisX)[8] = 100 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)WUETable2.axisX)[9] = 120 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)WUETable.values)[6] = 120;
  ((uint8_t*)WUETable.values)[7] = 130;
  ((uint8_t*)WUETable2.values)[6] = 200/10;
  ((uint8_t*)WUETable2.values)[7] = 240/10;

  set_flex_tables();

  //Force invalidate the cache
  WUETable.cacheTime = currentStatus.secl - 1;
  WUETable2.cacheTime = currentStatus.secl - 1;

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

void test_corrections_cranking(void)
{

}

void set_ASE_tables(void)
{
  //set duration table
  ((uint8_t*)ASECountTable.axisX)[0] = 0 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASECountTable.axisX)[1] = 40 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASECountTable.axisX)[2] = 120 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASECountTable.axisX)[3] = 180 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)ASECountTable.values)[0] = 16;
  ((uint8_t*)ASECountTable.values)[1] = 12;
  ((uint8_t*)ASECountTable.values)[2] = 3;
  ((uint8_t*)ASECountTable.values)[3] = 1;

  //set primary correction amount table
  ((uint8_t*)ASETable.axisX)[0] = 0 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASETable.axisX)[1] = 40 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASETable.axisX)[2] = 120 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASETable.axisX)[3] = 180 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)ASETable.values)[0] = 100;
  ((uint8_t*)ASETable.values)[1] = 40;
  ((uint8_t*)ASETable.values)[2] = 20;
  ((uint8_t*)ASETable.values)[3] = 5;

  //set secondary correction table
  ((uint8_t*)ASETable2.axisX)[0] = 0 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASETable2.axisX)[1] = 40 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASETable2.axisX)[2] = 120 + CALIBRATION_TEMPERATURE_OFFSET;
  ((uint8_t*)ASETable2.axisX)[3] = 180 + CALIBRATION_TEMPERATURE_OFFSET;

  ((uint8_t*)ASETable2.values)[0] = 600/5;
  ((uint8_t*)ASETable2.values)[1] = 400/5;
  ((uint8_t*)ASETable2.values)[2] = 50/5;
  ((uint8_t*)ASETable2.values)[3] = 20/5;
}
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

  set_ASE_tables();

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

  set_ASE_tables();

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

  set_ASE_tables();
  set_flex_tables();

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

  set_ASE_tables();

  correctionASE();

  TEST_ASSERT_BIT_HIGH(BIT_ENGINE_ASE, currentStatus.engine);
}
void test_corrections_ASE_active_value(void)
{
  //test condition: cold start
  currentStatus.coolant = 0;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = 0;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;

  set_ASE_tables();

  ASETable.cacheTime = currentStatus.secl - 1;
  
  //Should be 100 + 100 = 200
  TEST_ASSERT_EQUAL(200, correctionASE());
}
void test_corrections_ASE_active_flex_value(void)
{
  //test condition: cold start
  currentStatus.coolant = 0;
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_CRANK);
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  currentStatus.runSecs = 1;
  configPage2.flexEnabled = true;
  configPage2.aseTaperTime = 3;
  aseTaper = 0;
  currentStatus.ethanolPct = 40;

  set_ASE_tables();
  set_flex_tables();

  ASETable.cacheTime = currentStatus.secl - 1;
  ASETable2.cacheTime = currentStatus.secl - 1;

  //Should be 100 + (1 - 0.47)100 + (0.47)600 = 435
  TEST_ASSERT_EQUAL(435, correctionASE());
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

void test_corrections_floodclear(void)
{

}
void test_corrections_closedloop(void)
{

}

void set_flex_tables()
{
  //set flex fuel table
  ((uint8_t*)flexFuelTable.axisX)[0] = 0;
  ((uint8_t*)flexFuelTable.axisX)[1] = 20;
  ((uint8_t*)flexFuelTable.axisX)[2] = 40;
  ((uint8_t*)flexFuelTable.axisX)[3] = 60;
  ((uint8_t*)flexFuelTable.axisX)[4] = 85;
  ((uint8_t*)flexFuelTable.axisX)[5] = 100;

  ((uint8_t*)flexFuelTable.values)[0] = 0;
  ((uint8_t*)flexFuelTable.values)[1] = 24;
  ((uint8_t*)flexFuelTable.values)[2] = 47;
  ((uint8_t*)flexFuelTable.values)[3] = 70;
  ((uint8_t*)flexFuelTable.values)[4] = 100;
  ((uint8_t*)flexFuelTable.values)[5] = 110;

  //set flex ignition table
  ((uint8_t*)flexAdvTable.axisX)[0] = 0;
  ((uint8_t*)flexAdvTable.axisX)[1] = 20;
  ((uint8_t*)flexAdvTable.axisX)[2] = 40;
  ((uint8_t*)flexAdvTable.axisX)[3] = 60;
  ((uint8_t*)flexAdvTable.axisX)[4] = 85;
  ((uint8_t*)flexAdvTable.axisX)[5] = 100;

  ((uint8_t*)flexAdvTable.values)[0] = 0;
  ((uint8_t*)flexAdvTable.values)[1] = 23;
  ((uint8_t*)flexAdvTable.values)[2] = 46;
  ((uint8_t*)flexAdvTable.values)[3] = 69;
  ((uint8_t*)flexAdvTable.values)[4] = 100;
  ((uint8_t*)flexAdvTable.values)[5] = 120;
}
void test_corrections_flex(void)
{
  TEST_ASSERT_EQUAL(100, biasedAverage(0, 100, 200)); //0 bias, should return val1
  TEST_ASSERT_EQUAL(200, biasedAverage(100, 100, 200)); //100 bias, should return val2

  TEST_ASSERT_EQUAL(150, biasedAverage(50, 100, 200)); //should return 50% val1 + 50% val2
  TEST_ASSERT_EQUAL(150, biasedAverage(200, 50, 100));
  TEST_ASSERT_EQUAL(125, biasedAverage(150, 50, 100));

  TEST_ASSERT_EQUAL(255, biasedAverage(200, 100, 200)); //should return 255 for calculations that exceed 255


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
//**********************************************************************************************************************
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
//Setup a basic TAE enrichment curve, threshold etc that are common to all tests. Specifica values maybe updated in each individual test
void test_corrections_TAE_setup()
{
  configPage2.aeMode = AE_MODE_TPS; //Set AE to TPS

  configPage4.taeValues[0] = 70;
  configPage4.taeValues[1] = 103; 
  configPage4.taeValues[2] = 124;
  configPage4.taeValues[3] = 136; 

  //Note: These values are divided by 10
  configPage4.taeBins[0] = 0;
  configPage4.taeBins[1] = 8; 
  configPage4.taeBins[2] = 22;
  configPage4.taeBins[3] = 97; 
  
  configPage2.taeThresh = 0;
  configPage2.taeMinChange = 0;

  //Divided by 100
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000
	
	//Set the coolant to be above the warmup AE taper
	configPage2.aeColdTaperMax = 60;
	configPage2.aeColdTaperMin = 0;
	currentStatus.coolant = (int)(configPage2.aeColdTaperMax - CALIBRATION_TEMPERATURE_OFFSET) + 1;

  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Make sure AE is turned off
  BIT_CLEAR(currentStatus.engine, BIT_ENGINE_DCC); //Make sure AE is turned off
}

void test_corrections_TAE_no_rpm_taper()
{
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
  test_corrections_TAE_setup();


  RUN_TEST(test_corrections_TAE_no_rpm_taper);
	BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Flag must be cleared between tests
  RUN_TEST(test_corrections_TAE_50pc_rpm_taper);
	BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Flag must be cleared between tests
  RUN_TEST(test_corrections_TAE_110pc_rpm_taper);
	BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Flag must be cleared between tests
  RUN_TEST(test_corrections_TAE_under_threshold);
	BIT_CLEAR(currentStatus.engine, BIT_ENGINE_ACC); //Flag must be cleared between tests
  RUN_TEST(test_corrections_TAE_50pc_warmup_taper);
	
	
}