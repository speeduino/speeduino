#include <iostream>
#include <Arduino.h>
#include <EEPROM.h>
#include "init.h"
#include "mock_globals.h"
#include "storage.h"

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

void init_memory()
{
    int i = EEPROMClass::mem_size;
    while(i--) EEPROM.write(i,1);
}

void show()
{
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

int main()
{
    init_memory();

    //           ReqFuel    nCyl    nSqrt   alternate   injLayout
    set_constants(12.0,     3,      1,      false,      INJ_PAIRED);
    initialiseAll();
    show();

    std::cout << "PW:"<< PW(req_fuel_uS, 15, 27, 138, 1000) << std::endl;

    return 0;
}