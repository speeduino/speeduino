#include <unity.h>
#include "test_PW.h"
#include "../test_utils.h"
#include "globals.h"
#include "pw_calcs.h"
#include "pw_test_context.h"

#define PW_ALLOWED_ERROR  30

extern pulseWidths computePulseWidths(uint16_t REQ_FUEL, const config2 &page2, const config6 &page6, const config10 &page10, statuses &current);

// Convenience function
static pulseWidths computePulseWidths(uint16_t REQ_FUEL, ComputePulseWidthsContext &context) {
  return computePulseWidths(REQ_FUEL, context.page2, context.page6, context.page10, context.current);
}

static ComputePulseWidthsContext getBasicFullContext(void) {
  auto context = getBasicPwContext();
  context.page2.injOpen = 10;
  context.current.MAP = 94;
  context.current.VE = 130U;
  context.current.corrections = 113U;
  return context;
}

static constexpr uint16_t NO_MULTIPLY_EXPECTED = 2557U;

static void test_PW_batt_correction(void) {
  // Same as test_PW_No_Multiply, but we apply battery correction to open time
  auto context = getBasicFullContext();

  context.page2.battVCorMode = BATTV_COR_MODE_OPENTIME;
  context.current.batCorrection = 50U;
  uint16_t expectedOffset = (context.page2.injOpen*(100-context.current.batCorrection));

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_GREATER_THAN(PW_ALLOWED_ERROR, expectedOffset);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, NO_MULTIPLY_EXPECTED - expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_ae_adder(void) {
  auto context = getBasicFullContext();

  // Same as test_PW_No_Multiply, but we add in acceleration enrichment
  BIT_SET(context.current.engine, BIT_ENGINE_ACC);
  context.page2.aeApplyMode = AE_MODE_ADDER;
  context.current.AEamount = 105U;
  uint16_t reqFuel = 1060U;
  uint16_t expectedOffset = (reqFuel*(context.current.AEamount-100U))/100;

  pulseWidths result = computePulseWidths(reqFuel, context);
  TEST_ASSERT_GREATER_THAN(PW_ALLOWED_ERROR, expectedOffset);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, NO_MULTIPLY_EXPECTED + expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_nitrous_stage1(void) {
  // Same as test_PW_No_Multiply, but we add in nitrous
  auto context = getBasicFullContext();
  
  context.current.nitrous_status = NITROUS_STAGE1;
  context.current.RPMdiv100 = 24;
  setup_nitrous_stage1(context.page10, context.current);
  uint16_t expectedOffset = 820; // uS (3*100)+(1.0-(2350-(20*100))/((30-20)*100))*((11-3)*100)

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_GREATER_THAN(PW_ALLOWED_ERROR, expectedOffset);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, NO_MULTIPLY_EXPECTED + expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_nitrous_stage2(void) {
  // Same as test_PW_No_Multiply, but we add in nitrous
  auto context = getBasicFullContext();

  context.current.nitrous_status = NITROUS_STAGE2;
  context.current.RPMdiv100 = 27;
  setup_nitrous_stage2(context.page10, context.current);
  uint16_t expectedOffset = 520; // uS (1*100)+(1.0-(2650-(25*100))/((30-25)*100))*((7-1)*100)

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_GREATER_THAN(PW_ALLOWED_ERROR, expectedOffset);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, NO_MULTIPLY_EXPECTED + expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_nitrous_stageboth(void) {
  // Same as test_PW_No_Multiply, but we add in nitrous
  auto context = getBasicFullContext();

  setup_nitrous_stage1(context.page10, context.current);
  setup_nitrous_stage2(context.page10, context.current);
  context.current.RPMdiv100 = 27;
  context.current.nitrous_status = NITROUS_BOTH;
  uint16_t expectedOffset = 520+580; // uS

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_GREATER_THAN(PW_ALLOWED_ERROR, expectedOffset);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, NO_MULTIPLY_EXPECTED + expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
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
  RUN_TEST_P(test_PW_batt_correction);
  RUN_TEST_P(test_PW_ae_adder);
  RUN_TEST_P(test_PW_nitrous_stage1);
  RUN_TEST_P(test_PW_nitrous_stage2);
  RUN_TEST_P(test_PW_nitrous_stageboth);
  }
}

void test_PW_No_Multiply()
{
  auto context = getBasicFullContext();

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, NO_MULTIPLY_EXPECTED, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

void test_PW_MAP_Multiply()
{
  auto context = getBasicFullContext();

  context.page2.multiplyMAP = 1;
  context.current.baro = 103;
  context.page2.includeAFR = 0;
  context.page2.incorporateAFR = 0;
  context.page2.aeApplyMode = 0;

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, 2400, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

void test_PW_MAP_Multiply_Compatibility()
{
  auto context = getBasicFullContext();

  context.page2.multiplyMAP = 2; //Divide MAP reading by 100 rather than by Baro reading
  context.current.baro = 103;
  context.page2.includeAFR = 0;
  context.page2.incorporateAFR = 0;
  context.page2.aeApplyMode = 0;

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, 2449, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

void test_PW_AFR_Multiply()
{
  auto context = getBasicFullContext();

  context.page2.multiplyMAP = 0;
  context.current.baro = 100;
  context.page2.includeAFR = 1;
  context.page2.incorporateAFR = 0;
  context.page2.aeApplyMode = 0;
  context.page6.egoType = 2; //Set O2 sensor type to wideband
  context.current.runSecs = 20; configPage6.ego_sdelay = 10; //Ensure that the run time is longer than the O2 warmup time
  context.current.O2 = 150;
  context.current.afrTarget = 147;

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, 2588, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
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
  auto context = getBasicFullContext();
  
  context.current.corrections = 600;
  context.page2.multiplyMAP = 0;
  context.page2.includeAFR = 0;
  context.page2.incorporateAFR = 0;
  context.page2.aeApplyMode = 0;

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR, 9268, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

void test_PW_Very_Large_Correction()
{
  //This is the same as the test_PW_No_Multiply, but with correction changed to 1500
  auto context = getBasicFullContext();
  
  context.current.corrections = 1500;

  context.page2.multiplyMAP = 0;
  context.page2.includeAFR = 0;
  context.page2.incorporateAFR = 0;
  context.page2.aeApplyMode = 0;

  pulseWidths result = computePulseWidths(1060, context);
  TEST_ASSERT_UINT16_WITHIN(PW_ALLOWED_ERROR+30, 21670, result.primary); //Additional allowed error here 
  TEST_ASSERT_EQUAL(0, result.secondary);
}
