#ifndef DECODERS_H
#define DECODERS_H

#include <SimplyAtomic.h>
#include <stdint.h>
#include <SimplyAtomic.h>
#include "decoder_t.h"

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
decoder_t triggerSetup_missingTooth(void);
decoder_t triggerSetup_DualWheel(void);
decoder_t triggerSetup_BasicDistributor(void);
decoder_t triggerSetup_GM7X(void);
decoder_t triggerSetup_4G63(void);
decoder_t triggerSetup_24X(void);
decoder_t triggerSetup_Jeep2000(void);
decoder_t triggerSetup_Audi135(void);
decoder_t triggerSetup_HondaD17(void);
decoder_t triggerSetup_HondaJ32(void);
decoder_t triggerSetup_Miata9905(void);
int getCamAngle_Miata9905(void);
decoder_t triggerSetup_MazdaAU(void);
decoder_t triggerSetup_non360(void);
decoder_t triggerSetup_Nissan360(void);
decoder_t triggerSetup_Subaru67(void);
decoder_t triggerSetup_Daihatsu(void);
decoder_t triggerSetup_Harley(void);
decoder_t triggerSetup_ThirtySixMinus222(void);
decoder_t triggerSetup_ThirtySixMinus21(void);
decoder_t triggerSetup_420a(void);
decoder_t triggerSetup_Webber(void);
decoder_t triggerSetup_FordST170(void);
decoder_t triggerSetup_DRZ400(void);
decoder_t triggerSetup_NGC(void);
decoder_t triggerSetup_Renix(void);
decoder_t triggerSetup_RoverMEMS(void);
decoder_t triggerSetup_Vmax(void);
decoder_t triggerSetup_SuzukiK6A(void);
decoder_t triggerSetup_FordTFI(void);

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
