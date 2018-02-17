/*
 * This routine is used for doing any data conversions that are required during firmware changes
 * This prevents users getting difference reports in TS when such a data change occurs.
 * It also can be used for setting good values when there are viarables that move locations in the ini
 * When a user skips multiple firmware versions at a time, this will roll through the updates 1 at a time
 */


void doUpdates()
{
  #define CURRENT_DATA_VERSION    8

  //May 2017 firmware introduced a -40 offset on the ignition table. Update that table to +40
  if(EEPROM.read(EEPROM_DATA_VERSION) == 2)
  {
    for(int x=0; x<16; x++)
    {
      for(int y=0; y<16; y++)
      {
        ignitionTable.values[x][y] = ignitionTable.values[x][y] + 40;
      }
    }
    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 3);
  }
  //June 2017 required the forced addition of some CAN values to avoid weird errors
  if(EEPROM.read(EEPROM_DATA_VERSION) == 3)
  {
    configPage9.speeduino_tsCanId = 0;
    configPage9.true_address = 256;
    configPage9.realtime_base_address = 336;

    //There was a bad value in the May base tune for the spark duration setting, fix it here if it's a problem
    if(configPage4.sparkDur == 255) { configPage4.sparkDur = 10; }

    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 4);
  }
  //July 2017 adds a cranking enrichment curve in place of the single value. This converts that single value to the curve
  if(EEPROM.read(EEPROM_DATA_VERSION) == 4)
  {
    //Some default values for the bins (Doesn't matter too much here as the values against them will all be identical)
    configPage10.crankingEnrichBins[0] = 0;
    configPage10.crankingEnrichBins[1] = 40;
    configPage10.crankingEnrichBins[2] = 70;
    configPage10.crankingEnrichBins[3] = 100;

    configPage10.crankingEnrichValues[0] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[1] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[2] = 100 + configPage2.crankingPct;
    configPage10.crankingEnrichValues[3] = 100 + configPage2.crankingPct;

    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 5);
  }
  //September 2017 had a major change to increase the minimum table size to 128. This required multiple pieces of data being moved around
  if(EEPROM.read(EEPROM_DATA_VERSION) == 5)
  {
    //Data after page 4 has to move back 128 bytes
    for(int x=0; x < 1152; x++)
    {
      int endMem = EEPROM_CONFIG10_END - x;
      int startMem = endMem - 128; //
      byte currentVal = EEPROM.read(startMem);
      EEPROM.update(endMem, currentVal);
    }
    //The remaining data only has to move back 64 bytes
    for(int x=0; x < 352; x++)
    {
      int endMem = EEPROM_CONFIG10_END - 1152 - x;
      int startMem = endMem - 64; //
      byte currentVal = EEPROM.read(startMem);
      EEPROM.update(endMem, currentVal);
    }

    EEPROM.write(EEPROM_DATA_VERSION, 6);
    loadConfig(); //Reload the config after changing everything in EEPROM
  }
  //November 2017 added the staging table that comes after boost and vvt in the EEPROM. This required multiple pieces of data being moved around
  if(EEPROM.read(EEPROM_DATA_VERSION) == 6)
  {
    //Data after page 8 has to move back 82 bytes
    for(int x=0; x < 529; x++)
    {
      int endMem = EEPROM_CONFIG10_END - x;
      int startMem = endMem - 82; //
      byte currentVal = EEPROM.read(startMem);
      EEPROM.update(endMem, currentVal);
    }

    EEPROM.write(EEPROM_DATA_VERSION, 7);
    loadConfig(); //Reload the config after changing everything in EEPROM
  }

  if (EEPROM.read(EEPROM_DATA_VERSION) == 7) {
    //Convert whatever flex fuel settings are there into the new tables

    configPage10.flexBoostBins[0] = 0;
    configPage10.flexBoostAdj[0]  = (int8_t)configPage2.unused2_1;

    configPage10.flexFuelBins[0] = 0;
    configPage10.flexFuelAdj[0]  = configPage2.unused2_57;

    configPage10.flexAdvBins[0] = 0;
    configPage10.flexAdvAdj[0]  = configPage2.unused2_59;

    for (uint8_t x = 1; x < 6; x++)
    {
      uint8_t pct = x * 20;
      configPage10.flexBoostBins[x] = pct;
      configPage10.flexFuelBins[x] = pct;
      configPage10.flexAdvBins[x] = pct;

      int16_t boostAdder = (((configPage2.unused2_2 - (int8_t)configPage2.unused2_1) * pct) / 100) + (int8_t)configPage2.unused2_1;
      configPage10.flexBoostAdj[x] = boostAdder;

      uint8_t fuelAdder = (((configPage2.unused2_58 - configPage2.unused2_57) * pct) / 100) + configPage2.unused2_57;
      configPage10.flexFuelAdj[x] = fuelAdder;

      uint8_t advanceAdder = (((configPage2.unused2_60 - configPage2.unused2_59) * pct) / 100) + configPage2.unused2_59;
      configPage10.flexAdvAdj[x] = advanceAdder;
    }

    writeAllConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 8);
  }

  //Final check is always for 255 and 0 (Brand new arduino)
  if( (EEPROM.read(EEPROM_DATA_VERSION) == 0) || (EEPROM.read(EEPROM_DATA_VERSION) == 255) )
  {
    configPage9.true_address = 0x200;
    EEPROM.write(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION);
  }

  //Check to see if someone has downgraded versions:
  if( EEPROM.read(EEPROM_DATA_VERSION) > CURRENT_DATA_VERSION ) { EEPROM.write(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION); }
}
