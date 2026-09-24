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

void __attribute__((optimize("Os"))) initAuxChannels(statuses &current, const config9 &page9)
{
    //The following checks the aux inputs and initialises pins if required
    _auxState = state();
    for (uint8_t channel = 0U; channel < _countof(page9.caninput_sel) ; channel++)
    {            
        if (((page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_EXTERNAL)
        && ((page9.enable_secondarySerial == 1U) || ((page9.enable_intcan == 1U) && (page9.intcan_available == 1U))))
        { //if current input channel is enabled as external input in caninput_selxb(bits 2:3) and secondary serial or internal canbus is enabled(and is mcu supported)                 
            _auxState.enabled = true;
        }
        else if ((((page9.enable_secondarySerial == 1U) || ((page9.enable_intcan == 1U) && (page9.intcan_available == 1U))) && (page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_ANALOG)
                || (((page9.enable_secondarySerial == 0U) && ( (page9.enable_intcan == 1U) && (page9.intcan_available == 0U) )) && (page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_ANALOG)  
                || (((page9.enable_secondarySerial == 0U) && (page9.enable_intcan == 0U)) && ((page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_ANALOG)))  
        {  //if current input channel is enabled as analog local pin check caninput_selxb(bits 2:3) with &12 and caninput_selxa(bits 0:1) with &3
            uint8_t pinNumber = pinTranslateAnalog(page9.Auxinpina[channel] & MASK_ANALOG_PIN);
            if( pinIsUsed(pinNumber) )
            {
                //Do nothing here as the pin is already in use.
                current.ioError = true; //Tell user that there is problem by lighting up the I/O error indicator
            }
            else
            {
                //Channel is active and analog
                pinMode( pinNumber, INPUT);
                _auxState.enabled = true;
            }  
        }
        else if ((((page9.enable_secondarySerial == 1U) || ((page9.enable_intcan == 1U) && (page9.intcan_available == 1U))) && (page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_DIGITAL)
                || (((page9.enable_secondarySerial == 0U) && ( (page9.enable_intcan == 1U) && (page9.intcan_available == 0U) )) && (page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_DIGITAL)
                || (((page9.enable_secondarySerial == 0U) && (page9.enable_intcan == 0U)) && ((page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_DIGITAL)))
        {  //if current input channel is enabled as digital local pin check caninput_selxb(bits 2:3) with &12 and caninput_selxa(bits 0:1) with &3
            uint8_t pinNumber = (page9.Auxinpinb[channel] & MASK_DIGITAL_PIN) + 1U;
            if( pinIsUsed(pinNumber) )
            {
                //Do nothing here as the pin is already in use.
                current.ioError = true; //Tell user that there is problem by lighting up the I/O error indicator
            }
            else
            {
                //Channel is active and digital
                pinMode( pinNumber, INPUT);
                _auxState.enabled = true;
            }  
        }
        else {
            //  Do nothing. Keep MISRA checker happy
        }
    } //For loop iterating through aux in lines
}

using fnSendCanCommand_t = void (*)(uint8_t cmdtype, uint16_t canaddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress);
using fnReadAuxanalog_t = uint16_t (*)(uint8_t analogPin);
using fnReadAuxdigital_t = decltype(&digitalRead);

TESTABLE_STATIC void auxChannelControl(statuses &current, const config9 &page9, fnSendCanCommand_t fnSendCanCommand, fnReadAuxanalog_t fnReadAuxanalog, fnReadAuxdigital_t fnReadAuxdigital)
{
    //check through the Aux input channels if enabled for Can or local use
    for (uint8_t channel = 0; channel < _countof(page9.caninput_sel); channel++)
    {
        if (((page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_EXTERNAL) 
            && (((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 0)&&(page9.intcan_available == 1)))
            || ((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 1))&& 
            ((page9.caninput_sel[channel] & MASK_SOURCEB) == SOURCEB_SECONDARY_SERIAL))
            || ((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 0)))))              
        { //if current input channel is enabled as external & secondary serial enabled & internal can disabled(but internal can is available)
        // or current input channel is enabled as external & secondary serial enabled & internal can enabled(and internal can is available)
        if (page9.enable_secondarySerial == 1)  // megas only support can via secondary serial
        {
            fnSendCanCommand(2,0,channel,0,((page9.caninput_source_can_address[channel] & MASK_SOURCE_ADDRESS)+0x100));
            //send an R command for data from caninput_source_address[AuxinChan] from secondarySerial
        }
        }  
        else if (((page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_EXTERNAL) 
            && (((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 1))&& 
            ((page9.caninput_sel[channel] & MASK_SOURCEB) == SOURCEB_INTERNAL_CAN))
            || ((page9.enable_secondarySerial == 0) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 1))&& 
            ((page9.caninput_sel[channel] & MASK_SOURCEC) == SOURCEC_INTERNAL_CAN))))                             
        { //if current input channel is enabled as external for canbus & secondary serial enabled & internal can enabled(and internal can is available)
        // or current input channel is enabled as external for canbus & secondary serial disabled & internal can enabled(and internal can is available)
        #if defined(CORE_STM32) || defined(CORE_TEENSY)
        if (page9.enable_intcan == 1) //  if internal can is enabled 
        {
            fnSendCanCommand(3,page9.speeduino_tsCanId,AuxinChan,0,((page9.caninput_source_can_address[AuxinChan] & MASK_SOURCE_ADDRESS)+0x100));  
            //send an R command for data from caninput_source_address[AuxinChan] from internal canbus
        }
        #endif
        }   
        else if ((((page9.enable_secondarySerial == 1) || ((page9.enable_intcan == 1) && (page9.intcan_available == 1))) && (page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_ANALOG)
                || (((page9.enable_secondarySerial == 0) && ( (page9.enable_intcan == 1) && (page9.intcan_available == 0) )) && (page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_ANALOG)  
                || (((page9.enable_secondarySerial == 0) && (page9.enable_intcan == 0)) && ((page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_ANALOG)))  
        { //if current input channel is enabled as analog local pin
        //read analog channel specified
        current.canin[channel] = fnReadAuxanalog(pinTranslateAnalog(page9.Auxinpina[channel] & MASK_ANALOG_PIN));
        }
        else if ((((page9.enable_secondarySerial == 1) || ((page9.enable_intcan == 1) && (page9.intcan_available == 1))) && (page9.caninput_sel[channel] & MASK_SELECTORB) == SELECTORB_DIGITAL)
                || (((page9.enable_secondarySerial == 0) && ( (page9.enable_intcan == 1) && (page9.intcan_available == 0) )) && (page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_DIGITAL)
                || (((page9.enable_secondarySerial == 0) && (page9.enable_intcan == 0)) && ((page9.caninput_sel[channel] & MASK_SELECTORA) == SELECTORA_DIGITAL)))
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