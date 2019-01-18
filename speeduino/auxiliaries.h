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

#if defined(CORE_AVR)
  #define ENABLE_BOOST_TIMER()  TIMSK1 |= (1 << OCIE1A)
  #define DISABLE_BOOST_TIMER() TIMSK1 &= ~(1 << OCIE1A)
  #define ENABLE_VVT_TIMER()    TIMSK1 |= (1 << OCIE1B)
  #define DISABLE_VVT_TIMER()   TIMSK1 &= ~(1 << OCIE1B)

  #define BOOST_TIMER_COMPARE   OCR1A
  #define BOOST_TIMER_COUNTER   TCNT1
  #define VVT_TIMER_COMPARE     OCR1B
  #define VVT_TIMER_COUNTER     TCNT1

#elif defined(CORE_TEENSY)
  

#elif defined(CORE_STM32)
  #if defined(ARDUINO_ARCH_STM32) // STM32GENERIC core
    #define ENABLE_BOOST_TIMER()  (TIM1)->CCER |= TIM_CCER_CC2E
    #define DISABLE_BOOST_TIMER() (TIM1)->CCER &= ~TIM_CCER_CC2E

    #define ENABLE_VVT_TIMER()    (TIM1)->CCER |= TIM_CCER_CC3E
    #define DISABLE_VVT_TIMER()   (TIM1)->CCER &= ~TIM_CCER_CC3E

    #define BOOST_TIMER_COMPARE   (TIM1)->CCR2
    #define BOOST_TIMER_COUNTER   (TIM1)->CNT
    #define VVT_TIMER_COMPARE     (TIM1)->CCR3
    #define VVT_TIMER_COUNTER     (TIM1)->CNT
  #else //libmaple core aka STM32DUINO
    #define ENABLE_BOOST_TIMER()  (TIMER1->regs).gen->CCER |= TIMER_CCER_CC2E
    #define DISABLE_BOOST_TIMER() (TIMER1->regs).gen->CCER &= ~TIMER_CCER_CC2E

    #define ENABLE_VVT_TIMER()    (TIMER1->regs).gen->CCER |= TIMER_CCER_CC3E
    #define DISABLE_VVT_TIMER()   (TIMER1->regs).gen->CCER &= ~TIMER_CCER_CC3E

    #define BOOST_TIMER_COMPARE   (TIMER1->regs).gen->CCR2
    #define BOOST_TIMER_COUNTER   (TIMER1->regs).gen->CNT
    #define VVT_TIMER_COMPARE     (TIMER1->regs).gen->CCR3
    #define VVT_TIMER_COUNTER     (TIMER1->regs).gen->CNT
  #endif
#endif

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

volatile PORT_TYPE *boost_pin_port;
volatile byte boost_pin_mask;
volatile PORT_TYPE *vvt_pin_port;
volatile byte vvt_pin_mask;
volatile PORT_TYPE *fan_pin_port;
volatile byte fan_pin_mask;
volatile PORT_TYPE *n2o_stage1_pin_port;
volatile byte n2o_stage1_pin_mask;
volatile PORT_TYPE *n2o_stage2_pin_port;
volatile byte n2o_stage2_pin_mask;
volatile PORT_TYPE *n2o_arming_pin_port;
volatile byte n2o_arming_pin_mask;

volatile bool boost_pwm_state;
unsigned int boost_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int boost_pwm_cur_value;
long boost_pwm_target_value;
long boost_cl_target_boost;
byte boostCounter;

byte fanHIGH = HIGH;             // Used to invert the cooling fan output
byte fanLOW = LOW;               // Used to invert the cooling fan output

volatile bool vvt_pwm_state;
unsigned int vvt_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int vvt_pwm_cur_value;
long vvt_pwm_target_value;
#if defined (CORE_TEENSY) || defined(CORE_STM32)
  static inline void boostInterrupt();
  static inline void vvtInterrupt();
#endif


#endif
