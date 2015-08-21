#define STEPPER_FORWARD 1
#define STEPPER_BACKWARD 0
#define IDLE_TABLE_SIZE 10
#define DRV8825_STEP_TIME 800 //The time in uS between steps for the DRV8825

enum StepperStatus {SOFF, STEPPING}; //The 2 statuses that a stepper can have. STEPPING means that a high pulse is currently being sent and will need to be turned off at some point.

struct StepperIdle
{
  int curIdleStep; //Tracks the current location of the stepper
  int targetIdleStep; //What the targetted step is
  volatile StepperStatus stepperStatus; 
  volatile unsigned long stepStartTime; //The time the curren
};

struct table2D iacClosedLoopTable;
struct table2D iacPWMTable;
struct table2D iacStepTable;
//Open loop tables specifically for cranking
struct table2D iacCrankStepsTable;
struct table2D iacCrankDutyTable;

struct StepperIdle idleStepper;
bool idleOn; //Simply tracks whether idle was on last time around

void initialiseIdle();
