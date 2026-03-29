#include "globals.h"
#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "preprocessor.h"

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control

static fastOutputPin_t pins[INJ_CHANNELS];

template <uint8_t channel>
static void channel_High(void)
{
    if (channel<=_countof(pins))
    {
        pins[channel-1U].setPinHigh();
    }
}

template <uint8_t channel>
static void channel_Low(void)
{
    if (channel<=_countof(pins))
    {
        pins[channel-1U].setPinLow();
    }
}

void initInjDirectIO(const uint8_t (&pinNumbers)[INJ_CHANNELS])
{
    for (uint8_t i = 0; i < _countof(pins); i++)
    {
        pins[i].setPin(pinNumbers[i], OUTPUT);
    }
}


//Macros are used to define how each injector control system functions. These are then called by the master openInjectx() function.
//The DIRECT macros (ie individual pins) are defined below. Others should be defined in their relevant acc_x.h file
void openInjector1_DIRECT(void)  { channel_High<1>(); currentStatus.isInj1Open = true; }
void closeInjector1_DIRECT(void) { channel_Low<1>();  currentStatus.isInj1Open = false; }
void openInjector2_DIRECT(void)  { channel_High<2>(); currentStatus.isInj2Open = true; }
void closeInjector2_DIRECT(void) { channel_Low<2>();  currentStatus.isInj2Open = false; }
void openInjector3_DIRECT(void)  { channel_High<3>(); currentStatus.isInj3Open = true; }
void closeInjector3_DIRECT(void) { channel_Low<3>();  currentStatus.isInj3Open = false; }
void openInjector4_DIRECT(void)  { channel_High<4>(); currentStatus.isInj4Open = true; }
void closeInjector4_DIRECT(void) { channel_Low<4>();  currentStatus.isInj4Open = false; }
void openInjector5_DIRECT(void)  { channel_High<5>(); }
void closeInjector5_DIRECT(void) { channel_Low<5>(); }
void openInjector6_DIRECT(void)  { channel_High<6>(); }
void closeInjector6_DIRECT(void) { channel_Low<6>(); }
void openInjector7_DIRECT(void)  { channel_High<7>(); }
void closeInjector7_DIRECT(void) { channel_Low<7>(); }
void openInjector8_DIRECT(void)  { channel_High<8>(); }
void closeInjector8_DIRECT(void) { channel_Low<8>(); }

// LCOV_EXCL_STOP