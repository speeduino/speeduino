#include "auxChannelController.h"
#include "src/pins/pinMapping.h"
#include "unit_testing.h"
#include "sensors.h"
#include "comms_secondary.h"
#include "globals.h"

TESTABLE_STATIC bool _auxEnabled = false;

void __attribute__((optimize("Os"))) initAuxChannels(statuses &current, const config9 &page9)
{
    //The following checks the aux inputs and initialises pins if required
    _auxEnabled = false;
    for (uint8_t AuxinChan = 0U; AuxinChan <16U ; AuxinChan++)
    {
        current.current_caninchannel = AuxinChan;                   
        if (((page9.caninput_sel[current.current_caninchannel]&12U) == 4U)
        && ((page9.enable_secondarySerial == 1U) || ((page9.enable_intcan == 1U) && (page9.intcan_available == 1U))))
        { //if current input channel is enabled as external input in caninput_selxb(bits 2:3) and secondary serial or internal canbus is enabled(and is mcu supported)                 
            _auxEnabled = true;
        }
        else if ((((page9.enable_secondarySerial == 1U) || ((page9.enable_intcan == 1U) && (page9.intcan_available == 1U))) && (page9.caninput_sel[current.current_caninchannel]&12U) == 8U)
                || (((page9.enable_secondarySerial == 0U) && ( (page9.enable_intcan == 1U) && (page9.intcan_available == 0U) )) && (page9.caninput_sel[current.current_caninchannel]&3U) == 2U)  
                || (((page9.enable_secondarySerial == 0U) && (page9.enable_intcan == 0U)) && ((page9.caninput_sel[current.current_caninchannel]&3U) == 2U)))  
        {  //if current input channel is enabled as analog local pin check caninput_selxb(bits 2:3) with &12 and caninput_selxa(bits 0:1) with &3
            uint8_t pinNumber = pinTranslateAnalog(page9.Auxinpina[current.current_caninchannel]&63U);
            if( pinIsUsed(pinNumber) )
            {
                //Do nothing here as the pin is already in use.
                current.ioError = true; //Tell user that there is problem by lighting up the I/O error indicator
            }
            else
            {
                //Channel is active and analog
                pinMode( pinNumber, INPUT);
                _auxEnabled = true;
            }  
        }
        else if ((((page9.enable_secondarySerial == 1U) || ((page9.enable_intcan == 1U) && (page9.intcan_available == 1U))) && (page9.caninput_sel[current.current_caninchannel]&12U) == 12U)
                || (((page9.enable_secondarySerial == 0U) && ( (page9.enable_intcan == 1U) && (page9.intcan_available == 0U) )) && (page9.caninput_sel[current.current_caninchannel]&3U) == 3U)
                || (((page9.enable_secondarySerial == 0U) && (page9.enable_intcan == 0U)) && ((page9.caninput_sel[current.current_caninchannel]&3U) == 3U)))
        {  //if current input channel is enabled as digital local pin check caninput_selxb(bits 2:3) with &12 and caninput_selxa(bits 0:1) with &3
            uint8_t pinNumber = (page9.Auxinpinb[current.current_caninchannel]&63U) + 1U;
            if( pinIsUsed(pinNumber) )
            {
                //Do nothing here as the pin is already in use.
                current.ioError = true; //Tell user that there is problem by lighting up the I/O error indicator
            }
            else
            {
                //Channel is active and digital
                pinMode( pinNumber, INPUT);
                _auxEnabled = true;
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
    for (byte AuxinChan = 0; AuxinChan <16 ; AuxinChan++)
    {
        current.current_caninchannel = AuxinChan;          
        
        if (((page9.caninput_sel[current.current_caninchannel]&12) == 4) 
            && (((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 0)&&(page9.intcan_available == 1)))
            || ((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 1))&& 
            ((page9.caninput_sel[current.current_caninchannel]&64) == 0))
            || ((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 0)))))              
        { //if current input channel is enabled as external & secondary serial enabled & internal can disabled(but internal can is available)
        // or current input channel is enabled as external & secondary serial enabled & internal can enabled(and internal can is available)
        //current.canin[13] = 11;  Dev test use only!
        if (page9.enable_secondarySerial == 1)  // megas only support can via secondary serial
        {
            fnSendCanCommand(2,0,current.current_caninchannel,0,((page9.caninput_source_can_address[current.current_caninchannel]&2047)+0x100));
            //send an R command for data from caninput_source_address[current.current_caninchannel] from secondarySerial
        }
        }  
        else if (((page9.caninput_sel[current.current_caninchannel]&12) == 4) 
            && (((page9.enable_secondarySerial == 1) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 1))&& 
            ((page9.caninput_sel[current.current_caninchannel]&64) == 64))
            || ((page9.enable_secondarySerial == 0) && ((page9.enable_intcan == 1)&&(page9.intcan_available == 1))&& 
            ((page9.caninput_sel[current.current_caninchannel]&128) == 128))))                             
        { //if current input channel is enabled as external for canbus & secondary serial enabled & internal can enabled(and internal can is available)
        // or current input channel is enabled as external for canbus & secondary serial disabled & internal can enabled(and internal can is available)
        //current.canin[13] = 12;  Dev test use only!  
        #if defined(CORE_STM32) || defined(CORE_TEENSY)
        if (page9.enable_intcan == 1) //  if internal can is enabled 
        {
            fnSendCanCommand(3,page9.speeduino_tsCanId,current.current_caninchannel,0,((page9.caninput_source_can_address[current.current_caninchannel]&2047)+0x100));  
            //send an R command for data from caninput_source_address[current.current_caninchannel] from internal canbus
        }
        #endif
        }   
        else if ((((page9.enable_secondarySerial == 1) || ((page9.enable_intcan == 1) && (page9.intcan_available == 1))) && (page9.caninput_sel[current.current_caninchannel]&12) == 8)
                || (((page9.enable_secondarySerial == 0) && ( (page9.enable_intcan == 1) && (page9.intcan_available == 0) )) && (page9.caninput_sel[current.current_caninchannel]&3) == 2)  
                || (((page9.enable_secondarySerial == 0) && (page9.enable_intcan == 0)) && ((page9.caninput_sel[current.current_caninchannel]&3) == 2)))  
        { //if current input channel is enabled as analog local pin
        //read analog channel specified
        //current.canin[13] = (page9.Auxinpina[current.current_caninchannel]&63);  Dev test use only!127
        current.canin[current.current_caninchannel] = fnReadAuxanalog(pinTranslateAnalog(page9.Auxinpina[current.current_caninchannel]&63));
        }
        else if ((((page9.enable_secondarySerial == 1) || ((page9.enable_intcan == 1) && (page9.intcan_available == 1))) && (page9.caninput_sel[current.current_caninchannel]&12) == 12)
                || (((page9.enable_secondarySerial == 0) && ( (page9.enable_intcan == 1) && (page9.intcan_available == 0) )) && (page9.caninput_sel[current.current_caninchannel]&3) == 3)
                || (((page9.enable_secondarySerial == 0) && (page9.enable_intcan == 0)) && ((page9.caninput_sel[current.current_caninchannel]&3) == 3)))
        { //if current input channel is enabled as digital local pin
        //read digital channel specified
        //current.canin[14] = ((page9.Auxinpinb[current.current_caninchannel]&63)+1);  Dev test use only!127+1
        current.canin[current.current_caninchannel] = fnReadAuxdigital((page9.Auxinpinb[current.current_caninchannel]&63)+1);
        } //Channel type
    } //For loop going through each channel
}

// LCOV_EXCL_START
void auxChannelControl(statuses &current, const config9 &page9)
{
    if(_auxEnabled
    && BIT_CHECK(current.LOOP_TIMER, BIT_TIMER_4HZ))
    {
        auxChannelControl(current, page9, sendCancommand, readAnalogSensor, digitalRead);
    }
}
// LCOV_EXCL_STOP