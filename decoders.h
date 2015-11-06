volatile unsigned long curTime;
volatile unsigned int curGap;
volatile unsigned long curTime2;
volatile unsigned int curGap2;
volatile unsigned int lastGap;
volatile unsigned int targetGap; 

volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastSecToothTime = 0; //The time (micros()) that the last tooth was registered on the secondary input
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered
volatile unsigned long toothOneTime = 0; //The time (micros()) that tooth 1 last triggered
volatile unsigned long toothOneMinusOneTime = 0; //The 2nd to last time (micros()) that tooth 1 last triggered
volatile unsigned int toothHistory[TOOTH_LOG_BUFFER];
volatile unsigned int toothHistoryIndex = 0;
volatile long toothDeltaV; //Represents the change in time taken between the last 2 teeth compared to the previous 2. Positive value represents accleration, negative = deccleration

volatile byte secondaryToothCount; //Used for identifying the current secondary (Usually cam) tooth for patterns with multiple secondary teeth
volatile unsigned long secondaryLastToothTime = 0; //The time (micros()) that the last tooth was registered (Cam input)

volatile int triggerActualTeeth;
volatile unsigned long triggerFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
unsigned int triggerSecFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the secondary input
int triggerToothAngle; //The number of crank degrees that elapse per tooth
unsigned long revolutionTime; //The time in uS that one revolution would take at current speed (The time tooth 1 was last seen, minus the time it was seen prior to that)

int toothAngles[24]; //An array for storing fixed tooth angles. Currently sized at 24 for the GM 24X decoder, but may grow later if there are other decoders that use this style

//Used for identifying long and short pulses on the 4G63 (And possibly other) trigger patterns
#define LONG 0;
#define SHORT 1;
