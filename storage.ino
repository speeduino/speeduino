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
  //Set the table sizes to 3 (This allocates memory for the arrays as well)
  table2D_setSize(&cltCalibrationTable, 3);
  table2D_setSize(&iatCalibrationTable, 3);
  table2D_setSize(&o2CalibrationTable, 3);
  
  for(byte x=0; x<3; x++)
  {
    int y = EEPROM_CALIBRATION_CLT + (4*x);
    cltCalibrationTable.axisX16[x] = int(word(EEPROM.read(y), EEPROM.read(y+1)));
    cltCalibrationTable.values16[x] = int(word(EEPROM.read(y+2), EEPROM.read(y+3)));
    
    y = EEPROM_CALIBRATION_IAT + (4*x);
    iatCalibrationTable.axisX16[x] = int(word(EEPROM.read(y), EEPROM.read(y+1)));
    iatCalibrationTable.values16[x] = int(word(EEPROM.read(y+2), EEPROM.read(y+3)));
    
    y = EEPROM_CALIBRATION_O2 + (4*x);
    o2CalibrationTable.axisX16[x] = int(word(EEPROM.read(y), EEPROM.read(y+1)));
    o2CalibrationTable.values16[x] = int(word(EEPROM.read(y+2), EEPROM.read(y+3)));
  }
  
}

void writeCalibration()
{
  
  for(byte x=0; x<3; x++)
  {
    int y = EEPROM_CALIBRATION_CLT + (4*x);
    EEPROM.write(y, highByte(cltCalibrationTable.axisX16[x]));
    EEPROM.write(y+1, lowByte(cltCalibrationTable.axisX16[x]));
    EEPROM.write(y+2, highByte(cltCalibrationTable.values16[x]));
    EEPROM.write(y+3, lowByte(cltCalibrationTable.values16[x]));
    
    y = EEPROM_CALIBRATION_IAT + (4*x);
    EEPROM.write(y, highByte(iatCalibrationTable.axisX16[x]));
    EEPROM.write(y+1, lowByte(iatCalibrationTable.axisX16[x]));
    EEPROM.write(y+2, highByte(iatCalibrationTable.values16[x]));
    EEPROM.write(y+3, lowByte(iatCalibrationTable.values16[x]));
    
    y = EEPROM_CALIBRATION_O2 + (4*x);
    EEPROM.write(y, highByte(o2CalibrationTable.axisX16[x]));
    EEPROM.write(y+1, lowByte(o2CalibrationTable.axisX16[x]));
    EEPROM.write(y+2, highByte(o2CalibrationTable.values16[x]));
    EEPROM.write(y+3, lowByte(o2CalibrationTable.values16[x]));
  }
  
}
