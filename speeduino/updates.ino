/*
 * This routine is used for doing any data conversions that are required during firmware changes
 * This prevents users getting difference reports in TS when such a data change occurs.
 * It also can be used for setting good values when there are viarables that move locations in the ini
 * When a user skips multiple firmware versions at a time, this will roll through the updates 1 at a time
 */


void doUpdates()
{
  #define CURRENT_DATA_VERSION    3

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
    writeConfig();
    EEPROM.write(EEPROM_DATA_VERSION, 3);
  }

  //Final check is always for 255 and 0 (Brand new arduino)
  if(EEPROM.read(EEPROM_DATA_VERSION) == 0 || EEPROM.read(EEPROM_DATA_VERSION) == 255)
  {
    EEPROM.write(EEPROM_DATA_VERSION, CURRENT_DATA_VERSION);
  }
}
