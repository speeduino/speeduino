#ifndef IDLE_H
#define IDLE_H

#include "globals.h"
#include "table.h"
#include BOARD_H //Note that this is not a real file, it is defined in globals.h. 

#define IAC_ALGORITHM_NONE    0
#define IAC_ALGORITHM_ONOFF   1
#define IAC_ALGORITHM_PWM_OL  2
#define IAC_ALGORITHM_PWM_CL  3
#define IAC_ALGORITHM_STEP_OL 4
#define IAC_ALGORITHM_STEP_CL 5
#define IAC_ALGORITHM_PWM_OLCL  6 //Openloop plus closedloop IAC control

#define STEPPER_FORWARD 0
#define STEPPER_BACKWARD 1
#define IDLE_TABLE_SIZE 10

enum StepperStatus {SOFF, STEPPING, COOLING}; //The 2 statuses that a stepper can have. STEPPING means that a high pulse is currently being sent and will need to be turned off at some point.

struct StepperIdle
{
  int16_t curIdleStep; //Tracks the current location of the stepper
  int16_t targetIdleStep; //What the targetted step is
  volatile StepperStatus stepperStatus;
  volatile unsigned long stepStartTime; //The time the curren
  uint8_t lessAirDirection;
  uint8_t moreAirDirection;
};

struct table2D iacClosedLoopTable;
struct table2D iacPWMTable;
struct table2D iacStepTable;
//Open loop tables specifically for cranking
struct table2D iacCrankStepsTable;
struct table2D iacCrankDutyTable;

struct StepperIdle idleStepper;
bool idleOn; //Simply tracks whether idle was on last time around
uint8_t idleInitComplete = 99; //TRacks which idle method was initialised. 99 is a method that will never exist
uint16_t iacStepTime_uS;
uint16_t iacCoolTime_uS;
uint16_t completedHomeSteps;

volatile PORT_TYPE *idle_pin_port;
volatile PINMASK_TYPE idle_pin_mask;
volatile PORT_TYPE *idle2_pin_port;
volatile PINMASK_TYPE idle2_pin_mask;
volatile PORT_TYPE *idleUpOutput_pin_port;
volatile PINMASK_TYPE idleUpOutput_pin_mask;

volatile bool idle_pwm_state;
uint16_t idle_pwm_max_count; //Used for variable PWM frequency
volatile uint16_t idle_pwm_cur_value;
long idle_pid_target_value;
long FeedForwardTerm;
unsigned long idle_pwm_target_value;
long idle_cl_target_rpm;
uint8_t idleCounter; //Used for tracking the number of calls to the idle control function

uint8_t idleUpOutputHIGH = HIGH; // Used to invert the idle Up Output 
uint8_t idleUpOutputLOW = LOW;   // Used to invert the idle Up Output 

void initialiseIdle();
void initialiseIdleUpOutput();
static inline void disableIdle();
static inline void enableIdle();
static inline uint8_t isStepperHomed();
static inline uint8_t checkForStepping();
static inline void doStep();
static inline void idleInterrupt();

#endif
