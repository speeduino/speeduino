#include "../test_utils.h"
#include "shared.h"
#include "globals.h"

static test_context_t setup_simplepid_tune(void)
{
    test_context_t context;
    context.pins.pinBoost = 19U;

    context.page6.boostEnabled = true;
    context.page6.boostMode = BOOST_MODE_SIMPLE;
    context.page2.boostMinDuty = 0; 
    context.page2.boostMaxDuty = 255;
    context.page10.boostIntv = 0;
    context.page10.boostSens = 1;
    return context;
}

static test_context_t setup_fullpid_tune(void)
{
    auto context = setup_simplepid_tune();
    context.page6.boostMode = BOOST_MODE_FULL;
    context.page6.boostKP = 5;
    context.page6.boostKI = 3;
    context.page6.boostKD = 1;
    return context;
}

test_context_t setup_boost_tune(bool fullPid, uint8_t vssMode, uint8_t boostType, uint8_t gearMode)
{
    auto context = fullPid ? setup_fullpid_tune() : setup_simplepid_tune();
    context.page2.flexEnabled = false;
    context.page2.vssMode = vssMode;
    context.page4.boostType = boostType;
    context.page9.boostByGearEnabled = gearMode;
    context.page9.boostByGear[0] = 1;
    context.page9.boostByGear[1] = 2;
    context.page9.boostByGear[2] = 3;
    context.page9.boostByGear[3] = 4;
    context.page9.boostByGear[4] = 5;
    context.page9.boostByGear[5] = 6;
    context.page15.boostControlEnable = EN_BOOST_CONTROL_FIXED;
    context.page15.boostControlEnableThreshold = 0;
    fill_table_values(boostTable, 33);
    populate_table_axis(boostTable.axisX, 10);
    populate_table_axis(boostTable.axisY, 10);

    fill_table_values(boostTableLookupDuty, 11);
    populate_table_axis(boostTableLookupDuty.axisX, 10);
    populate_table_axis(boostTableLookupDuty.axisY, 10);
    return context;
}
