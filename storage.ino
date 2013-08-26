#include <EEPROM.h>

/*
Takes the current configuration (config pages and maps)
and writes the to EEPROM in the layout defined in storage.h
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
    if(EEPROM.read(x) != (byte)(fuelTable.axisX[offset]/100)) { EEPROM.write(x, (byte)(fuelTable.axisX[offset]/100)); } //RPM bins are divided by 100 and converted to a byte
  }
  //TPS/MAP bins
  for(int x=EEPROM_CONFIG1_YBINS; x<EEPROM_CONFIG1_SETTINGS; x++) 
  {
    offset = x - EEPROM_CONFIG1_YBINS;
    if(EEPROM.read(x) != fuelTable.axisY[offset]) { EEPROM.write(x, fuelTable.axisY[offset]); }
  }
  //The next 125 bytes can simply be pulled straight from the configTable
  for(int x=EEPROM_CONFIG1_SETTINGS; x<EEPROM_CONFIG2_XSIZE; x++) 
  { 
    if(EEPROM.read(x) != *(pnt_configPage - EEPROM_CONFIG1_SETTINGS + x)) { EEPROM.write(x, *(pnt_configPage - EEPROM_CONFIG1_SETTINGS + x)); }
  }
  //That concludes the writing of the VE table
  
  //Begin writing the Ignition table, basically the same thing as above
  if(EEPROM.read(EEPROM_CONFIG2_XSIZE) != ignitionTable.xSize) { EEPROM.write(EEPROM_CONFIG2_XSIZE,ignitionTable.xSize); } //Write the ignition Table RPM dimension size
  if(EEPROM.read(EEPROM_CONFIG2_YSIZE) != ignitionTable.ySize) { EEPROM.write(EEPROM_CONFIG2_YSIZE,ignitionTable.ySize); } //Write the ignition Table MAP/TPS dimension size
  
  //The next 125 bytes can simply be pulled straight from the ignitionTable
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG2_YBINS; x<EEPROM_SIZE; x++) 
  { 
    if(EEPROM.read(x) != *(pnt_configPage + x)) { EEPROM.write(x, *(pnt_configPage + x)); }
  }
  
}

void loadConfig()
{
  byte* pnt_configPage;
  //The next 125 bytes can simply be pulled straight from the fuelTable
  pnt_configPage = (byte *)&configPage1; //Create a pointer to Page 1 in memory
  for(int x=EEPROM_CONFIG1_MAP; x<EEPROM_CONFIG2_XSIZE; x++) 
  { 
    *(pnt_configPage + x) = EEPROM.read(x);
  }
  //That concludes the writing of the VE table
  
  //The next 125 bytes can simply be pulled straight from the ignitionTable
  pnt_configPage = (byte *)&configPage2; //Create a pointer to Page 2 in memory
  for(int x=EEPROM_CONFIG2_MAP; x<EEPROM_SIZE; x++) 
  { 
     *(pnt_configPage + x) = EEPROM.read(x);
  }
  
}
