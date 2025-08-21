
#ifndef AIRCON_H
#define AIRCON_H

#include "globals.h"

void initialiseAirCon(void);
void airConControl(void);
bool READ_AIRCON_REQUEST(void);


#if(defined(CORE_TEENSY) || defined(CORE_STM32))
    #define AIRCON_PIN_LOW()        (digitalWrite(pinAirConComp, LOW))
    #define AIRCON_PIN_HIGH()       (digitalWrite(pinAirConComp, HIGH))
    #define AIRCON_FAN_PIN_LOW()    (digitalWrite(pinAirConFan, LOW))
    #define AIRCON_FAN_PIN_HIGH()   (digitalWrite(pinAirConFan, HIGH))
#else
    //Note the below macros cannot use ATOMIC() as they are called from within ternary operators. The ATOMIC is instead placed around the ternary call below
    #define AIRCON_PIN_LOW()        *aircon_comp_pin_port &= ~(aircon_comp_pin_mask)
    #define AIRCON_PIN_HIGH()       *aircon_comp_pin_port |= (aircon_comp_pin_mask)
    #define AIRCON_FAN_PIN_LOW()    *aircon_fan_pin_port &= ~(aircon_fan_pin_mask)
    #define AIRCON_FAN_PIN_HIGH()   *aircon_fan_pin_port |= (aircon_fan_pin_mask)
#endif

#define AIRCON_ON()             ATOMIC() { ((((configPage15.airConCompPol)==1)) ? AIRCON_PIN_LOW() : AIRCON_PIN_HIGH()); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_OFF()            ATOMIC() { ((((configPage15.airConCompPol)==1)) ? AIRCON_PIN_HIGH() : AIRCON_PIN_LOW()); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_COMPRESSOR); }
#define AIRCON_FAN_ON()         ATOMIC() { ((((configPage15.airConFanPol)==1)) ? AIRCON_FAN_PIN_LOW() : AIRCON_FAN_PIN_HIGH()); BIT_SET(currentStatus.airConStatus, BIT_AIRCON_FAN); }
#define AIRCON_FAN_OFF()        ATOMIC() { ((((configPage15.airConFanPol)==1)) ? AIRCON_FAN_PIN_HIGH() : AIRCON_FAN_PIN_LOW()); BIT_CLEAR(currentStatus.airConStatus, BIT_AIRCON_FAN); }

#endif