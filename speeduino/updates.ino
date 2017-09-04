/*
 * This routine is used for doing any data conversions that are required during firmware changes
 * This prevents users getting difference reports in TS when such a data change occurs.
 * It also can be used for setting good values when there are viarables that move locations in the ini
 * When a user skips multiple firmware versions at a time, this will roll through the updates 1 at a time
 */


void doUpdates()
{
  #define CURRENT_DATA_VERSION    5
  uint8_t currentVersion = EEPROM.read(EEPROM_DATA_VERSION);
  //May 2017 firmware introduced a -40 offset on the ignition table. Update that table to +40
  if(currentVersion == 2)
  {
    for(int x=0; x<16; x++)
    {
      for(int y=0; y<16; y++)
      {
        ignitionTable.values[x][y] = ignitionTable.values[x][y] + 40;
      }
    }
    writeConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 3);
  }
  //June 2017 required the forced addition of some CAN values to avoid weird errors
  if(currentVersion == 3)
  {
    configPage10.speeduino_tsCanId = 0;
    configPage10.true_address = 256;
    configPage10.realtime_base_address = 336;

    //There was a bad value in the May base tune for the spark duration setting, fix it here if it's a problem
    if(configPage2.sparkDur == 255) { configPage2.sparkDur = 10; }

    writeConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 4);
  }
  //July 2017 adds a cranking enrichment curve in place of the single value. This converts that single value to the curve
  if(currentVersion == 4)
  {
    //Some default values for the bins (Doesn't matter too much here as the values against them will all be identical)
    configPage11.crankingEnrichBins[0] = 0;
    configPage11.crankingEnrichBins[1] = 40;
    configPage11.crankingEnrichBins[2] = 70;
    configPage11.crankingEnrichBins[3] = 100;

    configPage11.crankingEnrichValues[0] = 100 + configPage1.crankingPct;
    configPage11.crankingEnrichValues[1] = 100 + configPage1.crankingPct;
    configPage11.crankingEnrichValues[2] = 100 + configPage1.crankingPct;
    configPage11.crankingEnrichValues[3] = 100 + configPage1.crankingPct;

    writeConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 5);
  }

  //Final check is always for 255 and 0 (Brand new arduino)
  if( (currentVersion == 0) || (currentVersion == 255) )
  {
    configPage10.true_address = 0x200;
    EEPROM.write(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION);

  }
}
