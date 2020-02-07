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

#define STEPPER_FORWARD 0
#define STEPPER_BACKWARD 1
#define IDLE_TABLE_SIZE 10

enum StepperStatus {SOFF, STEPPING, COOLING}; //The 2 statuses that a stepper can have. STEPPING means that a high pulse is currently being sent and will need to be turned off at some point.

struct StepperIdle
{
  int curIdleStep; //Tracks the current location of the stepper
  int targetIdleStep; //What the targetted step is
  volatile StepperStatus stepperStatus;
  volatile unsigned long stepStartTime; //The time the curren
  byte lessAirDirection;
  byte moreAirDirection;
};


void initialiseIdle();
void disableIdle();
void idleInterrupt();

#endif
