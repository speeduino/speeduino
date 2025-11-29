#include <unity.h>
#include "../test_utils.h"
#include "fuel_calcs.h"
#include "test_pw_applyNitrous.h"

// The tests here don't have to be super detailed: computePulseWidths() just orchestrates
// calls to other functions which all have detailed tests.
//
// So we need broad tests here, rather than deep.


// Gather all inputs to the calculation into one place
struct ComputePulseWidthsContext {
  config2 page2 = {};
  config6 page6 = {};
  config10 page10 = {};
  statuses current = {};
};

static inline ComputePulseWidthsContext getBasicPwContext(void) {
  ComputePulseWidthsContext context = {};
  context.current.nitrous_status = NITROUS_OFF;
  context.page10.stagingEnabled = false;
  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_OFF;
  context.page2.includeAFR = false;
  context.page2.incorporateAFR = false; 
  context.page2.dutyLim = 100;
  context.page2.strokes = TWO_STROKE;
  context.current.revolutionTime = UINT16_MAX;
  context.current.nSquirts = 1;
  return context;
}

// Convenience function
static pulseWidths computePulseWidths(ComputePulseWidthsContext &context) {
  return computePulseWidths(context.page2, context.page6, context.page10, context.current);
}

static ComputePulseWidthsContext getBasicFullContext(void) {
  auto context = getBasicPwContext();
  context.page2.injOpen = 10;
  context.page2.reqFuel = 11;
  context.current.MAP = 94;
  context.current.VE = 130U;
  context.current.corrections = 113U;
  return context;
}

static constexpr uint16_t NO_MULTIPLY_EXPECTED = 1618U;

static void test_PW_batt_correction(void) {
  // Same as test_PW_No_Multiply, but we apply battery correction to open time
  auto context = getBasicFullContext();

  context.current.batCorrection = 50U;
  uint16_t expectedOffset = (context.page2.injOpen*(100-context.current.batCorrection));

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(NO_MULTIPLY_EXPECTED + expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_ae_adder(void) {
  auto context = getBasicFullContext();

  // Same as test_PW_No_Multiply, but we add in acceleration enrichment
  context.current.isAcceleratingTPS = true;
  context.page2.aeApplyMode = AE_MODE_ADDER;
  context.current.AEamount = 105U;
  uint16_t reqFuel = context.page2.reqFuel * 100U;
  uint16_t expectedOffset = (reqFuel*(context.current.AEamount-100U))/100;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(NO_MULTIPLY_EXPECTED + expectedOffset, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_nitrous_stageboth(void) {
  // Same as test_PW_No_Multiply, but we add in nitrous
  auto context = getBasicFullContext();

  setup_nitrous_stage1(context.page10, context.current);
  setup_nitrous_stage2(context.page10, context.current);
  context.current.RPMdiv100 = 27;
  context.current.nitrous_status = NITROUS_BOTH;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(NO_MULTIPLY_EXPECTED+1600, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_No_Multiply()
{
  auto context = getBasicFullContext();

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(NO_MULTIPLY_EXPECTED, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_MAP_Multiply()
{
  auto context = getBasicFullContext();

  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_BARO;
  context.current.baro = 103;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(1466, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_MAP_Multiply_Compatibility()
{
  auto context = getBasicFullContext();

  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_100; //Divide MAP reading by 100 rather than by Baro reading
//   context.current.MAP = 103;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(1516, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_AFR_Multiply()
{
  auto context = getBasicFullContext();

  context.page2.includeAFR = 1;
  context.page6.egoType = 2; //Set O2 sensor type to wideband
  context.page6.ego_sdelay = 10; 
  context.current.runSecs = context.page6.ego_sdelay+5; //Ensure that the run time is longer than the O2 warmup time
  context.current.O2 = 150;
  context.current.afrTarget = 147;
  context.current.baro = 100;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(1643, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

/*
  To avoid overflow errors, the PW() function reduces accuracy slightly when the corrections figure becomes large.
  There are 3 levels of this:
  1) Corrections below 511 - No change in accuracy
  2) Corrections between 512 and 1023 - Minor reduction to accuracy
  3) Corrections above 1023 - Further reduction to accuracy
*/
static void test_PW_Large_Correction()
{
  //This is the same as the test_PW_No_Multiply, but with correction changed to 600
  auto context = getBasicFullContext();
  
  context.current.corrections = 600;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(8586, result.primary);
  TEST_ASSERT_EQUAL(0, result.secondary);
}

static void test_PW_Very_Large_Correction()
{
  //This is the same as the test_PW_No_Multiply, but with correction changed to 1500
  auto context = getBasicFullContext();
  
  context.current.corrections = 1500;

  pulseWidths result = computePulseWidths(context);
  TEST_ASSERT_EQUAL(21465, result.primary); //Additional allowed error here 
  TEST_ASSERT_EQUAL(0, result.secondary);
}

void testComputePulseWidths(void)
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
    RUN_TEST_P(test_PW_nitrous_stageboth);
  }
}