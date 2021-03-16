#include "pages.h"
#include "src/FastCRC/FastCRC.h"
#include "globals.h"


const uint16_t npage_size[NUM_PAGES] = {0,128,288,288,128,288,128,240,384,192,192,288,192,128,288}; /**< This array stores the size (in bytes) of each configuration page */


void setPageValue(byte pageNum, uint16_t valueOffset, byte newValue)
{

  void* pnt_configPage;//This only stores the address of the value that it's pointing to and not the max size
  int tempOffset;

  switch (pageNum)
  {
    case veMapPage:
      if (valueOffset < 256) //New value is part of the fuel map
      {
        fuelTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          fuelTable.axisX[(valueOffset - 256)] = ((int)(newValue) * TABLE_RPM_MULTIPLIER); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
          fuelTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
          //This should never happen. It means there's an invalid offset value coming through
        }
      }
      fuelTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case veSetPage:
      pnt_configPage = &configPage2; //Setup a pointer to the relevant config page
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[veSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case ignMapPage: //Ignition settings page (Page 2)
      if (valueOffset < 256) //New value is part of the ignition map
      {
        ignitionTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          ignitionTable.axisX[(valueOffset - 256)] = (int)(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiple it back by 100 to make it correct
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          ignitionTable.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
      }
      ignitionTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case ignSetPage:
      pnt_configPage = &configPage4;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[ignSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case afrMapPage: //Air/Fuel ratio target settings page
      if (valueOffset < 256) //New value is part of the afr map
      {
        afrTable.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          afrTable.axisX[(valueOffset - 256)] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          afrTable.axisY[tempOffset] = int(newValue) * TABLE_LOAD_MULTIPLIER;

        }
      }
      afrTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case afrSetPage:
      pnt_configPage = &configPage6;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[afrSetPage])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case boostvvtPage: //Boost, VVT and staging maps (all 8x8)
      if (valueOffset < 64) //New value is part of the boost map
      {
        boostTable.values[7 - (valueOffset / 8)][valueOffset % 8] = newValue;
      }
      else if (valueOffset < 72) //New value is on the X (RPM) axis of the boost table
      {
        boostTable.axisX[(valueOffset - 64)] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      }
      else if (valueOffset < 80) //New value is on the Y (TPS) axis of the boost table
      {
        boostTable.axisY[(7 - (valueOffset - 72))] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
      }
      //End of boost table
      else if (valueOffset < 144) //New value is part of the vvt map
      {
        tempOffset = valueOffset - 80;
        vvtTable.values[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
      }
      else if (valueOffset < 152) //New value is on the X (RPM) axis of the vvt table
      {
        tempOffset = valueOffset - 144;
        vvtTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      }
      else if (valueOffset < 160) //New value is on the Y (Load) axis of the vvt table
      {
        tempOffset = valueOffset - 152;
        vvtTable.axisY[(7 - tempOffset)] = int(newValue); //TABLE_LOAD_MULTIPLIER is NOT used for vvt as it is TPS based (0-100)
      }
      //End of vvt table
      else if (valueOffset < 224) //New value is part of the staging map
      {
        tempOffset = valueOffset - 160;
        stagingTable.values[7 - (tempOffset / 8)][tempOffset % 8] = newValue;
      }
      else if (valueOffset < 232) //New value is on the X (RPM) axis of the staging table
      {
        tempOffset = valueOffset - 224;
        stagingTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      }
      else if (valueOffset < 240) //New value is on the Y (Load) axis of the staging table
      {
        tempOffset = valueOffset - 232;
        stagingTable.axisY[(7 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER;
      }
      boostTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      vvtTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      stagingTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case seqFuelPage:
      if (valueOffset < 36) { trim1Table.values[5 - (valueOffset / 6)][valueOffset % 6] = newValue; } //Trim1 values
      else if (valueOffset < 42) { trim1Table.axisX[(valueOffset - 36)] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the trim1 table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 48) { trim1Table.axisY[(5 - (valueOffset - 42))] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (TPS) axis of the boost table
      //Trim table 2
      else if (valueOffset < 84) { tempOffset = valueOffset - 48; trim2Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim2 map
      else if (valueOffset < 90) { tempOffset = valueOffset - 84; trim2Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 96) { tempOffset = valueOffset - 90; trim2Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 3
      else if (valueOffset < 132) { tempOffset = valueOffset - 96; trim3Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim3 map
      else if (valueOffset < 138) { tempOffset = valueOffset - 132; trim3Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 144) { tempOffset = valueOffset - 138; trim3Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 4
      else if (valueOffset < 180) { tempOffset = valueOffset - 144; trim4Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim4 map
      else if (valueOffset < 186) { tempOffset = valueOffset - 180; trim4Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 192) { tempOffset = valueOffset - 186; trim4Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 5
      else if (valueOffset < 228) { tempOffset = valueOffset - 192; trim5Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim5 map
      else if (valueOffset < 234) { tempOffset = valueOffset - 228; trim5Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 240) { tempOffset = valueOffset - 234; trim5Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 6
      else if (valueOffset < 276) { tempOffset = valueOffset - 240; trim6Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim6 map
      else if (valueOffset < 282) { tempOffset = valueOffset - 276; trim6Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 288) { tempOffset = valueOffset - 282; trim6Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 7
      else if (valueOffset < 324) { tempOffset = valueOffset - 288; trim7Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim7 map
      else if (valueOffset < 330) { tempOffset = valueOffset - 324; trim7Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 336) { tempOffset = valueOffset - 330; trim7Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table
      //Trim table 8
      else if (valueOffset < 372) { tempOffset = valueOffset - 336; trim8Table.values[5 - (tempOffset / 6)][tempOffset % 6] = newValue; } //New value is part of the trim8 map
      else if (valueOffset < 378) { tempOffset = valueOffset - 372; trim8Table.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER; } //New value is on the X (RPM) axis of the table. The RPM values sent by TunerStudio are divided by 100, need to multiply it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
      else if (valueOffset < 384) { tempOffset = valueOffset - 378; trim8Table.axisY[(5 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER; } //New value is on the Y (Load) axis of the table

      trim1Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim2Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim3Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim4Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim5Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim6Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim7Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      trim8Table.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case canbusPage:
      pnt_configPage = &configPage9;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[pageNum])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case warmupPage:
      pnt_configPage = &configPage10;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[pageNum])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    case fuelMap2Page:
      if (valueOffset < 256) //New value is part of the fuel map
      {
        fuelTable2.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          fuelTable2.axisX[(valueOffset - 256)] = ((int)(newValue) * TABLE_RPM_MULTIPLIER); //The RPM values sent by megasquirt are divided by 100, need to multiple it back by 100 to make it correct (TABLE_RPM_MULTIPLIER)
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order (Due to us using (0,0) in the top left rather than bottom right
          fuelTable2.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
        else
        {
          //This should never happen. It means there's an invalid offset value coming through
        }
      }
      fuelTable2.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;

    case wmiMapPage:
      if (valueOffset < 64) //New value is part of the wmi map
      {
        wmiTable.values[7 - (valueOffset / 8)][valueOffset % 8] = newValue;
      }
      else if (valueOffset < 72) //New value is on the X (RPM) axis of the wmi table
      {
        wmiTable.axisX[(valueOffset - 64)] = int(newValue) * TABLE_RPM_MULTIPLIER;
      }
      else if (valueOffset < 80) //New value is on the Y (MAP) axis of the wmi table
      {
        wmiTable.axisY[(7 - (valueOffset - 72))] = int(newValue) * TABLE_LOAD_MULTIPLIER;
      }
      //End of wmi table
      else if (valueOffset < 176) //New value is part of the dwell map
      {
        tempOffset = valueOffset - 160;
        dwellTable.values[3 - (tempOffset / 4)][tempOffset % 4] = newValue;
      }
      else if (valueOffset < 180) //New value is on the X (RPM) axis of the dwell table
      {
        tempOffset = valueOffset - 176;
        dwellTable.axisX[tempOffset] = int(newValue) * TABLE_RPM_MULTIPLIER;
      }
      else if (valueOffset < 184) //New value is on the Y (Load) axis of the dwell table
      {
        tempOffset = valueOffset - 180;
        dwellTable.axisY[(3 - tempOffset)] = int(newValue) * TABLE_LOAD_MULTIPLIER;
      }
      //End of dwell table
      wmiTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      dwellTable.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;
      
    case progOutsPage:
      pnt_configPage = &configPage13;
      //For some reason, TunerStudio is sending offsets greater than the maximum page size. I'm not sure if it's their bug or mine, but the fix is to only update the config page if the offset is less than the maximum size
      if (valueOffset < npage_size[pageNum])
      {
        *((byte *)pnt_configPage + (byte)valueOffset) = newValue;
      }
      break;

    default:
      break;
    
    case ignMap2Page: //Ignition settings page (Page 2)
      if (valueOffset < 256) //New value is part of the ignition map
      {
        ignitionTable2.values[15 - (valueOffset / 16)][valueOffset % 16] = newValue;
      }
      else
      {
        //Check whether this is on the X (RPM) or Y (MAP/TPS) axis
        if (valueOffset < 272)
        {
          //X Axis
          ignitionTable2.axisX[(valueOffset - 256)] = (int)(newValue) * TABLE_RPM_MULTIPLIER; //The RPM values sent by TunerStudio are divided by 100, need to multiple it back by 100 to make it correct
        }
        else if(valueOffset < 288)
        {
          //Y Axis
          tempOffset = 15 - (valueOffset - 272); //Need to do a translation to flip the order
          ignitionTable2.axisY[tempOffset] = (int)(newValue) * TABLE_LOAD_MULTIPLIER;
        }
      }
      ignitionTable2.cacheIsValid = false; //Invalid the tables cache to ensure a lookup of new values
      break;
  }
  //if(Serial.available() > 16) { command(); }
}



/**
 * @brief Retrieves a single value from a memory page, with data aligned as per the ini file
 * 
 * @param page The page number to retrieve data from
 * @param valueAddress The address in the page that should be returned. This is as per the page definition in the ini
 * @return byte The requested value
 */
byte getPageValue(byte page, uint16_t valueAddress)
{
  void* pnt_configPage = &configPage2; //Default value is for safety only. Will be changed below if needed.
  uint16_t tempAddress;
  byte returnValue = 0;

  switch (page)
  {
    case veMapPage:
        if( valueAddress < 256) { returnValue = fuelTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(fuelTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(fuelTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case veSetPage:
        pnt_configPage = &configPage2; //Create a pointer to Page 1 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case ignMapPage:
        if( valueAddress < 256) { returnValue = ignitionTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(ignitionTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(ignitionTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case ignSetPage:
        pnt_configPage = &configPage4; //Create a pointer to Page 2 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case afrMapPage:
        if( valueAddress < 256) { returnValue = afrTable.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(afrTable.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(afrTable.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;

    case afrSetPage:
        pnt_configPage = &configPage6; //Create a pointer to Page 3 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case boostvvtPage:

        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the MS expected format
          if(valueAddress < 80)
          {
            //Boost table
            if(valueAddress < 64) { returnValue = boostTable.values[7 - (valueAddress / 8)][valueAddress % 8]; }
            else if(valueAddress < 72) { returnValue = byte(boostTable.axisX[(valueAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(valueAddress < 80) { returnValue = byte(boostTable.axisY[7 - (valueAddress - 72)]); }
          }
          else if(valueAddress < 160)
          {
            tempAddress = valueAddress - 80;
            //VVT table
            if(tempAddress < 64) { returnValue = vvtTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(vvtTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(vvtTable.axisY[7 - (tempAddress - 72)]); }
          }
          else
          {
            tempAddress = valueAddress - 160;
            //Staging table
            if(tempAddress < 64) { returnValue = stagingTable.values[7 - (tempAddress / 8)][tempAddress % 8]; }
            else if(tempAddress < 72) { returnValue = byte(stagingTable.axisX[(tempAddress - 64)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 80) { returnValue = byte(stagingTable.axisY[7 - (tempAddress - 72)] / TABLE_LOAD_MULTIPLIER); }
          }
        }
        break;

    case seqFuelPage:

        {
          //Need to perform a translation of the values[MAP/TPS][RPM] into the TS expected format
          if(valueAddress < 48)
          {
            //trim1 table
            if(valueAddress < 36) { returnValue = trim1Table.values[5 - (valueAddress / 6)][valueAddress % 6]; }
            else if(valueAddress < 42) { returnValue = byte(trim1Table.axisX[(valueAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(valueAddress < 48) { returnValue = byte(trim1Table.axisY[5 - (valueAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 96)
          {
            tempAddress = valueAddress - 48;
            //trim2 table
            if(tempAddress < 36) { returnValue = trim2Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim2Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim2Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 144)
          {
            tempAddress = valueAddress - 96;
            //trim3 table
            if(tempAddress < 36) { returnValue = trim3Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim3Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim3Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 192)
          {
            tempAddress = valueAddress - 144;
            //trim4 table
            if(tempAddress < 36) { returnValue = trim4Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim4Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim4Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 240)
          {
            tempAddress = valueAddress - 192;
            //trim5 table
            if(tempAddress < 36) { returnValue = trim5Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim5Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim5Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 288)
          {
            tempAddress = valueAddress - 240;
            //trim6 table
            if(tempAddress < 36) { returnValue = trim6Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim6Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim6Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 336)
          {
            tempAddress = valueAddress - 288;
            //trim7 table
            if(tempAddress < 36) { returnValue = trim7Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim7Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim7Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
          else if(valueAddress < 385)
          {
            tempAddress = valueAddress - 336;
            //trim8 table
            if(tempAddress < 36) { returnValue = trim8Table.values[5 - (tempAddress / 6)][tempAddress % 6]; }
            else if(tempAddress < 42) { returnValue = byte(trim8Table.axisX[(tempAddress - 36)] / TABLE_RPM_MULTIPLIER); }
            else if(tempAddress < 48) { returnValue = byte(trim8Table.axisY[5 - (tempAddress - 42)] / TABLE_LOAD_MULTIPLIER); }
          }
        }
        break;

    case canbusPage:
        pnt_configPage = &configPage9; //Create a pointer to Page 10 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case warmupPage:
        pnt_configPage = &configPage10; //Create a pointer to Page 11 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case fuelMap2Page:
        if( valueAddress < 256) { returnValue = fuelTable2.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(fuelTable2.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(fuelTable2.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;
        
    case wmiMapPage:
        if(valueAddress < 80)
        {
          if(valueAddress < 64) { returnValue = wmiTable.values[7 - (valueAddress / 8)][valueAddress % 8]; }
          else if(valueAddress < 72) { returnValue = byte(wmiTable.axisX[(valueAddress - 64)] / TABLE_RPM_MULTIPLIER); }
          else if(valueAddress < 80) { returnValue = byte(wmiTable.axisY[7 - (valueAddress - 72)] / TABLE_LOAD_MULTIPLIER); }
        }
        else if(valueAddress < 184)
        {
          tempAddress = valueAddress - 160;
          //Dwell table
          if(tempAddress < 16) { returnValue = dwellTable.values[3 - (tempAddress / 4)][tempAddress % 4]; }
          else if(tempAddress < 20) { returnValue = byte(dwellTable.axisX[(tempAddress - 16)] / TABLE_RPM_MULTIPLIER); }
          else if(tempAddress < 24) { returnValue = byte(dwellTable.axisY[3 - (tempAddress - 20)] / TABLE_LOAD_MULTIPLIER); }
        }
        break;

    case progOutsPage:
        pnt_configPage = &configPage13; //Create a pointer to Page 13 in memory
        returnValue = *((byte *)pnt_configPage + valueAddress);
        break;

    case ignMap2Page:
        if( valueAddress < 256) { returnValue = ignitionTable2.values[15 - (valueAddress / 16)][valueAddress % 16]; } //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). Columns are unchanged. Every 16 loops, manually call loop() to avoid potential misses
        else if(valueAddress < 272) { returnValue =  byte(ignitionTable2.axisX[(valueAddress - 256)] / TABLE_RPM_MULTIPLIER); }  //RPM Bins for VE table (Need to be dvidied by 100)
        else if (valueAddress < 288) { returnValue = byte(ignitionTable2.axisY[15 - (valueAddress - 272)] / TABLE_LOAD_MULTIPLIER); } //MAP or TPS bins for VE table
        break;
      
    default:
    #ifndef SMALL_FLASH_MODE
        Serial.println(F("\nPage has not been implemented yet"));
    #endif
        //Just set default Values to avoid warnings
        pnt_configPage = &configPage10;
        break;
  }
  return returnValue;
}


namespace {
    
    FastCRC32 CRC32;

}

/*
Calculates and returns the CRC32 value of a given page of memory
*/
uint32_t calculateCRC32(byte pageNo)
{
  uint32_t CRC32_val;
  byte raw_value;
  void* pnt_configPage;

  //This sucks (again) for all the 3D map pages that have to have a translation performed
  switch(pageNo)
  {
    case veMapPage:
      //Confirmed working
      raw_value = getPageValue(veMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[veMapPage]; x++)
      //for(uint16_t x=1; x< 288; x++)
      {
        raw_value = getPageValue(veMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case veSetPage:
      //Confirmed working
      pnt_configPage = &configPage2; //Create a pointer to Page 1 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage2) );
      break;

    case ignMapPage:
      //Confirmed working
      raw_value = getPageValue(ignMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMapPage]; x++)
      {
        raw_value = getPageValue(ignMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case ignSetPage:
      //Confirmed working
      pnt_configPage = &configPage4; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage4) );
      break;

    case afrMapPage:
      //Confirmed working
      raw_value = getPageValue(afrMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[afrMapPage]; x++)
      {
        raw_value = getPageValue(afrMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case afrSetPage:
      //Confirmed working
      pnt_configPage = &configPage6; //Create a pointer to Page 4 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage6) );
      break;

    case boostvvtPage:
      //Confirmed working
      raw_value = getPageValue(boostvvtPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[boostvvtPage]; x++)
      {
        raw_value = getPageValue(boostvvtPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case seqFuelPage:
      //Confirmed working
      raw_value = getPageValue(seqFuelPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[seqFuelPage]; x++)
      {
        raw_value = getPageValue(seqFuelPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case canbusPage:
      //Confirmed working
      pnt_configPage = &configPage9; //Create a pointer to Page 9 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage9) );
      break;

    case warmupPage:
      //Confirmed working
      pnt_configPage = &configPage10; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage10) );
      break;

    case fuelMap2Page:
      //Confirmed working
      raw_value = getPageValue(fuelMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[fuelMap2Page]; x++)
      //for(uint16_t x=1; x< 288; x++)
      {
        raw_value = getPageValue(fuelMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    case wmiMapPage:
      //Confirmed working
      raw_value = getPageValue(wmiMapPage, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[wmiMapPage]; x++)
      {
        raw_value = getPageValue(wmiMapPage, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;
      
    case progOutsPage:
      //Confirmed working
      pnt_configPage = &configPage13; //Create a pointer to Page 10 in memory
      CRC32_val = CRC32.crc32((byte *)pnt_configPage, sizeof(configPage13) );
      break;
    
    case ignMap2Page:
      //Confirmed working
      raw_value = getPageValue(ignMap2Page, 0);
      CRC32_val = CRC32.crc32(&raw_value, 1, false);
      for(uint16_t x=1; x< npage_size[ignMap2Page]; x++)
      {
        raw_value = getPageValue(ignMap2Page, x);
        CRC32_val = CRC32.crc32_upd(&raw_value, 1, false);
      }
      //Do a manual reflection of the CRC32 value
      CRC32_val = ~CRC32_val;
      break;

    default:
      CRC32_val = 0;
      break;
  }
  
  return CRC32_val;
}
