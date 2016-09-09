

volatile byte *boost_pin_port;
volatile byte boost_pin_mask;
volatile byte *vvt_pin_port;
volatile byte vvt_pin_mask;

volatile bool boost_pwm_state;
unsigned int boost_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int boost_pwm_cur_value;
long boost_pwm_target_value;
long boost_cl_target_boost;

volatile bool vvt_pwm_state;
unsigned int vvt_pwm_max_count; //Used for variable PWM frequency
volatile unsigned int vvt_pwm_cur_value;
long vvt_pwm_target_value;


