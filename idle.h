#define STEPPER_FORWARD 1
#define STEPPER_BACKWARD 0

enum StepperStatus {SOFF, STEPPING}; //The 2 statuses that a stepper can have. STEPPING means that a high pulse is currently being sent and will need to be turned off at some point.

struct StepperIdle
{
  int curIdleStep; //Tracks the current location of the stepper
  int targetIdleStep; //What the targetted step is
  volatile StepperStatus stepperStatus; 
  volatile unsigned long stepStartTime; //The time the curren
};


void initialiseIdle();
