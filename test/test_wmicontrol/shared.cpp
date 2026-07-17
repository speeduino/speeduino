#include "globals.h"
#include "shared.h"

void setup_wmi_tune(uint8_t mode)
{
    pinWMIEmpty = TANK_EMPTY_PIN;

    configPage10.vvt2Enabled = false;
    configPage10.wmiEnabled = true;
    configPage10.wmiEmptyEnabled = true;
    configPage10.wmiEmptyPolarity = true; 
    configPage10.wmiTPS = 50;
    configPage10.wmiRPM = 15;
    configPage10.wmiMAP = 33;
    configPage10.wmiMAP2 = 99;
    configPage10.wmiIAT = 45;
    configPage10.wmiMode = mode;
    configPage10.wmiOffset = -33;
}