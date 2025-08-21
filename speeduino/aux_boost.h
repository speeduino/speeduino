#ifndef BOOST_H
#define BOOST_H

#include "globals.h"

void initialiseBoost(void);
void boostControl(void);
void boostDisable(void);
void boostByGear(void);

#define SIMPLE_BOOST_P  1
#define SIMPLE_BOOST_I  1
#define SIMPLE_BOOST_D  1

#if(defined(CORE_TEENSY) || defined(CORE_STM32))
    #define BOOST_PIN_LOW()         (digitalWrite(pinBoost, LOW))
    #define BOOST_PIN_HIGH()        (digitalWrite(pinBoost, HIGH))
#else
    #define BOOST_PIN_LOW()         ATOMIC() { *boost_pin_port &= ~(boost_pin_mask); }
    #define BOOST_PIN_HIGH()        ATOMIC() { *boost_pin_port |= (boost_pin_mask);  }
#endif

extern uint16_t boost_pwm_max_count; //Used for variable PWM frequency

void boostInterrupt(void);

#endif