#include <EEPROM.h>
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
  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
  
  if(EEPROM.read(0) != data_structure_version) { EEPROM.write(0,data_structure_version); }   //Write the data structure version
  
  //Fuel table (See storage.h for data layout)
  if(EEPROM.read(EEPROM_CONFIG1_XSIZE) != fuelTable.xSize) { EEPROM.write(EEPROM_CONFIG1_XSIZE, fuelTable.xSize); } //Write the VE Tables RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG1_YSIZE) != fuelTable.ySize) { EEPROM.write(EEPROM_CONFIG1_YSIZE, fuelTable.ySize); } //Write the VE Tables MAP/TPS dimension size
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG1_MAP;
    if(EEPROM.read(x) != fuelTable.values[7-offset/8][offset%8]) { EEPROM.write(x, fuelTable.values[7-offset/8][offset%8]); }  //Write the 8x8 map
  }
  
  //RPM bins
  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    if(EEPROM.read(x) != byte(fuelTable.axisX[offset]/100)) { EEPROM.write(x, byte(fuelTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG1_SETTINGS; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    if(EEPROM.read(x) != fuelTable.axisY[offset]) { EEPROM.write(x, fuelTable.axisY[offset]); }
  }
  //The next 45 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG1_SETTINGS; x<EEPROM_CONFIG1_END; x++) 
  { 
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG1_SETTINGS))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG1_SETTINGS))); }
  }
  //That concludes the writing of the VE table
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROM.read(EEPROM_CONFIG2_XSIZE) != ignitionTable.xSize) { EEPROM.write(EEPROM_CONFIG2_XSIZE,ignitionTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG2_YSIZE) != ignitionTable.ySize) { EEPROM.write(EEPROM_CONFIG2_YSIZE,ignitionTable.ySize); } //Write the ignition Table MAP/TPS dimension size
  
  for(int x=EEPROM_CONFIG2_MAP; x<EEPROM_CONFIG2_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG2_MAP;
    if(EEPROM.read(x) != ignitionTable.values[7-offset/8][offset%8]) { EEPROM.write(x, ignitionTable.values[7-offset/8][offset%8]); }  //Write the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG2_XBINS; x<EEPROM_CONFIG2_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG2_XBINS;
    if(EEPROM.read(x) != byte(ignitionTable.axisX[offset]/100)) { EEPROM.write(x, byte(ignitionTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG2_YBINS; x<EEPROM_CONFIG2_SETTINGS; x++) 
  {
    offset = x - EEPROM_CONFIG2_YBINS;
    if(EEPROM.read(x) != ignitionTable.axisY[offset]) { EEPROM.write(x, ignitionTable.axisY[offset]); }
  }
  //The next 45 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG2_SETTINGS; x<EEPROM_CONFIG_END; x++) 
  { 
    if(EEPROM.read(x) != *(pnt_configPage + byte(x - EEPROM_CONFIG2_SETTINGS))) { EEPROM.write(x, *(pnt_configPage + byte(x - EEPROM_CONFIG2_SETTINGS))); }
  }
  
}

void loadConfig()
{
  int offset;
  //Create a pointer to the config page
  byte* pnt_configPage;
  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
  
  //Fuel table (See storage.h for data layout)
  //fuelTable.xSize = EEPROM.read(EEPROM_CONFIG1_XSIZE); //Read the VE Tables RPM dimension size
  //fuelTable.ySize = EEPROM.read(EEPROM_CONFIG1_YSIZE); //Read the VE Tables MAP/TPS dimension size
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG1_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG1_MAP;
    fuelTable.values[7-offset/8][offset%8] = EEPROM.read(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG1_XBINS; x<EEPROM_CONFIG1_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG1_XBINS;
    fuelTable.axisX[offset] = (EEPROM.read(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG1_SETTINGS; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    fuelTable.axisY[offset] = EEPROM.read(x);
  }
  //The next 45 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG1_SETTINGS; x<EEPROM_CONFIG1_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG1_SETTINGS)) = EEPROM.read(x);
  }
  //That concludes the reading of the VE table
  
  //*********************************************************************************************************************************************************************************
  //IGNITION CONFIG PAGE (2)
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  //Begin writing the Ignition table, basically the same thing as above
  //ignitionTable.xSize = EEPROM.read(EEPROM_CONFIG2_XSIZE); //Read the ignition Table RPM dimension size (Currently not supproted)
  //ignitionTable.ySize = EEPROM.read(EEPROM_CONFIG2_YSIZE); //Read the ignition Table MAP/TPS dimension size (Currently not supproted)
  
  for(int x=EEPROM_CONFIG2_MAP; x<EEPROM_CONFIG2_XBINS; x++) 
  { 
    offset = x - EEPROM_CONFIG2_MAP;
    ignitionTable.values[7-offset/8][offset%8] = EEPROM.read(x); //Read the 8x8 map
  }
  //RPM bins
  for(int x=EEPROM_CONFIG2_XBINS; x<EEPROM_CONFIG2_YBINS; x++) 
  {
    offset = x - EEPROM_CONFIG2_XBINS;
    ignitionTable.axisX[offset] = (EEPROM.read(x) * 100); //RPM bins are divided by 100 when stored. Multiply them back now
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG2_YBINS; x<EEPROM_CONFIG2_SETTINGS; x++) 
  {
    offset = x - EEPROM_CONFIG2_YBINS;
    ignitionTable.axisY[offset] = EEPROM.read(x);
  }
  //The next 45 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG2_SETTINGS; x<EEPROM_CONFIG_END; x++) 
  { 
    *(pnt_configPage + byte(x - EEPROM_CONFIG2_SETTINGS)) = EEPROM.read(x);
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

TODO: Need to add checking prior to write in order to prolong EEPROM life
*/
void writeCalibration()
{
  
  for(int x=0; x<CALIBRATION_TABLE_SIZE; x++) //Each calibration table is 512 bytes long
  {
    int y = EEPROM_CALIBRATION_CLT + x;
    EEPROM.write(y, cltCalibrationTable[x]);
    
    y = EEPROM_CALIBRATION_IAT + x;
    EEPROM.write(y, iatCalibrationTable[x]);
    
    y = EEPROM_CALIBRATION_O2 + x;
    EEPROM.write(y, o2CalibrationTable[x]);
  }
  
}
