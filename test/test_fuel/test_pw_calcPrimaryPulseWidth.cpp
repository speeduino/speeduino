#include <unity.h>
#include "../test_utils.h"
#include "pw_test_context.h"
#include "test_pw_applyNitrous.h"

extern uint16_t calcPrimaryPulseWidth(uint16_t injOpenTime, const config2 &page2, const config6 &page6, const config10 &page10, const statuses &current);

static uint16_t calcPrimaryPulseWidth(uint16_t REQ_FUEL, uint8_t VE, uint16_t MAP, uint16_t corrections, uint16_t injOpenTime, ComputePulseWidthsContext &context) {
  context.current.VE = VE;
  context.current.MAP = MAP;
  context.current.corrections = corrections; 
  context.page2.reqFuel = REQ_FUEL/100U;
  return calcPrimaryPulseWidth(injOpenTime, context.page2, context.page6, context.page10, context.current);
}

static void test_calcPrimaryPulseWidth_basic(void) {
  auto context = getBasicPwContext();

  // No correction (100)
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));
  // With +25% corrections (125)
  TEST_ASSERT_EQUAL(750+(750/4)+1, calcPrimaryPulseWidth(1000, 75, 1, 125, 0, context));
  // With -25% corrections (125)
  TEST_ASSERT_EQUAL(750-(750/4), calcPrimaryPulseWidth(1000, 75, 1,  75, 0, context));

  // With injection open time
  TEST_ASSERT_EQUAL(750+10, calcPrimaryPulseWidth(1000, 75, 1, 100, 10, context));
}

static ComputePulseWidthsContext getIncludeAFRContext(void) {
  auto context = getBasicPwContext();

  context.page6.egoType = EGO_TYPE_WIDE;
  context.page2.includeAFR = true;
  context.current.runSecs = context.page6.ego_sdelay + 1;
  context.current.afrTarget = 120;
  context.current.O2 = (context.current.afrTarget*3)/4;

  return context;
}

static void test_calcPrimaryPulseWidth_includeAFR(void) {
  // Positive test 
  auto context = getIncludeAFRContext();
  TEST_ASSERT_EQUAL(((750*3)/4), calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  // Should have no effect since O2==afrTarget
  context = getIncludeAFRContext();
  context.current.O2 = context.current.afrTarget;
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncludeAFRContext();
  context.page2.includeAFR = false;
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncludeAFRContext();
  context.page6.egoType = EGO_TYPE_OFF;
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));
}

static ComputePulseWidthsContext getIncorporateAFRContext(void) {
  auto context = getBasicPwContext();

  context.page2.includeAFR = false;
  context.page2.incorporateAFR = true;
  context.current.afrTarget = 120;
  context.page2.stoich = (context.current.afrTarget*3)/4;

  return context;
}

static void test_calcPrimaryPulseWidth_incorporateAFR(void) {
  auto context = getIncorporateAFRContext();

  // Positive test 
  TEST_ASSERT_EQUAL(((750*3)/4), calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  // Should have no effect since O2==afrTarget
  context = getIncorporateAFRContext();
  context.page2.stoich = context.current.afrTarget;
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncorporateAFRContext();
  context.page2.includeAFR = true;
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncorporateAFRContext();
  context.page2.incorporateAFR = false;
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));
}

static void test_calcPrimaryPulseWidth_MapMode100(void) {
  auto context = getBasicPwContext();

  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_100;
  // Equal so no effect
  TEST_ASSERT_EQUAL(750,         calcPrimaryPulseWidth(1000, 75, 100, 100, 0, context));
  // Map>100
  TEST_ASSERT_EQUAL(902 /*(750*MAP)/100*/, calcPrimaryPulseWidth(1000, 75, 120, 100, 0, context));
  // Map<100
  TEST_ASSERT_EQUAL(597 /*(750*MAP)/100*/, calcPrimaryPulseWidth(1000, 75, 80, 100, 0, context));
}

static void test_calcPrimaryPulseWidth_MapModeBaro(void) {
  auto context = getBasicPwContext();

  context.page2.multiplyMAP = MULTIPLY_MAP_MODE_BARO;
  // Equal so no effect
  context.current.baro = 100;
  TEST_ASSERT_EQUAL(750, calcPrimaryPulseWidth(1000, 75, context.current.baro,     100, 0, context));
  // MAP>baro
  context.current.baro = 90;
  TEST_ASSERT_EQUAL(996, calcPrimaryPulseWidth(1000, 75, context.current.baro+30U, 100, 0, context));
  // MAP<baro
  context.current.baro = 120;
  TEST_ASSERT_EQUAL(562, calcPrimaryPulseWidth(1000, 75, context.current.baro-30U, 100, 0, context));
}


static ComputePulseWidthsContext getIncludeAeContext(void) {
  auto context = getBasicPwContext();
  context.page2.aeApplyMode = AE_MODE_ADDER;
  context.current.AEamount = 105U;
  context.current.isAcceleratingTPS = true;
  return context;
}

static void test_calcPrimaryPulseWidth_AeAdder(void) {
  // AE needs to be added
  auto context = getIncludeAeContext();
  TEST_ASSERT_EQUAL(800 /* (1000*0.75)+(1000*0.05) */, calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  // AE off in all cases below
  context = getIncludeAeContext();
  context.current.AEamount = 100U;
  TEST_ASSERT_EQUAL(750, calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncludeAeContext();
  context.current.AEamount = 50U;
  TEST_ASSERT_EQUAL(750, calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncludeAeContext();
  context.current.isAcceleratingTPS = false;
  TEST_ASSERT_EQUAL(750, calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));

  context = getIncludeAeContext();
  context.page2.aeApplyMode = AE_MODE_MULTIPLIER;
  TEST_ASSERT_EQUAL(750, calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));
}

static void test_calcPrimaryPulseWidth_Nitrous(void) {
  // We only need to check that nitrous is added, as that is tested in pwApplyNitrous
  auto context = getBasicPwContext();
  setup_nitrous_stage1(context.page10, context.current);
  TEST_ASSERT_GREATER_THAN(750, calcPrimaryPulseWidth(1000, 75, 1, 100, 0, context));
}


void testCalcPrimaryPulseWidth(void)
{
  SET_UNITY_FILENAME() {
    RUN_TEST_P(test_calcPrimaryPulseWidth_basic);
    RUN_TEST_P(test_calcPrimaryPulseWidth_includeAFR);
    RUN_TEST_P(test_calcPrimaryPulseWidth_incorporateAFR);
    RUN_TEST_P(test_calcPrimaryPulseWidth_MapMode100);
    RUN_TEST_P(test_calcPrimaryPulseWidth_MapModeBaro);
    RUN_TEST_P(test_calcPrimaryPulseWidth_AeAdder);
    RUN_TEST_P(test_calcPrimaryPulseWidth_Nitrous);
  }  
}