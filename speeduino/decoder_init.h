#pragma once

#include <stdint.h>

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

/** @brief This constant represents no trigger edge */
static constexpr uint8_t TRIGGER_EDGE_NONE = 99;

/** @brief This structure represents a trigger interrupt */
struct interrupt_t
{
  /** @brief The callback function to be called on interrupt */
  void (*callback)(void);
  /** @brief The edge type for the interrupt. E.g. RISING, FALLING, CHANGE */
  uint8_t edge;

  /** @brief Attach the interrupt to a pin */
  void attach(uint8_t pin) const;

  /** @brief Detach the interrupt from a pin */
  void detach(uint8_t pin) const;
};

/** @brief This structure represents a decoder configuration */
struct decoder_t
{
  /** @brief The primary interrupt configuration - usually the crank trigger */
  interrupt_t primary;
  /** @brief The secondary interrupt configuration - usually the cam trigger */
  interrupt_t secondary;
  /** @brief The tertiary interrupt configuration - for decoders that use a 3rd input. E.g. VVT */
  interrupt_t tertiary;

  /** @brief The function to get the RPM */
  uint16_t (*getRPM)(void);
  /** @brief The function to get the crank angle */
  int (*getCrankAngle)(void);
  /** @brief The function to set the end teeth for ignition calculations */
  void (*setEndTeeth)(void);  
};
const decoder_t& getDecoder(void);

void initialiseDecoder(uint8_t decoderType);
