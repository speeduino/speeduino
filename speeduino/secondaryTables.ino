#include "globals.h"
#include "secondaryTables.h"
#include "corrections.h"

void calculateSecondaryFuel()
{
  //If the secondary fuel table is in use, also get the VE value from there
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Clear the bit indicating that the 2nd fuel table is in use. 
  if(configPage10.fuel2Mode > 0)
  { 
    if(configPage10.fuel2Mode == FUEL2_MODE_MULTIPLY)
    {
      currentStatus.VE2 = getVE2();
      //Fuel 2 table is treated as a % value. Table 1 and 2 are multiplied together and divded by 100
      uint16_t combinedVE = ((uint16_t)currentStatus.VE1 * (uint16_t)currentStatus.VE2) / 100;
      if(combinedVE <= 255) { currentStatus.VE = combinedVE; }
      else { currentStatus.VE = 255; }
    }
    else if(configPage10.fuel2Mode == FUEL2_MODE_ADD)
    {
      currentStatus.VE2 = getVE2();
      //Fuel tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
      uint16_t combinedVE = (uint16_t)currentStatus.VE1 + (uint16_t)currentStatus.VE2;
      if(combinedVE <= 255) { currentStatus.VE = combinedVE; }
      else { currentStatus.VE = 255; }
    }
    else if(configPage10.fuel2Mode == FUEL2_MODE_CONDITIONAL_SWITCH )
    {
      if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_RPM)
      {
        if(currentStatus.RPM > configPage10.fuel2SwitchValue)
        {
          BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
          currentStatus.VE2 = getVE2();
          currentStatus.VE = currentStatus.VE2;
        }
      }
      else if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_MAP)
      {
        if(currentStatus.MAP > configPage10.fuel2SwitchValue)
        {
          BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
          currentStatus.VE2 = getVE2();
          currentStatus.VE = currentStatus.VE2;
        }
      }
      else if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_TPS)
      {
        if(currentStatus.TPS > configPage10.fuel2SwitchValue)
        {
          BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
          currentStatus.VE2 = getVE2();
          currentStatus.VE = currentStatus.VE2;
        }
      }
      else if(configPage10.fuel2SwitchVariable == FUEL2_CONDITION_ETH)
      {
        if(currentStatus.ethanolPct > configPage10.fuel2SwitchValue)
        {
          BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
          currentStatus.VE2 = getVE2();
          currentStatus.VE = currentStatus.VE2;
        }
      }
    }
    else if(configPage10.fuel2Mode == FUEL2_MODE_INPUT_SWITCH)
    {
      if(digitalRead(pinFuel2Input) == configPage10.fuel2InputPolarity)
      {
        BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use. 
        currentStatus.VE2 = getVE2();
        currentStatus.VE = currentStatus.VE2;
      }
    }
  }
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

/**
 * @brief Looks up and returns the VE value from the secondary fuel table
 * 
 * This performs largely the same operations as getVE() however the lookup is of the secondary fuel table and uses the secondary load source
 * @return byte 
 */
byte getVE2()
{
  byte tempVE = 100;
  if( configPage10.fuel2Algorithm == LOAD_SOURCE_MAP)
  {
    //Speed Density
    currentStatus.fuelLoad2 = currentStatus.MAP;
  }
  else if (configPage10.fuel2Algorithm == LOAD_SOURCE_TPS)
  {
    //Alpha-N
    currentStatus.fuelLoad2 = currentStatus.TPS * 2;
  }
  else if (configPage10.fuel2Algorithm == LOAD_SOURCE_IMAPEMAP)
  {
    //IMAP / EMAP
    currentStatus.fuelLoad2 = (currentStatus.MAP * 100) / currentStatus.EMAP;
  }
  else { currentStatus.fuelLoad2 = currentStatus.MAP; } //Fallback position
  tempVE = get3DTableValue(&fuelTable2, currentStatus.fuelLoad2, currentStatus.RPM); //Perform lookup into fuel map for RPM vs MAP value

  return tempVE;
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
    currentStatus.ignLoad2 = currentStatus.TPS * 2;

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