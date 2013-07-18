#include <EEPROM.h>

void writeConfig()
{
  /*
  We only ever write to the EEPROM where the new value is different from the currently stored byte
  This is due to the limited write life of the EEPROM (Approximately 100,000 writes)
  */
  
  if(EEPROM.read(0) != data_structure_version) { EEPROM.write(0,data_structure_version); }   //Write the data structure version
  if(EEPROM.read(EEPROM_CONFIG1_XSIZE) != fuelTable.xSize) { EEPROM.write(EEPROM_CONFIG1_XSIZE, fuelTable.xSize); } //Write the VE Tables RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG1_YSIZE) != fuelTable.ySize) { EEPROM.write(EEPROM_CONFIG1_YSIZE, fuelTable.ySize); } //Write the VE Tables MAP/TPS dimension size
  
  byte* pnt_configPage;
  //The next 125 bytes can simply be pulled straight from the fuelTable
  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG2_XSIZE; x++) 
  { 
    if(EEPROM.read(x) != *(pnt_configPage + x)) { EEPROM.write(x, *(pnt_configPage + x)); }
  }
  //That concludes the writing of the VE table
  
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROM.read(EEPROM_CONFIG2_XSIZE) != ignitionTable.xSize) { EEPROM.write(EEPROM_CONFIG2_XSIZE,ignitionTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG2_YSIZE) != ignitionTable.ySize) { EEPROM.write(EEPROM_CONFIG2_YSIZE,ignitionTable.ySize); } //Write the ignition Table MAP/TPS dimension size
  
  //The next 125 bytes can simply be pulled straight from the ignitionTable
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG2_MAP; x<EEPROM_SIZE; x++) 
  { 
    if(EEPROM.read(x) != *(pnt_configPage + x)) { EEPROM.write(x, *(pnt_configPage + x)); }
  }
  
}
void writeTables();

void loadConfig()
{
     
}

void loadTables();
