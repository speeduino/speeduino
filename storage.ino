/*
Speeduino - Simple engine management for the Arduino Mega 2560 platform
Copyright (C) Josh Stewart
A full copy of the license may be found in the projects root directory
*/

#include <EEPROM.h>
#include "storage.h"
#include "globals.h"
//#include "table.h"


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
  
  if(EEPROMRead(0) != data_structure_version) { EEPROMWrite(0,data_structure_version); }   //Write the data structure version
  
  
  /*---------------------------------------------------
  | Fuel table (See storage.h for data layout) - Page 1
  | 16x16 table itself + the 16 values along each of the axis 
  -----------------------------------------------------*/
  if(EEPROMRead(EEPROM_CONFIG1_XSIZE) != fuelTable.xSize) { EEPROMWrite(EEPROM_CONFIG1_XSIZE, fuelTable.xSize); } //Write the VE Tables RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG1_YSIZE) != fuelTable.ySize) { EEPROMWrite(EEPROM_CONFIG1_YSIZE, fuelTable.ySize); } //Write the VE Tables MAP/TPS dimension size
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG1_MAP;
    if(EEPROMRead(x) != fuelTable.values[15-offset/16][offset%16]) { EEPROMWrite(x, fuelTable.values[15-offset/16][offset%16]); }  //Write the 16x16 map
  }
  
  //RPM bins
  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    if(EEPROMRead(x) != byte(fuelTable.axisX[offset]/100)) { EEPROMWrite(x, byte(fuelTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    if(EEPROMRead(x) != fuelTable.axisY[offset]) { EEPROMWrite(x, fuelTable.axisY[offset]); }
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
    if(EEPROMRead(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))) { EEPROMWrite(x, *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))); }
  }
  //*********************************************************************************************************************************************************************************

  
  /*---------------------------------------------------
  | Ignition table (See storage.h for data layout) - Page 1
  | 16x16 table itself + the 16 values along each of the axis 
  -----------------------------------------------------*/
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROMRead(EEPROM_CONFIG3_XSIZE) != ignitionTable.xSize) { EEPROMWrite(EEPROM_CONFIG3_XSIZE,ignitionTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG3_YSIZE) != ignitionTable.ySize) { EEPROMWrite(EEPROM_CONFIG3_YSIZE,ignitionTable.ySize); } //Write the ignition Table MAP/TPS dimension size
  
  for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG3_MAP;
    if(EEPROMRead(x) != ignitionTable.values[15-offset/16][offset%16]) { EEPROMWrite(x, ignitionTable.values[15-offset/16][offset%16]); }  //Write the 16x16 map with translation
  }
  //RPM bins
  for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG3_XBINS;
    if(EEPROMRead(x) != byte(ignitionTable.axisX[offset]/100)) { EEPROMWrite(x, byte(ignitionTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++) 
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    if(EEPROMRead(x) != ignitionTable.axisY[offset]) { EEPROMWrite(x, ignitionTable.axisY[offset]); }
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
    if(EEPROMRead(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))) { EEPROMWrite(x, *(pnt_configPage + byte(x - EEPROM_CONFIG4_START))); }
  }
  //*********************************************************************************************************************************************************************************

  
  /*---------------------------------------------------
  | AFR table (See storage.h for data layout) - Page 5
  | 16x16 table itself + the 16 values along each of the axis 
  -----------------------------------------------------*/
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROMRead(EEPROM_CONFIG5_XSIZE) != afrTable.xSize) { EEPROMWrite(EEPROM_CONFIG5_XSIZE,afrTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG5_YSIZE) != afrTable.ySize) { EEPROMWrite(EEPROM_CONFIG5_YSIZE,afrTable.ySize); } //Write the ignition Table MAP/TPS dimension size
  
  for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG5_MAP;
    if(EEPROMRead(x) != afrTable.values[15-offset/16][offset%16]) { EEPROMWrite(x, afrTable.values[15-offset/16][offset%16]); }  //Write the 16x16 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG5_XBINS;
    if(EEPROMRead(x) != byte(afrTable.axisX[offset]/100)) { EEPROMWrite(x, byte(afrTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++) 
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    if(EEPROMRead(x) != afrTable.axisY[offset]) { EEPROMWrite(x, afrTable.axisY[offset]); }
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
    if(EEPROMRead(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))) { EEPROMWrite(x, *(pnt_configPage + byte(x - EEPROM_CONFIG6_START))); }
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
    if(EEPROMRead(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG7_START))) { EEPROMWrite(x, *(pnt_configPage + byte(x - EEPROM_CONFIG7_START))); }
  }
  
  /*---------------------------------------------------
  | Boost, vvt and E85 tables (See storage.h for data layout) - Page 8
  | 8x8 table itself + the 8 values along each of the axis //boost and vvt
  | 4x4 tables for E85
  -----------------------------------------------------*/
  //Begin writing the 4 tables, basically the same thing as above but we're doing these 4 together (4 tables per page instead of 1)
  if(EEPROMRead(EEPROM_CONFIG8_XSIZE1) != boostTable.xSize) { EEPROMWrite(EEPROM_CONFIG8_XSIZE1,boostTable.xSize); } //Write the boost Table RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG8_YSIZE1) != boostTable.ySize) { EEPROMWrite(EEPROM_CONFIG8_YSIZE1,boostTable.ySize); } //Write the boost Table MAP/TPS dimension size
  if(EEPROMRead(EEPROM_CONFIG8_XSIZE2) != vvtTable.xSize) { EEPROMWrite(EEPROM_CONFIG8_XSIZE2,vvtTable.xSize); } //Write the vvt Table RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG8_YSIZE2) != vvtTable.ySize) { EEPROMWrite(EEPROM_CONFIG8_YSIZE2,vvtTable.ySize); } //Write the vvt Table MAP/TPS dimension size
  if(EEPROMRead(EEPROM_CONFIG8_XSIZE3) != E85TableINJ.xSize) { EEPROMWrite(EEPROM_CONFIG8_XSIZE3,E85TableINJ.xSize); } //Write the e85inj Table RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG8_YSIZE3) != E85TableINJ.ySize) { EEPROMWrite(EEPROM_CONFIG8_YSIZE3,E85TableINJ.ySize); } //Write the e85inj Table MAP dimension size
  if(EEPROMRead(EEPROM_CONFIG8_XSIZE4) != E85TableIGN.xSize) { EEPROMWrite(EEPROM_CONFIG8_XSIZE4,E85TableIGN.xSize); } //Write the e85ign Table RPM dimension size
  if(EEPROMRead(EEPROM_CONFIG8_YSIZE4) != E85TableIGN.ySize) { EEPROMWrite(EEPROM_CONFIG8_YSIZE4,E85TableIGN.ySize); } //Write the e85ign Table MAP dimension size
  
  int y = EEPROM_CONFIG8_MAP2; //We do boost and vvt maps together in the same loop
  for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++) 
  { 
    offset = x - EEPROM_CONFIG8_MAP1;
    if(EEPROMRead(x) != boostTable.values[7-offset/8][offset%8]) { EEPROMWrite(x, boostTable.values[7-offset/8][offset%8]); }  //Write the 8x8 map
    offset = y - EEPROM_CONFIG8_MAP2;
    if(EEPROMRead(y) != vvtTable.values[7-offset/8][offset%8]) { EEPROMWrite(y, vvtTable.values[7-offset/8][offset%8]); }  //Write the 8x8 map
    y++;
  }
  y = EEPROM_CONFIG8_MAP4; //Do e85 ign and inj maps together in the same loop
  for(int x=EEPROM_CONFIG8_MAP3; x<EEPROM_CONFIG8_XBINS3; x++) 
  { 
    offset = x - EEPROM_CONFIG8_MAP3;
    if(EEPROMRead(x) != E85TableINJ.values[3-offset/4][offset%4]) { EEPROMWrite(x, E85TableINJ.values[3-offset/4][offset%4]); }  //Write the 4x4 map
    offset = y - EEPROM_CONFIG8_MAP4;
    if(EEPROMRead(y) != E85TableIGN.values[3-offset/4][offset%4]) { EEPROMWrite(y, E85TableIGN.values[3-offset/4][offset%4]); }  //Write the 4x4 map
    y++;
  }

  //RPM bins boost and vvt
  y = EEPROM_CONFIG8_XBINS2;
  for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++) 
  {
    offset = x - EEPROM_CONFIG8_XBINS1;
    if(EEPROMRead(x) != byte(boostTable.axisX[offset]/100)) { EEPROMWrite(x, byte(boostTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
    offset = y - EEPROM_CONFIG8_XBINS2;
    if(EEPROMRead(y) != byte(vvtTable.axisX[offset]/100)) { EEPROMWrite(y, byte(vvtTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
    y++;
  }
  //TPS/MAP bins boost and vvt
  y=EEPROM_CONFIG8_YBINS2;
  for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++) 
  {
    offset = x - EEPROM_CONFIG8_YBINS1;
    if(EEPROMRead(x) != boostTable.axisY[offset]) { EEPROMWrite(x, boostTable.axisY[offset]); }
    offset = y - EEPROM_CONFIG8_YBINS2;
    if(EEPROMRead(y) != vvtTable.axisY[offset]) { EEPROMWrite(y, vvtTable.axisY[offset]); }
    y++;
  }

  //RPM bins E85
  y = EEPROM_CONFIG8_XBINS4;
  for(int x=EEPROM_CONFIG8_XBINS3; x<EEPROM_CONFIG8_YBINS3; x++) 
  {
    offset = x - EEPROM_CONFIG8_XBINS3;
    if(EEPROMRead(x) != byte(E85TableINJ.axisX[offset]/100)) { EEPROMWrite(x, byte(E85TableINJ.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
    offset = y - EEPROM_CONFIG8_XBINS4;
    if(EEPROMRead(y) != byte(E85TableIGN.axisX[offset]/100)) { EEPROMWrite(y, byte(E85TableIGN.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
    y++;
  }
  //TPS/MAP bins E85
  y=EEPROM_CONFIG8_YBINS4;
  for(int x=EEPROM_CONFIG8_YBINS3; x<EEPROM_CONFIG8_XSIZE4; x++) 
  {
    offset = x - EEPROM_CONFIG8_YBINS3;
    if(EEPROMRead(x) != E85TableINJ.axisY[offset]) { EEPROMWrite(x, E85TableINJ.axisY[offset]); }
    offset = y - EEPROM_CONFIG8_YBINS4;
    if(EEPROMRead(y) != E85TableIGN.axisY[offset]) { EEPROMWrite(y, E85TableIGN.axisY[offset]); }
    y++;
  }
}

void loadConfig()
{
  int offset;
  //Create a pointer to the config page
  byte* pnt_configPage;
  
  
  //Fuel table (See storage.h for data layout)
  //fuelTable.xSize = EEPROMRead(EEPROM_CONFIG1_XSIZE); //Read the VE Tables RPM dimension size
  //fuelTable.ySize = EEPROMRead(EEPROM_CONFIG1_YSIZE); //Read the VE Tables MAP/TPS dimension size
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG1_MAP;
    fuelTable.values[15-offset/16][offset%16] = EEPROMRead(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    fuelTable.axisX[offset] = (EEPROMRead(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    fuelTable.axisY[offset] = EEPROMRead(x);
  }
  
  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
  for(int x=EEPROM_CONFIG2_START; x<EEPROM_CONFIG2_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG2_START)) = EEPROMRead(x);
  }
  //That concludes the reading of the VE table
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)

  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROMRead(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROMRead(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)
  
  for(int x=EEPROM_CONFIG3_MAP; x<EEPROM_CONFIG3_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG3_MAP;
    ignitionTable.values[15-offset/16][offset%16] = EEPROMRead(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG3_XBINS; x<EEPROM_CONFIG3_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG3_XBINS;
    ignitionTable.axisX[offset] = (EEPROMRead(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++) 
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    ignitionTable.axisY[offset] = EEPROMRead(x);
  }
  
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG4_START; x<EEPROM_CONFIG4_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG4_START)) = EEPROMRead(x);
  }
  
  //*********************************************************************************************************************************************************************************
  //AFR TARGET CONFIG PAGE (3)
  
  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROMRead(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROMRead(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)
  
  for(int x=EEPROM_CONFIG5_MAP; x<EEPROM_CONFIG5_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG5_MAP;
    afrTable.values[15-offset/16][offset%16] = EEPROMRead(x); //Read the 16x16 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG5_XBINS;
    afrTable.axisX[offset] = (EEPROMRead(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++) 
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    afrTable.axisY[offset] = EEPROMRead(x);
  }
  
  pnt_configPage = (byte *)&configPage3; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG6_START; x<EEPROM_CONFIG6_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG6_START)) = EEPROMRead(x);
  }
 
  //*********************************************************************************************************************************************************************************
 
  //CONFIG PAGE (4)                                                                                                                                           //############
  pnt_configPage = (byte *)&configPage4; //Create a pointer to Page 3 in memory
  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROMRead(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROMRead(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)
  
  //The next 64 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG7_START; x<EEPROM_CONFIG7_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG7_START)) = EEPROMRead(x);
  }
  
  //*********************************************************************************************************************************************************************************
  // Boost, vvt and E85 Trim tables load
  int y = EEPROM_CONFIG8_MAP2;
  for(int x=EEPROM_CONFIG8_MAP1; x<EEPROM_CONFIG8_XBINS1; x++) 
  { 
    offset = x - EEPROM_CONFIG8_MAP1;
    boostTable.values[7-offset/8][offset%8] = EEPROMRead(x); //Read the 8x8 map
    offset = y - EEPROM_CONFIG8_MAP2;
    vvtTable.values[7-offset/8][offset%8] = EEPROMRead(y); //Read the 8x8 map
    y++;
  }

  //E85
  y = EEPROM_CONFIG8_MAP4;
  for(int x=EEPROM_CONFIG8_MAP3; x<EEPROM_CONFIG8_XBINS3; x++) 
  { 
    offset = x - EEPROM_CONFIG8_MAP3;
    E85TableINJ.values[3-offset/4][offset%4] = EEPROMRead(x); //Read the 4x4 map
    offset = y - EEPROM_CONFIG8_MAP4;
    E85TableIGN.values[3-offset/4][offset%4] = EEPROMRead(y); //Read the 4x4 map
    y++;
  }

  //RPM bins boost, vvt
  y = EEPROM_CONFIG8_XBINS2;
  for(int x=EEPROM_CONFIG8_XBINS1; x<EEPROM_CONFIG8_YBINS1; x++) 
  {
    offset = x - EEPROM_CONFIG8_XBINS1;
    boostTable.axisX[offset] = (EEPROMRead(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = y - EEPROM_CONFIG8_XBINS2;
    vvtTable.axisX[offset] = (EEPROMRead(y) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
    y++;
  }
    
  //TPS/MAP bins boost vvt
  y = EEPROM_CONFIG8_YBINS2;
  for(int x=EEPROM_CONFIG8_YBINS1; x<EEPROM_CONFIG8_XSIZE2; x++) 
  {
    offset = x - EEPROM_CONFIG8_YBINS1;
    boostTable.axisY[offset] = EEPROMRead(x);
    offset = y - EEPROM_CONFIG8_YBINS2;
    vvtTable.axisY[offset] = EEPROMRead(y);
    y++;
  }

  //RPM bins e85
  y = EEPROM_CONFIG8_XBINS4;
  for(int x=EEPROM_CONFIG8_XBINS3; x<EEPROM_CONFIG8_YBINS3; x++) 
  {
    offset = x - EEPROM_CONFIG8_XBINS3;
    E85TableINJ.axisX[offset] = (EEPROMRead(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
    offset = y - EEPROM_CONFIG8_XBINS4;
    E85TableIGN.axisX[offset] = (EEPROMRead(y) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
    y++;
  }
    
  //TPS/MAP bins e85
  y = EEPROM_CONFIG8_YBINS4;
  for(int x=EEPROM_CONFIG8_YBINS3; x<EEPROM_CONFIG8_XSIZE4; x++) 
  {
    offset = x - EEPROM_CONFIG8_YBINS3;
    E85TableINJ.axisY[offset] = EEPROMRead(x);
    offset = y - EEPROM_CONFIG8_YBINS4;
    E85TableIGN.axisY[offset] = EEPROMRead(y);
    y++;
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
    cltCalibrationTable[x] = EEPROMRead(y);
    
    y = EEPROM_CALIBRATION_IAT + x;
    iatCalibrationTable[x] = EEPROMRead(y);
    
    y = EEPROM_CALIBRATION_O2 + x;
    o2CalibrationTable[x] = EEPROMRead(y);
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
    if(EEPROMRead(y) != cltCalibrationTable[x]) { EEPROMWrite(y, cltCalibrationTable[x]); }
    
    y = EEPROM_CALIBRATION_IAT + x;
    if(EEPROMRead(y) != iatCalibrationTable[x]) { EEPROMWrite(y, iatCalibrationTable[x]); }
    
    y = EEPROM_CALIBRATION_O2 + x;
    if(EEPROMRead(y) != o2CalibrationTable[x]) { EEPROMWrite(y, o2CalibrationTable[x]); }
  }  
}

//Read from EEPROM either internal (Atmega) or external (MK20)
byte EEPROMRead(int addr){
  #if defined (__MK20DX256__)
    byte data = 0xFF;  
    Wire.beginTransmission(EEPROM_ADDR);  
    Wire.send((int)(addr >> 8));   // MSB  
    Wire.send((int)(addr & 0xFF)); // LSB  
    Wire.endTransmission();  
    Wire.requestFrom(EEPROM_ADDR,1); 
    if (Wire.available()) data = Wire.receive();   
    return data;
  #else
    return EEPROM.read(addr);
  #endif
}

//Write to EEPROM either internal (Atmega) or external (MK20)
void EEPROMWrite(int addr, int val){
  #if defined (__MK20DX256__)
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.send((int)(addr >> 8));   // MSB
    Wire.send((int)(addr & 0xFF)); // LSB
    Wire.send(val);
    Wire.endTransmission();
  #else
    EEPROM.write(addr, val);
  #endif
}