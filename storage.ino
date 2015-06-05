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
    if(EEPROM.read(x) != byte(fuelTable.axisX[offset]/100)) { EEPROM.write(x, byte(fuelTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    if(EEPROM.read(x) != fuelTable.axisY[offset]) { EEPROM.write(x, fuelTable.axisY[offset]); }
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
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_START))) { EEPROM.write(x, *(pnt_configPage + x - EEPROM_CONFIG2_START)); }
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
    if(EEPROM.read(x) != byte(ignitionTable.axisX[offset]/100)) { EEPROM.write(x, byte(ignitionTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++) 
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    if(EEPROM.read(x) != ignitionTable.axisY[offset]) { EEPROM.write(x, ignitionTable.axisY[offset]); }
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
    if(EEPROM.read(x) != byte(afrTable.axisX[offset]/100)) { EEPROM.write(x, byte(afrTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++) 
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    if(EEPROM.read(x) != afrTable.axisY[offset]) { EEPROM.write(x, afrTable.axisY[offset]); }
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
    fuelTable.axisX[offset] = (EEPROM.read(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG2_START; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    fuelTable.axisY[offset] = EEPROM.read(x);
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
    ignitionTable.axisX[offset] = (EEPROM.read(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG3_YBINS; x<EEPROM_CONFIG4_START; x++) 
  {
    offset = x - EEPROM_CONFIG3_YBINS;
    ignitionTable.axisY[offset] = EEPROM.read(x);
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
    afrTable.values[15-offset/16][offset%16] = EEPROM.read(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG5_XBINS; x<EEPROM_CONFIG5_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG5_XBINS;
    afrTable.axisX[offset] = (EEPROM.read(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG5_YBINS; x<EEPROM_CONFIG6_START; x++) 
  {
    offset = x - EEPROM_CONFIG5_YBINS;
    afrTable.axisY[offset] = EEPROM.read(x);
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
  
  //The next 45 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG7_START; x<EEPROM_CONFIG7_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG7_START)) = EEPROM.read(x);
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
