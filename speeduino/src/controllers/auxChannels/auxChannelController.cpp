#include "auxChannelController.h"
#include "auxChannelController_detail.h"
#include "src/pins/pinMapping.h"
#include "unit_testing.h"
#include "sensors.h"
#include "comms_secondary.h"
#include "globals.h"

using namespace auxChannelController::detail;

TESTABLE_STATIC state _auxState;

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

static inline bool isExternalInput(uint8_t selector)
{
    return (selector & MASK_SELECTORB) == SELECTORB_EXTERNAL;
}

static inline bool isAnalogInput(uint8_t selector)
{
    return ((selector & MASK_SELECTORA) == SELECTORA_ANALOG)
        || ((selector & MASK_SELECTORB) == SELECTORB_ANALOG)
        ;
}

static inline uint8_t getAnalogPin(uint8_t channel, const config9 &page9)
{
    return pinTranslateAnalog(page9.Auxinpina[channel] & MASK_ANALOG_PIN);
}

static inline bool isDigitalInput(uint8_t selector)
{
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
        const auto selector = page9.caninput_sel[channel];
        if (isExternalInput(selector))
        {                
            _auxState.enabled |= bus_enabled;
        }
        else if (isAnalogInput(selector))
        {  
            _auxState.enabled |= tryInitPin(current, getAnalogPin(channel, page9));
        }
        else if (isDigitalInput(selector))
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
    const bool is_sec_serial = page9.enable_secondarySerial;
    const bool is_intcan     = page9.enable_intcan;
    const bool intcan_avail  = page9.intcan_available;
    const bool use_intcan    = is_intcan && intcan_avail;

    for (uint8_t channel = 0; channel < _countof(page9.caninput_sel); channel++)
    {
        const auto selector = page9.caninput_sel[channel];
        const uint16_t can_addr = (page9.caninput_source_can_address[channel] & MASK_SOURCE_ADDRESS) + 0x100;

        if (isExternalInput(selector))
        {
            // Route A: Secondary Serial (Megas only support CAN via secondary serial)
            if (is_sec_serial && (!use_intcan || ((selector & MASK_SOURCEB) == SOURCEB_SECONDARY_SERIAL)))
            {
                fnSendCanCommand(2, 0, channel, 0, can_addr);
            }
            // Route B: Internal CAN (STM32 / Teensy only)
#if defined(CORE_STM32) || defined(CORE_TEENSY)
            else if (use_intcan && (!is_sec_serial || ((selector & MASK_SOURCEB) == SOURCEB_INTERNAL_CAN)))
            {
                fnSendCanCommand(3, page9.speeduino_tsCanId, channel, 0, can_addr);
            }
            else 
            {
                // Do nothing. Keep MISRA checker happy
            }
#endif
        }  
        else if (isAnalogInput(selector))
        { 
            current.canin[channel] = fnReadAuxanalog(getAnalogPin(channel, page9));
        }
        else if (isDigitalInput(selector))
        { 
            current.canin[channel] = fnReadAuxdigital(getDigitalPin(channel, page9));
        }
        else 
        {
            // Do nothing. Keep MISRA checker happy
        }
    } 
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