#include <iostream>
#include <Arduino.h>
#include <EEPROM.h>
#include "mock_globals.h"
#include <gtest/gtest.h>

void init_memory()
{
    for(int i = 0; i < EEPROMClass::mem_size; i++)
        EEPROM.write(i,1);
}

void set_constants(float reqFuel, int nCylinders, int nSquirts, bool alternate, int injLayout)
{
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
}


void show()
{
    std::cout << std::endl;
    std::cout << "req_fuel_uS\t\t" << (int)req_fuel_uS << std::endl;
    std::cout << "Channel1 " << (channel1InjEnabled ? "enabled" : "disabled") << ":\t" << channel1InjDegrees << std::endl;
    std::cout << "Channel2 " << (channel2InjEnabled ? "enabled" : "disabled") << ":\t" << channel2InjDegrees << std::endl;
    std::cout << "Channel3 " << (channel3InjEnabled ? "enabled" : "disabled") << ":\t" << channel3InjDegrees << std::endl;
    std::cout << "Channel4 " << (channel4InjEnabled ? "enabled" : "disabled") << ":\t" << channel4InjDegrees << std::endl;
    std::cout << "Channel5 " << (channel5InjEnabled ? "enabled" : "disabled") << ":\t" << channel5InjDegrees << std::endl;
    std::cout << "CRANK_ANGLE_MAX_INJ\t" << (int)CRANK_ANGLE_MAX_INJ << std::endl;
    std::cout << "currentStatus.nSquirts\t" << (int)currentStatus.nSquirts << std::endl;
    std::cout << std::endl;
}

TEST(SpeeduinoTestClass, TestTest)
{
    init_memory();

    //           ReqFuel    nCyl    nSqrt   alternate   injLayout
    set_constants(12.0,     3,      1,      false,      INJ_PAIRED);
    setup();

    EXPECT_EQ(6000, req_fuel_uS);
    EXPECT_EQ(1, currentStatus.nSquirts);
    EXPECT_EQ(720, CRANK_ANGLE_MAX_INJ);

//    Serial.put_char('Q');

//    loop();
//    std::cout << millis() << std::endl;
//    show();

//    std::cout << "PW:"<< PW(req_fuel_uS, 15, 27, 138, 1000) << std::endl;

//    return 0;
}


int main(int pArgCount, char* pArgValues[])
{
    ::testing::InitGoogleTest(&pArgCount, pArgValues);
    return RUN_ALL_TESTS();
}

