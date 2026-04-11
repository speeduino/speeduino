#include "board_definition.h"
#include "src/pins/fastOutputPin.h"
#include "preprocessor.h"

// LCOV_EXCL_START
// Exclude from code coverage, since this is all board output control
 
static fastOutputPin_t pins[IGN_CHANNELS];

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

void initIgnDirectIO(const uint8_t (&pinNumbers)[IGN_CHANNELS])
{
    for (uint8_t i = 0; i < _countof(pins); i++)
    {
        pins[i].setPin(pinNumbers[i], OUTPUT);
    }
}

void coil1Low_DIRECT(void)  { channel_Low<1U>(); }
void coil1High_DIRECT(void) { channel_High<1U>(); }
void coil2Low_DIRECT(void)  { channel_Low<2U>(); }
void coil2High_DIRECT(void) { channel_High<2U>(); }
void coil3Low_DIRECT(void)  { channel_Low<3U>(); }
void coil3High_DIRECT(void) { channel_High<3U>(); }
void coil4Low_DIRECT(void)  { channel_Low<4U>(); }
void coil4High_DIRECT(void) { channel_High<4U>(); }
void coil5Low_DIRECT(void)  { channel_Low<5U>(); }
void coil5High_DIRECT(void) { channel_High<5U>(); }
void coil6Low_DIRECT(void)  { channel_Low<6U>(); }
void coil6High_DIRECT(void) { channel_High<6U>(); }
void coil7Low_DIRECT(void)  { channel_Low<7U>(); }
void coil7High_DIRECT(void) { channel_High<7U>(); }
void coil8Low_DIRECT(void)  { channel_Low<8U>(); }
void coil8High_DIRECT(void) { channel_High<8U>(); }

// LCOV_EXCL_STOP