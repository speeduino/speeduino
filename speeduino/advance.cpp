/** \file advance.cpp
 * @brief Functions for calculating degrees of ignition advance
 * 
 */

#include "globals.h"
#include "advance.h"
#include "corrections.h"

/** Gets the correct advance based on which table and corrections
 * 
 * @return byte The current target advance value in degrees
 */

int8_t getAdvance() {
  int16_t tempAdvance = 0; // Result

  if( shouldWeUseSparkTable2() == true ) //Spark table 2
  {
    currentStatus.advance1 = 0; // Since this isn't valid anymore reset it

    BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use.
    int16_t tempAdvance2 = getAdvance2(); // Advance from table 2

    if(configPage10.spark2Mode == SPARK2_MODE_MULTIPLY)
    {
      if(tempAdvance2 < 0) { tempAdvance2 = 0; } //Negative values not supported
      tempAdvance2 = (getAdvance1() * tempAdvance2) / 100; //Spark 2 table is treated as a % value. Table 1 and 2 are multiplied together and divded by 100
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_ADD)
    {
      tempAdvance2 = getAdvance1() + tempAdvance2;
    }

    if (tempAdvance2 > 127) { tempAdvance2 = 127; } //make sure we don't overflow and accidentally set negative timing, currentStatus.advance can only hold a signed 8 bit value

    tempAdvance = currentStatus.advance2 = correctionsIgn(tempAdvance2); // Apply corrections
  }
  else { //Spark table 1
    currentStatus.advance2 = 0; // Since this isn't valid anymore reset it

    BIT_CLEAR(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Clear the bit indicating that the 2nd spark table is in use.

    tempAdvance = currentStatus.advance1 = correctionsIgn(getAdvance1()); // Apply corrections
  }

  return tempAdvance;
}

/** Lookup the ignition advance from 3D ignition table.
 * The values used to look this up will be RPM and whatever load source the user has configured.
 * 
 * @return byte The current target advance value in degrees
 */
int16_t getAdvance1()
{
  int16_t tempAdvance = 0;
  if (configPage2.ignAlgorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.ignLoad = currentStatus.MAP;
  }
  else if(configPage2.ignAlgorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.ignLoad = currentStatus.TPS * 2;

  }
  else if (configPage2.fuelAlgorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.ignLoad = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }

  tempAdvance = get3DTableValue(&ignitionTable, currentStatus.ignLoad, currentStatus.RPM) - OFFSET_IGNITION; //As above, but for ignition advance

  return tempAdvance;
}

/**
 * @brief Performs a lookup of the second ignition advance table. The values used to look this up will be RPM and whatever load source the user has configured
 * 
 * @return byte The current target advance value in degrees
 */
int16_t getAdvance2()
{
  int16_t tempAdvance = 0;
  if (configPage10.spark2Algorithm == LOAD_SOURCE_MAP) //Check which fuelling algorithm is being used
  {
    //Speed Density
    currentStatus.ignLoad2 = currentStatus.MAP;
  }
  else if(configPage10.spark2Algorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.ignLoad2 = currentStatus.TPS;
  }
  else if (configPage10.spark2Algorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.ignLoad2 = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.ignLoad2 = currentStatus.MAP; }

  tempAdvance = get3DTableValue(&ignitionTable2, currentStatus.ignLoad2, currentStatus.RPM) - OFFSET_IGNITION; //As above, but for ignition advance

  return tempAdvance;
}

/** Checks if we should use spark table 2
 * 
 * @return bool Returns true if the settings and currentStatus says we should use spark table 2
 */

bool shouldWeUseSparkTable2() {
  if (configPage10.spark2Mode <= 0)
  { return false; }

  if (configPage10.spark2Mode == SPARK2_MODE_MULTIPLY || configPage10.spark2Mode == SPARK2_MODE_ADD )
  { return true; }

  if (configPage10.spark2Mode == SPARK2_MODE_CONDITIONAL_SWITCH) {
    if ( (configPage10.spark2SwitchVariable == SPARK2_CONDITION_RPM && currentStatus.RPM > configPage10.spark2SwitchValue       ) ||
         (configPage10.spark2SwitchVariable == SPARK2_CONDITION_MAP && currentStatus.MAP > configPage10.spark2SwitchValue       ) ||
         (configPage10.spark2SwitchVariable == SPARK2_CONDITION_TPS && currentStatus.TPS > configPage10.spark2SwitchValue       ) ||
         (configPage10.spark2SwitchVariable == SPARK2_CONDITION_ETH && currentStatus.ethanolPct > configPage10.spark2SwitchValue) )
    { return true; }
  }

  else if ( configPage10.spark2Mode == SPARK2_MODE_INPUT_SWITCH && digitalRead(pinSpark2Input) == configPage10.spark2InputPolarity)
  { return true; }

  //Default
  return false;
}