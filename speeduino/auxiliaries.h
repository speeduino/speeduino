#ifndef AUX_H
#define AUX_H

#include "port_pin.h"
#include "atomic.h"

void initialiseAuxPWM(void);
void boostControl(void);
void boostDisable(void);
void vvtControl(void);
void initialiseFan(uint8_t fanPin);
void initialiseAirCon(void);
void nitrousControl(void);
void fanControl(void);
void airConControl(void);
void wmiControl(void);

#if(defined(CORE_TEENSY) || defined(CORE_STM32))

#define VVT1_PIN_LOW()          (digitalWrite(pinVVT_1, LOW))
#define VVT1_PIN_HIGH()         (digitalWrite(pinVVT_1, HIGH))
#define VVT2_PIN_LOW()          (digitalWrite(pinVVT_2, LOW))
#define VVT2_PIN_HIGH()         (digitalWrite(pinVVT_2, HIGH))

#else

extern port_register_t vvt1_pin_port;
extern pin_mask_t vvt1_pin_mask;
extern port_register_t vvt2_pin_port;
extern pin_mask_t vvt2_pin_mask;

#define VVT1_PIN_LOW()          ATOMIC() { *vvt1_pin_port &= ~(vvt1_pin_mask);   }
#define VVT1_PIN_HIGH()         ATOMIC() { *vvt1_pin_port |= (vvt1_pin_mask);    }
#define VVT2_PIN_LOW()          ATOMIC() { *vvt2_pin_port &= ~(vvt2_pin_mask);   }
#define VVT2_PIN_HIGH()         ATOMIC() { *vvt2_pin_port |= (vvt2_pin_mask);    }

#endif

void fuelPumpOn(void);
void fuelPumpOff(void);

void fanOn(void);
void fanOff(void);

#if defined(PWM_FAN_AVAILABLE)//PWM fan not available on Arduino MEGA
extern uint16_t fan_pwm_max_count; //Used for variable PWM frequency
void fanInterrupt(void);
#endif

extern uint16_t vvt_pwm_max_count; //Used for variable PWM frequency
extern uint16_t boost_pwm_max_count; //Used for variable PWM frequency

void boostInterrupt(void);
void vvtInterrupt(void);

#endif