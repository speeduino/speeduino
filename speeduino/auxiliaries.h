#ifndef AUX_H
#define AUX_H

#include BOARD_H //Note that this is not a real file, it is defined in globals.h. 

void initialiseAuxPWM();
void boostControl();
void boostDisable();
void idleControl();
void vvtControl();
void initialiseFan();
void nitrousControl();
void fanControl();

#define SIMPLE_BOOST_P  1
#define SIMPLE_BOOST_I  1
#define SIMPLE_BOOST_D  1

#define BOOST_PIN_LOW()  *boost_pin_port &= ~(boost_pin_mask)
#define BOOST_PIN_HIGH() *boost_pin_port |= (boost_pin_mask)
#define VVT_PIN_LOW()    *vvt_pin_port &= ~(vvt_pin_mask)
#define VVT_PIN_HIGH()   *vvt_pin_port |= (vvt_pin_mask)
#define FAN_PIN_LOW()    *fan_pin_port &= ~(fan_pin_mask)
#define FAN_PIN_HIGH()   *fan_pin_port |= (fan_pin_mask)
#define N2O_STAGE1_PIN_LOW()  *n2o_stage1_pin_port &= ~(n2o_stage1_pin_mask)
#define N2O_STAGE1_PIN_HIGH() *n2o_stage1_pin_port |= (n2o_stage1_pin_mask)
#define N2O_STAGE2_PIN_LOW()  *n2o_stage2_pin_port &= ~(n2o_stage2_pin_mask)
#define N2O_STAGE2_PIN_HIGH() *n2o_stage2_pin_port |= (n2o_stage2_pin_mask)
#define READ_N2O_ARM_PIN()    ((*n2o_arming_pin_port & n2o_arming_pin_mask) ? true : false)

extern volatile PORT_TYPE *boost_pin_port;
extern volatile PINMASK_TYPE boost_pin_mask;
extern volatile PORT_TYPE *vvt_pin_port;
extern volatile PINMASK_TYPE vvt_pin_mask;

extern unsigned int boost_pwm_max_count; //Used for variable PWM frequency
extern unsigned int vvt_pwm_max_count; //Used for variable PWM frequency
void boostInterrupt();
void vvtInterrupt();


#endif
