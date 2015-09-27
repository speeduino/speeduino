

volatile byte *boost_pin_port;
volatile byte boost_pin_mask;
volatile byte *vvt_pin_port;
volatile byte vvt_pin_mask;

volatile bool boost_pwm_state;
unsigned int boost_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int boost_pwm_cur_value;
unsigned int boost_pwm_target_value;

volatile bool vvt_pwm_state;
unsigned int vvt_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int vvt_pwm_cur_value;
unsigned int vvt_pwm_target_value;

