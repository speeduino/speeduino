/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include "storage.h"
#include "globals.h"
#include "table.h"
#include <EEPROM.h>

/*
Takes the current configuration (config pages and maps)
and writes them to EEPROM as per the layout defined in storage.h
*/
void writeConfig()
{
  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */

  int offset;
  //Create a pointer to the config page
  byte* pnt_configPage;

  if(EEPROM.read(0) != data_structure_version) { EEPROM.write(0,data_structure_version); }   //Write the data structure version


  /*---------------------------------------------------
  | Fuel table (See storage.h for data layout) - Page 1
  | 16x16 table itself + the 16 values along each of the axis
  -----------------------------------------------------*/
  if(EEPROM.read(EEPROM_CONFIG1_XSIZE) != fuelTable.xSize) { EEPROM.write(EEPROM_CONFIG1_XSIZE, fuelTable.xSize); } //Write the VE Tables RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG1_YSIZE) != fuelTable.ySize) { EEPROM.write(EEPROM_CONFIG1_YSIZE, fuelTable.ySize); } //Write the VE Tables MAP/TPS dimension size
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_MAP;
    if(EEPROM.read(x) != fuelTable.values[15-offset/16][offset%16]) { EEPROM.write(x, fuelTable.values[15-offset/16][offset%16]); }  //Write the 16x16 map
  }

  //RPM bins
  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    if(EEPROM.read(x) != byte(fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(fuelTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++)
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    EEPROM.update(x, fuelTable.axisY[offset] / TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
  }
  //That concludes the writing of the VE table
  //*********************************************************************************************************************************************************************************

  /*---------------------------------------------------
  | Config page 2 (See storage.h for data layout)
  | 64 byte long config table
  -----------------------------------------------------*/
  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
  {
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))); }
  }
  //*********************************************************************************************************************************************************************************


  /*---------------------------------------------------
  | Ignition table (See storage.h for data layout) - Page 1
  | 16x16 table itself + the 16 values along each of the axis
  -----------------------------------------------------*/
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROM.read(EEPROM_CONFIG3_XSIZE) != ignitionTable.xSize) { EEPROM.write(EEPROM_CONFIG3_XSIZE,ignitionTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG3_YSIZE) != ignitionTable.ySize) { EEPROM.write(EEPROM_CONFIG3_YSIZE,ignitionTable.ySize); } //Write the ignition Table MAP/TPS dimension size

  for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_MAP;
    if(EEPROM.read(x) != ignitionTable.values[15-offset/16][offset%16]) { EEPROM.write(x, ignitionTable.values[15-offset/16][offset%16]); }  //Write the 16x16 map with translation
  }
  //RPM bins
  for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_XBINS;
    if(EEPROM.read(x) != byte(ignitionTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(ignitionTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++)
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    EEPROM.update(x, ignitionTable.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
  }
  //That concludes the writing of the IGN table
//*********************************************************************************************************************************************************************************

  /*---------------------------------------------------
  | Config page 2 (See storage.h for data layout)
  | 64 byte long config table
  -----------------------------------------------------*/
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
  {
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))); }
  }
  //*********************************************************************************************************************************************************************************


  /*---------------------------------------------------
  | AFR table (See storage.h for data layout) - Page 5
  | 16x16 table itself + the 16 values along each of the axis
  -----------------------------------------------------*/
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROM.read(EEPROM_CONFIG5_XSIZE) != afrTable.xSize) { EEPROM.write(EEPROM_CONFIG5_XSIZE,afrTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG5_YSIZE) != afrTable.ySize) { EEPROM.write(EEPROM_CONFIG5_YSIZE,afrTable.ySize); } //Write the ignition Table MAP/TPS dimension size

  for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_MAP;
    if(EEPROM.read(x) != afrTable.values[15-offset/16][offset%16]) { EEPROM.write(x, afrTable.values[15-offset/16][offset%16]); }  //Write the 16x16 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_XBINS;
    if(EEPROM.read(x) != byte(afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(afrTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++)
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    EEPROM.update(x, afrTable.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
  }
  //That concludes the writing of the AFR table
  //*********************************************************************************************************************************************************************************

  /*---------------------------------------------------
  | Config page 3 (See storage.h for data layout)
  | 64 byte long config table
  -----------------------------------------------------*/
  pnt_configPage = (byte *)&configPage3; //Create a pointer to Page 3 in memory
  for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
  {
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))); }
  }
    //*********************************************************************************************************************************************************************************

  /*---------------------------------------------------
  | Config page 4 (See storage.h for data layout)
  | 64 byte long config table
  -----------------------------------------------------*/
  pnt_configPage = (byte *)&configPage4; //Create a pointer to Page 4 in memory
  //The next 128 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG7_START; x<EEPROM_CONFIG7_END; x++)
  {
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG7_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG7_START))); }
  }

  /*---------------------------------------------------
  | Boost and vvt tables (See storage.h for data layout) - Page 8
  | 8x8 table itself + the 8 values along each of the axis
  -----------------------------------------------------*/
  //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
  if(EEPROM.read(EEPROM_CONFIG8_XSIZE1) != boostTable.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE1,boostTable.xSize); } //Write the boost Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG8_YSIZE1) != boostTable.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE1,boostTable.ySize); } //Write the boost Table MAP/TPS dimension size
  if(EEPROM.read(EEPROM_CONFIG8_XSIZE2) != vvtTable.xSize) { EEPROM.write(EEPROM_CONFIG8_XSIZE2,vvtTable.xSize); } //Write the vvt Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG8_YSIZE2) != vvtTable.ySize) { EEPROM.write(EEPROM_CONFIG8_YSIZE2,vvtTable.ySize); } //Write the vvt Table MAP/TPS dimension size

  int y = EEPROM_CONFIG8_MAP2; //We do the 2 maps together in the same loop
  for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_MAP1;
    if(EEPROM.read(x) != boostTable.values[7-offset/8][offset%8]) { EEPROM.write(x, boostTable.values[7-offset/8][offset%8]); }  //Write the 8x8 map
    offset = y - EEPROM_CONFIG8_MAP2;
    if(EEPROM.read(y) != vvtTable.values[7-offset/8][offset%8]) { EEPROM.write(y, vvtTable.values[7-offset/8][offset%8]); }  //Write the 8x8 map
    y++;
  }
  //RPM bins
  y = EEPROM_CONFIG8_XBINS2;
  for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_XBINS1;
    if(EEPROM.read(x) != byte(boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(x, byte(boostTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); } //RPM bins are divided by 100 and converted to a byte
    offset = y - EEPROM_CONFIG8_XBINS2;
    if(EEPROM.read(y) != byte(vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER)) { EEPROM.write(y, byte(vvtTable.axisX[offset]/TABLE_RPM_MULTIPLIER)); } //RPM bins are divided by 100 and converted to a byte
    y++;
  }
  //TPS/MAP bins
  y=EEPROM_CONFIG8_YBINS2;
  for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG8_YBINS1;
    EEPROM.update(x, boostTable.axisY[offset]); //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
    offset = y - EEPROM_CONFIG8_YBINS2;
    EEPROM.update(y, vvtTable.axisY[offset]); //TABLE_LOAD_MULTIPLIER is NOT used for VVT as it is TPS based (0-100)
    y++;
  }

  /*---------------------------------------------------
  | Fuel trim tables (See storage.h for data layout) - Page 9
  | 6x6 tables itself + the 6 values along each of the axis
  -----------------------------------------------------*/
  //Begin writing the 2 tables, basically the same thing as above but we're doing these 2 together (2 tables per page instead of 1)
  EEPROM.update(EEPROM_CONFIG9_XSIZE1,trim1Table.xSize); //Write the boost Table RPM dimension size
  EEPROM.update(EEPROM_CONFIG9_YSIZE1,trim1Table.ySize); //Write the boost Table MAP/TPS dimension size
  EEPROM.update(EEPROM_CONFIG9_XSIZE2,trim2Table.xSize); //Write the boost Table RPM dimension size
  EEPROM.update(EEPROM_CONFIG9_YSIZE2,trim2Table.ySize); //Write the boost Table MAP/TPS dimension size
  EEPROM.update(EEPROM_CONFIG9_XSIZE3,trim3Table.xSize); //Write the boost Table RPM dimension size
  EEPROM.update(EEPROM_CONFIG9_YSIZE3,trim3Table.ySize); //Write the boost Table MAP/TPS dimension size
  EEPROM.update(EEPROM_CONFIG9_XSIZE4,trim4Table.xSize); //Write the boost Table RPM dimension size
  EEPROM.update(EEPROM_CONFIG9_YSIZE4,trim4Table.ySize); //Write the boost Table MAP/TPS dimension size

  y = EEPROM_CONFIG9_MAP2; //We do the 4 maps together in the same loop
  int z = EEPROM_CONFIG9_MAP3; //We do the 4 maps together in the same loop
  int i = EEPROM_CONFIG9_MAP4; //We do the 4 maps together in the same loop
  for(int x=EEPROM_CONFIG9_MAP1; x<EEPROM_CONFIG9_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG9_MAP1;
    EEPROM.update(x, trim1Table.values[5-offset/6][offset%6]);  //Write the 6x6 map
    offset = y - EEPROM_CONFIG9_MAP2;
    EEPROM.update(y, trim2Table.values[5-offset/6][offset%6]);  //Write the 6x6 map
    offset = z - EEPROM_CONFIG9_MAP3;
    EEPROM.update(z, trim3Table.values[5-offset/6][offset%6]);  //Write the 6x6 map
    offset = i - EEPROM_CONFIG9_MAP4;
    EEPROM.update(i, trim4Table.values[5-offset/6][offset%6]);  //Write the 6x6 map
    y++;
    z++;
    i++;
  }
  //RPM bins
  y = EEPROM_CONFIG9_XBINS2;
  z = EEPROM_CONFIG9_XBINS3;
  i = EEPROM_CONFIG9_XBINS4;
  for(int x=EEPROM_CONFIG9_XBINS1; x<EEPROM_CONFIG9_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG9_XBINS1;
    EEPROM.update(x, byte(trim1Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
    offset = y - EEPROM_CONFIG9_XBINS2;
    EEPROM.update(y, byte(trim2Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
    offset = z - EEPROM_CONFIG9_XBINS3;
    EEPROM.update(z, byte(trim3Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
    offset = i - EEPROM_CONFIG9_XBINS4;
    EEPROM.update(i, byte(trim4Table.axisX[offset]/TABLE_RPM_MULTIPLIER)); //RPM bins are divided by 100 and converted to a byte
    y++;
    z++;
    i++;
  }
  //TPS/MAP bins
  y=EEPROM_CONFIG9_YBINS2;
  z=EEPROM_CONFIG9_YBINS3;
  i=EEPROM_CONFIG9_YBINS4;
  for(int x=EEPROM_CONFIG9_YBINS1; x<EEPROM_CONFIG9_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG9_YBINS1;
    EEPROM.update(x, trim1Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
    offset = y - EEPROM_CONFIG9_YBINS2;
    EEPROM.update(y, trim2Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
    offset = z - EEPROM_CONFIG9_YBINS3;
    EEPROM.update(z, trim3Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
    offset = i - EEPROM_CONFIG9_YBINS4;
    EEPROM.update(i, trim4Table.axisY[offset]/TABLE_LOAD_MULTIPLIER); //Table load is divided by 2 (Allows for MAP up to 511)
    y++;
    z++;
    i++;
  }
    //*********************************************************************************************************************************************************************************

  /*---------------------------------------------------
  | Config page 10 (See storage.h for data layout)
  | 128 byte long config table
  -----------------------------------------------------*/
  pnt_configPage = (byte *)&configPage10; //Create a pointer to Page 10 in memory
  for(int x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
  {
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG10_START))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG10_START))); }
  }
    //*********************************************************************************************************************************************************************************

}

void loadConfig()
{
  int offset;
  //Create a pointer to the config page
  byte* pnt_configPage;


  //Fuel table (See storage.h for data layout)
  //fuelTable.xSize = EEPROM.read(EEPROM_CONFIG1_XSIZE); //Read the VE Tables RPM dimension size
  //fuelTable.ySize = EEPROM.read(EEPROM_CONFIG1_YSIZE); //Read the VE Tables MAP/TPS dimension size
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG1_MAP;
    fuelTable.values[15-offset/16][offset%16] = EEPROM.read(x); //Read the 8x8 map
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

  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
  for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG2_START)) = EEPROM.read(x);
  }
  //That concludes the reading of the VE table

  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROM.read(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROM.read(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)

  for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG3_MAP;
    ignitionTable.values[15-offset/16][offset%16] = EEPROM.read(x); //Read the 8x8 map
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

  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG4_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)

  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROM.read(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROM.read(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)

  for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++)
  {
    offset = x - EEPROM_CONFIG5_MAP;
    afrTable.values[15-offset/16][offset%16] = EEPROM.read(x); //Read the 16x16 map
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

  pnt_configPage = (byte *)&configPage3; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG6_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************

  //CONFIG PAGE (4)                                                                                                                                           //############
  pnt_configPage = (byte *)&configPage4; //Create a pointer to Page 3 in memory
  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROM.read(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROM.read(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)

  //The next 64 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG7_START; x<EEPROM_CONFIG7_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG7_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************
  // Boost and vvt tables load
  int y = EEPROM_CONFIG8_MAP2;
  for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_MAP1;
    boostTable.values[7-offset/8][offset%8] = EEPROM.read(x); //Read the 8x8 map
    offset = y - EEPROM_CONFIG8_MAP2;
    vvtTable.values[7-offset/8][offset%8] = EEPROM.read(y); //Read the 8x8 map
    y++;
  }

  //RPM bins
  y = EEPROM_CONFIG8_XBINS2;
  for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG8_XBINS1;
    boostTable.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = y - EEPROM_CONFIG8_XBINS2;
    vvtTable.axisX[offset] = (EEPROM.read(y) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    y++;
  }

  //TPS/MAP bins
  y = EEPROM_CONFIG8_YBINS2;
  for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG8_YBINS1;
    boostTable.axisY[offset] = EEPROM.read(x); //TABLE_LOAD_MULTIPLIER is NOT used for boost as it is TPS based (0-100)
    offset = y - EEPROM_CONFIG8_YBINS2;
    vvtTable.axisY[offset] = EEPROM.read(y); //TABLE_LOAD_MULTIPLIER is NOT used for VVT as it is TPS based (0-100)
    y++;
  }

  //*********************************************************************************************************************************************************************************
  // Fuel trim tables load
  y = EEPROM_CONFIG9_MAP2;
  int z = EEPROM_CONFIG9_MAP3;
  int i = EEPROM_CONFIG9_MAP4;
  for(int x=EEPROM_CONFIG9_MAP1; x<EEPROM_CONFIG9_XBINS1; x++)
  {
    offset = x - EEPROM_CONFIG9_MAP1;
    trim1Table.values[5-offset/6][offset%6] = EEPROM.read(x); //Read the 6x6 map
    offset = y - EEPROM_CONFIG9_MAP2;
    trim2Table.values[5-offset/6][offset%6] = EEPROM.read(y); //Read the 6x6 map
    offset = z - EEPROM_CONFIG9_MAP3;
    trim3Table.values[5-offset/6][offset%6] = EEPROM.read(z); //Read the 6x6 map
    offset = i - EEPROM_CONFIG9_MAP4;
    trim4Table.values[5-offset/6][offset%6] = EEPROM.read(i); //Read the 6x6 map
    y++;
    z++;
    i++;
  }

  //RPM bins
  y = EEPROM_CONFIG9_XBINS2;
  z = EEPROM_CONFIG9_XBINS3;
  i = EEPROM_CONFIG9_XBINS4;
  for(int x=EEPROM_CONFIG9_XBINS1; x<EEPROM_CONFIG9_YBINS1; x++)
  {
    offset = x - EEPROM_CONFIG9_XBINS1;
    trim1Table.axisX[offset] = (EEPROM.read(x) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = y - EEPROM_CONFIG9_XBINS2;
    trim2Table.axisX[offset] = (EEPROM.read(y) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = z - EEPROM_CONFIG9_XBINS3;
    trim3Table.axisX[offset] = (EEPROM.read(z) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = i - EEPROM_CONFIG9_XBINS4;
    trim4Table.axisX[offset] = (EEPROM.read(i) * TABLE_RPM_MULTIPLIER); //RPM bins are divided by 100 when stored. Multiply them back now
    y++;
    z++;
    i++;
  }

  //TPS/MAP bins
  y = EEPROM_CONFIG9_YBINS2;
  z = EEPROM_CONFIG9_YBINS3;
  i = EEPROM_CONFIG9_YBINS4;
  for(int x=EEPROM_CONFIG9_YBINS1; x<EEPROM_CONFIG9_XSIZE2; x++)
  {
    offset = x - EEPROM_CONFIG9_YBINS1;
    trim1Table.axisY[offset] = EEPROM.read(x) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    offset = y - EEPROM_CONFIG9_YBINS2;
    trim2Table.axisY[offset] = EEPROM.read(y) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    offset = z - EEPROM_CONFIG9_YBINS3;
    trim3Table.axisY[offset] = EEPROM.read(z) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    offset = i - EEPROM_CONFIG9_YBINS4;
    trim4Table.axisY[offset] = EEPROM.read(i) * TABLE_LOAD_MULTIPLIER; //Table load is divided by 2 (Allows for MAP up to 511)
    y++;
    z++;
    i++;
  }
  //*********************************************************************************************************************************************************************************
  //canbus control page load
    pnt_configPage = (byte *)&configPage10; //Create a pointer to Page 10 in memory
  for(int x=EEPROM_CONFIG10_START; x<EEPROM_CONFIG10_END; x++)
  {
    *(pnt_configPage + byte(x - EEPROM_CONFIG10_START)) = EEPROM.read(x);
  }

  //*********************************************************************************************************************************************************************************

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
