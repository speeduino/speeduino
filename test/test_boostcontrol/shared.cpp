#include "../test_utils.h"
#include "shared.h"
#include "globals.h"

void setup_simplepid_tune(void)
{
    pinNumbers.pinBoost = 19U;

    configPage6.boostEnabled = true;
    configPage6.boostMode = BOOST_MODE_SIMPLE;
    configPage2.boostMinDuty = 0; 
    configPage2.boostMaxDuty = 255;
    configPage10.boostIntv = 0;
    configPage10.boostSens = 1;
}

void setup_fullpid_tune(void)
{
    setup_simplepid_tune();
    configPage6.boostMode = BOOST_MODE_FULL;
    configPage6.boostKP = 5;
    configPage6.boostKI = 3;
    configPage6.boostKD = 1;
}

void setup_boost_tune(bool fullPid, uint8_t vssMode, uint8_t boostType, uint8_t gearMode)
{
  if (fullPid)
  {
    setup_fullpid_tune();
  }
  else
  {
   setup_simplepid_tune();
  }
  configPage2.flexEnabled = false;
  configPage2.vssMode = vssMode;
  configPage4.boostType = boostType;
  configPage9.boostByGearEnabled = gearMode;
  configPage9.boostByGear[0] = 1;
  configPage9.boostByGear[1] = 2;
  configPage9.boostByGear[2] = 3;
  configPage9.boostByGear[3] = 4;
  configPage9.boostByGear[4] = 5;
  configPage9.boostByGear[5] = 6;
  configPage15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
  fill_table_values(boostTable, 33);
  populate_table_axis(boostTable.axisX, 10);
  populate_table_axis(boostTable.axisY, 10);

  fill_table_values(boostTableLookupDuty, 11);
  populate_table_axis(boostTableLookupDuty.axisX, 10);
  populate_table_axis(boostTableLookupDuty.axisY, 10);
}
