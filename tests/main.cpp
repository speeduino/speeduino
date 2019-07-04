#include <iostream>
#include <gtest/gtest.h>

#include <Arduino.h>
#include <EEPROM.h>

#include "mock_globals.h"
#include "storage.h"

void init_memory()
{
    channel1InjEnabled = false;
    channel2InjEnabled = false;
    channel3InjEnabled = false;
    channel4InjEnabled = false;
    channel5InjEnabled = false;
    channel6InjEnabled = false;
    channel7InjEnabled = false;
    channel8InjEnabled = false;

    for(int i = 0; i < EEPROMClass::mem_size; i++)
        EEPROM.write(i,1);
}

bool set_constants(float reqFuel, unsigned nCylinders, unsigned nSquirts, bool alternate, int injLayout)
{
    if(nCylinders < nSquirts) return false;
//    if(alternate && (nSquirts % 2) == 1) return false;

    reqFuel /= nSquirts;
    if(alternate) reqFuel *= 2;

    configPage2.reqFuel = (byte)(reqFuel*10);
    configPage2.nCylinders = nCylinders;
    configPage2.divider = nCylinders/nSquirts;
    configPage2.injTiming = alternate;
    configPage2.strokes = FOUR_STROKE;
    configPage2.engineType = EVEN_FIRE;
    configPage2.injLayout = injLayout;
    configPage10.stagingEnabled = false;
    writeAllConfig();
    writeAllConfig();
    return true;
}

TEST(SpeeduinoTestClass, 2CylSemiSeqAltTest)
{
    init_memory();

    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     2,      2,      true,      INJ_SEMISEQUENTIAL));
    setup();

    EXPECT_EQ(6000, req_fuel_uS);
    EXPECT_EQ(2, currentStatus.nSquirts);
    EXPECT_EQ(360, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
}

TEST(SpeeduinoTestClass, 2CylSeqAltTest)
{
    init_memory();

    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     2,      2,      true,      INJ_SEQUENTIAL));
    setup();

    EXPECT_EQ(12000, req_fuel_uS);
    EXPECT_EQ(1, currentStatus.nSquirts);
    EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
}

TEST(SpeeduinoTestClass, 3CylPairedSimTest)
{
    init_memory();

    //                       ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     3,      1,      false,      INJ_PAIRED));
    setup();

    EXPECT_EQ(6000, req_fuel_uS);
    EXPECT_EQ(1, currentStatus.nSquirts);
    EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(true, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(0, channel2InjDegrees);
    EXPECT_EQ(0, channel3InjDegrees);
}

TEST(SpeeduinoTestClass, 3CylSeqAltTest)
{
    init_memory();

    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     3,      3,      true,      INJ_SEQUENTIAL));
    setup();

    EXPECT_EQ(8000, req_fuel_uS);
    EXPECT_EQ(1, currentStatus.nSquirts);
    EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(true, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0,   channel1InjDegrees);
    EXPECT_EQ(240, channel2InjDegrees);
    EXPECT_EQ(480, channel3InjDegrees);
}

TEST(SpeeduinoTestClass, 4CylPairedSimTest)
{
    init_memory();

    //                       ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     4,      1,      false,      INJ_PAIRED));
    setup();

    EXPECT_EQ(6000, req_fuel_uS);
    EXPECT_EQ(1, currentStatus.nSquirts);
    EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(0, channel2InjDegrees);
}

TEST(SpeeduinoTestClass, 4CylSemiSeqAltTest)
{
    init_memory();

    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     4,      2,      true,      INJ_SEMISEQUENTIAL));
    setup();

    EXPECT_EQ(6000, req_fuel_uS);
    EXPECT_EQ(2, currentStatus.nSquirts);
    EXPECT_EQ(360, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(false, channel3InjEnabled);
    EXPECT_EQ(false, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0, channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
}

TEST(SpeeduinoTestClass, 4CylSeqAltTest)
{
    init_memory();

    //                      ReqFuel    nCyl    nSqrt   alternate   injLayout
    ASSERT_TRUE(set_constants(12.0,     4,      2,      true,      INJ_SEQUENTIAL));
    setup();

    EXPECT_EQ(12000, req_fuel_uS);
    EXPECT_EQ(1, currentStatus.nSquirts);
    EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);

    EXPECT_EQ(true, channel1InjEnabled);
    EXPECT_EQ(true, channel2InjEnabled);
    EXPECT_EQ(true, channel3InjEnabled);
    EXPECT_EQ(true, channel4InjEnabled);
    EXPECT_EQ(false, channel5InjEnabled);

    EXPECT_EQ(0,   channel1InjDegrees);
    EXPECT_EQ(180, channel2InjDegrees);
    EXPECT_EQ(360, channel3InjDegrees);
    EXPECT_EQ(540, channel4InjDegrees);
}

int main(int pArgCount, char* pArgValues[])
{
    ::testing::InitGoogleTest(&pArgCount, pArgValues);
    return RUN_ALL_TESTS();
}

