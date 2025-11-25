/**
 * @file 
 * @brief Decoder initialization and management 
 */
#pragma once

#include "decoder_t.h"

/// @brief Decoder type definitions
///
/// These constants are used to identify the various supported decoder types. 
/// They **must** be kept in sync with the corresponding options in the ini file.
/// @{
#define DECODER_MISSING_TOOTH     0
#define DECODER_BASIC_DISTRIBUTOR 1
#define DECODER_DUAL_WHEEL        2
#define DECODER_GM7X              3
#define DECODER_4G63              4
#define DECODER_24X               5
#define DECODER_JEEP2000          6
#define DECODER_AUDI135           7
#define DECODER_HONDA_D17         8
#define DECODER_MIATA_9905        9
#define DECODER_MAZDA_AU          10
#define DECODER_NON360            11
#define DECODER_NISSAN_360        12
#define DECODER_SUBARU_67         13
#define DECODER_DAIHATSU_PLUS1    14
#define DECODER_HARLEY            15
#define DECODER_36_2_2_2          16
#define DECODER_36_2_1            17
#define DECODER_420A              18
#define DECODER_WEBER             19
#define DECODER_ST170             20
#define DECODER_DRZ400            21
#define DECODER_NGC               22
#define DECODER_VMAX              23
#define DECODER_RENIX             24
#define DECODER_ROVERMEMS         25
#define DECODER_SUZUKI_K6A        26
#define DECODER_HONDA_J32         27
#define DECODER_FORD_TFI          28
/// @}

/** @brief Get the current decoder configuration */
const decoder_t& getDecoder(void);

/** @brief Set the current decoder configuration */
void setDecoder(uint8_t decoderType);
