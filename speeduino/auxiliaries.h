#ifndef AUX_H
#define AUX_H

void initialiseAuxPWM();
void boostControl();
void vvtControl();
void initialiseFan();

volatile byte *boost_pin_port;
volatile byte boost_pin_mask;
volatile byte *vvt_pin_port;
volatile byte vvt_pin_mask;

volatile bool boost_pwm_state;
unsigned int boost_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int boost_pwm_cur_value;
long boost_pwm_target_value;
long boost_cl_target_boost;
byte boostCounter;
//Boost control uses a scaling factor of 100 on the MAP reading and MAP target in order to have a reasonable response time
//These are the values that are passed to the PID controller
long MAPx100;
long boostTargetx100;

volatile bool vvt_pwm_state;
unsigned int vvt_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int vvt_pwm_cur_value;
long vvt_pwm_target_value;


#endif
