#include "scheduledIO_ign.h"
#include "scheduledIO_direct_ign.h"
#include "acc_mc33810.h"
#include "timers.h"
#include "globals.h"

static IgnIoControlMode _controlMode = IgnIoControlMode::Direct;

void initIgnIoControl(IgnIoControlMode controlMode)
{
    _controlMode = controlMode;
}

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control
 
static void tachoOutputOn(void) { if(configPage6.tachoMode) { tachoPulseLow(); } else { tachoOutputFlag = READY; } }
static void tachoOutputOff(void) { if(configPage6.tachoMode) { tachoPulseHigh(); } }

void beginCoilCharge(uint8_t channel) 
{ 
#if defined(MC33810_SUPPORT)
    if(_controlMode==IgnIoControlMode::Direct) 
    {
        coilCharging_DIRECT(channel);
    }
    else
    {
        coilCharging_MC33810(channel);
    }
#else
    coilCharging_DIRECT(channel);
#endif
    tachoOutputOn(); 
}

void endCoilCharge(uint8_t channel)
{
#if defined(MC33810_SUPPORT)
    if(_controlMode==IgnIoControlMode::Direct) 
    {
        coilStopCharging_DIRECT(channel);
    }
    else
    {
        coilStopCharging_MC33810(channel);
    }
#else
    coilStopCharging_DIRECT(channel);
#endif
    tachoOutputOff();
}

void beginCoil1Charge(void) { beginCoilCharge(1U); }
void endCoil1Charge(void) { endCoilCharge(1U); }

void beginCoil2Charge(void) { beginCoilCharge(2U); }
void endCoil2Charge(void) { endCoilCharge(2U); }

void beginCoil3Charge(void) { beginCoilCharge(3U); }
void endCoil3Charge(void) { endCoilCharge(3U); }

void beginCoil4Charge(void) { beginCoilCharge(4U); }
void endCoil4Charge(void) { endCoilCharge(4U); }

void beginCoil5Charge(void) { beginCoilCharge(5U); }
void endCoil5Charge(void) { endCoilCharge(5U); }

void beginCoil6Charge(void) { beginCoilCharge(6U); }
void endCoil6Charge(void) { endCoilCharge(6U); }

void beginCoil7Charge(void) { beginCoilCharge(7U); }
void endCoil7Charge(void) { endCoilCharge(7U); }

void beginCoil8Charge(void) { beginCoilCharge(8U); }
void endCoil8Charge(void) { endCoilCharge(8U); }

//The below 3 calls are all part of the rotary ignition mode
void beginTrailingCoilCharge(void) { beginCoilCharge(2U); }
void endTrailingCoilCharge1(void) { endCoilCharge(2U); beginCoilCharge(3U); } //Sets ign3 (Trailing select) high
void endTrailingCoilCharge2(void) { endCoilCharge(2U); endCoilCharge(3U); } //sets ign3 (Trailing select) low

//As above but for ignition (Wasted COP mode)
void beginCoil1and3Charge(void) { beginCoilCharge(1U); beginCoilCharge(3U); }
void endCoil1and3Charge(void)   { endCoilCharge(1U);  endCoilCharge(3U); }
void beginCoil2and4Charge(void) { beginCoilCharge(2U); beginCoilCharge(4U); }
void endCoil2and4Charge(void)   { endCoilCharge(2U);  endCoilCharge(4U); }

//For 6cyl wasted COP mode)
void beginCoil1and4Charge(void) { beginCoilCharge(1U); beginCoilCharge(4U); }
void endCoil1and4Charge(void)   { endCoilCharge(1U);  endCoilCharge(4U); }
void beginCoil2and5Charge(void) { beginCoilCharge(2U); beginCoilCharge(5U); }
void endCoil2and5Charge(void)   { endCoilCharge(2U);  endCoilCharge(5U); }
void beginCoil3and6Charge(void) { beginCoilCharge(3U); beginCoilCharge(6U); }
void endCoil3and6Charge(void)   { endCoilCharge(3U); endCoilCharge(6U); }

//For 8cyl wasted COP mode)
void beginCoil1and5Charge(void) { beginCoilCharge(1U); beginCoilCharge(5U); }
void endCoil1and5Charge(void)   { endCoilCharge(1U);  endCoilCharge(5U); }
void beginCoil2and6Charge(void) { beginCoilCharge(2U); beginCoilCharge(6U); }
void endCoil2and6Charge(void)   { endCoilCharge(2U);  endCoilCharge(6U); }
void beginCoil3and7Charge(void) { beginCoilCharge(3U); beginCoilCharge(7U);  }
void endCoil3and7Charge(void)   { endCoilCharge(3U); endCoilCharge(7U); }
void beginCoil4and8Charge(void) { beginCoilCharge(4U); beginCoilCharge(8U); }
void endCoil4and8Charge(void)   { endCoilCharge(4U);  endCoilCharge(8U); }

// LCOV_EXCL_STOP