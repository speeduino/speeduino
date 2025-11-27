#include <unity.h>
#include "../test_utils.h"
#include "globals.h"
#include "corrections.h"
#include "init.h"
#include "sensors.h"
#include "sensors_map_structs.h"
#include "units.h"
#include "fuel_calcs.h"

extern byte correctionWUE(void);
extern table2D_u8_u8_10 WUETable; ///< 10 bin Warm Up Enrichment map (2D)

static void setup_wue_table(void) {
  initialiseCorrections();

  //Set some fake values in the table axis. Target value will fall between points 6 and 7
  TEST_DATA_P uint8_t bins[] = { 
    0, 0, 0, 0, 0, 0,
    temperatureAddOffset(70),
    temperatureAddOffset(90),
    temperatureAddOffset(100),
    temperatureAddOffset(120)
  };
  TEST_DATA_P uint8_t values[] = { 0, 0, 0, 0, 0, 0, 120, 130, 130, 130 };
  populate_2dtable_P(&WUETable, values, bins);
}

static void test_corrections_WUE_active(void)
{
  setup_wue_table();

  //Check for WUE being active
  currentStatus.coolant = 0;

  correctionWUE();
  TEST_ASSERT_TRUE(currentStatus.wueIsActive);
}

static void test_corrections_WUE_inactive(void)
{
  setup_wue_table();

  //Check for WUE being inactive due to the temp being too high
  currentStatus.coolant = 200;
  correctionWUE();
  TEST_ASSERT_FALSE(currentStatus.wueIsActive);
}

static void test_corrections_WUE_inactive_value(void)
{
  setup_wue_table();
  configPage4.wueBins[9] = 100;
  configPage2.wueValues[9] = 123; //Use a value other than 100 here to ensure we are using the non-default value

  //Check for WUE being set to the final row of the WUE curve if the coolant is above the max WUE temp
  currentStatus.coolant = 200;
  
  TEST_ASSERT_EQUAL(123, correctionWUE() );
}

static void test_corrections_WUE_active_value(void)
{
  //Check for WUE being made active and returning a correct interpolated value
  currentStatus.coolant = 80;

  setup_wue_table();

  //Force invalidate the cache
  WUETable.cache.cacheTime = currentStatus.secl - 1;
  
  //Value should be midway between 120 and 130 = 125
  TEST_ASSERT_EQUAL(125, correctionWUE() );
}

static void test_corrections_WUE(void)
{
  RUN_TEST_P(test_corrections_WUE_active);
  RUN_TEST_P(test_corrections_WUE_inactive);
  RUN_TEST_P(test_corrections_WUE_active_value);
  RUN_TEST_P(test_corrections_WUE_inactive_value);
}

extern uint16_t correctionCranking(void);
extern table2D_u8_u8_4 crankingEnrichTable; ///< 4 bin cranking Enrichment map (2D)

static void setup_correctionCranking_table(void) {
  initialiseCorrections();

  uint8_t values[] = { 120U / 5U, 130U / 5U, 140U / 5U, 150U / 5U };
  uint8_t bins[] = { 
    (uint8_t)(temperatureAddOffset(currentStatus.coolant) - 10U),
    (uint8_t)(temperatureAddOffset(currentStatus.coolant) + 10U),
    (uint8_t)(temperatureAddOffset(currentStatus.coolant) + 20U),
    (uint8_t)(temperatureAddOffset(currentStatus.coolant) + 30U)
  };
  populate_2dtable(&crankingEnrichTable, values, bins);
}

static void test_corrections_cranking_inactive(void) {
  initialiseCorrections();
  currentStatus.engineIsCranking = false;
  currentStatus.aseIsActive = false;
  configPage10.crankingEnrichTaper = 0U;

  TEST_ASSERT_EQUAL(100, correctionCranking() );
} 

static void test_corrections_cranking_cranking(void) {
  currentStatus.engineIsCranking = true;
  currentStatus.aseIsActive = false;
  configPage10.crankingEnrichTaper = 0U;
  currentStatus.coolant = temperatureRemoveOffset(150);
  setup_correctionCranking_table();

  // Should be half way between the 2 table values.
  TEST_ASSERT_EQUAL(125, correctionCranking() );
} 

static void test_corrections_cranking_taper_noase(void) {
  currentStatus.aseIsActive = false;
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 100U;
  currentStatus.ASEValue = 100U;
  
  currentStatus.coolant = temperatureRemoveOffset(150);
  setup_correctionCranking_table();

  // Reset taper
  currentStatus.engineIsCranking = true;
  (void)correctionCranking();

  // Advance taper to halfway
  currentStatus.engineIsCranking = false;
  for (uint8_t index=0; index<configPage10.crankingEnrichTaper/2U; ++index) {
    (void)correctionCranking();
  }

  // Should be half way between the interpolated table value and 100%.
  TEST_ASSERT_INT_WITHIN(1, 113U, correctionCranking() );
  
  // Final taper step
  for (uint8_t index=configPage10.crankingEnrichTaper/2U; index<configPage10.crankingEnrichTaper-2U; ++index) {
    (void)correctionCranking();
  }
  TEST_ASSERT_INT_WITHIN(1, 101U, correctionCranking() );

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionCranking());
  TEST_ASSERT_EQUAL(100U, correctionCranking());
} 


static void test_corrections_cranking_taper_withase(void) {
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage10.crankingEnrichTaper = 100U;
  
  currentStatus.coolant = temperatureRemoveOffset(150);
  setup_correctionCranking_table();

  currentStatus.aseIsActive = true;
  currentStatus.ASEValue = 50U;

  // Reset taper
  currentStatus.engineIsCranking = true;
  (void)correctionCranking();

  // Advance taper to halfway
  currentStatus.engineIsCranking = false;
  for (uint8_t index=0; index<configPage10.crankingEnrichTaper/2U; ++index) {
    (void)correctionCranking();
  }

  // Should be half way between the interpolated table value and 100%.
  TEST_ASSERT_INT_WITHIN(1, 175U, correctionCranking() );
  
  // Final taper step
  for (uint8_t index=configPage10.crankingEnrichTaper/2U; index<configPage10.crankingEnrichTaper-2U; ++index) {
    (void)correctionCranking();
  }
  TEST_ASSERT_INT_WITHIN(1, 102U, correctionCranking() );

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionCranking());
  TEST_ASSERT_EQUAL(100U, correctionCranking());
} 

static void test_corrections_cranking(void)
{
  RUN_TEST_P(test_corrections_cranking_inactive);
  RUN_TEST_P(test_corrections_cranking_cranking);
  RUN_TEST_P(test_corrections_cranking_taper_noase);
  RUN_TEST_P(test_corrections_cranking_taper_withase);
}

extern uint8_t correctionASE(void);

static void test_corrections_ASE_inactive_cranking(void)
{
  initialiseCorrections();
  currentStatus.engineIsCranking = true;

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionASE());
  TEST_ASSERT_FALSE(currentStatus.aseIsActive);
}

extern table2D_u8_u8_4 ASETable; ///< 4 bin After Start Enrichment map (2D)
extern table2D_u8_u8_4 ASECountTable; ///< 4 bin After Start duration map (2D)

static inline void setup_correctionASE(void) {
  initialiseCorrections();

  currentStatus.engineIsCranking = false;
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ) ;
  constexpr int16_t COOLANT_INITIAL = temperatureRemoveOffset(150); 
  currentStatus.coolant = COOLANT_INITIAL;
  currentStatus.ASEValue = 0U;
  currentStatus.runSecs = 3;

  {
    TEST_DATA_P uint8_t values[] = { 10, 8, 6, 4 };
    TEST_DATA_P uint8_t bins[] = { 
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) - 10U),
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) + 10U),
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) + 20U),
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) + 30U)
    };
    populate_2dtable_P(&ASECountTable, values, bins);
  }

  {
    TEST_DATA_P uint8_t values[] = { 20, 30, 40, 50 };
    TEST_DATA_P uint8_t bins[] = { 
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) - 10U),
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) + 10U),
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) + 20U),
      (uint8_t)(temperatureAddOffset(COOLANT_INITIAL) + 30U)
    };
    populate_2dtable_P(&ASETable, values, bins);
  } 
}

static void test_corrections_ASE_initial(void)
{
  setup_correctionASE();

  // Should be half way between the 2 table values.
  TEST_ASSERT_EQUAL(125, correctionASE());
  TEST_ASSERT_TRUE(currentStatus.aseIsActive);
}

static void test_corrections_ASE_taper(void) {
  setup_correctionASE();
  // Switch to ASE taper
  configPage2.aseTaperTime = 12U;
  currentStatus.runSecs = 9;

  // Advance taper to halfway
  currentStatus.engineIsCranking = false;
  for (uint8_t index=0; index<configPage2.aseTaperTime/2U; ++index) {
    (void)correctionASE();
  }

  // Should be half way between the interpolated table value and 100%.
  TEST_ASSERT_INT_WITHIN(1, 113, correctionASE());
  TEST_ASSERT_TRUE(currentStatus.aseIsActive);

  // Final taper step
  for (uint8_t index=configPage2.aseTaperTime/2U; index<configPage2.aseTaperTime-2U; ++index) {
    (void)correctionASE();
  }
  TEST_ASSERT_INT_WITHIN(1, 103U, correctionASE() );

  // Taper finished
  TEST_ASSERT_EQUAL(100U, correctionASE());  
  TEST_ASSERT_EQUAL(100U, correctionASE());  
}

static void test_corrections_ASE(void)
{
  RUN_TEST_P(test_corrections_ASE_inactive_cranking);
  RUN_TEST_P(test_corrections_ASE_initial);
  RUN_TEST_P(test_corrections_ASE_taper);
}

uint8_t correctionFloodClear(void);

static void test_corrections_floodclear_no_crank_inactive(void) {
  currentStatus.engineIsCranking = false;
  configPage4.floodClear = 90;
  currentStatus.TPS = configPage4.floodClear + 10;

  TEST_ASSERT_EQUAL(100U, correctionFloodClear() );
}

static void test_corrections_floodclear_crank_below_threshold_inactive(void) {
  currentStatus.engineIsCranking = true;
  configPage4.floodClear = 90;
  currentStatus.TPS = configPage4.floodClear - 10;

  TEST_ASSERT_EQUAL(100U, correctionFloodClear() );
}

static void test_corrections_floodclear_crank_above_threshold_active(void) {
  currentStatus.engineIsCranking = true;
  configPage4.floodClear = 90;
  currentStatus.TPS = configPage4.floodClear + 10;

  TEST_ASSERT_EQUAL(0U, correctionFloodClear() );
}

static void test_corrections_floodclear(void)
{
  RUN_TEST_P(test_corrections_floodclear_no_crank_inactive);
  RUN_TEST_P(test_corrections_floodclear_crank_below_threshold_inactive);
  RUN_TEST_P(test_corrections_floodclear_crank_above_threshold_active);
}

uint8_t correctionAFRClosedLoop(void);

static void setup_valid_ego_cycle(void) {
  AFRnextCycle = 4196;
  ignitionCount = AFRnextCycle + (configPage6.egoCount/2U); 
}

static void setup_ego_simple(void) {
  initialiseCorrections();

  configPage6.egoType = EGO_TYPE_NARROW;
  configPage6.egoAlgorithm = EGO_ALGORITHM_SIMPLE;
  configPage6.egoLimit = 30U;

  configPage6.ego_sdelay = 10;
  currentStatus.runSecs = configPage6.ego_sdelay + 2U;

  configPage6.egoTemp = 150U;
  currentStatus.coolant = temperatureRemoveOffset(configPage6.egoTemp) + 1; 

  configPage6.egoRPM = 30U;
  currentStatus.RPM = configPage6.egoRPM*100U + 1U;

  configPage6.egoTPSMax = 33;
  currentStatus.TPS = configPage6.egoTPSMax - 1U;

  configPage6.ego_max = 150U;
  configPage6.ego_min = 50U;
  currentStatus.O2 = configPage6.ego_min + ((configPage6.ego_max-configPage6.ego_min)/2U);

  configPage9.egoMAPMax = 100U;
  configPage9.egoMAPMin = 50U;
  currentStatus.MAP = (configPage9.egoMAPMin + ((configPage9.egoMAPMax-configPage9.egoMAPMin)/2U))*2U;
  
  currentStatus.afrTarget = currentStatus.O2;
  currentStatus.egoCorrection = 100U;
  
  currentStatus.isDFCOActive = false;

  configPage6.egoCount = 100U;
  setup_valid_ego_cycle();
}

static void test_corrections_closedloop_off_nosensor(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  configPage6.egoType = EGO_TYPE_OFF;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_dfco(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.isDFCOActive =  true;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_no_algorithm(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  configPage6.egoAlgorithm = EGO_ALGORITHM_NONE;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());

  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  configPage6.egoAlgorithm = EGO_ALGORITHM_INVALID1;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_coolant(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.coolant = temperatureRemoveOffset(configPage6.egoTemp) - 1; 
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_rpm(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.RPM = (configPage6.egoRPM*100U) - 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_tps(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.TPS = configPage6.egoTPSMax + 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_o2(void) {
  setup_ego_simple();
  currentStatus.O2 = configPage6.ego_min - 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());

  setup_ego_simple();
  currentStatus.O2 = configPage6.ego_max + 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_off_invalidconditions_map(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.MAP = (configPage9.egoMAPMin*2U) - 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());

  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.MAP = (configPage9.egoMAPMax*2U) + 1U;
  TEST_ASSERT_EQUAL(100U, correctionAFRClosedLoop());
}

//Test calling correctionAFRClosedLoop() before the required number of ignition cycles have taken place
static void test_corrections_closedloop_outsidecycle(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.egoCorrection = 123U;
  ignitionCount = AFRnextCycle - (configPage6.egoCount/2U); 
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection, correctionAFRClosedLoop());
}

//Test what happens when AFRnextCycle has rolled over but ignitionCount has not. 
//Expected that a correction should NOT occur in this instance
static void test_corrections_closedloop_cycle_countrollover(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  currentStatus.egoCorrection = 101U;
  ignitionCount = UINT16_MAX - (configPage6.egoCount/2); 
  AFRnextCycle = ignitionCount + configPage6.egoCount; //This will overflow AFRnextCycle
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_simple_nocorrection(void) {
  setup_ego_simple();
  currentStatus.egoCorrection = 101U;
  currentStatus.O2 = currentStatus.afrTarget;
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_simple_lean(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget + 1U;
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection+1U, correctionAFRClosedLoop());
}

static void test_corrections_closedloop_simple_lean_maxcorrection(void) {
  setup_ego_simple();

  currentStatus.O2 = configPage6.ego_max-1U;

  for (uint8_t index=0; index<configPage6.egoLimit; ++index) {
    setup_valid_ego_cycle();
    currentStatus.egoCorrection = 100U + index;
    TEST_ASSERT_EQUAL(currentStatus.egoCorrection+1U, correctionAFRClosedLoop());
  }
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());  
}

static void test_corrections_closedloop_simple_rich(void) {
  setup_ego_simple();
  currentStatus.O2 = currentStatus.afrTarget - 1U;
  TEST_ASSERT_EQUAL(currentStatus.egoCorrection-1U, correctionAFRClosedLoop());
}

static void test_rich_max_correction(void) {
  currentStatus.O2 = configPage6.ego_min+1U;

  uint8_t correction = 100U; 
  uint8_t counter = 0;
  while (correction>(100U-configPage6.egoLimit)) {
    setup_valid_ego_cycle();
    currentStatus.egoCorrection = 100U - counter;
    correction = correctionAFRClosedLoop();
    TEST_ASSERT_LESS_THAN(100U, correction);
    ++counter;
  }
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());  
}

static void test_corrections_closedloop_simple_rich_maxcorrection(void) {
  setup_ego_simple();

  test_rich_max_correction();
}

static void setup_ego_pid(void) {
  setup_ego_simple();
  configPage6.egoType = EGO_TYPE_WIDE;
  configPage6.egoAlgorithm = EGO_ALGORITHM_PID;  
  configPage6.egoKP = 50U;
  configPage6.egoKI = 20U;
  configPage6.egoKD = 10U;

  // Initial PID controller setup
  correctionAFRClosedLoop();
  setup_valid_ego_cycle();
}

// PID is time based and may need multiple cycles to move
static uint8_t run_pid(uint8_t cycles, uint8_t delayMillis) {
  for (uint8_t index=0; index<cycles-1U; ++index) {
    setup_valid_ego_cycle();
    // Serial.print(currentStatus.O2); Serial.print(" ");
    // Serial.print(currentStatus.afrTarget); Serial.print(" ");
    // Serial.println(correctionAFRClosedLoop());
    (void)correctionAFRClosedLoop();
    delay(delayMillis);
  }
  setup_valid_ego_cycle();
  return correctionAFRClosedLoop();
}

static void test_corrections_closedloop_pid_nocorrection(void) {
  setup_ego_pid();
  currentStatus.O2 = currentStatus.afrTarget;
  TEST_ASSERT_EQUAL(100U, run_pid(10, 10));
}

static void test_corrections_closedloop_pid_lean(void) {
  setup_ego_pid();
  currentStatus.O2 = configPage6.ego_max-1U;

  TEST_ASSERT_GREATER_THAN(100U, run_pid(10, 10));
}

static void test_corrections_closedloop_pid_lean_maxcorrection(void) {
  setup_ego_pid();

  currentStatus.O2 = configPage6.ego_max-1U;

  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, run_pid(40, 10));
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U+configPage6.egoLimit, correctionAFRClosedLoop());
}


static void test_corrections_closedloop_pid_rich(void) {
  setup_ego_pid();
  currentStatus.O2 = configPage6.ego_min+1U;
  TEST_ASSERT_LESS_THAN(100U, run_pid(10, 10));
}

static void test_corrections_closedloop_pid_rich_maxcorrection(void) {
  setup_ego_pid();

  currentStatus.O2 = configPage6.ego_min+1U;

  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, run_pid(40, 10));
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
  setup_valid_ego_cycle();
  TEST_ASSERT_EQUAL(100U-configPage6.egoLimit, correctionAFRClosedLoop());
}

static void test_corrections_closedloop(void)
{
  RUN_TEST_P(test_corrections_closedloop_off_nosensor);
  RUN_TEST_P(test_corrections_closedloop_off_dfco);
  RUN_TEST_P(test_corrections_closedloop_off_no_algorithm);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_coolant);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_rpm);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_tps);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_map);
  RUN_TEST_P(test_corrections_closedloop_off_invalidconditions_o2);
  RUN_TEST_P(test_corrections_closedloop_outsidecycle);
  RUN_TEST_P(test_corrections_closedloop_cycle_countrollover);
  RUN_TEST_P(test_corrections_closedloop_simple_nocorrection);
  RUN_TEST_P(test_corrections_closedloop_simple_lean);
  RUN_TEST_P(test_corrections_closedloop_simple_lean_maxcorrection);
  RUN_TEST_P(test_corrections_closedloop_simple_rich);
  RUN_TEST_P(test_corrections_closedloop_simple_rich_maxcorrection);
  RUN_TEST_P(test_corrections_closedloop_pid_nocorrection);
  RUN_TEST_P(test_corrections_closedloop_pid_lean);
  RUN_TEST_P(test_corrections_closedloop_pid_lean_maxcorrection);
  RUN_TEST_P(test_corrections_closedloop_pid_rich);
  RUN_TEST_P(test_corrections_closedloop_pid_rich_maxcorrection);
}

uint8_t correctionFlex(void);
extern table2D_u8_u8_6 flexFuelTable;  ///< 6 bin flex fuel correction table for fuel adjustments (2D)

static void setupFlexFuelTable(void) {
  initialiseCorrections();

  TEST_DATA_P uint8_t bins[] = { 0, 10, 30, 50, 60, 70 };
  TEST_DATA_P uint8_t values[] = { 0, 20, 40, 80, 120, 150 };
  populate_2dtable_P(&flexFuelTable, values, bins);  
}

static void test_corrections_flex_flex_off(void) {
  setupFlexFuelTable();
  configPage2.flexEnabled = false;
  currentStatus.ethanolPct = 65;
  TEST_ASSERT_EQUAL(100U, correctionFlex() );
}

static void test_corrections_flex_flex_on(void) {
  setupFlexFuelTable();
  configPage2.flexEnabled = true;
  currentStatus.ethanolPct = 65;
  TEST_ASSERT_EQUAL(135U, correctionFlex() );
}

uint8_t correctionFuelTemp(void);
extern table2D_u8_u8_6 fuelTempTable;  ///< 6 bin flex fuel correction table for fuel adjustments (2D)

static void setupFuelTempTable(void) {
  initialiseCorrections();

  TEST_DATA_P uint8_t bins[] = { 0, 10, 30, 50, 60, 70 };
  TEST_DATA_P uint8_t values[] = { 0, 20, 40, 80, 120, 150 };
  populate_2dtable_P(&fuelTempTable, values, bins);   
}

static void test_corrections_fueltemp_off(void) {
  setupFuelTempTable();
  configPage2.flexEnabled = false;
  currentStatus.fuelTemp = temperatureRemoveOffset(65);
  TEST_ASSERT_EQUAL(100U, correctionFuelTemp() );
}

static void test_corrections_fueltemp_on(void) {
  setupFuelTempTable();
  configPage2.flexEnabled = true;
  currentStatus.fuelTemp = temperatureRemoveOffset(65);
  TEST_ASSERT_EQUAL(135U, correctionFuelTemp() );
}

static void test_corrections_flex(void)
{
  RUN_TEST_P(test_corrections_flex_flex_off);
  RUN_TEST_P(test_corrections_flex_flex_on);
  RUN_TEST_P(test_corrections_fueltemp_off);
  RUN_TEST_P(test_corrections_fueltemp_on);
}

uint8_t correctionBatVoltage(void);
extern table2D_u8_u8_6 injectorVCorrectionTable; ///< 6 bin injector voltage correction (2D)

static void setup_battery_correction(void) {
  initialiseCorrections();

  TEST_DATA_P uint8_t bins[] = { 60, 70, 80, 90, 100, 110 };
  TEST_DATA_P uint8_t values[] = { 115, 110, 105, 100, 95, 90 };
  populate_2dtable_P(&injectorVCorrectionTable, values, bins);   
}

static void test_corrections_bat_normal(void) {
  setup_battery_correction();

  currentStatus.battery10 = 75;
  configPage2.injOpen = 10;
  uint8_t correctionValue = correctionBatVoltage();

  TEST_ASSERT_EQUAL(108U, correctionValue);
  correctionsFuel();
  TEST_ASSERT_EQUAL(configPage2.injOpen * correctionValue, inj_opentime_uS );
}

static void test_corrections_bat(void)
{
  RUN_TEST_P(test_corrections_bat_normal);
}

uint8_t correctionLaunch(void);

static void test_corrections_launch_inactive(void) {
  currentStatus.launchingHard = false;
  currentStatus.launchingSoft = false;
  configPage6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(100U, correctionLaunch() );
}

static void test_corrections_launch_hard(void) {
  currentStatus.launchingHard = true;
  currentStatus.launchingSoft = false;
  configPage6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(125U, correctionLaunch() );
}

static void test_corrections_launch_soft(void) {
  currentStatus.launchingHard = false;
  currentStatus.launchingSoft = true;
  configPage6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(125U, correctionLaunch() );
}

static void test_corrections_launch_both(void) {
  currentStatus.launchingHard = true;
  currentStatus.launchingSoft = true;
  configPage6.lnchFuelAdd = 25;

  TEST_ASSERT_EQUAL(125U, correctionLaunch() );
}

static void test_corrections_launch_removeFuel(void) {
  currentStatus.launchingHard = true;
  currentStatus.launchingSoft = true;
  configPage6.lnchFuelAdd = -25;

  TEST_ASSERT_EQUAL(75U, correctionLaunch() );
}

static void test_corrections_launch(void)
{
  RUN_TEST_P(test_corrections_launch_inactive);
  RUN_TEST_P(test_corrections_launch_hard);
  RUN_TEST_P(test_corrections_launch_soft);
  RUN_TEST_P(test_corrections_launch_both);
  RUN_TEST_P(test_corrections_launch_removeFuel);
}

extern bool correctionDFCO(void);

static void setup_DFCO_on_taper_off_no_delay()
{
  initialiseCorrections();

  //Sets all the required conditions to have the DFCO be active
  configPage2.dfcoEnabled = 1; //Ensure DFCO option is turned on
  currentStatus.RPM = 4000; //Set the current simulated RPM to a level above the DFCO rpm threshold
  currentStatus.TPS = 0; //Set the simulated TPS to 0 
  currentStatus.coolant = 80;
  configPage4.dfcoRPM = 150; //DFCO enable RPM = 1500
  configPage4.dfcoTPSThresh = 1;
  configPage4.dfcoHyster = 25;
  configPage2.dfcoMinCLT = 40; //Actually 0 with offset
  configPage2.dfcoDelay = 0;
  configPage9.dfcoTaperEnable = 0; //Enable

  correctionDFCO();
}

//**********************************************************************************************************************
static void test_corrections_dfco_on(void)
{
  //Test under ideal conditions that DFCO goes active
  setup_DFCO_on_taper_off_no_delay();

  TEST_ASSERT_TRUE(correctionDFCO());
}

static void test_corrections_dfco_off_RPM()
{
  //Test that DFCO comes on and then goes off when the RPM drops below threshold
  setup_DFCO_on_taper_off_no_delay();

  TEST_ASSERT_TRUE(correctionDFCO()); //Make sure DFCO is on initially
  currentStatus.RPM = 1000; //Set the current simulated RPM below the threshold + hyster
  TEST_ASSERT_FALSE(correctionDFCO()); //Test DFCO is now off
}

static void test_corrections_dfco_off_TPS()
{
  //Test that DFCO comes on and then goes off when the TPS goes above the required threshold (ie not off throttle)
  setup_DFCO_on_taper_off_no_delay();

  TEST_ASSERT_TRUE(correctionDFCO()); //Make sure DFCO is on initially
  currentStatus.TPS = 10; //Set the current simulated TPS to be above the threshold
  TEST_ASSERT_FALSE(correctionDFCO()); //Test DFCO is now off
}

static void test_corrections_dfco_off_delay()
{
  //Test that DFCO comes will not activate if there has not been a long enough delay
  //The steup function below simulates a 2 second delay
  setup_DFCO_on_taper_off_no_delay();

  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  configPage2.dfcoDelay = 5;
  
  for (uint8_t index = 0; index < configPage2.dfcoDelay; ++index) {
    TEST_ASSERT_FALSE(correctionDFCO()); //Make sure DFCO does not come on...
  }
  // ...until simulated delay period expires
  TEST_ASSERT_TRUE(correctionDFCO()); 
}

static void setup_DFCO_on_taper_on_no_delay()
{
  setup_DFCO_on_taper_off_no_delay();

  configPage9.dfcoTaperEnable = 1; //Enable
  configPage9.dfcoTaperTime = 20; //2.0 second
  configPage9.dfcoTaperFuel = 0; //Scale fuel to 0%
  configPage9.dfcoTaperAdvance = 20; //Reduce 20deg until full fuel cut
}

extern byte correctionDFCOfuel(void);

static void test_correctionDFCOfuel_DFCO_off()
{
  setup_DFCO_on_taper_off_no_delay();

  currentStatus.isDFCOActive = false;
  TEST_ASSERT_EQUAL(100, correctionDFCOfuel());
}

static void test_correctionDFCOfuel_notaper()
{
  setup_DFCO_on_taper_off_no_delay();

  configPage9.dfcoTaperEnable = 0; //Disable
  currentStatus.isDFCOActive =  true;
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel());
}

static inline void reset_dfco_taper(void) {
  currentStatus.isDFCOActive = false;
  TEST_ASSERT_EQUAL(100, correctionDFCOfuel());
  TEST_ASSERT_EQUAL(20, correctionDFCOignition(20));
}

static inline void advance_dfco_taper(uint8_t count) {
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  for (uint8_t index = 0; index < count; ++index) {
    (void)correctionDFCOfuel();
  }
}

static void test_correctionDFCOfuel_taper()
{
  setup_DFCO_on_taper_on_no_delay();

  reset_dfco_taper();

  currentStatus.isDFCOActive =  true;

  // 50% test
  advance_dfco_taper(configPage9.dfcoTaperTime/2);
  BIT_CLEAR(LOOP_TIMER, BIT_TIMER_10HZ);
  TEST_ASSERT_EQUAL(50, correctionDFCOfuel());

  // 75% test
  advance_dfco_taper(configPage9.dfcoTaperTime/4);
  BIT_CLEAR(LOOP_TIMER, BIT_TIMER_10HZ);
  TEST_ASSERT_EQUAL(25, correctionDFCOfuel());

  // Advance taper to 100%
  advance_dfco_taper(configPage9.dfcoTaperTime/4);

  // 100% & beyond test
  BIT_SET(LOOP_TIMER, BIT_TIMER_10HZ);
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel());
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel());
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel());
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel());
  TEST_ASSERT_EQUAL(0, correctionDFCOfuel());
}

extern int8_t correctionDFCOignition(int8_t advance);

static void test_correctionDFCOignition_DFCO_off()
{
  setup_DFCO_on_taper_off_no_delay();

  currentStatus.isDFCOActive = false;
  TEST_ASSERT_EQUAL(45, correctionDFCOignition(45));
}

static void test_correctionDFCOignition_notaper()
{
  setup_DFCO_on_taper_off_no_delay();

  configPage9.dfcoTaperEnable = 0; //Disable
  currentStatus.isDFCOActive =  true;
  TEST_ASSERT_EQUAL(45, correctionDFCOignition(45));
}

static void test_correctionDFCOignition_taper()
{
  setup_DFCO_on_taper_on_no_delay();

  reset_dfco_taper();

  currentStatus.isDFCOActive =  true;

  // 25% test
  advance_dfco_taper(configPage9.dfcoTaperTime/4);
  TEST_ASSERT_EQUAL(15, correctionDFCOignition(20));

  // 50% test
  advance_dfco_taper(configPage9.dfcoTaperTime/4);
  TEST_ASSERT_EQUAL(10, correctionDFCOignition(20));

  // 75% test
  advance_dfco_taper(configPage9.dfcoTaperTime/4);
  TEST_ASSERT_EQUAL(5, correctionDFCOignition(20));

  // 100% & beyond test
  advance_dfco_taper(configPage9.dfcoTaperTime/4);
  TEST_ASSERT_EQUAL(0, correctionDFCOignition(20));
  advance_dfco_taper(1);
  TEST_ASSERT_EQUAL(0, correctionDFCOignition(20));
  advance_dfco_taper(1);
  TEST_ASSERT_EQUAL(0, correctionDFCOignition(20));
}

static void test_corrections_dfco()
{
  RUN_TEST_P(test_corrections_dfco_on);
  RUN_TEST_P(test_corrections_dfco_off_RPM);
  RUN_TEST_P(test_corrections_dfco_off_TPS);
  RUN_TEST_P(test_corrections_dfco_off_delay);
  RUN_TEST_P(test_correctionDFCOfuel_DFCO_off);
  RUN_TEST_P(test_correctionDFCOfuel_notaper);
  RUN_TEST_P(test_correctionDFCOfuel_taper);
  RUN_TEST_P(test_correctionDFCOignition_DFCO_off);
  RUN_TEST_P(test_correctionDFCOignition_notaper);
  RUN_TEST_P(test_correctionDFCOignition_taper);
}

//**********************************************************************************************************************
//Setup a basic TAE enrichment curve, threshold etc that are common to all tests. Specifica values maybe updated in each individual test

static void reset_AE(void) {
  currentStatus.isAcceleratingTPS = false;
  currentStatus.isDeceleratingTPS = false;
}

static void setup_AE(void) {
  initialiseCorrections();

  //Divided by 100
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000
  configPage2.aeTime = 255;

	//Set the coolant to be above the warmup AE taper
	configPage2.aeColdTaperMax = 60;
	configPage2.aeColdTaperMin = 0;
	
  currentStatus.coolant = temperatureRemoveOffset(configPage2.aeColdTaperMax) + 1;
  currentStatus.AEEndTime = micros();

  reset_AE();
}

extern table2D_u8_u8_4 taeTable; ///< 4 bin TPS Acceleration Enrichment map (2D)

static void setup_TAE()
{
  setup_AE();

  configPage2.aeMode = AE_MODE_TPS; //Set AE to TPS

  TEST_DATA_P uint8_t bins[] = { 0, 8, 22, 97 };
  TEST_DATA_P uint8_t values[] = { 70, 103, 124, 136 };
  populate_2dtable_P(&taeTable, values, bins); 
  
  configPage2.taeThresh = 0;
  configPage2.taeMinChange = 0;
}

extern uint16_t correctionAccel(void);

static void disable_AE_taper(void) {
  //Disable the taper
  currentStatus.RPM = 2000;
  configPage2.aeTaperMin = 50; //5000
  configPage2.aeTaperMax = 60; //6000
}

static void test_corrections_TAE_no_rpm_taper()
{
  setup_TAE();
  disable_AE_taper();

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+132), accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // No change
  reset_AE();
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 50;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(0, currentStatus.tpsDOT);
  TEST_ASSERT_EQUAL(100, accelValue);
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged off
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Small change   
  reset_AE();
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 51;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(15, currentStatus.tpsDOT);
  TEST_ASSERT_EQUAL(100+74, accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Large change
  reset_AE();
  currentStatus.TPSlast = 0;
  currentStatus.TPS = 200;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(3000, currentStatus.tpsDOT);
  TEST_ASSERT_EQUAL(100+127, accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE_negative_tpsdot()
{
  setup_TAE();
  disable_AE_taper();

  configPage2.decelAmount = 50;
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 0;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(-750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL(configPage2.decelAmount, accelValue);
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_TRUE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE_50pc_rpm_taper()
{
  setup_TAE();

  //RPM is 50% of the way through the taper range
  currentStatus.RPM = 3000;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+66), accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE_110pc_rpm_taper()
{
  setup_TAE();

  //RPM is 110% of the way through the taper range, which should result in no additional AE
  currentStatus.RPM = 5400;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE_under_threshold()
{
  setup_TAE();

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
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged off
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE_50pc_warmup_taper()
{
  setup_TAE();
  disable_AE_taper();

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value
	
	//Set a cold % of 50% increase
	configPage2.aeColdPct = 150;
	configPage2.aeColdTaperMax = temperatureAddOffset(60);
	configPage2.aeColdTaperMin = temperatureAddOffset(0);
	//Set the coolant to be 50% of the way through the warmup range
	currentStatus.coolant = 30;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
  TEST_ASSERT_EQUAL((100+165), accelValue); //Total AE should be 132 + (50% * 50%) = 132 * 1.25 = 165
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
  TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE_timout()
{
  setup_TAE();
  disable_AE_taper();

  // Confirm TAE is on
  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value
  configPage2.aeTime = 0; // This should cause the current cycle to expire & the next one to not occur.

  TEST_ASSERT_EQUAL((100+132), correctionAccel());
  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
  
  // TAE should have timed out
  TEST_ASSERT_EQUAL(100, correctionAccel());
  TEST_ASSERT_EQUAL(0, currentStatus.tpsDOT);
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged off
  TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged off

  // But TPS hasn't changed position so another cycle should begin
  TEST_ASSERT_EQUAL((100+132), correctionAccel());
  TEST_ASSERT_EQUAL(750, currentStatus.tpsDOT); //DOT is 750%/s (25 * 30)
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_TAE()
{
  RUN_TEST_P(test_corrections_TAE_negative_tpsdot);
  RUN_TEST_P(test_corrections_TAE_no_rpm_taper);
  RUN_TEST_P(test_corrections_TAE_50pc_rpm_taper);
  RUN_TEST_P(test_corrections_TAE_110pc_rpm_taper);
  RUN_TEST_P(test_corrections_TAE_under_threshold);
  RUN_TEST_P(test_corrections_TAE_50pc_warmup_taper);
  RUN_TEST_P(test_corrections_TAE_timout);
}

extern map_last_read_t& getMapLast(void);

//**********************************************************************************************************************
//Setup a basic MAE enrichment curve, threshold etc that are common to all tests. Specifica values maybe updated in each individual test

extern table2D_u8_u8_4 maeTable;

static void setup_MAE(void)
{
  setup_AE();

  configPage2.aeMode = AE_MODE_MAP; //Set AE to TPS

  TEST_DATA_P uint8_t bins[] = { 0, 15, 19, 50 };
  TEST_DATA_P uint8_t values[] = { 70, 103, 124, 136 };
  populate_2dtable_P(&maeTable, values, bins); 

  configPage2.maeThresh = 0;
  configPage2.maeMinChange = 0;
}

static void test_corrections_MAE_negative_mapdot()
{
  setup_MAE();
  disable_AE_taper();

  configPage2.decelAmount = 50;
  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 50;
  currentStatus.MAP = 40;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(-400, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL(configPage2.decelAmount, accelValue);
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_TRUE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_MAE_no_rpm_taper()
{
  setup_MAE();
  disable_AE_taper();

  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 50;
  uint16_t accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(400, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL((100+132), accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // No change
  reset_AE();
  getMapLast().timeDeltaReadings = 1000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 40;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(0, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL(100, accelValue);
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged off
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Small change over small time period  
  reset_AE();
  getMapLast().timeDeltaReadings = 1000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 41;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(1000, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL((100+136), accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Small change over long (>UINT16_MAX) time period  
  reset_AE();
  getMapLast().timeDeltaReadings = UINT16_MAX*2UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 41;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(7, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL(100+70, accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Large change over small time period  
  reset_AE();
  getMapLast().timeDeltaReadings = 1000UL; 
  getMapLast().lastMAPValue = 10;
  currentStatus.MAP = 1000;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(6960, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL((100+136), accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Large change over long (>UINT16_MAX) time period  
  reset_AE();
  getMapLast().timeDeltaReadings = UINT16_MAX*2UL; 
  getMapLast().lastMAPValue = 10;
  currentStatus.MAP = 1000;
  accelValue = correctionAccel(); //Run the AE calcs
  TEST_ASSERT_EQUAL(6930, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL(100+136, accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged pn  
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_MAE_50pc_rpm_taper()
{
  setup_MAE();

  //RPM is 50% of the way through the taper range
  currentStatus.RPM = 3000;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 50;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(400, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL((100+66), accelValue);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_MAE_110pc_rpm_taper()
{
  setup_MAE();

  //RPM is 110% of the way through the taper range, which should result in no additional AE
  currentStatus.RPM = 5400;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 50;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(400, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_MAE_under_threshold()
{
  setup_MAE();

  //RPM is 50% of the way through the taper range, but TPS value will be below threshold
  currentStatus.RPM = 3000;
  configPage2.aeTaperMin = 10; //1000
  configPage2.aeTaperMax = 50; //5000

  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 0;
  currentStatus.MAP = 6; 
	configPage2.maeThresh = 241; //Above the reading of 240%/s

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(240, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL(100, accelValue); //Should be no AE as we're above the RPM taper end point
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged off
}

static void test_corrections_MAE_50pc_warmup_taper()
{
  setup_MAE();
  disable_AE_taper();

  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 50;

	//Set a cold % of 50% increase
	configPage2.aeColdPct = 150;
	configPage2.aeColdTaperMax = temperatureAddOffset(60);
	configPage2.aeColdTaperMin = temperatureAddOffset(0);
	//Set the coolant to be 50% of the way through the warmup range
	currentStatus.coolant = 30;

  uint16_t accelValue = correctionAccel(); //Run the AE calcs

  TEST_ASSERT_EQUAL(400, currentStatus.mapDOT);
  TEST_ASSERT_EQUAL((100+165), accelValue); 
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
}

static void test_corrections_MAE_timout()
{
  setup_MAE();
  disable_AE_taper();

  // Confirm MAE is on
  getMapLast().timeDeltaReadings = 25000UL; 
  getMapLast().lastMAPValue = 40;
  currentStatus.MAP = 50;
  configPage2.aeTime = 0; // This should cause the current cycle to expire & the next one to not occur.
  TEST_ASSERT_EQUAL((100+132), correctionAccel());
  TEST_ASSERT_EQUAL(400, currentStatus.mapDOT);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // Timeout TAE
  TEST_ASSERT_EQUAL(100, correctionAccel());
  TEST_ASSERT_EQUAL(0, currentStatus.mapDOT);
	TEST_ASSERT_FALSE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
  TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on

  // But we haven't changed MAP readings so another cycle should begin
  TEST_ASSERT_EQUAL((100+132), correctionAccel());
  TEST_ASSERT_EQUAL(400, currentStatus.mapDOT);
	TEST_ASSERT_TRUE(currentStatus.isAcceleratingTPS); //Confirm AE is flagged on
	TEST_ASSERT_FALSE(currentStatus.isDeceleratingTPS); //Confirm AE is flagged on
}


static void test_corrections_MAE()
{
  RUN_TEST_P(test_corrections_MAE_negative_mapdot);
  RUN_TEST_P(test_corrections_MAE_no_rpm_taper);
  RUN_TEST_P(test_corrections_MAE_50pc_rpm_taper);
  RUN_TEST_P(test_corrections_MAE_110pc_rpm_taper);
  RUN_TEST_P(test_corrections_MAE_under_threshold);
  RUN_TEST_P(test_corrections_MAE_50pc_warmup_taper);
  RUN_TEST_P(test_corrections_MAE_timout);
}

static void setup_afrtarget(table3d16RpmLoad &afrLookUpTable,
                            statuses &current,
                            config2 &page2,
                            config6 &page6) {
  TEST_DATA_P table3d_value_t values[] = {
    //0    1    2   3     4    5    6    7    8    9   10   11   12   13    14   15
    34,  34,  34,  34,  34,  34,  34,  34,  34,  35,  35,  35,  35,  35,  35,  35, 
    34,  35,  36,  37,  39,  41,  42,  43,  43,  44,  44,  44,  44,  44,  44,  44, 
    35,  36,  38,  41,  44,  46,  47,  48,  48,  49,  49,  49,  49,  49,  49,  49, 
    36,  39,  42,  46,  50,  51,  52,  53,  53,  53,  53,  53,  53,  53,  53,  53, 
    38,  43,  48,  52,  55,  56,  57,  58,  58,  58,  58,  58,  58,  58,  58,  58, 
    42,  49,  54,  58,  61,  62,  62,  63,  63,  63,  63,  63,  63,  63,  63,  63, 
    48,  56,  60,  64,  66,  66,  68,  68,  68,  68,  68,  68,  68,  68,  68,  68, 
    54,  62,  66,  69,  71,  71,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72, 
    61,  69,  72,  74,  76,  76,  77,  77,  77,  77,  77,  77,  77,  77,  77,  77, 
    68,  75,  78,  79,  81,  81,  81,  82,  82,  82,  82,  82,  82,  82,  82,  82, 
    74,  80,  83,  84,  85,  86,  86,  86,  87,  87,  87,  87,  87,  87,  87,  87, 
    81,  86,  88,  89,  90,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91,  91, 
    93,  96,  98,  99,  99,  100, 100, 101, 101, 101, 101, 101, 101, 101, 101, 101, 
    98,  101, 103, 103, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 
    104, 106, 107, 108, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 
    109, 111, 112, 113, 114, 114, 114, 115, 115, 115, 114, 114, 114, 114, 114, 114, 
    };
  TEST_DATA_P table3d_axis_t xAxis[] = {500U/100U, 700U/100U, 900U/100U, 1200U/100U, 1600U/100U, 2000U/100U, 2500U/100U, 3100U/100U, 3500U/100U, 4100U/100U, 4700U/100U, 5300U/100U, 5900U/100U, 6500U/100U, 6750U/100U, 7000U/100U};
  TEST_DATA_P table3d_axis_t yAxis[] = { 16U/2U, 26U/2U, 30U/2U, 36U/2U, 40U/2U, 46U/2U, 50U/2U, 56U/2U, 60U/2U, 66U/2U, 70U/2U, 76U/2U, 86U/2U, 90U/2U, 96U/2U, 100U/2U};  
  populate_table_P(afrLookUpTable, xAxis, yAxis, values);

  memset(&page2, 0, sizeof(page2));
  page2.incorporateAFR = true;

  memset(&page6, 0, sizeof(page6));
  page6.egoType = EGO_TYPE_NARROW;
  page6.ego_sdelay = 10;

  memset(&current, 0, sizeof(current));
  current.runSecs = page6.ego_sdelay + 2U;
  current.fuelLoad = 60;
  current.RPM = 3100;
  current.O2 = 75U;
}


static void test_corrections_afrtarget_no_compute(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = false;
  page6.egoType = EGO_TYPE_OFF;
  current.afrTarget = 111;

  TEST_ASSERT_EQUAL(current.afrTarget, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget_no_compute_egodelay(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = false;
  current.runSecs = page6.ego_sdelay - 2U;
  current.afrTarget = current.O2/2U;

  TEST_ASSERT_EQUAL(current.O2, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget_incorporteafr(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = true;
  page6.egoType = EGO_TYPE_OFF;

  TEST_ASSERT_EQUAL(77U, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget_ego(void) {
  table3d16RpmLoad afrLookUpTable;
  statuses current;
  config2 page2;
  config6 page6;
  setup_afrtarget(afrLookUpTable, current, page2, page6);

  page2.incorporateAFR = false;
  page6.egoType = EGO_TYPE_NARROW;

  TEST_ASSERT_EQUAL(77U, calculateAfrTarget(afrLookUpTable, current, page2, page6));
}

static void test_corrections_afrtarget(void) {
  RUN_TEST_P(test_corrections_afrtarget_no_compute);
  RUN_TEST_P(test_corrections_afrtarget_no_compute_egodelay);
  RUN_TEST_P(test_corrections_afrtarget_incorporteafr);
  RUN_TEST_P(test_corrections_afrtarget_ego);
}

extern byte correctionIATDensity(void);

#if !defined(_countof)
#define _countof(x) (sizeof(x) / sizeof (x[0]))
#endif
 
extern byte correctionBaro(void);

extern table2D_u8_u8_9 IATDensityCorrectionTable; ///< 9 bin inlet air temperature density correction (2D)
extern table2D_u8_u8_8 baroFuelTable; ///< 8 bin baro correction curve (2D)

static void test_corrections_correctionsFuel_ae_modes(void) {
  setup_TAE();
  populate_2dtable(&injectorVCorrectionTable, (uint8_t)100, (uint8_t)100);
  populate_2dtable(&baroFuelTable, (uint8_t)100, (uint8_t)100);
  populate_2dtable(&IATDensityCorrectionTable, (uint8_t)100, (uint8_t)100);
  populate_2dtable(&flexFuelTable, (uint8_t)100, (uint8_t)100);
  populate_2dtable(&fuelTempTable, (uint8_t)100, (uint8_t)100);

  //Disable the taper
  currentStatus.RPM = 2000;
  configPage2.aeTaperMin = 50; //5000
  configPage2.aeTaperMax = 60; //6000
  configPage2.decelAmount = 33U;

  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value
  currentStatus.coolant = 212;
  currentStatus.runSecs = 255; 
  currentStatus.battery10 = 90;  
  currentStatus.IAT = 100;
  currentStatus.launchingHard = false;
  currentStatus.launchingSoft = false;
  currentStatus.isDFCOActive = false;
  currentStatus.engineIsCranking = false;

  configPage2.dfcoEnabled = 0;
  configPage10.crankingEnrichTaper = 0U; //Disable cranking enrich taper

  configPage4.dfcoRPM = 100;
  configPage4.wueBins[9] = 100;
  configPage2.wueValues[9] = 100; //Use a value other than 100 here to ensure we are using the non-default value
  WUETable.cache.cacheTime = currentStatus.secl - 1;

  configPage4.floodClear = 100;

  configPage6.egoType = 0;
  configPage6.egoAlgorithm = EGO_ALGORITHM_SIMPLE;

  TEST_ASSERT_EQUAL_MESSAGE(100, correctionWUE(), "correctionWUE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionASE(), "correctionASE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionCranking(), "correctionCranking");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFloodClear(), "correctionFloodClear");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionAFRClosedLoop(), "correctionAFRClosedLoop");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionBatVoltage(), "correctionBatVoltage");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionIATDensity(), "correctionIATDensity");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionBaro(), "correctionBaro");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFlex(), "correctionFlex");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFuelTemp(), "correctionFuelTemp");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionLaunch(), "correctionLaunch");
  TEST_ASSERT_FALSE(correctionDFCO());
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionDFCOfuel(), "correctionDFCOfuel");

  // Acceeleration
  configPage2.aeApplyMode = AE_MODE_MULTIPLIER;
  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50; //25% actual value

  reset_AE();
  TEST_ASSERT_EQUAL(232U, correctionsFuel());

  configPage2.aeApplyMode = AE_MODE_ADDER;
  currentStatus.TPSlast = 0;
  currentStatus.TPS = 50;
  reset_AE();
  TEST_ASSERT_EQUAL(100U, correctionsFuel());

  // Deceeleration
  configPage2.aeApplyMode = AE_MODE_MULTIPLIER;
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 45; 
  reset_AE();
  TEST_ASSERT_EQUAL(configPage2.decelAmount, correctionsFuel());
  TEST_ASSERT_LESS_THAN(0U, currentStatus.tpsDOT); 

  configPage2.aeApplyMode = AE_MODE_ADDER;
  currentStatus.TPSlast = 50;
  currentStatus.TPS = 45;
  reset_AE();
  TEST_ASSERT_EQUAL(configPage2.decelAmount, correctionsFuel());
  TEST_ASSERT_LESS_THAN(0U, currentStatus.tpsDOT); 
}

static void test_corrections_correctionsFuel_clip_limit(void) {
  initialiseCorrections();

  populate_2dtable(&injectorVCorrectionTable, (uint8_t)255, (uint8_t)100);
  populate_2dtable(&baroFuelTable, (uint8_t)255, (uint8_t)100);
  populate_2dtable(&IATDensityCorrectionTable, (uint8_t)255, (uint8_t)100);
  populate_2dtable(&flexFuelTable, (uint8_t)255, (uint8_t)100);
  populate_2dtable(&fuelTempTable, (uint8_t)255, (uint8_t)100);

  configPage2.flexEnabled = 1;
  configPage2.dfcoEnabled = 0;
  currentStatus.coolant = 212;
  currentStatus.runSecs = 255; 
  currentStatus.battery10 = 100;  
  currentStatus.IAT = temperatureRemoveOffset(100);
  currentStatus.baro = 100;
  currentStatus.ethanolPct = 100;
  currentStatus.launchingHard = false;
  currentStatus.launchingSoft = false;
  currentStatus.AEamount = 100;

  configPage4.wueBins[9] = 100;
  configPage2.wueValues[9] = 100; //Use a value other than 100 here to ensure we are using the non-default value

  configPage10.crankingEnrichTaper = 0U; //Disable cranking enrich taper

  TEST_ASSERT_EQUAL_MESSAGE(100, correctionWUE(), "correctionWUE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionASE(), "correctionASE");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionCranking(), "correctionCranking");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionAccel(), "correctionAccel");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionFloodClear(), "correctionFloodClear");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionAFRClosedLoop(), "correctionAFRClosedLoop");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionBatVoltage(), "correctionBatVoltage");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionIATDensity(), "correctionIATDensity");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionBaro(), "correctionBaro");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionFlex(), "correctionFlex");
  TEST_ASSERT_EQUAL_MESSAGE(255, correctionFuelTemp(), "correctionFuelTemp");
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionLaunch(), "correctionLaunch");
  TEST_ASSERT_FALSE(correctionDFCO());
  TEST_ASSERT_EQUAL_MESSAGE(100, correctionDFCOfuel(), "correctionDFCOfuel");

  TEST_ASSERT_EQUAL(1500U, correctionsFuel());
}

static void test_corrections_correctionsFuel(void) {
  RUN_TEST_P(test_corrections_correctionsFuel_ae_modes);
  RUN_TEST_P(test_corrections_correctionsFuel_clip_limit);
}

void testCorrections()
{
  SET_UNITY_FILENAME() {
    test_corrections_WUE();
    test_corrections_dfco();
    test_corrections_TAE(); //TPS based accel enrichment corrections
    test_corrections_MAE(); //MAP based accel enrichment corrections
    test_corrections_cranking();
    test_corrections_ASE();
    test_corrections_floodclear();
    test_corrections_bat();
    test_corrections_launch();
    test_corrections_flex();
    test_corrections_afrtarget();
    test_corrections_closedloop();
    test_corrections_correctionsFuel();
  }
}