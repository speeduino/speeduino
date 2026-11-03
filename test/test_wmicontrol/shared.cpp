#include "globals.h"
#include "shared.h"

constexpr uint8_t TANK_EMPTY_PIN = 11;

test_context_t setup_wmi_tune(uint8_t mode)
{
    test_context_t context;
    context.pins.pinWMIEmpty = TANK_EMPTY_PIN;
    context.pins.pinWMIEnabled = 18;
    context.page10.vvt2Enabled = false;
    context.page10.wmiEnabled = true;
    context.page10.wmiEmptyEnabled = true;
    context.page10.wmiEmptyPolarity = true; 
    context.page10.wmiTPS = 50;
    context.page10.wmiRPM = 15;
    context.page10.wmiMAP = 33;
    context.page10.wmiMAP2 = 99;
    context.page10.wmiIAT = 45;
    context.page10.wmiMode = mode;
    context.page10.wmiOffset = -33;
    return context;
}