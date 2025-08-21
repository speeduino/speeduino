#ifndef VVT_H
#define VVT_H

#include "globals.h"

void initialiseVVT(void);
void vvtControl(void);
void vvtInterrupt(void);

#if(defined(CORE_TEENSY) || defined(CORE_STM32))
  #define VVT1_PIN_LOW()          (digitalWrite(pinVVT_1, LOW))
  #define VVT1_PIN_HIGH()         (digitalWrite(pinVVT_1, HIGH))
  #define VVT2_PIN_LOW()          (digitalWrite(pinVVT_2, LOW))
  #define VVT2_PIN_HIGH()         (digitalWrite(pinVVT_2, HIGH))
#else
  #define VVT1_PIN_LOW()          ATOMIC() { *vvt1_pin_port &= ~(vvt1_pin_mask);   }
  #define VVT1_PIN_HIGH()         ATOMIC() { *vvt1_pin_port |= (vvt1_pin_mask);    }
  #define VVT2_PIN_LOW()          ATOMIC() { *vvt2_pin_port &= ~(vvt2_pin_mask);   }
  #define VVT2_PIN_HIGH()         ATOMIC() { *vvt2_pin_port |= (vvt2_pin_mask);    }
#endif

#define VVT1_PIN_ON()     VVT1_PIN_HIGH();
#define VVT1_PIN_OFF()    VVT1_PIN_LOW();
#define VVT2_PIN_ON()     VVT2_PIN_HIGH();
#define VVT2_PIN_OFF()    VVT2_PIN_LOW();
#define VVT_TIME_DELAY_MULTIPLIER  50

extern volatile PORT_TYPE *vvt1_pin_port;
extern volatile PINMASK_TYPE vvt1_pin_mask;
extern volatile PORT_TYPE *vvt2_pin_port;
extern volatile PINMASK_TYPE vvt2_pin_mask;
extern uint16_t vvt_pwm_max_count; //Used for variable PWM frequency
extern long vvt2_pwm_value;
extern volatile bool vvt2_max_pwm;
extern volatile bool vvt2_pwm_state;

#endif