#include <unity.h>
#include "fuel_calcs.h"
#include "../test_utils.h"
#include "globals.h"

extern uint16_t PW(int REQ_FUEL, byte VE, long MAP, uint16_t corrections, int injOpen, const config10 &page10, const statuses &current);

#define PW_ALLOWED_ERROR  30

int16_t REQ_FUEL;
byte VE;
long MAP;
uint16_t corrections;
int injOpen;

void test_PW_setCommon()
{
  // initialiseAll();
  REQ_FUEL = 1100;
  VE = 130;
  MAP = 94;
  corrections = 113;
  injOpen = 1000;
  // Turns off pwLimit
  configPage2.dutyLim = 100;
  currentStatus.revolutionTime = UINT16_MAX;
  currentStatus.nSquirts = 1;
  // No staging
  configPage10.stagingEnabled = false;
  // Nitrous off
  currentStatus.nitrous_status = NITROUS_OFF;

  configPage2.injOpen = 10;
  currentStatus.batCorrection = 100;
}

static void setup_basic_pw(void) {
  configPage2.multiplyMAP = 0;
  configPage2.includeAFR = 0;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;
}

void test_PW_No_Multiply()
{
  test_PW_setCommon();
  setup_basic_pw();
  currentStatus.revolutionTime = 10000UL; //6000 rpm

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_EQUAL(2618, result);
}

void test_PW_MAP_Multiply()
{
  test_PW_setCommon();
  setup_basic_pw();

  configPage2.multiplyMAP = 1;
  currentStatus.baro = 103;
  configPage2.includeAFR = 0;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_EQUAL(2466, result);
}

void test_PW_MAP_Multiply_Compatibility()
{
  test_PW_setCommon();
  setup_basic_pw();

  configPage2.multiplyMAP = 2; //Divide MAP reading by 100 rather than by Baro reading
  currentStatus.baro = 103;
  configPage2.includeAFR = 0;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_EQUAL(2516, result);
}

void test_PW_AFR_Multiply()
{
  test_PW_setCommon();
  setup_basic_pw();

  configPage2.multiplyMAP = 0;
  currentStatus.baro = 100;
  configPage2.includeAFR = 1;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;
  configPage6.egoType = 2; //Set O2 sensor type to wideband
  currentStatus.runSecs = 20; configPage6.ego_sdelay = 10; //Ensure that the run time is longer than the O2 warmup time
  currentStatus.O2 = 150;
  currentStatus.afrTarget = 147;


  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_EQUAL(2643, result);
}

/*
  To avoid overflow errors, the PW() function reduces accuracy slightly when the corrections figure becomes large.
  There are 3 levels of this:
  1) Corrections below 511 - No change in accuracy
  2) Corrections between 512 and 1023 - Minor reduction to accuracy
  3) Corrections above 1023 - Further reduction to accuracy
*/
void test_PW_Large_Correction()
{
  //This is the same as the test_PW_No_Multiply, but with correction changed to 600
  test_PW_setCommon();
  setup_basic_pw();

  corrections = 600;

  configPage2.multiplyMAP = 0;
  configPage2.includeAFR = 0;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_EQUAL(9586, result);
}

void test_PW_Very_Large_Correction()
{
  //This is the same as the test_PW_No_Multiply, but with correction changed to 1500
  test_PW_setCommon();
  setup_basic_pw();

  corrections = 1500;

  configPage2.multiplyMAP = 0;
  configPage2.includeAFR = 0;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR+30, 22465, result); //Additional allowed error here 
}

void test_PW_Zero_Correction()
{
  //This is the same as the test_PW_No_Multiply, but with correction changed to 600
  test_PW_setCommon();
  setup_basic_pw();

  corrections = 0;

  configPage2.multiplyMAP = 0;
  configPage2.includeAFR = 0;
  configPage2.incorporateAFR = 0;
  configPage2.aeApplyMode = 0;

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_EQUAL(0, result);
}


//Test that unused pulse width values are set to 0
//This test is for a 4 cylinder using paired injection where only INJ 1 and 2 should have PW > 0
void test_PW_4Cyl_PW0(void)
{
  test_PW_setCommon();
  setup_basic_pw();

  configPage2.nCylinders = 4;
  configPage2.injLayout = INJ_PAIRED;
  configPage10.stagingEnabled = false; //Staging must be off or channels 3 and 4 will be used

  loop();
  TEST_ASSERT_EQUAL(0, currentStatus.PW3);
  TEST_ASSERT_EQUAL(0, currentStatus.PW4);
}

static constexpr uint16_t NO_MULTIPLY_EXPECTED = 2618U;


static void test_PW_ae_adder(void) {
  // Same as test_PW_No_Multiply, but we add in acceleration enrichment
  test_PW_setCommon();
  setup_basic_pw();

  currentStatus.isAcceleratingTPS = true;
  configPage2.aeApplyMode = AE_MODE_ADDER;
  currentStatus.AEamount = 105U;
  uint16_t expectedOffset = (REQ_FUEL*(currentStatus.AEamount-100U))/100;

  uint16_t result = PW(REQ_FUEL, VE, MAP, corrections, injOpen, configPage10, currentStatus);
  TEST_ASSERT_GREATER_THAN(PW_ALLOWED_ERROR, expectedOffset);
  TEST_ASSERT_EQUAL(NO_MULTIPLY_EXPECTED + expectedOffset, result);
}


void testPW(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_PW_No_Multiply);
    RUN_TEST_P(test_PW_MAP_Multiply);
    RUN_TEST_P(test_PW_MAP_Multiply_Compatibility);
    RUN_TEST_P(test_PW_AFR_Multiply);
    RUN_TEST_P(test_PW_Large_Correction);
    RUN_TEST_P(test_PW_Very_Large_Correction);
    RUN_TEST_P(test_PW_Zero_Correction);
    RUN_TEST_P(test_PW_4Cyl_PW0);
    RUN_TEST_P(test_PW_ae_adder);
  }
}
