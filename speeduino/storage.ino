/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/


#include "globals.h"
#include "table.h"
#include "comms.h"
#include EEPROM_LIB_H //This is defined in the board .h files
#include "storage.h"

void writeAllConfig()
{
  writeConfig(veSetPage);
  if (eepromWritesPending == false) { writeConfig(veMapPage); }
  if (eepromWritesPending == false) { writeConfig(ignMapPage); }
  if (eepromWritesPending == false) { writeConfig(ignSetPage); }
  if (eepromWritesPending == false) { writeConfig(afrMapPage); }
  if (eepromWritesPending == false) { writeConfig(afrSetPage); }
  if (eepromWritesPending == false) { writeConfig(boostvvtPage); }
  if (eepromWritesPending == false) { writeConfig(seqFuelPage); }
  if (eepromWritesPending == false) { writeConfig(canbusPage); }
  if (eepromWritesPending == false) { writeConfig(warmupPage); }
  if (eepromWritesPending == false) { writeConfig(fuelMap2Page); }
  if (eepromWritesPending == false) { writeConfig(wmiMapPage); }
  if (eepromWritesPending == false) { writeConfig(progOutsPage); }
  if (eepromWritesPending == false) { writeConfig(ignMap2Page); }
}


/*
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h
*/
void writeConfig(byte tableNum)
{
  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */

  int offset;
  int i, z, y;
  int writeCounter = 0;
  byte newVal; //Used for tempoerarily storing the new intended value
  //Create a pointer to the config page
  byte* pnt_configPage;

  switch(tableNum)
  {
    case veMapPage:
      /*---------------------------------------------------
      | Fuel table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      if(EEPROM.read(EEPROM_CONFIG1_XSIZE) != fuelTable.xSize) { EEPROM.write(EEPROM_CONFIG1_XSIZE, fuelTable.xSize); writeCounter++; } //Write the VE Tables RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG1_YSIZE) != fuelTable.ySize) { EEPROM.write(EEPROM_CONFIG1_YSIZE, fuelTable.ySize); writeCounter++; } //Write the VE Tables MAP/TPS dimension size
      for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG1_MAP;
        if( EEPROM.read(x) != (fuelTable.values[15-(offset/16)][offset%16]) ) { EEPROM.write(x, fuelTable.values[15-(offset/16)][offset%16]); writeCounter++; }  //Write the 16x16 map
      }

      //RPM bins
      for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG1_XBINS;
        if( EEPROM.read(x) != (byte(fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(x, byte(fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
      }
      //TPS/MAP bins
      for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG1_YBINS;
        if( EEPROM.read(x) != (byte(fuelTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)) ) { EEPROM.write(x, byte(fuelTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //Table load is divided by 2 (Allows for MAP up to 511)
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }
      break;
      //That concludes the writing of the VE table

    case veSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
      for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case ignMapPage:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      if(EEPROM.read(EEPROM_CONFIG3_XSIZE) != ignitionTable.xSize) { EEPROM.write(EEPROM_CONFIG3_XSIZE,ignitionTable.xSize); writeCounter++; } //Write the ignition Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG3_YSIZE) != ignitionTable.ySize) { EEPROM.write(EEPROM_CONFIG3_YSIZE,ignitionTable.ySize); writeCounter++; } //Write the ignition Table MAP/TPS dimension size

      for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG3_MAP;
        newVal = ignitionTable.values[15-(offset/16)][offset%16];
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; }  //Write the 16x16 map with translation
      }
      //RPM bins
      for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG3_XBINS;
        newVal = ignitionTable.axisX[offset]/TABLE_RPM_MULTIPLIER;
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
      }
      //TPS/MAP bins
      for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG3_YBINS;
        newVal = ignitionTable.axisY[offset]/TABLE_LOAD_MULTIPLIER;
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; } //Table load is divided by 2 (Allows for MAP up to 511)
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case ignSetPage:
      /*---------------------------------------------------
      | Config page 2 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      pnt_configPage = (byte *)&configPage4; //Create a pointer to Page 2 in memory
      for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case afrMapPage:
      /*---------------------------------------------------
      | AFR table (See storage.h for data layout) - Page 5
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      if(EEPROM.read(EEPROM_CONFIG5_XSIZE) != afrTable.xSize) { EEPROM.write(EEPROM_CONFIG5_XSIZE,afrTable.xSize); writeCounter++; } //Write the ignition Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG5_YSIZE) != afrTable.ySize) { EEPROM.write(EEPROM_CONFIG5_YSIZE,afrTable.ySize); writeCounter++; } //Write the ignition Table MAP/TPS dimension size

      for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG5_MAP;
        if(EEPROM.read(x) != (afrTable.values[15-(offset/16)][offset%16]) ) { EEPROM.write(x, afrTable.values[15-(offset/16)][offset%16]); writeCounter++; }  //Write the 16x16 map
      }
      //RPM bins
      for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG5_XBINS;
        if(EEPROM.read(x) != byte(afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
      }
      //TPS/MAP bins
      for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG5_YBINS;
        if(EEPROM.read(x) != byte(afrTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)) { EEPROM.write(x, byte(afrTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //Table load is divided by 2 (Allows for MAP up to 511)
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case afrSetPage:
      /*---------------------------------------------------
      | Config page 3 (See storage.h for data layout)
      | 64 byte long config table
      -----------------------------------------------------*/
      pnt_configPage = (byte *)&configPage6; //Create a pointer to Page 3 in memory
      for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case boostvvtPage:
      /*---------------------------------------------------
      | Boost and vvt tables (See storage.h for data layout) - Page 8
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      if(EEPROM.read(EEPROM_CONFIG7_XSIZE1) != boostTable.xSize) { EEPROM.write(EEPROM_CONFIG7_XSIZE1,boostTable.xSize); writeCounter++; } //Write the boost Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG7_YSIZE1) != boostTable.ySize) { EEPROM.write(EEPROM_CONFIG7_YSIZE1,boostTable.ySize); writeCounter++; } //Write the boost Table MAP/TPS dimension size
      if(EEPROM.read(EEPROM_CONFIG7_XSIZE2) != vvtTable.xSize) { EEPROM.write(EEPROM_CONFIG7_XSIZE2,vvtTable.xSize); writeCounter++; } //Write the vvt Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG7_YSIZE2) != vvtTable.ySize) { EEPROM.write(EEPROM_CONFIG7_YSIZE2,vvtTable.ySize); writeCounter++; } //Write the vvt Table MAP/TPS dimension size
      if(EEPROM.read(EEPROM_CONFIG7_XSIZE3) != stagingTable.xSize) { EEPROM.write(EEPROM_CONFIG7_XSIZE3,stagingTable.xSize); writeCounter++; } //Write the staging Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG7_YSIZE3) != stagingTable.ySize) { EEPROM.write(EEPROM_CONFIG7_YSIZE3,stagingTable.ySize); writeCounter++; } //Write the staging Table MAP/TPS dimension size

      y = EEPROM_CONFIG7_MAP2; //We do the 3 maps together in the same loop
      z = EEPROM_CONFIG7_MAP3;
      for(int x=EEPROM_CONFIG7_MAP1; x<EEPROM_CONFIG7_XBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG7_MAP1;
        if(EEPROM.read(x) != (boostTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(x, boostTable.values[7-(offset/8)][offset%8]); writeCounter++; }  //Write the 8x8 map
        offset = y - EEPROM_CONFIG7_MAP2;
        if(EEPROM.read(y) != (vvtTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(y, vvtTable.values[7-(offset/8)][offset%8]); writeCounter++; }  //Write the 8x8 map
        offset = z - EEPROM_CONFIG7_MAP3;
        if(EEPROM.read(z) != (stagingTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(z, stagingTable.values[7-(offset/8)][offset%8]); writeCounter++; }  //Write the 8x8 map
        y++;
        z++;
      }
      //RPM bins
      y = EEPROM_CONFIG7_XBINS2;
      z = EEPROM_CONFIG7_XBINS3;
      for(int x=EEPROM_CONFIG7_XBINS1; x<EEPROM_CONFIG7_YBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG7_XBINS1;
        if(EEPROM.read(x) != byte(boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = y - EEPROM_CONFIG7_XBINS2;
        if(EEPROM.read(y) != byte(vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(y, byte(vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = z - EEPROM_CONFIG7_XBINS3;
        if(EEPROM.read(z) != byte(stagingTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(z, byte(stagingTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        y++;
        z++;
      }
      //TPS/MAP bins
      y=EEPROM_CONFIG7_YBINS2;
      z=EEPROM_CONFIG7_YBINS3;
      for(int x=EEPROM_CONFIG7_YBINS1; x<EEPROM_CONFIG7_XSIZE2; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG7_YBINS1;
        if(EEPROM.read(x) != boostTable.axisY[offset]) { EEPROM.write(x, boostTable.axisY[offset]); writeCounter++; } //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
        offset = y - EEPROM_CONFIG7_YBINS2;
        if(EEPROM.read(y) != vvtTable.axisY[offset]) { EEPROM.write(y, vvtTable.axisY[offset]); writeCounter++; } //TABLE_LOAD_MULTIPLIER is NOT used for VVT as it is TPS based (0-100)
        offset = z - EEPROM_CONFIG7_YBINS3;
        if(EEPROM.read(z) != byte(stagingTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)) { EEPROM.write(z, byte(stagingTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        y++;
        z++;
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case seqFuelPage:
      /*---------------------------------------------------
      | Fuel trim tables (See storage.h for data layout) - Page 9
      | 6x6 tables itself + the 6 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE1) != trim1Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE1,trim1Table.xSize); writeCounter++; } //Write the boost Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE1) != trim1Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE1,trim1Table.ySize); writeCounter++; } //Write the boost Table MAP/TPS dimension size
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE2) != trim2Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE2,trim2Table.xSize); writeCounter++; } //Write the boost Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE2) != trim2Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE2,trim2Table.ySize); writeCounter++; } //Write the boost Table MAP/TPS dimension size
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE3) != trim3Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE3,trim3Table.xSize); writeCounter++; } //Write the boost Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE3) != trim3Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE3,trim3Table.ySize); writeCounter++; } //Write the boost Table MAP/TPS dimension size
      if(EEPROM.read(EEPROM_CONFIG8_XSIZE4) != trim4Table.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE4,trim4Table.xSize); writeCounter++; } //Write the boost Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG8_YSIZE4) != trim4Table.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE4,trim4Table.ySize); writeCounter++; } //Write the boost Table MAP/TPS dimension size

      y = EEPROM_CONFIG8_MAP2; //We do the 4 maps together in the same loop
      z = EEPROM_CONFIG8_MAP3; //We do the 4 maps together in the same loop
      i = EEPROM_CONFIG8_MAP4; //We do the 4 maps together in the same loop

      for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG8_MAP1;
        newVal = trim1Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(x) != newVal ) { EEPROM.write(x, newVal ); writeCounter++; } //Write the 6x6 map

        offset = y - EEPROM_CONFIG8_MAP2;
        newVal = trim2Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(y) != newVal ) { EEPROM.write(y, newVal); writeCounter++; } //Write the 6x6 map

        offset = z - EEPROM_CONFIG8_MAP3;
        newVal = trim3Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(z) != newVal ) { EEPROM.write(z, newVal); writeCounter++; } //Write the 6x6 map

        offset = i - EEPROM_CONFIG8_MAP4;
        newVal = trim4Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(i) != newVal ) { EEPROM.write(i, newVal); writeCounter++; } //Write the 6x6 map

        y++;
        z++;
        i++;
      }
      //RPM bins
      y = EEPROM_CONFIG8_XBINS2;
      z = EEPROM_CONFIG8_XBINS3;
      i = EEPROM_CONFIG8_XBINS4;
      for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { eepromWritesPending = true; break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG8_XBINS1;
        if( EEPROM.read(x) != (byte(trim1Table.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(x, byte(trim1Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = y - EEPROM_CONFIG8_XBINS2;
        if( EEPROM.read(y) != (byte(trim2Table.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(y, byte(trim2Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = z - EEPROM_CONFIG8_XBINS3;
        if( EEPROM.read(z) != (byte(trim3Table.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(z, byte(trim3Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = i - EEPROM_CONFIG8_XBINS4;
        if( EEPROM.read(i) != (byte(trim4Table.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(i, byte(trim4Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        y++;
        z++;
        i++;
      }
      //TPS/MAP bins
      y=EEPROM_CONFIG8_YBINS2;
      z=EEPROM_CONFIG8_YBINS3;
      i=EEPROM_CONFIG8_YBINS4;
      for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { eepromWritesPending = true; break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG8_YBINS1;
        if( EEPROM.read(x) != (byte(trim1Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)) ) { EEPROM.write(x, byte(trim1Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = y - EEPROM_CONFIG8_YBINS2;
        if( EEPROM.read(y) != (byte(trim2Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)) ) { EEPROM.write(y, byte(trim2Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = z - EEPROM_CONFIG8_YBINS3;
        if( EEPROM.read(z) != (byte(trim3Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)) ) { EEPROM.write(z, byte(trim3Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        offset = i - EEPROM_CONFIG8_YBINS4;
        if( EEPROM.read(i) != (byte(trim4Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)) ) { EEPROM.write(i, byte(trim4Table.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
        y++;
        z++;
        i++;
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case canbusPage:
      /*---------------------------------------------------
      | Config page 10 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      pnt_configPage = (byte *)&configPage9; //Create a pointer to Page 10 in memory
      for(int x=EEPROM_CONFIG9_START; x<EEPROM_CONFIG9_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG9_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG9_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case warmupPage:
      /*---------------------------------------------------
      | Config page 11 (See storage.h for data layout)
      | 192 byte long config table
      -----------------------------------------------------*/
      pnt_configPage = (byte *)&configPage10; //Create a pointer to Page 11 in memory
      //As there are no 3d tables in this page, all 192 bytes can simply be read in
      for(int x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG10_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG10_START))); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    case fuelMap2Page:
      /*---------------------------------------------------
      | Fuel table 2 (See storage.h for data layout)
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      if(EEPROM.read(EEPROM_CONFIG11_XSIZE) != fuelTable2.xSize) { EEPROM.write(EEPROM_CONFIG11_XSIZE, fuelTable2.xSize); writeCounter++; } //Write the 2nd fuel Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG11_YSIZE) != fuelTable2.ySize) { EEPROM.write(EEPROM_CONFIG11_YSIZE, fuelTable2.ySize); writeCounter++; } //Write the 2nd fuel Table MAP dimension size
      for(int x=EEPROM_CONFIG11_MAP; x<EEPROM_CONFIG11_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG11_MAP;
        if( EEPROM.read(x) != (fuelTable2.values[15-(offset/16)][offset%16]) ) { EEPROM.write(x, fuelTable2.values[15-(offset/16)][offset%16]); writeCounter++; }  //Write the 16x16 map
      }

      //RPM bins
      for(int x=EEPROM_CONFIG11_XBINS; x<EEPROM_CONFIG11_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG11_XBINS;
        if( EEPROM.read(x) != (byte(fuelTable2.axisX[offset]/TABLE_RPM_MULTIPLIER)) ) { EEPROM.write(x, byte(fuelTable2.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
      }
      //TPS/MAP bins
      for(int x=EEPROM_CONFIG11_YBINS; x<EEPROM_CONFIG11_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG11_YBINS;
        if( EEPROM.read(x) != (byte(fuelTable2.axisY[offset]/TABLE_LOAD_MULTIPLIER)) ) { EEPROM.write(x, byte(fuelTable2.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; } //Table load is divided by 2 (Allows for MAP up to 511)
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }
      break;
      //That concludes the writing of the 2nd fuel table

    case wmiMapPage:
      /*---------------------------------------------------
      | WMI tables (See storage.h for data layout) - Page 12
      | 8x8 table itself + the 8 values along each of the axis
      -----------------------------------------------------*/
      if(EEPROM.read(EEPROM_CONFIG12_XSIZE) != wmiTable.xSize) { EEPROM.write(EEPROM_CONFIG12_XSIZE,wmiTable.xSize); writeCounter++; } //Write the wmi Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG12_YSIZE) != wmiTable.ySize) { EEPROM.write(EEPROM_CONFIG12_YSIZE,wmiTable.ySize); writeCounter++; } //Write the wmi Table MAP dimension size

      for(int x=EEPROM_CONFIG12_MAP; x<EEPROM_CONFIG12_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG12_MAP;
        if(EEPROM.read(x) != (wmiTable.values[7-(offset/8)][offset%8]) ) { EEPROM.write(x, wmiTable.values[7-(offset/8)][offset%8]); writeCounter++; }  //Write the 8x8 map
      }
      //RPM bins
      for(int x=EEPROM_CONFIG12_XBINS; x<EEPROM_CONFIG12_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG12_XBINS;
        if(EEPROM.read(x) != byte(wmiTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(wmiTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
      }
      //MAP bins
      for(int x=EEPROM_CONFIG12_YBINS; x<EEPROM_CONFIG12_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; }
        offset = x - EEPROM_CONFIG12_YBINS;
        if(EEPROM.read(x) != byte(wmiTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)) { EEPROM.write(x, byte(wmiTable.axisY[offset]/TABLE_LOAD_MULTIPLIER)); writeCounter++; }
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;
      
  case progOutsPage:
      /*---------------------------------------------------
      | Config page 13 (See storage.h for data layout)
      -----------------------------------------------------*/
      pnt_configPage = (byte *)&configPage13; //Create a pointer to Page 12 in memory
      //As there are no 3d tables in this page, all bytes can simply be read in
      for(int x=EEPROM_CONFIG13_START; x<EEPROM_CONFIG13_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG13_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG13_START))); writeCounter++; }
      }
      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }
      break;
    
    case ignMap2Page:
      /*---------------------------------------------------
      | Ignition table (See storage.h for data layout) - Page 1
      | 16x16 table itself + the 16 values along each of the axis
      -----------------------------------------------------*/
      //Begin writing the Ignition table, basically the same thing as above
      if(EEPROM.read(EEPROM_CONFIG14_XSIZE) != ignitionTable2.xSize) { EEPROM.write(EEPROM_CONFIG14_XSIZE,ignitionTable2.xSize); writeCounter++; } //Write the ignition Table RPM dimension size
      if(EEPROM.read(EEPROM_CONFIG14_YSIZE) != ignitionTable2.ySize) { EEPROM.write(EEPROM_CONFIG14_YSIZE,ignitionTable2.ySize); writeCounter++; } //Write the ignition Table MAP/TPS dimension size

      for(int x=EEPROM_CONFIG14_MAP; x<EEPROM_CONFIG14_XBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG14_MAP;
        newVal = ignitionTable2.values[15-(offset/16)][offset%16];
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; }  //Write the 16x16 map with translation
      }
      //RPM bins
      for(int x=EEPROM_CONFIG14_XBINS; x<EEPROM_CONFIG14_YBINS; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG14_XBINS;
        newVal = ignitionTable2.axisX[offset]/TABLE_RPM_MULTIPLIER;
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; } //RPM bins are divided by 100 and converted to a byte
      }
      //TPS/MAP bins
      for(int x=EEPROM_CONFIG14_YBINS; x<EEPROM_CONFIG14_END; x++)
      {
        if( (writeCounter > EEPROM_MAX_WRITE_BLOCK) ) { break; } //This is a safety check to make sure we don't attempt to write too much to the EEPROM at a time.
        offset = x - EEPROM_CONFIG14_YBINS;
        newVal = ignitionTable2.axisY[offset]/TABLE_LOAD_MULTIPLIER;
        if(EEPROM.read(x) != newVal) { EEPROM.write(x, newVal); writeCounter++; } //Table load is divided by 2 (Allows for MAP up to 511)
      }

      if(writeCounter > EEPROM_MAX_WRITE_BLOCK) { eepromWritesPending = true; }
      else { eepromWritesPending = false; }

      break;

    default:
      break;
  }

}

void resetConfigPages()
{
  memset(&configPage2, 0, sizeof(config2));
  memset(&configPage4, 0, sizeof(config4));
  memset(&configPage6, 0, sizeof(config6));
  memset(&configPage9, 0, sizeof(config9));
  memset(&configPage10, 0, sizeof(config10));
  memset(&configPage13, 0, sizeof(config13));
}

namespace
{
  inline int8_t offsetToValueYIndex(const table3D *pTable, uint16_t offset)
  {
    //This is slightly non-intuitive, but essentially just flips the table vertically (IE top line becomes the bottom line etc). 
    return pTable->xSize-1 - (offset / pTable->xSize);
  }

  inline int8_t offsetToValueXIndex(const table3D *pTable, uint16_t offset)
  {
    return offset % pTable->xSize;
  }

  int loadTableValues(table3D *pTable, int index)
  {
    for(int offset=0; offset<sq(pTable->xSize); ++offset,++index)
    {
      pTable->values[offsetToValueYIndex(pTable, offset)][offsetToValueXIndex(pTable, offset)] = EEPROM.read(index);
    }

    return index; 
  }

  int loadTableAxisX(table3D *pTable, int index, int xAxisMultiplier)
  {
    for(int offset=0; offset<pTable->xSize; ++offset,++index)
    {
      pTable->axisX[offset] = (EEPROM.read(index) * xAxisMultiplier); //RPM bins are divided by 100 when stored. Multiply them back now
    }
    return index;
  }

  int loadTableAxisY(table3D *pTable, int index, int yAxisMultiplier)
  {
    for(int offset=0; offset<pTable->xSize; ++offset,++index)
    {
      pTable->axisY[offset] = EEPROM.read(index) * yAxisMultiplier;
    }

    return index;
  }

  int loadTable(table3D *pTable, int index, int xAxisMultiplier, int yAxisMultiplier)
  {
    return loadTableAxisY(pTable,
                          loadTableAxisX(pTable, 
                                          loadTableValues(pTable, index), 
                                          xAxisMultiplier),
                          yAxisMultiplier);
  }

  int loadMemoryBlock(byte *pStart, byte *pEnd, int index)
  {
    while (pStart!=pEnd)
    {
      *pStart = EEPROM.read(index);
      ++pStart;
      ++index;
    }
    return index;
  }
}

void loadConfig()
{
  loadTable(&fuelTable, EEPROM_CONFIG1_MAP, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadMemoryBlock((byte *)&configPage2, (byte *)&configPage2+sizeof(configPage2), EEPROM_CONFIG2_START);
  //That concludes the reading of the VE table
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&ignitionTable, EEPROM_CONFIG3_MAP, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadMemoryBlock((byte *)&configPage4, (byte *)&configPage4+sizeof(configPage4), EEPROM_CONFIG4_START);

  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&afrTable, EEPROM_CONFIG5_MAP, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadMemoryBlock((byte *)&configPage6, (byte *)&configPage6+sizeof(configPage6), EEPROM_CONFIG6_START);

  //*********************************************************************************************************************************************************************************
  // Boost and vvt tables load
  loadTable(&boostTable, EEPROM_CONFIG7_MAP1, TABLE_RPM_MULTIPLIER, 1);
  loadTable(&vvtTable, EEPROM_CONFIG7_MAP2, TABLE_RPM_MULTIPLIER, 1);
  loadTable(&stagingTable, EEPROM_CONFIG7_MAP3, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
  // Fuel trim tables load
  loadTable(&trim1Table, EEPROM_CONFIG8_MAP1, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadTable(&trim2Table, EEPROM_CONFIG8_MAP2, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadTable(&trim3Table, EEPROM_CONFIG8_MAP3, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
  loadTable(&trim4Table, EEPROM_CONFIG8_MAP4, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
  //canbus control page load
  loadMemoryBlock((byte *)&configPage9, (byte *)&configPage9+sizeof(configPage9), EEPROM_CONFIG9_START);

  //*********************************************************************************************************************************************************************************

  //CONFIG PAGE (10)
  loadMemoryBlock((byte *)&configPage10, (byte *)&configPage10+sizeof(configPage10), EEPROM_CONFIG10_START);

  //*********************************************************************************************************************************************************************************
  //Fuel table 2 (See storage.h for data layout)
  loadTable(&fuelTable2, EEPROM_CONFIG11_MAP, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
  // WMI table load
  loadTable(&wmiTable, EEPROM_CONFIG12_MAP, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);
    
  //*********************************************************************************************************************************************************************************
  //CONFIG PAGE (13)
  loadMemoryBlock((byte *)&configPage13, (byte *)&configPage13+sizeof(configPage13), EEPROM_CONFIG13_START);

  //*********************************************************************************************************************************************************************************
  //SECOND IGNITION CONFIG PAGE (14)

  //Begin writing the Ignition table, basically the same thing as above
  loadTable(&ignitionTable2, EEPROM_CONFIG14_MAP, TABLE_RPM_MULTIPLIER, TABLE_LOAD_MULTIPLIER);

  //*********************************************************************************************************************************************************************************
}

/*
Reads the calibration information from EEPROM.
This is separate from the config load as the calibrations do not exist as pages within the ini file for Tuner Studio
*/
void loadCalibration()
{

  for(int x=0; x<32; x++) //Each calibration table is 32 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + (x * 2);
    EEPROM.get(y, cltCalibration_bins[x]);
    y += 64; 
    EEPROM.get(y, cltCalibration_values[x]);

    y = EEPROM_CALIBRATION_IAT + (x * 2);
    EEPROM.get(y, iatCalibration_bins[x]);
    y += 64; 
    EEPROM.get(y, iatCalibration_values[x]);

    y = EEPROM_CALIBRATION_O2 + (x * 2);
    EEPROM.get(y, o2Calibration_bins[x]);
    y = EEPROM_CALIBRATION_O2 + 64 + x;
    o2Calibration_values[x] = EEPROM.read(y); //Byte values

  }

}

/*
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void writeCalibration()
{

  for(int x=0; x<32; x++) //Each calibration table is 32 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + (x * 2);
    EEPROM.put(y, cltCalibration_bins[x]);
    y += 64; 
    EEPROM.put(y, cltCalibration_values[x]);

    y = EEPROM_CALIBRATION_IAT + (x * 2);
    EEPROM.put(y, iatCalibration_bins[x]);
    y += 64; 
    EEPROM.put(y, iatCalibration_values[x]);

    y = EEPROM_CALIBRATION_O2 + (x * 2);
    EEPROM.put(y, o2Calibration_bins[x]);
    y = EEPROM_CALIBRATION_O2 + 64 + x; 
    EEPROM.update(y, o2Calibration_values[x]);
  }

}

/*
Takes a page number and CRC32 value then stores it in the relevant place in EEPROM
Note: Each pages requires 4 bytes for its CRC32. These are stored in reverse page order (ie the last page is store first in EEPROM)
*/
void storePageCRC32(byte pageNo, uint32_t crc32_val)
{
  uint16_t address; //Start address for the relevant page
  address = EEPROM_PAGE_CRC32 + ((NUM_PAGES - pageNo) * 4);

  //One = Most significant -> Four = Least significant byte
  byte four = (crc32_val & 0xFF);
  byte three = ((crc32_val >> 8) & 0xFF);
  byte two = ((crc32_val >> 16) & 0xFF);
  byte one = ((crc32_val >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.update(address, four);
  EEPROM.update(address + 1, three);
  EEPROM.update(address + 2, two);
  EEPROM.update(address + 3, one);
}

/*
Retrieves and returns the 4 byte CRC32 for a given page from EEPROM
*/
uint32_t readPageCRC32(byte pageNo)
{
  uint16_t address; //Start address for the relevant page
  address = EEPROM_PAGE_CRC32 + ((NUM_PAGES - pageNo) * 4);

  //Read the 4 bytes from the eeprom memory.
  uint32_t four = EEPROM.read(address);
  uint32_t three = EEPROM.read(address + 1);
  uint32_t two = EEPROM.read(address + 2);
  uint32_t one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

// Utility functions.
// By having these in this file, it prevents other files from calling EEPROM functions directly. This is useful due to differences in the EEPROM libraries on different devces
byte readLastBaro() { return EEPROM.read(EEPROM_LAST_BARO); }
void storeLastBaro(byte newValue) { EEPROM.update(EEPROM_LAST_BARO, newValue); }
void storeCalibrationValue(uint16_t location, byte value) { EEPROM.update(location, value); } //This is essentially just an abstraction for EEPROM.update()
byte readEEPROMVersion() { return EEPROM.read(EEPROM_DATA_VERSION); }
void storeEEPROMVersion(byte newVersion) { EEPROM.update(EEPROM_DATA_VERSION, newVersion); }
