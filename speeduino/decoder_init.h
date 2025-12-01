/**
 * @file 
 * @brief Decoder initialization and management 
 */
#pragma once

#include "decoder_t.h"

/// @brief Decoder type definitions
///
/// These constants are used to identify the various supported decoder types. 
/// They **must** be kept in sync with:
///   * the corresponding options in the ini file.
///   * the corresponding array in decoder_init.cpp
/// @{
constexpr uint8_t DECODER_MISSING_TOOTH     = 0;
constexpr uint8_t DECODER_BASIC_DISTRIBUTOR = 1;
constexpr uint8_t DECODER_DUAL_WHEEL        = 2;
constexpr uint8_t DECODER_GM7X              = 3;
constexpr uint8_t DECODER_4G63              = 4;
constexpr uint8_t DECODER_24X               = 5;
constexpr uint8_t DECODER_JEEP2000          = 6;
constexpr uint8_t DECODER_AUDI135           = 7;
constexpr uint8_t DECODER_HONDA_D17         = 8;
constexpr uint8_t DECODER_MIATA_9905        = 9;
constexpr uint8_t DECODER_MAZDA_AU          = 10;
constexpr uint8_t DECODER_NON360            = 11;
constexpr uint8_t DECODER_NISSAN_360        = 12;
constexpr uint8_t DECODER_SUBARU_67         = 13;
constexpr uint8_t DECODER_DAIHATSU_PLUS1    = 14;
constexpr uint8_t DECODER_HARLEY            = 15;
constexpr uint8_t DECODER_36_2_2_2          = 16;
constexpr uint8_t DECODER_36_2_1            = 17;
constexpr uint8_t DECODER_420A              = 18;
constexpr uint8_t DECODER_WEBER             = 19;
constexpr uint8_t DECODER_ST170             = 20;
constexpr uint8_t DECODER_DRZ400            = 21;
constexpr uint8_t DECODER_NGC               = 22;
constexpr uint8_t DECODER_VMAX              = 23;
constexpr uint8_t DECODER_RENIX             = 24;
constexpr uint8_t DECODER_ROVERMEMS         = 25;
constexpr uint8_t DECODER_SUZUKI_K6A        = 26;
constexpr uint8_t DECODER_HONDA_J32         = 27;
constexpr uint8_t DECODER_FORD_TFI          = 28;
/// @}

/** @brief Get the current decoder configuration */
const decoder_t& getDecoder(void);

/** @brief Set the current decoder configuration */
void setDecoder(uint8_t decoderType);
