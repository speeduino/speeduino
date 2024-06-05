#include "globals.h"
#include "secondaryTables.h"
#include "corrections.h"
#include "load_source.h"
#include "maths.h"
#include "unit_testing.h"

/**
 * @brief Looks up and returns the VE value from the secondary fuel table
 * 
 * This performs largely the same operations as getVE() however the lookup is of the secondary fuel table and uses the secondary load source
 * @return byte 
 */
static inline uint8_t getVE2(void)
{
  currentStatus.fuelLoad2 = getLoad(configPage10.fuel2Algorithm, currentStatus);
  return get3DTableValue(&fuelTable2, currentStatus.fuelLoad2, (table3d_axis_t)currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value
}

static inline bool fuelModeCondSwitchRpmActive(void) {
  return (configPage10.fuel2SwitchVariable == FUEL2_CONDITION_RPM)
      && (currentStatus.RPM > configPage10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchMapActive(void) {
  return (configPage10.fuel2SwitchVariable == FUEL2_CONDITION_MAP)
      && ((uint16_t)(int16_t)currentStatus.MAP > configPage10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchTpsActive(void) {
  return (configPage10.fuel2SwitchVariable == FUEL2_CONDITION_TPS)
      && (currentStatus.TPS > configPage10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchEthanolActive(void) {
  return (configPage10.fuel2SwitchVariable == FUEL2_CONDITION_ETH)
      && (currentStatus.ethanolPct > configPage10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchActive(void) {
  return (configPage10.fuel2Mode == FUEL2_MODE_CONDITIONAL_SWITCH)
      && ( fuelModeCondSwitchRpmActive()
        || fuelModeCondSwitchMapActive() 
        || fuelModeCondSwitchTpsActive()
        || fuelModeCondSwitchEthanolActive());
}

static inline bool fuelModeInputSwitchActive(void) {
  return (configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH)
      && (digitalRead(pinFuel2Input) == configPage10.fuel2InputPolarity);
}

void calculateSecondaryFuel(void)
{
  //If the secondary fuel table is in use, also get the VE value from there
  if(configPage10.fuel2Mode == FUEL2_MODE_MULTIPLY)
  {
    currentStatus.VE2 = getVE2();
    BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
    //Fuel 2 table is treated as a % value. Table 1 and 2 are multiplied together and divided by 100
    auto combinedVE = percentage(currentStatus.VE2, currentStatus.VE1);
    currentStatus.VE = (uint8_t)min((uint32_t)UINT8_MAX, combinedVE);
  }
  else if(configPage10.fuel2Mode == FUEL2_MODE_ADD)
  {
    currentStatus.VE2 = getVE2();
    BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
    //Fuel tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
    uint16_t combinedVE = (uint16_t)currentStatus.VE1 + (uint16_t)currentStatus.VE2;
    currentStatus.VE = (uint8_t)min((uint16_t)UINT8_MAX, combinedVE);
  }
  else if(fuelModeCondSwitchActive() || fuelModeInputSwitchActive())
  {
    currentStatus.VE2 = getVE2();
    BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
    currentStatus.VE = currentStatus.VE2;
  }
  else
  {
    // Unknown mode or mode not activated
    BIT_CLEAR(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Clear the bit indicating that the 2nd fuel table is in use.
    currentStatus.VE2 = 0U;
    currentStatus.fuelLoad2 = 0U;
  }
}

// The bounds of the spark table vary depending on the mode (see the INI file).
// int16_t is wide enough to capture the full range of the table.
static inline int16_t lookupSpark2(void) {
  currentStatus.ignLoad2 = getLoad(configPage10.spark2Algorithm, currentStatus);
  return (int16_t)get3DTableValue(&ignitionTable2, currentStatus.ignLoad2, (table3d_axis_t)currentStatus.RPM) - INT16_C(OFFSET_IGNITION);  
}

static inline int8_t constrainAdvance(int16_t advance)
{
  // Clamp to return type range.
  return (int8_t)clamp(advance, (int16_t)INT8_MIN, (int16_t)INT8_MAX);
}

static inline bool sparkModeCondSwitchRpmActive(void) {
  return (configPage10.spark2SwitchVariable == SPARK2_CONDITION_RPM)
      && (currentStatus.RPM > configPage10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchMapActive(void) {
  return (configPage10.spark2SwitchVariable == SPARK2_CONDITION_MAP)
      && ((uint16_t)(int16_t)currentStatus.MAP > configPage10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchTpsActive(void) {
  return (configPage10.spark2SwitchVariable == SPARK2_CONDITION_TPS)
      && (currentStatus.TPS > configPage10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchEthanolActive(void) {
return (configPage10.spark2SwitchVariable == SPARK2_CONDITION_ETH)
    && (currentStatus.ethanolPct > configPage10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchActive(void) {
  return (configPage10.spark2Mode == SPARK2_MODE_CONDITIONAL_SWITCH)
      && ( sparkModeCondSwitchRpmActive()
        || sparkModeCondSwitchMapActive() 
        || sparkModeCondSwitchTpsActive()
        || sparkModeCondSwitchEthanolActive());
}

static inline bool sparkModeInputSwitchActive(void) {
  return (configPage10.spark2Mode == SPARK2_MODE_INPUT_SWITCH)
      && (digitalRead(pinSpark2Input) == configPage10.spark2InputPolarity);
}

TESTABLE_INLINE_STATIC bool isFixedTimingOn(void) {
            // Fixed timing is in effect
    return  (configPage2.fixAngEnable == 1U)
            // Cranking, so the cranking advance angle is in effect
            || (BIT_CHECK(currentStatus.engine, BIT_ENGINE_CRANK));
}

void calculateSecondarySpark(void)
{
  BIT_CLEAR(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE); //Clear the bit indicating that the 2nd spark table is in use. 
  currentStatus.ignLoad2 = 0;
  currentStatus.advance2 = 0;

  if (!isFixedTimingOn())
  {
    if(configPage10.spark2Mode == SPARK2_MODE_MULTIPLY)
    {
      BIT_SET(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE);
      uint8_t spark2Percent = (uint8_t)clamp(lookupSpark2(), (int16_t)0, (int16_t)UINT8_MAX);
      //Spark 2 table is treated as a % value. Table 1 and 2 are multiplied together and divided by 100
      int16_t combinedAdvance = div100((int16_t)spark2Percent * (int16_t)currentStatus.advance1);
      //make sure we don't overflow and accidentally set negative timing: currentStatus.advance can only hold a signed 8 bit value
      currentStatus.advance = constrainAdvance(combinedAdvance);

      // This is informational only, but the value needs corrected into the int8_t range
      currentStatus.advance2 = constrainAdvance((int16_t)spark2Percent-(int16_t)INT8_MAX);
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_ADD)
    {
      BIT_SET(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
      currentStatus.advance2 = constrainAdvance(lookupSpark2());
      //Spark tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
      int16_t combinedAdvance = (int16_t)currentStatus.advance1 + (int16_t)currentStatus.advance2;
      currentStatus.advance = constrainAdvance(combinedAdvance);
    }
    else if(sparkModeCondSwitchActive() || sparkModeInputSwitchActive())
    {
      BIT_SET(currentStatus.status5, BIT_STATUS5_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
#if defined(UNIT_TEST)
      currentStatus.advance2 = constrainAdvance(lookupSpark2());
#else
      //Perform the corrections calculation on the secondary advance value, only if it uses a switched mode
      currentStatus.advance2 = correctionsIgn(constrainAdvance(lookupSpark2()));
#endif      
      currentStatus.advance = currentStatus.advance2;
    }
    else
    {
      // Unknown mode or mode not activated
      // Keep MISRA checker happy.
    }
  }
}
