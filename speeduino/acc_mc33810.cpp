#include <SPI.h>
#include "acc_mc33810.h"
#include "src/pins/fastOutputPin.h"
#include "preprocessor.h"

#if defined(MC33810_SUPPORT)

static uint8_t MC33810_BIT_INJ[8] = {};
static uint8_t MC33810_BIT_IGN[8] = {};

static constexpr uint8_t MC33810_ONOFF_CMD = 0x30; //48 in decimal

struct mc33810_t
{
    fastOutputPin_t pin;
    volatile uint8_t requestedState; //Current binary state of the 2nd ICs IGN and INJ values
    volatile uint8_t returnState; //Current binary state of the 1st ICs IGN and INJ values

    void init(uint8_t pinNumber)
    {
        pin.setPin(pinNumber, OUTPUT);

        //Set the output states to be off to fuel and ignition
        requestedState = 0;
        returnState = 0;
    }

    uint8_t sendCommand(uint16_t command);
    
    void setBit(uint8_t bit)
    {
        BIT_SET(requestedState, bit); 
        returnState = sendCommand(word(MC33810_ONOFF_CMD, requestedState));        
    }
    
    void clearBit(uint8_t bit)
    {
        BIT_CLEAR(requestedState, bit); 
        returnState = sendCommand(word(MC33810_ONOFF_CMD, requestedState));        
    }
};

// RAII scope guard for MC33810 active/inactive
struct mc33810_active_guard_t
{
    mc33810_t &_mc33810;
    mc33810_active_guard_t(mc33810_t &mc33810)
    : _mc33810(mc33810)
    {
        _mc33810.pin.setPinHigh();
    }
    ~mc33810_active_guard_t()
    {
        _mc33810.pin.setPinLow();
    }
};

uint8_t mc33810_t::sendCommand(uint16_t command)
{
    mc33810_active_guard_t active(*this);
    return SPI.transfer16(command);
}

static mc33810_t mc33810_1;
static mc33810_t mc33810_2;

static inline mc33810_t& getMC33810ForChannel(uint8_t channel)
{ 
    // Upper 4 channels (injection *and* ignition) are on the 2nd IC, lower 4 on the first IC
    return channel>4U ? mc33810_2 : mc33810_1;
}

static void coilHigh(uint8_t channel)
{ 
    if (channel<=_countof(MC33810_BIT_IGN))
    {
        getMC33810ForChannel(channel).setBit(MC33810_BIT_IGN[channel-1U]); 
    }
}

static void coilLow(uint8_t channel)
{ 
    if (channel<=_countof(MC33810_BIT_IGN))
    {
        getMC33810ForChannel(channel).clearBit(MC33810_BIT_IGN[channel-1U]); 
    }
}

using channelFunc = void(*)(uint8_t);
static channelFunc coilChargingFn = coilHigh;
static channelFunc coilDischargingFn = coilLow;

void __attribute__((optimize("Os"))) initMC33810(const config4 &page4,
                                                 uint8_t pinMC33810_1, uint8_t pinMC33810_2,
                                                 const uint8_t (&injBits)[8], const uint8_t (&ignBits)[8])
{
    static_assert(sizeof(MC33810_BIT_INJ)==sizeof(injBits), "Mismatch!");
    memcpy(MC33810_BIT_INJ, injBits, sizeof(MC33810_BIT_INJ));
    static_assert(sizeof(MC33810_BIT_IGN)==sizeof(ignBits), "Mismatch!");
    memcpy(MC33810_BIT_IGN, ignBits, sizeof(MC33810_BIT_IGN));

    //Set pin port/masks
    mc33810_1.init(pinMC33810_1);
    mc33810_2.init(pinMC33810_2);

    SPI.begin();
    //These are the SPI settings per the datasheet
	SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE0)); 

    //Set the ignition outputs to GPGD mode
    /*
    0001 = Mode select command
    1111 = Set all 1 GD[0...3] outputs to use GPGD mode
    00000000 = All remaining values are unused (For us)
    */
    //uint16_t cmd = 0b000111110000;
    constexpr uint16_t modeCmd = 0b0001'1111'0000'0000;
    mc33810_1.sendCommand(modeCmd);
    mc33810_2.sendCommand(modeCmd);

    //Disable the Open Load pull-down current sync (See page 31 of MC33810 DS)
    /*
    0010 = LSD Fault Command
    1000 = LSD Fault operation is Shutdown (Default)
    1111 = Open load detection fault when active (Default)
    0000 = Disable open load detection when off (Changed from 1111 to 0000)
    */
    constexpr uint16_t loadDetectCmd = 0b0010'1000'1111'0000;
    mc33810_1.sendCommand(loadDetectCmd);
    mc33810_2.sendCommand(loadDetectCmd);

    if (page4.IgInv == GOING_HIGH)
    {
        coilChargingFn = coilLow;
        coilDischargingFn = coilHigh;
    }
    else
    {
        coilChargingFn = coilHigh;
        coilDischargingFn = coilLow;
    }
}

void openInjector_MC33810(uint8_t channel)
{
    if (channel<=_countof(MC33810_BIT_INJ))
    {
        getMC33810ForChannel(channel).setBit(MC33810_BIT_INJ[channel-1U]);
    }
}

void closeInjector_MC33810(uint8_t channel)
{ 
    if (channel<=_countof(MC33810_BIT_INJ))
    {
        getMC33810ForChannel(channel).clearBit(MC33810_BIT_INJ[channel-1U]); 
    }
}

void coilCharging_MC33810(uint8_t channel) 
{ 
    coilChargingFn(channel);
}

void coilStopCharging_MC33810(uint8_t channel)
{
    coilDischargingFn(channel);
}
#endif