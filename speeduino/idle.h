#ifndef IDLE_H
#define IDLE_H

#include <stdint.h>

extern uint16_t idle_pwm_max_count; //Used for variable PWM frequency

void initialiseIdle(bool forcehoming);
void idleControl(void);
void disableIdle(void);
void idleInterrupt(void);

#endif
