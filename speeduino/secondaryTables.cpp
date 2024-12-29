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
static inline uint8_t lookupVE2(const config10 &page10, const table3d16RpmLoad &veLookupTable, const statuses &current)
{
  return get3DTableValue(&veLookupTable, getLoad(page10.fuel2Algorithm, current), (table3d_axis_t)current.RPM); //Perform lookup into fuel map for RPM vs MAP value
}

static inline bool fuelModeCondSwitchRpmActive(const config10 &page10, const statuses &current) {
  return (page10.fuel2SwitchVariable == FUEL2_CONDITION_RPM)
      && (current.RPM > page10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchMapActive(const config10 &page10, const statuses &current) {
  return (page10.fuel2SwitchVariable == FUEL2_CONDITION_MAP)
      && ((uint16_t)(int16_t)current.MAP > page10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchTpsActive(const config10 &page10, const statuses &current) {
  return (page10.fuel2SwitchVariable == FUEL2_CONDITION_TPS)
      && (current.TPS > page10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchEthanolActive(const config10 &page10, const statuses &current) {
  return (page10.fuel2SwitchVariable == FUEL2_CONDITION_ETH)
      && (current.ethanolPct > page10.fuel2SwitchValue);
}

static inline bool fuelModeCondSwitchActive(const config10 &page10, const statuses &current) {
  return (page10.fuel2Mode == FUEL2_MODE_CONDITIONAL_SWITCH)
      && ( fuelModeCondSwitchRpmActive(page10, current)
        || fuelModeCondSwitchMapActive(page10, current) 
        || fuelModeCondSwitchTpsActive(page10, current)
        || fuelModeCondSwitchEthanolActive(page10, current));
}

static inline bool fuelModeInputSwitchActive(const config10 &page10) {
  return (page10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH)
      && (digitalRead(pinFuel2Input) == page10.fuel2InputPolarity);
}

void calculateSecondaryFuel(const config10 &page10, const table3d16RpmLoad &veLookupTable, statuses &current)
{
  //If the secondary fuel table is in use, also get the VE value from there
  if(page10.fuel2Mode == FUEL2_MODE_MULTIPLY)
  {
    current.VE2 = lookupVE2(page10, veLookupTable, current);
    BIT_SET(current.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
    //Fuel 2 table is treated as a % value. Table 1 and 2 are multiplied together and divided by 100
    auto combinedVE = percentage(current.VE2, current.VE1);
    current.VE = (uint8_t)min((uint32_t)UINT8_MAX, combinedVE);
  }
  else if(page10.fuel2Mode == FUEL2_MODE_ADD)
  {
    current.VE2 = lookupVE2(page10, veLookupTable, current);
    BIT_SET(current.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
    //Fuel tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
    uint16_t combinedVE = (uint16_t)current.VE1 + (uint16_t)current.VE2;
    current.VE = (uint8_t)min((uint16_t)UINT8_MAX, combinedVE);
  }
  else if(fuelModeCondSwitchActive(page10, current) || fuelModeInputSwitchActive(page10))
  {
    current.VE2 = lookupVE2(page10, veLookupTable, current);
    BIT_SET(current.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
    current.VE = current.VE2;
  }
  else
  {
    // Unknown mode or mode not activated
    BIT_CLEAR(current.status3, BIT_STATUS3_FUEL2_ACTIVE); //Clear the bit indicating that the 2nd fuel table is in use.
    current.VE2 = 0U;
  }
}

// The bounds of the spark table vary depending on the mode (see the INI file).
// int16_t is wide enough to capture the full range of the table.
static inline int16_t lookupSpark2(const config10 &page10, const table3d16RpmLoad &sparkLookupTable, const statuses &current) {
  return (int16_t)get3DTableValue(&sparkLookupTable, getLoad(page10.spark2Algorithm, current), (table3d_axis_t)current.RPM) - INT16_C(OFFSET_IGNITION);  
}

static inline int8_t constrainAdvance(int16_t advance)
{
  // Clamp to return type range.
  return (int8_t)clamp(advance, (int16_t)INT8_MIN, (int16_t)INT8_MAX);
}

static inline bool sparkModeCondSwitchRpmActive(const config10 &page10, const statuses &current) {
  return (page10.spark2SwitchVariable == SPARK2_CONDITION_RPM)
      && (current.RPM > page10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchMapActive(const config10 &page10, const statuses &current) {
  return (page10.spark2SwitchVariable == SPARK2_CONDITION_MAP)
      && ((uint16_t)(int16_t)current.MAP > page10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchTpsActive(const config10 &page10, const statuses &current) {
  return (page10.spark2SwitchVariable == SPARK2_CONDITION_TPS)
      && (current.TPS > page10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchEthanolActive(const config10 &page10, const statuses &current) {
return (page10.spark2SwitchVariable == SPARK2_CONDITION_ETH)
    && (current.ethanolPct > page10.spark2SwitchValue);
}

static inline bool sparkModeCondSwitchActive(const config10 &page10, const statuses &current) {
  return (page10.spark2Mode == SPARK2_MODE_CONDITIONAL_SWITCH)
      && ( sparkModeCondSwitchRpmActive(page10, current)
        || sparkModeCondSwitchMapActive(page10, current) 
        || sparkModeCondSwitchTpsActive(page10, current)
        || sparkModeCondSwitchEthanolActive(page10, current));
}

static inline bool sparkModeInputSwitchActive(const config10 &page10) {
  return (page10.spark2Mode == SPARK2_MODE_INPUT_SWITCH)
      && (digitalRead(pinSpark2Input) == page10.spark2InputPolarity);
}

static inline bool isFixedTimingOn(const config2 &page2, const statuses &current) {
            // Fixed timing is in effect
    return  (page2.fixAngEnable == 1U)
            // Cranking, so the cranking advance angle is in effect
            || (BIT_CHECK(current.engine, BIT_ENGINE_CRANK));
}

void calculateSecondarySpark(const config2 &page2, const config10 &page10, const table3d16RpmLoad &sparkLookupTable, statuses &current)
{
  BIT_CLEAR(current.status5, BIT_STATUS5_SPARK2_ACTIVE); //Clear the bit indicating that the 2nd spark table is in use. 
  current.advance2 = 0;

  if (!isFixedTimingOn(page2, current))
  {
    if(page10.spark2Mode == SPARK2_MODE_MULTIPLY)
    {
      BIT_SET(current.status5, BIT_STATUS5_SPARK2_ACTIVE);
      uint8_t spark2Percent = (uint8_t)clamp(lookupSpark2(page10, sparkLookupTable, current), (int16_t)0, (int16_t)UINT8_MAX);
      //Spark 2 table is treated as a % value. Table 1 and 2 are multiplied together and divided by 100
      int16_t combinedAdvance = div100((int16_t)spark2Percent * (int16_t)current.advance1);
      //make sure we don't overflow and accidentally set negative timing: current.advance can only hold a signed 8 bit value
      current.advance = constrainAdvance(combinedAdvance);

      // This is informational only, but the value needs corrected into the int8_t range
      current.advance2 = constrainAdvance((int16_t)spark2Percent-(int16_t)INT8_MAX);
    }
    else if(page10.spark2Mode == SPARK2_MODE_ADD)
    {    
      BIT_SET(current.status5, BIT_STATUS5_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
      current.advance2 = constrainAdvance(lookupSpark2(page10, sparkLookupTable, current));
      //Spark tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
      int16_t combinedAdvance = (int16_t)current.advance1 + (int16_t)current.advance2;
      current.advance = constrainAdvance(combinedAdvance);
    }
    else if(sparkModeCondSwitchActive(page10, current) || sparkModeInputSwitchActive(page10))
    {
      BIT_SET(current.status5, BIT_STATUS5_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
#if defined(UNIT_TEST)
      current.advance2 = constrainAdvance(lookupSpark2(page10, sparkLookupTable, current));
#else
      //Perform the corrections calculation on the secondary advance value, only if it uses a switched mode
      current.advance2 = correctionsIgn(constrainAdvance(lookupSpark2(page10, sparkLookupTable, current)));
#endif      
      current.advance = current.advance2;
    }
    else
    {
      // Unknown mode or mode not activated
      // Keep MISRA checker happy.
    }
  }
}
