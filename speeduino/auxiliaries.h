#ifndef AUX_H
#define AUX_H

#include "config_pages.h"

void initialiseAuxPWM(void);
void boostControl(void);
void boostDisable(void);
void vvtControl(void);
void initialiseFan(uint8_t fanPin);
void initialiseAirCon(void);
bool initialiseFuelPump(const config2 &page2, uint8_t pumpPin);
void nitrousControl(void);
void fanControl(void);
void airConControl(void);
void wmiControl(void);

void vvt1On(void);
void vvt1Off(void);
void vvt2On(void);
void vvt2Off(void);

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