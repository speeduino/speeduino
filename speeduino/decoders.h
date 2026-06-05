#ifndef DECODERS_H
#define DECODERS_H

#include <stdint.h>
#include "decoder_t.h"

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
