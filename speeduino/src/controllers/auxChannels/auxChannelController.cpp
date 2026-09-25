#include "auxChannelController.h"
#include "auxChannelController_detail.h"
#include "src/pins/pinMapping.h"
#include "unit_testing.h"
#include "sensors.h"
#include "comms_secondary.h"
#include "globals.h"

using namespace auxChannelController::detail;

TESTABLE_STATIC state _auxState;

//   caninput_sel0a            = bits,   U08,     1, [0:1], "Off", "INVALID", "Analog_local", "Digital_local"
//   caninput_sel0b            = bits,   U08,     1, [2:3], "Off", "External Source", "Analog_local", "Digital_local"
//   caninput_sel0extsourcea   = bits,   U08,     1, [5:5], "Via Secondary Serial", "INVALID"
//   caninput_sel0extsourceb   = bits,   U08,     1, [6:6], "Via Secondary Serial", "Via Internal CAN"        
//   caninput_sel0extsourcec   = bits,   U08,     1, [7:7], "INVALID", "Via Internal CAN"

constexpr uint8_t MASK_SELECTORA =           0b00000011;
constexpr uint8_t SELECTORA_ANALOG =         0b00000010;
constexpr uint8_t SELECTORA_DIGITAL =        0b00000011;

constexpr uint8_t MASK_SELECTORB =           0b00001100;
constexpr uint8_t SELECTORB_EXTERNAL =       0b00000100;
constexpr uint8_t SELECTORB_ANALOG =         0b00001000;
constexpr uint8_t SELECTORB_DIGITAL =        0b00001100;

constexpr uint8_t MASK_SOURCEA =             0b00100000;

constexpr uint8_t MASK_SOURCEB =             0b01000000;
constexpr uint8_t SOURCEB_SECONDARY_SERIAL = 0b00000000;
constexpr uint8_t SOURCEB_INTERNAL_CAN =     0b01000000;

constexpr uint8_t MASK_SOURCEC =             0b10000000;
constexpr uint8_t SOURCEC_INTERNAL_CAN =     0b10000000;

constexpr uint8_t MASK_ANALOG_PIN =          0b00111111;
constexpr uint8_t MASK_DIGITAL_PIN =         0b00111111;

constexpr uint16_t MASK_SOURCE_ADDRESS =     0b0000'0111'1111'1111;

static inline bool isExternalInput(uint8_t channel, const config9 &page9)
{
    return (page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_EXTERNAL;
}

static inline bool isAnalogInput(uint8_t channel, const config9 &page9)
{
    auto selector = page9.caninput_sel[channel];
    return ((selector & MASK_SELECTORA) == SELECTORA_ANALOG)
        || ((selector & MASK_SELECTORB) == SELECTORB_ANALOG)
        ;
}

static inline uint8_t getAnalogPin(uint8_t channel, const config9 &page9)
{
    return pinTranslateAnalog(page9.Auxinpina[channel] & MASK_ANALOG_PIN);
}

static inline bool isDigitalInput(uint8_t channel, const config9 &page9)
{
    auto selector = page9.caninput_sel[channel];
    return ((selector & MASK_SELECTORA) == SELECTORA_DIGITAL)
        || ((selector & MASK_SELECTORB) == SELECTORB_DIGITAL)
        ;
}

static inline uint8_t getDigitalPin(uint8_t channel, const config9 &page9)
{
    return (page9.Auxinpinb[channel] & MASK_DIGITAL_PIN) + 1U;
}

void __attribute__((optimize("Os"))) initAuxChannels(statuses &current, const config9 &page9)
{
    _auxState = state();

    const bool is_sec_serial = page9.enable_secondarySerial;
    const bool is_intcan     = page9.enable_intcan;
    const bool intcan_avail  = page9.intcan_available;
    const bool bus_enabled   = is_sec_serial || (is_intcan && intcan_avail);

    // Helper lambda to consolidate pin initialization and error tracking
    auto tryInitPin = [](statuses &current, uint8_t pinNumber) {
        if (pinIsUsed(pinNumber)) {
            current.ioError = true;
            return false;
        } else {
            pinMode(pinNumber, INPUT);
            return true;
        }
    };

    for (uint8_t channel = 0U; channel < _countof(page9.caninput_sel); channel++)
    {
        if (isExternalInput(channel, page9))
        {                
            _auxState.enabled |= bus_enabled;
        }
        else if (isAnalogInput(channel, page9))
        {  
            _auxState.enabled |= tryInitPin(current, getAnalogPin(channel, page9));
        }
        else if (isDigitalInput(channel, page9))
        {  
            _auxState.enabled |= tryInitPin(current, getDigitalPin(channel, page9));
        }
        else 
        {
            // Do nothing. Keep MISRA checker happy
        }
    } 
}

using fnSendCanCommand_t = void (*)(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress);
using fnReadAuxanalog_t = uint16_t (*)(uint8_t analogPin);
using fnReadAuxdigital_t = decltype(&digitalRead);

TESTABLE_STATIC void auxChannelControl(statuses &current, const config9 &page9, fnSendCanCommand_t fnSendCanCommand, fnReadAuxanalog_t fnReadAuxanalog, fnReadAuxdigital_t fnReadAuxdigital)
{
    bool is_sec_serial = page9.enable_secondarySerial;
    bool is_intcan     = page9.enable_intcan;
    bool intcan_avail  = page9.intcan_available;

    //check through the Aux input channels if enabled for Can or local use
    for (uint8_t channel = 0; channel < _countof(page9.caninput_sel); channel++)
    {
        auto selector = page9.caninput_sel[channel];

        if ((isExternalInput(channel, page9)) 
            && ((is_sec_serial && (!is_intcan && intcan_avail))
            || (is_sec_serial && (is_intcan && intcan_avail)&& 
            ((selector & MASK_SOURCEB) == SOURCEB_SECONDARY_SERIAL))
            || (is_sec_serial && (is_intcan && !intcan_avail))))              
        { //if current input channel is enabled as external & secondary serial enabled & internal can disabled(but internal can is available)
        // or current input channel is enabled as external & secondary serial enabled & internal can enabled(and internal can is available)
            if (is_sec_serial)  // megas only support can via secondary serial
            {
                fnSendCanCommand(2,0,channel,0,((page9.caninput_source_can_address[channel] & MASK_SOURCE_ADDRESS)+0x100));
                //send an R command for data from caninput_source_address[AuxinChan] from secondarySerial
            }
        }  
        else if ((isExternalInput(channel, page9)) 
            && ((is_sec_serial && (is_intcan && intcan_avail)&& 
            ((selector & MASK_SOURCEB) == SOURCEB_INTERNAL_CAN))
            || (!is_sec_serial && (is_intcan && intcan_avail)&& 
            ((selector & MASK_SOURCEC) == SOURCEC_INTERNAL_CAN))))                             
        { //if current input channel is enabled as external for canbus & secondary serial enabled & internal can enabled(and internal can is available)
        // or current input channel is enabled as external for canbus & secondary serial disabled & internal can enabled(and internal can is available)
#if defined(CORE_STM32) || defined(CORE_TEENSY)
            if is_intcan //  if internal can is enabled 
            {
                fnSendCanCommand(3,page9.speeduino_tsCanId,AuxinChan,0,((page9.caninput_source_can_address[AuxinChan] & MASK_SOURCE_ADDRESS)+0x100));  
                //send an R command for data from caninput_source_address[AuxinChan] from internal canbus
            }
#endif
        }   
        else if (((is_sec_serial || (is_intcan && intcan_avail)) && isAnalogInput(channel, page9))
                || ((!is_sec_serial && ( is_intcan && !intcan_avail )) && isAnalogInput(channel, page9))  
                || ((!is_sec_serial && !is_intcan) && (isAnalogInput(channel, page9))))  
        { //if current input channel is enabled as analog local pin
            //read analog channel specified
            current.canin[channel] = fnReadAuxanalog(getAnalogPin(channel, page9));
        }
        else if (((is_sec_serial || (is_intcan && intcan_avail)) && isDigitalInput(channel, page9))
                || ((!is_sec_serial && ( is_intcan && !intcan_avail )) && isDigitalInput(channel, page9))
                || ((!is_sec_serial && !is_intcan) && isDigitalInput(channel, page9)))
        { //if current input channel is enabled as digital local pin
            //read digital channel specified
            current.canin[channel] = fnReadAuxdigital((page9.Auxinpinb[channel] & MASK_DIGITAL_PIN)+1);
        } //Channel type
    } //For loop going through each channel
}

// LCOV_EXCL_START
void auxChannelControl(statuses &current, const config9 &page9)
{
    if(_auxState.enabled
    && BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_4HZ))
    {
        auxChannelControl(current, page9, sendCancommand, readAnalogSensor, digitalRead);
    }
}
// LCOV_EXCL_STOP