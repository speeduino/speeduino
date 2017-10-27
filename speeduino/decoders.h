#ifndef DECODERS_H
#define DECODERS_H

#include <limits.h>

#if defined(CORE_AVR)
  #define READ_PRI_TRIGGER() ((*triggerPri_pin_port & triggerPri_pin_mask) ? true : false)
  #define READ_SEC_TRIGGER() ((*triggerSec_pin_port & triggerSec_pin_mask) ? true : false)
#elif defined(CORE_TEENSY) || defined(CORE_STM32)
  #define READ_PRI_TRIGGER() digitalRead(pinTrigger)
  #define READ_SEC_TRIGGER() digitalRead(pinTrigger2)
#endif

static inline void addToothLogEntry(unsigned long);
static inline uint16_t stdGetRPM();
static inline void setFilter(unsigned long);
static inline int crankingGetRPM(byte);
static inline void doPerToothTiming(uint16_t crankAngle);

void triggerSetup_missingTooth();
void triggerPri_missingTooth();
void triggerSec_missingTooth();
uint16_t getRPM_missingTooth();
int getCrankAngle_missingTooth(int timePerDegree);
void triggerSetup_DualWheel();
void triggerPri_DualWheel();
void triggerSec_DualWheel();
uint16_t getRPM_DualWheel();
int getCrankAngle_DualWheel(int timePerDegree);

unsigned long MAX_STALL_TIME = 500000UL; //The maximum time (in uS) that the system will continue to function before the engine is considered stalled/stopped. This is unique to each decoder, depending on the number of teeth etc. 500000 (half a second) is used as the default value, most decoders will be much less.

volatile unsigned long curTime;
volatile unsigned long curGap;
volatile unsigned long curTime2;
volatile unsigned long curGap2;
volatile unsigned long lastGap;
volatile unsigned long targetGap;

volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile byte toothSystemCount = 0; //Used for decoders such as Audi 135 where not every tooth is used for calculating crank angle. This variable stores the actual number of teeth, not the number being used to calculate crank angle
volatile unsigned long toothSystemLastToothTime = 0; //As below, but used for decoders where not every tooth count is used for calculation
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastSecToothTime = 0; //The time (micros()) that the last tooth was registered on the secondary input
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered
volatile unsigned long toothOneTime = 0; //The time (micros()) that tooth 1 last triggered
volatile unsigned long toothOneMinusOneTime = 0; //The 2nd to last time (micros()) that tooth 1 last triggered
volatile bool revolutionOne = 0; // For sequential operation, this tracks whether the current revolution is 1 or 2 (not 1)
volatile uint16_t toothHistory[TOOTH_LOG_BUFFER];
volatile unsigned int toothHistoryIndex = 0;
volatile bool toothLogRead = false; //Flag to indicate whether the current tooth log values have been read out yet

volatile unsigned int secondaryToothCount; //Used for identifying the current secondary (Usually cam) tooth for patterns with multiple secondary teeth
volatile unsigned long secondaryLastToothTime = 0; //The time (micros()) that the last tooth was registered (Cam input)
volatile unsigned long secondaryLastToothTime1 = 0; //The time (micros()) that the last tooth was registered (Cam input)

volatile int triggerActualTeeth;
volatile unsigned long triggerFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
unsigned long triggerSecFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the secondary input
unsigned int triggerSecFilterTime_duration; // The shortest valid time (in uS) pulse DURATION
volatile int triggerToothAngle; //The number of crank degrees that elapse per tooth
bool secondDerivEnabled; //The use of the 2nd derivative calculation is limited to certain decoders. This is set to either true or false in each decoders setup routine
bool decoderIsSequential; //Whether or not the decoder supports sequential operation
bool decoderIsLowRes = false; //Is set true, certain extra calculations are performed for better timing accuracy
bool decoderHasFixedCrankingTiming = false; //Whether or not the decoder supports fixed cranking timing
byte checkSyncToothCount; //How many teeth must've been seen on this revolution before we try to confirm sync (Useful for missing tooth type decoders)

int16_t ignition1EndTooth = 0;
int16_t ignition2EndTooth = 0;
int16_t ignition3EndTooth = 0;
int16_t ignition4EndTooth = 0;

int16_t toothAngles[24]; //An array for storing fixed tooth angles. Currently sized at 24 for the GM 24X decoder, but may grow later if there are other decoders that use this style

//Used for identifying long and short pulses on the 4G63 (And possibly other) trigger patterns
#define LONG 0;
#define SHORT 1;

#endif
