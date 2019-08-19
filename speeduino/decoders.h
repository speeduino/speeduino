#ifndef DECODERS_H
#define DECODERS_H

#if defined(CORE_AVR)
  #define READ_PRI_TRIGGER() ((*triggerPri_pin_port & triggerPri_pin_mask) ? true : false)
  #define READ_SEC_TRIGGER() ((*triggerSec_pin_port & triggerSec_pin_mask) ? true : false)
#else
  #define READ_PRI_TRIGGER() digitalRead(pinTrigger)
  #define READ_SEC_TRIGGER() digitalRead(pinTrigger2)
#endif

static inline void addToothLogEntry(unsigned long, bool);
void loggerPrimaryISR();
void loggerSecondaryISR();
static inline uint16_t stdGetRPM(uint16_t);
static inline void setFilter(unsigned long);
static inline int crankingGetRPM(byte);
//static inline void doPerToothTiming(uint16_t);

void (*triggerHandler)(); //Pointer for the trigger function (Gets pointed to the relevant decoder)
void (*triggerSecondaryHandler)(); //Pointer for the secondary trigger function (Gets pointed to the relevant decoder)
uint16_t (*getRPM)(); //Pointer to the getRPM function (Gets pointed to the relevant decoder)
int (*getCrankAngle)(); //Pointer to the getCrank Angle function (Gets pointed to the relevant decoder)
void (*triggerSetEndTeeth)(); //Pointer to the triggerSetEndTeeth function of each decoder

//All of the below are the 6 required functions for each decoder / pattern
void triggerSetup_missingTooth();
void triggerPri_missingTooth();
void triggerSec_missingTooth();
uint16_t getRPM_missingTooth();
int getCrankAngle_missingTooth();
void triggerSetEndTeeth_missingTooth();

void triggerSetup_DualWheel();
void triggerPri_DualWheel();
void triggerSec_DualWheel();
uint16_t getRPM_DualWheel();
int getCrankAngle_DualWheel();
void triggerSetEndTeeth_DualWheel();

void triggerSetup_BasicDistributor();
void triggerPri_BasicDistributor();
void triggerSec_BasicDistributor();
uint16_t getRPM_BasicDistributor();
int getCrankAngle_BasicDistributor();
void triggerSetEndTeeth_BasicDistributor();

void triggerSetup_GM7X();
void triggerPri_GM7X();
void triggerSec_GM7X();
uint16_t getRPM_GM7X();
int getCrankAngle_GM7X();
void triggerSetEndTeeth_GM7X();

void triggerSetup_4G63();
void triggerPri_4G63();
void triggerSec_4G63();
uint16_t getRPM_4G63();
int getCrankAngle_4G63();
void triggerSetEndTeeth_4G63();

void triggerSetup_24X();
void triggerPri_24X();
void triggerSec_24X();
uint16_t getRPM_24X();
int getCrankAngle_24X();
void triggerSetEndTeeth_24X();

void triggerSetup_Jeep2000();
void triggerPri_Jeep2000();
void triggerSec_Jeep2000();
uint16_t getRPM_Jeep2000();
int getCrankAngle_Jeep2000();
void triggerSetEndTeeth_Jeep2000();

void triggerSetup_Audi135();
void triggerPri_Audi135();
void triggerSec_Audi135();
uint16_t getRPM_Audi135();
int getCrankAngle_Audi135();
void triggerSetEndTeeth_Audi135();

void triggerSetup_HondaD17();
void triggerPri_HondaD17();
void triggerSec_HondaD17();
uint16_t getRPM_HondaD17();
int getCrankAngle_HondaD17();
void triggerSetEndTeeth_HondaD17();

void triggerSetup_Miata9905();
void triggerPri_Miata9905();
void triggerSec_Miata9905();
uint16_t getRPM_Miata9905();
int getCrankAngle_Miata9905();
void triggerSetEndTeeth_Miata9905();

void triggerSetup_MazdaAU();
void triggerPri_MazdaAU();
void triggerSec_MazdaAU();
uint16_t getRPM_MazdaAU();
int getCrankAngle_MazdaAU();
void triggerSetEndTeeth_MazdaAU();

void triggerSetup_non360();
void triggerPri_non360();
void triggerSec_non360();
uint16_t getRPM_non360();
int getCrankAngle_non360();
void triggerSetEndTeeth_non360();

void triggerSetup_Nissan360();
void triggerPri_Nissan360();
void triggerSec_Nissan360();
uint16_t getRPM_Nissan360();
int getCrankAngle_Nissan360();
void triggerSetEndTeeth_Nissan360();

void triggerSetup_Subaru67();
void triggerPri_Subaru67();
void triggerSec_Subaru67();
uint16_t getRPM_Subaru67();
int getCrankAngle_Subaru67();
void triggerSetEndTeeth_Subaru67();

void triggerSetup_Daihatsu();
void triggerPri_Daihatsu();
void triggerSec_Daihatsu();
uint16_t getRPM_Daihatsu();
int getCrankAngle_Daihatsu();
void triggerSetEndTeeth_Daihatsu();

void triggerSetup_Harley();
void triggerPri_Harley();
void triggerSec_Harley();
uint16_t getRPM_Harley();
int getCrankAngle_Harley();
void triggerSetEndTeeth_Harley();

void triggerSetup_ThirtySixMinus222();
void triggerPri_ThirtySixMinus222();
void triggerSec_ThirtySixMinus222();
uint16_t getRPM_ThirtySixMinus222();
int getCrankAngle_ThirtySixMinus222();
void triggerSetEndTeeth_ThirtySixMinus222();

unsigned long MAX_STALL_TIME = 500000UL; //The maximum time (in uS) that the system will continue to function before the engine is considered stalled/stopped. This is unique to each decoder, depending on the number of teeth etc. 500000 (half a second) is used as the default value, most decoders will be much less.

volatile unsigned long curTime;
volatile unsigned long curGap;
volatile unsigned long curTime2;
volatile unsigned long curGap2;
volatile unsigned long lastGap;
volatile unsigned long targetGap;
volatile unsigned long compositeLastToothTime;

volatile int toothCurrentCount = 0; //The current number of teeth (Onec sync has been achieved, this can never actually be 0
volatile byte toothSystemCount = 0; //Used for decoders such as Audi 135 where not every tooth is used for calculating crank angle. This variable stores the actual number of teeth, not the number being used to calculate crank angle
volatile unsigned long toothSystemLastToothTime = 0; //As below, but used for decoders where not every tooth count is used for calculation
volatile unsigned long toothLastToothTime = 0; //The time (micros()) that the last tooth was registered
volatile unsigned long toothLastSecToothTime = 0; //The time (micros()) that the last tooth was registered on the secondary input
volatile unsigned long toothLastMinusOneToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered
#ifndef SMALL_FLASH_MODE
volatile unsigned long toothLastMinusOneSecToothTime = 0; //The time (micros()) that the tooth before the last tooth was registered on secondary input
volatile unsigned long targetGap2;
#endif
volatile unsigned long toothOneTime = 0; //The time (micros()) that tooth 1 last triggered
volatile unsigned long toothOneMinusOneTime = 0; //The 2nd to last time (micros()) that tooth 1 last triggered
volatile bool revolutionOne = 0; // For sequential operation, this tracks whether the current revolution is 1 or 2 (not 1)

volatile unsigned int secondaryToothCount; //Used for identifying the current secondary (Usually cam) tooth for patterns with multiple secondary teeth
volatile unsigned long secondaryLastToothTime = 0; //The time (micros()) that the last tooth was registered (Cam input)
volatile unsigned long secondaryLastToothTime1 = 0; //The time (micros()) that the last tooth was registered (Cam input)

volatile int triggerActualTeeth;
volatile unsigned long triggerFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
volatile unsigned long triggerSecFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the secondary input
volatile bool validTrigger; //Is set true when the last trigger (Primary or secondary) was valid (ie passed filters)
unsigned int triggerSecFilterTime_duration; // The shortest valid time (in uS) pulse DURATION
volatile uint16_t triggerToothAngle; //The number of crank degrees that elapse per tooth
volatile bool triggerToothAngleIsCorrect = false; //Whether or not the triggerToothAngle variable is currently accurate. Some patterns have times when the triggerToothAngle variable cannot be accurately set.
bool secondDerivEnabled = false; //The use of the 2nd derivative calculation is limited to certain decoders. This is set to either true or false in each decoders setup routine
bool decoderIsSequential; //Whether or not the decoder supports sequential operation
bool decoderIsLowRes = false; //Is set true, certain extra calculations are performed for better timing accuracy
bool decoderHasSecondary = false; //Whether or not the pattern uses a secondary input
bool decoderHasFixedCrankingTiming = false; //Whether or not the decoder supports fixed cranking timing
byte checkSyncToothCount; //How many teeth must've been seen on this revolution before we try to confirm sync (Useful for missing tooth type decoders)
unsigned long elapsedTime;
unsigned long lastCrankAngleCalc;
int16_t lastToothCalcAdvance = 99; //Invalid value here forces calculation of this on first main loop
unsigned long lastVVTtime; //The time between the vvt reference pulse and the last crank pulse

int16_t ignition1EndTooth = 0;
int16_t ignition2EndTooth = 0;
int16_t ignition3EndTooth = 0;
int16_t ignition4EndTooth = 0;

int16_t toothAngles[24]; //An array for storing fixed tooth angles. Currently sized at 24 for the GM 24X decoder, but may grow later if there are other decoders that use this style

//Used for identifying long and short pulses on the 4G63 (And possibly other) trigger patterns
#define LONG 0;
#define SHORT 1;

#define CRANK_SPEED 0
#define CAM_SPEED   1

#define TOOTH_CRANK 0
#define TOOTH_CAM   1

#endif
