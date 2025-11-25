#ifndef DECODERS_H
#define DECODERS_H

#include <SimplyAtomic.h>
#include <stdint.h>

#define TRIGGER_FILTER_OFF              0
#define TRIGGER_FILTER_LITE             1
#define TRIGGER_FILTER_MEDIUM           2
#define TRIGGER_FILTER_AGGRESSIVE       3

struct decoder_features_t {
  bool supports2ndDeriv : 1; ///> The use of the 2nd derivative calculation is limited to certain decoders. 
  bool supportsSequential : 1; ///> Whether or not the decoder supports sequential operation
  bool hasSecondary : 1; ///> Whether or not the pattern uses a secondary input
  bool hasFixedCrankingTiming : 1; ///> Whether or not the decoder supports fixed cranking timing

  decoder_features_t(void)
    : supports2ndDeriv(false)
    , supportsSequential(false)
    , hasSecondary(false)
    , hasFixedCrankingTiming(false)
  {}
};

const decoder_features_t& getDecoderFeatures(void);

/** \enum SyncStatus
 * @brief The decoder trigger status
 * */
enum class SyncStatus : uint8_t {
  /** No trigger pulses are being received. Either loss of sync or engine has stopped */
  None, 
  /** Primary & secondary triggers are configured, but we are only receiving pulses from the primary.
   *  *Not a valid state if no secondary trigger is configured* 
   */
  Partial,
  /** We are receiving pulses from both primary & secondary (where specified) triggers */
  Full,
}; 

/** @brief Current decoder status */
struct decoder_status_t {
  bool validTrigger; ///> Is set true when the last trigger (Primary or secondary) was valid (ie passed filters)
  bool toothAngleIsCorrect; ///> Whether or not the triggerToothAngle variable is currently accurate. Some patterns have times when the triggerToothAngle variable cannot be accurately set.ly set.
  SyncStatus syncStatus; ///> Current sync status (none/partial/full)
};

/** @brief Access the current decoder status */
const decoder_status_t getDecoderStatus(void);

/**
 * @brief Is the engine running?
 * 
 * This is based on whether or not the decoder has detected a tooth recently
 * 
 * @param curTime The time in µS to use for the liveness check. Typically the result of a recent call to micros() 
 * @return true If the engine is turning
 * @return false If the engine is not turning
 */
bool engineIsRunning(uint32_t curTime);

void loggerPrimaryISR(void);
void loggerSecondaryISR(void);
void loggerTertiaryISR(void);

//All of the below are the 6 required functions for each decoder / pattern
void triggerSetup_missingTooth(void);
void triggerPri_missingTooth(void);
void triggerSec_missingTooth(void);
void triggerThird_missingTooth(void);
uint16_t getRPM_missingTooth(void);
int getCrankAngle_missingTooth(void);
extern void triggerSetEndTeeth_missingTooth(void);


void triggerSetup_DualWheel(void);
void triggerPri_DualWheel(void);
void triggerSec_DualWheel(void);
uint16_t getRPM_DualWheel(void);
int getCrankAngle_DualWheel(void);
void triggerSetEndTeeth_DualWheel(void);

void triggerSetup_BasicDistributor(void);
void triggerPri_BasicDistributor(void);
void triggerSec_BasicDistributor(void);
uint16_t getRPM_BasicDistributor(void);
int getCrankAngle_BasicDistributor(void);
void triggerSetEndTeeth_BasicDistributor(void);

void triggerSetup_GM7X(void);
void triggerPri_GM7X(void);
void triggerSec_GM7X(void);
uint16_t getRPM_GM7X(void);
int getCrankAngle_GM7X(void);
void triggerSetEndTeeth_GM7X(void);

void triggerSetup_4G63(void);
void triggerPri_4G63(void);
void triggerSec_4G63(void);
uint16_t getRPM_4G63(void);
int getCrankAngle_4G63(void);
void triggerSetEndTeeth_4G63(void);

void triggerSetup_24X(void);
void triggerPri_24X(void);
void triggerSec_24X(void);
uint16_t getRPM_24X(void);
int getCrankAngle_24X(void);
void triggerSetEndTeeth_24X(void);

void triggerSetup_Jeep2000(void);
void triggerPri_Jeep2000(void);
void triggerSec_Jeep2000(void);
uint16_t getRPM_Jeep2000(void);
int getCrankAngle_Jeep2000(void);
void triggerSetEndTeeth_Jeep2000(void);

void triggerSetup_Audi135(void);
void triggerPri_Audi135(void);
void triggerSec_Audi135(void);
uint16_t getRPM_Audi135(void);
int getCrankAngle_Audi135(void);
void triggerSetEndTeeth_Audi135(void);

void triggerSetup_HondaD17(void);
void triggerPri_HondaD17(void);
void triggerSec_HondaD17(void);
uint16_t getRPM_HondaD17(void);
int getCrankAngle_HondaD17(void);
void triggerSetEndTeeth_HondaD17(void);

void triggerSetup_HondaJ32(void);
void triggerPri_HondaJ32(void);
void triggerSec_HondaJ32(void);
uint16_t getRPM_HondaJ32(void);
int getCrankAngle_HondaJ32(void);
void triggerSetEndTeeth_HondaJ32(void);

void triggerSetup_Miata9905(void);
void triggerPri_Miata9905(void);
void triggerSec_Miata9905(void);
uint16_t getRPM_Miata9905(void);
int getCrankAngle_Miata9905(void);
void triggerSetEndTeeth_Miata9905(void);
int getCamAngle_Miata9905(void);

void triggerSetup_MazdaAU(void);
void triggerPri_MazdaAU(void);
void triggerSec_MazdaAU(void);
uint16_t getRPM_MazdaAU(void);
int getCrankAngle_MazdaAU(void);
void triggerSetEndTeeth_MazdaAU(void);

void triggerSetup_non360(void);
void triggerPri_non360(void);
void triggerSec_non360(void);
uint16_t getRPM_non360(void);
int getCrankAngle_non360(void);
void triggerSetEndTeeth_non360(void);

void triggerSetup_Nissan360(void);
void triggerPri_Nissan360(void);
void triggerSec_Nissan360(void);
uint16_t getRPM_Nissan360(void);
int getCrankAngle_Nissan360(void);
void triggerSetEndTeeth_Nissan360(void);

void triggerSetup_Subaru67(void);
void triggerPri_Subaru67(void);
void triggerSec_Subaru67(void);
uint16_t getRPM_Subaru67(void);
int getCrankAngle_Subaru67(void);
void triggerSetEndTeeth_Subaru67(void);

void triggerSetup_Daihatsu(void);
void triggerPri_Daihatsu(void);
void triggerSec_Daihatsu(void);
uint16_t getRPM_Daihatsu(void);
int getCrankAngle_Daihatsu(void);
void triggerSetEndTeeth_Daihatsu(void);

void triggerSetup_Harley(void);
void triggerPri_Harley(void);
void triggerSec_Harley(void);
uint16_t getRPM_Harley(void);
int getCrankAngle_Harley(void);
void triggerSetEndTeeth_Harley(void);

void triggerSetup_ThirtySixMinus222(void);
void triggerPri_ThirtySixMinus222(void);
void triggerSec_ThirtySixMinus222(void);
uint16_t getRPM_ThirtySixMinus222(void);
int getCrankAngle_ThirtySixMinus222(void);
void triggerSetEndTeeth_ThirtySixMinus222(void);

void triggerSetup_ThirtySixMinus21(void);
void triggerPri_ThirtySixMinus21(void);
void triggerSec_ThirtySixMinus21(void);
uint16_t getRPM_ThirtySixMinus21(void);
int getCrankAngle_ThirtySixMinus21(void);
void triggerSetEndTeeth_ThirtySixMinus21(void);

void triggerSetup_420a(void);
void triggerPri_420a(void);
void triggerSec_420a(void);
uint16_t getRPM_420a(void);
int getCrankAngle_420a(void);
void triggerSetEndTeeth_420a(void);

void triggerPri_Webber(void);
void triggerSec_Webber(void);

void triggerSetup_FordST170(void);
void triggerSec_FordST170(void);
uint16_t getRPM_FordST170(void);
int getCrankAngle_FordST170(void);
void triggerSetEndTeeth_FordST170(void);

void triggerSetup_DRZ400(void);
void triggerSec_DRZ400(void);

void triggerSetup_NGC(void);
void triggerPri_NGC(void);
void triggerSec_NGC4(void);
void triggerSec_NGC68(void);
uint16_t getRPM_NGC(void);
void triggerSetEndTeeth_NGC(void);

void triggerSetup_Renix(void);
void triggerPri_Renix(void);
void triggerSetEndTeeth_Renix(void);

void triggerSetup_RoverMEMS(void);
void triggerPri_RoverMEMS(void);
void triggerSec_RoverMEMS(void);
uint16_t getRPM_RoverMEMS(void);
int getCrankAngle_RoverMEMS(void);
void triggerSetEndTeeth_RoverMEMS(void);

void triggerSetup_Vmax(void);
void triggerPri_Vmax(void);
void triggerSec_Vmax(void);
uint16_t getRPM_Vmax(void);
int getCrankAngle_Vmax(void);
void triggerSetEndTeeth_Vmax(void);

void triggerSetup_SuzukiK6A(void);
void triggerPri_SuzukiK6A(void);
void triggerSec_SuzukiK6A(void);
uint16_t getRPM_SuzukiK6A(void);
int getCrankAngle_SuzukiK6A(void);
void triggerSetEndTeeth_SuzukiK6A(void);

void triggerSetup_FordTFI(void);
void triggerPri_FordTFI(void);
void triggerSec_FordTFI(void);
uint16_t getRPM_FordTFI(void);
int getCrankAngle_FordTFI(void);
void triggerSetEndTeeth_FordTFI(void);

/**
 * @brief This function is called when the engine is stopped, or when the engine is started. It resets the decoder state and the tooth tracking variables
 *
 * @return void
 */
void resetDecoder(void);

void initDecoderPins(uint8_t primaryPin, uint8_t secondaryPin, uint8_t tertiaryPin);

extern volatile unsigned long curTime;
extern volatile unsigned long curGap;
extern volatile unsigned long curTime2;
extern volatile unsigned long curGap2;
extern volatile unsigned long lastGap;
extern volatile unsigned long targetGap;

extern volatile uint16_t toothCurrentCount; //The current number of teeth (Once sync has been achieved, this can never actually be 0
extern volatile unsigned long toothSystemLastToothTime; //As below, but used for decoders where not every tooth count is used for calculation
extern volatile unsigned long toothLastThirdToothTime; //The time (micros()) that the last tooth was registered on the second cam input
extern volatile unsigned long toothLastMinusOneToothTime; //The time (micros()) that the tooth before the last tooth was registered
extern volatile unsigned long toothLastMinusOneSecToothTime; //The time (micros()) that the tooth before the last tooth was registered on secondary input
extern volatile unsigned long targetGap2;

extern volatile unsigned long toothOneTime; //The time (micros()) that tooth 1 last triggered
extern volatile unsigned long toothOneMinusOneTime; //The 2nd to last time (micros()) that tooth 1 last triggered
extern volatile bool revolutionOne; // For sequential operation, this tracks whether the current revolution is 1 or 2 (not 1)

extern volatile unsigned long secondaryLastToothTime; //The time (micros()) that the last tooth was registered (Cam input)
extern volatile unsigned long secondaryLastToothTime1; //The time (micros()) that the last tooth was registered (Cam input)

extern uint16_t triggerActualTeeth;
extern volatile unsigned long triggerFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering)
extern volatile unsigned long triggerSecFilterTime; // The shortest time (in uS) that pulses will be accepted (Used for debounce filtering) for the secondary input
extern unsigned int triggerSecFilterTime_duration; // The shortest valid time (in uS) pulse DURATION
extern volatile uint16_t triggerToothAngle; //The number of crank degrees that elapse per tooth
extern unsigned long elapsedTime;
extern unsigned long lastCrankAngleCalc;
extern unsigned long lastVVTtime; //The time between the vvt reference pulse and the last crank pulse

extern uint16_t ignition1EndTooth;
extern uint16_t ignition2EndTooth;
extern uint16_t ignition3EndTooth;
extern uint16_t ignition4EndTooth;
extern uint16_t ignition5EndTooth;
extern uint16_t ignition6EndTooth;
extern uint16_t ignition7EndTooth;
extern uint16_t ignition8EndTooth;

extern int16_t toothAngles[24]; //An array for storing fixed tooth angles. Currently sized at 24 for the GM 24X decoder, but may grow later if there are other decoders that use this style

#define CRANK_SPEED 0U
#define CAM_SPEED   1U

#define TOOTH_CRANK 0
#define TOOTH_CAM_SECONDARY 1
#define TOOTH_CAM_TERTIARY  2

// used by the ROVER MEMS pattern
#define ID_TOOTH_PATTERN 0 // have we identified teeth to skip for calculating RPM?
#define SKIP_TOOTH1 1
#define SKIP_TOOTH2 2
#define SKIP_TOOTH3 3
#define SKIP_TOOTH4 4

#endif
