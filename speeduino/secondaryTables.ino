#include "globals.h"
#include "secondaryTables.h"
#include "corrections.h"
#include "speeduino.h"
#include "table2d.h"

void calculateSecondaryFuel(void)
{
  //If the secondary fuel table is in use, also get the VE value from there
  BIT_CLEAR(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Clear the bit indicating that the 2nd fuel table is in use. 
  if(configPage10.fuel2Mode > 0)
  { 
    if(configPage10.fuel2Mode == FUEL2_MODE_MULTIPLY)
    {
      currentStatus.VE2 = getVE2();
      //Fuel 2 table is treated as a % value. Table 1 and 2 are multiplied together and divided by 100
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
    else if(configPage10.fuel2Mode == FUEL2_MODE_FLEX) {
      if(configPage2.flexEnabled > 0 && currentStatus.ethanolPct > 0)
      {
        BIT_SET(currentStatus.status3, BIT_STATUS3_FUEL2_ACTIVE); //Set the bit indicating that the 2nd fuel table is in use.
        int t2FuelBias = table2D_getValue(&flexFuelTable, currentStatus.ethanolPct);
        currentStatus.VE2 = getVE2();
        currentStatus.VE = biasedAverage(t2FuelBias, currentStatus.VE1, currentStatus.VE2); //calculate biased average between VE1 and VE2 based on specified bias at current ETH%
        currentStatus.flexCorrection = currentStatus.VE - currentStatus.VE1;
      }
    }
  }
}


void calculateSecondarySpark(void)
{
  //Same as above but for the secondary ignition table
  BIT_CLEAR(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Clear the bit indicating that the 2nd spark table is in use. 
  if(configPage10.spark2Mode > 0)
  { 
    if(configPage10.spark2Mode == SPARK2_MODE_MULTIPLY)
    {
      BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE);
      currentStatus.advance2 = getAdvance2();
      //make sure we don't have a negative value in the multiplier table (sharing a signed 8 bit table)
      if(currentStatus.advance2 < 0) { currentStatus.advance2 = 0; }
      //Spark 2 table is treated as a % value. Table 1 and 2 are multiplied together and divided by 100
      int16_t combinedAdvance = ((int16_t)currentStatus.advance1 * (int16_t)currentStatus.advance2) / 100;
      //make sure we don't overflow and accidentally set negative timing, currentStatus.advance can only hold a signed 8 bit value
      if(combinedAdvance <= 127) { currentStatus.advance = combinedAdvance; }
      else { currentStatus.advance = 127; }
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_ADD)
    {
      BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
      currentStatus.advance2 = getAdvance2();
      //Spark tables are added together, but a check is made to make sure this won't overflow the 8-bit VE value
      int16_t combinedAdvance = (int16_t)currentStatus.advance1 + (int16_t)currentStatus.advance2;
      //make sure we don't overflow and accidentally set negative timing, currentStatus.advance can only hold a signed 8 bit value
      if(combinedAdvance <= 127) { currentStatus.advance = combinedAdvance; }
      else { currentStatus.advance = 127; }
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_CONDITIONAL_SWITCH )
    {
      if(configPage10.spark2SwitchVariable == SPARK2_CONDITION_RPM)
      {
        if(currentStatus.RPM > configPage10.spark2SwitchValue)
        {
          BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
          currentStatus.advance2 = getAdvance2();
          currentStatus.advance = currentStatus.advance2;
        }
      }
      else if(configPage10.spark2SwitchVariable == SPARK2_CONDITION_MAP)
      {
        if(currentStatus.MAP > configPage10.spark2SwitchValue)
        {
          BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
          currentStatus.advance2 = getAdvance2();
          currentStatus.advance = currentStatus.advance2;
        }
      }
      else if(configPage10.spark2SwitchVariable == SPARK2_CONDITION_TPS)
      {
        if(currentStatus.TPS > configPage10.spark2SwitchValue)
        {
          BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
          currentStatus.advance2 = getAdvance2();
          currentStatus.advance = currentStatus.advance2;
        }
      }
      else if(configPage10.spark2SwitchVariable == SPARK2_CONDITION_ETH)
      {
        if(currentStatus.ethanolPct > configPage10.spark2SwitchValue)
        {
          BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
          currentStatus.advance2 = getAdvance2();
          currentStatus.advance = currentStatus.advance2;
        }
      }
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_INPUT_SWITCH)
    {
      if(digitalRead(pinSpark2Input) == configPage10.spark2InputPolarity)
      {
        BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use. 
        currentStatus.advance2 = getAdvance2();
        currentStatus.advance = currentStatus.advance2;
      }
    }
    else if(configPage10.spark2Mode == SPARK2_MODE_FLEX) {
      if(configPage2.flexEnabled > 0 && currentStatus.ethanolPct > 0)
      {
        BIT_SET(currentStatus.spark2, BIT_SPARK2_SPARK2_ACTIVE); //Set the bit indicating that the 2nd spark table is in use.
        int t2SparkBias = table2D_getValue(&flexAdvTable, currentStatus.ethanolPct);
        currentStatus.advance2 = getAdvance2();
        currentStatus.advance = biasedAverage(t2SparkBias, currentStatus.advance1, currentStatus.advance2); //calculate biased average between Adv1 and Adv2 based on specified bias at current ETH%
        currentStatus.flexIgnCorrection = currentStatus.advance - currentStatus.advance1;
      }
    }
    //Apply the fixed timing correction manually. This has to be done again here if any of the above conditions are met to prevent any of the seconadary calculations applying instead of fixec timing
    currentStatus.advance = correctionFixedTiming(currentStatus.advance);
    currentStatus.advance = correctionCrankingFixedTiming(currentStatus.advance); //This overrides the regular fixed timing, must come last
  }
}

/**
 * @brief Looks up and returns the VE value from the secondary fuel table
 * 
 * This performs largely the same operations as getVE() however the lookup is of the secondary fuel table and uses the secondary load source
 * @return byte 
 */
byte getVE2(void)
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
byte getAdvance2(void)
{
  byte tempAdvance = 0;
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
  tempAdvance = correctionsIgn(tempAdvance);

  return tempAdvance;
}

/**
 * @brief Performs a calculation of the biased average between two table values based on the specified bias at the current ETH% 
 * @param val2bias the bias or weight toward val2, usually looked up from a table
 * @param val1 first value, usually looked up from a table; ex) VE1, Adv1
 * @param val2 second value, usually looked up from a table; ex) VE2, Adv2
 * @return byte the current VE or the target advance value in degrees
*/
byte biasedAverage(int val2Bias, byte val1, byte val2) 
{
  //calculation to return (1 - t2Bias/100) * val1 + t2Bias/100 * val2;
  if (val2Bias == 0) 
  {
    return val1;
  }
  
  else if (val2Bias == 100)
  {
    return val2;
  }

	uint16_t result = ((100 - val2Bias) * val1 + val2Bias * val2 + 50)/100; //adding 50, dividing by 100, then truncating to int works the same as rounding

	if (result > 255)
		result = 255;

	return result;
}

/**
 * @brief Performs a calculation of the biased average between two table values based on the specified bias at the current ETH% 
 * @param val2bias the bias or weight toward val2, usually looked up from a table
 * @param val1 first value, usually looked up from a table; ex) VE1, Adv1
 * @param val2 second value, usually looked up from a table; ex) VE2, Adv2
 * @return uint16_t the interpolated average result
*/
uint16_t biasedAverage_uint16(int val2Bias, uint16_t val1, uint16_t val2) 
{
  //calculation to return (1 - t2Bias/100) * val1 + t2Bias/100 * val2;
  if (val2Bias == 0) 
  {
    return val1;
  }
  
  else if (val2Bias == 100)
  {
    return val2;
  }

	uint16_t result = ((100 - val2Bias) * val1 + val2Bias * val2 + 50)/100; //adding 50, dividing by 100, then truncating to int works the same as rounding

	return result;
}

uint16_t biasedAverage_uint16(int val2Bias, uint16_t val1, uint16_t val2) 
{
  //calculation to return (1 - t2Bias/100) * val1 + t2Bias/100 * val2;
  if (val2Bias == 0) 
  {
    return val1;
  }

  else if (val2Bias == 100)
  {
    return val2;
  }

	uint16_t result = ((100 - val2Bias) * val1 + val2Bias * val2 + 50)/100; //adding 50, dividing by 100, then truncating to int works the same as rounding

	return result;
}