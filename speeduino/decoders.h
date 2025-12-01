#ifndef DECODERS_H
#define DECODERS_H

#include <SimplyAtomic.h>
#include <stdint.h>
#include "decoder_t.h"

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
// TODO: move to decoder_t
bool engineIsRunning(uint32_t curTime);

// TODO: move these to logger.cpp
void loggerPrimaryISR(void);
void loggerSecondaryISR(void);
void loggerTertiaryISR(void);

/// @brief Setup function for each decoder type.
/// @{
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
/// @}

// TODO: use same VVT scheme as other decoders
int getCamAngle_Miata9905(void);

/** @brief Set the input pins for the decoders. Pin numbers are pulled from the tune
 * 
 * @param primaryPin Primary pin - usually the crank trigger
 * @param secondaryPin Secondary pin - optional, usually the cam trigger
 * @param tertiaryPin Tertiary pin - optional, for decoders that use a 3rd input. E.g. VVT
 */
void initDecoderPins(uint8_t primaryPin, uint8_t secondaryPin, uint8_t tertiaryPin);

#endif
