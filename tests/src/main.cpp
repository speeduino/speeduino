#include <iostream>
#include <string>

#include "speeduino.cpp"

uint16_t mock_getRPM()
{
    return 1200;
}

// This function takes the input parameters we would put in TunerStudio and converts them to what speeduino knows.
bool set_constants(float reqFuel, unsigned nCylinders, unsigned nSquirts, bool alternate, int injLayout)
{
    // The checks below were removed from TunerStudio.
    //    if(nCylinders < nSquirts) return false;
    //    if(alternate && (nSquirts & 1) == 1) return false; // Can not alternate odd number of squirts

    std::cout << std::endl << "reqFuel:" << reqFuel << ", nCylinders:" << nCylinders << ", nSquirts:" << nSquirts << ", alternate:" << (alternate ? "TRUE":"FALSE") << std::endl;

    reqFuel /= nSquirts;
    if (alternate)
        reqFuel *= 2;

    configPage2.reqFuel = (byte)(reqFuel * 10);
    configPage2.nCylinders = nCylinders;
    configPage2.divider = nCylinders / nSquirts;
    configPage2.injTiming = alternate;
    configPage2.strokes = FOUR_STROKE;
    configPage2.engineType = EVEN_FIRE;
    configPage2.injLayout = injLayout;
    configPage10.stagingEnabled = false;

    // Needs two writes because reasons
    writeAllConfig();
    writeAllConfig();

    // Reset the variables below to their default value.
    channel1InjEnabled = false;
    channel2InjEnabled = false;
    channel3InjEnabled = false;
    channel4InjEnabled = false;
    channel5InjEnabled = false;
    channel6InjEnabled = false;
    channel7InjEnabled = false;
    channel8InjEnabled = false;

    return true;
}

int main()
{
    //            ReqFuel, nCyl, nSqrt, alternate, injLayout
    set_constants(12.0,    4,    2,     false,     INJ_PAIRED);
    initialiseAll();
    std::cout << EEPROM.read(0) << std::endl;
    std::cout << "req_fuel_uS:" << req_fuel_uS << std::endl;
    std::cout << "CRANK_ANGLE_MAX_INJ:" << CRANK_ANGLE_MAX_INJ << std::endl;

    std::cout << "channel1Inj:" << (channel1InjEnabled ? std::to_string(channel1InjDegrees) : "disabled") << std::endl;
    std::cout << "channel2Inj:" << (channel2InjEnabled ? std::to_string(channel2InjDegrees) : "disabled") << std::endl;
    std::cout << "channel3Inj:" << (channel3InjEnabled ? std::to_string(channel3InjDegrees) : "disabled") << std::endl;
    std::cout << "channel4Inj:" << (channel4InjEnabled ? std::to_string(channel4InjDegrees) : "disabled") << std::endl;

    //            ReqFuel, nCyl, nSqrt, alternate, injLayout
    set_constants(12.0,    4,    2,     true,     INJ_PAIRED);
    initialiseAll();
    std::cout << "req_fuel_uS:" << req_fuel_uS << std::endl;
    std::cout << "CRANK_ANGLE_MAX_INJ:" << CRANK_ANGLE_MAX_INJ << std::endl;

    std::cout << "channel1Inj:" << (channel1InjEnabled ? std::to_string(channel1InjDegrees) : "disabled") << std::endl;
    std::cout << "channel2Inj:" << (channel2InjEnabled ? std::to_string(channel2InjDegrees) : "disabled") << std::endl;
    std::cout << "channel3Inj:" << (channel3InjEnabled ? std::to_string(channel3InjDegrees) : "disabled") << std::endl;
    std::cout << "channel4Inj:" << (channel4InjEnabled ? std::to_string(channel4InjDegrees) : "disabled") << std::endl;

    getRPM = mock_getRPM;
    std::cout << getRPM() << std::endl;

    int pw = PW(12000, 50, 50, 100, 0);
    std::cout << pw << std::endl;
}
