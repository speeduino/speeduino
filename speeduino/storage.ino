/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include "storage.h"
#include "globals.h"
#include "table.h"
#include "comms.h"
#include <EEPROM.h>

void writeAllConfig()
{
  writeConfig(1);
  if (eepromWritesPending == false) { writeConfig(2); }
  if (eepromWritesPending == false) { writeConfig(3); }
  if (eepromWritesPending == false) { writeConfig(4); }
  if (eepromWritesPending == false) { writeConfig(5); }
  if (eepromWritesPending == false) { writeConfig(6); }
  if (eepromWritesPending == false) { writeConfig(7); }
  if (eepromWritesPending == false) { writeConfig(8); }
  if (eepromWritesPending == false) { writeConfig(9); }
  if (eepromWritesPending == false) { writeConfig(10); }
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
        EEPROM.update(x, fuelTable.axisY[offset] / TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
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
        EEPROM.update(x, afrTable.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
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
        if (EEPROM.read(x) != newVal ) { EEPROM.update(x, newVal ); writeCounter++; } //Write the 6x6 map

        offset = y - EEPROM_CONFIG8_MAP2;
        newVal = trim2Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(y) != newVal ) { EEPROM.update(y, newVal); writeCounter++; } //Write the 6x6 map

        offset = z - EEPROM_CONFIG8_MAP3;
        newVal = trim3Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(z) != newVal ) { EEPROM.update(z, newVal); writeCounter++; } //Write the 6x6 map

        offset = i - EEPROM_CONFIG8_MAP4;
        newVal = trim4Table.values[5-(offset/6)][offset%6];
        if (EEPROM.read(i) != newVal ) { EEPROM.update(i, newVal); writeCounter++; } //Write the 6x6 map

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
        EEPROM.update(x, byte(trim1Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
        offset = y - EEPROM_CONFIG8_XBINS2;
        EEPROM.update(y, byte(trim2Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
        offset = z - EEPROM_CONFIG8_XBINS3;
        EEPROM.update(z, byte(trim3Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
        offset = i - EEPROM_CONFIG8_XBINS4;
        EEPROM.update(i, byte(trim4Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
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
        EEPROM.update(x, trim1Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
        offset = y - EEPROM_CONFIG8_YBINS2;
        EEPROM.update(y, trim2Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
        offset = z - EEPROM_CONFIG8_YBINS3;
        EEPROM.update(z, trim3Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
        offset = i - EEPROM_CONFIG8_YBINS4;
        EEPROM.update(i, trim4Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
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
      | 128 byte long config table
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

    default:
      break;
  }
}

void loadConfig()
{
  int offset;
  //Create a pointer to the config page
  byte* pnt_configPage;


  //Fuel table (See storage.h for data layout)
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_MAP;
    fuelTable.values[15-(offset/16)][offset%16] = EEPROM.read(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    fuelTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    fuelTable.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER;
  }

  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 1 in memory
  for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG2_START)) = EEPROM.read(x);
  }
  //That concludes the reading of the VE table

  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  //Begin writing the Ignition table, basically the same thing as above
  for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_MAP;
    ignitionTable.values[15-(offset/16)][offset%16] = EEPROM.read(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_XBINS;
    ignitionTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    ignitionTable.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
  }

  pnt_configPage = (byte *)&configPage4; //Create a pointer to Page 4 in memory
  for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG4_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)

  //Begin writing the Ignition table, basically the same thing as above
  for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_MAP;
    afrTable.values[15-(offset/16)][offset%16] = EEPROM.read(x); //Read the 16x16 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_XBINS;
    afrTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    afrTable.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
  }

  pnt_configPage = (byte *)&configPage6; //Create a pointer to Page 6 in memory
  for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG6_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************
  // Boost and vvt tables load
  int y = EEPROM_CONFIG7_MAP2;
  int z = EEPROM_CONFIG7_MAP3;
  for(int x=EEPROM_CONFIG7_MAP1; x<EEPROM_CONFIG7_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG7_MAP1;
    boostTable.values[7-(offset/8)][offset%8] = EEPROM.read(x); //Read the 8x8 map
    offset = y - EEPROM_CONFIG7_MAP2;
    vvtTable.values[7-(offset/8)][offset%8] = EEPROM.read(y); //Read the 8x8 map
    offset = z - EEPROM_CONFIG7_MAP3;
    stagingTable.values[7-(offset/8)][offset%8] = EEPROM.read(z); //Read the 8x8 map
    y++;
    z++;
  }

  //RPM bins
  y = EEPROM_CONFIG7_XBINS2;
  z = EEPROM_CONFIG7_XBINS3;
  for(int x=EEPROM_CONFIG7_XBINS1; x<EEPROM_CONFIG7_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG7_XBINS1;
    boostTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = y - EEPROM_CONFIG7_XBINS2;
    vvtTable.axisX[offset] = (EEPROM.read(y) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = z - EEPROM_CONFIG7_XBINS3;
    stagingTable.axisX[offset] = (EEPROM.read(z) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    y++;
    z++;
  }

  //TPS/MAP bins
  y = EEPROM_CONFIG7_YBINS2;
  z = EEPROM_CONFIG7_YBINS3;
  for(int x=EEPROM_CONFIG7_YBINS1; x<EEPROM_CONFIG7_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG7_YBINS1;
    boostTable.axisY[offset] = EEPROM.read(x); //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
    offset = y - EEPROM_CONFIG7_YBINS2;
    vvtTable.axisY[offset] = EEPROM.read(y); //TABLE_LOAD_MULTIPLIER is NOT used for VVT as it is TPS based (0-100)
    offset = z - EEPROM_CONFIG7_YBINS3;
    stagingTable.axisY[offset] = EEPROM.read(z) * TABLE_LOAD_MULTIPLIER;
    y++;
    z++;
  }

  //*********************************************************************************************************************************************************************************
  // Fuel trim tables load
  y = EEPROM_CONFIG8_MAP2;
  z = EEPROM_CONFIG8_MAP3;
  int i = EEPROM_CONFIG8_MAP4;
  for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_MAP1;
    trim1Table.values[5-(offset/6)][offset%6] = EEPROM.read(x); //Read the 6x6 map
    offset = y - EEPROM_CONFIG8_MAP2;
    trim2Table.values[5-(offset/6)][offset%6] = EEPROM.read(y); //Read the 6x6 map
    offset = z - EEPROM_CONFIG8_MAP3;
    trim3Table.values[5-(offset/6)][offset%6] = EEPROM.read(z); //Read the 6x6 map
    offset = i - EEPROM_CONFIG8_MAP4;
    trim4Table.values[5-(offset/6)][offset%6] = EEPROM.read(i); //Read the 6x6 map
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
    offset = x - EEPROM_CONFIG8_XBINS1;
    trim1Table.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = y - EEPROM_CONFIG8_XBINS2;
    trim2Table.axisX[offset] = (EEPROM.read(y) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = z - EEPROM_CONFIG8_XBINS3;
    trim3Table.axisX[offset] = (EEPROM.read(z) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = i - EEPROM_CONFIG8_XBINS4;
    trim4Table.axisX[offset] = (EEPROM.read(i) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    y++;
    z++;
    i++;
  }

  //TPS/MAP bins
  y = EEPROM_CONFIG8_YBINS2;
  z = EEPROM_CONFIG8_YBINS3;
  i = EEPROM_CONFIG8_YBINS4;
  for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG8_YBINS1;
    trim1Table.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    offset = y - EEPROM_CONFIG8_YBINS2;
    trim2Table.axisY[offset] = EEPROM.read(y) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    offset = z - EEPROM_CONFIG8_YBINS3;
    trim3Table.axisY[offset] = EEPROM.read(z) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    offset = i - EEPROM_CONFIG8_YBINS4;
    trim4Table.axisY[offset] = EEPROM.read(i) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    y++;
    z++;
    i++;
  }
  //*********************************************************************************************************************************************************************************
  //canbus control page load
    pnt_configPage = (byte *)&configPage9; //Create a pointer to Page 10 in memory
  for(int x=EEPROM_CONFIG9_START; x<EEPROM_CONFIG9_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG9_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************

  //CONFIG PAGE (10)
  pnt_configPage = (byte *)&configPage10; //Create a pointer to Page 11 in memory
  //All 192 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG10_START)) = EEPROM.read(x);
  }

}

/*
Reads the calibration information from EEPROM.
This is separate from the config load as the calibrations do not exist as pages within the ini file for Tuner Studio
*/
void loadCalibration()
{

  for(int x=0; x<CALIBRATION_TABLE_SIZE; x++) //Each calibration table is 512 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + x;
    cltCalibrationTable[x] = EEPROM.read(y);

    y = EEPROM_CALIBRATION_IAT + x;
    iatCalibrationTable[x] = EEPROM.read(y);

    y = EEPROM_CALIBRATION_O2 + x;
    o2CalibrationTable[x] = EEPROM.read(y);
  }

}

/*
This takes the values in the 3 calibration tables (Coolant, Inlet temp and O2)
and saves them to the EEPROM.
*/
void writeCalibration()
{

  for(int x=0; x<CALIBRATION_TABLE_SIZE; x++) //Each calibration table is 512 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + x;
    if(EEPROM.read(y) != cltCalibrationTable[x]) { EEPROM.write(y, cltCalibrationTable[x]); }

    y = EEPROM_CALIBRATION_IAT + x;
    if(EEPROM.read(y) != iatCalibrationTable[x]) { EEPROM.write(y, iatCalibrationTable[x]); }

    y = EEPROM_CALIBRATION_O2 + x;
    if(EEPROM.read(y) != o2CalibrationTable[x]) { EEPROM.write(y, o2CalibrationTable[x]); }
  }

}
